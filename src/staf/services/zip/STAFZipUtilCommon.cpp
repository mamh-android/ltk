/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFTrace.h"

#include "zlib.h"

#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <map>
#include <errno.h>     // For ENOENT
#include <stdio.h>     // For fseeko

#include "STAFZip.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipUtil.h"


// constructor
STAFZipUtil::STAFZipUtil(STAFHandlePtr h)
{
    handle = h;
}

// constructor
STAFZipUtil::STAFZipUtil()
{
}

// convert STAFString to C string
void STAFZipUtil::convertSTAFString(STAFString stafStr, const char* cStr)
{
    unsigned int length = 0;
    const char *temp;
    char *p = (char*)cStr;

    temp = stafStr.buffer(&length);

    memcpy((void*)cStr, (void*)temp, length);

    p[length] = '\0';
}


// copy STAFString buffer to string buffer
void STAFZipUtil::convertSTAFStringBuffer(STAFStringBuffer* stafStrBuf, const char* cStr)
{
    unsigned int length = 0;
    const char *temp;
    char *p = (char*)cStr;

    length = stafStrBuf->length();

    temp = stafStrBuf->buffer();

    memcpy((void*)cStr, (void*)temp, length);

    p[length] = '\0';
}


// save nbByte of data to file
STAFRC_t STAFZipUtil::putValue (FILE *f, uLong x, int nbByte)
{
    unsigned char buf[4];
    int n;

/*    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::putValue_CP1")
                     + " x ["
                     + x
                     + "] nbByte ["
                     + nbByte
                     + "]");
*/

    for (n = 0; n < nbByte; n++)
    {
        buf[n] = (unsigned char)(x & 0xff);

        x >>= 8;
    }

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::putValue_CP2")
                     + "] buf ["
                     + (char*)buf
                     + "]");
*/

    if (fwrite(buf, nbByte, 1, f) != 1)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::putValue_CP3"));

        return kSTAFFileWriteError;
    }
    else
    {
        return kSTAFOk;
    }

}


// get a byte from file and put it into *pi
STAFRC_t STAFZipUtil::getByte(FILE *fin, int *pi)
{
    unsigned char c;


    int err = fread(&c, 1, 1, fin);

    if (err == 1)
    {
        *pi = (int)c;

/*
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getByte_CP1")
                     + " *pi ["
                     + *pi
                     + "]");
*/

        return kSTAFOk;
    }
    else
    {
/*
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getByte_CP2")
                     + " err ["
                     + err
                     + "]");
*/

        if (ferror(fin))
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getByte_CP3")
                     + " err ["
                     + err
                     + "]");

            return kSTAFFileReadError;
        }
        else
        {
/*
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getByte_CP4")
                     + " err ["
                     + err
                     + "]");
*/

            return kSTAFOk;
        }
    }
}


// get 2 bytes data from file and put it into *pX
STAFRC_t STAFZipUtil::getShort (FILE *fin, uLong *pX)
{
    uLong x ;
    int i;
    STAFRC_t rc;

    rc = getByte(fin, &i);

    x = (uLong)i;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getShort_CP1")
                     + " rc ["
                     + rc
                     + "] x ["
                     + x
                     + "] i ["
                     + i
                     + "]");
*/

    if (rc == kSTAFOk)
    {
        if ((rc = getByte(fin, &i)) == kSTAFOk)
        {
            x += ((uLong)i) << 8;
        }
    }

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getShort_CP2")
                     + " err ["
                     + rc
                     + "] x ["
                     + x
                     + "] i ["
                     + i
                     + "]");
*/

    if (rc == kSTAFOk)
    {
        *pX = x;
    }
    else
    {
        *pX = 0;
    }

    return rc;
}


// get 4 bytes data from file and put it into *pX
STAFRC_t STAFZipUtil::getLong (FILE *fin, uLong *pX)
{
    uLong x ;
    int i;
    STAFRC_t rc;

    rc = getByte(fin, &i);
    x = (uLong)i;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getLong_CP1")
                     + " rc ["
                     + rc
                     + "] x ["
                     + x
                     + "] i ["
                     + i
                     + "]");
