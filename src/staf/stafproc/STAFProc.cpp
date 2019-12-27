/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAF_exception.h"
#include "STAFString.h"
#include "STAFProc.h"
#include "STAFConfig.h"
#include "STAFRefPtr.h"
#include "STAFSocket.h"
#include "STAFConnectionProvider.h"
#include "STAFSimpleServices.h"
#include "STAFProcessService.h"
#include "STAFHandleService.h"
#include "STAFVariableService.h"
#include "STAFFSService.h"
#include "STAFServiceService.h"
#include "STAFTrustService.h"
#include "STAFDiagService.h"
#include "STAFQueueService.h"
#include "STAFShutdownService.h"
#include "STAFMiscService.h"
#include "STAFTraceService.h"
#include "STAFSemService.h"
#include "STAFHelpService.h"
#include "STAFLifeCycleService.h"
#include "STAFConfigService.h"
#include "STAFEventSem.h"
#include "STAFHandleManager.h"
#include "STAFRequestManager.h"
#include "STAFFSCopyManager.h"
#include "STAFTrustManager.h"
#include "STAFDiagManager.h"
#include "STAFConnectionManager.h"
#include "STAFServiceManager.h"
#include "STAFNotificationList.h"
#include "STAFException.h"
#include "STAFUtil.h"
#include "STAFProcUtil.h"
#include "STAFProcOSUtil.h"
#include "STAFTrace.h"
#include "STAFThreadManager.h"
#include "STAFFileSystem.h"
#include "STAFConverter.h"

//
// Prototypes
//

STAFRC_t HandleRequest(const STAFConnectionProvider *provider,
                       STAFConnectionPtr &connection);
STAFServiceResult submitRemoteRequest(STAFConnectionPtr &connection,
                                      STAFServiceRequest &serviceRequest);
void handleLocalServiceRequestAPI(unsigned int level,
                                  const STAFConnectionProvider *provider,
                                  STAFConnectionPtr &connection,
                                  unsigned int &doShutdown);
void handleRemoteServiceRequestAPI(unsigned int level,
                                   const STAFConnectionProvider *provider,
                                   STAFConnectionPtr &connection,
                                   unsigned int &doShutdown);
void handleRemoteServiceRequestAPI2(unsigned int level,
                                    const STAFConnectionProvider *provider,
                                    STAFConnectionPtr &connection,
                                    unsigned int &doShutdown);
STAFServiceResult submitLocalRequest(STAFConnectionPtr &connection,
                                     STAFServiceRequest &serviceRequest);
void handleProcessRegistrationAPI(unsigned int level,
                                  const STAFConnectionProvider *provider,
                                  STAFConnectionPtr &connection,
                                  unsigned int &);
void handleProcessUnRegistrationAPI(unsigned int level,
                                    const STAFConnectionProvider *provider,
                                    STAFConnectionPtr &connection,
                                    unsigned int &);
void handleFileTransferAPI(unsigned int level,
                           const STAFConnectionProvider *provider,
                           STAFConnectionPtr &connection,
                           unsigned int &);
void handleFileTransferAPI2(unsigned int level,
                            const STAFConnectionProvider *provider,
                            STAFConnectionPtr &connection,
                            unsigned int &);
void handleFileTransfer(unsigned int apiType, unsigned int level,
                        const STAFConnectionProvider *provider,
                        STAFConnectionPtr &connection);
void handleDirectoryCopyAPI(unsigned int level,
                            const STAFConnectionProvider *provider,
                            STAFConnectionPtr &connection,
                            unsigned int &);
void handleHandleTerminationRegistrationAPI(
    unsigned int level, const STAFConnectionProvider *provider,
    STAFConnectionPtr &connection, unsigned int &);

void handleHandleTerminationUnRegistrationAPI(
    unsigned int level, const STAFConnectionProvider *provider,
    STAFConnectionPtr &connection, unsigned int &);

void handleRemoteHandleTerminatedAPI(
    unsigned int level, const STAFConnectionProvider *provider,
    STAFConnectionPtr &connection, unsigned int &);

STAFRC_t writeFile(fstream &outFile, const char *fileBuffer,
                   unsigned int writeLength, const STAFString toFile,
                   const STAFString fromMachine,
                   unsigned int fileLength, unsigned int currentPos);

STAFServiceResult checkTrustForFSCopy(unsigned int apiType, unsigned int level,
                                      const STAFConnectionProvider *provider,
                                      STAFConnectionPtr &connection,
                                      const STAFString &orgMachine,
                                      const STAFString &fromMachine,
                                      unsigned int copyType);
void stafProcTerminate();
unsigned int verifySTAFInstanceName(const STAFString stafInstanceName,
                                    STAFString &errorBuffer);
void replaceStringInBuffer (char* current, const char* searchStr, const char* replaceStr,
              int currentLen, int searchStrLen, int replaceStrLen, int* newLen,
              int* lastReplace, char* newBuffer);
void rollBuffer(char * buffer, int bufferLen, int rollLen);
void updateRemoveDirResultString(STAFString &result, STAFFSEntryPtr &entry,
                                 STAFRC_t rc, unsigned int osRC);
STAFRC_t removeDirChildren(STAFFSEntryPtr entry, const STAFString &namePattern,
                           const STAFString &extPattern,
                           unsigned int entryTypesUInt,
                           STAFFSCaseSensitive_t caseSensitive,
                           STAFString &result);
STAFRC_t removeDir(STAFFSEntryPtr entry, const STAFString &namePattern,
                   const STAFString &extPattern, unsigned int entryTypesUInt,
                   STAFFSCaseSensitive_t caseSensitive, STAFString &result);

// type definitions

typedef void (*STAFAPIHandler)(unsigned int level,
                               const STAFConnectionProvider *provider,
                               STAFConnectionPtr &connection,
                               unsigned int &doShutdown);

const unsigned int kOldAPI = 0;
const unsigned int kNewAPI = 1;

struct STAFAPIDescriptor
{
    unsigned int apiType;       // 0 = old API type, 1 = new API type
    unsigned int minLevel;
    unsigned int maxLevel;
    bool         availLocal;    // Can this API be called locally?
    bool         availRemote;   // Can this API be called from a remote system?
    STAFAPIHandler handler;
};

//
// Global variables
//

// Note: New APIs must have a minimum/maximum level >= 1

// Note: Whenever an entry is added to the gAPITable, add a corresponding entry
// in enum STAFAPINumber in STAFProc.h so that the API number can be referenced

STAFAPIDescriptor gAPITable[] =
{
        // APINum 0
    { kOldAPI, 2, 2, true,  false, handleLocalServiceRequestAPI },
        // APINum 1
    { kOldAPI, 0, 0, false, true,  handleRemoteServiceRequestAPI }, 
        // APINum 2
    { kOldAPI, 0, 0, true,  false, handleProcessRegistrationAPI },
        // APINum 3
    { kOldAPI, 0, 0, true,  false, handleProcessUnRegistrationAPI },
        // APINum 4
    { kOldAPI, 0, 0, true,  true,  handleFileTransferAPI },
        // APINum 5
    { kNewAPI, 1, 3, true,  true,  handleFileTransferAPI2 },
        // APINum 6
    { kNewAPI, 1, 4, true,  true,  handleDirectoryCopyAPI },
        // APINum 7
    { kNewAPI, 2, 2, false, true,  handleRemoteServiceRequestAPI2 },
        // APINum 8
    { kNewAPI, 1, 2, false, true,  handleHandleTerminationRegistrationAPI },
        // APINum 9
    { kNewAPI, 1, 1, false, true,  handleRemoteHandleTerminatedAPI },
        // APINum 10
    { kNewAPI, 1, 1, false, true,  handleHandleTerminationUnRegistrationAPI }
};

unsigned int gMaxAPINumber = (sizeof(gAPITable) / sizeof(STAFAPIDescriptor)) - 1;

STAFString gVersion("3.4.16");
STAFThreadManager gThreadManager(0, 1);
STAFThreadManager *gThreadManagerPtr = &gThreadManager;
STAFHandleManager gHandleManager;
STAFHandleManager *gHandleManagerPtr = &gHandleManager;
STAFRequestManager gRequestManager;
STAFRequestManager *gRequestManagerPtr = &gRequestManager;
STAFFSCopyManager gFSCopyManager;
STAFFSCopyManager *gFSCopyManagerPtr = &gFSCopyManager;
STAFTrustManager gTrustManager(5);
STAFTrustManager *gTrustManagerPtr = &gTrustManager;
STAFConnectionProviderPtr gLocalConnProv;
STAFConnectionProviderPtr *gLocalConnProvPtr = &gLocalConnProv;
STAFConnectionProviderPtr gTCPConnProv;
STAFConnectionProviderPtr *gTCPConnProvPtr = &gTCPConnProv;
STAFConnectionManager gConnectionManager;
STAFConnectionManager *gConnectionManagerPtr = &gConnectionManager;
STAFServiceManager gServiceManager;
STAFServiceManager *gServiceManagerPtr = &gServiceManager;
STAFDiagManager gDiagManager(0);
STAFDiagManager *gDiagManagerPtr = &gDiagManager;

STAFEventSemPtr gShutdownSemaphore;
STAFEventSemPtr *gShutdownSemaphorePtr = &gShutdownSemaphore;

STAFEventSemPtr gGCPollingSem = STAFEventSemPtr(
    new STAFEventSem(), STAFEventSemPtr::INIT);
STAFEventSemPtr *gGCPollingSemPtr = &gGCPollingSem;

unsigned int gContinueGCPolling = 1;

const unsigned int gDefaultHandleGCInterval = 60000;  // 1 minute
unsigned int gHandleGCInterval = gDefaultHandleGCInterval;

STAFProcessID_t gSTAFProcPID = 0;
STAFHandle_t gSTAFProcHandle = 0;
STAFHandlePtr gSTAFProcHandlePtr;
STAFEnvMap gEnvMap;
STAFEnvMap *gEnvMapPtr = &gEnvMap;
unsigned char *gEnvBuffer = 0;
int gEnvSize = 0;

// Constant for debugging memory usage
unsigned int gResultWarningSize = 0;

STAFMutexSem gDirectorySem;
STAFMutexSem *gDirectorySemPtr = &gDirectorySem;
STAFString gSTAFInstanceUUID;
STAFString *gSTAFInstanceUUIDPtr = &gSTAFInstanceUUID;
STAFString gMachineNickname;
STAFString *gMachineNicknamePtr = &gMachineNickname;
STAFString gMachine;
STAFString *gMachinePtr = &gMachine;
STAFString gDefaultAuthenticator;

STAFString gDefaultSTAFWriteLocation;
STAFString gSTAFWriteLocation;
STAFString *gSTAFWriteLocationPtr = &gSTAFWriteLocation;

STAFString gSTAFInstanceName;
STAFString *gSTAFInstanceNamePtr = &gSTAFInstanceName;
STAFString gSTAFTempDir;
STAFString *gSTAFTempDirPtr = &gSTAFTempDir;
STAFVariablePoolPtr gGlobalVariablePool(new STAFVariablePool,
                                        STAFVariablePoolPtr::INIT);
STAFVariablePoolPtr *gGlobalVariablePoolPtr = &gGlobalVariablePool;
STAFVariablePoolPtr gSharedVariablePool(new STAFVariablePool,
                                        STAFVariablePoolPtr::INIT);
STAFVariablePoolPtr *gSharedVariablePoolPtr = &gSharedVariablePool;
unsigned short gTCPIPPort = 6500;

const unsigned gDefaultConnectionAttempts = 2;
unsigned int gConnectionAttempts = gDefaultConnectionAttempts;

const unsigned int gDefaultMaxQueueSize = 100;
unsigned int gMaxQueueSize = gDefaultMaxQueueSize;

const unsigned int gDefaultConnectionRetryDelay = 1000;
unsigned int gConnectionRetryDelay = gDefaultConnectionRetryDelay;

const unsigned int gDefaultNumInitialThreads = 10;
unsigned int gNumInitialThreads = gDefaultNumInitialThreads;

// gDefaultDefaultConsoleMode will always contain the default value for the
// operating system (used by the CONFIG service)
STAFProcessConsoleMode_t gDefaultDefaultConsoleMode = kSTAFProcessNewConsole;

// gDefaultConsoleMode is initially set to the default value for the
// operating system but can be changed via SET DEFAULTNEWCONSOLE or
// SET DEFAULTSAMECONSOLE in the STAF.cfg file or dynamically via a
// PROCESS SET NEWCONSOLE or PROCESS SET SAMECONSOLE request
STAFProcessConsoleMode_t gDefaultConsoleMode = kSTAFProcessNewConsole;

STAFProcessStopMethod_t gDefaultProcessStopMethod =
    kSTAFProcessStopWithSigKillAll;

// Set default Result Compatibility Mode
const unsigned int gDefaultResultCompatibilityMode =
    kSTAFResultCompatibilityVerbose;
unsigned int gResultCompatibilityMode = gDefaultResultCompatibilityMode;

// Disable strict FS Copy Trust by default
const unsigned int gDefaultStrictFSCopyTrust = 0;
unsigned int gStrictFSCopyTrust = gDefaultStrictFSCopyTrust;

unsigned int gMaxFiles = 500;

const unsigned int gDefaultMaxReturnFileSize = 0;
unsigned int gMaxReturnFileSize = gDefaultMaxReturnFileSize;

STAFNotificationList gNotifyOnStart;
STAFNotificationList *gNotifyOnStartPtr = &gNotifyOnStart;
STAFNotificationList gNotifyOnShutdown;
STAFNotificationList *gNotifyOnShutdownPtr = &gNotifyOnShutdown;
STAFEventSem gInterfaceSem;
STAFString gSTAFRoot;
STAFString *gSTAFRootPtr = &gSTAFRoot;
STAFString gLineSeparator;
STAFString *gLineSeparatorPtr = &gLineSeparator;
STAFString gFileSeparator;
STAFString *gFileSeparatorPtr = &gFileSeparator;
STAFString gPathSeparator;
STAFString *gPathSeparatorPtr = &gPathSeparator;
STAFString gCommandSeparator;
STAFString *gCommandSeparatorPtr = &gCommandSeparator;

// Separator for interface and system identifier and for authenticator and
// user identifier specifications:
//   [<Interface>://]<System Identifier>
//   [<Authenticator>://]<User Identifier>
const STAFString gSpecSeparator(STAFString(kUTF8_COLON) + 
                                STAFString(kUTF8_SLASH) +
                                STAFString(kUTF8_SLASH));

// Handles which are un-authenticated are assigned authenticator "none"
// and user identifier "anonymous".

const STAFString gNoneString("none");
const STAFString gAnonymousString("anonymous");
const STAFString gUnauthenticatedUser(gNoneString + gSpecSeparator +
                                      gAnonymousString);

const STAFString sQueueHandle("QUEUE HANDLE ");
const STAFString sMessage(" MESSAGE ");
const STAFString sType(" TYPE ");
const STAFString sRequestComplete("STAF/RequestComplete ");
const STAFString sSemiColon(";");
const STAFString sColon(":");
const STAFString sLocal("LOCAL");
const STAFString sLowerLocal("local");
const STAFString sLocalLong("local://local");
const STAFString sQueue("QUEUE");
const STAFString sLifeCycle("LIFECYCLE");
const STAFString sProcess("PROCESS");

