#include "include/filetransfer.h"
#include "include/TcpConnector.h"
#include "include/MacroDefine.h"
#include "include/udpsocketthread.h"
#include "include/global.h"
#include "include/UdpClient.h"
#include <QDir>
#include <QSettings>

QMap<QString, FileTransfer*> FileTransfer::pathTansferMap;
quint16 FileTransfer::FTport1;
FileTransfer::FileTransfer(QString filePath, quint8 type , int role,
                           QObject *parent)
    :QThread(parent)
{
    this->filePath = filePath;
    if (!this->filePath.isEmpty()) {
        pathTansferMap.insert(this->filePath, this);
    }
    m_requestType = type;
    m_role = role;
}

FileTransfer::~FileTransfer()
{
    //delete m_tcpSocket;
}

void FileTransfer::run()
{
    qDebug() << "[File Transfer Thread run]";
    m_tcpSocket = new QTcpSocket;
    m_tcpServer = new QTcpServer;
    m_localFile = new QFile;
    if (!filePath.isEmpty()) {
        qDebug() << "filePath is not empty";
        m_localFile->setFileName(filePath);
    }
    if (m_requestType == FILE_TRANSFER) {
        if (m_role == ROLE_RECEIVER) {
            startTcpServer();
        } else if (m_role == ROLE_SENDER) {
            connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(startTransfer()), Qt::DirectConnection);
            m_tcpSocket->connectToHost(m_peerIp, FTport1);
        }
    }
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)),Qt::DirectConnection);
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()),
            Qt::DirectConnection);

    this->exec();
}

void FileTransfer::setPeerIp(QString peerIp)
{
    qDebug() << "setPeerIp:" << peerIp;
    m_peerIp = peerIp;
}

void FileTransfer::startTransfer()
{
    qDebug() << "File path = " << filePath;
    payloadSize  = 64 * 1024; //64KB
    totalBytes   = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    connect(m_tcpSocket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(updateProgress(qint64)),Qt::DirectConnection);

    if (!m_localFile->open(QFile::ReadOnly)) {
        qDebug() << "client:open file " << filePath << " error!";
        m_tcpSocket->close();
        this->quit();
        return;
    }
    totalBytes = m_localFile->size();
    qDebug() << "file size == " << totalBytes;

    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_0);

    QString fileName = filePath.right(filePath.size()
                                      - filePath.lastIndexOf('/') - 1);
    if (m_requestType == FILE_TRANSFER) {
        sendOut << quint64(0) << m_requestType << TcpConnector::selfId << fileName << totalBytes;
    }
    bytesToWrite = totalBytes;

    sendOut.device()->seek(0);
    sendOut << quint64(outBlock.size() - sizeof(quint64));
    totalBytes += m_tcpSocket->write(outBlock);
    outBlock.resize(0);
}


void FileTransfer::updateProgress(qint64 numBytes)
{
    bytesWritten += numBytes;
    qDebug() << "Total Bytes:" << totalBytes;
    qDebug() << "Bytes Written:" << bytesWritten;

    if (bytesToWrite > 0) { //如果已经发送了数据
        outBlock = m_localFile->read(qMin(bytesToWrite, payloadSize));

        bytesToWrite -= (quint64)m_tcpSocket->write(outBlock);
        outBlock.resize(0);
    } else {
        m_localFile->close();
    }
    emit ft_progress_change(filePath, totalBytes, bytesWritten);
    if (bytesWritten == totalBytes) {
        qDebug() << "finish send file";
        m_localFile->close();
        m_tcpSocket->close();
        this->quit();
    }
}

void FileTransfer::startTcpServer()
{
    quint16 port;
    if (m_requestType == FILE_TRANSFER) {
        port = FTport1;
    }
    qDebug() << "start tcpserver,port:" << port;

    if (!m_tcpServer->listen(QHostAddress::LocalHost, port)) {
        qDebug() << m_tcpServer->errorString();
        m_tcpServer->close();
        this->quit();
        return;
    }
    qDebug() << "[File Transfer]listen " << port;
}

void FileTransfer::stopTcpServer()
{
    qDebug() << "stop TcpServer!";
    m_tcpServer->close();
    this->quit();
}

void FileTransfer::acceptConnection()
{
    qDebug() << "accept connection";
    tcpServerConnection = m_tcpServer->nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(updateReceiveProgress()),
            Qt::DirectConnection);

    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)),Qt::DirectConnection);
    m_tcpServer->close();
    totalBytes = 0;
    bytesReceived = 0;
    inBlock.resize(0);
}

void FileTransfer::updateReceiveProgress()
{
    qDebug() << "[FileTransfer::updateReceiveProgress]--data receive" ;
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_5_0);

    //quint8  requestType;
    unsigned int peerId;
    if (bytesReceived < sizeof(quint64)) {
        if (tcpServerConnection->bytesAvailable() >= sizeof(quint64)) {
            in >> m_headSize;
            bytesReceived += sizeof(quint64);
            qDebug() << "headSize = " << m_headSize;
        }
    }
    if ((tcpServerConnection->bytesAvailable() >= m_headSize) &&
               totalBytes == 0 && bytesReceived != 0) {
        if (m_requestType == FILE_TRANSFER) {
            in >> m_requestType >> peerId >> fileName >> totalBytes;
        } else {
            in >> fileName >> totalBytes;
        }
        qDebug() << "requestType = " << m_requestType;
        qDebug() << "totalBytes:" << totalBytes;

        bytesReceived += m_headSize;
        totalBytes += bytesReceived;

        qDebug() << "all bytes:" << totalBytes;
        m_localFile->setFileName(filePath);
        qDebug() << "filePath ==" << filePath;
        m_localFile->open(QIODevice::WriteOnly | QIODevice::Truncate);

        if (!m_localFile->exists()) {
            m_localFile->close();
            m_localFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
        }
        pathTansferMap.insert(filePath, this);  //收完删除资源
    }

    if (bytesReceived < totalBytes) {
        bytesReceived += tcpServerConnection->bytesAvailable();
        inBlock = tcpServerConnection->readAll();
        m_localFile->write(inBlock);
        inBlock.resize(0);
    }

    qDebug() << "[Transfer File]bytesReceived: " << bytesReceived;
    qDebug() << "[Transfer File]-totalBytes:" << totalBytes;
    emit ft_progress_change(filePath, totalBytes, bytesReceived);
    if (bytesReceived == totalBytes) {
        qDebug() << "[Transfer File]-finish recv file";
        tcpServerConnection->close();
        m_localFile->flush();
        m_localFile->close();
        finishRecvFile();
    }
}

void FileTransfer::finishRecvFile()
{
    pathTansferMap.remove(filePath);
    quit();
}

void FileTransfer::socketError(QAbstractSocket::SocketError)
{
    qDebug() << "[socket error]" << m_tcpSocket->errorString();
    m_tcpSocket->close();
}

FileTransfer* FileTransfer::getTransfer(QString filePath)
{
    QMap<QString, FileTransfer*>::iterator it = pathTansferMap.find(filePath);
    if ( it != pathTansferMap.end()) {
        return it.value();
    } else {
        return Q_NULLPTR;
    }
}

void FileTransfer::readConfig()
{
    QSettings settings(":/config/config/config.ini", QSettings::IniFormat);
    FTport1 = settings.value("FT_port1").toUInt();
}



