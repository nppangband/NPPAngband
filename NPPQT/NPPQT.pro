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
            src/cmd4.cpp\
            src/globals.cpp \
            src/init_edit_files.cpp\
            src/init_game.cpp\
            src/randart.cpp \
            src/tables.cpp \
            src/utilities.cpp \
            src/random_numbers.cpp \
            src/mon_ranged_attacks.cpp \
            src/calcs.cpp \
            src/obj-util.cpp





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
            src/function_declarations.h


RESOURCES += \
    NPP_Resources.qrc