int main(int argc, char **argv, char **envp)
{
#ifdef STAF_OS_NAME_ZOS
    extern char **environ;
    envp = environ;
#endif

    // Register our termination handler.  It will simply tell us if we have
    // ended abruptly due to an uncaught or unexpected exception.

    STAF_set_terminate(stafProcTerminate);

    gSTAFProcPID = STAFUtilGetPID();

    // Set STAF instance name
    
    gSTAFInstanceName = "STAF";

    if (getenv("STAF_INSTANCE_NAME") != NULL)
    {
        gSTAFInstanceName = getenv("STAF_INSTANCE_NAME");
        STAFString errorBuffer = "";

        if (verifySTAFInstanceName(gSTAFInstanceName, errorBuffer) != 0)
        {
            cout << "Invalid value for STAF_INSTANCE_NAME ("
                 << gSTAFInstanceName << ")" << endl << errorBuffer << endl;
            return 1;
        }
    }

    // Initialize Platform specific code

    STAFString errorBuffer = "";
    unsigned int osRC = 0;
    int stafOSInitRC = STAFProcOSInit(errorBuffer, osRC);

    if (stafOSInitRC != 0)
    {
        cout << "Error during platform-specific initialization"  << endl;
        
        if (osRC != 0)
            cout << errorBuffer << ", OS RC: " << osRC << endl;
        else
            cout << errorBuffer << endl;

        return 1;
    }

    // Default tracing to standard output

    STAFTrace::setTraceDestination(kSTAFTraceToStdout);

    // Initialize the global variable pool
    //
    // Note: The variable "Machine" is set after the config is read

    STAFString configHead("STAF/Config/");
    STAFConfigInfo configInfo;
    STAFString_t errorBufferT;

    STAFRC_t rc = STAFUtilGetConfigInfo(&configInfo, &errorBufferT, &osRC);

    if (rc != kSTAFOk)
    {
        cout << STAFString(errorBufferT, STAFString::kShallow);
        cout << ", STAF RC: " << rc << ", OS RC: " << osRC << endl;
        STAFProcOSTerm(errorBuffer, osRC);
        return 1;
    }

    gGlobalVariablePool->set("STAF/Version", gVersion);
    gGlobalVariablePool->set(configHead + "InstanceName", gSTAFInstanceName);
    gGlobalVariablePool->set(configHead + "BootDrive", configInfo.bootDrive);
    gGlobalVariablePool->set(configHead + "OS/Name", configInfo.osName);
    gGlobalVariablePool->set(configHead + "OS/MajorVersion",
                             configInfo.osMajorVersion);
    gGlobalVariablePool->set(configHead + "OS/MinorVersion",
                             configInfo.osMinorVersion);
    gGlobalVariablePool->set(configHead + "OS/Revision",
                             configInfo.osRevision);
    gGlobalVariablePool->set(configHead + "Mem/Physical/Bytes",
                             STAFString(configInfo.physicalMemory));
    gGlobalVariablePool->set(configHead + "Mem/Physical/KB",
                             STAFString(configInfo.physicalMemory / 1024));
    gGlobalVariablePool->set(configHead + "Mem/Physical/MB",
                             STAFString(configInfo.physicalMemory /
                                           (1024 * 1024)));
    gGlobalVariablePool->set(configHead + "Processor/NumAvail",
                             STAFString(configInfo.numProcessors));
    gGlobalVariablePool->set(configHead + "STAFRoot", configInfo.exePath);
    gGlobalVariablePool->set(configHead + "Sep/Line",
                             configInfo.lineSeparator);
    gGlobalVariablePool->set(configHead + "Sep/File",
                             configInfo.fileSeparator);
    gGlobalVariablePool->set(configHead + "Sep/Path",
                             configInfo.pathSeparator);
    gGlobalVariablePool->set(configHead + "Sep/Command",
                             configInfo.commandSeparator);
    gGlobalVariablePool->set(configHead + "CodePage",
                             STAFConverter::determineCodePage());

    gSTAFRoot = configInfo.exePath;
    gLineSeparator = configInfo.lineSeparator;
    gFileSeparator = configInfo.fileSeparator;
    gPathSeparator = configInfo.pathSeparator;
    gCommandSeparator = configInfo.commandSeparator;
    gDefaultDefaultConsoleMode = configInfo.defaultProcessConsoleMode;
    gDefaultConsoleMode = configInfo.defaultProcessConsoleMode;
    gDefaultProcessStopMethod = configInfo.defaultProcessStopMethod;

    // Set the default writeable STAF location (if not changed by the DATADIR
    // operational parameter.  The default is:
    //   {STAF/Config/STAFRoot}/data/{STAF/Config/InstanceName}

    STAFFSPath writeLocation;

    writeLocation.setRoot(configInfo.exePath);
    writeLocation.addDir("data");
    writeLocation.addDir(gSTAFInstanceName);
    gSTAFWriteLocation = writeLocation.asString();
    gDefaultSTAFWriteLocation = gSTAFWriteLocation;
    gGlobalVariablePool->set("STAF/DataDir", gSTAFWriteLocation);

    // Set the config file to use

    STAFString configFile;

    if (argc == 2)
    {
        configFile = argv[1];
    }
    else if (argc > 2)
    {
        cout << "Usage: STAFProc [Configuration file]" << endl;
        STAFProcOSTerm(errorBuffer, osRC);
        return 1;
    }
    else
    {
        configFile = STAFString(configInfo.exePath) +
                     STAFString(configInfo.fileSeparator) + "bin" +
                     STAFString(configInfo.fileSeparator) + "STAF.cfg";
    }

    // Set environment buffer for use by Process service

    STAFString envHead("STAF/Env/");

    for (int envi = 0; envp[envi] != 0; ++envi)
    {
        STAFString envVar(envp[envi]);
        unsigned int equalPos = envVar.find(kUTF8_EQUAL);
        STAFString envName(envVar.subString(0, equalPos));
        STAFString envValue;

        if (equalPos != STAFString::kNPos)
            envValue = envVar.subString(equalPos + 1);

        gEnvMap[envName] = envValue;
        gGlobalVariablePool->set(envHead + envName, envValue);
    }

    int envc = 0;

    for(; envp[envc] != 0; ++envc)
        gEnvSize += strlen(envp[envc]) + 1;

    gEnvBuffer = new unsigned char[++gEnvSize];
    memset(gEnvBuffer, 0, gEnvSize);

    for(gEnvSize = 0, envc = 0; envp[envc] != 0; ++envc)
    {
        int length = strlen(envp[envc]);
        memcpy(gEnvBuffer + gEnvSize, envp[envc], length);
        gEnvSize += length + 1;
    }

    STAFRC_t regRC = gHandleManager.registerHandle(
                          gSTAFProcHandle, gSTAFProcPID, "STAF_Process");
    if (regRC)
    {
        cout << "Error registering STAF Process handle, RC: " << regRC << endl;
        STAFProcOSTerm(errorBuffer, osRC);
        return 1;
    }

    STAFRC_t createRC = STAFHandle::create(gSTAFProcHandle, gSTAFProcHandlePtr);

    if (createRC)
    {
        cout << "Error creating STAF Process handle, RC: " << createRC << endl;
        STAFProcOSTerm(errorBuffer, osRC);
        return 1;
    }

    try
    {
        // Add all the internal services

        gServiceManager.add(STAFServicePtr(new STAFDelayService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFDiagService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFEchoService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFHelpService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFFSService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFHandleService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFMiscService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFPingService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFProcessService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFQueueService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFSemService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFServiceService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFShutdownService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFTraceService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFTrustService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFVariableService(),
                                           STAFServicePtr::INIT));
        gServiceManager.add(STAFServicePtr(new STAFConfigService(),
                                           STAFServicePtr::INIT));

        // Set to the number of internal services added above, so later we can
        // know when we reach the last internal service in the list, before
        // external services in the list.
        unsigned int numInternalServices = 16;
        
        // Add local interface

        STAFRC_t localInterfaceRC =
            gConnectionManager.addConnectionProvider(
                "local", "STAFLIPC",
                STAFConnectionManager::ConnectionProviderOptionList(),
                errorBuffer);

        if (localInterfaceRC != kSTAFOk)
        {
            cout << "Error creating local interface" << endl;
            cout << "Error code: " << localInterfaceRC << endl;
            cout << "Reason    : " << errorBuffer << endl;

            STAFProcOSTerm(errorBuffer, osRC);
            return 1;
        }

        // Process the config file

        gGlobalVariablePool->set(configHead + "ConfigFile", configFile);

        unsigned int configRC = readConfigFile(configFile);

        if (configRC != 0)
        {                    
            cout << endl << "Error in configuration file: "
                 << configFile << endl;
            STAFProcOSTerm(errorBuffer, osRC);
            return 1;
        }
        
        // Set default interface (aka Connection Provider) variable after
        // all interfaces, including the local interface, have been added
        
        STAFString defaultInterface = 
            gConnectionManager.getDefaultConnectionProvider();

        gGlobalVariablePool->set(
            configHead + "DefaultInterface", defaultInterface);

        // Set default authenticator variable after all authenticators
        // have been added

        gDefaultAuthenticator = gServiceManagerPtr->getDefaultAuthenticator();
        gGlobalVariablePool->set(configHead + "DefaultAuthenticator",
                                 gDefaultAuthenticator);

        // Obtaining the lock on data directory

        osRC = 0;
        errorBuffer = "";
        int stafOSLockDataDirRC = STAFProcOSLockDataDir(errorBuffer, osRC);

        if (stafOSLockDataDirRC != 0)
        {
            if (osRC != 0)
                cout << errorBuffer << ", osRC: " << osRC << endl;
            else
                cout << errorBuffer << endl;

            STAFProcOSTerm(errorBuffer, osRC);
            return 1;
        }

        // Set initial number of threads
        gThreadManagerPtr->growThreadPool(gNumInitialThreads);

        // Make performance adjustments

        STAFPerfInfo perfInfo = { gMaxFiles };

        if (makePerformanceAdjustments(perfInfo, errorBuffer, osRC) != 0)
        {
            cout << errorBuffer << ", RC:" << osRC << endl;
            STAFProcOSTerm(errorBuffer, osRC);
            return 1;
        }

        gShutdownSemaphore = STAFEventSemPtr(new STAFEventSem(),
                                             STAFEventSemPtr::INIT);

        gGCPollingSem->reset();

        // Start interfaces

        STAFString logicalInterfaceID = STAFString();
        STAFString physicalInterfaceID = STAFString();

        STAFConnectionManager::ConnectionProviderList connProvList =
            gConnectionManager.getConnectionProviderListCopy();

        for (STAFConnectionManager::ConnectionProviderList::iterator iter =
                 connProvList.begin();
             iter != connProvList.end();
             ++iter)
        {
            try
            {
                (*iter)->start(HandleRequest);
            }
            catch (STAFException &se)
            {
                cout << "Error starting " << (*iter)->getName() << " interface"
                     << endl;
                cout << "Error code: " << se.getErrorCode() << endl;
                cout << "Reason    : " << se.getText() << endl;

                STAFProcOSTerm(errorBuffer, osRC);
                return 1;
            }

            if ((*iter)->getName() == defaultInterface)
            {
                (*iter)->getMyNetworkIDs(logicalInterfaceID,
                                         physicalInterfaceID);
            }
        }

        // Get rid of connection provider references
        connProvList = STAFConnectionManager::ConnectionProviderList();

        // Determine the machine name
            
        gMachine = logicalInterfaceID;

        // If SET MACHINE not specified in STAF Configuration file, assign
        // machine nickname based on the default interface's logical ID.
        
        if (gMachineNickname == STAFString())
        {
            gMachineNickname = logicalInterfaceID;
        
            if (gMachineNickname == STAFString())
            {
                cout << "Unknown machine nickname" << endl;
                STAFProcOSTerm(errorBuffer, osRC);
                return 1;
            }
        }

        // Set the Machine and MachineNickname variables

        gGlobalVariablePool->set(configHead + "Machine", gMachine);
        gGlobalVariablePool->set(configHead + "MachineNickname",
                                 gMachineNickname);

        // Determine startup time

        STAFTimestamp startupTime = STAFTimestamp::now();
        gGlobalVariablePool->set(configHead + "StartupTime",
                                 startupTime.asString());

        // Calculate STAF Instance UUID
        //
        // This is a 128-bit/16-byte value.  The first four bytes are computed
        // from the current timestamp.  The second four bytes are computed from
        // the PID.  The third four bytes are computed from the physical
        // interface ID.  The fourth four bytes are computed from the logical
        // interface ID.
        //
        // The 128-bit value is turned into a 32 character hexadecimal string.

        // We are going to use the last four significant numbers of the
        // physical address, going on the assumption we are dealing with
        // TCP/IP.  If we aren't using TCP/IP, then our UUID might not be
        // as unique.
        // - If the physical address is an IPv4 address, use a Period as the
        //   separator and base 10.
        // - If the physical address is an IPv6 address, use a Colon as the
        //   separator and base 16.
        

        STAFString physicalID = physicalInterfaceID;
        std::deque<unsigned char> physicalIDValues;

        STAFString physicalIDSeparator = STAFString(kUTF8_PERIOD);
        int base = 10;
        
        if (physicalID.find(kUTF8_COLON) != STAFString::kNPos)
        {
            physicalIDSeparator = STAFString(kUTF8_COLON);
            base = 16;
        }

        unsigned int separatorPos = 0;
        while ((separatorPos = physicalID.find(physicalIDSeparator)) !=
               STAFString::kNPos)
        {
            STAFString thisValueString = physicalID.subString(0, separatorPos);

            physicalID = physicalID.subString(separatorPos + 1);

            physicalIDValues.push_back(
                thisValueString.asUIntWithDefault(0, base) & 0xFF);
        }

        physicalIDValues.push_back(
            physicalID.asUIntWithDefault(0, base) & 0xFF);

        unsigned char UUIDBytes[16];

        *(reinterpret_cast<unsigned int *>(UUIDBytes)) =
            STAFTimestamp::now().getImpl();
        *(reinterpret_cast<unsigned int *>(UUIDBytes + 4)) = STAFUtilGetPID();

        for (unsigned int physicalIDValuesIndex = 0;
             (physicalIDValuesIndex < 4) && (physicalIDValues.size() != 0);
             ++physicalIDValuesIndex)
        {
            UUIDBytes[8 + physicalIDValuesIndex] = physicalIDValues.front();

            physicalIDValues.pop_front();
        }

        STAFString logicalID = logicalInterfaceID.subString(0,
                                   logicalInterfaceID.find(kUTF8_PERIOD));
        unsigned int numLogicalBytes =
            (logicalID.length() < 4) ? logicalID.length() : 4;
        unsigned int logicalStartIndex =
            logicalID.length() - numLogicalBytes;

        for (unsigned int logicalIndex = 0; logicalIndex < numLogicalBytes;
            ++logicalIndex)
        {
            UUIDBytes[12 + logicalIndex] =
                logicalID.buffer()[logicalStartIndex + logicalIndex];
        }

        for (unsigned int UUIDByteIndex = 0; UUIDByteIndex < sizeof(UUIDBytes);
             ++UUIDByteIndex)
        {
            unsigned int aUUIDByte = UUIDBytes[UUIDByteIndex];
            unsigned int upperValue = (aUUIDByte >> 4) & 0x0000000F;
            unsigned int lowerValue = aUUIDByte & 0x0000000F;

            gSTAFInstanceUUID += STAFString(upperValue, 16);
            gSTAFInstanceUUID += STAFString(lowerValue, 16);
        }

        // Display STAFProc startup information

        cout << endl << "Machine          : " << gMachine << endl;
        cout << "Machine nickname : " << gMachineNickname << endl;
        cout << "Startup time     : " << startupTime.asString() << endl;

        // Check if STAF_RESULT_WARNING_SIZE environment variable is set.
        // If it is, that indicates to log a trace warning message the
        // length of the result from a service request is greater than the
        // size specified by this environment variable.
        // The warning size is specified in Megabytes and must be at least 1.

        if (getenv("STAF_RESULT_WARNING_SIZE") != NULL)
        {
            STAFString warningSizeString = getenv("STAF_RESULT_WARNING_SIZE");
            STAFString errorBuffer;
            unsigned int warningSize;

            // Max warning size is UINT_MAX divided by 1,048,476 which is the
            // number of bytes in a megabyte

            unsigned int maxWarningSize = UINT_MAX / 1048576;

            STAFRC_t rc = convertStringToUInt(
                warningSizeString, "STAF_RESULT_WARNING_SIZE", warningSize,
                errorBuffer, 1, maxWarningSize);

            if (rc == kSTAFOk)
            {
                gResultWarningSize = warningSize * 1048576;
            }
            else
            {
                cout << endl
                     << "WARNING: Ignoring the STAF_RESULT_WARNING_SIZE "
                     << "environment variable because its value is not valid."
                     << "  It must be an unsigned integer in range 1 to "
                     << maxWarningSize << " which represents a size in "
                     << "megabytes.  Invalid value: " << warningSizeString
                     << endl;
            }
        }

        // Create gSTAFWriteLocation directory if it doesn't exist 
        // Note that if the default location was overridden, it will have
        // already been created.

        STAFFSPath dataPath;
        dataPath.setRoot(gSTAFWriteLocation);
        
        try
        {
            if (!dataPath.exists())
            {
                try
                {
                    dataPath.createDirectory(kSTAFFSCreatePath);
                }
                catch (...)
                { /* Do Nothing */ }

                if (!dataPath.exists())
                {
                    cout << "Error creating DATADIR directory: "
                         << gSTAFWriteLocation << endl;
                    STAFProcOSTerm(errorBuffer, osRC);
                    return 1;
                }
            }
        }
        catch (...)
        {
            cout << "Error checking if DATADIR directory "
                 << gSTAFWriteLocation << " exists" << endl;
            STAFProcOSTerm(errorBuffer, osRC);
            return 1;
        }

        // Delete <gSTAFWriteLocation>/tmp directory and all its contents
        // if it exists and then create the directory
        
        STAFFSPath tmpPath;
        tmpPath.setRoot(gSTAFWriteLocation);
        tmpPath.addDir("tmp");
        
        try
        {
            if (tmpPath.exists())
            {
                STAFFSEntryPtr entry = tmpPath.getEntry();
                STAFString namePattern(kUTF8_STAR);
                STAFString extPattern(kUTF8_STAR);
                unsigned int entryTypesUInt = kSTAFFSAll;
                STAFFSCaseSensitive_t caseSensitive = kSTAFFSCaseDefault;
                STAFString removeResult;

                STAFRC_t removeRC = removeDir(
                    entry, namePattern, extPattern, entryTypesUInt,
                    caseSensitive, removeResult);

                if (removeRC != kSTAFOk)
                {
                    cout << "Error deleting temp directory: "
                         << tmpPath.asString()
                         << ", RC: " << removeRC << ", Result: "
                         << removeResult << endl;
                }
            }

            try
            {
                // Don't want exceptions here
                STAFFSEntryPtr tmpdir = 
                    tmpPath.createDirectory(kSTAFFSCreatePath);
            }
            catch (...)
            { /* Do Nothing */ }

            if (!tmpPath.exists())
            {
                cout << "Error creating temp directory: "
                     << tmpPath.asString() << endl;
            }
        }
        catch (...)
        {
            cout << "Error checking existance of temp directory "
                 << tmpPath.asString() << endl;
        }

        // Create <gSTAFWriteLocation>/user directory if it doesn't exist.

        STAFFSPath userPath;
        userPath.setRoot(gSTAFWriteLocation);
        userPath.addDir("user");
        
        try
        {
            if (!userPath.exists())
            {
                try
                {
                    userPath.createDirectory(kSTAFFSCreatePath);
                }
                catch (...)
                { /* Do Nothing */ }

                if (!userPath.exists())
                {
                    cout << "Error creating user directory: "
                         << userPath.asString() << endl;
                }
            }
        }
        catch (...)
        {
            cout << "Error checking existance of user directory "
                 << userPath.asString() << endl;
        }
        
        // Initialize the internal LifeCycle service after the STAF Config
        // file has been processed (so that the DataDir operational parameter
        // was used in setting gSTAFWriteLocation, if applicable) and after
        // this writeable data directory has been created

        gServiceManager.add(STAFServicePtr(new STAFLifeCycleService(),
                                           STAFServicePtr::INIT));

        // Initialize the internal services (except for the LifeCycle service).
        // The internal services (except for the LifeCycle service) are first
        // in the services list, so stop before we reach the external services
        // in the list as they will be initialized later.

        STAFServiceManager::ServiceList serviceList =
            gServiceManager.getServiceListCopy();
        STAFServiceManager::ServiceList::iterator serviceIter =
            serviceList.begin();
        
        for (unsigned int i = 0; serviceIter != serviceList.end() &&
             i < numInternalServices; ++serviceIter, ++i)
        {
            STAFServiceResult result = (*serviceIter)->initialize();

            if (result.fRC)
            {
                cout << "Error initializing service, "
                     << (*serviceIter)->name() 
                     << ", RC: " << result.fRC 
                     << ", Result: " << result.fResult 
                     << endl;

                gServiceManager.remove((*serviceIter)->name());
            }
        }
        
        // Initialize the service loader services

        STAFServiceManager::ServiceList slsList =
            gServiceManager.getSLSListCopy();

        for (STAFServiceManager::ServiceList::iterator slsIter =
             slsList.begin(); slsIter != slsList.end(); ++slsIter)
        {
            STAFServiceResult result = (*slsIter)->initialize();

            if (result.fRC)
            {
                cout << "Error initializing service loader service, "
                     << (*slsIter)->name() 
                     << ", RC: " << result.fRC 
                     << ", Result: " << result.fResult
                     << endl;

                gServiceManager.removeSLS(*slsIter);
            }

        }

        // Don't hold on to service pointers we will never use
        slsList = STAFServiceManager::ServiceList();

        // Initialize the Authenticator services

        STAFServiceManager::OrderedServiceList authenticatorMap =
            gServiceManager.getAuthenticatorMapCopy();

        for (STAFServiceManager::OrderedServiceList::iterator authIter =
             authenticatorMap.begin(); authIter != authenticatorMap.end();
             ++authIter)
        {
            STAFServiceResult result = authIter->second->initialize();

            if (result.fRC)
            {
                cout << "Error initializing authenticator service, "
                     << authIter->second->name() 
                     << ", RC: " << result.fRC 
                     << ", Result: " << result.fResult
                     << endl;

                gServiceManager.removeAuthenticator(authIter->second->name());
            }
        }

        // Don't hold on to service pointers we will never use
        authenticatorMap = STAFServiceManager::OrderedServiceList();

        // Initialize the external services and the LifeCycle service.
        // Note that the internal services were already initialized.

        serviceList = gServiceManager.getServiceListCopy();
        serviceIter = serviceList.begin();
        
        for (unsigned int j = 0; serviceIter != serviceList.end();
             ++serviceIter, ++j)
        {
            if (j < numInternalServices)
            {
                // Skip the internal services in the list which have already
                // been initialized
                continue;
            }

            STAFServiceResult result = (*serviceIter)->initialize();

            if (result.fRC)
            {
                cout << "Error initializing service, "
                     << (*serviceIter)->name() 
                     << ", RC: " << result.fRC 
                     << ", Result: " << result.fResult 
                     << endl;

                gServiceManager.remove((*serviceIter)->name());
            }
        }

        // Don't hold on to service pointers we will never use
        serviceList = STAFServiceManager::ServiceList();

        cout << endl << "STAFProc version " << gVersion << " initialized"
             << endl;

        // Call the STAF registration program if not already run

        STAFString infFileName = STAFString(configInfo.exePath) +
            gFileSeparator + "STAFReg.inf";

        STAFString cmpFileName = gSTAFWriteLocation + *gFileSeparatorPtr +
            "register" + *gFileSeparatorPtr + "STAFReg.cmp";

        try
        {
            if (STAFFSPath(infFileName).exists() &&
                !STAFFSPath(cmpFileName).exists())
            {
                // Run the STAF Registration program by submitting a PROCESS
                // START request and not waiting for it to complete (so that
                // it runs a separate thread).  This way, if a SHUTDOWN
                // request is submitted before the STAFReg program is
                // complete, then STAF can shut down without waiting for
                // STAFReg to complete.  Note that STAFReg can take 20 seconds
                // or more to run if it fails to connect to the REGISTER
                // service on the IBM internal registration machine due to
                // machines that don't have access to the IBM internal
                // registration machine because they are outside the IBM
                // firewall, have BSO issues, or are on a isolated network,
                // etc.)
                
                STAFResultPtr result = gSTAFProcHandlePtr->submit(
                    sLocal, sProcess,
                    "START COMMAND {STAF/Config/STAFRoot}/bin/STAFReg"
                    " SAMECONSOLE");
            }
        }
        catch (...)
        { /* Do Nothing */}
        
        // Send start notifications

        gNotifyOnStart.sendNotification("STAF/Start", "");

        // Trigger the Startup registrations for the LifeCycle service
        
        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            sLocal, sLifeCycle, "TRIGGER PHASE Startup CONFIRM");

        // Wait for signal to end

        gShutdownSemaphore->wait();

        cout << "STAFProc ending normally" << endl;
        
        gContinueGCPolling = 0;
        gGCPollingSem->post();
        
        // Allow the polling thread to end
        gThreadManagerPtr->sleepCurrentThread(1000);

        // Send shutdown notifications

        gNotifyOnShutdown.sendNotification("STAF/Shutdown", "");
        
        // Trigger the Shutdown registrations for the LifeCycle service

        result = gSTAFProcHandlePtr->submit(
            sLocal, sLifeCycle, "TRIGGER PHASE Shutdown CONFIRM");

        // Terminate external services

        serviceList = gServiceManager.getServiceListCopy();
        unsigned int numExternalServices = serviceList.size() -
            numInternalServices;
        unsigned int k = 0;
        
        for (STAFServiceManager::ServiceList::reverse_iterator serviceIter2 =
             serviceList.rbegin(); serviceIter2 != serviceList.rend() &&
             k < numExternalServices; ++serviceIter2, ++k)
        {
            STAFServicePtr theService = *serviceIter2;

            gServiceManager.remove(theService->name());
            
            STAFServiceResult result = theService->terminate();

            if (result.fRC)
            {
                STAFString message("Error terminating service, ");
                message += theService->name() + ", RC: " +
                    STAFString(result.fRC) + ", Result: " + result.fResult;
                STAFTrace::trace(kSTAFTraceError, message);
            }
        }

        // Terminate authenticator services

        authenticatorMap = gServiceManager.getAuthenticatorMapCopy();

        for(STAFServiceManager::OrderedServiceList::reverse_iterator
            authIter2 = authenticatorMap.rbegin();
            authIter2 != authenticatorMap.rend(); ++authIter2)
        {
            STAFServicePtr theService = authIter2->second;

            gServiceManager.removeAuthenticator(theService->name());
            
            STAFServiceResult result = theService->terminate();

            if (result.fRC)
            {
                STAFString message(
                    "Error terminating authenticator service, ");
                message += theService->name() + ", RC: " +
                    STAFString(result.fRC) + ", Result: " + result.fResult;
                STAFTrace::trace(kSTAFTraceError, message);
            }
        }

        // Terminate service loader services

        slsList = gServiceManager.getSLSListCopy();

        for (STAFServiceManager::ServiceList::reverse_iterator slsIter2 =
             slsList.rbegin(); slsIter2 != slsList.rend(); ++slsIter2)
        {
            STAFServicePtr theService = *slsIter2;

            gServiceManager.removeSLS(theService);
            
            STAFServiceResult result = theService->terminate();

            if (result.fRC)
            {
                STAFString message(
                    "Error terminating service loader service, ");
                message += theService->name() + ", RC: " +
                    STAFString(result.fRC) + ", Result: " + result.fResult;
                STAFTrace::trace(kSTAFTraceError, message);
            }
        }

        // Terminate any remaining services that might have been re-initialized
        // by a service loader during another service's termination and
        // terminate the internal services

        serviceList = gServiceManager.getServiceListCopy();

        for (STAFServiceManager::ServiceList::reverse_iterator serviceIter3 =
             serviceList.rbegin(); serviceIter3 != serviceList.rend();
             ++serviceIter3)
        {
            STAFServicePtr theService = *serviceIter3;

            gServiceManager.remove(theService->name());

            STAFServiceResult result = theService->terminate();

            if (result.fRC)
            {
                STAFString message("Error terminating service, ");
                message += theService->name() + ", RC: " +
                    STAFString(result.fRC) + ", Result: " + result.fResult;
                STAFTrace::trace(kSTAFTraceError, message);
            }
        }

        // Stop the connection providers
        
        connProvList = gConnectionManager.getConnectionProviderListCopy();
        
        for (STAFConnectionManager::ConnectionProviderList::iterator
             cpIter = connProvList.begin(); cpIter != connProvList.end();
             ++cpIter)
        {
            try
            {
                // XXX: Cannot stop the local connection provider because this
                // hangs the shutdown of STAFProc when exiting the outer try
                // block.  It's hanging in the local connection provider's
                // STAFConnectionProviderConnect method when it calls
                // ConnectToPipe() (on the WaitForMutipleObjects() call within
                // the ConnectToPipe method).  It actually should not have
                // gotten that far as it should have failed in GetPipeData()
                // issued before ConnectToPipe() in the local connection
                // provider's STAFConnectionProviderConnect method.
                // Probably, it's due to a file mapping (that we aren't saving
                // a reference to) not getting closed.

                if ((*cpIter)->getName() != "local")
                    (*cpIter)->stop();
            }
            catch (STAFException &se)
            {
                STAFString message = STAFString("Error stopping ") +
                    (*cpIter)->getName() + " interface.  Error code: " +
                    se.getErrorCode() + " Reason: " + se.getText();
                STAFTrace::trace(kSTAFTraceError, message);
            }
        }

        // Get rid of connection provider references
        connProvList = STAFConnectionManager::ConnectionProviderList();
        
        // XXX: Without this sleep, the system doesn't get time to cleanup
        // everything before the main thread ends, resulting in an extra
        // message stating that terminate() was called.  I should probably
        // try to debug this further, but this masks the problem sufficiently
        // at the moment.

        gThreadManagerPtr->sleepCurrentThread(1000);
        
        // Clean up any OS specific stuff.

        int stafOSTermRC = STAFProcOSTerm(errorBuffer, osRC);

        if (stafOSTermRC != 0)
        {
            STAFString message("Error during platform-specific termination: ");
            message += errorBuffer + ", RC: " + STAFString(osRC);
            STAFTrace::trace(kSTAFTraceError, message);
            return 1;
        }
    }
    catch (STAFException &se)
    {
        se.write("main()");
        STAFProcOSTerm(errorBuffer, osRC);
    }
    catch (...)
    {
        
        cout << "Caught unknown exception in main()" << endl;
        STAFProcOSTerm(errorBuffer, osRC);
    }

    return 0;
}


