/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include <sys/types.h>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <deque>
#include <map>
#include <list>
#include "STAFProcess.h"
#include "STAFMutexSem.h"
#include "STAFEventSem.h"
#include "STAFUtil.h"
#include "STAFUtilUnix.h"
#include "STAFThreadManager.h"
#include "STAFTrace.h"
#include "STAFInternalProcess.h"

#ifdef STAF_OS_NAME_ZOS
#include <spawn.h>

extern char **environ;
#endif

extern "C"
{
// XXX: What's up with these #define's?
#ifndef   _XOPEN_SOURCE
#define   _XOPEN_SOURCE
#endif // _XOPEN_SOURCE

#include <pwd.h>
}

#define MONITOR_SLEEP_SECONDS 1

// Static data used to start process
struct ProcessCreateInfo
{
    ProcessCreateInfo(STAFProcessCommandType_t theCommandType =
                          kSTAFProcessCommand,
                      const STAFStringBufferPtr &cmd  = 
                          STAFStringBufferPtr(), 
                      char **arg = 0, char **env = 0,
                      STAFUserID_t  UID = getuid(),
                      STAFGroupID_t GID = getgid(),
                      const STAFStringBufferPtr &username  = 
                          STAFStringBufferPtr(),
                      const STAFStringBufferPtr &dir  = 
                          STAFStringBufferPtr(),
                      STAFProcessRedirectedIOMode_t theStdinMode =
                          kSTAFProcessIONoRedirect,
                      int stdinFD = dup(0),
                      STAFProcessRedirectedIOMode_t theStdoutMode =
                          kSTAFProcessIONoRedirect,
                      int stdoutFD = dup(1),
                      STAFProcessRedirectedIOMode_t theStderrMode =
                          kSTAFProcessIONoRedirect,
                      int stderrFD = dup(2),
                      STAFProcessEndCallbackLevel1 cback =
                          STAFProcessEndCallbackLevel1())
            : commandType(theCommandType), command(cmd), argv(arg), envp(env),
              workdir(dir), uid(UID), gid(GID), userName(username),
              stdinMode(theStdinMode), child_stdin(stdinFD),
              stdoutMode(theStdoutMode), child_stdout(stdoutFD),
              stderrMode(theStderrMode), child_stderr(stderrFD),
              callback(cback)
    {  /* Do nothing */ }
        
    int                           pid;  
    STAFUserID_t                  uid;
    STAFGroupID_t                 gid;
    STAFStringBufferPtr           userName;
    char                        **argv;
    char                        **envp;
    STAFProcessCommandType_t      commandType;
    STAFStringBufferPtr           command;
    STAFStringBufferPtr           workdir;
    STAFProcessRedirectedIOMode_t stdinMode;
    int                           child_stdin;
    STAFProcessRedirectedIOMode_t stdoutMode;
    int                           child_stdout;
    STAFProcessRedirectedIOMode_t stderrMode;
    int                           child_stderr;
    STAFProcessEndCallbackLevel1  callback;
};

// Data passed to the callback thread
struct ProcessMonitorCallbackInfo
{
    ProcessMonitorCallbackInfo(STAFProcessEndCallbackLevel1 theCallback,
                               STAFProcessID_t thePID,
                               STAFProcessHandle_t theHandle,
                               unsigned int theRC)
        : callback(theCallback), pid(thePID), handle(theHandle), rc(theRC)
    { /* Do Nothing */ }

    STAFProcessEndCallbackLevel1 callback;
    STAFProcessID_t pid;
    STAFProcessHandle_t handle;
    unsigned int rc;
};

struct ProcessMonitorInfo
{
    ProcessMonitorInfo (STAFProcessHandle_t aHandle = 0,
                        STAFProcessID_t     aPID    = 0,
                        STAFProcessEndCallbackLevel1 aCallback =
                            STAFProcessEndCallbackLevel1())
        : handle(aHandle), pid(aPID), callback(aCallback)
    {
        /* Do nothing */
    }

    STAFProcessHandle_t handle;
    STAFProcessID_t pid;
    STAFProcessEndCallbackLevel1 callback;
};

typedef std::deque<ProcessMonitorInfo> ProcessMonitorList;
typedef std::map<STAFProcessID_t, ProcessMonitorList> ProcessMonitorMap;

// Perform OS Specific User Authentication
static unsigned int UserAuthenticate(STAFUserID_t &uid, STAFGroupID_t &gid,
                                     const STAFString &username,
                                     const STAFString &password,
                                     bool mustValidate, unsigned int *osRC);

static unsigned int ProcessMonitorCallbackThread(void *data);
static unsigned int ProcessMonitorThread(void *);
static int ParseCommandParms(STAFString &, char ***);

static ProcessCreateInfo sProcessCreateInfo;
static STAFMutexSem sMonitorDataSem;
static ProcessMonitorMap sMonitorMap;  
static STAFUserID_t  sOurUID;
static STAFGroupID_t sOurGID;
static STAFMutexSem sCreateProcessSem;
static STAFEventSem sProcessCreated;
static STAFEventSem sProcessThread;

// private prototypes for user authentication functions
static unsigned int sAuthNone(const char *, const char *);
static unsigned int sAuthPam(const char *, const char *);

// private function pointer must be initialized to whatever fAuthMode is
static unsigned int (*sAuthenticateFuncPtr)(const char *username,
                                            const char *password) = sAuthNone;

// Default stdout/stderr redirection file if new console
static STAFString sDefaultRedirectFile = STAFString("/dev/null");

// Substitution characters valid for a shell command on Unix
char *gSTAFProcessShellSubstitutionChars = "cCpPtTuUwWxX%";


static STAFThreadManager &getProcessThreadManager()
{
  static STAFThreadManager theThreadManager(1);
  return theThreadManager;
}


// XXX: This function needs to be called in a number of different places.
//      Check in windows version also.

