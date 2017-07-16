#include "include/UdpClient.h"
#include <QDateTime>

QUdpSocket UdpClient::m_udpSocket;

UdpClient::UdpClient(QObject *parent) :
    QObject(parent)
{
}

void UdpClient::sendRequest(QString peerIp, quint16 peerPort,
                            QStack<QVariant> &args)
{
    if (args.empty()) return;
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_2);

    quint8 requestType = args.pop().toUInt();
    out << requestType;
    while (!args.empty()) {
        quint8 argType = args.pop().toUInt();

        QString argStr;
        QDateTime argDateTime;
        switch (argType) {
            case TYPE_QSTRING:
                argStr = args.pop().toString();
                out << argStr;
                qDebug() << argStr;
                break;
            case TYPE_UINT:
                unsigned int argInt;
                argInt = args.pop().toUInt();
                out << argInt;
                qDebug() << argInt;
                break;
            case TYPE_INT:
                int argUInt;
                argUInt = args.pop().toInt();
                out << argUInt;
                qDebug() << argUInt;
                break;
            case TYPE_DATE_TIME:
                argDateTime = args.pop().toDateTime();
                out << argDateTime;
                qDebug() << argDateTime;
                break;
           case TYPE_INT64:
                qint64 argI64;
                argI64 = (qint64)args.pop().toInt();
                out << argI64;
                qDebug() << argI64;
                break;
           case TYPE_UINT16:
                quint16 argUI16;
                argUI16 = (quint16)args.pop().toUInt();
                out << argUI16;
                qDebug() << argUI16;
                break;
        }
    }

    m_udpSocket.writeDatagram(datagram, QHostAddress(peerIp), peerPort);
}

void UdpClient::sendVideoData(qint64 size, QByteArray bArr,
                             QString peerIp, quint16 port)
{
    qDebug() << "sendVideoData;size = " << size;
    if (size == 0) return;
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_2);
    out << qint64(size) << bArr;
    m_udpSocket.writeDatagram(datagram, QHostAddress(peerIp), port);
}
