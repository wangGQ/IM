#ifndef USERMANAGERFORM_H
#define USERMANAGERFORM_H

#include <QWidget>
#include <QVector>
#include <QStandardItemModel>
#include <QMenu>
#include "include/User.h"
#include <QUdpSocket>
#include <QPair>
#define ROLE_BUTTON    Qt::UserRole + 7
#define ROLE_USER_ID   Qt::UserRole + 8

namespace Ui {
class UserManagerForm;
}

class UserManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagerForm(QWidget *parent = 0);
    ~UserManagerForm();

    void showUsers(QVector<User *> &usersVector, int allPageNum, int curPage);

    void sendAddFriendRequest(bool online, uint peerId, QString peerIp, quint16 peerPort);
    void pageRequest(int page);

    QPair<QString, quint16> getPeerAdder(uint peerId) const;
signals:
    void form_close();
private slots:
    void on_searchBtn_clicked();
    void addFriend(QModelIndex);

    void on_prePageBtn_clicked();

    void on_nextPageBtn_clicked();

    void on_refreshBtn_clicked();

private:
    Ui::UserManagerForm *ui;
    QStandardItemModel *userModel;
    QMenu itemMenu;
    unsigned int m_peerId;
    QString m_peerName;
    int m_allPageNum;
    int m_curPage;
    QMap<uint, QPair<QString, quint16> > m_idAddrMap;
};

#endif // USERMANAGERFORM_H
