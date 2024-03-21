#ifndef DOIPSETTINGWINDOW_H
#define DOIPSETTINGWINDOW_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class DoIPSettingWindow;
}

class DoIPSettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DoIPSettingWindow(QWidget *parent = nullptr);
    ~DoIPSettingWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initWindow();

private:
    Ui::DoIPSettingWindow *ui;
};

#endif // DOIPSETTINGWINDOW_H
