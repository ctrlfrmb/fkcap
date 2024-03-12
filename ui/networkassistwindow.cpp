#include <QComboBox>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QtDebug>
#include <QMessageBox>
#include <QTime>

#include "networkassistwindow.h"
#include "ui_networkassistwindow.h"
#include "config.h"
#include "common/tcpcomm.h"
#include "common/udpcomm.h"

#define SET_PROTOCOL_LABEL "Protocol"
#define SET_LOCAL_IP_LABEL "Local IP"
#define SET_CONNECT_IP_LABEL "Connect IP"
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
    QStringList localIps;
    auto address = figkey::CaptureConfig::Instance().getConfigInfo().network.address;
    for (const auto& addr : address) {
        localIps << QString(addr.ip.c_str());
    }
    addSettingItem(true, SET_LOCAL_IP_LABEL, localIps);
    addSettingItem(true, SET_CONNECT_IP_LABEL, {"192.168.1.1"});
    addSettingItem(true, SET_SERVER_PORT_LABEL, {"8080"});
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
    if(label == SET_CONNECT_IP_LABEL)
    {
        connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(syncPort(int)));
    }

    layout->addWidget(nameLabel);
    layout->addWidget(comboBox);
    layout->setAlignment(Qt::AlignVCenter);
    layout->setContentsMargins(5, 5, 5, 5);
    widget->setLayout(layout);

    item->setSizeHint(widget->sizeHint());
    ui->listSetting->setItemWidget(item, widget);

    if(label == SET_CONNECT_IP_LABEL || label == SET_SERVER_PORT_LABEL)
    {
       comboBoxes.append(comboBox);
    }
}

void NetworkAssistWindow::initTableSend() {

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
    ui->tableReceive->setColumnWidth(2, ui->tableReceive->width()-155);
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
    QComboBox* comboBoxLocalIP = getSettingComboBox(SET_LOCAL_IP_LABEL);
    isServer = (packet.err == 1)?true:false;

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
            case 2: // Connect IP
                comboBox->clear();
                if (comboBoxLocalIP->findText(QString::fromStdString(packet.srcIP)) == -1) {
                    comboBox->addItem(QString::fromStdString(packet.srcIP));
                }
                else {
                    packet.srcPort = 0;
                }
                if (comboBoxLocalIP->findText(QString::fromStdString(packet.destIP)) == -1) {
                    comboBox->addItem(QString::fromStdString(packet.destIP));
                }
                else {
                    packet.destPort = 0;
                }
                break;
            case 3: // Server Port
                comboBox->clear();
                if (packet.srcPort > 0)
                    comboBox->addItem(QString::number(packet.srcPort));
                if (packet.destPort > 0)
                    comboBox->addItem(QString::number(packet.destPort));
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

void NetworkAssistWindow::onDataReceived(const QByteArray& data) {
    QString timeStamp = QTime::currentTime().toString("hh:mm:ss.zzz");
    QString dataType = getSettingItemValue(SET_DATA_TYPE_LABEL);
    QString dataString;

    if (dataType == "ASCII") {
        dataString = data;
    } else {
        for (uint8_t byte : data) {
            dataString.append(QString("%1 ").arg(byte, 2, 16, QChar('0')).toUpper());
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

void NetworkAssistWindow::on_buttonConnect_clicked()
{
    ui->buttonConnect->setEnabled(false);

    // 获取用户的设置
    on_buttonDisconnect_clicked();

    QString localIp = getSettingItemValue(SET_LOCAL_IP_LABEL);
    QString serverIp = getSettingItemValue(SET_CONNECT_IP_LABEL);
    int serverPort = getSettingItemValue(SET_SERVER_PORT_LABEL).toInt();

    QString protocol = getSettingItemValue(SET_PROTOCOL_LABEL);
    bool useTCP = (protocol == "TCP");

    if (useTCP) {
        comm = new TCPComm(localIp, serverIp, serverPort, isServer, this);
    } else {
        comm = new UDPComm(localIp, serverIp, serverPort, isServer, this);
    }

    // 连接接收到数据的信号
    connect(comm, &BaseComm::dataReceived, this, &NetworkAssistWindow::onDataReceived);

    // 开始通信
    if (!comm->start()) {
        QMessageBox::warning(this, "Warning", "Failed to start communication.");
        ui->buttonConnect->setEnabled(true);
    }
    else {
        ui->buttonDisconnect->setEnabled(true);
        ui->buttonSend->setEnabled(true);
    }
}

void NetworkAssistWindow::on_buttonDisconnect_clicked()
{
    ui->buttonDisconnect->setEnabled(false);
    ui->buttonSend->setEnabled(false);

    if (comm) {
        comm->stop();
        comm->deleteLater();
        comm = nullptr;
    }

    ui->buttonConnect->setEnabled(true);
}

void NetworkAssistWindow::on_buttonSend_clicked()
{
     if (comm) {
         // 转化为QByteArray
         static int row = 0;
         QString num = QString::number(++row);
         QByteArray data = num.toUtf8();

         // 发送数据
         comm->sendData(data);
     }
}
