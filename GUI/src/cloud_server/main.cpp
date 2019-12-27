#include <QtGui/QApplication>
#include "dialog.h"
#include <QDebug>
#include "mainwindow.h"
#include <QSettings>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;


    QSettings settings("cloud_server_config.ini",QSettings::IniFormat);
    QString window_width=settings.value("window_width").toString();
    QString window_height=settings.value("window_height").toString();
    if ( window_width!="0" && !window_width.isEmpty() && window_height!="0" && !window_height.isEmpty())
    {w.resize(window_width.toInt(),window_height.toInt());}
    qDebug()<<"win_size"<<window_width<<window_height;

    w.show();

   return a.exec();

}
