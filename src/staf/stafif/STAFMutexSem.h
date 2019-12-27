/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_MutexSem
#define STAF_MutexSem

#include "STAF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

#define STAF_MUTEX_SEM_INDEFINITE_WAIT (unsigned int)-1

typedef struct STAFMutexSemImplementation *STAFMutexSem_t;

/***********************************************************************/
/* STAFMutexSemConstruct - Creates a STAF Mutex sempahore              */
/*                                                                     */
/* Accepts: (Out) Pointer to a mutex                                   */
/*          (In)  The name of the mutex (This should be 0/NULL for a   */
/*                private mutex) (Currently only private mutexes are   */
/*                supported)                                           */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFMutexSemConstruct(STAFMutexSem_t *pMutex, 
                               const char *name, unsigned int *osRC);

/***********************************************************************/
/* STAFMutexSemRequest - Requests ownership of a STAF Mutex semaphore  */
/*                                                                     */
/* Accepts: (In)  Pointer to a mutex                                   */
/*          (In)  The amount of time to wait (in milliseconds)         */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           kSTAFTimeout, on timeout                                  */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFMutexSemRequest(STAFMutexSem_t mutex, unsigned int timeout,
                             unsigned int *osRC);


/***********************************************************************/
/* STAFMutexSemRelease - Releases ownership of a STAF Mutex semaphore  */
/*                                                                     */
/* Accepts: (In)  Pointer to a mutex                                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFMutexSemRelease(STAFMutexSem_t mutex, unsigned int *osRC);


/***********************************************************************/
/* STAFMutexSemDestruct - Destructs a STAF Mutex semaphore             */
/*                                                                     */
/* Accepts: (In)  Pointer to a mutex                                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFMutexSemDestruct(STAFMutexSem_t *pMutex, unsigned int *osRC);

/* End C language definitions */

#ifdef __cplusplus
}

// Begin C++ language definitions

#include "STAFRefPtr.h"
#include "STAFException.h"

// Forward declaration for typedef
class STAFMutexSem;
typedef STAFRefPtr<STAFMutexSem> STAFMutexSemPtr;

// STAFMutexSem - This class provides a C++ wrapper around the STAF Mutex
//                C APIs.  This class will throw exceptions in all error
//                cases except for a timeout on a request().

class STAFMutexSem
{
public:

    STAFMutexSem();

    // Returns: kSTAFOk, if the semaphore was acquired
    //          kSTAFTimeout, if you timed out waiting     
    STAFRC_t request(unsigned int timeout = STAF_MUTEX_SEM_INDEFINITE_WAIT);
    void release();

    ~STAFMutexSem();

private:

    // Don't allow copy or assignment
    STAFMutexSem(const STAFMutexSem &);
    STAFMutexSem &operator=(const STAFMutexSem &);

    STAFMutexSem_t fMutexImpl;
};


// STAFMutexSemLock - This class provides a simply way to acquire a STAFMutexSem
//                    for the duration of a block                            

class STAFMutexSemLock
{
public:

    STAFMutexSemLock(STAFMutexSem &theMutex,
                     unsigned int timeout = STAF_MUTEX_SEM_INDEFINITE_WAIT)
        : fMutex(theMutex)
    { fMutex.request(timeout); }

    ~STAFMutexSemLock()
    { fMutex.release(); }

private:

    STAFMutexSem &fMutex;
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFMutexSemInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif


