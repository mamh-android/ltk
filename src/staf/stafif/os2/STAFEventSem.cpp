/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <errno.h>
#include "STAFOSTypes.h"
#include "STAFEventSem.h"
#include "STAFString.h"


struct STAFEventSemImplementation
{
    HEV fHandle;
};


unsigned int STAFEventSemConstruct(STAFEventSem_t *pEvent, 
                                   const char *name,
                                   unsigned int *osRC)
{
    unsigned int rc = 0;
    
    if (pEvent == 0) return 2;

    try
    {
        *pEvent = new STAFEventSemImplementation;
        STAFEventSemImplementation &eventSem = **pEvent;
        APIRET theRC = 0;

        if (name == 0)
        {
            theRC = DosCreateEventSem(0, &eventSem.fHandle, 0, 0);
        }
        else
        {
            STAFString theName("/SEM32/STAF/EventSem/");
            theName += STAFString(name);

            theRC =  DosCreateEventSem(
                         reinterpret_cast<const unsigned char *>(
                             theName.toCurrentCodePage()->buffer()),
                         &eventSem.fHandle, 0, 0);
        }

        if (theRC != 0)
        {
            if (osRC) *osRC = theRC;
            delete *pEvent;
            return 1;
        }
    }
    catch (...)
    { rc = 1; if (osRC) *osRC = 0xFFFFFFFF; }
    
    return rc;
}


unsigned int STAFEventSemPost(STAFEventSem_t pEvent, unsigned int *osRC)
{
    APIRET rc = DosPostEventSem(pEvent->fHandle);

    if ((rc != 0) && (rc != ERROR_ALREADY_POSTED) &&
        (rc != ERROR_TOO_MANY_POSTS))
    {
        if (osRC) *osRC = rc;
        return 1;
    }

    return 0;
}


unsigned int STAFEventSemReset(STAFEventSem_t pEvent, unsigned int *osRC)
{
    unsigned long postCount = 0;
    APIRET rc = DosResetEventSem(pEvent->fHandle, &postCount);

    if ((rc != 0) && (rc != ERROR_ALREADY_RESET))
    {
        if (osRC) *osRC = rc;
        return 1;
    }

    return 0;
}


unsigned int STAFEventSemWait(STAFEventSem_t pEvent, unsigned int timeout,
                              unsigned int *osRC)
{
    APIRET rc = DosWaitEventSem(pEvent->fHandle, timeout);

    if (rc == ERROR_TIMEOUT)
    {
        return 1;
    }
    else if (rc != 0)
    {
        if (osRC) *osRC = rc;
        return 2;
    }

    return 0;
}


unsigned int STAFEventSemQuery(STAFEventSem_t pEvent, unsigned int *osRC)
{
    unsigned long postCount = 0;
    APIRET rc = DosQueryEventSem(pEvent->fHandle, &postCount);

    if (rc != 0)
    {
        if (osRC) *osRC = rc;
        return 2;
    }

    return (postCount > 0) ? 1 : 0;
}


unsigned int STAFEventSemDestruct(STAFEventSem_t *pEvent, unsigned int *osRC)
{
    if (pEvent == 0) return 2;

    DosCloseEventSem((*pEvent)->fHandle);
    delete *pEvent;
    *pEvent = 0;
    return 0;
}

