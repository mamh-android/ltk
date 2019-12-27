/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFError.h"
#include "STAFServiceService.h"
#include "STAFCommandParser.h"
#include "STAFServiceManager.h"
#include "STAFRequestManager.h"
#include "STAFExternalService.h"

typedef STAFRefPtr<STAFCommandParser> STAFCommandParserPtr;

STAFCommandParserPtr fListParser;
STAFCommandParserPtr fFreeParser;
STAFCommandParserPtr fQueryParser;
STAFCommandParserPtr fAddParser;
STAFCommandParserPtr fReplaceParser;
STAFCommandParserPtr fRenameParser;
STAFCommandParserPtr fRemoveParser;
STAFCommandParserPtr fHelpParser;

static STAFString sHelpMsg;

static const STAFString sList("list");
static const STAFString sQuery("query");
static const STAFString sFree("free");
static const STAFString sAdd("add");
static const STAFString sReplace("replace");
static const STAFString sRename("rename");
static const STAFString sRemove("remove");
static const STAFString sForce("force");
static const STAFString sHelp("help");
static const STAFString sServices("services");
static const STAFString sServiceLoaders("serviceloaders");
static const STAFString sAuthenticators("authenticators");
static const STAFString sRequests("requests");
static const STAFString sPending("Pending");
static const STAFString sComplete("Complete");
static const STAFString sStatus("status");
static const STAFString sResult("result");
static const STAFString sRequest("request");

static const STAFString sServiceInfoClassName =
    "STAF/Service/Service/ServiceInfo";
static const STAFString sRequestInfoClassName =
    "STAF/Service/Service/RequestInfo";
static const STAFString sPendingRequestClassName =
    "STAF/Service/Service/RequestInfo";
static const STAFString sCompleteRequestClassName =
    "STAF/Service/Service/RequestInfo";
static const STAFString sRequestInfoLongClassName =
    "STAF/Service/Service/RequestInfo";
static const STAFString sRequestSummaryClassName =
    "STAF/Service/Service/RequestSummary";
static const STAFString sQueryRequestClassName =
    "STAF/Service/Service/QueryRequest";
static const STAFString sQueryServiceClassName =
    "STAF/Service/Service/QueryService";
static const STAFString sFreeRequestInfoClassName =
    "STAF/Service/Service/FreeRequestInfo";

STAFMapClassDefinitionPtr fServiceInfoClass;
STAFMapClassDefinitionPtr fRequestInfoClass;
STAFMapClassDefinitionPtr fPendingRequestClass;
STAFMapClassDefinitionPtr fCompleteRequestClass;
STAFMapClassDefinitionPtr fRequestInfoLongClass;
STAFMapClassDefinitionPtr fRequestSummaryClass;
STAFMapClassDefinitionPtr fQueryRequestClass;
STAFMapClassDefinitionPtr fQueryServiceClass;
STAFMapClassDefinitionPtr fFreeRequestInfoClass;

