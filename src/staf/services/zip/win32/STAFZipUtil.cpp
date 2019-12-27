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
#include <direct.h>
#include <io.h>

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

    HANDLE hFile;
    FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

    hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
    GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
    DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
    LocalFileTimeToFileTime(&ftLocal,&ftm);
    SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
    CloseHandle(hFile);
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

    ret = mkdir(dirname);

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

    ret = mkdir(dirname);

    if (ret != 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipUtil::myMkDir2_CP2")
                     + " ret ["
                     + ret
                     + "]");

        return kZIPErrorCreatingDir;
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
STAFRC_t STAFZipUtil::changeFileSize(const char *zipfilename, STAFInt64_t newsize)
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

    int fd = _open( zipfilename, _O_RDWR );

    // XXX: Once we migrate to MS Visual Studio 2005 or later (instead of V6.0)
    // we can use:
    //   int err = _chsize_s(fd, newsize)
    // instead of _lseeki64() and SetEndOfFile().

    if (_lseeki64(fd, newsize, SEEK_SET) < 0)
    {
        _close(fd);

        STAFTrace::trace(
            kSTAFTraceServiceResult,
            STAFString("STAFZipUtil::changeFileSize ") +
            "_lseeki64 failed, newsize [" + newsize + "]");

        return kZIPChangeFileSizeError;
    }

    if (!SetEndOfFile((HANDLE)_get_osfhandle(fd)))
    {
        _close(fd);

        STAFTrace::trace(
            kSTAFTraceServiceResult,
            STAFString("STAFZipUtil::changeFileSize ") +
            "SetEndOfFile failed, newsize [" + newsize + "]");

        return kZIPChangeFileSizeError;
    }

    _close(fd);

    return kSTAFOk;
}

