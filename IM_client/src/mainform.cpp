#include "include/mainform.h"
#include "include/global.h"
#include "ui_mainform.h"
#include "include/chatwindow.h"
#include "include/MacroDefine.h"
#include "include/UdpClient.h"
#include "include/AddFriendDialog.h"
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QDir>
#include <QtAlgorithms>
#include <QSettings>

MainForm::MainForm(QWidget *parent, unsigned int selfId, QString selfName, QString iconFile) :
    QWidget(parent), m_avatar(iconFile), m_faceDlg(0), m_userMgrForm(0),
    ui(new Ui::MainForm)
{
    ui->setupUi(this);
    setWindowTitle("IM");
    this->selfId = selfId;
    this->selfName = selfName;
    ui->selfNameLbl->setText(selfName);
    model = new QStandardItemModel(ui->treeView);
    ui->treeView->setModel(model);
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTreeViewDoubleClicked()));

    model->setHorizontalHeaderLabels(QStringList()<<QStringLiteral("联系人"));
    ui->treeView->setIconSize(QSize(45, 45));
    //ui->treeView->setFont();

    gTcpConnector->setMainForm(this);
    QStack<QVariant> args;
    args.push(GET_GROUP);
    gTcpConnector->sendRequest(args);
    initContexMenu();
    m_iconFile = ":/avatar/avatar/" + iconFile;
    setHeadIcon();
    udpThread = new UdpSocketThread;
    connect(udpThread, SIGNAL(sig_video(uint, quint16)), this, SLOT(slot_videoRequest(uint, quint16)));
    connect(udpThread, SIGNAL(sig_recv_video_port(uint,quint16)), this,
            SLOT(slot_recv_video_port(uint,quint16)));
    connect(udpThread, SIGNAL(sig_newMsg(QDateTime,uint,QString,QString,int)), this,
            SLOT(newMsgArrive(QDateTime,uint,QString,QString,int)));
    connect(udpThread, SIGNAL(sig_FTRequest(uint,QString,QString,qint64,QString,quint16)), this,
            SLOT(transferFileRequest(uint,QString,QString,qint64,QString,quint16)));

    connect(udpThread, SIGNAL(sig_addFriend(QString,quint16,uint,QString,QString,QString)), this,
            SLOT(slot_addFriendMsg(QString,quint16,uint,QString,QString,QString)));
    connect(udpThread, SIGNAL(sig_addFriendReply(uint,QString,int,QString)), this,
            SLOT(slot_addFriendReply(uint,QString,int,QString)));

    connect(udpThread, SIGNAL(sig_sendFile(uint,int,QString)), this,
            SLOT(slot_sendFile(uint,int,QString)));
    createSysTray();
    ui->headIconLbl->installEventFilter(this);
    ui->frame->installEventFilter(this);
}

bool MainForm::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->headIconLbl) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (m_faceDlg == 0) {
                m_faceDlg = new FaceDialog(0, CHOOSE_AVATAR);
                connect(m_faceDlg, SIGNAL(select_avatar(QString)), this,
                        SLOT(cg_avatar(QString)));
                m_faceDlg->show();
            } else {
                m_faceDlg->setVisible(true);
            }
            QPoint pos = ui->headIconLbl->mapToGlobal(QPoint(0, 0));
            m_faceDlg->move(pos.x() + 90, pos.y());
            m_faceDlg->setFocus();
            m_faceDlg->activateWindow();
        }
    }
    else if (target == ui->frame) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (m_faceDlg) m_faceDlg->hide();
        }
    }

    //return QWidget::eventFilter(target ,event);
}

void MainForm::cg_avatar(QString imgPath)
{
    if (m_iconFile == imgPath) return;
    m_iconFile = imgPath;
    setHeadIcon();
    if (m_faceDlg) {
        m_faceDlg->hide();
    }
    m_avatar = imgPath.right(imgPath.length() - imgPath.lastIndexOf("/") - 1);
    QStack<QVariant> args;
    args.push(m_avatar);
    args.push(TYPE_QSTRING);
    args.push(CG_AVATAR);
    gTcpConnector->sendRequest(args);
}

QString MainForm::getAvatar() const
{
    return m_avatar;
}

