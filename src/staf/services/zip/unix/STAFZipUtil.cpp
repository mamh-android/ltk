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
#include <utime.h>

#include "STAFZip.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipUtil.h"


// change_file_date : change the date/time of a file
// filename : the filename of the file where date/time must be modified
// dosdate : the new date at the MSDos format (4 bytes)
// tmu_date : the SAME new date at the tm_unz format
void STAFZipUtil::changeFileDate(const char *filename, uLong dosdate, tm
tmu_date)
{

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::changeFileDate_CP1")
                     + " filename ["
                     + filename
                     + "] dosdate ["
                     + dosdate
                     + "]");
*/

    struct utimbuf ut;
    struct tm newdate;

    newdate.tm_sec = tmu_date.tm_sec;
    newdate.tm_min = tmu_date.tm_min;
    newdate.tm_hour = tmu_date.tm_hour;
    newdate.tm_mday = tmu_date.tm_mday;
    newdate.tm_mon = tmu_date.tm_mon;

    if (tmu_date.tm_year > 1900)
    {
        newdate.tm_year = tmu_date.tm_year - 1900;
    }
    else
    {
        newdate.tm_year = tmu_date.tm_year;
    }

    newdate.tm_isdst = -1;

    ut.actime = ut.modtime = mktime(&newdate);
    utime(filename, &ut);
}


// make a directory
STAFRC_t STAFZipUtil::myMkDir(const char *dirname)
{
/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir1_CP1")
                     + " dirname ["
                     + dirname
                     + "]");
*/

    int ret = 0;

    ret = mkdir(dirname, 0755);

    if (ret != 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir1_CP2")
                     + " ret ["
                     + ret
                     + "]");

        return kZIPErrorCreatingDir;
    }

    return kSTAFOk;
}


// create a directory and set permission
STAFRC_t STAFZipUtil::myMkDir(const char *dirname, void *cde,
                              const char *outputdir)
{
/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir2_CP1")
                     + " dirname ["
                     + dirname
                     + "] outputdir ["
                     + outputdir
                     + "]");
*/

    STAFRC_t rc = kSTAFOk;

    int ret = 0;

    ret = mkdir(dirname, 0755);

    if (ret != 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir2_CP2")
                     + " ret ["
                     + ret
                     + "]");

        return kZIPErrorCreatingDir;
    }


    char buffer[MAXFILENAME] = "";

    char *p;

    strcpy(buffer, dirname);

    p = buffer + strlen(outputdir);

/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir2_CP3")
                     + " p ["
                     + p
                     + "]");
*/

    STAFZipFileAttribute *fa = ((STAFZipCentralDirExtension*)cde)->find(p);

    if (fa != NULL)
    {

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir2_CP4")
                     + " outputdir ["
                     + outputdir
                     + "]");

        rc = fa->set(outputdir);
    }


/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir2_CP5")
                     + " rc ["
                     + rc
                     + "]");
*/

    return rc;
}


// change file size
STAFRC_t STAFZipUtil::changeFileSize(const char *zipfilename,
                                     STAFInt64_t newsize)
{
/*
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::changeFileSize_CP1")
                     + " zipfilename ["
                     + zipfilename
                     + "] newsize ["
                     + newsize
                     + "]");
*/
    int fd = open(zipfilename, O_RDWR);

    int err = ftruncate(fd, newsize);

    close(fd);
    
    if (err == -1)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::changeFileSize_CP2")
                     + " err ["
                     + err
                     + "]");

        return kZIPChangeFileSizeError;
    }

    return kSTAFOk;
}



