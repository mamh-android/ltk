/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ThreadManager 
#define STAF_ThreadManager

#include "STAF.h"
#include <deque>
#include "STAFEventSem.h"
#include "STAFMutexSem.h"
#include "STAFThread.h"
#include "STAFRefPtr.h"


// STAFThreadManager - Handles interacting with the systems threads

class STAFThreadManager
{
public:

    STAFThreadManager(unsigned int initThreads = 1,
                      unsigned int growthDelta = 1);

    // Some utility routines that allow us to get our current thread ID and
    // to sleep for a given amount of time
    static STAFThreadID_t getCurrentThreadID();
    static void sleepCurrentThread(unsigned int milliseconds);

    // This dispatches the provided function on a thread.  It returns 0 if
    // the function was dispatched.  It returns >0 if the function could not
    // be dispatched (e.g., the thread pool could not be grown)
   
    unsigned int dispatch(STAFThreadFunc_t func, void *data);

    // Determines size of thread pool, and number of ready and working threads
    unsigned int getThreadPoolSize();
    unsigned int getNumReadyThreads();
    unsigned int getNumWorkingThreads();

    // Allow us to get and alter whether the thread pool can grow
    unsigned int getGrowthDelta();
    void setGrowthDelta(unsigned int growthDelta);

    // Grows the thread pool by the growth delta
    STAFRC_t growThreadPool();

    // Grows the thread pool by a specific amount
    STAFRC_t growThreadPool(unsigned int deltaThreads);

    ~STAFThreadManager();

private:

    // Disallow copying
    STAFThreadManager(const STAFThreadManager &);
    STAFThreadManager &operator=(const STAFThreadManager &);

    // This function grows the thread pool by the growth delta
    STAFRC_t doGrowThreadPool();

    // This function grows the thread pool by a specific amount
    STAFRC_t doGrowThreadPool(unsigned int deltaThreads);

    // This function simply launches the workerThread()
    static unsigned int callWorkerThread(void *manager);

    // This function handles executing work on a thread
    void workerThread();

    struct STAFThreadFunc
    {
        STAFThreadFunc(STAFThreadFunc_t theFunc, void *theData)
            : func(theFunc), data(theData)
        { /* Do Nothing */ }

        STAFThreadFunc_t func;
        void *data;
    };

    // This structure defines when/what work is a available

    struct STAFReadyThread;
    friend struct STAFReadyThread;

    struct STAFReadyThread
    {
        STAFReadyThread(STAFThreadFunc *theWork = 0) :
            work(theWork), alive(true)
        { /* Do Nothing */ }

        ~STAFReadyThread()
        { /* Do Nothing */; }

        STAFEventSem workAvailable;
        STAFThreadFunc *work;
        bool alive;
    };

    typedef std::deque<STAFReadyThread *> STAFThreadPool;

    unsigned int fGrowthDelta;
    unsigned int fThreadPoolSize;
    STAFReadyThread *fCurrReadyThread;
    STAFEventSem fWorkerSynchSem;
    STAFMutexSemPtr fThreadPoolSemPtr;
    STAFThreadPool fThreadList;
    STAFThreadPool fThreadPool;
};


typedef STAFRefPtr<STAFThreadManager> STAFThreadManagerPtr;

#endif

