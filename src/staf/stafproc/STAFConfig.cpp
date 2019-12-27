/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFString.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFConfig.h"
#include "STAFUtil.h"
#include "STAFConnectionManager.h"
#include "STAFCommandParser.h"
#include "STAFException.h"
#include "STAFDelegatedService.h"
#include "STAFExternalService.h"
#include "STAFNotificationList.h"
#include "STAFHandleManager.h"
#include "STAFTrustManager.h"
#include "STAFDiagManager.h"
#include "STAFServiceManager.h"
#include "STAFThreadManager.h"
#include "STAFProcessService.h"
#include "STAFTrace.h"
#include "STAFTraceService.h"

std::deque<STAFString> fSetVarLines;

unsigned int handleMachineNicknameConfig(const STAFString &line);
unsigned int handleInterfaceConfig(const STAFString &line);
unsigned int handleTrustConfig(const STAFString &line);
unsigned int handleNotifyConfig(const STAFString &line);
unsigned int handleTraceConfig(const STAFString &line);
unsigned int handleServiceConfig(const STAFString &line);
unsigned int handleServiceLoaderConfig(const STAFString &line);
unsigned int handleAuthenticatorConfig(const STAFString &line);
unsigned int handleSetConfig(const STAFString &line);

static void printErrorMessage(const STAFString &type,
                              const STAFString &line,
                              const STAFString &errorMsg = STAFString(""),
                              const STAFRC_t rc = kSTAFOk);
static void printSetErrorMessage(const STAFString &optionName,
                                 const STAFString &line,
                                 const STAFString &errorMsg = STAFString(""),
                                 const STAFRC_t rc = kSTAFOk);

unsigned int fSLSNum = 1;
const STAFVariablePool *varPoolList[] = { 0, 0 };
unsigned int varPoolListSize = 2;
const unsigned int maxLineLength = 2048;

unsigned int readConfigFile(const STAFString &filename)
{
    // Init the variable pool list
    varPoolList[0] = *gSharedVariablePoolPtr;
    varPoolList[1] = *gGlobalVariablePoolPtr;

    unsigned int rc = 0;

    ifstream config(filename.toCurrentCodePage()->buffer());

    if (!config)
    {
        cout << "Cannot find configuration file: " << filename << endl;
        return 1;
    }

    STAFString whiteSpace("\x20\x09\x0A\x0D");
    STAFString line;
    char configLine[maxLineLength];

    for(; config.good() && !rc;)
    {
        configLine[0] = 0;
        config.getline(configLine, maxLineLength);

        if (!config.good())
        {
            if (!config.eof())
            {
                cout << "Line in configuration file exceeds maximum length of "
                     << STAFString(maxLineLength) << " characters" << endl;
                rc = 1;
                break;
            }

            if (configLine == "") break;
        }

        STAFString thisLine(configLine);

        if ((thisLine == "") ||
            (thisLine.findFirstNotOf(whiteSpace) == STAFString::kNPos) ||
            (thisLine.subString(0, 1) == kUTF8_POUND))
        {
            if (line.length() != 0)
            {
                cout << "Invalid continuation line:" << endl << endl
                     << thisLine << endl;
                rc = 1;
            }

            continue;
        }
        else if (thisLine.subString(thisLine.length() - 1, 1) == kUTF8_BSLASH)
        {
            line += thisLine.subString(0,
                    thisLine.length() - 1).strip(STAFString::kFront);
            continue;
        }
        else
        {
            line += thisLine.strip(STAFString::kFront);
        }

        STAFString firstLowerWord = line.subWord(0, 1).lowerCase();

        if (firstLowerWord == "machinenickname")
            rc = handleMachineNicknameConfig(line);
        else if (firstLowerWord == "interface")
            rc = handleInterfaceConfig(line);
        else if (firstLowerWord == "trust")
            rc = handleTrustConfig(line);
        else if (firstLowerWord == "notify")
            rc = handleNotifyConfig(line);
        else if (firstLowerWord == "trace")
            rc = handleTraceConfig(line);
        else if (firstLowerWord == "service")
            rc = handleServiceConfig(line);
        else if (firstLowerWord == "serviceloader")
            rc = handleServiceLoaderConfig(line);
        else if (firstLowerWord == "authenticator")
            rc = handleAuthenticatorConfig(line);
        else if (firstLowerWord == "set")
            rc = handleSetConfig(line);
        else
        {
            cout << "Unknown config command '" << line.subWord(0, 1)
                 << "' in line:" << endl << endl
                 << line << endl << endl
                 << "Valid config commands include:" << endl;
            cout << "  authenticator, interface, machinenickname, "
                 << "notify, trace, service," << endl
                 << "  serviceloader, set" << endl;
            rc = 1;
        }

        // Reset line for the next statement
        line = STAFString();
    }

    return rc;
}

std::deque<STAFString> getSetVarLines()
{
    return fSetVarLines;
}

