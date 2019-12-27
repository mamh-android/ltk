/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_FileSystemInlImpl
#define STAF_FileSystemInlImpl

#include "STAF.h"
#include "STAFFileSystem.h"
#include "STAFString.h"
#include "STAFTimestamp.h"
#include "STAFRefPtr.h"
#include "STAFException.h"
#include <deque>


/**************************/
/* STAFFSPath definitions */
/**************************/

STAF_INLINE STAFFSPath::STAFFSPath() : fPathUpToDate(1), fPiecesUpToDate(1)
{ /* Do Nothing */ }


STAF_INLINE STAFFSPath::STAFFSPath(const STAFString &path)
    : fPathUpToDate(1), fPiecesUpToDate(1), fPath(path)
{
    updatePieces();
}


STAF_INLINE STAFFSPath &STAFFSPath::setRoot(const STAFString &root)
{
    fRoot = root;
    fPathUpToDate = 0;
    return *this;
}


STAF_INLINE STAFFSPath &STAFFSPath::addDir(const STAFString &dir)
{
    fDirs.push_back(dir);
    fPathUpToDate = 0;
    return *this;
}


STAF_INLINE STAFFSPath &STAFFSPath::setName(const STAFString &name)
{
    fName = name;
    fPathUpToDate = 0;
    return *this;
}


STAF_INLINE STAFFSPath &STAFFSPath::setExtension(const STAFString &extension)
{
    fExtension = extension;
    fPathUpToDate = 0;
    return *this;
}


STAF_INLINE STAFFSPath &STAFFSPath::clearDirList()
{
    fDirs = std::deque<STAFString>();
    fPathUpToDate = 0;
    return *this;
}


STAF_INLINE STAFString STAFFSPath::asString() const
{
    if (!fPathUpToDate) const_cast<STAFFSPath *>(this)->updatePath();

    return fPath;
}


STAF_INLINE STAFString STAFFSPath::root() const
{
    if (!fPiecesUpToDate) const_cast<STAFFSPath *>(this)->updatePieces();

    return fRoot;
}


STAF_INLINE unsigned int STAFFSPath::numDirs() const
{
    if (!fPiecesUpToDate) const_cast<STAFFSPath *>(this)->updatePieces();

    return fDirs.size();
}


STAF_INLINE STAFString STAFFSPath::dir(unsigned int index) const
{
    if (!fPiecesUpToDate) const_cast<STAFFSPath *>(this)->updatePieces();

    if (index >= fDirs.size())
    {
        STAFString message(STAFString("STAFFSPath::dir(") + index +
                           STAFString(" of ") + fDirs.size());
        STAFOutOfBoundsException error(message.toCurrentCodePage()->buffer());
        THROW_STAF_EXCEPTION(error);
    }

    return fDirs[index];
}


STAF_INLINE STAFString STAFFSPath::name() const
{
    if (!fPiecesUpToDate) const_cast<STAFFSPath *>(this)->updatePieces();

    return fName;
}


STAF_INLINE STAFString STAFFSPath::extension() const
{
    if (!fPiecesUpToDate) const_cast<STAFFSPath *>(this)->updatePieces();

    return fExtension;
}


STAF_INLINE unsigned int STAFFSPath::exists() const
{
    unsigned int doesExist = 0;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSExists(asString().getImpl(), &doesExist, &osRC);

    STAFFSException::checkRC(rc, "STAFFSExists", osRC);

    return doesExist;
}


STAF_INLINE STAFFSEntryPtr STAFFSPath::getEntry() const
{
    STAFFSEntry_t entry = 0;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSGetEntry(asString().getImpl(), &entry, &osRC);

    STAFFSException::checkRC(rc, "STAFFSGetEntry", osRC);

    return STAFFSEntryPtr(new STAFFSEntry(entry), STAFFSEntryPtr::INIT);
}


