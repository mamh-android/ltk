/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Process
#define STAF_Process

#include "STAFOSTypes.h"
#include "STAFError.h"
#include <map>

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

typedef enum STAFProcessConsoleMode_e
{
    kSTAFProcessNewConsole  = 0,
    kSTAFProcessSameConsole = 1
} STAFProcessConsoleMode_t;

typedef enum STAFProcessConsoleFocus_e
{
    kSTAFProcessBackground = 0,
    kSTAFProcessForeground = 1,
    kSTAFProcessMinimized  = 2
} STAFProcessConsoleFocus_t;

typedef enum STAFProcessAuthenticationMode_e
{
    kSTAFProcessAuthDisabled = 0,
    kSTAFProcessAuthNone     = 1,
    kSTAFProcessAuthWindows  = 2,
    kSTAFProcessAuthPasswd   = 3,
    kSTAFProcessAuthShadow   = 4,
    kSTAFProcessAuthPam      = 5
} STAFProcessAuthenticationMode_t;

typedef enum STAFProcessDisabledAuthAction_e
{
    kSTAFProcessDisabledAuthIgnore = 0,
    kSTAFProcessDisabledAuthError  = 1
} STAFProcessDisabledAuthAction_t;

typedef enum STAFProcessCommandType_e
{
    kSTAFProcessCommand = 0,
    kSTAFProcessShell   = 1
} STAFProcessCommandType_t;

typedef enum STAFProcessRedirectedIOMode_e
{
    kSTAFProcessIONoRedirect  = 0,
    kSTAFProcessIOReadFile    = 1,
    kSTAFProcessIOReplaceFile = 2,
    kSTAFProcessIOAppendFile  = 3,
    kSTAFProcessIOStdout      = 4
} STAFProcessRedirectedIOMode_t;

typedef void (*STAFProcessEndCallback_t)(STAFProcessID_t     pid,
                                         STAFProcessHandle_t processHandle,
                                         unsigned int        returnCode,
                                         void               *data);

struct STAFProcessEndCallbackLevel1
{
    STAFProcessEndCallback_t callback;
    void *data;
};

struct STAFProcessStartInfoLevel1
{
    STAFStringConst_t                command;
    STAFStringConst_t                parms;
    STAFStringConst_t                workdir;
    STAFStringConst_t                title;
    STAFStringConst_t                username;
    STAFStringConst_t                password;
    STAFProcessRedirectedIOMode_t    stdinMode;
    STAFStringConst_t                stdinRedirect;
    STAFProcessRedirectedIOMode_t    stdoutMode;
    STAFStringConst_t                stdoutRedirect;
    STAFProcessRedirectedIOMode_t    stderrMode;
    STAFStringConst_t                stderrRedirect;
    STAFProcessConsoleFocus_t        consoleFocus;
    STAFProcessConsoleMode_t         consoleMode;
    STAFProcessDisabledAuthAction_t  disabledAuthAction;
    STAFProcessAuthenticationMode_t  authMode;
    STAFProcessCommandType_t         commandType;
    STAFStringConst_t                shellCommand;
    STAFStringConst_t                workload;
    char                            *environment;
    unsigned int                     userEnvCount;
    STAFStringConst_t               *userEnvList;
    STAFProcessEndCallbackLevel1    *callback;
};

typedef enum STAFProcessStopMethod_e
{
    kSTAFProcessStopWithSigKill    = 0,
    kSTAFProcessStopWithSigTerm    = 1,
    kSTAFProcessStopWithSigInt     = 2,
    kSTAFProcessStopWithSigKillAll = 3,
    kSTAFProcessStopWithWM_CLOSE   = 4,
    kSTAFProcessStopWithSigTermAll = 5,
    kSTAFProcessStopWithSigIntAll  = 6
} STAFProcessStopMethod_t;

typedef enum STAFProcessStopFlag_e
{
    // kSTAFProcessStopRequest means only processes started by STAF can be
    // stopped (e.g. used by a PROCESS STOP request)
    kSTAFProcessStopRequest = 0,

    // kSTAFProcessKillRequest means any process (not just those started by
    // STAF) can be killed (e.g. used by a PROCESS KILL request).
    kSTAFProcessKillRequest = 1

} STAFProcessStopFlag_t;