unsigned int handleTrustConfig(const STAFString &line)
{
    STAFString type("Trust");
    STAFCommandParser fTrustParser;

    // trust config options

    fTrustParser.addOption("TRUST",   1,
        STAFCommandParser::kValueNotAllowed);
    fTrustParser.addOption("LEVEL",   1,
        STAFCommandParser::kValueRequired);
    fTrustParser.addOption("MACHINE", 0,
        STAFCommandParser::kValueRequired);
    fTrustParser.addOption("USER", 0,
        STAFCommandParser::kValueRequired);
    fTrustParser.addOption("DEFAULT", 1,
        STAFCommandParser::kValueNotAllowed);

    // trust config needs

    fTrustParser.addOptionNeed("TRUST", "LEVEL");
    fTrustParser.addOptionNeed("LEVEL", "TRUST");

    // trust config groups

    fTrustParser.addOptionGroup("LEVEL",                1, 1);
    fTrustParser.addOptionGroup("MACHINE USER DEFAULT", 1, 1);

    STAFCommandParseResultPtr parsedResult = fTrustParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    STAFString errorBuffer;

    // Resolve any variables in the LEVEL option and convert to an
    // unsigned integer in the range of 0 - maxTrustLevel

    unsigned int level;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "LEVEL", level, 0, gTrustManagerPtr->getMaxTrustLevel());

    if (rc != kSTAFOk)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    if (parsedResult->optionTimes("DEFAULT") != 0)
    {
        rc = gTrustManagerPtr->setDefaultTrusteeLevel(level);

        if (rc != kSTAFOk)
        {
            STAFString errorMsg = STAFString(
                "Error setting default trust to level ") + level;

            printErrorMessage(type, line, errorMsg, rc);
            return 1;
        }
    }
    else if (parsedResult->optionTimes("MACHINE") != 0)
    {
        for(int i = 1; i <= parsedResult->optionTimes("MACHINE"); ++i)
        {
            STAFString machine;
            rc = RESOLVE_INDEXED_STRING_OPTION("MACHINE", i, machine);

            if (rc != kSTAFOk)
            {
                STAFString errorMsg = STAFString(
                    "Error resolving STAF variables in MACHINE option.\n") +
                    errorBuffer;

                printErrorMessage(type, line, errorMsg, rc);
                return 1;
            }

            rc = gTrustManagerPtr->setMachineTrusteeLevel(machine, level);

            if (rc != kSTAFOk)
            {
                STAFString errorMsg = STAFString(
                    "Error adding trust definition for machine ") + machine +
                    " at trust level " + level;

                printErrorMessage(type, line, errorMsg, rc);
                return 1;
            }
        }
    }
    else
    {
        for(int i = 1; i <= parsedResult->optionTimes("USER"); ++i)
        {
            STAFString user;
            rc = RESOLVE_INDEXED_STRING_OPTION("USER", i, user);

            if (rc != kSTAFOk)
            {
                STAFString errorMsg = STAFString(
                    "Error resolving STAF variables in USER option.\n") +
                    errorBuffer;

                printErrorMessage(type, line, errorMsg, rc);
                return 1;
            }

            rc = gTrustManagerPtr->setUserTrusteeLevel(user, level);

            if (rc != kSTAFOk)
            {
                STAFString errorMsg = STAFString(
                    "Error adding trust definition for user ") + user +
                    " at trust level " + level;

                printErrorMessage(type, line, errorMsg, rc);
                return 1;
            }
        }
    }

    return 0;
}


unsigned int handleInterfaceConfig(const STAFString &line)
{
    STAFString type("Interface");
    STAFCommandParser fInterfaceParser;

    // interface config options

    fInterfaceParser.addOption("INTERFACE", 1,
        STAFCommandParser::kValueRequired);
    fInterfaceParser.addOption("LIBRARY",   1,
        STAFCommandParser::kValueRequired);
    fInterfaceParser.addOption("OPTION",    0,
        STAFCommandParser::kValueRequired);

    // interface config groups

    fInterfaceParser.addOptionGroup("INTERFACE", 1, 1);
    fInterfaceParser.addOptionGroup("LIBRARY",   1, 1);

    // interface config needs

    fInterfaceParser.addOptionNeed("LIBRARY", "INTERFACE");
    fInterfaceParser.addOptionNeed("OPTION", "LIBRARY");

    STAFCommandParseResultPtr parsedResult = fInterfaceParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    STAFString errorBuffer;
    STAFString name;
    STAFRC_t rc = RESOLVE_STRING_OPTION("INTERFACE", name);

    if (rc != kSTAFOk)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    if (name.toLowerCase() == "local")
    {
        STAFString errorMsg = STAFString("Interface name ") + name +
             " is reserved for the local IPC interface";

        printErrorMessage(type, line, errorMsg, rc);
        return 1;
    }

    STAFString library;
    rc = RESOLVE_STRING_OPTION("LIBRARY", library);

    if (rc != kSTAFOk)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    STAFConnectionManager::ConnectionProviderOptionList optionList;

    for (int i = 1; i <= parsedResult->optionTimes("OPTION"); ++i)
    {
        STAFString option;
        rc = RESOLVE_INDEXED_STRING_OPTION("OPTION", i, option);

        if (rc != kSTAFOk)
        {
            printErrorMessage(type, line, errorBuffer, rc);
            return 1;
        }

        optionList.push_back(option);
    }

    rc = gConnectionManagerPtr->addConnectionProvider(name, library, optionList,
                                                      errorBuffer);

    if (rc != kSTAFOk)
    {
        printErrorMessage(
            type, line,
            STAFString("Error creating interface.  ") + errorBuffer, rc);
        return 1;
    }

    return 0;
}

unsigned int handleMachineNicknameConfig(const STAFString &line)
{
    STAFString type("MachineNickname");
    STAFCommandParser fMachineNicknameParser;

    // machine config options

    fMachineNicknameParser.addOption("MACHINENICKNAME", 1,
        STAFCommandParser::kValueRequired);

    STAFCommandParseResultPtr parsedResult = fMachineNicknameParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }
    
    STAFString errorBuffer;
    STAFString machineNickname;
    STAFRC_t rc = RESOLVE_STRING_OPTION("MACHINENICKNAME", machineNickname);

    if (rc != kSTAFOk)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    *gMachineNicknamePtr = machineNickname;

    return 0;
}