*/

    if (rc == kSTAFOk)
    {
        rc = getByte(fin, &i);
    }

    x += ((uLong)i) << 8;


/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getLong_CP2")
                     + " rc ["
                     + rc
                     + "] x ["
                     + x
                     + "] i ["
                     + i
                     + "]");
*/


    if (rc == kSTAFOk)
    {
        rc = getByte(fin, &i);
    }

    x += ((uLong)i) << 16;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getLong_CP3")
                     + " rc ["
                     + rc
                     + "] x ["
                     + x
                     + "] i ["
                     + i
                     + "]");
*/

    if (rc == kSTAFOk)
    {
        rc = getByte(fin, &i);
    }

    x += ((uLong)i) << 24;


/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::getLong_CP4")
                     + " rc ["
                     + rc
                     + "] x ["
                     + x
                     + "] i ["
                     + i
                     + "]");
*/

    if (rc == kSTAFOk)
    {
        *pX = x;
    }
    else
    {
        *pX = 0;
    }

    return rc;
}


// copy source file to dest file
STAFRC_t STAFZipUtil::copyFile(char *source, const char *dest)
{
    FILE *sf, *df;
/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::copyFile_CP1")
                     + " source ["
                     + source
                     + "] dest ["
                     + dest
                     + "]");
*/
    if ((sf = fopen(source, "rb")) == NULL)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::copyFile_CP2"));

        return kSTAFFileOpenError;
    }

    if ((df = fopen(dest, "wb")) == NULL)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::copyFile_CP3"));

        return kSTAFFileWriteError;
    }
    
    int c;

    while ((c = fgetc(sf))!= EOF)
    {
        fputc(c, df);
    }

    fclose(sf);
    fclose(df);

    return kSTAFOk;
}


// compare two strings
int STAFZipUtil::myStrCmp(const char *fileName1, const char *fileName2)
{

    char f1[MAXFILENAME] = "";
    char f2[MAXFILENAME] = "";
    char *p, *p1, *p2;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myStrCmp_CP1")
                     + " fileName1 ["
                     + fileName1
                     + "] fileName2 ["
                     + fileName2
                     + "]");
*/

    strcpy(f1, fileName1);
    strcpy(f2, fileName2);

    p = f1;

    while (*p != '\0')
    {
        if (*p == '\\')
        {
            *p = '/';
        }

        p++;
    }

    p = f2;

    while (*p != '\0')
    {
        if (*p == '\\')
        {
            *p = '/';
        }

        p++;
    }

    int iCaseSensitivity = CASESENSITIVITYDEFAULTVALUE;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myStrCmp_CP2")
                     + " f1 ["
                     + f1
                     + "] f2 ["
                     + f2
                     + "] iCaseSensitivity ["
                     + iCaseSensitivity
                     + "]");
*/

    // if case sensitive
    if (iCaseSensitivity == 1)
    {
        return strcmp(f1, f2);
    }


    // if case insensitive
    p1 = f1;
    p2 = f2;

    for (;;)
    {
        char c1 = *(p1++);
        char c2 = *(p2++);

        if ((c1 >= 'a') && (c1 <= 'z'))
        {
            c1 -= 0x20;
        }

        if ((c2 >= 'a') && (c2 <= 'z'))
        {
            c2 -= 0x20;
        }

        if (c1 == '\0')
        {
            return ((c2 == '\0') ? 0 : -1);
        }

        if (c2 == '\0')
        {
            return 1;
        }

        if (c1 < c2)
        {
            return -1;
        }

        if (c1 > c2)
        {
            return 1;
        }
    }
}


// get file time of a given file name
uLong STAFZipUtil::fileTime(char *f)
{
    struct stat s;        /* results of stat() */
    struct tm* filedate;
    time_t tm_t=0;


/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::fileTime1_CP1")
                     + " f ["
                     + f
                     + "]");