static void InitProcessManager()
{
    static STAFMutexSem initSem;
    static bool alreadyInited = false;

    if (alreadyInited) return;

    STAFMutexSemLock initLock(initSem);

    if (alreadyInited) return;

    sOurUID = getuid(); 
    sOurGID = getgid();

    unsigned int rc = getProcessThreadManager().dispatch(
        ProcessMonitorThread, 0);

    if (rc != 0)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            STAFString("STAFProcess::InitProcessManager: "
                       "Error dispatching a thread, RC: ") + rc);

        return;
    }
    
    alreadyInited = true;
}


unsigned int ProcessMonitorCallbackThread(void *data)
{
    ProcessMonitorCallbackInfo *pInfo =
        static_cast<ProcessMonitorCallbackInfo *>(data);

    pInfo->callback.callback(pInfo->pid, pInfo->handle, pInfo->rc,
                             pInfo->callback.data);

    delete pInfo;

    return 0;
}


unsigned int ProcessMonitorThread(void *)
{
    // Monitor processes in a forever loop until STAF shuts down.  This thread
    // will clean both processes registered outside of STAF (outsiders) and
    // processes registered from inside of STAF (insiders).

    // Due to the linux threading model, we have changed this code so that
    // processes get started from the thread that monitors which processes have
    // terminated.  This is b/c of a bug on waitpid() which does not work if a
    // process gets started from another thread, which was the case before.

    std::list<STAFProcessID_t> fgPIDList;
    int theTTY = open("/dev/tty", O_RDONLY);

    if (theTTY < 0)
    {
        STAFString errorMessage("STAFProcess::processMonitorThread: "
                                "Error opening /dev/tty, errno: ");
        errorMessage += errno;
        STAFTrace::trace(kSTAFTraceError, errorMessage);
    }

    while (1)
    {
        int foundProcess = 0;

        try
        {
            // NOTE: waitpid() returns -1 if no more child processes exist,
            //       returns 0 if no child processes have ended, or returns
            //       pid of process that has sent a stop or terminate signal.
            //       Outsider processes are NOT considered child processes.
     
            ProcessMonitorList processMonitorList;
            int retCode = 0;
            int childId = 0;
            
            {
                // Acquire lock

                STAFMutexSemLock lock(sMonitorDataSem);

                // Note: the waitpid function with WNOHANG does NOT block, so
                // the lock will NOT be held for long periods of time.

                // waitpid() is protected to avoid the following situation:
                // assume no insiders exist (ie. no children) and 0 or more
                // outsiders exist. then waitpid would return -1 (childId = -1)
                // and set retCode to 0.  This thread gets preempted by
                // startProcess2() and an internal process is added to the front
                // of the list terminating quickly after being added.  The
                // thread then gets control back.  The first condition fails
                // since childId < 0, and the second condition succeeds since
                // childId < 0 and the process has terminated.  This makes the
                // return code of the process to be a fake 0 rather than its
                // real return code.  Locking before calling waitpid() solves
                // this problem. 

                childId = waitpid(-1, &retCode, WNOHANG | WUNTRACED);

                // if the childId > 0 then we look for it in the list, else
                // if the childId <= 0, we look for a victim outsider process
                // that has terminated.
                // Note:  For outsider processes, we have to check that kill()
                //         didn't set errno to EPERM because that means it does
                //         not have permission to send the signal to the
                //         process pid (e.g. could have been started by another
                //         user), not that the process is terminated.

                ProcessMonitorMap::iterator iter = sMonitorMap.begin();

                while (iter != sMonitorMap.end())
                {
                    if (((childId > 0) && (iter->first == childId) &&
                        (WIFEXITED(retCode) || WIFSIGNALED(retCode))) ||
                        ((childId <= 0) && (kill(iter->first, 0) == -1) &&
                         (errno != EPERM)))
                    {
                        // If the child is no longer running, then remove it
                        // from the list of foreground waiters

                        if (childId > 0) fgPIDList.remove(childId);

                        // If the child was in the foreground then put STAF
                        // back in the foreground.  Farther below, we check for
                        // other children who want the foreground.

                        if ((childId > 0) && (tcgetpgrp(theTTY) == childId) &&
                            (tcsetpgrp(theTTY, getpgrp()) < 0))
                        {
                            STAFString errorMessage("STAFProcess::"
                                                    "processMonitorThread: "
                                                    "Error on tcsetpgrp(), "
                                                    "errno: ");
                            errorMessage += errno;
                            STAFTrace::trace(kSTAFTraceError, errorMessage);
                        }
 
                        processMonitorList = iter->second;
                        sMonitorMap.erase(iter->first);
                        foundProcess = 1;
                        break;
                    }

                    iter++;
                }

            }   // release lock  

            if (foundProcess)
            {
                // Note: The callback needs to be executed by a separate
                // thread to avoid deadlock with STAFProcessService when
                // started process finishes before it's added to the list
                // and attempts to call it's callback function.

                for (ProcessMonitorList::iterator monitorIter =
                         processMonitorList.begin();
                     (monitorIter != processMonitorList.end()) &&
                         (monitorIter->callback.callback != 0);
                     ++monitorIter)
                {
                    unsigned int dispatchRC = getProcessThreadManager().dispatch(
                        ProcessMonitorCallbackThread,
                        new ProcessMonitorCallbackInfo(monitorIter->callback,
                        monitorIter->pid, monitorIter->handle,
                        WIFEXITED(retCode) ? WEXITSTATUS(retCode) :
                        WTERMSIG(retCode)));

                    if (dispatchRC != 0)
                    {
                        STAFTrace::trace(
                            kSTAFTraceError,
                            STAFString("STAFProcess::processMonitorThread: "
                                       "Error dispatching a thread, RC: ") +
                            dispatchRC);
                    }

                    // Wait for 1 millisecond to prevent a flood of callback
                    // requests being submitted all at once, possibly
                    // resulting in new threads being created to run the
                    // callback requests

                    STAFThreadSleepCurrentThread(1, 0);
                }
            }
            else if (!foundProcess && (childId > 0))
            {
                if (WIFSTOPPED(retCode))
                {
                    fgPIDList.push_back(childId);
                }
                else
                {
                    STAFTrace::trace(kSTAFTraceError, "STAFProcess::"
                                     "processMonitorThread: Signaled process "
                                     "(PID: " + STAFString(childId) +
                                     ") not in process monitor's thread list");
                }
            }

            // If STAF is in the foreground and the list of children wanting
            // the foreground is not empty, then put the first one in the list
            // in the foreground

            if (!fgPIDList.empty() && (getpgrp() == tcgetpgrp(theTTY)))
            {
                STAFProcessID_t nextFGPID = fgPIDList.front();

                fgPIDList.pop_front();

                if (tcsetpgrp(theTTY, nextFGPID) < 0)
                {
                    STAFString errorMessage("STAFProcess::"
                                            "processMonitorThread: Error on "
                                            "tcsetpgrp(), errno: ");
                    errorMessage += errno;
                    STAFTrace::trace(kSTAFTraceError, errorMessage);
                }

                if (kill(nextFGPID, SIGCONT) < 0)
                {
                    STAFString errorMessage("STAFProcess::"
                                            "processMonitorThread: "
                                            "Error continuing PID: ");
                    errorMessage += nextFGPID;
                    errorMessage += ", errno: ";
                    errorMessage += errno;
                    STAFTrace::trace(kSTAFTraceError, errorMessage);
                }
            }

            // If no process was found, lets go to sleep for some time,
            // unless we get posted meaning we need to create a process

            if (!foundProcess)
            {
                // if posted, create the process indicated in fProcessCreateInfo
                if (!sProcessThread.wait(MONITOR_SLEEP_SECONDS * 1000))
                {
#ifndef STAF_OS_NAME_ZOS
                    // create the new process
                    if ((sProcessCreateInfo.pid = STAF_FORK()) == 0)
                    {
                        // child process

                        // setgid must be done first, as if we change the uid
                        // first then we are not able to change the gid since
                        // we may no longer have enough privilege to do that.

                        if (setgid(sProcessCreateInfo.gid) < 0)
                        {
                            STAFTrace::trace(kSTAFTraceError,
                                STAFString("STAFProcess::"
                                           "processMonitorThread: Child could "
                                           "not set child's gid: ") + errno);
                            _exit(1);
                        }

                        if (setuid(sProcessCreateInfo.uid) < 0)
                        {
                            STAFTrace::trace(kSTAFTraceError,
                                STAFString("STAFProcess::"
                                           "processMonitorThread: Child could "
                                           "not set child's uid: ") + errno);
                            _exit(1);
                        }

                        if (setpgid(0, 0) < 0)
                        {
                            STAFTrace::trace(kSTAFTraceInfo,
                                STAFString("STAFProcess::"
                                           "processMonitorThread: Child could "
                                           "not set child's pgid: ") + errno);
                            _exit(1);
                        }

                        close(0);
                        dup(sProcessCreateInfo.child_stdin);
                        close(sProcessCreateInfo.child_stdin);

                        close(1);
                        dup(sProcessCreateInfo.child_stdout);
                        close(sProcessCreateInfo.child_stdout);

                        close(2);
                        dup(sProcessCreateInfo.child_stderr);
                        close(sProcessCreateInfo.child_stderr);

                        // change working directory if appropriate
                        if (sProcessCreateInfo.workdir->length() != 0)
                        {
                            chdir(sProcessCreateInfo.workdir->buffer());
                        }
                        
                        if (sProcessCreateInfo.envp == 0)
                        {
                            // overwrite memory with new process image
                            if (execv((sProcessCreateInfo.commandType ==
                                       kSTAFProcessCommand) ?
                                          sProcessCreateInfo.command->buffer() :
                                          "/bin/sh",
                                      sProcessCreateInfo.argv))
                            {
                                STAFTrace::trace(kSTAFTraceError, 
                                                 STAFString("STAFProcess::"
                                                 "processMonitorThread: Could "
                                                 "not start process (execve):") +
                                                 errno);
                                _exit(1);  // this is the child process
                            }
                        }
                        else
                        {
                            // overwrite memory with new process image
                            if (execve((sProcessCreateInfo.commandType ==
                                        kSTAFProcessCommand) ?
                                           sProcessCreateInfo.command->buffer() :
                                           "/bin/sh",
                                       sProcessCreateInfo.argv,
                                       sProcessCreateInfo.envp))
                            {
                                STAFTrace::trace(kSTAFTraceError, 
                                                 STAFString("STAFProcess::"
                                                 "processMonitorThread: Could "
                                                 "not start process (execve):") +
                                                 errno);
                                _exit(1);  // this is the child process
                            }
                        }
                    }

                    // parent process

                    if (setpgid(sProcessCreateInfo.pid,
                                sProcessCreateInfo.pid) < 0)
                    {
                        STAFTrace::trace(kSTAFTraceInfo,
                            STAFString("STAFProcess::"
                                       "processMonitorThread: Parent could not "
                                       "set child's pgid: ") + errno);
                    }

#else // STAF_OS_NAME_ZOS

                    struct __inheritance inherit;

                    // Set inheritance flags to create a new process group
                    inherit.flags = SPAWN_SETGROUP;
                    inherit.pgroup = SPAWN_NEWPGROUP;

                    int fd_count = 3;
                    int fd_map[3] = { sProcessCreateInfo.child_stdin,
                                      sProcessCreateInfo.child_stdout,
                                      sProcessCreateInfo.child_stderr };

                    //Set working directory
                    if (sProcessCreateInfo.workdir->length() != 0) {
                        inherit.flags |= SPAWN_SETCWD;
                        inherit.cwdptr = const_cast<char *>(
                                         sProcessCreateInfo.workdir->buffer());
                        inherit.cwdlen = sProcessCreateInfo.workdir->length();
                    }

                    //Set userid
                    if (sProcessCreateInfo.uid != getuid())
                    {
                        inherit.flags |= SPAWN_SETUSERID;

                        // Note: The length of the inherit.userid buffer is
                        //       eight.  Thus, we don't want to overwrite
                        //       anything after it.

                        strncpy(inherit.userid,
                                sProcessCreateInfo.userName->buffer(), 8);
                        inherit.userid[8] = 0;
                    }

                    // create the new process
                    sProcessCreateInfo.pid = __spawn2(
                        (sProcessCreateInfo.commandType == kSTAFProcessCommand)
                            ? sProcessCreateInfo.command->buffer()
                            : "/bin/sh",
                        fd_count, fd_map, &inherit,
                        (const char**) sProcessCreateInfo.argv,
                        (const char**) ((sProcessCreateInfo.envp == 0) ?
                                        environ : sProcessCreateInfo.envp));

                    // parent process
                    if (sProcessCreateInfo.pid == -1)
                    {
                        STAFTrace::trace(kSTAFTraceError, 
                                         STAFString("STAFProcess::"
                                         "processMonitorThread(): Could "
                                         "not start process: ") + errno);
                    }
#endif
                    //Close file descriptors in the parent
                    close(sProcessCreateInfo.child_stdin);
                    close(sProcessCreateInfo.child_stdout);
                    close(sProcessCreateInfo.child_stderr);

                    {   // acquire lock
                        STAFMutexSemLock lock(sMonitorDataSem);
                
                        // add new process to list
                        sMonitorMap[sProcessCreateInfo.pid].push_back(
                            ProcessMonitorInfo(sProcessCreateInfo.pid,
                                               sProcessCreateInfo.pid, 
                                               sProcessCreateInfo.callback));
                    }   // release lock

                    // reset this thread's sem and post the process manager
                    // so that it can deallocate any used resources

                    sProcessThread.reset();
                    sProcessCreated.post();

                }  // if process thread was posted during wait
            }  // if not process found by waitpid
        } // try block
        catch (STAFException &se)
        {
            se.trace("STAFProcess::ProcessMonitorThread()");
        }
        catch (...)
        {
            STAFTrace::trace(
                kSTAFTraceError,
                "Caught unknown exception in "
                "STAFProcess::ProcessMonitorThread()");
        }
    }  // endless loop

    return kSTAFOk;
}

