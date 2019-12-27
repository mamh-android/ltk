/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <map>
#include "STAF_fstream.h"
#include "STAF_iostream.h"
#include "STAFString.h"
#include "STAFError.h"
#include "STAFException.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFTimestamp.h"
#include "STAFUtil.h"
#include "STAFInternalUtil.h"
#include "STAFMonitorService.h"

typedef STAFRefPtr<STAFCommandParser> STAFCommandParserPtr;

typedef std::map<STAFHandle_t, STAFString> HandleMap;

struct NameData
{
    STAFString originalCaseName;  // The case specified on the first LOG request
    STAFString message;
};

typedef std::map<STAFString, NameData> NameMap;

struct MachineData
{
    STAFString realMachineName;
    HandleMap handleMap;
    NameMap nameMap;
};

typedef std::map<STAFString, MachineData> MachineMap;

struct MonitorServiceData
{
    unsigned int fDebugMode;
    STAFString fShortName;
    STAFString fName;
    STAFHandlePtr fHandlePtr;
    unsigned int fResolveMessage;
    unsigned int fMaxRecordSize;    
    unsigned int fEnableResolveMessageVar;
    STAFCommandParserPtr fLogParser;
    STAFCommandParserPtr fQueryParser;
    STAFCommandParserPtr fListParser;
    STAFCommandParserPtr fDeleteParser;
    STAFCommandParserPtr fSetParser;
    STAFMutexSemPtr fMapSem;
    MachineMap fMap;
    STAFString fLocalMachineName;

    // Map class definitions for marshalled results
    STAFMapClassDefinitionPtr fSettingsClass; 
    STAFMapClassDefinitionPtr fHandleInfoClass;
    STAFMapClassDefinitionPtr fNameInfoClass;
    STAFMapClassDefinitionPtr fMonitorInfoClass;
};

typedef STAFRefPtr<MonitorServiceData> MonitorServiceDataPtr;

static STAFString sHelpMsg;
static STAFString sLineSep;
static const STAFString monitorVersion("3.4.0");
static const STAFString sMachine("MACHINE");
static const STAFString sHandle("HANDLE");
static const STAFString sName("NAME");
static const STAFString sNames("NAMES");
static const STAFString sResolveMessage("RESOLVEMESSAGE");
static const STAFString sNoResolveMessage("NORESOLVEMESSAGE");
static const STAFString sMaxRecordSize("MAXRECORDSIZE");
static const STAFString sEnableResolveMessageVar("ENABLERESOLVEMESSAGEVAR");
static const STAFString sDisableResolveMessageVar("DISABLERESOLVEMESSAGEVAR");
static const STAFString sLeftCurlyBrace(kUTF8_LCURLY);
static const STAFString sSlash(kUTF8_SLASH);
static const STAFString sSpace(kUTF8_SPACE);
static const STAFString sColon(kUTF8_COLON);
static const STAFString sEqual(kUTF8_EQUAL);
static const STAFString sHandleList("Handle");
static const STAFString sSize("Size");
static const STAFString sUsing("Using ");
static const STAFString sToday("today");
static const STAFString sBefore("before");
static const STAFString sMessage("message");
static const STAFString sLocal("local");
static const STAFString sSettings("settings");
static const STAFString sMachines("machines");
static const STAFString sVar("var");
static const STAFString sHelp("help");
static const STAFString sResStrResolve("RESOLVE REQUEST ");
static const STAFString sString(" STRING ");

static STAFResultPtr handleLog(STAFServiceRequestLevel30 *, MonitorServiceData *);
static STAFResultPtr handleQuery(STAFServiceRequestLevel30 *,
                                 MonitorServiceData *);
static STAFResultPtr handleList(STAFServiceRequestLevel30 *,
                                MonitorServiceData *);
static STAFResultPtr handleDelete(STAFServiceRequestLevel30 *,
                                  MonitorServiceData *);
static STAFResultPtr handleSet(STAFServiceRequestLevel30 *,
                               MonitorServiceData *);
static STAFResultPtr handleHelp(STAFServiceRequestLevel30 *,
                                MonitorServiceData *);
static STAFResultPtr handleVersion(STAFServiceRequestLevel30 *,
                                   MonitorServiceData *);
static STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo, 
                                MonitorServiceData *pData,
                                const STAFString &theString);
static STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, 
                               MonitorServiceData *pData,
                               STAFCommandParseResultPtr &parsedResult,
                               const STAFString &fOption,
                               unsigned int optionIndex = 1);
static STAFResultPtr resolveOpLocal(MonitorServiceData *pData,
                                    STAFCommandParseResultPtr &parsedResult,
                                    const STAFString &fOption,
                                    unsigned int optionIndex = 1);

static STAFResultPtr convertOptionStringToUInt(
    const STAFString &theString,
    const STAFString &optionName,
    unsigned int &number,
    const unsigned int minValue = 0,
    const unsigned int maxValue = UINT_MAX);

