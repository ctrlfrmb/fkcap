#ifndef VEHICLEIDENTIFYWINDOW_H
#define VEHICLEIDENTIFYWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QTableWidget>
#include <QTreeWidget>
#include <QUdpSocket>
#include <QTimer>
#include <QCloseEvent>

class VehicleIdentifyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit VehicleIdentifyWindow(QWidget* parent = nullptr);
    ~VehicleIdentifyWindow();

private slots:
    void onRequestButtonClicked();
    void onUpdateButtonClicked();
    void onDiagnoseButtonClicked();
    void onTimeout();
    void onReadyRead();

private:
    void updateParameterValidator(int requestType);
    void addTreeItem(const QMap<QString, QString>& info);

    void initTableSetting();
    void initActions();
    void initTreeReceive();
    void initWindow();

    void sendMessage(int type, QByteArray param);
    void startTimer();
    bool stopTimer();

private:
    QTableWidget* tableSetting;
    QTreeWidget* treeReceive;
    QGroupBox* settingBox;
    QGroupBox* buttonBox;
    QGroupBox* receiveBox;
    QUdpSocket* udpSocket;
    QTimer* timer;

    bool isTimeout { true };
    QHostAddress ipAddress;
    quint16 port;
    QString ecuName;
};


#endif // VEHICLEIDENTIFYWINDOW_H
