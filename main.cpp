#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Oscilloscope");
    w.setWindowIcon(QIcon(":/Icon/mainwindow1.png"));
    w.show();

    return a.exec();
}
