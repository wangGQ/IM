#ifndef CSOCKETTHREAD_H
#define CSOCKETTHREAD_H
#include <QThread>
#include <QTcpSocket>
#include <QMap>
#include <QHostAddress>
#include "include/DbHelper.h"
#include "include/User.h"

class CSocketThread : public QThread
{
    Q_OBJECT

public:
    CSocketThread(int socketDescriptor, QObject *parent);
    ~CSocketThread();
    void run();
    void hanleAddUser(QString nickName, QString passWord, int sex);
    void handleLogin(unsigned int id, QString passWord);
    void handleInitFriendList();
    void handlePeerSocket(unsigned int id);
    void handleDelFriend(unsigned int friendId);
    void handleDelGroup(QString groupName);
    void handleRenameGroup(QString oldName, QString newName);
    void handleAddGroup(QString groupName);
    void handleGetGroup();
    void handleFindUser(QString keyWord, int searchType, int sex);
    void handleAddFriend(unsigned int peerId, QString groupName);
    void handleMoveFriend(unsigned int friendId, QString oldGName, QString newGName);

    void onlineUserList();
    void getUsersByPage(int page);
    void cgAvatar(QString avatar);
    QTcpSocket* getTcpSocket() const;
    uint getUserId() const;
    void test();

    QHostAddress getPeerAddr() const;
    void brocastMyStatus(bool isOnline);
signals:
    void error(QTcpSocket::SocketError socketError);
    void userOnOrOffLine(unsigned int, QString, quint16);
protected slots:
    void dataReceived();
    void slotDisconnected();
    void socketError(QAbstractSocket::SocketError);
private:
    int socketDescriptor;
    QTcpSocket *m_tcpSocket;
    quint64 nextBlockSize;
    DbHelper *m_dbHelper;

    unsigned int userId; //一条线程对应一个用户账号
    //static QMap<unsigned int, QTcpSocket*> onlineUserMap;
    static QMap<unsigned int, CSocketThread*> idThreadMap;
    QVector<User> m_userVect;
    QVector<User> m_friendsVect;
};

#endif // CSOCKETTHREAD_H
