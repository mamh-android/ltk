/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFUtil.h"
#include "STAFServiceManager.h"
#include "STAFMiscService.h"
#include "STAFThreadManager.h"
#include "STAFConnectionManager.h"
#include "STAFProcessService.h"
#include "STAF_fstream.h"

static STAFMutexSem sConnectAttemptsSem;
static STAFMutexSem sConnectRetryDelaySem;
static STAFMutexSem sMaxQueueSizeSem;
static STAFMutexSem sDefaultInterfaceSem;
static STAFMutexSem sDefaultAuthenticatorSem;
static STAFMutexSem sResultCompatibilityModeSem;
static STAFMutexSem sHandleGCInterval;

static STAFString sHelpMsg;

static const STAFString sVersion = "version";
static const STAFString sPlatform = "platform";
static const STAFString sArchitecture = "architecture";
static const STAFString sInstaller = "installer";
static const STAFString sFile = "file";
static const STAFString sOsname = "osname";
static const STAFString sOsversion = "osversion";
static const STAFString sOsarch = "osarch";

STAFMiscService::STAFMiscService() : STAFService("MISC")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** MISC Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "VERSION" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "WHOAMI" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "WHOAREYOU" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST  INTERFACES | SETTINGS | ENDPOINTCACHE | PROPERTIES" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "QUERY INTERFACE <Name>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SET   [CONNECTATTEMPTS <Number>] [CONNECTRETRYDELAY <Number>[s|m|h|d|w]]" +
        *gLineSeparatorPtr +
        "      [MAXQUEUESIZE <Number>] [HANDLEGCINTERVAL <Number>[s|m|h|d]]" +
        *gLineSeparatorPtr +
        "      [INTERFACECYCLING <Enabled | Disabled>]" +
        *gLineSeparatorPtr +
        "      [DEFAULTINTERFACE <Name>] [DEFAULTAUTHENTICATOR <Name>] " +
        *gLineSeparatorPtr +
        "      [RESULTCOMPATIBILITYMODE <Verbose | None>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "PURGE ENDPOINTCACHE <ENDPOINT <Endpoint>... | CONFIRM>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // The following are undocumented command requests for this service:
    // THREAD INFO
    // TEST MARSHALLING DATA <X>
    // DEBUG MEMORY <CHECKPOINT | DUMP [ALL] | STATS | STATSDIFF | LEAKS>

    // Create the command request parsers

    // thread options

    fThreadParser.addOption("THREAD", 1,
        STAFCommandParser::kValueNotAllowed);
    fThreadParser.addOption("INFO", 1,
        STAFCommandParser::kValueNotAllowed);

    // thread groups

    fThreadParser.addOptionGroup("THREAD", 1, 1);
    fThreadParser.addOptionGroup("INFO", 1, 1);

    // list options
    
    fListParser.addOption("LIST", 1,
                          STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("INTERFACES", 1,
                          STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SETTINGS", 1,
                          STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("ENDPOINTCACHE", 1,
                          STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("PROPERTIES", 1,
                          STAFCommandParser::kValueNotAllowed);

    // list groups/needs

    fListParser.addOptionGroup(
        "INTERFACES SETTINGS ENDPOINTCACHE PROPERTIES", 1, 1);

    // query options
    
    fQueryParser.addOption("QUERY", 1,
                           STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption("INTERFACE", 1,
                           STAFCommandParser::kValueRequired);
    // query groups/needs

    fQueryParser.addOptionGroup("INTERFACE", 1, 1);

    // purge options

    fPurgeParser.addOption("PURGE", 1,
                           STAFCommandParser::kValueNotAllowed);
    fPurgeParser.addOption("ENDPOINTCACHE", 1,
                           STAFCommandParser::kValueNotAllowed);
    fPurgeParser.addOption("ENDPOINT", 0,
                           STAFCommandParser::kValueRequired);
    fPurgeParser.addOption("CONFIRM", 1,
                           STAFCommandParser::kValueNotAllowed);

    // purge groups/needs

    fPurgeParser.addOptionNeed("ENDPOINTCACHE", "PURGE");
    fPurgeParser.addOptionGroup("ENDPOINTCACHE", 1, 1);
    fPurgeParser.addOptionGroup("ENDPOINT CONFIRM", 1, 1);
    
    // set options

    fSetParser.addOption("SET", 1,
                         STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("CONNECTATTEMPTS", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("CONNECTRETRYDELAY", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("INTERFACECYCLING", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("MAXQUEUESIZE", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("HANDLEGCINTERVAL", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTINTERFACE", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHENTICATOR", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("RESULTCOMPATIBILITYMODE", 1,
                         STAFCommandParser::kValueRequired);
    
    // test options

    fDebugParser.addOption("DEBUG", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("MEMORY", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("CHECKPOINT", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("STATS", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("STATSDIFF", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("DUMP", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("LEAKS", 1,
                           STAFCommandParser::kValueNotAllowed);
    fDebugParser.addOption("ALL", 1,
                           STAFCommandParser::kValueNotAllowed);

    // debug groups/needs

    fDebugParser.addOptionGroup("CHECKPOINT DUMP STATS STATSDIFF LEAKS", 1, 1);
    fDebugParser.addOptionNeed("DEBUG", "MEMORY");
    fDebugParser.addOptionNeed("ALL", "DUMP");

    // test options

    fTestParser.addOption("TEST", 1,
                           STAFCommandParser::kValueNotAllowed);
    fTestParser.addOption("MARSHALLING", 1,
                           STAFCommandParser::kValueNotAllowed);
    fTestParser.addOption("DATA", 1,
                           STAFCommandParser::kValueRequired);
    // test groups/needs

    fTestParser.addOptionNeed("TEST", "MARSHALLING");

    // Initialize debug memory structures

    #if defined(STAF_OS_TYPE_WIN32) && defined(_DEBUG)
    _CrtMemCheckpoint(&fFromMemState);
  
    // Send all reports to STDOUT
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    #endif

    // Create the map class definition for "test marshalling" data

    fLogRecordClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/SampleLogRecord");

    fLogRecordClass->addKey("timestamp", "Date-time");
    fLogRecordClass->addKey("level", "Level");
    fLogRecordClass->addKey("machine", "Machine");
    fLogRecordClass->addKey("message", "Message");

    // Create a map of lists

    STAFObjectPtr innerList1 = STAFObject::createList();

    innerList1->append("String 1");
    innerList1->append("String 2");

    STAFObjectPtr innerList2 = STAFObject::createList();

    innerList2->append("String 3");
    innerList2->append("String 4");

    fMapOfLists = STAFObject::createMap();
    fMapOfLists->put("List1", innerList1);
    fMapOfLists->put("List2", innerList2);

    // Create a list of maps

    STAFObjectPtr innerMap1 = STAFObject::createMap();

    innerMap1->put("Key1", "Value1");
    innerMap1->put("Key2", "Value2");

    STAFObjectPtr innerMap2 = STAFObject::createMap();

    innerMap2->put("Key3", "Value3");
    innerMap2->put("Key4", "Value4");

    fListOfMaps = STAFObject::createList();
    fListOfMaps->append(innerMap1);
    fListOfMaps->append(innerMap2);

    // Construct map class for whoami information

    fWhoamiClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/Whoami");
 
    fWhoamiClass->addKey("instanceName",    "Instance Name");
    fWhoamiClass->addKey("instanceUUID",    "Instance UUID");
    fWhoamiClass->addKey("requestNumber",   "Request Number");
    fWhoamiClass->addKey("interface",       "Interface");
    fWhoamiClass->addKey("logicalID",       "Logical ID");
    fWhoamiClass->addKey("physicalID",      "Physical ID");
    fWhoamiClass->addKey("endpoint",        "Endpoint");
    fWhoamiClass->addKey("machine",         "Machine");
    fWhoamiClass->addKey("machineNickname", "Machine Nickname");
    fWhoamiClass->addKey("isLocalRequest",  "Local Request");
    fWhoamiClass->addKey("handle",          "Handle");
    fWhoamiClass->addKey("handleName",      "Handle Name");    
    fWhoamiClass->addKey("user",            "User");
    fWhoamiClass->addKey("trustLevel",      "Trust Level");

    // Construct map class for whoAreYou information

    fWhoAreYouClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/WhoAreYou");
 
    fWhoAreYouClass->addKey("instanceName",    "Instance Name");
    fWhoAreYouClass->addKey("instanceUUID",    "Instance UUID");
    fWhoAreYouClass->addKey("machine",         "Machine");
    fWhoAreYouClass->addKey("machineNickname", "Machine Nickname");
    fWhoAreYouClass->addKey("isLocalRequest",  "Local Request");
    fWhoAreYouClass->addKey("currentTimestamp",  "Current Date-Time");

    // Construct map class for interface information

    fInterfaceClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/Interface");
 
    fInterfaceClass->addKey("name",      "Interface Name");
    fInterfaceClass->addKey("library",   "Library");
    fInterfaceClass->addKey("optionMap", "Options");

    // Construct map class for settings information

    fSettingsClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/Settings");

    fSettingsClass->addKey("connectAttempts", "Connection Attempts");
    fSettingsClass->addKey("connectRetryDelay", "Connect Retry Delay");
    fSettingsClass->addKey("interfaceCycling", "Interface Cycling");
    fSettingsClass->addKey("maxQueueSize", "Maximum Queue Size");
    fSettingsClass->addKey("maxReturnFileSize", "Maximum Return File Size");
    fSettingsClass->addKey("handleGCInterval", "Handle GC Interval");
    fSettingsClass->addKey("initialThreads", "Initial Threads");
    fSettingsClass->addKey("threadGrowthDelta", "Thread Growth Delta");
    fSettingsClass->addKey("dataDir", "Data Directory");
    fSettingsClass->addKey("defaultInterface","Default Interface");
    fSettingsClass->addKey("defaultAuthenticator", "Default Authenticator");
    fSettingsClass->addKey("resultCompatibilityMode",
                           "Result Compatibility Mode");

    // Construct map class for listing the endpoint cache

    fListCacheClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/EndpointCache");

    fListCacheClass->addKey("endpoint", "Endpoint");
    fListCacheClass->addKey("interface", "Interface");
    fListCacheClass->addKey("createdTimestamp", "Date-Time");

    // Construct map class for purging the endpoint cache

    fPurgeCacheClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/PurgeStats");

    fPurgeCacheClass->addKey("numPurged",    "Purged Endpoints");
    fPurgeCacheClass->addKey("numRemaining", "Remaining Endpoints");

    // Construct map class for thread information

    fThreadClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/Thread");

    fThreadClass->addKey("totalThreads",   "Total Threads");
    fThreadClass->addKey("workingThreads", "Working Threads");
    fThreadClass->addKey("idleThreads" ,   "Idle Threads");

    // Construct map class for properties information

    fPropertiesClass = STAFMapClassDefinition::create(
        "STAF/Service/Misc/Properties");

    fPropertiesClass->addKey("version", "version");
    fPropertiesClass->addKey("platform", "platform");
    fPropertiesClass->addKey("architecture", "architecture");
    fPropertiesClass->addKey("installer", "installer");
    fPropertiesClass->addKey("file", "file");
    fPropertiesClass->addKey("osname", "osname");
    fPropertiesClass->addKey("osversion", "osversion");
    fPropertiesClass->addKey("osarch","osarch");
}


STAFServiceResult STAFMiscService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();

    if      (action == "version")   return handleVersion(requestInfo);
    else if (action == "whoami")    return handleWhoAmI(requestInfo);
    else if (action == "whoareyou") return handleWhoAreYou(requestInfo);
    else if (action == "thread")    return handleThread(requestInfo);
    else if (action == "list")      return handleList(requestInfo);
    else if (action == "query")     return handleQuery(requestInfo);
    else if (action == "purge")     return handlePurge(requestInfo);
    else if (action == "set")       return handleSet(requestInfo);
    else if (action == "test")      return handleTest(requestInfo);
    else if (action == "debug")     return handleDebug(requestInfo);
    else if (action == "help")      return handleHelp(requestInfo);
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


STAFServiceResult STAFMiscService::handleVersion(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "VERSION");
    
    return STAFServiceResult(kSTAFOk, gVersion);
}


STAFServiceResult STAFMiscService::handleWhoAmI(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "WHOAMI");

    // Create a marshalled map containing whoami information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    mc->setMapClassDefinition(fWhoamiClass->reference());

    STAFObjectPtr whoamiMap = fWhoamiClass->createInstance();

    whoamiMap->put("instanceName",    *gSTAFInstanceNamePtr);
    whoamiMap->put("instanceUUID",    requestInfo.fSTAFInstanceUUID);
    whoamiMap->put("requestNumber",   STAFString(requestInfo.fRequestNumber));
    whoamiMap->put("machine",         requestInfo.fMachine);
    whoamiMap->put("machineNickname", requestInfo.fMachineNickname);
    whoamiMap->put("interface",       requestInfo.fInterface);
    whoamiMap->put("physicalID",      requestInfo.fPhysicalInterfaceID);
    whoamiMap->put("logicalID",       requestInfo.fLogicalInterfaceID);
    whoamiMap->put("endpoint",        requestInfo.fEndpoint);
    whoamiMap->put("isLocalRequest",
                   requestInfo.fIsLocalRequest ? "Yes" : "No");
    whoamiMap->put("handle",          STAFString(requestInfo.fHandle));
    whoamiMap->put("handleName",      requestInfo.fHandleName);
    whoamiMap->put("user",            requestInfo.fUser);

    // This is safe to give them, as they can deduce it anyway

    whoamiMap->put("trustLevel",    STAFString(requestInfo.fTrustLevel));

    mc->setRootObject(whoamiMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handleWhoAreYou(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "WHOAREYOU");

    // Create a marshalled map containing whoareyou information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    mc->setMapClassDefinition(fWhoAreYouClass->reference());

    STAFObjectPtr whoAreYouMap = fWhoAreYouClass->createInstance();

    whoAreYouMap->put("instanceName",    *gSTAFInstanceNamePtr);
    whoAreYouMap->put("instanceUUID",    *gSTAFInstanceUUIDPtr);
    whoAreYouMap->put("machine",         *gMachinePtr);
    whoAreYouMap->put("machineNickname", *gMachineNicknamePtr);
    whoAreYouMap->put("isLocalRequest",
                      requestInfo.fIsLocalRequest ? "Yes" : "No");

    STAFTimestamp currentTime = STAFTimestamp::now();
    whoAreYouMap->put("currentTimestamp", currentTime.asString());

    mc->setRootObject(whoAreYouMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handleThread(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    IVALIDATE_TRUST(3, "THREAD");

    // Create a marshalled map containing whoami information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fThreadClass->reference());

    STAFObjectPtr threadMap = fThreadClass->createInstance();

    threadMap->put("totalThreads",
                   STAFString(gThreadManagerPtr->getThreadPoolSize()));
    threadMap->put("workingThreads",
                   STAFString(gThreadManagerPtr->getNumWorkingThreads()));
    threadMap->put("idleThreads",
                   STAFString(gThreadManagerPtr->getNumReadyThreads()));

    mc->setRootObject(threadMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fListParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Create a marshalling context to represent the result

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("INTERFACES") > 0)
    {
        // LIST INTERFACES

        // Create a marshalled list of maps containing interface information

        mc->setMapClassDefinition(fInterfaceClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();
    
        STAFConnectionManager::ConnectionProviderList connProvList =
            gConnectionManagerPtr->getConnectionProviderListCopy();

        for (STAFConnectionManager::ConnectionProviderList::iterator
             iter = connProvList.begin(); iter != connProvList.end(); ++iter)
        {
            STAFObjectPtr interfaceMap = fInterfaceClass->createInstance();
            interfaceMap->put("name", (*iter)->getName());
            interfaceMap->put("library", (*iter)->getLibrary());

            // Get the options from the connection provider

            STAFObjectPtr options;
            (*iter)->getOptions(options);
            interfaceMap->put("optionMap", options->reference());

            outputList->append(interfaceMap);
        }

        mc->setRootObject(outputList);

        // Get rid of connection provider references
        connProvList = STAFConnectionManager::ConnectionProviderList();
    }
    else if (parsedResult->optionTimes("ENDPOINTCACHE") > 0)
    {
        // LIST ENDPOINTCACHE

        // Create a marshalled list of maps representing the entries in the
        // endpoint cache map.

        mc->setMapClassDefinition(fListCacheClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();

        STAFConnectionManager::EndpointCacheMap cacheMap =
            gConnectionManagerPtr->getEndpointCacheMapCopy();

        for (STAFConnectionManager::EndpointCacheMap::iterator
             iter = cacheMap.begin(); iter != cacheMap.end(); ++ iter)
        {
            STAFObjectPtr outputMap = fListCacheClass->createInstance();

            outputMap->put("endpoint", (*iter).first);
            outputMap->put("interface", (*iter).second.interface);
            outputMap->put("createdTimestamp",
                           (*iter).second.createdTimestamp.asString());

            outputList->append(outputMap);
        }

        mc->setRootObject(outputList);

        // Get rid of cache map references

        cacheMap = STAFConnectionManager::EndpointCacheMap();
    }
    else if (parsedResult->optionTimes("PROPERTIES") > 0)
    {
        // LIST PROPERTIES

        // Get the contents of the install.properties file

        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            "local",
            "VAR",
            "RESOLVE STRING "
            "{STAF/Config/STAFRoot}{STAF/Config/Sep/File}install.properties");

        ifstream propertiesFile(result->result.toCurrentCodePage()->buffer());

        if (!propertiesFile)
        {
            return STAFServiceResult(kSTAFDoesNotExist, result->result);
        }

        // Create a marshalled map containing properties information

        mc->setMapClassDefinition(fPropertiesClass->reference());
        STAFObjectPtr propertiesMap = fPropertiesClass->createInstance();

        STAFString line;
        char propertyLine[80];

        for(; propertiesFile.good() ;)
        {
            propertyLine[0] = 0;
            propertiesFile.getline(propertyLine, 80);

            if (!propertiesFile.good())
            {
                if (propertyLine == "") break;
            }

            STAFString thisProperty(propertyLine);

            int equalPos = thisProperty.findFirstOf(kUTF8_EQUAL);

            if (equalPos == STAFString::kNPos)
                break;

            STAFString key = thisProperty.subString(0, equalPos);
            STAFString value = thisProperty.subString(equalPos + 1);

            if (key.isEqualTo(sVersion))
                propertiesMap->put(sVersion, value);
            else if (key.isEqualTo(sPlatform))
                propertiesMap->put(sPlatform, value);
            else if (key.isEqualTo(sArchitecture))
                propertiesMap->put(sArchitecture, value);
            else if (key.isEqualTo(sInstaller))
                propertiesMap->put(sInstaller, value);
            else if (key.isEqualTo(sFile))
                propertiesMap->put(sFile, value);
            else if (key.isEqualTo(sOsname))
                propertiesMap->put(sOsname, value);
            else if (key.isEqualTo(sOsversion))
                propertiesMap->put(sOsversion, value);
            else if (key.isEqualTo(sOsarch))
                propertiesMap->put(sOsarch, value);
        }

        propertiesFile.close();

        mc->setRootObject(propertiesMap);
    }
    else
    {
        // LIST SETTINGS

        // Create a marshalled map containing settings information

        mc->setMapClassDefinition(fSettingsClass->reference());
        STAFObjectPtr settingsMap = fSettingsClass->createInstance();
        settingsMap->put("connectAttempts",
                         STAFString(gConnectionAttempts));
        settingsMap->put("connectRetryDelay",
                         STAFString(gConnectionRetryDelay));

        if (gConnectionManagerPtr->getAutoInterfaceCycling())
            settingsMap->put("interfaceCycling", "Enabled");
        else
            settingsMap->put("interfaceCycling", "Disabled");

        settingsMap->put("maxQueueSize", STAFString(gMaxQueueSize));
        settingsMap->put("maxReturnFileSize", STAFString(gMaxReturnFileSize));
        settingsMap->put("handleGCInterval", STAFString(gHandleGCInterval));
        settingsMap->put("initialThreads", STAFString(gNumInitialThreads));
        settingsMap->put("threadGrowthDelta",
                         STAFString(gThreadManagerPtr->getGrowthDelta()));
        settingsMap->put("dataDir", *gSTAFWriteLocationPtr);
        settingsMap->put("defaultInterface",
                         gConnectionManagerPtr->getDefaultConnectionProvider());
        settingsMap->put("defaultAuthenticator",
                         gServiceManagerPtr->getDefaultAuthenticator());

        if (gResultCompatibilityMode == kSTAFResultCompatibilityVerbose)
            settingsMap->put("resultCompatibilityMode", "Verbose");
        else
            settingsMap->put("resultCompatibilityMode", "None");

        mc->setRootObject(settingsMap);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handlePurge(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "PURGE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fPurgeParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
    
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFRC_t rc = 0;

    unsigned int numPurged = 0;
    unsigned int numTotal = gConnectionManagerPtr->getEndpointCacheSize();
    
    if (parsedResult->optionTimes("CONFIRM"))
    {
        // Purge all entries from the Endpoint Cache

        rc = gConnectionManagerPtr->purgeEndpointCache();

        if (rc == kSTAFOk)
            numPurged = numTotal;
    }
    else
    {
        // Remove each specified endpoint from the Endpoint Cache

        // Create a list of the resolved endpoints to be purged

        unsigned int numEndpoints = parsedResult->optionTimes("ENDPOINT");
        std::vector<STAFString> endpointList;
        unsigned int i = 0;

        for (i = 1; i <= numEndpoints; ++i)
        {
            STAFString endpoint;
            rc = RESOLVE_INDEXED_STRING_OPTION("ENDPOINT", i, endpoint);

            if (rc) return STAFServiceResult(rc, errorBuffer);

            endpointList.push_back(endpoint);
        }

        // Iterate through endpoint list and remove each endpoint from
        // the cache

        for (i = 0; i < endpointList.size(); ++i)
        {
            rc = gConnectionManagerPtr->removeFromEndpointCache(
                endpointList[i]);

            if (rc == kSTAFOk)
                numPurged++;
        }
    }

    // Create a marshalling context to represent the result

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fPurgeCacheClass->reference());
    STAFObjectPtr resultMap = fPurgeCacheClass->createInstance();
    resultMap->put("numPurged",   STAFString(numPurged));
    resultMap->put("numRemaining", STAFString(numTotal - numPurged));
    mc->setRootObject(resultMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handleSet(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "SET");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fSetParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFRC_t rc = 0;

    if (parsedResult->optionTimes("CONNECTATTEMPTS") > 0)
    {
        // SET CONNECTATTEMPTS

        unsigned int connectAttempts;
        
        rc = RESOLVE_UINT_OPTION_RANGE(
            "CONNECTATTEMPTS", connectAttempts, 1, UINT_MAX);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Use a mutex semaphore to get exclusive access to gConnectionAttempts
        
        STAFMutexSemLock lock(sConnectAttemptsSem);
        gConnectionAttempts = connectAttempts;
    }

    if (parsedResult->optionTimes("CONNECTRETRYDELAY") > 0)
    {
        // SET CONNECTRETRYDELAY

        unsigned int connectRetryDelay;
        
        rc = RESOLVE_DEFAULT_DURATION_OPTION(
            "CONNECTRETRYDELAY", connectRetryDelay, 1000);
        
        if (rc) return STAFServiceResult(rc, errorBuffer);
        
        // Use a mutex semaphore to get exclusive access to
        // gConnectionRetryDelay
        
        STAFMutexSemLock lock(sConnectRetryDelaySem);
        gConnectionRetryDelay = connectRetryDelay;
    }

    if (parsedResult->optionTimes("INTERFACECYCLING") > 0)
    {
        // SET INTERFACECYCLING

        STAFString interfaceCycling;
        rc = RESOLVE_STRING_OPTION("INTERFACECYCLING", interfaceCycling);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        if (interfaceCycling.toUpperCase() == "ENABLED")
        {
            gConnectionManagerPtr->enableAutoInterfaceCycling();
        }
        else if (interfaceCycling.toUpperCase() == "DISABLED")
        {
            gConnectionManagerPtr->disableAutoInterfaceCycling();
        }
        else
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "INTERFACECYCLING must be set to Enabled or Disabled");
        }
    }

    if (parsedResult->optionTimes("MAXQUEUESIZE") > 0)
    {
        // SET MAXQUEUESIZE

        unsigned int maxQueueSize;
        
        rc = RESOLVE_UINT_OPTION_RANGE(
            "MAXQUEUESIZE", maxQueueSize, 0, UINT_MAX);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Use a mutex semaphore to get exclusive access to gMaxQueueSize
        
        STAFMutexSemLock lock(sMaxQueueSizeSem);
        gMaxQueueSize = maxQueueSize;
    }

    if (parsedResult->optionTimes("DEFAULTINTERFACE") > 0)
    {
        // SET DEFAULTINTERFACE

        STAFString defaultInterface;

        rc = RESOLVE_STRING_OPTION("DEFAULTINTERFACE", defaultInterface);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Use a mutex semaphore to get exclusive access to the default
        // interface
        
        {
            STAFMutexSemLock lock(sDefaultInterfaceSem);
            rc = gConnectionManagerPtr->setDefaultConnectionProvider(
                defaultInterface);
        }

        if (rc)
        {
            return STAFServiceResult(
                rc, "DEFAULTINTERFACE " + defaultInterface +
                " is not registered");
        }

        // Change the global STAF/Config/DefaultInterface variable value

        (*gGlobalVariablePoolPtr)->set("STAF/Config/DefaultInterface",
                                       defaultInterface);
    }

    if (parsedResult->optionTimes("DEFAULTAUTHENTICATOR") > 0)
    {
        // SET DEFAULTAUTHENTICATOR

        STAFString defaultAuth;

        rc = RESOLVE_STRING_OPTION("DEFAULTAUTHENTICATOR", defaultAuth);

        // Use a mutex semaphore to get exclusive access to the default
        // authenticator
        
        {
            STAFMutexSemLock lock(sDefaultAuthenticatorSem);
            rc = gServiceManagerPtr->setDefaultAuthenticator(defaultAuth);
        }

        if (rc)
        {
            return STAFServiceResult(
                rc, "DEFAULTAUTHENTICATOR " + defaultAuth +
                " is not registered");
        }
        
        // Change the global STAF/Config/DefaultAuthenticator variable value

        (*gGlobalVariablePoolPtr)->set("STAF/Config/DefaultAuthenticator",
                                       defaultAuth);
    }

    if (parsedResult->optionTimes("RESULTCOMPATIBILITYMODE") > 0)
    {
        // SET RESULTCOMPATIBILITYMODE

        STAFString mode;
        rc = RESOLVE_STRING_OPTION("RESULTCOMPATIBILITYMODE", mode);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        if (mode.isEqualTo("NONE", kSTAFStringCaseInsensitive))
        {
            // Use a mutex semaphore to get exclusive access to
            // gResultCompatibilityMode
         
            STAFMutexSemLock lock(sResultCompatibilityModeSem);
            gResultCompatibilityMode = kSTAFResultCompatibilityNone;
        }
        else if (mode.isEqualTo("VERBOSE", kSTAFStringCaseInsensitive))
        {
            // Use a mutex semaphore to get exclusive access to
            // gResultCompatibilityMode
         
            STAFMutexSemLock lock(sResultCompatibilityModeSem);
            gResultCompatibilityMode = kSTAFResultCompatibilityVerbose;
        }
        else
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "RESULTCOMPATIBILITYMODE must be None or Verbose");
        }
    }

    if (parsedResult->optionTimes("HANDLEGCINTERVAL") > 0)
    {
        // SET HANDLEGCINTERVAL

        unsigned int handleGCInterval = 0;

        if (!rc) rc = RESOLVE_DEFAULT_DURATION_OPTION(
            "HANDLEGCINTERVAL", handleGCInterval, 60000);

        if (rc) return STAFServiceResult(rc, errorBuffer);
        
        // Check if the interval specified for handle garbage collection is
        // valid.  It must be >= 5 seconds and <= 24 hours.

        if ((handleGCInterval < 5000) || (handleGCInterval > 86400000))
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "HANDLEGCINTERVAL must be between 5 seconds and 24 hours.\n\n"
                "This value may be expressed in milliseconds, seconds, "
                "minutes, hours, or a day.  Its format is <Number>[s|m|h|d] "
                "where <Number> is an integer >= 0 and indicates milliseconds "
                "unless one of the following case-insensitive suffixes is "
                "specified:  s (for seconds), m (for minutes), h (for hours), "
                "or d (for day).  The calculated value must be >= 5000 and <= "
                " 86400000 milliseconds.\n\nExamples: \n"
                "  60000 specifies 60000 milliseconds (or 1 minute), \n"
                "  30s specifies 30 seconds, \n"
                "  5m specifies 5 minutes, \n"
                "  2h specifies 2 hours, \n"
                "  1d specifies 1 day.");
            return 1;
        }

        // Use a mutex semaphore to get exclusive access to set the
        // gHandleGCInterval variable

        STAFMutexSemLock lock(sHandleGCInterval);
        gHandleGCInterval = handleGCInterval;
    }

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFMiscService::handleQuery(
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
    STAFString theInterface;
    STAFString errorBuffer;
    STAFRC_t rc = RESOLVE_STRING_OPTION("INTERFACE", theInterface);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Create a marshalled map containing interface information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fInterfaceClass->reference());

    bool interfaceFound = false;

    STAFConnectionManager::ConnectionProviderList connProvList =
        gConnectionManagerPtr->getConnectionProviderListCopy();

    for (STAFConnectionManager::ConnectionProviderList::iterator
         iter = connProvList.begin(); iter != connProvList.end(); ++iter)
    {
        if (theInterface == (*iter)->getName())
        {
            interfaceFound = true;

            STAFObjectPtr interfaceMap = fInterfaceClass->createInstance();
            interfaceMap->put("name", theInterface);
            interfaceMap->put("library", (*iter)->getLibrary());

            // Get the options from the connection provider

            STAFObjectPtr options;
            (*iter)->getOptions(options);
            interfaceMap->put("optionMap", options->reference());

            mc->setRootObject(interfaceMap);

            // Found the interface so break out of the loop
            break;
        }
    }

    // Get rid of connection provider references
    connProvList = STAFConnectionManager::ConnectionProviderList();
    
    if (!interfaceFound)
        return STAFServiceResult(kSTAFDoesNotExist, theInterface);
    else
        return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handleTest(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    IVALIDATE_TRUST(3, "TEST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fTestParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Set up some constants that are used repeatedly

    STAFString sTimestamp("timestamp");
    STAFString sLevel("level");
    STAFString sMachine("machine");
    STAFString sSystem("x.y.z.com");
    STAFString sMessage("message");
    STAFString sSTAFMapClassName("staf-map-class-name");
    STAFString sLogRecord("STAF/Service/Misc/SampleLogRecord");
    STAFString sInfo("info");

    // Create the marshalling context

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    // Figure out which data set the user asked for

    unsigned int dataSet = 1;

    if (parsedResult->optionTimes("DATA") != 0)
    {
        // Convert the DATA option string to an unsigned integer and return
        // an error if it is not in the range of 1 to 7

        STAFString errorBuffer;

        STAFRC_t rc = convertStringToUInt(
            parsedResult->optionValue("DATA"), "DATA", dataSet,
            errorBuffer, 1, 7);

        if (rc) return STAFServiceResult(kSTAFInvalidValue, errorBuffer);
    }

    if ((dataSet == 1) || (dataSet == 2))
    {
        mc->setMapClassDefinition(fLogRecordClass->reference());

        // Populate the list with regular log records

        STAFObjectPtr logList = STAFObject::createList();
        int i = 0;

        for (i = 1; i < 6; ++i)
        {
            STAFObjectPtr logRecord = STAFObject::createMap();

            logRecord->put(sSTAFMapClassName, sLogRecord);
            logRecord->put(sLevel, sInfo);
            logRecord->put(sMessage, STAFString("Message #") + i);
            logRecord->put(sMachine, sSystem);
            logRecord->put(sTimestamp, STAFTimestamp::now().asString());

            logList->append(logRecord);
        }

        // If we are in dataSet == 2, then we add some "broken" log records to
        // the list

        if (dataSet == 2)
        {
            // Now, let's add one record that doesn't use the map class

            STAFObjectPtr logRecord = STAFObject::createMap();

            logRecord->put(sLevel, sInfo);
            logRecord->put(sMessage, STAFString("Message #") + i++);
            logRecord->put(sMachine, sSystem);
            logRecord->put(sTimestamp, STAFTimestamp::now().asString());

            logList->append(logRecord);

            // Now, let's add one record using the map class that doesn't include
            // all the necessary keys

            logRecord = STAFObject::createMap();

            logRecord->put(sSTAFMapClassName, sLogRecord);
            logRecord->put(sMessage, STAFString("Message #") + i++);
            logRecord->put(sMachine, sSystem);
            logRecord->put(sTimestamp, STAFTimestamp::now().asString());

            logList->append(logRecord);
        }

        mc->setRootObject(logList);
    }
    else if (dataSet == 3)
    {
        STAFObjectPtr outerMap = STAFObject::createMap();

        outerMap->put("SomeMap", fMapOfLists->reference());
        outerMap->put("SomeList", fListOfMaps->reference());

        mc->setRootObject(outerMap);
    }
    else if (dataSet == 4)
    {
        // Dataset 4 is a nice little nested structure like [[{}], {[]}]

        STAFObjectPtr outerList = STAFObject::createList();

        outerList->append(fListOfMaps->reference());
        outerList->append(fMapOfLists->reference());

        mc->setRootObject(outerList);
    }
    else if (dataSet == 5)
    {
        // Dataset 5 is a simple list containing a nested marshalling context
        // containing no map class definition, and another containing a map
        // class definition

        // Create the list and add a couple of strings

        STAFObjectPtr theList = STAFObject::createList();

        theList->append("String1");
        theList->append("String2");

        // Create the nested MC without any map class definitions

        STAFObjectPtr nestedMC1 = STAFObject::createMarshallingContext();
        STAFObjectPtr innerMap1 = STAFObject::createMap();

        innerMap1->put("key1", "value1");
        innerMap1->put("key2", "value2");

        nestedMC1->setRootObject(innerMap1);

        theList->append(nestedMC1);

        // Create the nested MC with the map class definition

        STAFObjectPtr nestedMC2 = STAFObject::createMarshallingContext();

        // Create the map class definition

        STAFMapClassDefinitionPtr mapClass =
            STAFMapClassDefinition::create("STAF/Service/Misc/TrivialMap");

        mapClass->addKey("key1", "Key 1");
        mapClass->addKey("key2", "Key 2");

        nestedMC2->setMapClassDefinition(mapClass);

        // Now, create the map using the definition

        STAFObjectPtr innerMap2 = STAFObject::createMap();

        innerMap2->put(sSTAFMapClassName, "STAF/Service/Misc/TrivialMap");
        innerMap2->put("key1", "value1");
        innerMap2->put("key2", "value2");

        nestedMC2->setRootObject(innerMap2);

        theList->append(nestedMC2);

        // Finally, add a couple more simple strings

        theList->append("String3");
        theList->append("String4");

        mc->setRootObject(theList);
    }
    else if (dataSet == 6)
    {
        // Dataset 6 is a simple list of strins where one of the strings in the 
        // list is the string from marshalling a simple map.

        STAFObjectPtr theList = STAFObject::createList();
        STAFObjectPtr aMap = STAFObject::createMap();

        aMap->put("key1", "value1");
        aMap->put("key2", "value2");

        theList->append("String1");
        theList->append("String2");
        theList->append(aMap->marshall());
        theList->append("String3");
        theList->append("String4");

        mc->setRootObject(theList);
    }
    else if (dataSet == 7)
    {
        // Dataset 6 is a list of map class instances (which should normally
        // print in table mode).  However, some of the entries are structures
        // instead of strings.

        // Create a couple dummy structures

        STAFObjectPtr aList = STAFObject::createList();
        STAFObjectPtr aMap = STAFObject::createMap();

        aMap->put("key1", "value1");
        aMap->put("key2", "value2");

        aList->append("String1");
        aList->append("String2");
        aList->append("String3");

        // Now populate the output list with some records

        mc->setMapClassDefinition(fLogRecordClass->reference());

        STAFObjectPtr theList = STAFObject::createList();
        STAFObjectPtr logRecord1 = STAFObject::createMap();

        logRecord1->put(sSTAFMapClassName, sLogRecord);
        logRecord1->put(sLevel, sInfo);
        logRecord1->put(sMessage, aMap);
        logRecord1->put(sMachine, sSystem);
        logRecord1->put(sTimestamp, STAFTimestamp::now().asString());

        theList->append(logRecord1);

        STAFObjectPtr logRecord2 = STAFObject::createMap();

        logRecord2->put(sSTAFMapClassName, sLogRecord);
        logRecord2->put(sLevel, sInfo);
        logRecord2->put(sMessage, aList);
        logRecord2->put(sMachine, sSystem);
        logRecord2->put(sTimestamp, STAFTimestamp::now().asString());

        theList->append(logRecord2);

        STAFObjectPtr logRecord3 = STAFObject::createMap();

        logRecord3->put(sSTAFMapClassName, sLogRecord);
        logRecord3->put(sLevel, sInfo);
        logRecord3->put(sMessage, aMap);
        logRecord3->put(sMachine, sSystem);
        logRecord3->put(sTimestamp, STAFTimestamp::now().asString());

        STAFObjectPtr logRecord4 = STAFObject::createMap();

        logRecord4->put(sSTAFMapClassName, sLogRecord);
        logRecord4->put(sLevel, sInfo);
        logRecord4->put(sMessage, logRecord3);
        logRecord4->put(sMachine, sSystem);
        logRecord4->put(sTimestamp, STAFTimestamp::now().asString());

        theList->append(logRecord4);

        STAFObjectPtr logRecord5 = STAFObject::createMap();

        logRecord5->put(sSTAFMapClassName, sLogRecord);
        logRecord5->put(sLevel, sInfo);
        logRecord5->put(sMessage, "A simple string message");
        logRecord5->put(sMachine, sSystem);
        logRecord5->put(sTimestamp, STAFTimestamp::now().asString());

        theList->append(logRecord5);

        mc->setRootObject(theList);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFMiscService::handleDebug(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "DEBUG");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fDebugParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    #if defined(STAF_OS_TYPE_WIN32) && defined(_DEBUG)

    if (parsedResult->optionTimes("CHECKPOINT") > 0)
    {
        _CrtMemCheckpoint(&fFromMemState);
    }
    else if (parsedResult->optionTimes("DUMP") > 0)
    {
        if (parsedResult->optionTimes("ALL") > 0)
            _CrtMemDumpAllObjectsSince(0);
        else
            _CrtMemDumpAllObjectsSince(&fFromMemState);
    }
    else if (parsedResult->optionTimes("STATS") > 0)
    {
        _CrtMemDumpStatistics(&fFromMemState);
    }
    else if (parsedResult->optionTimes("STATSDIFF") > 0)
    {
        _CrtMemState nowMemState;
        _CrtMemState deltaMemState;

        _CrtMemCheckpoint(&nowMemState);
        _CrtMemDifference(&deltaMemState, &fFromMemState, &nowMemState);

        _CrtMemDumpStatistics(&deltaMemState);
    }
    else if (parsedResult->optionTimes("LEAKS") > 0)
    {
        _CrtDumpMemoryLeaks();
    }

    #endif

    return kSTAFOk;
}

STAFServiceResult STAFMiscService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}


STAFString STAFMiscService::info(unsigned int) const
{
    return name() + ": Internal";
}

STAFMiscService::~STAFMiscService()
{
    /* Do Nothing */
}
