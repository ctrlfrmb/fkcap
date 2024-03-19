#ifndef DOIPKHELPER_H
#define DOIPKHELPER_H

#include <QObject>
#include "ui_networkassistwindow.h"
#include "common/basecomm.h"
#include "doip/doipgenericheaderhandler.h"

class DoIPHelper : public QObject
{
    Q_OBJECT

public:
    explicit DoIPHelper(Ui::NetworkAssistWindow *ui, QObject *parent = nullptr);
    ~DoIPHelper();

    bool start(BaseComm *handle);
    const bool& hasRequst() const;
    const bool& hasActivated() const;

    void parseMessage(const QByteArray& data);

signals:
    void sendMessage(int row);

private:
    bool endTest();
    bool startTest();
    bool parseRoutingActivationResponse(const QByteArray& payload);
    bool responseAliveCheckRequest();

private:
    Ui::NetworkAssistWindow *ui;
    BaseComm *comm { nullptr };

    bool isRequest { false };
    bool isRoutingActivation { false };
};

#endif // DOIPKHELPER_H
