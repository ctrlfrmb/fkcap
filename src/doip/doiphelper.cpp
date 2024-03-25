#include <QMessageBox>
#include <QTimer>
#include <qDebug>

#include "doip/doiphelper.h"
#include "doip/doipclientconfig.h"
#include "doip/doipserverconfig.h"

// DoIPInfoDisplayWindow implementation
DoIPInfoDisplayWindow::DoIPInfoDisplayWindow(QWidget *parent)
    : QWidget(parent)
    , table(new QTableWidget(this))
    , button(new QPushButton("Button Text", this))  // add this line, replace Button Text with your button text
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(table);
    layout->addWidget(button);  // add this line

    button->setEnabled(false);  // disable button initially

    // connect button clicked signal to your slot function
    connect(button, &QPushButton::clicked, this, &DoIPInfoDisplayWindow::buttonClicked);
}

void DoIPInfoDisplayWindow::buttonClicked() {
    QString text = button->text();
    if (text == "sync info") {
        using namespace figkey;
        auto& server = DoIPServerConfig::Instance();

        QByteArray eid = QByteArray::fromHex(data["EID"].toUtf8());
        server.setEid(eid);

        QByteArray gid = QByteArray::fromHex(data["GID"].toUtf8());
        server.setGid(gid);

        QByteArray vin = QByteArray::fromHex(data["VIN"].toUtf8());
        server.setVin(vin);

        server.save();
    }
    this->close();
}

void DoIPInfoDisplayWindow::displayInfo(int type, const QMap<QString, QString>& info) {
    data = info;
    table->setRowCount(data.size());
    table->setColumnCount(2);

    QStringList headers;
    headers << "Attribute" << "Value";
    table->setHorizontalHeaderLabels(headers);

    int row = 0;
    for(auto i = data.begin(); i != data.end(); ++i) {
        QTableWidgetItem* attributeItem = new QTableWidgetItem(i.key());
        table->setItem(row, 0, attributeItem);

        QTableWidgetItem* valueItem = new QTableWidgetItem(i.value());
        table->setItem(row, 1, valueItem);

        ++row;
    }

    switch (type) {
    case DOIP_VEHICLE_ANNOUNCEMENT:
        setWindowTitle("Vehicle annoucement information");
        button->setText("sync info");
        button->setToolTip("Information is synchronized to the configuration file");
        button->setEnabled(true);
        break;
    default:
        setWindowTitle("Vehicle information");
        button->setEnabled(false);
        break;
    }

    table->setColumnWidth(0, 195);
    table->setColumnWidth(1, 400);
    this->show();
}

DoIPHelper::DoIPHelper(Ui::NetworkAssistWindow *ui, QObject *parent)
    : QObject(parent), ui(ui)
{
    display = new DoIPInfoDisplayWindow();
    display->setFixedSize(640, 480);
}

DoIPHelper::~DoIPHelper() {
    //delete display;
    stopTimer();
}

bool DoIPHelper::startTimer(int interval, int type) {
    stopTimer();

    timer = new QTimer(this);
    timer->setSingleShot(true); // 单次触发
    timer->setInterval(interval); // 设置定时间隔

    connect(timer, &QTimer::timeout, this, [this, type]() {
        if (this->isRequest) {
            this->isRequest = false;
            switch (type) {
            case DOIP_ROUTING_ACTIVATION_REQUEST:
                this->endTest();
                QMessageBox::critical(nullptr, "Routing activation request failed", "Route activation response timeout");
                break;
            case DOIP_VEHICLE_IDENTIFICATION_REQUEST:
                QMessageBox::critical(nullptr, "Request vehicle information failed", "Timeout waiting for vehicle response");
                break;
            case DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_EID:
                QMessageBox::critical(nullptr, "Request vehicle information with EID failed", "Timeout waiting for vehicle response");
                break;
            case DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_VIN:
                QMessageBox::critical(nullptr, "Request vehicle information with VIN failed", "Timeout waiting for vehicle response");
                break;
            default:
                break;
            }
        }
    });

    timer->start();
    return true;
}

bool DoIPHelper::stopTimer() {
    if (timer) {
        timer->stop();
        timer->deleteLater();
        timer = nullptr;
    }

    return false;
}

bool DoIPHelper::endTest() {
    isRequest = false;
    ui->buttonConnect->setEnabled(true);
    return false;
}

bool DoIPHelper::startTest() {
    isRequest = false;
    isRoutingActivation = true;
    ui->buttonDisconnect->setEnabled(true);
    ui->buttonSend->setEnabled(true);
    return true;
}

