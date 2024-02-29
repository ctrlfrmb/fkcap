#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ipcap.h"
#include "config.h"
#include "protocol/ip.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

#define SQLITE_DATABASE_PATH "/db/figkey.db"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), pim(nullptr), tvm(nullptr), m_timerUpdateUI(nullptr)
{
    ui->setupUi(this);

    initWindow();
}

MainWindow::~MainWindow()
{
    if (m_timerUpdateUI)
        delete m_timerUpdateUI;  // 在析构函数中删除定时器
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
//    int tableWidth = ui->tableView->geometry().width();
//    int total = 2 + 5 + 5 + 5 + 1 + 2 + 30; // 比例和
//    ui->tableView->setColumnWidth(0, tableWidth * 2 / total);
//    ui->tableView->setColumnWidth(1, tableWidth * 5 / total);
//    ui->tableView->setColumnWidth(2, tableWidth * 5 / total);
//    ui->tableView->setColumnWidth(3, tableWidth * 5 / total);
//    ui->tableView->setColumnWidth(4, tableWidth * 1 / total);
//    ui->tableView->setColumnWidth(5, tableWidth * 2 / total);
//    ui->tableView->setColumnWidth(6, tableWidth * 30 / total);
//    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

void MainWindow::initWindow()
{
    initTableView();
    initTreeView();

    using namespace figkey;

    db.loadFile(SQLITE_DATABASE_PATH);

    const auto& cfg = CaptureConfig::Instance().getConfigInfo();

    NpcapCom::Instance().asyncStartCapture();

    // 使用std::bind设置回调函数
    IPPacketParse::Instance().setCallback(std::bind(&MainWindow::processPacket, this, std::placeholders::_1));

    m_timerUpdateUI = new QTimer(this);
    connect(m_timerUpdateUI, &QTimer::timeout, this, &MainWindow::updateUI);
    m_timerUpdateUI->start(cfg.timeUpdateUI); // 设置时间间隔为 1000 毫秒

    ui->actionStart->setEnabled(false);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    int ret = QMessageBox::question(this, tr("Tips"), tr("Are you sure to close the program?"), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        // 当你确认需要关闭窗口时，调用此句：
        figkey::NpcapCom::Instance().stopCapture();
        m_timerUpdateUI->stop();
        event->accept();
    } else {
        // 当你确认不需要关闭窗口时，调用此句：
        event->ignore();
    }
}

void MainWindow::updateTreeView() {
    const auto& info = pim->getFirstPacket(); //获取首行数据

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
    ui->tableView->update();  // 或者你需要的其他刷新表格的操作
    updateTreeView(); //更新Treeview
}

void MainWindow::processPacket(figkey::PacketInfo packetInfo)
{
    qDebug() <<packetInfo.index<<", payload length:"<<packetInfo.payloadLength;
    if (packetInfo.index == 0)
        qDebug() << packetInfo.srcIP.c_str()<<"->"<<packetInfo.destIP.c_str()<<" payload length:"
             <<packetInfo.payloadLength<<", "<<packetInfo.data.c_str();
    else
        pim->addPacket(std::move(packetInfo));
}

void MainWindow::on_actionStop_triggered()
{
    figkey::NpcapCom::Instance().stopCapture();

    ui->actionStart->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);
}

void MainWindow::on_actionStart_triggered()
{
    figkey::NpcapCom::Instance().asyncStartCapture();

    ui->actionStart->setEnabled(false);
    ui->actionPause->setEnabled(true);
    ui->actionStop->setEnabled(true);
}

void MainWindow::on_actionPause_triggered()
{
    auto& pcap = figkey::NpcapCom::Instance();
    if (ui->actionPause->text() == "Pause") {
        ui->actionPause->setText("Resume");
        QIcon icon(":/images/resource/icons/capture_resume.png");
        ui->actionPause->setIcon(icon);

        pcap.setIsRunning(false);
    }
    else {
        ui->actionPause->setText("Pause");
        QIcon icon(":/images/resource/icons/capture_pause.png");
        ui->actionPause->setIcon(icon);

        pcap.setIsRunning(true);
    }
}
