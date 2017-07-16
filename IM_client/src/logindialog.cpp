#include "include/logindialog.h"
#include "ui_logindialog.h"
#include "include/registerdialog.h"
#include "include/global.h"
#include "include/mainform.h"
#include "include/MacroDefine.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QSettings>
#include "include/FaceDialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    Qt::WindowFlags flags = Qt::Widget;
    setWindowFlags(flags&~Qt::WindowMaximizeButtonHint);
    ui->setupUi(this);
    initIds();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(connectTimeOut()));
    connect(ui->idComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(initConf(QString)));
    m_loginOk = false;
    setWindowTitle(tr("登录"));
}

LoginDialog::~LoginDialog()
{
    qDebug() << "~login dialog";
    delete ui;
}

void LoginDialog::on_loginBtn_clicked()
{
    QString id  = ui->idComboBox->currentText();
    QString passWord = ui->passwdLineEdit->text();

    if (id.isEmpty() || passWord.isEmpty()) {
        QMessageBox::warning(this, "warining", tr("请将登录信息填写完整!"));
        return;
    }
    ui->loginBtn->setEnabled(false);
    gTcpConnector->setLoginMsg(id.toUInt(), passWord, this);
    m_timer.start(TIME_OUT);
}

QString LoginDialog::myEncrypt(QString str)
{
    char *c_arr = new char[str.length() + 1];
    strcpy(c_arr, str.toStdString().c_str());
    for (int i = 0; i < str.length(); ++i) {
        c_arr[i] += i;
    }
    QString resultStr(c_arr);
    delete c_arr;
    return resultStr;

}

QString LoginDialog::myDecrypt(QString str)
{
    char *c_arr = new char[str.length() + 1];
    strcpy(c_arr, str.toStdString().c_str());
    for (int i = 0; i < str.length(); ++i) {
        c_arr[i] -= i;
    }
    QString resultStr(c_arr);
    delete c_arr;
    return resultStr;
}

void LoginDialog::saveConfig()
{
    QSettings settings("./Users/" + m_curId + "/" + m_curId + ".ini", QSettings::IniFormat);
    if (ui->savePwdChkBox->isChecked()) {
        settings.setValue("save_pwd", true);
        settings.setValue("pwd", myEncrypt(ui->passwdLineEdit->text()));
    } else if (settings.contains("pwd")) {
        settings.remove("save_pwd");
        settings.remove("pwd");
    }
    if (!m_idList.contains(m_curId)) {
        QSettings appSettings("IM_client.ini", QSettings::IniFormat);
        appSettings.setValue(m_curId, "user_id");
    }
}

void LoginDialog::initIds()
{
    QSettings settings("IM_client.ini", QSettings::IniFormat);
    m_idList = settings.allKeys();
    foreach (QString id, m_idList) {
        if (settings.value(id).toString() != "user_id") continue;
        if (m_curId.isEmpty()) {
            m_curId = id;
        }
        ui->idComboBox->addItem(id);
    }
    ui->idComboBox->setCurrentText(m_curId);
    initConf(m_curId);
}

void LoginDialog::initConf(QString newId)
{
    m_curId = newId;
    QSettings settings("./Users/" + m_curId + "/" + m_curId + ".ini", QSettings::IniFormat);
    bool savePwd = settings.value("save_pwd", QVariant(false)).toBool();
    ui->savePwdChkBox->setChecked(savePwd);
    if (savePwd) {
        QString tmp = settings.value("pwd").toString();
        ui->passwdLineEdit->setText(myDecrypt(tmp));
        qDebug() << "[myDecrypt]:" << myDecrypt(tmp);
    } else {
        ui->passwdLineEdit->clear();
    }
    m_iconFile = settings.value("avatar", QVariant(":/avatar/avatar/4.png")).toString();
    ui->headIconLbl->setScaledContents(true);
    ui->headIconLbl->setPixmap( QPixmap(m_iconFile));
    ui->headIconLbl->setStyleSheet("border:0.5px solid black");
    //------------------------------------
}

void LoginDialog::on_registerBtn_clicked()
{
    RegisterDialog *regDlg = new RegisterDialog();
    regDlg->setAttribute(Qt::WA_DeleteOnClose);
    regDlg->show();
}

void LoginDialog::handleResponse(quint16 result, QString nickName, QString avatar)
{
    m_timer.stop();
    ui->loginBtn->setEnabled(true);
    if (result == (quint16)LOGIN_OK) {
        saveConfig();
        m_loginOk = true;
        this->close();
        MainForm *mainForm = new MainForm(0, m_curId.toUInt(), nickName, avatar);
        qDebug() << "[iconFile in loginDlg] = " << avatar;
        mainForm->setAttribute(Qt::WA_DeleteOnClose);
        mainForm->show();
    } else if (result == (quint16)LOGIN_FAIL) {
        QMessageBox::information(this, tr("提示"), tr("账号不存在或密码错误!"));
    } else if (result == (quint16)REPEAT_LOGIN) {
        QMessageBox::warning(this, tr("warnning"), tr("该账号已登录,不能重复登录!"));
    }
}

void LoginDialog::connectTimeOut()
{
    QMessageBox::information(this, tr("info"), tr("连接超时"));
    ui->loginBtn->setEnabled(true);
    m_timer.stop();
}

void LoginDialog::closeEvent(QCloseEvent *)
{
    if (!m_loginOk)
        qApp->quit();
}
