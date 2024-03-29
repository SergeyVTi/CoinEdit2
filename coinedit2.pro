TEMPLATE = app
QT       += core gui widgets printsupport

greaterThan(QT_MAJOR_VERSION, 5.15): QT += statemachine core5compat

CONFIG += c++17 static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    celldialog.cpp \
    coin.cpp \
    coinedit.cpp \
    graphics.cpp \
    main.cpp \
    texteditor.cpp

HEADERS += \
    celldialog.h \
    coin.h \
    coinedit.h \
    graphics.h \
    texteditor.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc \

DISTFILES += \
    CMakeLists.txt
