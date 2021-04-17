QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets designer

CONFIG += c++11 plugin release

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# For shared memory operations
LIBS += -lrt

SOURCES += \
    LED.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    LED.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
