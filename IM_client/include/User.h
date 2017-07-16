#ifndef USER_H
#define USER_H
#include <QString>

class User
{
public:
    User();
    User(QString nickName, QString pwd, int sex);
    User(unsigned int userId, QString nickName, QString group = "",
         QString avatar = "4.png", QString ip = "", uint port = 0);
    User(unsigned int userId, QString nickName, int sex, QString avatar, bool isOnline);
public:
    unsigned int m_userId;
    QString m_nickName;
    int m_sex;
    QString m_group;
    QString m_pwd;
    bool m_isOnline;
    QString m_avatar; //头像
    QString m_ip;
    uint m_port;
};


#endif // USER_H
