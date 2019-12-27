/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFSemService.h"
#include "STAFCommandParser.h"
#include "STAFHandleService.h"
#include "STAFHandleManager.h"
#include "STAFUtil.h"

static const STAFString sNotificationKey = "SemServiceMutex";
static STAFString sHelpMsg;

STAFHandleService *handleService;

STAFSemService::STAFSemService() : STAFService("SEM")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** SEM (Semaphore) Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "REQUEST MUTEX <Name> [TIMEOUT <Number>[s|m|h|d|w]] [GARBAGECOLLECT <Yes | No>]" +
        *gLineSeparatorPtr +
        "RELEASE MUTEX <Name> [FORCE]" +
        *gLineSeparatorPtr +
        "CANCEL  MUTEX <Name>" +
        *gLineSeparatorPtr +
        "        [FORCE [MACHINE <Machine>] [HANDLE <Handle #> | NAME <Handle Name>]]" +
        *gLineSeparatorPtr +
        "        [FIRST | LAST]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "POST    EVENT <Name>" +
        *gLineSeparatorPtr +
        "RESET   EVENT <Name>" +
        *gLineSeparatorPtr +
        "PULSE   EVENT <Name>" +
        *gLineSeparatorPtr +
        "WAIT    EVENT <Name> [TIMEOUT <Number>[s|m|h|d|w]]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DELETE  MUTEX <Name> | Event <Name>" +
        *gLineSeparatorPtr +
        "QUERY   MUTEX <Name> | Event <Name>" +
        *gLineSeparatorPtr +
        "LIST    MUTEX | EVENT" +
        *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // Set up Request Parser

    fRequestParser.addOption("REQUEST", 1,
                             STAFCommandParser::kValueNotAllowed);
    fRequestParser.addOption("MUTEX", 1,
                             STAFCommandParser::kValueRequired);
    fRequestParser.addOption("TIMEOUT", 1,
                             STAFCommandParser::kValueRequired);
    fRequestParser.addOption("GARBAGECOLLECT", 1,
                             STAFCommandParser::kValueRequired);
    fRequestParser.addOptionGroup("MUTEX", 1, 1);
    
    // Set up Release Parser

    fReleaseParser.addOption("RELEASE", 1,
                             STAFCommandParser::kValueNotAllowed);
    fReleaseParser.addOption("MUTEX", 1,
                             STAFCommandParser::kValueRequired);
    fReleaseParser.addOption("FORCE", 1,
                             STAFCommandParser::kValueNotAllowed);
    fRequestParser.addOptionGroup("MUTEX", 1, 1);

    // Set up Cancel Parser

    fCancelParser.addOption("CANCEL", 1,
                            STAFCommandParser::kValueNotAllowed);
    fCancelParser.addOption("MUTEX", 1,
                            STAFCommandParser::kValueRequired);
    fCancelParser.addOption("FORCE", 1,
                            STAFCommandParser::kValueNotAllowed);
    fCancelParser.addOption("HANDLE", 1,
                            STAFCommandParser::kValueRequired);
    fCancelParser.addOption("NAME", 1,
                            STAFCommandParser::kValueRequired);
    fCancelParser.addOption("MACHINE", 1,
                            STAFCommandParser::kValueRequired);
    fCancelParser.addOption("FIRST", 1,
                            STAFCommandParser::kValueNotAllowed);
    fCancelParser.addOption("LAST", 1,
                            STAFCommandParser::kValueNotAllowed);
    fCancelParser.addOptionNeed("CANCEL", "MUTEX");
    fCancelParser.addOptionNeed("MUTEX FORCE FIRST LAST", "CANCEL");
    fCancelParser.addOptionNeed("MACHINE HANDLE NAME", "FORCE");
    fCancelParser.addOptionGroup("MUTEX", 1, 1);
    fCancelParser.addOptionGroup("HANDLE NAME", 0, 1);
    fCancelParser.addOptionGroup("FIRST LAST", 0, 1);

    // Set up Post/Pulse Parser

    fPostPulseParser.addOption("POST", 1,
                               STAFCommandParser::kValueNotAllowed);
    fPostPulseParser.addOption("PULSE", 1,
                               STAFCommandParser::kValueNotAllowed);
    fPostPulseParser.addOption("EVENT", 1,
                               STAFCommandParser::kValueRequired);
    fPostPulseParser.addOptionGroup("EVENT", 1, 1);
    fPostPulseParser.addOptionGroup("POST PULSE", 1, 1);

    // Set up Reset Parser

    fResetParser.addOption("RESET", 1,
                           STAFCommandParser::kValueNotAllowed);
    fResetParser.addOption("EVENT", 1,
                           STAFCommandParser::kValueRequired);
    fResetParser.addOptionGroup("EVENT", 1, 1);

    // Set up Wait Parser

    fWaitParser.addOption("WAIT", 1,
                          STAFCommandParser::kValueNotAllowed);
    fWaitParser.addOption("EVENT", 1,
                          STAFCommandParser::kValueRequired);
    fWaitParser.addOption("TIMEOUT", 1,
                          STAFCommandParser::kValueRequired);
    fWaitParser.addOptionGroup("EVENT", 1, 1);

    // Delete Parser Setup
 
    fDeleteParser.addOption("DELETE",  1, 
                            STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("MUTEX", 1, 
                            STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("EVENT", 1, 
                            STAFCommandParser::kValueRequired);
    fDeleteParser.addOptionGroup("MUTEX EVENT", 1, 1);

    // Query Parser Setup
 
    fQueryParser.addOption("QUERY",  1, 
                           STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption("MUTEX", 1, 
                           STAFCommandParser::kValueRequired);
    fQueryParser.addOption("EVENT", 1, 
                           STAFCommandParser::kValueRequired);
    fQueryParser.addOptionGroup("MUTEX EVENT", 1, 1);

    // List Parser Setup
 
    fListParser.addOption("LIST",  1, 
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("MUTEX", 1, 
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("EVENT", 1, 
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOptionGroup("MUTEX EVENT", 1, 1);
        
    // staf_callback Parser Setup
 
    fSTAFCallbackParser.addOption("STAF_CALLBACK",  1, 
        STAFCommandParser::kValueNotAllowed);
    fSTAFCallbackParser.addOption("HANDLEDELETED", 1, 
        STAFCommandParser::kValueNotAllowed);
    fSTAFCallbackParser.addOption("HANDLE", 1, 
        STAFCommandParser::kValueRequired);
    fSTAFCallbackParser.addOption("MACHINE", 1, 
        STAFCommandParser::kValueRequired);
    fSTAFCallbackParser.addOption("UUID", 1, 
        STAFCommandParser::kValueRequired);
    fSTAFCallbackParser.addOption("KEY", 1, 
        STAFCommandParser::kValueRequired);
    
    handleService = STAFHandleService::getInstance();
    
    // Construct map class for listing mutex semaphore information

    fMutexInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/MutexInfo");
 
    fMutexInfoClass->addKey("name", "Name");
    fMutexInfoClass->addKey("state", "State");
    fMutexInfoClass->addKey("pendingRequests", "Pending Requests");

    // Construct map class for listing event semaphore information

    fEventInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/EventInfo");
 
    fEventInfoClass->addKey("name", "Name");
    fEventInfoClass->addKey("state", "State");
    fEventInfoClass->addKey("waiters", "Waiters");

    // Construct map class for detailed information on an event semaphore

    fQueryEventClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/QueryEvent");
 
    fQueryEventClass->addKey("state", "State");
    fQueryEventClass->addKey("lastPosted", "Last Posted");
    fQueryEventClass->addKey("lastReset", "Last Reset");
    fQueryEventClass->addKey("waiterList", "Waiters");
    
    // Construct map class for detailed information on a mutex semaphore

    fQueryMutexClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/QueryMutex");
 
    fQueryMutexClass->addKey("state", "State");
    fQueryMutexClass->addKey("owner", "Owner");
    fQueryMutexClass->addKey("requestList", "Pending Requests");

    // Construct map class for an event semaphore requester

    fEventRequesterClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/EventRequester");

    fEventRequesterClass->addKey("machine", "Machine");
    fEventRequesterClass->addKey("handleName", "Handle Name");
    fEventRequesterClass->addKey("handle", "Handle");
    fEventRequesterClass->addKey("user", "User");
    fEventRequesterClass->addKey("endpoint", "Endpoint");
    fEventRequesterClass->addKey("timestamp", "Date-Time");

    // Construct map class for an event semaphore requester

    fMutexOwnerClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/MutexOwner");

    fMutexOwnerClass->addKey("machine", "Machine");
    fMutexOwnerClass->addKey("handleName", "Handle Name");
    fMutexOwnerClass->addKey("handle", "Handle");
    fMutexOwnerClass->addKey("user", "User");
    fMutexOwnerClass->addKey("endpoint", "Endpoint");
    fMutexOwnerClass->addKey("requestTimestamp", "Date-Time Requested");
    fMutexOwnerClass->addKey("acquireTimestamp", "Date-Time Acquired");
    fMutexOwnerClass->addKey("gc", "Perform Garbage Collection");

    // Construct map class for an mutex semaphore requester

    fPendingRequestClass = STAFMapClassDefinition::create(
        "STAF/Service/Sem/PendingRequest");

    fPendingRequestClass->addKey("machine", "Machine");
    fPendingRequestClass->addKey("handleName", "Handle Name");
    fPendingRequestClass->addKey("handle", "Handle");
    fPendingRequestClass->addKey("user", "User");
    fPendingRequestClass->addKey("endpoint", "Endpoint");
    fPendingRequestClass->addKey("requestTimestamp", "Date-Time Requested");
    fPendingRequestClass->addKey("gc", "Perform Garbage Collection");
}


STAFSemService::~STAFSemService()
{
    /* Do Nothing */
}


STAFString STAFSemService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFSemService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();
 
    if      (action == "request") return handleRequest(requestInfo);
    else if (action == "release") return handleRelease(requestInfo);
    else if (action == "cancel") return handleCancel(requestInfo);
    else if (action == "post" || action == "pulse")
        return handlePostPulse(requestInfo);
    else if (action == "reset") return handleReset(requestInfo);
    else if (action == "wait") return handleWait(requestInfo);
    else if (action == "delete") return handleDelete(requestInfo);
    else if (action == "query") return handleQuery(requestInfo);
    else if (action == "list")  return handleList(requestInfo);
    else if (action == "staf_callback") return handleSTAFCallback(requestInfo);
    else if (action == "help")  return handleHelp(requestInfo);
    else
    {
        STAFString errMsg = STAFString("'") +
            requestInfo.fRequest.subWord(0, 1) +
            "' is not a valid command request for the " + name() +
            " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
            sHelpMsg;

        return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
    }
}


STAFServiceResult STAFSemService::handleRequest(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    IVALIDATE_TRUST(3, "REQUEST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fRequestParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("MUTEX", semName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString uppercaseSemName = semName.toUpperCase();

    unsigned int timeout = STAF_EVENT_SEM_INDEFINITE_WAIT;
    
    rc = RESOLVE_DEFAULT_DURATION_OPTION(
        "TIMEOUT", timeout, STAF_EVENT_SEM_INDEFINITE_WAIT);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Determine if garbage collection should be performed.

    bool garbageCollect = true;  // Default if GARBAGECOLLECT not specified

    if (parsedResult->optionTimes("GARBAGECOLLECT") > 0)
    {
        // Resolve the garbageCollect option value

        STAFString gcValue;

        rc = RESOLVE_OPTIONAL_STRING_OPTION("GARBAGECOLLECT", gcValue);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // If "YES", garbage collection will be performed when the handle
        // that requested the resource is deleted.  If "NO", no garbage
        // collection will be performed.

        if (gcValue.isEqualTo("No", kSTAFStringCaseInsensitive))
        {
            garbageCollect = false;
        }
        else if (gcValue.isEqualTo("Yes", kSTAFStringCaseInsensitive))
        {
            garbageCollect = true;
        }
        else
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                STAFString("GARBAGECOLLECT value must be Yes or No.") +
                "  Invalid value: " + gcValue);
        }
    }

    fMutexSemListSem.request();

    if (garbageCollect)
    {
        // Register for notification when the handle ends so can perform
        // garbage collection
  
        STAFServiceResult serviceResult = gHandleManagerPtr->addNotification(
            requestInfo.fHandle, requestInfo.fEndpoint, requestInfo.fMachine,
            requestInfo.fSTAFInstanceUUID, name(), sNotificationKey);

        if (serviceResult.fRC != kSTAFOk)
        {
            fMutexSemListSem.release();

            return STAFServiceResult(
                serviceResult.fRC,
                STAFString("An error occurred when the ") + name() +
                " service on machine '" + *gMachinePtr +
                "' attempted to register for garbage collection notification"
                " on endpoint '" + requestInfo.fEndpoint +
                "'.  Reason: " + serviceResult.fResult);
        }
    }

    if (fMutexSemList.find(uppercaseSemName) != fMutexSemList.end())
    {
        // The sempahore exists.  Get it and lock it.

        MutexSemPtr sem = fMutexSemList[uppercaseSemName];

        sem->lockPtr->request();

        if (sem->isOwned)
        {
            // It is safe to unlock the semaphore list here, as this
            // semaphore can't be deleted with requests pending

            fMutexSemListSem.release();

            // Add a request to the list, unlock the semaphore,
            // and wait to acquire ownership

            MutexRequest request(requestInfo.fSTAFInstanceUUID,
                                 requestInfo.fMachine,
                                 requestInfo.fHandleName,
                                 requestInfo.fHandle,
                                 requestInfo.fUser,
                                 requestInfo.fEndpoint,
                                 garbageCollect);

            MutexRequestPtr requestPtr = MutexRequestPtr(
                new MutexRequest(request), MutexRequestPtr::INIT);

            sem->requestList.push_back(requestPtr);
            sem->lockPtr->release();

            // Wait for the specified time for the mutex to become available

            requestPtr->avail->wait(timeout);

            sem->lockPtr->request();

            // Check if the handle that submitted this request was garbage
            // collected while we were waiting to acquire the mutex semaphore,
            // and if so, return an error.

            if (*requestPtr->garbageCollectedPtr)
            {
                sem->lockPtr->release();

                return STAFServiceResult(
                    kSTAFRequestCancelled,
                    "The handle that submitted this request no longer exists");
            }

            if ((requestPtr->avail->query() == kSTAFEventSemPosted) &&
                (requestPtr->retCode == kSTAFOk))
            {
                // We were posted and no error occurred, so we should have
                // been given ownership.  Yeah.

                sem->lockPtr->release();
            }
            else
            {
                // We timed out waiting for the semaphore or the request was
                // cancelled, so remove us from the pending request list

                sem->requestList.remove(requestPtr);

                // Now unlock the semaphore and return an error

                sem->lockPtr->release();

                // Delete the notification from the handle notification list

                if (garbageCollect)
                {
                    gHandleManagerPtr->deleteNotification(
                        requestInfo.fHandle, requestInfo.fEndpoint,
                        requestInfo.fMachine, requestInfo.fSTAFInstanceUUID,
                        name(), sNotificationKey);
                }
                
                if (requestPtr->retCode != kSTAFOk)
                {
                    return STAFServiceResult(
                        requestPtr->retCode, requestPtr->resultBuffer);
                }
                
                return STAFServiceResult(
                    kSTAFTimeout,
                    "Request did not complete within the requested time");
            }
        }
        else
        {
            // Take ownership of the semaphore

            sem->isOwned = 1;
            sem->owner = MutexRequest(requestInfo.fSTAFInstanceUUID,
                                      requestInfo.fMachine,
                                      requestInfo.fHandleName,
                                      requestInfo.fHandle,
                                      requestInfo.fUser,
                                      requestInfo.fEndpoint,
                                      garbageCollect);
            sem->acquireTimeDate = STAFTimestamp::now();

            // Now release the locks on the semphore and the list

            fMutexSemListSem.release();
            sem->lockPtr->release();
        }
    }
    else
    {
        // The semaphore doesn't exist, so create it.

        fMutexSemList[uppercaseSemName] = MutexSemPtr(
            new MutexSem(
                semName, 1, 
                MutexRequest(requestInfo.fSTAFInstanceUUID,
                             requestInfo.fMachine,
                             requestInfo.fHandleName,
                             requestInfo.fHandle,
                             requestInfo.fUser,
                             requestInfo.fEndpoint,
                             garbageCollect)),
            MutexSemPtr::INIT);

        // Now release the lock on the list

        fMutexSemListSem.release();
    }

    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFSemService::handleRelease(
    const STAFServiceRequest &requestInfo)
{
    STAFCommandParseResultPtr parsedResult =
        fReleaseParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("MUTEX", semName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString uppercaseSemName = semName.toUpperCase();

    // Verify that the requesting machine/user has at least trust level 3 if
    // FORCE is not specified and at least trust level 4 if FORCE is specified

    if (parsedResult->optionTimes("FORCE") == 0)
    {
        IVALIDATE_TRUST(3, "RELEASE");
    }
    else
    {
        IVALIDATE_TRUST(4, "RELEASE FORCE");
    }
    
    fMutexSemListSem.request();

    if (fMutexSemList.find(uppercaseSemName) == fMutexSemList.end())
    {
        fMutexSemListSem.release();
        return kSTAFSemaphoreDoesNotExist;
    }

    // Ok, the semaphore exists.  Get it and lock it, then release the
    // lock on the semaphore list.

    MutexSemPtr sem = fMutexSemList[uppercaseSemName];
    STAFMutexSemLock semLock(*sem->lockPtr);

    fMutexSemListSem.release();

    // If the semaphore is not owned, then there is nothing to do so
    // we can just return

    if (!sem->isOwned)
        return STAFServiceResult(kSTAFOk, result);
    
    // Verify we are allowed to release the semaphore
     
    if (((requestInfo.fSTAFInstanceUUID != sem->owner.stafInstanceUUID) ||
        (requestInfo.fHandle != sem->owner.handle)) &&
        (parsedResult->optionTimes("FORCE") == 0))
    {
        return STAFServiceResult(
            kSTAFNotSemaphoreOwner,
            "Cannot release a sempahore for which your handle is not the "
            "owner unless the FORCE option is specified");
    }
    
    // Check if garbage collection is enabled
     
    if (sem->owner.garbageCollect)
    {
        gHandleManagerPtr->deleteNotification(sem->owner.handle,
                                              sem->owner.endpoint,
                                              sem->owner.machine,
                                              sem->owner.stafInstanceUUID,
                                              name(), sNotificationKey);
    }
    
    if (sem->requestList.size() != 0)
    {
        // There is a pending request.  Remove it from the list and
        // notify the waiter.

        sem->acquireTimeDate = STAFTimestamp::now();

        sem->owner = *sem->requestList.front();
        sem->requestList.pop_front();

        sem->owner.avail->post();
    }
    else
    {
        sem->isOwned = 0;
    }

    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFSemService::handleCancel(
    const STAFServiceRequest &requestInfo)
{
    STAFCommandParseResultPtr parsedResult = fCancelParser.parse(
        requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("MUTEX", semName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString uppercaseSemName = semName.toUpperCase();

    // Determine if the FORCE option is specified

    unsigned int force = parsedResult->optionTimes("FORCE");

    // Verify that the requesting machine/user has at least trust level 3
    // We'll check later if the FORCE is specified and if the requester is
    // not the owner of the mutex that has at least trust level 4.
    
    IVALIDATE_TRUST(3, "CANCEL");
    
    // Default machine, handle, and handle name to that of the originating
    // requester who is submitting the CANCEL request

    STAFString orgMachine = requestInfo.fMachine;
    STAFHandle_t orgHandle = requestInfo.fHandle;
    STAFString orgHandleName = requestInfo.fHandleName;

    if (force)
    {
        // Determine if the MACHINE option is specified

        if (parsedResult->optionTimes("MACHINE") > 0)
        {
            // Resolve the MACHINE option value

            rc = RESOLVE_OPTIONAL_STRING_OPTION("MACHINE", orgMachine);

            if (rc) return STAFServiceResult(rc, errorBuffer);
        }

        // Determine if the HANDLE or NAME option is specified

        if (parsedResult->optionTimes("HANDLE") > 0)
        {
            // Resolve the HANDLE option and verify that it can be converted
            // to an unsigned integer in the range for valid handle numbers

            STAFRC_t rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
                "HANDLE", orgHandle, gHandleManagerPtr->getMinHandleNumber(),
                gHandleManagerPtr->getMaxHandleNumber());

            if (rc) return STAFServiceResult(rc, errorBuffer);
            
            orgHandleName = "";  // Indicates not to check for a match
        }
        else if (parsedResult->optionTimes("NAME") > 0)
        {
            // Resolve the NAME option value

            rc = RESOLVE_OPTIONAL_STRING_OPTION("NAME", orgHandleName);

            if (rc) return STAFServiceResult(rc, errorBuffer);

            orgHandle = 0;  // Indicates not to check for a match
        }
    }

    // Determine if the FIRST or LAST option is specified.
    // The default is to find the last entry in the Pending Requests List
    // that matches the criteria.  The FIRST option can be specified to
    // find the first entry in the Pending Requests List that matches.

    bool first = false;   // Last is the default

    if (parsedResult->optionTimes("FIRST") > 0)
        first = true;

    // Get a lock on the semaphore list

    fMutexSemListSem.request();

    // Check if the mutex semaphore exists

    if (fMutexSemList.find(uppercaseSemName) == fMutexSemList.end())
    {
        fMutexSemListSem.release();

        return STAFServiceResult(
            kSTAFSemaphoreDoesNotExist,
            "Mutex semaphore " + semName + " does not exist");
    }

    // Ok, the semaphore exists.  Get it and lock it, then release the
    // lock on the semaphore list.

    MutexSemPtr sem = fMutexSemList[uppercaseSemName];
    STAFMutexSemLock semLock(*sem->lockPtr);

    fMutexSemListSem.release();

    // Check if a pending request entry matches all the following conditions:
    // - A handle# is not specified or it matches and
    // - A handle name is not specified or it matches (case-insensitive)
    // - A machine is not specified or it matches (case-insensitive)

    bool requestFound = false;
    MutexRequestPtr reqPtr;  // Pointer to the matching pending request;
    
    if (sem->requestList.size() != 0)
    {
        if (first)
        {
            // The FIRST option was specified, so iterate forward in the
            // Pending Requests list to find the first entry that matches
            // the criteria
        
            for (MutexRequestList::iterator iter = sem->requestList.begin();
                 iter != sem->requestList.end() && !requestFound; ++iter)
            {                                          
                reqPtr = *iter;

                if (((orgHandle == 0) || (reqPtr->handle == orgHandle)) &&
                    ((orgHandleName == "") || (reqPtr->handleName.isEqualTo(
                        orgHandleName, kSTAFStringCaseInsensitive))) &&
                    ((orgMachine == "") || (reqPtr->machine.isEqualTo(
                        orgMachine, kSTAFStringCaseInsensitive))))
                {
                    requestFound = true;
                }
            }
        }
        else
        {
            // The LAST option is specified (or neither the FIRST or LAST
            // option was specified), so do a reverse iterate to find the
            // last entry in the Pending Requests list that matches the
            // criteria

            for (MutexRequestList::reverse_iterator riter = sem->requestList.rbegin();
                 riter != sem->requestList.rend() && !requestFound; ++riter)
            {
                reqPtr = *riter;

                if (((orgHandle == 0) || (reqPtr->handle == orgHandle)) &&
                    ((orgHandleName == "") || (reqPtr->handleName.isEqualTo(
                        orgHandleName, kSTAFStringCaseInsensitive))) &&
                    ((orgMachine == "") || (reqPtr->machine.isEqualTo(
                        orgMachine, kSTAFStringCaseInsensitive))))
                {
                    requestFound = true;
                }
            }
        }
    }

    if (!requestFound)
    {
        STAFString noMatchMessage = "No pending requests exist that match "
            "the following criteria:";

        if (orgHandle != 0)
            noMatchMessage = noMatchMessage + " Handle=" + orgHandle;

        if (orgHandleName != "")
            noMatchMessage = noMatchMessage + " HandleName=" + orgHandleName;

        if (orgMachine != "")
            noMatchMessage = noMatchMessage + " Machine=" + orgMachine;

        return STAFServiceResult(kSTAFDoesNotExist, noMatchMessage);
    }

    // Check if you are the requester of the matching pending request entry

    unsigned int requester = 0;   // 0 means not the requester

    if ((reqPtr->stafInstanceUUID == requestInfo.fSTAFInstanceUUID) &&
        (reqPtr->handle == requestInfo.fHandle))
    {
        requester = 1;    // 1 means you are the requester
    }

    // If you are not the requester and FORCE is specified, need trust level 4

    if (!requester && force)
    {
        // Verify the requester has at least trust level 4

        IVALIDATE_TRUST(4, "CANCEL FORCE");
    }

    // To cancel the request, you must be the requester or the FORCE option
    // must be specified

    if (!requester && !force)
    {
        return STAFServiceResult(
            kSTAFNotRequester, "Cannot cancel a request you did not submit "
            "unless you specify the FORCE option");
    }
    
    // Tell its requester the request was cancelled and wake it up

    reqPtr->retCode = kSTAFRequestCancelled;
    reqPtr->resultBuffer = "The request was cancelled by a SEM CANCEL "
        "request submitted by handle " + STAFString(requestInfo.fHandle) +
        " (" + requestInfo.fHandleName + ") on machine " +
        requestInfo.fMachine;
    reqPtr->avail->post();
    
    return STAFServiceResult(kSTAFOk);
}

STAFServiceResult STAFSemService::handlePostPulse(
    const STAFServiceRequest &requestInfo)
{
    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fPostPulseParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }                             

    // Verify that the requesting machine/user has at least trust level 3
    
    if (parsedResult->optionTimes("POST") != 0)
    {
        IVALIDATE_TRUST(3, "POST");
    }
    else
    {
        IVALIDATE_TRUST(3, "PULSE");
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("EVENT", semName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString uppercaseSemName = semName.toUpperCase();

    fEventSemListSem.request();

    if (fEventSemList.find(uppercaseSemName) == fEventSemList.end())
    {
        // Semaphore does not exist, so create it in a posted state
        // if on POST, or in a reset state if on PULSE

        EventSemPtr sem = EventSemPtr(new EventSem(semName), 
                                      EventSemPtr::INIT);

        sem->lastPost = EventUnit(requestInfo.fSTAFInstanceUUID,
                                  requestInfo.fMachine,
                                  requestInfo.fHandleName,
                                  requestInfo.fHandle,
                                  requestInfo.fUser,
                                  requestInfo.fEndpoint);

        if (parsedResult->optionTimes("POST") != 0)
        {
            sem->eventSem->post();
        }
        else
        {
            sem->eventSem->reset();
            sem->lastReset = sem->lastPost;
        }

        fEventSemList[uppercaseSemName] = sem;

        fEventSemListSem.release();
    }
    else
    {
        // The semaphore does exist, so lock the sem, unlock the main
        // list, clear the waiter list, and post/pulse it.

        EventSemPtr sem = fEventSemList[uppercaseSemName];

        sem->lockPtr->request();
        fEventSemListSem.release();

        sem->waiterList.clear();
        sem->lastPost = EventUnit(requestInfo.fSTAFInstanceUUID,
                                  requestInfo.fMachine,
                                  requestInfo.fHandleName,
                                  requestInfo.fHandle,
                                  requestInfo.fUser,
                                  requestInfo.fEndpoint);
        sem->eventSem->post();

        if (parsedResult->optionTimes("PULSE") != 0)
        {
            sem->eventSem->reset();
            sem->lastReset = sem->lastPost;
        }

        sem->lockPtr->release();
    }

    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFSemService::handleReset(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    IVALIDATE_TRUST(3, "RESET");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fResetParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }                             

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("EVENT", semName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString uppercaseSemName = semName.toUpperCase();

    fEventSemListSem.request();

    if (fEventSemList.find(uppercaseSemName) == fEventSemList.end())
    {
        // Semaphore does not exist, so create it in a reset state

        EventSemPtr sem = EventSemPtr(new EventSem(semName), 
                                      EventSemPtr::INIT);

        sem->lastReset = EventUnit(requestInfo.fSTAFInstanceUUID,
                                   requestInfo.fMachine,
                                   requestInfo.fHandleName,
                                   requestInfo.fHandle,
                                   requestInfo.fUser,
                                   requestInfo.fEndpoint);
        sem->eventSem->reset();

        fEventSemList[uppercaseSemName] = sem;

        fEventSemListSem.release();
    }
    else
    {
        // The semaphore does exist, so lock the sem, unlock the main
        // list, and reset it.

        EventSemPtr sem = fEventSemList[uppercaseSemName];

        sem->lockPtr->request();
        fEventSemListSem.release();

        sem->lastReset = EventUnit(requestInfo.fSTAFInstanceUUID,
                                   requestInfo.fMachine,
                                   requestInfo.fHandleName,
                                   requestInfo.fHandle,
                                   requestInfo.fUser,
                                   requestInfo.fEndpoint);
        sem->eventSem->reset();

        sem->lockPtr->release();
    }

    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFSemService::handleWait(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    IVALIDATE_TRUST(3, "WAIT");
    
    // Parse the request
    
    STAFCommandParseResultPtr parsedResult =
        fWaitParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }                             

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("EVENT", semName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString uppercaseSemName = semName.toUpperCase();

    unsigned int timeout = STAF_EVENT_SEM_INDEFINITE_WAIT;
    
    rc = RESOLVE_DEFAULT_DURATION_OPTION(
        "TIMEOUT", timeout, STAF_EVENT_SEM_INDEFINITE_WAIT);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    EventUnit us = EventUnit(requestInfo.fSTAFInstanceUUID,
                             requestInfo.fMachine,
                             requestInfo.fHandleName,
                             requestInfo.fHandle,
                             requestInfo.fUser,
                             requestInfo.fEndpoint);
    EventSemPtr sem;

    fEventSemListSem.request();

    if (fEventSemList.find(uppercaseSemName) == fEventSemList.end())
    {
        // Semaphore does not exist, so create it in a reset state

        sem = EventSemPtr(new EventSem(semName), EventSemPtr::INIT);

        sem->lastReset = us;
        sem->eventSem->reset();

        fEventSemList[uppercaseSemName] = sem;
    }
    else
    {
        // The semaphore does exist, so get it.

        sem = fEventSemList[uppercaseSemName];
    }

    // Lock the semaphore, and unlock the main list.

    sem->lockPtr->request();
    fEventSemListSem.release();

    // First check to make sure that the semaphore is not already posted.
    // If it is posted, simply unlock the semaphore and return.

    if (sem->eventSem->query() == kSTAFEventSemPosted)
    {
        sem->lockPtr->release();
        return kSTAFOk;
    }

    // Ok, the semaphore is reset, so add us to the waiting list, and
    // unlock the semaphore.

    sem->waiterList.push_front(us);
    sem->lockPtr->release();

    // Now wait on it

    unsigned int waitRC = sem->eventSem->wait(timeout);

    // Re-acquire our lock on the semaphore

    sem->lockPtr->request();

    if (waitRC == 0)
    {
        // The sem was posted while we were trying to get a lock.
        // Yeah!!  We were removed from the waiting list by the
        // posting thread.

        sem->lockPtr->release();
    }
    else
    {
        // We timed out waiting for the semaphore, so remove
        // us from the waiting list

        int foundUs = 0;
        EventWaiterList::iterator iter;

        for(iter = sem->waiterList.begin(); !foundUs && 
            iter != sem->waiterList.end(); iter++)
        {
            if ((iter->stafInstanceUUID == requestInfo.fSTAFInstanceUUID) &&
                (iter->machine == requestInfo.fMachine) &&
                (iter->handle == requestInfo.fHandle))
            {
                foundUs = 1;
                sem->waiterList.erase(iter);
                break;
            }
        }

        // Now unlock the semaphore and return a timeout error

        sem->lockPtr->release();

        return kSTAFTimeout;
    }

    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFSemService::handleDelete(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "DELETE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fDeleteParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;
    STAFString semName;

    if (parsedResult->optionTimes("MUTEX") != 0)
    {
        STAFRC_t rc = RESOLVE_STRING_OPTION("MUTEX", semName);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString uppercaseSemName = semName.toUpperCase();

        STAFMutexSemLock lock(fMutexSemListSem);

        if (fMutexSemList.find(uppercaseSemName) == fMutexSemList.end())
        {
            return STAFServiceResult(
                kSTAFSemaphoreDoesNotExist,
                "Mutex semaphore " + semName + " does not exist");
        }

        // Ok, the semaphore exists.  Get it and lock it.

        MutexSemPtr sem = fMutexSemList[uppercaseSemName];
        STAFMutexSemLock semLock(*sem->lockPtr);

        if (!sem->isOwned)
            fMutexSemList.erase(uppercaseSemName);
        else
            return kSTAFSemaphoreHasPendingRequests;
    }
    else
    {
        STAFRC_t rc = RESOLVE_STRING_OPTION("EVENT", semName);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString uppercaseSemName = semName.toUpperCase();

        STAFMutexSemLock lock(fEventSemListSem);

        if (fEventSemList.find(uppercaseSemName) == fEventSemList.end())
            return kSTAFSemaphoreDoesNotExist;

        // Ok, the semaphore exists.  Get it and lock it.

        EventSemPtr sem = fEventSemList[uppercaseSemName];
        STAFMutexSemLock semLock(*sem->lockPtr);

        if (sem->waiterList.empty())
            fEventSemList.erase(uppercaseSemName);
        else
            return kSTAFSemaphoreHasPendingRequests;
    }
    
    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFSemService::handleQuery(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "QUERY");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fQueryParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString semName;

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("MUTEX") != 0)
    {
        STAFRC_t rc = RESOLVE_STRING_OPTION("MUTEX", semName);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString uppercaseSemName = semName.toUpperCase();
        
        fMutexSemListSem.request();

        if (fMutexSemList.find(uppercaseSemName) == fMutexSemList.end())
        {
            fMutexSemListSem.release();
            return STAFServiceResult(
                kSTAFSemaphoreDoesNotExist,
                "Mutex semaphore " + semName + " does not exist");
        }

        // Create a marshalled map containing information about the semaphore

        mc->setMapClassDefinition(fQueryMutexClass->reference());
        mc->setMapClassDefinition(fMutexOwnerClass->reference());
        mc->setMapClassDefinition(fPendingRequestClass->reference());
        STAFObjectPtr mutexMap = fQueryMutexClass->createInstance();

        // Ok, the semaphore exists.  Get it and lock it, then release the
        // lock on the semaphore list.

        MutexSemPtr sem = fMutexSemList[uppercaseSemName];
        STAFMutexSemLock semLock(*sem->lockPtr);

        fMutexSemListSem.release();

        STAFObjectPtr pendingRequests = STAFObject::createList();

        if (sem->isOwned)
        {
            mutexMap->put("state", "Owned");

            STAFObjectPtr ownerMap = fMutexOwnerClass->createInstance();
            ownerMap->put("machine", sem->owner.machine);
            ownerMap->put("handleName", sem->owner.handleName);
            ownerMap->put("handle", STAFString(sem->owner.handle));
            ownerMap->put("user", sem->owner.user);
            ownerMap->put("endpoint", sem->owner.endpoint);
            ownerMap->put("requestTimestamp",
                          sem->owner.timeDate.asString());
            ownerMap->put("acquireTimestamp",
                          sem->acquireTimeDate.asString());

            if (sem->owner.garbageCollect)
                ownerMap->put("gc", "Yes");
            else
                ownerMap->put("gc", "No");

            mutexMap->put("owner", ownerMap);

            if (!sem->requestList.empty())
            {
                MutexRequestList::iterator iter;
                int i = 1;

                for(iter = sem->requestList.begin();
                    iter != sem->requestList.end();
                    ++i, ++iter)
                {
                    MutexRequestPtr request = *iter;

                    STAFObjectPtr requestMap =
                        fPendingRequestClass->createInstance();

                    requestMap->put("machine", request->machine);
                    requestMap->put("handleName", request->handleName);
                    requestMap->put("handle", STAFString(request->handle));
                    requestMap->put("user", request->user);
                    requestMap->put("endpoint", request->endpoint);
                    requestMap->put("requestTimestamp",
                                    request->timeDate.asString());

                    if (request->garbageCollect)
                        requestMap->put("gc", "Yes");
                    else
                        requestMap->put("gc", "No");

                    pendingRequests->append(requestMap);
                }
            }

        }
        else
        {
            mutexMap->put("state", "Unowned");
        }

        mutexMap->put("requestList", pendingRequests);

        mc->setRootObject(mutexMap);
    }
    else
    {
        STAFRC_t rc = RESOLVE_STRING_OPTION("EVENT", semName);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString uppercaseSemName = semName.toUpperCase();

        fEventSemListSem.request();

        if (fEventSemList.find(uppercaseSemName) == fEventSemList.end())
        {
            fEventSemListSem.release();
            return kSTAFSemaphoreDoesNotExist;
        }

        // Create a marshalled map containing information about the semaphore

        mc->setMapClassDefinition(fQueryEventClass->reference());
        mc->setMapClassDefinition(fEventRequesterClass->reference());
        STAFObjectPtr eventMap = fQueryEventClass->createInstance();

        // Ok, the semaphore exists.  Get it and lock it, then release the
        // lock on the semaphore list.

        EventSemPtr sem = fEventSemList[uppercaseSemName];
        STAFMutexSemLock lock(*sem->lockPtr);

        fEventSemListSem.release();

        if (sem->eventSem->query() == kSTAFEventSemPosted)
            eventMap->put("state", STAFString("Posted"));
        else
            eventMap->put("state", STAFString("Reset"));

        STAFObjectPtr lastPostedMap = fEventRequesterClass->createInstance();

        lastPostedMap->put("machine", sem->lastPost.machine);
        lastPostedMap->put("handleName", sem->lastPost.handleName);
        lastPostedMap->put("handle", STAFString(sem->lastPost.handle));
        lastPostedMap->put("user", sem->lastPost.user);
        lastPostedMap->put("endpoint", sem->lastPost.endpoint);
        lastPostedMap->put("timestamp", sem->lastPost.timeDate.asString());

        eventMap->put("lastPosted", lastPostedMap);

        STAFObjectPtr lastResetMap = fEventRequesterClass->createInstance();

        lastResetMap->put("machine", sem->lastReset.machine);
        lastResetMap->put("handleName", sem->lastReset.handleName);
        lastResetMap->put("handle", STAFString(sem->lastReset.handle));
        lastResetMap->put("user", sem->lastReset.user);
        lastResetMap->put("endpoint", sem->lastReset.endpoint);
        lastResetMap->put("timestamp", sem->lastReset.timeDate.asString());

        eventMap->put("lastReset", lastResetMap);
        
        STAFObjectPtr outputList = STAFObject::createList();

        if (sem->eventSem->query() == kSTAFEventSemReset)
        {
            if (sem->waiterList.size() != 0)
            {
                EventWaiterList::iterator iter;
                int i = 1;

                for(iter = sem->waiterList.begin();
                    iter != sem->waiterList.end(); ++i, ++iter)
                {
                    EventUnit eventUnit = *iter;

                    STAFObjectPtr waiterMap =
                        fEventRequesterClass->createInstance();
                    waiterMap->put("machine", eventUnit.machine);
                    waiterMap->put("handleName", eventUnit.handleName);
                    waiterMap->put("handle", STAFString(eventUnit.handle));
                    waiterMap->put("user", eventUnit.user);
                    waiterMap->put("endpoint", eventUnit.endpoint);
                    waiterMap->put("timestamp", eventUnit.timeDate.asString());

                    outputList->append(waiterMap);
                }
            }

        }

        eventMap->put("waiterList", outputList);

        mc->setRootObject(eventMap);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}

STAFServiceResult STAFSemService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListParser.parse(
        requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("MUTEX") != 0)
    {
        // Create a marshalled list of maps containing mutex semaphore info

        mc->setMapClassDefinition(fMutexInfoClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();
        
        STAFMutexSemLock lock(fMutexSemListSem);
        MutexSemList::iterator iter;

        for(iter = fMutexSemList.begin(); iter != fMutexSemList.end(); iter++)
        {
            MutexSemPtr sem = iter->second;
            STAFMutexSemLock semLock(*sem->lockPtr);

            STAFObjectPtr mutexInfoMap = fMutexInfoClass->createInstance();

            mutexInfoMap->put("name", STAFString(sem->name));

            if (sem->isOwned)
            {
                mutexInfoMap->put("state", STAFString("Owned"));
            }
            else
            {
                mutexInfoMap->put("state", STAFString("Unowned"));
            }

            mutexInfoMap->put("pendingRequests",
                              STAFString(sem->requestList.size()));

            outputList->append(mutexInfoMap);
        }

        mc->setRootObject(outputList);
    }
    else
    {
       // Create a marshalled list of maps containing event semaphore info

        mc->setMapClassDefinition(fEventInfoClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();

        STAFMutexSemLock lock(fEventSemListSem);
        EventSemList::iterator iter;
        
        for(iter = fEventSemList.begin(); iter != fEventSemList.end(); iter++)
        {
            EventSemPtr sem = iter->second;
            STAFMutexSemLock semLock(*sem->lockPtr);

            STAFObjectPtr eventInfoMap = fEventInfoClass->createInstance();

            eventInfoMap->put("name", STAFString(sem->name));

            if (sem->eventSem->query() == kSTAFEventSemReset)
            {
                eventInfoMap->put("state", STAFString("Reset"));
            }
            else
            {
                eventInfoMap->put("state", STAFString("Posted"));
            }

            eventInfoMap->put("waiters", STAFString(sem->waiterList.size()));

            outputList->append(eventInfoMap);
        }

        mc->setRootObject(outputList);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFSemService::handleSTAFCallback(
    const STAFServiceRequest &requestInfo)
{
    STAFString result = " ";

    // Don't check the TRUST level, but make sure the requesting handle
    // is the STAFProc handle

    if (requestInfo.fHandle != 1)
    {
        return STAFServiceResult(
            kSTAFAccessDenied,
            "This request is only valid when submitted by STAFProc");
    }
    
    STAFCommandParseResultPtr parsedResult =
        fSTAFCallbackParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(
            kSTAFInvalidRequestString, parsedResult->errorBuffer, 0);
    }
    
    STAFString handle = parsedResult->optionValue("HANDLE");
    STAFString machine = parsedResult->optionValue("MACHINE");
    STAFString uuid = parsedResult->optionValue("UUID");

    STAFMutexSemLock lock(fMutexSemListSem);
    MutexSemList::iterator iter;

    // For each mutex semaphore, perform garbage collection as follows:
    // 1) Remove any pending requests that were requested by this handle
    //    that no longer exists, and
    // 2) Release the mutex semaphore if owned by this handle that no
    //    longer exists and wake up any pending requests

    for(iter = fMutexSemList.begin(); iter != fMutexSemList.end(); iter++)
    {
        MutexSemPtr sem = fMutexSemList[iter->second->uppercaseName];
        STAFMutexSemLock semLock(*sem->lockPtr);

        // First, remove any pending requests for this mutex semaphore that
        // were requested by this handle that no longer exists
        
        if (sem->requestList.size() > 0)
        {
            bool removedRequest = true;

            while (removedRequest)
            {
                removedRequest = false;
                MutexRequestList::iterator reqListIter;

                for (reqListIter = sem->requestList.begin();
                     reqListIter != sem->requestList.end(); ++reqListIter)
                {
                    // Check if this handle submitted the pending request
                    // and if it's supposed to be garbage collected

                    if (((*reqListIter)->garbageCollect) &&
                        (STAFString((*reqListIter)->handle) == handle) &&
                        ((*reqListIter)->stafInstanceUUID == uuid))
                    {
                        // The handle for this pending request no longer
                        // exists, so remove this pending request

                        sem->requestList.erase(reqListIter);

                        // Set the flag indicating that the pending request
                        // has been garbage collected and wake up the
                        // requester

                        *(*reqListIter)->garbageCollectedPtr = true;
                        (*reqListIter)->avail->post();
                          
                        // Break since can't continue iterating a list
                        // that has been changed

                        removedRequest = true;
                        break;
                    }
                }
            }
        }

        // Second, release the mutex semaphore if owned by this handle that
        // no longer exists and then wake up any pending requests

        if (sem->isOwned && sem->owner.garbageCollect &&
            (STAFString(sem->owner.handle) == handle) &&
            (sem->owner.stafInstanceUUID == uuid))
        {
            if (sem->requestList.size() > 0)
            {
                // There is a pending request.  Remove it from the list and
                // notify the waiter.
    
                sem->acquireTimeDate = STAFTimestamp::now();
    
                sem->owner = *sem->requestList.front();
                sem->requestList.pop_front();

                sem->owner.avail->post();
            }
            else 
            {
                sem->isOwned = 0;
            }
        }
    }

    return STAFServiceResult(kSTAFOk, result);
}


STAFServiceResult STAFSemService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
