#include "include/CSocketThread.h"
#include "include/MacroDefine.h"
#include <QTcpServer>
#include <QDebug>
#include <QDataStream>
#include <QSet>
#include "include/MsgProxy.h"

//QMap<unsigned int, QTcpSocket*> CSocketThread::onlineUserMap;
QMap<unsigned int, CSocketThread*> CSocketThread::idThreadMap;

CSocketThread::CSocketThread(int socketDescriptor, QObject *parent)
    :QThread(parent), socketDescriptor(socketDescriptor)
{
    nextBlockSize = 0;
}

void CSocketThread::socketError(QAbstractSocket::SocketError)
{
    qDebug() << "[socket error]" << m_tcpSocket->errorString();
}

CSocketThread::~CSocketThread()
{
    delete m_tcpSocket;
    delete m_dbHelper;
}

void CSocketThread::hanleAddUser(QString nickName, QString passWord, int sex)
{
    unsigned int res = m_dbHelper->addUser(nickName, passWord, sex);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)USER_REGISTER;
    quint32 result = (quint32)res; //返回生成的账号或0
    out << result;
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleLogin(unsigned int id, QString passWord)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)USER_LOGIN;
    bool loginOk = false;
    if (idThreadMap.find(id) != idThreadMap.end()) {
        out << (quint16)REPEAT_LOGIN;
    } else {
        QString nickName, avatar;
        loginOk = m_dbHelper->checkUser(id, passWord, nickName, avatar);
        if (loginOk) {
            qDebug() << "avatar = " << avatar;
            out << (quint16)LOGIN_OK << nickName << avatar;
            qDebug() << "loginOK";
            //onlineUserMap.insert(id, m_tcpSocket);
            userId = id;
            idThreadMap.insert(userId, this);
            onlineUserList();
        } else {
            out << (quint16)LOGIN_FAIL;
            qDebug() << "login fail";
        }
    }

    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
    if (loginOk) {
        qDebug() << "[brocastMyStatus true]";
        brocastMyStatus(true);
    }
}

void CSocketThread::brocastMyStatus(bool isOnline)
{
    QVector<uint> fidsVect = m_dbHelper->getFriendsIds(userId);
    for (int i = 0; i < fidsVect.size(); ++i) {
        if (idThreadMap.find(fidsVect[i]) != idThreadMap.end()) {
            QTcpSocket *ts = idThreadMap[fidsVect[i]]->getTcpSocket();
//                qDebug() << "[ts's thread]:" << ts->thread();
//                qDebug() << "[current thread]:" << QThread::currentThread();
            qDebug() << "tell " << fidsVect[i];
            MsgProxy *msgProxy = new MsgProxy(0, ts); //when delete
            msgProxy->moveToThread(ts->thread());
            connect(this, SIGNAL(userOnOrOffLine(uint, QString ,quint16)),
                    msgProxy, SLOT(brocastUserStatus(uint, QString, quint16)));
        }
    }
    if (isOnline) {
        emit userOnOrOffLine(userId, m_tcpSocket->peerAddress().toString(),
                             m_tcpSocket->peerPort());
    } else {
        emit userOnOrOffLine(userId, "", 0);
    }
}

