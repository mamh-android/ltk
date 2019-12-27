#include "shareboard.h"
#include <iostream>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>

using namespace std;

static QMap<QString, QString> board;
static QStringList Sec;//which stores the user input argument value
static QString OutFileName("out.xml");
static QString TemPFileName;

/*
  according to the usage to parser command, classic code to parser the -start <start> -end <end> arguments.
  it does NOT care about the input argument sequence.
  */
int parser_cmd(int argc, char **argv)
{
    int oc;
    int i = 1;
    bool found=false;


    while (i < argc) {
        for(int j=0;j<Sec.size();j++){
            if ((i<argc)&&(strcmp(argv[i], qPrintable(QString(Sec.at(j))))) == 0) {
                QString key=argv[i];
                QString value=argv[i+1];
                board.insert(key,value);
                qDebug()<<__LINE__<<"j="<<j<<" i="<<i<<" key="<<key<<" value="<<value;
                i=i+2;
                found=true;
                break;
            }//end of while            
//            continue;
        }

        if(false==found){
            qDebug()<<__LINE__<<"go through all, found nothing";
            i++;
        }
        else
            qDebug()<<__LINE__<<"go through all, found";
//        break;
    }//end of for

    return 0;
}

/*
  print the usage, it supports share board_id register, it will return the P_ID; unshare board_id needs P_ID;
  */
void usage()
{
    qDebug()<<"usage:"
           <<"\t./ShareBoardRecord -h"<<endl
          <<"\t./ShareBoardRecord -Share <board_id> -StartTime <TimeStamp> -UserName <username> -UserTeam <userteam> -Ipaddr <ipaddr> -Machine <machine>"<<endl
         <<"\t./ShareBoardRecord -Unshare <board_id> -EndTime <TimeStamp> -StartTime <StartTime>"<<endl
           ;


}

int main(int argc, char **argv)
{
	QCoreApplication::addLibraryPath("./");
	qDebug()<<"lib path"<<QCoreApplication::libraryPaths();
    if(argc%2==0){//print the usage() if the num of arguments is even.
        usage();
        return -1;
    }

    //initialize the Sec,could be extended
    Sec<<"-Share";
    Sec<<"-Unshare";
    Sec<<"-StartTime";
    Sec<<"-EndTime";
    Sec<<"-UserName";
    Sec<<"-UserTeam";
    Sec<<"-Ipaddr";
    Sec<<"-Machine";
//    Sec<<"-P_ID";

    shareboard share;
    qDebug()<<__LINE__<<__FILE__<<"database connection="<<share.createConnection();

    //parse the command to change the value of ComMap and CaseVec
    parser_cmd(argc, argv);

    if(board.count("-Share")){//share the board
        if(board.count("-StartTime")&&board.count("-UserName")&&board.count("-Ipaddr"))//user needs input these three items
        {
            QString board_id=board.value("-Share");
            QString startTime=board.value("-StartTime");
            QString userName=board.value("-UserName");
            QString userTeam=board.value("-UserTeam");		
            QString ipaddr=board.value("-Ipaddr");
            QString machine=board.value("-Machine");
            qDebug()<<__LINE__<<__FILE__<<"board_id="<<board_id<<" startTime="<<startTime<<" userName="<<userName<<" ipaddr="<<ipaddr<<" machine="<<machine;
            int ret=share.share(board_id,startTime,userName,userTeam,ipaddr,machine);
            if(ret!=0)
                qDebug()<<__LINE__<<__FILE__<<"there is some error while share the board"<<endl;
            else
                qDebug()<<__LINE__<<__FILE__<<"pass to share the board"<<endl;
            return ret;
        }
        else{
            qDebug()<<__LINE__<<__FILE__<<"for share, user needs input the startTime,username and ipaddr"<<endl;
            return -1;
        }
    }
    else if(board.count("-Unshare")){//unshare the board
        if(board.count("-EndTime")&&board.count("-StartTime"))//user needs input these two items
        {
            QString board_id=board.value("-Unshare");
            QString startTime=board.value("-StartTime");
            QString endTime=board.value("-EndTime");
            qDebug()<<__LINE__<<__FILE__<<"board_id="<<board_id<<" endtime="<<endTime<<" startTime="<<startTime;
            int ret=share.unshare(board_id,startTime,endTime);
            if(ret!=0)
                qDebug()<<__LINE__<<__FILE__<<"there is some error while unshare the board"<<endl;
            else
                qDebug()<<__LINE__<<__FILE__<<"unshare update successful";
            return ret;
        }
        else{
            qDebug()<<__LINE__<<__FILE__<<"for unshare, user needs input the startTime,username and ipaddr"<<endl;
            return -1;
        }
    }
    return 0;
}


