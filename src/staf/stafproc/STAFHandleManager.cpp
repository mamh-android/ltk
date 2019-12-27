/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFHandleManager.h"
#include "STAFProcess.h"
#include "STAFTrace.h"
#include "STAFServiceManager.h"
#include "STAFProcUtil.h"
#include "STAFHandleService.h"
#include "STAFDiagManager.h"
#include "STAFConnectionManager.h"
#include "STAFThreadManager.h"

unsigned int HandleMonitorThread(void *);

static STAFHandleManager *pHandleManager = 0;

// Note that the maximum value for a handle number must not exceed the
// maximum value for the STAFHandle_t type (unsigned int), nor the
// maximum value for the JNI type (jint) used for the handle field in the
// STAFServiceInterfaceLevel30 RequestInfo Java class.

// XXX: If we changed the STAF JNI interface to use jlong instead of jint
//      for handle, then could use UINT_MAX instead of INT_MAX

static STAFHandle_t MAX_HANDLE_NUMBER = INT_MAX; // (e.g. 2147483647)
static STAFHandle_t MIN_HANDLE_NUMBER = 1;

// Maximum number of active handles that can reside in the handle list
// based on the range (minimum and maximum values) for handle number values
static STAFHandle_t MAX_ACTIVE_HANDLES =
    MAX_HANDLE_NUMBER - MIN_HANDLE_NUMBER + 1;


static STAFThreadManager &getHandleThreadManager()
{
  static STAFThreadManager theThreadManager(1);
  return theThreadManager;
}

STAFHandleManager::STAFHandleManager() : fNextHandle(MIN_HANDLE_NUMBER - 1),
                                         fResetCount(0),
                                         fTotalHandles(0)
{
    pHandleManager = this;

    getHandleThreadManager().dispatch(HandleMonitorThread, 0);
}


STAFServiceResult STAFHandleManager::assignNextHandleNumber()
{
    // Note: This method should only be used by other STAFHandleManager
    // methods to set fNextHandle to the next available handle number
    // before creating a HandleData object and adding it to the handle list.
    // It should only be called if already have obtained a lock on
    // fHandleListSem.
    //
    // If the maximum number of active handles is exceeded, returns RC 59. 

    if (fNextHandle < MAX_HANDLE_NUMBER)
    {
        ++fNextHandle;
    }
    else
    {
        // Handle 1 is reserved for STAFProc so we should skip it
        fNextHandle = MIN_HANDLE_NUMBER + 1;
        fResetCount++;
    }

    if (fResetCount)
    {
        if (fHandleList.size() < MAX_ACTIVE_HANDLES)
        {
            // Find the next available request number (the next one that is
            // not in the request map)

            while (true)
            {
                if (fHandleList.find(fNextHandle) == fHandleList.end())
                    break;  // Handle number not found so it is available

                if (fNextHandle < MAX_HANDLE_NUMBER)
                {
                    ++fNextHandle;
                }
                else
                {
                    // Handle 1 is reserved for STAFProc so we should skip it
                    fNextHandle = MIN_HANDLE_NUMBER + 1; 
                    fResetCount++;
                }
            }
        }
        else
        {
            STAFString errorMsg = STAFString(
                "STAFHandleManager::assignNextHandleNumber():  Exceeded the "
                "maximum number of active handles, ") + MAX_ACTIVE_HANDLES +
                ", after resetting the handle number " + fResetCount +
                " times";

            STAFTrace::trace(kSTAFTraceError, errorMsg);

            return STAFServiceResult(kSTAFMaximumHandlesExceeded, errorMsg);
        }
    }

    return STAFServiceResult(kSTAFOk);
}


void STAFHandleManager::processTerminated(STAFProcessID_t pid,
                                           STAFProcessHandle_t procHandle,
                                           unsigned int rc, void *)
{
    pHandleManager->handleProcessTerminated(pid, procHandle);
}


unsigned int callHandleRemovedCallback(void *handleAsVoid)
{
    STAFHandle_t *handlePtr = reinterpret_cast<STAFHandle_t *>(handleAsVoid);

    gHandleManagerPtr->handleRemoved(*handlePtr);
    
    delete handlePtr;

    return 0;
}


void postHandleQueueNotifySem(STAFHandleQueuePtr handleQueuePtr)
{
    // Called after a handle is removed from the handle list to post the
    // handle queue's event semaphore so that if any QUEUE GET/PEEK WAIT
    // requests are waiting for a message to appear on this handle's queue,
    // they'll return with a Handle Does Not Exist error instead of waiting
    // forever or until a specified wait time expires

    STAFMutexSemLock queueLock(handleQueuePtr->fQueueSem);
    handleQueuePtr->fNotify->post();
}


void STAFHandleManager::handleProcessTerminated(STAFProcessID_t pid,
                                                STAFProcessHandle_t procHandle)
{
    HandleList::iterator handleIter;

    // Basically, we loop through the list finding a registered handle with
    // the specified pid and procHandle.  We only have to worry about registered
    // handles here.  Pending handles are removed by the Process service, and we
    // never have static or in-process handles monitored by the ProcessManager.

    STAFMutexSemLock handleLock(fHandleListSem);

    for(handleIter = fHandleList.begin();
        (handleIter != fHandleList.end()) &&
            ((handleIter->second.pid != pid) ||
             (handleIter->second.procHandle != procHandle) ||
             (handleIter->second.state != kRegistered));
        ++handleIter)
    { /* Do Nothing */ }
    
    if (handleIter != fHandleList.end())
    {
        // Save the handle 
        STAFHandle_t savedHandle = handleIter->second.handle;

        // Remove the handle from the list and post the handle queue's
        // notify event semaphore

        STAFHandleQueuePtr savedQueuePtr = handleIter->second.handleQueuePtr;

        fHandleList.erase(handleIter);

        postHandleQueueNotifySem(savedQueuePtr);

        // Dispatch calling the handleRemoved(handle) method so that it runs on
        // another thread because it needs a lock on fHandleListSem and because
        // it contains a 100 ms wait in a loop for deleting notifications for
        // garbage collection.

        STAFRC_t rc = getHandleThreadManager().dispatch(
            callHandleRemovedCallback, new STAFHandle_t(savedHandle));

        if (rc != kSTAFOk)
        {
            STAFTrace::trace(
                kSTAFTraceError,
                "In STAFHandleManager::handleProcessTerminated(): "
                "Error dispatching a thread, RC: " + rc);
        }
    }
}


