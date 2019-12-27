/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <list>
#include "STAFRWSem.h"
#include "STAFEventSem.h"
#include "STAFMutexSem.h"
#include "STAFThreadManager.h"

struct RWSemWaiter
{
    RWSemWaiter(bool isReader)
        : fIsReader(isReader), fEvent(new STAFEventSem, STAFEventSemPtr::INIT)
    { /* Do Nothing */ }

    bool operator==(const RWSemWaiter &rhs)
    { return (fEvent == rhs.fEvent); }

    bool fIsReader;
    STAFEventSemPtr fEvent;
};

typedef std::list<RWSemWaiter> RWSemWaiterList;


struct STAFRWSemImplementation
{
    STAFMutexSem fImplSem;
    bool fIsOwned;
    unsigned int fCurrReaders;
    RWSemWaiterList fWaiters;
};


STAFRC_t STAFRWSemConstructCommon(STAFRWSem_t *pRWSem, const char *name,
                                  unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (pRWSem == 0) return kSTAFInvalidObject;

    try
    {
        // We don't currently support named r/w sems
        if (name != 0) return kSTAFInvalidParm;

        *pRWSem = new STAFRWSemImplementation;
        STAFRWSemImplementation &rwSem = **pRWSem;
        rwSem.fIsOwned = false;
        rwSem.fCurrReaders = 0;
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFRWSemQueryCommon(STAFRWSem_t rwsem, STAFRWSemInfo *rwSemInfo,
                              unsigned int *osRC)
{
    if (rwsem == 0) return kSTAFInvalidObject;
    if (rwSemInfo == 0) return kSTAFInvalidParm;

    STAFRWSemImplementation &rwSem = *rwsem;
    STAFRC_t rc = kSTAFOk;

    try
    {
        STAFMutexSemLock lock(rwSem.fImplSem);

        rwSemInfo->numReaders = rwSem.fCurrReaders;

        if (rwSem.fIsOwned && (rwSem.fCurrReaders == 0))
            rwSemInfo->numWriters = 1;
        else
            rwSemInfo->numWriters = 0;

        rwSemInfo->numWaitingReaders = 0;
        rwSemInfo->numWaitingWriters = 0;

        for (RWSemWaiterList::iterator iter = rwSem.fWaiters.begin();
             iter != rwSem.fWaiters.end(); ++iter)
        {
            if ((*iter).fIsReader) ++rwSemInfo->numWaitingReaders;
            else ++rwSemInfo->numWaitingWriters;
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFRWSemReadLockCommon(STAFRWSem_t rwsem, unsigned int timeout,
                                 unsigned int *osRC)
{
    if (rwsem == 0) return kSTAFInvalidObject;

    STAFRWSemImplementation &rwSem = *rwsem;
    STAFRC_t rc = kSTAFOk;

    try
    {
        rwSem.fImplSem.request();

        if ((rwSem.fIsOwned && (rwSem.fCurrReaders == 0)) ||
            (rwSem.fWaiters.size() != 0))
        {
            // There is a writer, so we have to put ourselves on the list
            // and wait to get control

            RWSemWaiter waiter(true);
            rwSem.fWaiters.push_back(waiter);
            rwSem.fImplSem.release();

            rc = waiter.fEvent->wait(timeout);

            if (rc == kSTAFOk)
            {
                // We have a read lock.  We were automatically taken off the
                // wait queue and fCurrReaders was incremented.  We have nothing
                // to do but be happy.
            }
            else
            {
                // We need to take ourselves off the list

                rwSem.fImplSem.request();
                rwSem.fWaiters.remove(waiter);
                rwSem.fImplSem.release();
            }
        }
        else
        {
            // We get to be a reader

            rwSem.fIsOwned = true;
            ++rwSem.fCurrReaders;
            rwSem.fImplSem.release();
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFRWSemReadUnlockCommon(STAFRWSem_t rwsem, unsigned int *osRC)
{
    if (rwsem == 0) return kSTAFInvalidObject;

    STAFRWSemImplementation &rwSem = *rwsem;
    STAFRC_t rc = kSTAFOk;

    try
    {
        STAFMutexSemLock lock(rwSem.fImplSem);

        if (--rwSem.fCurrReaders == 0)
        {
            if (rwSem.fWaiters.size() != 0)
            {
                if (rwSem.fWaiters.front().fIsReader)
                {
                    do
                    {
                        rwSem.fWaiters.front().fEvent->post();
                        rwSem.fWaiters.pop_front();
                        ++rwSem.fCurrReaders;
                    } while (rwSem.fWaiters.size() != 0 &&
                             rwSem.fWaiters.front().fIsReader);
                }
                else
                {
                    rwSem.fWaiters.front().fEvent->post();
                    rwSem.fWaiters.pop_front();
                }
            }
            else rwSem.fIsOwned = false;
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFRWSemWriteLockCommon(STAFRWSem_t rwsem, unsigned int timeout,
                                  unsigned int *osRC)
{
    if (rwsem == 0) return kSTAFInvalidObject;

    STAFRWSemImplementation &rwSem = *rwsem;
    STAFRC_t rc = kSTAFOk;

    try
    {
        rwSem.fImplSem.request();

        if (rwSem.fIsOwned)
        {
            // There is a reader or writer, so we have to put ourselves on the
            // list and wait to get control

            RWSemWaiter waiter(false);
            rwSem.fWaiters.push_back(waiter);
            rwSem.fImplSem.release();

            rc = waiter.fEvent->wait(timeout);

            if (rc == kSTAFOk)
            {
                // We have a write lock.  We were automatically taken off the
                // wait queue.  We have nothing to do but be happy.
            }
            else
            {
                // We need to take ourselves off the list

                rwSem.fImplSem.request();
                rwSem.fWaiters.remove(waiter);
                rwSem.fImplSem.release();
            }
        }
        else
        {
            // We get to be a writer

            rwSem.fIsOwned = true;
            rwSem.fImplSem.release();
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFRWSemWriteUnlockCommon(STAFRWSem_t rwsem, unsigned int *osRC)
{
    if (rwsem == 0) return kSTAFInvalidObject;

    STAFRWSemImplementation &rwSem = *rwsem;
    STAFRC_t rc = kSTAFOk;

    try
    {
        STAFMutexSemLock lock(rwSem.fImplSem);

        if (rwSem.fWaiters.size() != 0)
        {
            if (rwSem.fWaiters.front().fIsReader)
            {
                do
                {
                    rwSem.fWaiters.front().fEvent->post();
                    rwSem.fWaiters.pop_front();
                    ++rwSem.fCurrReaders;
                } while (rwSem.fWaiters.size() != 0 &&
                         rwSem.fWaiters.front().fIsReader);
            }
            else
            {
                rwSem.fWaiters.front().fEvent->post();
                rwSem.fWaiters.pop_front();
            }
        }
        else rwSem.fIsOwned = false;
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFRWSemDestructCommon(STAFRWSem_t *pRWSem, unsigned int *osRC)
{
    if (pRWSem == 0) return kSTAFInvalidObject;

    STAFRC_t rc = kSTAFOk;
    STAFRWSemImplementation &rwSem = **pRWSem;

    try
    {
        {
            STAFMutexSemLock lock(rwSem.fImplSem);

            if (rwSem.fIsOwned) return kSTAFSemaphoreHasPendingRequests;
        }

        delete *pRWSem;
        *pRWSem = 0;
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}
