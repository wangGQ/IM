#include <QCoreApplication>
#include <QDebug>
#include "include/Server.h"
#include "include/DbHelper.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Server *server = new Server;

    if (server->listen(QHostAddress::Any, Server::port))
    {
        qDebug() << "[start server] listen port:" << Server::port;
        DbHelper::readConf();
    }
    else
    {
        qDebug() << "[start server] fail:" << server->errorString();
    }
    app.setApplicationName("server");
    return app.exec();
}
