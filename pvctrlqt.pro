QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialbus serialport

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    basedataunitwidget.cpp \
    cmodbuschannel.cpp \
    cmodbuscontroller.cpp \
    main.cpp \
    mainwindow.cpp \
    chart.cpp \
    mymodbusfunc.cpp \
    slaveconfdialog.cpp \
#    slaveinfoframe.cpp \
    slaveinfowidget.cpp \
    voltagechartform.cpp

HEADERS += \
    basedataunitwidget.h \
    cmodbuschannel.h \
    cmodbuscontroller.h \
    mainwindow.h \
    chart.h \
    mymodbusfunc.h \
    slaveconfdialog.h \
#    slaveinfoframe.h \
    slaveinfowidget.h \
    voltagechartform.h

FORMS += \
    mainwindow.ui \
    slaveconfdialog.ui \
#    slaveinfoframe.ui \
    slaveinfowidget.ui \
    voltagechartform.ui

TRANSLATIONS += \
    pvctrlqt_zh_CN.ts


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix {
    SOURCES += rtumaster.cpp
    HEADERS += rtumaster.h
    INCLUDEPATH += /usr/local/include/modbus
    LIBS += -L/usr/local/lib -lmodbus
}

RESOURCES += \
    pvctrlqt.qrc