STAFRC_t STAFServiceGetLevelBounds(unsigned int levelID,
                                   unsigned int *minimum,
                                   unsigned int *maximum)
{
    switch (levelID)
    {
        case kServiceInfo:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceInit:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceAcceptRequest:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceTerm:
        case kServiceDestruct:
        {
            *minimum = 0;
            *maximum = 0;
            break;
        }
        default:
        {
            return kSTAFInvalidAPILevel;
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFServiceConstruct(STAFServiceHandle_t *pServiceHandle,
                              void *pServiceInfo, unsigned int infoLevel,
                              STAFString_t *pErrorBuffer)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        if (infoLevel != 30) return kSTAFInvalidAPILevel;

        STAFServiceInfoLevel30 *pInfo =
            reinterpret_cast<STAFServiceInfoLevel30 *>(pServiceInfo);

        MonitorServiceData data;
        data.fDebugMode = 0;
        data.fMaxRecordSize = 1024;
        data.fResolveMessage = 0;        
        data.fEnableResolveMessageVar = 0;
        data.fShortName = pInfo->name;
        data.fName = "STAF/Service/";
        data.fName += pInfo->name;
        
        for (unsigned int i = 0; i < pInfo->numOptions; ++i)
        {            
            if (STAFString(pInfo->pOptionName[i]).upperCase() == "DEBUG")
            {
                data.fDebugMode = 1;
            }           
            else
            {
                STAFString optionError(pInfo->pOptionName[i]);
                *pErrorBuffer = optionError.adoptImpl();
                return kSTAFServiceConfigurationError;
            }
        }        

        // Set service handle

        *pServiceHandle = new MonitorServiceData(data);

        return kSTAFOk;
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                "STAFMonitorSerice.cpp: STAFServiceConstruct").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFMonitorService.cpp: STAFServiceConstruct: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }    

    return kSTAFUnknownError;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
                         void *pInitInfo, unsigned int initLevel,
                         STAFString_t *pErrorBuffer)
{
    STAFRC_t retCode = kSTAFUnknownError;

    try
    {    
        if (initLevel != 30) return kSTAFInvalidAPILevel;

        MonitorServiceData *pData =
            reinterpret_cast<MonitorServiceData *>(serviceHandle);
        
        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);        

        retCode = STAFHandle::create(pData->fName, pData->fHandlePtr);
        
        if (retCode != kSTAFOk)
            return retCode;

        pData->fMapSem = STAFMutexSemPtr(new STAFMutexSem,
                                         STAFMutexSemPtr::INIT);    
        
        //LOG options
        pData->fLogParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fLogParser->addOption("LOG",              1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fLogParser->addOption("MESSAGE",          1,
                                     STAFCommandParser::kValueRequired);
        pData->fLogParser->addOption("NAME",             1,
                                     STAFCommandParser::kValueRequired);
        pData->fLogParser->addOption("RESOLVEMESSAGE",   1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fLogParser->addOption("NORESOLVEMESSAGE", 1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fLogParser->addOptionNeed("LOG", "MESSAGE");
        pData->fLogParser->addOptionGroup("RESOLVEMESSAGE NORESOLVEMESSAGE",
                                          0, 1);

        //QUERY options
        pData->fQueryParser = STAFCommandParserPtr(new STAFCommandParser,
                                                   STAFCommandParserPtr::INIT);
        pData->fQueryParser->addOption("QUERY",   1,
                                       STAFCommandParser::kValueNotAllowed);
        pData->fQueryParser->addOption("MACHINE", 1,
                                       STAFCommandParser::kValueRequired);
        pData->fQueryParser->addOption("HANDLE",  1,
                                       STAFCommandParser::kValueRequired);                
        pData->fQueryParser->addOption("NAME",    1,
                                       STAFCommandParser::kValueRequired);
        pData->fQueryParser->addOptionGroup("MACHINE HANDLE NAME", 2, 2);
        pData->fQueryParser->addOptionGroup("HANDLE NAME", 0, 1);

        //LIST options
        pData->fListParser = STAFCommandParserPtr(new STAFCommandParser,
                                                  STAFCommandParserPtr::INIT);
        pData->fListParser->addOption("LIST",     1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("MACHINES", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("MACHINE",  1,
                                      STAFCommandParser::kValueRequired);
        pData->fListParser->addOption("NAMES",    1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("SETTINGS", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOptionGroup("MACHINES MACHINE SETTINGS", 1, 1);
        pData->fListParser->addOptionGroup("MACHINES NAME", 0, 1);
        pData->fListParser->addOptionGroup("SETTINGS NAME", 0, 1);


        //DELETE options
        pData->fDeleteParser = STAFCommandParserPtr(new STAFCommandParser,
                                                    STAFCommandParserPtr::INIT);
        pData->fDeleteParser->addOption("DELETE",  1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOption("CONFIRM", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOption("BEFORE",  1,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("MACHINE", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("NAME",    1,
                                        STAFCommandParser::kValueRequired);

        pData->fDeleteParser->addOptionNeed("DELETE", "CONFIRM");
        pData->fDeleteParser->addOptionNeed("MACHINE", "NAME");
        pData->fDeleteParser->addOptionNeed("NAME", "MACHINE");
        pData->fDeleteParser->addOptionGroup("BEFORE NAME", 0, 1);
        pData->fDeleteParser->addOptionGroup("BEFORE MACHINE", 0, 1);

        //SET options
        pData->fSetParser = STAFCommandParserPtr(new STAFCommandParser,
                            STAFCommandParserPtr::INIT);
        pData->fSetParser->addOption("SET",                 1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fSetParser->addOption("NORESOLVEMESSAGE",    1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fSetParser->addOption("RESOLVEMESSAGE",      1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fSetParser->addOptionGroup("NORESOLVEMESSAGE RESOLVEMESSAGE",
                                          0, 1);
        pData->fSetParser->addOption("MAXRECORDSIZE",    1,
                                     STAFCommandParser::kValueRequired);
        pData->fSetParser->addOption("ENABLERESOLVEMESSAGEVAR",    1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fSetParser->addOption("DISABLERESOLVEMESSAGEVAR",      1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fSetParser->addOptionGroup(
                        "ENABLERESOLVEMESSAGEVAR DISABLERESOLVEMESSAGEVAR",
                        0, 1);

        // PARMS        
        STAFCommandParseResultPtr parsedParmsResult =
            pData->fSetParser->parse(pInfo->parms);

        if (parsedParmsResult->rc != kSTAFOk) 
        {
            return parsedParmsResult->rc;
        }
        
        if (parsedParmsResult->optionTimes(sResolveMessage))
        {
            pData->fResolveMessage = 1;
        }

        if (parsedParmsResult->optionTimes(sNoResolveMessage))
        {
            pData->fResolveMessage = 0;
        }
        
        if (parsedParmsResult->optionTimes(sMaxRecordSize))
        {
            STAFResultPtr maxResult = resolveOpLocal(pData, parsedParmsResult,
                                                     sMaxRecordSize);

            if (maxResult->rc != kSTAFOk)
            {
                *pErrorBuffer = maxResult->result.adoptImpl();
                return maxResult->rc;
            }

            // Convert resolved option string to an unsigned integer in range
            // 0 to UINT_MAX

            maxResult = convertOptionStringToUInt(
                maxResult->result, sMaxRecordSize, pData->fMaxRecordSize);

            if (maxResult->rc != kSTAFOk)
            {
                *pErrorBuffer = maxResult->result.adoptImpl();
                return maxResult->rc;
            }
        }        
        
        if (parsedParmsResult->optionTimes(sEnableResolveMessageVar))
        {
            pData->fEnableResolveMessageVar = 1;
        }

        if (parsedParmsResult->optionTimes(sDisableResolveMessageVar))
        {
            pData->fEnableResolveMessageVar = 0;
        }

        // Construct map class for the marshalling LIST SETTINGS output

        pData->fSettingsClass = STAFMapClassDefinition::create(
            "STAF/Service/Monitor/Settings");

        pData->fSettingsClass->addKey("maxRecordSize", "Max Record Size");
        pData->fSettingsClass->addKey("resolveMessage", "Resolve Message");
        pData->fSettingsClass->addKey("resolveMessageVar",
                                      "Resolve Message Var");
        
        // Construct map class for handle information

        pData->fHandleInfoClass = STAFMapClassDefinition::create(
            "STAF/Service/Monitor/HandleInfo");

        pData->fHandleInfoClass->addKey("handle",    "Handle");
        pData->fHandleInfoClass->addKey("timestamp", "Date-Time");
        pData->fHandleInfoClass->addKey("size"     , "Size");

        // Construct map class for name information

        pData->fNameInfoClass = STAFMapClassDefinition::create(
            "STAF/Service/Monitor/NameInfo");

        pData->fNameInfoClass->addKey("name",    "Name");
        pData->fNameInfoClass->addKey("timestamp", "Date-Time");
        pData->fNameInfoClass->addKey("size"     , "Size");
        
        // Construct map class for monitor information

        pData->fMonitorInfoClass = STAFMapClassDefinition::create(
            "STAF/Service/Monitor/MonitorInfo");

        pData->fMonitorInfoClass->addKey("timestamp", "Date-Time");
        pData->fMonitorInfoClass->addKey("message", "Message");

        // Get the line separator

        STAFResultPtr result = pData->fHandlePtr->submit(
            "local", "VAR", "RESOLVE REQUEST STRING {STAF/Config/Sep/Line}");

        if (result->rc != kSTAFOk)
        {
            *pErrorBuffer = result->result.adoptImpl();
            return result->rc;
        }

        sLineSep = result->result;

         // Get the machine name for the local machine

        result = pData->fHandlePtr->submit(
            sLocal, sVar, "RESOLVE REQUEST STRING {STAF/Config/Machine}");

        if (result->rc != kSTAFOk)
        {
            *pErrorBuffer = result->result.adoptImpl();
            return result->rc;
        }

        pData->fLocalMachineName = result->result;

        // Assign the invalid directory mesasge

        STAFString invalidDirMessage("The STAF/Service/");

        invalidDirMessage += pData->fShortName;
        invalidDirMessage += "/Directory variable in the STAF configuration "
                             "file is invalid";

        // Assign the help text string for the service

        sHelpMsg = STAFString("*** ") + pData->fShortName + " Service Help ***" +
            sLineSep + sLineSep +
            "LOG     MESSAGE <Message> [NAME <Name>] [RESOLVEMESSAGE | NORESOLVEMESSAGE]" +
            sLineSep +
            "QUERY   MACHINE <Machine> < HANDLE <Handle> | NAME <Name> >" +
            sLineSep +
            "LIST    <MACHINES | MACHINE <Machine> [NAMES] | SETTINGS>" +
            sLineSep +
            "DELETE  [BEFORE <Timestamp> | MACHINE <Machine> NAME <Name>] CONFIRM" +
            sLineSep +
            "SET     [RESOLVEMESSAGE | NORESOLVEMESSAGE]" +
            sLineSep +
            "        [MAXRECORDSIZE <Size>]" +
            sLineSep +
            "        [ENABLERESOLVEMESSAGEVAR | DISABLERESOLVEMESSAGEVAR] " +
            sLineSep +
            "VERSION" +
            sLineSep +
            "HELP";
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                        "STAFMonitorService.cpp: STAFServiceInit").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFMonitorService.cpp: STAFServiceInit: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }    

    return retCode;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                  void *pRequestInfo, unsigned int reqLevel,
                                  STAFString_t *pResultBuffer)
{
    if (reqLevel != 30) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    try
    {
        STAFResultPtr result(new STAFResult(kSTAFOk, STAFString()),
                             STAFResultPtr::INIT);        

        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

        MonitorServiceData *pData =
            reinterpret_cast<MonitorServiceData *>(serviceHandle);

        STAFString request(pInfo->request);
        STAFString action = request.subWord(0, 1).toLowerCase();
        
        //call functions for the request
        if (action == "log")
            result = handleLog(pInfo, pData);
        else if (action == "query")
            result = handleQuery(pInfo, pData);
        else if (action == "list")
            result = handleList(pInfo, pData);
        else if (action == "delete")
            result = handleDelete(pInfo, pData);
        else if (action == "set")
            result = handleSet(pInfo, pData);
        else if (action == "help")
            result = handleHelp(pInfo, pData);
         else if (action == "version")
            result = handleVersion(pInfo, pData);
        else
        {
            STAFString errMsg = STAFString("'") + request.subWord(0, 1) +
                "' is not a valid command request for the " +
                pData->fShortName + " service" + sLineSep + sLineSep +
                sHelpMsg;

            result = STAFResultPtr(new STAFResult(
                kSTAFInvalidRequestString, errMsg), STAFResultPtr::INIT);
        }

        *pResultBuffer = result->result.adoptImpl();
        retCode = result->rc;
    }
    catch (STAFException &e)
    { 
        retCode = e.getErrorCode();

        *pResultBuffer = getExceptionString(
            e, "STAFMonitorService.cpp: STAFServiceAcceptRequest").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFMonitorService.cpp: STAFServiceAcceptRequest: "
                         "Caught unknown exception");
        *pResultBuffer = error.adoptImpl();
    }

    return retCode;
}


STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                         void *pTermInfo, unsigned int termLevel,
                         STAFString_t *pErrorBuffer)
{
    if (termLevel != 0) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    try
    {
        retCode = kSTAFOk;

        MonitorServiceData *pData = 
            reinterpret_cast<MonitorServiceData *>(serviceHandle);

        // Un-register Help Data (if add any service-specific error codes)

    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(
            e, "STAFMonitorSerice.cpp: STAFServiceTerm").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFMonitorService.cpp: STAFServiceTerm: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                             void *pDestructInfo, unsigned int destructLevel,
                             STAFString_t *pErrorBuffer)
{
    if (destructLevel != 0) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    try
    {
        MonitorServiceData *pData =
            reinterpret_cast<MonitorServiceData *>(*serviceHandle);

        delete pData;
        *serviceHandle = 0;

        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                    "STAFMonitorSerice.cpp: STAFServiceDestruct").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFMonitorService.cpp: STAFServiceDestruct: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


STAFResultPtr handleLog(STAFServiceRequestLevel30 *pInfo, 
                        MonitorServiceData *pData)
{
    // Verify the requesting machine/user has at least trust level 3

    VALIDATE_TRUST(3, pData->fShortName, "LOG", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fLogParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    STAFString fMessage = STAFTimestamp::now().asString() + sSpace;

    // Determine if we need to resolve the message
    // Start with the PARMS/SET resolve value        
    unsigned int resolveMessage = pData->fResolveMessage;

    // Request contains [NO]RESOLVEMESSAGE, so use its resolve value
    if (parsedResult->optionTimes(sNoResolveMessage))
    {
        resolveMessage = 0;
    }
    else if (parsedResult->optionTimes(sResolveMessage))
    {
        resolveMessage = 1;
    }
    else if (pData->fEnableResolveMessageVar)
    {                 
        // Check the STAF/Service/<Monitor-Name>/ResolveMessage variable
        STAFString varReq = "{STAF/Service/";
        varReq += pData->fShortName;
        varReq += "/ResolveMessage}";        
        
        STAFResultPtr resolveResult = resolveStr(pInfo, pData, varReq);

        if (resolveResult->rc == kSTAFOk) 
        {
            // Variable was found, so use its resolve value

            try
            {
                if (resolveResult->result.asUInt() != 0)
                    resolveMessage = 1;
                else
                    resolveMessage = 0;
            }
            catch (STAFException)
            {
                STAFString errorMsg = STAFString(
                    "The resolved value for variable ") + varReq +
                    " must be 0 or 1.  Invalid value: " +
                    resolveResult->result;

                return STAFResultPtr(
                    new STAFResult(kSTAFInvalidValue, errorMsg),
                    STAFResultPtr::INIT);
            }
        }        
    }
    
    if (resolveMessage)
    {
        STAFResultPtr messageResult = resolveOp(
            pInfo, pData, parsedResult, sMessage);
        
        if (messageResult->rc != 0) return messageResult;

        fMessage += messageResult->result;
    }
    else
    {
        fMessage += parsedResult->optionValue(sMessage);
    }
    
    fMessage = STAFHandle::maskPrivateData(fMessage);

    if (fMessage.length() > pData->fMaxRecordSize) 
    {
        fMessage = fMessage.subString(0, pData->fMaxRecordSize, 
                                      STAFString::kChar);    
    }

    STAFMutexSemLock lock(*pData->fMapSem);
    
    STAFString upperCaseMachine = 
        ((STAFString)(pInfo->machineNickname)).toUpperCase();

    //Write message into the log
    if (pData->fMap[upperCaseMachine].realMachineName == "")
    {
        pData->fMap[upperCaseMachine].realMachineName = 
            pInfo->machineNickname;
    }

    if (parsedResult->optionTimes(sName))
    {
        // Get and resolve NAME option

        STAFResultPtr res = resolveOp(pInfo, pData, parsedResult, sName);

        if (res->rc != kSTAFOk) return res;

        STAFString name = res->result;

        pData->fMap[upperCaseMachine].nameMap[name.toUpperCase()].message =
            fMessage;

        // Store the original case of the name the first time the
        // named monitor is logged to

        if (pData->fMap[upperCaseMachine].nameMap[name.toUpperCase()].
            originalCaseName == "")
        {
            pData->fMap[upperCaseMachine].nameMap[name.toUpperCase()].
                originalCaseName = name;
        }
    }
    else
    {
        pData->fMap[upperCaseMachine].handleMap[pInfo->handle] = fMessage;
    }

    return STAFResultPtr(new STAFResult(kSTAFOk, STAFString()),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleQuery(STAFServiceRequestLevel30 *pInfo, 
                          MonitorServiceData *pData)
{
    // Verify the requesting machine/user has at least trust level 2

    VALIDATE_TRUST(2, pData->fShortName, "QUERY", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fQueryParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
       
    STAFString result = "";

    MachineMap::iterator machineIterator;
    HandleMap::iterator handleIterator;
    NameMap::iterator nameIterator;

    // Get and resolve MACHINE option

    STAFResultPtr res = resolveOp(pInfo, pData, parsedResult, sMachine);
    
    if (res->rc != kSTAFOk) return res;
    
    STAFString queryMachine = res->result;

    // Create the marshalling context and set its map class definitions

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    mc->setMapClassDefinition(pData->fMonitorInfoClass->reference());

    // Get a lock

    STAFMutexSemLock lock(*pData->fMapSem);

    if (parsedResult->optionTimes(sHandle))
    {
        // Get and resolve HANDLE option

        res = resolveOp(pInfo, pData, parsedResult, sHandle);

        if (res->rc != kSTAFOk) return res;
    
        // Convert resolved option string to an unsigned integer in range
        // 1 to UINT_MAX

        unsigned int queryHandle;

        res = convertOptionStringToUInt(res->result, sHandle, queryHandle, 1);

        if (res->rc != kSTAFOk) return res;

        // Find the entry for the specified machine and handle (if one exists)

        if ((machineIterator = pData->fMap.find(queryMachine.toUpperCase())) ==
            pData->fMap.end())
        {
            result = "Entry for machine " + STAFString(queryMachine) +
                     " not found" + sLineSep;

            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, result),
                                 STAFResultPtr::INIT);
        }
        else if ((handleIterator =
              machineIterator->second.handleMap.find(queryHandle)) ==
              machineIterator->second.handleMap.end())
        {
            result = "Entry for handle " + STAFString(queryHandle) +
                " not found" + sLineSep;

            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, result),
                                 STAFResultPtr::INIT);
        }
        else
        {
            // Create a map object to contain the query monitor information

            STAFObjectPtr monitorInfoMap = 
                pData->fMonitorInfoClass->createInstance();

            monitorInfoMap->put("timestamp",
                                handleIterator->second.subString(0, 17));
            monitorInfoMap->put("message",
                                handleIterator->second.subString(18));

            mc->setRootObject(monitorInfoMap);
        }
    }
    else if (parsedResult->optionTimes(sName))
    {
        // Get and resolve NAME option

        res = resolveOp(pInfo, pData, parsedResult, sName);

        if (res->rc != kSTAFOk) return res;

        STAFString queryName = res->result;

        STAFError_e errorCode;

        // Find the entry for the specified machine and name (if one exists)

        if ((machineIterator = pData->fMap.find(queryMachine.toUpperCase())) ==
            pData->fMap.end())
        {
            result = "Entry for machine " + STAFString(queryMachine) +
                     " not found" + sLineSep;
            errorCode = (STAFError_e)kSTAFDoesNotExist;

            return STAFResultPtr(new STAFResult(errorCode, result),
                                 STAFResultPtr::INIT);
        }
        else if ((nameIterator =
               machineIterator->second.nameMap.find(queryName.toUpperCase())) ==
               machineIterator->second.nameMap.end())
        {
            result = "Entry for name " + queryName + " not found" + sLineSep;
            errorCode = (STAFError_e)kSTAFDoesNotExist;

            return STAFResultPtr(new STAFResult(errorCode, result),
                                 STAFResultPtr::INIT);
        }
        else
        {
            // Create a map object to contain the query monitor information

            STAFObjectPtr monitorInfoMap = 
                pData->fMonitorInfoClass->createInstance();

            monitorInfoMap->put("timestamp",
                                nameIterator->second.message.subString(0, 17));
            monitorInfoMap->put("message",
                                nameIterator->second.message.subString(18));

            mc->setRootObject(monitorInfoMap);
        }
    }

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()), 
                         STAFResultPtr::INIT);
}


STAFResultPtr handleList(STAFServiceRequestLevel30 *pInfo, 
                         MonitorServiceData *pData)
{
    // Verify the requesting machine/user has at least trust level 2

    VALIDATE_TRUST(2, pData->fShortName, "LIST", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        pData->fListParser->parse(pInfo->request);
    
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }    
            
    // Create the marshalling context

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    STAFString result = "";

    // Generate output based on the option(s) specified

    if (parsedResult->optionTimes(sSettings))
    {
        // Set the map class definitions for the marshalling context

        mc->setMapClassDefinition(pData->fSettingsClass->reference());

        // Create a map object for the marshalled map of monitor settings

        STAFObjectPtr settingsMap = pData->fSettingsClass->createInstance();

        settingsMap->put("maxRecordSize", STAFString(pData->fMaxRecordSize));

        if (pData->fResolveMessage != 0)
            settingsMap->put("resolveMessage", "Enabled");
        else
            settingsMap->put("resolveMessage", "Disabled");

        if (pData->fEnableResolveMessageVar != 0)
            settingsMap->put("resolveMessageVar", "Enabled");
        else
            settingsMap->put("resolveMessageVar", "Disabled");

        mc->setRootObject(settingsMap);
    }
    else if (parsedResult->optionTimes(sMachines) != 0)
    {
        // Create a empty list object for the marshalled list of machines

        STAFObjectPtr machineList = STAFObject::createList();

        STAFMutexSemLock lock(*pData->fMapSem);
        MachineMap::iterator machineIterator;

        // Test to see if anything has been logged

        if (pData->fMap.begin() == pData->fMap.end())
        {
            result = "No monitor data found." + sLineSep;
            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, result),
                                 STAFResultPtr::INIT);
        }
        else
        {
            // Write all the machine names to result

            for(machineIterator = pData->fMap.begin(); 
                machineIterator != pData->fMap.end(); ++machineIterator)
            {
                machineList->append(machineIterator->second.realMachineName);
            }           
        }

        mc->setRootObject(machineList);
    }
    else if (parsedResult->optionTimes(sMachine) != 0)
    {
        MachineMap::iterator machineIterator;
        STAFMutexSemLock lock(*pData->fMapSem);
        STAFString listMachine = parsedResult->optionValue(sMachine);

        if(listMachine.count(sLeftCurlyBrace) > 0)
        {
            STAFResultPtr machineResult = resolveOp(pInfo, pData, parsedResult,
                                                    "Machine");
            listMachine = machineResult->result;
        }

        if (parsedResult->optionTimes(sNames))
        {
            // Set the map class definitions for the marshalling context

            mc->setMapClassDefinition(pData->fNameInfoClass->reference());

            // Create a empty list object for the marshalled list of names
            // for the specified machine

            STAFObjectPtr nameList = STAFObject::createList();
            
            if ((machineIterator = pData->fMap.find(listMachine.toUpperCase()))
                != pData->fMap.end())
            {
                NameMap::iterator nameIterator;

                for(nameIterator = machineIterator->second.nameMap.begin();
                    nameIterator != machineIterator->second.nameMap.end();
                    ++nameIterator)
                {
                    STAFObjectPtr nameInfoMap =
                        pData->fNameInfoClass->createInstance();

                    nameInfoMap->put("name",
                        nameIterator->second.originalCaseName);
                    nameInfoMap->put("timestamp",
                        nameIterator->second.message.subString(0,17));
                    nameInfoMap->put("size",
                        STAFString(nameIterator->second.message.length()));

                    nameList->append(nameInfoMap);
                }
            }

            mc->setRootObject(nameList);
        }
        else
        {
            // Set the map class definitions for the marshalling context

            mc->setMapClassDefinition(pData->fHandleInfoClass->reference());

            // Create a empty list object for the marshalled list of handles
            // for the specified machine

            STAFObjectPtr handleList = STAFObject::createList();
            
            if ((machineIterator = pData->fMap.find(listMachine.toUpperCase()))
                != pData->fMap.end())
            {
                HandleMap::iterator handleIterator;

                for(handleIterator = machineIterator->second.handleMap.begin();
                    handleIterator != machineIterator->second.handleMap.end();
                    ++handleIterator)
                {
                    STAFObjectPtr handleInfoMap =
                        pData->fHandleInfoClass->createInstance();

                    handleInfoMap->put("handle", handleIterator->first);
                    handleInfoMap->put("timestamp",
                                       handleIterator->second.subString(0,17));
                    handleInfoMap->put("size",
                                   STAFString(handleIterator->second.length()));

                    handleList->append(handleInfoMap);
                }
            }

            mc->setRootObject(handleList);
        }
    }

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()), 
                         STAFResultPtr::INIT);
}


STAFResultPtr handleDelete(STAFServiceRequestLevel30 *pInfo, 
                           MonitorServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requesting machine/user has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "DELETE", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fDeleteParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    if (pData->fMap.begin() == pData->fMap.end())
    {
        result = "There are no entries in the Monitor Log." + sLineSep;
        return STAFResultPtr(new STAFResult(kSTAFOk, result),
                             STAFResultPtr::INIT);
    } 
    else if (parsedResult->optionTimes(sBefore) != 0)
    {        
        MachineMap::iterator machineIterator;
        HandleMap::iterator handleIterator;
        NameMap::iterator nameIterator;
        STAFMutexSemLock lock(*pData->fMapSem);
        STAFString beforeDateString = parsedResult->optionValue(sBefore);                                
        
        try
        {
            if (beforeDateString.isEqualTo(sToday, kSTAFStringCaseInsensitive))
            {   
                beforeDateString = 
                    (STAFTimestamp::now().asString()).subString(0,8);
            }
           
            STAFTimestamp beforeTimeStamp = STAFTimestamp(beforeDateString);            
           
            for(machineIterator = pData->fMap.begin();
                machineIterator != pData->fMap.end(); ++machineIterator)
            {
                for(handleIterator = machineIterator->second.handleMap.begin();
                    handleIterator != machineIterator->second.handleMap.end();
                    )                
                {                      
                    if (STAFTimestamp(handleIterator->second.subString(0,8)) <
                            beforeTimeStamp)
                     machineIterator->second.handleMap.erase(handleIterator++);
                    else                 
                        handleIterator++;                
                }

                for(nameIterator = machineIterator->second.nameMap.begin();
                    nameIterator != machineIterator->second.nameMap.end();
                    )
                {
                    if (STAFTimestamp(
                        nameIterator->second.message.subString(0,8)) <
                        beforeTimeStamp)
                     machineIterator->second.nameMap.erase(nameIterator++);
                    else
                        nameIterator++;
                }
            }
        }
        catch(STAFTimestampInvalidDateException &e)
        {
            result = "Invalid Date format.  YYYYMMDD, MM/DD/YY, or ";
            result += "MM/DD/YYYY" + sLineSep;
            result += getExceptionString(e) + sLineSep;
            return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                 result), STAFResultPtr::INIT);
        }        
        catch (STAFException &e)
        { 
            result = getExceptionString(e,
                    "STAFMonitorSerice.cpp: Delete ").adoptImpl();
            return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                                 result), STAFResultPtr::INIT);                
        }
        catch (...)
        {
            result = "Unknown exception caught in STAFMonitorService ";            
            return STAFResultPtr(new STAFResult(kSTAFUnknownError,
                                 result), STAFResultPtr::INIT);
        }    
    }
    else if (parsedResult->optionTimes(sName) != 0)
    {
        MachineMap::iterator machineIterator;
        NameMap::iterator nameIterator;
        STAFMutexSemLock lock(*pData->fMapSem);

        // Get and resolve MACHINE option

        STAFResultPtr res = resolveOp(pInfo, pData, parsedResult, sMachine);

        if (res->rc != kSTAFOk) return res;

        STAFString deleteMachine = res->result;

        // Get and resolve NAME option

        res = resolveOp(pInfo, pData, parsedResult, sName);

        if (res->rc != kSTAFOk) return res;

        STAFString deleteName = res->result;

        STAFError_e errorCode;

        // Find the entry for the specified machine and name (if one exists)

        if ((machineIterator = pData->fMap.find(deleteMachine.toUpperCase()))
                == pData->fMap.end())
        {
            result = "Entry for machine " + STAFString(deleteMachine) +
                     " not found" + sLineSep;
            errorCode = (STAFError_e)kSTAFDoesNotExist;

            return STAFResultPtr(new STAFResult(errorCode, result),
                                 STAFResultPtr::INIT);
        }
        else if ((nameIterator =
               machineIterator->second.nameMap.find(deleteName.toUpperCase()))
                   == machineIterator->second.nameMap.end())
        {
            result = "Entry for name " + deleteName + " not found" + sLineSep;
            errorCode = (STAFError_e)kSTAFDoesNotExist;

            return STAFResultPtr(new STAFResult(errorCode, result),
                                 STAFResultPtr::INIT);
        }
        else
        {
            machineIterator->second.nameMap.erase(nameIterator);
        }
    }
    else
    {
        pData->fMap.clear();
    }
  
    return STAFResultPtr(new STAFResult(kSTAFOk, STAFString()),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleSet(STAFServiceRequestLevel30 *pInfo, 
                        MonitorServiceData *pData)
{
    // Verify the requesting machine/user has at least trust level 5

    VALIDATE_TRUST(5, pData->fShortName, "SET", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fSetParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    if (parsedResult->optionTimes(sMaxRecordSize) != 0)
    {          
        STAFResultPtr maxResult = resolveOp(pInfo, pData, parsedResult,
                                            sMaxRecordSize);

        if (maxResult->rc != kSTAFOk) return maxResult;

        // Convert resolved option string to an unsigned integer in range
        // 0 to UINT_MAX

        maxResult = convertOptionStringToUInt(
            maxResult->result, sMaxRecordSize, pData->fMaxRecordSize);

        if (maxResult->rc != kSTAFOk) return maxResult;
    }
    
    if (parsedResult->optionTimes(sResolveMessage) != 0)
    {
        pData->fResolveMessage = 1;
    }
    else if (parsedResult->optionTimes(sNoResolveMessage) != 0)
    {
        pData->fResolveMessage = 0;
    }

    if (parsedResult->optionTimes(sEnableResolveMessageVar) != 0)
    {
        pData->fEnableResolveMessageVar = 1;
    }
    else if (parsedResult->optionTimes(sDisableResolveMessageVar) != 0)
    {
        pData->fEnableResolveMessageVar = 0;
    }    
    
    return STAFResultPtr(new STAFResult(kSTAFOk, ""), STAFResultPtr::INIT);
}


STAFResultPtr handleHelp(STAFServiceRequestLevel30 *pInfo, 
                         MonitorServiceData *pData)
{
    // Verify the requesting machine/user has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "HELP", pData->fLocalMachineName);

    // Return the help text

    return STAFResultPtr(new STAFResult(kSTAFOk, sHelpMsg),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleVersion(STAFServiceRequestLevel30 *pInfo, 
                            MonitorServiceData *pData)
{
    // Verify the requesting machine/user has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "VERSION", pData->fLocalMachineName);

    return STAFResultPtr(new STAFResult(kSTAFOk, monitorVersion), 
                         STAFResultPtr::INIT);
}


STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
                         MonitorServiceData *pData, 
                         const STAFString &theString)
{
    return pData->fHandlePtr->submit(sLocal, sVar, sResStrResolve +
                                     STAFString(pInfo->requestNumber) +
                                     sString +
                                     pData->fHandlePtr->wrapData(theString));
}


STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, 
                        MonitorServiceData *pData,
                        STAFCommandParseResultPtr &parsedResult,
                        const STAFString &fOption, unsigned int optionIndex)
{
    // ???: Would const STAFString & work here?
    STAFString optionValue = parsedResult->optionValue(fOption, optionIndex);

    if (optionValue.find(sLeftCurlyBrace) == STAFString::kNPos)
    {
        return STAFResultPtr(new STAFResult(kSTAFOk, optionValue),
                             STAFResultPtr::INIT);
    }

    return resolveStr(pInfo, pData, optionValue);
}


STAFResultPtr resolveOpLocal(MonitorServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption,
                             unsigned int optionIndex)
{
    // ???: Would const STAFString & work here?
    STAFString optionValue = parsedResult->optionValue(fOption, optionIndex);

    if (optionValue.find(sLeftCurlyBrace) == STAFString::kNPos)
    {
        return STAFResultPtr(new STAFResult(kSTAFOk, optionValue),
                             STAFResultPtr::INIT);
    }

    return pData->fHandlePtr->submit(sLocal, sVar, sResStrResolve +
                                      sString +
                                      pData->fHandlePtr->wrapData(optionValue));
}


STAFResultPtr convertOptionStringToUInt(const STAFString &theString,
                                        const STAFString &optionName,
                                        unsigned int &number,
                                        const unsigned int minValue,
                                        const unsigned int maxValue)
{
    // Convert an option value to an unsigned integer

    STAFString_t errorBufferT = 0;

    STAFRC_t rc = STAFUtilConvertStringToUInt(
        theString.getImpl(), optionName.getImpl(), &number,
        &errorBufferT, minValue, maxValue);

    if (rc == kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(), STAFResultPtr::INIT);
    }
    else
    {
        return STAFResultPtr(
            new STAFResult(rc, STAFString(errorBufferT, STAFString::kShallow)),
            STAFResultPtr::INIT);
    }
}