STAFString STAFHandleManager::name(STAFHandle_t handle)
{
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) != fHandleList.end())
        return fHandleList[handle].name;

    return STAFString();
}


STAFRC_t STAFHandleManager::variablePool(STAFHandle_t handle,
                                         STAFVariablePoolPtr &pool)
{
    STAFRC_t rc = kSTAFOk;
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) != fHandleList.end())
        pool = fHandleList[handle].variablePoolPtr;
    else rc = kSTAFHandleDoesNotExist;

    return rc;
}


STAFRC_t STAFHandleManager::handleQueue(STAFHandle_t handle,
                                        STAFHandleQueuePtr &queue)
{
    STAFRC_t rc = kSTAFOk;
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) != fHandleList.end())
        queue = fHandleList[handle].handleQueuePtr;
    else rc = kSTAFHandleDoesNotExist;
    
    return rc;
}


STAFRC_t STAFHandleManager::getHandleData(STAFHandle_t handle,
                                          HandleData &data)
{
    STAFRC_t rc = kSTAFOk;
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) != fHandleList.end())
        data = fHandleList[handle];
    else
        rc = kSTAFHandleDoesNotExist;

    return rc;
}


std::vector<STAFHandle_t> STAFHandleManager::handlesWithName(STAFString name)
{
    name.lowerCase();

    std::vector<STAFHandle_t> handles;
    STAFMutexSemLock handleLock(fHandleListSem);
    HandleList::iterator handleIter;

    // XXX: This should be able to be done with some form of copy_if

    for(handleIter = fHandleList.begin(); handleIter != fHandleList.end();
        ++handleIter)
    {
        if (name == handleIter->second.name.toLowerCase())
            handles.push_back(handleIter->second.handle);
    }

    return handles;
}

STAFRC_t STAFHandleManager::updateTimestamp(STAFHandle_t handle,
                                            STAFProcessID_t pid)
{
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) == fHandleList.end())
        return kSTAFHandleDoesNotExist;

    HandleData &theHandle = fHandleList[handle];

    if ((theHandle.state != kStatic) && (theHandle.pid != pid))
        return kSTAFInvalidHandle;

    theHandle.lastUsedTimestamp = STAFTimestamp::now();

    return kSTAFOk;
}


STAFRC_t STAFHandleManager::updateStaticHandle(STAFHandle_t handle,
                                               STAFProcessID_t pid,
                                               STAFProcessHandle_t procHandle)
{
    // Note: This method should only be used by STAFProcessService.  It will
    //       obtain a lock on our fHandleListSem before calling this method.
    //       Should anyone else need to call this method, they must also lock
    //       our fHandleListSem before calling this method.

    if (fHandleList.find(handle) == fHandleList.end())
        return kSTAFHandleDoesNotExist;

    HandleData &theHandle = fHandleList[handle];

    if (theHandle.state != kStatic)
        return kSTAFInvalidHandle;

    theHandle.pid = pid;
    theHandle.procHandle = procHandle;

    return kSTAFOk;
}


