#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

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

QByteArray NetworkHelper::getDataFromString(const std::string& str) {
    QString dataString(str.c_str());
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
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) {
            this->isASCII = (index == 1);
        });
    }
    else if (label == SET_ERROR_PROCESS_LABEL)
    {
        // Connect the signal that fires when the ComboBox selection changes.
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) {
            // Update flag based on the selection.
            this->isPass = (index == 0);
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

void NetworkHelper::initListSetting() {
    // Create QListWidgetItem and corresponding custom QWidget for each setting.
    addSettingItem(false, SET_PROTOCOL_LABEL, {"TCP", "UDP", "DOIP"});

    QStringList clientIps;
    auto address = figkey::CaptureConfig::Instance().getConfigInfo().network.address;
    for (const auto& addr : address) {
        clientIps << QString(addr.ip.c_str());
    }
    addSettingItem(true, SET_CLIENT_IP_LABEL, clientIps);
    addSettingItem(true, SET_SERVER_IP_LABEL, {"192.168.1.1"});
    addSettingItem(true, SET_SERVER_PORT_LABEL, {"13400"});

    addSettingItem(false, SET_DATA_TYPE_LABEL, {"HEX", "ASCII"});
    addSettingItem(false, SET_ERROR_PROCESS_LABEL, {"PASS", "STOP"});
    addSettingItem(false, SET_JSON_TEST_LABEL, {"Auto", "Manual"});
}

bool NetworkHelper::isLooped(int start) {
    QSet<int> visited;
    int current = start;
    while (true) {
        QTableWidgetItem* currentItem = ui->tableSend->item(current, 4); // assuming "Next" is at column 4
        if (!currentItem) {
            break;
        }
        bool ok;
        int next = currentItem->data(Qt::UserRole).toInt(&ok);
        if (!ok || next < 1 || next > ui->tableSend->rowCount()) {
            break; // Invalid "Next" value or reach the end
        }
        if (visited.contains(next)) {
            return true; // Loop detected
        }
        visited.insert(next);  // should insert 'next' but not 'current'
        current = next;
    }
    return false; // No loop
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
        connect(checkBox, &QCheckBox::toggled, this, [=](bool checked) {
            if (isCycleSend) {
                QSpinBox* intervalSpinBox = qobject_cast<QSpinBox*>(ui->tableSend->cellWidget(i, 5));
                if (intervalSpinBox) {
                    int interval = intervalSpinBox->value();
                    if (checked) {
                        if (interval > SET_CYCLE_SEND_MIN_TIME) {
                            QTimer *timer = new QTimer(this);
                            timer->setInterval(interval);
                            connect(timer, &QTimer::timeout, this, [=]() {
                                emit sendMessage(i);
                            });
                            if (isSending) {
                                timer->start();
                            }
                            cycleTimerMap[i] = timer;
                        } else {
                            QMessageBox::warning(nullptr, "Invalid Interval", QString("Cycle send interval must be greater than %1!").arg(SET_CYCLE_SEND_MIN_TIME));
                            checkBox->setChecked(false);
                        }
                    } else {
                        deleteTimer(cycleTimerMap.value(i, nullptr));
                        cycleTimerMap.remove(i);
                    }
                }
            }
        });
        ui->tableSend->setCellWidget(i, 0, widget);

        // Timer column
        QTableWidgetItem *timeItem = new QTableWidgetItem();
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);  // Make the item not editable
        ui->tableSend->setItem(i, 1, timeItem);

        // Data column
        QTableWidgetItem *dataItem = new QTableWidgetItem();
        dataItem->setData(Qt::UserRole, "");  // initialize lastValidData using UserRole

        connect(ui->tableSend, &QTableWidget::itemChanged, this, [&, dataItem](QTableWidgetItem* item) {
           if (item == dataItem && !item->text().isEmpty()) {
               // data validation
               if (!isASCII) { // if hex data
                   QRegularExpression regExp("^([0-9A-Fa-f]{2} )*[0-9A-Fa-f]{2}$");
                   QString lastValidData = item->data(Qt::UserRole).toString(); // Get last valid data from item
                   if (!regExp.match(item->text()).hasMatch()) {
                       item->setText(lastValidData);  // restore the last valid input
                       QMessageBox::warning(nullptr, "Warning", "Invalid hexadecimal input, like 02 FD 80 01 00 00 00 00");
                       return;
                   }
               }
               // check the prior row data
               if (item->row() > 0 && ui->tableSend->item(item->row()-1, 2)->text().isEmpty()) {
                   QString lastValidData = item->data(Qt::UserRole).toString(); // Get last valid data from item
                   item->setText(lastValidData);  // restore the last valid input
                   QMessageBox::warning(nullptr, "Warning", "Previous row data must not be empty");
                   return;
               }

               // clear time column
               ui->tableSend->item(item->row(), 1)->setText("");
               // If we reach here, data is valid. So we update the last valid data and store it in item.
               item->setData(Qt::UserRole, item->text());
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
                            // Check if we are in sending state and if send and receive has started
                            if (isSending && isSendAndReceive) {
                                QSpinBox* intervalSpinBox = qobject_cast<QSpinBox*>(ui->tableSend->cellWidget(i, 5));
                                if (intervalSpinBox) {
                                    int interval = intervalSpinBox->value();
                                    if (interval > 0) {
                                        // Add to the recvTimerMap and start the timer
                                        QTimer* timer = new QTimer(this);
                                        recvTimerMap[i] = qMakePair(interval, timer);
                                        timer->setInterval(interval);
                                        timer->setSingleShot(true);

                                        connect(timer, &QTimer::timeout, this, [this, i]() {
                                            // On timer timeout, remove from the map and set error info
                                            recvTimerMap.remove(i);
                                            setErrorInfo(i, "Receive timeout");
                                        });

                                        timer->start();
                                    }
                                }
                            }
                        } else {
                            comboBox->setCurrentIndex(0);  // Reset to "Send"
                            QMessageBox::warning(nullptr, "Warning", "Data must not be empty");
                        }
                    }
                    else {
                        recvDataMap.remove(i);
                        removeReceiveTimer(i);
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
                QTableWidgetItem* nextItem = ui->tableSend->item(nextRow - 1, 2);
                if (!ok || !nextItem || nextRow < 1 || nextRow > ui->tableSend->rowCount() ||
                        nextItem->text().isEmpty()) {
                    item->setText("");
                    QMessageBox::warning(nullptr, "Warning", QString("Invalid Next value %1 at row %2").arg(nextRow).arg(item->row()+1));
                } else {
                    // Update the item's UserRole with the actual number
                    item->setData(Qt::UserRole, nextRow);
                    if (isLooped(item->row())) { // Check for loops starting from this row
                        item->setText("");
                        QMessageBox::warning(nullptr, "Warning", "Invalid sequence, loop detected");
                    } else {
                        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(nextRow - 1, 3));
                        if (typeBox && typeBox->currentIndex() != 0) {
                            item->setText("");
                            QMessageBox::warning(nullptr, "Warning", "Next row type must be Send");
                        }
                    }
                }
            }
        });
        ui->tableSend->setItem(i, 4, nextRowItem);

        // Interval column
        QSpinBox *intervalSpinBox = new QSpinBox();
        intervalSpinBox->setRange(0, 1000000);
        connect(intervalSpinBox, QOverload<>::of(&QSpinBox::editingFinished), [=]() {
            if (isCycleSend) {
                if (intervalSpinBox->value() < SET_CYCLE_SEND_MIN_TIME) {
                    intervalSpinBox->setValue(0);
                    setColumnCheckState(i, false);
                    QMessageBox::warning(nullptr, "Invalid Interval", QString("Cycle send interval must be greater than %1!").arg(SET_CYCLE_SEND_MIN_TIME));
                }
            }
        });
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

