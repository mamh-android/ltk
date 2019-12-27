#include "dialog.h"
#include "ui_dialog.h"
#include <QFile>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QDebug>
#include <QCloseEvent>
#include <QDateTime>
#include <QTimer>
#include <iostream>
#include <QMessageBox>
#include <QDesktopWidget>
#include "ftp.h"
using namespace std;

QString Dialog::tempDirDateTime =QDateTime::currentDateTime().toString(Qt::ISODate).replace(QRegExp("[-:]"),"_")+"tmp";
QString Dialog::blf_folder =QDateTime::currentDateTime().toString(Qt::ISODate).replace(QRegExp("[-:]"),"_")+"tmp_blf";
QString Dialog::AllBlfTextPreImage=QString();
QString Dialog::AllBlfTextFromImage=QString();

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    QDesktopWidget * desktopWidget = QApplication::desktop();
    QRect deskRect = desktopWidget->availableGeometry();
    QRect temp(deskRect);
    //QRect screenRect = desktopWidget->screenGeometry();
    if(desktopWidget->screenCount() > 1)
    {
        QRect desk2Rect = desktopWidget->availableGeometry(1);
        temp.setWidth(deskRect.width() < desk2Rect.width() ? deskRect.width(): desk2Rect.width());
        temp.setHeight(deskRect.height() < desk2Rect.height() ? deskRect.height(): desk2Rect.height());
    }

    deskRect = temp;
    resize(QSize(deskRect.width()/2,deskRect.height()/2));
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(autoupload()));
    connect(ui->checkBox,SIGNAL(clicked(bool)),this,SLOT(appendImageText(bool)));
    ui->pushButton->setDisabled(true);
//    ui->tabWidget->setTabPosition();
    ui->tabWidget->setCurrentIndex(0);//set current focus to the image update tab

    blfFileChange=false;
    blfFileName=QString();
}

Dialog::~Dialog()
{
    FileTemp.close();
    delete ui;
}

// wget --http-user=lbzhu --http-passwd=welcome2AMD77
//http://10.38.116.40/autobuild/android/pxa988/2014-05-22_pxa988-kk4.4/README
//    QString fileTemp("/tmp/temp.blf");
bool Dialog::ParseFile(const QString &fileName)
{

    if(FileTemp.open())
    {
        qDebug()<<__LINE__<<__FILE__<<"pass: tempfile";
    }else{
        qDebug()<<__LINE__<<__FILE__<<"failure: tempfile";
        return false;
    }


    qDebug()<<__LINE__<<__FILE__<<"filename="<<FileTemp.fileName();
    QString command("wget --http-user=wsun --http-passwd=marvell789 ");
    command+=fileName;
    command+=" -v -O ";
    command+=FileTemp.fileName();


    int ret;
    if(ret=system(command.toLocal8Bit().constData()))
    {
        qDebug()<<__LINE__<<__FILE__<<"failure:"<<command;
        return false;
    }else{
        QString temp=fileName;
        temp.remove(0,temp.lastIndexOf("/")+1);
        temp.remove(0,temp.lastIndexOf("\\")+1);
        blfFileName=temp;
        qDebug()<<__LINE__<<__FILE__<<"pass:"<<command<<" blfFileName="<<blfFileName;
    }
    ui->tableWidget->clear();
    qDebug()<<__LINE__<<__FILE__<<" Debug:,ret="<<SubParseFile(FileTemp);

    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->setVisible(true);
    //    ui->tableWidget->itemAt(0,0)->setFlags(0);
    //    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
    connect( ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(SlotupdateCellDoubleClicked(int,int)));
    FileTemp.seek(0);
//    ui->textEdit->setText(FileTemp.readAll());
    QString temp=FileTemp.readAll();
    QString keyImageMap="Image_Maps";
    int index=temp.indexOf(keyImageMap,0);
    qDebug()<<__LINE__<<__FILE__<<" find the key image map="<<index;
    AllBlfTextPreImage=temp.left(index);
    ui->textEdit->setText(AllBlfTextPreImage);
    AllBlfTextFromImage=temp.right(temp.size()-index);
    ui->textEdit_2->setText(AllBlfTextFromImage);
    ui->textEdit_2->hide();
    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(blfFileUpdate()));
    connect(ui->textEdit_2,SIGNAL(textChanged()),this,SLOT(blfFileUpdate()));
    return true;

}

