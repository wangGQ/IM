#include "include/usermanagerform.h"
#include "ui_usermanagerform.h"
#include "include/global.h"
#include "include/MacroDefine.h"
#include "include/UdpClient.h"
#include "include/AddFriendDialog.h"
#include "include/mainform.h"
#include <QDebug>
#include <QMessageBox>
#include <QAction>
#include <QInputDialog>

UserManagerForm::UserManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserManagerForm)
{
    ui->setupUi(this);
    userModel = new QStandardItemModel;
    ui->tableView->setModel(userModel);
    ui->tableView->setIconSize(QSize(40, 40));
    ui->keyWordLineEdit->selectAll();
    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(addFriend(QModelIndex)));
    setWindowTitle(tr("用户查询"));
}

UserManagerForm::~UserManagerForm()
{
    emit form_close();
    delete ui;
    delete userModel;
}

void UserManagerForm::on_searchBtn_clicked()
{
    qDebug() << "search button click";
    QString keyWord = ui->keyWordLineEdit->text();

    int searchType =  ui->typeComboBox->currentIndex();
    int sex = ui->sexComboBox->currentIndex();
    if (searchType == 2) { //接性别查找
        if (sex == 0) {
            QMessageBox::warning(this, tr("Warning"), tr("请选择性别"));
            return;
        }
    } else {
        if (keyWord.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("请输入关键字"));
            return;
        }
    }
    qDebug() << "search type = " << searchType;
    qDebug() << "sex type = " << sex;

    gTcpConnector->setUserMgrForm(this); //---------------------------------

    QStack<QVariant> args;
    args.push(sex);
    args.push(TYPE_INT);
    args.push(searchType);
    args.push(TYPE_INT);
    args.push(keyWord);
    args.push(TYPE_QSTRING);
    args.push(FIND_USER);

    gTcpConnector->sendRequest(args);
}

void UserManagerForm::showUsers(QVector<User*>& usersVector, int allPageNum, int curPage)
{
    userModel->clear();
    if (usersVector.empty()) {
        ui->nextPageBtn->setEnabled(false);
        ui->prePageBtn->setEnabled(false);
        m_allPageNum = m_curPage = 0;
        ui->cur_allPageLbl->setText("0/0");
        return;
    }
    userModel->setHorizontalHeaderItem(0, new QStandardItem(tr("头像")));
    userModel->setHorizontalHeaderItem(1, new QStandardItem(tr("昵称")));
    userModel->setHorizontalHeaderItem(2, new QStandardItem(tr("账号")));
    userModel->setHorizontalHeaderItem(3, new QStandardItem(tr("性别")));
    //userModel->setHorizontalHeaderItem(4, new QStandardItem(tr("是否在线")));
    userModel->setHorizontalHeaderItem(4, new QStandardItem(tr("加好友")));
    ui->tableView->setColumnWidth(0, 42);
    ui->tableView->setColumnWidth(1, 100);
    ui->tableView->setColumnWidth(2, 150);
    ui->tableView->setColumnWidth(3, 50);
    ui->tableView->setColumnWidth(4, 42);
//    ui->tableView->resizeColumnsToContents();
    for (int i = 0; i < usersVector.size(); ++i) {
        ui->tableView->setRowHeight(i, 42);
        QString prefix = ":/avatar/avatar/";
        if (!usersVector[i]->m_isOnline) {
            prefix = ":/avatar_gray/avatar_gray/";
        }
        QString avatar = prefix + usersVector[i]->m_avatar;
        QStandardItem *attrItem = new QStandardItem(QIcon(avatar), "");
        attrItem->setTextAlignment(Qt::AlignCenter);
        userModel->setItem(i, 0, attrItem);
        qDebug() << usersVector[i]->m_nickName << ";" << usersVector[i]->m_userId;
        attrItem = new QStandardItem(usersVector[i]->m_nickName);
        attrItem->setTextAlignment(Qt::AlignCenter);
        userModel->setItem(i, 1, attrItem);
        attrItem = new QStandardItem(QString::number(usersVector[i]->m_userId));
        attrItem->setTextAlignment(Qt::AlignCenter);
        userModel->setItem(i, 2, attrItem);
        QString sex = (usersVector[i]->m_sex == 0)?tr("男"):tr("女");
        attrItem = new QStandardItem(sex);
        attrItem->setTextAlignment(Qt::AlignCenter);
        userModel->setItem(i, 3, attrItem);
//        QString onlineState = usersVector[i]->m_isOnline?tr("在线"):tr("离线");
//        attrItem = new QStandardItem(onlineState);
//        userModel->setItem(i, 4, attrItem);
        attrItem = new QStandardItem(QIcon(":/image/add_friend.png"), "");
        attrItem->setTextAlignment(Qt::AlignCenter);
        userModel->setItem(i, 4, attrItem);
        attrItem->setData(true, ROLE_BUTTON);
        attrItem->setData(usersVector[i]->m_userId, ROLE_USER_ID);
        delete usersVector[i];
    }
    usersVector.clear();
    m_allPageNum = allPageNum;
    m_curPage = curPage;
    ui->cur_allPageLbl->setText(QString::number(m_curPage) + "/" + QString::number(m_allPageNum));
    if (m_allPageNum > m_curPage)
        ui->nextPageBtn->setEnabled(true);
    else
        ui->nextPageBtn->setEnabled(false);
    if (m_curPage > 1) {
        ui->prePageBtn->setEnabled(true);
    } else {
        ui->prePageBtn->setEnabled(false);
    }
}

