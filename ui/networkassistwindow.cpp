#include <QComboBox>
#include <QListWidgetItem>
#include <QLabel>
#include <QtDebug>
#include <QMessageBox>
#include <QTime>
#include <QCheckBox>
#include <QTimer>
#include <QSpinBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>

#include "networkassistwindow.h"
#include "ui_networkassistwindow.h"
#include "config.h"
#include "common/tcpcomm.h"
#include "common/udpcomm.h"

NetworkAssistWindow::NetworkAssistWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkAssistWindow)
{
    ui->setupUi(this);
    helper = new NetworkHelper(ui, this);
    connect(helper, &NetworkHelper::sendMessage, this, &NetworkAssistWindow::sendMessage);

    initWindow();
}

NetworkAssistWindow::~NetworkAssistWindow()
{
    delete ui;
}

void NetworkAssistWindow::initWindow() {
    if (!helper) {
        // 显示错误消息框
        QMessageBox::critical(this, tr("Initialization Error"), tr("Unable to initialize the window because helper is null."));

        // 关闭当前窗口
        this->close();
        return;
    }
    helper->initListSetting();
    helper->initTableSend();
    helper->initTableReceive();
}

void NetworkAssistWindow::closeComm() {
    if (comm) {
        comm->stop();
        comm->deleteLater();
        comm = nullptr;
    }
}

void NetworkAssistWindow::isSaveFile() {
    if (!canSaveFile)
        return;

    int ret = QMessageBox::question(this,
                                    tr("Exit Window"),
                                    tr("Do you want to save your current test data?"),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        saveConfigToFile();
    }
}

void NetworkAssistWindow::exitWindow() {
    isSaveFile();
    closeComm();
    helper->stopReceiveTimers();
    helper->stopCycleTimers();
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
        ui->buttonConnect->setText("Satrt");
        ui->buttonDisconnect->setText("Stop");
    }
}

bool NetworkAssistWindow::saveConfigToFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save config to file",
                                                        QDir::homePath(),
                                                        "Config Files (*.json)");

    // Check if fileName is empty (cancel was pressed)
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No file selected.");
        return false;
    }

    QJsonDocument doc;
    QJsonObject jsonObject = doc.object();
    jsonObject["server"] = isServer;

    // Save Settings
    QJsonArray settingsArray;
    for (int i = 0; i < ui->listSetting->count(); ++i) {
        QListWidgetItem *item = ui->listSetting->item(i);
        QWidget *widget = ui->listSetting->itemWidget(item);
        QLabel *nameLabel = widget->findChild<QLabel*>();
        QComboBox *comboBox = widget->findChild<QComboBox*>();

        QJsonObject settingObject;
        settingObject["label"] = nameLabel->text();
        settingObject["value"] = comboBox->currentText();

        settingsArray.append(settingObject);
    }
    jsonObject["settings"] = settingsArray;

    // Save Send List
    QJsonArray sendListArray;
    for (int i = 0; i < ui->tableSend->rowCount(); ++i) {
        if ( ui->tableSend->item(i, 2)->text().isEmpty())
            break;

        QCheckBox *checkBox = ui->tableSend->cellWidget(i, 0)->findChild<QCheckBox*>();
        QTableWidgetItem *timeItem = ui->tableSend->item(i, 1);
        QTableWidgetItem *dataItem = ui->tableSend->item(i, 2);
        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
        QTableWidgetItem *nextItem = ui->tableSend->item(i, 4);
        QSpinBox *intervalSpinBox = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(i, 5));

        QJsonObject sendItemObject;
        sendItemObject["ck"] = checkBox->isChecked();
        sendItemObject["time"] = timeItem->text();
        sendItemObject["data"] = dataItem->text();
        sendItemObject["type"] = typeBox->currentIndex();
        sendItemObject["next"] = nextItem->text().toInt();
        sendItemObject["interval"] = intervalSpinBox->value();

        sendListArray.append(sendItemObject);
    }
    jsonObject["sendList"] = sendListArray;

    doc.setObject(jsonObject);

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return false;
    }

    QTextStream out(&file);
    try {
        out << doc.toJson(QJsonDocument::Indented);
    } catch (const std::exception &e) {  // Catch any other errors
        QMessageBox::critical(this, "Error", "Caught exception while writing to file: " + QString::fromUtf8(e.what()));
        return false;
    }

    QMessageBox::information(this, tr("Information"), tr("Test data saved successfully."));
    return true;  // Successful save
}

