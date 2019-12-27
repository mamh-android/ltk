/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_RWSem
#define STAF_RWSem

#include "STAF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

#define STAF_RW_SEM_INDEFINITE_WAIT (unsigned int)-1

typedef struct STAFRWSemImplementation *STAFRWSem_t;

typedef struct
{
    unsigned int numReaders;
    unsigned int numWriters;
    unsigned int numWaitingReaders;
    unsigned int numWaitingWriters;
} STAFRWSemInfo;


/**************************************************************************/
/* Read/Write Semaphores are semaphores that allow multiple threads       */
/* to own a read lock on the semaphore at the same time, but only         */
/* one thread may own a write lock on the semaphore at any given time.    */
/* There may not be simultaneous read and write locks on the semaphore.   */
/*                                                                        */
/* If a write lock is requested while read locks exist, the request is    */
/* put on a queue.  All further lock requests (read or write) will be     */
/* placed on the queue until the write lock can be fullfilled.  When, the */
/* write lock is released, requests will be handled on a first come first */
/* serve basis.                                                           */
/**************************************************************************/


/***********************************************************************/
/* STAFRWSemConstruct - Creates a STAF Read/Write semaphore            */
/*                                                                     */
/* Accepts: (Out) Pointer to a r/w sem                                 */
/*          (In)  The name of the r/w sem (This should be 0/NULL for a */
/*                private r/w sem) (Currently only private r/w sems    */
/*                are supported)                                       */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemConstruct(STAFRWSem_t *pRWSem, const char *name,
                            unsigned int *osRC);


/***********************************************************************/
/* STAFRWSemQuery - Obtains information about a Read/Write semaphore   */
/*                                                                     */
/* Accepts: (In)  A Read/Write semaphore                               */
/*          (Out) Pointer to STAFRWSemInfo structure                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           kSTAFTimeout, on timeout                                  */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemQuery(STAFRWSem_t rwsem, STAFRWSemInfo *rwSemInfo,
                        unsigned int *osRC);


/***********************************************************************/
/* STAFRWSemReadLock - Requests a read lock on a STAF Read/Write       */
/*                     semaphore                                       */
/*                                                                     */
/* Accepts: (In)  A Read/Write semaphore                               */
/*          (In)  The amount of time to wait (in milliseconds)         */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           kSTAFTimeout, on timeout                                  */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemReadLock(STAFRWSem_t rwsem, unsigned int timeout,
                           unsigned int *osRC);


/***********************************************************************/
/* STAFRWSemReadUnlock - Releases ownership of a STAF Read/Write       */
/*                       read lock                                     */
/*                                                                     */
/* Accepts: (In)  A Read/Write semaphore                               */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemReadUnlock(STAFRWSem_t rwsem, unsigned int *osRC);


/***********************************************************************/
/* STAFRWSemWriteLock - Requests a write lock on a STAF Read/Write     */
/*                      semaphore                                      */
/*                                                                     */
/* Accepts: (In)  A Read/Write semaphore                               */
/*          (In)  The amount of time to wait (in milliseconds)         */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           kSTAFTimeout, on timeout                                  */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemWriteLock(STAFRWSem_t rwsem, unsigned int timeout,
                            unsigned int *osRC);


/***********************************************************************/
/* STAFRWSemWriteUnlock - Releases ownership of a STAF Read/Write      */
/*                        write lock                                   */
/*                                                                     */
/* Accepts: (In)  Pointer to a r/w sem                                 */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemWriteUnlock(STAFRWSem_t rwsem, unsigned int *osRC);


/***********************************************************************/
/* STAFRWSemDestruct - Destructs a STAF Read/Write semaphore           */
/*                                                                     */
/* Accepts: (In)  Pointer to a r/w sem                                 */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFRWSemDestruct(STAFRWSem_t *pRWSem, unsigned int *osRC);

/* End C language definitions */

/* The following definitions are for OS independent versions of the */
/* Read/Write APIs.  These should not be used by user applications. */

STAFRC_t STAFRWSemConstructCommon(STAFRWSem_t *pRWSem, const char *name,
                                  unsigned int *osRC);
STAFRC_t STAFRWSemQueryCommon(STAFRWSem_t rwsem, STAFRWSemInfo *rwSemInfo,
                              unsigned int *osRC);
STAFRC_t STAFRWSemReadLockCommon(STAFRWSem_t rwsem, unsigned int timeout,
                                 unsigned int *osRC);