STAFRC_t HandleRequest(const STAFConnectionProvider *provider,
                       STAFConnectionPtr &connection)
{
    unsigned int doShutdown = 0;

    bool doTimeout = true;

    try
    {
        unsigned int readBuffer[2];

        connection->read(readBuffer, sizeof(readBuffer), doTimeout);

        unsigned int apiNum = STAFUtilConvertLEUIntToNative(readBuffer[0]);
        unsigned int apiLevel = STAFUtilConvertLEUIntToNative(readBuffer[1]);
        unsigned int minLevel = apiLevel;
        unsigned int maxLevel = apiLevel;
        
        if ((apiNum <= gMaxAPINumber) && (apiLevel == 0) &&
            (gAPITable[apiNum].apiType == kNewAPI))
        {
            // Need to send the ok and then read the min and max levels

            connection->writeUInt(kSTAFOk, doTimeout);
            minLevel = connection->readUInt(doTimeout);
            maxLevel = connection->readUInt(doTimeout);
        }

        if (apiNum > gMaxAPINumber)
        {
            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString message("Invalid API number (");
                message += STAFString(apiNum) + ") received";
                STAFTrace::trace(kSTAFTraceDebug, message);
            }

            connection->writeUInt(kSTAFInvalidAPI, doTimeout);
        }
        else if ((maxLevel < gAPITable[apiNum].minLevel) ||
                 (minLevel > gAPITable[apiNum].maxLevel))
        {
            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString message("Invalid API level (");
                message += STAFString(apiLevel) + ") received for API number " +
                           apiNum;
                STAFTrace::trace(kSTAFTraceDebug, message);
            }

            connection->writeUInt(kSTAFInvalidAPILevel, doTimeout);
        }
        else if ((provider->getName() == sLowerLocal) ?
                 !gAPITable[apiNum].availLocal :
                 !gAPITable[apiNum].availRemote)
        {
            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString message("Invalid connection provider (");
                message += provider->getName() + ") used for API number " +
                           apiNum;
                STAFTrace::trace(kSTAFTraceDebug, message);
            }

            connection->writeUInt(kSTAFInvalidAPI, doTimeout);
        }
        else
        {
            if (gAPITable[apiNum].apiType == kOldAPI)
            {
                connection->writeUInt(kSTAFOk, doTimeout);
            }
            else
            {
                apiLevel = STAF_MIN(maxLevel, gAPITable[apiNum].maxLevel);

                connection->writeUInt(apiLevel, doTimeout);
            }

            gAPITable[apiNum].handler(apiLevel, provider, connection,
                                      doShutdown);
        }
    }
    catch (STAFConnectionIOException &ioe)
    {
        // A client has unexpectedly terminated the connection
        // Do nothing, cleanup is automatic

        if (STAFTrace::doTrace(kSTAFTraceDebug))
        {
            ioe.trace(kSTAFTraceDebug,
                      "HandleRequest(): Connection terminated unexpectedly. ");
        }
    }
    catch (STAFException &se)
    {
        se.trace("HandleRequest()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in HandleRequest()");
    }

    if (doShutdown) gShutdownSemaphore->post();

    return kSTAFOk;
}


void handleLocalServiceRequestAPI(unsigned int level,
                                  const STAFConnectionProvider *provider,
                                  STAFConnectionPtr &connection,
                                  unsigned int &doShutdown)
{
    bool doTimeout = true;

    unsigned int readBuffer[6];

    connection->read(readBuffer, sizeof(readBuffer), doTimeout);

    STAFProcessID_t pid        = STAFUtilConvertLEUIntToNative(readBuffer[1]);
    unsigned int whereLength   = STAFUtilConvertLEUIntToNative(readBuffer[3]);
    unsigned int serviceLength = STAFUtilConvertLEUIntToNative(readBuffer[4]);
    unsigned int requestLength = STAFUtilConvertLEUIntToNative(readBuffer[5]);
    unsigned int buffSize      = whereLength + serviceLength + requestLength;
    STAFBuffer<char> buffer(new char[buffSize], STAFBuffer<char>::INIT);

    connection->read(buffer, buffSize, doTimeout);

    STAFServiceRequestPtr serviceRequestPtr =
                          gRequestManager.getNewServiceRequest();
    STAFServiceRequest &serviceRequest = *serviceRequestPtr;

    serviceRequest.fSyncMode  = STAFUtilConvertLEUIntToNative(readBuffer[0]);
    serviceRequest.fMachineNickname = gMachineNickname;
    serviceRequest.fHandle    = STAFUtilConvertLEUIntToNative(readBuffer[2]);
    serviceRequest.fRequest   = STAFString(buffer + whereLength + serviceLength,
                                           requestLength, STAFString::kUTF8);
    serviceRequest.fDiagEnabled = gDiagManager.getEnabled();
    serviceRequest.fInterface = provider->getName();

    connection->getPeerNetworkIDs(serviceRequest.fLogicalInterfaceID,
                                  serviceRequest.fPhysicalInterfaceID);

    serviceRequest.fMachine = serviceRequest.fLogicalInterfaceID;

    serviceRequest.fEndpoint = serviceRequest.fInterface + gSpecSeparator +
        serviceRequest.fLogicalInterfaceID +
        provider->getProperty(kSTAFConnectionProviderPortProperty);

    serviceRequest.fSTAFInstanceUUID = gSTAFInstanceUUID;
    serviceRequest.fIsLocalRequest = true;

    STAFRC_t rc = gHandleManager.updateTimestamp(serviceRequest.fHandle, pid);

    if (rc != kSTAFOk)
    {
        STAFString errorMsg = STAFString(
            "HandleManager::updateTimestamp() failed to update handle ") +
            serviceRequest.fHandle;

        connection->writeUInt(rc, doTimeout);
        connection->writeString(errorMsg, doTimeout);
        return;
    }

    serviceRequest.fHandleName = gHandleManager.name(serviceRequest.fHandle);
    rc = gHandleManager.variablePool(serviceRequest.fHandle,
                                     serviceRequest.fRequestVarPool);

    if (rc != kSTAFOk)
    {
        STAFString errorMsg = STAFString(
            "HandleManager::variablePool() failed to get the variable pool "
            "for handle ") + serviceRequest.fHandle;

        connection->writeUInt(rc, doTimeout);
        connection->writeString(errorMsg, doTimeout);
        return;
    }

    serviceRequest.fSourceSharedVarPool =
        STAFVariablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT);
    serviceRequest.fLocalSharedVarPool = *gSharedVariablePoolPtr;
    serviceRequest.fLocalSystemVarPool = *gGlobalVariablePoolPtr;

    gHandleManager.getAuthenticationInfo(serviceRequest.fHandle,
                                         serviceRequest.fAuthenticator,
                                         serviceRequest.fUserIdentifier,
                                         serviceRequest.fAuthenticationData);

    serviceRequest.fUser = serviceRequest.fAuthenticator + gSpecSeparator +
        serviceRequest.fUserIdentifier;

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, serviceRequest);
    STAFString unresWhere(buffer, whereLength, STAFString::kUTF8);
    STAFString errorBuffer;

    rc = RESOLVE_STRING(unresWhere, serviceRequest.fTargetMachine);

    if (rc != kSTAFOk)
    {
        connection->writeUInt(rc, doTimeout);
        connection->writeString(errorBuffer, doTimeout);
        return;
    }

    serviceRequest.fTargetService = STAFString(buffer + whereLength,
                                               serviceLength, STAFString::kUTF8);

    // Resolved string, where, is stripped to allow begining and/or trailing
    // white spaces

    serviceRequest.fTargetMachine =
        serviceRequest.fTargetMachine.strip(STAFString::kBoth);

    rc = gRequestManager.add(serviceRequestPtr);

    try
    {
        if ((serviceRequest.fTargetMachine.isEqualTo(
                sLocal, kSTAFStringCaseInsensitive)) ||
            (serviceRequest.fTargetMachine.isEqualTo(
                sLocalLong, kSTAFStringCaseInsensitive)))
        {
            // Local service request

            serviceRequest.fTargetMachine = gMachine;
            serviceRequest.fMachine = gMachine;

            STAFString serviceName;

            rc = RESOLVE_STRING(serviceRequest.fTargetService, serviceName);

            if (rc != kSTAFOk)
            {
                serviceRequest.fResult.fRC = rc;
                serviceRequest.fResult.fResult = errorBuffer;
            }
            else
            {
                // Resolved string, serviceName, is stripped to allow
                // begining and/or trailing white spaces

                serviceRequest.fTargetService = 
                    serviceName.strip(STAFString::kBoth);

                // Strip any whitespace from the beginning of a request

                serviceRequest.fRequest = 
                    serviceRequest.fRequest.strip(STAFString::kFront);

                serviceRequest.fResult = submitLocalRequest(connection,
                                                            serviceRequest);
            }
        }
        else
        {
            // Service request to a remote system

            serviceRequest.fResult = submitRemoteRequest(connection,
                                                         serviceRequest);
        }

        if (serviceRequest.fSyncMode == kSTAFReqSync)
        {
            connection->writeUInt(serviceRequest.fResult.fRC, doTimeout);
            connection->writeString(serviceRequest.fResult.fResult,
                                    doTimeout);
        }

        doShutdown = serviceRequest.fResult.fDoShutdown;
    }
    catch (STAFException &e)
    {
        if ((serviceRequest.fSyncMode == kSTAFReqRetain) ||
            (serviceRequest.fSyncMode == kSTAFReqQueueRetain))
        {
            serviceRequest.fResult.fRC = e.getErrorCode();
            serviceRequest.fResult.fResult = e.getText();
            gRequestManager.requestCompleted(serviceRequest.fRequestNumber,
                                             serviceRequest.fResult);
        }
        else
        {
            if (gRequestManager.requestExists(serviceRequest.fRequestNumber))
                gRequestManager.deleteRequest(serviceRequest.fRequestNumber);
        }

        throw;
    }
    catch (...)
    {
        if (gRequestManager.requestExists(serviceRequest.fRequestNumber))
            gRequestManager.deleteRequest(serviceRequest.fRequestNumber);

        throw;
    }
}


