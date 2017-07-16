#ifndef UDPVIDEOTHREAD_H
#define UDPVIDEOTHREAD_H

#include <QThread>
#include <QUdpSocket>
#include <QImage>

class UdpVideoThread : public QThread
{
    Q_OBJECT
public:
    explicit UdpVideoThread(QObject *parent = 0);

    void run();
signals:
    void sendImage(QImage);
    void sig_video_port(quint16);
private slots:
    void dataReceived();
private:
    QUdpSocket *m_udpSocket;
};

#endif // UDPVIDEOTHREAD_H