STAFServiceResult STAFHandleManager::addAndGetPendingHandle(
    STAFHandle_t &handle, STAFProcessID_t pid, STAFProcessHandle_t procHandle,
    STAFVariablePoolPtr pool)
{
    // Note: This method should only be used by STAFProcessService.  It will
    //       obtain a lock on our fHandleListSem before calling this method.
    //       Should anyone else need to call this method, they must also lock
    //       our fHandleListSem before calling this method.

    STAFServiceResult result = assignNextHandleNumber();

    if (result.fRC) return result;

    HandleData theHandle(fNextHandle, pid, procHandle, kPending,
                         STAFTimestamp::now(), STAFString(), pool);

    fHandleList[theHandle.handle] = theHandle;
    handle = theHandle.handle;
    fTotalHandles++;

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFHandleManager::addAndGetStaticHandle(
    STAFHandle_t &handle, const STAFString &name)
{
    STAFMutexSemLock handleLock(fHandleListSem);

    STAFServiceResult result = assignNextHandleNumber();

    if (result.fRC != kSTAFOk) return result;

    HandleData theHandle(fNextHandle, 0, 0, kStatic,
                         STAFTimestamp::now(), name);

    fHandleList[theHandle.handle] = theHandle;
    handle = theHandle.handle;
    fTotalHandles++;

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFHandleManager::addAndGetStaticHandle(
    STAFHandle_t &handle, const STAFString &name, STAFVariablePoolPtr pool)
{
    STAFMutexSemLock handleLock(fHandleListSem);

    STAFServiceResult result = assignNextHandleNumber();

    if (result.fRC) return result;

    HandleData theHandle(fNextHandle, 0, 0, kStatic,
                         STAFTimestamp::now(), name, pool);

    fHandleList[theHandle.handle] = theHandle;
    handle = theHandle.handle;
    fTotalHandles++;

    return STAFServiceResult(kSTAFOk);
}

STAFServiceResult STAFHandleManager::addNotification(
    STAFHandle_t handle, const STAFString &endpoint,
    const STAFString &machine, const STAFString &uuid,
    const STAFString &service, const STAFString &key)
{
    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString("Starting addNotification(") +
            STAFString(handle) + ", " + endpoint + ", " + machine + ", " +
            uuid + ", " + service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }

    STAFServiceResult serviceResult(kSTAFOk);

    if (isLocalMachine(machine, 1))
    {
        // Local machine

        addNotificationEntry(handle, endpoint, machine, uuid, service, key);
        return serviceResult;
    }
    else
    {
        // Remote machine

        STAFRC_t rc = kSTAFOk;
        STAFConnectionProviderPtr provider;
        STAFConnectionPtr connection;
        STAFString result;

        if (STAFTrace::doTrace(kSTAFTraceDebug))
        {
            STAFString msg = STAFString("Make connection to endpoint ") +
                endpoint + " via HandleTerminationRegistrationAPI.";
            STAFTrace::trace(kSTAFTraceDebug, msg);
        }

        rc = gConnectionManagerPtr->makeConnection(
            endpoint, provider, connection, result);

        if (rc)
        {
            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString msg = STAFString("Make connection to endpoint ") +
                    endpoint + " failed with RC: " + rc + " Result: " + result;
                STAFTrace::trace(kSTAFTraceDebug, msg);
            }

            serviceResult.fRC = rc;
            serviceResult.fResult = result;
            return serviceResult;
        }
        
        addPollingEntry(handle, endpoint, machine, uuid, service, key);

        connection->writeUInt(kSTAFHandleTerminationRegistrationAPI);
        connection->writeUInt(0);    // Dummy level

        STAFRC_t ack = connection->readUInt();

        if (ack != kSTAFOk)
        {
            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString msg = STAFString(
                    "Error connecting kSTAFHandleTerminationRegistrationAPI");
                STAFTrace::trace(kSTAFTraceDebug, msg);
            }

            serviceResult.fRC = ack;
            serviceResult.fResult = "Error connecting "
                "kSTAFHandleTerminationRegistrationAPI";
            return serviceResult;
        }
        
        // Determine the specific level of the API to use

        connection->writeUInt(1);  // min level
        connection->writeUInt(2);  // max level
        
        unsigned int levelToUse = connection->readUInt();

        if (levelToUse == 0)
        {
            serviceResult.fRC = kSTAFInvalidAPILevel;
            serviceResult.fResult = "Level 0 is invalid for "
                "kSTAFHandleTerminationRegistrationAPI";
            return serviceResult;
        }

        connection->writeUInt(handle);
        connection->writeString(*gMachinePtr);

        if (levelToUse > 1)
        {
            // Need to provide the port in order to generate the endpoint

            connection->writeString(
                provider->getProperty(kSTAFConnectionProviderPortProperty));
        }

        connection->writeString(uuid);
        connection->writeString(service);
        connection->writeString(key);

        return serviceResult;
    }
}


STAFRC_t STAFHandleManager::addNotificationEntry(
    STAFHandle_t &handle, const STAFString &endpoint,
    const STAFString &machine, const STAFString &uuid,
    const STAFString &service, const STAFString &key)
{
    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString("addNotificationEntry(") +
            STAFString(handle) + ", " + endpoint + ", " + machine + ", " +
            uuid + ", " + service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }

    STAFMutexSemLock notificationLock(fNotificationListSem);
    
    NotificationData theNotification(
        handle, endpoint, machine, uuid, key, service);
    
    fNotificationList.push_back(theNotification);

    return kSTAFOk;
}


STAFRC_t STAFHandleManager::deleteNotification(
    STAFHandle_t handle, const STAFString &endpoint,
    const STAFString &machine, const STAFString &uuid,
    const STAFString &service, const STAFString &key)
{
    // Try to delete the notification entry from the notification list first
        
    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString("STAFHandleManager::deleteNotification(") +
            STAFString(handle) + ", " + endpoint + ", " + machine + ", " +
            uuid + ", " + service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }

    {
        STAFMutexSemLock notificationLock(fNotificationListSem);

        NotificationList::iterator iter;

        for(iter = fNotificationList.begin(); iter != fNotificationList.end();
            ++iter)
        {
            if ((iter->handle == handle) && (iter->machine == machine) &&
                (iter->uuid == uuid) && (iter->notifyService == service) &&
                (iter->key == key))
            {
                fNotificationList.erase(iter);
                return kSTAFOk;
            }
        }
    }

    // Not found in the notification list, so try to delete the notification
    // entry from the gc polling list

    STAFRC_t rc = deletePolling(handle, machine, uuid, service, key);

    if (rc == kSTAFOk)
    {
        // Connect to the remote machine to unregister for notification
        // via the kSTAFHandleTerminationUnregistrationAPI (similar to
        // what we do in addNotification() using the
        // kSTAFHandleTerminationRegistrationAPI.

        STAFConnectionProviderPtr provider;
        STAFConnectionPtr connection;
        STAFString result;

        rc = gConnectionManagerPtr->makeConnection(
            endpoint, provider, connection, result);

        if (rc)
        {
            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString msg = STAFString(
                    "STAFHandleManager::deleteNotification(): Make "
                    "connection to endpoint ") + endpoint +
                    " failed with RC: " + rc + " Result: " + result;
                STAFTrace::trace(kSTAFTraceDebug, msg);
            }

            return rc;
        }

        connection->writeUInt(kSTAFHandleTerminationUnRegistrationAPI);
        connection->writeUInt(0);    // Dummy level

        STAFRC_t ack = connection->readUInt();

        if (ack != kSTAFOk)
        {
            // The kSTAFHandleTerminationUnRegistrationAPI was added in STAF
            // V3.4.1 so connecting to earlier versions of STAF will fail

            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString msg = STAFString(
                    "STAFHandleManager::deleteNotification(): Connection to "
                    "endpoint ") + endpoint + " failed using "
                    "HandleTerminationUnRegistrationAPI with ack=" + ack;
                STAFTrace::trace(kSTAFTraceDebug, msg);
            }

            rc = ack;
            return rc;
        }
        
        // Determine the specific level of the API to use

        connection->writeUInt(1);  // min level
        connection->writeUInt(2);  // max level
        
        unsigned int levelToUse = connection->readUInt();

        if (levelToUse == 0)
        {
            // Level 0 is invalid for kSTAFHandleTerminationUnRegistrationAPI
            rc = kSTAFInvalidAPILevel;
            return rc;
        }

        connection->writeUInt(handle);
        connection->writeString(*gMachinePtr);
        connection->writeString(uuid);
        connection->writeString(service);
        connection->writeString(key);
    }
    else if (rc == kSTAFDoesNotExist)
    {
        // The notification was not found in either the notification list or
        // the gc polling list.  Log a warning if the service is not DELAY,
        // as this probably indicates a problem in garbage collection.
        // Note that since the DELAY service uses the request# as the key,
        // it doesn't hurt anything if the notification for this request# has
        // already been removed so no warning needs to be logged in this case.

        if (service != "DELAY")
        {
            STAFString msg = STAFString(
                "In STAFHandleManager::deleteNotification(), No notification "
                "found for handle=") + STAFString(handle) + " machine=" +
                machine + " uuid=" + uuid + " service=" + service +
                " key=" + key;

            STAFTrace::trace(kSTAFTraceWarning, msg);
        }
    }

    return rc;
}


