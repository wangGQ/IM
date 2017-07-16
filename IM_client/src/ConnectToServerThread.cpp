#include "include/ConnectToServerThread.h"
#include "include/TcpConnector.h"
#include "include/global.h"
#include "include/logindialog.h"
#include <QDebug>
#include "include/MacroDefine.h"

//QTcpSocket* ConnectToServerThread::m_tcpSocket = NULL;

ConnectToServerThread::ConnectToServerThread(QObject *parent) :
    QThread(parent)
{
    m_requestType  = -1;
}

void ConnectToServerThread::run()
{
//    gTcpConnector->m_tcpSocket = new QTcpSocket;

//    connect(gTcpConnector->m_tcpSocket, SIGNAL(hostFound()), this, SLOT(foundServer()));
//    connect(gTcpConnector->m_tcpSocket, SIGNAL(connected()), this, SLOT(successConnected()));

//    gTcpConnector->m_tcpSocket->connectToHost(TcpConnector::serverIp, TcpConnector::serverPort);
//    gTcpConnector->m_tcpSocket->waitForConnected();
}

void ConnectToServerThread::foundServer()
{
    qDebug() << "[ConnectToServerThread/foundServer]";
}

void ConnectToServerThread::successConnected()
{
    qDebug() << "[ConnectToServerThread/successConnected]";
    //gTcpConnector->setTcpSocketSLOT();

}

void ConnectToServerThread::setLoginMsg(QString userId, QString pwd, QDialog *loginDlg)
{
    m_requestType = USER_LOGIN;
    m_userId = userId;
    m_pwd = pwd;
    m_loginDlg = loginDlg;
}

