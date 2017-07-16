#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QStack>
#include "include/MacroDefine.h"

class UdpClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(QObject *parent = 0);

    static void sendRequest(QString peerIp, quint16 peerPort, QStack<QVariant> &args);
    static void sendVideoData(qint64 size, QByteArray bArr, QString peerIp,
                              quint16 peerPort);
signals:

public slots:
private:
    static QUdpSocket m_udpSocket;
};

#endif // UDPCLIENT_H
