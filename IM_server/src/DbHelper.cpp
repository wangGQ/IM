#include "include/DbHelper.h"
#include <QHostAddress>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QByteArray>
#include <QCryptographicHash>
#include <QVariant>
#include <QSettings>

DbHelper::DbHelper() {}

DbHelper::~DbHelper() //释放数据库连接
{
    sqlDB.close();
}

QString DbHelper::dbHost;
QString DbHelper::dbType;
QString DbHelper::dbName;
QString DbHelper::dbUserName;
QString DbHelper::dbUserPwd;

QString DbHelper::md5Encrypt(QString str)
{
    QByteArray byteArr = QCryptographicHash::hash(str.toLocal8Bit(),
                            QCryptographicHash::Md5);
    QString md5String;
    md5String.append(byteArr.toHex());
    return md5String;
}

bool DbHelper::checkUser(unsigned int account, QString password, QString &nickName,
                         QString &avatar)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return false;
        }
    }
    QString checkSql = QString("select nick_name, avatar from user where account = %1 and password = '%2'").
            arg(account).arg(md5Encrypt(password));
    qDebug() << checkSql;
    QSqlQuery query;
    query.exec(checkSql);
    if (query.next()) {
        nickName = query.value(0).toString();
        avatar = query.value(1).toString();
        qDebug() << "nickName in dbhelper = " << nickName;
        return true;
    }
    return false;
}

qint8 DbHelper::addFriend(unsigned int userId, unsigned int peerId, QString group)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return -1;
        }
    }
    QSqlQuery query;
    QString checkSql = QString("select group_name from friend where id = %1 and friend_id = %2").
            arg(userId).arg(peerId);
    query.exec(checkSql);
    if (query.next()) {
        QString oldGroupName = query.value(0).toString();
        if (oldGroupName != group) {
            QString updateSql = QString("update friend set group_name = '%1' where id = %2 and friend_id = %3").
                    arg(group).arg(userId).arg(peerId);
            query.exec(updateSql);
            return query.numRowsAffected();
        } else {
            return 0;
        }
    }

    checkSql = QString("select * from user_group where user_id = %1 and group_name = '%2'").
            arg(userId).arg(group);
    query.exec(checkSql);
    QString insertSql;
    if (!query.next()) {
        insertSql = QString("insert into user_group(user_id, group_name) values (%1, '%2')").
                arg(userId).arg(group);
        query.exec(insertSql);
        if (query.numRowsAffected() == -1) return -1;
    }

    insertSql = QString("insert into friend(id, friend_id, group_name) values(%1, %2, '%3')").
            arg(userId).arg(peerId).arg(group);
    query.exec(insertSql);
    return query.numRowsAffected();
}

qint8 DbHelper::delFriend(unsigned int userId, unsigned int friendId)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return -1;
        }
    }
    QSqlQuery query;
    QString delSql = QString("delete from friend where id = %1 and friend_id = %2").
            arg(userId).arg(friendId);
    query.exec(delSql);
    return query.numRowsAffected();
}

qint8 DbHelper::delGroup(unsigned int userId, QString group)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return -1;
        }
    }
    QSqlQuery query;
    QString delGroupSql1 = QString("delete from user_group where user_id = %1"
                                   " and group_name = '%2'").arg(userId).arg(group);
    query.exec(delGroupSql1);
    QString delGroupSql2 = QString("delete from friend where id = %1 and group_name = '%2'").
            arg(userId).arg(group);
    query.exec(delGroupSql2);
    qDebug() << "[dbHelper]:" << delGroupSql1;
    qDebug() << "[dbHelper]:" << delGroupSql2;

    return 0;
}

qint8 DbHelper::addGroup(unsigned int userId, QString group)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return -1;
        }
    }
    QSqlQuery query;
    QString addGroupSql = QString("insert into user_group(user_id, group_name) values(%1, '%2')").
            arg(userId).arg(group);
    qDebug() << "[dbHelper]:" << addGroupSql;
    query.exec(addGroupSql);
    return query.numRowsAffected();
}

