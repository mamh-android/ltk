/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFMutexSem.h"


struct STAFMutexSemImplementation
{
    HANDLE fHandle;
};


STAFRC_t STAFMutexSemConstruct(STAFMutexSem_t *pMutex, 
                               const char *name,
                               unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;
    
    if (pMutex == 0) return kSTAFInvalidObject;

    try
    {
        *pMutex = new STAFMutexSemImplementation;
        STAFMutexSemImplementation &mutexSem = **pMutex;

        if (name == 0)
        {
            HANDLE theHandle = CreateMutex(0, FALSE, 0);
            mutexSem.fHandle = theHandle;

            if (theHandle == 0)
            {
                if (osRC) *osRC = GetLastError();
                delete *pMutex;
                return kSTAFBaseOSError;
            }
        }
        else
        {
            // we currently not support named mutexes
            if (osRC) *osRC = GetLastError();
            delete *pMutex;
            return kSTAFInvalidParm;
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }
    
    return rc;
}


STAFRC_t STAFMutexSemRequest(STAFMutexSem_t mutex,
                             unsigned int timeout, unsigned int *osRC)
{
    if (mutex == 0) return kSTAFInvalidObject;

    DWORD rc = WaitForSingleObject(mutex->fHandle, timeout);

    if ((rc != WAIT_TIMEOUT) && (rc != WAIT_ABANDONED) && (rc != WAIT_OBJECT_0))
    {
        if (osRC) *osRC = GetLastError();
        return kSTAFBaseOSError;
    }

    return (rc == WAIT_TIMEOUT) ? kSTAFTimeout : kSTAFOk;
}


STAFRC_t STAFMutexSemRelease(STAFMutexSem_t mutex, unsigned int *osRC)
{
    if (mutex == 0) return kSTAFInvalidObject;

    BOOL rc = ReleaseMutex(mutex->fHandle);

    if ((rc == FALSE) && (osRC))
        *osRC = GetLastError();

    return (rc == TRUE ? kSTAFOk : kSTAFBaseOSError);
}


STAFRC_t STAFMutexSemDestruct(STAFMutexSem_t *pMutex, unsigned int *osRC)
{
    if (pMutex == 0) return kSTAFInvalidObject;

    CloseHandle((*pMutex)->fHandle);
    delete *pMutex;
    *pMutex = 0;
    return kSTAFOk;
}