struct STAFProcessEnvData
{
    STAFProcessEnvData() { /* Do Nothing */ }

    STAFProcessEnvData(STAFString aEnvName, STAFString aEnvValue)
        : envName(aEnvName), envValue(aEnvValue)
    { /* Do Nothing */ }
 
    STAFString envName;
    STAFString envValue;
};

typedef std::map<STAFString, STAFProcessEnvData> STAFProcessEnvMap;


/****************************************************************************/
/* STAFProcessStart - Starts a process                                      */
/*                                                                          */
/* Accepts: (OUT) Pointer to process identifier                             */
/*          (OUT) Pointer to process handle                                 */
/*          (IN)  Pointer to process start data structure                   */
/*          (IN)  The level of the processs start data structure            */
/*          (IN)  Process termination callback                              */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/*                                                                          */
/* Notes: 1) The process identifier is only guaranteed to be unique while   */
/*           the process is running.  The process handle is guaranteed to   */
/*           be unique so long as the handle is "open".                     */
/****************************************************************************/
STAFRC_t STAFProcessStart(STAFProcessID_t     *processID,
                          STAFProcessHandle_t *processHandle,
                          void                *startData,
                          unsigned int         startDataLevel,
                          unsigned int        *osRC);

STAFRC_t STAFProcessStart2(STAFProcessID_t     *processID,
                           STAFProcessHandle_t *processHandle,
                           void                *startData,
                           unsigned int         startDataLevel,
                           unsigned int        *osRC,
                           STAFString_t        *errorBuffer);


/****************************************************************************/
/* STAFProcessGetHandleFromID - Obtains a process handle given a process ID */
/*                                                                          */
/* Accepts: (IN)  Process identifier                                        */
/*          (OUT) Pointer to process handle                                 */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFProcessGetHandleFromID(STAFProcessID_t      processID,
                                    STAFProcessHandle_t *processHandle,
                                    unsigned int        *osRC);

/****************************************************************************/
/* STAFProcessGetHandleFromID2 - Obtains a process handle given a process ID */
/*                                                                          */
/* Accepts: (IN)  Process identifier                                        */
/*          (OUT) Pointer to process handle                                 */
/*          (IN)  Stop flag (indicates if should "stop" a STAF process or   */
/*                           "kill" any process)                            */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFProcessGetHandleFromID2(STAFProcessID_t       processID,
                                     STAFProcessHandle_t  *processHandle,
                                     STAFProcessStopFlag_t stopFlag,
                                     unsigned int         *osRC);


/****************************************************************************/
/* STAFProcessIsRunning - Determines if a given process is still running    */
/*                                                                          */
/* Accepts: (IN)  Process handle                                            */
/*          (OUT) Pointer to running indicator                              */
/*                (0 = Not running, 1 = Running)                            */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFProcessIsRunning(STAFProcessHandle_t processHandle,
                              unsigned int       *isRunning,
                              unsigned int       *osRC);


/****************************************************************************/
/* STAFProcessStop - Stops a currently running process that was previously  */
/*                   started via STAF                                       */
/*                                                                          */
/* Accepts: (IN)  Process identifier                                        */
/*          (IN)  Stop method                                               */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFProcessStop(STAFProcessID_t         processID,
                         STAFProcessStopMethod_t method,
                         unsigned int           *osRC);

/****************************************************************************/
/* STAFProcessStop2 - Stops a currently running process.                    */
/* Use STAFProcessStop2 if want to kill any running process (not just       */
/* processes started by STAF) by passing stopFlag kSTAFProcessKillRequest.  */
/* Has equivalent function as STAFProcessStop if pass stopFlag              */
/* kSTAFProcessStopRequest.                                                 */
/*                                                                          */
/* Accepts: (IN)  Process identifier                                        */
/*          (IN)  Stop method                                               */
/*          (IN)  Stop flag (indicates if should "stop" a STAF process or   */
/*                           "kill" any process)                            */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFProcessStop2(STAFProcessID_t         processID,
                          STAFProcessStopMethod_t method,
                          STAFProcessStopFlag_t   flag,
                          unsigned int           *osRC);


