#include "filterwindow.h"
#include "ui_filterwindow.h"
#include "config.h"
#include <QComboBox>
#include <QMessageBox>

FilterWindow::FilterWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterWindow)
{
    ui->setupUi(this);
    initWindow();
}

FilterWindow::~FilterWindow()
{
    delete ui;
}

void FilterWindow::initWindow()
{
    QTableWidget* table = ui->tableWidget;
    // 一般来说，这个检查应该不会失败，只有当你的UI出问题，或者没有正确地初始化时，才会为nullptr。
    if (table == nullptr) {
        // 这里错误处理
        return;
    }

    const auto& filter = figkey::CaptureConfig::Instance().getConfigInfo().filter;

    table->setColumnCount(2);
    table->setRowCount(11);
    table->setHorizontalHeaderLabels(QStringList() << "name" << "value");

    table->setItem(0, 0, new QTableWidgetItem("Protocol Type"));
    QComboBox* comboBox = new QComboBox();
    comboBox->addItems(QStringList() << "DEFAULT" << "TCP" << "UDP" << "DOIP" << "UDS");
    switch(filter.protocolType) {
    case figkey::PROTOCOL_TYPE_TCP:
        comboBox->setCurrentText("TCP");
        break;
    case figkey::PROTOCOL_TYPE_UDP:
        comboBox->setCurrentText("UDP");
        break;
    case figkey::PROTOCOL_TYPE_DOIP:
        comboBox->setCurrentText("DOIP");
        break;
    case figkey::PROTOCOL_TYPE_UDS:
        comboBox->setCurrentText("UDS");
        break;
    default:
        comboBox->setCurrentText("DEFAULT");
    }
    table->setCellWidget(0, 1, comboBox);

    table->setItem(1, 0, new QTableWidgetItem("IP"));
    if (!filter.ip.empty())
        table->setItem(1, 1, new QTableWidgetItem(QString::fromStdString(filter.ip)));

    table->setItem(2, 0, new QTableWidgetItem("Port"));
    if (0 != filter.port)
        table->setItem(2, 1, new QTableWidgetItem(QString::number(filter.port)));

    table->setItem(3, 0, new QTableWidgetItem("Source IP"));
    if (!filter.srcIP.empty())
        table->setItem(3, 1, new QTableWidgetItem(QString::fromStdString(filter.srcIP)));

    table->setItem(4, 0, new QTableWidgetItem("Destination IP"));
    if (!filter.destIP.empty())
        table->setItem(4, 1, new QTableWidgetItem(QString::fromStdString(filter.destIP)));

    table->setItem(5, 0, new QTableWidgetItem("Source MAC"));
    if (!filter.srcMAC.empty())
        table->setItem(5, 1, new QTableWidgetItem(QString::fromStdString(filter.srcMAC)));

    table->setItem(6, 0, new QTableWidgetItem("Destination MAC"));
    if (!filter.destMAC.empty())
        table->setItem(6, 1, new QTableWidgetItem(QString::fromStdString(filter.destMAC)));

    table->setItem(7, 0, new QTableWidgetItem("Source Port"));
    if (0 != filter.srcPort)
        table->setItem(7, 1, new QTableWidgetItem(QString::number(filter.srcPort)));

    table->setItem(8, 0, new QTableWidgetItem("Destination Port"));
    if (0 != filter.destPort)
        table->setItem(8, 1, new QTableWidgetItem(QString::number(filter.destPort)));

    table->setItem(9, 0, new QTableWidgetItem("Min Payload Length"));
    if (0 != filter.minLen)
        table->setItem(9, 1, new QTableWidgetItem(QString::number(filter.minLen)));

    table->setItem(10, 0, new QTableWidgetItem("Max payload Length"));
    if (0 != filter.maxLen)
        table->setItem(10, 1, new QTableWidgetItem(QString::number(filter.maxLen)));

    table->setColumnWidth(0, 145);
    table->setColumnWidth(1, 250);
}

bool isValidIpAddress(const QString& ip) {
    // Check IPv4
    QRegExp regExpV4("^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");
    if (regExpV4.exactMatch(ip))
        return true;
    // Check IPv6, 把 "\\." 改为 "."
    QRegExp regExpV6("^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,5}(:[0-9A-Fa-f]{1,4}){1,2})|(([0-9A-Fa-f]{1,4}:){1,4}(:[0-9A-Fa-f]{1,4}){1,3})|(([0-9A-Fa-f]{1,4}:){1,3}(:[0-9A-Fa-f]{1,4}){1,4})|(([0-9A-Fa-f]{1,4}:){1,2}(:[0-9A-Fa-f]{1,4}){1,5})|([0-9A-Fa-f]{1,4}:((:[0-9A-Fa-f]{1,4}){1,6}))|(:((:[0-9A-Fa-f]{1,4}){1,7}|:))|(::[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|(([0-9A-Fa-f]{1,4}:){1,5}|:):[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|(([0-9A-Fa-f]{1,4}:){1,4}|:)::[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|(([0-9A-Fa-f]{1,4}:){1,3}|:):[0-9A-Fa-f]{1,4}:[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|(([0-9A-Fa-f]{1,4}:){1,2}|:):[0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4}:[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|(([0-9A-Fa-f]{1,4}:){1,1}|:):[0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4}:[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|-:{0,1}([0-9A-Fa-f]{0,4}(:[0-9A-Fa-f]{1,4}){0,2}):[0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4}:[0-9A-Fa-f]{1,4}:[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|::[0-9A-Fa-f]{1,4}:[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3})|::[Ff]{4}(:0{1,4}:){0,1}:{0,1}([0-9]{1,3}.){3}[0-9]{1,3}))");
    if (regExpV6.exactMatch(ip))
        return true;

    return false;
}

