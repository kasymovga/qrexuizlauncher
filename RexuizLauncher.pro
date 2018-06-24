#-------------------------------------------------
#
# Project created by QtCreator 2018-06-12T12:52:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RexuizLauncher
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


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    launcher.cpp \
    dialogselectinstalllocation.cpp \
    rexuiz.cpp \
	miniz.c \
    unzip.cpp \
    sign.cpp

HEADERS += \
        mainwindow.h \
    launcher.h \
    dialogselectinstalllocation.h \
    rexuiz.h \
	miniz.h \
    unzip.h \
    sign.h

FORMS += \
        mainwindow.ui \
    dialogselectinstalllocation.ui

RESOURCES += \
    qrl.qrc

TRANSLATIONS = RexuizLauncher_ru.ts

LIBS += -lmbedcrypto

win32:RC_ICONS += icon.ico
