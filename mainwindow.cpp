#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ipcap/include/ipcap.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    using namespace figkey;
    NpcapCom pcap;
    auto networkList = pcap.getNetworkList();
}

MainWindow::~MainWindow()
{
    delete ui;
}
