/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFUtil.h"
#include "STAFTrace.h"
#include "STAFServiceManager.h"
#include "STAFTraceService.h"
#include "STAFThreadManager.h"

static const int kColonColumn = 21;
static const STAFString kEnabledString = "Enabled";
static const STAFString kDisabledString = "Disabled";

static STAFString sHelpMsg;

STAFTraceService::STAFTracepointMap
        STAFTraceService::kSTAFTracepointMap =
        STAFTraceService::fillSTAFTracepointMap();
STAFTraceService::STAFTracepointNameMap
        STAFTraceService::kSTAFTracepointNameMap =
        STAFTraceService::fillSTAFTracepointNameMap();


STAFTraceService::STAFTraceService() : STAFService("Trace")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** TRACE Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "ENABLE ALL  [ TRACEPOINTS | SERVICES ]" +
        *gLineSeparatorPtr +
        "ENABLE TRACEPOINTS <Trace point list> | SERVICES <Service list>" +
        *gLineSeparatorPtr +
        "ENABLE TRACEPOINT <Trace point> [TRACEPOINT <Trace point>]..." +
        *gLineSeparatorPtr +
        "ENABLE SERVICE <Service> [SERVICE <Service>]..." +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DISABLE ALL  [ TRACEPOINTS | SERVICES ]" +
        *gLineSeparatorPtr +
        "DISABLE TRACEPOINTS <Trace point list> | SERVICES <Service list>" +
        *gLineSeparatorPtr +
        "DISABLE TRACEPOINT <Trace point> [TRACEPOINT <Trace point>]..." +
        *gLineSeparatorPtr +
        "DISABLE SERVICE <Service> [SERVICE <Service>]..." +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SET DESTINATION TO < [STDOUT | STDERR] [FILE <File name> [APPEND]] >" +
        *gLineSeparatorPtr +
        "SET DEFAULTSERVICESTATE < Enabled | Disabled >" +
        *gLineSeparatorPtr +
        "SET MAXSERVICERESULTSIZE <Number>[k|m]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST [SETTINGS]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "PURGE" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // enable|disable options
    fEnableDisableParser.addOption("ENABLE", 1,
        STAFCommandParser::kValueNotAllowed);
    fEnableDisableParser.addOption("DISABLE", 1,
        STAFCommandParser::kValueNotAllowed);
    fEnableDisableParser.addOption("ALL", 1,
        STAFCommandParser::kValueNotAllowed);
    fEnableDisableParser.addOption("TRACEPOINTS", 1,
        STAFCommandParser::kValueAllowed);
    fEnableDisableParser.addOption("TRACEPOINT", 0,
        STAFCommandParser::kValueRequired);
    fEnableDisableParser.addOption("SERVICES", 1,
        STAFCommandParser::kValueAllowed);
    fEnableDisableParser.addOption("SERVICE", 0,
        STAFCommandParser::kValueRequired);

    // enable|disable groups
    fEnableDisableParser.addOptionGroup("ENABLE DISABLE", 1, 1);

    // enable|disable needs
    fEnableDisableParser.addOptionNeed("ENABLE DISABLE",
        "ALL TRACEPOINT SERVICE TRACEPOINTS SERVICES");

    //SET options
    fSetParser.addOption("SET", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("DESTINATION", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("TO", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("STDOUT", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("STDERR", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("FILE", 1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("APPEND", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("DEFAULTSERVICESTATE", 1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("MAXSERVICERESULTSIZE", 1,
        STAFCommandParser::kValueRequired);


    //SET groups
    fSetParser.addOptionGroup("STDOUT STDERR", 0, 1);
    fSetParser.addOptionGroup(
        "DEFAULTSERVICESTATE DESTINATION MAXSERVICERESULTSIZE", 1, 1);

    //SET needs
    fSetParser.addOptionNeed(
        "SET", "TO DEFAULTSERVICESTATE MAXSERVICERESULTSIZE");
    fSetParser.addOptionNeed("DESTINATION", "SET");
    fSetParser.addOptionNeed("TO", "DESTINATION");
    fSetParser.addOptionNeed("TO", "STDOUT STDERR FILE");
    fSetParser.addOptionNeed("APPEND", "FILE");
    fSetParser.addOptionNeed("STDOUT STDERR FILE DEFAULTSERVICESTATE", "SET");


    //LIST options
    fListParser.addOption("LIST", 1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SETTINGS", 1,
        STAFCommandParser::kValueNotAllowed);

    //LIST groups
    fListParser.addOptionGroup("LIST", 1, 1);
    fListParser.addOptionGroup("SETTINGS", 0, 1);

    //LIST needs
    fListParser.addOptionNeed("SETTINGS", "LIST");

    // Construct map class for a trace info

    fTraceInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Trace/TraceInfo");
    fTraceInfoClass->addKey("tracingTo", "Tracing To");
    fTraceInfoClass->addKey("fileMode", "File Mode");
    fTraceInfoClass->addKey("defaultServiceState", "Default Service State");
    fTraceInfoClass->addKey("maxServiceResultSize",
                            "Maximum Service Result Size");
    fTraceInfoClass->addKey("tracePoints", "Trace Points");
    fTraceInfoClass->addKey("services", "Services");

    // Construct map class for a tracepoint states

    fTracepointClass = STAFMapClassDefinition::create(
        "STAF/Service/Trace/Tracepoint");
    fTracepointClass->addKey("INFO", STAFString("Info"));
    fTracepointClass->addKey("WARNING", STAFString("Warning"));
    fTracepointClass->addKey("ERROR", STAFString("Error"));
    fTracepointClass->addKey("SERVICEREQUEST", STAFString("ServiceRequest"));
    fTracepointClass->addKey("SERVICERESULT", STAFString("ServiceResult"));
    fTracepointClass->addKey("SERVICEERROR", STAFString("ServiceError"));
    fTracepointClass->addKey("SERVICEACCESSDENIED",
                             STAFString("ServiceAccessDenied"));
    //fTracepointClass->addKey("serviceManagement",
    //                         STAFString("ServiceManagement"));
    fTracepointClass->addKey("REMOTEREQUESTS", STAFString("RemoteRequests"));
    fTracepointClass->addKey("REGISTRATION", STAFString("Registration"));
    fTracepointClass->addKey("DEPRECATED", STAFString("Deprecated"));
    fTracepointClass->addKey("DEBUG", STAFString("Debug"));
    fTracepointClass->addKey("SERVICECOMPLETE", STAFString("ServiceComplete"));
}


STAFServiceResult STAFTraceService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();

    if ((action == "enable") || (action == "disable"))
        return handleEnableDisable(requestInfo);
    else if (action == "set")   return handleSet(requestInfo);
    else if (action == "list")  return handleList(requestInfo);
    else if (action == "purge") return handlePurge(requestInfo);
    else if (action == "help")    return handleHelp(requestInfo);
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

STAFServiceResult STAFTraceService::handleList(
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

    STAFString errorBuffer;
    STAFString traceFilename;

    STAFTraceDestination_e traceDestination = 
        STAFTrace::getTraceDestination(traceFilename);

    STAFTraceFileMode_e traceFileMode =
        STAFTrace::getTraceFileMode();

    // Create a marshalled map containing the trace information.
    // It will contain a map of the tracepoints and their trace states and
    // a map of the services and their trace states.

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fTraceInfoClass->reference());
    mc->setMapClassDefinition(fTracepointClass->reference());

    STAFObjectPtr traceInfoMap = fTraceInfoClass->createInstance();

    if (traceDestination == kSTAFTraceToStdout)
    {
        traceInfoMap->put("tracingTo", STAFString("Stdout"));
        traceInfoMap->put("fileMode", STAFObject::createNone());
    }
    else if (traceDestination == kSTAFTraceToStderr)
    {
        traceInfoMap->put("tracingTo", STAFString("Stderr"));
        traceInfoMap->put("fileMode", STAFObject::createNone());
    }
    else
    {
        if (traceDestination == kSTAFTraceToFile)
        {
            traceInfoMap->put("tracingTo", traceFilename);
        }
        else
        {
            // Multiple trace destinations so build a list of the trace
            // destinations (e.g. "Stdout" or "Stderr", and filename)

            STAFObjectPtr traceDestinationList = STAFObject::createList();

            if (traceDestination == kSTAFTraceToStdoutAndFile)
                traceDestinationList->append(STAFString("Stdout"));
            else if (traceDestination == kSTAFTraceToStderrAndFile)
                traceDestinationList->append(STAFString("Stderr"));

            traceDestinationList->append(traceFilename);

            traceInfoMap->put("tracingTo", traceDestinationList);
        }

        if (traceFileMode == kSTAFTraceFileAppend)
        {
            traceInfoMap->put("fileMode", "Append");
        }
        else
        {
            traceInfoMap->put("fileMode", "Replace");
        }
    }
    
    if (STAFServiceManager::getDefaultTraceState() == 
        STAFServiceManager::kTraceEnabled)
        traceInfoMap->put("defaultServiceState", STAFString("Enabled"));
    else
        traceInfoMap->put("defaultServiceState", STAFString("Disabled"));

    traceInfoMap->put("maxServiceResultSize",
                      STAFServiceManager::getMaxServiceResultSize());

    STAFObjectPtr tracepointMap = fTracepointClass->createInstance();

    STAFTraceService::STAFTracepointMap::iterator tracepointMapIter;

    for (tracepointMapIter = kSTAFTracepointMap.begin();
         tracepointMapIter != kSTAFTracepointMap.end(); tracepointMapIter++)
    {
        if (STAFTrace::doTrace(tracepointMapIter->first))
            tracepointMap->put(tracepointMapIter->second, kEnabledString);
        else
            tracepointMap->put(tracepointMapIter->second, kDisabledString);
    }

    traceInfoMap->put("tracePoints", tracepointMap);
   
    // Construct map class for the service states.  Note that this map
    // class definition is dynamically generated based on the services in
    // the serviceTraceStatusList at the time of this request.

    fServiceClass = STAFMapClassDefinition::create(
        "STAF/Service/Trace/Service");
    mc->setMapClassDefinition(fServiceClass->reference());
    STAFObjectPtr serviceMap = fServiceClass->createInstance();

    STAFServiceManager::ServiceTraceStatusList serviceTraceStatusList =
        STAFServiceManager::getServiceTraceStatusList();
    STAFServiceManager::OrderedServiceList allServiceList =
         gServiceManagerPtr->getOrderedServiceListCopy();
    STAFServiceManager::ServiceTraceStatusList::iterator traceStatusIter;

    for (traceStatusIter = serviceTraceStatusList.begin();
         traceStatusIter != serviceTraceStatusList.end(); traceStatusIter++)
    {
        fServiceClass->addKey(STAFString(traceStatusIter->first),
                              STAFString(traceStatusIter->first));

        if (traceStatusIter->second == STAFServiceManager::kTraceEnabled)
        {
            if (allServiceList.find(traceStatusIter->first) ==
                allServiceList.end())
            {
                serviceMap->put(
                    STAFString(traceStatusIter->first),
                    kEnabledString + STAFString(" (Not registered)"));
            }
            else
            {
                serviceMap->put(
                    STAFString(traceStatusIter->first), kEnabledString);
            }
        }
        else if (allServiceList.find(traceStatusIter->first) ==
                 allServiceList.end())
        {
            serviceMap->put(
                STAFString(traceStatusIter->first),
                kDisabledString + STAFString(" (Not registered)"));
        }
        else
        {
            serviceMap->put(
                STAFString(traceStatusIter->first), kDisabledString);
        }
    }

    traceInfoMap->put("services", serviceMap);

    mc->setRootObject(traceInfoMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
} 

STAFServiceResult STAFTraceService::handleSet(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "SET");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fSetParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFString errorBuffer;
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString filename;
    STAFString result;
    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("FILE", filename);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (parsedResult->optionTimes("DESTINATION") != 0)
    {
        // Set the trace destination

        if (parsedResult->optionTimes("FILE") == 0)
        {
            // FILE is not specified for the trace destination

            if (parsedResult->optionTimes("STDOUT"))
                STAFTrace::setTraceDestination(kSTAFTraceToStdout);
            else if (parsedResult->optionTimes("STDERR"))
                STAFTrace::setTraceDestination(kSTAFTraceToStderr);
        }
        else
        {
            // FILE is specified for the trace destination.
            // STDOUT or STDERR could have been specified too.

            STAFTraceDestination_t traceDestination;

            if (parsedResult->optionTimes("STDOUT"))
                traceDestination = kSTAFTraceToStdoutAndFile;
            else if (parsedResult->optionTimes("STDERR"))
                traceDestination = kSTAFTraceToStderrAndFile;
            else
                traceDestination = kSTAFTraceToFile;

            if (parsedResult->optionTimes("APPEND") != 0)
            {
                STAFTrace::setTraceDestination(traceDestination,
                                               filename,
                                               kSTAFTraceFileAppend);
            }
            else
            {
                STAFTrace::setTraceDestination(traceDestination,
                                               filename,
                                               kSTAFTraceFileReplace);
            }
        }
    }
    else if(parsedResult->optionTimes("DEFAULTSERVICESTATE") != 0)
    {
        //set default service tracing state
        bool stateToSetTo;
        STAFString defaultServiceState;

        rc = RESOLVE_STRING_OPTION("DEFAULTSERVICESTATE", defaultServiceState);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        defaultServiceState = defaultServiceState.upperCase();

        if(defaultServiceState == "DISABLED")
        {
            stateToSetTo = STAFServiceManager::kTraceDisabled;
        }
        else if (defaultServiceState == "ENABLED")
        {
            stateToSetTo = STAFServiceManager::kTraceEnabled;
        }
        else
        {
            STAFString msg = "Must specify either ENABLED or DISABLED";
            return STAFServiceResult(kSTAFInvalidRequestString, msg);
        }

        STAFServiceManager::setDefaultTraceState(stateToSetTo);
    }
    else if (parsedResult->optionTimes("MAXSERVICERESULTSIZE") != 0)
    {
        STAFString maxServiceResultSize;

        rc = RESOLVE_STRING_OPTION("MAXSERVICERESULTSIZE",
                                   maxServiceResultSize);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString_t errorBufferT = 0;
        unsigned int maxSize = 0;

        rc = STAFUtilConvertSizeString(
            maxServiceResultSize.getImpl(), &maxSize, &errorBufferT);

        if (rc == kSTAFOk)
        {
            STAFServiceManager::setMaxServiceResultSize(maxSize);
        }
        else
        {
            return STAFServiceResult(rc, errorBufferT);
        }
    }

    return STAFServiceResult(kSTAFOk, result);
}



STAFServiceResult STAFTraceService::handleEnableDisable(
    const STAFServiceRequest &requestInfo)
{
    // Parse the request

    STAFCommandParseResultPtr parsedResult = fEnableDisableParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFString errorBuffer;
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString result;
    STAFString filename;
    STAFRC_t rc;
    
    // Verify that the requesting machine/user has at least trust level 5
    
    bool stateToSetTo;

    if (parsedResult->optionTimes("ENABLE") != 0)
    {
        stateToSetTo = STAFServiceManager::kTraceEnabled;

        IVALIDATE_TRUST(5, "ENABLE");
    }
    else //DISABLE
    {
        stateToSetTo = STAFServiceManager::kTraceDisabled;

        IVALIDATE_TRUST(5, "DISABLE");
    }

    if (parsedResult->optionTimes("ALL") != 0)
    {
        // Handles ENAGLE|DISABLE ALL

        bool doTracepoints = false, doServices = false;

        if ((parsedResult->optionTimes("TRACEPOINTS") == 0) &&
            (parsedResult->optionTimes("SERVICES") == 0))
        {
            doTracepoints = true;
            doServices = true;
        }

        if (parsedResult->optionTimes("SERVICES") != 0)
            doServices = true;
        if (parsedResult->optionTimes("TRACEPOINTS") != 0)
            doTracepoints = true;

        if (doTracepoints)
        {
            if(stateToSetTo == STAFServiceManager::kTraceEnabled)
                   STAFTrace::traceOn(kSTAFTraceAll);
            else
                   STAFTrace::traceOff(kSTAFTraceAll);
        }
        if (doServices)
            STAFServiceManager::traceServicesChangeAll(stateToSetTo);
    }

    else
    {
        STAFString tracepointList = "";
        STAFTracePoint_t tracepointChangeSet = kSTAFTraceNone;

        if (parsedResult->optionTimes("TRACEPOINTS") != 0)
        {
            STAFString tracepoints;

            rc = RESOLVE_STRING_OPTION("TRACEPOINTS", tracepoints);

            if (rc) return STAFServiceResult(rc, errorBuffer);

            tracepointList = tracepoints;
        }

        if (parsedResult->optionTimes("TRACEPOINT") != 0)
        {
            STAFString tracepoint;
            for (int i = 1, optionCount =
                parsedResult->optionTimes("TRACEPOINT"); i <= optionCount; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("TRACEPOINT", i, tracepoint);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                tracepointList += " " + tracepoint;
            }
        }

        tracepointList = tracepointList.upperCase();

        //validate list of tracepoints
        for (int i = 0; i < tracepointList.numWords(); ++i )
        {
            STAFString tracepoint = tracepointList.subWord(i, 1).upperCase();

            if (kSTAFTracepointNameMap.find(tracepoint) !=
                    kSTAFTracepointNameMap.end())
            {
                tracepointChangeSet =
                    tracepointChangeSet | kSTAFTracepointNameMap[tracepoint];
            }
            else
                return STAFServiceResult(kSTAFInvalidValue, tracepoint);
        }

        //now handle service tracing

        STAFString serviceList = "";

        if (parsedResult->optionTimes("SERVICES") != 0)
        {
            STAFString services;

            rc = RESOLVE_STRING_OPTION("SERVICES", services);

            if (rc) return STAFServiceResult(rc, errorBuffer);

            serviceList = services;
        }

        if (parsedResult->optionTimes("SERVICE") != 0)
        {
            STAFString service;

            for (int i = 1, optionCount =
                parsedResult->optionTimes("SERVICE"); i<= optionCount; ++i)
            {

                rc = RESOLVE_INDEXED_STRING_OPTION("SERVICE", i, service);
                serviceList += " " + service;

                if (rc) return STAFServiceResult(rc, errorBuffer);
            }
        }

        //after ensuring there are no errors, make desired changes
        serviceList = serviceList.upperCase();

        STAFServiceManager::traceServicesChange(serviceList, stateToSetTo);

        if(stateToSetTo == STAFServiceManager::kTraceEnabled)
            STAFTrace::traceOn(tracepointChangeSet);
        else
            STAFTrace::traceOff(tracepointChangeSet);
    }

    return STAFServiceResult(kSTAFOk, result);
}

STAFServiceResult STAFTraceService::handlePurge(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "PURGE");

    STAFServiceManager::purgeUnregisteredServices();

    return STAFServiceResult(kSTAFOk, "");
}


STAFServiceResult STAFTraceService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}

STAFString STAFTraceService::spaces(int numSpaces)
{
    STAFString str = "";
    for (int i = 0; i < numSpaces; i++)
    {
        str += " ";
    }
    return str;
}

STAFTraceService::STAFTracepointMap STAFTraceService::fillSTAFTracepointMap()
{
    STAFTracepointMap tMap;

    tMap[kSTAFTraceServiceComplete] = "ServiceComplete";
    tMap[kSTAFTraceDebug] = "Debug";
    tMap[kSTAFTraceDeprecated] = "Deprecated";
    tMap[kSTAFTraceError] = "Error";
    tMap[kSTAFTraceInfo] = "Info";
    tMap[kSTAFTraceRegistration] = "Registration";
    tMap[kSTAFTraceRemoteRequests] = "RemoteRequests";
    tMap[kSTAFTraceServiceAccessDenied] = "ServiceAccessDenied";
    tMap[kSTAFTraceServiceError] = "ServiceError";
    tMap[kSTAFTraceServiceManagement] = "ServiceManagement";
    tMap[kSTAFTraceServiceRequest] = "ServiceRequest";
    tMap[kSTAFTraceServiceResult] = "ServiceResult";
    tMap[kSTAFTraceWarning] = "Warning";

    return tMap;
}

STAFTraceService::STAFTracepointNameMap STAFTraceService::fillSTAFTracepointNameMap()
{
    STAFTracepointNameMap tNameMap;
    STAFTracepointMap::iterator tMapIter;

    for (tMapIter = kSTAFTracepointMap.begin();
        tMapIter != kSTAFTracepointMap.end();
        tMapIter++)
    {
        tNameMap[(tMapIter->second).upperCase()] = tMapIter->first;
    }
    return tNameMap;
}

STAFString STAFTraceService::info(unsigned int) const
{
    return (name() + ": Internal");
}

STAFTraceService::~STAFTraceService()
{
   ; /* Do Nothing */
}
