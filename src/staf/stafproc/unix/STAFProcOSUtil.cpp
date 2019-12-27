/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "STAFProc.h"
#include "STAFProcOSUtil.h"
#include "STAFUtil.h"
#include "STAFTrace.h"

unsigned int makePerformanceAdjustments(STAFPerfInfo perfInfo,
                                        STAFString& errorBuffer,
                                        unsigned int& osRC)
{
    osRC = 0;
    errorBuffer = "";
    return 0;
}

void generic_signal_handler(int signum)
{
    STAFString error = STAFString("Received signal ") + STAFString(signum);

    switch (signum)
    {
        case SIGQUIT:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGQUIT)");

            (*gShutdownSemaphorePtr)->post();
            break;
        }

        case SIGINT:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGINT)");

            (*gShutdownSemaphorePtr)->post();
            break;
        }

        case SIGTERM:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGTERM)");
            (*gShutdownSemaphorePtr)->post();
            break;
        }

        case SIGFPE:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGFPE)");
            abort();
            break;
        }

        case SIGABRT:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGABRT)");
            break;
        }

        case SIGILL:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGILL)");
            abort();
            break;
        }

        case SIGSEGV:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (SIGSEGV)");
            abort();
            break;
        }

        default:
        {
            STAFTrace::trace(kSTAFTraceError, error + " (Unknown)");
        }
    }
}

static int staflckdes = -1;
static int datadirlckdes = -1;

