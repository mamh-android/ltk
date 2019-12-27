/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_HandleManager
#define STAF_HandleManager

#include <map>
#include <list>
#include <vector>
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAF.h"
#include "STAFProc.h"
#include "STAFTimestamp.h"
#include "STAFVariablePool.h"
#include "STAFMutexSem.h"
#include "STAFHandleQueue.h"

// STAFHandleManager - This class manages all the process handles used by
//                     STAFProc.  It handles the semantics of adding,
//                     registering, unregistering, and removing the various
//                     types of handles.  It also provides facilities to
//                     query information about a given handle, as well as to
//                     retrieve a set of handles.


class STAFHandleManager
{
public:

    // Public types

    // kInProcess         - Represents a handle for a service executing with
    //                      the STAFProc process
    // kPending           - Represents an unregistered handle for a process
    //                      that was started via STAF
    // kPendingRegistered - Represents a registered handle for a process that
    //                      was started via STAF
    // kRegistered        - Represents a registered handle for a process that
    //                      was not started via STAF
    // kStatic            - Represents a handle shared across multiple
    //                      native processes

    enum HandleState { kInProcess,  kPending, kPendingRegistered,
                       kRegistered, kStatic };

    struct HandleData
    {
        HandleData() : lastUsedTimestamp(STAFTimestamp::now()),
                       authenticator(gNoneString),
                       userIdentifier(gAnonymousString)
        { /* Do Nothing */ }


        // Note: We need two constructors here, because VC++ V6 doesn't like the
        //       declaration of default values for variablePoolPtr and
        //       handleQueuePtr

        HandleData(STAFHandle_t aHandle, STAFProcessID_t aPID,
                   STAFProcessHandle_t aProcHandle, HandleState aState,
                   STAFTimestamp aTimestamp, const STAFString &aName)
            : handle(aHandle), pid(aPID), procHandle(aProcHandle),
              state(aState), lastUsedTimestamp(aTimestamp), name(aName),
              lowerName(aName.toLowerCase()),
              variablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT),
              handleQueuePtr(new STAFHandleQueue(gMaxQueueSize),
                             STAFHandleQueuePtr::INIT),
              authenticator(gNoneString),
              userIdentifier(gAnonymousString)
        { /* Do Nothing */ }

        HandleData(STAFHandle_t aHandle, STAFProcessID_t aPID,
                   STAFProcessHandle_t aProcHandle, HandleState aState,
                   STAFTimestamp aTimestamp, const STAFString &aName,
                   STAFVariablePoolPtr varPool)
             : handle(aHandle), pid(aPID), procHandle(aProcHandle),
               state(aState), lastUsedTimestamp(aTimestamp), name(aName),
               lowerName(aName.toLowerCase()), variablePoolPtr(varPool),
               handleQueuePtr(new STAFHandleQueue(gMaxQueueSize),
                              STAFHandleQueuePtr::INIT),
               authenticator(gNoneString),
               userIdentifier(gAnonymousString)
        { /* Do Nothing */ }

