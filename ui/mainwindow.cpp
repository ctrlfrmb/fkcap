#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filterwindow.h"
#include "doipclientwindow.h"
#include "ipcap.h"
#include "config.h"
#include "protocol/ip.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QInputDialog>
#include <QFileDialog>

#define SQLITE_DATABASE_PATH "/db/figkey.db"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initWindow();
}

MainWindow::~MainWindow()
{
    exitWindow();

    figkey::IPPacketParse::Instance().setCallback(nullptr);

    if (timerUpdateUI)
        delete timerUpdateUI;  // 在析构函数中删除定时器
    if (pim)
        delete pim;
    if (tvm)
        delete tvm;
    delete ui;
}

void MainWindow::initTableView() {
    pim = new PacketInfoModel(this);
    ui->tableView->setModel(pim);
    // 设置列宽比例 "Index" "Timestamp" "Source IP" "Destination IP" "Protocol" "Payload Length" "Information"
    ui->tableView->setColumnWidth(0, 50);
    ui->tableView->setColumnWidth(1, 100);
    ui->tableView->setColumnWidth(2, 110);
    ui->tableView->setColumnWidth(3, 110);
    ui->tableView->setColumnWidth(4, 55);
    ui->tableView->setColumnWidth(5, 50);
    ui->tableView->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
}

void MainWindow::initTreeView() {
    tvm = new QStandardItemModel(this);

    // 对PacketInfo中的每一个字段，新建一行，标签在第一列，初始值在第二列
    QStringList headerList;
    headerList << "timestamp:" << "error code:" << "source ip:" << "destination ip:" << "source mac:"
               << "destination mac:" << "source port:" << "destination port:" << "protocol type:"
               << "payload length:" << "data:";

    for(int i = 0; i < headerList.size(); i++) {
        QStandardItem *item = new QStandardItem(headerList[i] + " ");
        // 设置文本对齐方式为右对齐
        item->setTextAlignment(Qt::AlignVCenter);
        tvm->setItem(i, 0, item);
    }

    QStringList headers;
    headers<<"Index:"<<"0";
    tvm->setHorizontalHeaderLabels(headers);

    ui->treeView->setModel(tvm);

    // 设置第一列的列宽为180，且获取水平表头对象并设置对齐方式为 Qt::AlignHCenter
    ui->treeView->setColumnWidth(0, 150);
    ui->treeView->header()->setDefaultAlignment(Qt::AlignHCenter);
}

void renameFileIfExists(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        bool ok;
        QString text = QInputDialog::getText(nullptr, "File exists",
                                             "File is already exists, please enter new name:",
                                             QLineEdit::Normal,
                                             fileInfo.fileName(), &ok);
        if (ok && !text.isEmpty())
        {
            // 执行重命名操作
            QString newFilePath = fileInfo.dir().filePath(text);
            if (QFile::rename(filePath, newFilePath))
            {
                QMessageBox::information(nullptr, "Rename success",
                                         "File has been renamed successfully",
                                         QMessageBox::Ok);
                return;
            }
        }

        if(QFile::remove(filePath))
        {
            QMessageBox::information(nullptr, "File deletion",
                                     "File has been deleted successfully",
                                     QMessageBox::Ok);
        }
        else
        {
            QMessageBox::warning(nullptr, "File deletion",
                                 "Failed to delete the file",
                                 QMessageBox::Ok);
        }
    }
    else
    {
        // 文件不存在，进行其他操作...
    }
}

void MainWindow::initWindow()
{
    initTableView();
    initTreeView();

    using namespace figkey;

    QString path = QCoreApplication::applicationDirPath();
    path+=SQLITE_DATABASE_PATH;
    renameFileIfExists(path);
    db.openFile(path);

    const auto& cfg = CaptureConfig::Instance().getConfigInfo();

    NpcapCom::Instance().asyncStartCapture();

    // 使用std::bind设置回调函数
    IPPacketParse::Instance().setCallback(std::bind(&MainWindow::processPacket, this, std::placeholders::_1));

    timerUpdateUI = new QTimer(this);
    connect(timerUpdateUI, &QTimer::timeout, this, &MainWindow::updateUI);
    timerUpdateUI->start(cfg.timeUpdateUI); // 设置时间间隔为 1000 毫秒
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
             this, [&](int value) {
               QScrollBar *scrollBar = ui->tableView->verticalScrollBar();
               scrollBarAtBottom = value == scrollBar->maximum();
               userHasScrolled = true;
             });
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::sliderReleased, this, [&]() {
       if (scrollBarAtBottom)
           userHasScrolled = false;
     });

    ui->actionStart->setEnabled(false);
}

