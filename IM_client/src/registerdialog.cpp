#include "include/registerdialog.h"
#include "ui_registerdialog.h"
#include "include/TcpConnector.h"
#include <QMessageBox>
#include <QDir>
#include "include/global.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->msg_label->setStyleSheet("color:red");
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(connectTimeOut()));
    setWindowTitle(tr("注册"));
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::handleResponse(unsigned int id)
{
    m_timer.stop();
    if (id != 0) {
        //生成以账号命名的文件夹
        QString idStr = QString::number(id);
        QDir newDir;
        if (newDir.exists("./Users/" + idStr)) {
            newDir.rmdir("./Users/" + idStr);
        }
        newDir.mkdir("./Users/" + idStr);
        QString msg = tr("注册成功!\n您的登录号码为:") + idStr;
        if (QMessageBox::information(this, tr("注册成功"), msg, QMessageBox::Ok) == QMessageBox::Ok) {
            this->close();
        }
    } else {
        QMessageBox::information(this, tr("提示"), tr("注册失败!"));
        ui->regButton->setEnabled(true);
    }
}

bool RegisterDialog::checkRegMsg()
{
    nickName = ui->nick_nameLineEdit->text();
    if (nickName.length() < 3 || nickName.length() > 15)
    {
        ui->msg_label->setText(tr("昵称长度要求3 ~ 15!"));
        return false;
    }
    passWord = ui->passwdLineEdit->text();
    if (passWord.length() < 5 || passWord.length() > 15)
    {
        ui->msg_label->setText(tr("密码长度要求5 ~ 15!"));
        return false;
    }
    QString pattern("^(?![0-9]+$)(?![a-zA-Z]+$)");
    QRegExp rx(pattern);
    if (passWord.indexOf(rx) == -1)
    {
        ui->msg_label->setText(tr("密码必须同时包含数字和字母!"));
        return false;
    }
    QString repPassWord = ui->repPasswdLineEdit->text();
    if (passWord != repPassWord)
    {
        ui->msg_label->setText(tr("两次密码不一致!"));
        return false;
    }
    if (ui->male_radioButton->isChecked())
        sex = 0;
    else if (ui->female_radioButton->isChecked())
        sex = 1;
    else
    {
        ui->msg_label->setText(tr("请选择性别!"));
        return false;
    }
    m_newUser = new User(nickName, passWord, sex);
    return true;
}


void RegisterDialog::on_regButton_clicked()
{
   qDebug() << "regButton click";
   if (checkRegMsg())
   {
       ui->msg_label->clear();
       ui->regButton->setEnabled(false);
       gTcpConnector->setRegisterMsg(m_newUser, this);
       m_timer.start(3 * 1000);
   }
}

void RegisterDialog::connectTimeOut()
{
    QMessageBox::information(this, tr("info"), tr("连接超时"));
    ui->regButton->setEnabled(true);
    m_timer.stop();
}
