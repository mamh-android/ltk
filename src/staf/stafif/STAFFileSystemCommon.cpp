/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <map>
#include "STAFFileSystem.h"
#include "STAFString.h"
#include "STAFMutexSem.h"
#include "STAFRWSem.h"
#include "STAF_fstream.h"

struct FileLock
{
    FileLock() : lock(0), lockSem(new STAFMutexSem, STAFMutexSemPtr::INIT),
                 numOwners(0), rwSem(new STAFRWSem, STAFRWSemPtr::INIT)
    { /* Do Nothing */ }

    STAFFSOSFileLock_t lock;
    STAFMutexSemPtr lockSem;
    unsigned int numOwners;
    STAFRWSemPtr rwSem;
};

typedef std::map<STAFString, FileLock> LockMap;

static LockMap sLockMap;
static STAFMutexSem sLockMapSem;
static STAFMutexSem sCurrDirSem;
static STAFString sSemiColon(kUTF8_SCOLON);


/*****************************************************************************/
/* STAFFSStringMatchesWildcards - Determines whether a given string matches  */
/*                                a given wildcard string                    */
/*                                                                           */
/* Accepts: (In)  String to check                                            */
/*          (In)  String containg wildcards                                  */
/*          (In)  Case sensitivity indicator                                 */
/*          (Out) Result of comparison (0 = no match, 1 = matches)           */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) Two wildcard characters are understood.  '*' matches zero or  */
/*             more characters.  '?' matches one single character.           */
/*          2) Doesn't do any backtracking, so for example, if the           */
/*             stringToCheck is XYYXYXY and the wildcardString is ?Y*X?,     */
/*             they will not match, but strings XYYYXY and XYYYXX will.      */
/*****************************************************************************/
STAFRC_t STAFFSStringMatchesWildcards(STAFStringConst_t stringToCheck,
                                      STAFStringConst_t wildcardString,
                                      STAFFSCaseSensitive_t sensitive,
                                      unsigned int *matches)
{
    if (sensitive == kSTAFFSCaseDefault)
        STAFFSInfo(&sensitive, kSTAFFSCaseSensitivity);

    STAFStringCaseSensitive_t caseSensitive;

    if (sensitive == kSTAFFSCaseInsensitive)
        caseSensitive = kSTAFStringCaseInsensitive;
    else
        caseSensitive = kSTAFStringCaseSensitive;

    return STAFStringMatchesWildcards(stringToCheck, wildcardString, 
                                      caseSensitive, matches, 0);
}


/*****************************************************************************/
/* STAFFSComparePaths - Compares two path names after "normalizing" them.    */
/*                      Checks if path1 includes (starts with) path2 or if   */
/*                      path1 is the same as path2.                          */
/*                                                                           */
/* Accepts: (In)  Path name 1                                                */
/*          (In)  Path name 2                                                */
/*          (In)  Case sensitivity indicator                                 */
/*          (Out) Compare result:                                            */
/*                - kSTAFFSDoesNotIncludePath (path1 does not include path2) */
/*                - kSTAFFSDoesIncludePath (path1 includes path2)            */
/*                - kSTAFFSSamePath (path1 and path2 specify the same path)  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Examples:                                                                 */
/*                                                                           */
/* 1) If called this API passing in the following arguments:                 */
/*      pathName1:  C:/temp/dir1/dir2                                        */
/*      pathName2:  C:/temp/dir2                                             */
/*    this API would set result to kSTAFFSDoesNotIncludePath                 */
/*                                                                           */
/* 2) If called this API passing in the following arguments:                 */
/*      pathName1:  C:/temp/dir1/dir2                                        */
/*      pathName2:  C:/temp  -OR-  C:/temp/dir1                              */
/*    this API would set result to kSTAFFSDoesIncludePath                    */
/*                                                                           */
/* 3) If called this API passing in the following arguments:                 */
/*      pathName1:  C:/temp/dir1/dir2                                        */
/*      pathName2:  C:/temp/dir1/dir2                                        */
/*    this API would set result to kSTAFFSSamePath                           */
/*****************************************************************************/
STAFRC_t STAFFSComparePaths(STAFStringConst_t pathName1,
                            STAFStringConst_t pathName2,
                            STAFFSCaseSensitive_t sensitive,
                            STAFFSComparePathResult_t *result)
{
    if (sensitive == kSTAFFSCaseDefault)
        STAFFSInfo(&sensitive, kSTAFFSCaseSensitivity);

    // Get a "normalized" form of the path names so that every path separator
    // (e.g. \, /, \\) is set to the line separator for the local system's
    // operating system.
    
    STAFFSPath path2 = STAFFSPath(pathName2);
    path2.setRoot(path2.root());

    STAFFSPath path1 = STAFFSPath(pathName1);
    path1.setRoot(path1.root());

    // Check if path1 starts with (e.g. includes) path2

    STAFString fileSep = "/";
    STAFFSInfo(&fileSep, kSTAFFSFileSep);
    STAFString path2String = path2.asString() + fileSep + "*";
    STAFString path1String = path1.asString() + fileSep;
    unsigned int matches = 0;

    STAFRC_t rc = STAFFSStringMatchesWildcards(
        path1String.getImpl(), path2String.getImpl(), sensitive, &matches);

    if (rc != kSTAFOk) return rc;
    
    if (!matches)
    {
        *result = kSTAFFSDoesNotIncludePath;
    }
    else
    {
        *result = kSTAFFSDoesIncludePath;

        // Check if the same path names

        path2String = path2.asString() + fileSep;

        rc = STAFFSStringMatchesWildcards(
            path1String.getImpl(), path2String.getImpl(), sensitive, &matches);

        if (rc != kSTAFOk) return rc;
        
        if (matches) 
            *result = kSTAFFSSamePath;
    }

    return rc;
}


