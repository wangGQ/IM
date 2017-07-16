#ifndef VIDEOFORM_H
#define VIDEOFORM_H

#include <QWidget>
#include <QTimer>
#include <highgui.h>
#include <cv.h>
#include "include/UdpVideoThread.h"
#include "include/MacroDefine.h"
namespace Ui {
class VideoForm;
}

class VideoForm : public QWidget
{
    Q_OBJECT

public:
    explicit VideoForm(QWidget *parent = 0, QString peerIp = "127.0.0.1",
                       int role = ROLE_SENDER);
    ~VideoForm();
    void closeCamara(); //
    void startSendImage(quint16); //打开摄像头
    void closeEvent(QCloseEvent*);
signals:
    void sig_video_ready(quint16, int);
private slots:
    void slot_readFrame(); // 读取当前帧信息
    void slot_showImage(QImage);
    void slot_sendBindingPort(quint16);
private:
    Ui::VideoForm *ui;
    QTimer *m_timer;
    QImage *m_img;
    CvCapture *m_cam;
    IplImage *m_frame;
    UdpVideoThread *m_recvThread;
    quint16 m_peerPort;
    QString m_peerIp;
    int m_role;
};

#endif // VIDEOFORM_H
