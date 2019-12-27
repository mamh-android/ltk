/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_fstream.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFLifeCycleService.h"
#include "STAFCommandParser.h"
#include "STAFFileSystem.h"
#include "STAFTrace.h"
#include <algorithm>

static const unsigned int sCurrFileFormatID = 2;
static const STAFString sStartup = "Startup";
static const STAFString sShutdown = "Shutdown";
static const STAFString sEnabled = "Enabled";
static const STAFString sDisabled = "Disabled";
static const STAFString sTrue = "True";
static const STAFString sFalse = "False";
static const STAFString sLocal = "local";
static const STAFString sLog = "LOG";

static STAFString sHelpMsg;

enum RegistrationPhase { kStartup, kShutdown };
enum RegistrationState { kEnabled, kDisabled };
enum RegistrationOnce  { kFalse, kTrue};
    
typedef unsigned int RegistrationID_t;

struct RegistrationData
{
    RegistrationData()
    { /* Do nothing */ }

    RegistrationData(RegistrationID_t aID,
                     const RegistrationPhase &aPhase,
                     const unsigned int aPriority,
                     const RegistrationOnce &aOnce,
                     const STAFString &aMachine,
                     const STAFString &aService,
                     const STAFString &aRequest,
                     const STAFString &aDescription)
        : id(aID), phase(aPhase), priority(aPriority), once(aOnce),
          machine(aMachine), service(aService), request(aRequest),
          description(aDescription)
    {
        state = kEnabled;
    }

    RegistrationID_t id;
    RegistrationPhase phase;
    unsigned int priority;
    RegistrationState state;
    RegistrationOnce once;
    STAFString machine;
    STAFString service;
    STAFString request;
    STAFString description;
};

typedef std::map<RegistrationID_t, RegistrationData> RegistrationMap;
typedef std::vector<RegistrationData> RegDataVector;

RegistrationID_t fNextRegistrationID;
STAFString       fRegistrationFileName;
RegistrationMap  fRegistrationMap;
unsigned int     fNumRegistrations;

// Sort function for listing/triggering registrations which sorts in
// ascending order by phase, by priority, and then by registration ID.

static bool SortAscendingByPhasePriorityID(
    RegistrationData lhs, RegistrationData rhs)
{
    if (lhs.phase == rhs.phase)
        if (lhs.priority == rhs.priority)
            return (lhs.id < rhs.id);
        else
            return (lhs.priority < rhs.priority);
    else
        return (lhs.phase < rhs.phase);
}

static STAFString getPhaseAsString(RegistrationPhase phase)
{
    STAFString phaseString;

    if (phase == kStartup)
        phaseString = sStartup;    
    else
        phaseString = sShutdown;

    return phaseString;
}

static STAFString getStateAsString(RegistrationState state)
{
    STAFString stateString;

    if (state == kEnabled)
        stateString = sEnabled;    
    else
        stateString = sDisabled;

    return stateString;
}
    
static STAFString getOnceAsString(RegistrationOnce once)
{
    STAFString onceString;

    if (once == kTrue)
        onceString = sTrue;    
    else
        onceString = sFalse;

    return onceString;
}

static void logMessage(const STAFString &message,
                       const STAFString &level="Info")
{
    gSTAFProcHandlePtr->submit(
        sLocal, sLog, STAFString("LOG MACHINE LOGNAME LIFECYCLE LEVEL ") +
        level + " MESSAGE " + STAFHandle::wrapData(message));
}

void readUIntFromFile(istream &input, unsigned int &data,
                      unsigned int length = 4);

void writeUIntToFile(ostream &output, unsigned int data,
                     unsigned int length = 4);

void readStringFromFile(istream &input, STAFString &inString);

void writeStringToFile(ostream &output, STAFString &outString);

STAFRC_t readRegistrationFile(RegistrationMap &regMap,
                              STAFString &errorBuffer);

STAFServiceResult saveRegistrationData(RegistrationMap &regMap);