void handleRemoteServiceRequestAPI(unsigned int level,
                                   const STAFConnectionProvider *provider,
                                   STAFConnectionPtr &connection,
                                   unsigned int &doShutdown)
{
    STAFServiceRequestPtr serviceRequestPtr =
                          gRequestManager.getNewServiceRequest();
    STAFServiceRequest &serviceRequest = *serviceRequestPtr;

    bool doTimeout = true;

    serviceRequest.fTargetMachine   = gMachine;
    serviceRequest.fMachineNickname = connection->readString(doTimeout);
    serviceRequest.fHandle          = connection->readUInt(doTimeout);
    serviceRequest.fHandleName      = connection->readString(doTimeout);
    serviceRequest.fTargetService   = connection->readString(doTimeout);
    serviceRequest.fRequest         = connection->readString(doTimeout);
    serviceRequest.fDiagEnabled     = gDiagManager.getEnabled();

    // Note: The request pool and source shared pool here are empty pools.
    //       This is the downlevel protocol in which these pools aren't provided
    //       by the remote end.

    serviceRequest.fRequestVarPool =
        STAFVariablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT);
    serviceRequest.fSourceSharedVarPool =
        STAFVariablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT);
    serviceRequest.fLocalSharedVarPool = *gSharedVariablePoolPtr;
    serviceRequest.fLocalSystemVarPool = *gGlobalVariablePoolPtr;

    // Remote is always synchronous

    serviceRequest.fSyncMode  = kSTAFReqSync;
    serviceRequest.fInterface = provider->getName();

    connection->getPeerNetworkIDs(serviceRequest.fLogicalInterfaceID,
                                  serviceRequest.fPhysicalInterfaceID);

    serviceRequest.fMachine = serviceRequest.fLogicalInterfaceID;

    serviceRequest.fAuthenticator        = gNoneString;
    serviceRequest.fUserIdentifier       = gAnonymousString;
    serviceRequest.fAuthenticationData   = "";
    serviceRequest.fUser                 = gNoneString + gSpecSeparator +
        gAnonymousString;

    serviceRequest.fEndpoint = serviceRequest.fInterface + gSpecSeparator +
        serviceRequest.fLogicalInterfaceID + serviceRequest.fPort;

    // Since this is a pre-V3 system, we don't have a real UUID, so we
    // simply assign the fEndpoint.  Note, there is no way that this can
    // be a local request, as a STAF V3+ system would always come through
    // handleRemoteServiceRequestAPI2.

    serviceRequest.fSTAFInstanceUUID = serviceRequest.fEndpoint;
    serviceRequest.fIsLocalRequest   = false;

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, serviceRequest);
    STAFString errorBuffer;
    STAFString serviceName;
    STAFRC_t rc = RESOLVE_STRING(serviceRequest.fTargetService, serviceName);

    if (rc)
    {
        serviceRequest.fResult.fRC = rc;
        serviceRequest.fResult.fResult = errorBuffer;
    }
    else
    {
        // Resolved string, serviceName, is stripped to allow begining and/or
        // trailing white spaces

        serviceRequest.fTargetService = serviceName.strip(STAFString::kBoth);

        // Strip any whitespace from the beginning of a request

        serviceRequest.fRequest = serviceRequest.fRequest.strip(
            STAFString::kFront);

        rc = gRequestManager.add(serviceRequestPtr);

        try
        {
            serviceRequest.fResult = submitLocalRequest(connection,
                                                        serviceRequest);
        }
        catch (...)
        {
            if (gRequestManager.requestExists(serviceRequest.fRequestNumber))
                gRequestManager.deleteRequest(serviceRequest.fRequestNumber);
            throw;
        }
    }

    connection->writeUInt(serviceRequest.fResult.fRC, doTimeout);

    if (gResultCompatibilityMode == kSTAFResultCompatibilityVerbose)
    {
        // Unmarshall the result into a verbose pretty print format before
        // providing to a pre-STAF V3 machine

        connection->writeString(STAFObject::unmarshall(
            serviceRequest.fResult.fResult)->asFormattedString(), doTimeout);
    }
    else
    {
        // Don't change anything in the result

        connection->writeString(serviceRequest.fResult.fResult, doTimeout);
    }

    doShutdown = serviceRequest.fResult.fDoShutdown;
}


void handleRemoteServiceRequestAPI2(unsigned int level,
                                    const STAFConnectionProvider *provider,
                                    STAFConnectionPtr &connection,
                                    unsigned int &doShutdown)
{
    STAFServiceRequestPtr serviceRequestPtr =
                          gRequestManager.getNewServiceRequest();
    STAFServiceRequest &serviceRequest = *serviceRequestPtr;

    bool doTimeout = true;

    serviceRequest.fTargetMachine      = gMachine;
    serviceRequest.fPort               = connection->readString(doTimeout);
    serviceRequest.fSTAFInstanceUUID   = connection->readString(doTimeout);
    serviceRequest.fHandle             = connection->readUInt(doTimeout);
    serviceRequest.fHandleName         = connection->readString(doTimeout);
    serviceRequest.fTargetService      = connection->readString(doTimeout);
    serviceRequest.fRequest            = connection->readString(doTimeout);
    serviceRequest.fAuthenticator      = connection->readString(doTimeout);
    serviceRequest.fUserIdentifier     = connection->readString(doTimeout);
    serviceRequest.fAuthenticationData = connection->readString(doTimeout);
    serviceRequest.fMachineNickname    = connection->readString(doTimeout);
    serviceRequest.fDiagEnabled        = gDiagManager.getEnabled();

    // Set up the intial/empty variable pools

    serviceRequest.fRequestVarPool =
        STAFVariablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT);
    serviceRequest.fSourceSharedVarPool =
        STAFVariablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT);
    serviceRequest.fLocalSharedVarPool = *gSharedVariablePoolPtr;
    serviceRequest.fLocalSystemVarPool = *gGlobalVariablePoolPtr;

    // Read request var pool

    unsigned int requestPoolSize = connection->readUInt(doTimeout);

    for (unsigned int requestPoolIndex = 0;
         requestPoolIndex < requestPoolSize;
         ++requestPoolIndex)
    {
        STAFString varName = connection->readString(doTimeout);
        STAFString varValue = connection->readString(doTimeout);

        serviceRequest.fRequestVarPool->set(varName, varValue);
    }

    // Read shared var pool

    unsigned int sharedPoolSize = connection->readUInt(doTimeout);

    for (unsigned int sharedPoolIndex = 0;
         sharedPoolIndex < sharedPoolSize;
         ++sharedPoolIndex)
    {
        STAFString varName = connection->readString(doTimeout);
        STAFString varValue = connection->readString(doTimeout);

        serviceRequest.fSourceSharedVarPool->set(varName, varValue);
    }

    STAFRC_t rc = kSTAFOk;

    // Remote Service Request is always synchronous

    serviceRequest.fSyncMode  = kSTAFReqSync;

    // Get Interface information

    serviceRequest.fInterface = provider->getName();

    connection->getPeerNetworkIDs(serviceRequest.fLogicalInterfaceID,
                                  serviceRequest.fPhysicalInterfaceID);

    serviceRequest.fMachine = serviceRequest.fLogicalInterfaceID;

    serviceRequest.fEndpoint = serviceRequest.fInterface + gSpecSeparator +
        serviceRequest.fMachine + serviceRequest.fPort;

    // Determine if request is really from the local machine
    
    STAFString myLogicalInterfaceID = STAFString();
    STAFString myPhysicalInterfaceID = STAFString();
    
    provider->getMyNetworkIDs(myLogicalInterfaceID, myPhysicalInterfaceID);

    // Determine if it's really a local request by checking if the physical
    // interface id matches.  Also, to handle if the STAF service request is
    // submitted to "localhost", "127.0.0.1" (IPv4 address for localhost), or
    // "0:0:0:0:0:0:0:1" (IPv6 address for localhost), added checks to see if
    // the physical interface id matches these localhost adresses.

    if (((myPhysicalInterfaceID == serviceRequest.fPhysicalInterfaceID) ||
         (serviceRequest.fPhysicalInterfaceID == "127.0.0.1") ||
         (serviceRequest.fPhysicalInterfaceID == "0:0:0:0:0:0:0:1")) &&
        (serviceRequest.fSTAFInstanceUUID == gSTAFInstanceUUID))
        serviceRequest.fIsLocalRequest = true;
    else
        serviceRequest.fIsLocalRequest = false;
    
    if (serviceRequest.fAuthenticator != gNoneString)
    {
        // Issue an Authenticate request to the specified Authenticator service
        // passing in the authentication data.  If the authenticator is not
        // registered, set authenticator to "none" and userIdentifier to
        // "anonymous".

        serviceRequest.fResult = gHandleManager.authenticate(
            serviceRequest.fMachine, serviceRequest.fHandle,
            serviceRequest.fAuthenticator, serviceRequest.fUserIdentifier,
            STAFHandleManager::kData, serviceRequest.fAuthenticationData);
    }

    serviceRequest.fUser = serviceRequest.fAuthenticator + gSpecSeparator +
        serviceRequest.fUserIdentifier;

    if (serviceRequest.fResult.fRC == kSTAFOk)
    {
        DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, serviceRequest);
        STAFString serviceName;
        STAFString errorBuffer;

        rc = RESOLVE_STRING(serviceRequest.fTargetService, serviceName);

        if (rc)
        {
            serviceRequest.fResult.fRC = rc;
            serviceRequest.fResult.fResult = errorBuffer;
        }
        else
        {
            // Resolved string, serviceName, is stripped to allow beginning
            // and/or trailing white spaces

            serviceRequest.fTargetService = serviceName.strip(STAFString::kBoth);

            // Strip any whitespace from the beginning of a request

            serviceRequest.fRequest = serviceRequest.fRequest.strip(
                STAFString::kFront);

            rc = gRequestManager.add(serviceRequestPtr);

            try
            {
                serviceRequest.fResult = submitLocalRequest(connection,
                                                            serviceRequest);
            }
            catch (...)
            {
                if (gRequestManager.requestExists(serviceRequest.fRequestNumber))
                    gRequestManager.deleteRequest(serviceRequest.fRequestNumber);
                throw;
            }
        }
    }

    connection->writeUInt(serviceRequest.fResult.fRC, doTimeout);
    connection->writeString(serviceRequest.fResult.fResult, doTimeout);
    doShutdown = serviceRequest.fResult.fDoShutdown;
}


STAFServiceResult submitLocalRequest(STAFConnectionPtr &connection,
                                     STAFServiceRequest &serviceRequest)
{
    STAFServicePtr service;
    STAFServiceResult serviceResult = STAFServiceResult(kSTAFOk);
    STAFServiceResult requestResult;
    STAFString errorBuffer;

    bool doTimeout = true;

    try
    {
        STAFRC_t rc = gServiceManager.get(serviceRequest.fTargetService,
                                          service, errorBuffer);

        if (rc)
        {
            // Service could not be found

            if (serviceRequest.fSyncMode != kSTAFReqSync)
            {
                // Pass back the request number

                serviceResult = STAFServiceResult(
                    kSTAFOk, serviceRequest.fRequestNumber);

                connection->writeUInt(serviceResult.fRC, doTimeout);
                connection->writeString(serviceResult.fResult, doTimeout);
            }

            if (rc == kSTAFDoesNotExist) rc = kSTAFUnknownService;

            serviceResult = STAFServiceResult(
                rc, serviceRequest.fTargetService + errorBuffer);

            gRequestManager.requestCompleted(
                serviceRequest.fRequestNumber, serviceResult);

            if ((serviceRequest.fSyncMode == kSTAFReqQueue) ||
                (serviceRequest.fSyncMode == kSTAFReqQueueRetain))
            {
                // Create the message to queue.  The message is a marshalled
                // map containing the request completion information.

                STAFObjectPtr mc = STAFObject::createMarshallingContext();
                STAFObjectPtr messageMap = STAFObject::createMap();
                messageMap->put("requestNumber",
                                STAFString(serviceRequest.fRequestNumber));
                messageMap->put("rc", STAFString(rc));
                messageMap->put("result", serviceResult.fResult);
                mc->setRootObject(messageMap);
                STAFString resultMessage = mc->marshall();
                
                // Submit the QUEUE request to the QUEUE service

                STAFString queueRequest(sQueueHandle +
                                        serviceRequest.fHandle);
                queueRequest += sType + sRequestComplete;

                queueRequest += sMessage + sColon +
                    resultMessage.length(STAFString::kChar) + sColon +
                    resultMessage;

                gSTAFProcHandlePtr->submit(sLocal, sQueue, queueRequest,
                                           kSTAFReqFireAndForget);
            }
            
            if ((serviceRequest.fSyncMode != kSTAFReqRetain) &&
                (serviceRequest.fSyncMode != kSTAFReqQueueRetain))
            {
                gRequestManager.freeRequest(serviceRequest.fRequestNumber,
                                            &requestResult);
            }

            return STAFServiceResult(rc, serviceResult.fResult);
        }
        else
        {
            // Let the submitter continue by sending back the service result
            // and then submit the actual request

            if (serviceRequest.fSyncMode != kSTAFReqSync)
            {
                // pass back the request number
                serviceResult = STAFServiceResult(
                    kSTAFOk, serviceRequest.fRequestNumber);
                connection->writeUInt(serviceResult.fRC, doTimeout);
                connection->writeString(serviceResult.fResult, doTimeout);
            }

            // If the request originated from the local machine, assign trust
            // based on the machine being local://local

            if (serviceRequest.fIsLocalRequest)
            {
                serviceRequest.fTrustLevel = gTrustManager.getTrustLevel(
                    "local", "local", "local", serviceRequest.fAuthenticator,
                    serviceRequest.fUserIdentifier);
            }
            else
            {
                serviceRequest.fTrustLevel = gTrustManager.getTrustLevel(
                    serviceRequest.fInterface, serviceRequest.fMachine,
                    serviceRequest.fPhysicalInterfaceID,
                    serviceRequest.fAuthenticator,
                    serviceRequest.fUserIdentifier);
            }

            serviceResult = service->submitRequest(serviceRequest);

            if (gResultWarningSize &&
                (serviceResult.fResult.length() > gResultWarningSize))
            {
                // Log a warning tracepoint message

                STAFString warningMsg = STAFString(
                    "WARNING: Result size is large (") +
                    serviceResult.fResult.length() +
                    " bytes).  STAFProc::submitLocalRequest() - " +
                    serviceRequest.fTargetService +
                    " Service Request(" + serviceResult.fRC +
                    ") Client: " + serviceRequest.fMachine +
                    ", Handle: " + serviceRequest.fHandle +
                    ", Handle Name: " + serviceRequest.fHandleName +
                    ", Request: " + serviceRequest.fRequest;

                STAFTrace::trace(kSTAFTraceWarning, warningMsg);
            }
        }

        gRequestManager.requestCompleted(
            serviceRequest.fRequestNumber, serviceResult);

        if ((serviceRequest.fSyncMode == kSTAFReqQueue) ||
            (serviceRequest.fSyncMode == kSTAFReqQueueRetain))
        {
            // Create the message to queue.  The message is a marshalled map
            // containing the request completion information.

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            STAFObjectPtr messageMap = STAFObject::createMap();
            messageMap->put("requestNumber",
                            STAFString(serviceRequest.fRequestNumber));
            messageMap->put("rc", STAFString(serviceResult.fRC));
            messageMap->put("result", serviceResult.fResult);
            mc->setRootObject(messageMap);
            STAFString resultMessage = mc->marshall();

            // Submit the QUEUE request to the QUEUE service

            STAFString queueRequest(sQueueHandle + serviceRequest.fHandle);
            queueRequest += sType + sRequestComplete;
            queueRequest += sMessage + sColon + resultMessage.length(
                STAFString::kChar) + sColon + resultMessage;

            gSTAFProcHandlePtr->submit(sLocal, sQueue, queueRequest,
                                       kSTAFReqFireAndForget);
        }

        if ((serviceRequest.fSyncMode != kSTAFReqRetain) &&
            (serviceRequest.fSyncMode != kSTAFReqQueueRetain))
        {
            gRequestManager.freeRequest(serviceRequest.fRequestNumber,
                                        &requestResult);
        }
    }
    catch (STAFException)
    {
        throw;
    }
    catch (std::bad_alloc)
    {
        // Doing a cout instead of STAFTrace::trace as may not have
        // enough memory to create a STAFString trace message

        cout << "ERROR: Ran out of memory.  STAFProc::submitLocalRequest()"
             << " - " << serviceRequest.fTargetService
             << " Service Request - Client: " << serviceRequest.fMachine
             << ", Handle: " << serviceRequest.fHandle
             << ", Handle Name: " << serviceRequest.fHandleName
             << ", Request: " << serviceRequest.fRequest << endl;

        throw;
    }
    catch (...)
    {
        STAFString data("Caught unknown exception in submitLocalRequest():  ");

        data += serviceRequest.fTargetService + " Service Request - Client: " +
            serviceRequest.fMachine +
            ", Handle: " + STAFString(serviceRequest.fHandle) +
            ", Handle Name: " + serviceRequest.fHandleName +
            ", Request: " + serviceRequest.fRequest;

        STAFTrace::trace(kSTAFTraceError, data);

        throw;
    }

    return serviceResult;
}