unsigned int STAFProcOSInit(STAFString &errorBuffer, unsigned int &osRC)
{
    // Set up our signal handling
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTTOU, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set SIGTTOU to SIG_IGN";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGPIPE, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set SIGPIPE to SIG_IGN";
        osRC = errno;
        return 1;
    }

    sa.sa_handler = generic_signal_handler;

    if (sigaction(SIGFPE, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGFPE";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGILL, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGILL";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGSEGV, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGSEGV";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGABRT, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGABRT";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGINT, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGINT";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGQUIT, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGQUIT";
        osRC = errno;
        return 1;
    }

    if (sigaction(SIGTERM, &sa, 0) < 0)
    {
        errorBuffer = "Unable to set signal handler for SIGTERM";
        osRC = errno;
        return 1;
    }

    // create a temporary file to serve as indicator that STAFProc 
    // is already running.

    umask(0);

    // Verify that the STAFInstanceNamePtr value is valid by making sure
    // it only contains characters that are valid for a file name and is
    // not blank.

    if ((*gSTAFInstanceNamePtr).find(kUTF8_SLASH) != STAFString::kNPos)
    {
        osRC = 0;

        errorBuffer = STAFString(
            "Invalid value set for the STAF_INSTANCE_NAME environment "
            "variable.  It cannot contain a /.  STAF_INSTANCE_NAME=") +
            *gSTAFInstanceNamePtr;

        return 1;
    }
    else if ((*gSTAFInstanceNamePtr).length() == 0)
    {
        osRC = 0;

        errorBuffer = STAFString(
            "Invalid value set for the STAF_INSTANCE_NAME environment "
            "variable.  It cannot be blank.");

        return 1;
    }

    // Determine the directory for storing files that STAF uses while
    // STAFProc is running to ensure that each instance of STAF is using
    // a unique STAF_INSTANCE_NAME and a unique DATADIR, etc

    *gSTAFTempDirPtr = "/tmp";

    if (getenv("STAF_TEMP_DIR") != NULL)
    {
        *gSTAFTempDirPtr = getenv("STAF_TEMP_DIR");

        if ((*gSTAFTempDirPtr).length() != 0)
        {
            // Check if the STAF_TEMP_DIR directory exists and if not,
            // create it

            struct stat st;
        
            int rc = stat(
                (*gSTAFTempDirPtr).toCurrentCodePage()->buffer(), &st);

            if ((rc != 0) && (errno != EOVERFLOW))
            {
                if ((mkdir((*gSTAFTempDirPtr).toCurrentCodePage()->buffer(),
                           01766)) == -1)
                {
                    errorBuffer = STAFString(
                        "Error creating the directory specified by the "
                        "STAF_TEMP_DIR environment variable: ") +
                        *gSTAFTempDirPtr;
                    return 1;
                }
            }
        }
        else
        {
            errorBuffer = STAFString(
                "Invalid value set for the STAF_TEMP_DIR environment "
                "variable.  It cannot be blank.");
            return 1;
        }
    }

    STAFString staflckInstance = *gSTAFTempDirPtr + "/" +
        *gSTAFInstanceNamePtr + ".tmp" + STAFString(kUTF8_NULL);

    staflckdes = open(staflckInstance.toCurrentCodePage()->buffer(),
                      O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    if (staflckdes == -1)
    {
        // Opening the temporary file failed

        osRC = errno;

        errorBuffer = STAFString("Error opening/creating temporary file ") +
            staflckInstance + " with read and write permissions";

        if (osRC == EACCES)
        {
            errorBuffer += ".  Make sure the user starting STAFProc has "
                "write permissions to the " + *gSTAFTempDirPtr +
                " directory and to this file, if it already exists; or "
                "perhaps a sharing violation occurred because STAFProc is "
                "already running using STAF_INSTANCE_NAME=" +
                *gSTAFInstanceNamePtr;
        }

        return 1;
    }

    // Lock the temporary file that we just opened

    struct flock theLock = { 0 };
    theLock.l_type = F_WRLCK;

    if (fcntl(staflckdes, F_SETLK, &theLock) == -1)
    {
        osRC = errno;
        errorBuffer = "STAFProc is already running using STAF_INSTANCE_NAME=" +
            (*gSTAFInstanceNamePtr);
        return 1;
    }

    return 0;
}


unsigned int STAFProcOSLockDataDir(STAFString &errorBuffer, unsigned int &osRC)
{
    STAFString datadirlckInstance = *gSTAFTempDirPtr + "/DataDir_" +
        (*gSTAFWriteLocationPtr).replace(kUTF8_SLASH, kUTF8_HYPHEN) + 
        ".tmp" + STAFString(kUTF8_NULL);

    datadirlckdes = open(datadirlckInstance.toCurrentCodePage()->buffer(),
                         O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    struct flock theLock = { 0 };
    theLock.l_type = F_WRLCK;

    if (fcntl(datadirlckdes, F_SETLK, &theLock) == -1)
    {
        datadirlckdes = -1;
        osRC = errno;
        errorBuffer = "Error obtaining a lock on DATADIR " +
            *gSTAFWriteLocationPtr + " because file " + datadirlckInstance +
            " appears to already be in use by another instance of STAFProc";
        return 1;
    }

    // Make sure that the directory where the datadirlockInstnce and
    // staflckInstance files are stored is not the same as the {DATADIR}/tmp
    // directory since STAFProc will be deleting this tmp directory next
    // in the startup process

    if (*gSTAFTempDirPtr == (*gSTAFWriteLocationPtr + "/tmp"))
    {
        errorBuffer = STAFString(
            "ERROR: You cannot set the STAF_TEMP_DIR environment variable "
            "to ") + *gSTAFWriteLocationPtr + "/tmp because this DATADIR tmp "
            "directory is about to be deleted and this would delete the "
            "files that STAF has already created to ensure each instance of "
            "of STAFProc has a unique STAF_INSTANCE_NAME and a unique "
            "DATADIR";
        return 1;
    }

    return 0;
}


unsigned int STAFProcOSTerm(STAFString &errorBuffer, unsigned int &osRC)
{
    osRC = 0;
    errorBuffer = "";

    // here we unlock the file and on success, flock returns 0 and unlocks
    // the file, otherwise, it fails with -1 (but this should not happen)

    struct flock theLock = { 0 };
    theLock.l_type = F_UNLCK;

    STAFString staflckInstance = *gSTAFTempDirPtr + "/" +
        *gSTAFInstanceNamePtr + ".tmp" + STAFString(kUTF8_NULL);

    if (fcntl(staflckdes, F_SETLK, &theLock) == -1)
    {
        if (errorBuffer.length() != 0)
        {
            osRC = errno;
            errorBuffer = "Unable to unlock " + staflckInstance;
        }
    }
    else if (unlink(staflckInstance.toCurrentCodePage()->buffer()) == -1)
    {
        if (errorBuffer.length() != 0)
        {
            osRC = errno;
            errorBuffer = "Could not remove " + staflckInstance;
        }
    }

    // Unlock the Data directory lock file (if it has been locked)

    if (datadirlckdes != -1)
    {
        STAFString datadirlckInstance = *gSTAFTempDirPtr + "/DataDir_" +
            (*gSTAFWriteLocationPtr).replace(kUTF8_SLASH, kUTF8_HYPHEN) +
            ".tmp" + STAFString(kUTF8_NULL);

        if (fcntl(datadirlckdes, F_SETLK, &theLock) == -1)
        {
            if (errorBuffer.length() != 0)
            {
                osRC = errno;
                errorBuffer = "Unable to unlock " + datadirlckInstance; 
            }
        }
        else if(unlink(datadirlckInstance.toCurrentCodePage()->buffer()) == -1)
        {
            if (errorBuffer.length() != 0)
            {
                osRC = errno;
                errorBuffer = "Could not remove " + datadirlckInstance;
            }
        }
    }

    if (errorBuffer.length() != 0) return 1;
    
    return 0;
}

// The following is all code written by Dan Lepore to handle signals for
// AIX.  This code will probably be reincorporated once I figure out exactly
// how/which signals should be handled.  Note, there are definite platform
// differences as to which signals exist.

#if 0

#include <signal.h>
#include <sys/context.h>
#include <sys/mstsave.h>
#include <assert.h>

void HandleSignal(unsigned long);
void staf_errhandler(int, int, struct sigcontext *);

STAFEventSem gSigThread;

stafproc_main()
{
    gSigThread.reset(); // reset the signal thread wait semaphore

    gThreadManagerPtr->dispatch(new STAFThreadFunc(HandleSignal,0));

    gSigThread.wait();
}

stafProcTerminate()
{
    int pid;
    int rc;

    if ((pid = fork ()) != 0)
    {
        // parent process
        // if fork failed, parent returns error, otherwise returns ok
        if (pid == -1)
        {
            system("./cleanstaf.ksh 2>/dev/null");
            exit(1);
        }
    }
    else
    {
        rc = execlp("./cleanstaf",NULL);
        printf("rc = %d\n",rc);
        exit(1);
    }

    exit(1);
}

void _System HandleSignal(unsigned long nada)
{
    struct sigaction action;
    sigset_t set;
    sigset_t setasynch;
    long rc;
    int sig;
    int pid;

    // set up signal handling for exception conditions.

    action.sa_handler = (void(*)(int))staf_errhandler;
    sigemptyset(&set);      // empty set blocked in handler 
    action.sa_mask = set;   // update sigaction mask 
    action.sa_flags = 0x0;  // zero all flags 

    rc = sigaction(SIGILL,&action,NULL);  // illegal op 
    assert (rc == 0);

    rc = sigaction(SIGSEGV,&action,NULL); // addressing 
    assert(rc == 0);

    // Now, set up signal handling for asynch events.

    sigemptyset(&setasynch);  /* null set blocked */

    sigaddset(&setasynch, SIGINT);    /* handle interrupt */
    sigaddset(&setasynch, SIGTERM);   /* handle termination */

    sigthreadmask(SIG_BLOCK, &set, NULL); /* block for signal */

    // Post Main thread.

    gSigThread.post(); 

    // Issue sigwait to block waiting for termination.

    rc = sigwait(&setasynch, &sig);

    gShutdownSemaphore->post();

    if ((pid = fork ()) != 0)
    {
        // parent process 
        // if fork failed, parent returns error, otherwise returns ok
        if (pid == -1)
        {
            system("./cleanstaf.ksh 2>/dev/null");
            exit(1);
        }
    }
    else
    {
        rc = execlp("./cleanstaf",NULL);
        printf("rc = %d\n",rc);
        exit(1);
    }

    exit(1);
}

void staf_errhandler(int signal, int code, struct sigcontext *scp)
{
    int rc = 0;
    int pid = 0;

    STAFTrace::trace(
        kSTAFTraceError,
        "STAFProc: Signal Received - exception detected. Exception Type: " +
        STAFString(scp->sc_jmpbuf.jmp_context.excp_type));

    gShutdownSemaphore->post();

    if ((pid = fork ()) != 0)
    {
        // parent process
        // if fork failed, parent returns error, otherwise returns ok
        if (pid == -1)
        {
            system("./cleanstaf.ksh 2>/dev/null");
            exit(1);
        }
    }
    else
    {
        rc = execlp("./cleanstaf",NULL);
        printf("rc = %d\n",rc);
        exit(1);
    }

    abort();
}

#endif
