#include "include/chatwindow.h"
#include "ui_chatwindow.h"
#include "include/TcpConnector.h"
#include "include/global.h"
#include "include/filetransfer.h"
#include "include/MacroDefine.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStack>
#include <QProgressBar>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QFontDialog>
#include <QColorDialog>
#include <QTextStream>
#include <QDatetime>
#include <QDir>
#include "include/UdpClient.h"

QMap<unsigned int, ChatWindow*> ChatWindow::idChatWinMap;
QString ChatWindow::selfName;
unsigned int ChatWindow::selfId;

ChatWindow::ChatWindow(QWidget *parent, unsigned int peerId, QString peerName, QString peerIp, quint16 peerPort) :
    QWidget(parent),m_msgCnt(0), m_curFTCnt(0), m_launchRole(ROLE_RECEIVER),
    ui(new Ui::ChatWindow)
{
    qDebug() << "new chat window:ip = " << peerIp << ",port = " << peerPort;
    ui->setupUi(this);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    this->peerName = peerName;
    this->peerId = peerId;

    setPeerAddr(peerIp, peerPort);
    idChatWinMap.insert(peerId, this);
    setWindowTitle(tr("与") + peerName + tr("聊天"));
    selfId  = TcpConnector::selfId;
    selfName = TcpConnector::selfName;

    ui->tabWidget->hide();
    resize(QSize(m_chatFormWidth, m_formHeight));
    initConf();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(turnToNewDay()));
    m_timer.start(1000);

    ui->widget_3->installEventFilter(this);
    qDebug() << "before setPeerIcon";
    setPeerIcon();
}

void ChatWindow::initConf()
{
    ui->peerNameLbl->setText(peerName);
    m_font = tr("微软雅黑");

    connect(ui->fontComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(fontChanged(QString)));
    ui->fsComboBox->setCurrentText("11");
    m_fontSize = ui->fsComboBox->currentText().toInt();
    qDebug() << "[m_fontSize = ]" << m_fontSize;
    connect(ui->fsComboBox, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(fontSizeChanged(QString)));
    m_isBold = 0;
    ui->msgTextEdit->setFont(QFont(m_font, m_fontSize));
    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(date_edit_value_changed(QDate)));
    ui->fontComboBox->setCurrentText(m_font);

    ui->dateEdit->hide();
    m_color.setRgb(0, 0, 0);
}

bool ChatWindow::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->widget_3) {
        if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "QEvent::Mouse press";
            if (m_faceDlg) {
                m_faceDlg->hide();
            }
        }
    }
}

void ChatWindow::setPeerIcon()
{
    QString peerAvatar = gTcpConnector->getMainForm()->getPeerAvatar(peerId);
    ui->peerIconLbl->setScaledContents(true);
    ui->peerIconLbl->setPixmap(QPixmap(peerAvatar));
}

void ChatWindow::setLaunchRole(int role)
{
    m_launchRole = role;
}

ChatWindow::~ChatWindow()
{
    qDebug() << "~ChatWindow";
    if (m_faceDlg) delete m_faceDlg;
    delete ui;
}

void ChatWindow::setPeerAddr(QString peerIp, quint16 peerPort)
{
    qDebug() << "[ChatWindow::setPeerAddr]";
    this->peerIp   = peerIp;
    this->peerPort = peerPort;
}

void ChatWindow::displayMsg(QString sender, QDateTime sendTime, QString htmlMsg, int launchRole)
{
    ++m_msgCnt;
    QString title = sender + " " + sendTime.toString("yyyy/MM/dd hh:mm:ss");
    QString htmlTitle;
    if (launchRole == ROLE_SENDER) {
        htmlTitle = QString("<font style = \"font-size:10pt;font-family:微软雅黑;color:rgb(0,0,255)\">%1</font>").
                arg(title);
    } else {
        htmlTitle = QString("<font style = \"font-size:10pt;font-family:微软雅黑;color:rgb(255,0,0)\">%1</font>").
                arg(title);
    }
    ui->displayTextEdit->append(htmlTitle);
    ui->displayTextEdit->append(htmlMsg);
}

ChatWindow* ChatWindow::getChatWindow(unsigned int peerId)
{
    return idChatWinMap[peerId];
}

