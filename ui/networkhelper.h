#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QTableWidget>
#include "ui_networkassistwindow.h"

#define SET_PROTOCOL_LABEL "Protocol"
#define SET_CLIENT_IP_LABEL "Client IP"
#define SET_SERVER_IP_LABEL "Server IP"
#define SET_SERVER_PORT_LABEL "Server Port"
#define SET_DATA_TYPE_LABEL "Data Type"

class NetworkHelper : public QObject
{
    Q_OBJECT

public:
    explicit NetworkHelper(Ui::NetworkAssistWindow *ui, QObject *parent = nullptr);
    ~NetworkHelper();

    bool dataIsAscii() const;
    bool enableSend() const;
    bool isReceiveDataEmpty() const;

    QByteArray getSendData(int row);
    void addSettingItem(bool isEdit, const QString& label, const QStringList& options);

    int getNextRow(int row);

    void initListSetting();
    void initTableSend();
    void initTableReceive();

    void setSendState(bool state);
    void setCheckedSendTable(int row, bool check);
    bool isCheckedSendTable(int row);
    void uncheckTableSend();

    void setSendAndReceive(bool state);
    void stopReceiveTimers();
    void getReceiveTimerMap();
    void startReceiveTimers();
    QList<int> checkReceiveDataMap(const QString& timeStamp, const QString& dataString);

    int getCheckedTableSend();
    QList<int> getSendSequence(int start);

    void tryStopSend();

private:
    Ui::NetworkAssistWindow *ui;
    bool isASCII { false };
    bool isSendAndReceive {false};
    bool isSending { true };
    QMap<int, QByteArray> recvDataMap;
    QMap<int, QPair<int, QTimer*>> recvTimerMap;
};

#endif // NETWORKHELPER_H