STAFLifeCycleService::STAFLifeCycleService() : STAFService("LIFECYCLE")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** LifeCycle Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "REGISTER   PHASE <Startup | Shutdown>" +
        *gLineSeparatorPtr +
        "           MACHINE <Machine> SERVICE <Service> REQUEST <Request>" +
        *gLineSeparatorPtr +
        "           [ONCE] [PRIORITY <Priority>] [DESCRIPTION <Description>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "UNREGISTER ID <Registration ID>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "UPDATE     ID <Registration ID> [PRIORITY <Priority>] [ONCE <True | False>]" +
        *gLineSeparatorPtr +
        "           [MACHINE <Machine>] [SERVICE <Service>] [REQUEST <Request>]" +
        *gLineSeparatorPtr +
        "           [PHASE <Startup | Shutdown>] [DESCRIPTION <Description>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST       [PHASE <Startup | Shutdown>] [LONG]" + 
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "QUERY      ID <Registration ID>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "TRIGGER    <ID <Registration ID> | PHASE <Startup | Shutdown>> CONFIRM" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "ENABLE     ID <Registration ID>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DISABLE    ID <Registration ID>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP" ;

    // Set options, groups, and needs for the Register parser
 
    fRegisterParser.addOption(
        "REGISTER", 1, STAFCommandParser::kValueNotAllowed);
    fRegisterParser.addOption(
        "PHASE", 1, STAFCommandParser::kValueRequired);
    fRegisterParser.addOption(
        "MACHINE", 1, STAFCommandParser::kValueRequired);
    fRegisterParser.addOption(
        "SERVICE", 1, STAFCommandParser::kValueRequired);
    fRegisterParser.addOption(
        "REQUEST", 1, STAFCommandParser::kValueRequired);
    fRegisterParser.addOption(
        "ONCE", 1, STAFCommandParser::kValueNotAllowed);
    fRegisterParser.addOption(
        "PRIORITY", 1, STAFCommandParser::kValueRequired);
    fRegisterParser.addOption(
        "DESCRIPTION", 1, STAFCommandParser::kValueRequired);
 
    fRegisterParser.addOptionNeed("REGISTER", "PHASE");
    fRegisterParser.addOptionNeed("REGISTER", "MACHINE");
    fRegisterParser.addOptionNeed("REGISTER", "SERVICE");
    fRegisterParser.addOptionNeed("REGISTER", "REQUEST");
 
    // Set options, groups, and needs for the Unregister parser

    fUnregisterParser.addOption(
        "UNREGISTER", 1, STAFCommandParser::kValueNotAllowed);
    fUnregisterParser.addOption(
        "ID", 1, STAFCommandParser::kValueRequired);

    fUnregisterParser.addOptionNeed("UNREGISTER", "ID");
    
    // Set options, groups, and needs for the Update parser

    fUpdateParser.addOption(
        "UPDATE", 1, STAFCommandParser::kValueNotAllowed);
    fUpdateParser.addOption(
        "ID", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "PRIORITY", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "MACHINE", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "SERVICE", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "REQUEST", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "PHASE", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "ONCE", 1, STAFCommandParser::kValueRequired);
    fUpdateParser.addOption(
        "DESCRIPTION", 1, STAFCommandParser::kValueRequired);
 
    fUpdateParser.addOptionNeed("UPDATE", "ID");
    fUpdateParser.addOptionNeed(
        "ID", "PRIORITY MACHINE SERVICE REQUEST PHASE ONCE DESCRIPTION");
    
    // Set options, groups, and needs for the List parser

    fListParser.addOption(
        "LIST", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "PHASE", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption(
        "LONG", 1, STAFCommandParser::kValueNotAllowed);
 
    // Set options, groups, and needs for the Query parser
 
    fQueryParser.addOption(
        "QUERY",  1, STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption(
        "ID", 1, STAFCommandParser::kValueRequired);
 
    fQueryParser.addOptionNeed("QUERY", "ID");

    // Set options, groups, and needs for the Trigger parser

    fTriggerParser.addOption(
        "TRIGGER",  1, STAFCommandParser::kValueNotAllowed);
    fTriggerParser.addOption(
        "ID", 1, STAFCommandParser::kValueRequired);
    fTriggerParser.addOption(
        "PHASE", 1, STAFCommandParser::kValueRequired);
    fTriggerParser.addOption(
        "CONFIRM", 1, STAFCommandParser::kValueNotAllowed);

    fTriggerParser.addOptionGroup("ID PHASE", 0, 1);
    fTriggerParser.addOptionNeed("TRIGGER", "ID PHASE");
    fTriggerParser.addOptionNeed("TRIGGER", "CONFIRM");

    // Set options, groups, and needs for the Enable parser
    
    fEnableParser.addOption(
        "ENABLE", 1, STAFCommandParser::kValueNotAllowed);
    fEnableParser.addOption(
        "ID", 1, STAFCommandParser::kValueRequired);
 
    fEnableParser.addOptionNeed("ENABLE", "ID");

    // Set options, groups, and needs for the Disable parser

    fDisableParser.addOption(
        "DISABLE", 1, STAFCommandParser::kValueNotAllowed);
    fDisableParser.addOption(
        "ID", 1, STAFCommandParser::kValueRequired);
 
    fDisableParser.addOptionNeed("DISABLE", "ID");

    // Construct map class for listing registrations

    fRegMapClass = STAFMapClassDefinition::create(
        "STAF/Service/LifeCycle/Reg");
    fRegMapClass->addKey("phase", "Phase");
    fRegMapClass->addKey("priority", "Priority");
    fRegMapClass->setKeyProperty("priority", "display-short-name", "P");
    fRegMapClass->addKey("id", "ID");
    fRegMapClass->addKey("state", "State");
    fRegMapClass->addKey("machine", "Machine");
    fRegMapClass->addKey("service", "Service");
    fRegMapClass->addKey("request", "Request");
    fRegMapClass->addKey("once", "Once");

    // Construct map class for listing registrations using the LONG option

    fRegDetailsMapClass = STAFMapClassDefinition::create(
        "STAF/Service/LifeCycle/RegDetails");
    fRegDetailsMapClass->addKey("phase", "Phase");
    fRegDetailsMapClass->addKey("priority", "Priority");
    fRegDetailsMapClass->setKeyProperty(
        "priority", "display-short-name", "P");
    fRegDetailsMapClass->addKey("id", "ID");
    fRegDetailsMapClass->addKey("state", "State");
    fRegDetailsMapClass->addKey("machine", "Machine");
    fRegDetailsMapClass->addKey("service", "Service");
    fRegDetailsMapClass->addKey("request", "Request");
    fRegDetailsMapClass->addKey("description", "Description");
    fRegDetailsMapClass->setKeyProperty(
        "description", "display-short-name", "Desc");
    fRegDetailsMapClass->addKey("once", "Once");

    // Construct map class for querying a registration ID

    fRegQueryMapClass = STAFMapClassDefinition::create(
        "STAF/Service/LifeCycle/RegQuery");
    fRegQueryMapClass->addKey("phase", "Phase");
    fRegQueryMapClass->addKey("priority", "Priority");
    fRegQueryMapClass->addKey("state", "State");
    fRegQueryMapClass->addKey("machine", "Machine");
    fRegQueryMapClass->addKey("service", "Service");
    fRegQueryMapClass->addKey("request", "Request");
    fRegQueryMapClass->addKey("description", "Description");
    fRegQueryMapClass->addKey("once", "Once");

    // Construct map class for the result from a TRIGGER ID request

    fTriggerIdMapClass = STAFMapClassDefinition::create(
        "STAF/Service/LifeCycle/TriggerId");
    fTriggerIdMapClass->addKey("machine", "Machine");
    fTriggerIdMapClass->addKey("service", "Service");
    fTriggerIdMapClass->addKey("request", "Request");
    fTriggerIdMapClass->addKey("rc", "RC");
    fTriggerIdMapClass->addKey("result", "Result");

    // Construct map class for a triggers info

    fTriggerIdsMapClass = STAFMapClassDefinition::create(
        "STAF/Service/LifeCycle/TriggerIds");
    fTriggerIdsMapClass->addKey("id", "ID");
    fTriggerIdsMapClass->addKey("machine", "Machine");
    fTriggerIdsMapClass->addKey("service", "Service");
    fTriggerIdsMapClass->addKey("request", "Request");
    fTriggerIdsMapClass->addKey("rc", "RC");
    fTriggerIdsMapClass->addKey("result", "Result");

    // Store persistent data for the LifeCycle service in directory:
    //    <STAF writeLocation>/service/lifecycle/registration.data

    STAFFSPath servicePath;
    servicePath.setRoot(*gSTAFWriteLocationPtr);
    servicePath.addDir("service");
    servicePath.addDir(name().toLowerCase());

    // Create the service data directory if it does't already exist

    try
    {
        if (!servicePath.exists())
        {
            servicePath.createDirectory(kSTAFFSCreatePath);
        }
    }
    catch (...)
    {
        // Don't want any exceptions here
    }

    if (!servicePath.exists())
    {
        cout << endl << "LifeCycle Service Initialization Error:  " << endl
             << "Cannot create directory " << servicePath.asString() << endl;
    }

    fRegistrationFileName = servicePath.asString() + *gFileSeparatorPtr +
        "lifecycle.reg";
    
    // Read the registration data file and load the registrations into the
    // Registration Map

    STAFString errorBuffer;

    STAFRC_t rc = readRegistrationFile(fRegistrationMap, errorBuffer);

    if (rc != kSTAFOk)
    {
        cout << endl << "LifeCycle Service Initialization Error:  " << endl
             << "Cannot read registration file " << fRegistrationFileName
             << endl << errorBuffer << endl;
    }

    // Determine what the next registration ID should be based on the
    // ID with the highest number in the registration map

    if (fRegistrationMap.size() == 0)
    {
        fNextRegistrationID = 1;
    }
    else
    {
        fNextRegistrationID = 1;

        RegistrationMap::iterator iter;

        for (iter = fRegistrationMap.begin(); iter != fRegistrationMap.end();
             iter++)
        {
            if (iter->second.id > fNextRegistrationID)
            {
                fNextRegistrationID = iter->second.id;
            }
        }

        fNextRegistrationID++;
    }

    // Assign the STAF service machine log name for the service

    fServiceLogName = name();
}


STAFLifeCycleService::~STAFLifeCycleService()
{
    /* Do Nothing */
}


STAFString STAFLifeCycleService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFLifeCycleService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();
 
    if (action == "register")
        return handleRegister(requestInfo);
    else if (action == "unregister")
        return handleUnregister(requestInfo);
    else if (action == "update")
        return handleUpdate(requestInfo);
    else if (action == "list")
        return handleList(requestInfo);
    else if (action == "query")
        return handleQuery(requestInfo);
    else if (action == "trigger")
        return handleTrigger(requestInfo);
    else if (action == "enable")
        return handleEnable(requestInfo);
    else if (action == "disable")
        return handleDisable(requestInfo);
    else if (action == "help") 
        return handleHelp(requestInfo);
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


STAFServiceResult STAFLifeCycleService::handleRegister(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "REGISTER");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fRegisterParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in values for PHASE, MACHINE,
    // SERVICE, REQUEST, and, if specified, PRIORITY.
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString phaseString;
    RegistrationPhase phase;
    STAFString machine;
    STAFString service;
    STAFString request;
    unsigned int priority = 50;
    RegistrationOnce once = kFalse;
    STAFString description = STAFString("");

    STAFRC_t rc = RESOLVE_STRING_OPTION("PHASE", phaseString);

    if (rc) return STAFServiceResult(rc, errorBuffer);
    
    // Verify that phase is either "Startup" or "Shutdown",
    // not case-sensitive

    if (phaseString.isEqualTo(sStartup, kSTAFStringCaseInsensitive))
    {
        phase = kStartup;
    }
    else if (phaseString.isEqualTo(sShutdown, kSTAFStringCaseInsensitive))
    {
        phase = kShutdown;
    }
    else
    {
        errorBuffer = STAFString(
            "The value for the PHASE option must be Startup or Shutdown.  "
            "Invalid value: ") + phaseString;
        return STAFServiceResult(kSTAFInvalidValue, errorBuffer);
    }

    // Don't resolve variables in the MACHINE, SERVICE, and REQUEST option
    // values so that they will be resolved when the STAF service request is
    // submitted (instead of when the REGISTER request is submitted).

    machine = parsedResult->optionValue("MACHINE");
    service = parsedResult->optionValue("SERVICE");
    request = parsedResult->optionValue("REQUEST");
    
    if (parsedResult->optionTimes("PRIORITY") > 0)
    {
        rc = RESOLVE_UINT_OPTION_RANGE("PRIORITY", priority, 1, 99);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }

    if (parsedResult->optionTimes("ONCE") > 0)
    {
        once = kTrue;
    }

    if (parsedResult->optionTimes("DESCRIPTION") > 0)
    {
        description = parsedResult->optionValue("DESCRIPTION");
    }

    unsigned int regID = 0;

    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);

        // Create a copy of the registrations map to update

        RegistrationMap newRegMap = fRegistrationMap;

        // Add the registration to the new registrations map

        RegistrationData regData(fNextRegistrationID++, phase, priority, once,
                                 machine, service, request, description);

        newRegMap[regData.id] = regData;
    
        regID = regData.id;

        // Save the updated registration map in the registration file
        // and in memory

        STAFServiceResult result = saveRegistrationData(newRegMap);

        if (result.fRC != kSTAFOk)
        {
            return result;
        }
    }

    // Log a message in the service log

    STAFString message = STAFString("[ID=") + regID + "] [" +
        requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
        requestInfo.fHandle + "] Register request: " + requestInfo.fRequest;

    logMessage(message);
    
    // Return the registration ID

    return STAFServiceResult(kSTAFOk, STAFString(regID));
}


