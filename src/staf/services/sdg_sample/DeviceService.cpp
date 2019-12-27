/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <deque>
#include <map>
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFUtil.h"
#include "DeviceService.h"

// Device Data - contains data for a device

struct DeviceData
{
    DeviceData()
    { /* Do Nothing */ }

    DeviceData(const STAFString &aName, const STAFString &aModel,
               const STAFString &aSN) :
         name(aName), model(aModel), serialNumber(aSN)
    { /* Do Nothing */ }

    STAFString   name;            // Device name
    STAFString   model;           // Device model
    STAFString   serialNumber;    // Device serial number
};

typedef std::deque<DeviceData> DeviceList;

typedef STAFRefPtr<DeviceData> DeviceDataPtr;

// DeviceMap -- KEY:   Device name,
//              VALUE: Pointer to DeviceData information

typedef std::map<STAFString, DeviceDataPtr> DeviceMap;

// DEVICE Service Data

struct DeviceServiceData
{
    unsigned int  fDebugMode;              // Debug Mode flag
    STAFString    fShortName;              // Short service name
    STAFString    fName;                   // Registered service name    
    STAFHandlePtr fHandlePtr;              // Device service's STAF handle
    STAFString    fLocalMachineName;       // Local machine name
    STAFCommandParserPtr fAddParser;       // DEVICE ADD command parser
    STAFCommandParserPtr fDeleteParser;    // DEVICE DELETE command parser
    STAFCommandParserPtr fQueryParser;     // DEVICE QUERY command parser
    STAFCommandParserPtr fListParser;      // DEVICE LIST command parser
    STAFCommandParserPtr fHelpParser;      // DEVICE HELP command parser
    STAFCommandParserPtr fVersionParser;   // DEVICE VERSION command parser
    STAFCommandParserPtr fParmsParser;     // DEVIC PARMS command parser

    // Map Class Definitions for marshalled results
    STAFMapClassDefinitionPtr fListDeviceMapClass; 
    STAFMapClassDefinitionPtr fQueryDeviceMapClass;

    STAFMutexSemPtr      fPrinterMapSem;   // Semaphore to control 
                                           //   access to the PrinterMap
    STAFMutexSemPtr      fModemMapSem;     // Semaphore to control 
                                           //   access to the ModemMap
    DeviceMap            fPrinterMap;      // Map of all printers
    DeviceMap            fModemMap;        // Map of all modems
};

typedef STAFRefPtr<DeviceData> DeviceServiceDataPtr;

// Static Variables

static STAFString sHelpMsg;
static STAFString sLineSep;
static const STAFString sVersionInfo("3.4.0");
static const STAFString sLocal("local");
static const STAFString sHelp("HELP");
static const STAFString sVar("VAR");
static const STAFString sResStrResolve("RESOLVE REQUEST ");
static const STAFString sString(" STRING ");
static const STAFString sLeftCurlyBrace(kUTF8_LCURLY);

// Prototypes

static STAFResultPtr handleAdd(STAFServiceRequestLevel30 *,
                               DeviceServiceData *);
static STAFResultPtr handleDelete(STAFServiceRequestLevel30 *,
                                  DeviceServiceData *);
static STAFResultPtr handleQuery(STAFServiceRequestLevel30 *,
                                 DeviceServiceData *);
static STAFResultPtr handleList(STAFServiceRequestLevel30 *,
                                DeviceServiceData *);
static STAFResultPtr handleHelp(STAFServiceRequestLevel30 *,
                                DeviceServiceData *);
static STAFResultPtr handleVersion(STAFServiceRequestLevel30 *,
                                   DeviceServiceData *);

static STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo, 
                                DeviceServiceData *pData,
                                const STAFString &theString);

static STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, 
                               DeviceServiceData *pData,
                               STAFCommandParseResultPtr &parsedResult,
                               const STAFString &fOption,
                               unsigned int optionIndex = 1);

STAFResultPtr resolveOpLocal(DeviceServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption,
                             unsigned int optionIndex = 1);

static void registerHelpData(DeviceServiceData *pData,
                             unsigned int errorNumber,
                             const STAFString &shortInfo,
                             const STAFString &longInfo);

static void unregisterHelpData(DeviceServiceData *pData,
                               unsigned int errorNumber);