void MainWindow::exitWindow() {
    figkey::NpcapCom::Instance().stopCapture();
    if(timerUpdateUI)
        timerUpdateUI->stop();
    db.closeFile();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    int ret = QMessageBox::question(this, tr("Tips"), tr("Are you sure to close the program?"), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        exitWindow();
        event->accept();
    } else {
        // 当你确认不需要关闭窗口时，调用此句：
        event->ignore();
    }
}

void MainWindow::updateTreeView() {
    const auto& info = pim->getLastPacket(); //获取首行数据

    QStringList valueList;
    valueList << QString::fromStdString(info.timestamp)
              << QString::number(info.err)
              << QString::fromStdString(info.srcIP)
              << QString::fromStdString(info.destIP)
              << QString::fromStdString(info.srcMAC)
              << QString::fromStdString(info.destMAC)
              << QString::number(info.srcPort)
              << QString::number(info.destPort)
              << QString::number(info.protocolType)
              << QString::number(info.payloadLength)
              << QString::fromStdString(info.data);

    for(int i = 0; i < valueList.size(); i++) {
        QStandardItem *item = new QStandardItem(valueList[i]);
        tvm->setItem(i, 1, item);
    }

    QStringList headers;
    headers <<"Index:"<<QString::number(info.index);
    tvm->setHorizontalHeaderLabels(headers);

    ui->treeView->update();
}

void MainWindow::updateUI() {
    if (figkey::NpcapCom::Instance().getIsRunning()) {
        ui->tableView->update();
        updateTreeView(); //更新Treeview

        if (!userHasScrolled || scrollBarAtBottom)
            ui->tableView->scrollToBottom();
    }
}

void MainWindow::processPacket(figkey::PacketInfo packetInfo)
{
    //static QAtomicInt count{0};
    //qDebug() <<"capture "<<++count;
    QMutexLocker locker(&mutexPacket);
    packetInfo.index = ++packetCounter;

    pim->addPacket(packetInfo);
    db.storePacket(packetInfo);
}

void MainWindow::on_actionStop_triggered()
{
    qDebug() <<"stop capture";
    figkey::NpcapCom::Instance().stopCapture();
    db.closeFile();

    ui->actionStart->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);
}

void MainWindow::on_actionStart_triggered()
{
    qDebug() <<"start capture";
    figkey::NpcapCom::Instance().asyncStartCapture();

    ui->actionStart->setEnabled(false);
    ui->actionPause->setEnabled(true);
    ui->actionStop->setEnabled(true);
}

void MainWindow::pauseCapture()
{
    auto& pcap = figkey::NpcapCom::Instance();
    if (ui->actionPause->text() == "Pause") {
        qDebug() <<"pause capture";
        ui->actionPause->setText("Resume");
        QIcon icon(":/images/resource/icons/capture_resume.png");
        ui->actionPause->setIcon(icon);

        pcap.setIsRunning(false);
    }
    else {
        qDebug() <<"resume capture";
        ui->actionPause->setText("Pause");
        QIcon icon(":/images/resource/icons/capture_pause.png");
        ui->actionPause->setIcon(icon);

        pcap.setIsRunning(true);
    }
}

void MainWindow::on_actionPause_triggered()
{
    pauseCapture();
}

void MainWindow::on_actionFilter_triggered()
{
    pauseCapture();

    FilterWindow f;
    f.adjustSize();
    f.setFixedSize(f.size());
    f.exec();

    pauseCapture();
}

void MainWindow::on_actionFilter_Clear_triggered()
{
    pauseCapture();

    figkey::FilterInfo filter;
    figkey::CaptureConfig::Instance().setFilter(filter);

    pauseCapture();
}

void MainWindow::on_actionDoIP_Client_triggered()
{
    DoIPClientWindow dc;
    dc.adjustSize();
    dc.setFixedSize(dc.size());
    dc.exec();
}

void MainWindow::on_actionOpen_triggered()
{
    auto& pcap = figkey::NpcapCom::Instance();
    if(pcap.getIsRunning()) {
        QMessageBox::warning(this, "warnning", "Data is currently being captured, please stop capturing first and remember to save the data.");
        return;
    }


    QString fileName = QFileDialog::getOpenFileName(
    nullptr,             // 父窗口，如果没有则为nullptr
    "Open Capture File",         // 对话框标题
    QCoreApplication::applicationDirPath(),             // 默认打开路径
    "Sqlite DataBase (*.db);;Text files (*.txt)");    // 文件过滤器
    //db.openFile()
}

void MainWindow::on_actionSave_triggered()
{

}
