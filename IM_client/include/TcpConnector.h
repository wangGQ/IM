#ifndef TCPCONNECTOR_H
#define TCPCONNECTOR_H
#include <QString>
#include <QTcpSocket>
#include <QObject>
#include "include/logindialog.h"
#include "include/registerdialog.h"
#include "include/mainform.h"
#include "include/chatwindow.h"
#include "include/usermanagerform.h"
#include "include/filetransfer.h"
#include <QHostAddress>
#include <QTimer>
#include <QStack>

class TcpConnector : public QObject
{
    Q_OBJECT
public:

    TcpConnector();
    void regUserRequest(); //参数应该为User对象 //其实不用id吧,selfId就行了,
    void setChatWindow(ChatWindow* pChatWin);
    //void addGroupRequest(QString groupName);
    void setMainForm(MainForm *pMainForm);
    void setLoginMsg(unsigned int userId, QString pwd, LoginDialog* loginDlg);
    void setRegisterMsg(User *newUser, RegisterDialog* pRegDlg);

    void setUserMgrForm(UserManagerForm *userMgrForm);

    bool isConnectedToServer();
    void sendRequest(QStack<QVariant> &args);

    quint16 getSelfPort();
    QString getSelfIp() const;

    MainForm* getMainForm();
    UserManagerForm* getUserMgrForm() const;

    static void readConfig();

private slots:
    void socketError();
    void dataReceived();
    void foundServer();
    void successConnected();
    void disconnectToServer();
    void reconnectToServer();
public:
    static QString serverIp;
    static quint16 serverPort;
    static QString selfName;
    static unsigned int selfId;
private:
    QTcpSocket tcpSocket;
    FileTransfer *m_ft;  //
    quint64 nextBlockSize;
    bool isConnected;
    QString m_pwd;
    int m_requestType;
    User *m_newUser;
    QTimer m_timer;

    LoginDialog *m_pLoginDlg;
    RegisterDialog *m_pRegDlg;
    MainForm  *m_pMainForm;
    ChatWindow *m_pChatWin;
    UserManagerForm *m_pUserMgrForm;
};

#endif // TCPCONNECTOR_H
