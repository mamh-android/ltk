/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <deque>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <dirent.h>
#include "STAFFileSystem.h"
#include "STAFMutexSem.h"

// Types

struct STAFFSEntryImpl
{
    STAFFSEntryImpl(const STAFString &thePathString, STAFFSEntryType_t theType,
                    unsigned int isALink, off_t theSize, time_t theModTime,
                    const STAFString &theLinkTarget)
        : pathString(thePathString), type(theType), isLink(isALink),
        size(theSize), modTime(theModTime), linkTarget(theLinkTarget)
    { /* Do Nothing*/ }

    STAFString pathString;
    STAFFSEntryType_t type;
    unsigned int isLink;     // 1 = Link;  0 = Not a link
    off_t size;
    time_t modTime;
    STAFString linkTarget;
};

typedef std::deque<STAFFSEntry_t> STAFFSEntryList;

struct STAFFSEnumHandleImpl
{
    STAFFSEntryList entries;
};

struct STAFFSOSFileLockImpl
{
    STAFFSOSFileLockImpl(int theFileDes) : fileDes(theFileDes)
    { /* Do Nothing */ }

    int fileDes;
};


// Some global strings

static STAFString sSlash(kUTF8_SLASH);
static STAFString sPeriod(kUTF8_PERIOD);
static STAFString sDoublePeriod(sPeriod + sPeriod);

// Semaphore for directory enumeration
static STAFMutexSem sEnumSem;


/*****************************************************************************/
/*                               Helper APIs                                 */
/*****************************************************************************/

struct STAFSortEnumByName
{
    STAFSortEnumByName(STAFFSCaseSensitive_t caseSensitive)
        : fCaseSensitive(caseSensitive)
    { /* Do nothing */ }

    bool operator()(STAFFSEntry_t lhs, STAFFSEntry_t rhs)
    {
        unsigned int comp = 0;

        if (fCaseSensitive == kSTAFFSCaseSensitive)
        {
            STAFStringCompareTo(lhs->pathString.getImpl(),
                                rhs->pathString.getImpl(), &comp, 0);
        }
        else
        {
            STAFStringCompareTo(lhs->pathString.toUpperCase().getImpl(),
                                rhs->pathString.toUpperCase().getImpl(),
                                &comp, 0);
        }

        return (comp == 1);
    }

    STAFFSCaseSensitive_t fCaseSensitive;
};


static bool sortEnumBySize(STAFFSEntry_t lhs, STAFFSEntry_t rhs)
{
    return (lhs->size < rhs->size);
}


static bool sortEnumByModTime(STAFFSEntry_t lhs, STAFFSEntry_t rhs)
{
    return (STAFTimestamp(lhs->modTime) < STAFTimestamp(rhs->modTime));
}


// Remove any trailing slashes (that are not also leading slashes) from a path
// and return the updated string.

static STAFString removeTrailingSlashes(const STAFString &path)
{
    STAFString newPath = path;

    if (newPath.findFirstNotOf(sSlash, STAFString::kChar) !=
        STAFString::kNPos)
    {
        unsigned int lastNonSlashLoc =
            newPath.findLastNotOf(sSlash, STAFString::kChar);

        if (lastNonSlashLoc + 1 != newPath.length())
        {
            newPath = newPath.subString(0, lastNonSlashLoc + 1,
                                        STAFString::kChar);
        }
    }

    return newPath;
}


/*****************************************************************************/
/*                         General File System APIs                          */
/*****************************************************************************/

