/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_EventSem
#define STAF_EventSem

#include "STAF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

#define STAF_EVENT_SEM_INDEFINITE_WAIT (unsigned int)-1

typedef struct STAFEventSemImplementation *STAFEventSem_t;
typedef enum   STAFEventSemState_e {
    kSTAFEventSemReset  = 0, 
    kSTAFEventSemPosted = 1
} STAFEventSemState_t;

/***********************************************************************/
/* STAFEventSemConstruct - Creates a STAF Event sempahore.  The        */
/*                         semaphore will be created in the reset      */
/*                         state.                                      */
/*                                                                     */
/* Accepts: (Out) Pointer to a event                                   */
/*          (In)  The name of the event (This should be 0/NULL for a   */
/*                private event)                                       */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFEventSemConstruct(STAFEventSem_t *pEvent, 
                               const char *name, unsigned int *osRC);

/***********************************************************************/
/* STAFEventSemPost - Posts a STAF Event semaphore.  It is safe to     */
/*                    post a posted semaphore.                         */
/*                                                                     */
/* Accepts: (In)  Pointer to a event                                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFEventSemPost(STAFEventSem_t pEvent, unsigned int *osRC);


/***********************************************************************/
/* STAFEventSemReset - Resets a STAF Event semaphore.  It is safe to   */
/*                     reset a reset semaphore.                        */
/*                                                                     */
/* Accepts: (In)  Pointer to a event                                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFEventSemReset(STAFEventSem_t pEvent, unsigned int *osRC);


/***********************************************************************/
/* STAFEventSemWait - Waits for a STAF Event semaphore                 */
/*                                                                     */
/* Accepts: (In)  Pointer to a event                                   */
/*          (In)  The amount of time to wait (in milliseconds)         */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, if the event semaphore was posted                */
/*           kSTAFTimeout, on timeout                                  */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFEventSemWait(STAFEventSem_t pEvent, unsigned int timeout,
                          unsigned int *osRC);


/***********************************************************************/
/* STAFEventSemQuery - Queries the state of a STAF Event semaphore     */
/*                                                                     */
/* Accepts: (In)  Pointer to a event                                   */
/*          (Out) Pointer to event sem state                           */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, if the event semaphore was queried succesfully   */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFEventSemQuery(STAFEventSem_t pEvent, STAFEventSemState_t *pState,
                           unsigned int *osRC);


/***********************************************************************/
/* STAFEventSemDestruct - Destructs a STAF Event semaphore             */
/*                                                                     */
/* Accepts: (In)  Pointer to a event                                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFEventSemDestruct(STAFEventSem_t *pEvent, unsigned int *osRC);

/* End C language definitions */

#ifdef __cplusplus
}

// Begin C++ language definitions

#include "STAFRefPtr.h"
#include "STAFString.h"
#include "STAFException.h"

// Forward declaration for typedef
class STAFEventSem;
typedef STAFRefPtr<STAFEventSem> STAFEventSemPtr;

// STAFEventSem - This class provides a C++ wrapper around the STAF Event
//                C APIs.  This class will throw exceptions in all error
//                cases except for a timeout on a request().

class STAFEventSem
{
public:

    STAFEventSem(const char *name = 0);

    void post();
    void reset();

    // Waits the indicated amount in milliseconds.
    // Returns:  kSTAFOk, if the sem was posted
    //           kSTAFTimeout, if the call timed out
    STAFRC_t wait(unsigned int timeout = STAF_EVENT_SEM_INDEFINITE_WAIT);

    // Checks whether the semaphore is posted or reset
    // Returns:  kSTAFEventSemReset, if the semaphore is reset
    //           kSTAFEventSemPosted, if the semaphore is posted
    STAFEventSemState_t query();

    STAFEventSem_t getImpl() { return fEventImpl; }

    ~STAFEventSem();

private:

    // Don't allow copy or assignment
    STAFEventSem(const STAFEventSem &);
    STAFEventSem &operator=(const STAFEventSem &);

    STAFEventSem_t fEventImpl;
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFEventSemInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