STAF_INLINE STAFFSEntryRC STAFFSPath::getEntry(unsigned int *osRC) const
{
    STAFFSEntry_t entry = 0;
    STAFRC_t rc = STAFFSGetEntry(asString().getImpl(), &entry, osRC);
    STAFFSEntryPtr entryPtr;

    if (rc == kSTAFOk)
        entryPtr = STAFFSEntryPtr(new STAFFSEntry(entry), STAFFSEntryPtr::INIT);

    return STAFFSEntryRC(rc, entryPtr);
}


STAF_INLINE STAFFSEntryPtr STAFFSPath::createDirectory(
    STAFFSDirectoryCreateMode_t mode) const
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSCreateDirectory(asString().getImpl(), mode, &osRC);

    STAFFSException::checkRC(rc, "STAFFSCreateDirectory", osRC);

    return getEntry();
}


STAF_INLINE STAFFSEntryRC STAFFSPath::createDirectory(unsigned int *osRC,
    STAFFSDirectoryCreateMode_t mode) const
{
    STAFRC_t rc = STAFFSCreateDirectory(asString().getImpl(), mode, osRC);

    if (rc == kSTAFOk) return getEntry(osRC);

    return STAFFSEntryRC(rc, STAFFSEntryPtr());
}


STAF_INLINE void STAFFSPath::updatePieces()
{
    STAFString_t theRoot = 0;
    STAFString_t theName = 0;
    STAFString_t theExtension = 0;
    STAFString_t *theDirs = 0;
    unsigned int numDirs = 0;
    STAFRC_t rc = STAFFSDisassemblePath(fPath.getImpl(), &theRoot, &numDirs,
                                        &theDirs, &theName, &theExtension);

    STAFFSException::checkRC(rc, "STAFFSDisassemblePath");

    fRoot = STAFString(theRoot, STAFString::kShallow);
    fName = STAFString(theName, STAFString::kShallow);
    fExtension = STAFString(theExtension, STAFString::kShallow);
    fDirs = std::deque<STAFString>();

    for (unsigned int i = 0; i < numDirs; ++i)
        fDirs.push_back(STAFString(theDirs[i], STAFString::kShallow));

    STAFFSFreePathDirs(theDirs);

    fPiecesUpToDate = 1;
}


STAF_INLINE void STAFFSPath::updatePath()
{
    STAFStringConst_t *theDirs = new STAFStringConst_t[fDirs.size()];
    STAFRefPtr<STAFStringConst_t> theDirsPtr =
        STAFRefPtr<STAFStringConst_t>(theDirs,
        STAFRefPtr<STAFStringConst_t>::INIT,
        STAFRefPtr<STAFStringConst_t>::ARRAY);

    int i = 0;

    for (std::deque<STAFString>::iterator iter = fDirs.begin();
         iter != fDirs.end(); ++iter)
    { theDirs[i++] = iter->getImpl(); }

    STAFString_t thePath = 0;
    STAFRC_t rc = STAFFSAssemblePath(&thePath, fRoot.getImpl(), i, theDirs,
                                     fName.getImpl(), fExtension.getImpl());

    STAFFSException::checkRC(rc, "STAFFSAssemblePath");

    fPath = STAFString(thePath, STAFString::kShallow);
    fPathUpToDate = 1;
}


/***************************/
/* STAFFSEntry definitions */
/***************************/

