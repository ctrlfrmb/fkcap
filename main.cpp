#include "mainwindow.h"
#include <QApplication>
#include "devicewindow.h"
#include "ipcap.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    {
        DeviceWindow d;
        d.adjustSize();
        d.setFixedSize(d.size());
        int result = d.exec();
        if (result == QDialog::Accepted) {
            auto& pcap = figkey::NpcapCom::Instance();
            if (!pcap.setCaptureNetwork(d.getNetworkName().toStdString())) {
                QMessageBox::critical(nullptr, "Error", "Failed to set network.");
                return -1;
            }

        } else if (result == QDialog::Rejected) {
            return -1;
        }
    }

    MainWindow m;
    m.show();

    return a.exec();
}
