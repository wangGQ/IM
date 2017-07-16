#include "include/MsgProxy.h"
#include "include/MacroDefine.h"
#include <QThread>

MsgProxy::MsgProxy(QObject *parent, QTcpSocket *tcpSocket) :
    QObject(parent)
{
    m_tcpSocket = tcpSocket;
}

void MsgProxy::brocastUserStatus(unsigned int userId, QString ip, quint16 port)
{
    qDebug() << "brocastUserStatus:" << userId << ip << port;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)FRIEND_ON_OFF_LINE << userId << ip << port;
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
    m_tcpSocket->flush();
}
