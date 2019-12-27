/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ProcessManager
#define STAF_ProcessManager

#include "STAFOSTypes.h"
#include <list>
#include "STAFError.h"
#include "STAFString.h"
#include "STAFThread.h"
#include "STAFEventSem.h"
#include "STAFMutexSem.h"

class STAFProcessManager
{
public:

    STAFProcessManager();

    // Callback function prototype
    typedef void (*ProcessTermCallback)(STAFProcessID pid, unsigned int retCode);

    // Enumeration for starting in the foreground or background
    enum ForegroundBackground { kBackground = 0, kForeground = 1 };

    // Process initialization information
    struct ProcessInitInfo
    {
        ProcessInitInfo() : foregroundBackground(kBackground), environment(0)
        { /* Do Nothing */ }

        ForegroundBackground foregroundBackground;
        STAFString command;
        STAFString workdir;
        STAFString parms;
        STAFString title;
        char *environment;
    };

    // Starts a process and registers a callback routine that gets invoked
    // when the process ends
    STAFError::ID startProcess(const ProcessInitInfo &processInit,
                               STAFProcessID &pid, unsigned int &osRC,
                               ProcessTermCallback callback);

    // Stops a process that was previously started
    STAFError::ID stopProcess(STAFProcessID pid, unsigned int &osRC);

    // Register for a callback when an arbitrary process ends.
    // Note: The retCode passed to the callback routine will always be 0.
    STAFError::ID registerForProcessTermination(STAFProcessID pid,
                                                unsigned int &osRC,
                                                ProcessTermCallback callback);

    virtual ~STAFProcessManager();

    // Internal processes and typedefs which must be public due to the use
    // of Open Class collections

    struct ProcessMonitorInfo
    {
        ProcessMonitorInfo(STAFProcessHandle aHandle = 0,
                           STAFProcessID aPID = 0,
                           STAFProcessManager::ProcessTermCallback
                               aCallback = 0)
            : handle(aHandle), pid(aPID), callback(aCallback)
        { /* Do Nothing */ }

        STAFProcessHandle handle;
        STAFProcessID pid;
        STAFProcessManager::ProcessTermCallback callback;
    };

    typedef list<ProcessMonitorInfo> ProcessMonitorList;

private:

    // Don't allow copy construction or assignment
    STAFProcessManager(const STAFProcessManager &);
    STAFProcessManager &operator=(const STAFProcessManager &);

    struct QueueData
    {
        USHORT childSessionID;
        USHORT childRC;
    };

    static void callHandleQueue(STAFThreadFunctionData_t thisPtr);
    void handleQueue();
    void processMonitorThread();

    STAFMutexSem fQueueMonitorListSem;
    ProcessMonitorList fQueueMonitorList;
// XXX:    STAFEventSem fMonitorWakeUp;
    HQUEUE fQueueHandle;
};

const STAFProcessID &key(const STAFProcessManager::ProcessMonitorInfo
                         &processMonitorInfo);
#endif
