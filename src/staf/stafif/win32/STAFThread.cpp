/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFThread.h"
#include "STAFString.h"


struct STAFThread
{
    STAFThread(STAFThreadFunc_t theFunc, void *theData)
        : func(theFunc), data(theData)
    { /* Do Nothing */ }

    STAFThreadFunc_t func;
    void *data;
};


static unsigned int __stdcall RealSTAFThread(void *data)
{
    STAFThread *pThread = static_cast<STAFThread *>(data);

    unsigned int rc = pThread->func(pThread->data);

    delete pThread;

    return 0;
}

STAFRC_t STAFThreadStart(STAFThreadID_t *threadID,
    STAFThreadFunc_t theFunc, void *theData, unsigned int flags,
    unsigned int *osRC)
{
    unsigned threadStackSize = 0;
    
    if (getenv("STAF_THREAD_STACK_SIZE") != NULL)
    {
        STAFString threadStackSizeString = getenv("STAF_THREAD_STACK_SIZE");
        
        // Max thread stack size is UINT_MAX divided by 1,024 (1K) 

        unsigned int maxThreadStackSize = UINT_MAX / 1024;
        STAFString_t errorBufferT = 0;

        STAFRC_t rc = STAFUtilConvertStringToUInt(
            threadStackSizeString.getImpl(), STAFString("").getImpl(),
            &threadStackSize, &errorBufferT, 1, maxThreadStackSize);

        if (rc == kSTAFOk)
        {
            // Change from size in kilobytes to size in bytes

            threadStackSize *= 1024;
        }
    }

    unsigned int rc = _beginthreadex(
        0, threadStackSize, RealSTAFThread, new STAFThread(theFunc, theData),
        0, threadID);
    
    if (rc == 0)
    {
        *osRC = _doserrno;
        return kSTAFCreateThreadError;
	}

    return kSTAFOk;
}


STAFThreadID_t STAFThreadCurrentThreadID()
{
    return GetCurrentThreadId();
}


STAFRC_t STAFThreadSleepCurrentThread(unsigned int milliseconds,
                                      unsigned int *osRC)
{
    Sleep(milliseconds);

    return kSTAFOk;
}


STAFThreadSafeScalar_t STAFThreadSafeIncrement(STAFThreadSafeScalar_t *ptr)
{
    return InterlockedIncrement(ptr);
}


STAFThreadSafeScalar_t STAFThreadSafeDecrement(STAFThreadSafeScalar_t *ptr)
{
    return InterlockedDecrement(ptr);
}
