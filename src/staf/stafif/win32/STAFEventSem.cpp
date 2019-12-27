/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <errno.h>
#include "STAFOSTypes.h"
#include "STAFEventSem.h"


struct STAFEventSemImplementation
{
    HANDLE fHandle;
};


STAFRC_t STAFEventSemConstruct(STAFEventSem_t *pEvent, 
                               const char *name,
                               unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;
    
    if (pEvent == 0) return kSTAFInvalidObject;

    try
    {
        *pEvent = new STAFEventSemImplementation;
        STAFEventSemImplementation &eventSem = **pEvent;

        if (name == 0)
        {
            HANDLE theHandle = CreateEvent(0, TRUE, FALSE, 0);
                               eventSem.fHandle = theHandle;

            if (theHandle == 0)
            {
                if (osRC) *osRC = GetLastError();
                delete *pEvent;
                return kSTAFBaseOSError;
            }
        }
        else
        {
            STAFString theName = "STAF/EventSem/";
            theName += STAFString(name);

            HANDLE theHandle = CreateEvent(0, TRUE, FALSE, 
                               theName.toCurrentCodePage()->buffer());
            eventSem.fHandle = theHandle;

            if (eventSem.fHandle == 0)
            {
                if (osRC) *osRC = GetLastError();
                delete *pEvent;
                return kSTAFBaseOSError;
            }
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }
    
    return rc;
}


STAFRC_t STAFEventSemPost(STAFEventSem_t pEvent, unsigned int *osRC)
{
    BOOL rc = SetEvent(pEvent->fHandle);

    if ((rc == FALSE) && ((GetLastError() == ERROR_SEM_IS_SET) ||
        (GetLastError() == ERROR_TOO_MANY_POSTS)))
    {
        // These are not true error conditions??
        rc = TRUE;
    }
    else if (rc != TRUE && osRC)
    {
        *osRC = GetLastError();
    }

    return (rc == TRUE ? kSTAFOk : kSTAFBaseOSError);
}


STAFRC_t STAFEventSemReset(STAFEventSem_t pEvent, unsigned int *osRC)
{
    BOOL rc = ResetEvent(pEvent->fHandle);

    // Note: On Win95/NT it is not an error to call reset on an already
    //       reset semaphore

    if (rc == FALSE && osRC)
    {
        *osRC = GetLastError();
    }

    return (rc == TRUE ? kSTAFOk : kSTAFBaseOSError);
}


STAFRC_t STAFEventSemWait(STAFEventSem_t pEvent, unsigned int timeout,
                          unsigned int *osRC)
{
    DWORD rc = WaitForSingleObject(pEvent->fHandle, timeout);

    if ((rc != WAIT_TIMEOUT) && (rc != WAIT_ABANDONED) && 
        (rc != WAIT_OBJECT_0) && (osRC))
    {
        // XXX: CHECKRC("WaitForSingleObject");
        *osRC = GetLastError();
        return kSTAFBaseOSError;
    }

    return (rc == WAIT_TIMEOUT) ? kSTAFTimeout : kSTAFOk;
}


STAFRC_t STAFEventSemQuery(STAFEventSem_t pEvent, 
                           STAFEventSemState_t *pState, unsigned int *osRC)
{
    if (pEvent == 0) return kSTAFInvalidObject;

    if (pState == 0) return kSTAFInvalidParm;

    DWORD rc = WaitForSingleObject(pEvent->fHandle, 0);

    // Note: I have no idea what 258 really is, but it occurs when calling
    //       this after a reset()

    if ((rc != WAIT_TIMEOUT) && (rc != WAIT_ABANDONED)  &&
        (rc != WAIT_OBJECT_0) && (rc != 258) && (osRC))
    {
        // XXX: CHECKRC("WaitForSingleObject");
        *osRC = GetLastError();
        return kSTAFBaseOSError;
    }

    *pState = ((rc == WAIT_TIMEOUT) || (rc == 258)) ? kSTAFEventSemReset 
                                                    : kSTAFEventSemPosted;
    return kSTAFOk;
}


STAFRC_t STAFEventSemDestruct(STAFEventSem_t *pEvent, unsigned int *osRC)
{
    if (pEvent == 0) return kSTAFInvalidObject;

    CloseHandle((*pEvent)->fHandle);
    delete *pEvent;
    *pEvent = 0;
    return kSTAFOk;
}

