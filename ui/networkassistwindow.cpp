#include <QComboBox>
#include <QListWidgetItem>
#include <QLabel>
#include <QtDebug>
#include <QMessageBox>
#include <QTime>
#include <QCheckBox>
#include <QTimer>
#include <QSpinBox>

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

void NetworkAssistWindow::exitWindow() {
    closeComm();
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
        ui->buttonDisconnect->setEnabled(true);
        ui->buttonSend->setEnabled(true);
    }
}

void NetworkAssistWindow::on_buttonDisconnect_clicked()
{
    closeComm();

    ui->buttonDisconnect->setEnabled(false);
    ui->buttonSend->setEnabled(false);
    ui->buttonStopSend->setEnabled(false);
}

bool NetworkAssistWindow::sendMessage(int row) {
    if (!comm || (row < 0))
        return false;

    helper->setCheckedSendTable(row, true);
    if (comm->sendData(helper->getSendData(row))) {
        ui->tableSend->setItem(row, 1, new QTableWidgetItem(QTime::currentTime().toString("hh:mm:ss.zzz")));
        return true;
    }

    ui->tableSend->setItem(row, 1, new QTableWidgetItem(comm->getLastError()));
    ui->tableSend->selectRow(row);
    return false;
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
                    if (!sendMessage(sendRowIndex)) {
                        // mark send fail
                        helper->setSendState(false);
                    }
                });
            } else {
                if (!sendMessage(sendRowIndex)) {
                    helper->setSendState(false);
                    return false;
                }
            }
        }
    }

    return true;
}

void NetworkAssistWindow::onDataReceived(const QByteArray& data) {
    QString timeStamp = QTime::currentTime().toString("hh:mm:ss.zzz");
    QString dataString;

    if (helper->dataIsAscii()) {
        dataString = data;
    } else {
        for (uint8_t byte : data) {
            dataString.append(QString("%1 ").arg(byte, 2, 16, QChar('0')).toUpper());
        }
    }

    if (helper->enableSend()) {
        auto sendList = helper->checkReceiveDataMap(timeStamp, dataString);
        if (!sendList.empty()) {
            sendMessages(sendList);
        }
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
    auto sendList = helper->getSendSequence(0);
    if (sendList.empty()) {
        QMessageBox::warning(nullptr, "Warning", "Failed to send and receive: please check the data or sending type.");
        return false;
    }

    helper->setSendState(true);
    return sendMessages(sendList);
}

void NetworkAssistWindow::on_buttonSend_clicked() {
    switch (ui->comboBox->currentIndex()) {
    case 0: {
            auto sendRowIndex = helper->getCheckedTableSend();
            if (sendRowIndex == -1) {
                QMessageBox::warning(nullptr, "Warning", "Please check one piece of data to send");
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
    }
}

void NetworkAssistWindow::on_buttonStopSend_clicked()
{
    helper->setSendState(false);
}


void NetworkAssistWindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 1:
        helper->setSendAndReceive(true);
        break;
    default:
        helper->setSendAndReceive(false);
        break;
    }
}
