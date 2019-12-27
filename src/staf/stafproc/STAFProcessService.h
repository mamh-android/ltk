/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ProcessService
#define STAF_ProcessService

// Note: We need to include queues from os2.h here, otherwise the template
//       generation in Visual Age won't know what an HQUEUE is.

#include "STAF.h"
#include <map>
#include <vector>
#include "STAFString.h"
#include "STAFTimestamp.h"
#include "STAFService.h"
#include "STAFEventSem.h"
#include "STAFNotificationList.h"
#include "STAFCommandParser.h"
#include "STAFFileSystem.h"

class STAFProcessService : public STAFService
{
public:

    STAFProcessService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFProcessService();

    enum ProcessType { kCommand, kShell };
    enum ProcessState { kRunning, kComplete };
    enum ProcessSync { kWait, kAsync };
    enum ProcessHandleType { kPending, kStatic };

    // The deleteFlag is used to indicate if the temp file should be
    // deleted.  If true, indicates it should be deleted.  If false,
    // indicates it should not be deleted.  Since wait and notify occur
    // on different threads, and the notify happens before the wait
    // finishes, if the WAIT option is specified (and it doesn't timeout),
    // the temp files should not be deleted until the contents of the
    // returned files have been provided in the start result.
    struct TempFileInfo
    {
        STAFString name;   // Fully-qualified name of temporary file
        bool deleteFlag;
    };

    struct Process
    {
        typedef std::vector<STAFString> FileList;

        STAFString workload;
        ProcessType processType;
        STAFString shellCommand;
        STAFString command;
        STAFString workdir;
        STAFString parms;
        STAFString title;
        STAFString username;
        STAFTimestamp startStamp;
        STAFTimestamp endStamp;
        STAFHandle_t handle;
        ProcessHandleType handleType;
        STAFProcessID_t pid;
        STAFProcessHandle_t procHandle;
        unsigned int RC;
        ProcessState state;
        STAFProcessStopMethod_t stopMethod;
        STAFProcessConsoleFocus_t consoleFocus;
        STAFEventSemPtr notify;
        STAFNotificationListPtr notificationList;
        STAFString key;
        FileList retFileList;
        STAFMutexSemPtr tempFileMutex;
        TempFileInfo stdoutTempFileInfo;
        TempFileInfo stderrTempFileInfo;
        STAFString authenticator;
        STAFString userIdentifier;
        unsigned int maxReturnFileSize;

        Process() : startStamp(STAFTimestamp::now()),
                    endStamp(STAFTimestamp::now()),
                    tempFileMutex(STAFMutexSemPtr(
                        new STAFMutexSem(), STAFMutexSemPtr::INIT)),
                    maxReturnFileSize(gMaxReturnFileSize)
        {  /* Do Nothing */ }
    };

    typedef STAFRefPtr<Process> ProcessPtr;
    typedef std::map<STAFHandle_t, ProcessPtr> ProcessList;

    static void processTerminationCallback(STAFProcessID_t pid,
                                           STAFProcessHandle_t procHandle,
                                           unsigned int retCode,
                                           void *);
    static unsigned int sendNotificationCallback(void *);

    // Manipulates process temp directory (stores temp stdout/stderr files)
    static STAFString getTempDirectory();
    static STAFRC_t setTempDirectory(const STAFString &tempDirectory);

    // Manipulates process stop method
    static STAFRC_t getStopMethodFromString(STAFProcessStopMethod_t &stopType,
                 const STAFString &methodString);
    static STAFRC_t setDefaultStopMethod(STAFProcessStopMethod_t stopType);
    static STAFProcessStopMethod_t getDefaultStopMethod();
    static STAFString getDefaultStopMethodAsString();

    // Manipulates process console focus
    static STAFRC_t getConsoleFocusFromString(
        STAFProcessConsoleFocus_t &consoleFocus,
        const STAFString &focusString);
    static STAFRC_t setDefaultConsoleFocus(
        STAFProcessConsoleFocus_t consoleFocus);
    static STAFProcessConsoleFocus_t getDefaultConsoleFocus();
    static STAFString getDefaultConsoleFocusAsString();
    static STAFString getConsoleFocusAsString(
        STAFProcessConsoleFocus_t consoleFocus);