void NetworkHelper::clearTableSend() {
    // 遍历所有行
    for (int i = 0; i < ui->tableSend->rowCount(); ++i) {
        // 清空时间和数据列
        ui->tableSend->item(i, 1)->setText("");
        ui->tableSend->item(i, 2)->setText("");

        // 取消勾选CK列
        QWidget *widget = ui->tableSend->cellWidget(i, 0);
        QCheckBox *checkBox = widget ? widget->findChild<QCheckBox *>() : nullptr;
        if (checkBox) {
            checkBox->setChecked(false);
        }

        // 重置类型列为"Send"
        QComboBox *comboBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(i, 3));
        if (comboBox) {
            comboBox->setCurrentIndex(0);
        }

        // 清空下一行列
        ui->tableSend->item(i, 4)->setText("");

        // 重置间隔列为0
        QSpinBox *intervalSpinBox = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(i, 5));
        if (intervalSpinBox) {
            intervalSpinBox->setValue(0);
        }
    }
}

bool NetworkHelper::setErrorInfo(int row, const QString& info) {
    QTableWidgetItem* item = this->ui->tableSend->item(row, 1);
    if (item) {
        QBrush redBrush(Qt::red);
        item->setBackground(redBrush); // 将单元格的背景色设置为空，恢复默认颜色
        item->setText(info);
    }

    if (!isPass) {
        setSendState(false);
        if (isSendAndReceive) {
            stopReceiveTimers();
        }
        if (isCycleSend) {
            stopCycleTimers();
        }
    }
    return false;
}

void NetworkHelper::setSendState(bool state) {
    isSending = state;
    if (isSending) {
        ui->comboBox->setEnabled(false);
        ui->buttonSend->setEnabled(false);
        ui->buttonStopSend->setEnabled(true);
    }
    else {
        ui->comboBox->setEnabled(true);
        ui->buttonSend->setEnabled(true);
        ui->buttonStopSend->setEnabled(false);
    }
}

