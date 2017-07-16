#include "include/AddFriendDialog.h"
#include "ui_AddFriendDialog.h"
#include "include/UdpClient.h"
#include <QDebug>
#include <QMessageBox>
#include "include/global.h"

AddFriendDialog::AddFriendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFriendDialog)
{
    ui->setupUi(this);
    m_reply = -1;
}

AddFriendDialog::~AddFriendDialog()
{
    delete ui;
}

void AddFriendDialog::setMsg(QString peerIp, quint16 peerPort,
                             unsigned int peerId, QString peerName, QString additionMsg,
                             QString avatar)
{
    qDebug() << "AddFriendDialog setMsg";
    QString tmp = peerName + "(" + QString::number(peerId) + ")" +
            tr("申请添加您为好友\n验证信息:\n") + additionMsg;

    ui->textEdit->setText(tmp);
    if (m_reply != 0) {
        QStringList groupList = gTcpConnector->getMainForm()->getGroupList();
        foreach(QString group, groupList) {
            ui->groupComboBox->addItem(group);
        }
    }

    m_peerIp = peerIp;
    m_peerPort = peerPort;
    m_peerId = peerId;
    m_peerName = peerName;
    m_peerAvatar = avatar;
    qDebug() << "peerAvatar = " << m_peerAvatar;
}

void AddFriendDialog::setReplyMg(unsigned int peerId, QString peerName,
                                int reply, QString avatar)
{
    m_reply = reply;
    QString msg;
    ui->rejectBtn->hide();
    ui->okBtn->setText(tr("确定"));
    setMsg("", 0, peerId, peerName, msg, avatar);
    if (reply) {
        msg = peerName + "(" + QString::number(peerId) + tr(")同意了您的添加好友申请");
    } else {
        msg = peerName + "(" + QString::number(peerId) + tr(")拒绝了您的添加好友申请");
        ui->label_2->hide();
        ui->groupComboBox->hide();
    }
    ui->textEdit->setText(msg);
}

void AddFriendDialog::on_okBtn_clicked()
{
    if (m_reply == 0) {
        this->close();
        return;
    }

    QString group = ui->groupComboBox->currentText();
    if (group.isEmpty()) {
        QMessageBox::warning(this, tr("warining"), tr("分组名不能为空!"));
        return;
    }
    QStack<QVariant> args;
    User newFriend;
    if (m_reply == -1) { // 回复对方
        qDebug() << "ok to be friends";
        QString avatar = gTcpConnector->getMainForm()->getAvatar();
        args.push(avatar);
        args.push(TYPE_QSTRING);
        args.push(TcpConnector::selfName);
        args.push(TYPE_QSTRING);
        args.push(TcpConnector::selfId);
        args.push(TYPE_UINT);
        args.push(1);
        args.push(TYPE_INT);
        args.push(ADD_FRIEDN_REPLY);
        UdpClient::sendRequest(m_peerIp, m_peerPort, args);
        //对方的ip跟地址为m_peerIp跟m_peerPort
        User user(m_peerId, m_peerName, group, m_peerAvatar, m_peerIp, m_peerPort);
        newFriend = user;
    } else if (m_reply == 1) {
        qDebug() << "comes to here:" << m_peerAvatar;
        QPair<QString, quint16> peerAddr = gTcpConnector->getUserMgrForm()->getPeerAdder(m_peerId);
        User user(m_peerId, m_peerName, group, m_peerAvatar, peerAddr.first, peerAddr.second);
        newFriend = user;
    }
    //发给服务器
    args.clear();
    args.push(group);
    args.push(TYPE_QSTRING);
    args.push(m_peerId);
    args.push(TYPE_UINT);
    args.push(ADD_FRIEND);
    gTcpConnector->sendRequest(args);
    gTcpConnector->getMainForm()->addFriend(newFriend);
    this->close();
}

void AddFriendDialog::on_rejectBtn_clicked()
{
    QStack<QVariant> args;
    args.push(TcpConnector::selfName);
    args.push(TYPE_QSTRING);
    args.push(TcpConnector::selfId);
    args.push(TYPE_UINT);
    args.push(0);
    args.push(TYPE_INT);
    args.push(ADD_FRIEDN_REPLY);
    UdpClient::sendRequest(m_peerIp, m_peerPort, args);
    qDebug() << "reject";
    this->close();
}
