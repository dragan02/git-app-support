/**
 * @file main.cpp
 * @brief Main functions.
 *
 * File which represents GUI functionality.
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 04/2021] Initial version
 */

/* Default includes */
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    /* GUI application's control flow */
    QApplication a(argc, argv);

    /* Show main window */
    MainWindow w;
    w.show();

    /* Execute application */
    return a.exec();
}
