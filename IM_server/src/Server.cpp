#include "include/Server.h"
#include <QDebug>
#include <QSettings>

int Server::port = 0;

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
    //readConfig();
    QSettings settings(":/config/config.ini", QSettings::IniFormat);
    port = settings.value("port").toInt();
    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(serverError()));
}

void Server::incomingConnection(int socketDescriptor)
{
    qDebug() << "[server]:new connection arrive";
    CSocketThread *thread = new CSocketThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void Server::serverError()
{
    qDebug() << "[server]accept error:" << errorString();
}

