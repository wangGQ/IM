#ifndef UDPSOCKETTHREAD_H
#define UDPSOCKETTHREAD_H

#include <QThread>
#include <QUdpSocket>
#include <QMap>
#include <QDateTime>

class UdpSocketThread : public QThread
{
    Q_OBJECT
public:
    explicit UdpSocketThread(QObject *parent = 0);

    void run();
signals:
    void sig_video(uint, quint16);
    void sig_recv_video_port(uint, quint16);
    void sig_newMsg(QDateTime, uint, QString, QString, int);
    void sig_FTRequest(uint, QString, QString, qint64, QString, quint16);
    void sig_addFriend(QString, quint16, uint, QString, QString, QString);
    void sig_addFriendReply(uint, QString, int, QString);
    void sig_sendFile(uint, int, QString);
public slots:
    void dataReceived();
private:
    static int port;
    QUdpSocket *m_udpSocket;
};

#endif // UDPSOCKETTHREAD_H
