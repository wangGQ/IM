#include "include/UdpVideoThread.h"

UdpVideoThread::UdpVideoThread(QObject *parent) :
    QThread(parent)
{
}

void UdpVideoThread::run()
{
    m_udpSocket = new QUdpSocket;

    for (quint16 port = 4850; port < 65535; ++port) {
        if (m_udpSocket->bind(port)) {
            emit sig_video_port(port);
            break;
        }
    }
    connect(m_udpSocket, SIGNAL(readyRead()), this,
            SLOT(dataReceived()), Qt::DirectConnection);
    this->exec();
}

void UdpVideoThread::dataReceived()
{
    //qDebug() << "video data receive";
    QByteArray datagram;
    do {
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
    } while (m_udpSocket->hasPendingDatagrams());
    //qDebug() << "comes to here";
    QDataStream in(&datagram, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_2);
    qint64 size;
    in >> size;
    //qDebug() << "size = " << size;
    QByteArray inArr;
    inArr.resize(size);
    in >> inArr;
    QImage image;
    image.loadFromData(inArr);
    emit sendImage(image);
}