// Begin implementation

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

        DeviceServiceData data;
        data.fDebugMode = 0;
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

        *pServiceHandle = new DeviceServiceData(data);

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString result;
        
        result += STAFString("In DeviceService.cpp: STAFServiceConstruct")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("DeviceService.cpp: STAFServiceConstruct: "
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

        DeviceServiceData *pData =
            reinterpret_cast<DeviceServiceData *>(serviceHandle);
        
        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);        

        retCode = STAFHandle::create(pData->fName, pData->fHandlePtr);

        if (retCode != kSTAFOk)
            return retCode;
        
        //ADD options

        pData->fAddParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fAddParser->addOption("ADD", 1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fAddParser->addOption("PRINTER", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("MODEM", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("MODEL", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("SN", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOptionNeed("PRINTER MODEM", "ADD");
        pData->fAddParser->addOptionNeed("ADD", "PRINTER MODEM");
        pData->fAddParser->addOptionNeed("ADD", "MODEL");
        pData->fAddParser->addOptionNeed("ADD", "SN");
        pData->fAddParser->addOptionGroup("PRINTER MODEM", 0, 1);
        
        //DELETE options

        pData->fDeleteParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fDeleteParser->addOption("DELETE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOption("PRINTER", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("MODEM",  1,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("CONFIRM", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOptionGroup("PRINTER MODEM", 0, 1);
        pData->fDeleteParser->addOptionNeed("PRINTER MODEM", "DELETE");
        pData->fDeleteParser->addOptionNeed("DELETE", "PRINTER MODEM");
        pData->fDeleteParser->addOptionNeed("DELETE", "CONFIRM");
        
        //QUERY options

        pData->fQueryParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fQueryParser->addOption("QUERY", 1,
                                       STAFCommandParser::kValueNotAllowed);
        pData->fQueryParser->addOption("PRINTER", 1,
                                       STAFCommandParser::kValueRequired);
        pData->fQueryParser->addOption("MODEM", 1,
                                       STAFCommandParser::kValueRequired);
        pData->fQueryParser->addOptionGroup("PRINTER MODEM", 0, 1);
        pData->fQueryParser->addOptionNeed("PRINTER MODEM", "QUERY");
        pData->fQueryParser->addOptionNeed("QUERY", "PRINTER MODEM");

        //LIST options

        pData->fListParser = STAFCommandParserPtr(new STAFCommandParser,
                                                  STAFCommandParserPtr::INIT);
        pData->fListParser->addOption("LIST", 1,
                                       STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("PRINTERS", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("MODEMS", 1,
                                      STAFCommandParser::kValueNotAllowed);

        //HELP options

        pData->fHelpParser = STAFCommandParserPtr(new STAFCommandParser,
                                                  STAFCommandParserPtr::INIT);
        pData->fHelpParser->addOption("HELP", 1,
                                      STAFCommandParser::kValueNotAllowed);

        //VERSION options

        pData->fVersionParser = STAFCommandParserPtr(new STAFCommandParser,
                                                  STAFCommandParserPtr::INIT);
        pData->fVersionParser->addOption("VERSION", 1,
                                         STAFCommandParser::kValueNotAllowed);

        // Construct map class for the result from a LIST request.

        pData->fListDeviceMapClass = STAFMapClassDefinition::create(
            "STAF/Service/Device/ListDevice");

        pData->fListDeviceMapClass->addKey("name",    "Name");
        pData->fListDeviceMapClass->addKey("type",    "Type");
        pData->fListDeviceMapClass->addKey("model",   "Model");
        pData->fListDeviceMapClass->addKey("serial#", "Serial Number");
        pData->fListDeviceMapClass->setKeyProperty(
            "serial#", "display-short-name", "Serial #");

        // Construct map class for the result from a QUERY request.

        pData->fQueryDeviceMapClass = STAFMapClassDefinition::create(
            "STAF/Service/Device/QueryDevice");

        pData->fQueryDeviceMapClass->addKey("model",   "Model");
        pData->fQueryDeviceMapClass->addKey("serial#", "Serial Number");

        // Resolve the line separator variable for the local machine

        STAFResultPtr result = pData->fHandlePtr->submit(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Sep/Line}");

        if (result->rc != 0)
        {
            *pErrorBuffer = result->result.adoptImpl();
            return result->rc;
        }
        else sLineSep = result->result;
        
        // Resolve the machine name variable for the local machine

        result = pData->fHandlePtr->submit(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Machine}");

        if (result->rc != 0)
        {
            *pErrorBuffer = result->result.adoptImpl();
            return result->rc;
        }
        else pData->fLocalMachineName = result->result;
        
        // Create mutex semaphores for the printer and modem data maps

        pData->fPrinterMapSem = STAFMutexSemPtr(
            new STAFMutexSem, STAFMutexSemPtr::INIT);
        pData->fModemMapSem = STAFMutexSemPtr(
            new STAFMutexSem, STAFMutexSemPtr::INIT);

        // Assign the help text string for the service

        sHelpMsg = STAFString("*** ") + pData->fShortName + " Service Help ***" +
            sLineSep + sLineSep +
            "ADD     < PRINTER <PrinterName> | MODEM <ModemName> > MODEL <Model> SN <Serial#>" +
            sLineSep +
            "DELETE  < PRINTER <printerName> | MODEM <ModemName> > CONFIRM" +
            sLineSep +
            "LIST    [PRINTERS] [MODEMS]" +
            sLineSep +
            "QUERY   PRINTER <PrinterName> | MODEM <ModemName>" +
            sLineSep +
            "VERSION" +
            sLineSep +
            "HELP";

        // Register Help Data

        registerHelpData(pData, kDeviceInvalidSerialNumber,
            STAFString("Invalid serial number"),
            STAFString("A non-numeric value was specified for serial number"));
    }

    catch (STAFException &e)
    {
        STAFString result;
        
        result += STAFString("In DeviceService.cpp: STAFServiceInit")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("DeviceService.cpp: STAFServiceInit: "
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
        STAFResultPtr result(new STAFResult(),
                             STAFResultPtr::INIT);        

        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

        DeviceServiceData *pData =
            reinterpret_cast<DeviceServiceData *>(serviceHandle);

        // Determine the command request (the first word in the request)

        STAFString request(pInfo->request);
        STAFString action = request.subWord(0, 1).toLowerCase();
        
        // Call functions for the request

        if (action == "add")
            result = handleAdd(pInfo, pData);
        else if (action == "delete")
            result = handleDelete(pInfo, pData);
        else if (action == "query")
            result = handleQuery(pInfo, pData);
        else if (action == "list")
            result = handleList(pInfo, pData);
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

        STAFString result;
        
        result += STAFString("In DeviceService.cpp: STAFServiceAcceptRequest")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pResultBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("DeviceService.cpp: STAFServiceAcceptRequest: "
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

        DeviceServiceData *pData =
            reinterpret_cast<DeviceServiceData *>(serviceHandle);

        // Un-register Help Data

        unregisterHelpData(pData, kDeviceInvalidSerialNumber);
    }
    catch (STAFException &e)
    { 
        STAFString result;
        
        result += STAFString("In DeviceService.cpp: STAFServiceTerm")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("DeviceService.cpp: STAFServiceTerm: "
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
        DeviceServiceData *pData =
            reinterpret_cast<DeviceServiceData *>(*serviceHandle);

        delete pData;
        *serviceHandle = 0;

        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    { 
        STAFString result;
        
        result += STAFString("In DeviceService.cpp: STAFServiceDestruct")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("DevicePoolService.cpp: STAFServiceDestruct: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


// Handles device add entry requests

STAFResultPtr handleAdd(STAFServiceRequestLevel30 *pInfo, 
                        DeviceServiceData *pData)
{
    // Verify the requester has at least trust level 3

    VALIDATE_TRUST(3, pData->fShortName, "ADD", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fAddParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // Resolve any STAF variables in the printer option's value
   
    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "PRINTER");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString printer = resultPtr->result;

    // Resolve any STAF variables in the modem option's value
   
    resultPtr = resolveOp(pInfo, pData, parsedResult, "MODEM");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString modem = resultPtr->result;
    
    // Resolve any STAF variables in the model option's value

    resultPtr = resolveOp(pInfo, pData, parsedResult, "MODEL");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString model = resultPtr->result;
    
    // Resolve any STAF variables in the sn option's value

    resultPtr = resolveOp(pInfo, pData, parsedResult, "SN");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString serialNumber = resultPtr->result;

    // Verify that the serial number is numeric
    
    if (!serialNumber.isDigits())
    {
        // Note that instead of creating a new error code specific for
        // this service, could use kSTAFInvalidValue instead.
        return STAFResultPtr(
            new STAFResult(kDeviceInvalidSerialNumber, serialNumber), 
            STAFResultPtr::INIT);
    }
    
    // Add the device to the printer map or the modem map and
    // write an informational message to the service log

    if (printer != "")
    {
        STAFMutexSemLock lock(*pData->fPrinterMapSem);
        
        pData->fPrinterMap.insert(DeviceMap::value_type(printer,
            DeviceDataPtr(new DeviceData(printer, model, serialNumber), 
            DeviceDataPtr::INIT)));

        STAFString logMsg = "ADD PRINTER request.  Name=" + printer +
                            " Model=" + model + " Serial#=" + serialNumber;

        pData->fHandlePtr->submit(
            sLocal, "LOG", "LOG MACHINE LOGNAME " + pData->fShortName +
            " LEVEL info MESSAGE " + pData->fHandlePtr->wrapData(logMsg));
    }
    else if (modem != "")
    {
        STAFMutexSemLock lock(*pData->fModemMapSem);
        
        pData->fModemMap.insert(DeviceMap::value_type(modem,
            DeviceDataPtr(new DeviceData(modem, model, serialNumber), 
            DeviceDataPtr::INIT)));

        STAFString logMsg = "ADD MODEM request.  Name=" + modem +
                            " Model=" + model + " Serial#=" + serialNumber;

        pData->fHandlePtr->submit(
            sLocal, "LOG", "LOG MACHINE LOGNAME " + pData->fShortName +
            " LEVEL info MESSAGE " + pData->fHandlePtr->wrapData(logMsg));
    }

    // Return an Ok result
    
    return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
}


// Handles device deletion requests

STAFResultPtr handleDelete(STAFServiceRequestLevel30 *pInfo, 
                           DeviceServiceData *pData)
{
    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "DELETE", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fDeleteParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
   
    // Resolve any STAF variables in the print option's value

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "PRINTER");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString printer = resultPtr->result;
    
    // Resolve any STAF variables in the modem option's value

    resultPtr = resolveOp(pInfo, pData, parsedResult, "MODEM");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString modem = resultPtr->result;

    // Find the device in the printer or modem map and remove it and
    // write an informational message to the service log

    if (printer != "")
    {
        STAFMutexSemLock lock(*pData->fPrinterMapSem);
        
        pData->fPrinterMap.erase(printer);

        STAFString logMsg = "DELETE PRINTER request.  Name=" + printer;

        pData->fHandlePtr->submit(
            sLocal, "LOG", "LOG MACHINE LOGNAME " + pData->fShortName +
            " LEVEL info MESSAGE " + pData->fHandlePtr->wrapData(logMsg));
    }
    else if (modem != "")
    {
        STAFMutexSemLock lock(*pData->fModemMapSem);
        
        pData->fModemMap.erase(modem);

        STAFString logMsg = "DELETE MODEM request.  Name=" + modem;

        pData->fHandlePtr->submit(
            sLocal, "LOG", "LOG MACHINE LOGNAME " + pData->fShortName +
            " LEVEL info MESSAGE " + pData->fHandlePtr->wrapData(logMsg));
    }    

    // Return an Ok result

    return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
}


// Handles device list requests

STAFResultPtr handleList(STAFServiceRequestLevel30 *pInfo, 
                         DeviceServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 2

    VALIDATE_TRUST(2, pData->fShortName, "LIST", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fListParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Check if specified printers or modems

    bool all = false;
    bool printers = false;
    bool modems = false;

    if (!(parsedResult->optionTimes("PRINTERS")) &&
        !(parsedResult->optionTimes("MODEMS")))
    {
        all = true;
    }
    
    if (parsedResult->optionTimes("PRINTERS"))
    {
        printers = true;
    }
    
    if (parsedResult->optionTimes("MODEMS"))
    {
        modems = true;
    }
    
    // Create a marshalling context and set any map classes (if any).

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(pData->fListDeviceMapClass->reference());

    // Create an empty result list to contain the result

    STAFObjectPtr resultList = STAFObject::createList();

    // Add printer entries to the result list

    if (printers || all)
    {
        STAFMutexSemLock lock(*pData->fPrinterMapSem);
        
        DeviceMap::iterator iter;

        for (iter = pData->fPrinterMap.begin(); 
             iter != pData->fPrinterMap.end(); ++iter)
        {
            STAFObjectPtr resultMap = pData->fListDeviceMapClass->createInstance();
            resultMap->put("name",    iter->second->name);
            resultMap->put("type",    "Printer");
            resultMap->put("model",   iter->second->model);
            resultMap->put("serial#", iter->second->serialNumber);

            resultList->append(resultMap);
        }
    }
    
    // Add modem entries to the result list

    if (modems || all)
    {
        STAFMutexSemLock lock(*pData->fModemMapSem);
        
        DeviceMap::iterator iter;

        for (iter = pData->fModemMap.begin();
             iter != pData->fModemMap.end(); ++iter)
        {
            STAFObjectPtr resultMap = pData->fListDeviceMapClass->createInstance();
            resultMap->put("name",    iter->second->name);
            resultMap->put("type",    "Modem");
            resultMap->put("model",   iter->second->model);
            resultMap->put("serial#", iter->second->serialNumber);

            resultList->append(resultMap);
        }
    }    

    // Set the result list as the root object for the marshalling context
    // and return the marshalled result

    mc->setRootObject(resultList);

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()),
                         STAFResultPtr::INIT);
}


// Handles device query requests

STAFResultPtr handleQuery(STAFServiceRequestLevel30 *pInfo, 
                          DeviceServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;
    
    // Verify the requester has at least trust level 2

    VALIDATE_TRUST(2, pData->fShortName, "QUERY", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fQueryParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Resolve any STAF variables in the printer option's value

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "PRINTER");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString printer = resultPtr->result;
    
    // Resolve any STAF variables in the modem option's value

    resultPtr = resolveOp(pInfo, pData, parsedResult, "MODEM");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString modem = resultPtr->result;
    
    // Create a marshalling context and set any map classes (if any).

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(pData->fQueryDeviceMapClass->reference());

    // Create an empty result map to contain the result

    STAFObjectPtr resultMap = pData->fQueryDeviceMapClass->createInstance();

    // Find the specified printer/modem and add its info to the result map

    if (printer != "")
    {
        STAFMutexSemLock lock(*pData->fPrinterMapSem);
        
        DeviceMap::iterator iter = pData->fPrinterMap.find(printer); 
        
        if (iter == pData->fPrinterMap.end())
        {
            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, printer),
                                 STAFResultPtr::INIT);
        }
        
        resultMap->put("model",   iter->second->model);
        resultMap->put("serial#", iter->second->serialNumber);
    }
    else if (modem != "")
    {
        STAFMutexSemLock lock(*pData->fModemMapSem);
        
        DeviceMap::iterator iter = pData->fModemMap.find(modem);

        if (iter == pData->fModemMap.end())
        {
            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, modem),
                                 STAFResultPtr::INIT);
        }

        resultMap->put("model",   iter->second->model);
        resultMap->put("serial#", iter->second->serialNumber);
    }

    // Set the result map as the root object for the marshalling context
    // and return the marshalled result

    mc->setRootObject(resultMap);

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()), 
                         STAFResultPtr::INIT);
}


STAFResultPtr handleHelp(STAFServiceRequestLevel30 *pInfo, 
                         DeviceServiceData *pData)
{    
    // Verify the requester has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "HELP", pData->fLocalMachineName);
    
    // Return help text for the service

    return STAFResultPtr(new STAFResult(kSTAFOk, sHelpMsg),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleVersion(STAFServiceRequestLevel30 *pInfo, 
                            DeviceServiceData *pData)
{    
    // Verify the requester has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "VERSION", pData->fLocalMachineName);
    
    // Return the version of the service

    return STAFResultPtr(new STAFResult(kSTAFOk, sVersionInfo), 
                         STAFResultPtr::INIT);
}


STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, 
                        DeviceServiceData *pData,
                        STAFCommandParseResultPtr &parsedResult,
                        const STAFString &fOption, unsigned int optionIndex)
{
    STAFString optionValue = parsedResult->optionValue(fOption, optionIndex);

    if (optionValue.find(sLeftCurlyBrace) == STAFString::kNPos)
    {
        return STAFResultPtr(new STAFResult(kSTAFOk, optionValue),
                             STAFResultPtr::INIT);
    }

    return resolveStr(pInfo, pData, optionValue);
}


STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
                         DeviceServiceData *pData, 
                         const STAFString &theString)
{
    return pData->fHandlePtr->submit(sLocal, sVar, sResStrResolve +
                                     STAFString(pInfo->requestNumber) +
                                     sString +
                                     pData->fHandlePtr->wrapData(theString));
}


void registerHelpData(DeviceServiceData *pData, unsigned int errorNumber,
                      const STAFString &shortInfo, const STAFString &longInfo)
{
    static STAFString regString("REGISTER SERVICE %C ERROR %d INFO %C "
                                "DESCRIPTION %C");

    pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber,
        shortInfo.getImpl(), longInfo.getImpl()));
}


void unregisterHelpData(DeviceServiceData *pData, unsigned int errorNumber)
{
    static STAFString regString("UNREGISTER SERVICE %C ERROR %d");

    pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber));
}
