#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QString>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDateTime>
#include <QTimer>
#include <QMap>
#include <QPoint>
#include <QProgressBar>
#include <QFont>
#include <QColor>
#include <QIcon>
#include "include/filetransfer.h"
#include "include/FaceDialog.h"
#include "include/VideoForm.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = 0, unsigned int peerId = 0, QString peerName = "",
            QString peerIp = "", quint16 peerPort = 0);
    ~ChatWindow();

    void setPeerAddr(QString peerIp, quint16 peerPort);
    void displayMsg(QString sender, QDateTime sendTime, QString htmlMsg, int launchRole);
    void setPeerIcon();
    void setLaunchRole(int role); //设置对话发起的角色,sender or receiver
    void sendFileRequest(QString filePath);
    
    void createNewFtTask(QString filePath, int role); //new filetransfer task,role ->sender/receiver

    void startVideoMessage(quint16 peerPort, int role);
    void recvVideoPort(quint16 peerPort);
    void closeEvent(QCloseEvent*);
    QString getPeerIp() const;
    quint16 getPeerPort() const;
    static ChatWindow* getChatWindow(unsigned int peerId);

    static void handleSendFile(unsigned int peerId, int response, QString filePath);
    static void removeCW(uint peerId);
signals:
    void chatwindow_close(uint peerId);
protected:
    void initConf();
    void saveChatLog();
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *);
    void changeEvent(QEvent *);
    bool eventFilter(QObject *, QEvent *);
private slots:
    void on_sendBtn_clicked();

    void on_closeBtn_clicked();

    void on_fileToolBtn_clicked();
    void fontChanged(QString newFont);
    void fontSizeChanged(QString fs);
    void on_chatLogBtn_clicked();

    void on_FT_closeBtn_clicked();

    void on_logCloseBtn_clicked();

    void setTextFormat(int type);

    void on_colorToolBtn_clicked();
    void date_edit_value_changed(QDate date);
    void updateProBar(QString fileName, qint64 totalBytes, qint64 bytesFinished);
    void turnToNewDay();
    void on_textBoldBtn_clicked();

    void on_textitalicBtn_clicked();

    void on_textUnderlineBtn_clicked();

    void on_faceBtn_clicked();
    void handle_face(QString);
    void slot_sendVideoRequest(quint16, int role);
    void on_videoBtn_clicked();

private:
    Ui::ChatWindow *ui;
    QString peerName;
    unsigned int peerId;
    quint16 peerPort;

    int m_launchRole;
    int m_msgCnt;
    QString peerIp;
    QPoint windowPos, mousePos, dPos;

    quint8 m_recvFileOption;
    QString m_filePath;

    static QString selfName;
    static unsigned int selfId;
    static QMap<unsigned int, ChatWindow*> idChatWinMap;

    int m_curFTCnt; //当前文件传输的数量
    //------------------------------------
    QString m_font;
    int m_fontSize;
    bool m_isBold;  //是否粗体
    bool m_isUnderLine;
    bool m_isItalic;
    QColor m_color;
    QMap<QString, QProgressBar*> fileProBarMap;
    QTimer m_timer;
    FaceDialog *m_faceDlg;
    VideoForm *m_videoForm;
    static const int  m_formWidth = 1182;
    static const int m_formHeight = 680;
    static const int m_chatFormWidth = 693;
};

#endif // CHATWINDOW_H