STAFServiceResult submitRemoteRequest(STAFConnectionPtr &orgConnection,
                                      STAFServiceRequest &serviceRequest)
{
    // Note: This routine only handles sending remote requests which
    //       originated on this machine

    // Let the submitter continue and then submit the request

    bool doTimeout = true;

    if (serviceRequest.fSyncMode != kSTAFReqSync)
    {
        // pass back the request number
        orgConnection->writeUInt(kSTAFOk, doTimeout);
        orgConnection->writeString(serviceRequest.fRequestNumber, doTimeout);
    }

    if (STAFTrace::doTrace(kSTAFTraceServiceRequest |
                           kSTAFTraceRemoteRequests) &&
        STAFServiceManager::doTraceService(serviceRequest.fTargetService))
    {
        STAFString data("Remote ");

        data += serviceRequest.fTargetService + " Service Request - To: " +
                serviceRequest.fTargetMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Process: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest;

        STAFTrace::trace(kSTAFTraceServiceRequest, data);
    }

    STAFRC_t rc = kSTAFUnknownError;
    STAFString result;
    unsigned int useNewAPI = 1;

    try
    {
        STAFConnectionProviderPtr provider;
        STAFConnectionPtr connection;

        rc = gConnectionManager.makeConnection(serviceRequest.fTargetMachine,
                                               provider, connection, result);
        
        if (rc == kSTAFOk)
        {
            // First, lets try the new API

            // Write new API number
            connection->writeUInt(kSTAFRemoteServiceRequestAPI2, doTimeout);
            connection->writeUInt(0, doTimeout);    // Dummy level

            STAFRC_t ack = static_cast<STAFRC_t>
                (connection->readUInt(doTimeout));

            if (ack != kSTAFOk)
            {
                // They don't support tbe new API, so try the old API
                useNewAPI = 0;

                rc = gConnectionManager.makeConnection(
                         serviceRequest.fTargetMachine, provider,
                         connection, result);

                if (rc) return STAFServiceResult(rc, result);

                // Write old API number
                connection->writeUInt(kSTAFRemoteServiceRequestAPI, doTimeout);
                connection->writeUInt(0, doTimeout);    // API Level

                ack = static_cast<STAFRC_t>(connection->readUInt(doTimeout));

                if (ack != kSTAFOk) return ack;
            }
            else
            {
                // Now find out the specific level to use
                unsigned int minLevel = 2;
                unsigned int maxLevel = 2;

                connection->writeUInt(minLevel, doTimeout);
                connection->writeUInt(maxLevel, doTimeout);

                unsigned int levelToUse = connection->readUInt(doTimeout);

                if (levelToUse == 0) return kSTAFInvalidAPILevel;
            }

            if (useNewAPI)
            {
                connection->writeString(provider->getProperty(
                    kSTAFConnectionProviderPortProperty), doTimeout);
                connection->writeString(serviceRequest.fSTAFInstanceUUID,
                                        doTimeout);
            }
            else
            {
                connection->writeString(gMachine, doTimeout);
            }

            connection->writeUInt(serviceRequest.fHandle, doTimeout);
            connection->writeString(serviceRequest.fHandleName, doTimeout);
            connection->writeString(serviceRequest.fTargetService, doTimeout);
            connection->writeString(serviceRequest.fRequest, doTimeout);

            if (useNewAPI)
            {
                // If the authenticator requires a secure connection, then
                // check if using a secure connection and if not, don't send
                // authentication data over non-secure connection.

                if (requiresSecureConnection(serviceRequest.fAuthenticator) &&
                    (provider->getProperty(
                        kSTAFConnectionProviderIsSecureProperty) != "1"))
                {
                    // Don't send authentication data since non-secure
                    // connection.  Instead set authenticator to "none" and
                    // user to "anonymous" and set authentication data to ""

                    connection->writeString(gNoneString, doTimeout);
                    connection->writeString(gAnonymousString, doTimeout);
                    connection->writeString("", doTimeout);
                }
                else
                {
                    connection->writeString(serviceRequest.fAuthenticator,
                                            doTimeout);
                    connection->writeString(serviceRequest.fUserIdentifier,
                                            doTimeout);
                    connection->writeString(serviceRequest.fAuthenticationData,
                                            doTimeout);
                }

                connection->writeString(serviceRequest.fMachineNickname,
                                        doTimeout);

                // Write request var pool

                STAFVariablePool::VariableMap requestVarMap =
                    serviceRequest.fRequestVarPool->getVariableMapCopy();

                connection->writeUInt(requestVarMap.size(), doTimeout);

                for (STAFVariablePool::VariableMap::iterator requestIter =
                         requestVarMap.begin();
                     requestIter != requestVarMap.end();
                     ++requestIter)
                {
                    connection->writeString(requestIter->second.name,
                                            doTimeout);
                    connection->writeString(requestIter->second.value,
                                            doTimeout);
                }

                // Write shared var pool

                STAFVariablePool::VariableMap sharedVarMap =
                    serviceRequest.fLocalSharedVarPool->getVariableMapCopy();

                connection->writeUInt(sharedVarMap.size(), doTimeout);

                for (STAFVariablePool::VariableMap::iterator sharedIter =
                         sharedVarMap.begin();
                     sharedIter != sharedVarMap.end();
                     ++sharedIter)
                {
                    connection->writeString(sharedIter->second.name,
                                            doTimeout);
                    connection->writeString(sharedIter->second.value,
                                            doTimeout);
                }

            }

            rc = static_cast<STAFRC_t>(connection->readUInt());

            if (!gResultWarningSize)
            {
                result = connection->readString();
            }
            else
            {
                // Added for debugging memory issues
                // Copied code from STAFConnection::readString()

                // First read in the UTF-8 data

                unsigned int dataSize = connection->readUInt();

                if (dataSize > gResultWarningSize)
                {
                    // Log a warning tracepoint message

                    STAFString warningMsg = STAFString(
                        "WARNING: Result size is large (") + dataSize +
                        " bytes).  STAFProc::submitRemoteRequest() - Remote " +
                        serviceRequest.fTargetService +
                        " Service Request(" + rc +
                        ") To: " + serviceRequest.fTargetMachine +
                        ", from Handle: " + serviceRequest.fHandle +
                        ", Handle Name: " + serviceRequest.fHandleName +
                        ", Request: " + serviceRequest.fRequest;

                    STAFTrace::trace(kSTAFTraceWarning, warningMsg);
                }
                
                char *inputData = 0;

                try
                {
                    inputData = new char[dataSize];

                    connection->read((void *)inputData, dataSize);

                    // Indicate UTF-8 data since default is Current Code Page
                    result = STAFString(inputData, dataSize, STAFString::kUTF8);
                    delete [] inputData;
                }
                catch (std::bad_alloc)
                {
                    if (inputData != 0)
                        delete [] inputData;

                    // Doing a cout instead of STAFTrace::trace as may not have
                    // enough memory to create a STAFString trace message

                    cout << "ERROR: Ran out of memory creating a STAFString "
                         << "for a result (" << dataSize << " bytes).  "
                         << "STAFProc::submitRemoteRequest() - Remote "
                         << serviceRequest.fTargetService
                         << " Service Request(" << rc
                         << ") - To: " + serviceRequest.fTargetMachine
                         << ", from Handle: " << serviceRequest.fHandle
                         << ", Handle Name: " << serviceRequest.fHandleName
                         << ", Request: " << serviceRequest.fRequest << endl;
                }
                catch (...)
                {
                    if (inputData != 0)
                        delete [] inputData;

                    throw;
                }
            }
        }
    }
    catch (STAFConnectionProviderConnectException &e)
    {
        rc = kSTAFNoPathToMachine;
        result = e.getText() + STAFString(": ") + STAFString(e.getErrorCode());
    }
    catch (STAFConnectionIOException &e)
    {
        rc = kSTAFCommunicationError;
        result = e.getText() + STAFString(": ") + STAFString(e.getErrorCode());
    }
    catch (STAFException &se)
    {
        rc = se.getErrorCode();
        result = se.getText();

        try
        {
            se.trace("STAFProc::submitRemoteRequest()");

            STAFString data = STAFString(
                "Caught STAFException om STAFProc::submitRemoteRequest(): ") +
                se.getName() + ", Text: " + se.getText() + ", " +
                serviceRequest.fTargetService + " Service Request (" +
                STAFString(se.getErrorCode()) +
                ") - To: " + serviceRequest.fMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Handle Name: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest;

            STAFTrace::trace(kSTAFTraceError, data);
        }
        catch (...)
        {
            // Ignore errors get while creating a STAFString for trace message
        }
    }
    catch (std::bad_alloc)
    {
        rc = kSTAFUnknownError;
        result = "ERROR: Out of memory in STAFProc::suubmitRemoteRequest()";

        try
        {
            // Doing a cout instead of STAFTrace::trace as may not have
            // enough memory to create a STAFString trace message

            cout << "ERROR: Out of memory in "
                 << "STAFProc::submitRemoteRequest() - Remote "
                 << serviceRequest.fTargetService
                 << " Service Request(" << rc
                 << ") - To: " + serviceRequest.fTargetMachine
                 << ", from Handle: " << serviceRequest.fHandle
                 << ", Handle Name: " << serviceRequest.fHandleName
                 << ", Request: " << serviceRequest.fRequest << endl;
        }
        catch (...)
        {
            // Ignore errors get while outputting an error message
        }
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
        result = "Caught unknown exception in submitRemoteRequest():  ";

        try
        {
            STAFString data = result + serviceRequest.fTargetService +
                " Service Request (" + rc + ") - To: " +
                serviceRequest.fMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Handle Name: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest;

            STAFTrace::trace(kSTAFTraceError, data);
        }
        catch (...)
        {
            // Ignore errors get while creating a STAFString for trace message
        }
    }

    try
    {
        if (STAFTrace::doTrace(kSTAFTraceServiceResult |
                               kSTAFTraceRemoteRequests) &&
            STAFServiceManager::doTraceService(serviceRequest.fTargetService))
        {
            STAFString data("Remote ");

            data += serviceRequest.fTargetService + " Service Result (" +
                rc + ") - To: " + serviceRequest.fTargetMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Process: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest +
                ", Result: " + ((result.length() == 0) ?
                                STAFString("<No Result>") : result);
        
            STAFTrace::trace(kSTAFTraceServiceResult, data);
        }
        else if (STAFTrace::doTrace(kSTAFTraceServiceComplete |
                                    kSTAFTraceRemoteRequests) &&
                 STAFServiceManager::doTraceService(
                     serviceRequest.fTargetService))
        {
            STAFString data("Remote ");

            data += serviceRequest.fTargetService + " Service Complete (" +
                rc + ") - To: " + serviceRequest.fTargetMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Process: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest +
                 ", Result Length: " + STAFString(result.length());

            STAFTrace::trace(kSTAFTraceServiceComplete, data);
        }
        else if ((rc != 0) &&
                 STAFTrace::doTrace(kSTAFTraceServiceError |
                                    kSTAFTraceRemoteRequests) &&
                 STAFServiceManager::doTraceService(
                     serviceRequest.fTargetService))
        {
            STAFString data("Remote ");

            data += serviceRequest.fTargetService + " Service Non-Zero (" +
                rc + ") Return Code - To: " + serviceRequest.fTargetMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Process: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest +
                ", Result: " + ((result.length() == 0) ?
                STAFString("<No Result>") : result);

            STAFTrace::trace(kSTAFTraceServiceError, data);
        }
        else if ((rc == kSTAFAccessDenied) &&
                 STAFTrace::doTrace(kSTAFTraceServiceAccessDenied |
                                    kSTAFTraceRemoteRequests) &&
                 STAFServiceManager::doTraceService(
                     serviceRequest.fTargetService))
        {
            STAFString data("Remote ");

            data += serviceRequest.fTargetService +
                " Service Access Denied - To: " + serviceRequest.fTargetMachine +
                ", from Handle: " + STAFString(serviceRequest.fHandle) +
                ", Process: " + serviceRequest.fHandleName +
                ", Request: " + serviceRequest.fRequest;

            STAFTrace::trace(kSTAFTraceServiceAccessDenied, data);
        }
    }
    catch (...)
    {
        cout << "Caught unknown exception in "
             << "STAFProc::submitRemoteRequest() while trying to write "
             << "STAF trace messages" << endl;
    }

    STAFServiceResult serviceResult = STAFServiceResult(rc, result);

    try
    {
        gRequestManager.requestCompleted(serviceRequest.fRequestNumber,
                                         serviceResult);

        if ((serviceRequest.fSyncMode == kSTAFReqQueue) |
            (serviceRequest.fSyncMode == kSTAFReqQueueRetain))
        {
            // Create the message to queue.  The message is a marshalled map
            // containing the request completion information.

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            STAFObjectPtr messageMap = STAFObject::createMap();
            messageMap->put("requestNumber",
                            STAFString(serviceRequest.fRequestNumber));
            messageMap->put("rc", STAFString(serviceResult.fRC));
            messageMap->put("result", serviceResult.fResult);
            mc->setRootObject(messageMap);
            STAFString resultMessage = mc->marshall();

            // Submit the QUEUE request to the QUEUE service

            STAFString queueRequest(sQueueHandle + serviceRequest.fHandle);
            queueRequest += sType + sRequestComplete;
            queueRequest += sMessage + sColon +
                resultMessage.length(STAFString::kChar) + sColon + resultMessage;

            gSTAFProcHandlePtr->submit(sLocal, sQueue, queueRequest,
                                       kSTAFReqFireAndForget);
        }

        STAFServiceResult requestResult;

        if ((serviceRequest.fSyncMode != kSTAFReqRetain) &&
            (serviceRequest.fSyncMode != kSTAFReqQueueRetain))
        {
            gRequestManager.freeRequest(serviceRequest.fRequestNumber,
                                        &requestResult);
        }
    }
    catch (...)
    {
        cout << "Caught unknown exception at end of "
             << "STAFProc::submitRemoteRequest()" << endl;
    }

    return serviceResult;
}


void handleProcessRegistrationAPI(unsigned int level,
                                  const STAFConnectionProvider *provider,
                                  STAFConnectionPtr &connection,
                                  unsigned int &)
{
    STAFHandle_t handle = 0;
    STAFProcessID_t pid = connection->readUInt();
    STAFString name = connection->readString();
    STAFRC_t rc = gHandleManager.registerHandle(handle, pid, name);

    if (STAFTrace::doTrace(kSTAFTraceRegistration))
    {
        STAFString message;

        if (rc == 0)
        {
            message += STAFString("Registered Handle: ") + handle +
                       ",PID: " + pid + ", Name: " + name;
        }
        else
        {
            message += STAFString("Error registering handle with PID: ") + pid +
                       "Name: " + name + ", RC: " + rc;
        }

        STAFTrace::trace(kSTAFTraceRegistration, message);
    }

    connection->writeUInt(rc);
    connection->writeUInt(handle);
}


void handleProcessUnRegistrationAPI(unsigned int level,
                                    const STAFConnectionProvider *provider,
                                    STAFConnectionPtr &connection,
                                    unsigned int &)
{
    STAFProcessID_t pid = connection->readUInt();
    STAFHandle_t handle = connection->readUInt();

    connection->writeUInt(gHandleManager.unRegister(handle, pid));

    if (STAFTrace::doTrace(kSTAFTraceRegistration))
    {
        STAFString message("Unregistered Handle: ");

        message += STAFString(handle) + ", PID: " + pid;

        STAFTrace::trace(kSTAFTraceRegistration, message);
    }
}

void handleFileTransferAPI(unsigned int level,
                           const STAFConnectionProvider *provider,
                           STAFConnectionPtr &connection,
                           unsigned int &)
{
    handleFileTransfer(kOldAPI, level, provider, connection);
}


void handleFileTransferAPI2(unsigned int level,
                            const STAFConnectionProvider *provider,
                            STAFConnectionPtr &connection,
                            unsigned int &)
{
    handleFileTransfer(kNewAPI, level, provider, connection);
}