bool NetworkAssistWindow::isServerByLoadFile(const QString& fileName, bool& hasServer) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Could not open file for reading.");
        return false;
    }

    QByteArray fileData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fileData);

    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(nullptr, "Error", "Invalid JSON document.");
        return false;
    }

    QJsonObject obj = doc.object();
    hasServer = obj["server"].toBool();
    return true;
}

bool NetworkAssistWindow::loadConfigFromFile(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open file for reading.");
        return false;
    }

    QByteArray fileData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fileData);

    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "Error", "Invalid JSON document.");
        return false;
    }

    QJsonObject obj = doc.object();

    // Load Settings
    QJsonArray settingsArray = obj["settings"].toArray();
    for (int i = 0; i < settingsArray.size(); ++i) {
        QJsonObject settingObject = settingsArray[i].toObject();
        QComboBox *comboBox =getSettingComboBox(settingObject["label"].toString());
        if (comboBox) {
            int index = comboBox->findText(settingObject["value"].toString(), Qt::MatchExactly);
            if (index >= 0) {
                comboBox->setCurrentIndex(index);
            }
            else {
                comboBox->clear();
                comboBox->addItem(settingObject["value"].toString());
            }
        }
    }

    // Load Send List
    QJsonArray sendListArray = obj["sendList"].toArray();
    for (int i = 0; i < sendListArray.size(); ++i) {
        QJsonObject sendItemObject = sendListArray[i].toObject();
        QCheckBox *checkBox = ui->tableSend->cellWidget(i, 0)->findChild<QCheckBox*>();
        QTableWidgetItem *timeItem = ui->tableSend->item(i, 1);
        QTableWidgetItem *dataItem = ui->tableSend->item(i, 2);
        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
        QSpinBox *intervalSpinBox = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(i, 5));

        checkBox->setChecked(sendItemObject["ck"].toBool());
        timeItem->setText(sendItemObject["time"].toString());
        dataItem->setText(sendItemObject["data"].toString());
        typeBox->setCurrentIndex(sendItemObject["type"].toInt());
        if (sendItemObject["interval"].toInt() > 0) {
            intervalSpinBox->setValue(sendItemObject["interval"].toInt());
        }
    }

    for (int i = 0; i < sendListArray.size(); ++i) {
        QJsonObject sendItemObject = sendListArray[i].toObject();
        QTableWidgetItem *nextItem = ui->tableSend->item(i, 4);
        if (sendItemObject["next"].toInt() > 0) {
            nextItem->setText(QString::number(sendItemObject["next"].toInt()));
        }
    }

    this->show();

    return true;
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
        uint16_t porst[5]{13400, 13400, 8080, 1234, 5678};
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
    canSaveFile = false;

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

void NetworkAssistWindow::on_buttonConnect_clicked()
{
    ui->buttonConnect->setEnabled(false);

    // 获取用户的设置
    closeComm();

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
        canSaveFile = true;
        ui->buttonDisconnect->setEnabled(true);
        ui->buttonSend->setEnabled(true);
    }
}

void NetworkAssistWindow::on_buttonDisconnect_clicked()
{
    ui->buttonDisconnect->setEnabled(false);

    closeComm();

    ui->buttonConnect->setEnabled(true);
    ui->buttonSend->setEnabled(false);
    ui->buttonStopSend->setEnabled(false);
}

bool NetworkAssistWindow::sendMessage(int row) {
    if (!comm || (row < 0))
        return false;

    if (ui->comboBox->currentIndex() != 2) {
        helper->setColumnCheckState(row, true);
    }

    if (comm->sendData(helper->getSendData(row))) {
        ui->tableSend->setItem(row, 1, new QTableWidgetItem(QTime::currentTime().toString("hh:mm:ss.zzz")));
        return true;
    }

    return helper->setErrorInfo(row, comm->getLastError());
}