STAFServiceService::STAFServiceService() : STAFService("SERVICE")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** SERVICE Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST    [ SERVICES | SERVICELOADERS | AUTHENTICATORS |" +
        *gLineSeparatorPtr +
        "          REQUESTS <[PENDING] [COMPLETE] [LONG]> | [SUMMARY] ]" +
        *gLineSeparatorPtr +
        "QUERY   SERVICE <Service Name> | SERVICELOADER <ServiceLoader Name> |" +
        *gLineSeparatorPtr +
        "        AUTHENTICATOR <Authenticator Name> | REQUEST <Request Number>" +
        *gLineSeparatorPtr +
        "ADD     SERVICE <Service Name> LIBRARY <Library Name>" +
        *gLineSeparatorPtr +
        "        [EXECUTE <Executable>] [OPTION <Name[=Value]>]..." +
        *gLineSeparatorPtr +
        "        [PARMS <Parameters>]" +
        *gLineSeparatorPtr +
        // XXX: Commented out REPLACE and RENAME options
        //"REPLACE SERVICE <Service Name> LIBRARY <Library Name>" +
        //*gLineSeparatorPtr +
        //"        [EXECUTE <Executable>] [OPTION <Name[=Value]>]..." +
        //*gLineSeparatorPtr +
        //"        [PARMS <Parameters>] [RENAME <Service Name>]" +
        //*gLineSeparatorPtr +
        //"RENAME  SERVICE <Service Name> TO <New Service Name>" +
        //*gLineSeparatorPtr +
        "REMOVE  SERVICE <Service Name>" +
        *gLineSeparatorPtr +
        "FREE    REQUEST <Request Number> [FORCE]" +
        *gLineSeparatorPtr +
        "HELP";
    
    // Create the command request parsers

    // LIST Options

    fListParser = STAFCommandParserPtr(new STAFCommandParser,
                                       STAFCommandParserPtr::INIT);
    fListParser->addOption("LIST", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("SERVICES", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("SERVICELOADERS", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("AUTHENTICATORS", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("REQUESTS", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOptionGroup("SERVICES SERVICELOADERS AUTHENTICATORS REQUESTS", 0, 1);
    fListParser->addOption("PENDING", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("COMPLETE", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("LONG", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOption("SUMMARY", 1,
                           STAFCommandParser::kValueNotAllowed);
    fListParser->addOptionNeed("PENDING COMPLETE LONG SUMMARY", "REQUESTS");
    fListParser->addOptionGroup("SUMMARY LONG", 0, 1);
    fListParser->addOptionGroup("SUMMARY PENDING", 0, 1);
    fListParser->addOptionGroup("SUMMARY COMPLETE", 0, 1);

    // FREE Options
    fFreeParser = STAFCommandParserPtr(new STAFCommandParser,
                                       STAFCommandParserPtr::INIT);
    fFreeParser->addOption("FREE", 1,
                           STAFCommandParser::kValueNotAllowed);
    fFreeParser->addOption("REQUEST", 1,
                           STAFCommandParser::kValueRequired);
    fFreeParser->addOption("FORCE", 1,
                           STAFCommandParser::kValueNotAllowed);
    fFreeParser->addOptionNeed("FREE", "REQUEST");

    // QUERY Options
    fQueryParser = STAFCommandParserPtr(new STAFCommandParser,
                                        STAFCommandParserPtr::INIT);
    fQueryParser->addOption("QUERY", 1,
                           STAFCommandParser::kValueNotAllowed);
    fQueryParser->addOption("REQUEST", 1,
                           STAFCommandParser::kValueRequired);
    fQueryParser->addOption("STATUS", 1,
                           STAFCommandParser::kValueNotAllowed);
    fQueryParser->addOption("RESULT", 1,
                           STAFCommandParser::kValueNotAllowed);

    fQueryParser->addOption("SERVICE", 1,
                           STAFCommandParser::kValueRequired);
    fQueryParser->addOption("AUTHENTICATOR", 1,
                           STAFCommandParser::kValueRequired);
    fQueryParser->addOption("SERVICELOADER", 1,
                           STAFCommandParser::kValueRequired);

    fQueryParser->addOptionNeed("QUERY", "SERVICE SERVICELOADER AUTHENTICATOR REQUEST");
    fQueryParser->addOptionGroup("SERVICE SERVICELOADER AUTHENTICATOR REQUEST", 0, 1);
    fQueryParser->addOptionNeed("STATUS RESULT", "REQUEST");
    fQueryParser->addOptionGroup("STATUS RESULT", 0, 1);

    // ADD Options
    fAddParser = STAFCommandParserPtr(new STAFCommandParser,
                                      STAFCommandParserPtr::INIT);
    fAddParser->addOption("ADD", 1, STAFCommandParser::kValueNotAllowed);
    fAddParser->addOption("SERVICE", 1, STAFCommandParser::kValueRequired);
    fAddParser->addOption("LIBRARY", 1, STAFCommandParser::kValueRequired);
    fAddParser->addOption("EXECUTE", 1, STAFCommandParser::kValueRequired);
    fAddParser->addOption("OPTION", 0, STAFCommandParser::kValueRequired);
    fAddParser->addOption("PARMS", 1, STAFCommandParser::kValueRequired);
    fAddParser->addOptionNeed("ADD", "SERVICE");
    fAddParser->addOptionNeed("ADD", "LIBRARY");

    // REPLACE Options
    fReplaceParser = STAFCommandParserPtr(new STAFCommandParser,
                                          STAFCommandParserPtr::INIT);
    fReplaceParser->addOption("REPLACE", 1,
                              STAFCommandParser::kValueNotAllowed);
    fReplaceParser->addOption("SERVICE", 1, STAFCommandParser::kValueRequired);
    fReplaceParser->addOption("LIBRARY", 1, STAFCommandParser::kValueRequired);
    fReplaceParser->addOption("EXECUTE", 1, STAFCommandParser::kValueRequired);
    fReplaceParser->addOption("OPTION", 0, STAFCommandParser::kValueRequired);
    fReplaceParser->addOption("PARMS", 1, STAFCommandParser::kValueRequired);
    fReplaceParser->addOption("RENAME", 1, STAFCommandParser::kValueRequired);
    fReplaceParser->addOptionNeed("REPLACE", "SERVICE");
    fReplaceParser->addOptionNeed("REPLACE", "LIBRARY");

    // RENAME Options
    fRenameParser = STAFCommandParserPtr(new STAFCommandParser,
                                      STAFCommandParserPtr::INIT);
    fRenameParser->addOption("RENAME", 1, STAFCommandParser::kValueNotAllowed);
    fRenameParser->addOption("SERVICE", 1, STAFCommandParser::kValueRequired);
    fRenameParser->addOption("TO", 1, STAFCommandParser::kValueRequired);

    // REMOVE Options
    fRemoveParser = STAFCommandParserPtr(new STAFCommandParser,
                                         STAFCommandParserPtr::INIT);
    fRemoveParser->addOption("REMOVE", 1, STAFCommandParser::kValueNotAllowed);
    fRemoveParser->addOption("SERVICE", 1, STAFCommandParser::kValueRequired);
    fRemoveParser->addOptionNeed("REMOVE", "SERVICE");

    // HELP Options
    fHelpParser = STAFCommandParserPtr(new STAFCommandParser,
                                       STAFCommandParserPtr::INIT);
    fHelpParser->addOption("HELP", 1, STAFCommandParser::kValueNotAllowed);

    // Construct map class for list services/authenticators/serviceloaders

    fServiceInfoClass = STAFMapClassDefinition::create(sServiceInfoClassName);
    fServiceInfoClass->addKey("name", "Name");
    fServiceInfoClass->addKey("library", "Library");
    fServiceInfoClass->addKey("executable", "Executable");

    // Construct map class for list requests

    fRequestInfoClass = STAFMapClassDefinition::create(sRequestInfoClassName);
    fRequestInfoClass->addKey("requestNumber", "Request#");
    fRequestInfoClass->setKeyProperty("requestNumber",
                                      "display-short-name", "Req#");
    fRequestInfoClass->addKey("startTimestamp", "Start Date-Time");
    fRequestInfoClass->setKeyProperty("startTimestamp",
                                      "display-short-name", "Date-Time");
    fRequestInfoClass->addKey("service", "Service");
    fRequestInfoClass->addKey("request", "Request");

    // Construct map class for listing pending requests in detailed form (LONG)

    fPendingRequestClass = STAFMapClassDefinition::create(
        sPendingRequestClassName);
    fPendingRequestClass->addKey("requestNumber", "Request#");
    fPendingRequestClass->setKeyProperty("requestNumber",
                                          "display-short-name", "Req#");
    fPendingRequestClass->addKey("sourceMachine", "Source Machine");
    fPendingRequestClass->setKeyProperty("sourceMachine",
                                         "display-short-name", "Source");
    fPendingRequestClass->addKey("sourceHandleName", "Handle Name");
    fPendingRequestClass->setKeyProperty("sourceHandleName",
                                          "display-short-name", "HName");
    fPendingRequestClass->addKey("sourceHandle", "Handle#");
    fPendingRequestClass->setKeyProperty("sourceHandle",
                                          "display-short-name", "H#");
    fPendingRequestClass->addKey("startTimestamp", "Start Date-Time");
    fPendingRequestClass->setKeyProperty("startTimestamp",
                                          "display-short-name", "Date-Time");
    fPendingRequestClass->addKey("targetMachine", "Target Machine");
    fPendingRequestClass->setKeyProperty("targetMachine",
                                          "display-short-name", "Target");
    fPendingRequestClass->addKey("service", "Service");
    fPendingRequestClass->addKey("request", "Request");

    // Construct map class for listing complete requests in detailed form (LONG)

    fCompleteRequestClass = STAFMapClassDefinition::create(
        sCompleteRequestClassName);
    fCompleteRequestClass->addKey("requestNumber", "Request#");
    fCompleteRequestClass->setKeyProperty("requestNumber",
                                          "display-short-name", "Req#");
    fCompleteRequestClass->addKey("sourceMachine", "Source Machine");
    fCompleteRequestClass->setKeyProperty("sourceMachine",
                                          "display-short-name", "Source");
    fCompleteRequestClass->addKey("sourceHandleName", "Handle Name");
    fCompleteRequestClass->setKeyProperty("sourceHandleName",
                                          "display-short-name", "HName");
    fCompleteRequestClass->addKey("sourceHandle", "Handle#");
    fCompleteRequestClass->setKeyProperty("sourceHandle",
                                          "display-short-name", "H#");
    fCompleteRequestClass->addKey("startTimestamp", "Start Date-Time");
    fCompleteRequestClass->setKeyProperty("startTimestamp",
                                          "display-short-name", "Date-Time");
    fCompleteRequestClass->addKey("targetMachine", "Target Machine");
    fCompleteRequestClass->setKeyProperty("targetMachine",
                                          "display-short-name", "Target");
    fCompleteRequestClass->addKey("service", "Service");
    fCompleteRequestClass->addKey("request", "Request");
    fCompleteRequestClass->addKey("rc", "Return Code");
    fCompleteRequestClass->setKeyProperty("rc", "display-short-name", "RC");
    fCompleteRequestClass->addKey("result", "Result");

    // Construct map class for listing pending and complete requests in
    // detailed form (LONG)

    fRequestInfoLongClass = STAFMapClassDefinition::create(
        sRequestInfoLongClassName);
    fRequestInfoLongClass->addKey("requestNumber", "Request#");
    fRequestInfoLongClass->setKeyProperty("requestNumber",
                                          "display-short-name", "Req#");
    fRequestInfoLongClass->addKey("state", "Status");
    fRequestInfoLongClass->addKey("sourceMachine", "Source Machine");
    fRequestInfoLongClass->setKeyProperty("sourceMachine",
                                          "display-short-name", "Source");
    fRequestInfoLongClass->addKey("sourceHandleName", "Handle Name");
    fRequestInfoLongClass->setKeyProperty("sourceHandleName",
                                          "display-short-name", "HName");
    fRequestInfoLongClass->addKey("sourceHandle", "Handle#");
    fRequestInfoLongClass->setKeyProperty("sourceHandle",
                                          "display-short-name", "H#");
    fRequestInfoLongClass->addKey("startTimestamp", "Start Date-Time");
    fRequestInfoLongClass->setKeyProperty("startTimestamp",
                                          "display-short-name", "Date-Time");
    fRequestInfoLongClass->addKey("targetMachine", "Target Machine");
    fRequestInfoLongClass->setKeyProperty("targetMachine",
                                          "display-short-name", "Target");
    fRequestInfoLongClass->addKey("service", "Service");
    fRequestInfoLongClass->addKey("request", "Request");
    fRequestInfoLongClass->addKey("rc", "Return Code");
    fRequestInfoLongClass->setKeyProperty("rc", "display-short-name", "RC");
    fRequestInfoLongClass->addKey("result", "Result");

    // Construct map class for listing a summary of requests submitted

    fRequestSummaryClass = STAFMapClassDefinition::create(
        sRequestSummaryClassName);
    fRequestSummaryClass->addKey("activeRequests", "Active Requests");
    fRequestSummaryClass->addKey("totalRequests", "Total Requests");
    fRequestSummaryClass->addKey("resetCount", "Reset Count");
    fRequestSummaryClass->addKey("requestNumberRange", "Request Number Range");
    fRequestSummaryClass->addKey("maxActiveRequests",
                                 "Maximum Active Requests");

    // Construct map class for query request

    fQueryRequestClass = STAFMapClassDefinition::create(sQueryRequestClassName);
    fQueryRequestClass->addKey("requestNumber", "Request Number");
    fQueryRequestClass->addKey("state", "Request Status");
    fQueryRequestClass->addKey("sourceMachine", "Source Machine");
    fQueryRequestClass->addKey("sourceHandleName", "Source Handle Name");
    fQueryRequestClass->addKey("sourceHandle", "Source Handle#");
    fQueryRequestClass->addKey("startTimestamp", "Start Date-Time");
    fQueryRequestClass->addKey("targetMachine", "Target Machine");
    fQueryRequestClass->addKey("service", "Service");
    fQueryRequestClass->addKey("request", "Request");
    fQueryRequestClass->addKey("rc", "Return Code");
    fQueryRequestClass->addKey("result", "Result");
    
    // Construct map class for QUERY SERVICE request

    fQueryServiceClass = STAFMapClassDefinition::create(sQueryServiceClassName);
    fQueryServiceClass->addKey("name", "Name");
    fQueryServiceClass->addKey("library", "Library");
    fQueryServiceClass->addKey("executable", "Executable");
    fQueryServiceClass->addKey("options", "Options");
    fQueryServiceClass->addKey("parameters", "Parameters");

    // Construct map class for free request

    fFreeRequestInfoClass = STAFMapClassDefinition::create(
        sFreeRequestInfoClassName);
    fFreeRequestInfoClass->addKey("rc", "Return Code");
    fFreeRequestInfoClass->addKey("result", "Result");
}


STAFServiceService::~STAFServiceService()
{
    /* Do Nothing */
}


STAFString STAFServiceService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFServiceService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).toLowerCase();

    if      (action == sList)    return handleList(requestInfo);
    else if (action == sHelp)    return handleHelp(requestInfo);
    else if (action == sQuery)   return handleQuery(requestInfo);
    else if (action == sFree)    return handleFree(requestInfo);
    else if (action == sAdd)     return handleAdd(requestInfo);
    // XXX: Commented out Replace and Rename options
    // else if (action == sReplace) return handleReplace(requestInfo);
    // else if (action == sRename)  return handleRename(requestInfo);
    else if (action == sRemove)  return handleRemove(requestInfo);
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

STAFServiceResult STAFServiceService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFRC_t rc = kSTAFOk;
    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    STAFObjectPtr outputList = STAFObject::createList();

    if (parsedResult->optionTimes(sServiceLoaders) > 0)
    {
        // LIST SERVICELOADERSS

        STAFServiceManager::ServiceList serviceLoaderList =
            gServiceManagerPtr->getSLSListCopy();

        // Create a marshalled list of maps for service loaders
        
        mc->setMapClassDefinition(fServiceInfoClass->reference());
        
        STAFServiceManager::ServiceList::iterator iter;

        for(iter = serviceLoaderList.begin();
            iter != serviceLoaderList.end(); iter++)
        {
            STAFObjectPtr serviceLoaderMap = 
                fServiceInfoClass->createInstance();
            serviceLoaderMap->put("name", (*iter)->name());
            serviceLoaderMap->put("library", (*iter)->getLibName());

            STAFString exec = (*iter)->getExecutable();

            if (exec.length() != 0)
                serviceLoaderMap->put("executable", exec);
            
            outputList->append(serviceLoaderMap);
        }
    }
    else if (parsedResult->optionTimes(sAuthenticators) > 0)
    {
        // LIST AUTHENTICATORS

        STAFServiceManager::OrderedServiceList authenticatorMap =
            gServiceManagerPtr->getAuthenticatorMapCopy();

        // Create a marshalled list of maps for authenticators
        
        mc->setMapClassDefinition(fServiceInfoClass->reference());

        STAFServiceManager::OrderedServiceList::iterator iter;

        for(iter = authenticatorMap.begin();
            iter != authenticatorMap.end(); iter++)
        {
            STAFObjectPtr authServiceMap = fServiceInfoClass->createInstance();
            authServiceMap->put("name", iter->first);
            authServiceMap->put("library", iter->second->getLibName());

            STAFString exec = iter->second->getExecutable();
            
            if (exec.length() != 0)
                authServiceMap->put("executable", exec);

            outputList->append(authServiceMap);
        }
    }
    else if (parsedResult->optionTimes(sRequests) > 0)
    {
        // LIST REQUESTS

        if (parsedResult->optionTimes("SUMMARY") > 0)
        {
            // LIST REQUESTS SUMMARY (provides summary information about
            // submitted service requests)

            mc->setMapClassDefinition(fRequestSummaryClass->reference());
            
            STAFObjectPtr resultMap = fRequestSummaryClass->createInstance();
            
            resultMap->put("activeRequests",
                           gRequestManagerPtr->getRequestMapSize());
            resultMap->put("totalRequests",
                           gRequestManagerPtr->getTotalRequests());
            resultMap->put("resetCount",
                           gRequestManagerPtr->getResetCount());

            STAFString range = STAFString(
                gRequestManagerPtr->getMinRequestNumber()) + " - " +
                STAFString(gRequestManagerPtr->getMaxRequestNumber());

            resultMap->put("requestNumberRange", range);
            resultMap->put("maxActiveRequests",
                           gRequestManagerPtr->getMaxActiveRequests());

            mc->setRootObject(resultMap);

            return STAFServiceResult(kSTAFOk, mc->marshall());
        }

        //  All other LIST REQUESTS which lists the pending/complete requests

        bool pending = true;  // PENDING is the default
        bool complete = false;
        bool details = false;

        if (parsedResult->optionTimes(sComplete) > 0)
        {
            complete = true;

            if (parsedResult->optionTimes(sPending) == 0)
                pending = false;
        }

        if (parsedResult->optionTimes("LONG") > 0)
        {
            details = true;
        }

        // Create a marshalled list of maps for requests
        
        if (!details)
            mc->setMapClassDefinition(fRequestInfoClass->reference());
        else if (pending && !complete)
            mc->setMapClassDefinition(fPendingRequestClass->reference());
        else if (complete && !pending)
            mc->setMapClassDefinition(fCompleteRequestClass->reference());
        else // pending and complete
            mc->setMapClassDefinition(fRequestInfoLongClass->reference());

        STAFRequestManager::RequestMap requestMap = gRequestManagerPtr->
            getRequestMapCopy();

        STAFRequestManager::RequestMap::iterator theIterator;

        STAFServiceRequestPtr serviceRequest;

        for (theIterator = requestMap.begin(); theIterator != requestMap.end();
             ++theIterator)
        {
            serviceRequest = theIterator->second;
            STAFServiceRequestState_t requestState = serviceRequest->
                fProcessingState;
            
            if ((pending  && (requestState == kSTAFServiceRequestPending)) ||
                (complete && (requestState == kSTAFServiceRequestComplete)))
            {
                STAFObjectPtr requestMap;

                if (!details)
                    requestMap = fRequestInfoClass->createInstance();
                else if (pending && !complete)
                    requestMap = fPendingRequestClass->createInstance();
                else if (complete && !pending)
                    requestMap = fCompleteRequestClass->createInstance();
                else // pending && !complete
                    requestMap = fRequestInfoLongClass->createInstance();
                
                requestMap->put("requestNumber",
                                STAFString(theIterator->first));
                requestMap->put("startTimestamp",
                                serviceRequest->fStartStamp.asString());
                requestMap->put("service", serviceRequest->fTargetService);
                requestMap->put("request",
                                STAFHandle::maskPrivateData(
                                    serviceRequest->fRequest));

                if (details)
                {
                    if (pending && complete)
                    {
                        if (requestState == kSTAFServiceRequestPending)
                            requestMap->put("state", sPending);
                        else if (requestState == kSTAFServiceRequestComplete)
                            requestMap->put("state", sComplete);
                    }

                    requestMap->put("targetMachine",
                                    serviceRequest->fTargetMachine);
                    requestMap->put("sourceMachine",
                                    serviceRequest->fEndpoint);
                    requestMap->put("sourceHandleName",
                                    serviceRequest->fHandleName);
                    requestMap->put("sourceHandle",
                                    STAFString(serviceRequest->fHandle));

                    if (complete &&
                        (requestState == kSTAFServiceRequestComplete))
                    {
                        requestMap->put(
                            "rc", STAFString(serviceRequest->fResult.fRC));
                        requestMap->put(
                            "result", serviceRequest->fResult.fResult);
                    }
                }

                outputList->append(requestMap);
            }
        }
    }
    else
    {
        // LIST SERVICES (services is the default)

        STAFServiceManager::OrderedServiceList serviceList =
            gServiceManagerPtr->getOrderedServiceListCopy();
        STAFServiceManager::OrderedServiceList::iterator iter;

        // Create a marshalled list of maps for services

        mc->setMapClassDefinition(fServiceInfoClass->reference());
        
        for (iter = serviceList.begin(); iter != serviceList.end(); iter++)
        {
            STAFObjectPtr serviceMap = fServiceInfoClass->createInstance();
            serviceMap->put("name", iter->first);
            serviceMap->put("library", iter->second->getLibName());

            STAFString exec = iter->second->getExecutable();
            
            if (exec.length() != 0)
                serviceMap->put("executable", exec);

            outputList->append(serviceMap);
        }
    }

    mc->setRootObject(outputList);

    return STAFServiceResult(rc, mc->marshall());
}

STAFServiceResult STAFServiceService::handleQuery(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "QUERY");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fQueryParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
    
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    STAFRC_t rc = kSTAFOk;

    if (parsedResult->optionTimes(sRequest))
    {
        // Handle QUERY REQUEST request

        // Convert the REQUEST option string to an unsigned integer and return
        // an error if it is not in the range of minRequestNumber to
        // maxRequestNumber

        STAFRequestNumber_t reqNum;
        STAFString errorBuffer;

        STAFRC_t rc = convertStringToUInt(
            parsedResult->optionValue(sRequest), sRequest, reqNum,
            errorBuffer, gRequestManagerPtr->getMinRequestNumber(),
            gRequestManagerPtr->getMaxRequestNumber());

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Check if the undocumented STATUS or RESULT option is specified

        if (parsedResult->optionTimes(sStatus))
        {
            STAFServiceRequestState_t statusResult = 
                kSTAFServiceRequestStateUnknown;
            rc = gRequestManagerPtr->getStatus(reqNum, &statusResult);
            return STAFServiceResult(rc, statusResult);
        }
        else if (parsedResult->optionTimes(sResult))
        {
            STAFServiceResult requestResult;
            rc = gRequestManagerPtr->getResult(reqNum, &requestResult);

            if (rc == kSTAFOk) return requestResult;
            else return STAFServiceResult(rc);
        }

        // Otherwise, return all the info about the request

        STAFServiceRequestPtr serviceRequest;
        rc = gRequestManagerPtr->getRequestData(reqNum, serviceRequest);

        if (rc != kSTAFOk) return STAFServiceResult(
            rc, STAFString("Request number ") + reqNum + " not found");

        // Create a marshalled map containing details for the specified request

        mc->setMapClassDefinition(fQueryRequestClass->reference());
        STAFObjectPtr requestMap = fQueryRequestClass->createInstance();

        requestMap->put("requestNumber", STAFString(reqNum));

        STAFServiceRequestState_t requestState = 
            serviceRequest->fProcessingState;
 
        if (requestState == kSTAFServiceRequestPending)
            requestMap->put("state", sPending);
        else if (requestState == kSTAFServiceRequestComplete)
            requestMap->put("state", sComplete);

        requestMap->put("sourceMachine", serviceRequest->fEndpoint);
        requestMap->put("sourceHandleName", serviceRequest->fHandleName);
        requestMap->put("sourceHandle", STAFString(serviceRequest->fHandle));
        requestMap->put("startTimestamp",
                        serviceRequest->fStartStamp.asString());
        requestMap->put("targetMachine", serviceRequest->fTargetMachine);
        requestMap->put("service", serviceRequest->fTargetService);
        requestMap->put("request", serviceRequest->fRequest);

        if (requestState == kSTAFServiceRequestComplete)
        {
            requestMap->put(
                "rc", STAFString(serviceRequest->fResult.fRC));
            requestMap->put("result", serviceRequest->fResult.fResult);
        }

        mc->setRootObject(requestMap);
    }
    else if (parsedResult->optionTimes("SERVICE"))
    {
        // Handle QUERY SERVICE request

        STAFString errorBuffer;
        STAFString serviceName;

        // Resolve SERVICE value
        STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICE", serviceName);

        // Get information about the service

        STAFServicePtr service;

        rc = gServiceManagerPtr->get(serviceName, service, errorBuffer);
            
        if (rc != kSTAFOk)
        {
            return STAFServiceResult(kSTAFDoesNotExist, serviceName);
        }

        mc->setMapClassDefinition(fQueryServiceClass->reference());
        STAFObjectPtr outputMap = fQueryServiceClass->createInstance();
        outputMap->put("name", service->name());
        outputMap->put("library", service->getLibName());

        STAFString exec = service->getExecutable();
            
        if (exec.length() != 0)
            outputMap->put("executable", exec);

        STAFString parms = service->getParameters();
            
        if (parms.length() != 0)
            outputMap->put("parameters", parms);

        outputMap->put("options", service->getOptions());

        mc->setRootObject(outputMap);
    }
    else if (parsedResult->optionTimes("AUTHENTICATOR"))
    {
        // Handle QUERY AUTHENTICATOR request

        STAFString errorBuffer;
        STAFString authName;

        // Resolve AUTHENTICATOR value
        STAFRC_t rc = RESOLVE_STRING_OPTION("AUTHENTICATOR", authName);

        // Get information about the authenticator service
        
        STAFServicePtr authenticator;

        rc = gServiceManagerPtr->getAuthenticator(authName, authenticator);

        if (rc != kSTAFOk)
        {
            return STAFServiceResult(kSTAFDoesNotExist, authName);
        }

        mc->setMapClassDefinition(fQueryServiceClass->reference());
        STAFObjectPtr outputMap = fQueryServiceClass->createInstance();
        outputMap->put("name", authenticator->name());
        outputMap->put("library", authenticator->getLibName());

        STAFString exec = authenticator->getExecutable();
            
        if (exec.length() != 0)
            outputMap->put("executable", exec);

        STAFString parms = authenticator->getParameters();
            
        if (parms.length() != 0)
            outputMap->put("parameters", parms);

        outputMap->put("options", authenticator->getOptions());

        mc->setRootObject(outputMap);
    }
    else if (parsedResult->optionTimes("SERVICELOADER"))
    {
        // Handle QUERY SERVICELOADER request

        STAFString errorBuffer;
        STAFString serviceLoaderName;

        // Resolve AUTHENTICATOR value
        STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICELOADER",
                                            serviceLoaderName);

        // Get information about the service loader

        STAFServiceManager::ServiceList serviceLoaderList =
            gServiceManagerPtr->getSLSListCopy();
        
        STAFServiceManager::ServiceList::iterator iter;
        STAFServicePtr serviceLoader;
        bool found = false;

        for (iter = serviceLoaderList.begin();
             iter != serviceLoaderList.end(); iter++)
        {
            if ((*iter)->name() == serviceLoaderName.toUpperCase())
            {
                serviceLoader = *iter;
                found = true;
            }
        }

        if (!found)
        {
            return STAFServiceResult(kSTAFDoesNotExist, serviceLoaderName);
        }

        mc->setMapClassDefinition(fQueryServiceClass->reference());
        STAFObjectPtr outputMap = fQueryServiceClass->createInstance();
        outputMap->put("name", serviceLoader->name());
        outputMap->put("library", serviceLoader->getLibName());

        STAFString exec = serviceLoader->getExecutable();

        if (exec.length() != 0)
            outputMap->put("executable", exec);

        STAFString parms = serviceLoader->getParameters();
            
        if (parms.length() != 0)
            outputMap->put("parameters", parms);

        outputMap->put("options", serviceLoader->getOptions());
        
        mc->setRootObject(outputMap);
    }

    return STAFServiceResult(rc, mc->marshall());
}

STAFServiceResult STAFServiceService::handleFree(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "FREE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fFreeParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFServiceResult result;

    // Convert the REQUEST option string to an unsigned integer and return
    // an error if it is not in the range of minRequestNumber to
    // maxRequestNumber

    STAFRequestNumber_t requestNumber;
    STAFString errorBuffer;

    STAFRC_t rc = convertStringToUInt(
        parsedResult->optionValue(sRequest), sRequest, requestNumber,
        errorBuffer, gRequestManagerPtr->getMinRequestNumber(),
        gRequestManagerPtr->getMaxRequestNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Check if the request is in the request map

    STAFServiceRequestPtr serviceRequest;
    rc = gRequestManagerPtr->getRequestData(requestNumber, serviceRequest);
    
    if (rc != kSTAFOk) return STAFServiceResult(
        rc, STAFString("Request number ") + requestNumber + " not found");

    // Check for FORCE if this is not the originating process

    if ((requestInfo.fMachine != serviceRequest->fMachine) ||
        (requestInfo.fHandle != serviceRequest->fHandle))
    {
        if (parsedResult->optionTimes(sForce) == 0)
        {
            return STAFServiceResult(
                kSTAFAccessDenied, "You are not the originator of the "
                "request you are trying to FREE.  Use the FORCE option if "
                "you are sure that the correct request number is specified.");
        }
    }

    rc = gRequestManagerPtr->freeRequest(requestNumber, &result);

    if (rc != kSTAFOk)
        return STAFServiceResult(rc);

    // Create a marshalled map containing information for the freed request

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fFreeRequestInfoClass->reference());
    STAFObjectPtr freeRequestMap = fFreeRequestInfoClass->createInstance();

    freeRequestMap->put("rc", STAFString(result.fRC));
    freeRequestMap->put("result", result.fResult);

    mc->setRootObject(freeRequestMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}

STAFServiceResult STAFServiceService::handleAdd(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "ADD");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fAddParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFServiceResult result;
    STAFString errorBuffer;
    STAFString serviceName;
    STAFString libraryName;
    STAFString executable;
    STAFString parms;
    STAFString option;
    STAFExternalService::OptionList options;

    // Resolve SERVICE value
    STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICE", serviceName);

    if (!rc) rc = RESOLVE_STRING_OPTION("LIBRARY", libraryName);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("EXECUTE", executable);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("PARMS", parms);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Resolve OPTION values (if any are specified)
    for (int i = 1; i <= parsedResult->optionTimes("OPTION"); ++i)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("OPTION", i, option);
        if (rc) return STAFServiceResult(rc, errorBuffer);
        options.push_back(option);
    }

    // Verify that the Service name specified is a valid name

    rc = gServiceManagerPtr->verifyServiceName(serviceName);

    if (rc != kSTAFOk)
    {
        return STAFServiceResult(
            rc, STAFString("Service name ") + serviceName +
            " is not valid.  " INVALID_SERVICE_NAME_ERROR_MESSAGE());
    }

    // Verify that the Service name specified does not already exist.

    STAFServicePtr service;

    rc = gServiceManagerPtr->get(serviceName, service, errorBuffer, true);

    if (rc == kSTAFOk)
        return STAFServiceResult(kSTAFAlreadyExists, serviceName);

    // Determine if a service loader service is loading this service

    STAFString slsName("");

    if (requestInfo.fHandleName.find("STAF/Service/STAFServiceLoader") == 0)
    {
        slsName = requestInfo.fHandleName.subString(
                STAFString("STAF/Service/").length());
    }

    // Create the service

    try
    {
        service = STAFServicePtr(new STAFExternalService(
                  serviceName, libraryName, executable, options, parms,
                  kSTAFServiceTypeService, slsName), STAFServicePtr::INIT);
    }
    catch (STAFServiceCreateException &e)
    {
        return STAFServiceResult(kSTAFServiceConfigurationError,
                                 STAFString(e.getErrorCode()) + ":" +
                                 e.getText());
    }
    catch (STAFException &e)
    {
        return STAFServiceResult(kSTAFServiceConfigurationError,
                                 STAFString(e.getErrorCode()) + ":" +
                                 e.getText());
    }
    catch (...)
    {
        return STAFServiceResult(kSTAFServiceConfigurationError);
    }

    // Initialize the service
    result = service->initialize();
    if (result.fRC) return result;

    // Add the service
    rc = gServiceManagerPtr->add(service);

    if (rc != kSTAFOk)
    {
        result = service->terminate();
        return STAFServiceResult(rc, serviceName);
    }

    return STAFServiceResult(kSTAFOk);
}

STAFServiceResult STAFServiceService::handleReplace(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "REPLACE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fReplaceParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFServiceResult result;
    STAFString errorBuffer;
    STAFString serviceName;
    STAFString libraryName;
    STAFString executable;
    STAFString parms;
    STAFString option;
    STAFString newServiceName;
    STAFExternalService::OptionList options;

    // Resolve SERVICE value
    STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICE", serviceName);

    if (!rc) rc = RESOLVE_STRING_OPTION("LIBRARY", libraryName);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("EXECUTE", executable);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("PARMS", parms);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Resolve OPTION values (if any are specified)
    for (int i = 1; i <= parsedResult->optionTimes("OPTION"); ++i)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("OPTION", i, option);
        if (rc) return STAFServiceResult(rc, errorBuffer);
        options.push_back(option);
    }

    // Resolve RENAME value (if specified)

    STAFServicePtr newService;

    if (parsedResult->optionTimes("RENAME") != 0)
    {
        rc = RESOLVE_STRING_OPTION("RENAME", newServiceName);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        if (newServiceName.toUpperCase() == serviceName.toUpperCase())
            return STAFServiceResult(kSTAFAlreadyExists, newServiceName);
    }

    // Create the new service
    try
    {
        newService = STAFServicePtr(new STAFExternalService(
                     serviceName, libraryName, executable, options, parms,
                     kSTAFServiceTypeService),
                     STAFServicePtr::INIT);
    }
    catch (STAFServiceCreateException &e)
    {
        return STAFServiceResult(kSTAFServiceConfigurationError,
                                 STAFString(e.getErrorCode()) + ":" +
                                 e.getText());
    }
    catch (STAFException &e)
    {
        return STAFServiceResult(kSTAFServiceConfigurationError,
                                 STAFString(e.getErrorCode()) + ":" +
                                 e.getText());
    }
    catch (...)
    {
        return STAFServiceResult(kSTAFServiceConfigurationError);
    }

    // Initialize the new service
    result = newService->initialize();
    if (result.fRC) return result;

    // Replace with the new service
    rc = gServiceManagerPtr->replace(newService, newServiceName, errorBuffer);
    if (rc)
    {
        result = newService->terminate();
        return STAFServiceResult(rc);
    }

    return STAFServiceResult(kSTAFOk);
}

STAFServiceResult STAFServiceService::handleRename(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "RENAME");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fRenameParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFServiceResult result;
    STAFString errorBuffer;
    STAFString serviceName;
    STAFString newServiceName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICE", serviceName);

    if (!rc) rc = RESOLVE_STRING_OPTION("TO", newServiceName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (newServiceName.toUpperCase() == serviceName.toUpperCase())
        return STAFServiceResult(kSTAFAlreadyExists, newServiceName);

    // Rename with the new service name

    rc = gServiceManagerPtr->rename(serviceName, newServiceName, errorBuffer);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    return kSTAFOk;
}

STAFServiceResult STAFServiceService::handleRemove(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "REMOVE");
    
    // Parse the request
    
    STAFCommandParseResultPtr parsedResult = fRemoveParser->parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFServiceResult result;
    STAFString errorBuffer;
    STAFString serviceName;
    STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICE", serviceName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFServicePtr service;
    rc = gServiceManagerPtr->get(serviceName, service, errorBuffer);
    if (rc) return STAFServiceResult(rc, serviceName);

    // Remove the service
    rc = gServiceManagerPtr->remove(serviceName);
    if (rc) return STAFServiceResult(rc, serviceName);

    // Terminate the service
    result = service->terminate();
    if (result.fRC) return result;

    return kSTAFOk;
}

STAFServiceResult STAFServiceService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