STAFRC_t STAFRWSemReadUnlockCommon(STAFRWSem_t rwsem, unsigned int *osRC);
STAFRC_t STAFRWSemWriteLockCommon(STAFRWSem_t rwsem, unsigned int timeout,
                                  unsigned int *osRC);
STAFRC_t STAFRWSemWriteUnlockCommon(STAFRWSem_t rwsem, unsigned int *osRC);
STAFRC_t STAFRWSemDestructCommon(STAFRWSem_t *pRWSem, unsigned int *osRC);

#ifdef __cplusplus
}

// Begin C++ language definitions

#include "STAFRefPtr.h"
#include "STAFException.h"

// Forward declaration for typedef
class STAFRWSem;
typedef STAFRefPtr<STAFRWSem> STAFRWSemPtr;

// STAFRWSem - This class provides a C++ wrapper around the STAF Read/Write
//             semaphore C APIs.  This class will throw exceptions in all
//             error cases except for a timeout on a request().

class STAFRWSem
{
public:

    STAFRWSem();

    // Returns: kSTAFOk, if the semaphore was acquired
    //          kSTAFTimeout, if you timed out waiting     
    STAFRC_t readLock(unsigned int timeout = STAF_RW_SEM_INDEFINITE_WAIT);
    void readUnlock();

    // Returns: kSTAFOk, if the semaphore was acquired
    //          kSTAFTimeout, if you timed out waiting     
    STAFRC_t writeLock(unsigned int timeout = STAF_RW_SEM_INDEFINITE_WAIT);
    void writeUnlock();

    ~STAFRWSem();

private:

    // Don't allow copy or assignment
    STAFRWSem(const STAFRWSem &);
    STAFRWSem &operator=(const STAFRWSem &);

    STAFRWSem_t fRWSemImpl;
};


// STAFRWSemRLock - This class provides a simply way to acquire a STAFRWSem
//                  read lock for the duration of a block

class STAFRWSemRLock
{
public:

    STAFRWSemRLock(STAFRWSem &theRWSem,
                   unsigned int timeout = STAF_RW_SEM_INDEFINITE_WAIT)
        : fRWSem(theRWSem)
    { fRWSem.readLock(timeout); }

    ~STAFRWSemRLock()
    { fRWSem.readUnlock(); }

private:

    STAFRWSem &fRWSem;
};


// STAFRWSemWLock - This class provides a simply way to acquire a STAFRWSem
//                  write lock for the duration of a block

class STAFRWSemWLock
{
public:

    STAFRWSemWLock(STAFRWSem &theRWSem,
                   unsigned int timeout = STAF_RW_SEM_INDEFINITE_WAIT)
        : fRWSem(theRWSem)
    { fRWSem.writeLock(timeout); }

    ~STAFRWSemWLock()
    { fRWSem.writeUnlock(); }

private:

    STAFRWSem &fRWSem;
};



// Begin inline definitions for STAFRWSem class

inline STAFRWSem::STAFRWSem() : fRWSemImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFRWSemConstruct(&fRWSemImpl, 0, &osRC);

    STAFException::checkRC(rc, "STAFRWSemConstruct", osRC);
}


inline STAFRC_t STAFRWSem::readLock(unsigned int timeout)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFRWSemReadLock(fRWSemImpl, timeout, &osRC);

    if ((rc != kSTAFOk) && (rc != kSTAFTimeout))
        STAFException::checkRC(rc, "STAFRWSemReadLock", osRC);

    return rc;
}


inline void STAFRWSem::readUnlock()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFRWSemReadUnlock(fRWSemImpl, &osRC);

    STAFException::checkRC(rc, "STAFRWSemReadUnlock", osRC);
}


inline STAFRC_t STAFRWSem::writeLock(unsigned int timeout)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFRWSemWriteLock(fRWSemImpl, timeout, &osRC);

    if ((rc != kSTAFOk) && (rc != kSTAFTimeout))
        STAFException::checkRC(rc, "STAFRWSemWriteLock", osRC);

    return rc;
}


inline void STAFRWSem::writeUnlock()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFRWSemWriteUnlock(fRWSemImpl, &osRC);

    STAFException::checkRC(rc, "STAFRWSemWriteUnlock", osRC);
}


inline STAFRWSem::~STAFRWSem()
{
  unsigned int osRC = 0;
  STAFRC_t rc = STAFRWSemDestruct(&fRWSemImpl, &osRC);
}

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif


