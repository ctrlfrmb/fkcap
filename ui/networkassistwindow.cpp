#include <QComboBox>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QtDebug>
#include <QMessageBox>
#include <QTime>
#include <QCheckBox>
#include <QTimer>

#include "networkassistwindow.h"
#include "ui_networkassistwindow.h"
#include "config.h"
#include "common/tcpcomm.h"
#include "common/udpcomm.h"

#define SET_PROTOCOL_LABEL "Protocol"
#define SET_CLIENT_IP_LABEL "Client IP"
#define SET_SERVER_IP_LABEL "Server IP"
#define SET_SERVER_PORT_LABEL "Server Port"
#define SET_DATA_TYPE_LABEL "Data Type"

NetworkAssistWindow::NetworkAssistWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkAssistWindow)
{
    ui->setupUi(this);

    initWindow();
}

NetworkAssistWindow::~NetworkAssistWindow()
{
    exitWindow();

    delete ui;
}

void NetworkAssistWindow::initListSetting() {
    // Create QListWidgetItem and corresponding custom QWidget for each setting.
    addSettingItem(false, SET_PROTOCOL_LABEL, {"TCP", "UDP"});
    QStringList clientIps;
    auto address = figkey::CaptureConfig::Instance().getConfigInfo().network.address;
    for (const auto& addr : address) {
        clientIps << QString(addr.ip.c_str());
    }
    addSettingItem(true, SET_CLIENT_IP_LABEL, clientIps);
    addSettingItem(true, SET_SERVER_IP_LABEL, {"192.168.1.1"});
    addSettingItem(true, SET_SERVER_PORT_LABEL, {"13400"});
    addSettingItem(false, SET_DATA_TYPE_LABEL, {"HEX", "ASCII"});
}

void NetworkAssistWindow::syncPort(int index)
{
   comboBoxes.last()->setCurrentIndex(index);
}

void NetworkAssistWindow::addSettingItem(bool isEdit, const QString& label, const QStringList& options) {
    QListWidgetItem* item = new QListWidgetItem(ui->listSetting);

    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* nameLabel = new QLabel(label);
    QComboBox* comboBox = new QComboBox();
    comboBox->addItems(options);
    comboBox->setEditable(isEdit);  // Allow user input

    // Connect currentIndexChanged signal to sync IP and Port
    if(label == SET_SERVER_IP_LABEL)
    {
        connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(syncPort(int)));
    }
    else if (label == SET_DATA_TYPE_LABEL)
    {
        // Connect the signal that fires when the ComboBox selection changes.
        connect(comboBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            [=](const QString& text) {
                // Update flag based on the selection.
                this->isASCII = (text == "ASCII");
        });
    }

    layout->addWidget(nameLabel);
    layout->addWidget(comboBox);
    layout->setAlignment(Qt::AlignVCenter);
    layout->setContentsMargins(5, 5, 5, 5);
    widget->setLayout(layout);

    item->setSizeHint(widget->sizeHint());
    ui->listSetting->setItemWidget(item, widget);

    if(label == SET_SERVER_IP_LABEL || label == SET_SERVER_PORT_LABEL)
    {
       comboBoxes.append(comboBox);
    }
}

