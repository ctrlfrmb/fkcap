#include "devicewindow.h"
#include "ui_devicewindow.h"
#include "ipcap.h"
#include "config.h"
#include <QStandardItemModel>
#include <sstream>
#include <QDebug>
#include <QMessageBox>

Q_DECLARE_METATYPE(figkey::NetworkInfo)

DeviceWindow::DeviceWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceWindow), model(nullptr)
{
    ui->setupUi(this);
    initWindow();
}

DeviceWindow::~DeviceWindow()
{
    if (model)
        delete model;
    delete ui;
}

void DeviceWindow::initWindow()
{
    auto& pcap = figkey::NpcapCom::Instance();
    auto networkList = pcap.getNetworkList();
    model = new QStandardItemModel();

    for (size_t i = 0; i < networkList.size(); ++i) {
        QStandardItem *parentItem = model->invisibleRootItem();
        QStandardItem *item = new QStandardItem;
        item->setText(QString::fromStdString(networkList[i].description));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(QVariant::fromValue(networkList[i]), Qt::UserRole+1);
        parentItem->appendRow(item);

        QStandardItem *subItem = new QStandardItem;
        subItem->setText(QString::fromStdString(networkList[i].name));
        subItem->setFlags(subItem->flags() & ~Qt::ItemIsEditable);
        item->appendRow(subItem);

        QString tip(QString::fromStdString(networkList[i].name));
        for (const auto& addr: networkList[i].address) {
            QString qstrIp = QString::fromStdString(addr.ip);
            QString qstrNetmask = QString::fromStdString(addr.netmask);
            QString str = QString("ip: %1 - %2").arg(qstrIp).arg(qstrNetmask);
            subItem = new QStandardItem;
            subItem->setText(str);
            subItem->setFlags(subItem->flags() & ~Qt::ItemIsEditable);
            item->appendRow(subItem);
            if (tip.size() > 0)
                tip +="\n";
            tip +=qstrIp;
        }

        item->setToolTip(tip);
    }

    model->setHorizontalHeaderLabels(QStringList()<<QStringLiteral("Network Card"));
    this->ui->treeView->setModel(model);
}

void DeviceWindow::exitWindow()
{
    // 选中项目，关闭窗口
    this->accept();
}

void DeviceWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    using namespace figkey;
    NetworkInfo network;
    if (!index.parent().isValid())
        // 取出自定义的 NetworkInfo 结构体数据
        network = index.data(Qt::UserRole+1).value<NetworkInfo>();
    else
        network = index.parent().data(Qt::UserRole+1).value<NetworkInfo>();

    CaptureConfig::Instance().setNetwork(network);

    exitWindow();
}

void DeviceWindow::on_pushButton_clicked()
{
    QItemSelectionModel *selectionModel = this->ui->treeView->selectionModel();
    QModelIndexList selectedIndices = selectionModel->selectedIndexes();
    if (selectedIndices.isEmpty()) {
        // 没有选中项目，弹出警告框
        QMessageBox::warning(this, "Warning", "No item was selected.");
    }
    else if (this->ui->treeView->currentIndex().isValid()) {
        QModelIndex &index = this->ui->treeView->currentIndex();
        on_treeView_doubleClicked(index);
    }
}
