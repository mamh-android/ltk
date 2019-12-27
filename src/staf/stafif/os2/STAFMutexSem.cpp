/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFMutexSem.h"
#include "STAFString.h"


struct STAFMutexSemImplementation
{
    HMTX fHandle;
};



unsigned int STAFMutexSemConstruct(STAFMutexSem_t *pMutex,
                                   const char *name,
                                   unsigned int *osRC)
{
   unsigned int rc = 0;

   if (pMutex == 0) return 3;

    try
    {
        *pMutex = new STAFMutexSemImplementation;
        STAFMutexSemImplementation &mutexSem = **pMutex;
        APIRET theOSRC = 0;

        if (name == 0)
        {
            theOSRC = DosCreateMutexSem(0, &mutexSem.fHandle, 0, 0);
        }
        else
        {
            STAFString theName("/SEM32/STAF/MutexSem/");
            theName += name;

            theOSRC = DosCreateMutexSem(
                          reinterpret_cast<const unsigned char *>(
                              theName.toCurrentCodePage()->buffer()),
                          &mutexSem.fHandle, 0, 0);
        }

        if (theOSRC != 0)
        {
            if (osRC) *osRC = theOSRC;
            delete *pMutex;
            return 1;
        }
    }
    catch (...)
    { rc = 1; *osRC = 0xFFFFFFFF; }

    return rc;
}


unsigned int STAFMutexSemRequest(STAFMutexSem_t mutex,
                                 unsigned int timeout, unsigned int *osRC)
{
    if (mutex == 0) return 3;

    APIRET rc = DosRequestMutexSem(mutex->fHandle, timeout);

    if ((rc != 0) && (rc != ERROR_TIMEOUT))
    {
        if (osRC) *osRC = rc;
        return 2;
    }

    return (rc == ERROR_TIMEOUT) ? 1 : 0;
}


unsigned int STAFMutexSemRelease(STAFMutexSem_t mutex, unsigned int *osRC)
{
    if (mutex == 0) return 2;

    APIRET rc = DosReleaseMutexSem(pMutex->fHandle);

    if (rc != 0)
    {
        if (osRC) *osRC = rc;
        return 1;
    }

    return 0;
}


unsigned int STAFMutexSemDestruct(STAFMutexSem_t *pMutex, unsigned int *osRC)
{
    if (pMutex == 0) return 2;

    DosCloseMutexSem((*pMutex)->fHandle);
    delete *pMutex;
    *pMutex = 0;
    return 0;
}


