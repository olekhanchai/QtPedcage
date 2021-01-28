#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setCursor(QCursor(Qt::BlankCursor));
    return a.exec();
}
