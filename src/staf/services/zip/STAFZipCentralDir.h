/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
#ifndef STAF_ZIPCentralDir
#define STAF_ZIPCentralDir


class STAFZipCentralDir
{

public:

    // attributes

    std::vector<STAFZipFileHeader*> fileHeaderList;

    std::map<STAFString, STAFZipFileHeader*> fileHeaderListSorted;

    STAFZipCentralDirEndRecord* cder;

    // methods

    STAFZipCentralDir();
    
    STAFRC_t readInData(FILE*, STAFInt64_t, STAFInt64_t, STAFString*);

    STAFRC_t flush(FILE*, STAFString*);

    void addFileHeader(STAFZipLocalFileHeader*);

    STAFRC_t list(STAFString*, STAFString*);

    STAFZipFileHeader* find(const char*);

    STAFZipFileHeader* findLastFileHeader();

    STAFZipFileHeader* remove(const char*, STAFZipLocalFileHeader*);

    unsigned short getNumberEntry();

    uLong getSize();

    STAFZipCentralDirEndRecord* getCentralDirEndRecord();

    uLong getOffset();

    ~STAFZipCentralDir();
};

#endif
