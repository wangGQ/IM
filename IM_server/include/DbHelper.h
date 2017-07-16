#ifndef DBHELPER_H
#define DBHELPER_H
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QVector>
#include <QSet>
#include "include/User.h"

class DbHelper
{
public:
    DbHelper();
    ~DbHelper();

    bool checkUser(unsigned int account, QString password, QString& nickName, QString &avatar);
    unsigned int addUser(QString userName, QString passWord, int sex); //return account
    QVector<User> getFriendList(unsigned int id);
    QSet<QString> getUserGroup(unsigned int userId);

    qint8 addFriend(unsigned int userId, unsigned int peerId, QString group );
    qint8 delFriend(unsigned int userId, unsigned int friendId);
    qint8 delGroup(unsigned int userId, QString group);
    qint8 addGroup(unsigned int userId, QString group);
    qint8 renameGroup(unsigned int userId, QString oldName, QString newName);
    qint8 moveFriend(unsigned int userId, unsigned int friendId, QString oldGName, QString newGName);
    QVector<User> findUser(QString keyWord, int searchType, int sex);
    qint8 cgAvatar(unsigned int userId, QString avatar);

    QVector<unsigned int> getFriendsIds(unsigned int userId);
    static void readConf();
private:
    static unsigned int createAccount();
    static QString md5Encrypt(QString str);
    bool createConnection();
private:
    QSqlDatabase sqlDB;

    static QString dbType;
    static QString dbHost;
    static QString dbName;
    static QString dbUserName;
    static QString dbUserPwd;
};

#endif // DBHELPER_H
