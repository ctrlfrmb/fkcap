#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QValidator>
#include <QByteArray>
#include <QMap>

#include "vehicleidentifywindow.h"
#include "doip/doipgenericheaderhandler.h"
#include "doip/doipclientconfig.h"
#include "doip/doipserverconfig.h"

// Add other required includes here...

VehicleIdentifyWindow::VehicleIdentifyWindow(QWidget* parent)
    : QDialog(parent)
    , tableSetting(new QTableWidget(4, 2, this))
    , treeReceive(new QTreeWidget(this))
    , settingBox(new QGroupBox("Settings", this))
    , buttonBox(new QGroupBox("Actions", this))
    , receiveBox(new QGroupBox("Received Vehicles", this))
    , udpSocket(new QUdpSocket(this))
    , timer(new QTimer(this))
{
    this->setModal(true);
    initWindow();
    connect(udpSocket, &QUdpSocket::readyRead, this, &VehicleIdentifyWindow::onReadyRead);
}

VehicleIdentifyWindow::~VehicleIdentifyWindow()= default;

void VehicleIdentifyWindow::addTreeItem(const QMap<QString, QString>& info) {
    // Find the ECU item with the same name as the ECU in the map
    QTreeWidgetItem *ecuItem = nullptr;
    for (int i = 0; i < treeReceive->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = treeReceive->topLevelItem(i);
        if (item->text(1) == info[DOIP_VEHICLE_LOGIC_ADDRESS_ATTRIBUTE]) {
            ecuItem = item;
            break;
        }
    }

    // If the ECU item doesn't exist, create it
    if (!ecuItem) {
        ecuItem = new QTreeWidgetItem(treeReceive);
        ecuItem->setText(0, "ECU ");
        ecuItem->setText(1, info[DOIP_VEHICLE_LOGIC_ADDRESS_ATTRIBUTE]);
        ecuItem->setFlags(ecuItem->flags() & ~Qt::ItemIsEditable);  // Make the first column non-editable
    }

    // Update the child items with the values from the map
    for (auto it = info.begin(); it != info.end(); ++it) {
        // Find the child item with the same key as the map key
        QTreeWidgetItem *childItem = nullptr;
        for (int i = 0; i < ecuItem->childCount(); ++i) {
            QTreeWidgetItem *item = ecuItem->child(i);
            if (item->text(0) == it.key()) {
                childItem = item;
                break;
            }
        }

        // If the child item doesn't exist, create it
        if (!childItem) {
            childItem = new QTreeWidgetItem(ecuItem);
            childItem->setText(0, it.key());
            childItem->setFlags(childItem->flags() | Qt::ItemIsEditable);  // Make the second column editable
        }

        // Update the child item's value
        childItem->setText(1, it.value());
    }
}

void VehicleIdentifyWindow::updateParameterValidator(int requestType) {
    QLineEdit *lineEditRequestParameter = qobject_cast<QLineEdit*>(tableSetting->cellWidget(3, 1));
    if (!lineEditRequestParameter) {
        QMessageBox::critical(this, "Error", "Request Parameter field is missing.");
        return;
    }

    switch (requestType) {
    case 1: // eid
        lineEditRequestParameter->setEnabled(true);
        // eid should be a 6-byte hexadecimal: matches exactly 6 pairs of [0-9A-Fa-f], separated by a space
        lineEditRequestParameter->setValidator(new QRegExpValidator(QRegExp("^([0-9A-Fa-f]{2}\\s){5}[0-9A-Fa-f]{2}$"), this));
        break;
    case 2: // vin
        lineEditRequestParameter->setEnabled(true);
        // vin should be a 17-byte hexadecimal: matches exactly 17 pairs of [0-9A-Fa-f], separated by a space
        lineEditRequestParameter->setValidator(new QRegExpValidator(QRegExp("^([0-9A-Fa-f]{2}\\s){16}[0-9A-Fa-f]{2}$"), this));
        break;
    default: // default
        lineEditRequestParameter->clear();
        lineEditRequestParameter->setEnabled(false);
        break;
    }
}