STAFRC_t STAFProcessStart(STAFProcessID_t *pid, STAFProcessHandle_t *procHandle,
                          void *data, unsigned int startDataLevel,
                          unsigned int *osRC)
{
    STAFString_t errorBuffer = 0;

    return STAFProcessStart2(pid, procHandle, data, startDataLevel,
                             osRC, &errorBuffer);
}

STAFRC_t STAFProcessStart2(STAFProcessID_t *pid, STAFProcessHandle_t *procHandle,
                           void *data, unsigned int startDataLevel,
                           unsigned int *osRC, STAFString_t *errorBuffer)
{
    if (data == 0)
    {
        if (errorBuffer)
        {
            STAFString errMsg = STAFString(
                "Invalid data provided to STAFProcessStart2()");
            *errorBuffer = errMsg.adoptImpl();
        }

        return kSTAFInvalidValue;
    }

    if (startDataLevel != 1)
    {
        if (errorBuffer)
        {
            STAFString errMsg = STAFString(
                "Invalid level provided to STAFProcessStart2(): ") +
                startDataLevel;
            *errorBuffer = errMsg.adoptImpl();
        }

        return kSTAFInvalidValue;
    }

    unsigned int systemErrorCode = 0;

    STAFProcessStartInfoLevel1 *startData =
        reinterpret_cast<STAFProcessStartInfoLevel1 *>(data);

    InitProcessManager();

    STAFString command(startData->command);

    struct stat fileinfo;

    if (startData->commandType == kSTAFProcessShell)
        command = command.subWord(0, 1);

    // Since a shell command could be "date; grep abc abc", cannot verify
    // that the first word (e.g. date;) is a valid command so only do this
    // check that the command is valid if not a shell command.
    if (startData->commandType != kSTAFProcessShell)
    {
        // Note: this checking is due to stat which requires a path'd file
        if (command.find(kUTF8_SLASH) == STAFString::kNPos)
        {
            STAFString_t path;

            // If no slashes found, resolve the command to form its path'd name
            // Note: getFilePath will always return a path that does not term-
            // inate with "/"

            STAFRC_t rc = STAFUtilUnixGetFilePath(command.getImpl(), &path,
                                                  osRC);
            if (rc != kSTAFOk)
            {
                if (rc == kSTAFDoesNotExist)
                {
                    if (osRC) *osRC = ENOENT;

                    if (errorBuffer)
                    {
                        STAFString errMsg = STAFString(
                            "Command does not exist: ") + command;
                        *errorBuffer = errMsg.adoptImpl();
                    }

                    return kSTAFBaseOSError;
                }

                if (errorBuffer)
                {
                    STAFString errMsg = STAFString("Invalid command: ") +
                        command;
                    *errorBuffer = errMsg.adoptImpl();
                }

                return rc;
            }

            // Command contains path of startData->command (from getFilePath 
            // call), so here we construct something like "/usr/bin" + "/" + 
            // "ls" = "/usr/bin/ls"

            // ???: This used to end with startData->command, not just command.
            //      This messed things up when SHELL was added.  There doesn't
            //      seem to be a reason why startData->command was used instead
            //      of just command.

            command = STAFString(path, STAFString::kShallow) +
                      STAFString("/") + command;
        }

        // Check if command is a regular file and has execute perms
        if (stat(command.toCurrentCodePage()->buffer(), &fileinfo) == -1)
        {
            systemErrorCode = errno;

            if (osRC) *osRC = systemErrorCode;  // set by stat

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Invalid command: ") + command +
                    "\nThe command is not a file or does not have execute "
                    "permissions.\nOS RC " + systemErrorCode;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError; 
        }
        else if (!S_ISREG(fileinfo.st_mode))
        {
            if (osRC) *osRC = ENOEXEC; // exec format error

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Invalid command: ") +
                    command + "\nThe command is not a valid executable.";
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
        else if (!S_ISEXE(fileinfo.st_mode))
        {
            if (osRC) *osRC = EACCES;  // permission denied

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Cannot execute command: ") +
                    command + "\nIt does not have the necessary permissions.";
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }

    // Check workdir (if any) is a directory file and has execute perms
    if (startData->workdir != 0)
    {
        if (stat(STAFString(startData->workdir).toCurrentCodePage()->buffer(), 
            &fileinfo) == -1)
        {
            systemErrorCode = errno;

            if (osRC) *osRC = systemErrorCode;  // set by stat

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Invalid working directory: ") +
                    startData->workdir + "\nOS RC " + systemErrorCode;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        } 
        else if (!S_ISDIR(fileinfo.st_mode))
        {
            if (osRC) *osRC = ENOTDIR;  // not a directory

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Working directory is not ") +
                    "a directory: " + startData->workdir;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
        else if (!S_ISEXE(fileinfo.st_mode))
        {
            if (osRC) *osRC = EACCES;   // permission denied

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Working directory ") +
                    startData->workdir +
                    " does not have the necessary permissions.";
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }

    // Note: These descriptors are closed in ProcessMonitorThread()

    int child_stdin  = dup(0);
    int child_stdout = dup(1);
    int child_stderr = dup(2);

    // change stdin if appropriate
    if (startData->stdinMode == kSTAFProcessIOReadFile)
    {
        close(child_stdin);

        child_stdin = open(STAFString(startData->stdinRedirect)
                           .toCurrentCodePage()->buffer(), O_RDONLY);

        if (child_stdin == -1)
        {
            systemErrorCode = errno;

            close(child_stdout);
            close(child_stderr);

            if (osRC) *osRC = systemErrorCode;

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Error opening stdin file ") +
                    startData->stdinRedirect + " as readonly.\nOS RC " +
                    systemErrorCode;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }

    // Check if is NEW_CONSOLE specified.  If no stdout file was specified,
    // redirect stdout to /dev/null instead of to STAFProc's stdout.
    // If no stderr file was specified, redirect stderr to /dev/null instead
    // of to STAFProc's stdout.

    if (startData->consoleMode == kSTAFProcessNewConsole)
    {
        if (startData->stdoutMode == kSTAFProcessIONoRedirect &&
            startData->stderrMode == kSTAFProcessIONoRedirect)
        {
            // Redirect stdout and stderr to /dev/null (instead of to
            // STAFProc's stdout/stderr)
            startData->stdoutMode = kSTAFProcessIOReplaceFile;
            startData->stdoutRedirect = sDefaultRedirectFile.getImpl();
            startData->stderrMode = kSTAFProcessIOStdout;
        }
        else if (startData->stdoutMode == kSTAFProcessIONoRedirect)
        {
            // Redirect stdout to /dev/null (instead of to STAFProc's stdout)
            startData->stdoutMode = kSTAFProcessIOReplaceFile;
            startData->stdoutRedirect = sDefaultRedirectFile.getImpl();
        }
        else if (startData->stderrMode == kSTAFProcessIONoRedirect)
        {
            // Redirect stderr to /dev/null (instead of to STAFProc's stderr)
            startData->stderrMode = kSTAFProcessIOReplaceFile;
            startData->stderrRedirect = sDefaultRedirectFile.getImpl();
        }
    }

    // change stdout if appropriate
    if (startData->stdoutMode != kSTAFProcessIONoRedirect)
    {
        int outFlags = O_CREAT | O_WRONLY;

        if (startData->stdoutMode == kSTAFProcessIOAppendFile)
            outFlags |= O_APPEND;
        else
            outFlags |= O_TRUNC;

        close(child_stdout);

        child_stdout = open(STAFString(startData->stdoutRedirect)
                            .toCurrentCodePage()->buffer(), outFlags,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                            S_IROTH | S_IWOTH);

        if (child_stdout == -1)
        {
            systemErrorCode = errno;

            close(child_stdin);
            close(child_stderr);

            if (osRC) *osRC = systemErrorCode;

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Error opening stdout file: ") +
                    startData->stdoutRedirect + "\nOS RC " + systemErrorCode;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }

    // change stderr if appropriate
    if (startData->stderrMode == kSTAFProcessIOStdout)
    {
        close(child_stderr);
        child_stderr = dup(child_stdout);
    }
    else if (startData->stderrMode != kSTAFProcessIONoRedirect)
    {
        int errFlags = O_CREAT | O_WRONLY;

        if (startData->stderrMode == kSTAFProcessIOAppendFile)
            errFlags |= O_APPEND;
        else
            errFlags |= O_TRUNC;

        close(child_stderr);

        child_stderr = open(STAFString(startData->stderrRedirect)
                            .toCurrentCodePage()->buffer(), errFlags,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                            S_IROTH | S_IWOTH);
        
        if (child_stderr == -1)
        {
            systemErrorCode = errno;

            close(child_stdin);
            close(child_stdout);

            if (osRC) *osRC = systemErrorCode;

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Error opening stderr file: ") +
                    startData->stderrRedirect + "\nOS RC " + systemErrorCode;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }

    // determine what to do with authentication based on the following
    // variables: authMode, default username/password, request user -
    // name/password, and operating system

    bool mustValidate = true;

    if (startData->authMode == kSTAFProcessAuthDisabled)
    {
        mustValidate = false;

        if (startData->disabledAuthAction == kSTAFProcessDisabledAuthError)
        {
            if (startData->username != 0)
            {
                if (osRC) *osRC = EACCES;

                if (errorBuffer)
                {
                    STAFString errMsg = STAFString(
                        "Process authentication denied.  You cannot specify a "
                        "userid when process authentication is disabled and "
                        "the process authentication disabled action is set "
                        "to 'Error'.");
                    *errorBuffer = errMsg.adoptImpl();
                }

                return kSTAFProcessAuthenticationDenied;
            }
        }
    }
    
    STAFUserID_t  usrID = sOurUID;
    STAFGroupID_t grpID = sOurGID;
    
    unsigned int validUser = UserAuthenticate(usrID, grpID,
                                              STAFString(startData->username),
                                              STAFString(startData->password),
                                              mustValidate, osRC);

    if (!validUser || ((sOurUID != 0) && (sOurUID != usrID)))
    {
        if (errorBuffer)
        {
            STAFString errMsg = STAFString("Error during process ") +
                "authentication for user name " + startData->username;
            *errorBuffer = errMsg.adoptImpl();
        }

        return kSTAFProcessAuthenticationDenied;
    }

    STAFString buffer = startData->command;

    if (startData->parms != 0)
        buffer += STAFString(" ") + startData->parms;

    // create argument table from the command and parameters passed. argv is
    // allocated by parseCommandParms and it must be deleted from this point
    // on before any return or thrown exception

    char **argv = 0;
    unsigned int argc = 4;

    if (startData->commandType == kSTAFProcessCommand)
    {
        argc = ParseCommandParms(buffer, &argv);

        if (argc == 0)
        {
            if (osRC) *osRC = EINVAL;  // invalid argument
            delete[] argv;

            if (errorBuffer)
            {
                STAFString errMsg = STAFString("Parsing command arguments ") +
                    "failed.  Invalid command: " + buffer;
                *errorBuffer = errMsg.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }
    else
    {
        // startData->commandType == kSTAFProcessShell

        STAFString commandString;
        STAFString output;

        if (startData->shellCommand != 0)
        {
            // Substitute the command and possibly other data

            commandString = startData->shellCommand;
            
            STAFProcessShellSubstitutionData subData;
            subData.command = buffer;
            subData.title = startData->title;
            subData.workload = startData->workload;
            subData.username = startData->username;
            subData.password = startData->password;

            if (startData->stdinMode == kSTAFProcessIOReadFile)
                subData.stdinfile = "< " +
                                    STAFString(startData->stdinRedirect);

            if (startData->stdoutMode != kSTAFProcessIONoRedirect)
            {
                if (startData->stdoutMode == kSTAFProcessIOAppendFile)
                    subData.stdoutfile = ">> " + 
                                         STAFString(startData->stdoutRedirect);
                else
                    subData.stdoutfile = "> " +
                                         STAFString(startData->stdoutRedirect);
            }

            if (startData->stderrMode == kSTAFProcessIOStdout)
            {
                subData.stdoutfile = subData.stdoutfile + " 2>&1";
            }
            else if (startData->stderrMode != kSTAFProcessIONoRedirect)
            {
                if (startData->stderrMode == kSTAFProcessIOAppendFile)
                    subData.stderrfile = "2>> " + 
                                         STAFString(startData->stderrRedirect);
                else
                    subData.stderrfile = "2> " +
                                         STAFString(startData->stderrRedirect);
            }

            int rc = STAFProcessReplaceShellSubstitutionChars(commandString,
                                                              subData, output);
            if (rc != kSTAFOk)
            {
                if (errorBuffer)
                {
                    STAFString errMsg = STAFString("Invalid shell command") +
                        " value: " + startData->shellCommand;
                    *errorBuffer = errMsg.adoptImpl();
                }

                return rc;
            }

            buffer = output;
        }

        argv = new char *[4];
        argv[0] = new char [3];
        argv[1] = new char [3];
        argv[2] = new char [buffer.toCurrentCodePage()->length() + 1];
        argv[3] = 0;

        strcpy(argv[0], "sh");
        strcpy(argv[1], "-c");
        strcpy(argv[2], buffer.toCurrentCodePage()->buffer());
    }

    // create environment table if appropriate (this block requires 2 passes
    // of the memory block, one pass to count number of variables so that we
    // can allocate the correct amount of memory for the table, and a second
    // pass to initialize the table to point to string in the memory block.
    // NOTE: this block allocates memory for the envp table. the memory gets
    // deleted before any return point in the parent process, whereas in the
    // child process it gets overwritten by execve

    int       i = 0;
    char **envp = 0;
    int    envc = 0;
    char *envstr = startData->environment;

    if (envstr != 0)
    {
        // count number of entries
        while (*envstr)
        {
            envc++;
            envstr += strlen(envstr) + 1;
        }

        // allocate memory for the environment table
        envp = new char* [envc + 1];
        envstr = startData->environment;

        // initialize environment table
        while (*envstr)
        {
            envp[i++] = envstr;
            envstr += strlen(envstr) + 1;
        }
        envp[i] = 0;
    }

    // now argv and envp are ready, so lock the static variables and let
    // the processMonitorThread know that we are ready to start a process

    STAFMutexSemLock lock(sCreateProcessSem);
    STAFRC_t rc = kSTAFOk;
    STAFProcessEndCallbackLevel1 dummyCallback = { 0 };

    sProcessCreateInfo = ProcessCreateInfo(
        startData->commandType, command.toCurrentCodePage(), argv, envp,
        usrID, grpID, STAFString(startData->username).toCurrentCodePage(),
        STAFString(startData->workdir).toCurrentCodePage(),
        startData->stdinMode, child_stdin,
        startData->stdoutMode, child_stdout,
        startData->stderrMode, child_stderr,
        (startData->callback != 0) ? *startData->callback : dummyCallback);

    // we have a process we want to start so let process thread know
    sProcessThread.post();  

    // wait until process has been created so that we can pick pid
    sProcessCreated.wait();
    sProcessCreated.reset();

    if (pid)        *pid = sProcessCreateInfo.pid;
    if (procHandle) *procHandle = sProcessCreateInfo.pid;

    if (sProcessCreateInfo.pid == -1)
    {
        rc = kSTAFBaseOSError;

        if (errorBuffer)
        {
            STAFString errMsg = STAFString("Error starting the process.");
            *errorBuffer = errMsg.adoptImpl();
        }
    }

    // recurse over arg vector and clean up
    for (unsigned int j = 0; j < argc; j++)
        delete[] argv[j];

    delete[] argv;
    delete[] envp;

    return rc;
}


STAFRC_t STAFProcessStop(STAFProcessID_t pid,
                         STAFProcessStopMethod_t stopMethod,
                         unsigned int *osRC)
{
    return STAFProcessStop2(pid, stopMethod, kSTAFProcessStopRequest, osRC);
}

STAFRC_t STAFProcessStop2(STAFProcessID_t pid,
                          STAFProcessStopMethod_t stopMethod,
                          STAFProcessStopFlag_t stopFlag,
                          unsigned int *osRC)
{
    // Determine the signal to send based on the stopMethod

    int theSignal = 0;
    bool sendSignalToChildProcesses = false;

    if (stopMethod == kSTAFProcessStopWithSigKillAll)
    {
        theSignal = SIGKILL;
        sendSignalToChildProcesses = true;
    }
    else if (stopMethod == kSTAFProcessStopWithSigKill)
    {
        theSignal = SIGKILL;
    }
    else if (stopMethod == kSTAFProcessStopWithSigTermAll)
    {
        theSignal = SIGTERM;
        sendSignalToChildProcesses = true;
    }
    else if (stopMethod == kSTAFProcessStopWithSigTerm)
    {
        theSignal = SIGTERM;
    }
    else if (stopMethod == kSTAFProcessStopWithSigIntAll)
    {
        theSignal = SIGINT;
        sendSignalToChildProcesses = true;
    }
    else if (stopMethod == kSTAFProcessStopWithSigInt)
    {
        theSignal = SIGINT;
    }
    else
    {
        return kSTAFInvalidValue;
    }
    
    // Check if a process with the specified pid exists

    if (stopFlag == kSTAFProcessKillRequest)
    {
        // Check if any process with this pid exists

        if (kill(pid, 0) == -1)
            return kSTAFDoesNotExist;
    }
    else // stopFlag == kSTAFProcessStopRequest
    {
        // Check if a process started by STAF exists by checking if the
        // process exists in sMonitorList
        
        STAFMutexSemLock lock(sMonitorDataSem);
        ProcessMonitorInfo processMonitorInfo;
        ProcessMonitorMap::iterator iter = sMonitorMap.find(pid);

        if (iter == sMonitorMap.end())
        {
            // Did not find a process with the specified pid
            return kSTAFHandleDoesNotExist;
        }
    }

    // Stop the process(es) using the specified stop method by sending a
    // signal to the process(es).  If the signal is to be sent to child
    // processes, we do that by specifying the negative value of the pid
    // to the kill() command in order to send the signal to every process
    // in the process group.

    if (kill((sendSignalToChildProcesses) ? -pid : pid, theSignal) == -1)
    {
        // Error sending the signal to the process(es)

        if (osRC) *osRC = errno;
        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}

STAFRC_t STAFProcessIsValidAuthMode(STAFProcessAuthenticationMode_t authMode)
{

    if ((authMode != kSTAFProcessAuthDisabled) &&
        (authMode != kSTAFProcessAuthNone))
        return kSTAFInvalidValue;
    else
        return kSTAFOk;
}


STAFRC_t STAFProcessIsValidStopMethod(STAFProcessStopMethod_t stopMethod)
{
    if (stopMethod == kSTAFProcessStopWithWM_CLOSE)
        return kSTAFInvalidValue;
    else
        return kSTAFOk;
}


STAFRC_t STAFProcessRegisterEndCallback(STAFProcessID_t pid,
                                        STAFProcessHandle_t procHandle,
                                        void *genericCallback,
                                        unsigned int callbackLevel)
{
    // NOTE: Registered processes will always return a fake rc of 0

    if (genericCallback == 0) return kSTAFInvalidValue;
    if (callbackLevel != 1) return kSTAFInvalidValue;

    STAFMutexSemLock lock(sMonitorDataSem);
    STAFProcessEndCallbackLevel1 *callback =
        reinterpret_cast<STAFProcessEndCallbackLevel1 *>(genericCallback);

    sMonitorMap[pid].push_back(ProcessMonitorInfo(procHandle, pid, *callback));
 
    // Needed here too in case "outsider" processes are run before any
    // "insider" processes are run
    InitProcessManager();

    return kSTAFOk;
}

STAFRC_t STAFProcessGetHandleFromID(STAFProcessID_t processID,
                                    STAFProcessHandle_t *processHandle,
                                    unsigned int *osRC)
{
    return STAFProcessGetHandleFromID2(
        processID, processHandle, kSTAFProcessStopRequest, osRC);
}

STAFRC_t STAFProcessGetHandleFromID2(STAFProcessID_t processID,
                                     STAFProcessHandle_t *procHandle,
                                     STAFProcessStopFlag_t stopFlag,
                                     unsigned int *osRC)
{
    if (procHandle == 0) return kSTAFInvalidValue;

    *procHandle = processID;

    return kSTAFOk;
}


STAFRC_t STAFProcessIsRunning(STAFProcessHandle_t processHandle,
                              unsigned int *isRunning,
                              unsigned int *osRC)
{
    if (isRunning == 0) return kSTAFInvalidValue;

    if (kill(processHandle, 0) != -1) *isRunning = 1;
    else *isRunning = 0;

    return kSTAFOk;
}


int ParseCommandParms(STAFString &command, char ***argv)
{
    static STAFString dquote = STAFString(kUTF8_DQUOTE);
    static STAFString bslash = STAFString(kUTF8_BSLASH);
    static STAFString space  = STAFString(kUTF8_SPACE);

    STAFString currChar, nextArg;

    // we actually allocate words (which may not match the number
    // of real args, but should guarantee enough slots in the array)

    int words = command.numWords();
    *argv = new char *[words + 1];
    memset(*argv, 0, sizeof(char *) * (words + 1));

    int i = 0, j = 0;
    int inQuotes = 0;
    int inEscape = 0;
    int argReady = 0;

    for (i = 0; i < command.length(STAFString::kChar); ++i)
    {
        currChar = command.subString(i, 1, STAFString::kChar);

        if ((currChar == space) && !inEscape && !inQuotes)
        {
            if (argReady)
            {
                argReady = 0;
                STAFStringBufferPtr nextArgInCurrCP = 
                    nextArg.toCurrentCodePage();
                unsigned int nextArgInCurrCPLen = 
                    nextArgInCurrCP->length();
                (*argv)[j] = new char[nextArgInCurrCPLen + 1];
                strcpy((*argv)[j++], nextArgInCurrCP->buffer());
                nextArg = "";
            }

            continue;
        }
        else if ((currChar == dquote) && !inEscape)
        {
            if (inQuotes)
            {
                // Needed to handle an empty string parameter
                argReady = 1;
            }
            
            inQuotes = !inQuotes;
            continue;
        }
        else if ((currChar == bslash) && !inEscape)
        {
            inEscape = 1;
            continue;
        }
        else
        {
            inEscape = 0;
            argReady = 1;
            nextArg += currChar;
        }
    }

    if (argReady)
    {
        STAFStringBufferPtr nextArgInCurrCP = 
            nextArg.toCurrentCodePage();
        unsigned int nextArgInCurrCPLen = 
            nextArgInCurrCP->length();
        (*argv)[j] = new char[nextArgInCurrCPLen + 1];
        strcpy((*argv)[j++], nextArgInCurrCP->buffer());
    }

    // return the number of allocated slots in argv
    return j;
}

unsigned int UserAuthenticate(STAFUserID_t &uid, STAFGroupID_t &gid,
                              const STAFString &username,
                              const STAFString &password, 
                              bool mustValidate, unsigned int *osRC)
{
    // Note: if username has not been specified (neither in the request
    //       nor as a default) then we simply take no action at all

    if (username.length() == 0 || mustValidate == false) return 1;

    char realUsername[256] = { 0 };
    char realPassword[256] = { 0 };

    STAFStringBufferPtr realUsernameInCurrCP = username.toCurrentCodePage();
    STAFStringBufferPtr realPasswordInCurrCP = password.toCurrentCodePage();

    strcpy(realUsername, realUsernameInCurrCP->buffer());
    strcpy(realPassword, realPasswordInCurrCP->buffer());

    // Note: we already handled the case where username is empty (above)

    struct passwd *n = 0;

    if (username.isDigits())
    {
        // if username is uid, resolve & convert ...
        try
        {
            n = getpwuid(uid = username.asUInt());
        
            // XXX: This was the old way
            // if (n) username = n->pw_name;
            // but, I think this is actually correct
            if (n) strcpy(realUsername, n->pw_name);
        }
        catch (...)
        {
            // Note:  asUInt() will throw an exception if not in range
            // 0 to UINT_MAX
        }
    }
    else
    {
        // ... otherwise, get username and set uid
        n = getpwnam(realUsername);

        if (n) uid = n->pw_uid;
    }

    if (n == NULL)
    {
        return 0;
    }

    gid = n->pw_gid;

    return (*sAuthenticateFuncPtr)(realUsername, realPassword);
}

unsigned int sAuthNone(const char *user, const char *pswd)
{
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef    STAF_PAM_AVAILABLE
#include <security/pam_appl.h>
#include <security/pam_misc.h>
static   struct pam_conv conv = { misc_conv, NULL };
#endif // STAF_PAM_AVAILABLE

unsigned int sAuthPam(const char *user, const char *pswd)
{

#ifdef    STAF_PAM_AVAILABLE

    // DOCUMENTATION FOUND AT:
    // http://www.us.kernel.org/pub/linux/libs/pam/Linux-PAM-html/
    // pam_appl.html

    // Add the following lines to /etc/pam.conf
    // STAFProc  auth    required  /lib/security/pam_unix_auth.so
    // STAFProc  account required  /lib/security/pam_unix_acct.so

    int rc = PAM_SUCCESS;
    pam_handle_t *pamh = NULL;
    const char *serviceName = "STAFProc";

    rc = pam_start(serviceName, user, &conv, &pamh);

    if (rc == PAM_SUCCESS)
    {
        // is user really user?
        rc = pam_authenticate(pamh, 0);
    }

    if (rc == PAM_SUCCESS)
    {
        // permitted access?
        rc = pam_acct_mgmt(pamh, 0);
    }

    // This is where we have been authorized or not.

    if (rc == PAM_SUCCESS)
    {
        if (pam_end(pamh, rc) != PAM_SUCCESS)
        {
            STAFTrace::trace(
                kSTAFTraceError,
                serviceName + ": Failed to release authenticator");
        }

        return 1;
    }

#endif // STAF_PAM_AVAILABLE

    return 0;
}

////////////////////////////////////////////////////////////////////////////////