// XXX: Should provide a common routine for the Read/WriteLock and
//      Read/WriteUnlock APIs.  They only differ by one line of code

STAFRC_t STAFFSEntryReadLock(STAFFSEntry_t entry, unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t filenameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &filenameT, osRC);

        if (rc) return rc;

        STAFString filename(filenameT);
        FileLock *pLock = 0;

        // First get the lock object from the map

        {
            STAFMutexSemLock mapLock(sLockMapSem);
            pLock = &sLockMap[filename];
        }

        // Now, get the underlying file system lock if needed, and increment
        // the number of owners

        {
            STAFMutexSemLock lockLock(*pLock->lockSem);

            if (pLock->lock == 0)
            {
                STAFRC_t rc = STAFFSOSGetExclusiveFileLock(filenameT,
                                                           &pLock->lock, osRC);
                if (rc != 0) return rc;
            }

            ++pLock->numOwners;
        }

        // Finally, get a read lock

        pLock->rwSem->readLock();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryReadUnlock(STAFFSEntry_t entry, unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t filenameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &filenameT, osRC);

        if (rc) return rc;

        STAFString filename(filenameT);
        FileLock *pLock = 0;

        // First get the lock object from the map

        {
            STAFMutexSemLock mapLock(sLockMapSem);
            pLock = &sLockMap[filename];
        }

        // Now, decrement the number of owners and release the underlying file
        // system lock if necessary

        STAFMutexSemLock lockLock(*pLock->lockSem);

        if (--pLock->numOwners == 0)
        {
            STAFRC_t rc = STAFFSOSReleaseExclusiveFileLock(&pLock->lock, osRC);

            if (rc != 0) return rc;

            pLock->lock = 0;
        }

        // Finally, release the read lock

        pLock->rwSem->readUnlock();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryWriteLock(STAFFSEntry_t entry, unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t filenameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &filenameT, osRC);

        if (rc) return rc;

        STAFString filename(filenameT);
        FileLock *pLock = 0;

        // First get the lock object from the map

        {
            STAFMutexSemLock mapLock(sLockMapSem);
            pLock = &sLockMap[filename];
        }

        // Now, get the underlying file system lock if needed, and increment
        // the number of owners

        {
            STAFMutexSemLock lockLock(*pLock->lockSem);

            if (pLock->lock == 0)
            {
                STAFRC_t rc = STAFFSOSGetExclusiveFileLock(filenameT,
                                                           &pLock->lock, osRC);
                if (rc != 0) return rc;
            }

            ++pLock->numOwners;
        }

        // Finally, get a write lock

        pLock->rwSem->writeLock();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryWriteUnlock(STAFFSEntry_t entry, unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t filenameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &filenameT, osRC);

        if (rc) return rc;

        STAFString filename(filenameT);
        FileLock *pLock = 0;

        // First get the lock object from the map

        {
            STAFMutexSemLock mapLock(sLockMapSem);
            pLock = &sLockMap[filename];
        }

        // Now, decrement the number of owners and release the underlying file
        // system lock if necessary

        STAFMutexSemLock lockLock(*pLock->lockSem);

        if (--pLock->numOwners == 0)
        {
            STAFRC_t rc = STAFFSOSReleaseExclusiveFileLock(&pLock->lock, osRC);

            if (rc != 0) return rc;

            pLock->lock = 0;
        }

        // Finally, release the write lock

        pLock->rwSem->writeUnlock();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSRequestCurrentDirectoryLock()
{
    STAFRC_t retCode = kSTAFOk;

    try
    {
        sCurrDirSem.request();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSReleaseCurrentDirectoryLock()
{
    STAFRC_t retCode = kSTAFOk;

    try
    {
        sCurrDirSem.release();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSCopyEntryCommon(STAFFSEntry_t source, STAFStringConst_t target,
                               unsigned int *osRC)
{
    if (source == 0) return kSTAFInvalidObject;
    if (target == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t filenameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(source, &filenameT, osRC);

        if (rc) return rc;

        STAFString filename(filenameT);

        fstream infile(STAFString(filenameT).toCurrentCodePage()->buffer(),
                       ios::in | STAF_ios_binary);
        fstream outfile(STAFString(target).toCurrentCodePage()->buffer(),
                        ios::out | STAF_ios_binary | ios::trunc);

        if (!infile || !outfile) return kSTAFFileOpenError;

        char input = 0;

        while (infile.read(&input, 1)) outfile.write(&input, 1);
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}
