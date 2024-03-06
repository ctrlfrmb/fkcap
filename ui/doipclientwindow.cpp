#include "doipclientwindow.h"
#include "ui_doipclientwindow.h"
#include "doip/doipclientconfig.h"
#include "config.h"
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QHBoxLayout>

DoIPClientWindow::DoIPClientWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoIPClientWindow)
{
    ui->setupUi(this);
    initWindow();
}

DoIPClientWindow::~DoIPClientWindow()
{
    delete ui;
}

void DoIPClientWindow::initSetting() {
    // Fetch the singleton configuration object
    auto& config = figkey::DoIPClientConfig::Instance();

    // Set the initial value of the controls
    //ui->spinDoIPVersion->setRange(0, 254);  // version范围0x00~0xFE
    ui->spinDoIPVersion->setValue(config.getVersion());

    // localIP、serverIP为ip地址，可以设定适当的验证规则
    QRegExp ipRegExp("^((2[0-4]\\d|25[0-5]|[01]?\\d?\\d)(\\.(2[0-4]\\d|25[0-5]|[01]?\\d?\\d)){3})$");
    ui->txtLocalIP->setValidator(new QRegExpValidator(ipRegExp, this));
    ui->txtLocalIP->setText(QString::fromStdString(config.getLocalIP()));

    ui->txtServerIP->setValidator(new QRegExpValidator(ipRegExp, this));
    ui->txtServerIP->setText(QString::fromStdString(config.getServerIP()));

    // tcpPort为网络端口，取值范围0-65535
    ui->txtTCPPort->setValidator(new QIntValidator(0, 65535, this));
    ui->txtTCPPort->setText(QString::number(config.getTcpPort()));

    // sourceAddress范围0x0e00~0x0eff，两者都是unsigned short类型
    ui->txtSourceAddress->setValidator(new QRegExpValidator(QRegExp("^[0][Ee][0-9a-fA-F]{2}$"), this));
    ui->txtSourceAddress->setText(QString("%1").arg(config.getSourceAddress(), 4, 16, QChar('0')).toUpper());

    // targetAddress为除 0x0e00~0x0eff 之外的值
    ui->txtTargetAddress->setValidator(new QRegExpValidator(QRegExp("^(?![0][Ee][0-9a-fA-F]{2}$)[0-9a-fA-F]{4}$"), this));
    ui->txtTargetAddress->setText(QString("%1").arg(config.getTargetAddress(), 4, 16, QChar('0')).toUpper());

    ui->spinActiveType->setValue(config.getActiveType());

    ui->checkUseOEMSpecific->setChecked(config.getUseOEMSpecific());

    // additionalOEMSpecific、futureStandardization均为4字节unsigned char，即8个十六进制字符
    QRegExp hex8RegExp("^[0-9a-fA-F]{8}$");
    ui->txtOEMSpecific->setValidator(new QRegExpValidator(hex8RegExp, this));
    ui->txtOEMSpecific->setText(QString::fromUtf8(config.getAdditionalOEMSpecific().toHex()).toUpper());

    // Connect the controls' signals to the slots updating the configuration
    connect(ui->spinDoIPVersion, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [](int newValue) {
            figkey::DoIPClientConfig::Instance().setVersion(newValue);
        });

    connect(ui->txtLocalIP, &QLineEdit::textChanged, [](const QString& newText){
        figkey::DoIPClientConfig::Instance().setLocalIP(newText);
    });
    connect(ui->txtServerIP, &QLineEdit::textChanged, [](const QString& newText){
        figkey::DoIPClientConfig::Instance().setServerIP(newText);
    });
    connect(ui->txtTCPPort, &QLineEdit::textChanged, [](const QString& newText) {
        figkey::DoIPClientConfig::Instance().setTcpPort(newText.toInt());
    });

    connect(ui->txtSourceAddress, &QLineEdit::textChanged, [](const QString& newText) {
        figkey::DoIPClientConfig::Instance().setSourceAddress(newText.toUpper().toUShort(nullptr, 16));
    });
    connect(ui->txtTargetAddress, &QLineEdit::textChanged, [](const QString& newText) {
        figkey::DoIPClientConfig::Instance().setTargetAddress(newText.toUpper().toUShort(nullptr, 16));
    });

    connect(ui->spinActiveType, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [](int newValue) {
            figkey::DoIPClientConfig::Instance().setActiveType(newValue);
        });

    connect(ui->checkUseOEMSpecific, &QCheckBox::stateChanged, [](int state) {
        figkey::DoIPClientConfig::Instance().setUseOEMSpecific(state == Qt::Checked);
    });

    connect(ui->txtOEMSpecific, &QLineEdit::textChanged, [](const QString& newText) {
        figkey::DoIPClientConfig::Instance().setAdditionalOEMSpecific(QByteArray::fromHex(newText.toUpper().toUtf8()));
    });
}

