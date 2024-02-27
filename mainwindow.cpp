#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ipcap.h"
#include "config.h"
#include "protocol/ip.h"
#include <QDebug>

#define CONFIG_FILE_PATH "/config/figkey.ini"
#define SQLITE_DATABASE_PATH "/db/figkey.db"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    LoadConfigFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LoadConfigFile()
{
    using namespace figkey;

    db.loadFile(SQLITE_DATABASE_PATH);

    CaptureConfigInfo cfg;
    QStringList arguments = QCoreApplication::arguments();
    QString path = QCoreApplication::applicationDirPath();
    if (arguments.size() > 1) {
        path =  arguments[1];
    }
    else {
        path += CONFIG_FILE_PATH;
    }

    // 使用std::bind设置回调函数
    auto& ip = IPPacketParse::Instance();
    ip.setCallback(std::bind(&MainWindow::processPacket, this, std::placeholders::_1));

    qDebug() << "figkey ip capture tool default config file: "<<path;
    cfg = CaptureConfig::Instance().getFilterProtocol(path.toStdString());

    auto& pcap = NpcapCom::Instance();

    if (!cfg.filter.empty())
        pcap.setPacketFilter(cfg.filter);

    pcap.startCapture(cfg.type, cfg.save);
}

void MainWindow::processPacket(figkey::PacketInfo packetInfo)
{

}