        STAFHandle_t handle;
        STAFProcessID_t pid;
        STAFProcessHandle_t procHandle;
        HandleState state;
        STAFTimestamp lastUsedTimestamp;
        STAFString name;
        STAFString lowerName;
        STAFVariablePoolPtr variablePoolPtr;
        STAFHandleQueuePtr handleQueuePtr;
        STAFString authenticator;
        STAFString userIdentifier;
        STAFString authenticationData;
    };

    typedef std::map<STAFHandle_t, HandleData> HandleList;
    
    struct NotificationData
    {
        NotificationData(STAFHandle_t aHandle, STAFString aEndpoint,
                         STAFString aMachine, STAFString aUuid,
                         STAFString aKey, STAFString aNotifyService)
            : handle(aHandle), endpoint(aEndpoint), machine(aMachine),
              uuid(aUuid), key(aKey), notifyService(aNotifyService)
        { /* Do Nothing */ }
        
        STAFHandle_t handle;
        STAFString endpoint;
        STAFString machine;
        STAFString uuid;
        STAFString key;
        STAFString notifyService;
    };
    
    typedef std::list<NotificationData> NotificationList;

    STAFServiceResult addNotification(
        STAFHandle_t handle, const STAFString &endpoint,
        const STAFString &machine, const STAFString &uuid,
        const STAFString &service, const STAFString &key);
                             
    STAFRC_t addNotificationEntry(
        STAFHandle_t &handle, const STAFString &endpoint,
        const STAFString &machine, const STAFString &uuid,
        const STAFString &service, const STAFString &key);
 
    STAFRC_t deleteNotification(
        STAFHandle_t handle, const STAFString &endpoint,
        const STAFString &machine, const STAFString &uuid,
        const STAFString &service, const STAFString &key);

    NotificationList getNotificationListCopy();
    
    struct PollingData
    {
        PollingData(STAFHandle_t aHandle, STAFString aEndpoint,
                    STAFString aMachine, STAFString aUuid,
                    STAFString aKey, STAFString aNotifyService)
            : handle(aHandle), endpoint(aEndpoint), machine(aMachine),
              uuid(aUuid), key(aKey), notifyService(aNotifyService)
        { /* Do Nothing */ }
        
        STAFHandle_t handle;
        STAFString endpoint;  // Endpoint of machine to poll
        STAFString machine; // machine to poll, requestor is always local
        STAFString uuid;
        STAFString key;
        STAFString notifyService;
    };
    
    typedef std::list<PollingData> PollingDataList;
    
    STAFRC_t addPollingEntry(
        STAFHandle_t &handle, const STAFString &endpoint,
        const STAFString &machine, const STAFString &uuid,
        const STAFString &service, const STAFString &key);
        
    STAFRC_t deletePolling(
        STAFHandle_t &handle, const STAFString &machine,
        const STAFString &uuid, const STAFString &service,
        const STAFString &key);
    
    PollingDataList getPollingDataListCopy();
        
    // Public Authentication types

    // kCredentials    - Represents CREDENTIALS being passed in to the
    //                   Authenticator service via an AUTHENTICATE request
    // KData           - Represents DATA being passed in to the Authenticator
    //                   service via an AUTHENTICATE request

    enum AuthenticateType { kCredentials, kData };

    // Defines a structure that contains the cached authentication information
    // stored for remote machines/handles.

    struct AuthenticationInfo
    {
        AuthenticationInfo()
        { /* Do Nothing */ }

        AuthenticationInfo(const STAFString &theAuthenticator,
                           const STAFString &theUserIdentifier,
                           const STAFString &theAuthenticationData)
            : authenticator(theAuthenticator.toUpperCase()),
              userIdentifier(theUserIdentifier),
              authenticationData(theAuthenticationData)
        { /* Do Nothing */ }

        STAFString authenticator;    
        STAFString userIdentifier;   
        STAFString authenticationData;
    };
    
    // RemoteAuthHandleMap contains a map of remote machines/handles and
    // their cached authentication information.
    // The map's key is machine (lower-case) + ";" + handle

    typedef std::map<STAFString, AuthenticationInfo> RemoteAuthHandleMap;

    STAFHandleManager();

    STAFString name(STAFHandle_t handle);
    STAFRC_t variablePool(STAFHandle_t handle, STAFVariablePoolPtr &pool);
    STAFRC_t handleQueue(STAFHandle_t handle, STAFHandleQueuePtr &queue);
    STAFRC_t getHandleData(STAFHandle_t handle, HandleData &data);

    std::vector<STAFHandle_t> handlesWithName(STAFString name);

    STAFRC_t updateTimestamp(STAFHandle_t handle, STAFProcessID_t pid);

    STAFRC_t updateStaticHandle(STAFHandle_t handle, STAFProcessID_t pid,
                                STAFProcessHandle_t procHandle);

    STAFServiceResult addAndGetPendingHandle(STAFHandle_t &handle,
                                             STAFProcessID_t pid,
                                             STAFProcessHandle_t procHandle,
                                             STAFVariablePoolPtr pool);
    STAFServiceResult addAndGetStaticHandle(STAFHandle_t &handle,
                                            const STAFString &name);
    STAFServiceResult addAndGetStaticHandle(STAFHandle_t &handle,
                                            const STAFString &name,
                                            STAFVariablePoolPtr pool);
    STAFRC_t registerHandle(STAFHandle_t &handle, STAFProcessID_t pid,
                            const STAFString &name);
    STAFRC_t unRegister(STAFHandle_t handle, STAFProcessID_t pid);
    void handleRemoved(STAFHandle_t handle);
    STAFRC_t removePendingHandle(STAFHandle_t handle, STAFProcessID_t pid);
    STAFRC_t removeStaticHandle(STAFHandle_t handle);

    HandleList getHandleListCopy();

    // Caution: The getHandleManagerSem() API is only intended for the use of
    //          the process service.  Do not use this API unless you REALLY
    //          know what you are doing.

    STAFMutexSem &getHandleManagerSem();

    STAFHandle_t getHandleListSize();
    STAFUInt64_t getTotalHandles();
    STAFHandle_t getMaxHandleNumber();
    STAFHandle_t getMinHandleNumber();
    STAFHandle_t getMaxActiveHandles();
    unsigned int getResetCount();

    STAFServiceResult authenticate(const STAFString &machine,
                                   const STAFHandle_t handle,
                                   STAFString &authenticator,
                                   STAFString &userIdentifier,
                                   const AuthenticateType authenticateType,
                                   const STAFString &authenticationData);

    STAFServiceResult unAuthenticate(const STAFHandle_t handle);

    STAFRC_t isAuthenticated(const STAFHandle_t handle);

    STAFRC_t getAuthenticationInfo(const STAFHandle_t handle,
                                   STAFString &authenticator,
                                   STAFString &userIdentifier,
                                   STAFString &authenticationData);

    STAFRC_t cacheAuthenticationInfo(const STAFString &machine,
                                     const STAFHandle_t handle, 
                                     const STAFString &authenticator,
                                     const STAFString &userIdentifier,
                                     const STAFString &authenticationData);

private:

    // Don't allow copy construction or assignment
    STAFHandleManager(const STAFHandleManager &);
    STAFHandleManager &operator=(const STAFHandleManager &);

    STAFServiceResult assignNextHandleNumber();

    // This is the callback
    static void processTerminated(STAFProcessID_t pid,
                                  STAFProcessHandle_t procHandle,
                                  unsigned int rc,
                                  void *);

    // This does the actual work
    void handleProcessTerminated(STAFProcessID_t pid,
                                 STAFProcessHandle_t procHandle);

    void gcPolling();
    friend unsigned int HandleMonitorThread(void *data);

    STAFHandle_t        fNextHandle;
    STAFMutexSem        fHandleListSem;
    HandleList          fHandleList;
   
    // Contains the total number of handles that have been created
    STAFUInt64_t fTotalHandles;

    // Contains the number of times the maximum value for the handle number
    // has been reached and it has been reset back to 2
    unsigned int fResetCount;

    STAFMutexSem        fRemoteHandleMapSem;
    RemoteAuthHandleMap fRemoteAuthHandleMap;
    STAFMutexSem        fNotificationListSem;
    NotificationList    fNotificationList;
    STAFMutexSem        fPollingDataListSem;
    PollingDataList     fPollingDataList;
};

#endif