void handleDirectoryCopyAPI(unsigned int level,
                            const STAFConnectionProvider *provider,
                            STAFConnectionPtr &connection,
                            unsigned int &)
{
    STAFString errorBuffer;
    STAFString result;
    STAFRC_t rc = kSTAFOk;
    STAFServiceResult serviceResult(kSTAFOk);
    unsigned int osRC = 0;
    STAFFSCopyManager::FSCopyDataPtr copyDataPtr;
    STAFString fromMachine = "<Unknown>";
    STAFString toDirectory = "<Unknown>";
    bool addedToCopyMap = false;
    
    try
    {
        // fromMachine is the machine that the files/directories are coming from
        fromMachine = connection->readString();
    }
    catch (STAFConnectionIOException)
    {
        // FS Service side closed the connection due to an error that it
        // already reported, so just return.

        // XXX: Should probably change the protocol so that the FS service
        //      side can tell us that it encountered an error so that we
        //      know to stop processing this directory copy request
        return;
    }

    try
    {
        // orgMachine is the machine that originated the request
        STAFString orgMachine = connection->readString();

        STAFString unresToDirectory = connection->readString();
        toDirectory = unresToDirectory;

        unsigned int flags = connection->readUInt();

        STAFString newEOL = STAFString(kUTF8_NULL);
    
        // Check if all machines involved have the correct trust levels 
        serviceResult = checkTrustForFSCopy(
            1, level, provider, connection, orgMachine, fromMachine, 
            kSTAFFSDirectoryCopy);
    
        if (level > 1)
        {
            newEOL = connection->readString();
        }
    
        // Set up the initial/empty variable pools
        STAFVariablePoolPtr requestVarPool = STAFVariablePoolPtr(
            new STAFVariablePool, STAFVariablePoolPtr::INIT);
        STAFVariablePoolPtr sourceSharedVarPool = STAFVariablePoolPtr(
            new STAFVariablePool, STAFVariablePoolPtr::INIT);

        if (level > 2)
        {
            // Read request var pool

            unsigned int requestPoolSize = connection->readUInt();

            for (unsigned int requestPoolIndex = 0;
                 requestPoolIndex < requestPoolSize;
                 ++requestPoolIndex)
            {
                STAFString varName = connection->readString();
                STAFString varValue = connection->readString();

                requestVarPool->set(varName, varValue);
            }

            // Read source shared var pool

            unsigned int sourceSharedPoolSize = connection->readUInt();

            for (unsigned int sourceSharedPoolIndex = 0;
                 sourceSharedPoolIndex < sourceSharedPoolSize;
                 ++sourceSharedPoolIndex)
            {
                STAFString varName = connection->readString();
                STAFString varValue = connection->readString();

                sourceSharedVarPool->set(varName, varValue);
            }
        }

        const STAFVariablePool *varPoolList[] = { requestVarPool,
                                                  sourceSharedVarPool,
                                                  *gSharedVariablePoolPtr,
                                                  *gGlobalVariablePoolPtr };

        unsigned int varPoolListSize = sizeof(varPoolList) /
            sizeof(const STAFVariablePool *);

        if (serviceResult.fRC == kSTAFOk)
        {
            try
            {
                serviceResult.fRC = RESOLVE_STRING(
                    unresToDirectory, toDirectory);

                if (serviceResult.fRC != kSTAFOk)
                    serviceResult.fResult = errorBuffer;
            }
            catch (STAFException &se)
            {
                // The STAFVariablePool::resolve() method can call the
                // STAFVariablePool::get() method which constructs a
                // STAFMutexSemLock which can raise an exception.  Thus,
                // need to do a try/catch here so that we respond back
                // with the appropriate error

                serviceResult.fRC = kSTAFBaseOSError;
                serviceResult.fResult = STAFString(
                    "Exception while resolving variables in TODIRECTORY '") +
                    toDirectory + "' on TOMACHINE\n" +
                    se.getText() + STAFString(": ") + se.getErrorCode();
            }
        }

        if (serviceResult.fRC != kSTAFOk)
        {
            connection->writeUInt(serviceResult.fRC);
            connection->writeString(serviceResult.fResult);

            return;
        }

        // Convert the toDirectory to the full long path name
        // (e.g. with correct file separators, correct case if Windows,
        // no unnecessary trailing slashes, etc)
        
        STAFFSPath toDirPath = STAFFSPath(toDirectory);
        toDirectory = toDirPath.setRoot(toDirPath.root()).asString();

        // Check if the toDirectory already exists

        unsigned int theDirectoryExists = 0;

        try
        {
            theDirectoryExists = toDirPath.exists();
        }
        catch (STAFBaseOSErrorException &sboe)
        {
            STAFString errMsg = "Error checking if TODIRECTORY '" +
                toDirectory + "' exists\n" +
                sboe.getText() + STAFString(": ") + sboe.getErrorCode();
            connection->writeUInt(kSTAFBaseOSError);
            connection->writeString(errMsg);

            return;
        }

        if ((flags & 0x00000001) && theDirectoryExists)
        {
            connection->writeUInt(kSTAFAlreadyExists);
            connection->writeString(toDirectory);

            return;
        }

        if ((flags & 0x00000002) && !theDirectoryExists)
        {
            connection->writeUInt(kSTAFDoesNotExist);
            connection->writeString(toDirectory);

            return;
        }

        // Establish eol
        STAFString tmpEOL = newEOL;

        // Determine new EOL
        if (newEOL.lowerCase() == "native")
        {
            STAFConfigInfo sysInfo;
            STAFString_t errorBuffer;
            unsigned int osRC;

            if (STAFUtilGetConfigInfo(&sysInfo, &errorBuffer, &osRC) != kSTAFOk)
            {
                // Kill transfer
                connection->writeUInt(kSTAFBaseOSError);
                connection->writeString("Get EOL failed");
                return;
            }
            newEOL=sysInfo.lineSeparator;
        }
        else if (newEOL.lowerCase() == "windows")
        {
            newEOL = STAFString(kUTF8_CR) + STAFString(kUTF8_LF);// \r\n
        }
        else if (newEOL.lowerCase() == "unix")
        {
            newEOL = STAFString(kUTF8_LF);// \n
        }
        else
            newEOL = tmpEOL;


        rc = kSTAFOk;
        result = STAFString();

        // Create directory if !theDirectoryExists

        if (!theDirectoryExists)
        {
            rc = STAFFSCreateDirectory(toDirectory.getImpl(),
                                       kSTAFFSCreatePath, &osRC);

            if (rc != kSTAFOk)
            {
                if (rc == kSTAFBaseOSError)
                    result = "OSRC: " + STAFString(osRC) +
                        ", Error on Creating a Directory: " + toDirectory;

                connection->writeUInt(rc);
                connection->writeString(result);

                return;
            }
        }
    
        // Add an entry to the FS Copy Map so can list copies in progress

        if (gFSCopyManagerPtr->add(toDirectory, fromMachine, kSTAFFSCopyTo,
                                   kSTAFFSDirectoryCopy, kSTAFFSBinary,
                                   0, copyDataPtr))
        {
            // The toDirectory already exists in the FS Copy Map

            connection->writeUInt(kSTAFFileWriteError);
            connection->writeString(
                STAFString("Cannot write to directory ") + toDirectory +
                " at the same time this request is reading from it or if "
                "another copy request is currently reading or writing to "
                "the directory.");

            return;
        }
    
        addedToCopyMap = true;

        connection->writeUInt(rc);
        connection->writeString(result);
    
        if (level > 1)
        {
            connection->writeString(newEOL);

            // Allow the use of the text noconvert routine if the codepages on
            // both sides are the same.  This allows for a faster transfer.

            bool doCodepageConvert =
                (connection->readUInt() == kSTAFFSTextConvert);

            if (doCodepageConvert)
            {
                STAFString cPage;
                rc = RESOLVE_STRING("{STAF/Config/CodePage}", cPage);

                // Send this codepage id
                connection->writeString(cPage);
            }
        }

        rc = kSTAFOk;
        result = STAFString();

        /************** Start transferring the directory ***************/

        for (unsigned int stop = connection->readUInt(); !stop;
             stop = connection->readUInt())
        {
            unsigned int isFile = connection->readUInt();

            if (isFile == kSTAFFSDirectory)
            {
                // Current entry is a directory

                STAFString subDirectory = connection->readString();
                STAFString newDirectory = toDirectory + subDirectory;
            
                gFSCopyManagerPtr->updateDirectoryCopy(
                    copyDataPtr, newDirectory, kSTAFFSBinary, 0);

                rc = STAFFSCreateDirectory(newDirectory.getImpl(),
                                           kSTAFFSCreatePath, &osRC);

                if (rc == kSTAFBaseOSError) result = osRC;
                else if (rc == kSTAFAlreadyExists)
                {
                    // We will override the current subDirectory
                    rc = kSTAFOk;
                    result = STAFString();
                }

                connection->writeUInt(rc);
                connection->writeString(result);
            }
            else if (isFile == kSTAFFSFile)
            {
                // Current entry is a file.  Copy it.

                STAFString newFile = connection->readString();
                STAFString toFile = toDirectory + newFile;
            
                bool textTransfer = false;
                bool doCodepageConvert = false;
                unsigned int transferType;

                //  If API appropriate, check for text transfer level > 1
                if (level > 1)
                {
                    // Determine transfer type
                    transferType= connection->readUInt();
                    textTransfer = (transferType != kSTAFFSBinary);
                    doCodepageConvert = (transferType == kSTAFFSTextConvert);
                }

                fstream outFile(toFile.toCurrentCodePage()->buffer(),
                                STAF_ios_binary | ios::out);

                if (!outFile)
                {
                    connection->writeUInt(kSTAFFileOpenError);
                    connection->writeString(toFile);

                    continue;
                }

                // outFile is ok
                connection->writeUInt(kSTAFOk);
                connection->writeString(STAFString());
            
                if (doCodepageConvert)
                {
                    // Perform a text transfer with conversion of eol chars and
                    // a codepage conversion.  It runs considerably slower than
                    // the binary transfer.
                    // - This is the type of transfer that will occur if the
                    //   TEXT option is specified and the codepages are
                    // different.

                    // XXX: Change to provide file length in the future
                    unsigned int fileLength = 0;

                    gFSCopyManagerPtr->updateDirectoryCopy(
                        copyDataPtr, toFile, kSTAFFSTextConvert, fileLength);

                    STAFString currentEOL = connection->readString();
                    STAFString transferStr;
                    STAFStringBufferPtr convertedStr;
                    unsigned int bytesCopied = 0;
                    int continueCopy = connection->readUInt();

                    while (continueCopy != kSTAFFSFinishedCopy)
                    {
                        transferStr = connection->readString();
                        transferStr = transferStr.replace(currentEOL, newEOL);
                        convertedStr = transferStr.toCurrentCodePage();

                        rc = writeFile(
                            outFile, convertedStr->buffer(),
                            convertedStr->length(), toFile, fromMachine,
                            fileLength, bytesCopied);
                    
                        if (rc == kSTAFOk)
                        {
                            bytesCopied += convertedStr->length();
                            gFSCopyManagerPtr->updateFileCopy(
                                copyDataPtr, bytesCopied);
                        }

                        continueCopy = connection->readUInt();
                    }

                    outFile.close();

                    // Indicate whether the copy was successful
                    connection->writeUInt(rc);
                }
                else if (textTransfer)
                {
                    // Perform a text transfer with conversion of eol chars
                    // without a codepage conversion.  It runs considerably
                    // faster than the codepage conversion transfer.
                    // -  This is the type of transfer that will occur if the
                    //    NOCONVERT option is enabled.
                    // -  This is the type of transfer that will occur if the
                    //    codepages are the same and a text transfer has been
                    //    specified.
                
                    // XXX: Change to provide fileLength in the future
                    unsigned int fileLength = 0;

                    gFSCopyManagerPtr->updateDirectoryCopy(
                        copyDataPtr, toFile, kSTAFFSTextNoConvert, fileLength);
                
                    STAFString currentEOL = connection->readString();
                    unsigned int transferLen = connection->readUInt();
                    unsigned int bufferLen = connection->readUInt();

                    STAFStringBufferPtr convertedNewEOL =
                        newEOL.toCurrentCodePage();
                    STAFStringBufferPtr convertedCurrentEOL =
                        currentEOL.toCurrentCodePage();
                    int newEOLLength = convertedNewEOL->length();
                    int len;
                    int offset = 0;
                    int endBuffer;
                    int strLen = 1;
                    int currentEOLLength = convertedCurrentEOL->length();
                    int currentEOLOffset = currentEOLLength - 1;
                    unsigned int bytesCopied = 0;

                    // This is the largest a buffer can grow to during an eol
                    // replace.
                    unsigned int currentBuffer = 
                        (1 + abs(currentEOLLength - newEOLLength)) *
                         transferLen + currentEOLLength + 1;

                    // This pointer marks the location to start loading
                    // characters into the buffer.
                    char * offsetBuffer;

                    // This is the buffer to store the incoming data
                    char * buffer = new char[
                        transferLen + currentEOLLength + 1];

                    // This is the buffer to store the data that has had the
                    // eol replaced.
                    char * newBuffer = new char[currentBuffer];

                    offsetBuffer = buffer;

                    while (bufferLen != kSTAFFSFinishedCopy)
                    {
                        // Update current buffer length
                        strLen += bufferLen;

                        connection->read(offsetBuffer,bufferLen);

                        // Mark the end of the string with a null
                        offsetBuffer[bufferLen] = '\0';

                        replaceStringInBuffer(
                            buffer, convertedCurrentEOL->buffer(),
                            convertedNewEOL->buffer(), strLen,
                            currentEOLLength, newEOLLength, &len, &endBuffer,
                            newBuffer);
                    
                        // Get length of the next transfer
                        bufferLen = connection->readUInt();

                        if (bufferLen == kSTAFFSFinishedCopy)
                        {
                            // Last line to be output

                            rc = writeFile(outFile, newBuffer, len - 1,
                                           toFile, fromMachine, fileLength,
                                           bytesCopied);

                            if (rc == kSTAFOk)
                            {
                                bytesCopied += (len - 1);
                                gFSCopyManagerPtr->updateFileCopy(
                                    copyDataPtr, bytesCopied);
                            }
                        }
                        else
                        {
                            // Buffer must roll

                            if (endBuffer != 0xffffffff)
                            {
                                // There is an eol marker in the area to be
                                // rolled. The roll will take place starting
                                // at the end of the eol.

                                endBuffer = endBuffer + newEOLLength;

                                rc = writeFile(
                                    outFile, newBuffer, endBuffer,
                                    toFile, fromMachine, fileLength,
                                    bytesCopied);

                                if (rc == kSTAFOk)
                                {
                                    bytesCopied += endBuffer;
                                    gFSCopyManagerPtr->updateFileCopy(
                                        copyDataPtr, bytesCopied);
                                }

                                // offset = buffer length - null char - ending
                                //   index of eol
                                offset = len - 1 - endBuffer;

                                rollBuffer(buffer, strLen, offset + 1);
                                offsetBuffer = buffer + offset;

                                // Set string length = offset + null
                                strLen = offset + 1;
                            }
                            else
                            {
                                // Wrap whole buffer
                                offset = currentEOLOffset;

                                // The end of the buffer to write is length of
                                // the buffer - offset - null
                                endBuffer = len - offset - 1;

                                rc = writeFile(
                                    outFile, newBuffer, endBuffer, 
                                    toFile, fromMachine, fileLength,
                                    bytesCopied);

                                if (rc == kSTAFOk)
                                {
                                    bytesCopied += endBuffer;
                                    gFSCopyManagerPtr->updateFileCopy(
                                        copyDataPtr, bytesCopied);
                                }

                                rollBuffer(buffer, strLen, offset + 1);
                                offsetBuffer = buffer + offset;

                                // Set string length = offset + null
                                strLen = offset + 1;
                            }
                        }
                    }

                    delete[] buffer;
                    delete[] newBuffer;
                    outFile.close();

                    // Indicate whether the copy was successful
                    connection->writeUInt(rc);
                }
                else
                {
                    // Perform a binary transfer of the file

                    unsigned char fileBuffer[4000] = { 0 };
                    unsigned int writeLength = 0;
                    unsigned int bytesCopied = 0;
                    unsigned int fileLength = connection->readUInt();
                    unsigned int bytesLeftToWrite = fileLength;
                    unsigned int fileCopyRC = kSTAFOk;

                    gFSCopyManagerPtr->updateDirectoryCopy(
                        copyDataPtr, toFile, kSTAFFSBinary, fileLength);
                
                    if (level > 3)
                    {
                        // Starting with level 4 for the STAFDirectoryCopyAPI,
                        // to improve performance, acks are no longer sent/
                        // received after each read/write. Instead, a final
                        // ack is sent after the entire file is processed
                        // which indicates if the copy was successful.

                        while (bytesLeftToWrite > 0)
                        {
                            writeLength = STAF_MIN(sizeof(fileBuffer),
                                                   bytesLeftToWrite);
                            connection->read(fileBuffer, writeLength);

                            rc = writeFile(
                                 outFile, reinterpret_cast<char *>(fileBuffer),
                                 writeLength, toFile, fromMachine, fileLength,
                                 bytesCopied);

                            if (rc == kSTAFOk)
                            {
                                bytesCopied += writeLength;
                                gFSCopyManagerPtr->updateFileCopy(
                                    copyDataPtr, bytesCopied);
                            }
                            else
                            {
                                // Save rc that indicates the copy failed
                                fileCopyRC = rc;
                            }
          
                            bytesLeftToWrite -= writeLength;
                        }

                        outFile.close();
                    
                        // Write a final ack indicating if the copy was
                        // successful

                        connection->writeUInt(fileCopyRC);
                    }
                    else
                    {
                        // Levels < 4 for the STAFDirectoryCopyAPI send/receive
                        // acknowledgements after each read/write.

                        while ((bytesLeftToWrite > 0) && (outFile.good()))
                        {
                            rc = connection->readUInt();

                            if (rc == kSTAFOk)
                            {
                                writeLength = STAF_MIN(sizeof(fileBuffer),
                                                       bytesLeftToWrite);
                                connection->read(fileBuffer, writeLength);

                                rc = writeFile(
                                    outFile,
                                    reinterpret_cast<char *>(fileBuffer),
                                    writeLength, toFile, fromMachine,
                                    fileLength, bytesCopied);

                                connection->writeUInt(rc);

                                if (rc != kSTAFOk)
                                {
                                    fileLength = 0;
                                    break;
                                }

                                bytesLeftToWrite -= writeLength;
                                bytesCopied += writeLength;
                                gFSCopyManagerPtr->updateFileCopy(
                                    copyDataPtr, bytesCopied);
                            }
                            else
                            {
                                // File read on other side of connection failed
                                bytesLeftToWrite = 0;
                                break;
                            }
                        }

                        outFile.close();
                    }
                }
            }
            else
            {
                // Should Never Get Here
            }
        } // end of for loop
    } /****************** End try directory transfer block *****************/
    catch (STAFConnectionIOException &e)
    {
        // Just log a Debug trace message as the problem is being handled on
        // FS service side

        STAFString message = "STAFProc::handleDirectoryCopyAPI(): Connection "
            "terminated while copying to directory '" + toDirectory +
            "' from machine '" + fromMachine + "'";

        e.trace(kSTAFTraceDebug, message.toCurrentCodePage()->buffer());
    }
    catch (STAFException &e)
    {
        // Log an Error trace message as an unexpected STAFException occurred

        STAFString message = "STAFProc::handleDirectoryCopyAPI() while "
            "copying to directory '" + toDirectory + "' from machine '" +
            fromMachine + "'";

        e.trace(message.toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        // Log an Error trace message as an unexpected exception occurred

        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFProc::handleDirectoryCopyAPI() "
            "while copying to directory '" + toDirectory +
            "' from machine '" + fromMachine + "'");
    }

    // Remove file copy entry from the map

    if (addedToCopyMap)
        gFSCopyManagerPtr->remove(copyDataPtr);
}

