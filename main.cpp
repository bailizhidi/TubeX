#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("TubeForm CAE - Main Window (Model-Based Tree)");
    w.show();
    return a.exec();
}
