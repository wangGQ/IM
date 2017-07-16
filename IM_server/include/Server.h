#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QString>
#include "include/CSocketThread.h"

class Server: public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject *parent = 0);

protected:
    void incomingConnection(int socketDescriptor);
signals:

private slots:
    void serverError();
public:
    static int port;
};

#endif // SERVER_H
