#ifndef SENDERWINDOW_H
#define SENDERWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimer>

class SenderWindow : public QDialog
{
    Q_OBJECT

public:
    SenderWindow(QWidget *parent = nullptr)
        : QDialog(parent)
        , udpSocket(new QUdpSocket(this))
        , tcpSocket(new QTcpSocket(this))
        , timer(new QTimer(this))
    {
        initWindow();
        connectSignalsAndSlots();
    }

public slots:
    void toggleSending()
    {
        if (timer->isActive()) {
            timer->stop();
            sendButton->setText("Send");
        } else {
            dataToSend = dataTextEdit->toPlainText().toUtf8();
            if (dataTypeComboBox->currentText() == "Hex") {
                dataToSend = QByteArray::fromHex(dataToSend);
            }
            timer->start(intervalSpinBox->value());
            sendButton->setText("Stop");
        }
    }

    void sendData()
    {
        QHostAddress serverIp(serverIpLineEdit->text());
        quint16 serverPort = serverPortSpinBox->value();

        if (protocolComboBox->currentText() == "UDP") {
            udpSocket->writeDatagram(dataToSend, serverIp, serverPort);
        } else {
            if (!tcpSocket->isOpen()) {
                tcpSocket->connectToHost(serverIp, serverPort);
                if (tcpSocket->waitForConnected(1000)) {
                    tcpSocket->write(dataToSend);
                }
            } else {
                tcpSocket->write(dataToSend);
            }
        }
    }

private:
    void initWindow()
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QFormLayout *formLayout = new QFormLayout;
        layout->addLayout(formLayout);

        protocolComboBox = new QComboBox;
        protocolComboBox->addItems({"UDP", "TCP"});
        formLayout->addRow("Protocol:", protocolComboBox);

        serverIpLineEdit = new QLineEdit("192.168.1.100");
        formLayout->addRow("Server IP:", serverIpLineEdit);

        serverPortSpinBox = new QSpinBox;
        serverPortSpinBox->setRange(1, 65535);
        serverPortSpinBox->setValue(13400);
        formLayout->addRow("Server Port:", serverPortSpinBox);

        intervalSpinBox = new QSpinBox;
        intervalSpinBox->setRange(1, 10000);
        formLayout->addRow("Interval (ms):", intervalSpinBox);

        dataTypeComboBox = new QComboBox;
        dataTypeComboBox->addItems({"ASCII", "Hex"});
        formLayout->addRow("Data Type:", dataTypeComboBox);

        dataTextEdit = new QPlainTextEdit;
        layout->addWidget(dataTextEdit);

        sendButton = new QPushButton("Send");
        layout->addWidget(sendButton);

        setLayout(layout);
        setWindowTitle("Network Sender");
    }

    void connectSignalsAndSlots()
    {
        connect(sendButton, &QPushButton::clicked, this, &SenderWindow::toggleSending);
        connect(timer, &QTimer::timeout, this, &SenderWindow::sendData);
    }

    QComboBox *protocolComboBox;
    QLineEdit *serverIpLineEdit;
    QSpinBox *serverPortSpinBox;
    QSpinBox *intervalSpinBox;
    QComboBox *dataTypeComboBox;
    QPlainTextEdit *dataTextEdit;
    QPushButton *sendButton;

    QUdpSocket *udpSocket;
    QTcpSocket *tcpSocket;
    QTimer *timer;

    QByteArray dataToSend;
};


#endif // SENDERWINDOW_H