STAFRC_t STAFHandleManager::addPollingEntry(
    STAFHandle_t &handle, const STAFString &endpoint,
    const STAFString &machine, const STAFString &uuid,
    const STAFString &service, const STAFString &key)
{
    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString("addPollingEntry(") +
            STAFString(handle) + ", " + endpoint + ", " + machine + ", " +
            uuid + ", " + service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }

    STAFMutexSemLock pollingLock(fPollingDataListSem);

    PollingData theNotification(handle, endpoint, machine, uuid, key, service);
    
    fPollingDataList.push_back(theNotification);

    return kSTAFOk;
}


STAFRC_t STAFHandleManager::deletePolling(
    STAFHandle_t &handle, const STAFString &machine, const STAFString &uuid,
    const STAFString &service, const STAFString &key)
{
    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFString msg = STAFString("deletePolling(") +
            STAFString(handle) + ", " + machine + ", " +
            uuid + ", " + service + ", " + key + ")";
        STAFTrace::trace(kSTAFTraceDebug, msg);
    }

    STAFMutexSemLock pollingLock(fPollingDataListSem);

    PollingDataList::iterator iter;

    for(iter = fPollingDataList.begin(); iter != fPollingDataList.end();
        ++iter)
    {
        if ((iter->handle == handle) && (iter->machine == machine) &&
            (iter->uuid == uuid) && (iter->notifyService == service) &&
            (iter->key == key))
        {
            fPollingDataList.erase(iter);
            return kSTAFOk;
        }
    }

    if (STAFTrace::doTrace(kSTAFTraceDebug))
    {
        STAFTrace::trace(kSTAFTraceDebug, "deletePolling: Not found in list");
    }

    return kSTAFDoesNotExist;
}


STAFRC_t STAFHandleManager::registerHandle(STAFHandle_t &handle,
                                           STAFProcessID_t pid,
                                           const STAFString &name)
{
    STAFString lowerName(name.toLowerCase());
    STAFMutexSemLock handleLock(fHandleListSem);
    HandleList::iterator handleIter;
    STAFProcessHandle_t procHandle = 0;

    for (handleIter = fHandleList.begin(); handleIter != fHandleList.end();
         ++handleIter)
    {

        if (handleIter->second.pid != pid) continue;

        if (pid == gSTAFProcPID)
        {
            if (lowerName == handleIter->second.lowerName)
            {
                // This is an existing kInProcess handle

                if (handleIter->second.state != kInProcess)
                {
                    STAFTrace::trace(kSTAFTraceError,
                                     "In STAFHandleManager::registerHandle(), "
                                     "found handle with STAFProc PID, but not "
                                     "state == kInProcess");
                }

                handle = handleIter->second.handle;

                return kSTAFOk;
            }
            else continue;
        }

        if (!STAFProcess::isRunning(handleIter->second.procHandle))
        {
            // This is a program that is no longer running, but which has not
            // been removed from the list yet.  Either our callback hasn't
            // been called for a regular kRegistered process, or the
            // ProcessManager has not told us to remove the kPending(Registered)
            // handle.  Either way, this handle is not a match.

            continue;
        }

        if (handleIter->second.state == kPending)
        {
            // This is an existing kPending handle, and the process is
            // just now registering.

            handle = handleIter->second.handle;
            handleIter->second.name = name;
            handleIter->second.state = kPendingRegistered;
            handleIter->second.lastUsedTimestamp = STAFTimestamp::now();

            return kSTAFOk;
        }

        procHandle = handleIter->second.procHandle;
    }

    // This is a valid new handle so register it

    if (procHandle == 0)
    {
        unsigned int osRC = 0;
        STAFRC_t rc = STAFProcess::getProcessHandleFromID(
                      pid, procHandle, osRC);
        if (rc != 0)
        {
            STAFTrace::trace(
                kSTAFTraceError, STAFString(
                    "In STAFHandleManager::registerHandle: "
                    "getProcessHandleFromID failed with OS_RC: ") + osRC);
        }
    }

    STAFServiceResult result = assignNextHandleNumber();

    if (result.fRC) return result.fRC;

    HandleData theHandle(fNextHandle, pid, procHandle,
                         (pid == gSTAFProcPID) ? kInProcess : kRegistered,
                         STAFTimestamp::now(), name);

    fHandleList[theHandle.handle] = theHandle;

    handle = theHandle.handle;

    fTotalHandles++;

    if (pid == gSTAFProcPID) return kSTAFOk;

    if (procHandle == 0)
    {
        // If an error occurred calling getProcessHandleFromID() above (e.g.
        // osRC 5, AccessDenied), the procHandle will still be 0.  We are
        // returning so that the callback will not be performed because it
        // will result in a WAIT_FAILED from WaitForMultipleObjects in
        // ProcessMonitorThread() with OS_RC: 6 (ERROR_INVALID_HANDLE) and
        // it will loop continuously spitting out this error trace message.
        // However, since we aren't doing the callback, the handle won't ever
        // be garbage collected.

        return kSTAFOk;
    }
    
    STAFProcessEndCallbackLevel1 callback = { processTerminated, 0 };

    STAFRC_t rc = STAFProcess::registerForProcessTermination(
        pid, theHandle.procHandle, callback);

    if (rc != kSTAFOk) return rc;

    return kSTAFOk;
}


