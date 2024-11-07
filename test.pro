# include( $${PWD}/examples.pri )

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
TARGET = test

SOURCES += \
    CustomFilterProxyModel.cpp \
    CustomQTreeWidget.cpp \
    CustomScaleDraw.cpp \
    Filechangemanager.cpp \
    Filedialog.cpp \
    ImportDialog.cpp \
    Plot.cpp \
    ProcessLine.cpp \
    TimeRangeDialog.cpp \
    TimeSeries.cpp \
    Tipperdialog.cpp \
    copyworker.cpp \
    customtabbar.cpp \
    customtabwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    progressdialog.cpp

HEADERS += \
    CustomFilterProxyModel.h \
    CustomQTreeWidget.h \
    CustomScaleDraw.h \
    Filechangemanager.h \
    Filedialog.h \
    ImportDialog.h \
    Plot.h \
    ProcessLine.h \
    TimeRangeDialog.h \
    TimeSeries.h \
    Tipperdialog.h \
    copyworker.h \
    customtabbar.h \
    customtabwidget.h \
    mainwindow.h \
    progressdialog.h

FORMS += \
    mainwindow.ui


# Default rules for depoyment.

RESOURCES += \
    resources.qrc

DISTFILES += \
        bins/processLine.exe

OTHER_FILES += \
        processLine.exe


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

message($$PWD)



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../QT_free/6.7.2/mingw_64/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../QT_free/6.7.2/mingw_64/lib/ -lqwtd
else:unix: LIBS += -L$$PWD/../../QT_free/6.7.2/mingw_64/lib/ -lqwt

INCLUDEPATH += $$PWD/../../QT_free/6.7.2/mingw_64/include/Qwt
DEPENDPATH += $$PWD/../../QT_free/6.7.2/mingw_64/include/Qwt



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../QT_free/6.7.2/mingw_64/lib/ -lqwtd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../QT_free/6.7.2/mingw_64/lib/ -lqwtd
else:unix: LIBS += -L$$PWD/../../QT_free/6.7.2/mingw_64/ -lqwtd

INCLUDEPATH += $$PWD/../../QT_free/6.7.2/mingw_64/include/Qwt
DEPENDPATH += $$PWD/../../QT_free/6.7.2/mingw_64/include/Qwt




