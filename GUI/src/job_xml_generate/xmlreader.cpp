
#include <QtCore/QFile>
#include <iostream>
#include "xmlreader.h"

XmlStreamReader::XmlStreamReader()
{
    Pri = "bookindex";
    Sec = "entry";
    Tri = "page";
}

bool XmlStreamReader::readFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        std::cerr << "Error: Cannot read file " <<qPrintable(fileName)
                  << ": " <<qPrintable(file.errorString())
                  <<std::endl;\
        return false;
    }
    std::cout<<__LINE__<<__FILE__<<" Debug:"<<std::endl;
    reader.setDevice(&file);

    reader.readNext();
    while(!reader.atEnd()){
        if(reader.isStartElement()){
            if(reader.name() == Pri){
                readPriElement();
            }else{
            reader.raiseError(QObject::tr("Not a indexed file"));
            }
        } else{
            reader.readNext();
        }//if
    }//while

    file.close();
    if(reader.hasError()){
        std::cerr << "Error: Failed to parse file "
                  << qPrintable(fileName) << ": "
                  << qPrintable(reader.errorString()) <<std::endl;
        return false;
    }else if (file.error() != QFile::NoError){
        std::cerr << "Error: can't read file "
                  << qPrintable(fileName) << ": "
                  << qPrintable(file.errorString()) <<std::endl;
        return false;
    }

    return true;
}


void XmlStreamReader::readPriElement()
{
    reader.readNext();
    while(!reader.atEnd()){
        if (reader.isEndElement()){
            reader.readNext();
            break;
        }

        if (reader.isStartElement()){
            if (reader.name() == Sec) {
                readSecElement();
            }else{
                skipUnknownElement();
            }
        }else {
            reader.readNext();
        }
    }
}

void XmlStreamReader::readSecElement()
{


    reader.readNext();
    while(!reader.atEnd()){
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()){
            if (reader.name() == Sec) {
                readSecElement();
            } else if(reader.name() == Tri){
                readTriElement();
            } else {
                skipUnknownElement();
            }
        }else{
            reader.readNext();
        }
    }
}


void XmlStreamReader::readTriElement()
{
    QString page =	reader.readElementText();
    keys.push_back(qPrintable(page));
    QStringRef name =	reader.name();
    keys.push_back(qPrintable(name.toString()));
    if (reader.isEndElement())
        reader.readNext();



}

void XmlStreamReader::skipUnknownElement()
{
    reader.readNext();
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
    }
}