/****************************************************************************/
/* STAFProcessRegisterEndCallback - Registers for a process ending callback */
/*                                                                          */
/* Accepts: (IN)  Process identifier                                        */
/*          (IN)  Process handle                                            */
/*          (IN)  Callback data                                             */
/*          (IN)  Callback data level                                       */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFProcessRegisterEndCallback(STAFProcessID_t     processID,
                                        STAFProcessHandle_t processHandle,
                                        void               *callback,
                                        unsigned int        callbackLevel);


/****************************************************************************/
/* STAFProcessIsValidAuthMode - Determines if authentication mode is valid  */
/*                                                                          */
/* Accepts: (IN)  Process authentication mode                               */
/*                                                                          */
/* Returns:  kSTAFOk if valid                                               */
/*           other if invalid                                               */
/****************************************************************************/
STAFRC_t STAFProcessIsValidAuthMode(STAFProcessAuthenticationMode_t authMode);


/****************************************************************************/
/* STAFProcessIsValidStopMethod - Determines if stop method is valid        */
/*                                                                          */
/* Accepts: (IN)  Process stop using method                                 */
/*                                                                          */
/* Returns:  kSTAFOk if valid                                               */
/*           other if invalid                                               */
/****************************************************************************/
STAFRC_t STAFProcessIsValidStopMethod(STAFProcessStopMethod_t stopMethod);


/* End C language definitions */

#ifdef __cplusplus
}

// Begin C++ language definitions

// STAFProcess - This class provides a C++ wrapper around the STAF Process
//               C APIs.

class STAFProcess
{
public:

    // Retrieves the per-process-specific handle for a given process ID
    static STAFRC_t getProcessHandleFromID(STAFProcessID_t      processID,
                                           STAFProcessHandle_t &procHandle,
                                           unsigned int        &osRC);

    // Determines if a given process is still running
    static bool isRunning(STAFProcessHandle_t processHandle);

    // Determines if a given process authentication mode is valid
    static STAFRC_t isValidAuthMode(STAFProcessAuthenticationMode_t authMode);
    
    // Determines if a given process stop method is valid
    static STAFRC_t isValidStopMethod(STAFProcessStopMethod_t stopMethod);

    // Register for a callback when an arbitrary process terminates
    static STAFRC_t registerForProcessTermination(
        STAFProcessID_t               pid,
        STAFProcessHandle_t           procHandle,
        STAFProcessEndCallbackLevel1 &callback);

    // Starts a process and registers callback routine that gets
    // invoked when process terminates

    static STAFRC_t startProcess(STAFProcessStartInfoLevel1 &startInfo,
                          STAFProcessID_t &pid,
                          STAFProcessHandle_t &procHandle,
                          unsigned int &osRC);

    // Use startProcess2 instead of startProcess to get more information
    // when an error occurs assigned to the errorBuffer.

    static STAFRC_t startProcess2(STAFProcessStartInfoLevel1 &startInfo,
                          STAFProcessID_t &pid,
                          STAFProcessHandle_t &procHandle,
                          unsigned int &osRC, STAFString &errorBuffer);

    // Stops a process that was previously started by STAF
    static STAFRC_t stopProcess(STAFProcessID_t pid,
                                STAFProcessStopMethod_t stopType,
                                unsigned int &osRC);

    // Use stopProcess2 if want to kill any running process (not just
    // processes started by STAF) by passing stopFlag kSTAFProcessKillRequest.
    // Has equivalent function as stopProcess if pass stopFlag
    // kSTAFProcessStopRequest.
    static STAFRC_t stopProcess2(STAFProcessID_t pid,
                                 STAFProcessStopMethod_t stopType,
                                 STAFProcessStopFlag_t stopFlag,
                                 unsigned int &osRC);

private:

    // Don't allow default construction, copy construction, or assignment
    STAFProcess();
    STAFProcess (const STAFProcess& );
    STAFProcess& operator= (const STAFProcess& );
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFProcessInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
