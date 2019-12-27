/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <deque>
#include <algorithm>
#include "STAFFileSystem.h"
#include "STAFThreadManager.h"
#include "STAFUtilWin32.h"

// Types

struct STAFFSEntryImpl
{
    STAFFSEntryImpl(const STAFString &thePathString, STAFFSEntryType_t theType,
                    unsigned int theHighSize, unsigned int theLowSize,
                    time_t theModTime)
        : pathString(thePathString), type(theType), highSize(theHighSize),
          lowSize(theLowSize), modTime(theModTime)
    { /* Do Nothing */ }

    STAFString pathString;
    STAFFSEntryType_t type;
    unsigned int highSize;
    unsigned int lowSize;
    time_t modTime;
};

typedef std::deque<STAFFSEntry_t> STAFFSEntryList;

struct STAFFSEnumHandleImpl
{
    STAFFSEntryList entries;
};

struct STAFFSOSFileLockImpl
{
    STAFFSOSFileLockImpl(HANDLE theHandle) : handle(theHandle)
    { /* Do Nothing */ }

    HANDLE handle;
};


// Some global strings

static STAFString sSlash(kUTF8_SLASH);
static STAFString sBackSlash(kUTF8_BSLASH);
static STAFString sBothSlash(sSlash + sBackSlash);
static STAFString s2BackSlashes(sBackSlash + sBackSlash);
static STAFString s2Slashes(sSlash + sSlash);
static STAFString sPeriod(kUTF8_PERIOD);
static STAFString sColon(kUTF8_COLON);
static STAFString sStar(kUTF8_STAR);
static STAFString sCR(kUTF8_CR);
static STAFString sLF(kUTF8_LF);


/*****************************************************************************/
/*                               Helper APIs                                 */
/*****************************************************************************/
static bool isWinNT()
{
    static STAFMutexSem helperSem;
    static bool haveChecked = false;
    static bool rc = false;

    if (!haveChecked)
    {
        STAFMutexSemLock lock(helperSem);

        if (!haveChecked)
        {
            OSVERSIONINFO osVersionInfo = { sizeof(OSVERSIONINFO), 0 };

            if ((GetVersionEx(&osVersionInfo) == TRUE) && 
                (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))
            {
                rc = true;
            }

            haveChecked = true;
        }
    }

    return rc;
}


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
    if (lhs->highSize == rhs->highSize)
        return (lhs->lowSize < rhs->lowSize);

    return (lhs->highSize < rhs->highSize);
}


static bool sortEnumByModTime(STAFFSEntry_t lhs, STAFFSEntry_t rhs)
{
    return (STAFTimestamp(lhs->modTime) < STAFTimestamp(rhs->modTime));
}


static bool hasTrailingSlash(const STAFString &path)
{
    unsigned int lastSlashLoc = path.findLastOf(
        sBothSlash, STAFString::kChar);

    return ((lastSlashLoc + 1) == path.length());
}


// Check if a path is a root directory.  Examples of root directories are:
//   C:\  or  C:/  or  \\computername\c$\  or  //computername/c$/

