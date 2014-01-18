#-------------------------------------------------
#
# Project created by QtCreator 2013-12-25T17:24:56
#
#-------------------------------------------------

QT       += core gui

QT += widgets

TARGET = NPPGAMES


SOURCES +=  src/qt_main.cpp\
            src/qt_mainwindow.cpp\
            src/calcs.cpp \
            src/cave.cpp \
            src/cmd4.cpp\
            src/effect.cpp \
            src/feature.cpp \
            src/generate.cpp \
            src/globals.cpp \
            src/init_edit_files.cpp\
            src/init_game.cpp\
            src/load.cpp \
            src/mon_ranged_attacks.cpp \
            src/object-util.cpp \
            src/player_ghost.cpp \
            src/randart.cpp \
            src/random_numbers.cpp \
            src/spells2.cpp \
            src/squelch.cpp \
            src/tables.cpp \
            src/utilities.cpp \
    src/save.cpp



HEADERS  += src/qt_mainwindow.h\
            src/npp.h\
            src/defines.h\
            src/globals.h \
            src/init.h \
            src/monster.h \
            src/object.h \
            src/player.h \
            src/store.h \
            src/structures.h \
            src/terrain.h \
            src/random_numbers.h \
            src/function_declarations.h \
            src/loadsave.h \
            src/user_macros.h \
    src/utilities.h


RESOURCES += \
    NPP_Resources.qrc
