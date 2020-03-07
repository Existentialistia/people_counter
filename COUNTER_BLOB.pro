QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = counter
TEMPLATE = app
CONFIG += c++11


INCLUDEPATH += C:\Qt\opencv\build\install\include

LIBS += C:\Qt\opencv\build\bin\libopencv_core420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_highgui420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_imgcodecs420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_imgproc420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_features2d420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_calib3d420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_videoio420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_video420.dll
LIBS += C:\Qt\opencv\build\bin\opencv_videoio_ffmpeg420.dll
LIBS += C:\Qt\opencv\build\bin\libopencv_objdetect420.dll

SOURCES += \
    blob.cpp \
    main.cpp

HEADERS += \
    blob.h