void CSocketThread::handleInitFriendList()
{
    qDebug() << "[initFriend] " << userId;
    m_friendsVect = m_dbHelper->getFriendList(userId);
    int size = m_friendsVect.size();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)GET_FRIEND_LIST << size;

    for (int i = 0; i < size; ++i) {
       out << m_friendsVect[i].id << m_friendsVect[i].nickName << m_friendsVect[i].group
              << m_friendsVect[i].m_avatar;
       QString ip = "";
       uint port = 0;
       if (idThreadMap.find(m_friendsVect[i].id) != idThreadMap.end()) {
           QTcpSocket *ts = idThreadMap[m_friendsVect[i].id]->getTcpSocket();
           ip = ts->peerAddress().toString();
           port = ts->peerPort();
       }
       out << ip << port;
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handlePeerSocket(unsigned int id)
{
    qDebug() << "[getPeerSocket]";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)GET_PEER_SOCKET;
    QMap<unsigned int, CSocketThread*>::iterator it = idThreadMap.find(id);
    if (it != idThreadMap.end()) { //返回ip
        QTcpSocket *localSocket = (it.value())->getTcpSocket();
        qDebug() << localSocket->peerAddress().toString() << localSocket->peerPort();
        out << true << id << localSocket->peerAddress().toString() << localSocket->peerPort();

    } else { //not online, return 0// true/false为在线与不在线
        out << false << id;
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::test()
{
    qDebug() << "[Test]";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)TEST << (quint8)7;
    for (int i = 0; i < 7; ++i) {

        out << (quint8)99;

    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
    //m_tcpSocket->flush();
    //m_tcpSocket->waitForBytesWritten();
}

void CSocketThread::handleDelFriend(unsigned int friendId)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)DEL_FRIEND;

    if (m_dbHelper->delFriend(userId, friendId) != -1) {
        qDebug() << "[del friend]:" << friendId << " success";
        out << friendId;
    } else {
        qDebug() << "[del friend]:" << friendId << " fail";
        out << (unsigned int)0;
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleDelGroup(QString groupName)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)DEL_GROUP;

    qDebug() << "[delGroup]:userId:" << userId << ",group:" << groupName;
    if (m_dbHelper->delGroup(userId, groupName) != -1) {
        qDebug() << "del group success";
        out << groupName;
    } else {
        qDebug() << "del group fail";
        out << "";
    }

    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleGetGroup()
{
    qDebug() << "[getGroup]:userId = " << userId;
    QSet<QString> groupSet = m_dbHelper->getUserGroup(userId);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)GET_GROUP << groupSet.size();
    QSet<QString>::iterator it = groupSet.begin();
    for (; it != groupSet.end(); ++it) {
       out << (*it);
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleRenameGroup(QString oldName, QString newName)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)RENAME_GROUP;

    if (m_dbHelper->renameGroup(userId, oldName, newName) != -1) {
        out << quint8(1);
    } else {
        out << quint8(0);
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleAddGroup(QString groupName)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)ADD_GROUP;

    if (m_dbHelper->addGroup(userId, groupName) != -1) {
        out << groupName;
    } else {
        out << "";
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleAddFriend(unsigned int peerId, QString groupName)
{
    if (m_dbHelper->addFriend(userId, peerId, groupName) != -1) {
        qDebug() << "add friend success";
    } else {
        qDebug() << "add friend fail";
    }
}

void CSocketThread::handleMoveFriend(unsigned int friendId, QString oldGName, QString newGName)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)MOVE_FRIEND;
    if (m_dbHelper->moveFriend(userId, friendId, oldGName, newGName) != -1) {
        qDebug() << "move friend success";
        out << friendId << oldGName << newGName;
    } else {
        qDebug() << "move friend fail";
        out << (unsigned int)0;
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::handleFindUser(QString keyWord, int searchType, int sex) //big data
{
    qDebug() << "[Find User] " << keyWord;
    m_userVect = m_dbHelper->findUser(keyWord, searchType, sex);

//    int size = m_userVect.size();
//    QByteArray block;
//    QDataStream out(&block, QIODevice::WriteOnly);
//    out.setVersion(QDataStream::Qt_5_0);
//    int allPageNum = size / 20;
//    if (size % 20) allPageNum++;
//    out << quint64(0) << (quint8)FIND_USER << allPageNum; //分页式,每次发20项数据
//    out << ((size > 20)?20:size);
//    for (int i = 0; i < size && i < 20; ++i) {
//       bool isOnline = false;
//       if (onlineUserMap.find(m_userVect[i].id) != onlineUserMap.end())
//           isOnline = true;
//       out << m_userVect[i].id << m_userVect[i].nickName << m_userVect[i].sex << isOnline;
//    }
//    out.device()->seek(0);
//    out << quint64(block.size() - sizeof(quint64));
//    m_tcpSocket->write(block);
    getUsersByPage(1);
}

void CSocketThread::getUsersByPage(int page)
{
    int size = m_userVect.size();
    int allPageNum = size / 20;
    if (size % 20) allPageNum++;
    int s = 20 * (page - 1);
    int num = 20;
    if (size < (s + 20))
        num = size - s;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0) << (quint8)FIND_USER << allPageNum <<page << num; //分页式,每次发20项数据
    for (int i = s; i < size && i < s + 20; ++i) {
       bool isOnline = false;
       if (idThreadMap.find(m_userVect[i].id) != idThreadMap.end())
           isOnline = true;
       out << m_userVect[i].id << m_userVect[i].nickName <<
                m_userVect[i].sex << m_userVect[i].m_avatar << isOnline;
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    m_tcpSocket->write(block);
}

void CSocketThread::cgAvatar(QString avatar)
{
    m_dbHelper->cgAvatar(userId, avatar);
}

void CSocketThread::onlineUserList()
{
    qDebug() << "---------------------------------------";
    QMap<unsigned int, CSocketThread*>::iterator it = idThreadMap.begin();
    for (; it != idThreadMap.end(); ++it) {
        qDebug() << it.key();
    }
    qDebug() << "---------------------------------------";
}

void CSocketThread::dataReceived()
{
    qDebug() << "data receive";
    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_5_0);

    if (nextBlockSize == 0)
    {
        if (m_tcpSocket->bytesAvailable() < sizeof(quint64))
        {
            return;
        }
        in >> nextBlockSize;
    }
    if ((quint64)m_tcpSocket->bytesAvailable() < nextBlockSize)
    {
        return;
    }
    quint8 requestType;
    in >> requestType;
    qDebug() << requestType;
    QString nickName, passWord;
    QString oldGroupName, groupName, avatar;
    QString keyWord;
    unsigned int id;
    int sex;
    switch(requestType)
    {
        case USER_REGISTER :
            in >> nickName >> passWord >> sex;
            qDebug() << "register" << nickName << "/" << passWord << "/" << sex;
            hanleAddUser(nickName, passWord, sex);
            break;
        case USER_LOGIN :
            in >> id >> passWord;
            handleLogin(id, passWord);
            break;
        case GET_FRIEND_LIST :
            handleInitFriendList();
            break;
        case GET_PEER_SOCKET :
            in >> id;
            qDebug() << "getPeerSocket:" << id;
            handlePeerSocket(id);
            break;
        case TEST :
            test();
            break;
        case CG_AVATAR:
            in >> avatar;
            qDebug() << avatar;
            cgAvatar(avatar);
            break;
        case DEL_GROUP:
            in >> groupName;
            handleDelGroup(groupName);
            break;
        case DEL_FRIEND:
            unsigned int friendId;
            in >> friendId;
            handleDelFriend(friendId);
            break;
        case RENAME_GROUP:
            in >> oldGroupName >> groupName;
            handleRenameGroup(oldGroupName, groupName);
            break;
        case GET_GROUP:
            handleGetGroup();
            break;
        case ADD_GROUP:
            in >> groupName;
            handleAddGroup(groupName);
            break;
        case FIND_USER:
            int searchType;
            in >> keyWord >> searchType >> sex;
            handleFindUser(keyWord, searchType, sex);
            break;
        case FIND_USER_PAGE:
            int page;
            in >> page;
            getUsersByPage(page);
            break;
        case RECONNECTION:
            in >> userId;
            idThreadMap.insert(userId, this);
            break;
        case ADD_FRIEND:
            unsigned int peerId;
            in >> peerId >> groupName;
            handleAddFriend(peerId, groupName);
            break;
        case MOVE_FRIEND:
            in >> id >> oldGroupName >> groupName;
            qDebug() << "Move friend:" << id ;
            handleMoveFriend(id, oldGroupName, groupName);
            break;
        default:
            break;
    }
    nextBlockSize = 0;
}

QTcpSocket* CSocketThread::getTcpSocket() const
{
    return m_tcpSocket;
}

uint CSocketThread::getUserId() const
{
    return userId;
}

QHostAddress CSocketThread::getPeerAddr() const
{
    return m_tcpSocket->peerAddress();
}

void CSocketThread::slotDisconnected()
{
    if (idThreadMap.remove(userId)) {
        qDebug() << "[disconnect]remove online userId:" << userId;
        //广播给该用户在线的好友
        //offlineBrocast();
        brocastMyStatus(false);
    }
    m_tcpSocket->close();
    this->quit();
}

void CSocketThread::run()
{
    qDebug() << "thread run";
    m_tcpSocket = new QTcpSocket;
    if (!m_tcpSocket->setSocketDescriptor(socketDescriptor)) {
        emit error(m_tcpSocket->error());
        return;
    }
    m_dbHelper = new DbHelper;
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()), Qt::DirectConnection);
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()), Qt::DirectConnection);
    this->exec();
}