static bool isValidMacAddress(const QString& mac) {
    QRegExp regExp("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$");
    return regExp.exactMatch(mac);
}

static bool isValidPort(const QString& port) {
    bool ok;
    int portNum = port.toInt(&ok);
    return ok && portNum >= 0 && portNum <= 65535;
}

static bool isValidPayloadLength(const QString& len) {
    bool ok;
    int length = len.toInt(&ok);
    return ok && length >= 0;
}
void FilterWindow::on_pushButton_clicked()
{
    try {
        QTableWidget* table = ui->tableWidget;
        if (!table) {
            QMessageBox::critical(this, "Critical Error", "Table object is null.");
            return;
        }

        figkey::FilterInfo filter;

        QComboBox *comboBox = qobject_cast<QComboBox*>(table->cellWidget(0, 1));
        QString protocolType = comboBox->currentText();
        if (protocolType == "DEFAULT") {
            filter.protocolType = figkey::PROTOCOL_TYPE_DEFAULT;
        } else if (protocolType == "TCP") {
            filter.protocolType = figkey::PROTOCOL_TYPE_TCP;
        } else if (protocolType == "UDP") {
            filter.protocolType = figkey::PROTOCOL_TYPE_UDP;
        } else if (protocolType == "DOIP") {
            filter.protocolType = figkey::PROTOCOL_TYPE_DOIP;
        } else if (protocolType == "UDS") {
            filter.protocolType = figkey::PROTOCOL_TYPE_UDS;
        }

        auto ipItem = table->item(1, 1);
        if (ipItem && !ipItem->text().isEmpty()) {
            if(!isValidIpAddress(ipItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid IP address.");
                return;
            } else {
                filter.ip = ipItem->text().toStdString();
            }
        }

        auto portItem = table->item(2, 1);
        if (portItem && !portItem->text().isEmpty()) {
            if(!isValidPort(portItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid port.");
                return;
            } else {
                filter.port = static_cast<uint16_t>(portItem->text().toUShort());
            }
        }

        auto srcIPItem = table->item(3, 1);
        if (srcIPItem && !srcIPItem->text().isEmpty()) {
            if(!isValidIpAddress(srcIPItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid source IP address.");
                return;
            } else if (!filter.ip.empty()) {
                QMessageBox::warning(this, "Error", "The IP address has been specified and the source IP cannot be specified.");
                return;
            } else {
                filter.srcIP = srcIPItem->text().toStdString();
            }
        }

        auto destIPItem = table->item(4, 1);
        if (destIPItem && !destIPItem->text().isEmpty()) {
            if(!isValidIpAddress(destIPItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid destination IP address.");
                return;
            } else if (!filter.ip.empty()) {
                QMessageBox::warning(this, "Error", "The IP address has been specified and the destination IP cannot be specified.");
                return;
            } else {
                filter.destIP = destIPItem->text().toStdString();
            }
        }

        auto srcMACItem = table->item(5, 1);
        if (srcMACItem && !srcMACItem->text().isEmpty()) {
            if(!isValidMacAddress(srcMACItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid source MAC address.");
                return;
            } else {
                filter.srcMAC = srcMACItem->text().toStdString();
            }
        }

        auto destMACItem = table->item(6, 1);
        if (destMACItem && !destMACItem->text().isEmpty()) {
            if(!isValidMacAddress(destMACItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid destination MAC address.");
                return;
            } else {
                filter.destMAC = destMACItem->text().toStdString();
            }
        }

        auto srcPortItem = table->item(7, 1);
        if (srcPortItem && !srcPortItem->text().isEmpty()) {
            if(!isValidPort(srcPortItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid source port.");
                return;
            } else if (0 != filter.port) {
                QMessageBox::warning(this, "Error", "The IP port has been specified and the source IP port cannot be specified.");
                return;
            } else {
                filter.srcPort = static_cast<uint16_t>(srcPortItem->text().toUShort());
            }
        }

        auto destPortItem = table->item(8, 1);
        if (destPortItem && !destPortItem->text().isEmpty()) {
            if(!isValidPort(destPortItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid destination port.");
                return;
            } else if (0 != filter.port) {
                QMessageBox::warning(this, "Error", "The IP port has been specified and the destination IP port cannot be specified.");
                return;
            } else {
                filter.destPort = static_cast<uint16_t>(destPortItem->text().toUShort());
            }
        }

        auto minLenItem = table->item(9, 1);
        if (minLenItem && !minLenItem ->text().isEmpty()) {
            if(!isValidPayloadLength(minLenItem ->text())) {
                QMessageBox::warning(this, "Error", "Invalid minimum payload length.");
                return;
            } else {
                filter.minLen = static_cast<uint16_t>(minLenItem ->text().toUShort());
            }
        }

        auto maxLenItem = table->item(10, 1);
        if (maxLenItem && !maxLenItem->text().isEmpty()) {
            if(!isValidPayloadLength(maxLenItem->text())) {
                QMessageBox::warning(this, "Error", "Invalid maximum payload length.");
                return;
            } else {
                filter.maxLen = static_cast<uint16_t>(maxLenItem->text().toUShort());
            }
        }

        //Should min length be less than max length?
        if(filter.maxLen > 0 && filter.minLen > filter.maxLen){
            QMessageBox::warning(this, "Error", "Minimum payload length should be less or equal to maximum payload length.");
            return;
        }

        figkey::CaptureConfig::Instance().setFilter(filter);
        QMessageBox::information(this, "Info", "Filter settings applied successfully.");
        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Critical Error", e.what());
    } catch (...) {
        QMessageBox::critical(this, "Critical Error", "Unknown error");
    }
}

