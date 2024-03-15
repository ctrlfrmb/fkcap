#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QInputDialog>
#include <QFileDialog>
#include <QClipboard>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filterwindow.h"
#include "ipcap.h"
#include "config.h"
#include "protocol/ip.h"

MainWindow::MainWindow(bool isStart, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initWindow(isStart);
}

MainWindow::~MainWindow()
{
    exitWindow();

    QSqlDatabase::removeDatabase(FKCAP_SQLITE_CONNECT_NAME);

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
    this->ui->tableView->setModel(pim);
    // 设置列宽比例 "Index" "Timestamp" "Source IP" "Destination IP" "Protocol" "Payload Length" "Information"
    this->ui->tableView->setColumnWidth(0, 50);
    this->ui->tableView->setColumnWidth(1, 100);
    this->ui->tableView->setColumnWidth(2, 110);
    this->ui->tableView->setColumnWidth(3, 110);
    this->ui->tableView->setColumnWidth(4, 55);
    this->ui->tableView->setColumnWidth(5, 50);

    connect(this->ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
             this, [&](int value) {
               QScrollBar *scrollBar = this->ui->tableView->verticalScrollBar();
               scrollBarAtBottom = value == scrollBar->maximum();
               userHasScrolled = true;
             });
    connect(this->ui->tableView->verticalScrollBar(), &QScrollBar::sliderReleased, this, [&]() {
       if (scrollBarAtBottom)
           userHasScrolled = false;
     });

    connect(this->ui->tableView, &QTableView::doubleClicked, this, &MainWindow::onTableViewDoubleClicked);
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
        item->setEditable(false); // 设置该项为不可编辑
        tvm->setItem(i, 0, item);
    }

    QStringList headers;
    headers<<"Index:"<<"0";
    tvm->setHorizontalHeaderLabels(headers);

    ui->treeView->setModel(tvm);

    // 设置第一列的列宽为150，且获取水平表头对象并设置对齐方式为 Qt::AlignHCenter
    ui->treeView->setColumnWidth(0, 150);
    ui->treeView->header()->setDefaultAlignment(Qt::AlignHCenter);
}

void MainWindow::initWindow(bool isStart)
{
    initTableView();
    initTreeView();
    server.setWindow(true);

    using namespace figkey;
    const auto& cfg = CaptureConfig::Instance().getConfigInfo();

    // 使用std::bind设置回调函数
    IPPacketParse::Instance().setCallback(std::bind(&MainWindow::processPacket, this, std::placeholders::_1));

    timerUpdateUI = new QTimer(this);
    connect(timerUpdateUI, &QTimer::timeout, this, &MainWindow::updateUI);
    timerUpdateUI->start(cfg.timeUpdateUI);

    if (isStart) {
        on_actionStart_triggered();
    }
}

void MainWindow::exitWindow() {
    figkey::NpcapCom::Instance().exit();
    if (client.isVisible()) {
        client.close();
    }
    if (server.isVisible()) {
        server.close();
    }
    if (doipClient.isVisible()) {
        doipClient.close();
    }
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

void MainWindow::updateTreeView(const figkey::PacketInfo& packet) {
    QStringList valueList;
    valueList << QString::fromStdString(packet.timestamp)
              << QString::number(packet.err)
              << QString::fromStdString(packet.srcIP)
              << QString::fromStdString(packet.destIP)
              << QString::fromStdString(packet.srcMAC)
              << QString::fromStdString(packet.destMAC)
              << QString::number(packet.srcPort)
              << QString::number(packet.destPort)
              << QString::number(packet.protocolType)
              << QString::number(packet.payloadLength)
              << QString::fromStdString(packet.data);

    for(int i = 0; i < valueList.size(); i++) {
        QStandardItem *item = new QStandardItem(valueList[i]);
        tvm->setItem(i, 1, item);
    }

    QStringList headers;
    headers <<"Index:"<<QString::number(packet.index);
    tvm->setHorizontalHeaderLabels(headers);

    ui->treeView->update();
}

void MainWindow::updateUI() {
    {
        QMutexLocker locker(&mutexPacket);
        if (packetCounter > 0)
            db.writeFile();
    }

    if (figkey::NpcapCom::Instance().getIsRunning()) {
        ui->tableView->update();

        if (!userHasScrolled || scrollBarAtBottom)
            ui->tableView->scrollToBottom();
    }
}

void MainWindow::onTableViewDoubleClicked(const QModelIndex& index) {
    if (index.column() == 6) {  // 如果是最后一列
        QString text = index.data().toString();  // 获取该项的文本

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(text);
    }
}

void MainWindow::processPacket(figkey::PacketInfo packetInfo)
{
    QMutexLocker locker(&mutexPacket);
    packetInfo.index = ++packetCounter;
    if (1 == packetInfo.index) {
        pim->clearPacket();
        ui->tableView->update();
        //updateTreeView(packetInfo);
    }

    db.storePacket(packetInfo);
    pim->addPacket(packetInfo);
}

void MainWindow::on_actionStop_triggered()
{
    qDebug() <<"stop capture";

    ui->actionStart->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);

    figkey::NpcapCom::Instance().stopCapture();

    {
        QMutexLocker locker(&mutexPacket);
        db.closeFile();
        packetCounter = 0;
    }
}

