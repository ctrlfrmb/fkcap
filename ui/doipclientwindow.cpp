#include "doipclientwindow.h"
#include "ui_doipclientwindow.h"

DoIPClientWindow::DoIPClientWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoIPClientWindow)
{
    ui->setupUi(this);
}

DoIPClientWindow::~DoIPClientWindow()
{
    delete ui;
}

void DoIPClientWindow::on_buttonRouteActive_clicked()
{
    doip.startTcpConnection();
    doip.sendRoutingActivationRequest();
}

void DoIPClientWindow::on_buttonSendMessage_clicked()
{
    QString input = ui->textEdit->toPlainText();
    QStringList hexStrings = input.split(' ', QString::SkipEmptyParts);
    std::vector<uint8_t> message;

    for (const QString& hexString : hexStrings) {
        bool conversionSucceeded = false;
        int byte = hexString.toInt(&conversionSucceeded, 16);

        if (conversionSucceeded) {
            message.push_back(static_cast<uint8_t>(byte));
        } else {
            // 处理转换失败的情况
        }
    }

    doip.sendDiagnosticMessage(message);
}