STAFServiceResult STAFLifeCycleService::handleUnregister(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "UNREGISTER");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fUnregisterParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in the value for ID
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    RegistrationID_t id;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("ID", id, 1, UINT_MAX);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);

        // Check if the registration ID exists

        if (fRegistrationMap.find(id) == fRegistrationMap.end())
        {
            errorBuffer = STAFString("ID ") + id + " does not exist";

            return STAFServiceResult(kSTAFDoesNotExist, errorBuffer);
        }
        
        // Create a copy of the registrations map to update

        RegistrationMap newRegMap = fRegistrationMap;

        // Remove the registration ID from the map

        newRegMap.erase(id);
        
        // Save the updated registration map in the registration file
        // and in memory

        STAFServiceResult result = saveRegistrationData(newRegMap);

        if (result.fRC != kSTAFOk)
        {
            return result;
        }

        // Update the fNextRegistrationID field the registration with the
        // highest ID number was just deleted so the ID can be reused.

        if (fRegistrationMap.size() == 0)
        {
            fNextRegistrationID = 1;
        }
        else if (id == (fNextRegistrationID - 1))
        {
            fNextRegistrationID--;
        }
    }
    
    // Log a message in the service log

    STAFString message = STAFString("[ID=") + id + "] [" +
        requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
        requestInfo.fHandle + "] Unregistered.";

    logMessage(message);
    
    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFLifeCycleService::handleUpdate(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "UPDATE");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fUpdateParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in values for ID, and if
    // specified, for PRIORITY, ONCE, MACHINE, SERVICE, REQUEST, and PHASE.
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    RegistrationID_t id;

    unsigned int priority;
    bool updatePriority = false;

    RegistrationOnce once;
    bool updateOnce = false;

    STAFString machine;
    bool updateMachine = false;

    STAFString service;
    bool updateService = false;

    STAFString request;
    bool updateRequest = false;

    RegistrationPhase phase;
    bool updatePhase = false;

    STAFString description;
    bool updateDescription = false;
    
    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("ID", id, 1, UINT_MAX);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (parsedResult->optionTimes("PRIORITY") > 0)
    {
        rc = RESOLVE_UINT_OPTION_RANGE("PRIORITY", priority, 1, 99);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Set flag to update the priority field
        updatePriority = true;
    }

    if (parsedResult->optionTimes("ONCE") > 0)
    {
        STAFString onceString;
        rc = RESOLVE_STRING_OPTION("ONCE", onceString);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    
        // Verify that once is either "True" or "False",
        // not case-sensitive

        if (onceString.isEqualTo(sTrue, kSTAFStringCaseInsensitive))
        {
            once = kTrue;
        }
        else if (onceString.isEqualTo(sFalse, kSTAFStringCaseInsensitive))
        {
            once = kFalse;
        }
        else
        {
            errorBuffer = STAFString(
                "The value for the ONCE option must be " + sTrue + " or " +
                sFalse + ".  Invalid value: ") + onceString;
            return STAFServiceResult(kSTAFInvalidValue, errorBuffer);
        }

        // Set flag to update the once field
        updateOnce = true;
    }

    if (parsedResult->optionTimes("MACHINE") > 0)
    {
        // Don't resolve variables in the MACHINE, SERVICE, and REQUEST option
        // values so that they will be resolved when the STAF service request
        // is submitted (instead of when the REGISTER request is submitted).

        machine = parsedResult->optionValue("MACHINE");

        // Set flag to update the priority field
        updateMachine = true;
    }

    if (parsedResult->optionTimes("SERVICE") > 0)
    {
        // Don't resolve variables in the MACHINE, SERVICE, and REQUEST option
        // values so that they will be resolved when the STAF service request
        // is submitted (instead of when the REGISTER request is submitted).

        service = parsedResult->optionValue("SERVICE");

        // Set flag to update the service field
        updateService = true;
    }
    
    if (parsedResult->optionTimes("REQUEST") > 0)
    {
        // Don't resolve variables in the MACHINE, SERVICE, and REQUEST option
        // values so that they will be resolved when the STAF service request
        // is submitted (instead of when the REGISTER request is submitted).

        request = parsedResult->optionValue("REQUEST");

        // Set flag to update the request field
        updateRequest = true;
    }

    if (parsedResult->optionTimes("PHASE") > 0)
    {
        STAFString phaseString;
        rc = RESOLVE_STRING_OPTION("PHASE", phaseString);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    
        // Verify that phase is either "Startup" or "Shutdown",
        // not case-sensitive

        if (phaseString.isEqualTo(sStartup, kSTAFStringCaseInsensitive))
        {
            phase = kStartup;
        }
        else if (phaseString.isEqualTo(sShutdown, kSTAFStringCaseInsensitive))
        {
            phase = kShutdown;
        }
        else
        {
            errorBuffer = STAFString(
                "The value for the PHASE option must be Startup or Shutdown.  "
                "Invalid value: ") + phaseString;
            return STAFServiceResult(kSTAFInvalidValue, errorBuffer);
        }

        // Set flag to update the phase field
        updatePhase = true;
    }

    if (parsedResult->optionTimes("DESCRIPTION") > 0)
    {
        description = parsedResult->optionValue("DESCRIPTION");

        // Set flag to update the description field
        updateDescription = true;
    }
    
    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);

        // Check if the ID exists in the registration map.

        if (fRegistrationMap.find(id) == fRegistrationMap.end())
        {
            errorBuffer = STAFString("ID ") + id + " does not exist";

            return STAFServiceResult(kSTAFDoesNotExist, errorBuffer);
        }

        // Create a copy of the registrations map to update

        RegistrationMap newRegMap = fRegistrationMap;

        // Update the registration

        RegistrationData &regData = newRegMap[id];

        if (updatePriority)
            regData.priority = priority;

        if (updateOnce)
            regData.once = once;
    
        if (updateMachine)
            regData.machine = machine;

        if (updateService)
            regData.service = service;

        if (updateRequest)
            regData.request = request;

        if (updatePhase)
            regData.phase = phase;
    
        if (updateDescription)
            regData.description = description;
        
        // Save the updated registration map in the registration file
        // and in memory

        STAFServiceResult result = saveRegistrationData(newRegMap);

        if (result.fRC != kSTAFOk)
        {
            return result;
        }
    }
    
    // Log a message in the service log

    STAFString message = STAFString("[ID=") + id + "] [" +
        requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
        requestInfo.fHandle + "] Update request: " + requestInfo.fRequest;

    logMessage(message);

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFLifeCycleService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");
    
    STAFCommandParseResultPtr parsedResult = 
        fListParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Check if the PHASE option was specified
    
    bool listPhase = false;
    RegistrationPhase phase = kStartup;

    if (parsedResult->optionTimes("PHASE") > 0)
    {
        // List the registrations for the specified phase

        listPhase = true;

        // Resolve any STAF variables specified in the value for PHASE

        DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
        STAFString errorBuffer;
        STAFString phaseString;
        STAFRC_t rc = RESOLVE_STRING_OPTION("PHASE", phaseString);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    
        // Verify that phase is either "Startup" or "Shutdown",
        // not case-sensitive

        if (phaseString.isEqualTo(sStartup, kSTAFStringCaseInsensitive))
        {
            phase = kStartup;
        }
        else if (phaseString.isEqualTo(sShutdown, kSTAFStringCaseInsensitive))
        {
            phase = kShutdown;
        }
        else
        {
            errorBuffer = STAFString(
                "The value for the PHASE option must be Startup or Shutdown.  "
                "Invalid value: ") + phaseString;
            return STAFServiceResult(kSTAFInvalidValue, errorBuffer);
        }
    }

    // Check if the LONG option was specified

    bool listLong = (parsedResult->optionTimes("LONG") != 0);

    // Create a vector containing the matching registrations from the
    // registration map

    RegDataVector regDataVector;

    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);
        RegistrationMap::iterator iter;

        for (iter = fRegistrationMap.begin(); iter != fRegistrationMap.end();
             iter++)
        {
            // If listing registrations for a phase, skip any registrations
            // that don't match the specified phase

            if (listPhase && (phase != iter->second.phase))
                continue;

            regDataVector.push_back(iter->second);
        }
    }

    // Sort the regDataVector in ascending order by phase, priority, and ID

    std::sort(regDataVector.begin(), regDataVector.end(),
              SortAscendingByPhasePriorityID);

    // Create a marshalling context and set its map class definition and
    // create an empty list object to contain the result.

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (listLong)
        mc->setMapClassDefinition(fRegDetailsMapClass->reference());
    else
        mc->setMapClassDefinition(fRegMapClass->reference());
    
    STAFObjectPtr outputList = STAFObject::createList();

    // Iterate through the sorted registration data vector and generate
    // the result
    
    RegDataVector::iterator iter;

    for (iter = regDataVector.begin(); iter != regDataVector.end(); iter++)
    {
        // Add the entry to the outputList

        STAFObjectPtr regDataMap;

        if (listLong)
        {
            regDataMap = fRegDetailsMapClass->createInstance();
            regDataMap->put("phase", getPhaseAsString(iter->phase));
            regDataMap->put("priority", iter->priority);
            regDataMap->put("id", iter->id);
            regDataMap->put("state", getStateAsString(iter->state));
            regDataMap->put("machine", iter->machine);
            regDataMap->put("service", iter->service);
            regDataMap->put("request",
                            STAFHandle::maskPrivateData(iter->request));

            if (iter->description.length() == 0)
                regDataMap->put("description", STAFObject::createNone());
            else
                regDataMap->put("description", iter->description);

            regDataMap->put("once", getOnceAsString(iter->once));
        }
        else
        {
            regDataMap = fRegMapClass->createInstance();
            regDataMap->put("phase", getPhaseAsString(iter->phase));
            regDataMap->put("priority", iter->priority);
            regDataMap->put("id", iter->id);
            regDataMap->put("state", getStateAsString(iter->state));
            regDataMap->put("machine", iter->machine);
            regDataMap->put("service", iter->service);
            regDataMap->put("request",
                            STAFHandle::maskPrivateData(iter->request));
            regDataMap->put("once", getOnceAsString(iter->once));
        }

        outputList->append(regDataMap);
    }

    mc->setRootObject(outputList);
    
    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFLifeCycleService::handleQuery(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "QUERY");
    
    STAFCommandParseResultPtr parsedResult = 
        fQueryParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in values for ID
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    RegistrationID_t id;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("ID", id, 1, UINT_MAX);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    mc->setMapClassDefinition(fRegQueryMapClass->reference());

    RegistrationData regData;

    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);

        // Check if the registration ID exists

        if (fRegistrationMap.find(id) == fRegistrationMap.end())
        {
            errorBuffer = STAFString("ID ") + id + " does not exist";

            return STAFServiceResult(kSTAFDoesNotExist, errorBuffer);
        }

        regData = fRegistrationMap[id];
    }
    
    // Create a marshalled map containing the registration data

    STAFObjectPtr regDataMap = fRegQueryMapClass->createInstance();

    regDataMap->put("phase", getPhaseAsString(regData.phase));
    regDataMap->put("priority", regData.priority);
    regDataMap->put("state", getStateAsString(regData.state));
    regDataMap->put("machine", regData.machine);
    regDataMap->put("service", regData.service);
    regDataMap->put("request", STAFHandle::maskPrivateData(regData.request));

    if (regData.description.length() == 0)
        regDataMap->put("description", STAFObject::createNone());
    else
        regDataMap->put("description", regData.description);

    regDataMap->put("once", getOnceAsString(regData.once));

    mc->setRootObject(regDataMap);
    
    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFLifeCycleService::handleTrigger(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "TRIGGER");
 
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fTriggerParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in values for ID or PHASE

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;

    if (parsedResult->optionTimes("ID") > 0)
    {
        RegistrationID_t id;
        
        STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("ID", id, 1, UINT_MAX);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        RegistrationData regData;

        // Retain a lock on the registration map only in this block
        {
            STAFMutexSemLock registrationLock(fRegistrationMapSem);

            // Check if the registration ID exists

            if (fRegistrationMap.find(id) == fRegistrationMap.end())
            {
                errorBuffer = STAFString("ID ") + id + " does not exist";

                return STAFServiceResult(kSTAFDoesNotExist, errorBuffer);
            }

            regData = fRegistrationMap[id];
        }
        
        // Log a message in the service log

        STAFString message = STAFString("[ID=") + id + "] [" +
            requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
            requestInfo.fHandle + "] [TRIGGER ID] Submitted: STAF " +
            regData.machine + " " + regData.service + " " + regData.request;

        logMessage(message);

        // Trigger the registration by submitting a synchronous request
        // to run the specified request by the specified service on the
        // specified machine

        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            regData.machine, regData.service, regData.request);
        
        // Log a message in the service log with the results 

        message = STAFString("[ID=") + id + "] [" +
            requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
            requestInfo.fHandle + "] [TRIGGER ID] Completed. RC=" +
            result->rc + ", Result=" +
            result->resultContext->asFormattedString();

        logMessage(message);

        // Return a marshalled map of the results

        STAFObjectPtr mc = STAFObject::createMarshallingContext();

        mc->setMapClassDefinition(fTriggerIdMapClass->reference());

        STAFObjectPtr resultMap = fTriggerIdMapClass->createInstance();

        resultMap->put("machine", regData.machine);
        resultMap->put("service", regData.service);
        resultMap->put("request",
                       STAFHandle::maskPrivateData(regData.request));
        resultMap->put("rc", result->rc);
        resultMap->put("result", result->result);
        
        mc->setRootObject(resultMap);
        
        return STAFServiceResult(kSTAFOk, mc->marshall());
    }

    // Trigger the registrations for the specified phase

    RegistrationPhase phase = kStartup;

    // Resolve any STAF variables specified in the value for PHASE

    STAFString phaseString;
    STAFRC_t rc = RESOLVE_STRING_OPTION("PHASE", phaseString);

    if (rc) return STAFServiceResult(rc, errorBuffer);
    
    // Verify that phase is either "Startup" or "Shutdown",
    // not case-sensitive

    if (phaseString.isEqualTo(sStartup, kSTAFStringCaseInsensitive))
    {
        phase = kStartup;
    }
    else if (phaseString.isEqualTo(sShutdown, kSTAFStringCaseInsensitive))
    {
        phase = kShutdown;
    }
    else
    {
        errorBuffer = STAFString(
            "The value for the PHASE option must be Startup or Shutdown.  "
            "Invalid value: ") + phaseString;
        return STAFServiceResult(kSTAFInvalidValue, errorBuffer);
    }
    
    // Create a vector containing only the registrations from the
    // registration map that match the specified phase.

    RegDataVector regDataVector;

    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);
        RegistrationMap::iterator iter;

        for (iter = fRegistrationMap.begin(); iter != fRegistrationMap.end();
             iter++)
        {
            // Skip any registrations that don't match the specified phase

            if (phase != iter->second.phase) continue;

            regDataVector.push_back(iter->second);
        }
    }

    // Sort the regDataVector in ascending order by phase, priority, and ID

    std::sort(regDataVector.begin(), regDataVector.end(),
              SortAscendingByPhasePriorityID);

    // Create a marshalling context the assign the map class definition and
    // create an empty list to contain the result

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fTriggerIdsMapClass->reference());
    STAFObjectPtr outputList = STAFObject::createList();

    // Iterate through the sorted registration data vector and trigger the
    // enabled registrations and generate the result
    
    RegDataVector::iterator iter;
    bool triggered = false;

    for (iter = regDataVector.begin(); iter != regDataVector.end(); iter++)
    {
        // If the registration is disabled, don't trigger it and log a
        // message about it not being triggered.
        
        if (iter->state != kEnabled)
        {
            // Log a message about not triggering a disabled registration

            STAFString message = STAFString("[ID=") + iter->id + "] [" +
                requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
                requestInfo.fHandle + "] [TRIGGER " + getPhaseAsString(phase) +
                "] ID is disabled. STAF service request not submitted.";

            logMessage(message);

            continue;
        }

        // If triggered by STAFProc (handle is 1), before triggering the first
        // registration, output a "Info" trace message.

        if ((requestInfo.fHandle == 1) && !triggered)
        {
            STAFString message = STAFString(
                "Begin running LifeCycle service ") +
                getPhaseAsString(phase) + " registrations";
            STAFTrace::trace(kSTAFTraceInfo, message);
            
            triggered = true;
        }

        // Log a message in the service log

        STAFString message = STAFString("[ID=") + iter->id + "] [" +
            requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
            requestInfo.fHandle + "] [TRIGGER " + getPhaseAsString(phase) +
            "] Submitted: STAF " + iter->machine + " " + iter->service +
            " " + iter->request;

        logMessage(message);

        // Trigger the registration by submitting a synchronous request
        // to run the specified request by the specified service on the
        // specified machine

        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            iter->machine, iter->service, iter->request);
        
        // Log a message in the service log with the results 

        message = STAFString("[ID=") + iter->id + "] [" +
            requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
            requestInfo.fHandle + "] [TRIGGER " + getPhaseAsString(phase) +
            "] Completed. RC=" + result->rc + ", Result=" +
            result->resultContext->asFormattedString();

        logMessage(message);

        // Add the entry to the outputList

        STAFObjectPtr resultMap = fTriggerIdsMapClass->createInstance();

        resultMap->put("id", iter->id);
        resultMap->put("machine", iter->machine);
        resultMap->put("service", iter->service);
        resultMap->put("request", STAFHandle::maskPrivateData(iter->request));
        resultMap->put("rc", result->rc);
        resultMap->put("result", result->result);

        outputList->append(resultMap);

        // If the registration's "once" field is true and the registration
        // is triggered by STAFProc (handle is 1), unregister the ID

        if ((iter->once == kTrue) && (requestInfo.fHandle == 1))
        {
            result = gSTAFProcHandlePtr->submit(
                "local", "LIFECYCLE", STAFString("UNREGISTER ID ") + iter->id);
            
            if (result->rc != 0)
            {
                // Should never happen

                STAFString message = STAFString("[ID=") + iter->id + "] [" +
                    requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
                    requestInfo.fHandle + "] [UNREGISTER due to ONCE=True]" +
                    " Failed. RC=" + result->rc + ", Result=" + result->result;

                logMessage(message);
            }
        }
    }

    mc->setRootObject(outputList);

    // If at least one registration was triggered by a request from STAFProc
    // (handle is 1), output an "Info" trace message.

    if ((requestInfo.fHandle == 1) && triggered)
    {
        STAFString message = STAFString(
            "Done running LifeCycle service ") +
            getPhaseAsString(phase) + " registrations";
        STAFTrace::trace(kSTAFTraceInfo, message);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFLifeCycleService::handleEnable(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "ENABLE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fEnableParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
    
    // Resolve any STAF variables specified in values for ID
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    RegistrationID_t id;
    
    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("ID", id, 1, UINT_MAX);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Retain a lock on the registration map only in this block
    {
        STAFMutexSemLock registrationLock(fRegistrationMapSem);

        // Check if the ID exists in the registration map.

        if (fRegistrationMap.find(id) == fRegistrationMap.end())
        {
            errorBuffer = STAFString("ID ") + id + " does not exist";

            return STAFServiceResult(kSTAFDoesNotExist, errorBuffer);
        }

        // Create a copy of the registrations map to update

        RegistrationMap newRegMap = fRegistrationMap;

        // Enable the registration

        RegistrationData &regData = newRegMap[id];

        regData.state = kEnabled;
        
        // Save the updated registration map in the registration file
        // and in memory

        STAFServiceResult result = saveRegistrationData(newRegMap);

        if (result.fRC != kSTAFOk)
        {
            return result;
        }
    }

    // Log a message in the service log

    STAFString message = STAFString("[ID=") + id + "] [" +
        requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
        requestInfo.fHandle + "] Enabled.";

    logMessage(message);

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFLifeCycleService::handleDisable(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "DISABLE");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fDisableParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in values for ID
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    RegistrationID_t id;
    
    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("ID", id, 1, UINT_MAX);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Retain a lock on the registration map only in this block
    {
        // Check if the ID exists in the registration map.

        STAFMutexSemLock registrationLock(fRegistrationMapSem);

        if (fRegistrationMap.find(id) == fRegistrationMap.end())
        {
            errorBuffer = STAFString("ID ") + id + " does not exist";

            return STAFServiceResult(kSTAFDoesNotExist, errorBuffer);
        }
                
        // Create a copy of the registrations map to update

        RegistrationMap newRegMap = fRegistrationMap;

        // Disable the registration

        RegistrationData &regData = newRegMap[id];

        regData.state = kDisabled;
        
        // Save the updated registration map in the registration file
        // and in memory

        STAFServiceResult result = saveRegistrationData(newRegMap);

        if (result.fRC != kSTAFOk)
        {
            return result;
        }
    }

    // Log a message in the service log

    STAFString message = STAFString("[ID=") + id + "] [" +
        requestInfo.fMachine + ", " + requestInfo.fHandleName + ", " +
        requestInfo.fHandle + "] Disabled.";

    logMessage(message);

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFLifeCycleService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}


void writeUIntToFile(ostream &output, unsigned int data, unsigned int length)
{
    union
    {
        char bytes[4];
        unsigned int uint;
    };

    uint = STAFUtilSwapUInt(STAFUtilConvertNativeUIntToLE(data));

    output.write(&bytes[4 - length], length);
}

void readUIntFromFile(istream &input, unsigned int &data, unsigned int length)
{
    union
    {
        char bytes[4];
        unsigned int uint;
    };

    uint = 0;

    input.read(&bytes[4 - length], length);

    data = STAFUtilConvertLEUIntToNative(STAFUtilSwapUInt(uint));
}

void readStringFromFile(istream &input, STAFString &inString)
{
    // First read in the UTF-8 data

    unsigned int stringLength = 0;

    readUIntFromFile(input, stringLength);

    char *inputData = new char[stringLength];

    input.read(inputData, stringLength);

    try
    {
        inString = STAFString(inputData, stringLength, STAFString::kUTF8);
    }
    catch(...)
    {
        delete [] inputData;
        throw;
    }

    delete [] inputData;
}

void writeStringToFile(ostream &output, STAFString &outString)
{
    unsigned int stringLength = outString.length();

    writeUIntToFile(output, stringLength);
    output.write(outString.buffer(), stringLength);
}

// Read the LifeCycle registration file and store its contents in a
// RegistrationMap.

// Format IDs 1 and 2 are supported for this file.  A file using format 1
// will be converted to the current format 2..

STAFRC_t readRegistrationFile(RegistrationMap &regMap, STAFString &errorBuffer)
{
    STAFRC_t rc = kSTAFOk;

    try
    {
        // Check if the LifeCycle registration file exists

        STAFFSPath path(fRegistrationFileName);

        try
        {
            if (!path.exists())
            {
                // The LifeCycle registration file does not exist, so just
                // return to use an empty registration map.

                return rc;
            }
        }
        catch (STAFBaseOSErrorException &e)
        {
            errorBuffer = STAFString(
                "Error checking if the file exists.\n") +
                e.getText() + STAFString(": ") + e.getErrorCode();
            
            return kSTAFFileOpenError;
        }

        // Open the LifeCycle registration file for reading in binary mode

        fstream regFile(fRegistrationFileName.toCurrentCodePage()->buffer(),
                        ios::in | STAF_ios_binary);

        if (!regFile)
        {
            errorBuffer = STAFString("Error opening the file");

            return kSTAFFileOpenError;
        }
    
        // Check the format ID for the file (the first byte) to determine
        // how to read the file

        unsigned int formatID = 0;

        readUIntFromFile(regFile, formatID, 1);

        if (regFile.eof())
        {
            errorBuffer = STAFString("Invalid file contents");

            return kSTAFFileReadError;
        }

        if (formatID == sCurrFileFormatID)
        {
            // This is the current binary format (changed in STAF V3.4.7).
            // A "once" field was added to each registration entry.
            // Its format is as follows:
            //
            // <formatID><numRegistrations>[<registration>]...
            //
            // - formatID         is an unsigned int with a length of 1 byte
            // - numRegistrations is an unsigned int with a length of 4 bytes
            //                    Note: This is the number of times a
            //                          <registration> entry exists in the
            //                          file.
            //
            // and each <registration> entry has format:
            //
            // <id><phase><priority><state><machine><service><request><descr>
            //
            // - id          is an unsigned int with a length of 4 bytes
            // - phase       is an unsigned int with a length of 1 byte
            // - priority    is an unsigned int with a length of 1 byte
            // - state       is an unsigned int with a length of 1 byte
            // - once        is an unsigned int with a length of 1 byte
            // - machine     is a STAFString
            // - service     is a STAFString
            // - request     is a STAFString
            // - descr       is a STAFString
            //
            // Note: A STAFString is written in the following format:
            //
            // <stringLength><String>
            //
            // - stringLength is an unsigned int with a legnth of 4 bytes
            // - string is the data in UTF-8 encoding
        
            unsigned int numRegistrations = 0;

            readUIntFromFile(regFile, numRegistrations);
        
            for (unsigned int i = 0; i < numRegistrations; i++) 
            {
                RegistrationData regData;

                readUIntFromFile(regFile, regData.id);

                if (regData.id < 1)
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "ID for registration entry #" + (i + 1) +
                        " is invalid: " + regData.id;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, (unsigned int &)regData.phase, 1);

                if ((regData.phase != kStartup) &&
                    (regData.phase != kShutdown))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "Phase for registration ID " + regData.id +
                        " is invalid: " + regData.phase;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, regData.priority, 1);

                if ((regData.priority < 1) || (regData.priority > 99))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "Priority for registration ID " + regData.id +
                        " is invalid: " + regData.priority;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, (unsigned int &)regData.state, 1);
            
                if ((regData.state != kEnabled) &&
                    (regData.state != kDisabled))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "State for registration ID " + regData.id +
                        " is invalid: " + regData.state;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, (unsigned int &)regData.once, 1);
            
                if ((regData.once != kFalse) && (regData.once != kTrue))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "Once for registration ID " + regData.id +
                        " is invalid: " + regData.once;

                    return kSTAFFileReadError;
                }

                readStringFromFile(regFile, regData.machine);
                readStringFromFile(regFile, regData.service);
                readStringFromFile(regFile, regData.request);
                readStringFromFile(regFile, regData.description);

                // Add to registrations map

                regMap[regData.id] = regData;
            }
        }
        else if (formatID == 1)
        {
            // This is the old binary format which was used from STAF V3.2.4
            // (when the LifeCycle service was first added) through V3.4.6.
            // Its format is as follows:
            //
            // <formatID><numRegistrations>[<registration>]...
            //
            // - formatID         is an unsigned int with a length of 1 byte
            // - numRegistrations is an unsigned int with a length of 4 bytes
            //                    Note: This is the number of times a
            //                          <registration> entry exists in the
            //                          file.
            //
            // and each <registration> entry has format:
            //
            // <id><phase><priority><state><machine><service><request><descr>
            //
            // - id          is an unsigned int with a length of 4 bytes
            // - phase       is an unsigned int with a length of 1 byte
            // - priority    is an unsigned int with a length of 1 byte
            // - state       is an unsigned int with a length of 1 byte
            // - machine     is a STAFString
            // - service     is a STAFString
            // - request     is a STAFString
            // - descr       is a STAFString
            //
            // Note: A STAFString is written in the following format:
            //
            // <stringLength><String>
            //
            // - stringLength is an unsigned int with a legnth of 4 bytes
            // - string is the data in UTF-8 encoding
        
            unsigned int numRegistrations = 0;

            readUIntFromFile(regFile, numRegistrations);
        
            for (unsigned int i = 0; i < numRegistrations; i++) 
            {
                RegistrationData regData;

                readUIntFromFile(regFile, regData.id);

                if (regData.id < 1)
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "ID for registration entry #" + (i + 1) +
                        " is invalid: " + regData.id;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, (unsigned int &)regData.phase, 1);

                if ((regData.phase != kStartup) &&
                    (regData.phase != kShutdown))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "Phase for registration ID " + regData.id +
                        " is invalid: " + regData.phase;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, regData.priority, 1);

                if ((regData.priority < 1) || (regData.priority > 99))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "Priority for registration ID " + regData.id +
                        " is invalid: " + regData.priority;

                    return kSTAFFileReadError;
                }

                readUIntFromFile(regFile, (unsigned int &)regData.state, 1);
            
                if ((regData.state != kEnabled) &&
                    (regData.state != kDisabled))
                {
                    errorBuffer = STAFString("Invalid file contents. ") +
                        "State for registration ID " + regData.id +
                        " is invalid: " + regData.state;

                    return kSTAFFileReadError;
                }

                // Default the once field to false

                regData.once = kFalse;

                readStringFromFile(regFile, regData.machine);
                readStringFromFile(regFile, regData.service);
                readStringFromFile(regFile, regData.request);
                readStringFromFile(regFile, regData.description);

                // Add to registrations map

                regMap[regData.id] = regData;
            }

            // Save the registration map in the registration file so that
            // it will now be in the current format

            STAFServiceResult result = saveRegistrationData(regMap);

            if (result.fRC != kSTAFOk)
            {
                errorBuffer = result.fResult;
                return result.fRC;
            }
        }
        else
        {
            // Unknown/invalid file format

            errorBuffer = STAFString("Invalid file format ID: ") + formatID;

            return kSTAFFileReadError;
        }
    }
    catch(...)
    {
        errorBuffer = STAFString("Exception occurred reading the file");

        return kSTAFFileReadError;
    }

    return rc;
}

