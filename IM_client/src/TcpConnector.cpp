#include "include/TcpConnector.h"
#include "include/MacroDefine.h"
#include "include/User.h"
#include <QDebug>
#include <QSettings>
#include "include/ConnectToServerThread.h"

QString TcpConnector::serverIp = "127.0.0.1";
QString TcpConnector::selfName;

unsigned int TcpConnector::selfId;
quint16 TcpConnector::serverPort;

TcpConnector::TcpConnector()
{
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(socketError()));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(&tcpSocket, SIGNAL(hostFound()), this, SLOT(foundServer()));
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(successConnected()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnectToServer()));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(reconnectToServer()));
    nextBlockSize = 0;
    isConnected = false;
    readConfig();
    m_newUser = NULL;
    m_requestType = -1;
}

void TcpConnector::regUserRequest() //注册用户请求
{
    if (isConnected) {
        if (m_newUser == NULL) return;

        QStack<QVariant> args;
        args.push(m_newUser->m_sex);
        args.push(TYPE_INT);
        args.push(m_newUser->m_pwd);
        args.push(TYPE_QSTRING);
        args.push(m_newUser->m_nickName);
        args.push(TYPE_QSTRING);
        args.push(USER_REGISTER);
        sendRequest(args);
        delete m_newUser;
    }
}

void TcpConnector::setLoginMsg(unsigned int userId, QString pwd, LoginDialog *loginDlg)
{
    m_requestType = USER_LOGIN;
    selfId = userId;
    m_pwd = pwd;
    m_pLoginDlg = loginDlg;
    if (!isConnected) {
        tcpSocket.connectToHost(serverIp, serverPort);
    } else {
        //loginRequest();
        QStack<QVariant> args;
        args.push(m_pwd);
        args.push(TYPE_QSTRING);
        args.push(selfId);
        args.push(TYPE_UINT);
        args.push(m_requestType);
        sendRequest(args);
    }
}

void TcpConnector::setRegisterMsg(User *newUser, RegisterDialog *pRegDlg)
{
    m_requestType = USER_REGISTER;
    m_newUser = newUser;
    m_pRegDlg = pRegDlg;
    if (!isConnected) {
        tcpSocket.connectToHost(serverIp, serverPort);
    } else {
        regUserRequest();
    }
}

void TcpConnector::dataReceived()
{
    qDebug() << "dataReceived";
    QDataStream in(&tcpSocket);
    //QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_5_0);
    if (nextBlockSize == 0) {
        if (tcpSocket.bytesAvailable() < sizeof(quint64)) return;
        in >> nextBlockSize;
    }
    if (nextBlockSize == 0xFFFF) return;
    if ((quint64)tcpSocket.bytesAvailable() < nextBlockSize) return;
    quint8 responseType;
    in >> responseType;
    qDebug() << "response type is " << responseType;

    QVector<User> friends;
    QVector<User*> usersVector;
    QVector<QString> groupVect;
    QString peerIp, groupName, oldGroupName, nickName, avatar;
    unsigned int friendId;
    switch(responseType)
    {
        case USER_REGISTER:
            quint32 id;
            in >> id;
            qDebug() << "id == " << id;
            m_pRegDlg->handleResponse(id);
            break;
        case USER_LOGIN:
            quint16 result;
            in >> result;
            if (result) {
                in >> selfName >> avatar;
                qDebug() << "selfName = " << selfName;
            }
            m_pLoginDlg->handleResponse(result, selfName, avatar);
            break;
        case GET_FRIEND_LIST:
              friends.clear();
              int friendNum;
              uint port;
              in >> friendNum;
              for (int i = 0; i < friendNum; ++i) {
                  unsigned int friendId;
                  QString nick_name, group, avatar;
                  in >> friendId >> nick_name >> group >> avatar >> peerIp >> port;
                  qDebug() << friendId << nick_name << group << avatar;
                  User user(friendId,nick_name, group, avatar, peerIp, port);
                  friends.push_back(user);
              }
              m_pMainForm->initFriends(friends);
              friends.clear();
            break;
       case GET_PEER_SOCKET :
            bool isOnline;
            quint16 peerPort;
            in >> isOnline >> friendId;
            if (isOnline) {
                in >> peerIp >> peerPort;
                qDebug() << "peerIp:" << peerIp << ";peerPort:" << peerPort;
            }
            if (m_pUserMgrForm)
                m_pUserMgrForm->sendAddFriendRequest(isOnline, friendId, peerIp, peerPort);
            break;
       case TEST :
//            quint8 count, value;
//            in >> count;
//            for (int i = 0; i < count; ++i) {
//                in >> value;
//                qDebug() << "recv:" << value;
//            }
            break;
       case DEL_GROUP :
            in >> groupName;
            m_pMainForm->responseDelGroup(groupName);
            break;
       case DEL_FRIEND :
            in >> friendId;
            m_pMainForm->responseDelFriend(friendId);
            break;
       case RENAME_GROUP :
            quint8 renameOk;
            in >> renameOk;
            m_pMainForm->responseRenameGroup(renameOk);
            break;
       case GET_GROUP:
            int size;
            groupVect.clear();
            in >> size;
            for (int i = 0; i < size; ++i) {
                in >> groupName;
                qDebug() << groupName;
                groupVect.push_back(groupName);
            }
            m_pMainForm->initGroups(groupVect); //考虑引用传递
            break;
       case ADD_GROUP:
            in >> groupName;
            m_pMainForm->responseAddGroup(groupName);
            break;
       case MOVE_FRIEND:
            in >> friendId;
            if (friendId) {
                qDebug() << "move friend:" << friendId;
                in >> oldGroupName >> groupName;
                m_pMainForm->responseMoveFriend(friendId, oldGroupName, groupName);
            }
            break;
       case FIND_USER:
            friends.clear();
            unsigned int userId;
            int allPageNum, curPage, sex, vectSize;
            bool isUserOnline;
            in >> allPageNum >> curPage >> vectSize;
            qDebug() << "[Find User]";
            for (int i = 0; i < vectSize; ++i) {
                in >> userId >> nickName >> sex >> avatar >> isUserOnline;
                qDebug() << userId << nickName << sex;
                User *pUser = new User(userId, nickName, sex, avatar, isUserOnline);
                usersVector.push_back(pUser);
            }
            m_pUserMgrForm->showUsers(usersVector, allPageNum, curPage);
            break;
       case FRIEND_ON_OFF_LINE:
            qDebug() << "FRIEND_ON_OFF_LINExxxxxxxxxxxxxxxxxxxxxxxx";
            quint16 udpPort;
            in >> friendId >> peerIp >> udpPort;
            m_pMainForm->changeFriendStatus(friendId, peerIp, udpPort);
            break;
    }
    nextBlockSize = 0;
}