void MainForm::createSysTray()
{
    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setIcon(QIcon(":/image/sys_tray.png"));
    QString toolTip = "IM:" + selfName + "(" + QString::number(selfId) + ")";
    m_systemTray->setToolTip(toolTip);
    connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(systemTrayActived(QSystemTrayIcon::ActivationReason)));
    QAction *showAct = new QAction(tr("打开主面板"), this);
    connect(showAct, SIGNAL(triggered()), this, SLOT(showMainForm()));
    QAction *quitAct = new QAction(tr("退出"), this);
    connect(quitAct, SIGNAL(triggered()), this, SLOT(quitApp()));
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(showAct);
    contextMenu->addAction(quitAct);
    m_systemTray->setContextMenu(contextMenu);
    m_systemTray->show();
}

void MainForm::systemTrayActived(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
        showMainForm();
        break;
    }
}

void MainForm::setHeadIcon()
{
    qDebug() << "[MainForm:setHeadIcon]-iconFile = " << m_iconFile;

    ui->headIconLbl->setScaledContents(true);
    ui->headIconLbl->setPixmap( QPixmap(m_iconFile));
    ui->headIconLbl->setStyleSheet("border:1px solid red");
}

void MainForm::changeFriendStatus(unsigned int friendId, QString ip, uint port)
{
   qDebug() << "changeFriendStatus:" << friendId << ":" << ip ;
   if (uidItemMap.find(friendId) == uidItemMap.end()) return;
   QStandardItem *item = uidItemMap[friendId];
   QString imgName = ":/avatar/avatar/" + item->data(ROLE_AVATAR).toString();
   item->setData(ip, ROLE_IP);
   item->setData(port, ROLE_PORT);
   if (ip.isEmpty()) {
       qDebug() << "peer offline";
       imgName = ":/avatar_gray/avatar_gray/" + item->data(ROLE_AVATAR).toString();
   }
   item->setData(QIcon(imgName), Qt::DecorationRole);
   ChatWindow* cw = ChatWindow::getChatWindow(friendId);
   if (cw) {
       cw->setPeerAddr(ip, port);
       cw->setPeerIcon();
   }
}

MainForm::~MainForm()
{
    delete ui;
    delete udpThread;
    delete model;
}

void MainForm::initContexMenu() //有空可以加图标
{
    QAction *ac = NULL;
    ac = new QAction(QIcon(":/image/add_group.ico"),QStringLiteral("添加分组"), this);
    groupMenu.addAction(ac);
    connect(ac, SIGNAL(triggered()), this, SLOT(addGroup()));
    ac = new QAction(QIcon(":/image/del_group.ico"), QStringLiteral("删除该组"), this);

    groupMenu.addAction(ac);
    connect(ac, SIGNAL(triggered()), this, SLOT(delGroup()));
    ac = new QAction(QStringLiteral("重命名"), this);
    groupMenu.addAction(ac);
    connect(ac, SIGNAL(triggered()), this, SLOT(renameGroup()));
    //--------------------------------------------------
    ac = new QAction(QIcon(":/image/send_msg.ico"),QStringLiteral("发送消息"), this);
    connect(ac, SIGNAL(triggered()), this, SLOT(openChatWindow()));
    itemMenu.addAction(ac);
    ac = new QAction(QIcon(":/image/del_user.ico"), QStringLiteral("删除联系人"), this);
    connect(ac, SIGNAL(triggered()), this, SLOT(delFriend()));
    itemMenu.addAction(ac);
    m_popupMenu = new QMenu(QStringLiteral("移动联系人至"), this);
    m_popupMenu->setIcon(QIcon(":/image/move_user.ico"));
    itemMenu.addMenu(m_popupMenu);
}

void MainForm::initPopupMenu()
{
    m_popupMenu->clear();
    if (groupItemMap.size() > 1) {
        QModelIndex curIndex = ui->treeView->currentIndex();
        QStandardItem *item = model->itemFromIndex(curIndex);
        m_oldGroupName = item->data(ROLE_GROUP_BELONG).toString();
        m_moveFriendId = item->data(ROLE_MARK_ITEM).toUInt();
        qDebug() << "initPopupMenu";
        QMap<QString, QStandardItem*>::iterator it = groupItemMap.begin();
        for (; it != groupItemMap.end(); ++it) {
            if (it.key() == m_oldGroupName) continue;
            QAction *ac = new QAction(it.key(), this);
            connect(ac, SIGNAL(triggered()), this, SLOT(move_user()));
            m_popupMenu->addAction(ac);
        }
    }
}