void VehicleIdentifyWindow::initTableSetting() {
    QStringList headerLabels {"Attribute", "Value"};
    tableSetting->setColumnCount(2);
    tableSetting->setRowCount(4);
    tableSetting->setHorizontalHeaderLabels(headerLabels);

    QLineEdit *lineEditIp = new QLineEdit("255.255.255.255", this);
    QValidator *validatorIp = new QRegExpValidator(QRegExp("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"), this);
    lineEditIp->setValidator(validatorIp);
    tableSetting->setItem(0, 0, new QTableWidgetItem("IP Address"));
    tableSetting->setCellWidget(0, 1, lineEditIp);

    QLineEdit *lineEditPort = new QLineEdit("13400", this);
    QValidator *validatorPort = new QIntValidator(0, 65535, this);
    lineEditPort->setValidator(validatorPort);
    tableSetting->setItem(1, 0, new QTableWidgetItem("IP Port"));
    tableSetting->setCellWidget(1, 1, lineEditPort);

    QComboBox *comboBox = new QComboBox();
    comboBox->addItem("default");
    comboBox->addItem("eid");
    comboBox->addItem("vin");
    tableSetting->setItem(2, 0, new QTableWidgetItem("Request With"));
    tableSetting->setCellWidget(2, 1, comboBox);

    QLineEdit *lineEditRequestParameter = new QLineEdit("", this);
    QValidator *validatorParameter = new QRegExpValidator(QRegExp("^[0-9a-fA-F\\s]*$"), this);
    lineEditRequestParameter->setValidator(validatorParameter);
    lineEditRequestParameter->setEnabled(false);
    tableSetting->setItem(3, 0, new QTableWidgetItem("Request Parameter"));
    tableSetting->setCellWidget(3, 1, lineEditRequestParameter);

    tableSetting->setColumnWidth(0, 155);
    tableSetting->setColumnWidth(1, 300);

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &VehicleIdentifyWindow::updateParameterValidator);

    settingBox->setLayout(new QVBoxLayout());
    settingBox->layout()->addWidget(tableSetting);
}

void VehicleIdentifyWindow::initActions() {
    QPushButton* requestButton = new QPushButton("Request", this);
    connect(requestButton, &QPushButton::clicked, this, &VehicleIdentifyWindow::onRequestButtonClicked);
    QPushButton* diagnoseButton = new QPushButton("Diagnose", this);
    connect(diagnoseButton, &QPushButton::clicked, this, &VehicleIdentifyWindow::onDiagnoseButtonClicked);
    QPushButton* updateButton = new QPushButton("Update", this);
    connect(updateButton, &QPushButton::clicked, this, &VehicleIdentifyWindow::onUpdateButtonClicked);

    QHBoxLayout* buttonBoxLayout = new QHBoxLayout();
    buttonBoxLayout->addWidget(requestButton);
    buttonBoxLayout->addWidget(diagnoseButton);
    buttonBoxLayout->addWidget(updateButton);
    buttonBox->setLayout(buttonBoxLayout);
}

void VehicleIdentifyWindow::initTreeReceive() {
    treeReceive->setHeaderLabels({"Name", "Value"});
    treeReceive->setColumnWidth(0, 155);
    treeReceive->setColumnWidth(1, 300);

    receiveBox->setLayout(new QVBoxLayout());
    receiveBox->layout()->addWidget(treeReceive);
}

void VehicleIdentifyWindow::initWindow() {
    resize(640, 480);

    initTableSetting();
    initActions();
    initTreeReceive();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(settingBox);
    layout->addWidget(buttonBox);
    layout->addWidget(receiveBox);

    connect(timer, &QTimer::timeout, this, &VehicleIdentifyWindow::onTimeout);
}

void VehicleIdentifyWindow::onRequestButtonClicked()
{
    QLineEdit* lineEditIp = qobject_cast<QLineEdit*>(tableSetting->cellWidget(0, 1));
    QLineEdit* lineEditPort = qobject_cast<QLineEdit*>(tableSetting->cellWidget(1, 1));
    QComboBox* comboBox = qobject_cast<QComboBox*>(tableSetting->cellWidget(2, 1));
    QLineEdit* lineEditRequestParameter = qobject_cast<QLineEdit*>(tableSetting->cellWidget(3, 1));

    // Check whether the QLineEdit and QComboBox widgets are available
    if(!lineEditIp || !lineEditPort || !comboBox || !lineEditRequestParameter) {
        QMessageBox::critical(this, "Error", "One or more required fields are missing.");
        return;
    }

    QHostAddress ipAddress(lineEditIp->text());
    bool okPort;
    quint16 port = lineEditPort->text().toUInt(&okPort);
    if(!okPort) {
        QMessageBox::critical(this, "Error", "Invalid port number. Please input a valid value.");
        return;
    }

    QByteArray param = QByteArray::fromHex(lineEditRequestParameter->text().simplified().toUtf8());
    QByteArray data;
    int requestType = comboBox->currentIndex();
    switch (requestType) {
        case 1:
            if (param.size() != 6) {
                QMessageBox::critical(this, "Error", "Invalid eid. Please input a valid value like 00 0c 29 bb 0e c5");
                return;
            }
            data = DoIPPacketCommon::ConstructVehicleIdentificationRequestWithEid(param);
            break;
        case 2:
            if (param.size() != 17) {
                QMessageBox::critical(this, "Error", "Invalid vin. Please input a valid value like 56 49 4e 30 31 32 33 34 35 36 37 38 39 39 39 39 39");
                return;
            }
            data = DoIPPacketCommon::ConstructVehicleIdentificationRequestWithVin(param);
            break;
        default:
            data = DoIPPacketCommon::ConstructVehicleIdentificationRequest();
            break;
    }

    treeReceive->clear();
    udpSocket->abort();
    udpSocket->bind(QHostAddress::Any, port);
    if(udpSocket->writeDatagram(data, ipAddress, port) == -1) {
        QMessageBox::critical(this, "Error", "Data transmission failed: " + udpSocket->errorString());
    } else {
        startTimer();
    }
}