void ChatWindow::on_sendBtn_clicked()
{
    QString message = ui->msgTextEdit->toPlainText();
    if (message == "") {
        QMessageBox::warning(this, tr("提示"), tr("发送消息不能为空!"), QMessageBox::Ok);
        return;
    }
    if (m_msgCnt == 0) {
        qDebug() << "I am senderxxxxxxxxxxxxxxxxxxxxxx";
        m_launchRole = ROLE_SENDER;
    }
    setTextFormat(1);
    QDateTime sendTime = QDateTime::currentDateTime();

    QTextDocument *document = ui->msgTextEdit->document();

    QString htmlMsg = document->toHtml();

    QStack<QVariant> args;
    args.push(m_launchRole);
    args.push(TYPE_INT);
    args.push(htmlMsg);
    args.push(TYPE_QSTRING);
    args.push(selfName);
    args.push(TYPE_QSTRING);
    args.push(selfId);
    args.push(TYPE_UINT);
    args.push(sendTime);
    args.push(TYPE_DATE_TIME);
    args.push(NORMAL_MSG);
    UdpClient::sendRequest(peerIp, peerPort, args);

    displayMsg(selfName, sendTime, htmlMsg, m_launchRole);
    ui->msgTextEdit->clear();
}

void ChatWindow::updateProBar(QString fileName, qint64 totalBytes, qint64 bytesFinished)
{
    QProgressBar *proBar = fileProBarMap[fileName];
    if (proBar) {
        proBar->setMaximum(totalBytes);
        proBar->setValue(bytesFinished);
    }
}

void ChatWindow::handleSendFile(unsigned int peerId, int response, QString filePath)
{
    qDebug() << "[chat window:handleSendFile]:response:" << response;
    ChatWindow *cw = NULL;
    if (idChatWinMap.find(peerId) != idChatWinMap.end()) {
        cw = idChatWinMap[peerId];
    } else {
        qDebug() << "[chat window:handleSendFile]-cw is null";
        return;
    }
    QString fileName = filePath.right(filePath.size()
                                        - filePath.lastIndexOf('/') - 1);
    if (response == RECV_FILE_CANCLE) {
        QString msg = tr("对方取消了文件") +  fileName + tr("的接收");
        QMessageBox::information(cw, "info", msg, QMessageBox::Ok);
        cw->on_FT_closeBtn_clicked();
    } else if (response == RECV_FILE_OK) {
        qDebug() << "accept to recv file";
        FileTransfer *ft = FileTransfer::getTransfer(filePath);
        if (ft) {
            qDebug() << "peerIp = " << cw->getPeerIp();
            ft->setPeerIp(cw->getPeerIp());
            ft->start();
        } else {
            qDebug() << "ft is null in chatwindow";
        }
    }
}

void ChatWindow::on_fileToolBtn_clicked() //include同时选中多个文件的情况
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this);

    foreach (QString filePath, filePaths) {
        qDebug() << "open file:" << filePath << "success!";
        createNewFtTask(filePath, ROLE_SENDER);
    }
}

void ChatWindow::createNewFtTask(QString filePath, int role)
{
    qDebug() << "createNewFtTask------------------------";
    QString fileName = filePath.right(filePath.size()
                                        - filePath.lastIndexOf('/') - 1);
    ui->tab->hide();
    ui->tabWidget->setVisible(true);
    ui->tabWidget->setCurrentIndex(1);
    this->resize(QSize(m_formWidth, m_formHeight));
    ui->FT_tbl_widget->horizontalHeader()->setVisible(false);
    ui->FT_tbl_widget->verticalHeader()->setVisible(false);
    ui->FT_tbl_widget->setRowCount(m_curFTCnt + 1);
    ui->FT_tbl_widget->setColumnCount(3); //最后一个单元格显示状态

    ui->FT_tbl_widget->setRowHeight(m_curFTCnt, 25);
    ui->FT_tbl_widget->setColumnWidth(0, 150);
    ui->FT_tbl_widget->setColumnWidth(1, 230);

    QFileInfo fileInfo(filePath);
    QFileIconProvider fip;
    ui->FT_tbl_widget->setItem(m_curFTCnt, 0, new QTableWidgetItem(fip.icon(fileInfo), fileName));
    QProgressBar *proBar = new QProgressBar; //when delete
    fileProBarMap.insert(filePath, proBar);
    ui->FT_tbl_widget->setCellWidget(m_curFTCnt, 1, proBar);
    ++m_curFTCnt;
    FileTransfer *fileTransfer = new FileTransfer(filePath, FILE_TRANSFER, role, this);   //when delete this pointer
    connect(fileTransfer, SIGNAL(ft_progress_change(QString,qint64,qint64)), this, SLOT(updateProBar(QString,qint64,qint64)));
    connect(fileTransfer, SIGNAL(finished()), fileTransfer, SLOT(deleteLater()));
    if (role == ROLE_SENDER) {
        qDebug() << "role == ROLE_SENDER";
        sendFileRequest(filePath);
    } else if (role == ROLE_RECEIVER) {
        fileTransfer->start();
    }
}

