#include "include/FaceDialog.h"
#include "ui_FaceDialog.h"
#include <QDir>
#include <QDebug>

FaceDialog::FaceDialog(QWidget *parent, int type) :
    QDialog(parent),
    ui(new Ui::FaceDialog)
{
    ui->setupUi(this);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);

    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    m_type = type;
    QString imgDir;
    if (m_type == CHOOSE_FACE) {
        imgDir = ":/faces/faces";
    } else if (m_type == CHOOSE_AVATAR) {
        imgDir = ":/avatar/avatar";
    }
    initFaces(imgDir);
}

void FaceDialog::initFaces(QString imgDir)
{
    QDir dir(imgDir);
    QStringList filter;
    filter << "*.jpg";
    filter << "*.png";
    dir.setNameFilters(filter);

    QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(filter));
    int row = 1;
    ui->tableWidget->setRowCount(row);
    ui->tableWidget->setColumnCount(NUMPER_ROW);
    ui->tableWidget->setIconSize(QSize(50, 50));
//    ui->tableWidget->resizeColumnsToContents();
//    ui->tableWidget->resizeRowsToContents();

    for (int i = 0; i < fileInfo->count(); ++i) {
        //qDebug() << (row - 1) << "," << (i % NUMPER_ROW);
        ui->tableWidget->setColumnWidth(i % NUMPER_ROW, 50);
        ui->tableWidget->setRowHeight(row - 1, 50);
        if ((i % NUMPER_ROW == 0) && (i > 0)) {
            row++;
            ui->tableWidget->setRowCount(row);
        }
        QString imgName = fileInfo->at(i).filePath();
        QTableWidgetItem *item = new QTableWidgetItem(QIcon(imgName), "");
        item->setData(ROLE_FACE_NAME, imgName);
        ui->tableWidget->setItem(row-1, i%NUMPER_ROW, item);
    }

    connect(ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this,
            SLOT(getItem(QTableWidgetItem*)));

}

void FaceDialog::getItem(QTableWidgetItem *faceItem)
{
    QString imgName = faceItem->data(ROLE_FACE_NAME).toString();
    if (m_type == CHOOSE_FACE)
        emit select_face(imgName);
    else if (m_type == CHOOSE_AVATAR)
        emit select_avatar(imgName);
}

FaceDialog::~FaceDialog()
{
    delete ui;
}