void VehicleIdentifyWindow::onDiagnoseButtonClicked()
{
    // Check if an item is selected
    if (treeReceive->selectedItems().count() == 0) {
        QMessageBox::warning(this, "Warn", "Please select the ecu or downstream node that needs to be diagnosed");
        return;
    }

    // Get the selected tree item
    QTreeWidgetItem *selectedItem = treeReceive->selectedItems().first();
    if (selectedItem->parent()) {
        selectedItem = selectedItem->parent();
    }
    // Get the map of information for the selected ECU
    QMap<QString, QString> info;
    info.insert(selectedItem->text(0), selectedItem->text(1));
    for (int i = 0; i < selectedItem->childCount(); ++i) {
        QTreeWidgetItem *childItem = selectedItem->child(i);
        info.insert(childItem->text(0), childItem->text(1));
    }

    // Update the configuration file with the ECU information
    using namespace figkey;
    auto& client = DoIPClientConfig::Instance();
    auto& server = DoIPServerConfig::Instance();
    if (info.contains(DOIP_VEHICLE_LOGIC_ADDRESS_ATTRIBUTE)) {
        client.setTargetAddress(info[DOIP_VEHICLE_LOGIC_ADDRESS_ATTRIBUTE].toInt(nullptr, 16));
    }
    client.save();
    server.setIpAddress(info[DOIP_VEHICLE_IP_ATTRIBUTE]);
    server.setTcpPort(info[DOIP_VEHICLE_PORT_ATTRIBUTE].toUShort());
    server.setEid(QByteArray::fromHex(info[DOIP_VEHICLE_EID_ATTRIBUTE].toUtf8()));
    server.setGid(QByteArray::fromHex(info[DOIP_VEHICLE_GID_ATTRIBUTE].toUtf8()));
    server.setVin(QByteArray::fromHex(info[DOIP_VEHICLE_VIN_ATTRIBUTE].toUtf8()));
    server.save();

    // Close the window
    treeReceive->clear();
    this->accept();
}

void VehicleIdentifyWindow::onUpdateButtonClicked()
{
    // TODO: Implement your update logic here...
}

void VehicleIdentifyWindow::onTimeout()
{
    stopTimer();
    QMessageBox::warning(this, "Request Timeout", "Vehicle identification request has timed out.");
}

void VehicleIdentifyWindow::startTimer()
{
    int interval = figkey::DoIPClientConfig::Instance().getControlTime() > 50 ? figkey::DoIPClientConfig::Instance().getControlTime() : 2000;
    timer->start(interval);
}

bool VehicleIdentifyWindow::stopTimer()
{
    if (timer->isActive()) {
        timer->stop();
        return true;
    }
    return false;
}

void VehicleIdentifyWindow::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        DoIPPacketCommon packet(datagram);
        if (DOIP_VEHICLE_ANNOUNCEMENT == packet.GetPayloadType()) {
            stopTimer();

            QMap<QString, QString> info = DoIPPacketCommon::ParseVehicleAnnouncementInformation(packet.GetPayloadMessage());
            sender.setAddress(sender.toIPv4Address());
            info.insert(DOIP_VEHICLE_IP_ATTRIBUTE, sender.toString());
            info.insert(DOIP_VEHICLE_PORT_ATTRIBUTE, QString::number(senderPort));
            addTreeItem(info);
        }
    }
}