bool NetworkAssistWindow::sendMessages(const QList<int>& sendList) {
    for (int i = 0; i < sendList.size(); ++i) {
        if (!helper->enableSend())
            return false;

        int sendRowIndex = sendList.at(i);
        QSpinBox *intervalSpinBox = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(sendRowIndex, 5));
        if (intervalSpinBox) {
            int interval = intervalSpinBox->value();
            if (interval > 0) {
                QTimer::singleShot(interval, [this, sendRowIndex]() {
                    // stop sending if sendMessage() returns false or if has sent fail before
                    sendMessage(sendRowIndex);
                });
            } else {
                sendMessage(sendRowIndex);
            }
        }
    }

    helper->tryStopSend();
    return true;
}

void NetworkAssistWindow::sendNextMessage(const QList<int>& sendList, int currentIndex) {
    static QList<int> timerList;
    if (0 == currentIndex) {
        timerList = sendList;
    }

    if (helper->enableSend() && currentIndex < timerList.size()) {
        int sendRowIndex = sendList.at(currentIndex);
        QSpinBox *intervalSpinBox = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(sendRowIndex, 5));
        if (intervalSpinBox) {
            int interval = intervalSpinBox->value();
            if (interval > 0) {
                QTimer::singleShot(interval, this, [this, sendRowIndex, currentIndex]() {
                    if (helper->enableSend()) {
                        sendMessage(sendRowIndex);
                    }
                    sendNextMessage(timerList, currentIndex + 1);
                });
            } else {
                sendMessage(sendRowIndex);
                sendNextMessage(timerList, currentIndex + 1);
            }
        }
    } else {
        timerList.clear();
        helper->tryStopSend();
    }
}

void NetworkAssistWindow::onDataReceived(const QByteArray& data) {
    QString timeStamp = QTime::currentTime().toString("hh:mm:ss.zzz");
    QString dataString;

    if (helper->enableSend()) {
        auto sendList = helper->checkReceiveDataMap(timeStamp, data);
        if (!sendList.empty()) {
            sendMessages(sendList);
        }
    }

    if (helper->dataIsAscii()) {
        dataString = data;
    } else {
        for (uint8_t byte : data) {
            dataString.append(QString("%1 ").arg(byte, 2, 16, QChar('0')).toUpper());
        }
        dataString = dataString.trimmed();
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

bool NetworkAssistWindow::sendAndReceiveMessage() {
    auto sendList = helper->getContinuousSendMessages(0);
    if (sendList.empty()) {
        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(0, 3));
        if (!typeBox || typeBox->currentIndex() != 1 ||
                ui->tableSend->item(0, 2)->text().isEmpty()) {
            QMessageBox::warning(nullptr, "Warning", "Failed to send and receive: The first row of data is empty.");
            return false;
        }
    }

    helper->startSendAndReceive();
    return sendMessages(sendList);
}


void NetworkAssistWindow::on_buttonSend_clicked() {
    switch (ui->comboBox->currentIndex()) {
    case 0: {
            auto sendRowIndex = helper->getCheckedTableSend();
            if (sendRowIndex == -1) {
                QMessageBox::warning(nullptr, "Warning", "Please check one piece of valid data to send");
                return;
            }
            helper->setSendState(true);
            sendMessage(sendRowIndex);
            helper->tryStopSend();
            break;
        }
    case 1: {
            sendAndReceiveMessage();
            break;
        }
    case 2: {
            if (!helper->startCycleTimers()) {
                QMessageBox::warning(nullptr, "Warning", "Failed to cycle send: Please check the lines to be sent periodically message");
                return;
            }
            break;
        }
    case 3: {
            auto sendList = helper->getAllSendMessages();
            if (sendList.empty()) {
                QMessageBox::warning(nullptr, "Warning", "Failed to many send: The first row of data is empty.");
                return;
            }
            helper->setSendState(true);
            helper->setAllColumnUncheck();
            sendNextMessage(sendList, 0);
            break;
        }
    }
}

void NetworkAssistWindow::on_buttonStopSend_clicked()
{
    helper->setSendState(false);
    switch (ui->comboBox->currentIndex()) {
    case 1:
        helper->stopReceiveTimers();
        break;
    case 2:
        helper->stopCycleTimers();
        break;
    default:
        break;
    }
}


void NetworkAssistWindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 1:
        helper->setSendAndReceive(true);
        break;
    case 2:
        helper->setCycleSend(true);
        break;
    default:
        helper->setSendAndReceive(false);
        helper->setCycleSend(false);
        break;
    }
}
