#include <QApplication>

#include "src/qt_mainwindow.h"
#include "src/npp.h"

#include <iostream>

int main(int argc, char *argv[])
{
    //first figure out which game mode we are playing
    game_mode = 0;

    QApplication app(argc, argv);

    app.setApplicationName("NPP Games");


    MainWindow *main_window = new MainWindow;
    main_window->show();
    return app.exec();
}
