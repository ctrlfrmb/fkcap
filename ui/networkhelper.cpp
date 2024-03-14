#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QTimer>

#include "networkhelper.h"
#include "config.h"

NetworkHelper::NetworkHelper(Ui::NetworkAssistWindow *ui, QObject *parent)
    : QObject(parent), ui(ui)
{
}

NetworkHelper::~NetworkHelper() {
    stopReceiveTimers();
}

bool NetworkHelper::dataIsAscii() const {
    return isASCII;
}

bool NetworkHelper::enableSend() const {
    return isSending;
}

bool NetworkHelper::isReceiveDataEmpty() const {
    return recvDataMap.empty();
}

void NetworkHelper::addSettingItem(bool isEdit, const QString& label, const QStringList& options) {
    QListWidgetItem* item = new QListWidgetItem(ui->listSetting);

    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* nameLabel = new QLabel(label);
    QComboBox* comboBox = new QComboBox();
    comboBox->addItems(options);
    comboBox->setEditable(isEdit);  // Allow user input

    if (label == SET_DATA_TYPE_LABEL)
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
}

int NetworkHelper::getNextRow(int row) {
    QString nextStr = ui->tableSend->item(row, 4)->text();
    bool isNum = false;
    int nextRow = nextStr.toInt(&isNum);
    if (isNum && nextRow > 0 && nextRow  < ui->tableSend->rowCount()) {
        return nextRow-1;
    }

    return -1;
}

QByteArray NetworkHelper::getSendData(int row) {
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

void NetworkHelper::initListSetting() {
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

void NetworkHelper::initTableSend() {
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

        // Data column
       QTableWidgetItem *dataItem = new QTableWidgetItem();
       connect(ui->tableSend, &QTableWidget::itemChanged, this, [&, dataItem](QTableWidgetItem* item) {
           if (item == dataItem && !item->text().isEmpty()) {
               // clear time column
               ui->tableSend->item(item->row(), 1)->setText("");
               // data validation
               if (!isASCII) { // if hex data
                   QRegularExpression regExp("^([0-9A-Fa-f]{2} )*[0-9A-Fa-f]{2}$");
                   if (!regExp.match(item->text()).hasMatch()) {
                       item->setText("");  // clear the invalid input
                       QMessageBox::warning(nullptr, "Warning", "Invalid hexadecimal input, like 02 FD 80 01 00 00 00 00");
                   }
               }
               // check the prior row data
               if (item->row() > 0 && ui->tableSend->item(item->row()-1, 2)->text().isEmpty()) {
                   item->setText("");
                   QMessageBox::warning(nullptr, "Warning", "Previous row data must not be empty");
               }
           }
       });
       ui->tableSend->setItem(i, 2, dataItem);

        // Type column
        QComboBox *comboBox = new QComboBox();
        comboBox->addItem("Send");
        comboBox->addItem("Receive");
        ui->tableSend->setCellWidget(i, 3, comboBox);
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [=](int index) {
                    if (index == 1) {
                        QTableWidgetItem *dataItem = ui->tableSend->item(i, 2);
                        if (dataItem && !dataItem->text().isEmpty()) {
                            recvDataMap[i] = getSendData(i);
                        } else {
                            comboBox->setCurrentIndex(0);  // Reset to "Send"
                            QMessageBox::warning(nullptr, "Warning", "Data must not be empty");
                        }
                    }
                    else {
                        recvDataMap.remove(i);
                    }
                }
        );

        // Next column
        QTableWidgetItem* nextRowItem = new QTableWidgetItem();
        // Connect signal to validate next when cell content changed
        connect(ui->tableSend, &QTableWidget::itemChanged, this, [&, nextRowItem](QTableWidgetItem* item) {
            if (item == nextRowItem && !item->text().isEmpty()) {
                bool ok;
                int nextRow = item->text().toInt(&ok);
                if (!ok || nextRow < 1 || nextRow > ui->tableSend->rowCount() ||
                    ui->tableSend->item(nextRow - 1, 2)->text().isEmpty()) {
                    item->setText("");
                    QMessageBox::warning(nullptr, "Warning", "Invalid Next value");
                }
                else {
                    QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(nextRow - 1, 3));
                    if (typeBox && typeBox->currentIndex() != 0) {
                        item->setText("");
                        QMessageBox::warning(nullptr, "Warning", "Next row type must be Send");
                    }
                }
            }
        });
        ui->tableSend->setItem(i, 4, nextRowItem);

        // Interval column
        QSpinBox *intervalSpinBox = new QSpinBox();
        intervalSpinBox->setRange(0, 1000000);
        ui->tableSend->setCellWidget(i, 5, intervalSpinBox);
    }
}

