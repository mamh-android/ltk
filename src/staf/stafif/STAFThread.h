/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Thread
#define STAF_Thread

#include "STAFError.h"
#include "STAFOSTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int (*STAFThreadFunc_t)(void *);

/****************************************************************************/
/* STAFThreadStart - Starts a thread, passing in the specified data         */
/*                                                                          */
/* Accepts: (OUT) Pointer to thread identifier                              */
/*          (IN)  Pointer to function to start on the thread                */
/*          (IN)  The data to be passed to the thread                       */
/*          (IN)  Flags (currently, this should be set to 0)                */
/*          (OUT) Pointer operating system return code                      */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFThreadStart(STAFThreadID_t *threadID,
    STAFThreadFunc_t theFunc, void *theData, unsigned int flags,
    unsigned int *osRC);

/****************************************************************************/
/* STAFThreadCurrentThreadID - Returns the current thread ID                */
/*                                                                          */
/* Accepts: Nothing                                                         */
/*                                                                          */
/* Returns:  The current thread ID                                          */
/****************************************************************************/
STAFThreadID_t STAFThreadCurrentThreadID();


/****************************************************************************/
/* STAFThreadSleepCurrentThread - Puts the current thread to sleep for a    */
/*                                specified amount of time                  */
/*                                                                          */
/* Accepts: (IN)  Sleep time (in milliseconds)                              */
/*          (OUT) Pointer operating system return code                      */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFThreadSleepCurrentThread(unsigned int milliseconds,
                                      unsigned int *osRC);


/****************************************************************************/
/* STAFThreadSafeIncrement - Atomically increments a value                  */
/*                                                                          */
/* Accepts: Pointer to value to increment                                   */
/*                                                                          */
/* Returns: A negative value, if the new value is less than zero            */
/*          Zero, if the new value is zero                                  */
/*          A positive value, if the new value is greater than zero         */
/*                                                                          */
/* Note: This function does not guarantee that it well return the new value */
/*       of the variable                                                    */
/****************************************************************************/
STAFThreadSafeScalar_t STAFThreadSafeIncrement(STAFThreadSafeScalar_t *ptr);


/****************************************************************************/
/* STAFThreadSafeDecrement - Atomically decrements a value                  */
/*                                                                          */
/* Accepts: Pointer to value to decrement                                   */
/*                                                                          */
/* Returns: A negative value, if the new value is less than zero            */
/*          Zero, if the new value is zero                                  */
/*          A positive value, if the new value is greater than zero         */
/*                                                                          */
/* Note: This function does not guarantee that it well return the new value */
/*       of the variable                                                    */
/****************************************************************************/
STAFThreadSafeScalar_t STAFThreadSafeDecrement(STAFThreadSafeScalar_t *ptr);

#ifdef __cplusplus
}
#endif

#endif
