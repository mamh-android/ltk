#include <QtCore>
#include <iostream>

#include "ftpget.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();

    if (args.count() != 3) {
        std::cerr << "Usage: ftpget url" << std::endl
                  << "Example:" << std::endl
                  << "    ftpget ftp://ftp.trolltech.com/mirrors /home/wsun/setup.exe"
                  << std::endl;
        return 1;
    }

    FtpGet getter;
    if (!getter.getFile(QUrl(args[1]),args[2]))
//if (!getter.getFile(QUrl("ftp://10.38.164.23/Daily_Build/Build_History/build_log_2013-10-08/pxa988dkb-jb4.2_beta3/setup.exe")))
      return 1;

    QObject::connect(&getter, SIGNAL(done()), &app, SLOT(quit()));

    return app.exec();
}
