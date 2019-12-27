#include <iostream>
#include <QtDebug>
#include "xmldomreader.h"

using namespace std;

static QMap<QString, QMap<QString, QString> > ComMap;//the user will modify the field if necessnary
static QString Pri("CTKJob");
static QStringList Sec;
static QString Tri("testcase");
static QVector<QMap<QString, QString> > CaseVec;//the user will add test case node each time
static QString OutFileName("out.xml");
static QString TemPFileName;

/*
  according to the usage to parser command, there are two types of key field
  one is CaseList, which can include nested testcase /testcase
  the other only include node.
  */
int parser_cmd(int argc, char **argv)
{
    int oc;
    int i = 0;

    while (i < argc) {
        for(int j=0;j<Sec.size();j++){
//            qDebug()<<__LINE__<<"j="<<j<<" value="<<Sec.at(j);
            if (strcmp(argv[i], qPrintable(QString(Sec.at(j)))) == 0) {
//                qDebug()<<__LINE__<<"j="<<j<<" value="<<Sec.at(j)<<" argv="<<argv[i];
                if(strcmp(argv[i],"CaseList")==0){//special case
                    i++;
                    while(strcmp(argv[i],qPrintable(QString("/CaseList")))){//check the end of /CaseList
                        QMap<QString, QString> tempTriMap;

                        if(strcmp(argv[i],qPrintable(Tri))==0){//if it equals Tri
                            i++;
                            while(strcmp(argv[i],qPrintable(QString("/")+Tri))){//end of Tri
                                QString key=argv[i];
                                QString value=argv[i+1];
                                tempTriMap.insert(key,value);
//                                qDebug()<<__LINE__<<"j="<<j<<" i="<<i<<" key="<<key<<" value="<<value;
                                i=i+2;
                            }//end of while
                            CaseVec.push_back(tempTriMap);
                        }
                        i++;
                    }
                }else{
                    QMap<QString, QString> tempSecMap;
                    i++;
                    while(strcmp(argv[i],qPrintable(QString(QString("/")+Sec.at(j))))){
                        QString key=argv[i];
                        QString value=argv[i+1];
                        tempSecMap.insert(key,value);
//                        qDebug()<<__LINE__<<"j="<<j<<" i="<<i<<" key="<<key<<" value="<<value;
                        i=i+2;
                    }//end of while
                    ComMap.insert(Sec.at(j),tempSecMap);
                    continue;
                }
            }
        }//end of for

        if(strcmp(argv[i],"-f")==0){//output file name
            OutFileName=argv[i+1];
            i=i+1;
        }else if(strcmp(argv[i],"-t")==0){//job template file name
            TemPFileName=argv[i+1];
            i=i+1;
        }
        i++;
    }

    return 0;
}

/*
  print the context of iMap and iVec according to the struct
  */
void print(const QMap<QString, QMap<QString, QString> > iMap,const QVector<QMap<QString, QString> > iVec)
{
    qDebug()<<__LINE__<<"before print iMap, keys="<<iMap.keys();
    QStringList keys=iMap.keys();
    for(int i=0;i<keys.size();i++)
    {
//        qDebug()<<__LINE__<<"before, key="<<keys.at(i);
        QMap<QString, QString> innerMap=iMap.value(keys.at(i));
        QStringList ikeys=innerMap.keys();
        for(int j=0;j<ikeys.size();j++){
            qDebug()<<__LINE__<<"\t key="<<ikeys.at(j)<<" value="<<innerMap.value(ikeys.at(j));
        }
    }

    qDebug()<<__LINE__<<"before print iVec";
    for(int i=0;i<iVec.size();i++)
    {
        QMap<QString, QString> innerMap=iVec.at(i);
        QStringList ikeys=innerMap.keys();
//        qDebug()<<__LINE__<<"before, ikey="<<ikeys<<" i="<<i;

        for(int j=0;j<ikeys.size();j++){
            qDebug()<<__LINE__<<"\t key="<<ikeys.at(j)<<" value="<<innerMap.value(ikeys.at(j));
        }
    }
    qDebug()<<__LINE__<<"after print";
}

/*
  merge two Map
  insert the value from srcMap into desMap
  */
void merge(const QMap<QString, QMap<QString, QString> > &srcMap, QMap<QString, QMap<QString, QString> > &desMap)
{
    for(int i=0;i<Sec.size();i++)
    {

        QMap<QString, QString> tempMap;
        tempMap=srcMap.value(Sec.at(i));
        for(int j=0;j<tempMap.size();j++){
            QStringList keys=tempMap.keys();
            for(int k=0;k<keys.size();k++){
                QMap<QString, QString> ktempMap=desMap.value(Sec.at(i));
                ktempMap.insert(keys.at(k),tempMap.value(keys.at(k)));
                //Inserts a new item with the key key and a value of value.
                //If there is already an item with the key key, that item's value is replaced with value.
                desMap.insert(Sec.at(i),ktempMap);
            }
        }
    }

}

/*
  print the usage
  */
void usage()
{
    qDebug()<<"./job_xml_generate -t template.xml "<<endl
            <<"\tGeneral [ key value ] /General"<<endl
            <<"\tImageConfig [ key value ] /ImageConfig"<<endl
            <<"\tLTKConfig [ key value ] /LTKConfig"<<endl
           <<"\tLogConfig [ key value ] /LogConfig"<<endl
          <<"\tExtraAct [ key value ] /ExtraAct"<<endl
          <<"\tLogConfig [ key value ] /LogConfig"<<endl
            <<"\tCaseList testcase [ key value ] /testcase testcase .. /testcase /CaseList"<<endl
            <<"\t -f <FileName>"<<endl
            ;


}

int main(int argc, char **argv)
{

    if((argc%2==0) || (argc==1)){//print the usage() if the num of arguments is even.
        for(int i=0;i<argc;i++)
            qDebug()<<__LINE__<<"i="<<i<<" argv="<<argv[i]<<endl;
        usage();
        return 0;
    }

    //initialize the Sec,could be extended
    Sec<<"General";
    Sec<<"ImageConfig";
    Sec<<"LTKConfig";
    Sec<<"LogConfig";
    Sec<<"ExtraAct";
    Sec<<"LogConfig";
    Sec<<"CaseList";

    //parse the command to change the value of ComMap and CaseVec
//    print(ComMap,CaseVec);
    parser_cmd(argc, argv);
//    print(ComMap,CaseVec);

    //reader the template xml and store the results into inside struct
    DomParser reader;
    reader.setPriQString(Pri);
    reader.setSecQString(Sec);
    reader.setTriQString(Tri);
//    reader.readFile(":/job.xml");
    reader.readFile(TemPFileName);

    QVector<QMap<QString, QString> > tempCaseVec=reader.getCaseVec();
    QMap<QString, QMap<QString, QString> > tempComMap=reader.getCommonMap();

//    print(tempComMap,tempCaseVec);
    //replace the CaseList elements value according to the input of argument----command line
    reader.setCaseVec(CaseVec);

    //merge the command line input into the template ComMap. the template xml value will
    //change only if command line change it explicitly
    merge(ComMap,tempComMap);//the Commap will merge the value tempComMap to ComMap together
    reader.setCommonMap(tempComMap);
//    print(tempComMap,CaseVec);

    //save the result
    reader.saveXml(OutFileName);
    return 0;
}