void handleFileTransfer(unsigned int apiType, unsigned int level,
                        const STAFConnectionProvider *provider,
                        STAFConnectionPtr &connection)
{
    STAFRC_t rc = kSTAFOk;
    STAFServiceResult serviceResult(kSTAFOk);

    STAFString toFile = "<Unknown>";
    STAFString fromMachine = "<Unknown>";
    STAFFSCopyManager::FSCopyDataPtr copyDataPtr;
    bool addedToCopyMap = false;
    bool textTransfer = false;
    bool doCodepageConvert = false;
    
    try
    {
        // If the text transfer can be requested it will be done here

        if (apiType == kNewAPI  && level > 1)
        {
            // Get transfer type
            unsigned int transferType = connection->readUInt();
            textTransfer = (transferType != kSTAFFSBinary);
            doCodepageConvert = (transferType == kSTAFFSTextConvert);
        }

        // fromMachine is the machine that the file is coming from
        // orgMachine is the machine that originated the request
        fromMachine = connection->readString();
        STAFString orgMachine = connection->readString();

        STAFString unresToFile = connection->readString();
    
        unsigned int flags = 0;

        try
        {
            flags = connection->readUInt();
        }
        catch (STAFConnectionIOException)
        {
            // FS Service side closed the connection due to an error that it
            // already reported, so just return.

            // XXX: Should probably change the protocol so that the FS service
            //      side can tell us that it encountered an error so that we
            //      know to stop processing this directory copy request
            return;
        }
        
        // Check if all machines involved have the correct trust levels 
        serviceResult = checkTrustForFSCopy(
            apiType, level, provider, connection, orgMachine, fromMachine,
            kSTAFFSFileCopy);

        // Set up the initial/empty variable pools
        STAFVariablePoolPtr requestVarPool = STAFVariablePoolPtr(
            new STAFVariablePool, STAFVariablePoolPtr::INIT);
        STAFVariablePoolPtr sourceSharedVarPool = STAFVariablePoolPtr(
            new STAFVariablePool, STAFVariablePoolPtr::INIT);

        if (apiType == kNewAPI && level > 2)
        {
            // Read request var pool

            unsigned int requestPoolSize = connection->readUInt();

            for (unsigned int requestPoolIndex = 0;
                 requestPoolIndex < requestPoolSize;
                 ++requestPoolIndex)
            {
                STAFString varName = connection->readString();
                STAFString varValue = connection->readString();

                requestVarPool->set(varName, varValue);
            }

            // Read source shared var pool

            unsigned int sourceSharedPoolSize = connection->readUInt();

            for (unsigned int sourceSharedPoolIndex = 0;
                 sourceSharedPoolIndex < sourceSharedPoolSize;
                 ++sourceSharedPoolIndex)
            {
                STAFString varName = connection->readString();
                STAFString varValue = connection->readString();

                sourceSharedVarPool->set(varName, varValue);
            }
        }

        const STAFVariablePool *varPoolList[] = { requestVarPool,
                                                  sourceSharedVarPool,
                                                  *gSharedVariablePoolPtr,
                                                  *gGlobalVariablePoolPtr };
        unsigned int varPoolListSize = sizeof(varPoolList) /
            sizeof(const STAFVariablePool *);
        STAFString errorBuffer;
        
        if (serviceResult.fRC == kSTAFOk)
        {
            try
            {
                serviceResult.fRC = RESOLVE_STRING(unresToFile, toFile);
                
                if (serviceResult.fRC != kSTAFOk)
                    serviceResult.fResult = errorBuffer;
            }
            catch (STAFException &se)
            {
                // The STAFVariablePool::resolve() method can call the
                // STAFVariablePool::get() method which constructs a
                // STAFMutexSemLock which can raise an exception, thus,
                // need to do a try/catch here so that we respond back
                // with the appropriate error

                serviceResult.fRC = kSTAFBaseOSError;
                serviceResult.fResult = STAFString(
                    "Exception while resolving variables in TOFILE '") +
                    toFile + "' on TOMACHINE\n" +
                    se.getText() + STAFString(": ") + se.getErrorCode();
            }
        }

        if (serviceResult.fRC != kSTAFOk)
        {
            connection->writeUInt(serviceResult.fRC);
            connection->writeString(serviceResult.fResult);

            return;
        }

        // Check if the to file already exists

        STAFFSPath toPath(toFile);
        unsigned int theFileExists = 0;

        try
        {
            theFileExists = toPath.exists();
        }
        catch (STAFBaseOSErrorException &sboe)
        {
            STAFString errMsg = "Error checking if to file '" +
                toFile + "' exists\n" +
                sboe.getText() + STAFString(": ") + sboe.getErrorCode();
            connection->writeUInt(kSTAFBaseOSError);
            connection->writeString(errMsg);

            return;
        }

        if (((flags & 0x00000001) && theFileExists) ||
            ((flags & 0x00000002) && !theFileExists))
        {
            connection->writeUInt(kSTAFFileOpenError);
            connection->writeString(toFile);

            return;
        }

        // Convert the file name to the full long path name
        // (e.g. with correct file separators, correct case if Windows,
        // no unnecessary trailing slashes, etc)

        toFile = toPath.setRoot(toPath.root()).asString();
        
        // Add an entry to the FS Copy Map so can list copies in progress

        int copyMode = kSTAFFSBinary;
    
        if (doCodepageConvert)
            copyMode = kSTAFFSTextConvert;   // Text, cp conversion
        else if (textTransfer)
            copyMode = kSTAFFSTextNoConvert; // Text, no cp conversion

        if (gFSCopyManagerPtr->add(toFile, fromMachine, kSTAFFSCopyTo,
                                   kSTAFFSFileCopy, copyMode, 0, copyDataPtr))
        {
            // The toFile already exists in the FS Copy Map so return an error

            connection->writeUInt(kSTAFFileWriteError);
            connection->writeString(
                STAFString("Cannot write to file ") + toFile + " at the same"
                " time this request is reading from it or if another copy "
                "request is currently reading or writing the file.");
            return;
        }

        addedToCopyMap = true;

        // Open the file in write mode (which erases it's content)

        fstream outFile(toFile.toCurrentCodePage()->buffer(),
                        STAF_ios_binary | ios::out);

        if (!outFile)
        {
            gFSCopyManagerPtr->remove(copyDataPtr);

            connection->writeUInt(kSTAFFileOpenError);
            connection->writeString(toFile);

            return;
        }
    
        // Write an acknowledge that it's ok to write to this file

        connection->writeUInt(kSTAFOk);
        connection->writeString(STAFString());

        STAFString newEOL;

        if (textTransfer)
        {
            // Establish the new eol marker.  It causes the use of binary
            // transfer if the eol on both sides are the same and no codepage
            // conversion is to be done.  This allows for a faster transfer.

            // Get EOL chars
            newEOL = connection->readString();
            STAFString tmpEOL = newEOL;

            // Determine new EOL
            if (newEOL.lowerCase() == "native")
            {
                STAFConfigInfo sysInfo;
                STAFString_t errorBuffer;
                unsigned int osRC;

                if (STAFUtilGetConfigInfo(&sysInfo, &errorBuffer, &osRC)
                    != kSTAFOk)
                {
                    // Kill the file transfer
                    outFile.close();
                    gFSCopyManagerPtr->remove(copyDataPtr);
                    connection->writeUInt(kSTAFFSStopCopy);
                    return;
                }
                newEOL = sysInfo.lineSeparator;
            }
            else if (newEOL.lowerCase() == "windows")
            {
                newEOL = STAFString(kUTF8_CR) + STAFString(kUTF8_LF);// \r\n
            }
            else if (newEOL.lowerCase() == "unix")
            {
                newEOL = STAFString(kUTF8_LF);// \n
            }
            else
                newEOL = tmpEOL;

            // Send new eol ok
            connection->writeUInt(kSTAFFSContinueCopy);

            // Send new EOL
            connection->writeString(newEOL);

            textTransfer = (connection->readUInt() == kSTAFFSTextNoConvert);
        }

        if (doCodepageConvert)
        {
            // Send codepage id so that can check if the codepages on both
            // sides are the same because then can use the text noconvert
            // routine which is faster.

            STAFString cPage;
            STAFString errorBuffer;

            // Get local codepage
            rc = RESOLVE_STRING("{STAF/Config/CodePage}", cPage);

            // Send this codepage id
            connection->writeString(cPage);
            doCodepageConvert = (connection->readUInt() == kSTAFFSTextConvert);

            // XXX: Uncomment next line to enable the codepage filter
            //      Currently, commented out to force the codepage conversion
            //      on all text transfers.
            //doCodepageConvert = true;
        }
    
        /************** Start transferring the file ***************/
        
        if (doCodepageConvert)
        {
            // Perform a text transfer with conversion of eol chars and a 
            // codepage conversion.  It runs considerably slower than the
            // binary transfer.  This is the type of transfer that will occur
            // if the TEXT option is specified and the codepages are different.

            STAFString currentEOL = connection->readString();

            STAFString transferStr;
            STAFStringBufferPtr convertedStr;
            int continueCopy = connection->readUInt();
            unsigned int bytesCopied = 0;

            // XXX: Change to get fileLength in the future
            unsigned int fileLength = 0;

            gFSCopyManagerPtr->updateFileCopy1(
                copyDataPtr, fileLength, kSTAFFSTextConvert);

            while (continueCopy != kSTAFFSFinishedCopy)
            {
                transferStr = connection->readString();
                transferStr = transferStr.replace(currentEOL, newEOL);
                convertedStr = transferStr.toCurrentCodePage();

                rc = writeFile(
                    outFile, convertedStr->buffer(), convertedStr->length(),
                    toFile, fromMachine, fileLength, bytesCopied);

                if (rc == kSTAFOk)
                {
                    bytesCopied += convertedStr->length();
                    gFSCopyManagerPtr->updateFileCopy(
                        copyDataPtr, bytesCopied);
                }

                continueCopy = connection->readUInt();
            }

            outFile.close();
        }
        else if (textTransfer)
        {
            // Perform a text transfer with conversion of eol chars without a
            // codepage conversion.  It runs considerably faster than the 
            // codepage conversion transfer.
            // - This is the type of transfer that will occur if the NOCONVERT 
            //   option is enabled.
            // - This is the type of transfer that will occur if the codepages 
            //   are the same and a text transfer has been specified

            // XXX: Don't have file length so assigning 0
            unsigned int fileLength = 0;

            gFSCopyManagerPtr->updateFileCopy1(
                copyDataPtr, fileLength, kSTAFFSTextNoConvert);

            STAFString currentEOL = connection->readString();
            unsigned int transferLen = connection->readUInt();
            unsigned int bufferLen = connection->readUInt();

            STAFStringBufferPtr convertedNewEOL = newEOL.toCurrentCodePage();
            STAFStringBufferPtr convertedCurrentEOL = currentEOL.toCurrentCodePage();
            int newEOLLength = convertedNewEOL->length();
            int len;
            int offset = 0;
            int endBuffer;
            int strLen = 1;
            int currentEOLLength = convertedCurrentEOL->length();
            int currentEOLOffset = currentEOLLength - 1;
            unsigned int bytesCopied = 0;

            // This is the largest a buffer can grow to during an eol replace
            unsigned int currentBuffer =
                (1 + abs(currentEOLLength - newEOLLength)) * transferLen +
                currentEOLLength + 1;

            // This pointer marks the location to start loading characters
            // into the buffer
            char * offsetBuffer;

            // This is the buffer to store the incoming data
            char * buffer = new char[transferLen + currentEOLLength + 1];

            // This is the buffer to store the data with the EOLs replaced
            char * newBuffer = new char[currentBuffer];

            offsetBuffer = buffer;

            while (bufferLen != kSTAFFSFinishedCopy)
            {
                // Update the current buffer length
                strLen += bufferLen;

                connection->read(offsetBuffer,bufferLen);

                // Mark the end of the string with a null
                offsetBuffer[bufferLen] = '\0';

                replaceStringInBuffer(buffer, convertedCurrentEOL->buffer(),
                    convertedNewEOL->buffer(), strLen, currentEOLLength,
                    newEOLLength, &len, &endBuffer, newBuffer);

                // Get length of the next transfer
                bufferLen = connection->readUInt();

                if (bufferLen == kSTAFFSFinishedCopy)
                {
                    // Last line to be output

                    rc = writeFile(outFile, newBuffer, len - 1,
                                   toFile, fromMachine,
                                   fileLength, bytesCopied);

                    if (rc == kSTAFOk)
                    {
                        bytesCopied += (len - 1);
                        gFSCopyManagerPtr->updateFileCopy(
                            copyDataPtr, bytesCopied);
                    }
                }
                else
                {
                    // Buffer must roll

                    if (endBuffer != 0xffffffff)
                    {
                        // There is an eol marker in the area to be rolled.
                        // The end of the buffer is the starting index of
                        // the last occurence of the new eol + the length of
                        // the new eol.

                        endBuffer = endBuffer + newEOLLength;

                        rc = writeFile(outFile, newBuffer, endBuffer,
                                       toFile, fromMachine,
                                       fileLength, bytesCopied);

                        if (rc == kSTAFOk)
                        {
                            bytesCopied += endBuffer;
                            gFSCopyManagerPtr->updateFileCopy(
                                 copyDataPtr, bytesCopied);
                        }

                        // offset = buffer length - null char - ending index
                        // of eol
                        offset = len - 1 - endBuffer;

                        rollBuffer(buffer, strLen, offset + 1);
                        offsetBuffer = buffer + offset;

                        // Set string length = offset + null
                        strLen = offset + 1;
                    }
                    else
                    {
                        // Wrap whole buffer
                        offset = currentEOLOffset;

                        // The end of the buffer to write is length of the
                        // buffer - offset - null
                        endBuffer = len - offset - 1;

                        rc = writeFile(outFile, newBuffer, endBuffer,
                                       toFile, fromMachine,
                                       fileLength, bytesCopied);
                        
                        if (rc == kSTAFOk)
                        {
                            bytesCopied += endBuffer;
                            gFSCopyManagerPtr->updateFileCopy(
                                copyDataPtr, bytesCopied);
                        }

                        rollBuffer(buffer, strLen, offset + 1);
                        offsetBuffer = buffer + offset;

                        // Set string length = offset + null
                        strLen = offset + 1;
                    }
                }
            }

            delete[] buffer;
            delete[] newBuffer;
            outFile.close();
        }
        else
        {
            // Binary file copy

            unsigned char fileBuffer[4000] = { 0 };
            unsigned int writeLength = 0;
            unsigned int bytesCopied = 0;
            unsigned int fileCopyRC = kSTAFOk;

            if (apiType == kNewAPI)
            {
                unsigned int fileLength = connection->readUInt();
                unsigned int bytesLeftToWrite = fileLength;
                unsigned int bytesRead = 0;

                gFSCopyManagerPtr->updateFileCopy1(
                    copyDataPtr, fileLength, kSTAFFSBinary);

                while (bytesLeftToWrite > 0)
                {
                    writeLength = STAF_MIN(sizeof(fileBuffer),
                                           bytesLeftToWrite);

                    connection->read(fileBuffer, writeLength);

                    rc = writeFile(
                        outFile, reinterpret_cast<char *>(fileBuffer),
                        writeLength, toFile, fromMachine, fileLength,
                        bytesCopied);

                    if (rc == kSTAFOk)
                    {
                        bytesCopied += writeLength;
                        gFSCopyManagerPtr->updateFileCopy(
                            copyDataPtr, bytesCopied);
                    }
                    else
                    {
                        fileCopyRC = rc;
                    }

                    bytesLeftToWrite -= writeLength;
                }

                rc = fileCopyRC;

                outFile.close();
            }
            else
            {
                // XXX: Don't have file length so assigning 0
                unsigned int fileLength = 0;

                gFSCopyManagerPtr->updateFileCopy1(
                    copyDataPtr, fileLength, kSTAFFSBinary);

                do
                {
                    connection->readUInt(writeLength);

                    if (writeLength != 0)
                    {
                        connection->read(fileBuffer, writeLength);

                        rc = writeFile(
                            outFile,reinterpret_cast<char *>(fileBuffer),
                            writeLength, toFile, fromMachine,
                            fileLength, bytesCopied);

                        if (rc == kSTAFOk)
                        {
                            connection->writeUInt(0);
                            bytesCopied += writeLength;
                            gFSCopyManagerPtr->updateFileCopy(copyDataPtr,
                                                              bytesCopied);
                        }
                        else
                        {
                            connection->writeUInt(kSTAFFileWriteError);
                            connection->writeString(toFile);
                        }
                    }
                } while ((writeLength != 0) && (outFile.good()));
             
                outFile.close();
            }
        }

        // Write the final ack indicating if the file copy was successful

        try
        {
            connection->writeUInt(rc);
        }
        catch (STAFConnectionIOException)
        {
            // Older clients will have already closed the connection, so we
            // need to ignore Connection IO exceptions at this point
        }

    } /****************** End try file transfer block *******************/
    catch (STAFConnectionIOException &e)
    {
        // Just log a Debug trace message as the problem is being handled on
        // FS service side

        STAFString message = "STAFProc::handleFileTransfer(): Connection "
            "terminated while copying to file '" + toFile +
            "' from machine '" + fromMachine + "'";

        e.trace(kSTAFTraceDebug, message.toCurrentCodePage()->buffer());
    }
    catch (STAFException &e)
    {
        // Log an Error trace message as an unexpected STAFException occurred

        STAFString message = "STAFProc::handleFileTransfer() while "
            "copying to file '" + toFile + "' from machine '" +
            fromMachine + "'";

        e.trace(message.toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        // Log an Error trace message as an unexpected exception occurred

        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFProc::handleFileTransfer() "
            "while copying to file '" + toFile + "' from machine '" +
            fromMachine + "'");
    }

    // Remove file copy entry from the map

    if (addedToCopyMap)
        gFSCopyManagerPtr->remove(copyDataPtr);
}


STAFRC_t writeFile(fstream &outFile, const char *fileBuffer, unsigned int writeLength,
                   const STAFString toFile, const STAFString fromMachine,
                   unsigned int fileLength, unsigned int currentPos)
{
    // If an error already occurred writing to the file, don't keep
    // writing to it

    if (!outFile.good())
        return kSTAFFileWriteError;

    // Write a block of data to the file

    /* Leave in (commented out) for future debugging purposes.
    // Make sure that currentPos is the correct current position in the file
    // (indicating the number of bytes written so far) now that we're no longer
    // using tellp() because it only returns an int which means it does not
    // support files >= 2G.
    unsigned int savePos = (unsigned int)outFile.tellp();
    if ((savePos != 4294967295) && (savePos != currentPos))
    {
        cout << "MISMATCH - writeFile - currentPos=" << STAFString(currentPos)
             << ", savePos=" << savePos << endl;
    }
    */

    outFile.write(fileBuffer, writeLength);

    if (outFile.good())
    {
#ifndef STAF_OS_NAME_AIX
        return kSTAFOk;  // Write was successful
#else
        // This check is needed for AIX to detect an out of disk space error.
        // For some odd reason, the file state is good when it really isn't
        // in this condition on AIX.

        // Also, on Linux PPC64-64, the seekg() sets !outFile to true and
        // causes a kSTAFFileWriteError (when no error really occurred) so
        // we should only do this extra check on AIX.        
 
        if ((fileLength != 0) && (fileLength <= INT_MAX))
        {
            outFile.seekg(0, ios::end);

            if (!outFile)
                return kSTAFFileWriteError;
            else
            {
                unsigned int newPos = (unsigned int)outFile.tellg();

                if ((newPos <= currentPos) && (writeLength > 0))
                    return kSTAFFileWriteError;
                else
                    return kSTAFOk;  // Write was successful
            }
        }
        else
        {
            // XXX: Don't have a way to check if out of disk space for large
            // files (>= 2G) or for files whose length isn't available
            // (e.g. text files)
            return kSTAFOk;
        }
#endif
    }
    
    // The write failed.  Retry write up to 20 times with a delay between
    // each attempt.

    for (int writeAttempt = 1;
         !outFile.good() && writeAttempt <= 20;
         writeAttempt++)
    {
        if (outFile.fail())
        {
            // Recoverable write error

            // Log a warning tracepoint message

            STAFString warningMsg(
                "STAFProc::writeFile() - Write attempt #" +
                STAFString(writeAttempt) + " failed while copying to file " +
                toFile + " from machine " + fromMachine + " after writing " +
                STAFString(currentPos) + " bytes");

            STAFTrace::trace(kSTAFTraceWarning, warningMsg);

            // Delay 1/2 second and retry write after clearing any error
            // flags and repositioning the file pointer

            STAFThreadManager::sleepCurrentThread(500);

            outFile.clear();

            if (currentPos <= INT_MAX)
            {
                outFile.seekp(currentPos, ios::beg);
                outFile.write(fileBuffer, writeLength);
            }
            else
            {
                // Reading a large file, so need to do multiple seekp()'s
                // because seekp() only accepts an integer for the offset

                outFile.seekp(INT_MAX, ios::beg);

                unsigned int setPos = currentPos - INT_MAX;

                while (setPos > INT_MAX)
                {
                    outFile.seekp(INT_MAX, ios::cur);
                    setPos = setPos - INT_MAX;
                }

                outFile.seekp((int)setPos, ios::cur);
                outFile.write(fileBuffer, writeLength);
            }
        }
        else if (outFile.bad())
        {
            // Unrecoverable write error.
            break;
        }
    }

    if (!outFile.good())
    {
        // Note:  If recovery fails, could try closing the file and retrying
        //        one more time

        // Unrecoverable write failure

        return kSTAFFileWriteError;
    }

#ifndef STAF_OS_NAME_AIX
    return kSTAFOk;  // Write was successful
#else
    // This check is needed for AIX to detect an out of disk space error.
    // For some odd reason, the file state is good when it really isn't
    // in this condition on AIX).
    
    // Also, on Linux PPC64-64, the seekg() sets !outFile to true and
    // causes a kSTAFFileWriteError (when no error really occurred) so
    // we should only do this extra check on AIX.        

    if ((fileLength != 0) && (fileLength <= INT_MAX))
    {
        outFile.seekg(0, ios::end);

        if (!outFile)
            return kSTAFFileWriteError;
        else
        {
            unsigned int newPos = (unsigned int)outFile.tellg();

            if ((newPos <= currentPos) && (writeLength > 0))
                return kSTAFFileWriteError;
            else
                return kSTAFOk;  // Write was successful
        }
    }
    else
    {
        // XXX: Don't have a way to check if out of disk space for large
        // files (>= 2G) or for files whose length isn't available
        // (e.g. text files)
        return kSTAFOk;
    }
#endif
}


