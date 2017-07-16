#ifndef MSGPROXY_H
#define MSGPROXY_H

#include <QObject>
#include <QTcpSocket>

class MsgProxy : public QObject
{
    Q_OBJECT
public:
    explicit MsgProxy(QObject *parent = 0, QTcpSocket *tcpSocket = 0);
public slots:
    void brocastUserStatus(unsigned int userId, QString ip = "", quint16 port = 0);
private:
    QTcpSocket* m_tcpSocket;
};

#endif // MSGPROXY_H