bool DoIPHelper::start(BaseComm *handle) {
    comm = handle;
    isRequest = true;
    isRoutingActivation = false;
    if (!comm->sendData(DoIPPacketCommon::ConstructRoutingActivationRequest())) {
        QMessageBox::critical(nullptr, "Routing activation request failed", comm->getLastError());
        return endTest();
    }

    using namespace figkey;
    auto& client = DoIPClientConfig::Instance();
    if (!client.getRequireRoutingActivation()) {
        return startTest();
    }

    int interval = client.getRoutingActivationWaitTime() > 50 ? client.getRoutingActivationWaitTime() : 2000;
    return startTimer(interval, DOIP_ROUTING_ACTIVATION_REQUEST);
}

bool DoIPHelper::hasRequst() const {
    if (isRoutingActivation || isRequest)
        return true;

    return false;
}

bool DoIPHelper::hasActivated() const {
    return isRoutingActivation;
}

bool DoIPHelper::parseVehicleAnnouncement(const QByteArray& payload) {
    isRequest = stopTimer();

    if (payload.size() < DOIP_VEHICLE_ANNOUNCEMENT_MIN_LENGTH) {
        QMessageBox::critical(nullptr, "Vehicle identify failed", "Vehicle announcement response data is invalid");
        return false;
    }

    if (display) {
        display->displayInfo(DOIP_VEHICLE_ANNOUNCEMENT, DoIPPacketCommon::ParseVehicleAnnouncementInformation(payload));
    }
    qDebug() <<"Vehicle announcement: "<<isRequest;
    return true;
}

bool DoIPHelper::parseRoutingActivationResponse(const QByteArray& payload) {
    isRequest = stopTimer();

    if (payload.size() < DOIP_ROUTE_ACTIVATION_RESPONSE_MIN_LENGTH) {
        QMessageBox::critical(nullptr, "Route activation failed", "Route activation response data is invalid");
        return endTest();
    }

    switch (payload[4])
    {
    case DOIP_ROUTING_ACTIVATION_SUCCESSFULLY_ACTIVATED:
        startTest();
        QMessageBox::information(nullptr, "Route Activation", "Route activation is successful, please start diagnosis");
        return true;

    case DOIP_ROUTING_ACTIVATION_DENIED_UNKNOWN_SA:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Unknown source address");
        break;

    case DOIP_ROUTING_ACTIVATION_DENIED_ALL_SOCKETS_REGISTERED:
        QMessageBox::critical(nullptr, "Route Activation Failed", "All sockets registered");
        break;

    case DOIP_ROUTING_ACTIVATION_DENIED_SA_DIFFERENT:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Source address different");
        break;

    case DOIP_ROUTING_ACTIVATION_DENIED_SA_ALREADY_REGISTERED_AND_ACTIVE:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Source address already registered and active");
        break;

    case DOIP_ROUTING_ACTIVATION_DENIED_REJECTED_CONFIRMATION:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Confirmation rejected");
        break;

    case DOIP_ROUTING_ACTIVATION_DENIED_UNSUPPORTED_ROUTING_ACTIVATION_TYPE:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Unsupported routing activation type");
        break;

    case DOIP_ROUTING_ACTIVATION_DENIED_MISSING_AUTHENTICATION:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Missing Authentication");
        break;

    case DOIP_ROUTING_ACTIVATION_WILL_ACTIVATED_CONFIRMATION_REQUIRED:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Confirmation Required");
        break;

    default:
        QMessageBox::critical(nullptr, "Route Activation Failed", "Unknown type");
        break;
    }
    return endTest();
}

bool DoIPHelper::responseAliveCheckRequest() {
    if (!figkey::DoIPClientConfig::Instance().getAliveCheckResponse()) {
        return true;
    }

    if (!comm->sendData(DoIPPacketCommon::ConstructAliveCheckResponse())) {
        qDebug() <<"Alive check repsonse failed: "<<comm->getLastError();
        return false;
    }

    qDebug() <<"Alive check repsonse";
    return true;
}

void DoIPHelper::parseMessage(const QByteArray& data) {
    DoIPPacketCommon packet(data);
    switch (packet.GetPayloadType()) {
    case DOIP_ROUTING_ACTIVATION_RESPONSE:
        parseRoutingActivationResponse(packet.GetPayloadMessage());
        break;
    case DOIP_VEHICLE_ANNOUNCEMENT:
        parseVehicleAnnouncement(packet.GetPayloadMessage());
        break;
    case DOIP_ROUTING_ACTIVATION_REQUEST:
        responseAliveCheckRequest();
        break;
    default:
        break;
    }
}
