QT += quick

CONFIG += c++17
CONFIG +=qtquickcompiler

#CONFIG += qmltypes
#QML_IMPORT_NAME = Tetris
#QML_IMPORT_MAJOR_VERSION = 1

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        keystate.cpp \
        main.cpp \
        tetris.cpp \
        tetrisBase.cpp \
        tetrisBot.cpp

RESOURCES += \
    qml.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    keystate.h \
    tetris.h \
    tetrisBase.h \
    tetrisBot.h

QT += concurrent

#INCLUDEPATH += D:/VLD/include
#DEPENDPATH += D:/VLD/include
#LIBS += -LD:/VLD/lib/Win64 -lvld
