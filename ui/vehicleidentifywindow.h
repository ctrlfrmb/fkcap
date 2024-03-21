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
    void onDiagnoseButtonClicked();
    void onUpdateButtonClicked();
    void onTimeout();
    void onReadyRead();

private:
    void updateParameterValidator(int requestType);
    void addTreeItem(const QMap<QString, QString>& info);

    void initTableSetting();
    void initActions();
    void initTreeReceive();
    void initWindow();

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
};


#endif // VEHICLEIDENTIFYWINDOW_H
