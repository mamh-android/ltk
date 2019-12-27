/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
 
#include "STAF.h"
#include "STAFProc.h"
#include "STAFHandleService.h"
#include "STAFHandleManager.h"
#include "STAFProcUtil.h"
#include "STAFServiceManager.h"
#include "STAFTrace.h"

STAFHandleService *STAFHandleService::sHandleService = 0;
static STAFString sHelpMsg;

STAFHandleService::STAFHandleService() : STAFService("HANDLE")
{
    sHandleService = this;

    // Assign the help text string for the service

    sHelpMsg = STAFString("*** HANDLE Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "CREATE HANDLE NAME <Handle Name>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DELETE HANDLE <Number>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "QUERY HANDLE <Handle>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST [ HANDLES <[NAME <Handle Name>] [LONG] [PENDING] [REGISTERED]" +
        *gLineSeparatorPtr +
        "                [INPROCESS] [STATIC]> | [SUMMARY] ]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        // We don't document the LIST NOTIFICATIONS request which we use
        // for debugging garbage collection
        //"LIST NOTIFICATIONS [POLLING] [LONG]" +
        //*gLineSeparatorPtr + *gLineSeparatorPtr +
        "AUTHENTICATE USER <User Identifier> CREDENTIALS <Credentials>" +
        *gLineSeparatorPtr +
        "             [AUTHENTICATOR <Authenticator Name>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "UNAUTHENTICATE" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // query options

    fQueryParser.addOption("QUERY",      1,
        STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption("HANDLE",    1,
        STAFCommandParser::kValueRequired);
 
    // query groups

    fQueryParser.addOptionGroup("QUERY",    1, 1);

    // query needs

    fQueryParser.addOptionNeed("QUERY", "HANDLE");

    // create options

    fCreateParser.addOption("CREATE",      1,
        STAFCommandParser::kValueNotAllowed);
    fCreateParser.addOption("HANDLE",      1,
        STAFCommandParser::kValueNotAllowed);
    fCreateParser.addOption("NAME",        1,
        STAFCommandParser::kValueRequired);

    // create needs

    fCreateParser.addOptionNeed("CREATE", "HANDLE");
    fCreateParser.addOptionNeed("CREATE", "NAME");

    // notify options

    fNotifyParser.addOption("STAF_NOTIFY", 1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("REGISTER", 1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("UNREGISTER", 1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("ONENDOFHANDLE", 1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("MACHINE", 1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("UUID", 1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("SERVICE", 1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("KEY", 1,
        STAFCommandParser::kValueRequired);

    // notify needs

    fNotifyParser.addOptionNeed("REGISTER UNREGISTER", "STAF_NOTIFY");
    fNotifyParser.addOptionNeed("STAF_NOTIFY", "ONENDOFHANDLE");
    fNotifyParser.addOptionNeed("STAF_NOTIFY", "MACHINE");
    fNotifyParser.addOptionNeed("STAF_NOTIFY", "SERVICE");
    fNotifyParser.addOptionNeed("STAF_NOTIFY", "KEY");

    // notify groups

    fNotifyParser.addOptionGroup("REGISTER UNREGISTER", 0, 1);

    // list options

    fListParser.addOption("LIST",          1,
        STAFCommandParser::kValueNotAllowed);

    // These are for the LIST NOTIFICATIONS request

    fListParser.addOption("NOTIFICATIONS", 1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("POLLING",       1,
        STAFCommandParser::kValueNotAllowed);

    // These are for the LIST HANDLES request

    fListParser.addOption("HANDLES",        1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("NAME",           1,
        STAFCommandParser::kValueRequired);
    fListParser.addOption("PENDING",        1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("REGISTERED",     1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("INPROCESS",      1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("STATIC",         1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("LONG",           1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SUMMARY",        1,
        STAFCommandParser::kValueNotAllowed);

    // LIST groups
    fListParser.addOptionGroup("HANDLES NOTIFICATIONS", 0, 1);
    fListParser.addOptionGroup("SUMMARY LONG", 0, 1);
    fListParser.addOptionGroup("SUMMARY NAME", 0, 1);
    fListParser.addOptionGroup("SUMMARY PENDING", 0, 1);
    fListParser.addOptionGroup("SUMMARY REGISTERED", 0, 1);
    fListParser.addOptionGroup("SUMMARY INPROCESS", 0, 1);
    fListParser.addOptionGroup("SUMMARY STATIC", 0, 1);

    // list needs
    fListParser.addOptionNeed("PENDING REGISTERED INPROCESS STATIC",
                              "HANDLES");
    fListParser.addOptionNeed("LONG", "HANDLES NOTIFICATIONS");
    fListParser.addOptionNeed("POLLING", "NOTIFICATIONS");
    fListParser.addOptionNeed("SUMMARY", "HANDLES");

    // delete options

    fDeleteParser.addOption("DELETE",      1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("HANDLE",      1,
        STAFCommandParser::kValueRequired);

    // delete needs

    fDeleteParser.addOptionNeed("DELETE", "HANDLE");

    // authenticate options

    fAuthParser.addOption("AUTHENTICATE", 1,
        STAFCommandParser::kValueNotAllowed);
    fAuthParser.addOption("USER",         1,
        STAFCommandParser::kValueRequired);
    fAuthParser.addOption("CREDENTIALS",  1,
        STAFCommandParser::kValueRequired);
    fAuthParser.addOption("AUTHENTICATOR",  1,
        STAFCommandParser::kValueRequired);

    // authenticate needs

    fAuthParser.addOptionNeed("AUTHENTICATE", "USER");
    fAuthParser.addOptionNeed("USER", "CREDENTIALS");

    // un-authenticate options

    fUnAuthParser.addOption("UNAUTHENTICATE", 1,
        STAFCommandParser::kValueNotAllowed);

    // Construct map class for handle information used in QUERY HANDLE

    fQueryHandleClass = STAFMapClassDefinition::create(
        "STAF/Service/Handle/QueryHandle");
    fQueryHandleClass->addKey("handle", "Handle");
    fQueryHandleClass->addKey("name", "Handle Name");
    fQueryHandleClass->addKey("state", "State");
    fQueryHandleClass->addKey("lastUsedTimestamp", "Last Used Date-Time");
    fQueryHandleClass->addKey("pid", "PID");
    fQueryHandleClass->addKey("user", "User");
    fQueryHandleClass->addKey("instanceUUID", "Instance UUID");

    // Construct map class for handle information used in LIST HANDLES

    fHandleInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Handle/HandleInfo");
    fHandleInfoClass->addKey("handle", "Handle");
    fHandleInfoClass->setKeyProperty("handle", "display-short-name", "H#");
    fHandleInfoClass->addKey("name", "Handle Name");
    fHandleInfoClass->setKeyProperty("name", "display-short-name", "Name");
    fHandleInfoClass->addKey("state", "State");
    fHandleInfoClass->addKey("lastUsedTimestamp", "Last Used Date-Time");
    fHandleInfoClass->setKeyProperty("lastUsedTimestamp",
                                     "display-short-name", "Last Used");
    
    // Construct map class for handle information used in LIST HANDLES LONG

    fHandleInfoLongClass = STAFMapClassDefinition::create(
        "STAF/Service/Handle/HandleInfoLong");
    fHandleInfoLongClass->addKey("handle", "Handle");
    fHandleInfoLongClass->setKeyProperty("handle", "display-short-name", "H#");
    fHandleInfoLongClass->addKey("name", "Handle Name");
    fHandleInfoLongClass->setKeyProperty("name", "display-short-name", "Name");
    fHandleInfoLongClass->addKey("state", "State");
    fHandleInfoLongClass->addKey("lastUsedTimestamp", "Last Used Date-Time");
    fHandleInfoLongClass->setKeyProperty("lastUsedTimestamp",
                                     "display-short-name", "Last Used");
    fHandleInfoLongClass->addKey("pid", "PID");

    // Construct map class for handle summary info used in LIST HANDLES SUMMARY

    fHandleSummaryClass = STAFMapClassDefinition::create(
        "STAF/Service/Handle/HandleSummary");
    fHandleSummaryClass->addKey("activeHandles", "Active Handles");
    fHandleSummaryClass->addKey("totalHandles", "Total Handles");
    fHandleSummaryClass->addKey("resetCount", "Reset Count");
    fHandleSummaryClass->addKey("handleNumberRange", "Handle Number Range");
    fHandleSummaryClass->addKey("maxActiveHandles", "Maximum Active Handles");

    // Construct map class for LIST NOTIFICATIONS notifiee info

    fNotificationClass = STAFMapClassDefinition::create(
        "STAF/Service/Handle/NotificationInfo");
 
    fNotificationClass->addKey("handle", "Handle");
    fNotificationClass->addKey("machine", "Machine");
    fNotificationClass->addKey("notifyService", "Notify Service");

    // Construct map class for  LIST NOTIFICATIONS LONG notifiee info

    fNotificationLongClass = STAFMapClassDefinition::create(
        "STAF/Service/Handle/NotificationLong");
 
    fNotificationLongClass->addKey("handle", "Handle");
    fNotificationLongClass->addKey("endpoint", "Endpoint");
    fNotificationLongClass->addKey("machine", "Machine");
    fNotificationLongClass->addKey("instanceUUID", "Instance UUID");
    fNotificationLongClass->addKey("notifyService", "Notify Service");
    fNotificationLongClass->setKeyProperty("notifyService",
                                     "display-short-name", "Service");
    fNotificationLongClass->addKey("key", "Key");
}


STAFHandleService::~STAFHandleService()
{
    /* Do Nothing */
}


STAFString STAFHandleService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFHandleService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();

    if      (action == "query")        return handleQuery(requestInfo);
    else if (action == "list")         return handleList(requestInfo);
    else if (action == "staf_notify")  return handleNotify(requestInfo);
    else if (action == "create")       return handleCreate(requestInfo);
    else if (action == "delete")       return handleDelete(requestInfo);
    else if (action == "authenticate") return handleAuthenticate(requestInfo);
    else if (action == "unauthenticate")
    {
        return handleUnAuthenticate(requestInfo);
    }
    else if (action == "help")         return handleHelp(requestInfo);
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


STAFServiceResult STAFHandleService::handleCreate(
    const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine
    // (at any trust level)

    IVALIDATE_LOCAL_TRUST(0, "CREATE");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = fCreateParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString name;
    STAFRC_t rc = RESOLVE_STRING_OPTION("NAME", name);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFHandle_t theHandle = 0;

    STAFServiceResult result = gHandleManagerPtr->addAndGetStaticHandle(
        theHandle, name);

    if (result.fRC) return result;
    
    return STAFServiceResult(kSTAFOk, theHandle);
}


STAFServiceResult STAFHandleService::handleDelete(
    const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine and that
    // the requesting machine/user has at least trust level 5

    IVALIDATE_LOCAL_TRUST(5, "DELETE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fDeleteParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFHandle_t theHandle = 0;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = gHandleManagerPtr->removeStaticHandle(theHandle);

    if (rc) return STAFServiceResult(rc, theHandle);

    return kSTAFOk;
}


STAFServiceResult STAFHandleService::handleQuery(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    // Note:  Changed from trust level 2 to 1 in STAF V3.4.1 so that when
    //        HandleManager's gcPolling() method submits a QUERY request to
    //        the HANDLE service on a remote machine, there will be less of
    //        a chance that it will fail with a trust issue

    IVALIDATE_TRUST(1, "QUERY");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = fQueryParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    // Resolve the HANDLE option value

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    unsigned int handleNumber;
    
    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "HANDLE", handleNumber, gHandleManagerPtr->getMinHandleNumber(),
         gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Check if the handle exists

    STAFHandleManager::HandleData handleData;

    rc = gHandleManagerPtr->getHandleData(handleNumber, handleData);

    if (rc != kSTAFOk)
    {
        // The handle does not exist

        return STAFServiceResult(
            rc, STAFString("Handle number '") + STAFString(handleNumber) +
            "' does not exist");
    }
    else
    {
        // The handle exists so create a marshalled map containing information
        // about the handle

        STAFObjectPtr mc = STAFObject::createMarshallingContext();
        mc->setMapClassDefinition(fQueryHandleClass->reference());
        STAFObjectPtr handleMap = fQueryHandleClass->createInstance();
        
        STAFString handleState;

        if (handleData.state == STAFHandleManager::kPending)
        {
            handleState = "Pending";
        }
        else if ((handleData.state == STAFHandleManager::kRegistered) ||
                 (handleData.state == STAFHandleManager::kPendingRegistered))
        {
            handleState = "Registered";
        }
        else if (handleData.state == STAFHandleManager::kInProcess)
        {
            handleState = "InProcess";
        }
        else if (handleData.state == STAFHandleManager::kStatic)
        {
            handleState = "Static";
        }
        else
        {
            return STAFServiceResult(kSTAFUnknownError, "Unknown Handle State");
        }

        handleMap->put("handle", STAFString(handleNumber));

        if (handleData.name.length() != 0)
            handleMap->put("name", handleData.name);

        handleMap->put("state", handleState);
        handleMap->put(
            "lastUsedTimestamp", handleData.lastUsedTimestamp.asString());
        handleMap->put("pid", STAFString(handleData.pid));
        handleMap->put(
            "user", STAFString(handleData.authenticator) +
            gSpecSeparator + STAFString(handleData.userIdentifier));
        handleMap->put("instanceUUID", *gSTAFInstanceUUIDPtr);

        mc->setRootObject(handleMap);

        return STAFServiceResult(kSTAFOk, mc->marshall());
    }
}


STAFServiceResult STAFHandleService::handleAuthenticate(
    const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine
    // (at any trust level)

    IVALIDATE_LOCAL_TRUST(0, "AUTHENTICATE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fAuthParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    // Resolve any variables in USER, CREDENTIALS, and AUTHENTICATOR

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString authenticator =  gServiceManagerPtr->getDefaultAuthenticator();
    STAFString userIdentifier;
    STAFString errorBuffer;

    STAFRC_t rc = RESOLVE_STRING_OPTION("USER", userIdentifier);

    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("AUTHENTICATOR",
                                                 authenticator);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (authenticator == gNoneString)
    {
        return STAFServiceResult(kSTAFHandleAuthenticationDenied,
                                 "No Authenticators are registered");
    }

    STAFString credentialsValue = parsedResult->optionValue("CREDENTIALS");

    // Remove privacy delimiters, if any, from the credentials
    // e.g. !!@password@!! -> password

    STAFString credentials = STAFHandle::removePrivacyDelimiters(
        credentialsValue);

    // Issue an Authenticate request to the specified Authenticator service
    // passing in the credentials.  If the handle is already authenticated,
    // or if the authenticator specified is not registered or if authentication
    // fails, an error is returned.  If successful, authentication data is
    // returned in the result.

    STAFServiceResult serviceResult = gHandleManagerPtr->authenticate(
        requestInfo.fMachine, requestInfo.fHandle, authenticator,
        userIdentifier, STAFHandleManager::kCredentials, credentials);

    if (serviceResult.fRC != kSTAFOk) return serviceResult;

    // User was successfully authenticated

    STAFString authenticationData = serviceResult.fResult;

    // Cache the authentication information for the machine/handle/user/data
    // Can ignore any errors returned by cacheAuthenticationInfo() as an
    // error can only occur if the handle no longer exists, which should
    // never happen, but if it did, we wouldn't need to cache authinfo for it.

    gHandleManagerPtr->cacheAuthenticationInfo(requestInfo.fMachine,
                                               requestInfo.fHandle,
                                               authenticator,
                                               userIdentifier,
                                               authenticationData);

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFHandleService::handleUnAuthenticate(
    const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine and that
    // the requesting machine/user has at least trust level 5

    IVALIDATE_LOCAL_TRUST(5, "UNAUTHENTICATE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fUnAuthParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    // Un-authenticate the handle

    return gHandleManagerPtr->unAuthenticate(requestInfo.fHandle);
}


STAFServiceResult STAFHandleService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    // Note:  Changed from trust level 2 to 1 in STAF V3.4.1 to be
    //        consistent with the trust level for a HANDLE QUERY request
    //        which was changed from  trust level from 2 to 1.

    IVALIDATE_TRUST(1, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("NOTIFICATIONS") != 0)
    {
        unsigned int longFormat = parsedResult->optionTimes("LONG");
                
        // Create a marshalled list of maps containing notification information

        if (!longFormat)
            mc->setMapClassDefinition(fNotificationClass->reference());
        else
            mc->setMapClassDefinition(fNotificationLongClass->reference());

        STAFObjectPtr outputList = STAFObject::createList();

        if (parsedResult->optionTimes("POLLING") == 0)
        {
            // List the entries in the notification data list

            STAFHandleManager::NotificationList notificationList =
                gHandleManagerPtr->getNotificationListCopy();
            STAFHandleManager::NotificationList::iterator iter;

            for (iter = notificationList.begin();
                 iter != notificationList.end(); iter++)
            {
                STAFHandleManager::NotificationData notifiee(*iter);

                STAFObjectPtr notifyMap;

                if (!longFormat)
                {
                    notifyMap = fNotificationClass->createInstance();
                }
                else
                {
                    notifyMap = fNotificationLongClass->createInstance();
                    notifyMap->put("endpoint", notifiee.endpoint);
                    notifyMap->put("key", notifiee.key);
                    notifyMap->put("instanceUUID", notifiee.uuid);
                }

                notifyMap->put("handle", STAFString(notifiee.handle));
                notifyMap->put("machine", STAFString(notifiee.machine));
                notifyMap->put("notifyService",
                               STAFString(notifiee.notifyService));

                outputList->append(notifyMap);
            }
        }
        else
        {
            // List the entries in the polling data list

            STAFHandleManager::PollingDataList pollingList =
                gHandleManagerPtr->getPollingDataListCopy();
            STAFHandleManager::PollingDataList::iterator iter;

            for (iter = pollingList.begin();
                 iter != pollingList.end(); iter++)
            {
                STAFHandleManager::PollingData notifiee(*iter);

                STAFObjectPtr notifyMap;

                if (!longFormat)
                {
                    notifyMap = fNotificationClass->createInstance();
                }
                else
                {
                    notifyMap = fNotificationLongClass->createInstance();
                    notifyMap->put("endpoint", notifiee.endpoint);
                    notifyMap->put("key", notifiee.key);
                    notifyMap->put("instanceUUID", notifiee.uuid);
                }

                notifyMap->put("handle", STAFString(notifiee.handle));
                notifyMap->put("machine", STAFString(notifiee.machine));
                notifyMap->put("notifyService",
                               STAFString(notifiee.notifyService));

                outputList->append(notifyMap);
            }
        }
        
        mc->setRootObject(outputList);
    }
    else
    {
        // LIST HANDLES

        int getPending = 0;
        int getRegistered = 0;
        int getInProcess = 0;
        int getStatic = 0;

        if (parsedResult->optionTimes("SUMMARY") == 1)
        {
            // LIST HANDLES SUMMARY (provides summary info about handles)

            mc->setMapClassDefinition(fHandleSummaryClass->reference());
            
            STAFObjectPtr resultMap = fHandleSummaryClass->createInstance();

            resultMap->put("activeHandles",
                           gHandleManagerPtr->getHandleListSize());
            resultMap->put("totalHandles",
                           gHandleManagerPtr->getTotalHandles());
            resultMap->put("resetCount", gHandleManagerPtr->getResetCount());

            STAFString range = STAFString(
                gHandleManagerPtr->getMinHandleNumber()) + " - " +
                STAFString(gHandleManagerPtr->getMaxHandleNumber());

            resultMap->put("handleNumberRange", range);
            resultMap->put("maxActiveHandles",
                           gHandleManagerPtr->getMaxActiveHandles());

            mc->setRootObject(resultMap);

            return STAFServiceResult(kSTAFOk, mc->marshall());
        }

        if ((parsedResult->optionTimes("PENDING") == 0) &&
            (parsedResult->optionTimes("REGISTERED") == 0) &&
            (parsedResult->optionTimes("INPROCESS") == 0) &&
            (parsedResult->optionTimes("STATIC") == 0))
        {
            getPending = 0;
            getRegistered = 1;
            getInProcess = 1;
            getStatic = 1;
        }
        else
        {
            if (parsedResult->optionTimes("PENDING") != 0)
                getPending = 1;
            if (parsedResult->optionTimes("REGISTERED") != 0)
                getRegistered = 1;
            if (parsedResult->optionTimes("INPROCESS") != 0)
                getInProcess = 1;
            if (parsedResult->optionTimes("STATIC") != 0)
                getStatic = 1;
        }

        unsigned int getAll = parsedResult->optionTimes("NAME") == 0;
        unsigned int longFormat = parsedResult->optionTimes("LONG");

        // Resolve the NAME option value

        DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
        STAFString errorBuffer;
        STAFString name;
        STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("NAME", name);

        if (rc) return STAFServiceResult(rc, errorBuffer);
        
        // Create the marshalled list of maps containing handle information

        if (!longFormat)
            mc->setMapClassDefinition(fHandleInfoClass->reference());
        else
            mc->setMapClassDefinition(fHandleInfoLongClass->reference());

        STAFObjectPtr outputList = STAFObject::createList();
        
        // Get a copy of the handle list

        STAFHandleManager::HandleList handleList =
                                      gHandleManagerPtr->getHandleListCopy();
        STAFHandleManager::HandleList::iterator iter;

        for (iter = handleList.begin(); iter != handleList.end(); iter++)
        {
            STAFHandleManager::HandleData handleData = iter->second;

            if (((getAll != 0) || (handleData.name == name))
                && (((getPending == 1) && (handleData.state == STAFHandleManager::kPending))
                || ((getRegistered == 1) && ((handleData.state == STAFHandleManager::kRegistered)
                                || (handleData.state == STAFHandleManager::kPendingRegistered)))
                || ((getInProcess == 1) && (handleData.state == STAFHandleManager::kInProcess))
                || ((getStatic == 1) && (handleData.state == STAFHandleManager::kStatic))))
            {
                STAFObjectPtr handleMap;

                if (!longFormat)
                {
                    handleMap = fHandleInfoClass->createInstance();
                }
                else
                {
                    handleMap = fHandleInfoLongClass->createInstance();
                    handleMap->put("pid", STAFString(handleData.pid));
                }

                handleMap->put("handle", STAFString(handleData.handle));

                if (handleData.state == STAFHandleManager::kPending)
                {
                    if (handleData.name.length() != 0)
                        handleMap->put("name", STAFString(handleData.name));

                    handleMap->put("state", STAFString("Pending"));
                }
                else if ((handleData.state == STAFHandleManager::kRegistered) ||
                         (handleData.state == STAFHandleManager::kPendingRegistered))
                {
                    handleMap->put("name", STAFString(handleData.name));
                    handleMap->put("state", STAFString("Registered"));
                }
                else if (handleData.state == STAFHandleManager::kInProcess)
                {
                    handleMap->put("name", STAFString(handleData.name));
                    handleMap->put("state", STAFString("InProcess"));
                }
                else if (handleData.state == STAFHandleManager::kStatic)
                {
                    handleMap->put("name", STAFString(handleData.name));
                    handleMap->put("state", STAFString("Static"));
                }
                else
                {
                    if (handleData.name.length() != 0)
                        handleMap->put("name", STAFString(handleData.name));
                    handleMap->put("state", STAFString("<Unknown>"));
                }

                handleMap->put("lastUsedTimestamp",
                               handleData.lastUsedTimestamp.asString());

                outputList->append(handleMap);
            }
        }
        
        mc->setRootObject(outputList);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFHandleService::handleNotify(
    const STAFServiceRequest &requestInfo)
{
    STAFCommandParseResultPtr parsedResult = fNotifyParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    // Determine if the REGISTER or UNREGISTER option was specified

    unsigned int registerOption = parsedResult->optionTimes("REGISTER");

    // Verify that this request came from the local machine
    // and that the requesting machine/user has at least trust level 1

    if (registerOption)
    {
        IVALIDATE_LOCAL_TRUST(1, "STAF_NOTIFY REGISTER");
    }
    else
    {
        IVALIDATE_LOCAL_TRUST(1, "STAF_NOTIFY UNREGISTER");
    }
    
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFHandle_t handle;
    STAFRC_t rc = RESOLVE_UINT_OPTION("ONENDOFHANDLE", handle);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Don't need to resolve these variables as STAF_NOTIFY request is not
    // documented and STAF doesn't need to pass any variables in these options

    // Note that the MACHINE value specified should really be an endpoint
    // if it needs to communicate via a particular interface and/or port.

    STAFString machine = parsedResult->optionValue("MACHINE");
    STAFString service = parsedResult->optionValue("SERVICE");
    STAFString key     = parsedResult->optionValue("KEY");
    
    STAFString uuid;
    
    // UUID is optional.  If not specified, use MISC WHOAREYOU.
    
    if (parsedResult->optionTimes("UUID") == 0)
    {
        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            machine, "MISC", "WHOAREYOU");

        if (result->rc == kSTAFOk)
        {
            uuid = result->resultObj->get("instanceUUID")->asString();
        }
        else
        {
            uuid = "N/A";
        }
    }
    else
    {
        uuid = parsedResult->optionValue("UUID");
    }
    
    if (registerOption)
    {
        // STAF_NOTIFY REGISTER request

        return gHandleManagerPtr->addNotification(
            handle, machine, machine, uuid, service, key);
    }
    else
    {
        // STAF_NOTIFY UNREGISTER request

        return gHandleManagerPtr->deleteNotification(
            handle, machine, machine, uuid, service, key);
    }
}


STAFServiceResult STAFHandleService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