void NetworkAssistWindow::initTableSend() {
    // Set the number of columns
    ui->tableSend->setColumnCount(6);

    // Set horizontal header labels
    QStringList headers;
    headers << "CK" << "Time" << "Data" << "Type" << "Next" << "Interval";
    ui->tableSend->setHorizontalHeaderLabels(headers);

    // Set column width
    ui->tableSend->setColumnWidth(0, 30);
    ui->tableSend->setColumnWidth(1, 100);
    ui->tableSend->setColumnWidth(2, 240);
    ui->tableSend->setColumnWidth(3, 70);
    ui->tableSend->setColumnWidth(4, 50);
    ui->tableSend->setColumnWidth(5, 60);

    // Initialize rows
    auto rows = figkey::CaptureConfig::Instance().getConfigInfo().sendRows;
    for (int i = 0; i < rows; i++) {
        // Insert a new row
        ui->tableSend->insertRow(i);

        // CK column
        QCheckBox *checkBox = new QCheckBox();
        // Create the QWidget and set its layout
        QWidget *widget = new QWidget(ui->tableSend);
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->addWidget(checkBox);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        widget->setLayout(layout);
        ui->tableSend->setCellWidget(i, 0, widget);

        // Timer column
        QTableWidgetItem *timeItem = new QTableWidgetItem();
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);  // Make the item not editable
        ui->tableSend->setItem(i, 1, timeItem);

        // Data column - Add an empty item
        QTableWidgetItem *dataItem = new QTableWidgetItem();
        ui->tableSend->setItem(i, 2, dataItem);

        // Type column
        QComboBox *comboBox = new QComboBox();
        comboBox->addItem("Send");
        comboBox->addItem("Receive");
        ui->tableSend->setCellWidget(i, 3, comboBox);
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [=](int index) {
                    if (index == 1) {
                        recvDataMap[i] = getSendData(i);
                    }
                    else {
                        recvDataMap.remove(i);
                    }
                }
        );

        // Next column - Add an empty item
        QTableWidgetItem *nextRowItem = new QTableWidgetItem();
        ui->tableSend->setItem(i, 4, nextRowItem);

        // Interval column - Add an empty item
        QTableWidgetItem *intervalItem = new QTableWidgetItem();
        ui->tableSend->setItem(i, 5, intervalItem);
    }
}

void NetworkAssistWindow::initTableReceive() {
    // Set the number of rows and columns
    ui->tableReceive->setColumnCount(3);

    // Set horizontal header labels
    QStringList headers;
    headers << "Time" << "Length" << "Data";
    ui->tableReceive->setHorizontalHeaderLabels(headers);

    // Set column width

    ui->tableReceive->setColumnWidth(0, 100);
    ui->tableReceive->setColumnWidth(1, 50);
    ui->tableReceive->setColumnWidth(2, 400);
}

void NetworkAssistWindow::initWindow() {
    initListSetting();
    initTableSend();
    initTableReceive();
}

void NetworkAssistWindow::exitWindow() {
    on_buttonDisconnect_clicked();
}

void NetworkAssistWindow::closeEvent(QCloseEvent *event) {
    exitWindow();
    event->accept();
}

void NetworkAssistWindow::setWindow(bool isService) {
    isServer = isService;
    if (isServer) {
        QIcon icon(":/images/resource/icons/server.png");
        setWindowIcon(icon);
        setWindowTitle("Network Assistants(Server)");
    }
}

void NetworkAssistWindow::setProtocol(uint8_t protocol, QComboBox* comboBox) {
    QString text{"TCP"};
    switch (protocol) {
        case figkey::PROTOCOL_TYPE_DOIP:
            return;
        case figkey::PROTOCOL_TYPE_UDP:
            text = "UDP";
        break;
    }

    if (!text.isEmpty()) {
        comboBox->setCurrentText(text);
        comboBox->setEnabled(false);
    }
}

void NetworkAssistWindow::setClientIP(QComboBox* comboBox, const figkey::PacketInfo& packet) {
    comboBox->clear();
    if (isServer) {
        comboBox->addItem(QString::fromStdString(packet.srcIP));
        comboBox->addItem(QString::fromStdString(packet.destIP));
    }
    else {
        auto address = figkey::CaptureConfig::Instance().getConfigInfo().network.address;
        for (const auto& addr : address) {
            comboBox->addItem(QString::fromStdString(addr.ip));
        }
    }
}

void NetworkAssistWindow::setServerIP(QComboBox* comboBox, const figkey::PacketInfo& packet) {
    comboBox->clear();
    if (isServer) {
        auto address = figkey::CaptureConfig::Instance().getConfigInfo().network.address;
        for (const auto& addr : address) {
            comboBox->addItem(QString::fromStdString(addr.ip));
        }
    }
    else {
        comboBox->addItem(QString::fromStdString(packet.srcIP));
        comboBox->addItem(QString::fromStdString(packet.destIP));
    }
}

