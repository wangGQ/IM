#include "include/logindialog.h"
#include "include/global.h"
#include "include/mainform.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QDebug>

TcpConnector *gTcpConnector = new TcpConnector; //global

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon("./IM.ico"));
//    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//    QTextCodec::setCodecForLocale(codec);

    QFile qssFile(":/config/config/mystyle.qss");
    qssFile.open(QFile::ReadOnly);
    //qDebug() << qssFile.readAll();
    app.setStyleSheet(qssFile.readAll());
    FileTransfer::readConfig();

    LoginDialog *loginDlg = new LoginDialog;
    loginDlg->show();
    loginDlg->activateWindow();
    loginDlg->setFocus();
    loginDlg->setAttribute(Qt::WA_DeleteOnClose);

    return app.exec();
}