bool Dialog::SubParseFile(QFile &fileTemp)
{
    const static QString KeyImage_Path="_Image_Path";
    const static QString KeyImage_ID="_Image_ID_Name";
    QVector<QString> PathVec;
    QVector<QString> IDVec;

    QString line=fileTemp.readLine();
    qDebug()<<__LINE__<<__FILE__<<"line="<<line;
    while(line.size())
    {

        qDebug()<<__LINE__<<__FILE__<<" line=:"<<line;
        line=fileTemp.readLine();
        line.remove("\n");
        //push_back the KeyImage_Path
        if(line.indexOf(KeyImage_Path)!=-1)
        {
            //                        qDebug()<<__LINE__<<__FILE__<<" match line="<<line;
            QStringList values=line.split("=");
            if(values.size()!=2)
            {
                //                qDebug()<<__LINE__<<__FILE__<<" the line value is bad, ="<<line;
                return false;
            }
            qDebug()<<__LINE__<<__FILE__<<"value="<<values.at(1);
            QString temp=values.at(1);
            temp.remove(0,temp.lastIndexOf("/")+1);
            temp.remove(0,temp.lastIndexOf("\\")+1);
            //            PathVec.push_back(values.at(1));
            PathVec.push_back(temp);
            continue;
        }
        else if(line.indexOf(KeyImage_ID)!=-1)
        {
            QVector<QString> valuesVec;
            QStringList values=line.split("=");
            if(values.size()!=2)
            {
                //                qDebug()<<__LINE__<<__FILE__<<" the line value is bad, ="<<line;
                return false;
            }
            //            qDebug()<<__LINE__<<__FILE__<<"value="<<values.at(1);
            IDVec.push_back(values.at(1));
            continue;
        }else{
            //            qDebug()<<__LINE__<<__FILE__<<"no key match";
            continue;
        }

    }

    qDebug()<<__LINE__<<__FILE__<<"id size="<<IDVec.size()<<" path size="<<PathVec.size();

    if(IDVec.size()!=PathVec.size()){
        qDebug()<<__LINE__<<__FILE__<<"the number of id and path does not match";
        return false;
    }
    ui->tableWidget->setRowCount(IDVec.size());



    for(int i=0;i<IDVec.size();i++)
    {

        QTableWidgetItem *__qtablewidgetitemKey = new QTableWidgetItem();
        QString idTemp=IDVec.at(i);
        __qtablewidgetitemKey->setText(idTemp.remove(' ').remove('\r'));
        ui->tableWidget->setVerticalHeaderItem(i,__qtablewidgetitemKey);

        //        qDebug()<<__LINE__<<__FILE__<<"debug,size="<<valuesVec.size();
        QTableWidgetItem *__qtablewidgetitemValue = new QTableWidgetItem();
        __qtablewidgetitemValue->setFlags(Qt::ItemIsSelectable|Qt::ItemIsDragEnabled|Qt::ItemIsEnabled);
        QString pathTemp=PathVec.at(i);
        //the windows txt where the last code is \r\t, we should do windows to linux change first, may update later
        //currently remove the redunant \r
        __qtablewidgetitemValue->setText(pathTemp.remove(' ').remove('\r'));


        ui->tableWidget->setItem(i, EmuBlfFileName, __qtablewidgetitemValue);

        QTableWidgetItem *__qtablewidgetitemValue2 = new QTableWidgetItem();
        __qtablewidgetitemValue2->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsDragEnabled|Qt::ItemIsEnabled);
        __qtablewidgetitemValue2->setIcon(QIcon(":/image/folder.png"));
        //        __qtablewidgetitemValue2->setIcon(QIcon(":/image/file.png"));
        ui->tableWidget->setItem(i, EmuBlfImgFilePath, __qtablewidgetitemValue2);

    }
    return true;
}

void Dialog::SlotupdateCellDoubleClicked(int row, int column)
{
    qDebug()<<__LINE__<<__FILE__<<"row="<<row<<" column"<<column;
    if(column!=EmuBlfImgFilePath)
        return;
    else{
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Modify File Properties"), ".","");
        if(fileName.size()){
            ui->tableWidget->item(row,column)->setText(fileName);
            ui->pushButton->setEnabled(true);
        }
        qDebug()<<__LINE__<<__FILE__<<"fileName"<<fileName;
    }
}

void Dialog::appendImageText(bool temp)//show or hide the image update part
{

        qDebug()<<__LINE__<<__FILE__<<"enable is ="<<temp;
        if(true==temp){
            ui->textEdit_2->show();
        }else{
            ui->textEdit_2->hide();
        }
}

void Dialog::blfFileUpdate()
{
    ui->pushButton->setEnabled(true);
    blfFileChange=true;
    ImageConfig2.insert("blf_modified","yes");
    qDebug()<<__LINE__<<__FILE__<<"text is changed";
}

