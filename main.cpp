#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int portNo = 0;
    if( argc>1 )
    {
        QString portNoStr = QString(argv[1]);
        portNo = portNoStr.toInt();
    }

    MainWindow w(portNo);
    w.show();
    return a.exec();
}