void NetworkHelper::initTableReceive() {
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

void NetworkHelper::setSendState(bool state) {
    isSending = state;
    if (!isSending) {
        ui->buttonSend->setEnabled(true);
        ui->buttonStopSend->setEnabled(false);
        ui->tableSend->clearSelection(); // 清除选中的行
    }
    else {
        ui->buttonSend->setEnabled(false);
        ui->buttonStopSend->setEnabled(true);
    }
}

void NetworkHelper::setCheckedSendTable(int row, bool check) {
    QWidget* widget = qobject_cast<QWidget*>(ui->tableSend->cellWidget(row, 0));
    if (widget) {
        QCheckBox* checkBox = widget->findChild<QCheckBox*>();
        if (checkBox) {
            checkBox->setChecked(check);
        }
    }
}

bool NetworkHelper::isCheckedSendTable(int row) {
    QWidget* widget = qobject_cast<QWidget*>(ui->tableSend->cellWidget(row, 0));
    if (widget) {
       QCheckBox* checkBox = widget->findChild<QCheckBox*>();
       if (checkBox) {
           return checkBox->isChecked();
       }
    }
    return false;  // Return false if QWidget or QCheckBox is null
}

void NetworkHelper::uncheckTableSend() {
    for(int i = 0; i < ui->tableSend->rowCount(); ++i) {
        QWidget* widget = ui->tableSend->cellWidget(i, 0);
        QCheckBox* checkBox = widget ? widget->findChild<QCheckBox*>() : nullptr;
        if(checkBox) {
            checkBox->setChecked(false);
        }
    }
}

void NetworkHelper::setSendAndReceive(bool enable) {
    isSendAndReceive = enable;
}

void NetworkHelper::stopReceiveTimers() {
    for (auto it = recvTimerMap.begin(); it != recvTimerMap.end(); ++it) {
        QTimer* timer = it.value().second;
        timer->stop();
        delete timer;
    }
    recvTimerMap.clear();
}

void NetworkHelper::getReceiveTimerMap() {
    for (int i = 0; i < ui->tableSend->rowCount(); i++) {
        if (ui->tableSend->item(i, 2)->text().isEmpty())
            return;

        // Get the type from the Type column
        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
        if (typeBox && typeBox->currentIndex() == 1) {
            // Get the interval from the Interval column
            QSpinBox *intervalSpinBox = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(i, 5));
            if (intervalSpinBox) {
                int interval = intervalSpinBox->value();
                if (interval > 0) {
                    if (recvTimerMap.contains(i)) {
                        recvTimerMap[i].first = interval;
                    } else {
                        recvTimerMap[i] = qMakePair(interval, new QTimer(this));
                    }
                }
            }
        }
    }
}

void NetworkHelper::startReceiveTimers() {
    if (!isSendAndReceive)
        return;

    uncheckTableSend();

    // 停止之前的定时器
    stopReceiveTimers();

    getReceiveTimerMap();

    // 创建新的定时器
    for (auto it = recvTimerMap.begin(); it != recvTimerMap.end(); ++it) {
        int rowNum = it.key();
        int interval = it.value().first;
        QTimer* timer = it.value().second;

        timer->setSingleShot(true); // 单次触发
        timer->setInterval(interval); // 设置定时间隔

        connect(timer, &QTimer::timeout, this, [this, rowNum]() {
            // 定时到达后处理逻辑
            QMessageBox::warning(nullptr, "Timeout", "Receive timeout for row " + QString::number(rowNum+1));
            recvTimerMap.remove(rowNum);
            QTableWidgetItem* item = this->ui->tableSend->item(rowNum, 1);
            if (item) {
                QBrush redBrush(Qt::red);
                item->setBackground(redBrush); // 将单元格的背景色设置为空，恢复默认颜色
                item->setText("Receive timeout");
            }
            this->setSendState(false);
        });

        timer->start();
    }
}

QList<int> NetworkHelper::checkReceiveDataMap(const QString& timeStamp, const QString& dataString) {
    for (auto it = recvDataMap.begin(); it != recvDataMap.end(); ++it) {
        if(it.value() == dataString) {
            // Uncheck the checkbox if received data matches the row data
            setCheckedSendTable(it.key(), true);
            ui->tableSend->setItem(it.key(), 1, new QTableWidgetItem(timeStamp));
            if (isSendAndReceive) {
                // 移除定时器
                if (recvTimerMap.contains(it.key())) {
                    QTimer* timer = recvTimerMap.value(it.key()).second;
                    timer->stop();
                    recvTimerMap.remove(it.key());
                    delete timer;
                }

                return getSendSequence(it.key());
            }
            return {};
        }
    }

    return {};
}

int NetworkHelper::getCheckedTableSend() {
    for (int i = 0; i < ui->tableSend->rowCount(); ++i) {
        if (isCheckedSendTable(i)) {
            QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
            if (typeBox && typeBox->currentIndex() == 0) {
                if (ui->tableSend->item(i, 2)->text().isEmpty()) {
                    return -1;
                }
                return i;
            }
        }
    }

    return -1;
}

QList<int> NetworkHelper::getSendSequence(int start) {
    QList<int> sendSequence;
    int currentRow = start;
    if (0 != currentRow) {
        currentRow = getNextRow(start);
    }

    while (currentRow < ui->tableSend->rowCount()) {
        if (currentRow < 0 || ui->tableSend->item(currentRow, 2)->text().isEmpty()) {
            break;
        }

        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(currentRow, 3));
        if (typeBox && typeBox->currentIndex() == 0) {
            sendSequence.push_back(currentRow);
            currentRow = getNextRow(currentRow);
        } else {
            break;
        }
    }

    if (isSendAndReceive) {
        if (0 != start && sendSequence.empty()) {
            tryStopSend();
        }

        if (0 == start && !sendSequence.empty()) {
            startReceiveTimers();
        }
    }
    return sendSequence;
}

void NetworkHelper::tryStopSend() {
    if (!recvDataMap.empty())
        return;

    setSendState(false);
}
