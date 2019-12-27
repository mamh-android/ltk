/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProcess.h"
#include "STAF_iostream.h"
#include "STAFThread.h"
#include "STAFMutexSem.h"
#include "STAFEventSem.h"
#include "STAFException.h"
#include "STAFTrace.h"
#include "STAFUtilWin32.h"
#include "STAFInternalProcess.h"
#include <deque>
#include <map>
#include <aclapi.h>    // For CreateProcessAsUser()
#include <errno.h>     // For EACCES

#define _WIN32_WINNT 0x400

// For some reason, winuser.h doesn't define these useful aliases
#ifndef WINSTA_ALL_ACCESS
#define WINSTA_ALL_ACCESS 0x0000037F
#endif

#ifndef DESKTOP_ALL_ACCESS
#define DESKTOP_ALL_ACCESS 0x00001FF
#endif

typedef struct PROFILEINFOIMPL {
   DWORD    dwSize;          // Must be set to sizeof(PROFILEINFO)
   DWORD    dwFlags;         // See flags above
   LPTSTR   lpUserName;      // User name (required)
   LPTSTR   lpProfilePath;   // Roaming profile path
   LPTSTR   lpDefaultPath;   // Default user profile path
   LPTSTR   lpServerName;    // Validating DC name in netbios format
   LPTSTR   lpPolicyPath;    // Path to the NT4 style policy file
   HANDLE   hProfile;        // Registry key handle - filled by function
} PROFILEINFO, FAR * LPPROFILEINFO;

// Typedefs for function pointers in USERENV.DLL
typedef BOOL (STDMETHODCALLTYPE FAR * LPFNLOADUSERPROFILE) (
   HANDLE hToken,                // user token
   LPPROFILEINFO lpProfileInfo   // user profile information
);

typedef BOOL (STDMETHODCALLTYPE FAR * LPFNUNLOADUSERPROFILE) (
   HANDLE hToken,            // user token
   HANDLE hProfile           // user profile handle
);

typedef BOOL (STDMETHODCALLTYPE FAR * LPFNCREATEENVBLOCK) (
   LPVOID *lpEnvironment,    // environment block
   HANDLE hToken,            // user token
   BOOL   bInherit           // inheritance
);

typedef BOOL (STDMETHODCALLTYPE FAR * LPFNDESTROYENVBLOCK) (
   LPVOID lpEnvironment    // environment block
);

HMODULE                 sUserEnvLib                  = NULL;
LPFNLOADUSERPROFILE     sLoadUserProfileFunc         = NULL;
LPFNUNLOADUSERPROFILE   sUnloadUserProfileFunc       = NULL;
LPFNCREATEENVBLOCK      sCreateEnvironmentBlockFunc  = NULL;
LPFNDESTROYENVBLOCK     sDestroyEnvironmentBlockFunc = NULL;

HANDLE sProcessHeap = GetProcessHeap();

// End of definitions for CreateUserAsProcess() stuff

// Typedefs for function pointers in ADVAPI32.DLL
typedef BOOL (STDMETHODCALLTYPE FAR * LPFNSETSECURITYINFO) (
   HANDLE hToken,                // user token
   SE_OBJECT_TYPE ObjectType, // type of object 
   SECURITY_INFORMATION SecurityInfo, // type of security information to set 
   PSID psidOwner, // pointer to the new owner SID 
   PSID psidGroup, // pointer to the new primary group SID 
   PACL pDacl, // pointer to the new DACL 
   PACL pSacl // pointer to the new SACL
);

HMODULE                 sAdvApi32Lib                 = NULL;
LPFNSETSECURITYINFO     sSetSecurityInfoFunc         = NULL;

// End of definitions for SetSecurityInfo() stuff

struct ProcessMonitorInfo
{
    ProcessMonitorInfo(STAFProcessHandle_t aHandle = 0, STAFProcessID_t aPID = 0,
                       STAFProcessEndCallbackLevel1 aCallback =
                           STAFProcessEndCallbackLevel1(),
                       HANDLE aUsrToken = 0, HANDLE aUsrProfile = 0)
        : handle(aHandle), pid(aPID), callback(aCallback),
          hUsrToken(aUsrToken), hUsrProfile(aUsrProfile)
    { /* Do Nothing */ }

    STAFProcessHandle_t handle;
    STAFProcessID_t pid;
    STAFProcessEndCallbackLevel1 callback;
    unsigned int callbackLevel;
    HANDLE hUsrToken;
    HANDLE hUsrProfile;
};

typedef std::deque<ProcessMonitorInfo> ProcessMonitorList;
typedef std::map<STAFProcessHandle_t, ProcessMonitorList> ProcessMonitorMap;

struct ProcessMonitorThreadData
{
    HANDLE monitorWakeUp;
    ProcessMonitorMap monitorMap;
};

typedef STAFRefPtr<ProcessMonitorThreadData> ProcessMonitorThreadDataPtr;
typedef std::deque<ProcessMonitorThreadDataPtr> ProcessMonitorThreadList;


// Prototypes

static BOOL InitUserEnv();
static BOOL InitAdvApi32();
static void GetPrivilegeError(HANDLE handle, STAFString &errorMsg);
static int GrantAccessToWinsta(HANDLE hUsrToken, bool bGrant,
                               STAFString &errorMsg);
static void DeleteMatchingAces(ACL* pdacl, void* psid);
static unsigned int ProcessMonitorThread(void *);
static unsigned int UserAuthenticate(STAFUserID_t &hUsrToken,
                                     STAFString &username,
                                     STAFString &password, bool mustValidate,
                                     unsigned int *osRC,
                                     STAFString &errorMsg);


// Static data

static STAFMutexSem sMonitorDataSem;
static STAFEventSem sThreadSyncSem;
static ProcessMonitorThreadDataPtr sTempThreadData;
static ProcessMonitorThreadList sThreadList;


// Substitution characters valid for a shell command on Windows
char *gSTAFProcessShellSubstitutionChars = "cCpPtTuUwWxX%";


/*****************************************************************************/
/*                               Helper APIs                                 */
/*****************************************************************************/

// Writes trace warning with GetLastError()
static void WriteTraceWarning(const STAFString &msg)
{
    STAFTrace::trace(kSTAFTraceWarning, STAFString(msg) + 
                     " in STAFProcess::startProcess, OS_RC: " +
                     GetLastError());
}


// Writes trace error with GetLastError()
static void WriteTraceError(const STAFString &msg)
{
    STAFTrace::trace(kSTAFTraceError, STAFString(msg) + 
                     " in STAFProcess::startProcess, OS_RC: " +
                     GetLastError());
}


// Writes trace error with GetLastError()
static void WriteTraceError2(const STAFString &msg)
{
    STAFTrace::trace(kSTAFTraceError, STAFString(msg) + ", OS_RC: " +
                     GetLastError());
}


// Writes trace error without GetLastError()
static void WriteTraceError3(const STAFString &msg)
{
    STAFTrace::trace(kSTAFTraceError, msg);
}


void InitProcessManager()
{
    static STAFMutexSem initSem;
    static bool alreadyInited = false;

    if (alreadyInited) return;

    STAFMutexSemLock initLock(initSem);

    if (alreadyInited) return;
    
    if (STAFUtilWin32GetWinType() & kSTAFWinNTPlus)
    {
        // Set USERENV.DLL function pointers for CreateUserAsProcess()
        // if running on Windows NT/2000/XP or above.

        int rc = InitUserEnv();

        if (rc != TRUE)
            WriteTraceError2("InitUserEnv() failed in InitProcessManager()");
            
        // Set ADVAPI32.DLL function pointers for SetSecurityInfo()
        // if running on Windows NT/2000/XP or above.

        rc = InitAdvApi32();

        if (rc != TRUE)
            WriteTraceError2("InitAdvApi32() failed in InitProcessManager()");
    }
    
    alreadyInited = true;
}


