#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>
#include <QTimer>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

    void handleResponse(quint16 result, QString nickName, QString avatar);
    void closeEvent(QCloseEvent*);
private:
    void initIds();
    void saveConfig();
    QString myEncrypt(QString str);
    QString myDecrypt(QString str);
private slots:
    void initConf(QString newId);  //初始化头像以及是否保存密码等配置信息
    void on_loginBtn_clicked();

    void on_registerBtn_clicked();
    void connectTimeOut();
private:

    QString m_iconFile;
    Ui::LoginDialog *ui;
    QTimer m_timer;
    QString m_curId;
    QStringList m_idList;
    bool m_loginOk;
};

#endif // LOGINDIALOG_H
