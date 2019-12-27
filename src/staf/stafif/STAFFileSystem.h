/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_FileSystem
#define STAF_FileSystem

#include "STAF.h"
#include "STAFString.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************/
/* C Definitions */
/*****************/

/**********************************************/
/* Enumeration constants and type definitions */
/**********************************************/

typedef enum STAFFSCaseSensitive_e
{
    kSTAFFSCaseDefault = 0,
    kSTAFFSCaseSensitive = 1,
    kSTAFFSCaseInsensitive = 2
} STAFFSCaseSensitive_t;

typedef enum STAFFSSortBy_e
{
    kSTAFFSNoSort = 0,
    kSTAFFSSortByName = 1,
    kSTAFFSSortBySize = 2,
    kSTAFFSSortByModTime = 3
} STAFFSSortBy_t;

typedef enum STAFFSDirectoryCreateMode_e
{
    kSTAFFSCreateDirOnly = 0,
    kSTAFFSCreatePath = 1
} STAFFSDirectoryCreateMode_t;

typedef enum STAFFSInfoType_e
{
    kSTAFFSPathSep = 0,
    kSTAFFSFileSep = 1,
    kSTAFFSLineSep = 2,
    kSTAFFSCaseSensitivity = 3
} STAFFSInfoType_t;

typedef enum STAFFSEntryType_e
{
    kSTAFFSFile             = 0x00000001,
    kSTAFFSDirectory        = 0x00000002,
    kSTAFFSPipe             = 0x00000004,
    kSTAFFSSocket           = 0x00000008,
    kSTAFFSSymLink          = 0x00000010,
    kSTAFFSCharDev          = 0x00000020,
    kSTAFFSBlkDev           = 0x00000040,
    kSTAFFSSpecialDirectory = 0x00000080,
    kSTAFFSOther            = 0x80000000,
    kSTAFFSNone             = 0x00000000,
    kSTAFFSNormal           = 0x00000003,
    kSTAFFSAll              = 0xFFFFFFFF
} STAFFSEntryType_t;

typedef enum STAFFSComparePathResult_e
{
    kSTAFFSDoesNotIncludePath = 0,
    kSTAFFSDoesIncludePath = 1,
    kSTAFFSSamePath = 2
} STAFFSComparePathResult_t;

typedef struct STAFFSEnumHandleImpl *STAFFSEnumHandle_t;
typedef struct STAFFSEntryImpl *STAFFSEntry_t;
typedef struct STAFFSOSFileLockImpl *STAFFSOSFileLock_t;

/*****************************************************************************/
/*                         General File System APIs                          */
/*****************************************************************************/

/*****************************************************************************/
/* STAFFSInfo - Returns information about the OS's file system               */
/*                                                                           */
/* Accepts: (Out) Pointer to return data                                     */
/*          (In)  The type of information to retrieve                        */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) The types kSTAFFSPathSep, kSTAFFSFileSep, and kSTAFFSLineSep  */
/*             return data of type STAFString_t.  You are responsible for    */
/*             call STAFStringDestruct on these returned strings.            */
/*          2) The type kSTAFFSCaseSensitivity returns data of type          */
/*             STAFFSCaseSensitive_t.                                        */
/*****************************************************************************/
STAFRC_t STAFFSInfo(void *info, STAFFSInfoType_t infoType);


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
/*****************************************************************************/
STAFRC_t STAFFSStringMatchesWildcards(STAFStringConst_t stringToCheck,
                                      STAFStringConst_t wildcardString,
                                      STAFFSCaseSensitive_t sensitive,
                                      unsigned int *matches);

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
                            STAFFSComparePathResult_t *result);

/*****************************************************************************/
/*                                Path APIs                                  */
/*****************************************************************************/
/* In general, any STAFString_t returned from an API must be freed by the    */
/* caller.  No STAFStringConst_t passed into an API will be freed.           */
/*****************************************************************************/

