

#include <QtGui>
#include <QtNetwork>
#include <QFileInfo>
#include <QMainWindow>

#include "ftp.h"

Ftp::Ftp(QWidget *parent)
    :  ftp(0)
{
    progressDialog = new QProgressDialog(NULL);
    uploadfileName = NULL;

    GfileName.clear(); //the whole file name locally including temp/Date/time/case.Goutput
    cdDirname.clear();//the rest need cd dir name, value is upload/ProjectIndex/temp/Date/time
    mkdirFilename.clear();//the new maked dir name

    ftp=NULL;
    file=NULL;
    uploadfileName=NULL;    

    isDirectory.clear();
}

//QString Ftp::FtpServerIP="10.38.36.128";
QString Ftp::FtpServerIP="10.38.164.23";

//![0]
void Ftp::connectOrDisconnect()
{
    if (ftp) {
        ftp->abort();
        ftp->deleteLater();
        ftp = 0;
        //![0]
        return;
    }

    //![1]
    qDebug()<<__LINE__<<__FILE__<<"debug";
    ftp = new QFtp(NULL);
    qDebug()<<__LINE__<<__FILE__<<"debug";
    connect(ftp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));
    connect(ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(updateDataTransferProgress(qint64,qint64)));
    qDebug()<<__LINE__<<__FILE__<<"debug";
    //![1]


    ftp->connectToHost(FtpServerIP);
    qDebug()<<__LINE__<<__FILE__<<"ftp->state="<<ftp->state();
//    cdDirname=GfileName.split("/");
//    if(cdDirname.size())
//        cdDirname.removeLast();//remove the last one which is the file name
//    ftp->login("ftpGui","123456");
    ftp->login();
    qDebug()<<__LINE__<<__FILE__<<"ftp->state="<<ftp->state();

    //![2]
//    statusbar()->setText(tr("Connecting to FTP server %1...")
//                         .arg(FtpServerIP));

}


void Ftp::uploadFile()
{


//    QString GfileName = "/home/lbzhu/good.log";
//    QString setupdir=QFileInfo(GfileName).absoluteDir();
//    qDebug()<<__LINE__<<__FILE__<<"file name="<<GfileName<<" setupdir name="<<setupdir;
//![4]


    progressDialog->setLabelText(tr("Uploading %1...").arg(GfileName));
    progressDialog->exec();
}


//![5]
void Ftp::cancelDownload()
{
    ftp->abort();

    if (file->exists()) {
        file->close();
        file->remove();
    }
    delete file;
}
//![5]



void Ftp::ftpCommandFinished(int commandId, bool error)
{
#ifndef QT_NO_CURSOR
    setCursor(Qt::ArrowCursor);
#endif


    if (ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server "
                                        "at %1. Please check that the host "
                                        "name is correct.")
                                     .arg(FtpServerIP));
            connectOrDisconnect();
            return;
        }
        return;
    }
//![6]

//![7]
    if (ftp->currentCommand() == QFtp::Login)
        ftp->list();
//![7]

    if (ftp->currentCommand() == QFtp::List) {
//        if (isDirectory.isEmpty()) {
//            fileList->addTopLevelItem(new QTreeWidgetItem(QStringList() << tr("<empty>")));
//            fileList->setEnabled(false);
//        }

        if(cdDirname.size()){
            qDebug()<<__LINE__<<__FILE__<<"target dir="<<cdDirname.at(0);
            if(isDirectory.contains(cdDirname.at(0))){
                ftp->cd(cdDirname.at(0));
                cdDirname.removeFirst();
            }else{
                mkdirFilename=cdDirname.at(0);
                ftp->mkdir(cdDirname.at(0));
                cdDirname.removeFirst();
            }
        }else{//we need put the file into server
            uploadfileName = new QFile(GfileName);
            if (!uploadfileName->open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("FTP"),
                                         tr("Unable to open the file %1: %2.")
                                         .arg(GfileName).arg(FtpServerIP));
                delete uploadfileName;
                return;
            }
            qDebug()<<__LINE__<<__FILE__<<"Gfilename="<<GfileName;                        
//            ftp->put(uploadfileName,QFileInfo(GfileName).fileName(),QFtp::Ascii);
            ftp->put(uploadfileName,QFileInfo(GfileName).fileName(),QFtp::Binary);
//            ftp->put(uploadfileName,"ntim_helan2.bin",QFtp::Binary);
//            ftp->put(uploadfileName,"ntim_helan2.bin",QFtp::Ascii);
//            }
            qDebug()<<__LINE__<<__FILE__<<"commandid="<<commandid;
        }
    }else if ((ftp->currentCommand() == QFtp::Put) ){
//        }else if (commandid==commandId){
        progressDialog->hide();
        qDebug()<<__LINE__<<__FILE__<<"put done"<<"error="<<error<<"finish commandid="<<commandId;
        if(!error){
            uploadfileName->close();
        }else{
            qDebug()<<__LINE__<<__FILE__<<"error string="<<ftp->errorString();
        }

    }else if ( ftp->currentCommand() == QFtp::Mkdir){
        qDebug()<<__LINE__<<__FILE__<<"Mkdir done"<<"error="<<error;
        if(!error){
            qDebug()<<__LINE__<<__FILE__<<"go to the dir"<<mkdirFilename;
            ftp->cd(mkdirFilename);
        }else{
            qDebug()<<__LINE__<<__FILE__<<"error string="<<ftp->errorString();
        }
    }else if( ftp->currentCommand() == QFtp::Cd){
        qDebug()<<__LINE__<<__FILE__<<"CD done"<<"error="<<error;
        if(!error){
            qDebug()<<__LINE__<<__FILE__<<"go to the dir";
            isDirectory.clear();
            ftp->list();
        }else{
        qDebug()<<__LINE__<<__FILE__<<"error string="<<ftp->errorString();
        }

    }else if(ftp->currentCommand() == QFtp::Close)
    {
        qDebug()<<__LINE__<<__FILE__<<"close ftp";
    }
//![9]
}


//![10]
void Ftp::addToList(const QUrlInfo &urlInfo)
{
//    qDebug()<<__LINE__<<__FILE__<<"Debug";

    isDirectory[urlInfo.name()] = urlInfo.isDir();
    qDebug()<<__LINE__<<__FILE__<<"urlInfo.name="<<urlInfo.name()<<" isdir="<<urlInfo.isDir();

}

//![13]
void Ftp::updateDataTransferProgress(qint64 readBytes,
                                           qint64 totalBytes)
{
    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(readBytes);
}
//![13]

QFtp::State Ftp::state()
{
    return ftp->state();

}

Ftp::~Ftp(){

    if(progressDialog!=NULL){
        delete progressDialog;
        progressDialog=NULL;
    }
    if(ftp!=NULL){
        delete ftp;
        ftp=NULL;
    }
    if(uploadfileName!=NULL){
        delete uploadfileName;
        uploadfileName=NULL;
    }
}


void Ftp::closeFtp()
{
    ftp->close();
    return;
}