STAF_INLINE STAFFSPath STAFFSEntry::path() const
{
    unsigned int osRC = 0;
    STAFStringConst_t pathString;
    STAFRC_t rc = STAFFSEntryGetPathString(fEntry, &pathString, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetPathString", osRC);

    return STAFFSPath(pathString);
}


STAF_INLINE STAFFSEntryType_t STAFFSEntry::type() const
{
    unsigned int osRC = 0;
    STAFFSEntryType_t theType = kSTAFFSFile;
    STAFRC_t rc = STAFFSEntryGetType(fEntry, &theType, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetType", osRC);

    return theType;
}


STAF_INLINE STAFFSEntry::FileSize STAFFSEntry::size() const
{
    unsigned int osRC = 0;
    unsigned int upperSize = 0;
    unsigned int lowerSize = 0;
    STAFRC_t rc = STAFFSEntryGetSize(fEntry, &upperSize, &lowerSize, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetSize", osRC);

    return std::make_pair(upperSize, lowerSize);
}


STAF_INLINE STAFUInt64_t STAFFSEntry::size64() const
{
    unsigned int osRC = 0;
    STAFUInt64_t size = 0;
    STAFRC_t rc = STAFFSEntryGetSize64(fEntry, &size, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetSize", osRC);

    return size;
}


STAF_INLINE STAFTimestamp STAFFSEntry::modTime() const
{
    unsigned int osRC = 0;
    time_t modTime = 0;
    STAFRC_t rc = STAFFSEntryGetModTime(fEntry, &modTime, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetModTime", osRC);

    return modTime;
}


STAF_INLINE unsigned int STAFFSEntry::isLink() const
{
    unsigned int osRC = 0;
    unsigned int isALink = 0;
    STAFRC_t rc = STAFFSEntryGetIsLink(fEntry, &isALink, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetIsLink", osRC);

    return isALink;
}


STAF_INLINE STAFString STAFFSEntry::linkTarget() const
{
    unsigned int osRC = 0;
    STAFString_t linkTargetString;

    STAFRC_t rc = STAFFSEntryGetLinkTarget(fEntry, &linkTargetString, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryGetLinkTarget", osRC);

    return STAFString(linkTargetString);
}


STAF_INLINE void STAFFSEntry::readLock()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSEntryReadLock(fEntry, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryReadLock", osRC);
}


STAF_INLINE void STAFFSEntry::readUnlock()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSEntryReadUnlock(fEntry, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryReadUnlock", osRC);
}


STAF_INLINE void STAFFSEntry::writeLock()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSEntryWriteLock(fEntry, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryWriteLock", osRC);
}


STAF_INLINE void STAFFSEntry::writeUnlock()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSEntryWriteUnlock(fEntry, &osRC);

    STAFFSException::checkRC(rc, "STAFFSEntryWriteUnlock", osRC);
}


STAF_INLINE void STAFFSEntry::remove()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSDeleteEntry(fEntry, &osRC);

    STAFFSException::checkRC(rc, "STAFFSDeleteEntry", osRC);
}


STAF_INLINE STAFRC_t STAFFSEntry::remove(unsigned int *osRC)
{
    return STAFFSDeleteEntry(fEntry, osRC);
}


STAF_INLINE void STAFFSEntry::copy(const STAFString &toName)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSCopyEntry(fEntry, toName.getImpl(), &osRC);

    STAFFSException::checkRC(rc, "STAFFSCopyEntry", osRC);
}


STAF_INLINE void STAFFSEntry::move(const STAFString &toName)
{
    unsigned int osRC = 0;

    STAFRC_t rc = STAFFSMoveEntry(fEntry, toName.getImpl(), &osRC);

    STAFFSException::checkRC(rc, "STAFFSMoveEntry", osRC);
}


STAF_INLINE void STAFFSEntry::rename(const STAFString &toName)
{
    unsigned int osRC = 0;

    STAFRC_t rc = STAFFSRenameEntry(fEntry, toName.getImpl(), &osRC);

    STAFFSException::checkRC(rc, "STAFFSRenameEntry", osRC);
}


STAF_INLINE STAFFSEnumPtr STAFFSEntry::enumerate(const STAFString &namePattern,
            const STAFString &extPattern, STAFFSEntryType_t types,
            STAFFSSortBy_t sortBy, STAFFSCaseSensitive_t casing) const
{
    STAFFSEnumHandle_t handle = 0;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSEnumOpen(&handle, fEntry, namePattern.getImpl(),
                                 extPattern.getImpl(), casing, types, sortBy,
                                 &osRC);

    STAFFSException::checkRC(rc, "STAFFSEnumOpen", osRC);

    return STAFFSEnumPtr(new STAFFSEnumeration(handle), STAFFSEnumPtr::INIT);
}


STAF_INLINE STAFFSEntry_t STAFFSEntry::getImpl() const
{
    return fEntry;
}


STAF_INLINE STAFFSEntry_t STAFFSEntry::adoptImpl()
{
    STAFFSEntry_t tempEntry = fEntry;
    fEntry = 0;
    return tempEntry;
}


STAF_INLINE STAFFSEntry::~STAFFSEntry()
{
    STAFFSFreeEntry(&fEntry);
}


/*********************************/
/* STAFFSEnumeration definitions */
/*********************************/

STAF_INLINE STAFFSEnumeration::STAFFSEnumeration(STAFFSEnumHandle_t handle)
    : fHandle(handle), fIsValid(1)
{ next(); }

STAF_INLINE unsigned int STAFFSEnumeration::isValid() const
{ return fIsValid; }

STAF_INLINE STAFFSEntryPtr STAFFSEnumeration::entry() const
{ return fCurrEntry; }

STAF_INLINE STAFFSEnumeration &STAFFSEnumeration::next()
{
    STAFFSEntry_t entry = 0;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSEnumNext(fHandle, &entry, &osRC);

    if ((rc == 0) && (entry == 0))
    {
        fIsValid = 0;
        fCurrEntry = STAFFSEntryPtr();
    }
    else
    {
        STAFFSException::checkRC(rc, "STAFFSDirectoryEnumNext", osRC);
        fCurrEntry = STAFFSEntryPtr(new STAFFSEntry(entry),
                                    STAFFSEntryPtr::INIT);
    }

    return *this;
}

STAF_INLINE STAFFSEnumeration::~STAFFSEnumeration()
{
    STAFFSEnumClose(&fHandle, 0);
}


/******************************************/
/* STAFFSCurrentDirectoryLock definitions */
/******************************************/

STAF_INLINE STAFFSCurrentDirectoryLock::STAFFSCurrentDirectoryLock()
{
    STAFRC_t rc = STAFFSRequestCurrentDirectoryLock();

    STAFFSException::checkRC(rc, "STAFFSRequestCurrentDirectoryLock");
}

STAF_INLINE STAFFSCurrentDirectoryLock::~STAFFSCurrentDirectoryLock()
{
    STAFFSReleaseCurrentDirectoryLock();
}


/******************************/
/* STAFFileSystem definitions */
/******************************/

STAF_INLINE STAFString STAFFileSystem::getInfo(STAFFSInfoType_t type)
{
    STAFString_t info = 0;
    STAFRC_t rc = STAFFSInfo(&info, type);

    STAFFSException::checkRC(rc, "STAFFSInfo");

    return STAFString(info, STAFString::kShallow);
}

STAF_INLINE unsigned int STAFFileSystem::matchesWildcards(
    const STAFString &stringToCheck, const STAFString &wildcardString,
    STAFFSCaseSensitive_t sensitive)
{
    unsigned int matches= 0;
    STAFRC_t rc = STAFFSStringMatchesWildcards(stringToCheck.getImpl(),
                                               wildcardString.getImpl(),
                                               sensitive, &matches);

    STAFFSException::checkRC(rc, "STAFFSStringMatchesWildcards");

    return matches;
}


STAF_INLINE STAFFSComparePathResult_t STAFFileSystem::comparePaths(
    const STAFString &path1, const STAFString &path2,
    STAFFSCaseSensitive_t sensitive)
{
    STAFFSComparePathResult_t result;
    STAFRC_t rc = STAFFSComparePaths(path1.getImpl(), path2.getImpl(),
                                     sensitive, &result);

    STAFFSException::checkRC(rc, "STAFFSComparePaths");

    return result;
}


STAF_INLINE STAFString STAFFileSystem::getCurrentDirectory()
{
    STAFString_t currDir = 0;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSGetCurrentDirectory(&currDir, &osRC);

    STAFFSException::checkRC(rc, "STAFFSGetCurrentDirectory", osRC);

    return STAFString(currDir, STAFString::kShallow);
}


STAF_INLINE void STAFFileSystem::setCurrentDirectory(const STAFString &dirName)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFFSSetCurrentDirectory(dirName.getImpl(), &osRC);

    STAFFSException::checkRC(rc, "STAFFSSetCurrentDirectory", osRC);
}

#endif