void ChatWindow::sendFileRequest(QString filePath)
{
    qDebug() << "sendFileRequest";
    QFile file(filePath);
    QStack<QVariant> args;
    args.push(gTcpConnector->getSelfPort());
    args.push(TYPE_UINT16);
    args.push(file.size());
    args.push(TYPE_INT64);
    args.push(filePath);
    args.push(TYPE_QSTRING);
    args.push(selfName);
    args.push(TYPE_QSTRING);
    args.push(selfId);
    args.push(TYPE_UINT);
    args.push(FILE_TRANSFER);

    UdpClient::sendRequest(peerIp, peerPort, args);
}

void ChatWindow::closeEvent(QCloseEvent *)
{
    saveChatLog();
    //idChatWinMap.remove(peerId);
    emit chatwindow_close(peerId);
}

void ChatWindow::saveChatLog() //保存聊天信息
{
    QString curDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    if (ui->displayTextEdit->toPlainText().isEmpty()) return;
    QTextDocument *document = ui->displayTextEdit->document();

    QString chatLog = document->toHtml();
    //qDebug() << chatLog;
    QDir dir;
    QString logDir = "./Users/" + QString::number(selfId) + "/ChatLog";
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }

    QString fileName = logDir + "/" + QString::number(peerId) + "-" + curDate + ".htm";
    QFile file(fileName);
    if (!file.exists()) {
        file.open(QIODevice::WriteOnly);
        file.close();
        file.open(QIODevice::WriteOnly);
    } else {
        file.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    QTextStream out(&file);
    out << chatLog;
    file.close();
}

void ChatWindow::date_edit_value_changed(QDate date)
{
    QString fileName = "./Users/" + QString::number(selfId) + "/ChatLog/" +
            QString::number(peerId) + "-" + date.toString("yyyy-MM-dd") + ".htm";
    QFile file(fileName);
    if (file.exists()) {
        if(!file.open(QFile::ReadOnly | QFile::Text)) {
            ui->textBrowser->clear();
        } else {
            QTextStream in(&file);
            ui->textBrowser->setHtml(in.readAll());
        }
    } else {
        ui->textBrowser->clear();
    }
}


void ChatWindow::fontChanged(QString newFont)
{
    m_font = newFont;
    setTextFormat(5);
    qDebug() << m_font;
}

void ChatWindow::fontSizeChanged(QString fs)
{
    m_fontSize = fs.toInt();
    setTextFormat(6);
    qDebug() << m_fontSize;
}

QString ChatWindow::getPeerIp() const
{
    return peerIp;
}

quint16 ChatWindow::getPeerPort() const
{
    return peerPort;
}

void ChatWindow::on_closeBtn_clicked()
{
    idChatWinMap.remove(peerId);
    qDebug() << "[remove chat window] " << peerId;
    this->close();
}

void ChatWindow::changeEvent(QEvent *event) //最小化
{
    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() & Qt::WindowMinimized) {
            if (m_faceDlg) m_faceDlg->hide();
        }
        QWidget::changeEvent(event);
    }
}

void ChatWindow::on_chatLogBtn_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->show();
    this->resize(QSize(m_formWidth, m_formHeight));
    ui->tabWidget->show();
}

void ChatWindow::on_FT_closeBtn_clicked()
{
    ui->FT_tbl_widget->clear();
    fileProBarMap.clear();
    ui->tabWidget->hide();
    this->resize(QSize(m_chatFormWidth, m_formHeight));
    m_curFTCnt = 0;
}

void ChatWindow::startVideoMessage(quint16 peerPort, int role)
{
    qDebug() << "startVideoMessage";
    m_videoForm = new VideoForm(0, peerIp, role);
    m_videoForm->setAttribute(Qt::WA_DeleteOnClose);
    //videoForm->startRecvThread();
    m_videoForm->startSendImage(peerPort);
    m_videoForm->show();
}

void ChatWindow::recvVideoPort(quint16 port)
{
    if (m_videoForm) {
        m_videoForm->startSendImage(peerPort);
    }
}