*/


    if (strcmp(f, "-") != 0)
    {
        char name[MAXFILENAME + 1];
        int len = strlen(f);

        strcpy(name, f);

        if (name[len - 1] == '/')
        {
            name[len - 1] = '\0';
        }

        /* not all systems allow stat'ing a file with / appended */
        if (stat(name,&s) == 0)
        {
            tm_t = s.st_mtime;
        }
    }

    filedate = localtime(&tm_t);

    uLong year = (uLong)filedate->tm_year;

    if (year > 1980)
    {
        year -= 1980;
    }
    else if (year > 80)
    {
        year -= 80;
    }


    return
      (uLong) (((filedate->tm_mday) + (32 * (filedate->tm_mon + 1))
                + (512 * year)) << 16)
              | ((filedate->tm_sec / 2) + (32 * filedate->tm_min)
                + (2048 * (uLong)filedate->tm_hour));

}


// convert file time from dos long to tm structure
void STAFZipUtil::fileTime(uLong dosdate, struct tm *filedate)
{
    uLong date;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::fileTime2_CP1")
                     + " dosdate ["
                     + dosdate
                     + "]");
*/

    date = (uLong)(dosdate >> 16);

    filedate->tm_mday = (uInt)(date & 0x1f) ;
    filedate->tm_mon = (uInt)(((date & 0x1E0) / 0x20) - 1) ;
    filedate->tm_year = (uInt)(((date & 0x0FE00) / 0x0200) + 1980) ;

    filedate->tm_hour = (uInt)((dosdate & 0xF800) / 0x800);
    filedate->tm_min = (uInt)((dosdate & 0x7E0) / 0x20) ;
    filedate->tm_sec = (uInt)(2 * (dosdate & 0x1f)) ;

}


// log util
STAFResultPtr STAFZipUtil::log(const char *level, const char *message)
{
    return handle->submit("LOCAL", "LOG",
                          STAFString("LOG GLOBAL LOGNAME zip LEVEL ")
                            + STAFString(level)
                            + STAFString(" MESSAGE \"")
                            + STAFHandle::wrapData(message)
                            + STAFString("\" NORESOLVEMESSAGE"));
}


// create a directory
STAFRC_t STAFZipUtil::makeDir (char *newdir)
{
    char *buffer ;
    char *p;
    int  len = strlen(newdir);


/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir1_CP1")
                     + " newdir ["
                     + newdir
                     + "] len ["
                     + len
                     + "]");
*/

    if (len <= 0)
    {
        return kSTAFInvalidValue;
    }

    if ((buffer = (char*)calloc(len + 1, 1)) == NULL)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir1_CP2"));

        return kZIPNotEnoughMemory;
    }

    strcpy(buffer, newdir);

    if (buffer[len - 1] == '/' || buffer[len - 1] == '\\')
    {
        buffer[len - 1] = '\0';
    }


/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir1_CP3")
                     + " buffer ["
                     + buffer
                     + "]");
*/

    // try creating the directory
    if (myMkDir(buffer) == kSTAFOk)
    {
        free(buffer);

        return kSTAFOk;
    }


    // try creating parent directories
    p = buffer+1;
    while (1)
    {
        char hold;

        while(*p && *p != '\\' && *p != '/')
        {
          p++;
        }

        hold = *p;

        *p = 0;


/*
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir1_CP4")
                     + " buffer ["
                     + buffer
                     + "]");
*/

        // try creating top parent directory
        if ((myMkDir(buffer) != 0) && (errno == ENOENT))
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir1_CP5"));

            free(buffer);

            return kZIPErrorCreatingDir;
        }

        if (hold == 0)
        {
          break;
        }

        *p++ = hold;
    }

    free(buffer);

    return kSTAFOk;
}


// create dir and restore permission
STAFRC_t STAFZipUtil::makeDir (char *newdir, void *cde, const char *outputdir)
{
    char *buffer ;
    char *p;
    int  len = strlen(newdir);

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir2_CP1")
                     + " newdir ["
                     + newdir
                     + "] len ["
                     + len
                     + "] outputdir ["
                     + outputdir
                     + "]");