STAFRC_t STAFHandleManager::unRegister(STAFHandle_t handle,
                                       STAFProcessID_t pid)
{
    STAFRC_t rc = kSTAFOk;
    STAFHandle_t unRegHandle;
    
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) == fHandleList.end())
    {
        rc = kSTAFHandleDoesNotExist;
        return rc;
    }
    else
    {
        HandleData &theHandle = fHandleList[handle];
            
        unRegHandle = theHandle.handle;

        if (theHandle.pid != pid)
        {
            rc = kSTAFInvalidHandle;
            return rc;
        }
        else if (theHandle.state == kPendingRegistered)
        {
            theHandle.state = kPending;
            return rc;
        }
        else if ((theHandle.state == kRegistered) ||
                 (theHandle.state == kInProcess))
        {
            // Remove the handle from the list and post the handle queue's
            // notify event semaphore

            STAFHandleQueuePtr savedQueuePtr = theHandle.handleQueuePtr;

            fHandleList.erase(handle);

            postHandleQueueNotifySem(savedQueuePtr);

            // Dispatch calling the handleRemoved(handle) method so that it
            // runs on another thread because it needs a lock on
            // fHandleListSem and because it contains a 100 ms wait in a loop
            // for deleting notifications for garbage collection.

            getHandleThreadManager().dispatch(
                callHandleRemovedCallback, new STAFHandle_t(unRegHandle));
        }
        else if ((theHandle.state == kPending) ||
                 (theHandle.state == kStatic))
        {
            // XXX: Is this what we really should be doing for these states?

            // Dispatch calling the handleRemoved(handle) method so that it
            // runs on another thread because it needs a lock on
            // fHandleListSem and because it contains a 100 ms wait in a loop
            // for deleting notifications for garbage collection.

            getHandleThreadManager().dispatch(
                callHandleRemovedCallback, new STAFHandle_t(unRegHandle));
        }
        else
        {
            // We're not currently handling any other Handle states

            if (STAFTrace::doTrace(kSTAFTraceError))
            {
                STAFString msg = STAFString("Unregistering invalid handle "
                                            "state: ") + theHandle.state;
                STAFTrace::trace(kSTAFTraceError, msg);
            }
        }
    }
    
    return rc;
}


void STAFHandleManager::handleRemoved(STAFHandle_t handle)
{
    // Remove all entries in the notification list with this handle

    STAFHandleManager::NotificationList notificationList =
        gHandleManagerPtr->getNotificationListCopy();

    STAFHandleManager::NotificationList::iterator iter;

    for (iter = notificationList.begin();
         iter != notificationList.end(); iter++)
    {
        STAFHandleManager::NotificationData notifiee(*iter);

        if (notifiee.handle == handle)
        {
            STAFString request = "STAF_CALLBACK HANDLEDELETED HANDLE ";
            request += notifiee.handle;
            request += " MACHINE ";
            request += STAFHandle::wrapData(notifiee.machine);
            request += " UUID ";
            request += STAFHandle::wrapData(notifiee.uuid);
            request += " KEY ";
            request += STAFHandle::wrapData(notifiee.key);

            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString msg = STAFString("STAF ") + notifiee.endpoint +
                    " " + notifiee.notifyService + " " + request;
                STAFTrace::trace(kSTAFTraceDebug, msg);
            }
            
            STAFResultPtr result = 
                gSTAFProcHandlePtr->submit(notifiee.endpoint,
                                           notifiee.notifyService,
                                           request,
                                           kSTAFReqFireAndForget);

            gHandleManagerPtr->deleteNotification(
                notifiee.handle, notifiee.endpoint, notifiee.machine,
                notifiee.uuid, notifiee.notifyService, notifiee.key);

            // Wait for 100 milliseconds to prevent a flood of
            // kSTAFReqFireAndForget requests being submitted all at once

            STAFThreadSleepCurrentThread(100, 0);
        }
    }
}


STAFRC_t STAFHandleManager::removePendingHandle(STAFHandle_t handle,
                                                STAFProcessID_t pid)
{
    STAFRC_t rc = kSTAFOk;
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) == fHandleList.end())
    {
        rc = kSTAFHandleDoesNotExist;
    }
    else
    {
        const HandleData &theHandle = fHandleList[handle];

        if (theHandle.pid != pid)
        {
            rc = kSTAFInvalidHandle;
        }
        else if ((theHandle.state == kPending) ||
                 (theHandle.state == kPendingRegistered))
        {
            // Remove the handle from the list and post the handle queue's
            // notify event semaphore

            STAFHandleQueuePtr savedQueuePtr = theHandle.handleQueuePtr;

            fHandleList.erase(handle);

            postHandleQueueNotifySem(savedQueuePtr);

            // Dispatch calling the handleRemoved(handle) method so that it runs on
            // another thread because it needs a lock on fHandleListSem and because
            // it contains a 100 ms wait in a loop for deleting notifications for
            // garbage collection.

            getHandleThreadManager().dispatch(
                callHandleRemovedCallback, new STAFHandle_t(handle));
        }
        else
        {
            rc = kSTAFInvalidHandle;
        }
    }

    return rc;
}


