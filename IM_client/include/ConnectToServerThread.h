#ifndef CONNECTTOSERVERTHREAD_H
#define CONNECTTOSERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include <QDialog>

class ConnectToServerThread : public QThread
{
    Q_OBJECT
public:
    explicit ConnectToServerThread(QObject *parent = 0);
    void run();

    void setLoginMsg(QString userId, QString pwd, QDialog *loginDlg);
signals:

public slots:
    void foundServer();
    void successConnected();
private:
    //QTcpSocket *m_tcpSocket;
    int m_requestType;
    QString m_userId;
    QString m_pwd;
    QDialog *m_loginDlg;
};

#endif // CONNECTTOSERVERTHREAD_H
