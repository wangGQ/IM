#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QDateTime>
#include "include/User.h"
#include "include/udpsocketthread.h"
#include <QStandardItemModel>
#include <QUdpSocket>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include "include/usermanagerform.h"
#include "include/filetransfer.h"
#include "include/FaceDialog.h"

#define ROLE_MARK_ITEM  Qt::UserRole + 1
#define ROLE_MARK_GROUP Qt::UserRole + 2
#define ROLE_GROUP_BELONG Qt::UserRole + 4
#define ROLE_IP   Qt::UserRole + 5
#define ROLE_PORT  Qt::UserRole + 6
#define ROLE_AVATAR   Qt::UserRole + 7

namespace Ui {
class MainForm;
}

class MainForm : public QWidget
{
    Q_OBJECT

public:
    explicit MainForm(QWidget *parent = 0, unsigned int selfid = 0, QString selfName = "",
                      QString iconFile = "");
    ~MainForm();
    void initFriends(QVector<User> &friends);

    void closeEvent(QCloseEvent*);
    void changeEvent(QEvent*);

    void saveAvatarConfig();
    void setHeadIconEnableState(bool enable);

    void responseDelGroup(QString groupName);
    void responseDelFriend(unsigned int friendId);
    void responseRenameGroup(quint8 renameOk);
    void responseAddGroup(QString groupName);
    void initGroups(QVector<QString> &groupVect);
    void addFriend(User user);
    void responseMoveFriend(unsigned int friendId, QString oldGName, QString newGName);
    QString hasFriend(unsigned int friendId); //若true，返回所在组名,否则返回""
    QStringList getGroupList() const;
    UserManagerForm* getUserMgrForm();
    QString getPeerAvatar(unsigned int peerId);
    void changeFriendStatus(unsigned int friendId, QString ip = "", uint port = 0);
    QString getAvatar() const;

private slots:
    void addGroup();
    void delGroup();
    void renameGroup();

    void delFriend();
    void openChatWindow();

    void onTreeViewDoubleClicked();

    void on_treeView_customContextMenuRequested(const QPoint &pos);

    void slot_addFriendMsg(QString, quint16, uint, QString, QString, QString);
    void slot_addFriendReply(uint, QString, int, QString);

    void on_searchToolBtn_clicked();
    void delUserMgrForm();
    void move_user();
    void move_user(uint fid, QString oldGName, QString newGName);
    void setHeadIcon();
    void quitApp();
    void showMainForm();
    void systemTrayActived(QSystemTrayIcon::ActivationReason);
    void cg_avatar(QString imgName);
    void remove_chatwindow(uint);
    void slot_videoRequest(uint peerId, quint16 peerPort);
    void slot_recv_video_port(uint, quint16);
    void transferFileRequest(unsigned int senderId, QString senderName, QString filePath,
                             qint64 fileSize, QString peerIp, quint16 peerPort);
    void newMsgArrive(QDateTime sendTime, unsigned int id,
                             QString sender, QString msg, int launchRole);
    void slot_sendFile(uint, int, QString);
private:

    void initContexMenu();
    void initPopupMenu();
    void createSysTray();
    bool eventFilter(QObject *, QEvent *);
private:
    Ui::MainForm *ui;
    unsigned int selfId;
    QString selfName;
    QString m_iconFile;
    QString m_avatar;

    //QSet<QString> delGroups;
    UdpSocketThread *udpThread;
    QStandardItemModel *model;
    QMap<unsigned int, QStandardItem*> uidItemMap;
    QMap<QString, QStandardItem*> groupItemMap;
    QMap<unsigned int, QIcon> m_uidGrayImgMap;

    QMenu groupMenu;
    QMenu itemMenu;
    QMenu *m_popupMenu;

    QString m_oldGroupName, m_newGroupName;
    UserManagerForm *m_userMgrForm;
    FileTransfer *m_pDownIconsThread;
    unsigned int m_moveFriendId;
    //--------------------------------
    QSystemTrayIcon *m_systemTray;
    FaceDialog *m_faceDlg;
};

#endif // MAINFORM_H
