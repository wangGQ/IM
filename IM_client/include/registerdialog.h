#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QString>
#include <QTimer>
#include "include/User.h"
namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = 0);
    ~RegisterDialog();
    void handleResponse(unsigned int id);
private slots:
    void connectTimeOut();
    void on_regButton_clicked();
private:
    bool checkRegMsg();
private:
    Ui::RegisterDialog *ui;
    QString nickName;
    QString passWord;
    int sex;
    QTimer m_timer;
    User *m_newUser;
};

#endif // REGISTERDIALOG_H