void MainForm::move_user()
{
    QAction *ac = dynamic_cast<QAction*>(sender());
    qDebug() << ac->text() << "move friendId = " << m_moveFriendId;
    move_user(m_moveFriendId, m_oldGroupName, ac->text());
}

void MainForm::move_user(uint fid, QString oldGName, QString newGName)
{
    QStack<QVariant> args;
    args.push(newGName);
    args.push(TYPE_QSTRING);
    args.push(oldGName);
    args.push(TYPE_QSTRING);
    args.push(fid);
    args.push(TYPE_UINT);
    args.push(MOVE_FRIEND);
    gTcpConnector->sendRequest(args);
}

void MainForm::addGroup()
{
    QString defaultName = tr("未命名");
    QString groupName = defaultName;
    int cnt = 1;
    while (groupItemMap.find(groupName) != groupItemMap.end()) {
        groupName = defaultName + QString::number(cnt);
        cnt++;
    }
    QStack<QVariant> args;
    args.push(groupName);
    args.push(TYPE_QSTRING);
    args.push(ADD_GROUP);
    gTcpConnector->sendRequest(args);
}

void MainForm::delGroup()
{
    QModelIndex curIndex = ui->treeView->currentIndex();

    if (QMessageBox::question(this, tr("question"), tr("确定删除这个分组及组内所有联系人?"),
                          QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        QString groupName = model->itemData(curIndex).values()[0].toString();
        qDebug() << "[del group]:" << groupName;

        QStack<QVariant> args;
        args.push(groupName);
        args.push(TYPE_QSTRING);
        args.push(DEL_GROUP);
        gTcpConnector->sendRequest(args);
    }
}

void MainForm::renameGroup()
{

    QModelIndex curIndex = ui->treeView->currentIndex();
    QStandardItem *stIt = model->itemFromIndex(curIndex);

    qDebug() << "[MainForm/renameGroup]-groupName = " << stIt->text();
    m_oldGroupName = stIt->text();
    bool isOk;
    m_newGroupName = QInputDialog::getText(this, tr("输入"),
                                         tr("请输入新的分组名:"), QLineEdit::Normal, stIt->text(), &isOk);

    if (isOk) {
        m_newGroupName = m_newGroupName.trimmed();
        qDebug() << "[MainForm/renameGroup]-newGroupName = " << m_newGroupName;
        if (m_newGroupName != "") {
            if (groupItemMap.find(m_newGroupName) != groupItemMap.end()) {
                QMessageBox::warning(this, tr("warning"), tr("该分组名已存在!"));
                return;
            }

            if (m_newGroupName != m_oldGroupName) {
                QStack<QVariant> args;
                args.push(m_newGroupName);
                args.push(TYPE_QSTRING);
                args.push(m_oldGroupName);
                args.push(TYPE_QSTRING);
                args.push(RENAME_GROUP);
                gTcpConnector->sendRequest(args);
            }
        } else {
            QMessageBox::warning(this, tr("warning"), tr("分组名不能为空!"));
        }
    }
    //ui->treeView->setEditTriggers();

}

void MainForm::delFriend()
{
    if (QMessageBox::question(this, tr("question"), tr("确定删除该联系人吗?"), QMessageBox::Ok|
                          QMessageBox::Cancel) == QMessageBox::Ok) {
        QModelIndex curIndex = ui->treeView->currentIndex();
        QStandardItem *item = model->itemFromIndex(curIndex);
        unsigned int friendId = item->data(ROLE_MARK_ITEM).toUInt();
        qDebug() << "[del friend]:friendId = " << friendId;
        QStack<QVariant> args;
        args.push(friendId);
        args.push(TYPE_UINT);
        args.push(DEL_FRIEND);
        gTcpConnector->sendRequest(args);
    }
}

void MainForm::openChatWindow()
{
    QModelIndex curIndex = ui->treeView->currentIndex();
    QStandardItem *item = model->itemFromIndex(curIndex);
    unsigned int friendId = item->data(ROLE_MARK_ITEM).toUInt();
    ChatWindow *cw = ChatWindow::getChatWindow(friendId);
    if (cw == Q_NULLPTR) {
        QString peerName = model->itemData(curIndex).values()[0].toString(); //文本
        cw = new ChatWindow(0, friendId, peerName);
        cw->setAttribute(Qt::WA_DeleteOnClose);
        cw->show();
    }
    cw->activateWindow();
}

void MainForm::initFriends(QVector<User> &friends) //
{
    qDebug() << "initFriends";
    for (int i = 0; i < friends.size(); ++i) {
        addFriend(friends[i]);
    }
}

void MainForm::responseDelGroup(QString groupName)
{
    qDebug() << "[responseDelGroup]:" << groupName;
    if (!groupName.isEmpty()) {
        //删除组内的好友
        QMap<uint, QStandardItem*>::iterator it2 = uidItemMap.begin();
        for (; it2 != uidItemMap.end();) {
            if (it2.value()->data(ROLE_GROUP_BELONG).toString() == groupName) {
                qDebug() << "[remove fid]:" << it2.key();
                //uidItemMap.remove(it2.key());
                uidItemMap.erase(it2++);
            } else {
                it2++;
            }
        }
        QMap<QString, QStandardItem*>::iterator it =
                groupItemMap.find(groupName);
        if (it != groupItemMap.end()) {
            model->removeRow(it.value()->row());
            groupItemMap.erase(it);
        }
    } else {
        qDebug() << "删除组失败";
    }
}

void MainForm::responseAddGroup(QString groupName)
{
    if (groupName != "") {
        QStandardItem *newGroupItem = new QStandardItem(groupName);

        newGroupItem->setData(groupName, ROLE_MARK_GROUP);

        model->appendRow(newGroupItem);
        groupItemMap.insert(groupName, newGroupItem);
        ui->treeView->setCurrentIndex(newGroupItem->index());
    } else {
        qDebug() << "add group fail";
    }
}

void MainForm::responseMoveFriend(unsigned int friendId,
                                  QString oldGName, QString newGName)
{
    qDebug() << "[MainForm/responseMoveFriend]-friendId = " << friendId;
    QStandardItem *userItem = uidItemMap[friendId];
    QStandardItem *groupItem = groupItemMap[oldGName];
    QStandardItem *newGItem = groupItemMap[newGName];
    if (userItem && groupItem && newGItem) {
       qDebug() << "three item is not null";
       //copy
       QStandardItem *tmpItem = userItem->clone();
       groupItem->removeRow(userItem->row());
       newGItem->appendRow(tmpItem);
       tmpItem->setData(newGName, ROLE_GROUP_BELONG);
       uidItemMap.remove(friendId);
       uidItemMap.insert(friendId, tmpItem);
    }
}

void MainForm::responseDelFriend(unsigned int friendId)
{
    qDebug() << "[MainForm/responseDelFriend]-friendId = " << friendId;
    if (friendId) {
        QMap<unsigned int, QStandardItem*>::iterator it =
                uidItemMap.find(friendId);
        if (it != uidItemMap.end()) {
            QMap<QString, QStandardItem*>::iterator groupIt = groupItemMap.begin();
            for (; groupIt != groupItemMap.end(); ++groupIt) {

                if (groupIt.value()->child(it.value()->row()) == it.value()) {
                    groupIt.value()->removeRow(it.value()->row());
                    uidItemMap.remove(friendId);
                    break;
                }
            }
        }
    } else {
        qDebug() << "[MainForm/responseDelFriend]-delete friend fail";
    }
}

void MainForm::responseRenameGroup(quint8 renameOk)
{
    if (renameOk) {
        QMap<QString, QStandardItem*>::iterator it =
                groupItemMap.find(m_oldGroupName);
        if (it != groupItemMap.end()) {
            it.value()->setText(m_newGroupName);
            QStandardItem *tmp = it.value();
            groupItemMap.remove(m_oldGroupName);
            groupItemMap.insert(m_newGroupName, tmp);
        }
    } else {
        qDebug() << "[MainForm::responseRenameGroup]-rename fail";
    }
}

void MainForm::initGroups(QVector<QString> &groupVect)
{
    qDebug() << "initGroups";
    qSort(groupVect.begin(), groupVect.end(), qGreater<QString>());
    groupItemMap.clear();
    QVector<QString>::iterator it = groupVect.begin();
    for (; it != groupVect.end(); ++it) {
        QString groupName = *it;
        QStandardItem *groupItem = new QStandardItem(groupName);
        groupItem->setData(groupName, ROLE_MARK_GROUP);
        groupItem->setEditable(false);
        model->appendRow(groupItem);
        groupItemMap.insert(groupName, groupItem);
        qDebug() << groupName;
    }
    if (!groupVect.empty()) {
        QStack<QVariant> args;
        args.push(GET_FRIEND_LIST);
        gTcpConnector->sendRequest(args);
    }
}

void MainForm::addFriend(User user)
{
    QString oldGroup = hasFriend(user.m_userId);
    if (!oldGroup.isEmpty()) {
        if (oldGroup == user.m_group) return;
        move_user(user.m_userId, oldGroup, user.m_group);
        return;
    }
    QStandardItem *groupItem = NULL;
    QMap<QString, QStandardItem*>::iterator it = groupItemMap.find(user.m_group);
    if (it != groupItemMap.end()) groupItem = it.value();
    if (groupItem == NULL) {
        groupItem = new QStandardItem(user.m_group);
        groupItem->setData(user.m_group, ROLE_MARK_GROUP);
        groupItem->setEditable(false);
        model->appendRow(groupItem);
        groupItemMap.insert(user.m_group, groupItem);
    }
    QString avatar = ":/avatar/avatar/" + user.m_avatar;
    if (user.m_ip.isEmpty()) //ip是否为空串表示是否在线
        avatar = ":/avatar_gray/avatar_gray/" + user.m_avatar;

    QStandardItem *childItem = new QStandardItem(QIcon(avatar), user.m_nickName);
    childItem->setEditable(false);
    childItem->setData(user.m_ip, ROLE_IP);
    childItem->setData(user.m_port, ROLE_PORT);
    childItem->setData(user.m_userId, ROLE_MARK_ITEM);
    childItem->setData(user.m_userId, Qt::ToolTipRole);
    childItem->setData(user.m_avatar, ROLE_AVATAR);
    childItem->setData(user.m_group, ROLE_GROUP_BELONG);
    uidItemMap.insert(user.m_userId, childItem);
    //childItem->setData(QColor(232,209,57,200),Qt::ForegroundRole);
    childItem->setSizeHint(QSize(200, 50));
    groupItem->appendRow(childItem);
}

void MainForm::onTreeViewDoubleClicked()
{
    qDebug() << "[MainForm/onTreeViewDoubleClicked]";
    QModelIndex curIndex = ui->treeView->currentIndex();

    QString peerName = model->itemData(curIndex).values()[0].toString(); //文本
    QString peerIp = model->data(curIndex, ROLE_IP).toString();
    quint16 peerPort = model->data(curIndex, ROLE_PORT).toUInt();
    qDebug() << peerName;
    unsigned int peerId = model->data(curIndex, ROLE_MARK_ITEM).toUInt();

    qDebug() << peerId;
    if (peerId) {
        ChatWindow *cw = ChatWindow::getChatWindow(peerId);
        if (!cw) {
            ChatWindow *chatWindow = new ChatWindow(0, peerId, peerName,
                                                    peerIp, peerPort);
            connect(chatWindow, SIGNAL(chatwindow_close(uint)), this, SLOT(remove_chatwindow(uint)));
            chatWindow->setAttribute(Qt::WA_DeleteOnClose);
            chatWindow->show();
        } else {
            cw->activateWindow();
        }
    }
}

void MainForm::remove_chatwindow(uint peerId)
{
    ChatWindow::removeCW(peerId);
}

void MainForm::newMsgArrive(QDateTime sendTime, unsigned int id,
            QString sender, QString msg, int launchRole)
{
    ChatWindow *cw = ChatWindow::getChatWindow(id);
    if (cw) {
        cw->displayMsg(sender, sendTime, msg, launchRole);
        if (launchRole == ROLE_SENDER) {
            cw->setLaunchRole(ROLE_RECEIVER);
        } else {
            cw->setLaunchRole(ROLE_RECEIVER);
        }
        return;
    }

    QString tmp = sender + tr("向您发送消息, 是否打开聊天窗口?");
    if (QMessageBox::question(NULL, "Question", tmp,
                             QMessageBox::Yes|QMessageBox::Ignore, QMessageBox::Yes) == QMessageBox::Yes) {

        QStandardItem *item = uidItemMap[id];
        QString peerIp = item->data(ROLE_IP).toString();
        quint16 peerPort = item->data(ROLE_PORT).toUInt();
        ChatWindow *cw = new ChatWindow(0, id, sender, peerIp, peerPort);
        connect(cw, SIGNAL(chatwindow_close(uint)), this, SLOT(remove_chatwindow(uint)));
        cw->setLaunchRole(ROLE_RECEIVER);
        cw->displayMsg(sender, sendTime, msg, launchRole);
        cw->show();
        cw->activateWindow();
    }
    //m_systemTray->showMessage(tr("新的消息"), tmp);
}

void MainForm::slot_sendFile(uint senderId, int reply, QString filePath)
{
    ChatWindow::handleSendFile(senderId, reply, filePath);
}

void MainForm::transferFileRequest(unsigned int senderId, QString senderName,
                                   QString filePath, qint64 fileSize, QString peerIp,
                                   quint16 peerPort)
{
    qDebug() << "[MainForm::transferFileRequest]-senderId = " << senderId << ";senderName = " << senderName;
    QString fileName = filePath.right(filePath.size()
                                        - filePath.lastIndexOf('/') - 1);

    QString msg = senderName + "(" + QString::number(senderId) + ")" + tr("请求发送文件") + fileName
            + "(" + QString::number(fileSize * 1.0 / 1024) + "KB)" + tr(",是否接收?");

    QMessageBox myMsgBox(QMessageBox::NoIcon, tr("Question"), msg, QMessageBox::Yes|
                        QMessageBox::No|QMessageBox::Ignore, NULL);
    myMsgBox.setButtonText(QMessageBox::Yes, tr("接收"));
    myMsgBox.setButtonText(QMessageBox::No, tr("取消"));
    myMsgBox.setButtonText(QMessageBox::Ignore, tr("另存为"));
    int answer = myMsgBox.exec();
    QString localFilePath;
    quint8 option = (quint8)RECV_FILE_OK;
    if (answer == QMessageBox::Yes) {
        localFilePath = "./Users/" + QString::number(selfId) + "/FileRecv";
        QDir dir;
        if (!dir.exists(localFilePath)) {
            dir.mkpath(localFilePath);
        }
        localFilePath.append("/" + fileName);
        qDebug() << "agree to receive file";
    } else if (answer == QMessageBox::No) {
        qDebug() << "reject to receive file";
        option = (quint8)RECV_FILE_CANCLE;
    } else if (answer == QMessageBox::Ignore) {
        QString dir = QFileDialog::getExistingDirectory(this, tr("另存为"),
                             QDir::homePath(), QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
        localFilePath = dir + "/" + fileName;
        qDebug() << localFilePath;
    } else {
        qDebug() << "no operation";
        return;
    }
    if (option == RECV_FILE_OK) {
        ChatWindow *cw = ChatWindow::getChatWindow(senderId);
        if (cw == Q_NULLPTR) {
            cw = new ChatWindow(0, senderId, senderName, peerIp, peerPort);
            cw->show();
        }
        cw->createNewFtTask(localFilePath, ROLE_RECEIVER);
        cw->activateWindow();
    }
    QStack<QVariant> args;
    args.push(filePath);
    args.push(TYPE_QSTRING);
    args.push(selfId);
    args.push(TYPE_UINT);
    args.push(option);  //是否同意接收文件
    UdpClient::sendRequest(peerIp, peerPort, args);
}

void MainForm::closeEvent(QCloseEvent *)
{
    saveAvatarConfig();
    qDebug() << "mainform quit";
    m_systemTray->hide();
    qApp->quit();
}

void MainForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() & Qt::WindowMinimized) {
            if (m_faceDlg) m_faceDlg->hide();
            QTimer::singleShot(0, this, SLOT(hide()));
        }
        QWidget::changeEvent(event);
    }
}

