#ifndef NETWORKASSISTWINDOW_H
#define NETWORKASSISTWINDOW_H

#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include "def.h"
#include "common/basecomm.h"

namespace Ui {
class NetworkAssistWindow;
}

class NetworkAssistWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkAssistWindow(QWidget *parent = nullptr);
    ~NetworkAssistWindow();

    void set(figkey::PacketInfo packet);

public slots:
    void syncPort(int index);
    void onDataReceived(const QByteArray& data);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

    void on_buttonSend_clicked();

private:
    void addSettingItem(bool isEdit, const QString& label, const QStringList& options);
    void initListSetting();
    void initTableSend();
    void initTableReceive();
    void initWindow();
    void exitWindow();
    void setProtocol(uint8_t protocol, QComboBox* comboBox);
    QComboBox* getSettingComboBox(const QString& label);
    QString getSettingItemValue(const QString& label) const;

private:
    Ui::NetworkAssistWindow *ui;
    QList<QComboBox*> comboBoxes;
    bool isServer{ false };
    BaseComm* comm{ nullptr };
};

#endif // NETWORKASSISTWINDOW_H
