#include "networkassistwindow.h"
#include "ui_networkassistwindow.h"

NetworkAssistWindow::NetworkAssistWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkAssistWindow)
{
    ui->setupUi(this);
}

NetworkAssistWindow::~NetworkAssistWindow()
{
    delete ui;
}
