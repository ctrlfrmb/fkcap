#include "devicewindow.h"
#include "ui_devicewindow.h"
#include "ipcap.h"
#include <QStandardItemModel>
#include <sstream>
#include <QDebug>
#include <QMessageBox>

static figkey::NpcapCom pcap;

DeviceWindow::DeviceWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceWindow)
{
    ui->setupUi(this);
    LoadDevices();
}

DeviceWindow::~DeviceWindow()
{
    delete ui;
}

void DeviceWindow::LoadDevices()
{
    auto networkList = pcap.getNetworkList();
    QStandardItemModel *model = new QStandardItemModel();

    for (size_t i = 0; i < networkList.size(); ++i) {
        QStandardItem *parentItem = model->invisibleRootItem();
        QStandardItem *item = new QStandardItem;
        item->setText(QString::fromStdString(networkList[i].description));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
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

void DeviceWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    on_pushButton_clicked();
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
        QString name = index.data(Qt::DisplayRole).toString();
        if (!index.parent().isValid())
        {
            index = index.child(0,0);
            name = index.data(Qt::DisplayRole).toString();
        }
        qDebug() << "name: " << name;

        if (!pcap.setCaptureNetwork(name.toStdString())) {
            QMessageBox::critical(this, "Error", "Failed to set network.");
        }
        else
            // 选中项目，关闭窗口
            this->close();
    }
}