qint8 DbHelper::renameGroup(unsigned int userId, QString oldName, QString newName)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return -1;
        }
    }
    QSqlQuery query;
    QString updateSql = QString("update user_group set group_name = '%1' where user_id = %2 and group_name = '%3'").
            arg(newName).arg(userId).arg(oldName);
    qDebug() << "[dbHelper]:" << updateSql;
    query.exec(updateSql);
    if (query.numRowsAffected() == -1) {
        return -1;
    }
    QString updateSql2 = QString("update friend set group_name = '%1' where id = %2 and group_name = '%3'").
            arg(newName).arg(userId).arg(oldName);
    query.exec(updateSql2);
    return 0;
}

qint8 DbHelper::moveFriend(unsigned int userId, unsigned int friendId,
                           QString oldGName, QString newGName)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db error";
            return -1;
        }
    }
    QSqlQuery query;
    QString updateSql = QString("update friend set group_name = '%1' where id = %2 "
                                "and friend_id = %3 and group_name = '%4'").
            arg(newGName).arg(userId).arg(friendId).arg(oldGName);
    qDebug() << "[sql]" << updateSql;
    query.exec(updateSql);
    return query.numRowsAffected();
}

unsigned int DbHelper::createAccount() //
{
    QSqlQuery query("select max(account) from user");
    if (!query.next()) {
        return 1208842580;
    } else {
        unsigned int newAccount = query.value(0).toUInt() + 1;
        return newAccount;
    }
}

unsigned int DbHelper::addUser(QString userName, QString passWord, int sex)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            return 0;
        }
    }
    QSqlQuery query;
    qDebug() << userName << passWord << sex;
    char sex_char = (sex == 0)?'M':'F';
    QString avatar = "4.png";
    if (sex_char == 'F') avatar = "3.png";
    unsigned int account = createAccount();
    QString insertSql = QString("insert into user (account, nick_name,"
            "password, sex, avatar) values(%1,'%2', '%3', '%4', '%5')").
            arg(account).arg(userName).arg(md5Encrypt(passWord)).arg(sex_char).arg(avatar);

    //qDebug() << insertSql;
    query.exec(insertSql);
    if (query.numRowsAffected() != -1)
    {
        qDebug() << "[DB]query success" << endl;
        insertSql = QString("insert into user_group(user_id, group_name) values"
                            "(%1, '%2')").arg(account).arg(QObject::tr("我的好友"));
        query.exec(insertSql);
        if (query.numRowsAffected() != -1) {
            return account;
        }
        return 0;
    }
    else
    {
        qDebug() << query.lastError().text();
        return 0;
    }
}

QSet<QString> DbHelper::getUserGroup(unsigned int userId)
{
    QSet<QString> groupSet;
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            //return false;
            qDebug() << "db connection error";
            return groupSet;
        }
    }

    QSqlQuery query;
    QString getGroupSql = QString("select group_name from user_group where user_id = %1").arg(userId);
    query.exec(getGroupSql);
    while (query.next()) {
        QString groupName = query.value(0).toString();
        groupSet.insert(groupName);
    }
    return groupSet;
}

QVector<User> DbHelper::getFriendList(unsigned int id)
{
    QVector<User> friendVector;
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            //return false;
            qDebug() << "db connection error";
            return friendVector;
        }
    }
    QSqlQuery query;
    QString sql = QString("select friend_id, group_name from friend where id = %1").arg(id);
    query.exec(sql);
    while (query.next()) {
        unsigned int friendId = query.value(0).toUInt();
        QString group = query.value(1).toString();
        QSqlQuery friendMsgQuery;
        QString sql2 = QString("select nick_name, avatar from user where account = %1").arg(friendId);
        friendMsgQuery.exec(sql2);
        while (friendMsgQuery.next()) {
            QString nick_name = friendMsgQuery.value(0).toString();
            QString avatar = friendMsgQuery.value(1).toString();
            qDebug() << nick_name;
            User user(friendId, nick_name, group, avatar);
            friendVector.push_back(user);
        }
    }
    return friendVector;
}