void NetworkAssistWindow::setServerPort(QComboBox* comboBox, const figkey::PacketInfo& packet) {
    comboBox->clear();
    if (isServer) {
        uint16_t porst[5]{5431, 13400, 8080, 1234, 5678};
        auto address = figkey::CaptureConfig::Instance().getConfigInfo().network.address;
        for (size_t i = 0; i < address.size() && i < 5; i++) {
            comboBox->addItem(QString::number(porst[i]));
        }
    }
    else {
        comboBox->addItem(QString::number(packet.srcPort));
        comboBox->addItem(QString::number(packet.destPort));
    }
}

QComboBox* NetworkAssistWindow::getSettingComboBox(const QString& label) {
    for (int i = 0; i < ui->listSetting->count(); ++i) {
        QListWidgetItem* item = ui->listSetting->item(i);
        QWidget* widget = ui->listSetting->itemWidget(item);
        QLabel* nameLabel = widget->findChild<QLabel*>();
        if (nameLabel->text() == label) {
            QComboBox* comboBox = widget->findChild<QComboBox*>();
            return comboBox;
        }
    }
    return nullptr;
}

void NetworkAssistWindow::set(figkey::PacketInfo packet) {
    for(int i = 0; i < ui->listSetting->count(); ++i)
    {
        QListWidgetItem* item = ui->listSetting->item(i);
        QWidget* widget = ui->listSetting->itemWidget(item);
        QComboBox* comboBox = widget->findChild<QComboBox*>();

        switch(i)
        {
            case 0: // Protocol
                if (0 == packet.index) {
                    comboBox->setEnabled(true);
                    return;
                }

                setProtocol(packet.protocolType, comboBox);
                break;
            case 1:
                setClientIP(comboBox, packet);
                break;
            case 2:
                setServerIP(comboBox, packet);
                break;
            case 3:
                setServerPort(comboBox, packet);
                break;
            default:
                break;
        }
    }
}

QString NetworkAssistWindow::getSettingItemValue(const QString& label) const {
    for (int i = 0; i < ui->listSetting->count(); ++i) {
        QListWidgetItem* item = ui->listSetting->item(i);
        QWidget* widget = ui->listSetting->itemWidget(item);
        QLabel *labelWidget = widget->findChild<QLabel*>();
        QComboBox *comboBox = widget->findChild<QComboBox*>();

        if (labelWidget->text() == label) {
            return comboBox->currentText();
        }
    }
    return QString();
}

void NetworkAssistWindow::setSendButton(bool enable) {
    ui->buttonDisconnect->setEnabled(enable);
    ui->buttonSend->setEnabled(enable);
    ui->buttonSendReceive->setEnabled(enable);
    ui->buttonSendPeriod->setEnabled(enable);
    ui->buttonSendSequence->setEnabled(enable);
    ui->buttonStopSend->setEnabled(enable);
}

void NetworkAssistWindow::on_buttonConnect_clicked()
{
    ui->buttonConnect->setEnabled(false);

    // 获取用户的设置
    on_buttonDisconnect_clicked();

    QString clientIp = getSettingItemValue(SET_CLIENT_IP_LABEL);
    QString serverIp = getSettingItemValue(SET_SERVER_IP_LABEL);
    int serverPort = getSettingItemValue(SET_SERVER_PORT_LABEL).toInt();

    QString protocol = getSettingItemValue(SET_PROTOCOL_LABEL);
    bool useTCP = (protocol == "TCP");

    if (useTCP) {
        comm = new TCPComm(clientIp, serverIp, serverPort, isServer, this);
    } else {
        comm = new UDPComm(clientIp, serverIp, serverPort, isServer, this);
    }

    // 连接接收到数据的信号
    connect(comm, &BaseComm::dataReceived, this, &NetworkAssistWindow::onDataReceived);

    // 开始通信
    if (!comm->start()) {
        QMessageBox::critical(this, "connect error", comm->getLastError());
        ui->buttonConnect->setEnabled(true);
    }
    else {
        setSendButton(true);
    }
}

void NetworkAssistWindow::on_buttonDisconnect_clicked()
{
    setSendButton(false);

    if (comm) {
        comm->stop();
        comm->deleteLater();
        comm = nullptr;
    }

    ui->buttonConnect->setEnabled(true);
}

