/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFFSCopyManager.h"
#include "STAFTimestamp.h"
#include "STAFFSService.h"
#include "STAFTrace.h"
#include "STAFServiceManager.h"
#include "STAFFileSystem.h"


STAFRC_t STAFFSCopyManager::add(
    const STAFString &name, const STAFString &machine,
    unsigned int direction, unsigned int type, unsigned int mode,
    unsigned int fileSize, FSCopyDataPtr &copyDataPtr)
{
    {
        // Get a lock on the FS Copy Map for the duration of this block

        STAFMutexSemLock fsCopyMapLock(fFSCopyMapSem);

        // Check if the name of the file or directory already exists
        
        FSCopyMap::iterator iter;
        bool found = false;

        // If the direction is kSTAFFSCopyTo, check if the name of the file or
        // directory being written to already exists in the FS Copy Map (which 
        // means it is currently being copied from or to).  If so, return an
        // error.
        // If the direction is kSTAFFSCopyFrom, check if the name of the file
        // or directory being read from already exists in an kSTAFFSCopyTo
        // (Inbound) entry in the FS Copy Map (which means it is currently
        // being copied to).  If so, return an error.

        for (iter = fFSCopyMap.begin(); iter != fFSCopyMap.end(); ++iter)
        {
            FSCopyDataPtr copyData = iter->second;
            STAFString entryName = "";

            try
            {
                entryName = STAFFSPath(copyData->name).getEntry()->path().
                    asString();
            }
            catch (STAFException)
            { 
                // Ignore as caused by getEntry() failing due to a problem
                // with the entry (e.g. no longer exists) which will be
                // reported during the copy procedure
                continue;
            }

            if (name == entryName)
            {
                if (direction == kSTAFFSCopyFrom)
                {
                    if (copyData->direction == kSTAFFSCopyTo)
                    {
                        return kSTAFAlreadyExists;
                    }
                }
                else
                {
                    return kSTAFAlreadyExists;
                }
            }
        }

        // Add an entry to the FS Copy map
    
        unsigned int mapID = ++fMapID;

        copyDataPtr = STAFRefPtr<FSCopyData>(
            new FSCopyData(mapID, name, machine,
                           direction, type, mode, fileSize),
            STAFRefPtr<FSCopyData>::INIT);

        fFSCopyMap[mapID] = copyDataPtr;
    }

    if (direction == kSTAFFSCopyTo)
    {
        if (STAFTrace::doTrace(kSTAFTraceServiceRequest) &&
            STAFServiceManager::doTraceService("FS"))
        {
            // Record a FS Copy Request Started trace message

            STAFString message = "FS Copy Request Started - From Machine: " +
                machine;
        
            if (type == kSTAFFSFileCopy)
            {
                message += ", To File: " + name;

                if (mode == kSTAFFSBinary)
                    message += ", Mode: Binary";
                else
                    message += ", Mode: Text";
            }
            else
            {
                message += ", To Directory: " + name;
            }

            STAFTrace::trace(kSTAFTraceServiceRequest, message);
        }
    }
    
    return kSTAFOk;
}

STAFRC_t STAFFSCopyManager::updateFileCopy1(
    FSCopyDataPtr &copyDataPtr, unsigned int fileSize, unsigned int mode)
{
    copyDataPtr->fileSize = fileSize;
    copyDataPtr->bytesCopied = 0;
    copyDataPtr->mode = mode;

    return kSTAFOk;
}

STAFRC_t STAFFSCopyManager::updateFileCopy(
    FSCopyDataPtr &copyDataPtr, unsigned int bytesCopied)
{
    copyDataPtr->bytesCopied = bytesCopied;

    return kSTAFOk;
}


STAFRC_t STAFFSCopyManager::updateDirectoryCopy(
    FSCopyDataPtr &copyDataPtr, const STAFString &name, unsigned int mode,
    unsigned int fileSize)
{
    copyDataPtr->entryName = name;
    copyDataPtr->mode = mode;
    copyDataPtr->fileSize = fileSize;
    copyDataPtr->bytesCopied = 0;

    return kSTAFOk;
}


STAFRC_t STAFFSCopyManager::remove(FSCopyDataPtr &copyDataPtr)
{
    if (copyDataPtr->direction == kSTAFFSCopyTo)
    {
        if (STAFTrace::doTrace(kSTAFTraceServiceResult) &&
            STAFServiceManager::doTraceService("FS"))
        {
            // Record a FS Copy Request Completed trace message

            STAFString message = "FS Copy Request Completed - From Machine: " +
                copyDataPtr->machine;
        
            if (copyDataPtr->type == kSTAFFSFileCopy)
            {
                message += ", To File: " + copyDataPtr->name;

                if (copyDataPtr->mode == kSTAFFSBinary)
                    message += ", Mode: Binary";
                else
                    message += ", Mode: Text";
            }
            else
            {
                message += ", To Directory: " + copyDataPtr->name;
            }

            STAFTrace::trace(kSTAFTraceServiceResult, message);
        }
    }

    // Remove the entry from the FS Copy map

    STAFMutexSemLock fsCopyMapLock(fFSCopyMapSem);
    
    fFSCopyMap.erase(copyDataPtr->mapID);
    
    // XXX: Should we add this line?
    // copyDataPtr = STAFRefPtr<FSCopyData>();

    return kSTAFOk;
}


unsigned int STAFFSCopyManager::getMapID()
{
    return fMapID;
}


STAFFSCopyManager::FSCopyMap STAFFSCopyManager::getFSCopyMapCopy()
{
    STAFMutexSemLock fsCopyMapLock(fFSCopyMapSem);

    return fFSCopyMap;
}
