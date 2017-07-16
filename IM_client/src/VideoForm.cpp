#include "include/VideoForm.h"
#include "ui_VideoForm.h"
#include "include/UdpClient.h"
#include <QBuffer>
#include <QDebug>

VideoForm::VideoForm(QWidget *parent, QString peerIp, int role) :
    QWidget(parent), m_cam(0), m_peerIp(peerIp), m_role(role),
    ui(new Ui::VideoForm)
{
    ui->setupUi(this);
    m_img = new QImage;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_readFrame()));
    //openCamara();
    m_recvThread = new UdpVideoThread;
    connect(m_recvThread, SIGNAL(sendImage(QImage)), this,
            SLOT(slot_showImage(QImage)));
    connect(m_recvThread, SIGNAL(sig_video_port(quint16)), this,
            SLOT(slot_sendBindingPort(quint16)));
    m_recvThread->start();
}

void VideoForm::startSendImage(quint16 peerPort)
{
    m_peerPort = peerPort;
    m_cam = cvCaptureFromCAM(0);
    //m_cam = cvCreateCameraCapture(0);
    m_timer->start(33);
}

void VideoForm::slot_sendBindingPort(quint16 myPort)
{
    qDebug() << "myPort is " << myPort;
    emit sig_video_ready(myPort, m_role); //确定了自己接收数据的端口
}

void VideoForm::slot_readFrame()
{
    m_frame = cvQueryFrame(m_cam);
    if (m_frame == 0) return;

    QImage image = QImage((const uchar*)m_frame->imageData,
         m_frame->width, m_frame->height, QImage::Format_RGB888).rgbSwapped();

    if (image.isNull()) {
        qDebug() << "Image is null";
        return;
    }
    //ui->videoLbl->setPixmap(QPixmap::fromImage(image));

    QByteArray bArr;
    QBuffer buffer(&bArr);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "jpg");
    UdpClient::sendVideoData(bArr.size(), bArr, m_peerIp, m_peerPort);
}

void VideoForm::slot_showImage(QImage image)
{
    //qDebug() << "slot_showImage";
    ui->videoLbl->setPixmap(QPixmap::fromImage(image));
}

void VideoForm::closeCamara()
{
    m_timer->stop();
    cvReleaseCapture(&m_cam);
}

void VideoForm::closeEvent(QCloseEvent *)
{
    m_recvThread->quit();
    m_recvThread->wait();
}

VideoForm::~VideoForm()
{
    qDebug() << "~VideoForm";
    delete ui;
    closeCamara();
    delete m_timer;
    delete m_recvThread;
}
