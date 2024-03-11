#include "mainwindow.h"
#include "devicewindow.h"
#include "config.h"
#include <QMessageBox>
#include <QApplication>

#define CONFIG_FILE_PATH "/config/figkey.ini"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");

    qRegisterMetaType< QList<QPersistentModelIndex> >("QList<QPersistentModelIndex>");

    qRegisterMetaType< QVector<int> >("QVector<int>");

    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");

    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");

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

    bool isStart{ true };
    {
        DeviceWindow d;
        d.adjustSize();
        d.setFixedSize(d.size());
        int result = d.exec();
        if (result == QDialog::Rejected) {
            return -1;
        }

        isStart = d.getChecked();
    }

    MainWindow m(isStart);
    m.show();

    return a.exec();
}
