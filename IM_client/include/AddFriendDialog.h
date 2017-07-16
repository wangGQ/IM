#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class AddFriendDialog;
}

class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget *parent = 0);
    ~AddFriendDialog();

    void setMsg(QString peerIp, quint16 peerPort, unsigned int peerId, QString peerName,
                QString additionMsg, QString avatar);

    void setReplyMg(unsigned int peerId, QString peerName, int reply, QString avatar = "");
private slots:
    void on_okBtn_clicked();

    void on_rejectBtn_clicked();

private:
    Ui::AddFriendDialog *ui;
    QString m_peerIp;
    quint16 m_peerPort;
    unsigned int m_peerId;
    QString m_peerName;
    QString m_peerAvatar;
    int m_reply; //-1:添加好友申请 0:对方拒绝 1:对方同意
};

#endif // ADDFRIENDDIALOG_H
