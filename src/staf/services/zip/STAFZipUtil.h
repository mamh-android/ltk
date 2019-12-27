/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPUtil
#define STAF_ZIPUtil


class STAFZipUtil
{
private:

    STAFHandlePtr handle;              // ZIP service's STAF handle

public:

    STAFZipUtil();

    STAFZipUtil(STAFHandlePtr h);

    STAFResultPtr log(const char*, const char*);

    STAFRC_t putValue (FILE*, uLong, int);

    STAFRC_t getByte(FILE*, int*);

    STAFRC_t getShort (FILE*, uLong*);

    STAFRC_t getLong (FILE*, uLong*);

    STAFRC_t copyFile(char*, const char*);

    int myStrCmp(const char*, const char*);

    uLong fileTime(char*);

    void fileTime(uLong, struct tm*);

    void changeFileDate(const char*, uLong, tm);

    STAFRC_t myMkDir(const char*);

    STAFRC_t myMkDir(const char*, void*, const char*);

    STAFRC_t makeDir (char*);

    STAFRC_t makeDir (char*, void*, const char*);

    int checkFileExist(const char*);

    char* calculateFileNameInZip(const char*, int);

    STAFRC_t changeFileSize(const char*, STAFInt64_t);

    void convertSTAFString(STAFString, const char*);

    void convertSTAFStringBuffer(STAFStringBuffer*, const char*);

    int seek(FILE *file, STAFInt64_t offset, int origin);

    STAFInt64_t tell(FILE *file);

    ~STAFZipUtil();
};

#endif