unsigned int ProcessMonitorThread(void *)
{
    try
    {
        ProcessMonitorThreadDataPtr threadData = sTempThreadData;
        threadData->monitorWakeUp = CreateEvent(0, TRUE, FALSE, 0);

        sThreadSyncSem.post();

        HANDLE handles[MAXIMUM_WAIT_OBJECTS] = { 0 };
        unsigned int signaledHandleIndex = 0;
        ProcessMonitorMap::iterator iter = threadData->monitorMap.begin();

        for(unsigned int numHandles = 0;; numHandles = 0)
        {
            // Use a nested block to ease lock usage
            {
                STAFMutexSemLock lock(sMonitorDataSem);

                for(iter = threadData->monitorMap.begin();
                    iter != threadData->monitorMap.end(); 
                    iter++)
                {
                    if (numHandles == (MAXIMUM_WAIT_OBJECTS - 1)) break;

                    handles[numHandles++] = iter->first;
                }

                handles[numHandles++] = threadData->monitorWakeUp;
            }

            DWORD rc = WaitForMultipleObjects(numHandles, handles, FALSE,
                                              INFINITE);

            if ((rc >= WAIT_OBJECT_0) &&
                (rc <= (WAIT_OBJECT_0 + numHandles - 1)))
            {
                signaledHandleIndex = (unsigned int)(rc - WAIT_OBJECT_0);
            }
            else if ((rc >= WAIT_ABANDONED_0) &&
                     (rc <= (WAIT_ABANDONED_0 + numHandles - 1)))
            {
                signaledHandleIndex = (unsigned int)(rc - WAIT_ABANDONED_0);
            }
            else
            {
                // We either got WAIT_FAILED, WAIT_TIMEOUT or some other return
                // code which we weren't expecting, so spit out an error
                // message and continue

                if (rc == WAIT_FAILED)
                {
                    WriteTraceError2("Got WAIT_FAILED from "
                                     "WaitForMultipleObjects() in "
                                     "ProcessMonitorThread()");
                }
                else if (rc == WAIT_TIMEOUT)
                {
                    STAFTrace::trace(kSTAFTraceError, "Got WAIT_TIMEOUT "
                                     "from WaitForMultipleObjects() "
                                     "in ProcessMonitorThread()");
                }
                else
                {
                    STAFTrace::trace(kSTAFTraceError, "Unexpected RC, " + 
                                     STAFString(rc) +
                                     ", from WaitForMultipleObjects() "
                                     "in processMonitorThread()");
                }
            }

            if (signaledHandleIndex != (numHandles - 1))
            {
                unsigned int retCode = 0;

                // Get the process return code and then free the handle

                GetExitCodeProcess(handles[signaledHandleIndex],
                                   (LPDWORD)&retCode);
                CloseHandle(handles[signaledHandleIndex]);

                // Now lock the list while we search for the appropriate
                // entry and call the callback

                sMonitorDataSem.request();

                for(iter = threadData->monitorMap.begin();
                    iter != threadData->monitorMap.end() &&
                    (iter->first != handles[signaledHandleIndex]);
                    iter++)
                { /* Do Nothing */ }

                if (iter != threadData->monitorMap.end())
                {
                    ProcessMonitorList monitorList = iter->second;

                    threadData->monitorMap.erase(iter->first);
                    sMonitorDataSem.release();

                    for (ProcessMonitorList::iterator monitorIter =
                                                      monitorList.begin();
                         monitorIter != monitorList.end(); ++monitorIter)
                    {
                        ProcessMonitorInfo &processMonitorInfo = *monitorIter;

                        if (processMonitorInfo.hUsrToken != 0)
                        {   // Perform cleanup for CreateProcessAsUser()

                            // Unload the user profile
                            if (processMonitorInfo.hUsrProfile != 0)
                            {
                                if (!sUnloadUserProfileFunc(
                                    processMonitorInfo.hUsrToken,
                                    processMonitorInfo.hUsrProfile))
                                {
                                    WriteTraceError2("UnloadUserProfile() "
                                                     "failed in ProcessMonitor"
                                                     "Thread()");
                                }
                            }

                            // Clean up the interactive winsta/desktop DACLs
                            STAFString errorMsg;
                            GrantAccessToWinsta(processMonitorInfo.hUsrToken,
                                                FALSE, errorMsg);

                            CloseHandle(processMonitorInfo.hUsrToken);
                        }

                        if (processMonitorInfo.callback.callback != 0)
                        {
                            processMonitorInfo.callback.callback(
                                processMonitorInfo.pid,
                                processMonitorInfo.handle, retCode,
                                processMonitorInfo.callback.data);
                        }
                    }
                }
                else
                {
                    // We shouldn't have been waiting on an object that is
                    // no longer in our list.  Spit out an error and continue.
                    // Move along ... Nothing to see here.

                    STAFTrace::trace(kSTAFTraceError, "Signaled object from "
                                     "WaitForMultipleObjects() not in list in "
                                     "processMonitorThread()");
                    sMonitorDataSem.release();
                }

            } // end if we were not awoken by our event semaphore
            else ResetEvent(threadData->monitorWakeUp);

        }  // end of endless for loop
    }
    catch (STAFException &se)
    {
        se.trace("STAFProcess::ProcessMonitorThread()");
    }
    catch (...)
    {
        STAFTrace::trace(kSTAFTraceError, "Caught unknown exception in "
                         "STAFProcess::ProcessMonitorThread()");
    }

    return kSTAFUnknownError;
}


STAFRC_t AddProcessMonitor(const ProcessMonitorInfo &monitor)
{
    STAFMutexSemLock lock(sMonitorDataSem);
    ProcessMonitorThreadList::iterator iter = sThreadList.begin();

    for (; iter != sThreadList.end(); ++iter)
    {
        if ((*iter)->monitorMap.size() < (MAXIMUM_WAIT_OBJECTS - 1)) break;
    }

    ProcessMonitorThreadDataPtr threadData;

    if (iter == sThreadList.end())
    {
        threadData = ProcessMonitorThreadDataPtr(
            new ProcessMonitorThreadData(), ProcessMonitorThreadDataPtr::INIT);

        sTempThreadData = threadData;

        sThreadSyncSem.reset();

        STAFThreadID_t newThread = 0;
        unsigned int osRC = 0;
        STAFRC_t stafRC = STAFThreadStart(&newThread, ProcessMonitorThread,
                                          0, 0, &osRC);

        if (stafRC != kSTAFOk)
        {
            WriteTraceError3("Error starting initial Process Manager thread in "
                             "InitProcessManager, RC: " + STAFString(stafRC) +
                             ", OS RC: " + STAFString(osRC));
            return stafRC;
        }

        sThreadList.push_back(threadData);

        sThreadSyncSem.wait();
    }
    else threadData = *iter;

    threadData->monitorMap[monitor.handle].push_back(monitor);
    SetEvent(threadData->monitorWakeUp);

    return 0;
}

STAFRC_t STAFProcessStart(STAFProcessID_t     *processID,
                          STAFProcessHandle_t *processHandle,
                          void                *data,
                          unsigned int         startDataLevel,
                          unsigned int        *osRC)
{
    STAFString_t errorBuffer = 0;

    return STAFProcessStart2(processID, processHandle, data, startDataLevel,
                             osRC, &errorBuffer);
}