void MainWindow::on_actionStart_triggered()
{
    qDebug() <<"start capture";
    using namespace figkey;
    ui->actionStart->setEnabled(false);

    if (db.openFile()) {
        ui->actionSave->setEnabled(true);
    }

    if (NpcapCom::Instance().run()) {
        ui->actionPause->setEnabled(true);
        ui->actionStop->setEnabled(true);
    }
    else {
        ui->actionStart->setEnabled(true);
        QMessageBox::critical(nullptr, "Error",
                              QString("Failed to capture network data, please check the configuration file or contact the administrator")
                              );
    }
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
    if (!doipClient.isVisible()) {
        doipClient.show();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    auto& pcap = figkey::NpcapCom::Instance();

    if (pcap.getIsRunning()) {
        // 调用函数停止数据捕获
        on_actionStop_triggered();
    }

    if (db.loadFile()) {
       auto packets = db.getPacket(1, figkey::CaptureConfig::Instance().getConfigInfo().displayRows);
       if (packets.empty()) {
           QMessageBox::warning(nullptr, "Warning",
                                QString("The current database file is an empty file")
                                );
           return;
       }

       pim->loadPackect(packets);
       ui->tableView->update();
    }
}

void MainWindow::on_actionSave_triggered()
{
    ui->actionSave->setEnabled(false);
    db.saveFile();
    ui->actionSave->setEnabled(true);
}

void MainWindow::updateTreeViewByIndex(const QModelIndex& index) {
    if (!index.isValid())
        return;

    QAbstractTableModel* model = qobject_cast<QAbstractTableModel*>(ui->tableView->model());
    if (!model)
        return;

    userHasScrolled = true;
    scrollBarAtBottom = false;
    currentIndex = index.row();
    updateTreeView(pim->getPacketByIndex(currentIndex));
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    updateTreeViewByIndex(index);
}

void MainWindow::on_actionClient_triggered()
{
    if (!client.isVisible()) {
        client.set(pim->getPacketByIndex(currentIndex));
        client.show();
    }
}

void MainWindow::on_actionServer_triggered()
{
    if (!server.isVisible()) {
        server.set(pim->getPacketByIndex(currentIndex));
        server.show();
    }
}

void MainWindow::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    updateTreeViewByIndex(ui->tableView->currentIndex());

    QMenu *menu = new QMenu(this);

    QAction *actionClient = new QAction("Client Test", this);
    QAction *actionServer = new QAction("Server Test", this);
    actionClient->setIcon(QIcon(":/images/resource/icons/client.png"));
    actionServer->setIcon(QIcon(":/images/resource/icons/server.png"));

    connect(actionClient, &QAction::triggered, this, &MainWindow::on_actionClient_triggered);
    connect(actionServer, &QAction::triggered, this, &MainWindow::on_actionServer_triggered);

    menu->addAction(actionClient);
    menu->addAction(actionServer);

    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::on_actionOpen_Test_triggered()
{

     QString fileName = QFileDialog::getOpenFileName(this, "Open config file",
                                                     QDir::homePath(),
                                                     "Config Files (*.json)");

     // Check if fileName is empty (cancel was pressed)
     if (fileName.isEmpty()) {
         QMessageBox::warning(this, "Warning", "No file selected.");
         return;
     }

     bool isServer { true };
     if (!NetworkAssistWindow::isServerByLoadFile(fileName, isServer))
         return;

     if (isServer) {
         if (server.isVisible()) {
             QMessageBox::information(this, tr("Information"),
                                          tr("Please close the server test window before loading the configuration file."));
             return;
         }
         server.loadConfigFromFile(fileName);
     }
     else {
         if (client.isVisible()) {
             QMessageBox::information(this, tr("Information"),
                                          tr("Please close the client test window before loading the configuration file."));
             return;
         }

         client.loadConfigFromFile(fileName);
     }
}

void MainWindow::on_actionSave_Test_triggered()
{
    client.saveConfigToFile();
}

void MainWindow::on_actionSave_Server_Test_triggered()
{
    server.saveConfigToFile();
}