void ChatWindow::on_logCloseBtn_clicked()
{
    ui->tabWidget->hide();
    this->resize(QSize(m_chatFormWidth, m_formHeight));
    ui->dateEdit->hide();
}

void ChatWindow::on_colorToolBtn_clicked()
{
    m_color = QColorDialog::getColor(Qt::black);
    if (m_color.isValid()) {
        setTextFormat(1);
        QPalette p;
        p.setColor( QPalette::Text, m_color);
        ui->msgTextEdit->setPalette(p);
    }
}

void ChatWindow::setTextFormat(int type) //1-color;2-bold;3-italic;4-underline;5-font;6-fontsize
{
    QTextCursor cursor = ui->msgTextEdit->textCursor();
    int pos = cursor.position();
    ui->msgTextEdit->selectAll();
    //ui->msgTextEdit->setTextColor(m_color);
    switch (type) {
        case 1:
            ui->msgTextEdit->setTextColor(m_color);
            break;
        case 2:
            if (m_isBold) {
                ui->msgTextEdit->setFontWeight(QFont::Bold);
            } else {
                ui->msgTextEdit->setFontWeight(QFont::Normal);
            }
            break;
        case 3:
            ui->msgTextEdit->setFontItalic(m_isItalic);
            break;
        case 4:
            ui->msgTextEdit->setFontUnderline(m_isUnderLine);
            break;
        case 5:
            ui->msgTextEdit->setFont(m_font);
            break;
        case 6:
            ui->msgTextEdit->setFontPointSize(m_fontSize);
            break;
    }
    cursor.movePosition(QTextCursor::Start);
    cursor.setPosition(pos);
    ui->msgTextEdit->setTextCursor(cursor);
}

void ChatWindow::turnToNewDay()
{
    QTime curTime = QTime::currentTime();

    if (curTime.hour() == 23 && curTime.minute() == 59 &&
            curTime.second() == 59) {
        saveChatLog();
        //ui->displayTextEdit->clear();

    }
}

void ChatWindow::on_textBoldBtn_clicked()
{
    m_isBold = ui->textBoldBtn->isChecked();
    setTextFormat(2);
}

void ChatWindow::on_textitalicBtn_clicked() //斜体
{
    m_isItalic = ui->textitalicBtn->isChecked();
    setTextFormat(3);
}

void ChatWindow::on_textUnderlineBtn_clicked()
{
    m_isUnderLine = ui->textUnderlineBtn->isChecked();
    setTextFormat(4);
}

void ChatWindow::handle_face(QString faceName)
{
    QString htmlCode = QString("<img src=\"%1\" width = \"35\" height = \"35\" />").arg(faceName);
    qDebug() << htmlCode;
    QTextCursor cursor = ui->msgTextEdit->textCursor();
    cursor.insertHtml(htmlCode);
    m_faceDlg->hide();
}

void ChatWindow::on_faceBtn_clicked()
{
    if (m_faceDlg == 0) {
        m_faceDlg = new FaceDialog;
        connect(m_faceDlg, SIGNAL(select_face(QString)), this, SLOT(handle_face(QString)));
        m_faceDlg->show();
    } else {
        m_faceDlg->setVisible(true);
    }
    QPoint pos = ui->faceBtn->mapToGlobal(QPoint(0, 0));
    m_faceDlg->move(pos.x(), pos.y() - m_faceDlg->height() - 10);
    m_faceDlg->setFocus();
    m_faceDlg->activateWindow();
}

void ChatWindow::slot_sendVideoRequest(quint16 myPort, int role)
{
    QStack<QVariant> args;
    args.push(myPort);
    args.push(TYPE_UINT16);
    args.push(selfId);
    args.push(TYPE_UINT);
    if (role == ROLE_SENDER)
       args.push(VIDEO_TRANSFER);
    else
       args.push(REPLY_VIDEO_PORT);
    UdpClient::sendRequest(peerIp, peerPort, args);
}

void ChatWindow::removeCW(uint peerId)
{
    idChatWinMap.remove(peerId);
}

void ChatWindow::on_videoBtn_clicked()
{
    m_videoForm = new VideoForm(0, peerIp, ROLE_SENDER);
    connect(m_videoForm, SIGNAL(sig_video_ready(quint16, int)), this,
            SLOT(slot_sendVideoRequest(quint16, int)));
    m_videoForm->setAttribute(Qt::WA_DeleteOnClose);
    m_videoForm->show();
    //videoForm->openCamara();
}
