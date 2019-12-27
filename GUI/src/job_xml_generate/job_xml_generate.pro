
QT       += core xml

TARGET = job_xml_generate
TEMPLATE = app


SOURCES += main.cpp \
    xmlreader.cpp \
    xmlwriter.cpp \
    xmldomreader.cpp

HEADERS += \
    xmlreader.h \
    xmlwriter.h \
    xmldomreader.h

OTHER_FILES +=

RESOURCES += \
    job_xml.qrc

