#ifndef XMLWRITER_H
#define XMLWRITER_H
#include <QtCore/QString>
#include <QtXml/QXmlStreamWriter>
class XmlStreamWriter
{
    public:
        //XmlStreamWriter():Pri("bookindex"),Sec("entry"),Tri("Page")
        //{
        //};

    XmlStreamWriter(QString iPri,QString iSec,QString iTri,QString iAttrName="keyword")
        {
            Pri=iPri;
            Sec=iSec;
            Tri=iTri;
                        AttrName=iAttrName;
        };
        void writeIndexEntry(QXmlStreamWriter *xmlWriter, QTreeWidgetItem *item);
        bool writeXml(const QString &fileName, QTreeWidget *treeWidget);
                bool writeXml(const QString &fileName, QTableWidget *tableWidget);

        void setSecQString(QString iSec){Sec=iSec;}
        void setTriQString(QString iTri){Tri=iTri;}
                void setAttriName(QString iAttrName) {AttrName=iAttrName;}

    private:
        QString 	Pri;
        QString		Sec;
        QString		Tri;
                QString         AttrName;
};
#endif // XMLWRITER_H
