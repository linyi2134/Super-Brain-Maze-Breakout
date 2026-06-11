QT       += core gui widgets \
    quick

CONFIG   += c++17

TARGET = MazeBreakout
TEMPLATE = app

SOURCES += \
    boardsizedialog.cpp \
    customboardeditor.cpp \
    customboardselector.cpp \
    main.cpp \
    menu.cpp \
    modeselection.cpp \
    replaywidget.cpp \
    savedialog.cpp \
    singleplayermode.cpp \
    gamewidget.cpp \
    gamecontroller.cpp \
    board.cpp \
    player.cpp \
    chess.cpp \
    challengequestion.cpp

HEADERS += \
    boardsizedialog.h \
    customboardeditor.h \
    customboardselector.h \
    gamedata.h \
    menu.h \
    modeselection.h \
    replaywidget.h \
    savedialog.h \
    singleplayermode.h \
    gamewidget.h \
    gamecontroller.h \
    board.h \
    player.h \
    chess.h \
    challengequestion.h

FORMS += \
    menu.ui \
    modeselection.ui \
    singleplayermode.ui