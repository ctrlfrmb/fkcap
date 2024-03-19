#include <QMessageBox>
#include <QTimer>
#include <qDebug>

#include "doip/doiphelper.h"
#include "doip/doipclientconfig.h"

DoIPHelper::DoIPHelper(Ui::NetworkAssistWindow *ui, QObject *parent)
    : QObject(parent), ui(ui)
{
}

DoIPHelper::~DoIPHelper() {
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
        QMessageBox::critical(nullptr, "routing activation request failed", comm->getLastError());
        return endTest();
    }

    using namespace figkey;
    // Fetch the singleton configuration object
    auto& config = DoIPClientConfig::Instance();

    if (!config.getRequireRoutingActivation()) {
        return startTest();
    }

    int interval = config.getRoutingActivationWaitTime() > 50 ? config.getRoutingActivationWaitTime() : 2000;
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true); // 单次触发
    timer->setInterval(interval); // 设置定时间隔

    connect(timer, &QTimer::timeout, this, [this]() {
        // 定时到达后处理逻辑
        if (!isRoutingActivation) {
            endTest();
            QMessageBox::critical(nullptr, "routing activation request failed", "Route activation response timeout");
        }
    });

    timer->start();
    return true;
}

const bool& DoIPHelper::hasRequst() const {
    return isRequest;
}

const bool& DoIPHelper::hasActivated() const {
    return isRoutingActivation;
}

bool DoIPHelper::parseRoutingActivationResponse(const QByteArray& payload) {
    if (payload.size() < DOIP_ROUTE_ACTIVATION_RESPONSE_MIN_LENGTH) {
        QMessageBox::critical(nullptr, "Route activation failed", "Route activation response data is invalid");
        return endTest();
    }

    switch (payload[4])
    {
    case DOIP_ROUTING_ACTIVATION_SUCCESSFULLY_ACTIVATED:
        startTest();
        QMessageBox::information(nullptr, "Route Activation", "Successfully activated");
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
    case DOIP_ROUTING_ACTIVATION_REQUEST:
        responseAliveCheckRequest();
        break;
    default:
        break;
    }
}