STAFRC_t STAFProcessStart2(STAFProcessID_t     *processID,
                           STAFProcessHandle_t *processHandle,
                           void                *data,
                           unsigned int        startDataLevel,
                           unsigned int        *osRC,
                           STAFString_t        *errorBuffer)
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

    STAFProcessStartInfoLevel1 *startData =
        reinterpret_cast<STAFProcessStartInfoLevel1 *>(data);

    InitProcessManager();

    BOOL rc = TRUE;
    STAFRC_t ret = kSTAFOk;
    unsigned int systemErrorCode = 0;
    
    // determine what to do with authentication based on the following
    // variables: authMode, default username/password, request user -
    // name/password, and operating system

    STAFString username(startData->username);
    STAFString password(startData->password);
    bool mustValidate = true;
    STAFStringBufferPtr usernamePtr;

    if (startData->authMode == kSTAFProcessAuthDisabled)
    {
        mustValidate = false;

        if (startData->disabledAuthAction == kSTAFProcessDisabledAuthError)
        {
            if (username.length() != 0)
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

    STAFUserID_t hUsrToken = 0;
    STAFString errorMsg;
    unsigned int validUser = UserAuthenticate(hUsrToken, username, password,
                                              mustValidate, osRC, errorMsg);

    if (!validUser)
    {
        if (errorBuffer) *errorBuffer = errorMsg.adoptImpl();
        return kSTAFProcessAuthenticationDenied;
    }
    
    PROFILEINFO profinfo = { sizeof profinfo, 0, NULL };
    
    if (hUsrToken != 0)
    {
        usernamePtr = username.toCurrentCodePage();
        profinfo.lpUserName = const_cast<char *>(usernamePtr->buffer());
    }

    STAFString commandString;
    STAFString output;

    if (startData->commandType == kSTAFProcessShell)
    {
        if (startData->shellCommand != 0)
        {
            // Substitute the command and possibly other data

            commandString = startData->shellCommand;
            
            STAFProcessShellSubstitutionData subData;
            subData.command = startData->command;
            if (startData->parms != 0)
            {
                subData.command += " ";
                subData.command += startData->parms;
            }
            subData.title = startData->title;
            subData.workload = startData->workload;
            subData.username = username;
            subData.password = password;
            
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
                    STAFString errMsg = STAFString(
                        "Invalid shell command: ") +
                        startData->shellCommand;
                    *errorBuffer = errMsg.adoptImpl();
                }

                return rc;
            }

            commandString = output;
        }
        else
        {
            // Added double quotes to the beginning and end of the entire command/
            // parms to handle cases where specifying cmd.exe with the /c option
            // does not preserve quotes.  Do a cmd /? to get more info on this.

            if (STAFUtilWin32GetWinType() & kSTAFWinNTPlus)
                commandString = "cmd.exe /c \"";
            else
                commandString = "command.com /c ";

            commandString += startData->command;

            if (startData->parms != 0)
            {
                commandString += " ";
                commandString += startData->parms;
            }

            if (STAFUtilWin32GetWinType() & kSTAFWinNTPlus)
                commandString += "\"";
        }
    }
    else
    {   
        // Not a shell command

        commandString = startData->command;
 
        if (startData->parms != 0)
        {
            commandString += " ";
            commandString += startData->parms;
        }
    }

    STAFStringBufferPtr commandPtr = commandString.toCurrentCodePage();
    STAFStringBufferPtr titlePtr;
    STAFStringBufferPtr workdirPtr;
    char *command = const_cast<char *>(commandPtr->buffer());
    char *workdir = 0;
    char *environment = startData->environment;
    
    STARTUPINFO startInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };

    startInfo.cb = sizeof(startInfo);

    if (startData->title != 0)
    {
        titlePtr = STAFString(startData->title).toCurrentCodePage();
        startInfo.lpTitle = const_cast<char *>(titlePtr->buffer());
    }

    if (startData->workdir != 0)
    {
        workdirPtr = STAFString(startData->workdir).toCurrentCodePage();
        workdir = const_cast<char *>(workdirPtr->buffer());
    }

    // Make sure that files specified for stdout and stderr are not the same
    // Note:  In STAFProcessService, could only do a case-insensitive check
    //        since Unix files are case-sensitive.
    if ((startData->stdoutMode != kSTAFProcessIONoRedirect) &&
        (startData->stderrMode != kSTAFProcessIONoRedirect) &&
        (STAFString(startData->stdoutRedirect).toLowerCase() ==
         STAFString(startData->stderrRedirect).toLowerCase()))
    {
        if (errorBuffer)
        {
            STAFString errMsg = STAFString(
                "You cannot specify the same file name for stdout "
                "and stderr.");
            *errorBuffer = errMsg.adoptImpl();
        }

        return kSTAFInvalidValue;
    }

    HANDLE newInpHandle;
    HANDLE newOutHandle;
    HANDLE newErrHandle;

    // set standard input, output, and error

    if (startData->stdinMode != kSTAFProcessIONoRedirect)
    {
        startInfo.dwFlags |= STARTF_USESTDHANDLES;

        newInpHandle = CreateFile(
                       STAFString(startData->stdinRedirect).
                           toCurrentCodePage()->buffer(),
                       GENERIC_READ | GENERIC_WRITE, 
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        if (newInpHandle == INVALID_HANDLE_VALUE)
        {
            if (ret == kSTAFOk)
            {
                ret = kSTAFBaseOSError;
                systemErrorCode = GetLastError();
                if (osRC) *osRC = systemErrorCode;

                if (errorBuffer)
                {
                    STAFString_t systemErrorMsg = 0;

                    STAFUtilWin32LookupSystemErrorMessage(
                        systemErrorCode, &systemErrorMsg);

                    STAFString errMsg = STAFString("Error creating ") +
                        "stdin file: " + startData->stdinRedirect +
                        "\nOS RC " + systemErrorCode;

                    STAFString systemErrorString = STAFString(
                        systemErrorMsg, STAFString::kShallow);

                    if (systemErrorString.length() != 0)
                        errMsg += ": " + STAFString(systemErrorString);

                    *errorBuffer = errMsg.adoptImpl();
                }
            }
        }
    }
    else newInpHandle = GetStdHandle(STD_INPUT_HANDLE);

    if (startData->stdoutMode != kSTAFProcessIONoRedirect)
    {
        startInfo.dwFlags |= STARTF_USESTDHANDLES;

        newOutHandle = CreateFile(
                       STAFString(startData->stdoutRedirect).
                           toCurrentCodePage()->buffer(),
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

        if (newOutHandle == INVALID_HANDLE_VALUE)
        {
            if (ret == kSTAFOk)
            {
                ret = kSTAFBaseOSError;
                systemErrorCode = GetLastError();
                if (osRC) *osRC = systemErrorCode;

                if (errorBuffer)
                {
                    STAFString_t systemErrorMsg = 0;

                    STAFUtilWin32LookupSystemErrorMessage(
                        systemErrorCode, &systemErrorMsg);

                    STAFString errMsg = STAFString("Error creating ") +
                        "stdout file: " + startData->stdoutRedirect +
                        "\nOS RC " + systemErrorCode;

                    STAFString systemErrorString = STAFString(
                        systemErrorMsg, STAFString::kShallow);

                    if (systemErrorString.length() != 0)
                        errMsg += ": " + STAFString(systemErrorString);

                    *errorBuffer = errMsg.adoptImpl();
                }
            }
        }
        else
        {
            if (startData->stdoutMode == kSTAFProcessIOAppendFile)
            {
                DWORD outFP = SetFilePointer(
                    newOutHandle, GetFileSize(newOutHandle, NULL),
                    NULL, FILE_BEGIN);

                if (outFP == (unsigned)-1)
                {
                    if (ret == kSTAFOk)
                    {
                        ret = kSTAFBaseOSError;
                        systemErrorCode = GetLastError();
                        if (osRC) *osRC = systemErrorCode;

                        if (errorBuffer)
                        {
                            STAFString_t systemErrorMsg = 0;

                            STAFUtilWin32LookupSystemErrorMessage(
                                systemErrorCode, &systemErrorMsg);

                            STAFString errMsg = STAFString(
                                "Error appending to stdout file: ") +
                                startData->stdoutRedirect +
                                "\nOS RC " + systemErrorCode;

                            STAFString systemErrorString = STAFString(
                                systemErrorMsg, STAFString::kShallow);

                            if (systemErrorString.length() != 0)
                                errMsg += ": " + STAFString(systemErrorString);

                            *errorBuffer = errMsg.adoptImpl();
                        }
                    }
                }
            }
            else if (!SetEndOfFile(newOutHandle))
            {
                if (osRC) *osRC = GetLastError();
                ret  = kSTAFBaseOSError;
            }
        }
    }
    else newOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    HANDLE currentProcess = GetCurrentProcess();

    if (startData->stderrMode == kSTAFProcessIOStdout)
    {
        if (!DuplicateHandle(currentProcess, newOutHandle, currentProcess,
            &newErrHandle, 0, TRUE, DUPLICATE_SAME_ACCESS))
        {
            ret  = kSTAFBaseOSError;
            systemErrorCode = GetLastError();
            if (osRC) *osRC = systemErrorCode;

            if (errorBuffer)
            {
                STAFString_t systemErrorMsg = 0;

                STAFUtilWin32LookupSystemErrorMessage(
                    systemErrorCode, &systemErrorMsg);

                STAFString errMsg = STAFString("Error redirecting ") +
                    "stderr to stdout.\nOS RC " + systemErrorCode;

                STAFString systemErrorString = STAFString(
                    systemErrorMsg, STAFString::kShallow);

                if (systemErrorString.length() != 0)
                    errMsg += ": " + STAFString(systemErrorString);

                *errorBuffer = errMsg.adoptImpl();
            }
        }
    }
    else if (startData->stderrMode != kSTAFProcessIONoRedirect)
    {
        startInfo.dwFlags |= STARTF_USESTDHANDLES;

        newErrHandle = CreateFile(
                       STAFString(startData->stderrRedirect).
                           toCurrentCodePage()->buffer(),
                       GENERIC_READ | GENERIC_WRITE, 
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

        if (newErrHandle == INVALID_HANDLE_VALUE)
        {
            if (ret == kSTAFOk)
            {
                ret = kSTAFBaseOSError;
                systemErrorCode = GetLastError();
                if (osRC) *osRC = systemErrorCode;

                if (errorBuffer)
                {
                    STAFString_t systemErrorMsg = 0;

                    STAFUtilWin32LookupSystemErrorMessage(
                        systemErrorCode, &systemErrorMsg);

                    STAFString errMsg = STAFString("Error creating ") +
                        "stderr file: " + startData->stderrRedirect +
                        "\nOS RC " + systemErrorCode;

                    STAFString systemErrorString = STAFString(
                        systemErrorMsg, STAFString::kShallow);

                    if (systemErrorString.length() != 0)
                        errMsg += ": " + STAFString(systemErrorString);

                    *errorBuffer = errMsg.adoptImpl();
                }
            }
        }
        else
        {
            if (startData->stderrMode == kSTAFProcessIOAppendFile)
            {
                DWORD errFP = SetFilePointer(
                    newErrHandle, GetFileSize(newErrHandle, NULL),
                    NULL, FILE_BEGIN);

                if (errFP == (unsigned)-1)
                {
                    if (ret == kSTAFOk)
                    {
                        ret = kSTAFBaseOSError;
                        systemErrorCode = GetLastError();
                        if (osRC) *osRC = systemErrorCode;

                        if (errorBuffer)
                        {
                            STAFString_t systemErrorMsg = 0;

                            STAFUtilWin32LookupSystemErrorMessage(
                                systemErrorCode, &systemErrorMsg);

                            STAFString errMsg = STAFString(
                                "Error appending to stderr file: ") +
                                startData->stderrRedirect +
                                "\nOS RC " + systemErrorCode;

                            STAFString systemErrorString = STAFString(
                                systemErrorMsg, STAFString::kShallow);

                            if (systemErrorString.length() != 0)
                                errMsg += ": " + STAFString(systemErrorString);

                            *errorBuffer = errMsg.adoptImpl();
                        }
                    }
                }
            }
            else if (!SetEndOfFile(newErrHandle))
            {
                if (ret == kSTAFOk)
                {
                    ret = kSTAFBaseOSError;
                    systemErrorCode = GetLastError();
                    if (osRC) *osRC = systemErrorCode;

                    if (errorBuffer)
                    {
                        STAFString_t systemErrorMsg = 0;

                        STAFUtilWin32LookupSystemErrorMessage(
                            systemErrorCode, &systemErrorMsg);

                        STAFString errMsg = STAFString(
                            "Error setting the end of file for stderr file ") +
                            startData->stderrRedirect +
                            "\nOS RC " + systemErrorCode;

                        STAFString systemErrorString = STAFString(
                            systemErrorMsg, STAFString::kShallow);

                        if (systemErrorString.length() != 0)
                            errMsg += ": " + STAFString(systemErrorString);

                        *errorBuffer = errMsg.adoptImpl();
                    }
                }
            }
        }
    }
    else newErrHandle = GetStdHandle(STD_ERROR_HANDLE);

    // Note: Do not use DUPLICATE_CLOSE_SOURCE, as I am closing
    //       the handles below. Since in some cases the  handle
    //       we are duplicating may be the actual console (when
    //       no stdin/stdout/stderr has been specified), we get
    //       into trouble by specifying this option.  Instead I
    //       check what we duplicated before closing the handle.

    startInfo.hStdInput  = INVALID_HANDLE_VALUE;
    startInfo.hStdOutput = INVALID_HANDLE_VALUE;
    startInfo.hStdError  = INVALID_HANDLE_VALUE;

    if ((ret == kSTAFOk) &&
        (!DuplicateHandle(currentProcess, newInpHandle, currentProcess,
                          &startInfo.hStdInput,  0, TRUE,
                          DUPLICATE_SAME_ACCESS)))
    {
        ret = kSTAFBaseOSError;
        systemErrorCode = GetLastError();
        if (osRC) *osRC = systemErrorCode;

        if (errorBuffer)
        {
            STAFString_t systemErrorMsg = 0;

            STAFUtilWin32LookupSystemErrorMessage(
                systemErrorCode, &systemErrorMsg);

            STAFString errMsg = STAFString(
                "Error duplicating the handle for stdin file: ") +
                startData->stdinRedirect + "\nOS RC " + systemErrorCode;

            STAFString systemErrorString = STAFString(
                systemErrorMsg, STAFString::kShallow);

            if (systemErrorString.length() != 0)
                errMsg += ": " + STAFString(systemErrorString);

            *errorBuffer = errMsg.adoptImpl();
        }
    }

    if ((ret == kSTAFOk) &&
        (!DuplicateHandle(currentProcess, newOutHandle, currentProcess,
                          &startInfo.hStdOutput, 0, TRUE,
                          DUPLICATE_SAME_ACCESS)))
    {
        ret = kSTAFBaseOSError;
        systemErrorCode = GetLastError();
        if (osRC) *osRC = systemErrorCode;

        if (errorBuffer)
        {
            STAFString_t systemErrorMsg = 0;

            STAFUtilWin32LookupSystemErrorMessage(
                systemErrorCode, &systemErrorMsg);

            STAFString errMsg = STAFString(
                "Error duplicating the handle for stdout file: ") +
                startData->stdoutRedirect + "\nOS RC " + systemErrorCode;

            STAFString systemErrorString = STAFString(
                systemErrorMsg, STAFString::kShallow);

            if (systemErrorString.length() != 0)
                errMsg += ": " + STAFString(systemErrorString);

            *errorBuffer = errMsg.adoptImpl();
        }
    }

    if ((ret == kSTAFOk) &&
        (!DuplicateHandle(currentProcess, newErrHandle, currentProcess,
                          &startInfo.hStdError, 0, TRUE,
                          DUPLICATE_SAME_ACCESS)))
    {
        ret  = kSTAFBaseOSError;
        systemErrorCode = GetLastError();
        if (osRC) *osRC = systemErrorCode;

        if (errorBuffer)
        {
            STAFString_t systemErrorMsg = 0;

            STAFUtilWin32LookupSystemErrorMessage(
                systemErrorCode, &systemErrorMsg);

            STAFString errMsg = STAFString(
                "Error duplicating the handle for stderr file: ") +
                startData->stderrRedirect + "\nOS RC " + systemErrorCode;

            STAFString systemErrorString = STAFString(
                systemErrorMsg, STAFString::kShallow);

            if (systemErrorString.length() != 0)
                errMsg += ": " + STAFString(systemErrorString);

            *errorBuffer = errMsg.adoptImpl();
        }
    }

    if (ret == kSTAFOk)
    {
        startInfo.dwFlags |= STARTF_USESHOWWINDOW;

        if (startData->consoleFocus == kSTAFProcessForeground)
            startInfo.wShowWindow = SW_SHOW;
        else if (startData->consoleFocus == kSTAFProcessMinimized)
            startInfo.wShowWindow = SW_SHOWMINNOACTIVE;
        else
            startInfo.wShowWindow = SW_SHOWNOACTIVATE;

        BOOL ranCreateProcessAsUser = FALSE;
        STAFString returnErrorMsg = "";
        STAFString failedFunction = "";

        if (hUsrToken == 0)
        {
            rc = CreateProcess(
                0, command, 0, 0, TRUE,
                ((startData->consoleMode == kSTAFProcessNewConsole) ? 
                 CREATE_NEW_CONSOLE : 0) | CREATE_NEW_PROCESS_GROUP,
                environment, workdir, &startInfo, &processInfo);

            if (rc != TRUE)
            {
                systemErrorCode = GetLastError();
                failedFunction = "Error starting the process. CreateProcess";
            }
        }
        else
        {   // Create Process As a Different User:
            
            // Direct the process into WinSta0, the interactive logon session
            startInfo.lpDesktop = "WinSta0\\default";

            BOOL errorFound = FALSE;

            // Adjust the interactive winsta/desktop DACLs
            STAFString grantAccessToWinstaErrMsg;
            rc = GrantAccessToWinsta(hUsrToken, TRUE,
                                     grantAccessToWinstaErrMsg);

            if (rc != TRUE)
            {
                errorFound = TRUE;
                systemErrorCode = GetLastError();
                failedFunction = "GrantAccessToWinsta (" +
                    grantAccessToWinstaErrMsg + ")";
            }
            else
            {
                // Load the user profile
                rc = sLoadUserProfileFunc(hUsrToken, &profinfo);
            }

            if (rc != TRUE)
            {
                if (!errorFound)
                {
                    profinfo.hProfile = 0;
                    errorFound = TRUE;
                    systemErrorCode = GetLastError();
                    failedFunction = "LoadUserProfile";
                }
            }
            else
            {
                // Create an UNICODE environment block (contains environment 
                // variables for the specified user in a form that can be
                // passed directly to CreateProcessAsUser
                rc = sCreateEnvironmentBlockFunc(
                     reinterpret_cast<void**>(&environment), hUsrToken, FALSE);
            }

            BOOL unicodeEnvBlock = TRUE;

            if (rc != TRUE)
            {
                if (!errorFound)
                {
                    errorFound = TRUE;
                    systemErrorCode = GetLastError();
                    failedFunction = "CreateEnvironmentBlock";
                }
            }
            else
            {
                char *newEnvBuf = 0;
                
                if (startData->environment != 0)
                {
                    // Merge environment variables (specified on the PROCESS
                    // START request via the ENV parameters) that are in UTF-8
                    // map with the UNICODE environment block for the user.
                    // Resulting environment block is the current code page.
                    
                    unicodeEnvBlock = FALSE;
                    
                    // Find the end of the user's UNICODE environment block

                    int numWideChars = 0;

                    for (int i = 0; !numWideChars; i += 2)
                    {
                        if (environment[i]   == 0 && environment[i+1] == 0 && 
                            environment[i+2] == 0 && environment[i+3] == 0)
                        {
                            numWideChars = (i + 4) / 2;  // Get # of wide chars
                        }
                    }
                    
                    // Determine the # of bytes required for MultiByte block

                    int numBytes = WideCharToMultiByte(CP_UTF8, 0,
                                   reinterpret_cast<wchar_t *>(environment),
                                   numWideChars, 0, 0, NULL, NULL);

                    // Allocate new multi-byte environment block

                    char *newEnvBlock = new char[numBytes];

                    // Convert the UNICODE env block to MultiByte (UTF-8)

                    WideCharToMultiByte(CP_UTF8, 0,
                        reinterpret_cast<wchar_t *>(environment),
                        numWideChars, newEnvBlock, numBytes, NULL, NULL);
                    
                    // Break up the env block string (in UTF-8) into a map

                    STAFString envBlock(newEnvBlock, numBytes, 
                                        STAFString::kUTF8);

                    delete [] newEnvBlock;  // Delete since already copied

                    // Create a STAFProcessEnvMap from the env block string so
                    // that the key (the environment variable name) can be
                    // case-insensitive if designated by the operating system.

                    STAFProcessEnvMap envMap;
                    BOOL moreEnvVars = TRUE;

                    while (moreEnvVars)
                    {
                        unsigned int endPos = envBlock.find(kUTF8_NULL);
                        if (endPos == 0)
                        {
                            moreEnvVars = FALSE;
                        }
                        else
                        {
                            STAFString envStr(envBlock.subString(0, endPos));
                            unsigned int equalPos = envStr.find(kUTF8_EQUAL);
                            if (equalPos == 0)
                            {
                                moreEnvVars = FALSE;
                            }
                            else
                            {
                                STAFString envName(
                                    envStr.subString(0, equalPos));
                                STAFString envNameKey(envName.toUpperCase());
                                STAFString envValue;
                                envValue = envStr.subString(equalPos + 1);

                                envMap[envNameKey] = STAFProcessEnvData(
                                    envName, envValue);

                                if (endPos + 1 > numBytes)
                                    moreEnvVars = FALSE;
                                else
                                    envBlock = STAFString(
                                        envBlock.subString(endPos+1));
                            }
                        }
                    }

                    // Merge the env vars specified on the process start
                    // request (stored in startData->userEnvMap) into envMap

                    for (unsigned int userEnvIndex = 0;
                         userEnvIndex < startData->userEnvCount;
                         ++userEnvIndex)
                    {
                        STAFString userEnv(startData->userEnvList[userEnvIndex]);
                        unsigned int equalPos = userEnv.find(kUTF8_EQUAL);

                        if (equalPos != STAFString::kNPos)
                        {
                            STAFString aEnvName(userEnv.subString(0, equalPos));
                            STAFString aEnvNameKey(aEnvName.toUpperCase());
                            STAFString aEnvValue(
                                userEnv.subString(equalPos + 1));

                            // Retain original env variable name if exists
                            if (envMap.find(aEnvNameKey) != envMap.end())
                                envMap[aEnvNameKey].envValue = aEnvValue;
                            else
                                envMap[aEnvNameKey] = STAFProcessEnvData(
                                    aEnvName, aEnvValue);
                        }
                    }

                    // Iterate thru the merged map (envMap): combine entries
                    // back into Name=Value form, get the current code page
                    // representation and figure out how big a buffer we need.
                    
                    int size = 0;
                    std::deque<STAFStringBufferPtr> envList;

                    for (STAFProcessEnvMap::iterator iter2 = envMap.begin();
                        iter2 != envMap.end(); ++iter2)
                    {
                        STAFProcessEnvData envData = iter2->second;

                        STAFString envCombo = envData.envName + kUTF8_EQUAL +
                            envData.envValue;

                        STAFStringBufferPtr envComboPtr = 
                            envCombo.toCurrentCodePage();

                        // Add 1 for the null byte
                        size += envComboPtr->length() + 1; 

                        envList.push_back(envComboPtr);
                    }

                    // Allocate the buffer

                    size += 1;                // Add one for the trailing null
                    newEnvBuf = new char[size];

                    // Walk the list and add the entries to the buffer

                    size = 0;

                    for (std::deque<STAFStringBufferPtr>::iterator iter = 
                         envList.begin(); iter != envList.end(); ++iter)
                    {
                        memcpy(newEnvBuf + size, (*iter)->buffer(), 
                               (*iter)->length());
                        newEnvBuf[size + (*iter)->length()] = 0;
                        size += (*iter)->length() + 1;
                    }

                    // Add the trailing null

                    newEnvBuf[size] = 0;

                }  // End merge of environment block

                ranCreateProcessAsUser = TRUE;

                // Call CreateProcessAsUser function to create a new process
                // which executes a specified executable file.  The new process
                // runs in the security context of the user represented by the
                // hToken parameter.  The process inherits the environment
                // associated with the specified user and any additional
                // environment variables specified.  Run process in the user's
                // interactive desktop (via startInfo).

                rc = CreateProcessAsUser(hUsrToken, 0, command, 0, 0, TRUE,
                     ((startData->consoleMode == kSTAFProcessNewConsole) ?
                      CREATE_NEW_CONSOLE : 0) | CREATE_NEW_PROCESS_GROUP |
                      (unicodeEnvBlock ? CREATE_UNICODE_ENVIRONMENT : 0),
                      (unicodeEnvBlock ? environment : newEnvBuf),
                      workdir, &startInfo, &processInfo);

                if (rc != TRUE)
                {
                    systemErrorCode = GetLastError();
                    failedFunction = "Error starting the process. "
                        "CreateProcessAsUser";
                }
                
                if (unicodeEnvBlock)
                    delete [] newEnvBuf;
                
                // Free the environment block (it's already been copied)
                if (!sDestroyEnvironmentBlockFunc(environment))
                    WriteTraceError("DestroyEnvironmentBlock()");
            }
        }

        if (rc != TRUE)
        {
            STAFString_t systemErrorMsg = 0;

            STAFUtilWin32LookupSystemErrorMessage(
                systemErrorCode, &systemErrorMsg);

            returnErrorMsg = failedFunction + " failed with OS RC " +
                STAFString(systemErrorCode);
             
            STAFString systemErrorString = STAFString(
                systemErrorMsg, STAFString::kShallow);

            if (systemErrorString.length() != 0)
                returnErrorMsg += ": " + STAFString(systemErrorString);

            if ((systemErrorCode == ERROR_FILE_NOT_FOUND) ||
                (systemErrorCode == ERROR_PATH_NOT_FOUND))
            {
                returnErrorMsg += "\nInvalid command: " +
                        STAFString(command);
            }
            else if (ERROR_DIRECTORY && startData->workdir != 0)
            {
                returnErrorMsg += "\nInvalid working directory: " +
                    STAFString(workdir);
            }
            else if (ranCreateProcessAsUser &&
                (systemErrorCode == ERROR_PRIVILEGE_NOT_HELD))
            {
                // Add additional information to the error message

                HANDLE processToken = 0;

                if (OpenProcessToken(GetCurrentProcess(),
                                     TOKEN_ALL_ACCESS, &processToken))
                {
                    STAFString privilegeErrorMsg = "";
                    GetPrivilegeError(processToken, privilegeErrorMsg);
                    returnErrorMsg += "\n" + privilegeErrorMsg;
                }
            }

            ret  = kSTAFBaseOSError;
            if (osRC) *osRC = systemErrorCode;
            if (errorBuffer) *errorBuffer = returnErrorMsg.adoptImpl();
        }
    }

    CloseHandle(processInfo.hThread);

    CloseHandle(startInfo.hStdInput);
    CloseHandle(startInfo.hStdOutput);
    CloseHandle(startInfo.hStdError);

    if (startData->stdinMode != kSTAFProcessIONoRedirect)
        CloseHandle(newInpHandle);
    if (startData->stdoutMode != kSTAFProcessIONoRedirect)
        CloseHandle(newOutHandle);      
    if (startData->stderrMode != kSTAFProcessIONoRedirect)
        CloseHandle(newErrHandle);

    if (ret != kSTAFOk) return ret;

    if (processID)     *processID     = processInfo.dwProcessId;
    if (processHandle) *processHandle = processInfo.hProcess;

    STAFProcessEndCallbackLevel1 dummyCallback = { 0 };
    
    ret = AddProcessMonitor(ProcessMonitorInfo(
        processInfo.hProcess, processInfo.dwProcessId,
        (startData->callback != 0) ? *startData->callback : dummyCallback,
        hUsrToken, profinfo.hProfile));

    return ret;
}


BOOL CALLBACK sendWM_CLOSE(HWND winHandle, LPARAM pid)
{
    DWORD winPID = 0;
    DWORD winTID = GetWindowThreadProcessId(winHandle, &winPID);

    if (winPID == pid)
    {
        BOOL rc = PostMessage(winHandle, WM_CLOSE, 0, 0);

        if (rc == FALSE)
        {
            // XXX: What to do?  Maybe STAFTrace?
        }
    }

    return TRUE;
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
    if ((stopMethod != kSTAFProcessStopWithSigKillAll) &&
        (stopMethod != kSTAFProcessStopWithSigKill) &&
        (stopMethod != kSTAFProcessStopWithSigTerm) &&
        (stopMethod != kSTAFProcessStopWithSigInt) &&
        (stopMethod != kSTAFProcessStopWithWM_CLOSE))
    {
        return kSTAFInvalidValue;
    }

    // Check if a process with the specified pid exists and get it's process
    // handle

    STAFProcessHandle_t processHandle = 0;

    if (stopFlag == kSTAFProcessKillRequest)
    {
        // Check if any process with this pid exists

        unsigned int osRC2 = 0;
        STAFRC_t rc = STAFProcessGetHandleFromID(pid, &processHandle, &osRC2);

        if (rc != kSTAFOk) return kSTAFDoesNotExist;  // Pid does not exist
    }
    else  // stopFlag == kSTAFProcessStopRequest
    {
        // Check if a process started by STAF exists by walking through
        // sThreadList and each corresponding monitor map to determine if the
        // pid exists

        bool found = false;
        STAFMutexSemLock monitorListLock(sMonitorDataSem);

        ProcessMonitorInfo info;

        for (ProcessMonitorThreadList::iterator threadIter = 
             sThreadList.begin(); threadIter != sThreadList.end();
             ++threadIter)
        {
            for (ProcessMonitorMap::iterator mapIter =
                 (*threadIter)->monitorMap.begin();
                 mapIter != (*threadIter)->monitorMap.end(); ++mapIter)
            {
                for (ProcessMonitorList::iterator listIter = 
                     mapIter->second.begin();
                     listIter != mapIter->second.end(); ++ listIter)
                {
                    if (listIter->pid == pid)
                    {
                        // Found the process pid
                        
                        processHandle = listIter->handle;
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found) return kSTAFHandleDoesNotExist;
    }
    
    // Stop the process using the specified stop method

    BOOL rc = TRUE;

    if (stopMethod == kSTAFProcessStopWithSigKillAll)
    {
        if (STAFUtilWin32GetWinType() & kSTAFWinXPPlus)
        {
            // Use the taskkill command (available on Windows XP and later)
            // with the force (/f), tree (/t), and /pid options to stop the
            // process (discarding its stdout/stderr by redirecting to NUL)
            // because taskkill is the most reliable way to kill a process
            // (and its child processes) on Windows

            STAFString taskkillCmd = STAFString("taskkill /f /t /pid ") +
                pid + STAFString(" > NUL 2>&1");

            int systemRC = system(taskkillCmd.toCurrentCodePage()->buffer());

            // If taskkill fails, use the WM_CLOSE method

            if (systemRC != 0)
            {
                rc = EnumWindows(sendWM_CLOSE, pid);
            }
        }
        else
        {
            // On Windows 2000 and earlier, taskkill is not available so
            // use the WM_CLOSE method

            rc = EnumWindows(sendWM_CLOSE, pid);
        }
    }
    else if (stopMethod == kSTAFProcessStopWithWM_CLOSE)
    {
        rc = EnumWindows(sendWM_CLOSE, pid);
    }
    else if (stopMethod == kSTAFProcessStopWithSigKill)
    {
        rc = TerminateProcess(processHandle, 0xFFFFFFFF);
    }
    else if ((stopMethod == kSTAFProcessStopWithSigInt) ||
             (stopMethod == kSTAFProcessStopWithSigTerm))
    {
        rc = GenerateConsoleCtrlEvent(
            (stopMethod == kSTAFProcessStopWithSigInt) ?
             CTRL_C_EVENT : CTRL_BREAK_EVENT, pid);
    }
    else return kSTAFUnknownError;

    if (stopFlag == kSTAFProcessKillRequest)
    {
        // Need to close the handle that STAFProcessGetHandleFromID() opened
        // so that no handle leak occurs

        CloseHandle(processHandle);
    }

    if (rc == FALSE)
    {
        if (osRC) *osRC = GetLastError();
        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}


STAFRC_t STAFProcessRegisterEndCallback(STAFProcessID_t      pid,
                                        STAFProcessHandle_t  procHandle,
                                        void                *callback,
                                        unsigned int         callbackLevel)
{
    if (callback == 0) return kSTAFInvalidValue;

    if (callbackLevel != 1) return kSTAFInvalidValue;

    STAFProcessEndCallbackLevel1 *callbackLevel1 =
        reinterpret_cast<STAFProcessEndCallbackLevel1 *>(callback);

    return AddProcessMonitor(ProcessMonitorInfo(
        procHandle, pid, *callbackLevel1));
}


STAFRC_t STAFProcessGetHandleFromID(STAFProcessID_t processID,
                                    STAFProcessHandle_t *processHandle,
                                    unsigned int *osRC)
{
    return STAFProcessGetHandleFromID2(
        processID, processHandle, kSTAFProcessStopRequest, osRC);
}

STAFRC_t STAFProcessGetHandleFromID2(STAFProcessID_t processID,
                                     STAFProcessHandle_t *processHandle,
                                     STAFProcessStopFlag_t stopFlag,
                                     unsigned int *osRC)
{
    if (processHandle == 0) return kSTAFInvalidValue;
    
    if (stopFlag == kSTAFProcessStopRequest)
    {
        // Walk through the monitorList and each corresponding monitor map to
        // determine if the pid already exists and is still running.  If so,
        // then simply return the existing handle, rather than creating a 
        // new handle.  This fixes a Windows 95/98/Me problem with Java
        // services and WaitForMultipleObjects.
        {
            STAFMutexSemLock monitorListLock(sMonitorDataSem);

            ProcessMonitorInfo info;
            bool found = false;
        
            for (ProcessMonitorThreadList::iterator threadIter =
                 sThreadList.begin(); threadIter != sThreadList.end();
                 ++threadIter)
            {
                for (ProcessMonitorMap::iterator mapIter =
                     (*threadIter)->monitorMap.begin();
                     mapIter != (*threadIter)->monitorMap.end(); ++mapIter)
                {
                    for (ProcessMonitorList::iterator listIter = 
                         mapIter->second.begin();
                         listIter != mapIter->second.end(); ++ listIter)
                    {
                        if (listIter->pid == processID)
                        {
                            info = *listIter;
                            found = true;
                            break;
                        }
                    }
                }
            }        

            if (found)
            {            
                if (STAFProcess::isRunning(info.handle))
                {
                    *processHandle = info.handle;
                    return kSTAFOk;
                } 
            }        
        }
    }

    HANDLE theHandle = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_TERMINATE |
                                   PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                                   FALSE, processID);
    if (theHandle == NULL)
    {
        if (osRC) *osRC = GetLastError();
        return kSTAFBaseOSError;
    }

    *processHandle = theHandle;

    return kSTAFOk;
}


STAFRC_t STAFProcessIsRunning(STAFProcessHandle_t  processHandle,
                              unsigned int        *isRunning,
                              unsigned int        *osRC)
{
    if (isRunning == 0) return kSTAFInvalidValue;

    DWORD retCode = 0;
    BOOL rc = GetExitCodeProcess(processHandle, &retCode);

    if (rc != TRUE)
    {
        if (osRC) *osRC = GetLastError();
        return kSTAFBaseOSError;
    }

    *isRunning = (retCode == STILL_ACTIVE) ? 1 : 0;

    return kSTAFOk;
}


STAFRC_t STAFProcessIsValidAuthMode(STAFProcessAuthenticationMode_t authMode)
{
    if ((authMode != kSTAFProcessAuthDisabled) &&
        (authMode != kSTAFProcessAuthWindows))
        return kSTAFInvalidValue;
    else
        return kSTAFOk;
}


STAFRC_t STAFProcessIsValidStopMethod(STAFProcessStopMethod_t stopMethod)
{
    if ((stopMethod == kSTAFProcessStopWithSigTermAll) ||
        (stopMethod == kSTAFProcessStopWithSigIntAll))
    {
        return kSTAFInvalidValue;
    }

    return kSTAFOk;
}


unsigned int UserAuthenticate(STAFUserID_t &hUsrToken, STAFString &username,
                              STAFString &password, bool mustValidate,
                              unsigned int *osRC, STAFString &errorBuffer)
{
    // Note: if no username has been specified, take
    //       normal action

    if (username.length() == 0 || mustValidate == false) return 1;
    
    unsigned int systemErrorCode = 0;
    STAFString systemErrorMsg = "";

    char usernameBuffer[256] = { 0 };
    unsigned long buffLen = 256;

    BOOL rc = GetUserName(usernameBuffer, &buffLen);
    
    STAFString usernameStr = usernameBuffer;

    if (rc == FALSE)
    {
        systemErrorCode = GetLastError();
        if (osRC) *osRC = systemErrorCode;
        STAFString_t systemErrorMsg = 0;

        STAFUtilWin32LookupSystemErrorMessage(
            systemErrorCode, &systemErrorMsg);
        
        errorBuffer = STAFString(
            "Error during process authentication for user name: ") +
            username + "\nGetUserName failed with OS RC " + systemErrorCode;

        STAFString systemErrorString = STAFString(
            systemErrorMsg, STAFString::kShallow);

        if (systemErrorString.length() != 0)
            errorBuffer += ": " + STAFString(systemErrorMsg);

        return 0;
    }

    // is user trying to start a process of its own? if so, no
    // authentication is needed, passwd is pretty much ignored

    if (usernameStr.toLowerCase() == username.toLowerCase())
    {
        return 1;
    }
    
    STAFStringBufferPtr usernameInCCP = username.toCurrentCodePage();
    STAFStringBufferPtr passwordInCCP = password.toCurrentCodePage();

    // otherwise, create a handle that represents the specified
    // user

    rc = LogonUser((char *)usernameInCCP->buffer(), NULL,
                   (char *)passwordInCCP->buffer(),
                   LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                   &hUsrToken);

    if (rc == FALSE)
    {
        // An error occurred logging on as the user.
        // Set the error code and create an error message.

        systemErrorCode = GetLastError();
        if (osRC) *osRC = systemErrorCode;
        
        STAFString_t systemErrorMsg = 0;

        STAFUtilWin32LookupSystemErrorMessage(
            systemErrorCode, &systemErrorMsg);
        
        errorBuffer = STAFString(
            "Error during process authentication for user name: ") +
            username + "\nLogonUser failed with OS RC " + systemErrorCode;

        STAFString systemErrorString = STAFString(
            systemErrorMsg, STAFString::kShallow);

        if (systemErrorString.length() != 0)
            errorBuffer += ": " + STAFString(systemErrorMsg);

        if (systemErrorCode == ERROR_PRIVILEGE_NOT_HELD)
        {
            // Get current process's privileges to add to the error message

            HANDLE processToken = 0;

            if (OpenProcessToken(GetCurrentProcess(),
                                 TOKEN_ALL_ACCESS, &processToken))
            {
                STAFString privilegeErrorMsg;
                GetPrivilegeError(processToken, privilegeErrorMsg);
                errorBuffer += "\n" + privilegeErrorMsg;
            }
        }
    }
    
    return (rc == FALSE) ? 0 : 1;
}


// Note:
// The following functions were added to run a process under another user id
// on Windows NT/2000/XP.  The code was derived from Keith Brown's
// cmdasuser.cpp code obtained from Keith Brown's security samples page at
// http://www.develop.com/kbrown.  Keith Brown gave us his approval on
// 12/06/2001 to use the code.

// Helper functions for the Logon Session Broker

ACL* GetUserObjectDacl(HANDLE h, DWORD cbExtra)
{
    DWORD cb = 0;
    SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

    GetUserObjectSecurity(h, &si, 0, cb, &cb);

    if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        WriteTraceError("first GetUserObjectSecurity()");

    void* psd = HeapAlloc(sProcessHeap, 0, cb);

    if (!psd)
    {
        WriteTraceError(STAFString("HeapAlloc() in GetUserObjectDacl(), ") +
                        E_OUTOFMEMORY + ", ");
    }

    si = DACL_SECURITY_INFORMATION;

    if (!GetUserObjectSecurity(h, &si, psd, cb, &cb))
        WriteTraceError("GetUserObjectSecurity()");
    
    BOOL bPresent, bDefaulted;
    ACL* pdaclOld;

    if (!GetSecurityDescriptorDacl(psd, &bPresent, &pdaclOld, &bDefaulted))
        WriteTraceError("GetSecurityDescriptorDacl()");
       
    // Get the size of the existing DACL
    
    ACL_SIZE_INFORMATION info;

    if (!GetAclInformation(pdaclOld, &info, sizeof info, AclSizeInformation))
        WriteTraceError("GetAclInformation()");
    
    // Allocate enough memory for the existing DACL plus whatever extra space
    // the caller requires.

    cb = info.AclBytesInUse + cbExtra;
    ACL* pdaclNew = reinterpret_cast<ACL*>(HeapAlloc(sProcessHeap, 0, cb));
    
    if (!pdaclNew)
        WriteTraceError("HeapAlloc() in GetUserObjectDacl()");

    if (!InitializeAcl(pdaclNew, cb, ACL_REVISION))
        WriteTraceError("InitializeAcl()");

    // Copy over all the old aces to the new DACL
    for (DWORD i = 0; i < info.AceCount; ++i)
    {
        ACE_HEADER* pace = 0;

        if (!GetAce(pdaclOld, i, reinterpret_cast<void**>(&pace)))
            WriteTraceError("GetAce()");

        if (!AddAce(pdaclNew, ACL_REVISION, MAXDWORD, pace, pace->AceSize))
            WriteTraceError("AddAce()");
    } 
    
    HeapFree(sProcessHeap, 0, psd);
    return pdaclNew;
}

DWORD CalcAceSizeFromSid(void* psid)
{
    return sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(psid);
}

#if _WIN32_WINNT < 0x500
// This is such a simple function, it's bizarre that we had to wait until
// W2K to get it.

BOOL AddAccessAllowedAceEx(PACL pAcl, DWORD dwAceRevision, DWORD AceFlags,
                           DWORD AccessMask, PSID pSid)
{
    if (!AddAccessAllowedAce(pAcl, dwAceRevision, AccessMask, pSid))
        return FALSE;

    ACL_SIZE_INFORMATION info;

    if (!GetAclInformation(pAcl, &info, sizeof info, AclSizeInformation))
        return FALSE;

    ACE_HEADER* pace = 0;

    if (!GetAce(pAcl, info.AceCount - 1, reinterpret_cast<void**>(&pace)))
        return FALSE;

    pace->AceFlags = static_cast<BYTE>(AceFlags);

    return TRUE;
}
#endif  

void DeleteMatchingAces(ACL* pdacl, void* psid)
{
    ACL_SIZE_INFORMATION info;

    if (!GetAclInformation(pdacl, &info, sizeof info, AclSizeInformation))
        WriteTraceError("GetAclInformation() in DeleteMatchingAces()");
 
    // It's a bit easier to delete aces while iterating backwards so that
    // the iterator doesn't get honked up.
 
    DWORD i = info.AceCount;

    while (i--)
    {
        ACCESS_ALLOWED_ACE* pAce = 0;

        if (!GetAce(pdacl, i, reinterpret_cast<void**>(&pAce)))
            WriteTraceError("GetAce() in DeleteMatchingAces()");

        if (EqualSid(psid, &pAce->SidStart))
            DeleteAce(pdacl, i);
    }
}


/*******************************************************************************/
/* GrantAccessToWinsta - This function grants session SID access to the        */
/*                       interactive window station and the desktop.  This is  */
/*                       needed to direct a a new process into the interactive */
/*                       window station and desktop via CreateProcessAsUser(). */
/*                                                                             */
/* Accepts: (IN)  User token                                                   */
/*          (IN)  Boolean (true = grant access, false = remove access          */
/*          (OUT) STAFString (if function failed, returns 0, it contains       */
/*                            information on why the function failed)          */
/*                                                                             */
/* Returns: >0, if successful                                                  */
/*          0 , if unsuccessful                                                */
/*******************************************************************************/
int GrantAccessToWinsta(HANDLE hUsrToken, bool bGrant, STAFString &errorMsg)
{
    BYTE tgs[4096];
    DWORD cbtgs = sizeof tgs;

    if ( !GetTokenInformation(hUsrToken, TokenGroups, tgs, cbtgs, &cbtgs))
    {
        errorMsg = "GetTokenInformation";
        return 0;
    }

    const TOKEN_GROUPS* ptgs = reinterpret_cast<TOKEN_GROUPS*>(tgs);
    const SID_AND_ATTRIBUTES* it = ptgs->Groups;
    const SID_AND_ATTRIBUTES* end = it + ptgs->GroupCount;

    while (end != it)
    {
        if (it->Attributes & SE_GROUP_LOGON_ID)
            break;
        ++it;
    }

    if (end == it)
    {
        errorMsg = "No Logon SID in Token Groups";
        return 0;
    }
    
    void* psidLogonSession = it->Sid;

    HWINSTA hws = GetProcessWindowStation();

    if (!hws)
    {
        errorMsg = "GetProcessWindowStation";
        return 0;
    }

    {
        ACL* pdacl = 0;

        if (bGrant)
        {
            // Get the existing DACL, with enough extra space for adding a
            // couple more aces
            
            const DWORD cbExtra = 2 * CalcAceSizeFromSid(psidLogonSession);
            pdacl = GetUserObjectDacl(hws, cbExtra);
            
            // Grant the logon session all access to winsta0
           
            if (!AddAccessAllowedAce(pdacl, ACL_REVISION,
                WINSTA_ALL_ACCESS | STANDARD_RIGHTS_REQUIRED,
                psidLogonSession))
            {
                errorMsg = "AddAccessAllowedAce for winsta";
                return 0;
            }
            
            // Grant the logon session all access to any new desktops created
            // in WinSta0 by adding an inherit-only ace
           
            if (!AddAccessAllowedAceEx(pdacl, ACL_REVISION,
                SUB_CONTAINERS_AND_OBJECTS_INHERIT | INHERIT_ONLY,
                DESKTOP_ALL_ACCESS | STANDARD_RIGHTS_REQUIRED,
                psidLogonSession))
            {
                errorMsg = "AddAccessAllowedAceEx for winsta";
                return 0;
            }
        } 
        else
        {
            pdacl = GetUserObjectDacl(hws, 0);
            DeleteMatchingAces(pdacl, psidLogonSession);
        }
 
        // Apply the changes to winsta0.
        // Note: Unlike GetSecurityInfo, SetSecurityInfo has no troubles on SP3

        DWORD err = sSetSecurityInfoFunc(hws, SE_WINDOW_OBJECT,
                                         DACL_SECURITY_INFORMATION, 0, 0,
                                         pdacl, 0);
        if (err)
        {
            errorMsg = "SetSecurityInfo for winsta";
            return 0;
        }

        HeapFree(sProcessHeap, 0, pdacl);
    }

    CloseWindowStation(hws);
    
    HDESK hd = GetThreadDesktop(GetCurrentThreadId());

    if (!hd)
    {
        errorMsg = "GetThreadDesktop";
        return 0;
    }
   
    {
        ACL* pdacl = 0;

        if (bGrant)
        {
            // Get the existing DACL, with enough extra space for adding a
            // couple more aces
            
            const DWORD cbExtra = CalcAceSizeFromSid(psidLogonSession);
            pdacl = GetUserObjectDacl(hd, cbExtra);
            
            // Grant the logon session all access to winsta0
           
            if (!AddAccessAllowedAce(pdacl, ACL_REVISION, DESKTOP_ALL_ACCESS |
                                  STANDARD_RIGHTS_REQUIRED, psidLogonSession))
            {
                errorMsg = "AddAccessAllowedAce for desktop";
                return 0;
            }
        }
        else
        {
            pdacl = GetUserObjectDacl(hd, 0);
            DeleteMatchingAces(pdacl, psidLogonSession);
        }

        // Apply the changes to the default desktop
        
        DWORD err = sSetSecurityInfoFunc(hd, SE_WINDOW_OBJECT,
                                         DACL_SECURITY_INFORMATION, 0, 0,
                                         pdacl, 0);
        if (err)
        {
            errorMsg = "SetSecurityInfo for desktop";
            return 0;
        }

        HeapFree(sProcessHeap, 0, pdacl);
    }
    
    CloseDesktop(hd);
   
    return 1;
}


/*******************************************************************************/
/* InitUserEnv - This function dynamically links to USERENV.DLL and sets up    */
/*               the required function pointers                                */
/*                                                                             */
/* Accepts: Nothing                                                            */
/*                                                                             */
/* Returns: TRUE , if successful                                               */
/*          FALSE, otherwise                                                   */
/*******************************************************************************/
BOOL InitUserEnv()
{
   sUserEnvLib = LoadLibrary("userenv.dll");

   if (!sUserEnvLib)
   {
       WriteTraceError2("LoadLibrary(userenv.dll) in InitUserEnv()");
       return FALSE;
   }

   sLoadUserProfileFunc = (LPFNLOADUSERPROFILE)GetProcAddress(
                          sUserEnvLib, "LoadUserProfileA" );
   if (!sLoadUserProfileFunc)
   {
       WriteTraceError2("GetProcAddress(sLoadUserProfile) in "
                        "InitUserEnv()");
       return FALSE;
   }

   sUnloadUserProfileFunc = (LPFNUNLOADUSERPROFILE)GetProcAddress(
                            sUserEnvLib, "UnloadUserProfile" );
   if (!sUnloadUserProfileFunc)
   {
       WriteTraceError2("GetProcAddress(sUnloadUserProfile) in "
                        "InitUserEnv()");
       return FALSE;
   }

   sCreateEnvironmentBlockFunc = (LPFNCREATEENVBLOCK)GetProcAddress(
                                 sUserEnvLib, "CreateEnvironmentBlock" );
   if (!sCreateEnvironmentBlockFunc)
   {
       WriteTraceError2("GetProcAddress(sCreateEnvironmentBlockFunc) "
                        "in InitUserEnv()");
       return FALSE;
   }

   sDestroyEnvironmentBlockFunc = (LPFNDESTROYENVBLOCK)GetProcAddress(
                                  sUserEnvLib, "DestroyEnvironmentBlock" );
   if (!sDestroyEnvironmentBlockFunc)
   {
       WriteTraceError2("GetProcAddress(sDestroyEnvironmentBlockFunc) "
                        "in InitUserEnv()");
       return FALSE;
   }

   return TRUE;
}


/*******************************************************************************/
/* InitAdvApi32 - This function dynamically links to ADVAPI32.DLL and sets up  */
/*                the required function pointers                               */
/*                                                                             */
/* Accepts: Nothing                                                            */
/*                                                                             */
/* Returns: TRUE , if successful                                               */
/*          FALSE, otherwise                                                   */
/*******************************************************************************/
BOOL InitAdvApi32()
{
   sAdvApi32Lib = LoadLibrary("advapi32.dll");

   if (!sAdvApi32Lib)
   {
       WriteTraceError2("LoadLibrary(advapi32.dll) in InitAdvApi32()");
       return FALSE;
   }

   sSetSecurityInfoFunc = (LPFNSETSECURITYINFO)GetProcAddress(
                           sAdvApi32Lib, "SetSecurityInfo" );
   if (!sSetSecurityInfoFunc)
   {
       WriteTraceError2("GetProcAddress(sSetSecurityInfoFunc) in "
                        "InitAdvApi32()");
       return FALSE;
   }

   return TRUE;
}


/*******************************************************************************/
/* GetPrivilegeError - This function assigns an error message containing the   */
/*                     privileges that the currently logged on user is missing */
/*                     in order to run CreateProcessAsUser().                  */
/*                                                                             */
/* Accepts: (IN)  Current process token                                        */
/*          (OUT) Error message                                                */
/*                                                                             */
/* Returns: Nothing                                                            */
/*******************************************************************************/
void GetPrivilegeError(HANDLE handle, STAFString &errorMsg)
{
    char processPrivBuff[512] = { 0 };
    TOKEN_PRIVILEGES *processPriv =
                     reinterpret_cast<TOKEN_PRIVILEGES *>(processPrivBuff);
    DWORD sizeReq = 0;
    
    int rc = GetTokenInformation(handle, TokenPrivileges, processPriv,
                                 sizeof(processPrivBuff), &sizeReq);
    if (!rc)
    {
        WriteTraceError("GetTokenInformation()");
    }
    
    // Determine if has required privileges

    BOOL has_SeTcbPrivilege = FALSE;
    BOOL has_SeAssignPrimaryTokenPrivilege = FALSE;
    char reqPriv1[128] = "SeTcbPrivilege";
    char reqPriv2[128] = "SeAssignPrimaryTokenPrivilege";
    
    for (int privIndex = 0; privIndex < processPriv->PrivilegeCount;
         ++privIndex)
    {
        char privName[128] = { 0 };
        DWORD privNameSize = sizeof(privName);
        char privDisplayName[128] = { 0 };
        DWORD privDisplayNameSize = sizeof(privDisplayName);
        DWORD langID = 0;

        LookupPrivilegeName(0, &processPriv->Privileges[privIndex].Luid,
                            privName, &privNameSize);
            
        if (!lstrcmpi(privName, reqPriv1))
            has_SeTcbPrivilege = TRUE;
        else if (!lstrcmpi(privName, reqPriv2))
            has_SeAssignPrimaryTokenPrivilege = TRUE;
    }
        
    // If WinNT/2000, must have SeTcb and SeAssignPrimaryToken Privileges
    // If WinXP (and above), must have SeAssignPrimaryTokenPrivilege

    if (!has_SeAssignPrimaryTokenPrivilege ||
        ((STAFUtilWin32GetWinType() & kSTAFWinNT2000) && !has_SeTcbPrivilege))
    {
        errorMsg += "\nLogged on user must be an administrator with the "
            "following privilege(s):";

        if (!has_SeTcbPrivilege &&
            (STAFUtilWin32GetWinType() & kSTAFWinNT2000))
        {
            char privDisplayName[128] = { 0 };
            DWORD privDisplayNameSize = sizeof(privDisplayName);
            DWORD langID = 0;
            
            LookupPrivilegeDisplayName(0, reqPriv1, privDisplayName,
                                       &privDisplayNameSize, &langID);

            errorMsg += STAFString("  ") + privDisplayName;
        }

        if (!has_SeAssignPrimaryTokenPrivilege)
        {
            char privDisplayName[128] = { 0 };
            DWORD privDisplayNameSize = sizeof(privDisplayName);
            DWORD langID = 0;
                
            LookupPrivilegeDisplayName(0, reqPriv2, privDisplayName,
                                       &privDisplayNameSize, &langID);

            errorMsg += STAFString("  ") + privDisplayName;
        }
    }
}