STAFRC_t STAFHandleManager::removeStaticHandle(STAFHandle_t handle)
{
    STAFRC_t rc = kSTAFOk;

    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) == fHandleList.end())
    {
        rc = kSTAFHandleDoesNotExist;
    }
    else
    {
        const HandleData &theHandle = fHandleList[handle];

        if (theHandle.state == kStatic)
        {
            // Remove the handle from the list and post the handle queue's
            // notify event semaphore

            STAFHandleQueuePtr savedQueuePtr = theHandle.handleQueuePtr;

            fHandleList.erase(handle);

            postHandleQueueNotifySem(savedQueuePtr);

            // Dispatch calling the handleRemoved(handle) method so that it
            // runs on another thread because it needs a lock on
            // fHandleListSem and because it contains a 100 ms wait in a loop
            // for deleting notifications for garbage collection.

            getHandleThreadManager().dispatch(
                callHandleRemovedCallback, new STAFHandle_t(handle));
        }
        else
        {
            rc = kSTAFInvalidHandle;
        }
    }

    return rc;
}


STAFHandleManager::HandleList STAFHandleManager::getHandleListCopy()
{
    STAFMutexSemLock handleLock(fHandleListSem);

    return fHandleList;
}


STAFHandle_t STAFHandleManager::getHandleListSize()
{
    STAFMutexSemLock handleLock(fHandleListSem);

    return fHandleList.size();
}


STAFHandleManager::NotificationList STAFHandleManager::getNotificationListCopy()
{
    STAFMutexSemLock handleLock(fNotificationListSem);

    return fNotificationList;
}


STAFHandleManager::PollingDataList STAFHandleManager::getPollingDataListCopy()
{
    STAFMutexSemLock handleLock(fPollingDataListSem);

    return fPollingDataList;
}


STAFMutexSem &STAFHandleManager::getHandleManagerSem()
{
    return fHandleListSem;
}


STAFUInt64_t STAFHandleManager::getTotalHandles()
{
    return fTotalHandles;
}


STAFHandle_t STAFHandleManager::getMaxHandleNumber()
{
    return MAX_HANDLE_NUMBER;
}


STAFHandle_t STAFHandleManager::getMinHandleNumber()
{
    return MIN_HANDLE_NUMBER;
}


STAFHandle_t STAFHandleManager::getMaxActiveHandles()
{
    return MAX_ACTIVE_HANDLES;
}


unsigned int STAFHandleManager::getResetCount()
{
    return fResetCount;
}


// STAFHandleManager::authenticate
//
// Perform Authentication for the handle.  If credentials are input, check if
// the handle is already authenticated, and if so, return an error.  If the
// specified Authenticator service is not registered, return an error if
// credentials are input; if authentication data is input, set authenticator to 
// "none" and set userIdentifier to "anonymous". If the specified Authenticator
// is found and the authenticationType is kData (indicating a remote system),
// check if there is cached authentication information for this remote
// machine's handle.  If so, and the cached authentication information is the
// same, then don't need to re-authenticate.  Otherwise, submit an
// AUTHENTICATE request to the Authenticator service, passing the specified 
// authentication data or credentials.  If the AUTHENTICATE request is
// successful and credentials were input, return authentication data in the 
// result.
//
// Parameters:
//   machine - machine name (Input)
//   handle  - handle (Input)
//   authenticator - name of authenticator service (Input/Output)
//   userIdentifier - name of the user (Input/Output)
//   authenticateType - authentication data type, kCredentials or kData (Input)
//   authenticationData - authentication credentials or data (Input)
//
// Returns:
//   rc 0 if authenticate was successful and if credentials passed
//   in via authenticationData, returns authentication data in result.
//   Non-zero rc if authenticate failed and any additional info about the
//   failure in the result.

STAFServiceResult STAFHandleManager::authenticate(
                                     const STAFString &machine,
                                     const STAFHandle_t handle,
                                     STAFString &authenticator,
                                     STAFString &userIdentifier,
                                     const AuthenticateType authenticateType,
                                     const STAFString &authenticationData)
{
    STAFRC_t rc = kSTAFOk;

    if (authenticateType == kCredentials)
    {
        // Check if handle is already authenticated.  If so, return an error.

        rc = isAuthenticated(handle);
        
        if (rc != kSTAFOk)
        {
            return STAFServiceResult(rc);
        }
    }

    // Check if the specified authenticator service is registered

    STAFServiceResult serviceResult(kSTAFOk);
    STAFServicePtr service;

    rc = gServiceManagerPtr->getAuthenticator(authenticator, service);

    if (rc != kSTAFOk)
    {
        // The authenticator service is not registered

        if (authenticateType == kCredentials)
        {
            // Return an error

            return STAFServiceResult(rc, authenticator);
        }
        else
        {   // Authentication data was passed in.
            // Don't return an error.  Set the authenticator to "none"
            // and the userIdentifier to "anonymous". 

            authenticator = gNoneString;
            userIdentifier = gAnonymousString;

            return serviceResult;
        }
    }

    if (authenticateType == kData)
    {
        // Get cached authentication information, if any, for the remote
        // machine's handle.

        STAFString theKey = machine.toLowerCase() + ";" + STAFString(handle);

        STAFMutexSemLock remoteHandle(fRemoteHandleMapSem);

        // If authentication information for remote machine/handle has been
        // cached, if it's the same, then there's no need to re-authenticate.

        if (fRemoteAuthHandleMap.find(theKey) != fRemoteAuthHandleMap.end())
        {
            AuthenticationInfo &authInfo = fRemoteAuthHandleMap[theKey];

            if ((authInfo.authenticator == authenticator.toUpperCase()) &&
                (authInfo.userIdentifier == userIdentifier) &&
                (authInfo.authenticationData == authenticationData))
            {   
                return serviceResult;
            }
        }
    }

    // Submit an AUTHENTICATE request to the Authenticator service on the
    // local machine

    STAFString request = "AUTHENTICATE USER " +
        STAFHandle::wrapData(userIdentifier);

    if (authenticateType == kCredentials)
        request += " CREDENTIALS " + STAFHandle::wrapData(authenticationData);
    else
        request += " DATA " + STAFHandle::wrapData(authenticationData);

    serviceResult = submitAuthServiceRequest(service, request);

    if (serviceResult.fRC == kSTAFOk)
    {
        // User was successfully authenticated

        STAFString authData = authenticationData;

        if (authenticateType == kCredentials)
        {
            // Assign authentication data returned by the Authenticator service
            authData = serviceResult.fResult;

            // Set handle variable STAF/Handle/User=authenticator://user

            STAFVariablePoolPtr handlePool;

            rc = variablePool(handle, handlePool);
            handlePool->set("STAF/Handle/User",
                            authenticator + gSpecSeparator + userIdentifier);
        }
        
        rc = gHandleManagerPtr->cacheAuthenticationInfo(machine, handle,
             authenticator, userIdentifier, authData);
    }

    return serviceResult;
}