void UserManagerForm::addFriend(QModelIndex index)
{
    qDebug() << "addFriendxxxxxxxxxxxxxxxxxxxxx";
    QStandardItem* item = userModel->itemFromIndex(index);
    m_peerId = item->data(ROLE_USER_ID).toUInt();
    qDebug() << m_peerId;
    if (!gTcpConnector->getMainForm()->hasFriend(m_peerId).isEmpty() ) {
        QMessageBox::information(this, tr("info"), tr("该用户已经在您的联系人列表中!"));
        return;
    }
    QStack<QVariant> args;
    args.push(m_peerId);
    args.push(TYPE_UINT);
    args.push(GET_PEER_SOCKET);
    gTcpConnector->sendRequest(args);
}

void UserManagerForm::sendAddFriendRequest(bool online, uint peerId, QString peerIp, quint16 peerPort)
{
    if (!online) {
        QMessageBox::information(this, tr("info"), tr("对方不在线!"));
        return;
    }
    bool isOk;
    QString additonMsg = QInputDialog::getText(this, tr("验证信息"),
                                         tr("请输入验证信息:"), QLineEdit::Normal, "", &isOk);
    if (isOk) { //发送自己的ip地址跟端口给对方,方便对方用udp回复
        QString avatar = gTcpConnector->getMainForm()->getAvatar();
        QStack<QVariant> args;
        args.push(avatar);
        args.push(TYPE_QSTRING);
        args.push(additonMsg.trimmed());
        args.push(TYPE_QSTRING);
        args.push(TcpConnector::selfName);
        args.push(TYPE_QSTRING);
        args.push(TcpConnector::selfId);
        args.push(TYPE_UINT);
        args.push(gTcpConnector->getSelfPort());
        args.push(TYPE_UINT16);
        args.push(gTcpConnector->getSelfIp());
        args.push(TYPE_QSTRING);
        args.push(ADD_FRIEND);

        UdpClient::sendRequest(peerIp, peerPort, args);
        m_idAddrMap.insert(peerId, qMakePair(peerIp, peerPort));
    }
}

QPair<QString, quint16> UserManagerForm::getPeerAdder(uint peerId) const
{
    return m_idAddrMap[peerId];
}

void UserManagerForm::on_prePageBtn_clicked()
{
    pageRequest(m_curPage - 1);
}

void UserManagerForm::on_nextPageBtn_clicked()
{
    pageRequest(m_curPage + 1);
}

void UserManagerForm::pageRequest(int page)
{
    QStack<QVariant> args;
    args.push(page);
    args.push(TYPE_INT);
    args.push(FIND_USER_PAGE);
    gTcpConnector->sendRequest(args);
}

void UserManagerForm::on_refreshBtn_clicked()
{
    on_searchBtn_clicked();
}
