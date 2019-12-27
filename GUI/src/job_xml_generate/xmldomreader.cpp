#include <QtCore/QFile>
#include <QtCore/QtDebug>
#include <QtCore/QString>
#include <iostream>
#include "xmldomreader.h"



DomParser::DomParser()
{
    Pri = "CTKJob";
    Sec<<"General";
    Sec<<"ImageConfig";
    Sec<<"LTKConfig";
    Sec<<"LogConfig";
    Sec<<"CaseList";
    Tri = "testcase";
}

DomParser::~DomParser()
{

}


bool DomParser::readFile(const QString &inputFile)
{


    QFile file(inputFile);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        std::cerr << "Error: Cannot read file " <<qPrintable(fileName)
                  << ": " <<qPrintable(file.errorString())
                  <<std::endl;
        return false;
    }
    //qDebug()<<__LINE__<<__FILE__<<" Debug:"<<std::endl;
//    qDebug()<<__LINE__<<__FILE__<<" Debug:";

    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if(!doc.setContent(&file,false,&errorStr,&errorLine,&errorColumn))
    {
        qDebug()<<"Error: Parse error at line" << errorLine << ", "
               << "column " << errorColumn << ": "
               << qPrintable(errorStr) <<"\n";
        return false;
    }

    QDomElement root = doc.documentElement();
    //qDebug()<<doc.toString();

    //  print out the element names of all elements that are direct children
    // of the outermost element.
    QDomNode n = root.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
//            qDebug() <<__LINE__<<__FILE__<<"child:"<< qPrintable(e.tagName()); // the node really is an element.
        }
        n = n.nextSibling();
    }


    //find the first <anon> element. skip another element.
    QDomNode child = root.firstChild();
    while (! child.isNull()){
        if (child.toElement().tagName() != Pri){
//            qDebug()<<__LINE__<<__FILE__<<child.toElement().tagName();

            child = child.nextSibling();
            continue;
        }
        break;
    }

    QDomElement rootTemp = root;
    if(rootTemp.tagName() == Pri){
        readPriElement(rootTemp);

    }else{
        qDebug()<<__LINE__<<__FILE__<<QObject::tr("Not a indexed file");
    }

    file.close();
    setFileName(inputFile);

    return true;
}





void DomParser::readPriElement(const QDomElement &element)
{
    QDomNode child = element.firstChild();
    while (! child.isNull()){
//        qDebug()<<__LINE__<<__FILE__<<"child text"<<child.toElement().text();
//        if (child.toElement().tagName() == Sec){
        if (Sec.contains(child.toElement().tagName()) ){
//            readSecElement(child.toElement(),treeWidget->invisibleRootItem()->child(0)); //there is only one Tree by default.
            readSecElement(child.toElement()); //there is only one Tree by default.
        }
        child = child.nextSibling();                //if it is not <Sec> element, then ignore it and search next element.
    }
}

//void DomParser::readSecElement(const QDomElement &element, QTreeWidgetItem *parent)
void DomParser::readSecElement(const QDomElement &element)
{
    QMap<QString, QString> secTempMap;
    QString priKey=element.tagName();

    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        //                if (child.toElement().tagName() == Sec){
        if (Sec.contains(child.toElement().tagName())){
            readSecElement(child.toElement());
        } else if (child.toElement().tagName() == Tri) {//check the testcase node.
            readTriElement(child.toElement());
//            return;
        }else{
            QDomElement tempElement=child.toElement();

//            qDebug()<<__LINE__<<"child "<<tempElement.text()<<" tag="<<tempElement.tagName();
            secTempMap.insert(tempElement.tagName(),tempElement.text());
        }
        child = child.nextSibling();
    }

    CommonMap.insert(priKey,secTempMap);
}





/*
  it will read the value of case_id of element as casename, put it into the first volumn,
  and put the description's value into the second volumn. and store the properities from xml into GmapProperties
  */
//void DomParser::readTriElement(const QDomElement &element,QTreeWidgetItem *parent)
void DomParser::readTriElement(const QDomElement &element)
{
    QMap<QString,QString> tempCaseMap;
    QDomNode child = element.firstChild();
    while (!child.isNull()) {

        QDomElement tempElement=child.toElement();

//        qDebug()<<__LINE__<<"child "<<tempElement.text()<<" tag="<<tempElement.tagName();
        tempCaseMap.insert(tempElement.tagName(),tempElement.text());
        child = child.nextSibling();
    }
    CaseVec.push_back(tempCaseMap);
}


void DomParser::skipUnknownElement()
{
    /*reader.readNext();
        while(!reader.atEnd()){
                if (reader.isEndElement()){
                        reader.readNext();
                        break;
                }

                if (reader.isStartElement()){
                        skipUnknownElement();
                }else{
                        reader.readNext();
                }
        }*/
}

/*
  save the template xml result to a xmlFileName file
  */
void DomParser::saveXml(QString xmlFileName)
{

    QFile file(xmlFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream outTemp(&file);


//add the head info into generated xml file
    QDomDocument doc;
    QDomNode node(doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"utf-8\""));
    doc.insertBefore(node,doc.firstChild());


    QDomElement top_root = doc.createElement(Pri);
    doc.appendChild(top_root);

//    QDomElement top2_root = doc.createElement(Pri);
//    top1_root.appendChild(top2_root);


//    QMap<QString,caseInfo>::const_iterator i;

    //    QDomElement level2 = doc.createElement("saveinfo");
    //    root.appendChild(level2);
//    for(i=GmapResult.constBegin(); i!=GmapResult.constEnd(); i++)
    for(int i=0;i<Sec.size();i++)
    {
        if(Sec.at(i)!="CaseList"){
            QDomElement root = doc.createElement(Sec.at(i));
            top_root.appendChild(root);


            QMap<QString,QString> tempMap=CommonMap.value(Sec.at(i));
//            qDebug()<<__LINE__<<"string="<<Sec.at(i)<<" size1="<<tempMap.size();

            QStringList keys=tempMap.keys();
            for(int k=0;k<keys.size();k++){
                QDomElement tag1 = doc.createElement(keys.at(k));
                root.appendChild(tag1);
                QDomText t1 = doc.createTextNode(tempMap.value(keys.at(k)));
                tag1.appendChild(t1);
            }

        }else{
            QDomElement root = doc.createElement(Sec.at(i));
            top_root.appendChild(root);

            for(int j=0;j<CaseVec.size();j++){
                QDomElement root2 = doc.createElement(Tri);
                root.appendChild(root2);

                QStringList keys=CaseVec.at(j).keys();
                for(int k=0;k<keys.size();k++){
                    QDomElement tag1 = doc.createElement(keys.at(k));
                    root2.appendChild(tag1);
                    QDomText t1 = doc.createTextNode(CaseVec.at(j).value(keys.at(k)));
                    tag1.appendChild(t1);
                }
            }
        }
    }
    doc.save(outTemp,4);
    file.close();

}
