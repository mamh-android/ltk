#include <QtCore>
#include <QtNetwork>
#include <iostream>

#include "ftpget.h"

FtpGet::FtpGet(QObject *parent)
    : QObject(parent)
{
    connect(&ftp, SIGNAL(done(bool)), this, SLOT(ftpDone(bool)));
}

bool FtpGet::getFile(const QUrl &url,const QString filename)
{
    if (!url.isValid()) {
        std::cerr << "Error: Invalid URL" << std::endl;
        return false;
    }

    if (url.scheme() != "ftp") {
        std::cerr << "Error: URL must start with 'ftp:'" << std::endl;
        return false;
    }

    if (url.path().isEmpty()) {
        std::cerr << "Error: URL has no path" << std::endl;
        return false;
    }


    //QString localFileName = QFileInfo(url.path()).fileName();
    //if (localFileName.isEmpty())
        //localFileName = "ftpget.out";

	QString localFileName = filename;
    file.setFileName(localFileName);
QFileInfo* file_info= new QFileInfo(file);
if ( file_info->isDir() )
{
QString FileName_ori = QFileInfo(url.path()).fileName();
localFileName=localFileName+"/"+FileName_ori;
}
file.setFileName(localFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        std::cerr << "Error: Cannot write file "
                  << qPrintable(file.fileName()) << ": "
                  << qPrintable(file.errorString()) << std::endl;
        return false;
    }

    ftp.connectToHost(url.host(), url.port(21));
    ftp.login();
    ftp.get(url.path(), &file);
    ftp.close();
    return true;
}

void FtpGet::ftpDone(bool error)
{
    if (error) {
        std::cerr << "Error: " << qPrintable(ftp.errorString())
                  << std::endl;
	exit(1);
    } else {
        std::cerr << "File downloaded as "
                  << qPrintable(file.fileName()) << std::endl;
    }
    file.close();
    emit done();
}
