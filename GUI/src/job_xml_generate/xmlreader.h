#ifndef XMLREADER_H
#define XMLREADER_H
#include <QtXml/QXmlStreamReader>
#include <vector>
#include <iostream>

class XmlStreamReader
{
    public:
        XmlStreamReader();

        bool readFile(const QString &fileName);
        void setPriQString(QString iPri){Pri=iPri;}
        void setSecQString(QString iSec){Sec=iSec;}
        void setTriQString(QString iTri){Tri=iTri;}
        std::vector<std::string> keys_from_xml(){return keys;}

    private:
        void readPriElement();
        void readSecElement();
        void readTriElement();
        void skipUnknownElement();

        QXmlStreamReader reader;
        QString 	Pri;
        QString		Sec;
        QString		Tri;
        std::vector<std::string> keys;
};

#endif // XMLREADER_H