// STAFHandleManager::unAuthenticate
//
// Unauthenticate the local handle
//
// Parameters:
//   handle  - handle (Input)
//
// Returns:
//   rc 0 if successfully un-authenticate the handle
//   Non-zero rc if could not un-authenticate the handle and the handle
//       in the result.

STAFServiceResult STAFHandleManager::unAuthenticate(const STAFHandle_t handle)
{
    {
        STAFMutexSemLock handleLock(fHandleListSem);

        if (fHandleList.find(handle) == fHandleList.end())
        {
            return STAFServiceResult(kSTAFHandleDoesNotExist, handle);
        }

        HandleData &theHandle = fHandleList[handle];

        theHandle.authenticator = gNoneString;
        theHandle.userIdentifier = gAnonymousString;
        theHandle.authenticationData = "";
    }

    // Set handle variable STAF/Handle/User=none://anonymous
    
    STAFVariablePoolPtr handlePool;

    STAFRC_t rc = variablePool(handle, handlePool);

    if (rc == kSTAFOk)
        handlePool->set("STAF/Handle/User", gUnauthenticatedUser);

    return STAFServiceResult(kSTAFOk);
}


// STAFHandleManager::cacheAuthenticationInfo
//
// Cache authentication information for the machine's handle.  If the
// machine is local, store the authentication information in the
// local Handle List.  If the machine is remote, store the authentication
// information in the Remote Authenticated Handle Map. 
//
// Parameters:
//   machine - machine name (Input)
//   handle  - handle (Input)
//   authenticator - name of authenticator service (Input)
//   userIdentifier - name of the user (Input)
//   authenticationData - authentication data (Input)
//
// Returns:
//   rc 0 if authentication information was cached successfully
//   Non-zero rc if authentication information was not cached successfully

STAFRC_t STAFHandleManager::cacheAuthenticationInfo(
                            const STAFString &machine,
                            const STAFHandle_t handle,
                            const STAFString &authenticator,
                            const STAFString &userIdentifier,
                            const STAFString &authenticationData)
{
    if (isLocalMachine(machine, 1))
    {
        STAFMutexSemLock handleLock(fHandleListSem);
        
        if (fHandleList.find(handle) == fHandleList.end())
        {
            // Should never happen
            return kSTAFHandleDoesNotExist;
        }

        HandleData &theHandle = fHandleList[handle];

        theHandle.authenticator = authenticator;
        theHandle.userIdentifier = userIdentifier;
        theHandle.authenticationData = authenticationData;
    }
    else
    {
        // Cache the authentication information for the remote machine/handle
        // in the fRemoteAuthHandleMap.

        STAFString theKey = machine.toLowerCase() + ";" + STAFString(handle);

        AuthenticationInfo authInfo(authenticator, userIdentifier,
                                    authenticationData);

        STAFMutexSemLock remoteHandleLock(fRemoteHandleMapSem);

        fRemoteAuthHandleMap[theKey] = authInfo;

        // XXX: Register for garbage collection so that when handle or machine
        // is unregistered, it can be removed from the fRemoteAuthHandleMap.
    }

    return kSTAFOk;
}


// STAFHandleManager::isAuthenticated
//
// Check if the local handle is authenticated.
//
// Parameters:
//   handle  - handle (Input)
//
// Returns:
//   0 if the local handle is not authenticated.
//   kSTAFHandleAlreadyAuthenticated if the handle is already authenticated.
//   kSTAFHandleDoesNotExist if the local handle does not exist.

STAFRC_t STAFHandleManager::isAuthenticated(
                            const STAFHandle_t handle)
{
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) == fHandleList.end())
    {
        // Should never happen
        return kSTAFHandleDoesNotExist;
    }

    HandleData &theHandle = fHandleList[handle];

    if (theHandle.authenticator == gNoneString)
        return kSTAFOk;
    else
        return kSTAFHandleAlreadyAuthenticated;
}


// STAFHandleManager::getAuthenticationInfo
//
// Get the authentication information for a local handle.
//
// Parameters:
//   handle  - handle (Input)
//   authenticator - name of authenticator service (Output)
//   userIdentifier - name of the user (Output)
//   authenticationData - authentication data (Output)
//
// Returns:
//   rc 0 if authentication info for a local handle was retrieved successfully
//   Non-zero rc if could not get authentication info for a local handle

