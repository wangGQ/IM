#ifndef USER_H
#define USER_H
#include <QString>
#include <QChar>

class User
{
public:
    User();
    User(unsigned int id, QString nickName, QString group,
         QString avatar);
    User(unsigned int id, QString nickName, int sex, QString avatar);
public:
    unsigned int id;
    QString nickName;
    int sex;
    QString group;
    bool m_isOnline;
    QString m_avatar;
};


#endif // USER_H