bool NetworkAssistWindow::getSendChecked(int row) {
    QWidget* widget = qobject_cast<QWidget*>(ui->tableSend->cellWidget(row, 0));
    if (widget) {
       QCheckBox* checkBox = widget->findChild<QCheckBox*>();
       if (checkBox) {
           return checkBox->isChecked();
       }
    }
    return false;  // Return false if QWidget or QCheckBox is null
}

void NetworkAssistWindow::setSendChecked(int row, bool check) {
    QWidget* widget = qobject_cast<QWidget*>(ui->tableSend->cellWidget(row, 0));
    if (widget) {
        QCheckBox* checkBox = widget->findChild<QCheckBox*>();
        if (checkBox) {
            checkBox->setChecked(check);
        }
    }
}

QByteArray NetworkAssistWindow::getSendData(int row) {
    QString dataString = ui->tableSend->item(row, 2)->text();
    QByteArray dataToSend;

    if (isASCII) {
        dataToSend = dataString.toLatin1();
    } else {
        QStringList byteStrings = dataString.split(' ', QString::SkipEmptyParts);
        for (const QString &byteString : byteStrings) {
            bool ok;
            int byteInt = byteString.toInt(&ok, 16); // Assume the data String is in hexadecimal
            if (ok) {
                dataToSend.append(static_cast<char>(byteInt));
            }
        }
    }

    return dataToSend;
}

bool NetworkAssistWindow::sendMessage(int row, const QByteArray& data) {
    if (!comm)
        return false;

    setSendChecked(row, false);
    if (comm->sendData(data)) {
        ui->tableSend->setItem(row, 1, new QTableWidgetItem(QTime::currentTime().toString("hh:mm:ss.zzz")));
        return true;
    }

    ui->tableSend->setItem(row, 1, new QTableWidgetItem(comm->getLastError()));
    return false;
}

void NetworkAssistWindow::updateSendChecked(const QString& timeStamp, const QString& dataString) {
    for (auto it = recvDataMap.begin(); it != recvDataMap.end(); ++it) {
        if(it.value() == dataString) {
            // Uncheck the checkbox if received data matches the row data
            setSendChecked(it.key(), true);
            ui->tableSend->setItem(it.key(), 1, new QTableWidgetItem(timeStamp));
            return;
        }
    }
}

void NetworkAssistWindow::onDataReceived(const QByteArray& data) {
    QString timeStamp = QTime::currentTime().toString("hh:mm:ss.zzz");
    QString dataString;

    if (isASCII) {
        dataString = data;
    } else {
        for (uint8_t byte : data) {
            dataString.append(QString("%1 ").arg(byte, 2, 16, QChar('0')).toUpper());
        }
    }

    if (!stopSending) {
        updateSendChecked(timeStamp, dataString);
    }

    if (ui->tableReceive->rowCount() >= figkey::CaptureConfig::Instance().getConfigInfo().receiveRows) {
        ui->tableReceive->removeRow(0);
    }

    int newRow = ui->tableReceive->rowCount();
    ui->tableReceive->insertRow(newRow);

    ui->tableReceive->setItem(newRow, 0, new QTableWidgetItem(timeStamp));
    ui->tableReceive->setItem(newRow, 1, new QTableWidgetItem(QString::number(data.size())));
    ui->tableReceive->setItem(newRow, 2, new QTableWidgetItem(dataString));
}

void NetworkAssistWindow::on_buttonSend_clicked() {
    int sendRowIndex = -1;
    for (int i = 0; i < ui->tableSend->rowCount(); ++i) {
        if (getSendChecked(i)) {
            QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
            if (typeBox && typeBox->currentIndex() == 0) {
                if (ui->tableSend->item(i, 2)->text().isEmpty()) {
                    QMessageBox::warning(nullptr, "Warning", "Send data is empty");
                    return;
                }
                sendRowIndex = i;
                break;
            }
        }
    }

    if (sendRowIndex == -1) {
        QMessageBox::warning(nullptr, "Warning", "Please check one piece of data to send");
    }
    else {
        on_buttonStopSend_clicked();
        stopSending = false;
        sendMessage(sendRowIndex, getSendData(sendRowIndex));
    }
}