    // Manipulates process authentication mode
    static STAFRC_t getAuthModeFromString(
                 STAFProcessAuthenticationMode_t &authMode,
                 const STAFString &modeString);
    static STAFRC_t setAuthMode(STAFProcessAuthenticationMode_t authMode,
                 unsigned &osRC);
    static STAFProcessAuthenticationMode_t getAuthMode();
    static STAFString getAuthModeAsString();

    // Manipulates process authentication action (for disabled)
    static STAFRC_t getDefaultDisabledAuthActionFromString(
                 STAFProcessDisabledAuthAction_t &authAction,
                 const STAFString &actionString);
    static STAFRC_t setDefaultDisabledAuthAction(
                 STAFProcessDisabledAuthAction_t authAction);
    static STAFProcessDisabledAuthAction_t getDefaultDisabledAuthAction();
    static STAFString getDefaultDisabledAuthActionAsString();

    // Sets/gets default username and password for authentication
    static STAFRC_t setDefaultAuthUsername(const STAFString &username);
    static STAFString getDefaultAuthUsername();

    static STAFRC_t setDefaultAuthPassword(const STAFString &password);
    static STAFString getDefaultAuthPassword();

    // Sets/gets default process shell command
    static STAFRC_t setDefaultShellCommand(const STAFString &shellCommand);
    static STAFString getDefaultShellCommand();

    // Sets/gets default process same console shell command
    static STAFRC_t setDefaultSameConsoleShell(const STAFString &shellCommand);
    static STAFString getDefaultSameConsoleShell();

    // Sets/gets default process new console shell command
    static STAFRC_t setDefaultNewConsoleShell(const STAFString &shellCommand);
    static STAFString getDefaultNewConsoleShell();


private:

    // Don't allow copy construction or assignment
    STAFProcessService(const STAFProcessService &);
    STAFProcessService &operator=(const STAFProcessService &);

    static STAFProcessAuthenticationMode_t fAuthMode;
    static STAFProcessStopMethod_t         fDefaultStopMethod;
    static STAFProcessConsoleFocus_t       fDefaultConsoleFocus;
    static STAFProcessDisabledAuthAction_t fAuthAction;
    static STAFString                      fDefaultAuthUsername;
    static STAFString                      fDefaultAuthPassword;
    static STAFString                      fDefaultShellCommand;
    static STAFString                      fDefaultSameConsoleShell;
    static STAFString                      fDefaultNewConsoleShell;
    static STAFString                      fTempDirectory;
    static unsigned int                    fEnvVarCaseSensitive;

    // Maximum value for a pid (Process ID)
    static STAFProcessID_t                 fMaxPid;

    static ProcessPtr fNotifyProcess;
    static STAFEventSem fNotifySem;
    void sendNotification();

    void handleProcessTermination(STAFProcessID_t pid, unsigned int retCode);

    STAFServiceResult handleStart(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleStop(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleKill(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleSet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleFree(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleNotifyRegistration(
                          unsigned int isRegister,
                          const STAFServiceRequest &requestInfo);
    STAFServiceResult handleNotifyList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);
    STAFServiceResult deleteTempFiles(Process &process,
                                      bool forceFlag = false);

    ProcessList fProcessList;
    STAFMutexSem fProcessListSem;
    STAFCommandParser fStartParser;
    STAFCommandParser fStopParser;
    STAFCommandParser fKillParser;
    STAFCommandParser fQueryParser;
    STAFCommandParser fNotifyListParser;
    STAFCommandParser fNotifyRegistrationParser;
    STAFCommandParser fListParser;
    STAFCommandParser fSetParser;
    STAFCommandParser fFreeParser;

    // Map class definitions for marshalled results
    STAFMapClassDefinitionPtr fCompletionMapClass;
    STAFMapClassDefinitionPtr fReturnFileMapClass;
    STAFMapClassDefinitionPtr fProcessInfoMapClass;
    STAFMapClassDefinitionPtr fListProcessMapClass;
    STAFMapClassDefinitionPtr fListLongProcessMapClass;
    STAFMapClassDefinitionPtr fSettingsMapClass;
    STAFMapClassDefinitionPtr fFreeMapClass;
    STAFMapClassDefinitionPtr fStopMapClass;
    STAFMapClassDefinitionPtr fNotifieeMapClass;
};

#endif
