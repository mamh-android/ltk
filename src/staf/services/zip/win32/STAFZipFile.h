/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPFile
#define STAF_ZIPFile


class STAFZipFile
{
private:

    STAFHandlePtr handle;              // ZIP service's STAF handle

    STAFZipCentralDir *centralDirPtr;

    STAFZipUtil *util;

    std::vector<STAFZipLocalFileHeader*> localFileHeaderListCurrent;

    std::map<STAFString, STAFZipLocalFileHeader*> localFileHeaderListCurrentSorted;

    std::vector<STAFZipLocalFileHeader*> localFileHeaderListNew;

    FILE *zf;

    int newZipFile;

    STAFInt64_t startPos;
    
    STAFInt64_t endPos;

    STAFString zipFileName;  // Zip file name

public:


    STAFZipFile(STAFHandlePtr, FILE*, STAFRC_t*, STAFString*, int,
                STAFInt64_t end=-1, STAFInt64_t start=-1,
                const STAFString &zfName=0);

    STAFRC_t zipFile(const char*, int, int, STAFString*);

    STAFRC_t zipDir(const char*, int, int, STAFString*);

    STAFRC_t deleteFile(const char*, STAFInt64_t*, STAFString*);

    STAFRC_t listFile(STAFString*, STAFString*);

    STAFRC_t unzipFile(const char*, char*, int, int, STAFString*);

    STAFRC_t unzipFile(char*, int, int, STAFString*);

    STAFZipLocalFileHeader* find(const char *);

    STAFRC_t readInData(STAFString *);

    STAFRC_t unzipDir(const char*, char*, int, int, STAFString*);

    std::vector<STAFString> findDir(const char *);

    std::vector<STAFString> findAll(const char *);

    ~STAFZipFile();
};

#endif