void MainForm::saveAvatarConfig()
{
    QString idStr = QString::number(selfId);
    QSettings settings("./Users/" + idStr + "/" + idStr + ".ini", QSettings::IniFormat);
    QString avatar = settings.value("avatar").toString();

    if ((m_iconFile == "./avatar/avatar/4.png") || (avatar == m_iconFile) ) {
        return;
    }
    settings.setValue("avatar", m_iconFile);
    settings.sync();
}

void MainForm::quitApp()
{
    qDebug() << "quitApp xxxxxxxxxxxxxxxxxxxxxxxxxx";
    m_systemTray->hide();
    qApp->quit();
}

void MainForm::showMainForm()
{
    this->show();
    this->activateWindow();
    this->setFocus();
    setWindowState(Qt::WindowNoState);
}

void MainForm::slot_addFriendMsg(QString peerIp, quint16 peerPort, uint senderId,
                                 QString senderName, QString message, QString avatar)
{
    AddFriendDialog *pAfDlg = new AddFriendDialog;
    pAfDlg->setMsg(peerIp, peerPort, senderId, senderName, message, avatar);
    pAfDlg->setAttribute(Qt::WA_DeleteOnClose);
    pAfDlg->show();
}

void MainForm::slot_addFriendReply(uint senderId, QString senderName,
                                   int reply, QString avatar)
{
    AddFriendDialog *pAfDlg = new AddFriendDialog;
    pAfDlg->setReplyMg(senderId, senderName, reply, avatar);
    pAfDlg->setAttribute(Qt::WA_DeleteOnClose);
    pAfDlg->show();
}