STAFServiceResult checkTrustForFSCopy(unsigned int apiType, unsigned int level,
                                      const STAFConnectionProvider *provider,
                                      STAFConnectionPtr &connection,
                                      const STAFString &orgMachine,
                                      const STAFString &fromMachine,
                                      unsigned int copyType)
{
    STAFServiceResult serviceResult(kSTAFOk);
    
    // Get the network interface information for this machine
    STAFString myLogicalInterfaceID = STAFString();
    STAFString myPhysicalInterfaceID = STAFString();
    provider->getMyNetworkIDs(myLogicalInterfaceID, myPhysicalInterfaceID);

    // Compute the network interface information for the fromMachine
    STAFString fromInterface = provider->getName();
    STAFString fromLogicalInterfaceID;
    STAFString fromPhysicalInterfaceID;
    connection->getPeerNetworkIDs(fromLogicalInterfaceID,
                                  fromPhysicalInterfaceID);

    // Check if copying from a STAF 3.x machine or a STAF 2.x machine

    if (((copyType == kSTAFFSFileCopy) && (apiType == kNewAPI) && (level > 2)) ||
        ((copyType == kSTAFFSDirectoryCopy) && (level > 2)))
    {
        STAFHandle_t handle = connection->readUInt();
        STAFString authenticator = connection->readString();
        STAFString userIdentifier = connection->readString();
        STAFString authenticationData = connection->readString();

        STAFString orgInterface = connection->readString();
        STAFString orgLogicalInterfaceID = connection->readString();
        STAFString orgPhysicalInterfaceID = connection->readString();
        STAFString orgSTAFInstanceUUID = connection->readString();
        STAFString fromSTAFInstanceUUID = connection->readString();
        
        // Authenticate using the originating handle's authenticator and user

        if (authenticator != gNoneString)
        {
            // Issue an Authenticate request to the specified Authenticator
            // service.  If the authenticator is not registered, sets
            // authenticator = "none" and sets userIdentifier = "anonymous".

            STAFServiceResult result = gHandleManager.authenticate(
                orgMachine, handle, authenticator, userIdentifier,
                STAFHandleManager::kData, authenticationData);

            if (result.fRC != kSTAFOk)
            {
                authenticator = gNoneString;
                userIdentifier = gAnonymousString;
            }
        }

        STAFString authUser = authenticator + gSpecSeparator + userIdentifier;

        // Determine if this machine (the TO machine) is the same machine as
        // the ORG machine and if strict FS Copy trust is disabled

        if ((orgSTAFInstanceUUID == gSTAFInstanceUUID) && !gStrictFSCopyTrust)
        {
            // Don't do additional trust checking
            return serviceResult;
        }
        
        // Determine if this machine (the TO machine) is the same machine as
        // the FROM machine

        if (fromSTAFInstanceUUID == gSTAFInstanceUUID)
        {
            // Same machine so get trust for the local://local machine
            fromInterface = sLowerLocal;
            fromLogicalInterfaceID = sLowerLocal;
            fromPhysicalInterfaceID = sLowerLocal;
        }
        
        // Determine if this machine trusts the FROM machine

        unsigned int trustLevel = gTrustManager.getTrustLevel(
                fromInterface, fromLogicalInterfaceID, fromPhysicalInterfaceID,
                authenticator, userIdentifier);

        if (trustLevel < 4)
        {
            serviceResult.fRC = kSTAFAccessDenied;

            serviceResult.fResult = 
                "Trust level 4 required for FS COPY request.\n"
                "Source machine/user has trust level " +
                STAFString(trustLevel) + " on TOMACHINE " +
                myLogicalInterfaceID +
                "\nSource machine: " +
                fromInterface + gSpecSeparator + fromLogicalInterfaceID +
                " (" + fromInterface + gSpecSeparator +
                fromPhysicalInterfaceID + ")" +
                "\nSource user   : " + authUser;
        }
        else
        {
            // Determine if this machine (the TO machine) is the same machine
            // as the ORG machine (the machine that originated the request)

            if (orgSTAFInstanceUUID == gSTAFInstanceUUID)
            {
                orgInterface = sLowerLocal; 
                orgLogicalInterfaceID = sLowerLocal;
                orgPhysicalInterfaceID = sLowerLocal;
            }
            else if (orgSTAFInstanceUUID == fromSTAFInstanceUUID)
            {
                // This machine (the TO machine) is not the same machine as
                // the ORG machine, but the ORG machine is the same machine as
                // the FROM machine, so assign the network interface
                // information obtained for the fromMachine (so that org
                // interface/logicalID/physicalID are not "local" in regards
                // to the TO machine).
                orgInterface = fromInterface;
                orgLogicalInterfaceID = fromLogicalInterfaceID;
                orgPhysicalInterfaceID = fromPhysicalInterfaceID;
            }
            else if (orgLogicalInterfaceID != sLowerLocal)
            {
                // Assign the interface name used by the connection provider
                // on this machine
                orgInterface = provider->getName();
            }

            // Determine if this machine also trusts the ORG machine

            trustLevel = gTrustManager.getTrustLevel(
                orgInterface, orgLogicalInterfaceID, orgPhysicalInterfaceID,
                authenticator, userIdentifier);

            if (trustLevel < 4)
            {
                serviceResult.fRC = kSTAFAccessDenied;

                serviceResult.fResult =
                    "Trust level 4 required for FS COPY request.\n"
                    "Requesting machine/user has trust level " +
                    STAFString(trustLevel) + " on TOMACHINE " +
                    myLogicalInterfaceID +
                    "\nRequesting machine: " +
                    orgInterface + gSpecSeparator + orgLogicalInterfaceID +
                    " (" + orgInterface + gSpecSeparator +
                    orgPhysicalInterfaceID + ")" +
                    "\nRequesting user   : " + authUser;
            }
        }
    }
    else
    {
        // Old way to check trust for older api levels (STAF V2.x machines)

        if (!isLocalMachine(orgMachine, 1))
        {
            // Determine if this machine trusts the FROM machine

            unsigned int trustLevel = gTrustManager.getTrustLevel(
                    fromInterface, fromLogicalInterfaceID,
                    fromPhysicalInterfaceID, gNoneString, gAnonymousString);
            
            if (trustLevel < 4)
            {
                serviceResult.fRC = kSTAFAccessDenied;

                serviceResult.fResult = 
                    "Trust level 4 required for FS COPY request.\n"
                    "Source machine has trust level " +
                    STAFString(trustLevel) +
                    " on TOMACHINE " + myLogicalInterfaceID +
                    "\nSource machine: " + fromInterface + gSpecSeparator +
                    fromLogicalInterfaceID + " (" +
                    fromInterface + gSpecSeparator + fromPhysicalInterfaceID +
                    ")";
            }
            else
            {
                // Determine if this machine also trusts the machine that
                // originated the copy request.

                // Assume interface for the originating machine is the same as
                // from machine's interface (since it's not provided via the
                // connection).  Also can't determine trust based on
                // physical ID or user since not provided via the connection).

                STAFString orgMachineTrustDef = fromInterface +
                    gSpecSeparator + orgMachine;

                trustLevel = gTrustManager.getTrustLevel(orgMachineTrustDef);

                if (trustLevel < 4)
                {
                    serviceResult.fRC = kSTAFAccessDenied;
                
                    serviceResult.fResult = 
                        "Trust level 4 required for FS COPY request.\n"
                        "Requesting machine has trust level " +
                        STAFString(trustLevel) +
                        " on TOMACHINE " + myLogicalInterfaceID +
                        "\nRequesting machine: " + orgMachineTrustDef;
                }
            }
        }
    }
    
    return serviceResult;
}

// This API is used for handling garbage collection

void handleHandleTerminationRegistrationAPI(
    unsigned int level, const STAFConnectionProvider *provider,
    STAFConnectionPtr &connection, unsigned int &)
{
    connection->writeUInt(kSTAFOk);
    
    STAFHandle_t handle = connection->readUInt();
    
    STAFString machine = connection->readString();

    STAFString port = STAFString();

    if (level > 1)
    {
        port = connection->readString();
    }
    
    STAFString uuid = connection->readString();
    
    STAFString service = connection->readString();
    
    STAFString key = connection->readString();

    // Determine the endpoint for the machine

    STAFString endpoint = provider->getName() + gSpecSeparator +
        machine + port;

    // Add the notification entry for garbage collection

    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString(
            "HandleTerminationRegistrationAPI - addNotificationEntry(") +
            STAFString(handle) + ", " + endpoint + ", " + machine + ", " +
            uuid + ", " + service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }
    
    gHandleManagerPtr->addNotificationEntry(
        handle, endpoint, machine, uuid, service, key);
}


// This API is used for handling garbage collection

void handleHandleTerminationUnRegistrationAPI(
    unsigned int level, const STAFConnectionProvider *provider,
    STAFConnectionPtr &connection, unsigned int &)
{
    connection->writeUInt(kSTAFOk);
    
    STAFHandle_t handle = connection->readUInt();
    STAFString machine = connection->readString();
    STAFString uuid = connection->readString();
    STAFString service = connection->readString();
    STAFString key = connection->readString();

    // Delete the notification entry for garbage collection

    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString(
            "HandleTerminationUnRegistrationAPI - deleteNotification(") +
            STAFString(handle) + ", " + machine + ", " + uuid + ", " +
            service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }
    
    // Set the endpoint argument passed to deleteNotification() to the
    // machine value as its value shouldn't matter since the notification
    // should be removed from the local machine

    gHandleManagerPtr->deleteNotification(
        handle, machine, machine, uuid, service, key);
}

void handleRemoteHandleTerminatedAPI(unsigned int level,
                                     const STAFConnectionProvider *provider,
                                     STAFConnectionPtr &connection,
                                     unsigned int &)
{
}


void stafProcTerminate()
{
    cout << "STAFProc ending abnormally: terminate() called" << endl;
}


/******************************************************************************/
/* verifySTAFInstanceName - This method verifies that the specified STAF      */
/*    instance name is valid.  This check is performed before the STAF        */
/*    instance name is used (e.g. as part of a directory name, etc).          */
/*                                                                            */
/* Accepts: (In)  The STAF instance name                                      */    
/*          (Out) An error message if an error occurs                         */
/*                                                                            */
/* Returns:  0 if the STAF instance name is valid.                            */
/*           1 if the STAF instance name is invalid.  A STAF instance name is */
/*           invalid if it contains any of the following special characters   */
/*           (e.g .which cannot be used as a directory name):                 */
/*             ~!#$%^&*+={}[]|;':"?/<>\                                      */
/*           or if it contains any whitespace at the beginning or end         */
/*           or if it only contains whitespace.                               */
/******************************************************************************/
unsigned int verifySTAFInstanceName(const STAFString stafInstanceName,
                                    STAFString &errorBuffer)
{
    if (stafInstanceName.findFirstOf("~!#$%^&*+={}[]|;':\"?/<>\\") != 
        STAFString::kNPos)
    {
        errorBuffer = "It cannot contain any of the following characters: "
            "~!#$%^&*+={}[]|;':\"?/<>\\";
        return kSTAFInvalidValue;
    }
    else
    {
        STAFString strippedInstanceName = stafInstanceName;
        strippedInstanceName = strippedInstanceName.strip();

        if (strippedInstanceName.length() == 0)
        {
            errorBuffer = "It cannot be empty or just spaces";
            return kSTAFInvalidValue;
        }
        else if (!strippedInstanceName.strip().isEqualTo(stafInstanceName))
        {
            errorBuffer = "It cannot contain any leading or trailing "
                "whitespace.";
            return kSTAFInvalidValue;
        }
    }
    
    return kSTAFOk;
}

/******************************************************************************/
/* replaceStringInBuffer -  This is a replace routine for preallocated buffers*/
/*    This function replaces all occurences of searchStr in current with      */
/*    replaceStr storing the result in newBuffer.  It updates newLen to the   */
/*    last copied character in newBuffer.  It updates lastReplace to the index*/
/*    of the first character of the last occurence of replaceStr or -1 if no  */
/*    replacements were performed.                                            */
/*                                                                            */
/*    This function may be usable else where and may be moved                 */
/*                                                                            */
/* Accepts: (In)  A pointer to a char buffer of the string to be searched     */
/*          (In)  A pointer to a char buffer of the pattern to match          */
/*          (In)  A pointer to a char buffer of the pattern to replace with   */
/*          (In)  The length of the string to be searched                     */
/*          (In)  The length of the string that is to be matched              */
/*          (In)  The length of the string that is to replace                 */
/*          (Out) The length of the modified string                           */
/*          (Out) The starting index of the last replacement,                 */
/*                -1 if no replacement done                                   */
/*          (Out) A pointer to a char buffer of the modified string           */
/*                !!!! This string must be allocated prior to                 */
/*                     calling the function !!!!                              */
/*                                                                            */
/* Returns: void                                                              */
/******************************************************************************/

void replaceStringInBuffer (char* current, const char* searchStr, const char*
              replaceStr, int currentLen, int searchStrLen, int replaceStrLen,
              int* newLen, int* lastReplace, char* newBuffer)
{
    *lastReplace = -1;
    *newLen = 0;
    int offset = currentLen - searchStrLen;
    int i, j, last, numEOL;
    last = -1;
    numEOL = 0;
    bool isMatch;

    for(i = 0; i < offset; i++)
    {
        isMatch = true;

        for(j = 0; j < searchStrLen; j++)
            isMatch &= (current[j] == searchStr[j]);

        if (isMatch)
        {
            last = i;

            for(j = 0; j < replaceStrLen; j++)
                newBuffer[j] = replaceStr[j];

            newBuffer += replaceStrLen;
            current += searchStrLen;
            i += (searchStrLen-1);
            numEOL++;
        }
        else
        {
            *newBuffer = *current;
            newBuffer++;
            current++;
        }
    }

    *lastReplace = last + ((numEOL - 1) * (replaceStrLen - searchStrLen));

    if ((i == offset) || (last == -1))
        *lastReplace = -1;

    j = searchStrLen - (i - offset);

    for (i = 0; i < j; i++)
        newBuffer[i] = current[i];

    *newLen = currentLen + (numEOL * (replaceStrLen - searchStrLen));

    return;
}


/******************************************************************************/
/* rollBuffer - This is a buffer rolling routine for preallocated buffers.    */
/*              This function copies the the last (rollLen) characters to the */
/*              front of buffer.                                              */
/*                                                                            */
/* Accepts: (Out)  A pointer to a char buffer of the string to be rolled      */
/*          (In)  The length of the string to be rolled                       */
/*          (In)  The length of the roll                                      */
/*                                                                            */
/* Returns: void                                                              */
/******************************************************************************/

void rollBuffer(char * buffer, int bufferLen, int rollLen)
{
    int endIndx = bufferLen - rollLen;
    char * end = buffer + endIndx;
    int i;

    for (i = 0; i < rollLen; i++)
        buffer[i] = end[i];

    return;
}

/******************************************************************************/
/* Note that the following methods are exact copies of methods from           */
/* STAFFSService.cpp:                                                         */
/*  - updateRemoveDirResultString (copy of updateRemoveResultString)          */
/*              front of buffer.                                              */
/*  - removeDirChildren (copy of removeChildren)                              */
/*  - removeDir (copy of recurseRemove)                                       */
/******************************************************************************/

void updateRemoveDirResultString(STAFString &result, STAFFSEntryPtr &entry,
                                 STAFRC_t rc, unsigned int osRC)
{
    if (rc != kSTAFOk)
    {
        result += entry->path().asString() + sSemiColon + rc;

        if (rc == kSTAFBaseOSError)
            result += sColon + osRC;

        result += *gLineSeparatorPtr;
    }
}

STAFRC_t removeDirChildren(STAFFSEntryPtr entry, const STAFString &namePattern,
                           const STAFString &extPattern,
                           unsigned int entryTypesUInt,
                           STAFFSCaseSensitive_t caseSensitive,
                           STAFString &result)
{
    STAFFSEnumPtr childEnum = entry->enumerate(
        namePattern, extPattern, STAFFSEntryType_t(entryTypesUInt),
        kSTAFFSNoSort, caseSensitive);

    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    for (; childEnum->isValid(); childEnum->next())
    {
        STAFFSEntryPtr entry = childEnum->entry();
        rc = entry->remove(&osRC);
        updateRemoveDirResultString(result, entry, rc, osRC);
    }

    return kSTAFOk;
}


STAFRC_t removeDir(STAFFSEntryPtr entry, const STAFString &namePattern,
                   const STAFString &extPattern, unsigned int entryTypesUInt,
                   STAFFSCaseSensitive_t caseSensitive, STAFString &result)
{
    STAFFSEnumPtr childDirEnum = entry->enumerate(kUTF8_STAR, kUTF8_STAR,
                                                  kSTAFFSDirectory);
    STAFRC_t rc = kSTAFOk;

    for (; childDirEnum->isValid(); childDirEnum->next())
    {
        rc = removeDir(childDirEnum->entry(), namePattern, extPattern,
                       entryTypesUInt, caseSensitive, result);
    }

    rc = removeDirChildren(entry, namePattern, extPattern, entryTypesUInt,
                           caseSensitive, result);

    return kSTAFOk;
}