/*****************************************************************************/
/* STAFFSAssemblePath - Generates a path string from component pieces        */
/*                                                                           */
/* Accepts: (Out) Pointer to the return string                               */
/*          (In)  A string representing the root of the path                 */
/*          (In)  The number of directory strings                            */
/*          (In)  An array of strings representing directories               */
/*          (In)  A string representing the name of the file system entry    */
/*          (In)  A string representing the extension of the file system     */
/*                entry                                                      */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSAssemblePath(STAFString_t *path, STAFStringConst_t root,
                            unsigned int numDirs, STAFStringConst_t *dirs,
                            STAFStringConst_t name,
                            STAFStringConst_t extension);


/*****************************************************************************/
/* STAFFSDisassemblePath - Breaks a path string into its component pieces    */
/*                                                                           */
/* Accepts: (In)  The path string to disassemble                             */
/*          (Out) Pointer to the string representing the root of the path    */
/*          (Out) Pointer to the number of directory strings                 */
/*          (Out) Pointer to an array of strings representing the            */
/*                directories                                                */
/*          (Out) Pointer to the string representing the name of the file    */
/*                system entry                                               */
/*          (Out) Pointer to the string representing the extension of the    */
/*                file system entry                                          */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) Remember to free the directory array with                     */
/*             STAFFSFreePathDirs()                                          */
/*****************************************************************************/
STAFRC_t STAFFSDisassemblePath(STAFStringConst_t path, STAFString_t *root,
                               unsigned int *numDirs, STAFString_t **dirs,
                               STAFString_t *name, STAFString_t *extension);


/*****************************************************************************/
/* STAFFSFreePathDirs - Frees up the the memory associated with the array    */
/*                      of STAFString_t returned from STAFFSDissassemblePath */
/*                                                                           */
/* Accepts: (In) Pointer to the array of STAFString_t                        */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) You are still responsible for freeing the individual          */
/*             STAFString_t that are in the array.  This function simply     */
/*             frees the memory  associated with the array itself.           */
/*****************************************************************************/
STAFRC_t STAFFSFreePathDirs(STAFString_t *dirs);


/*****************************************************************************/
/*                          File System Entry APIs                           */
/*****************************************************************************/

/*****************************************************************************/
/* STAFFSExists - Determines whether a given file system entry exists        */
/*                                                                           */
/* Accepts: (In)  Path string                                                */
/*          (Out) Pointer to unsigned int indicating if the entry exists     */
/*                (0 = no, 1 = yes)                                          */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSExists(STAFStringConst_t path, unsigned int *exists,
                      unsigned int *osRC);


/*****************************************************************************/
/* STAFFSGetEntry - Retrives an entry object representing the file system    */
/*                  entry                                                    */
/*                                                                           */
/* Accepts: (In)  Path string                                                */
/*          (Out) Pointer to the file system entry                           */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSGetEntry(STAFStringConst_t path, STAFFSEntry_t *entry,
                        unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetPathString - Gets the path string associated with an entry  */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to constant path string                            */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) This API returns a canonical path string.  This is not        */
