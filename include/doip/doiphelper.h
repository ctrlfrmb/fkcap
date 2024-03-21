#ifndef DOIPKHELPER_H
#define DOIPKHELPER_H

#include <QObject>
#include <QUdpSocket>

#include "ui_networkassistwindow.h"
#include "common/basecomm.h"
#include "doip/doipgenericheaderhandler.h"

// DoIPInfoDisplayWindow definition
class DoIPInfoDisplayWindow : public QWidget {
    Q_OBJECT
public:
    DoIPInfoDisplayWindow(QWidget *parent = nullptr);

public slots:
    void displayInfo(int type, const QMap<QString, QString>& info);
    void buttonClicked();

private:
    QTableWidget* table;
    QPushButton* button;
    QMap<QString, QString> data;
};

class DoIPHelper : public QObject
{
    Q_OBJECT

public:
    explicit DoIPHelper(Ui::NetworkAssistWindow *ui, QObject *parent = nullptr);
    ~DoIPHelper();

    bool start(BaseComm *handle);
    bool hasRequst() const;
    bool hasActivated() const;

    void parseMessage(const QByteArray& data);

signals:
    void sendMessage(int row);

private:
    bool startTimer(int interval, int type);
    bool stopTimer();

    bool endTest();
    bool startTest();
    bool parseVehicleAnnouncement(const QByteArray& payload);
    bool parseRoutingActivationResponse(const QByteArray& payload);
    bool responseAliveCheckRequest();

private:
    Ui::NetworkAssistWindow *ui;
    DoIPInfoDisplayWindow *display { nullptr };
    BaseComm *comm { nullptr };
    QTimer* timer { nullptr };

    bool isRequest { false };
    bool isRoutingActivation { false };
};

#endif // DOIPKHELPER_H