void DoIPClientWindow::initTableSend() {
    auto& config = figkey::CaptureConfig::Instance().getConfigInfo();
    ui->tableSend->setRowCount(config.doipClientSend);
    ui->tableSend->setColumnCount(5);
    QStringList headers;
    headers << "Time" << "UDS" << "Repeat" << "Interval" << "Send";
    ui->tableSend->setHorizontalHeaderLabels(headers);

    ui->tableSend->setColumnWidth(0, 100); // Time 列
    ui->tableSend->setColumnWidth(1, 240); // UDS 列
    ui->tableSend->setColumnWidth(2, 60);  // Repeat 列
    ui->tableSend->setColumnWidth(3, 80);  // Interval 列
    ui->tableSend->setColumnWidth(4, 100); // Send 列

    for(int i=0; i< config.doipClientSend; i++) {
        // 初始时间列设置为当前时间
        QTableWidgetItem *timeItem = new QTableWidgetItem(""); //QTime::currentTime().toString("hh:mm:ss.zzz")
        ui->tableSend->setItem(i, 0, timeItem);
        // UDS 列只接受16进制数字输入，使用正则表达式进行校验
        QRegExp rx("[0-9A-Fa-f ]{1,}");
        QRegExpValidator *validator = new QRegExpValidator(rx, this);
        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setValidator(validator);
        ui->tableSend->setCellWidget(i, 1, lineEdit);
        // 时间间隔设为SpinBox，范围从1到60*60*1000，一开始禁止编辑
        QSpinBox *intervalSpin = new QSpinBox(this);
        // Repeat 列使用复选框
        QCheckBox *checkBox = new QCheckBox(this);
        connect(checkBox, &QCheckBox::stateChanged, [intervalSpin](int state){
            if(state == Qt::Checked) {
                // CheckBox被勾选，interval可编辑
                intervalSpin->setEnabled(true);
            } else {
                // CheckBox没有被勾选，interval不可编辑
                intervalSpin->setEnabled(false);
            }
        });

        // 创建一个QWidget作为容器
        QWidget *checkBoxWidget = new QWidget(this);
        checkBoxWidget->setStyleSheet("background-color:transparent;"); // 设定为透明背景

        // 创建布局进行居中布局
        QHBoxLayout *layout = new QHBoxLayout(checkBoxWidget);
        layout->addWidget(checkBox);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0,0,0,0); // 设置布局内边距为0

        // 把QWidget容器设置为CellWidget
        ui->tableSend->setCellWidget(i, 2, checkBoxWidget);

        intervalSpin->setRange(1, 60*60*1000);
        intervalSpin->setEnabled(false);
        intervalSpin->setValue(1000);
        ui->tableSend->setCellWidget(i, 3, intervalSpin);

        // Send 列为PushButton
        QPushButton *sendButton = new QPushButton(this);
        sendButton->setText("Send");
        ui->tableSend->setCellWidget(i, 4, sendButton);

        // sendButton被点击时，调用slotSendData()槽函数
        connect(sendButton, &QPushButton::clicked, [this, i, lineEdit, checkBox, intervalSpin]() { slotSendData(i, lineEdit, checkBox, intervalSpin); });
    }
}

