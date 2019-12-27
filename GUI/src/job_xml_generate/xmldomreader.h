#ifndef XMLDOMREADER_H
#define XMLDOMREADER_H
#include <QtXml/QDomElement>
#include <vector>
#include <iostream>
#include <QtCore/QStringList>

class DomParser
{
public:
    DomParser();
    ~DomParser();

    bool readFile(const QString &inputFile);
    void setPriQString(QString iPri){Pri=iPri;}
    void setSecQString(QStringList &iSec){Sec=iSec;}
    void setTriQString(QString iTri){Tri=iTri;}
    QString getPriQString() {return Pri;}
    QStringList getSecQString() {return Sec;}
    QString getTriQString() {return Tri;}
    QString getFileName() {return fileName;}
    void setFileName(QString inFileName) {fileName=inFileName;}
    QVector<QMap<QString, QString> > & getCaseVec()  {return CaseVec;}
    void setCaseVec(QVector<QMap<QString, QString> > &temp) {CaseVec=temp;}
    QMap<QString, QMap<QString, QString> > & getCommonMap()  {return CommonMap;}
    void setCommonMap(QMap<QString, QMap<QString, QString> > &temp)  {CommonMap=temp;}
    void saveXml(QString xmlFileName);
    void appendDomElementFromGmapResult(const QString &keyName, QDomElement &saveChild, QDomDocument &doc,QString Prefix);



private:
    void readPriElement(const QDomElement &element);
    void readSecElement(const QDomElement &element);
    void readTriElement(const QDomElement &element);
    void skipUnknownElement();

    QString 	Pri;
    QStringList		Sec;//it is a list including hard-code node texts
    QString		Tri;
    QString         fileName;
    QMap<QString, QMap<QString, QString> > CommonMap;
    std::vector<std::string> keys;
    QVector<QMap<QString, QString> > CaseVec;
};
#endif // XMLDOMREADER_H
