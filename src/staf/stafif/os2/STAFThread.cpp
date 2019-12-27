/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFThread.h"
#include <stdlib.h>


unsigned int STAFThreadStart(STAFThreadID_t *threadID,
    STAFThreadFunctionReturn_t (SYSLINK *theFunc)(STAFThreadFunctionData_t),
    STAFThreadFunctionData_t theData, unsigned int flags, unsigned int *osRC)
{
    return _beginthread(theFunc, theData, 0, threadID) == 0;
}


STAFThreadID_t STAFThreadCurrentThreadID()
{
    PTIB ptib;
    PPIB ppib;
    APIRET rc = DosGetInfoBlocks(&ptib, &ppib);

    return rc ? 0 : ptib->tib_ptib2->tib2_ultid;
}


unsigned int STAFThreadSleepCurrentThread(unsigned int milliseconds,
                                          unsigned int *osRC)
{
    APIRET rc = DosSleep(milliseconds);

    if (!rc && osRC) *osRC = rc;

    return rc ? 1 : 0;
}

// XXX: Need to find a better way to do this
static bool sThreadSafeMutexInited = false;
static HMTX sThreadSafeMutex = 0;

STAFThreadSafeScalar_t STAFThreadSafeIncrement(STAFThreadSafeScalar_t *ptr)
{
    if (sThreadSafeMutexInited == false)
    {
        DosEnterCritSec();
        if (sThreadSafeMutexInited == false)
            DosCreateMutexSem(0, &sThreadSafeMutex, 0, 0);
        DosExitCritSec();
    }

    DosRequestMutexSem(sThreadSafeMutex, SEM_INDEFINITE_WAIT);
    STAFThreadSafeScalar_t retval = ++(*ptr);
    DosReleaseMutexSem(sThreadSafeMutex);

    return retval;
}


STAFThreadSafeScalar_t STAFThreadSafeDecrement(STAFThreadSafeScalar_t *ptr)
{
    if (sThreadSafeMutexInited == false)
    {
        DosEnterCritSec();
        if (sThreadSafeMutexInited == false)
            DosCreateMutexSem(0, &sThreadSafeMutex, 0, 0);
        DosExitCritSec();
    }

    DosRequestMutexSem(sThreadSafeMutex, SEM_INDEFINITE_WAIT);
    STAFThreadSafeScalar_t retval = --(*ptr);
    DosReleaseMutexSem(sThreadSafeMutex);

    return retval;
}