void NetworkHelper::setColumnCheckState(int row, bool check) {
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

void NetworkHelper::setAllColumnUncheck() {
    for(int i = 0; i < ui->tableSend->rowCount(); ++i) {
        QWidget* widget = ui->tableSend->cellWidget(i, 0);
        QCheckBox* checkBox = widget ? widget->findChild<QCheckBox*>() : nullptr;
        if(checkBox) {
            checkBox->setChecked(false);
        }
    }
}

void NetworkHelper::deleteTimer(QTimer* timer) {
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }
}

void NetworkHelper::setSendAndReceive(bool enable) {
    isSendAndReceive = enable;
}

void NetworkHelper::removeReceiveTimer(int row) {
    if (recvTimerMap.contains(row)) {
        deleteTimer(recvTimerMap.value(row).second);
        recvTimerMap.remove(row);
    }
}

void NetworkHelper::stopReceiveTimers() {
    for (auto it = recvTimerMap.begin(); it != recvTimerMap.end(); ++it) {
        deleteTimer(it.value().second);
    }
    recvTimerMap.clear();
}

void NetworkHelper::getReceiveTimers() {
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
    for (auto it = recvTimerMap.begin(); it != recvTimerMap.end(); ++it) {
        int row = it.key();
        int interval = it.value().first;
        QTimer* timer = it.value().second;

        timer->setSingleShot(true); // 单次触发
        timer->setInterval(interval); // 设置定时间隔

        connect(timer, &QTimer::timeout, this, [this, row]() {
            // 定时到达后处理逻辑
            recvTimerMap.remove(row);
            setErrorInfo(row, "Receive timeout");
        });

        timer->start();
    }
}

void NetworkHelper::startSendAndReceive() {
    if (!isSendAndReceive)
        return;

    setAllColumnUncheck();

    stopReceiveTimers();

    getReceiveTimers();

    startReceiveTimers();

    setSendState(true);
}

void NetworkHelper::setCycleSend(bool state) {
    isCycleSend = state;
}

bool NetworkHelper::startCycleTimers() {
    if (!isCycleSend)
        return false;

    setSendState(true);

    bool isStart {false};
    for (QTimer *timer : cycleTimerMap) {
        if (!timer->isActive()) {
            int interval = timer->interval();
            if (interval > 0) {
                timer->start(interval);
                isStart = true;
            }
        }
        else {
            isStart = true;
        }
    }

    return isStart;
}

void NetworkHelper::stopCycleTimers() {
    for (auto it = cycleTimerMap.begin(); it != cycleTimerMap.end(); ++it) {
        deleteTimer(it.value());
    }
    cycleTimerMap.clear();

    setAllColumnUncheck();
}

QList<int> NetworkHelper::checkReceiveDataMap(const QString& timeStamp, const QByteArray& data) {
    for (auto it = recvDataMap.begin(); it != recvDataMap.end(); ++it) {
        if(it.value() == data) {
            // Uncheck the checkbox if received data matches the row data
            if (!isCycleSend) {
                setColumnCheckState(it.key(), true);
            }
            ui->tableSend->setItem(it.key(), 1, new QTableWidgetItem(timeStamp));
            if (isSendAndReceive) {
                removeReceiveTimer(it.key());
                return getContinuousSendMessages(it.key(), false);
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

QList<int> NetworkHelper::getContinuousSendMessages(int start, bool isSend) {
    QList<int> sendList;
    int currentRow = start;
    int nexRow = -1;
    if (!isSend) {
        currentRow = getNextRow(start);
    }

    while (currentRow < ui->tableSend->rowCount()) {
        if (currentRow < 0 || ui->tableSend->item(currentRow, 2)->text().isEmpty()) {
            break;
        }

        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(currentRow, 3));
        if (typeBox && typeBox->currentIndex() == 0) {
            sendList.push_back(currentRow);
            nexRow = getNextRow(currentRow);
            if (nexRow == currentRow) {
                break;
            }
            currentRow = nexRow;
        } else {
            break;
        }
    }

    return sendList;
}

QList<int> NetworkHelper::getAllSendMessages() {
    QList<int> sendList;
    for (int currentRow = 0; currentRow < ui->tableSend->rowCount(); ++currentRow) {
        if (ui->tableSend->item(currentRow, 2)->text().isEmpty()) {
            break;
        }

        QComboBox *typeBox = qobject_cast<QComboBox *>(ui->tableSend->cellWidget(currentRow, 3));
        if (typeBox && typeBox->currentIndex() == 0) {
            sendList.push_back(currentRow);
        }
    }
    return sendList;
}

void NetworkHelper::tryStopSend() {
    if (!recvDataMap.empty())
        return;

    setSendState(false);
}
