#ifndef FILTERWINDOW_H
#define FILTERWINDOW_H

#include <QDialog>

namespace Ui {
class FilterWindow;
}

class FilterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FilterWindow(QWidget *parent = 0);
    ~FilterWindow();

private slots:
    void on_pushButton_clicked();

private:
    void initWindow();

private:
    Ui::FilterWindow *ui;
};

#endif // FILTERWINDOW_H
