#-------------------------------------------------
#
# Project created by QtCreator 2020-11-05T23:59:03
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Set program version
VERSION = 0.0.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT


TARGET = snap-herald
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        remotepixmaplabel.cpp \
        request.cpp \
        rungaurd.cpp \
        searchwidget.cpp \
        settingswidget.cpp \
        utils.cpp \
        widgets/cverticallabel.cpp \
        widgets/elidedlabel.cpp \
        widgets/waitingspinnerwidget.cpp

HEADERS += \
        elapsedtimer.h \
        mainwindow.h \
        notificationpopup.h \
        remotepixmaplabel.h \
        request.h \
        rungaurd.h \
        searchwidget.h \
        settingswidget.h \
        utils.h \
        widgets/cverticallabel.h \
        widgets/elidedlabel.h \
        widgets/waitingspinnerwidget.h

FORMS += \
        mainwindow.ui \
        searchwidget.ui \
        settingswidget.ui \
        track.ui

# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

icon.files = icons/snapherald.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = snapherald.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop

RESOURCES += \
    icons.qrc \
    theme/qbreeze.qrc