void MainForm::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);

    QVariant var;
    var = index.data(ROLE_MARK_GROUP);
    if (var.isValid()) {
        groupMenu.exec(QCursor::pos());//弹出右键菜单，菜单位置为光标位置
    }
    var = index.data(ROLE_MARK_ITEM);
    if (var.isValid()) {
        initPopupMenu();
        itemMenu.exec(QCursor::pos());
    }
}

QString MainForm::getPeerAvatar(unsigned int peerId)
{
    QStandardItem *item = uidItemMap[peerId];
    if (item) {
        bool online = item->data(ROLE_IP).toString().isEmpty()?false:true;
        QString avatar = item->data(ROLE_AVATAR).toString();
        if (online)
            avatar = ":/avatar/avatar/" + avatar;
        else
            avatar = ":/avatar_gray/avatar_gray/" + avatar;
        return avatar;
    }
    return "";
}

QString MainForm::hasFriend(unsigned int friendId)
{
    if (uidItemMap.find(friendId) != uidItemMap.end()) {
        return uidItemMap[friendId]->data(ROLE_GROUP_BELONG).toString();
    }
    return "";
}

QStringList MainForm::getGroupList() const
{
    QStringList groupList;
    //QMap<QString, QStandardItem*>::iterator it = groupItemMap.begin();
    QMap<QString, QStandardItem*>::const_iterator it = groupItemMap.begin();
    for (; it != groupItemMap.end(); ++it) {
        groupList += it.key();
    }
    return groupList;
}

