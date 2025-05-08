QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dfa2MiniDfa.cpp \
    main.cpp \
    mainwindow.cpp \
    nfa2Dfa.cpp \
    postfix2Nfa.cpp \
    regex2Postfix.cpp

HEADERS += \
    dfa2MiniDfa.h \
    mainwindow.h \
    nfa2Dfa.h \
    postfix2Nfa.h \
    regex2Postfix.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    regex2MiniDfa_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    regex2MiniDfa.pro.user
