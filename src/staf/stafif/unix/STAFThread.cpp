/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <cstdlib>
#include <time.h>
#include "STAFOSTypes.h"
#include "STAFThread.h"
#include "STAFString.h"
#include "STAFUtil.h"

struct STAFThread
{
    STAFThread(STAFThreadFunc_t theFunc, void *theData)
        : func(theFunc), data(theData)
    { /* Do Nothing */ }

    STAFThreadFunc_t func;
    void *data;
};


static void *RealSTAFThread(void *data);


STAFRC_t STAFThreadStart(STAFThreadID_t *threadID,
    STAFThreadFunc_t theFunc, void *theData, unsigned int flags,
    unsigned int *osRC)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);

#ifdef STAF_OS_NAME_ZOS
    pthread_attr_setweight_np(&attr, __MEDIUM_WEIGHT);
    pthread_attr_setsynctype_np(&attr, __PTATASYNCHRONOUS);
#else
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif

    // Note: UNIXWARE pthreads have a default stack size that is too small

#ifdef    STAF_OS_NAME_UNIXWARE
    pthread_attr_setstacksize(&attr, 0x00010000);
#endif // STAF_OS_NAME_UNIXWARE

    // Note: HPUX pthreads have a default stack size that is too small
    //       Had to increase to 384k (0x00060000) to register Java services

#ifdef    STAF_OS_NAME_HPUX
    pthread_attr_setstacksize(&attr, 0x00060000);
#endif // STAF_OS_NAME_HPUX

    // Note: AIX pthreads have a default stack size that is too small

#ifdef    STAF_OS_NAME_AIX
    pthread_attr_setstacksize(&attr, 0x00040000);
#endif // STAF_OS_NAME_AIX

    // Note: Linux pthreads have a default stack size that is too large (8M or 10M)
    //       Decrease default stack size to 4M to be able to create more threads.

#ifdef STAF_OS_NAME_LINUX
    size_t newThreadStackSize = 4194304; // 4M
    size_t currentThreadStackSize = 0;

    pthread_attr_getstacksize(&attr, &currentThreadStackSize);

    if (newThreadStackSize < currentThreadStackSize)
        pthread_attr_setstacksize(&attr, newThreadStackSize);
#endif // STAF_OS_NAME_LINUX

    if (getenv("STAF_THREAD_STACK_SIZE") != NULL)
    {
        // Environment variable STAF_THREAD_STACK_SIZE, if set, contains the
        // thread stack size in kilobytes

        STAFString threadStackSizeString = getenv("STAF_THREAD_STACK_SIZE");

        // Max thread stack size is UINT_MAX divided by 1,024 (1K) 

        unsigned int maxThreadStackSize = UINT_MAX / 1024;
        STAFString_t errorBufferT = 0;
        unsigned int threadStackSize;

        STAFRC_t rc = STAFUtilConvertStringToUInt(
            threadStackSizeString.getImpl(), STAFString("").getImpl(),
            &threadStackSize, &errorBufferT, 1, maxThreadStackSize);

        if (rc == kSTAFOk)
        {
            // Pass stack size in bytes

            pthread_attr_setstacksize(&attr, (size_t)threadStackSize * 1024);
        }
    }

    STAFThread *pThread = new STAFThread(theFunc, theData);

    unsigned int rc = pthread_create(threadID, &attr, RealSTAFThread, pThread); 

    if (rc && osRC) *osRC = rc; 

    return rc ? kSTAFCreateThreadError : kSTAFOk;
}


void *RealSTAFThread(void *data)
{
    STAFThread *pThread = static_cast<STAFThread *>(data);

    try
    {
        unsigned int rc = pThread->func(pThread->data);
    }
    catch (...)
    { /* Do Nothing */ }

    delete pThread;

    return 0;
}


STAFThreadID_t STAFThreadCurrentThreadID()
{
    return pthread_self();
}


STAFRC_t STAFThreadSleepCurrentThread(unsigned int milliseconds,
                                      unsigned int *osRC)
{
    struct timeval theTime = { (milliseconds / 1000),
                               ((milliseconds % 1000) * 1000) };

    int rc = select(0, 0, 0, 0, &theTime);

    if (rc < 0)
    {
        if (osRC) *osRC = errno;
        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}


static pthread_mutex_t threadSafeMutex = PTHREAD_MUTEX_INITIALIZER;

STAFThreadSafeScalar_t STAFThreadSafeIncrement(STAFThreadSafeScalar_t *ptr)
{
  pthread_mutex_lock(&threadSafeMutex);
  STAFThreadSafeScalar_t retval = ++(*ptr);
  pthread_mutex_unlock(&threadSafeMutex);
  return retval;
}


STAFThreadSafeScalar_t STAFThreadSafeDecrement(STAFThreadSafeScalar_t *ptr)
{
    pthread_mutex_lock(&threadSafeMutex);
    STAFThreadSafeScalar_t retval = --(*ptr);
    pthread_mutex_unlock(&threadSafeMutex);
    return retval;
}
