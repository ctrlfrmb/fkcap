#ifndef DOIPCLIENTWINDOW_H
#define DOIPCLIENTWINDOW_H

#include "doip/doipclient.h"
#include <QDialog>

namespace Ui {
class DoIPClientWindow;
}

class DoIPClientWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DoIPClientWindow(QWidget *parent = 0);
    ~DoIPClientWindow();

private slots:
    void on_buttonRouteActive_clicked();

    void on_buttonSendMessage_clicked();

private:
    Ui::DoIPClientWindow *ui;
    figkey::DoIPClient doip;
};

#endif // DOIPCLIENTWINDOW_H
