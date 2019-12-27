/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <cstdlib>
#include <pthread.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include "STAFOSTypes.h"
#include "STAFEventSem.h"

static int getNumMsgs(int msgqid);

struct STAFEventSemPrivateData
{
    enum State { kPosted = 0, kReset = 1 };

    pthread_mutex_t fMutex;
    pthread_cond_t fCond;
    State fState;
    unsigned int fPostCount;
};

struct STAFEventSemSharedData
{
    int fMsgQID;
};

struct STAFMsgPost
{
    long val;
    unsigned int data;
};

struct STAFEventSemImplementation
{
    enum Type { kPrivate = 0, kShared = 1 };

    Type fType;

    union
    {
        STAFEventSemPrivateData fPrivate;
        STAFEventSemSharedData fShared;
    };
};


STAFRC_t STAFEventSemConstruct(STAFEventSem_t *pEvent, 
                               const char *name,
                               unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (pEvent == 0) return kSTAFInvalidObject;

    try
    {
        *pEvent = new STAFEventSemImplementation;
        STAFEventSemImplementation &eventSem = **pEvent;

        if (name == 0)
        {
            eventSem.fType = STAFEventSemImplementation::kPrivate;
            eventSem.fPrivate.fState = STAFEventSemPrivateData::kReset;
            eventSem.fPrivate.fPostCount = 0;

            int rc2 = pthread_mutex_init(&eventSem.fPrivate.fMutex, 0);
            if (rc2 != 0)
            {
                if (osRC) *osRC = rc2;
                delete *pEvent;
                return kSTAFBaseOSError;
            }

            rc2 = pthread_cond_init(&eventSem.fPrivate.fCond, 0);

            if (rc2 != 0)
            {
                if (osRC) *osRC = rc2;
                pthread_mutex_destroy(&eventSem.fPrivate.fMutex);
                delete *pEvent;
                return kSTAFBaseOSError;
            }
        }
        else
        {
            // Shared event semaphore
            
            return kSTAFInvalidParm;

            // XXX: Shared event semaphores don't currently work and are not
            // being used.  So, commented out this code because was getting
            // the following error compiling on Solaris after adding
            // -D_FILE_OFFFSET_BITS=64 to makefile.solaris for large file
            // support:
            // *** Compiling STAFEventSem.o ***
            // In function `STAFRC_t STAFEventSemConstruct(...)'
            // implicit declaration of function `int open(...)'
            // make: *** [.../stafif/STAFEventSem.o] Error 1

            /*
            eventSem.fType = STAFEventSemImplementation::kShared;

            STAFString theName("/tmp/STAFEventSem_");
            theName += name;

            const char *theNameInCurrentCodePage = 
                theName.toCurrentCodePage()->buffer();

            int initEvent = 1;
            int fileID = open(theNameInCurrentCodePage, 
                              O_CREAT | O_EXCL | O_RDONLY);

            if ((fileID == -1) && (errno == EEXIST))
            {
                initEvent = 0;
                fileID = open(theNameInCurrentCodePage, O_CREAT | O_RDONLY);
            }
    
            if (fileID == -1)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }

            int closeRC = close(fileID);

            if (closeRC == -1)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }

            int chmodRC = chmod(theNameInCurrentCodePage, 
                                S_IRUSR | S_IWUSR | S_IRGRP |
                                S_IWGRP | S_IROTH | S_IWOTH);
            if (chmodRC == -1)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }

            key_t key = ftok(theNameInCurrentCodePage, 1);

            if (key == -1)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }

            eventSem.fShared.fMsgQID =  msgget(key, IPC_CREAT | S_IRUSR |
                                            S_IWUSR | S_IRGRP | S_IWGRP |
                                            S_IROTH | S_IWOTH);
            if (eventSem.fShared.fMsgQID == -1)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }
            */
        }
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFEventSemPost(STAFEventSem_t pEvent, unsigned int *osRC)
{
    STAFEventSemImplementation &eventSem = *pEvent;

    if (eventSem.fType == STAFEventSemImplementation::kPrivate)
    {
        unsigned int rc2 = 0;

        if ((rc2 = pthread_mutex_lock(&eventSem.fPrivate.fMutex)) != 0)
        {
            if (osRC) *osRC = rc2;
            return kSTAFBaseOSError;
        }

        // If the state is reset then broadcast all threads
        // waiting for the condition variable

        if (eventSem.fPrivate.fState == STAFEventSemPrivateData::kReset)
        {
            if ((rc2 = pthread_cond_broadcast(&eventSem.fPrivate.fCond)) != 0)
            {
                if (osRC) *osRC = rc2;
                pthread_mutex_unlock(&eventSem.fPrivate.fMutex);
                return kSTAFBaseOSError;
            }

            eventSem.fPrivate.fState = STAFEventSemPrivateData::kPosted;
            ++eventSem.fPrivate.fPostCount;

            // XXX: Should I check the return code here and report back to the
            //      caller?  Same thing in several places below.
        }

        pthread_mutex_unlock(&eventSem.fPrivate.fMutex);
    }
    else
    {
        STAFMsgPost postcmd = { 1, 1 };

        int postrc = msgsnd(eventSem.fShared.fMsgQID, &postcmd,
                            sizeof(unsigned int), IPC_NOWAIT);
        if (postrc == -1)
        {
            if (osRC) *osRC = errno;
            return kSTAFBaseOSError;
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFEventSemReset(STAFEventSem_t pEvent, unsigned int *osRC)
{
    STAFEventSemImplementation &eventSem = *pEvent;

    if (eventSem.fType == STAFEventSemImplementation::kPrivate)
    {
        unsigned int rc2 = 0;

        if ((rc2 = pthread_mutex_lock(&eventSem.fPrivate.fMutex)) != 0)
        {
            if (osRC) *osRC = rc2;
            return kSTAFBaseOSError;
        }

        eventSem.fPrivate.fState = STAFEventSemPrivateData::kReset;

        pthread_mutex_unlock(&eventSem.fPrivate.fMutex);
    }
    else
    {
        int numMsgs = getNumMsgs(eventSem.fShared.fMsgQID);

        if (numMsgs == -1)
        {
            if (osRC) *osRC = errno;
            return kSTAFBaseOSError;
        }

        STAFMsgPost data = { 0 };

        for (int i = 0; i < numMsgs; ++i)
        {
            // XXX: We simply ignore errors while emptying the queue.
            //      We might want to be smarter here.
            int rc = msgrcv(eventSem.fShared.fMsgQID, &data,
                            sizeof(unsigned int), 0,
                            MSG_NOERROR | IPC_NOWAIT);
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFEventSemWait(STAFEventSem_t pEvent, unsigned int timeout,
                          unsigned int *osRC)
{
    // Note that the timeout argument is the time to wait in milliseconds

    STAFRC_t rc = kSTAFOk;

    if (pEvent == 0) return kSTAFInvalidObject;

    STAFEventSemImplementation &eventSem = *pEvent;

    if (eventSem.fType == STAFEventSemImplementation::kPrivate)
    {
        struct timeval now; // Time when we started
        
        if (timeout != STAF_EVENT_SEM_INDEFINITE_WAIT)
        {
            // Get current time

            // Use gettimeofday() to get the actual time with a 0.01 second
            // resolution. A timeval struct is passed to it.  A timeval has
            // two components:  tv_sec is the time in seconds and tv_usec is
            // the number of microseconds. Although the units are microseconds,
            // the value may not be that accurate on some operating systems.

            if (gettimeofday(&now, NULL) != 0)
            {
                if (osRC) *osRC = errno;
                return kSTAFBaseOSError;
            }
        }

        unsigned int rc2 = 0;

        if ((rc2 = pthread_mutex_lock(&eventSem.fPrivate.fMutex)) != 0)
        {
            if (osRC) *osRC = rc2;
            return kSTAFBaseOSError;
        }

        if (eventSem.fPrivate.fState == STAFEventSemPrivateData::kPosted)
        {
            rc = kSTAFOk;
        }
        else
        {
            unsigned int savePostCount = eventSem.fPrivate.fPostCount;

            struct timespec absTime;
             
            if (timeout != STAF_EVENT_SEM_INDEFINITE_WAIT)
            {
                // Prepare the maximum time to block the thread, expressed as
                // an absolute time in a timespec structure, absTime
                // (consisting of seconds and nanoseconds).

                // To do this, add the specified timeout (in milliseconds) to
                // the starting time contained in the "now" timeval structure
                // (consisting of seconds and microseconds).
                // Notes:
                // 1) 1 microsecond == 1000 milliseconds
                // 2) 1 nanosecond == 1000 microseconds (1000000 microseconds)
                // 3) 1 second == 1000000000 nanoseconds
                // 4) A timespec's tv_nsec field (nanoseconds) must be less
                //    than 1000000000 (one billion) or else this will cause a
                //    pthread_cond_timedwait to return EINVAL (Invalid Value).

                absTime.tv_sec = now.tv_sec + (timeout / 1000) +
                    ((now.tv_usec + ((timeout % 1000) * 1000)) / 1000000);
                absTime.tv_nsec = 
                    ((now.tv_usec + ((timeout % 1000) * 1000)) % 1000000) * 1000;
            }

            int rc2 = EINTR;

            // For event semaphores that wait forever, to handle multiple
            // waiters on an event semaphore correctly, need to break out of
            // the loop if the post count for the semaphore changes so that
            // all of the waiters on the event sempahore get woken up.

            while ((rc2 == EINTR) ||
                   ((timeout == STAF_EVENT_SEM_INDEFINITE_WAIT) &&
                    (savePostCount == eventSem.fPrivate.fPostCount)))
            {
                if (timeout == STAF_EVENT_SEM_INDEFINITE_WAIT)
                {
                    rc2 = pthread_cond_wait(&eventSem.fPrivate.fCond,
                                            &eventSem.fPrivate.fMutex);
                }
                else
                {
                    rc2 = pthread_cond_timedwait(&eventSem.fPrivate.fCond,
                                                 &eventSem.fPrivate.fMutex,
                                                 &absTime);
                }
            }

            // XXX: Hack Hack!  z/OS returns EAGAIN instead of ETIMEDOUT
            //      This is not how this will ultimately be coded.  This will
            //      probably require the same fix as for the Solaris 9 bug
    
            if ((rc2 == ETIMEDOUT) || (rc2 == EAGAIN))
            {
                rc = kSTAFTimeout;
            }
            else if (rc2 != 0)
            {
                rc = kSTAFBaseOSError;
                if (osRC) *osRC = rc2;
            }
        }

        pthread_mutex_unlock(&eventSem.fPrivate.fMutex);
    }
    else
    {
        // XXX: We will need to do something different for shared Event sems
        //      It appears that only AIX allows this enhanced use of select() 
        //      for message queues.

        /* XXX: Shared event semaphores don't currently work and are not
        // being used.  So, commented out this code because it caused the
        // following error compiling on HPUX-IA64 after upgrading to the
        // latest aCC compiler version:
        //  warning #4232-D: conversion from "int *" to a more strictly
        //               aligned type "fd_set *" may cause misaligned access
        //  error #2167: argument of type "fd_set *" is incompatible with
        //               parameter of type "int *"

        int readmsg = eventSem.fShared.fMsgQID;
        timeval timeOut = { (timeout / 1000), (timeout % 1000) };
        int selRC = select((1 << 16), (fd_set *)&readmsg, 0 , 0,
                           timeout ? &timeOut : 0);
        if (selRC == -1)
        {
            if (osRC) *osRC = errno;
            return kSTAFBaseOSError;
        }

        rc = selRC ? kSTAFOk : kSTAFBaseOSError;
        */
    }

    return rc;
}


STAFRC_t STAFEventSemQuery(STAFEventSem_t pEvent, STAFEventSemState_t *pState,
                           unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (pEvent == 0)
        return kSTAFInvalidObject;
 
    if (pState == 0)
        return kSTAFInvalidParm;

    STAFEventSemImplementation &eventSem = *pEvent;

    if (eventSem.fType == STAFEventSemImplementation::kPrivate)
    {
        unsigned int rc2 = 0;

        if ((rc2 = pthread_mutex_lock(&eventSem.fPrivate.fMutex)) != 0)
        {
            if (osRC) *osRC = rc2;
            return kSTAFBaseOSError;
        }

        *pState = (eventSem.fPrivate.fState ==
              STAFEventSemPrivateData::kReset) ? kSTAFEventSemReset 
                                               : kSTAFEventSemPosted;

        pthread_mutex_unlock(&eventSem.fPrivate.fMutex);
    }
    else
    {
        int numMsgs = getNumMsgs(eventSem.fShared.fMsgQID);

        if (numMsgs == -1)
        {
           if (osRC) *osRC = 1;
           return kSTAFBaseOSError;
        }

        *pState = numMsgs ? kSTAFEventSemPosted : kSTAFEventSemReset;
    }

    return rc;
}


STAFRC_t STAFEventSemDestruct(STAFEventSem_t *pEvent, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (pEvent == 0) return kSTAFInvalidObject;

    STAFEventSemImplementation &eventSem = **pEvent;

    if (eventSem.fType == STAFEventSemImplementation::kPrivate)
    {
        unsigned int rc2 = 0;

        if ((rc2 = pthread_cond_destroy(&eventSem.fPrivate.fCond)) != 0)
        {
            rc = kSTAFBaseOSError;
            if (osRC) *osRC = rc2;
        }

        if ((rc2 = pthread_mutex_destroy(&eventSem.fPrivate.fMutex)) != 0)
        {
            rc = kSTAFBaseOSError;
            if (osRC) *osRC = rc2;
        }
    }
    else
    {
        // Note: We do not remove the message queue here, as other processes
        //       may be using it, and it is a shared resource
    }

    delete *pEvent;
    *pEvent = 0;

    return rc;
}


int getNumMsgs(int msgqid)
{
    msqid_ds msgData = { 0 };

    int rc = msgctl(msgqid, IPC_STAT, &msgData);

    return (rc == -1) ? rc : msgData.msg_qnum;
}
