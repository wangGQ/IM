#include "include/User.h"

User::User()
{

}

User::User(unsigned int id, QString nickName, QString group,
           QString avatar)
{
    this->id = id;
    this->nickName = nickName;
    this->group = group;
    m_avatar = avatar;
}

User::User(unsigned int id, QString nickName, int sex, QString avatar)
{
    this->id = id;
    this->nickName = nickName;
    this->sex = sex;
    m_avatar = avatar;
}