void DoIPClientWindow::initTableReceive() {
    // Set the number of rows and columns
    ui->tableReceive->setColumnCount(2);

    // Set horizontal header labels
    QStringList headers;
    headers << "Time" << "Data";
    ui->tableReceive->setHorizontalHeaderLabels(headers);

    // Set column width
    ui->tableReceive->setColumnWidth(0, 100);
    ui->tableReceive->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void DoIPClientWindow::initWindow() {
    qRegisterMetaType< QVector<int> >("QVector<int>");
    doip.SetMessageCallback(std::bind(&DoIPClientWindow::receiveMessage, this, std::placeholders::_1));

    initSetting();
    initTableSend();
    initTableReceive();

    ui->tableSend->setEnabled(false);
    ui->buttonDisconnect->setEnabled(false);
}

void DoIPClientWindow::closeEvent(QCloseEvent *event) {
    doip.closeTcpConnection();
    event->accept();
}

void DoIPClientWindow::on_buttonConnect_clicked()
{
    isConnected = doip.startTcpConnection();
    doip.sendRoutingActivationRequest();

    ui->tableSend->setEnabled(isConnected);
    ui->buttonDisconnect->setEnabled(isConnected);

    if (isConnected)
        ui->buttonConnect->setEnabled(false);
}

void DoIPClientWindow::on_buttonDisconnect_clicked()
{
    ui->buttonDisconnect->setEnabled(false);

    {
        // 停止所有定时器
        for(int timerID : timerIDs) {
            killTimer(timerID);
        }
        QMutexLocker locker(&mutexTimers);
        timerIDs.clear(); // 清空列表，为下一次连接做准备
    }

    doip.closeTcpConnection();
    isConnected = false;

    ui->tableSend->setEnabled(false);
    ui->buttonConnect->setEnabled(true);
}

void DoIPClientWindow::slotSendData(int row, QLineEdit *lineEdit, QCheckBox *checkBox, QSpinBox *intervalSpin) {
    if (!isConnected) {
        QMessageBox::warning(this, "warnning", "Not connected to server!");
        return;
    }

    // 检查是否勾选重复发送选项
    if(checkBox->isChecked()) {
        if(timerIDs.contains(row)) {
            killTimer(timerIDs[row]); // 如果已有定时器，杀掉前一个定时器
        }
        int timerID = startTimer(intervalSpin->value()); // 以用户设置的时间间隔启动定时器
        QMutexLocker locker(&mutexTimers);
        timerIDs[row] = timerID; // 将定时器ID保存到map中
    } else {
        if(timerIDs.contains(row)) {
            killTimer(timerIDs[row]); // 如果已有定时器，则取消定时器
            QMutexLocker locker(&mutexTimers);
            timerIDs.remove(row); // 从map中移除定时器ID
        }
    }

    // 首先检查用户是否输入了数据
    if(lineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "error", "Data cannot be empty!");
        return;
    }
    // 检查用户输入的数据是否是16进制
    QByteArray data = QByteArray::fromHex(lineEdit->text().toUtf8());
    if (data.isEmpty()) {
        QMessageBox::warning(this, "error", "Please enter valid hexadecimal data!");
        return;
    }

    //qDebug() << "send uds: " << data.toHex();
    std::vector<uint8_t> vecData(data.constBegin(), data.constEnd()); // 从 QByteArray 转换为 std::vector

    if(!doip.sendDiagnosticMessage(vecData)) {
        QMessageBox::warning(this, "error", "send failed!");
        return;
    }

    // 发送完数据后，更新当前行的时间列
   QTableWidgetItem *timeItem = new QTableWidgetItem(QTime::currentTime().toString("hh:mm:ss.zzz"));
   ui->tableSend->setItem(row, 0, timeItem);
}

void DoIPClientWindow::timerEvent(QTimerEvent *event) {
    int timerID = event->timerId(); // 获取定时器ID
    int row = timerIDs.key(timerID); // 从map中查找行号

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(ui->tableSend->cellWidget(row, 1));
    // 在 timerEvent 函数中，首先获取 checkBox 的包裹器（container）
    QWidget *checkBoxContainer = qobject_cast<QWidget *>(ui->tableSend->cellWidget(row, 2));
    // 然后从布局中获取 checkBox
    QCheckBox *checkBox = nullptr;
    if (checkBoxContainer) {
        checkBox = qobject_cast<QCheckBox *>(checkBoxContainer->layout()->itemAt(0)->widget());
    }
    QSpinBox *intervalSpin = qobject_cast<QSpinBox *>(ui->tableSend->cellWidget(row, 3));
    QPushButton *sendButton = qobject_cast<QPushButton *>(ui->tableSend->cellWidget(row, 4));
    if(lineEdit && checkBox && intervalSpin && sendButton) {
        slotSendData(row, lineEdit, checkBox, intervalSpin); // 再次调用发送数据函数
    }
}

void DoIPClientWindow::receiveMessage(std::vector<uint8_t> data) {
    // Format the data as a hexadecimal string
    QString dataString;
    for (uint8_t byte : data) {
        // use QString's arg function to format the byte value as a zero-padded two-digit hexadecimal number
        dataString.append(QString("%1 ").arg(byte, 2, 16, QChar('0')).toUpper());
    }

    QString timeStamp = QTime::currentTime().toString("hh:mm:ss.zzz");

    //qDebug() << timeStamp << dataString;

    // Remove the first row
    if (ui->tableReceive->rowCount() >= figkey::CaptureConfig::Instance().getConfigInfo().doipClientReceive)
        ui->tableReceive->removeRow(0);

    // Add a new row at the end
    int newRow = ui->tableReceive->rowCount();
    ui->tableReceive->insertRow(newRow);

    // Add the timestamp and message to the table
    ui->tableReceive->setItem(newRow, 0, new QTableWidgetItem(timeStamp));
    ui->tableReceive->setItem(newRow, 1, new QTableWidgetItem(dataString));
}