void Dialog::autoupload()
{
    int row=ui->tableWidget->rowCount();
    int column=ui->tableWidget->columnCount();
    qDebug()<<__LINE__<<__FILE__<<"row="<<row<<" column="<<column;
    QString cmd="mkdir ";
    cmd+=tempDirDateTime;

    qDebug()<<__LINE__<<__FILE__<<"ret="<<system(cmd.toLocal8Bit().constData());

    QStringList   filenameList;//which merge all changed file into one string;

    bool localimage=false;
    QString path2("Daily_Build/Blf_Update");

    for(int i=0;i<row;i++){
        if(ui->tableWidget->item(i,1)->text().size())
        {
            QString fileName=ui->tableWidget->item(i,1)->text();
            //            QFile file(fileName);
            QString rawfileName=ui->tableWidget->item(i,0)->text();
            QString newName=tempDirDateTime+QDir::separator()+ui->tableWidget->item(i,0)->text();

            QStringList filename_name=fileName.split("/");

            QString fileName_ori=tempDirDateTime+QDir::separator()+filename_name.at(filename_name.count()-1)+"_ori";
            //            QString newName=tempDirDateTime+QDir::separator()+"Makefile";
            qDebug()<<__LINE__<<__FILE__<<"fileName="<<fileName<<" newName="<<newName<<" fileName_ori="<<fileName_ori;
            qDebug()<<__LINE__<<__FILE__<<"ret of copy="<<QFile::copy(fileName,newName);
            qDebug()<<__LINE__<<__FILE__<<"ret of copy="<<QFile::copy(fileName,fileName_ori);
            //            QFile file(fileName);
            //            file.open(QFile::ReadOnly);

            Ftp temp;
            qDebug()<<__LINE__<<__FILE__<<"debug";
            temp.setGfileName(newName);//step 1: which is the related filename include related Dir.
            qDebug()<<__LINE__<<__FILE__<<path2+QDir::separator()+newName;
            QStringList cdDirname=(path2+QDir::separator()+newName).split(QDir::separator());
            if(cdDirname.size())
                cdDirname.removeLast();//remove the last one which is the file name
            temp.setcdDirname(cdDirname);//step 2: set the dir where FTP should go to, it will mkdir if there is no related dir
            qDebug()<<__LINE__<<__FILE__<<"debug,";

            temp.connectOrDisconnect(); //step 3: connect the FTP Server and go to the correct dir;
            qDebug()<<__LINE__<<__FILE__<<"debug";
            temp.uploadFile();//step 4: upload log file into FTP Server
            qDebug()<<__LINE__<<__FILE__<<"ftp upload done";
            localimage=true;
            temp.closeFtp();

            qDebug()<<__LINE__<<__FILE__<<temp.state();
            int times=0,timeout=10;
            while((temp.state() != QFtp::Unconnected) && (times <= timeout))
            {
                times++;
                QEventLoop eventloop;
                QTimer::singleShot(1000,&eventloop,SLOT(quit()));
                eventloop.exec();
                qDebug()<<__LINE__<<__FILE__<<"ftp has Not finished,times="<<times;
            }


            Ftp temp_ori;
            qDebug()<<__LINE__<<__FILE__<<"debug";
            temp_ori.setGfileName(fileName_ori);//step 1: which is the related filename include related Dir.
            QStringList cdDirname_ori=(path2+QDir::separator()+fileName_ori).split(QDir::separator());
            if(cdDirname_ori.size())
                cdDirname_ori.removeLast();//remove the last one which is the file name
            temp_ori.setcdDirname(cdDirname_ori);//step 2: set the dir where FTP should go to, it will mkdir if there is no related dir
            qDebug()<<__LINE__<<__FILE__<<"debug,";

            temp_ori.connectOrDisconnect(); //step 3: connect the FTP Server and go to the correct dir;
            qDebug()<<__LINE__<<__FILE__<<"debug";
            temp_ori.uploadFile();//step 4: upload log file into FTP Server
            qDebug()<<__LINE__<<__FILE__<<"ftp upload done";
            localimage=true;
            temp_ori.closeFtp();

            qDebug()<<__LINE__<<__FILE__<<temp_ori.state();
            times=0,timeout=10;
            while((temp_ori.state() != QFtp::Unconnected) && (times <= timeout))
            {
                times++;
                QEventLoop eventloop;
                QTimer::singleShot(1000,&eventloop,SLOT(quit()));
                eventloop.exec();
                qDebug()<<__LINE__<<__FILE__<<"ftp has Not finished,times="<<times;
            }



            qDebug()<<__LINE__<<__FILE__<<temp.state();
            filenameList+=rawfileName;
            //            file.close();
        }
    }

    if(true==blfFileChange){//add blf file update if necessnary
            qDebug()<<__LINE__<<__FILE__<<"blf file needs update too"<<endl;
            QString newName=tempDirDateTime+QDir::separator()+blfFileName;
            QString newName_blf=blf_folder+QDir::separator()+blfFileName;
            QFile tempFile(newName);
            if(tempFile.open(QIODevice::WriteOnly | QIODevice::Text ))
            {
                qDebug()<<__LINE__<<__FILE__<<"pass: tempFile";
            }else{
                qDebug()<<__LINE__<<__FILE__<<"failure: open tempFile";
                return ;
            }

            QString alltext=ui->textEdit->toPlainText();

            QTextStream outTemp(&tempFile);
            outTemp<<alltext;
            QString alltext2=ui->textEdit_2->toPlainText();
            outTemp<<alltext2;

            if(outTemp.status()!=QTextStream::Ok){
                qDebug()<<__LINE__<<__FILE__<<"fail to write to file"<<tempFile.fileName()<<" status="<<outTemp.status();
                qWarning()<<__LINE__<<__FILE__<<"fail to write to file"<<tempFile.fileName()<<" status="<<outTemp.status();;
            }else{
                qDebug()<<__LINE__<<__FILE__<<"PASS to write to file"<<tempFile.fileName()<<" status="<<outTemp.status();
            }
            tempFile.close();


            Ftp temp;
            qDebug()<<__LINE__<<__FILE__<<"debug";
            temp.setGfileName(newName);//step 1: which is the related filename include related Dir.
            qDebug()<<__LINE__<<__FILE__<<path2+QDir::separator()+newName;
            QStringList cdDirname=(path2+QDir::separator()+newName_blf).split(QDir::separator());
            if(cdDirname.size())
                cdDirname.removeLast();//remove the last one which is the file name
            temp.setcdDirname(cdDirname);//step 2: set the dir where FTP should go to, it will mkdir if there is no related dir
            qDebug()<<__LINE__<<__FILE__<<"debug,";

            temp.connectOrDisconnect(); //step 3: connect the FTP Server and go to the correct dir;
            qDebug()<<__LINE__<<__FILE__<<"debug";
            temp.uploadFile();//step 4: upload log file into FTP Server
            qDebug()<<__LINE__<<__FILE__<<"ftp upload done";
            //localimage=true;
            temp.closeFtp();

            qDebug()<<__LINE__<<__FILE__<<temp.state();
            int times=0,timeout=10;
            while((temp.state() != QFtp::Unconnected) && (times <= timeout))
            {
                times++;
                QEventLoop eventloop;
                QTimer::singleShot(1000,&eventloop,SLOT(quit()));
                eventloop.exec();
                qDebug()<<__LINE__<<__FILE__<<"ftp has Not finished,times="<<times;
            }
            qDebug()<<__LINE__<<__FILE__<<temp.state();
            QString blf_modified_path("Daily_Build/Blf_Update");
            ImageConfig2.insert("blf_modified_path",blf_modified_path+QDir::separator()+blf_folder);

            //filenameList+=blfFileName;
    }
    //if((true==localimage) ||(true==blfFileChange) ){
      if((true==localimage) ){
        qDebug()<<__LINE__<<__FILE__<<"map update done";
        ImageConfig2.insert("modified","yes");
        ImageConfig2.insert("location2",Ftp::FtpServerIP);
        ImageConfig2.insert("path2",path2+QDir::separator()+tempDirDateTime);
        QString filename=filenameList.join(";");

        ImageConfig2.insert("image2",filename);

        QString cmd="rm -rf ";
        cmd+=tempDirDateTime;
        qDebug()<<__LINE__<<__FILE__<<"remove template dir ret="<<system(cmd.toLocal8Bit().constData());
    }else{
        qDebug()<<__LINE__<<__FILE__<<"map do NOT need update";
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    qDebug()<<__LINE__<<__FILE__<<"close event";
    event->accept();

    //    int r=QMessageBox::warning(this,tr("BSP Test Kit"),tr("do you want to save the current setting?"),
    //                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    //    if(r == QMessageBox::Yes){
    //        qDebug()<<__LINE__<<__FILE__<<"yes";
    //        autoupload();
    //        event->accept();
    //    }else if(r==QMessageBox::No){
    //        qDebug()<<__LINE__<<__FILE__<<"no";
    //        event->accept();
    //    }else if(r==QMessageBox::Cancel){
    //        qDebug()<<__LINE__<<__FILE__<<"cancel";
    //        event->ignore();
    //    }


}


