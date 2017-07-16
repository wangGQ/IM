#ifndef FILETRANSFER_H
#define FILETRANSFER_H
#include <QString>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTcpServer>
#include <QFile>
#include <QThread>
#include "include/MacroDefine.h"

class FileTransfer : public QThread
{
    Q_OBJECT
public:
    FileTransfer(QString filePath = "", quint8 type = FILE_TRANSFER, int role = ROLE_SENDER, QObject *parent = 0);
    ~FileTransfer();

    void startTcpServer();
    void stopTcpServer();

    void setPeerIp(QString peerIp);

    void finishRecvFile();

    void run();

    static FileTransfer* getTransfer(QString filePath);
    static void readConfig();
signals:
    void ft_progress_change(QString, qint64, qint64);
private slots:
    void socketError(QAbstractSocket::SocketError);
    void updateProgress(qint64);
    void startTransfer();
    //--------------------------------
    void acceptConnection();
    void updateReceiveProgress();
private:
    static quint16 FTport1;

    QFile *m_localFile;  //要发送的文件
    QString filePath; //要发送的文件的路径
    quint64 totalBytes; //发送数据的总大小
    quint64 bytesWritten; //已经发送数据大小
    quint64 bytesToWrite; //剩余数据大小
    quint64 payloadSize; //每次发送数据大小
    QTcpSocket *m_tcpSocket;

    QByteArray outBlock; //数据缓冲区,即存放每次要发送的数据块
    //---------------------------------------------

    QString fileName;
    QString m_peerIp;
    QTcpServer * m_tcpServer;
    QTcpSocket *tcpServerConnection;
    quint64 bytesReceived; //已经收到数据大小

    qint64 m_headSize; //头部数据大小

    QByteArray inBlock; //数据缓冲区,即存放每次要发送的数据块

    static QMap<QString, FileTransfer*> pathTansferMap;
    quint8 m_requestType;
    int m_role;  //sender or receiver
};

#endif // FILETRANSFER_H