unsigned int handleNotifyConfig(const STAFString &line)
{
    STAFString type("Notify");
    STAFCommandParser fNotifyParser;

    // notify config options

    fNotifyParser.addOption("NOTIFY",     1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("ONSTART",    1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("ONSHUTDOWN", 1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("NAME",       1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("HANDLE",     1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("MACHINE",    1,
        STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("PRIORITY",   1,
        STAFCommandParser::kValueRequired);

    // notify config groups

    fNotifyParser.addOptionGroup("NAME HANDLE",        1, 1);
    fNotifyParser.addOptionGroup("ONSTART ONSHUTDOWN", 1, 1);
    fNotifyParser.addOptionGroup("MACHINE",            1, 1);
    fNotifyParser.addOptionGroup("NOTIFY",             1, 1);

    STAFCommandParseResultPtr parsedResult = fNotifyParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    unsigned int onStart = parsedResult->optionTimes("ONSTART");
    unsigned int notifyName = parsedResult->optionTimes("NAME");
    unsigned int priority = 5;
    unsigned int handle = 0;
    STAFString name;
    STAFString machine;
    STAFString errorBuffer;

    STAFRC_t rc = RESOLVE_OPTIONAL_UINT_OPTION("PRIORITY", priority);

    if (!rc) rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
        "HANDLE", handle, gHandleManagerPtr->getMinHandleNumber(),
         gHandleManagerPtr->getMaxHandleNumber());

    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("NAME", name);
    if (!rc) rc = RESOLVE_STRING_OPTION("MACHINE", machine);

    if (rc != kSTAFOk)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    STAFNotificationList *notificationList = gNotifyOnStartPtr;

    if (!onStart) notificationList = gNotifyOnShutdownPtr;

    if (notifyName) rc = notificationList->reg(machine, name, priority);
    else rc = notificationList->reg(machine, handle, priority);

    if (rc != kSTAFOk)
    {
        STAFString errorMsg = STAFString("Error registering to be notified");
        printErrorMessage(type, line, errorMsg, rc);
    }

    return rc;
}


unsigned int handleTraceConfig(const STAFString &line)
{
    STAFString type("Trace");
    STAFCommandParser fTraceParser;

    //trace options
    fTraceParser.addOption("TRACE", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("ENABLE", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("DISABLE", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("ALL", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("TRACEPOINTS", 1,
        STAFCommandParser::kValueAllowed);
    fTraceParser.addOption("TRACEPOINT", 0,
        STAFCommandParser::kValueRequired);
    fTraceParser.addOption("SERVICES", 1,
        STAFCommandParser::kValueAllowed);
    fTraceParser.addOption("SERVICE", 0,
        STAFCommandParser::kValueRequired);
    fTraceParser.addOption("SET", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("DESTINATION", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("TO", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("STDOUT", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("STDERR", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("FILE", 1,
        STAFCommandParser::kValueRequired);
    fTraceParser.addOption("APPEND", 1,
        STAFCommandParser::kValueNotAllowed);
    fTraceParser.addOption("DEFAULTSERVICESTATE", 1,
        STAFCommandParser::kValueRequired);
    fTraceParser.addOption("MAXSERVICERESULTSIZE", 1,
        STAFCommandParser::kValueRequired);

    // trace groups
    fTraceParser.addOptionGroup("ENABLE DISABLE SET", 0, 1);
    fTraceParser.addOptionGroup("STDOUT STDERR", 0, 1);
    fTraceParser.addOptionGroup(
        "DEFAULTSERVICESTATE DESTINATION MAXSERVICERESULTSIZE", 0, 1);

    // trace needs
    fTraceParser.addOptionNeed("TRACE", "ENABLE DISABLE SET");
    fTraceParser.addOptionNeed("ENABLE DISABLE SET", "TRACE");
    fTraceParser.addOptionNeed("ENABLE DISABLE",
        "ALL TRACEPOINT SERVICE TRACEPOINTS SERVICES");
    fTraceParser.addOptionNeed(
        "SET", "TO DEFAULTSERVICESTATE MAXSERVICERESULTSIZE");
    fTraceParser.addOptionNeed("DESTINATION", "SET");
    fTraceParser.addOptionNeed("TO", "DESTINATION");
    fTraceParser.addOptionNeed("TO", "STDOUT STDERR FILE");
    fTraceParser.addOptionNeed("APPEND", "FILE");
    fTraceParser.addOptionNeed("STDOUT STDERR FILE DEFAULTSERVICESTATE", "SET");


    STAFCommandParseResultPtr parsedResult = fTraceParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    STAFString errorBuffer;
    STAFString filename;
    STAFString tracepointList;
    STAFString serviceList;
    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("FILE", filename);

    if (rc != kSTAFOk)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    if (parsedResult->optionTimes("TO") != 0)
    {
        // Set the trace destination

        if (parsedResult->optionTimes("FILE") == 0)
        {
            // FILE is not specified for the trace destination

            if (parsedResult->optionTimes("STDOUT") != 0)
                STAFTrace::setTraceDestination(kSTAFTraceToStdout);
            else if (parsedResult->optionTimes("STDERR") != 0)
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
                STAFTrace::setTraceDestination(traceDestination,
                                               filename,
                                               kSTAFTraceFileAppend);
            else
                STAFTrace::setTraceDestination(traceDestination,
                                               filename,
                                               kSTAFTraceFileReplace);
        }
    }
    else if (parsedResult->optionTimes("DEFAULTSERVICESTATE") != 0)
    {
        //set default service tracing state
        bool stateToSetTo;
        STAFString defaultServiceState;

        rc = RESOLVE_STRING_OPTION("DEFAULTSERVICESTATE", defaultServiceState);

        if (rc != kSTAFOk)
        {
            printErrorMessage(type, line, errorBuffer, rc);
            return 1;
        }

        defaultServiceState = defaultServiceState.upperCase();

        if (defaultServiceState == "DISABLED")
        {
            stateToSetTo = STAFServiceManager::kTraceDisabled;
        }
        else if (defaultServiceState == "ENABLED")
        {
            stateToSetTo = STAFServiceManager::kTraceEnabled;
        }
        else
        {
            STAFString errorMsg = STAFString(
                "Invalid value for the DEFAULTSERICESTATE option: ") +
                defaultServiceState +
                ".   You must specify ENABLED or DISABLED";

            printErrorMessage(type, line, errorMsg, kSTAFInvalidValue);
            return 1;
        }

        STAFServiceManager::setDefaultTraceState(stateToSetTo);
    }
    else if (parsedResult->optionTimes("MAXSERVICERESULTSIZE") != 0)
    {
        STAFString maxServiceResultSize;

        rc = RESOLVE_STRING_OPTION("MAXSERVICERESULTSIZE",
                                   maxServiceResultSize);

        if (rc != kSTAFOk)
        {
            printErrorMessage(type, line, errorBuffer, rc);
            return 1;
        }

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
            printErrorMessage(type, line, errorBufferT, rc);
            return 1;
        }
    }
    else
    {
        //set tracing of services and tracepoints
        bool stateToSetTo;

        if (parsedResult->optionTimes("ENABLE") != 0)
        {
            stateToSetTo = STAFServiceManager::kTraceEnabled;
        }
        else if (parsedResult->optionTimes("DISABLE") != 0)
        {
            stateToSetTo = STAFServiceManager::kTraceDisabled;
        }
        else
        {
            STAFString errorMsg = STAFString(
                "You must specify either ENABLE or DISABLE");

            printErrorMessage(
                type, line, errorMsg, kSTAFInvalidRequestString);
            return 1;
        }

        if (parsedResult->optionTimes("ALL") != 0)
        //Handles ENABLE|DISABLE ALL ...
        {
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

                if (rc != kSTAFOk)
                {
                    printErrorMessage(type, line, errorBuffer, rc);
                    return 1;
                }

                tracepointList = tracepoints;
            }

            if (parsedResult->optionTimes("TRACEPOINT") != 0)
            {
                STAFString tracepoint;
                for (int i = 1,
                     optionCount = parsedResult->optionTimes("TRACEPOINT");
                     i <= optionCount; ++i)
                {
                    rc = RESOLVE_INDEXED_STRING_OPTION(
                        "TRACEPOINT", i, tracepoint);

                    if (rc != kSTAFOk)
                    {
                        printErrorMessage(type, line, errorBuffer, rc);
                        return 1;
                    }

                    tracepointList += " " + tracepoint;
                }
            }

            tracepointList = tracepointList.upperCase();

            //validate list of tracepoints
            for (int i = 0; i < tracepointList.numWords(); ++i )
            {
                STAFString tracepoint = tracepointList.subWord(i, 1).upperCase();

                if (STAFTraceService::kSTAFTracepointNameMap.find(tracepoint) !=
                        STAFTraceService::kSTAFTracepointNameMap.end())
                {
                    tracepointChangeSet =
                        tracepointChangeSet |
                        STAFTraceService::kSTAFTracepointNameMap[tracepoint];
                }
                else
                {
                    STAFString errorMsg = STAFString("Tracepoint ") +
                        tracepoint + " does not exist";

                    printErrorMessage(type, line, errorMsg, kSTAFDoesNotExist);
                    return 1;
                }
            }

            //now handle service tracing

            STAFString serviceList = "";

            if (parsedResult->optionTimes("SERVICES") != 0)
            {
                STAFString services;

                rc = RESOLVE_STRING_OPTION("SERVICES", services);

                if (rc != kSTAFOk)
                {
                    printErrorMessage(type, line, errorBuffer, rc);
                    return 1;
                }

                serviceList = services;
            }

            if (parsedResult->optionTimes("SERVICE") != 0)
            {
                STAFString service;

                for (int i = 1, optionCount =
                    parsedResult->optionTimes("SERVICE"); i<= optionCount; ++i)
                {
                    rc = RESOLVE_INDEXED_STRING_OPTION("SERVICE", i, service);

                    if (rc != kSTAFOk)
                    {
                        printErrorMessage(type, line, errorBuffer, rc);
                        return 1;
                    }

                    serviceList += " " + service;
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
    }
    return kSTAFOk;
}

unsigned int handleServiceConfig(const STAFString &line)
{
    STAFString type("Service");
    STAFCommandParser fServiceParser;

    // SERVICE <Name>
    // LIBRARY <Library> [EXECUTE <Exec>] {OPTION <OpName[=OpValue]>}
    //     [PARMS <Parms>]
    // or
    // DELEGATE <Machine> [TONAME <Service Name>]

    // service config options

    fServiceParser.addOption("SERVICE",  1,
        STAFCommandParser::kValueRequired);
    fServiceParser.addOption("LIBRARY",    1,
        STAFCommandParser::kValueRequired);
    fServiceParser.addOption("EXECUTE",    1,
        STAFCommandParser::kValueRequired);
    fServiceParser.addOption("OPTION",    0,
        STAFCommandParser::kValueRequired);
    fServiceParser.addOption("PARMS",    1,
        STAFCommandParser::kValueRequired);
    fServiceParser.addOption("DELEGATE", 1,
        STAFCommandParser::kValueRequired);
    fServiceParser.addOption("TONAME",   1,
        STAFCommandParser::kValueRequired);

    // service config groups

    fServiceParser.addOptionGroup("SERVICE",          1, 1);
    fServiceParser.addOptionGroup("LIBRARY DELEGATE", 1, 1);

    // service config needs

    fServiceParser.addOptionNeed("SERVICE", "LIBRARY DELEGATE");
    fServiceParser.addOptionNeed("LIBRARY DELEGATE", "SERVICE");
    fServiceParser.addOptionNeed("EXECUTE OPTION PARMS", "LIBRARY");
    fServiceParser.addOptionNeed("TONAME", "DELEGATE");

    STAFCommandParseResultPtr parsedResult = fServiceParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    STAFString errorBuffer;
    STAFString name;
    STAFRC_t rc = RESOLVE_STRING_OPTION("SERVICE", name);

    if (rc)
    {
         printErrorMessage(type, line, errorBuffer, rc);
         return 1;
    }

    rc = gServiceManagerPtr->verifyServiceName(name);

    if (rc)
    {
        STAFString errorMsg = STAFString(
            "Service name " + name + " is not valid.\n" +
            INVALID_SERVICE_NAME_ERROR_MESSAGE());

        printErrorMessage(type, line, errorMsg, rc);
        return 1;
    }

    try
    {
        STAFServicePtr service;

        if (parsedResult->optionTimes("LIBRARY") != 0)
        {
            // Setup external service data
            STAFString library;
            STAFString exec;
            STAFString parms;
            STAFExternalService::OptionList options;

            rc = RESOLVE_STRING_OPTION("LIBRARY", library);

            if (!rc) rc = RESOLVE_STRING_OPTION("EXECUTE", exec);
            
            if (rc)
            {
                printErrorMessage(type, line, errorBuffer, rc);
                return 1;
            }

            // The service will resolve PARMS

            if (parsedResult->optionTimes("PARMS") > 0)
            {
                parms = parsedResult->optionValue("PARMS");
            }

            for (int i = 1; i <= parsedResult->optionTimes("OPTION"); ++i)
            {
                STAFString option;
                rc = RESOLVE_INDEXED_STRING_OPTION("OPTION", i, option);
                
                if (rc)
                {
                    printErrorMessage(type, line, errorBuffer, rc);
                    return 1;
                }

                options.push_back(option);
            }

            service = STAFServicePtr(new STAFExternalService(name, library,
                                     exec, options, parms,
                                     kSTAFServiceTypeService),
                                     STAFServicePtr::INIT);
        }
        else
        {
            // This is a Delegated service

            STAFString toMachine;
            STAFString toName = name;

            rc = RESOLVE_STRING_OPTION("DELEGATE", toMachine);

            if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("TONAME", toName);

            if (rc)
            {
                printErrorMessage(type, line, errorBuffer, rc);
                return 1;
            }

            service = STAFServicePtr(new STAFDelegatedService(name, toMachine,
                                     toName), STAFServicePtr::INIT);
        }

        rc = gServiceManagerPtr->add(service);

        if (rc)
        {
            printErrorMessage(
                type, line, STAFString("Error adding service ") + name, rc);
            return 1;
        }
    }
    catch (STAFServiceCreateException &e)
    {
        printErrorMessage(type, line, e.getText(), e.getErrorCode());
        return 1;
    }
    catch (STAFException &e)
    {
        printErrorMessage(type, line, e.getText(), e.getErrorCode());
        return 1;
    }
    catch (...)
    {
        printErrorMessage(type, line, "Unexpected exception");
        return 1;
    }

    return 0;
}


unsigned int handleServiceLoaderConfig(const STAFString &line)
{
    STAFString type("ServiceLoader");
    STAFCommandParser fServiceLoaderParser;

    // SERVICELOADER
    // LIBRARY <Library> [EXECUTE <Exec>] {OPTION <OpName[=OpValue]>}
    //     [PARMS <Parms>]

    // serviceloader config options

    fServiceLoaderParser.addOption("SERVICELOADER",  1,
        STAFCommandParser::kValueNotAllowed);
    fServiceLoaderParser.addOption("LIBRARY",    1,
        STAFCommandParser::kValueRequired);
    fServiceLoaderParser.addOption("EXECUTE",    1,
        STAFCommandParser::kValueRequired);
    fServiceLoaderParser.addOption("OPTION",    0,
        STAFCommandParser::kValueRequired);
    fServiceLoaderParser.addOption("PARMS",    1,
        STAFCommandParser::kValueRequired);

    // service config groups

    fServiceLoaderParser.addOptionGroup("SERVICELOADER", 1, 1);
    fServiceLoaderParser.addOptionGroup("LIBRARY", 1, 1);

    // service config needs

    fServiceLoaderParser.addOptionNeed("EXECUTE OPTION PARMS",  "LIBRARY");

    STAFCommandParseResultPtr parsedResult = fServiceLoaderParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    try
    {
        STAFServicePtr service;

        // Setup external service data
        STAFString library;
        STAFString exec;
        STAFString parms;
        STAFExternalService::OptionList options;
        STAFString errorBuffer;
        STAFRC_t rc = RESOLVE_STRING_OPTION("LIBRARY", library);

        if (!rc) rc = RESOLVE_STRING_OPTION("EXECUTE", exec);
        if (!rc) rc = RESOLVE_STRING_OPTION("PARMS", parms);

        if (rc)
        {
            printErrorMessage(type, line, errorBuffer, rc);
            return 1;
        }

        for (int i = 1; !rc && (i <= parsedResult->optionTimes("OPTION"));
             ++i)
        {
            STAFString option;
            rc = RESOLVE_INDEXED_STRING_OPTION("OPTION", i, option);
            
            if (rc)
            {
                printErrorMessage(type, line, errorBuffer, rc);
                return 1;
            }

            options.push_back(option);
        }

        service = STAFServicePtr(
            new STAFExternalService("STAFServiceLoader" + STAFString(fSLSNum),
                                    library, exec, options, parms,
                                    kSTAFServiceTypeServiceLoader),
            STAFServicePtr::INIT);

        fSLSNum++;

        rc = gServiceManagerPtr->addSLS(service);

        if (rc)
        {
            printErrorMessage(
                type, line,
                STAFString("Error adding service loader service"), rc);

            return 1;
        }
    }
    catch (STAFServiceCreateException &e)
    {
        printErrorMessage(type, line, e.getText(), e.getErrorCode());
        return 1;
    }
    catch (STAFException &e)
    {
        printErrorMessage(type, line, e.getText(), e.getErrorCode());
        return 1;
    }
    catch (...)
    {
        printErrorMessage(type, line, "Unexpected exception");
        return 1;
    }

    return 0;
}


unsigned int handleAuthenticatorConfig(const STAFString &line)
{
    STAFString type("Authenticator");
    STAFCommandParser fAuthenticatorParser;

    // AUTHENTICATOR <Name>
    // LIBRARY <Library> [EXECUTE <Exec>] {OPTION <OpName[=OpValue]>}
    //     [PARMS <Parms>]

    // Authenticator config options

    fAuthenticatorParser.addOption("AUTHENTICATOR",  1,
        STAFCommandParser::kValueRequired);
    fAuthenticatorParser.addOption("LIBRARY",    1,
        STAFCommandParser::kValueRequired);
    fAuthenticatorParser.addOption("EXECUTE",    1,
        STAFCommandParser::kValueRequired);
    fAuthenticatorParser.addOption("OPTION",    0,
        STAFCommandParser::kValueRequired);
    fAuthenticatorParser.addOption("PARMS",    1,
        STAFCommandParser::kValueRequired);

    // service config groups

    fAuthenticatorParser.addOptionGroup("AUTHENTICATOR", 1, 1);
    fAuthenticatorParser.addOptionGroup("LIBRARY", 1, 1);

    // service config needs

    fAuthenticatorParser.addOptionNeed("EXECUTE OPTION PARMS", "LIBRARY");

    STAFCommandParseResultPtr parsedResult = fAuthenticatorParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        printErrorMessage(
            type, line, "Invalid syntax.  " + parsedResult->errorBuffer);
        return 1;
    }

    STAFString errorBuffer;
    STAFString name;
    STAFRC_t rc = RESOLVE_STRING_OPTION("AUTHENTICATOR", name);

    if (rc)
    {
        printErrorMessage(type, line, errorBuffer, rc);
        return 1;
    }

    rc = gServiceManagerPtr->verifyServiceName(name);

    if (rc != kSTAFOk)
    {
        STAFString errorMsg = STAFString(
            "Authenticator name " + name + " is not valid.\n" +
            INVALID_SERVICE_NAME_ERROR_MESSAGE());

        printErrorMessage(type, line, errorMsg, rc);
        return 1;
    }

    try
    {
        STAFServicePtr service;

        // Setup external service data
        STAFString library;
        STAFString exec;
        STAFString parms;
        STAFExternalService::OptionList options;

        rc = RESOLVE_STRING_OPTION("LIBRARY", library);

        if (!rc) rc = RESOLVE_STRING_OPTION("EXECUTE", exec);
        if (!rc) rc = RESOLVE_STRING_OPTION("PARMS", parms);
        
        if (rc)
        {
            printErrorMessage(type, line, errorBuffer, rc);
            return 1;
        }

        for (int i = 1; !rc && (i <= parsedResult->optionTimes("OPTION"));
             ++i)
        {
            STAFString option;
            rc = RESOLVE_INDEXED_STRING_OPTION("OPTION", i, option);
            
            if (rc)
            {
                printErrorMessage(type, line, errorBuffer, rc);
                return 1;
            }

            options.push_back(option);
        }

        service = STAFServicePtr(new STAFExternalService(name, library,
                                 exec, options, parms,
                                 kSTAFServiceTypeAuthenticator),
                                 STAFServicePtr::INIT);

        rc = gServiceManagerPtr->addAuthenticator(service);

        if (rc)
        {
            printErrorMessage(
                type, line, "Error adding authenticator service", rc);
            return 1;
        }

        // Assign as default authenticator if it has not been specified yet.
        if ((gServiceManagerPtr->getDefaultAuthenticator() == gNoneString) ||
            (gServiceManagerPtr->getDefaultAuthenticator() == ""))
        {
            gServiceManagerPtr->setDefaultAuthenticator(name);
        }
    }
    catch (STAFServiceCreateException &e)
    {
        printErrorMessage(type, line, e.getText(), e.getErrorCode());
        return 1;
    }
    catch (STAFException &e)
    {
        printErrorMessage(type, line, e.getText(), e.getErrorCode());
        return 1;
    }
    catch (...)
    {
        printErrorMessage(type, line, "Unexpected exception");
        return 1;
    }

    return 0;
}


static void printErrorMessage(const STAFString &type,
                              const STAFString &line,
                              const STAFString &errorMsg,
                              const STAFRC_t rc)
{
    cout << endl << "Error on " << type << " definition line:"
         << endl << endl << line << endl << endl;

    if (rc != kSTAFOk)
    {
        cout << "Error code: " << rc << endl;
    }
    
    if (errorMsg.length() != 0)
    {
        cout << "Reason    : " << errorMsg << endl;
    }
}


unsigned int handleSetConfig(const STAFString &line)
{
    STAFCommandParser fSetParser;

    // set config options

    // XXX: Note ALLOWMULTIREG is deprecated.  It is currently allowed by the
    //      parser, but ignored everywhere else, for backward compatibility.
    //      It will be removed entirely in STAF V3.0

    fSetParser.addOption("SET",                   1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("SYSTEM",                1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("SHARED",                1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("VAR",                   0,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DATADIR",               1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("CONNECTATTEMPTS",       1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("CONNECTRETRYDELAY",     1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("INTERFACECYCLING",      1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("MAXFILES",              1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("MAXQUEUESIZE",          1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("INITIALTHREADS",        1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("THREADGROWTHDELTA",     1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("ALLOWMULTIREG",         1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("DEFAULTINTERFACE",      1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHENTICATOR",  1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTSTOPUSING",      1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTNEWCONSOLE",     1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("DEFAULTSAMECONSOLE",    1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("DEFAULTFOCUS",          1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHDISABLEDACTION", 1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHUSERNAME",   1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHPASSWORD",   1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("PROCESSAUTHMODE",       1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTSHELL",          1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTNEWCONSOLESHELL", 1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTSAMECONSOLESHELL", 1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("ENABLEDIAGS", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("STRICTFSCOPYTRUST", 1,
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("RESULTCOMPATIBILITYMODE", 1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("MAXRETURNFILESIZE",       1,
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("HANDLEGCINTERVAL", 1,
        STAFCommandParser::kValueRequired);

    // set config groups

    fSetParser.addOptionGroup("SET", 1 , 1);
    fSetParser.addOptionGroup("DEFAULTNEWCONSOLE DEFAULTSAMECONSOLE", 0 , 1);
    fSetParser.addOptionGroup("SYSTEM SHARED", 0, 1);
    fSetParser.addOptionNeed("SHARED SYSTEM", "VAR");

    STAFCommandParseResultPtr parsedResult = fSetParser.parse(line);

    if (parsedResult->rc != kSTAFOk)
    {
        cout << "Invalid syntax in SET configuration line:" << endl << endl
             << line << endl << endl
             << "Reason: " << parsedResult->errorBuffer << endl;
        return 1;
    }

    STAFString errorBuffer;
    STAFString optionName = "VAR";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        // Save the SET VAR line so the CONFIG service can obtain the
        // startup variables and their values

        fSetVarLines.push_back(line);

        // handle var config

        STAFVariablePoolPtr thePool;

        if (parsedResult->optionTimes("SHARED") != 0)
        {
            thePool = *gSharedVariablePoolPtr;
        }
        else
        {
            thePool = *gGlobalVariablePoolPtr;
        }

        for(int i = 1; i <= parsedResult->optionTimes(optionName); ++i)
        {
            STAFString nameAndValue = parsedResult->optionValue(
                optionName, i);
            unsigned int equalPos = nameAndValue.find(kUTF8_EQUAL);

            if (equalPos == STAFString::kNPos)
            {
                cout << "Error setting a variable in line:" << endl << endl
                     << line << endl << endl
                     << "A VAR value must have format Name=Value. "
                     << "Missing the equal sign in VAR value: "
                     << nameAndValue << endl;

                return 1;
            }

            thePool->set(nameAndValue.subString(0, equalPos),
                         nameAndValue.subString(equalPos +
                         nameAndValue.sizeOfChar(equalPos)));
        }

        return 0;
    }

    STAFRC_t rc = kSTAFOk;

    optionName = "DATADIR";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        // Resolve the value for the DATADIR operational setting

        STAFString dataDir;

        rc = RESOLVE_STRING_OPTION(optionName, dataDir);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        // Create the data directory if it doesn't exist

        STAFFSPath dataPath;
        dataPath.setRoot(dataDir);

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
                errorBuffer = "Error creating DATADIR directory, " + dataDir;
                printSetErrorMessage(optionName, line, errorBuffer, rc);
                return 1;
            }
        }

        *gSTAFWriteLocationPtr = dataDir;

        // Set STAF system variable STAF/DataDir to the datadir value
        // This is a "read-only" STAF variable.

        (*gGlobalVariablePoolPtr)->set("STAF/DataDir", dataDir);

        STAFProcessService::setTempDirectory(
            dataDir + *gFileSeparatorPtr + "tmp");
    }

    optionName = "CONNECTATTEMPTS";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        rc = RESOLVE_UINT_OPTION_RANGE(
            optionName, gConnectionAttempts, 1, UINT_MAX);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "CONNECTRETRYDELAY";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        unsigned int connectRetryDelay = 0;

        rc = RESOLVE_DEFAULT_DURATION_OPTION(
            optionName, connectRetryDelay, 1000);
        
        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        gConnectionRetryDelay = connectRetryDelay;
    }
    
    optionName = "INTERFACECYCLING";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        if (optionValue.toUpperCase() == "ENABLED")
        {
            gConnectionManagerPtr->enableAutoInterfaceCycling();
        }
        else if (optionValue.toUpperCase() == "DISABLED")
        {
            gConnectionManagerPtr->disableAutoInterfaceCycling();
        }
        else
        {
            errorBuffer = "Valid values are Enabled or Disabled.  "
                "Invalid value: " + optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }
    
    optionName = "MAXFILES";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        rc = RESOLVE_UINT_OPTION(optionName, gMaxFiles);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "MAXQUEUESIZE";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        rc = RESOLVE_UINT_OPTION(optionName, gMaxQueueSize);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "INITIALTHREADS";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        rc = RESOLVE_UINT_OPTION_RANGE(
            optionName, gNumInitialThreads, 1, UINT_MAX);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "THREADGROWTHDELTA";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        unsigned int threadGrowthDelta;

        rc = RESOLVE_UINT_OPTION_RANGE(
            optionName, threadGrowthDelta, 1, UINT_MAX);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        gThreadManagerPtr->setGrowthDelta(threadGrowthDelta);
    }

    optionName = "DEFAULTINTERFACE";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = gConnectionManagerPtr->setDefaultConnectionProvider(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid " + optionName + " because interface '" +
                optionValue + "' is not registered";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTAUTHENTICATOR";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = gServiceManagerPtr->setDefaultAuthenticator(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid " + optionName + " because "
                "authenticator '" + optionValue + "' is not registered";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTSTOPUSING";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;
        STAFProcessStopMethod_t stopMethod;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::getStopMethodFromString(
            stopMethod, optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultStopMethod(stopMethod);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTFOCUS";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;
        STAFProcessConsoleFocus_t focus;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::getConsoleFocusFromString(
            focus, optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue + ".  Valid values are Background, Foreground, "
                "or Minimized.";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        STAFProcessService::setDefaultConsoleFocus(focus);
    }

    optionName = "PROCESSAUTHMODE";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;
        STAFProcessAuthenticationMode_t authMode;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::getAuthModeFromString(authMode,
                                                       optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        unsigned int osRC = 0;

        rc = STAFProcessService::setAuthMode(authMode, osRC);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTAUTHUSERNAME";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultAuthUsername(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTAUTHPASSWORD";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultAuthPassword(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTAUTHDISABLEDACTION";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;
        STAFProcessDisabledAuthAction_t defaultAuthDisabledAction;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::getDefaultDisabledAuthActionFromString(
             defaultAuthDisabledAction, optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue + ".  Must be Ignore or Error.";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultDisabledAuthAction(
             defaultAuthDisabledAction);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue + ".  Must be Ignore or Error.";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    if (parsedResult->optionTimes("DEFAULTNEWCONSOLE") != 0)
        gDefaultConsoleMode = kSTAFProcessNewConsole;
    else if (parsedResult->optionTimes("DEFAULTSAMECONSOLE") != 0)
        gDefaultConsoleMode = kSTAFProcessSameConsole;

    optionName = "DEFAULTSHELL";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultShellCommand(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTSAMECONSOLESHELL";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultSameConsoleShell(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "DEFAULTNEWCONSOLESHELL";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        rc = STAFProcessService::setDefaultNewConsoleShell(optionValue);

        if (rc != kSTAFOk)
        {
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue;
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    optionName = "ENABLEDIAGS";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        rc = gDiagManagerPtr->enable();

        if (rc != kSTAFOk)
        {
            errorBuffer = "";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }

    if (parsedResult->optionTimes("STRICTFSCOPYTRUST") != 0)
    {
        gStrictFSCopyTrust = 1;
    }
    
    optionName = "RESULTCOMPATIBILITYMODE";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        if (optionValue.isEqualTo("NONE", kSTAFStringCaseInsensitive))
        {
            gResultCompatibilityMode = kSTAFResultCompatibilityNone;
        }
        else if (optionValue.isEqualTo("VERBOSE", kSTAFStringCaseInsensitive))
        {
            gResultCompatibilityMode = kSTAFResultCompatibilityVerbose;
        }
        else
        {
            rc = kSTAFInvalidValue;
            errorBuffer = "Invalid value for " + optionName + ", " +
                optionValue + ".  Must be None or Verbose.";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
    }
    
    optionName = "MAXRETURNFILESIZE";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        STAFString optionValue;

        rc = RESOLVE_STRING_OPTION(optionName, optionValue);

        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        STAFString_t errorBufferT = 0;
        unsigned int maxSize = 0;

        rc = STAFUtilConvertSizeString(
            optionValue.getImpl(), &maxSize, &errorBufferT);
        
        if (rc == kSTAFOk)
        {
            gMaxReturnFileSize = maxSize;
        }
        else
        {
            printSetErrorMessage(
                optionName, line,
                STAFString(errorBufferT, STAFString::kShallow), rc);
            return 1;
        }
    }

    optionName = "HANDLEGCINTERVAL";

    if (parsedResult->optionTimes(optionName) != 0)
    {
        unsigned int handleGCInterval = 0;

        rc = RESOLVE_DEFAULT_DURATION_OPTION(
            optionName, handleGCInterval, 60000);
        
        if (rc != kSTAFOk)
        {
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }
        
        // Check if the interval specified for handle garbage collection is
        // valid.  It must be >= 5 seconds and <= 24 hours.

        if ((handleGCInterval < 5000) || (handleGCInterval > 86400000))
        {
            rc = kSTAFInvalidValue;
            errorBuffer = "Invalid value for " + optionName + ", " +
                STAFString(handleGCInterval) + ".  Must be between 5 "
                "seconds and 24 hours.\n\n"
                "This value may be expressed in milliseconds, seconds, "
                "minutes, hours, or a day.  Its format is <Number>[s|m|h|d] "
                "where <Number> is an integer >= 0 and indicates milliseconds "
                "unless one of the following case-insensitive suffixes is "
                "specified:  s (for seconds), m (for minutes), h (for hours), "
                "or d (for day).  The calculated value must be >= 5000 and "
                "<= 86400000 milliseconds.\n\nExamples: \n"
                "  60000 specifies 60000 milliseconds (or 1 minute), \n"
                "  30s specifies 30 seconds, \n"
                "  5m specifies 5 minutes, \n"
                "  2h specifies 2 hours, \n"
                "  1d specifies 1 day.";
            printSetErrorMessage(optionName, line, errorBuffer, rc);
            return 1;
        }

        gHandleGCInterval = handleGCInterval;
    }

    return 0;
}

static void printSetErrorMessage(const STAFString &optionName,
                                 const STAFString &line,
                                 const STAFString &errorMsg,
                                 const STAFRC_t rc)
{
    cout << endl << "Error setting the " << optionName
         << " operational parameter in line:"
         << endl << endl << line << endl << endl;

    if (rc != kSTAFOk)
    {
        cout << "Error code: " << rc << endl;
    }
    
    if (errorMsg.length() != 0)
    {
        cout << "Reason    : " << errorMsg << endl;
    }
}
