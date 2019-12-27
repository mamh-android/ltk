/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFThreadManager.h"
#include "STAFException.h"
#include "STAFTrace.h"


STAFThreadManager::STAFThreadManager(unsigned int initThreads,
                                     unsigned int growthDelta)
  : fGrowthDelta(growthDelta), fThreadPoolSize(0),
    fCurrReadyThread(0),
    fThreadPoolSemPtr(new STAFMutexSem, STAFMutexSemPtr::INIT)
{
    doGrowThreadPool(initThreads);
}


STAFThreadID_t STAFThreadManager::getCurrentThreadID()
{
    return STAFThreadCurrentThreadID();
}


void STAFThreadManager::sleepCurrentThread(unsigned int milliseconds)
{
    unsigned int osRC = 0;

    STAFThreadSleepCurrentThread(milliseconds, &osRC);
}


unsigned int STAFThreadManager::dispatch(STAFThreadFunc_t func, void *data)
{
    STAFMutexSemPtr threadPoolSemPtr = fThreadPoolSemPtr;
    STAFMutexSemLock semLock(*threadPoolSemPtr);

    if (fThreadPool.size() == 0)
    {
        // Need to grow pool
        if (!fGrowthDelta) return 1;

        unsigned int rc = doGrowThreadPool();

        if (rc != 0) return rc;
    }

    fCurrReadyThread = fThreadPool.back();
    fThreadPool.pop_back();

    fCurrReadyThread->work = new STAFThreadFunc(func, data);
    fCurrReadyThread->workAvailable.post();

    return 0;
}

unsigned int STAFThreadManager::getThreadPoolSize()
{
    return fThreadPoolSize;
}


unsigned int STAFThreadManager::getNumReadyThreads()
{
    STAFMutexSemPtr threadPoolSemPtr = fThreadPoolSemPtr;
    STAFMutexSemLock semLock(*threadPoolSemPtr);

    return fThreadPool.size();
}


unsigned int STAFThreadManager::getNumWorkingThreads()
{
    STAFMutexSemPtr threadPoolSemPtr = fThreadPoolSemPtr;
    STAFMutexSemLock semLock(*threadPoolSemPtr);

    return fThreadPoolSize - fThreadPool.size();
}


unsigned int STAFThreadManager::getGrowthDelta()
{
    return fGrowthDelta;
}


void STAFThreadManager::setGrowthDelta(unsigned int growthDelta)
{
    fGrowthDelta = growthDelta;
}


STAFRC_t STAFThreadManager::growThreadPool()
{
    return growThreadPool(fGrowthDelta);
}

STAFRC_t STAFThreadManager::growThreadPool(unsigned int deltaThreads)
{
    STAFMutexSemPtr threadPoolSemPtr = fThreadPoolSemPtr;
    STAFMutexSemLock semLock(*threadPoolSemPtr);
    return doGrowThreadPool(deltaThreads);
}


STAFRC_t STAFThreadManager::doGrowThreadPool()
{
    return doGrowThreadPool(fGrowthDelta);
}


STAFRC_t STAFThreadManager::doGrowThreadPool(unsigned int deltaThreads)
{
    for(int i = 0; i < deltaThreads; ++i)
    {
        fCurrReadyThread = new STAFReadyThread();
        fWorkerSynchSem.reset();

        // Note: STAFThreadID_t is not a simple integer type on z/OS, thus
        //       we can't initialize theThreadID

        STAFThreadID_t theThreadID;
        unsigned int osRC = 0;
        STAFRC_t rc = STAFThreadStart(&theThreadID, callWorkerThread,
                                      this, 0, &osRC);

        if (rc != kSTAFOk)
        {
            STAFString errMsg = STAFString(
                "STAFThreadManager::doGrowThreadPool:  "
                "Error creating a new thread.  May be out of memory.  RC: ") +
                STAFString(rc) + ", OSRC: " + STAFString(osRC);
            
            STAFTrace::trace(kSTAFTraceError, errMsg);

            return rc;
        }

        fWorkerSynchSem.wait();
        fThreadPool.push_back(fCurrReadyThread);
        fThreadList.push_back(fCurrReadyThread);
        ++fThreadPoolSize;
    }

    return 0;
}

unsigned int STAFThreadManager::callWorkerThread(void *manager)
{
     STAFThreadManager *threadManager =
         static_cast<STAFThreadManager *>(manager);

     threadManager->workerThread();

     return 0;
}


void STAFThreadManager::workerThread()
{
    try
    {
        STAFReadyThread *myThreadData = fCurrReadyThread;
        STAFMutexSemPtr threadPoolSemPtr = fThreadPoolSemPtr;
        fWorkerSynchSem.post();

        for(;;)
        {
            myThreadData->workAvailable.wait();
            myThreadData->workAvailable.reset();

            if (!myThreadData->alive) break;

            try
            {
                myThreadData->work->func(myThreadData->work->data);
                delete myThreadData->work;
            }
            catch (STAFException &se)
            {
                se.trace("STAFThreadManager::workerThread()");
            }
            catch (...)
            {
                STAFTrace::trace(
                    kSTAFTraceError, "Caught unknown exception in "
                    "STAFThreadManager::workerThread()");
            }
            
            // Add the thread back into the thread pool
            
            STAFMutexSemLock semLock(*threadPoolSemPtr);
            
            if (!myThreadData->alive) break;

            fThreadPool.push_back(myThreadData);
        }

        delete myThreadData;
    }
    catch (STAFException &se)
    {
        se.trace("STAFThreadManager::workerThread() outer try block" );
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Caught unknown exception in "
            "STAFThreadManager::workerThread() outer try block");
    }
}


STAFThreadManager::~STAFThreadManager()
{
    // Simply have all ready threads end

    {
        STAFMutexSemPtr threadPoolSemPtr = fThreadPoolSemPtr;
        STAFMutexSemLock semLock(*threadPoolSemPtr);

        while (fThreadList.size() > 0)
        {
            fCurrReadyThread = fThreadList.back();
            fThreadList.pop_back();

            fCurrReadyThread->alive = false;
            fCurrReadyThread->workAvailable.post();
        }
    }
}