void MainForm::setHeadIconEnableState(bool enable)
{
    QString imgName = m_iconFile;
    QString trayImg = ":/image/sys_tray.png";
    if (!enable) {
        imgName = ":/avatar_gray/avatar_gray/" + m_avatar;
        trayImg = ":/image/sys_tray_gray.png";
    }
    ui->headIconLbl->setPixmap( QPixmap(imgName));
    m_systemTray->setIcon(QIcon(trayImg));
}

void MainForm::slot_videoRequest(uint peerId, quint16 peerPort)
{
    QStandardItem *item = uidItemMap[peerId];
    if (item == 0) return;
    QString msg = item->text() + "(" + QString::number(peerId) + ")" +
            tr("请求与您视频通讯,是否接收?");
    if (QMessageBox::information(this, "info", msg, QMessageBox::Yes|QMessageBox::No)
            == QMessageBox::Yes) {
        ChatWindow *cw = ChatWindow::getChatWindow(peerId);
        if (cw == 0) {
            QString peerIp = item->data(ROLE_IP).toString();
            quint16 peerPort = item->data(ROLE_PORT).toUInt();
            cw = new ChatWindow(0, peerId, item->text(), peerIp, peerPort);
        }
        cw->activateWindow();
        cw->startVideoMessage(peerPort, ROLE_RECEIVER);
    }
}

void MainForm::slot_recv_video_port(uint peerId, quint16 peerPort)
{
    ChatWindow *cw = ChatWindow::getChatWindow(peerId);
    if (cw) {
        cw->recvVideoPort(peerPort);
    }
}

void MainForm::on_searchToolBtn_clicked()  //如何确保只有一个窗口
{
    if (m_userMgrForm == 0) {
        m_userMgrForm = new UserManagerForm;
        m_userMgrForm->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_userMgrForm, SIGNAL(form_close()), this, SLOT(delUserMgrForm()));
        m_userMgrForm->show();
    } else {
        m_userMgrForm->activateWindow();
    }
}

void MainForm::delUserMgrForm()
{
    m_userMgrForm = 0;
}

UserManagerForm* MainForm::getUserMgrForm()
{
    return m_userMgrForm;
}

