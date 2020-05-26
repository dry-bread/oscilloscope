#-------------------------------------------------
#
# Project created by QtCreator 2020-05-15T21:13:27
#
#-------------------------------------------------

QT       += core gui
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = oscilloscope
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    fft.cpp

HEADERS += \
        mainwindow.h \
    fft.h \
    fftw3.h

FORMS += \
        mainwindow.ui

ICON += ":/Icon/mainwindow1.png"

RESOURCES += \
    res.qrc


FFTWPATH = D:/qt/fftw-3.3.5-dll32

#win32: LIBS += D:/qt/fftw-3.3.5-dll32/libfftw3-3.dll

#INCLUDEPATH += $$PWD/D:/qt/fftw-3.3.5-dll32
#DEPENDPATH += $$PWD/D:/qt/fftw-3.3.5-dll32

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../fftw-3.3.5-dll32/ -llibfftw3-3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../fftw-3.3.5-dll32/ -llibfftw3-3
else:unix: LIBS += -L$$PWD/../fftw-3.3.5-dll32/ -llibfftw3-3

INCLUDEPATH += $$PWD/../fftw-3.3.5-dll32
DEPENDPATH += $$PWD/../fftw-3.3.5-dll32
