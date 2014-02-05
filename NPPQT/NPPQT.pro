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
            src/dun_effect.cpp \
            src/dun_generate.cpp \
            src/dungeon.cpp \
            src/cmd4.cpp \
            src/cmd_spell.cpp \
            src/dun_cave.cpp \
            src/dun_classes.cpp \
            src/dun_feature.cpp \
            src/dun_util.cpp \
            src/globals.cpp \
            src/init_edit_files.cpp\
            src/init_game.cpp\
            src/load.cpp \
            src/mon_cast.cpp \
            src/mon_classes.cpp \
            src/mon_damage.cpp \
            src/mon_player_ghost.cpp \
            src/mon_ranged_attacks.cpp \
            src/mon_util.cpp \
            src/object_classes.cpp \
            src/object_desc.cpp \
            src/object_hooks.cpp \
            src/object_make.cpp \
            src/object_util.cpp \
            src/optionsdialog.cpp \
            src/player_classes.cpp \
            src/player_spell.cpp \
            src/player_util.cpp \
            src/project.cpp \
            src/project_util.cpp \
            src/quest.cpp \
            src/randart.cpp \
            src/random_numbers.cpp \
            src/save.cpp \
            src/spells2.cpp \
            src/squelch.cpp \
            src/store.cpp \
            src/tables.cpp \
            src/target.cpp \
            src/timed.cpp \
            src/utilities.cpp \
    src/birthdialog.cpp \
    src/birth.cpp



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
            src/utilities.h \
            src/object_classes.h \
            src/squelch.h \
            src/randart.h \
            src/dun_classes.h \
            src/dun_generate.h \
            src/optionsdialog.h \
            src/mon_classes.h \
            src/player_classes.h \
    src/birthdialog.h


RESOURCES += \
    NPP_Resources.qrc

FORMS += \
    src/optionsdialog.ui \
    src/birthdialog.ui
