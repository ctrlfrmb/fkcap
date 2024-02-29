#include "mainwindow.h"
#include <QApplication>
#include "devicewindow.h"
#include "config.h"
#include <QMessageBox>

#define CONFIG_FILE_PATH "/config/figkey.ini"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList arguments = QCoreApplication::arguments();
    QString path = QCoreApplication::applicationDirPath();
    if (arguments.size() > 1) {
        path =  arguments[1];
    }
    else {
        path += CONFIG_FILE_PATH;
    }

    figkey::CaptureConfig::Instance().loadConfigFile(path.toStdString());

    {
        DeviceWindow d;
        d.adjustSize();
        d.setFixedSize(d.size());
        int result = d.exec();
        if (result == QDialog::Rejected) {
            return -1;
        }
    }

    MainWindow m;
    m.show();

    return a.exec();
}
