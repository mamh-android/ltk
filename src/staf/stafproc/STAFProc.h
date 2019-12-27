/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_STAFProc
#define STAF_STAFProc

#include "STAF.h"
#include "STAFRefPtr.h"
#include "STAFString.h"
#include "STAFVariablePool.h"
#include "STAFMutexSem.h"
#include "STAFEventSem.h"
#include "STAFProcess.h"
#include "STAFConnectionProvider.h"
#include <map>

extern STAFHandle_t gSTAFProcHandle;
extern STAFHandlePtr gSTAFProcHandlePtr;
extern STAFProcessID_t gSTAFProcPID;

extern STAFVariablePoolPtr *gGlobalVariablePoolPtr;

extern STAFVariablePoolPtr *gSharedVariablePoolPtr;

class STAFThreadManager;
extern STAFThreadManager *gThreadManagerPtr;

class STAFHandleManager;
extern STAFHandleManager *gHandleManagerPtr;

class STAFTrustManager;
extern STAFTrustManager *gTrustManagerPtr;

class STAFDiagManager;
extern STAFDiagManager *gDiagManagerPtr;

class STAFConnectionManager;
extern STAFConnectionManager *gConnectionManagerPtr;

class STAFServiceManager;
extern STAFServiceManager *gServiceManagerPtr;

class STAFRequestManager;
extern STAFRequestManager *gRequestManagerPtr;

class STAFFSCopyManager;
extern STAFFSCopyManager *gFSCopyManagerPtr;

class STAFNotificationList;
extern STAFNotificationList *gNotifyOnStartPtr;
extern STAFNotificationList *gNotifyOnShutdownPtr;
extern STAFEventSemPtr *gShutdownSemaphorePtr;
extern STAFEventSemPtr *gGCPollingSemPtr;
extern STAFEventSemPtr gGCPollingSem;
extern unsigned int gContinueGCPolling;

typedef std::map<STAFString, STAFString> STAFEnvMap;
extern STAFEnvMap *gEnvMapPtr;
extern unsigned char *gEnvBuffer;
extern int gEnvSize;

extern STAFMutexSem *gDirectorySemPtr;

extern STAFString gVersion;
extern STAFString *gMachineNicknamePtr;
extern STAFString *gMachinePtr;

extern STAFString *gSTAFRootPtr;
extern STAFString gDefaultSTAFWriteLocation;
extern STAFString *gSTAFWriteLocationPtr;
extern STAFString *gSTAFInstanceNamePtr;

extern STAFString *gSTAFTempDirPtr;
extern STAFString *gSTAFInstanceUUIDPtr;
extern unsigned int gResultWarningSize;

extern unsigned short gTCPIPPort;

extern const unsigned int gDefaultConnectionAttempts;
extern unsigned int gConnectionAttempts;

extern const unsigned int gDefaultConnectionRetryDelay;
extern unsigned int gConnectionRetryDelay;

extern STAFConnectionProviderPtr *gLocalConnProvPtr;
extern STAFConnectionProviderPtr *gTCPConnProvPtr;

extern const unsigned int gDefaultNumInitialThreads;
extern unsigned int gNumInitialThreads;

extern unsigned int gMaxFiles;

extern const unsigned int gDefaultMaxQueueSize;
extern unsigned int gMaxQueueSize;

extern const unsigned int gDefaultHandleGCInterval;
extern unsigned int gHandleGCInterval;

extern const unsigned int gDefaultMaxReturnFileSize;
extern unsigned int gMaxReturnFileSize;

extern STAFProcessStopMethod_t gDefaultProcessStopMethod;

extern STAFProcessConsoleMode_t gDefaultDefaultConsoleMode;
extern STAFProcessConsoleMode_t gDefaultConsoleMode;

extern const unsigned int gDefaultStrictFSCopyTrust;
extern unsigned int gStrictFSCopyTrust;

extern const unsigned int gDefaultResultCompatibilityMode;
extern unsigned int gResultCompatibilityMode;

extern STAFString *gLineSeparatorPtr;
extern STAFString *gFileSeparatorPtr;
extern STAFString *gPathSeparatorPtr;
extern STAFString *gCommandSeparatorPtr;

extern const STAFString gSpecSeparator;
extern const STAFString gNoneString;
extern const STAFString gAnonymousString;
extern const STAFString gUnauthenticatedUser;

// The following enum corresponds to entries in the gAPITable defined in
// STAFProc.cpp.

enum STAFAPINumber
{
    kSTAFLocalServiceRequestAPI = 0,
    kSTAFRemoteServiceRequestAPI = 1,
    kSTAFProcessRegistrationAPI = 2,
    kSTAFProcessUnRegistrationAPI = 3,
    kSTAFFileTransferAPI = 4,
    kSTAFFileTransferAPI2 = 5,
    kSTAFDirectoryCopyAPI = 6,
    kSTAFRemoteServiceRequestAPI2 = 7,
    kSTAFHandleTerminationRegistrationAPI = 8,
    kSTAFRemoteHandleTerminatedAPI = 9,
    kSTAFHandleTerminationUnRegistrationAPI = 10
};

// The following enum defines the Compatibility Modes for handling results
// passed back to pre-STAF V3 systems

enum STAFResultCompatibilityMode_e
{
    // Unmarshall result to verbose pretty print format
    kSTAFResultCompatibilityVerbose = 0,

    // Don't change anything in the result
    kSTAFResultCompatibilityNone = 1
};

#endif