static bool isRootDirectory(const STAFString &path)
{
    STAFString newPath = path;

    if (newPath.findFirstNotOf(sBothSlash, STAFString::kChar) !=
        STAFString::kNPos)
    {
        unsigned int lastNonSlashLoc =
            newPath.findLastNotOf(sBothSlash, STAFString::kChar);

        if (lastNonSlashLoc + 1 != newPath.length())
        {
            newPath = newPath.subString(0, lastNonSlashLoc + 1,
                                        STAFString::kChar);

            // We have to see if a root directory was specified
            //   C:\ or C:/ or \\computername\c$\ or //computername/c$
            // If so, we need to add a trailing slash back to the end
            // so that it doesn't become a relative path

            if ((newPath.length() == 2) &&
                (newPath.subString(1, 1) == kUTF8_COLON))
            {
                return true;
            }
            else if (newPath.subString(0, 2, STAFString::kChar) == s2BackSlashes ||
                     newPath.subString(0, 2, STAFString::kChar) == s2Slashes)
            {
                // Check if the path starts with a share name using format:
                //   \\computername\sharename
                //   or
                //   //computername/sharename
                // If so, we need to add a trailing slash back to the end
                // so that it doesn't become a relative path

                unsigned int nextSlashPos = newPath.findFirstOf(
                    sBothSlash, 2, STAFString::kChar);

                if ((nextSlashPos != STAFString::kNPos) && (nextSlashPos >= 3))
                {
                    // Has format \\...\

                    unsigned int startShareNamePos = newPath.findFirstNotOf(
                        sBothSlash, nextSlashPos + 1, STAFString::kChar);

                    if (startShareNamePos != STAFString::kNPos)
                    {
                        unsigned int endShareNamePos = newPath.findFirstOf(
                            sBothSlash, startShareNamePos, STAFString::kChar);

                        if (endShareNamePos == STAFString::kNPos)
                        {
                            // Has format \\...\...

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}


// Remove any trailing slashes (that are not also leading slashes and are
// not trailing slashes for a root directory) from a path and return the
// updated string.  For example:
//   C:\\mydir\\           -> C:\\mydir
//   C:/mydir/myfile.txt/  -> C:/mydir/myfile.txt
// Note the following are root directories so can't remove trailing slashes
// because then would become relative paths
//   C:\  or C:/  -> C:\
//   \\computername\c$\   -> \\computername\c$\

static STAFString removeTrailingSlashes(const STAFString &path)
{
    STAFString newPath = path;

    if (newPath.findFirstNotOf(sBothSlash, STAFString::kChar) !=
        STAFString::kNPos)
    {
        unsigned int lastNonSlashLoc =
            newPath.findLastNotOf(sBothSlash, STAFString::kChar);

        if (lastNonSlashLoc + 1 != newPath.length())
        {
            newPath = newPath.subString(0, lastNonSlashLoc + 1,
                                        STAFString::kChar);

            // We have to see if a root directory was specified
            //   C:\ or C:/ or \\computername\c$\ or //computername/c$
            // If so, we need to add a trailing slash back to the end
            // so that it doesn't become a relative path

            if ((newPath.length() == 2) &&
                (newPath.subString(1, 1) == kUTF8_COLON))
            {
                newPath += sBackSlash;
            }
            else if ((newPath.subString(0, 2, STAFString::kChar) ==
                      s2BackSlashes) ||
                     (newPath.subString(0, 2, STAFString::kChar) ==
                      s2Slashes))
            {
                // Check if the path starts with a share name using format:
                //   \\computername\sharename
                //   or
                //   //computername/sharename
                // If so, we need to add a trailing slash back to the end
                // so that it doesn't become a relative path

                unsigned int nextSlashPos = newPath.findFirstOf(
                    sBothSlash, 2, STAFString::kChar);

                if ((nextSlashPos != STAFString::kNPos) && (nextSlashPos >= 3))
                {
                    // Has format \\...\

                    unsigned int startShareNamePos = newPath.findFirstNotOf(
                        sBothSlash, nextSlashPos + 1, STAFString::kChar);

                    if (startShareNamePos != STAFString::kNPos)
                    {
                        unsigned int endShareNamePos = newPath.findFirstOf(
                            sBothSlash, startShareNamePos, STAFString::kChar);

                        if (endShareNamePos == STAFString::kNPos)
                        {
                            // Has format \\...\...

                            newPath += sBackSlash;
                        }
                    }
                }
            }
        }
    }

    return newPath;
}


// Retrieves the full path name for the specified path and if the path
// exists, converts it to its long form in the correct case and using a
// backslash as the file separator and returns the updated path.

static STAFString getFullLongPath(const STAFString &thePath)
{
    // Remove any trailing slashes (that are not also leading slashes)
    // except for a root directory

    STAFString orgPath = removeTrailingSlashes(thePath);

    // The GetFullPathName() API is not supported for Windows 95/98

    if (!(STAFUtilWin32GetWinType() & kSTAFWin2KPlus))
        return orgPath;
    
    // Retrieves the full path and file name of the specified file.
    // Merges the name of the current drive and directory with a specified
    // file name to determine the full path and file name of a specified file. 
    // Converts relative path names to absolute path names and resolves any .
    // and .. in the path name.

    TCHAR buffer[4096] = TEXT("");
    TCHAR** lppPart = {NULL};

    unsigned int length = GetFullPathName(
        orgPath.toCurrentCodePage()->buffer(), 4096, buffer, lppPart);

    if (length == 0)
    {
        return orgPath;
    }
    
    buffer[length] = 0; // Add the trailing null
    STAFString path = STAFString(buffer);

    // If the path starts with a drive letter and it's not upper-case,
    // convert it to upper case as GetLongPathName doesn't always do this

    if ((path.length() >= 2) &&
        (path.subString(1, 1, STAFString::kChar) == sColon) &&
        (path.subString(0, 1, STAFString::kChar) !=
         path.subString(0, 1, STAFString::kChar).toUpperCase()))
    {
        path = path.subString(0, 1, STAFString::kChar).toUpperCase() +
            path.subString(1, STAFString::kRemainder, STAFString::kChar);
    }

    // If the path exists, converts the specified path to its long form
    // and the correct case, and if the path contains "short" names, they
    // will be expanded to the "long" name.  For example:
    //   C:\PROGRA~1\MICROS~1 -> C:\Program Files\microsoft frontpage

    length = GetLongPathName(
        path.toCurrentCodePage()->buffer(), buffer, 4096);

    if (length == 0)
    {
        return path;
    }
        
    buffer[length] = 0; // Add the trailing null
    path = STAFString(buffer);

    return path;
}


/*****************************************************************************/
/*                         General File System APIs                          */
/*****************************************************************************/

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
                    STAFString(kUTF8_SCOLON).adoptImpl();
                break;
            }
            case kSTAFFSFileSep:
            {
                *reinterpret_cast<STAFString_t *>(info) =
                    STAFString(kUTF8_BSLASH).adoptImpl();
                break;
            }
            case kSTAFFSLineSep:
            {
                *reinterpret_cast<STAFString_t *>(info) =
                    STAFString(sCR + sLF).adoptImpl();
                break;
            }
            case kSTAFFSCaseSensitivity:
            {
                *reinterpret_cast<STAFFSCaseSensitive_t *>(info) =
                    kSTAFFSCaseInsensitive;
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
            // Add a backslash if we need it

            if ((result.length() != 0) &&
                (result.subString(result.length(STAFString::kChar) - 1,
                                  STAFString::kRemainder, STAFString::kChar)
                 != sBackSlash))
            {
                result += sBackSlash;
            }

            // Tack on each directory and a backslash, except the last one

            for (unsigned int i = 0; i < (numDirs - 1); ++i)
            {
                result += dirs[i];
                result += sBackSlash;
            }

            // Add the last directory without a backslash

            result += dirs[numDirs - 1];
        }

        if ((theName.length() != 0) || (theExtension.length() != 0))
        {
            // Add a backslash if we need it

            if ((result.length() != 0) &&
                (result.subString(result.length(STAFString::kChar) - 1,
                                  STAFString::kRemainder, STAFString::kChar)
                 != sBackSlash))
            {
                result += sBackSlash;
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
        STAFString orgPath(path);

        // Get the full path and, if the path exists, convert the path to the
        // long path name in the correct case

        STAFString thePath = getFullLongPath(orgPath);

        // See if they have a root slash present.  Remove it, if present.

        STAFString theRoot;

        if (thePath.subString(1, 1, STAFString::kChar) == sColon)
        {
            // Check if the path begins with a drive letter and a colon
            // If so, assign the root to be <Drive>:

            theRoot = thePath.subString(0, 2, STAFString::kChar);
            thePath = thePath.subString(thePath.findFirstNotOf(sBothSlash,
                                        2, STAFString::kChar));

            
            // If root directory, add trailing slash back to root

            if ((thePath.length() == 0) && hasTrailingSlash(orgPath))
            {
                theRoot += sBackSlash;
            }
        }
        else if (thePath.subString(0, 2, STAFString::kChar) == s2BackSlashes ||
                 thePath.subString(0, 2, STAFString::kChar) == s2Slashes)
        {
            // Check if the path starts with a share name using format:
            //   \\computername\sharename
            //   or
            //   //computername/sharename
            // If so, assign the root to be \\computername\sharename

            unsigned int nextSlashPos = thePath.findFirstOf(
                sBothSlash, 2, STAFString::kChar);

            if ((nextSlashPos != STAFString::kNPos) && (nextSlashPos >= 3))
            {
                // Has format \\...\

                unsigned int startShareNamePos = thePath.findFirstNotOf(
                    sBothSlash, nextSlashPos + 1, STAFString::kChar);

                if (startShareNamePos != STAFString::kNPos)
                {
                    unsigned int endShareNamePos = thePath.findFirstOf(
                        sBothSlash, startShareNamePos, STAFString::kChar);

                    if (endShareNamePos != STAFString::kNPos)
                    {
                        // Has format \\...\...\[...]

                        theRoot = thePath.subString(0, endShareNamePos);
                        thePath = thePath.subString(endShareNamePos + 1);
                    }
                    else
                    {
                        // Has format \\...\...

                        theRoot = thePath;
                        thePath = "";

                        // If root directory, add trailing slash back to root

                        if (hasTrailingSlash(orgPath))
                        {
                            theRoot += sBackSlash;
                        }
                    }
                }
            }
        }

        // Check if the root has not been set yet and if an absolute path was
        // specified.  Need to do this because otherwise FindFirstFile() will
        // fail if the path is something like "/temp" instead of "C:/temp"

        if ((theRoot.length() == 0) && (thePath.length() > 0) &&
            (thePath.findFirstOf(sBothSlash, 0, STAFString::kChar) == 0))
        {
            // Get the current drive (the drive of the directory from which
            // STAFProc was started), e.g. C:, and set it as the root

            STAFConfigInfo configInfo;
            STAFString_t errorBufferT;
            unsigned int osRC = 0;

            if (STAFUtilGetConfigInfo(&configInfo, &errorBufferT, &osRC) ==
                kSTAFOk)
            {
                theRoot = STAFString(configInfo.currentDrive);
                thePath = thePath.subString(
                    thePath.findFirstNotOf(sBothSlash, 0, STAFString::kChar));
            }
        }
        
        if (root) *root = theRoot.adoptImpl();

        // Now, find all the directories

        std::deque<STAFString> theDirs;

        for (unsigned int slashLoc = thePath.findFirstOf(sBothSlash);
             slashLoc != STAFString::kNPos;
             slashLoc = thePath.findFirstOf(sBothSlash))
        {
            theDirs.push_back(thePath.subString(0, slashLoc));
            thePath = thePath.subString(thePath.findFirstNotOf(sBothSlash,
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

        if ((thePath == sPeriod) || (thePath == (sPeriod + sPeriod)))
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
        // Get the full path and, if the path exists, convert the path to the
        // long path name in the correct case

        STAFString thePath = getFullLongPath(STAFString(path));

        // We have to see if we have a path like <alpha>[\/]:
        // If so, we can't call FindFirstFile() with a path like that.

        if ((thePath.length() != 2) || (thePath.subString(1,1) != kUTF8_COLON))
        {
            STAFString searchPath(thePath);

            if (isRootDirectory(thePath))
            {
                searchPath += sStar;
            }

            // Try using FindFirstFile() to see if the path exists
            //
            // Note: GetFileAttributes() fails if the file is in use
            //       (e.g. C:\PAGEFILE.SYS), but FindFirstFile() doesn't fail.
            //       However, FindFirstFile() doesn't set the error code to
            //       ERROR_FILE_NOT_FOUND if specify a path that contains a
            //       directory that doesn't exist.  That's why using both
            //       functions to determine if a path exists.

            WIN32_FIND_DATA data = { 0 };
            HANDLE findHandle = FindFirstFile(
                searchPath.toCurrentCodePage()->buffer(), &data);

            if (findHandle != INVALID_HANDLE_VALUE)
            {
                *exists = 1;
                FindClose(findHandle);
                return retCode;
            }
            else if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                *exists = 0;
                return retCode;
            }
        }

        // Try using GetFileAttributes() to see if the path exists
                
        DWORD rc = GetFileAttributes(thePath.toCurrentCodePage()->buffer());

        if (rc == INVALID_FILE_ATTRIBUTES)
            *exists = 0;
        else
            *exists = 1;
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
        STAFString thePath(path);

        // Get the full/long path name in the correct case

        thePath = getFullLongPath(thePath);

        // We have to see if we have a root directory path (e.g. C:\ or
        // \\computername\C$\) or a path like C: 
        // If so, we have to handle it specially as Win32 doesn't like a
        // call to FindFirstFile() with a path like that.

        if (isRootDirectory(thePath) ||
            ((thePath.length() == 2) &&
             (thePath.subString(1, 1) == kUTF8_COLON)))
        {
            *entry = new STAFFSEntryImpl(thePath, kSTAFFSDirectory, 0, 0,
                                         STAFTimestamp::now().getImpl());
            return kSTAFOk;
        }

        WIN32_FIND_DATA data = { 0 };
        HANDLE findHandle = FindFirstFile(
                            thePath.toCurrentCodePage()->buffer(), &data);

        if (findHandle == INVALID_HANDLE_VALUE)
        {
            // Save error information for FindFirstFile()

            unsigned int savedRC = 0;
            if (osRC) savedRC = GetLastError();

            // Since FindFirstFile() failed, try GetFileAttributes() in case
            // thePath points to a root directory of a network share
            // (e.g. \\server\service") because FindFirstFile() cannot handle
            // the root directory of a network share, but GetFileAttributes()
            // can handle the root directory of a network share.

            data.dwFileAttributes = GetFileAttributes(
                thePath.toCurrentCodePage()->buffer());

            // Validate that the entry type is a directory

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // The high and low size for a directory is always 0
                // Don't know the mod time, so just use the current time

                *entry = new STAFFSEntryImpl(thePath, kSTAFFSDirectory, 0, 0,
                                             STAFTimestamp::now().getImpl());

                return kSTAFOk;
            }
            else
            {
                // Return the saved error information from FindFirstFile()
                *osRC = savedRC;
                return kSTAFBaseOSError;
            }
        }

        FindClose(findHandle);

        STAFFSEntryType_t theType = kSTAFFSFile;

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            theType = kSTAFFSDirectory;

        FILETIME localFileTime = { 0 };
        SYSTEMTIME sysWriteTime = { 0 };
        
        if (FileTimeToLocalFileTime(&data.ftLastWriteTime, &localFileTime) == 
                FALSE)
        {
            if (osRC) *osRC = GetLastError();
            return kSTAFBaseOSError;
        }
        
        if (FileTimeToSystemTime(&localFileTime, &sysWriteTime) == FALSE)
        {
            if (osRC) *osRC = GetLastError();
            return kSTAFBaseOSError;
        }       

        STAFTimestamp writeTime(sysWriteTime.wYear, sysWriteTime.wMonth,
                                sysWriteTime.wDay, sysWriteTime.wHour,
                                sysWriteTime.wMinute, sysWriteTime.wSecond);
        
        *entry = new STAFFSEntryImpl(thePath, theType, data.nFileSizeHigh,
                                     data.nFileSizeLow, writeTime.getImpl());
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
        *upperSize = entry->highSize;
        *lowerSize = entry->lowSize;
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
        *size = 0;

        if (entry->highSize != 0)
        {
            *size += static_cast<STAFUInt64_t>(entry->highSize) << 32;
        }

        *size += entry->lowSize;
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

    *isLink = 0;

    return retCode;
}


STAFRC_t STAFFSEntryGetLinkTarget(STAFFSEntry_t entry,
                                  STAFString_t *linkTargetString,
                                  unsigned int *osRC)
{
    if (entry == 0) return kSTAFInvalidObject;
    if (linkTargetString == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;
    STAFString linkTarget = STAFString("");

    try
    {
        *linkTargetString = linkTarget.getImpl();
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

        if (entry->type == kSTAFFSFile)
        {
            if (DeleteFile(filename.toCurrentCodePage()->buffer()) == FALSE)
            {
                if (osRC) *osRC = GetLastError();
                retCode = kSTAFBaseOSError;
            }
        }
        else
        {
            if (RemoveDirectory(filename.toCurrentCodePage()->buffer()) == FALSE)
            {
                if (GetLastError() == ERROR_DIR_NOT_EMPTY)
                {
                    retCode = kSTAFDirectoryNotEmpty;
                }
                else
                {
                    unsigned int saveLastError = GetLastError();

                    if (!isWinNT())
                    {
                        // Windows 95 returns a ERROR_ACCESS_DENIED instead of
                        // ERROR_DIR_NOT_EMPTY if the directory is not empty.
                        // So, enumerate the directory to check if it's really
                        // non-empty.

                        STAFFSPath entryPath(filename);

                        if (entryPath.getEntry()->enumerate(
                                kUTF8_STAR, kUTF8_STAR, kSTAFFSAll)->isValid())
                        {
                            retCode = kSTAFDirectoryNotEmpty;
                        }
                    }

                    if (retCode == kSTAFOk)
                    {
                        if (osRC) *osRC = saveLastError;
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
            
        if (isWinNT())
        {
            if (MoveFileEx(fromname.toCurrentCodePage()->buffer(),
                           STAFString(toName).toCurrentCodePage()->buffer(),
                           MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH |
                           MOVEFILE_COPY_ALLOWED) == FALSE)
            {
                if (osRC) *osRC = GetLastError();
                retCode = kSTAFBaseOSError;
            }
        }
        else
        {
            // Since MoveFileEx is only supported on WinNT and later, copy
            // the file and then delete the file on Win95/98/ME.

            rc = STAFFSCopyEntry(entry, toName, osRC);
            if (rc) return rc;
            
            rc = STAFFSDeleteEntry(entry, osRC);
            if (rc) return rc;
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

        STAFString fromname(fromNameT);
            
        if (MoveFile(fromname.toCurrentCodePage()->buffer(),
                     STAFString(toName).toCurrentCodePage()->buffer()) == FALSE)
        {
            if (osRC) *osRC = GetLastError();
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
        unsigned int flags = FILE_SHARE_READ | FILE_SHARE_WRITE;

        if (isWinNT()) flags |= FILE_SHARE_DELETE;

        HANDLE theFile = CreateFile(filename.toCurrentCodePage()->buffer(),
                                    GENERIC_READ | GENERIC_WRITE, flags,
                                    0, OPEN_EXISTING, 0, 0);

        if (theFile == INVALID_HANDLE_VALUE)
        {
            if (osRC) *osRC = GetLastError();
            return kSTAFBaseOSError;
        }

        BOOL rc = TRUE;

        while (((rc = LockFile(theFile, 0xFFFFFFFE, 0, 1, 0)) ==
                FALSE) &&
               (GetLastError() == ERROR_LOCK_VIOLATION))
        {
            STAFThreadManager::sleepCurrentThread(200);
        }

        if (rc == FALSE)
        {
            if (osRC) *osRC = GetLastError();
            CloseHandle(theFile);
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
    if (lock == 0) return kSTAFInvalidObject;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        if (UnlockFile((**lock).handle, 0xFFFFFFFE, 0, 1, 0) == FALSE)
        {
            // Note: We fall through here so that we can close the file and
            //       free the lock

            if (osRC) *osRC = GetLastError();
            retCode = kSTAFBaseOSError;
        }

        CloseHandle((**lock).handle);

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

        if (GetFileAttributes(
            directory.toCurrentCodePage()->buffer()) != 0xFFFFFFFF)
        {
            return kSTAFAlreadyExists;
        }

        if (flags == kSTAFFSCreateDirOnly)
        {
            BOOL rc = CreateDirectory(directory.toCurrentCodePage()->buffer(),
                                      0);
            if (rc == FALSE)
            {
                if (osRC) *osRC = GetLastError();
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
                
                CreateDirectory(
                    currPath.asString().toCurrentCodePage()->buffer(), 0);
            }
            
            if (CreateDirectory(fsPath.asString().toCurrentCodePage()->buffer(),
                                0) == FALSE)
            {
                if (osRC) *osRC = GetLastError();
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

        STAFFSEnumHandleImpl enumData;

        STAFString theRootName(rootName);

        // Remove the trailing slash from a root directory like C:\

        if (hasTrailingSlash(theRootName))
        {
            theRootName = theRootName.subString(0, theRootName.length() - 1);
        }

        STAFString theRootNameFilter(theRootName + sBackSlash + sStar);
        STAFString theNamePattern(namePattern);
        STAFString theExtPattern(extPattern);

        WIN32_FIND_DATA data = { 0 };
        HANDLE dirHandle = FindFirstFile(
                           theRootNameFilter.toCurrentCodePage()->buffer(),
                           &data);

        if (dirHandle == INVALID_HANDLE_VALUE)
        {
            // Ignore this error as it is a duplicate of an error that is
            // caught in another place.  Also, this error can occur if the
            // directory has already been deleted in an earlier enumeration.

            *enumHandle = new STAFFSEnumHandleImpl(enumData);
            return kSTAFOk;
        }
        else if (dirHandle == FALSE)
        {
            // Return the error

            int errNum = GetLastError();
            *enumHandle = new STAFFSEnumHandleImpl(enumData);

            // XXX: 0 is not correct, find the correct error for no files found

            if ((errNum != 0) && osRC)
                *osRC = errNum;

            return kSTAFBaseOSError;
        }
        else
        {
            // Loop through all the entries in the directory and see if they
            // match the name and extension patterns

            do
            {
                // Ignore . and .. unless the user specifically requests them
                //
                // Note: Win32 doesn't return a ".." entry when scanning a
                //       "first-level" directory such as c:\dev.  This appears
                //       to be related to the fact that c:\ can't be used in
                //       FindFirstFile() (see STAFFSGetEntry).

                if (!(entryTypes & kSTAFFSSpecialDirectory) &&
                    ((STAFString(data.cFileName) == sPeriod) ||
                     (STAFString(data.cFileName) == (sPeriod + sPeriod))))
                {
                    continue;
                }

                STAFString pathName(theRootName);

                pathName += STAFString(sBackSlash);
                pathName += data.cFileName;

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
                        enumData.entries.push_back(goodEntry);
                    else
                        STAFFSFreeEntry(&goodEntry);
                }
            } while ((FindNextFile(dirHandle, &data) != 0) &&
                     (dirHandle != INVALID_HANDLE_VALUE));

            FindClose(dirHandle);
        }

        switch (sortBy)
        {
            case kSTAFFSSortByName:
            {
                std::sort(enumData.entries.begin(), enumData.entries.end(),
                          STAFSortEnumByName(caseSensitive));
                break;
            }

            case kSTAFFSSortBySize:
            {
                std::sort(enumData.entries.begin(), enumData.entries.end(),
                          sortEnumBySize);
                break;
            }

            case kSTAFFSSortByModTime:
            {
                std::sort(enumData.entries.begin(), enumData.entries.end(),
                          sortEnumByModTime);
                break;
            }

            default: break;
        }

        *enumHandle = new STAFFSEnumHandleImpl(enumData);
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