//STAFRC_t STAFFSInfo(STAFString_t *info, STAFFSInfoType_t infoType)
STAFRC_t STAFFSInfo(void *info, STAFFSInfoType_t infoType)
{
    if (info == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        switch (infoType)
        {
            case kSTAFFSPathSep:
            {
                *reinterpret_cast<STAFString_t *>(info) =
                    STAFString(kUTF8_COLON).adoptImpl();
                break;
            }
            case kSTAFFSFileSep:
            {
                *reinterpret_cast<STAFString_t *>(info) =
                    STAFString(kUTF8_SLASH).adoptImpl();
                break;
            }
            case kSTAFFSLineSep:
            {
                *reinterpret_cast<STAFString_t *>(info) =
                    STAFString(kUTF8_LF).adoptImpl();
                break;
            }
            case kSTAFFSCaseSensitivity:
            {
                *reinterpret_cast<STAFFSCaseSensitive_t *>(info) =
                    kSTAFFSCaseSensitive;
                break;
            }
            default:
            {
                retCode = kSTAFInvalidParm;
            }
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


/*****************************************************************************/
/*                                Path APIs                                  */
/*****************************************************************************/

STAFRC_t STAFFSAssemblePath(STAFString_t *path, STAFStringConst_t root,
                            unsigned int numDirs, STAFStringConst_t *dirs,
                            STAFStringConst_t name,
                            STAFStringConst_t extension)
{
    if (path == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFString theRoot(root);
        STAFString theName(name);
        STAFString theExtension(extension);
        STAFString result;

        // If a root was specified add it

        if (theRoot.length() != 0) result += theRoot;

        // Add the directories

        if (numDirs != 0)
        {
            // Add a slash if we need it

            if ((result.length() != 0) &&
                (result.subString(result.length(STAFString::kChar) - 1,
                                  STAFString::kRemainder, STAFString::kChar)
                 != sSlash))
            {
                result += sSlash;
            }

            // Tack on each directory and a slash, except the last one

            for (unsigned int i = 0; i < (numDirs - 1); ++i)
            {
                result += dirs[i];
                result += sSlash;
            }

            // Add the last directory without a slash

            result += dirs[numDirs - 1];
        }

        if ((theName.length() != 0) || (theExtension.length() != 0))
        {
            // Add a slash if we need it

            if ((result.length() != 0) &&
                (result.subString(result.length(STAFString::kChar) - 1,
                                  STAFString::kRemainder, STAFString::kChar)
                 != sSlash))
            {
                result += sSlash;
            }

            // Add the name

            if (theName.length() != 0) result += theName;

            // Add the extension

            if (theExtension.length() != 0)
            {
                result += sPeriod;
                result += theExtension;        
            }
        }

        *path = result.adoptImpl();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSDisassemblePath(STAFStringConst_t path, STAFString_t *root,
                               unsigned int *numDirs, STAFString_t **dirs,
                               STAFString_t *name, STAFString_t *extension)
{
    if (path == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFString thePath(path);

        // See if they have a root slash present.  Remove it, if present.

        STAFString theRoot;

        if (thePath.subString(0, 1, STAFString::kChar) == sSlash)
        {
            theRoot = sSlash;
            thePath = thePath.subString(thePath.findFirstNotOf(sSlash));
        }

        if (root) *root = theRoot.adoptImpl();

        // Remove any trailing slashes (that are not also leading slashes)

        thePath = removeTrailingSlashes(thePath);

        // Now, find all the directories

        std::deque<STAFString> theDirs;

        for (unsigned int slashLoc = thePath.find(sSlash);
             slashLoc != STAFString::kNPos;
             slashLoc = thePath.find(sSlash))
        {
            theDirs.push_back(thePath.subString(0, slashLoc));
            thePath = thePath.subString(thePath.findFirstNotOf(sSlash,
                                                               slashLoc));
        }

        if (numDirs) *numDirs = theDirs.size();

        if (dirs)
        {
            if (theDirs.size() == 0) *dirs = 0;
            else *dirs = new STAFString_t[theDirs.size()];
        
            for (unsigned int i = 0; i < theDirs.size(); ++i)
                (*dirs)[i] = theDirs[i].adoptImpl();
        }
    
        // If the name is "." or ".." then leave it alone, otherwise
        // separate the name and the extension

        if ((thePath == sPeriod) || (thePath == sDoublePeriod))
        {
            if (name) *name = thePath.adoptImpl();
            if (extension) *extension = STAFString().adoptImpl();
        }
        else
        {
            // Next find and remove the extension

            STAFString theExtension;
            unsigned int extLoc = thePath.findLastOf(sPeriod);

            // Note:  If find a period and it is the last character
            // (e.g. grannyapple.), keep the period as part of the name

            if ((extLoc != STAFString::kNPos) &&
                (extLoc != (thePath.length() - 1)))
            {
                theExtension = thePath.subString(extLoc +
                                                 thePath.sizeOfChar(extLoc));
                thePath = thePath.subString(0, extLoc);
            }

            if (extension) *extension = theExtension.adoptImpl();

            // We are now left with the name

            if (name) *name = thePath.adoptImpl();
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError;}

    return retCode;
}


STAFRC_t STAFFSFreePathDirs(STAFString_t *dirs)
{
    delete [] dirs;

    return kSTAFOk;
}


/*****************************************************************************/
/*                          File System Entry APIs                           */
/*****************************************************************************/

STAFRC_t STAFFSExists(STAFStringConst_t path, unsigned int *exists,
                      unsigned int *osRC)
{
    if (path == 0) return kSTAFInvalidParm;
    if (exists == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        // Remove any trailing slashes (that are not also leading slashes)

        STAFString thePath = removeTrailingSlashes(path);

        struct stat data = { 0 };
        int rc = stat(thePath.toCurrentCodePage()->buffer(), &data);

        if ((rc == 0) || (errno == EOVERFLOW))
        {
            // If an EOVERFLOW error occurs, we know the file exists, even
            // though a value was too large to be stored in the stat structure

            *exists = 1;
        }
        else if (errno == ENOENT)
        {
            *exists = 0;
        }
        else
        {
            if (osRC) *osRC = errno;
            retCode = kSTAFBaseOSError;
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSGetEntry(STAFStringConst_t path, STAFFSEntry_t *entry,
                        unsigned int *osRC)
{
    if ((path == 0) || (entry == 0)) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        // Remove any trailing slashes (that are not also leading slashes)

        STAFString thePath = removeTrailingSlashes(path);

        struct stat data = { 0 };
        int rc = lstat(thePath.toCurrentCodePage()->buffer(), &data);

        if (rc < 0)
        {
            if (osRC) *osRC = errno;
            return kSTAFBaseOSError;
        }

        unsigned int isLink = 0;
        STAFString theLinkTarget("");

        if (S_ISLNK(data.st_mode))
        {
            isLink = 1;
            theLinkTarget = STAFString("<Unknown>");

            rc = stat(thePath.toCurrentCodePage()->buffer(), &data);

            // Get the name of the target referred to by the symbolic link
            
#ifndef STAF_OS_NAME_ZOS
            char buf[PATH_MAX + 1] = { 0 };
#else            
            // XXX: Get an error compiling on z/OS if use PATH_MAX
            char buf[256] = { 0 };
#endif
            int len = readlink(thePath.toCurrentCodePage()->buffer(),
                               buf, sizeof(buf)-1);
            if (len < 0)
                theLinkTarget = theLinkTarget + STAFString(" OSRC:" + errno);
            else
                theLinkTarget = STAFString(buf);
        }

        STAFFSEntryType_t theType = kSTAFFSOther;

        if (S_ISREG(data.st_mode))       theType = kSTAFFSFile;
        else if (S_ISDIR(data.st_mode))  theType = kSTAFFSDirectory;
        else if (S_ISFIFO(data.st_mode)) theType = kSTAFFSPipe;
        else if (S_ISSOCK(data.st_mode)) theType = kSTAFFSSocket;
        else if (S_ISCHR(data.st_mode))  theType = kSTAFFSCharDev;
        else if (S_ISBLK(data.st_mode))  theType = kSTAFFSBlkDev;
        else if (S_ISLNK(data.st_mode))  theType = kSTAFFSSymLink;

        *entry = new STAFFSEntryImpl(thePath, theType, isLink, data.st_size,
                                     data.st_mtime, theLinkTarget);
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetPathString(STAFFSEntry_t entry,
                                  STAFStringConst_t *pathString,
                                  unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;
    if (pathString == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        *pathString = entry->pathString.getImpl();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetType(STAFFSEntry_t entry, STAFFSEntryType_t *type,
                            unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;
    if (type == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        *type = entry->type;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetSize(STAFFSEntry_t entry, unsigned int *upperSize,
                            unsigned int *lowerSize, unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        if (sizeof(off_t) > sizeof(unsigned int))
        {
            *upperSize = (entry->size >> 32) & 0xFFFFFFFF;
            *lowerSize = entry->size & 0xFFFFFFFF;
        }
        else
        {
            *upperSize = 0;
            *lowerSize = entry->size;
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetSize64(STAFFSEntry_t entry, STAFUInt64_t *size,
                              unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        *size = entry->size;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetModTime(STAFFSEntry_t entry, time_t *modTime,
                               unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        *modTime = entry->modTime;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetIsLink(STAFFSEntry_t entry, unsigned int *isLink,
                              unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        *isLink = entry->isLink;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEntryGetLinkTarget(STAFFSEntry_t entry,
                                  STAFString_t *linkTargetString,
                                  unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;
    if (linkTargetString == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        *linkTargetString = entry->linkTarget.getImpl();
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSDeleteEntry(STAFFSEntry_t entry, unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t filenameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &filenameT, osRC);

        if (rc) return rc;

        STAFString filename(filenameT);

        if ((entry->type == kSTAFFSFile) || entry->isLink)
        {
            if (unlink(filename.toCurrentCodePage()->buffer()) != 0)
            {
                if (osRC) *osRC = errno;
                retCode = kSTAFBaseOSError;
            }
        }
        else
        {
            if (rmdir(filename.toCurrentCodePage()->buffer()) != 0)
            {
                if (errno == ENOTEMPTY)
                {
                    retCode = kSTAFDirectoryNotEmpty;
                }
                else
                {
                    // Solaris returns a EEXIST instead of ENOTEMPTY like other
                    // Unix OS's if the directory is not empty.

                    #if defined(STAF_OS_NAME_SOLARIS) || defined(STAF_OS_NAME_HPUX)
                        if (errno == EEXIST) retCode = kSTAFDirectoryNotEmpty;
                    #endif

                    if (retCode == kSTAFOk)
                    {
                        if (osRC) *osRC = errno;
                        retCode = kSTAFBaseOSError;
                    }
                }
            }
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSCopyEntry(STAFFSEntry_t source, STAFStringConst_t target,
                         unsigned int *osRC)
{
    return STAFFSCopyEntryCommon(source, target, osRC);
}


STAFRC_t STAFFSMoveEntry(STAFFSEntry_t entry, STAFStringConst_t toName,
                         unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;
    if (toName == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t fromNameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &fromNameT, osRC);

        if (rc) return rc;

        STAFString fromname(fromNameT);
            
        if (rename(fromname.toCurrentCodePage()->buffer(),
                   STAFString(toName).toCurrentCodePage()->buffer()) != 0)
        {
            if (osRC) *osRC = errno;
            retCode = kSTAFBaseOSError;
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSRenameEntry(STAFFSEntry_t entry, STAFStringConst_t toName,
                           unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;
    if (toName == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFStringConst_t fromNameT = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &fromNameT, osRC);

        if (rc) return rc;
        
        // Check if the toName entry already exists and if so, return an
        // already exists error.

        unsigned int doesExist = 0;
        rc = STAFFSExists(toName, &doesExist, osRC);
        
        if (rc) return rc;

        if (doesExist) return kSTAFAlreadyExists;
        
        // Rename the entry to its new name.

        STAFString fromname(fromNameT);
            
        if (rename(fromname.toCurrentCodePage()->buffer(),
                   STAFString(toName).toCurrentCodePage()->buffer()) != 0)
        {
            if (osRC) *osRC = errno;
            retCode = kSTAFBaseOSError;
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSOSGetExclusiveFileLock(STAFStringConst_t path,
                                      STAFFSOSFileLock_t *lock,
                                      unsigned int *osRC)
{
    if (path == 0) return kSTAFInvalidParm;
    if (lock == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFString filename(path);
        struct flock theLock = { 0 };
        int theFile = 0;

        theLock.l_type = F_WRLCK;

        if ((theFile = open(filename.toCurrentCodePage()->buffer(),
                            O_RDWR)) < 0)
        {
            if (osRC) *osRC = errno;
            return kSTAFBaseOSError;
        }

        int rc2 = 0;

        do
        {
            rc2 = fcntl(theFile, F_SETLKW, &theLock);
        } while ((rc2 != 0) && (errno == EINTR));

        if (rc2 != 0)
        {
            if (osRC) *osRC = errno;
            close(theFile);
            return kSTAFBaseOSError;
        }

        *lock = new STAFFSOSFileLockImpl(theFile);
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSOSReleaseExclusiveFileLock(STAFFSOSFileLock_t *lock,
                                          unsigned int *osRC)
{
    if (lock == 0) return kSTAFInvalidParm;
    if (*lock == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        int rc = 0;
        struct flock theLock = { 0 };

        theLock.l_type = F_UNLCK;

        do
        {
            rc = fcntl((**lock).fileDes, F_SETLKW, &theLock);
        } while ((rc != 0) && (errno == EINTR));

        if (rc != 0)
        {
            // Note: We fall through here so that we can close the file and
            //       free the lock

            if (osRC) *osRC = errno;
            retCode = kSTAFBaseOSError;
        }

        close((**lock).fileDes);

        delete *lock;

        *lock = 0;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSFreeEntry(STAFFSEntry_t *entry)
{
    if (entry == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        delete *entry;
        *entry = 0;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


/*****************************************************************************/
/*                             Directory APIs                                */
/*****************************************************************************/

STAFRC_t STAFFSCreateDirectory(STAFStringConst_t path,
                               STAFFSDirectoryCreateMode_t flags,
                               unsigned int *osRC)
{
    if (path == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFString directory(path);
        struct stat data = { 0 };
        int rc = stat(directory.toCurrentCodePage()->buffer(), &data);

        if ((rc == 0) || (errno == EOVERFLOW))
        {
            // If an EOVERFLOW error occurs, we know the directory exists, even
            // though a value was too large to be stored in the stat structure

            return kSTAFAlreadyExists;
        }
        
        if (errno != ENOENT)
        {
            if (osRC) *osRC = errno;
            retCode = kSTAFBaseOSError;
        }

        if (flags == kSTAFFSCreateDirOnly)
        {
            int rc = mkdir(directory.toCurrentCodePage()->buffer(),
                           S_IRWXU | S_IRWXG | S_IRWXO);

            if (rc < 0)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }
        }
        else  // Create entire path
        {
            STAFFSPath fsPath(path);
            STAFFSPath currPath;

            currPath.setRoot(fsPath.root());

            for (unsigned int i = 0; i < fsPath.numDirs(); ++i)
            {
                currPath.addDir(fsPath.dir(i));

                mkdir(currPath.asString().toCurrentCodePage()->buffer(),
                      S_IRWXU | S_IRWXG | S_IRWXO);
            }

            if (mkdir(fsPath.asString().toCurrentCodePage()->buffer(),
                      S_IRWXU | S_IRWXG | S_IRWXO) < 0)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSGetCurrentDirectory(STAFString_t *path, unsigned int *osRC)
{
    if (path == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSSetCurrentDirectory(STAFStringConst_t path, unsigned int *osRC)
{
    if (path == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEnumOpen(STAFFSEnumHandle_t *enumHandle, STAFFSEntry_t entry,
                        STAFStringConst_t namePattern,
                        STAFStringConst_t extPattern,
                        STAFFSCaseSensitive_t caseSensitive,
                        STAFFSEntryType_t entryTypes,
                        STAFFSSortBy_t sortBy, unsigned int *osRC)
{
    if (enumHandle == 0) return kSTAFInvalidParm;
    if (entry == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    if (caseSensitive == kSTAFFSCaseDefault)
        STAFFSInfo(&caseSensitive, kSTAFFSCaseSensitivity);

    try
    {
        STAFStringConst_t rootName = 0;
        STAFRC_t rc = STAFFSEntryGetPathString(entry, &rootName, osRC);

        if (rc) return rc;

        STAFFSEnumHandleImpl data;

        STAFString theRootName(rootName);
        STAFString theNamePattern(namePattern);
        STAFString theExtPattern(extPattern);
        
        // We need to get the enumeration semaphore so that no other enum
        // operation can mess us up
        
        STAFMutexSemLock lock(sEnumSem);
        DIR *rootDir = opendir(theRootName.toCurrentCodePage()->buffer());

        // If we get an error other than this isn't a directory, then
        // return the error

        if (!rootDir && (errno != ENOTDIR))
        {
            if (osRC) *osRC = errno;
            return kSTAFBaseOSError;
        }
        else if (rootDir)
        {
            // Loop through all the entries in the directory and see if they
            // match the name and extension patterns

            for (struct dirent *anEntry = readdir(rootDir); anEntry != 0;
                 anEntry = readdir(rootDir))
            {
                // Ignore . and .. unless the user specifically requests them

                if (!(entryTypes & kSTAFFSSpecialDirectory) &&
                    ((STAFString(anEntry->d_name) == sPeriod) ||
                     (STAFString(anEntry->d_name) == sDoublePeriod)))
                {
                    continue;
                }

                STAFString pathName(theRootName);

                pathName += STAFString(kUTF8_SLASH);
                pathName += anEntry->d_name;

                STAFFSPath path(pathName);

                if ((STAFFileSystem::matchesWildcards(path.name(),
                                                      theNamePattern,
                                                      caseSensitive)) &&
                    (STAFFileSystem::matchesWildcards(path.extension(),
                                                      theExtPattern,
                                                      caseSensitive)))
                {
                    // Now, make sure that we can get the associated entry and
                    // that it matches the requested types

                    STAFFSEntry_t goodEntry = 0;
                    STAFFSEntryType_t entryType = kSTAFFSOther;

                    rc = STAFFSGetEntry(path.asString().getImpl(),
                                        &goodEntry, 0);

                    if (rc != 0) continue;

                    rc = STAFFSEntryGetType(goodEntry, &entryType, 0);

                    if ((rc == 0) && (entryType & entryTypes))
                        data.entries.push_back(goodEntry);
                    else
                        STAFFSFreeEntry(&goodEntry);
                }
            }

            closedir(rootDir);
        }

        switch (sortBy)
        {
            case kSTAFFSSortByName:
            {
                std::sort(data.entries.begin(), data.entries.end(),
                          STAFSortEnumByName(caseSensitive));
                break;
            }

            case kSTAFFSSortBySize:
            {
                std::sort(data.entries.begin(), data.entries.end(),
                          sortEnumBySize);
                break;
            }

            case kSTAFFSSortByModTime:
            {
                std::sort(data.entries.begin(), data.entries.end(),
                          sortEnumByModTime);
                break;
            }

            default: break;
        }

        *enumHandle = new STAFFSEnumHandleImpl(data);
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEnumNext(STAFFSEnumHandle_t enumHandle, STAFFSEntry_t *entry,
                        unsigned int *osRC)
{
    if (enumHandle == 0) return kSTAFInvalidObject;
    if (entry == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        if (enumHandle->entries.size() == 0)
        {
            *entry = 0;
        }
        else
        {
            *entry = enumHandle->entries.front();
            enumHandle->entries.pop_front();
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFFSEnumClose(STAFFSEnumHandle_t *enumHandle, unsigned int *osRC)
{
    if (enumHandle == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        for (STAFFSEntryList::iterator iter = (*enumHandle)->entries.begin();
             iter != (*enumHandle)->entries.end(); ++iter)
        {
            STAFFSEntry_t entry = *iter;

            STAFFSFreeEntry(&entry);
        }

        delete *enumHandle;

        *enumHandle = 0;
    }
    catch (...)
    { retCode = kSTAFUnknownError; }
    
    return retCode;
}

