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
#define SET_ERROR_PROCESS_LABEL "Error Process"
#define SET_JSON_TEST_LABEL "Json Test"
#define SET_CYCLE_SEND_MIN_TIME 5

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
    QByteArray getDataFromString(const std::string& str);
    void addSettingItem(bool isEdit, const QString& label, const QStringList& options);

    int getNextRow(int row);

    void initListSetting();
    bool isLooped(int start);
    void initTableSend();
    void initTableReceive();

    bool setErrorInfo(int row, const QString& info);

    void setSendState(bool state);
    void setColumnCheckState(int row, bool check);
    bool isCheckedSendTable(int row);
    void setAllColumnUncheck();

    void deleteTimer(QTimer* timer);

    void setSendAndReceive(bool state);
    void removeReceiveTimer(int row);
    void stopReceiveTimers();
    void getReceiveTimers();
    void startReceiveTimers();
    void startSendAndReceive();

    void setCycleSend(bool state);
    bool startCycleTimers();
    void stopCycleTimers();

    QList<int> checkReceiveDataMap(const QString& timeStamp, const QByteArray& data);

    int getCheckedTableSend();
    QList<int> getContinuousSendMessages(int start, bool isSend);
    QList<int> getAllSendMessages();

    void tryStopSend();

signals:
    void sendMessage(int row);

private:
    Ui::NetworkAssistWindow *ui;
    bool isASCII { false };
    bool isSendAndReceive {false};
    bool isCycleSend {false};
    bool isSending { true };
    bool isPass { true };
    QMap<int, QByteArray> recvDataMap;
    QMap<int, QPair<int, QTimer*>> recvTimerMap;
    QMap<int, QTimer*> cycleTimerMap;
};

#endif // NETWORKHELPER_H
