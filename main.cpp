#include "mainwindow.h"
#include <QApplication>
#include "devicewindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DeviceWindow d;
    //d.setModal(true);
    d.adjustSize();
    d.setFixedSize(d.size());
    d.show();

    //MainWindow w;
   // w.show();

    return a.exec();
}
