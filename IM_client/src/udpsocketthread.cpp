#include "include/udpsocketthread.h"
#include "include/chatwindow.h"
#include "include/MacroDefine.h"
#include "include/global.h"
#include <QDebug>

int UdpSocketThread::port;

UdpSocketThread::UdpSocketThread(QObject *parent) :
    QThread(parent)
{
    m_udpSocket = new QUdpSocket;
    port = gTcpConnector->getSelfPort();
    m_udpSocket->bind(port);

    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
}

void UdpSocketThread::run()
{
//    m_udpSocket = new QUdpSocket;
//    m_udpSocket->bind(port);
//    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()),
//            Qt::DirectConnection);
//    this->exec();
}

void UdpSocketThread::dataReceived()
{
    QHostAddress *peerAddr = new QHostAddress;
    quint16 peerPort;
    qDebug() << "udp data receive";
    QByteArray datagram;
    do {
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), peerAddr, &peerPort);
    } while (m_udpSocket->hasPendingDatagrams());
    QString peerIp = peerAddr->toString();
    delete peerAddr;
    QDateTime dateTime;
    unsigned int senderId;
    QString senderName, message, filePath, avatar;
    quint8  msgType;
    qint64 fileSize;

    QDataStream in(&datagram, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_2);
    in >> msgType;

    switch (msgType) {
        case NORMAL_MSG :
            int launchRole;
            in >> dateTime >> senderId >> senderName >> message >> launchRole;
            emit sig_newMsg(dateTime, senderId, senderName, message, launchRole);
            break;
        case FILE_TRANSFER :
            qDebug() << "[UdpSocketThread::dataReceived]-FILE_TRANSFER";
            in >> senderId >> senderName >> filePath >> fileSize >> peerPort;
            emit sig_FTRequest(senderId, senderName, filePath, fileSize, peerIp, peerPort);
            break;
        case RECV_FILE_OK :
            qDebug() << "receive file ok signal";
            in >> senderId >> filePath;
            qDebug() << "senderId = " << senderId;
            emit sig_sendFile(senderId, RECV_FILE_OK, filePath);
            break;
        case RECV_FILE_CANCLE :
            qDebug() << "cancel receive file signal";
            in >> senderId >> filePath;
            emit sig_sendFile(senderId, RECV_FILE_CANCLE, filePath);
            break;
        case ADD_FRIEND:
            qDebug() << "[add friend]";
            in >> peerIp >> peerPort >> senderId >> senderName >> message >> avatar;
            qDebug() << peerIp << ";" << peerPort << ";" << senderId << ";" << senderName << ";" << message;
            emit sig_addFriend(peerIp, peerPort, senderId, senderName, message, avatar);
            break;
        case ADD_FRIEDN_REPLY:
            int reply;
            in >> reply >> senderId >> senderName;
            if (reply) in >> avatar;
            qDebug() << "[Add Friend Reply]-reply = " << reply << senderId << senderName;
            emit sig_addFriendReply(senderId, senderName, reply, avatar);
            break;
        case VIDEO_TRANSFER:
            in >> senderId >> peerPort;
            emit sig_video(senderId, peerPort);
            break;
        case REPLY_VIDEO_PORT:
            in >> senderId >> peerPort;
            emit sig_recv_video_port(senderId, peerPort);
            break;
        default:
            break;
    }

}