/*             necessarily the same string as used in STAFFSGetEntry().      */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetPathString(STAFFSEntry_t entry,
                                  STAFStringConst_t *pathString,
                                  unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetType - Gets the type associated with an entry               */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to entry type                                      */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetType(STAFFSEntry_t entry, STAFFSEntryType_t *type,
                            unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetSize - Gets the size associated with an entry               */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to upper word of size                              */
/*          (Out) Pointer to lower word of size                              */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) The size is returned as two 32-bit quantities representing a  */
/*             64-bit quantity                                               */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetSize(STAFFSEntry_t entry, unsigned int *upperSize,
                            unsigned int *lowerSize, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetSize64 - Gets the size associated with an entry             */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to size                                            */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) The size is returned as a 64-bit size                         */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetSize64(STAFFSEntry_t entry, STAFUInt64_t *size,
                              unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetModTime - Gets the modification time associated with an     */
/*                         entry                                             */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to modification time                               */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetModTime(STAFFSEntry_t entry, time_t *modTime,
                               unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetIsLink - Gets a flag indicating if the entry is a link      */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to flag indicating if the entry is a link          */
/*                1 = link   0 = Not a link                                  */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetIsLink(STAFFSEntry_t entry, unsigned int *isLink,
                              unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryGetLinkTarget - Gets the link target associated with a         */
/*                            symbolic link entry                            */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to constant to (target) link path string           */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryGetLinkTarget(STAFFSEntry_t entry,
                                  STAFString_t *linkTargetString,
                                  unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryReadLock - Gets a read lock on a file system entry             */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryReadLock(STAFFSEntry_t entry, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryReadUnlock - Releases a read lock on a file system entry       */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryReadUnlock(STAFFSEntry_t entry, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryWriteLock - Gets a write lock on a file system entry           */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryWriteLock(STAFFSEntry_t entry, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEntryWriteUnlock - Releases a write lock on a file system entry     */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEntryWriteUnlock(STAFFSEntry_t entry, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSDeleteEntry - Deletes a file system entry                           */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) This does NOT free the entry itself.  You must still call     */
/*             STAFFSFreeEntry().                                            */
/*****************************************************************************/
STAFRC_t STAFFSDeleteEntry(STAFFSEntry_t entry, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSCopyEntry - Copies one file system entry to another path            */
/*                                                                           */
/* Accepts: (In)  Source file system entry                                   */
/*          (In)  Target path string                                         */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSCopyEntry(STAFFSEntry_t source, STAFStringConst_t target,
                         unsigned int *osRC);

/*****************************************************************************/
/* STAFFSMoveEntry - Moves one file system entry to another name.  If the    */
/*                   target already exists, it is overwritten.               */
/*                                                                           */
/* Accepts: (In)  Source file system entry (file to be moved)                */
/*          (In)  Target path string (new file name)                         */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSMoveEntry(STAFFSEntry_t source, STAFStringConst_t target,
                         unsigned int *osRC);

/*****************************************************************************/
/* STAFFSRenameEntry - Renames one file system entry to another name.  The   */
/*                     target must not already exist.                        */
/*                                                                           */
/* Accepts: (In)  Source file system entry (file to be renamed)              */
/*          (In)  Target path string (new file name)                         */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSRenameEntry(STAFFSEntry_t source, STAFStringConst_t target,
                           unsigned int *osRC);


/*****************************************************************************/
/* STAFFSFreeEntry - Frees a file system entry object                        */
/*                                                                           */
/* Accepts: (In)  File system entry                                          */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSFreeEntry(STAFFSEntry_t *entry);


/*****************************************************************************/
/*                             Directory APIs                                */
/*****************************************************************************/

/*****************************************************************************/
/* STAFFSCreateDirectory - Creates a directory in the file system            */
/*                                                                           */
/* Accepts: (In)  Path string                                                */
/*          (In)  Directory creation flags                                   */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSCreateDirectory(STAFStringConst_t path,
                               STAFFSDirectoryCreateMode_t flags,
                               unsigned int *osRC);


/*****************************************************************************/
/* STAFFSGetCurrentDirectory - Retrieves the current directory               */
/*                                                                           */
/* Accepts: (Out) Pointer to current directory string                        */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSGetCurrentDirectory(STAFString_t *path, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSSetCurrentDirectory - Sets the current directory                    */
/*                                                                           */
/* Accepts: (In)  Current directory string                                   */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) You should obtain the current directory lock before setting   */
/*             the current directory                                         */
/*****************************************************************************/
STAFRC_t STAFFSSetCurrentDirectory(STAFStringConst_t path, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSRequestCurrentDirectoryLock - Obtains the lock on the current       */
/*                                     directory                             */
/*                                                                           */
/* Accepts: Nothing                                                          */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSRequestCurrentDirectoryLock();


/*****************************************************************************/
/* STAFFSReleaseCurrentDirectoryLock - Releases the lock on the current      */
/*                                     directory                             */
/*                                                                           */
/* Accepts: Nothing                                                          */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSReleaseCurrentDirectoryLock();


/*****************************************************************************/
/* STAFFSEnumOpen - Obtains an enumeration handle for an entry object        */
/*                                                                           */
/* Accepts: (Out) Pointer to enumeration handle                              */
/*          (In)  File system entry                                          */
/*          (In)  Pattern used to match names                                */
/*          (In)  Pattern used to match extension                            */
/*          (In)  Are name/extension patterns case sensitive                 */
/*          (In)  The types of entries to enumerate                          */
/*          (In)  How should the entries be sorted                           */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) The name and extension patterns may contain the '*' and '?'   */
/*             characters.  The '*' character matches zero or more           */
/*             characters.  The '?' character matches one character.         */
/*          2) It is legal to enumerate a non-directory entry.  You will     */
/*             simply receive an enumeration with no entries.                */
/*****************************************************************************/
STAFRC_t STAFFSEnumOpen(STAFFSEnumHandle_t *enumHandle, STAFFSEntry_t entry,
                        STAFStringConst_t namePattern,
                        STAFStringConst_t extPattern,
                        STAFFSCaseSensitive_t caseSensitivity,
                        STAFFSEntryType_t entryTypes,
                        STAFFSSortBy_t sortBy, unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEnumNext - Retrieves the next entry in the enumeration              */
/*                                                                           */
/* Accepts: (In)  Enumeration handle                                         */
/*          (Out) Pointer to file system entry (this will zero if there are  */
/*                no more entries in the enumeration                         */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) You must free the entries obtained through this API           */
/*          2) No error will be returned when enumeration is complete.  You  */
/*             will simply receive a zero entry.                             */
/*****************************************************************************/
STAFRC_t STAFFSEnumNext(STAFFSEnumHandle_t enumHandle, STAFFSEntry_t *entry,
                        unsigned int *osRC);


/*****************************************************************************/
/* STAFFSEnumClose - Closes an enumeration handle                            */
/*                                                                           */
/* Accepts: (I/O) Pointer to enumeration handle                              */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t STAFFSEnumClose(STAFFSEnumHandle_t *enumHandle, unsigned int *osRC);


/*****************************************************************************/
/*                            Internal use only                              */
/*****************************************************************************/
/* Note: These APIs should not be used directly by the user                  */
/*****************************************************************************/

/*****************************************************************************/
/* STAFFSOSGetExclusiveFileLock - Gets an exclusive lock on a file           */
/*                                                                           */
/* Accepts: (In)  Path string                                                */
/*          (Out) Pointer to the lock                                        */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) This API is for internal STAF use only.  Users should use     */
/*             the STAFFSEntry<Read|Write>Lock APIs.                         */
/*          2) Porters - this need only provide an advisory lock.  A         */
/*             mandatory lock is not required.                               */
/*****************************************************************************/
STAFRC_t STAFFSOSGetExclusiveFileLock(STAFStringConst_t path,
                                      STAFFSOSFileLock_t *lock,
                                      unsigned int *osRC);


/*****************************************************************************/
/* STAFFSOSReleaseExclusiveFileLock - Releases an exclusive file lock        */
/*                                                                           */
/* Accepts: (I/O) Pointer to the lock                                        */
/*          (Out) Pointer to unsigned int indicating operating system error  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*                                                                           */
/* Notes  : 1) This API is for internal STAF use only.  Users should use     */
/*             the STAFFSEntry<Read|Write>Unlock APIs.                       */
/*****************************************************************************/
STAFRC_t STAFFSOSReleaseExclusiveFileLock(STAFFSOSFileLock_t *lock,
                                          unsigned int *osRC);

/* The following definitions are for OS independent versions of the  */
/* File System APIs.  These should not be used by user applications. */

STAFRC_t STAFFSCopyEntryCommon(STAFFSEntry_t source, STAFStringConst_t target,
                               unsigned int *osRC);

#ifdef __cplusplus
}

#include "STAFString.h"
#include "STAFTimestamp.h"
#include "STAFRefPtr.h"
#include "STAFException.h"
#include <deque>

// C++ Definitions

// STAFFSPath - This class represents a path in the file system.  The path
//              can be represented as a string, or as its constituent pieces.
//              These pieces are the root of the path, the directories in the
//              path, the name of the item, and the extension of the item.
//
//              This class provides numerous methods to build up a path string
//              from its pieces and to determine a path's pieces from a path
//              string.
//
//              Given an instance of this class, you can retrieve the file
//              system entry object associated with it, as well as create the
//              directory represented by it.

class STAFFSEntry;
typedef STAFRefPtr<STAFFSEntry> STAFFSEntryPtr;
typedef std::pair<STAFRC_t, STAFFSEntryPtr> STAFFSEntryRC;

class STAFFSPath
{
public:

    // This constructor is designed for building a path piece by piece
    STAFFSPath();

    // This constructor is designed for breaking down a path string
    STAFFSPath(const STAFString &path);

    // These methods allow you to build up a path piece by piece
    STAFFSPath &setRoot(const STAFString &root = STAFString());
    STAFFSPath &addDir(const STAFString &dir);
    STAFFSPath &setName(const STAFString &name = STAFString());
    STAFFSPath &setExtension(const STAFString &extension = STAFString());

    // Allows you to reset the list of directories in the path
    STAFFSPath &clearDirList();

    // Get a path string based on pieces
    STAFString asString() const;

    // Get various pieces based on path string
    STAFString root() const;
    unsigned int numDirs() const;
    STAFString dir(unsigned int index = 0) const;
    STAFString name() const;
    STAFString extension() const;

    unsigned int exists() const;

    STAFFSEntryPtr getEntry() const;
    STAFFSEntryRC getEntry(unsigned int *osRC) const;

    STAFFSEntryPtr createDirectory(STAFFSDirectoryCreateMode_t mode =
                                   kSTAFFSCreateDirOnly) const;
    STAFFSEntryRC createDirectory(unsigned int *osRC,
                                  STAFFSDirectoryCreateMode_t mode =
                                  kSTAFFSCreateDirOnly) const;

private:

    void updatePath();
    void updatePieces();

    unsigned int fPathUpToDate;
    unsigned int fPiecesUpToDate;

    STAFString fPath;
    STAFString fRoot;
    std::deque<STAFString> fDirs;
    STAFString fName;
    STAFString fExtension;
};


// STAFFSEntry - This class represents an object that exists (or, at least, has
//               existed at some time) in the file system.
//
//               Given an instance of this class, you can obtain various details
//               about the entry, as well as remove (a.k.a, delete), copy, and
//               rename the entry.  You may also obtain an enumeration of this
//               entries sub-entries (if it is a directory).

class STAFFSEnumeration;
typedef STAFRefPtr<STAFFSEnumeration> STAFFSEnumPtr;

class STAFFSEntry
{
public:

    STAFFSEntry(STAFFSEntry_t entry) : fEntry(entry)
    { /* Do Nothing */ }

    typedef std::pair<unsigned int, unsigned int> FileSize;

    // XXX: Add a trueType() function for differentiating symlinks
    // XXX: Add a linkedTo() function to get name of entry linked to

    // Informational methods

    STAFFSPath path() const;
    STAFFSEntryType_t type() const;
    FileSize size() const;
    STAFUInt64_t size64() const;
    STAFTimestamp modTime() const;
    unsigned int isLink() const;
    STAFString linkTarget() const;

    // Lock methods

    void readLock();
    void readUnlock();
    void writeLock();
    void writeUnlock();

    // Manipulation methods

    void remove();
    STAFRC_t remove(unsigned int *osRC);

    void copy(const STAFString &toName);
    
    void move(const STAFString &toName);

    void rename(const STAFString &toName);

    // Enumeration methods

    STAFFSEnumPtr enumerate(const STAFString &namePat = STAFString(kUTF8_STAR),
                            const STAFString &extPat = STAFString(kUTF8_STAR),
                            STAFFSEntryType_t types = kSTAFFSNormal,
                            STAFFSSortBy_t sortBy = kSTAFFSNoSort,
                            STAFFSCaseSensitive_t caseSensitivity =
                                kSTAFFSCaseDefault) const;

    // Data methods

    STAFFSEntry_t getImpl() const;
    STAFFSEntry_t adoptImpl();

    ~STAFFSEntry();

private:

    // Disallow copy construction and assignment
    STAFFSEntry(const STAFFSEntry &);
    STAFFSEntry &operator=(const STAFFSEntry &);

    STAFFSEntry_t fEntry;
};


// STAFFSEntryRLock - This class is used to acquire and automatically
//                    release the read lock on a STAFFSEntry object

class STAFFSEntryRLock
{
public:

    STAFFSEntryRLock(STAFFSEntryPtr &theEntry)
        : fEntry(theEntry)
    { fEntry->readLock(); }

    ~STAFFSEntryRLock()
    { fEntry->readUnlock(); }

private:

    // Disallow copy construction and assignment
    STAFFSEntryRLock(const STAFFSEntryRLock &);
    STAFFSEntryRLock &operator=(const STAFFSEntryRLock &);

    STAFFSEntryPtr fEntry;
};


// STAFFSEntryWLock - This class is used to acquire and automatically
//                    release the write lock on a STAFFSEntry object

class STAFFSEntryWLock
{
public:

    STAFFSEntryWLock(STAFFSEntryPtr &theEntry)
        : fEntry(theEntry)
    { fEntry->writeLock(); }

    ~STAFFSEntryWLock()
    { fEntry->writeUnlock(); }

private:

    // Disallow copy construction and assignment
    STAFFSEntryWLock(const STAFFSEntryWLock &);
    STAFFSEntryWLock &operator=(const STAFFSEntryWLock &);

    STAFFSEntryPtr fEntry;
};


// STAFFSEnumeration - This class represents the collection of entries that
//                     reside within a given directory.
//
//                     You may retrieve the current entry, move to the next
//                     entry, and determine if the enumeration is still valid.

class STAFFSEnumeration
{
public:

    STAFFSEnumeration(STAFFSEnumHandle_t handle);

    unsigned int isValid() const;
    STAFFSEntryPtr entry() const;
    STAFFSEnumeration &next();

    ~STAFFSEnumeration();

private:

    // Disallow copy construction and assignment
    STAFFSEnumeration(const STAFFSEnumeration &);
    STAFFSEnumeration &operator=(const STAFFSEnumeration &);

    STAFFSEnumHandle_t fHandle;
    unsigned int fIsValid;
    STAFFSEntryPtr fCurrEntry;
};



// STAFFSCurrentDirectoryLock - This class provides a wrapper around the
//                              current directory locking mechanism
//
//                              In general, you would create an instance of
//                              this class before setting the current directory.

class STAFFSCurrentDirectoryLock
{
public:

    STAFFSCurrentDirectoryLock();
    ~STAFFSCurrentDirectoryLock();

private:

    // Disallow copy construction and assignment
    STAFFSCurrentDirectoryLock(const STAFFSCurrentDirectoryLock &);
    STAFFSCurrentDirectoryLock &operator=(const STAFFSCurrentDirectoryLock &);

};


// STAFFileSystem - This class provides some utility functions for interfacing
//                  with the file system.
//
//                  You may get and set the current directory, as well as
//                  obtain certain information about the underlying file system.

class STAFFileSystem
{
public:

    static STAFString getInfo(STAFFSInfoType_t type);

    static unsigned int matchesWildcards(
        const STAFString &stringToCheck, const STAFString &wildcardString,
        STAFFSCaseSensitive_t sensitive = kSTAFFSCaseDefault);

    static STAFFSComparePathResult_t comparePaths(
        const STAFString &path1, const STAFString &path2,
        STAFFSCaseSensitive_t sensitive = kSTAFFSCaseDefault);

    // Note: You should own the current directory lock before setting the
    //       current directory

    static STAFString getCurrentDirectory();
    static void setCurrentDirectory(const STAFString &dirName);

private:

    // Disallow copy construction and assignment
    STAFFileSystem(const STAFFileSystem &);
    STAFFileSystem &operator=(const STAFFileSystem &);
};


// Begin C++ exception definitions

STAF_EXCEPTION_DEFINITION(STAFFSException, STAFException);

// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFFileSystemInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif


#endif

