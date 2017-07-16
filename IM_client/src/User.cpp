#include "include/User.h"

User::User()
{
}

User::User(QString nickName, QString pwd, int sex)
{
    m_nickName = nickName;
    m_pwd = pwd;
    m_sex = sex;
}

User::User(unsigned int userId, QString nickName, QString group, QString avatar, QString ip, uint port)
{
    m_userId = userId;
    m_nickName = nickName;
    m_group = group;
    m_avatar = avatar;
    m_ip = ip;
    m_port = port;
}

User::User(unsigned int userId, QString nickName, int sex,
           QString avatar,bool isOnline)
{
    m_userId = userId;
    m_nickName = nickName;
    m_sex = sex;
    m_isOnline = isOnline;
    m_avatar = avatar;
}