bool NetworkAssistWindow::validateSendReceiveSequence(QList<QMap<QString, QVariant>>& list) {
    int rowCount = ui->tableSend->rowCount();
    int startRow = -1;
    bool canAddToList = true;

    for (int i = 0; i < rowCount; ++i) {
        if (getSendChecked(i)) {
            QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
            if (typeBox->currentText() == "Receive") {
                canAddToList = false;
            }
            startRow = i;
            break;
        }
    }

    if (startRow == -1) {
        QMessageBox::warning(nullptr, "Warning", "Please select a piece of data to start sending and receiving.");
        return false;
    }

    int totoal = 0;
    int currentRow = startRow;
    while (currentRow >= startRow) {
        // Validate the "Data" column, assumed to be at index 2
        QString data = ui->tableSend->item(currentRow, 2)->text();
        if (data.isEmpty()) {
            QMessageBox::warning(nullptr, "Warning", "Invalid data in the 'Data' column");
            return false;
        }

        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(currentRow, 3));
        if (typeBox->currentIndex() == 1) {
            canAddToList = false;
        }

        // Validate the "Interval" column, assumed to be at index 4
        int interval = 0;
        QString intervalStr = ui->tableSend->item(currentRow, 5)->text();
        if (!intervalStr.isEmpty()) {
            bool ok;
            interval = intervalStr.toInt(&ok);
            if (!ok || (interval < 1 || interval > 3600000)) {
                QMessageBox::warning(nullptr, "Warning", "Invalid interval value. It should be in the range 1 to 3600000.");
                return false;
            }

        }

        ++totoal;
        if (canAddToList) {
            QMap<QString, QVariant> rowInfo;
            rowInfo.insert("row", currentRow);
            rowInfo.insert("interval", interval);
            list.append(rowInfo);
        }

        // Get and validate the "Next" column, assumed to be at index 4
        QString nextRowStr = ui->tableSend->item(currentRow, 4)->text();
        int nextRow = -1;
        if (!nextRowStr.isEmpty()) {
            bool ok;
            nextRow = nextRowStr.toInt(&ok);
            if (!ok || nextRow > rowCount) {
                QMessageBox::warning(nullptr, "Warning", "Invalid next row value. It should be a valid row number.");
                return false;
            }
        }
        else {
            break;
        }

        currentRow = nextRow-1;
    }

    if (totoal < 2) {
        QMessageBox::warning(nullptr, "Warning", "Sending and receiving data less than 2 pieces");
        return false;
    }
    return true;
}

void NetworkAssistWindow::on_buttonSendReceive_clicked()
{
    QList<QMap<QString, QVariant>> sendList;
    if (!validateSendReceiveSequence(sendList))
        return;

    on_buttonStopSend_clicked();
    stopSending = false;

    // 使用lamdba表达式和QTimer实现延时发送和停止发送功能
    std::function<void(int)> sendFunction = nullptr;
    sendFunction = [=](int index) mutable {
        if (index < sendList.size() && !stopSending) {
            auto sendInfo = sendList.at(index);
            int row = sendInfo["row"].toInt();
            int interval = sendInfo["interval"].toInt();

            // 发送数据
            if (!sendMessage(row, getSendData(row))) {
                stopSending = true;
                return;  // Stop sending if sending message failed
            }

            if (interval > 0) {
                // 间隔发送
                QTimer::singleShot(interval, this, [=]() {
                    sendFunction(index + 1);  // Send next data after interval
                });
            } else {
                // 立即发送下一条数据
                sendFunction(index + 1);
            }
        }
    };
    // Start sending from first data
    sendFunction(0);
}

void NetworkAssistWindow::on_buttonSendPeriod_clicked()
{

}

void NetworkAssistWindow::on_buttonSendSequence_clicked()
{

}

void NetworkAssistWindow::on_buttonStopSend_clicked()
{
    stopSending = true;
    for (auto it = recvDataMap.begin(); it != recvDataMap.end(); ++it) {
        setSendChecked(it.key(), false);
    }
}