// Write to the LifeCycle registration file.
// Note that this file is always written in the current format.

STAFServiceResult saveRegistrationData(RegistrationMap &regMap)
{
    // Write the updated registration data to the registration file

    try
    {
        // Open the registration data file to write to it in binary
    
        fstream regFile(fRegistrationFileName.toCurrentCodePage()->buffer(),
                        ios::out | STAF_ios_binary);
        
        if (!regFile)
        {
            STAFString errorBuffer(
                "Error opening LifeCycle Registration file " +
                fRegistrationFileName + " in write mode");

            return STAFServiceResult(kSTAFFileWriteError, errorBuffer);
        }
    
        // Write the new registration data to the registration data file

        writeUIntToFile(regFile, sCurrFileFormatID, 1);
        writeUIntToFile(regFile, regMap.size());

        RegistrationMap::iterator iter;

        for (iter = regMap.begin(); iter != regMap.end(); iter++)
        {
            writeUIntToFile(regFile, iter->second.id);
            writeUIntToFile(regFile, iter->second.phase, 1);
            writeUIntToFile(regFile, iter->second.priority, 1);
            writeUIntToFile(regFile, iter->second.state, 1);
            writeUIntToFile(regFile, iter->second.once, 1);
            writeStringToFile(regFile, iter->second.machine);
            writeStringToFile(regFile, iter->second.service);
            writeStringToFile(regFile, iter->second.request);
            writeStringToFile(regFile, iter->second.description);
        }
    }
    catch(...)
    {
        STAFString errorBuffer(
            "Exception occurred writing to LifeCycle Registration file " +
            fRegistrationFileName);

        return STAFServiceResult(kSTAFFileWriteError, errorBuffer);
    }

    // Update the registration map in memory.
    // Note:  This must be done after writing the updated registration data
    // to the file in case an error occurred writing the data.

    fRegistrationMap = regMap;

    return STAFServiceResult(kSTAFOk);
}