*/

    if (len <= 0)
    {
        return kSTAFInvalidValue;
    }

    if ((buffer = (char*)calloc(len + 1, 1)) == NULL)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir2_CP2"));

        return kZIPNotEnoughMemory;
    }

    strcpy(buffer, newdir);

    if (buffer[len - 1] == '/' || buffer[len - 1] == '\\')
    {
        buffer[len - 1] = '\0';
    }

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir2_CP3")
                     + " buffer ["
                     + buffer
                     + "] len ["
                     + len
                     + "] outputdir ["
                     + outputdir
                     + "]");
*/

    // try creating the directory
    if (myMkDir(buffer, cde, outputdir) == kSTAFOk)
    {
        free(buffer);

        return kSTAFOk;
    }

    // try creating parent directory
    p = buffer+1;
    while (1)
    {
        char hold;

        while(*p && *p != '\\' && *p != '/')
        {
          p++;
        }

        hold = *p;

        *p = 0;


/*
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir2_CP4")
                     + " buffer ["
                     + buffer
                     + "] len ["
                     + len
                     + "] outputdir ["
                     + outputdir
                     + "]");
*/

        // try creating top parent directory
        if ((myMkDir(buffer, cde, outputdir) != kSTAFOk) && (errno == ENOENT))
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::makeDir2_CP5"));

            free(buffer);

            return kZIPErrorCreatingDir;
        }

        if (hold == 0)
        {
          break;
        }

        *p++ = hold;
    }

    free(buffer);

    return kSTAFOk;
}


// check to see if file exists
int STAFZipUtil::checkFileExist(const char *filename)
{
/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::checkFileExist_CP1")
                     + " filename ["
                     + filename
                     + "]");
*/

    FILE* fTestExist;
    int ret;

    fTestExist = fopen(filename, "rb");

    if (fTestExist == NULL)
    {
        ret = 0;
    }
    else
    {
        fclose(fTestExist);

        ret = 1;
    }

    return ret;
}


// calculate file name in zip archive by removing the prefix
char* STAFZipUtil::calculateFileNameInZip(const char *filename,
                                          int prefix_length)
{
    char *filename_in_zip;

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::calculateFileNameInZip_CP1")
                     + " filename ["
                     + filename
                     + "] prefix_length ["
                     + prefix_length
                     + "]");
*/

    if (prefix_length != 0)
    {
        filename_in_zip = (char*)(filename + prefix_length);
        if (*filename_in_zip == '/' || *filename_in_zip == '\\')
        {
            filename_in_zip++;
        }
    }
    else
    {
        filename_in_zip = (char*)strchr(filename, '/');
        if (filename_in_zip == NULL)
        {
            filename_in_zip = (char*)strchr(filename, '\\');
            if (filename_in_zip == NULL)
            {
                filename_in_zip = (char*)filename;
            }
            else
            {
                filename_in_zip++;
            }
        }
        else
        {
            filename_in_zip++;
        }
    }

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::calculateFileNameInZip_CP2")
                     + " filename_in_zip ["
                     + filename_in_zip
                     + "]");
*/

    return filename_in_zip;
}


// Sets the current position in the file to the specified offset based on
// the origin by calling the appropriate seek function.  Supports files
// that are > 2G in length (unlike fseek).
// Returns 0 if successful; -1 if fails.

int STAFZipUtil::seek(FILE *file, STAFInt64_t offset, int origin)
{
#ifdef STAF_OS_TYPE_WIN32
    return _fseeki64(file, offset, origin);
#else
    return fseeko(file, offset, origin);
#endif
}


// Gets the current position in the file by calling the appropriate tell
// function.  Supports files that are > 2G in length (unlike ftell).
// Returns the current position in the file if successful; -1 if fails.

STAFInt64_t STAFZipUtil::tell(FILE *file)
{
#ifdef STAF_OS_TYPE_WIN32
    return _ftelli64(file);
#else
    return (STAFInt64_t)ftello(file);
#endif
}


// Destructor
STAFZipUtil::~STAFZipUtil()
{
    /* Do Nothing */
}