void TcpConnector::socketError() //服务器断开连接,重连
{
    qDebug() << "tcp connect error" << tcpSocket.errorString();
    isConnected = false;
}

void TcpConnector::foundServer()
{
    qDebug() << "found server!";
}

void TcpConnector::successConnected()
{
    qDebug() << "success connected";
    isConnected = true;
    m_timer.stop();
    if (m_pMainForm)
        m_pMainForm->setHeadIconEnableState(true);
    QStack<QVariant> args;
    if (m_requestType == USER_LOGIN) {
        //loginRequest();
        args.push(m_pwd);
        args.push(TYPE_QSTRING);
        args.push(selfId);
        args.push(TYPE_UINT);
        args.push(m_requestType);
        sendRequest(args);

    } else if (m_requestType == USER_REGISTER) {
        regUserRequest();
    } else if (m_requestType == RECONNECTION) {
        args.push(selfId);
        args.push(TYPE_UINT);
        args.push(RECONNECTION);
        sendRequest(args);
    }
    m_requestType = -1;
}

void TcpConnector::disconnectToServer()
{
    qDebug() << "disconnect to server";
    isConnected = false;
    m_pMainForm->setHeadIconEnableState(false);
    m_timer.start(4000);
}

void TcpConnector::reconnectToServer()
{
    m_requestType = RECONNECTION;
    qDebug() << "[TcpConnector::reconnectToServer]";
    tcpSocket.connectToHost(serverIp, serverPort);
}

void TcpConnector::readConfig()
{
    QSettings settings(":/config/config/config.ini", QSettings::IniFormat);
    serverIp = settings.value("server_ip").toString();
    serverPort = settings.value("server_port").toUInt();
}

void TcpConnector::setChatWindow(ChatWindow *pChatWin)
{
    m_pChatWin = pChatWin;
}

void TcpConnector::setMainForm(MainForm *pMainForm)
{
    m_pMainForm = pMainForm;
}

void TcpConnector::setUserMgrForm(UserManagerForm *userMgrForm)
{
    m_pUserMgrForm = userMgrForm;
}

quint16 TcpConnector::getSelfPort()
{
    return tcpSocket.localPort();
}

QString TcpConnector::getSelfIp() const
{
    return tcpSocket.localAddress().toString();
}

MainForm* TcpConnector::getMainForm()
{
    return m_pMainForm;
}

UserManagerForm* TcpConnector::getUserMgrForm() const
{
    return m_pUserMgrForm;
}

bool TcpConnector::isConnectedToServer()
{
    return isConnected;
}

void TcpConnector::sendRequest(QStack<QVariant> &args)
{
    if (!isConnected || args.empty()) return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << quint64(0);

    quint8 requestType = args.pop().toUInt();
    out << requestType;
    while (!args.empty()) {
        quint8 argType = args.pop().toUInt();

        QString argStr;
        QDateTime argDateTime;
        switch (argType) {
            case TYPE_QSTRING:
                argStr = args.pop().toString();
                out << argStr;
                break;
            case TYPE_UINT:
                unsigned int argInt;
                argInt = args.pop().toUInt();
                out << argInt;
                break;
            case TYPE_INT:
                int argUInt;
                argUInt = args.pop().toInt();
                out << argUInt;
                break;
            case TYPE_DATE_TIME:
                argDateTime = args.pop().toDateTime();
                out << argDateTime;
                break;
           case TYPE_INT64:
                qint64 argI64;
                argI64 = (qint64)args.pop().toInt();
                out << argI64;
                break;
        }
    }
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    tcpSocket.write(block);
}
