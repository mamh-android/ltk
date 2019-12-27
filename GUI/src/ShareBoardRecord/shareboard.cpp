#include "shareboard.h"
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#ifdef Q_OS_UNIX
#include <mysql/mysql.h>
#endif
#include <QDebug>

QString shareboard::MySQLDataBaseName="ltkMySQL";

shareboard::shareboard(){

}

shareboard::~shareboard(){
    QSqlDatabase dbSQL = QSqlDatabase::database(MySQLDataBaseName);
    QSqlQuery query(dbSQL);
    dbSQL.close();
}

/*
  invoked at the first to create MySQL Database
  */
bool shareboard::createConnection()
{
    QSqlDatabase dbMySQL=QSqlDatabase::addDatabase("QMYSQL",MySQLDataBaseName);
    qDebug()<<__LINE__<<__FILE__<<QSqlDatabase::drivers();
    dbMySQL.setHostName("10.38.34.91");
    //    db.setDatabaseName("users");
    dbMySQL.setDatabaseName("ltk");
    dbMySQL.setUserName("lbzhu");
    dbMySQL.setPassword("123456");
    if(!dbMySQL.open()){
        qDebug()<<"Unable to open database "<<dbMySQL.lastError();
    }else{
        qDebug()<<"Database connection established"<<dbMySQL.connectionName()<<"and ;"
               <<dbMySQL.connectionNames()<<" and isopen="<<dbMySQL.isOpen()
              <<"and connection Name="<<dbMySQL.connectionName()
             <<"and connecOptions="<<dbMySQL.connectOptions()
               ;
    }
#ifdef Q_OS_UNIX
    QVariant v=dbMySQL.driver()->handle();
    if(qstrcmp(v.typeName(),"MYSQL*")==0){
        MYSQL *handle=*static_cast<MYSQL **>(v.data());
        if(handle !=0){
            qDebug()<<"the connector_fd field value"<<handle->connector_fd
                   <<"and the server_status="<<handle->server_status
                  <<"error="<<mysql_error(handle);
            my_bool reconnect=1;
            int mysql_ret=mysql_options(handle,MYSQL_OPT_RECONNECT,&reconnect);
            qDebug()<<__LINE__<<__FILE__<<"mysql_ret="<<mysql_ret<<"error="<<mysql_error(handle);
        }else{
            qDebug()<<"the handle is null";}
    }
#endif

    return true;
}

int shareboard::addNewRowSQL(const QString board_id, const QString startTime, const QString userName, const QString userTeam, const QString ipaddr, const QString machine)
{
    QSqlDatabase dbSQL = QSqlDatabase::database(MySQLDataBaseName);
    QSqlQuery query(dbSQL);
    query.prepare("INSERT INTO Cloud_Share_Info (board_id, StartTime, UserName, UserTeam, Ipaddr,Machine) "
                  "VALUES (:board_id, :StartTime, :UserName, :UserTeam, :Ipaddr,:Machine)");
    query.bindValue(SQLBoard_id,board_id);
    query.bindValue(SQLStartTime, startTime);
    query.bindValue(SQLUserName,userName);
    query.bindValue(SQLUserTeam,userTeam);
    query.bindValue(SQLIpaddr,ipaddr);
    query.bindValue(SQLMachine,machine);

    int tempret=query.exec();
    if(false==tempret){
        qDebug()<<__LINE__<<__FILE__<<"Fail to insert record"<<" ID="<<board_id;        
    }
    return tempret;
}

int shareboard::share(const QString &board_id,  const QString &startTime, const QString &userName, const QString &userTeam, const QString &ipaddr, const QString &machine)
{


    int P_ID;

    QSqlDatabase dbSQL = QSqlDatabase::database(MySQLDataBaseName);
    QSqlQuery query(dbSQL);
    QString tempSQLCommand="select P_ID from  Cloud_Share_Info where board_id='";
    tempSQLCommand+=board_id;
    tempSQLCommand+="' and StartTime='";
    tempSQLCommand+=startTime;
    tempSQLCommand+="';";
    query.exec(tempSQLCommand);


    //check if this record has been existed
    QSqlRecord temprec = query.record();
    int nameCol = temprec.indexOf("P_ID"); // index of the field "name"
    while (query.next()){
        P_ID=query.value(nameCol).toInt();
        qDebug() <<__LINE__<< __FILE__<<"it is an issue, the board,startTime are existed"<<endl;
        return -1;
    }

    //insert it into table
    int ret=addNewRowSQL(board_id,startTime,userName,userTeam,ipaddr,machine);
    qDebug() <<__LINE__<< "ret="<<ret;

    //get the auto generated P_ID
    query.exec(tempSQLCommand);
    temprec = query.record();
    qDebug() <<__LINE__<< "after add,valid="<<query.isValid()<<" command="<<tempSQLCommand;
    while (query.next()){
         P_ID=query.value(nameCol).toInt();
         break;
    }
    return !ret;
}

/*
  do below update task, i.e.:
    update Cloud_Share_Info set EndTime='endTime' where board_id='board_id' and startTime='startTime';
  */
int shareboard::unshare(const QString &board_id, const QString &startTime, const QString &endTime)
{



    QSqlDatabase dbSQL = QSqlDatabase::database(MySQLDataBaseName);
    QSqlQuery query(dbSQL);
    QString tempSQLCommand="update Cloud_Share_Info set EndTime='";
    tempSQLCommand+=endTime;
    tempSQLCommand+="' where board_id='";
    tempSQLCommand+=board_id;
    tempSQLCommand+="' and StartTime='";
    tempSQLCommand+=startTime;
    tempSQLCommand+="';";
    int ret=query.exec(tempSQLCommand);

    if(false==ret){
        qDebug()<<__LINE__<<__FILE__<<"Fail to unshare record";
    }else
        qDebug()<<__LINE__<<__FILE__<<"pass to unshare record";
    return !ret;
}