STAFRC_t STAFHandleManager::getAuthenticationInfo(
                            const STAFHandle_t handle,
                            STAFString &authenticator,
                            STAFString &userIdentifier,
                            STAFString &authenticationData)
{
    STAFMutexSemLock handleLock(fHandleListSem);

    if (fHandleList.find(handle) == fHandleList.end())
    {
        // Should never happen
        authenticator = gNoneString;
        userIdentifier = gAnonymousString;
        return kSTAFHandleDoesNotExist;
    }

    HandleData &theHandle = fHandleList[handle];

    authenticator = theHandle.authenticator;
    userIdentifier = theHandle.userIdentifier;
    authenticationData = theHandle.authenticationData;

    return kSTAFOk;
}

void STAFHandleManager::gcPolling()
{
    // Delay 10 seconds before beginning the polling loop

    gThreadManagerPtr->sleepCurrentThread(10000);
    
    while (1)
    {
        try
        {
            // Wait for the time specified for the handle GC interval 

            gGCPollingSem->wait(gHandleGCInterval);
             
            if (!gContinueGCPolling)
                return;
             
            PollingDataList pollingDataList = getPollingDataListCopy();
            PollingDataList::iterator iter;

            for (iter = pollingDataList.begin();
                 iter != pollingDataList.end(); iter++)
            {
                PollingData notifiee(*iter);
               
                // Submit a HANDLE QUERY HANDLE <notifiee.handle> request to
                // see if the handle still exists on the notifiee machine and
                // that the notifiee machine is currently running STAF.

                STAFString request = "QUERY HANDLE " +
                    STAFString(notifiee.handle);

                STAFResultPtr result = gSTAFProcHandlePtr->submit(
                    notifiee.endpoint, "HANDLE", request);
                 
                if (result->rc == kSTAFAccessDenied)
                {
                    // The HANDLE QUERY request failed with a trust issue
                    // (as this request had trust level 2 in STAF V3.4.0 and
                    // earlier), so submit a MISC WHOAREYOU request (which has
                    // trust level 1) to see if the notifiee machine is still
                    // running the same STAF instance.

                    result = gSTAFProcHandlePtr->submit(
                        notifiee.endpoint, "MISC", "WHOAREYOU");

                    if (result->rc == kSTAFOk)
                    {
                        STAFString uuid = result->resultObj->
                            get("instanceUUID")->asString();
                         
                        if (uuid != notifiee.uuid)
                        {
                            result->rc = kSTAFHandleDoesNotExist;
                        }
                    }
                }
                else if (result->rc == kSTAFOk)
                {
                    // Need to make sure that the handle in the result map
                    // isn't None as there was a bug in the QUERY HANDLE
                    // request for the HANDLE service in STAF V3.2.2 or
                    // earlier where it returned RC 0 even if the handle
                    // did not exist.  In this case, it set all the fields
                    // including handle to None.

                    STAFString handle = result->resultObj->get("handle")->
                        asString();

                    if (handle == "<None>")
                    {
                        result->rc = kSTAFHandleDoesNotExist;
                    }
                    else
                    {
                        // Verify that the the same STAFProc instance is
                        // still running by verifying that the uuid matches

                        STAFString uuid = result->resultObj->get("uuid")->
                            asString();

                        if (uuid == "<None>")
                        {
                            // The UUID is not available in the HANDLE QUERY
                            // request because the notifiee endpoint is
                            // running STAF V3.4.1 or later.  So, submit a
                            // MISC WHOAREYOU request to get the UUID for
                            // the notifiee endpoint.
                             
                            result = gSTAFProcHandlePtr->submit(
                                notifiee.endpoint, "MISC", "WHOAREYOU");

                            if (result->rc == kSTAFOk)
                            {
                                uuid = result->resultObj->
                                    get("instanceUUID")->asString();

                                if (uuid != notifiee.uuid)
                                {
                                    result->rc = kSTAFHandleDoesNotExist;
                                }
                            }
                        }
                        else if (uuid != notifiee.uuid)
                        {
                            result->rc = kSTAFHandleDoesNotExist;
                        }
                    }
                }

                // If the request fails, then we assume the notifiee handle on
                // the notifiee machine no longer exists (either because the
                // handle has been deleted or STAF was shutdown on the
                // notifiee machine).  So, we then submit a
                // STAF_CALLBACK HANDLEDELETED HANDLE request to notify any
                // services who registered for the callback that the handle
                // no longer exists and we delete the polling entry.

                if (result->rc != kSTAFOk)
                {
                    request = "STAF_CALLBACK HANDLEDELETED HANDLE " +
                        STAFString(notifiee.handle) +
                        " MACHINE " + STAFHandle::wrapData(notifiee.machine) +
                        " UUID " + STAFHandle::wrapData(notifiee.uuid) +
                        " KEY " + STAFHandle::wrapData(notifiee.key);

                    STAFResultPtr result = gSTAFProcHandlePtr->submit(
                        "local", notifiee.notifyService, request,
                        kSTAFReqFireAndForget);
            
                    deletePolling(notifiee.handle, notifiee.machine,
                                  notifiee.uuid, notifiee.notifyService,
                                  notifiee.key);
                }
            }
        }
        catch (STAFException &se)
        {
            se.trace("STAFHandleManager::gcPolling()");
        }
        catch (...)
        {
            STAFTrace::trace(kSTAFTraceError, "Caught unknown exception in "
                             "STAFHandleManager::gcPolling()");
        }
    } // end while forever loop
}

unsigned int HandleMonitorThread(void *data)
{
    pHandleManager->gcPolling();

    return kSTAFUnknownError;
}

 	  	 