QVector<unsigned int> DbHelper::getFriendsIds(unsigned int userId)
{
    QVector<unsigned int> friendsIds;
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db connection error";
            return friendsIds;
        }
    }
    QSqlQuery query;
    QString sql = QString("select friend_id from friend where id = %1").arg(userId);
    query.exec(sql);
    while (query.next()) {
        friendsIds.push_back(query.value(0).toUInt());
    }
    return friendsIds;
}

qint8 DbHelper::cgAvatar(unsigned int userId, QString avatar)
{
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db connection error";
            return -1;
        }
    }
    qDebug() << "cg avatar";
    QSqlQuery query;
    QString sql;
    sql = QString("update user set avatar = '%1' where account = %2").arg(avatar).arg(userId);
    qDebug() << sql;
    query.exec(sql);
    return query.numRowsAffected();
}

QVector<User> DbHelper::findUser(QString keyWord, int searchType, int sex)
{
    QVector<User> userVector;
    if (!sqlDB.isOpen()) {
        if (!createConnection()) {
            qDebug() << "db connection error";
            return userVector;
        }
    }
    QSqlQuery query;
    QString selectSql;
    char sexChar;
    if (sex == 1) sexChar = 'M';
    else if (sex == 2) sexChar = 'F';
    if (searchType == 0) { //账号查找
        if (sex != 2) {
            selectSql = QString("select * from user where account = %1 and sex = '%2'").
                    arg(keyWord.toUInt()).arg(sexChar);
        } else {
            selectSql = QString("select * from user where account = %1").arg(keyWord.toUInt());
        }
    } else if (searchType == 1) { //昵称查找
        if (sex != 0) {
            selectSql = QString("select * from user where nick_name like '%%1%' and sex = '%2'").
                    arg(keyWord).arg(sexChar);
        } else {
            selectSql = QString("select * from user where nick_name like '%%1%'").
                    arg(keyWord);
        }
    } else if (searchType == 2) { //性别查找
        selectSql = QString("select * from user where sex = '%1'").
                arg(sexChar);
    }
    qDebug() << "[find user]-sql = " << selectSql;
    query.exec(selectSql);
    while (query.next()) {
        unsigned int userId = query.value(0).toUInt();
        QString nickName = query.value(1).toString();
        QString sexStr = query.value(3).toString();
        int sex = (sexStr == "M")?0:1;
        QString avatar = query.value(4).toString();
        User user(userId, nickName, sex, avatar);
        userVector.push_back(user);
    }
    return userVector;
}

bool DbHelper::createConnection()
{
    sqlDB = QSqlDatabase::addDatabase(dbType);
    sqlDB.setHostName(dbHost);
    sqlDB.setDatabaseName(dbName);
    sqlDB.setUserName(dbUserName);
    sqlDB.setPassword(dbUserPwd);

    if (!sqlDB.open())
    {
        qDebug() << "[DB]database connect fail\n" + sqlDB.lastError().text();
        return false;
    }

    return true;
}

void DbHelper::readConf() //读配置文件
{
    QSettings settings(":/config/config.ini", QSettings::IniFormat);
    dbType = settings.value("db_type").toString();
    //qDebug() << dbType;
    dbHost = settings.value("db_host").toString();
    //qDebug() << dbHost;
    dbName = settings.value("db_name").toString();
    //qDebug() << dbName;
    dbUserName = settings.value("user_name").toString();
    //qDebug() << dbUserName;
    dbUserPwd = settings.value("pwd").toString();
    //qDebug() << dbUserPwd;
}


