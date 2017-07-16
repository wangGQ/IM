#-------------------------------------------------
#
# Project created by QtCreator 2015-01-01T10:19:26
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += rtti
CONFIG += exceptions

TARGET = IM_client
TEMPLATE = app
INCLUDEPATH += D:\opencv\MinGW\install\include\
INCLUDEPATH += D:\opencv\MinGW\install\include\opencv

LIBS += D:\opencv\MinGW\lib\libopencv_core249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_highgui249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_imgproc249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_calib3d249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_contrib249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_features2d249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_flann249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_legacy249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_ml249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_gpu249d.dll.a

LIBS += D:\opencv\MinGW\lib\libopencv_video249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_objdetect249d.dll.a
LIBS += D:\opencv\MinGW\lib\libopencv_ts249d.a

SOURCES += \
    src/main.cpp \
    src/registerdialog.cpp \
    src/logindialog.cpp \
    src/TcpConnector.cpp \
    src/chatwindow.cpp \
    src/User.cpp \
    src/mainform.cpp \
    src/udpsocketthread.cpp \
    src/usermanagerform.cpp \
    src/filetransfer.cpp \
    src/AddFriendDialog.cpp \
    src/UdpClient.cpp \
    src/FaceDialog.cpp \
    src/VideoForm.cpp \
    src/UdpVideoThread.cpp

HEADERS  += \
    include/registerdialog.h \
    include/logindialog.h \
    include/TcpConnector.h \
    include/chatwindow.h \
    include/MacroDefine.h \
    include/User.h \
    include/mainform.h \
    include/udpsocketthread.h \
    include/usermanagerform.h \
    include/global.h \
    include/filetransfer.h \
    include/AddFriendDialog.h \
    include/UdpClient.h \
    include/FaceDialog.h \
    include/VideoForm.h \
    include/UdpVideoThread.h

FORMS    += \
    ui/registerdialog.ui \
    ui/logindialog.ui \
    ui/chatwindow.ui \
    ui/mainform.ui \
    ui/usermanagerform.ui \
    ui/AddFriendDialog.ui \
    ui/FaceDialog.ui \
    ui/VideoForm.ui

RESOURCES += \
    res/IM.qrc
RC_FILE = icon.rc
