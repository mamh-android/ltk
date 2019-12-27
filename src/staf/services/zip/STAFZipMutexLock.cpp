/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <map>
#include "STAFString.h"
#include "STAFTrace.h"
#include "STAFMutexSem.h"

#include "STAFZipMutexLock.h"


void STAFZipMutexLock::request(STAFString filename)
{

    ZipLock *pLock = 0;

    // First get the lock object from the map

    {
        STAFMutexSemLock mapLock(sLockMapSem);
        pLock = &sLockMap[filename];
    }

    // Now, get the underlying file system lock if needed, and increment
    // the number of owners

    (*pLock->lockSem).request(STAF_MUTEX_SEM_INDEFINITE_WAIT);

    ++pLock->numOwners;
}
    

void STAFZipMutexLock::release(STAFString filename)
{
    ZipLock *pLock = 0;

    // First get the lock object from the map

    {
        STAFMutexSemLock mapLock(sLockMapSem);
        pLock = &sLockMap[filename];
    }

    // Now, decrement the number of owners and release the underlying file
    // system lock if necessary

    (*pLock->lockSem).release();

    --pLock->numOwners;
    
    if (pLock->numOwners == 0)
    {
        // if this is the last owner, then remove the Mutex
        LockMap::iterator thisElement;

        if ((thisElement = sLockMap.find(filename)) != sLockMap.end())
        {
            sLockMap.erase(thisElement, thisElement);
        }
    }
}


