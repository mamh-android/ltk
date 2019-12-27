/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFTrace.h"
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFUtil.h"
#include "STAFFileSystem.h"

#include <vector>
#include <map>

#include "zlib.h"

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipFileHeader.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipLocalFileHeader.h"
#include "STAFZipCentralDirEndRecord.h"
#include "STAFZipCentralDir.h"
#include "STAFZipFile.h"
#include "STAFZipMutexLock.h"



// STAFZip Service Data
struct STAFZipServiceData
{
    unsigned int  fDebugMode;              // Debug Mode flag
    STAFString    fShortName;              // Short service name
    STAFString    fName;                   // Registered service name
    STAFString    fLocalMachineName;       // Logical identifier for the local
                                           //   machine
    STAFHandlePtr fHandlePtr;              // STAFZip service's STAF handle
    STAFCommandParserPtr fAddParser;       // STAFZip ADD command parser
    STAFCommandParserPtr fUnzipParser;     // STAFZip UNZIP command parser
    STAFCommandParserPtr fListParser;      // STAFZip LIST command parser
    STAFCommandParserPtr fDeleteParser;    // STAFZip DELETE command parser
    STAFCommandParserPtr fHelpParser;      // STAFZip HELP command parser
    STAFCommandParserPtr fVersionParser;   // STAFZip VERSION command parser
};

// static STAFZipMutexLock class
static STAFZipMutexLock zipLock;


// Static Variables

static STAFString sHelpMsg;
static STAFString sLineSep;
static const STAFString sVersionInfo("3.4.1");
// 3.0.0    Original release
// 3.0.1    Bug 1012202: Can't read permission info in the latest InfoZip archive    
// 3.0.2    Bug 1033654: Error inflate file    
// 3.0.3    Feature 1055682: Improve unzip's performance on large files
// 3.0.4    Bug 1076948: zero bytes when unzipping JAR archives
// 3.0.5    Feature 1084669: Move Zip archive handling out of STAFZipFile class
//          Bug 1084676: unable to unzip symbolic link on SuSE Linux
// 3.3.0    Feature 2637949: Provides support for zip files >= 2G
// ...
// 3.4.1    Bug 3530590: Zip service does not preserve all files' permissions
//          on Unix 64-bit systems when using the RESTOREPERMISSIONS option

static const STAFString sLocal("local");
static const STAFString sHelp("help");
static const STAFString sVar("var");
static const STAFString sResStrResolve("RESOLVE REQUEST ");
static const STAFString sString(" STRING ");
static const STAFString sLeftCurlyBrace(kUTF8_LCURLY);

// Prototypes

static STAFResultPtr handleAdd(STAFServiceRequestLevel30 *,
                               STAFZipServiceData *, STAFString *);
static STAFResultPtr handleUnzip(STAFServiceRequestLevel30 *,
                                  STAFZipServiceData *, STAFString *);
static STAFResultPtr handleList(STAFServiceRequestLevel30 *,
                                  STAFZipServiceData *, STAFString *);
static STAFResultPtr handleDelete(STAFServiceRequestLevel30 *,
                                  STAFZipServiceData *, STAFString *);
static STAFResultPtr handleHelp(STAFServiceRequestLevel30 *,
                                STAFZipServiceData *);
static STAFResultPtr handleVersion(STAFServiceRequestLevel30 *,
                                   STAFZipServiceData *);

static STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
                                STAFZipServiceData *pData,
                                const STAFString &theString);

static STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo,
                               STAFZipServiceData *pData,
                               STAFCommandParseResultPtr &parsedResult,
                               const STAFString &fOption,
                               unsigned int optionIndex = 1);

STAFResultPtr resolveOpLocal(STAFZipServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption,
                             unsigned int optionIndex = 1);

static void registerHelpData(STAFZipServiceData *pData,
                             unsigned int errorNumber,
                             const STAFString &shortInfo,
                             const STAFString &longInfo);

static void unregisterHelpData(STAFZipServiceData *pData,
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

        STAFZipServiceData data;
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

        *pServiceHandle = new STAFZipServiceData(data);

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString result;

        result += STAFString("In STAFZip.cpp: STAFServiceConstruct")
            + kUTF8_SCOLON;

        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFZip.cpp: STAFServiceConstruct: "
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

        STAFZipServiceData *pData =
            reinterpret_cast<STAFZipServiceData *>(serviceHandle);

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
        // This option has been deprecated
        pData->fAddParser->addOption("ZIP", 1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fAddParser->addOption("ZIPFILE", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("FILE", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("DIRECTORY", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("RELATIVETO", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("RECURSE", 1,
                                     STAFCommandParser::kValueNotAllowed);

        pData->fAddParser->addOptionNeed("ADD", "ZIPFILE");
        pData->fAddParser->addOptionNeed("RELATIVETO", "ADD");
        pData->fAddParser->addOptionNeed("RELATIVETO", "ZIPFILE");
        pData->fAddParser->addOptionNeed("RELATIVETO", "ADD");
        pData->fAddParser->addOptionNeed("RECURSE", "ADD");
        pData->fAddParser->addOptionNeed("RECURSE", "ZIPFILE");
        pData->fAddParser->addOptionNeed("RECURSE", "DIRECTORY");
        pData->fAddParser->addOptionNeed("ADD", "FILE DIRECTORY");
        pData->fAddParser->addOptionNeed("FILE DIRECTORY", "ADD");
        pData->fAddParser->addOptionGroup("FILE DIRECTORY", 0, 1);

        //UNZIP options
        pData->fUnzipParser = STAFCommandParserPtr(new STAFCommandParser,
                                                   STAFCommandParserPtr::INIT);
        pData->fUnzipParser->addOption("UNZIP", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fUnzipParser->addOption("ZIPFILE", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fUnzipParser->addOption("FILE", 0,
                                        STAFCommandParser::kValueRequired);
        pData->fUnzipParser->addOption("DIRECTORY", 0,
                                        STAFCommandParser::kValueRequired);
        pData->fUnzipParser->addOption("TODIRECTORY",  1,
                                        STAFCommandParser::kValueRequired);
        pData->fUnzipParser->addOption("REPLACE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fUnzipParser->addOption("RESTOREPERMISSION", 1,
                                        STAFCommandParser::kValueNotAllowed);

        pData->fUnzipParser->addOptionNeed("UNZIP", "ZIPFILE");
        pData->fUnzipParser->addOptionNeed("UNZIP", "TODIRECTORY");
        pData->fUnzipParser->addOptionNeed("FILE DIRECTORY", "UNZIP");
        pData->fUnzipParser->addOptionNeed("FILE DIRECTORY", "ZIPFILE");
        pData->fUnzipParser->addOptionNeed("FILE DIRECTORY", "TODIRECTORY");

        pData->fUnzipParser->addOptionNeed("REPLACE", "UNZIP");
        pData->fUnzipParser->addOptionNeed("REPLACE", "ZIPFILE");
        pData->fUnzipParser->addOptionNeed("REPLACE", "TODIRECTORY");

        pData->fUnzipParser->addOptionNeed("RESTOREPERMISSION", "UNZIP");
        pData->fUnzipParser->addOptionNeed("RESTOREPERMISSION", "ZIPFILE");
        pData->fUnzipParser->addOptionNeed("RESTOREPERMISSION", "TODIRECTORY");


        //LIST options
        pData->fListParser = STAFCommandParserPtr(new STAFCommandParser,
                                                   STAFCommandParserPtr::INIT);
        pData->fListParser->addOption("LIST", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("ZIPFILE", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fListParser->addOptionNeed("LIST", "ZIPFILE");


        //DELETE options
        pData->fDeleteParser = STAFCommandParserPtr(new STAFCommandParser,
                                                   STAFCommandParserPtr::INIT);
        pData->fDeleteParser->addOption("DELETE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOption("ZIPFILE", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("FILE",  0,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("CONFIRM", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOptionNeed("DELETE", "ZIPFILE");
        pData->fDeleteParser->addOptionNeed("DELETE", "FILE");
        pData->fDeleteParser->addOptionNeed("DELETE", "CONFIRM");


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

        // Get line separator
        
        STAFResultPtr result = pData->fHandlePtr->submit(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Sep/Line}");
        if (result->rc != 0)
        {
            *pErrorBuffer = result->result.adoptImpl();
            return result->rc;
        }
        else sLineSep = result->result;

        // Get local machine name (logical identifier)

        result = pData->fHandlePtr->submit(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Machine}");

        if (result->rc != 0)
        {
            *pErrorBuffer = result->result.adoptImpl();
            return result->rc;
        }
        else pData->fLocalMachineName = result->result;

        // Assign the help text string for the service

        sHelpMsg = STAFString("*** ") + pData->fShortName + " Service Help ***" +
            sLineSep + sLineSep +
            "UNZIP  ZIPFILE <Name> TODIRECTORY <Name>" +
            sLineSep +
            "       [FILE <Name>]... [DIRECTORY <Name>]..." +
            sLineSep +
            "       [RESTOREPERMISSION] [REPLACE]" +
            sLineSep + sLineSep +
            "ADD    ZIPFILE <Name> < FILE <Name> | DIRECTORY <Name> [RECURSE] >" +
            sLineSep +
            "       [RELATIVETO <Directory>]" +
            sLineSep + sLineSep +
            "DELETE ZIPFILE <Name> FILE <Name> [FILE <Name>]... CONFIRM" +
            sLineSep + sLineSep +
            "LIST   ZIPFILE <Name>" +
            sLineSep + sLineSep +
            "VERSION" +
            sLineSep + sLineSep +
            "HELP";
        
        // Note: The ADD request syntax is preferred over the ZIP ADD request
        // syntax (so only documented the ADD request in the above help text).

        // Register help information for service errors

        registerHelpData(pData, kZIPGeneralZipError,
            STAFString("General Zip Error"),
            STAFString("ZIP service returns a general error "));

        registerHelpData(pData, kZIPNotEnoughMemory,
            STAFString("Not Enought Memory"),
            STAFString("There is not enough memory in system "));

        registerHelpData(pData, kZIPChangeFileSizeError,
            STAFString("Change File Size Error"),
            STAFString("Change file size is unsuccessful "));

        registerHelpData(pData, kZIPErrorCreatingDir,
            STAFString("Error Creating Dir"),
            STAFString("Creating dir is unsuccessful "));

        registerHelpData(pData, kZIPInvalidZipFile,
            STAFString("Invalid Zip File"),
            STAFString("The Zip archive has an invalid format "));

        registerHelpData(pData, kZIPBadCRC,
            STAFString("Bad CRC"),
            STAFString("Bad CRC in the Zip archive "));

        registerHelpData(pData, kZIPInvalidOwnerGroup,
            STAFString("Invalid Owner Group"),
            STAFString("Owner or Group does not exist in system "));

        registerHelpData(pData, kZIPInvalidFileMode,
            STAFString("Invalid File Mode"),
            STAFString("Cannot set the file's mode (permission) in system "));

    }
    catch (STAFException &e)
    {
        STAFString result;

        result += STAFString("In STAFZip.cpp: STAFServiceInit")
            + kUTF8_SCOLON;

        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFZip.cpp: STAFServiceInit: "
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

    STAFString mutexName("STAFZIPMUTEX:");

    try
    {
        STAFResultPtr result(new STAFResult(),
                             STAFResultPtr::INIT);

        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

        STAFZipServiceData *pData =
            reinterpret_cast<STAFZipServiceData *>(serviceHandle);

        STAFString request(pInfo->request);
        STAFString action = request.subWord(0, 1).toLowerCase();

        // Call functions for the request

        if (action == "add" || action == "zip")
            result = handleAdd(pInfo, pData, &mutexName);
        else if (action == "unzip")
            result = handleUnzip(pInfo, pData, &mutexName);
        else if (action == "list")
            result = handleList(pInfo, pData, &mutexName);
        else if (action == "delete")
            result = handleDelete(pInfo, pData, &mutexName);
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

        result += STAFString("In STAFZip.cpp: STAFServiceAcceptRequest")
            + kUTF8_SCOLON;

        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

        *pResultBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFZip.cpp: STAFServiceAcceptRequest: "
                         "Caught unknown exception");
        *pResultBuffer = error.adoptImpl();
    }

    if (mutexName.length() > STAFString("STAFZIPMUTEX:").length())
    {
        // release the mutex
        zipLock.release(mutexName);
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

        STAFZipServiceData *pData =
            reinterpret_cast<STAFZipServiceData *>(serviceHandle);

        // Unregister help data

        unregisterHelpData(pData, kZIPGeneralZipError);
        unregisterHelpData(pData, kZIPNotEnoughMemory);
        unregisterHelpData(pData, kZIPChangeFileSizeError);
        unregisterHelpData(pData, kZIPErrorCreatingDir);
        unregisterHelpData(pData, kZIPInvalidZipFile);
        unregisterHelpData(pData, kZIPBadCRC);
        unregisterHelpData(pData, kZIPInvalidOwnerGroup);
        unregisterHelpData(pData, kZIPInvalidFileMode);

    }
    catch (STAFException &e)
    {
        STAFString result;

        result += STAFString("In STAFZip.cpp: STAFServiceTerm")
            + kUTF8_SCOLON;

        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFZip.cpp: STAFServiceTerm: "
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
        STAFZipServiceData *pData =
            reinterpret_cast<STAFZipServiceData *>(*serviceHandle);

        delete pData;
        *serviceHandle = 0;

        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString result;

        result += STAFString("In STAFZip.cpp: STAFServiceDestruct")
            + kUTF8_SCOLON;

        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFZip.cpp: STAFServiceDestruct: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


// Handles Add (and Zip Add) requests

STAFResultPtr handleAdd(STAFServiceRequestLevel30 *pInfo,
                        STAFZipServiceData *pData, STAFString *mutexName)
{
    STAFString result;
    STAFString resultBuffer;
    
    char zipFile[MAXFILENAME + 1] = "";
    char entry[MAXFILENAME + 1] = "";
    char excludePrefix[MAXFILENAME + 1] = "";
    char zipFileBackup[MAXFILENAME + 1] = "";

    unsigned int entryLength = 0;
    unsigned int prefixLength = 0;

    int replace = 0, recursive = 0;

    STAFZipFile *zf;
    STAFZipUtil util = STAFZipUtil(pData->fHandlePtr);

    FILE *file;       

    STAFRC_t rc = kSTAFOk;

    // An ADD request is equivalent to a ZIP ADD request (except for
    // slightly different parsers).  The ADD request syntax is preferred
    // over the ZIP ADD request syntax, since ZIP ADD has been deprecated.
    
    // Check if the deprecated ZIP ADD command was specified

    STAFString requestType = "ADD";
    STAFString request = STAFString(pInfo->request);

    if (request.subWord(0, 1).toLowerCase() == "zip")
        requestType = "ZIP";

    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, requestType, pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fAddParser->
        parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    if (requestType == "ZIP")
    {
        // Log a deprecated tracepoint message

        STAFString errorBuffer = STAFString(
            "STAFZip::handleAdd() - The ZIP service has deprecated the "
            "ZIP ADD request.  Use the ADD request instead.");

        STAFTrace::trace(kSTAFTraceDeprecated, errorBuffer);
    }

    // get "ZIPFILE" parameter value

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "ZIPFILE");
    if (resultPtr->rc != 0) return resultPtr;

    if (resultPtr->result.length() <= MAXFILENAME)
    {
        util.convertSTAFStringBuffer(resultPtr->result.toCurrentCodePage(), zipFile);
    }
    else
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                 STAFString("STAFZip::handleAdd: ZIPFILE [")
                                 + zipFile
                                 + "] file name length exceeds "
                                 + MAXFILENAME
                                 + " charactors.\n"),
                                 STAFResultPtr::INIT);
    }

    resultPtr->result = resultPtr->result.replace(kUTF8_BSLASH, kUTF8_SLASH);

    // use zip file name to construct the mutex name

    *mutexName += resultPtr->result;  
    *mutexName = (*mutexName).toUpperCase();
    
    // obtain mutex semphore on the zipfile

    zipLock.request(*mutexName);
    
    // get "RELATIVETO" parameter value

    resultPtr = resolveOp(pInfo, pData, parsedResult, "RELATIVETO");
    if (resultPtr->rc != 0) return resultPtr;

    resultPtr->result = resultPtr->result.replace(kUTF8_BSLASH, kUTF8_SLASH);
    prefixLength = resultPtr->result.length();

    if (prefixLength > 0)
    {
        if (prefixLength <= MAXFILENAME)
        {
            util.convertSTAFStringBuffer(resultPtr->result.toCurrentCodePage(), excludePrefix);

            if (excludePrefix[prefixLength - 1] != '/')
            {
                excludePrefix[prefixLength] = '/';                
                excludePrefix[prefixLength + 1] = kUTF8_NULL;
                
                prefixLength++;
            }
        }
        else
        {
            return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                     STAFString("STAFZip::handleAdd:"
                                     " RELATIVETO [")
                                     + excludePrefix
                                     + "] file name length exceeds "
                                     + MAXFILENAME
                                     + " charactors.\n"),
                                     STAFResultPtr::INIT);
        }
    }

    // get "RECURSE" parameter value

    if (parsedResult->optionTimes("RECURSE"))
    {
        recursive = 1;
    }

    // make a backup copy of the current zip file

    int fileExist = util.checkFileExist(zipFile);

    STAFString zipFileName;

    if (fileExist)
    {
        strcpy(zipFileBackup, zipFile);
        strcat(zipFileBackup, ".ZIP");

        if(util.copyFile(zipFile, zipFileBackup) != kSTAFOk)
        {
            return STAFResultPtr(new STAFResult(kZIPGeneralZipError,
                                     STAFString("STAFZip::handleAdd:"
                                     " Can't create file [")
                                     + zipFile
                                     + ".ZIP].\n"),
                                     STAFResultPtr::INIT);
        }

        // open the file for reading and writing

        if ((file = fopen(zipFileBackup, "r+b")) == NULL) 
        {
            result = STAFString("STAFZip::handleAdd_CP1.1: ")
                     + "ZipFile [" + zipFileBackup + "] does not exist.\n";
            rc = kSTAFDoesNotExist;
        }
    }
    else
    {
        // create a new file

        if ((file = fopen(zipFile, "wb")) == NULL) 
        {
            result = STAFString("STAFZipFile::STAFZipFile: ")
                     + STAFString("Can't create file [")
                     + zipFile + STAFString("].\n");
            rc = kZIPGeneralZipError;
        }
    }

    // Assign the zipFileName in the STAFZipFile class to a name where
    // any backslashes have been converted to a forward slash.  The
    // zipFileName will be used by the STAFZipFile class to ignore zipping
    // up the zip file or backup zip file being created or updated.

    zipFileName = STAFString(zipFile);

    try
    {
        STAFFSPath fsPath(zipFileName);

        // Do so that any \\ will be converted to a \ and any // are converted
        // to a /, etc, and so that any trailing slashes are removed
        fsPath.setRoot(fsPath.root());

        // Convert backslashes to forward slashes, if any, in the file name
        zipFileName = fsPath.getEntry()->path().asString().replace(
            kUTF8_BSLASH, kUTF8_SLASH);
    }
    catch (STAFBaseOSErrorException)
    {
        zipFileName = zipFileName.replace(kUTF8_BSLASH, kUTF8_SLASH);
    }

    // get number of entries for FILE or DIRECTORY option

    unsigned int numFilesToAdd = parsedResult->optionTimes("FILE");    
    unsigned int numDirsToAdd = parsedResult->optionTimes("DIRECTORY");
        
    // zipping the files

    for (unsigned int i = 1; i <= numFilesToAdd && rc == kSTAFOk; i++)
    {
        // resolve variables

        STAFString thisEntry = parsedResult->optionValue("FILE", i);

        resultPtr = resolveStr(pInfo, pData, thisEntry);

        if (resultPtr->rc != 0) 
        {
            rc = resultPtr->rc;
            result = resultPtr->result;
            
            break;
        }

        thisEntry = resultPtr->result;

        // replace all backward slashes with forward slashes

        thisEntry = thisEntry.replace(kUTF8_BSLASH, kUTF8_SLASH);
        entryLength = thisEntry.length();

        if (entryLength <= MAXFILENAME)
        {
            util.convertSTAFStringBuffer(thisEntry.toCurrentCodePage(), entry);
        }
        else
        {
            result = STAFString("STAFZip::handleAdd: FILE [")
                     + entry
                     + "] file name length exceeds "
                     + MAXFILENAME
                     + " charactors.\n";
            rc = kSTAFInvalidValue;

            break;
        }

        if (prefixLength != 0)
        {
            // check if excludeprefix is shorter than entry name

            if (prefixLength > strlen(entry))
            {
                result = STAFString("STAFZip::handleAdd: The length of"
                                    " RELATIVETO [")
                         + excludePrefix
                         + "] exceeds the length of FILE ["
                         + entry
                         + "].\n";
                rc = kSTAFInvalidValue;

                break;
            }

            // save the char of entry at the prefixLength

            char c = *(entry + prefixLength);

            // truncate the entry to be a identical c string as excludeprefix

            entry[prefixLength] = kUTF8_NULL;

            // check if excludeprefix is a part of the entry name

            if (util.myStrCmp(entry, excludePrefix))
            {
                result = STAFString("STAFZip::handleAdd: RELATIVETO [")
                         + excludePrefix
                         + "] is not a leading sub-string of FILE ["
                         + entry
                         + "].\n";
                rc = kSTAFInvalidValue;

                break;
            }

            // restore the char of entry at the prefixLength

            *(entry + prefixLength) = c;
        }

        zf = new STAFZipFile(pData->fHandlePtr, file,
                             &rc, &resultBuffer, fileExist,
                             -1, -1, zipFileName);

        if (rc != kSTAFOk)
        {
            delete zf,
            result = resultBuffer;

            break;
        }


        if (STAFTrace::doTrace(kSTAFTraceServiceResult))
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("handleAdd_CP3")
                         + " entry ["
                         + entry
                         + "] prefixLength ["
                         + prefixLength
                         + "] recursive ["
                         + recursive
                         + "]");
        }
        
        // adding the entry to the zip archive

        if ((rc = zf->zipFile(entry, prefixLength, recursive, &resultBuffer)) !=
             kSTAFOk)
        {
            result = resultBuffer;
        }

        delete zf;
    }

    // zipping the directories

    for (unsigned int j = 1; j <= numDirsToAdd && rc == kSTAFOk; j++)
    {
        // resolve variables

        STAFString thisEntry = parsedResult->optionValue("DIRECTORY", j);

        resultPtr = resolveStr(pInfo, pData, thisEntry);

        if (resultPtr->rc != 0) 
        {
            rc = resultPtr->rc;
            result = resultPtr->result;
            
            break;
        }

        thisEntry = resultPtr->result;

        // replace all backward slashes with forward slashes

        thisEntry = thisEntry.replace(kUTF8_BSLASH, kUTF8_SLASH);
        entryLength = thisEntry.length();

        if (entryLength <= MAXFILENAME)
        {
            util.convertSTAFStringBuffer(thisEntry.toCurrentCodePage(), entry);

            // append a '/' to the end of directory name

            if (entry[entryLength - 1] != '/')
            {
                entry[entryLength] = '/';
                entry[entryLength + 1] = kUTF8_NULL;             
                entryLength++;
            }
        }
        else
        {
            result = STAFString("STAFZip::handleAdd: DIRECTORY [")
                     + entry
                     + "] file name length exceeds "
                     + MAXFILENAME
                     + " charactors.\n";
            rc = kSTAFInvalidValue;

            break;
        }

        if (prefixLength != 0)
        {
            // check if excludeprefix is shorter than entry name

            if (prefixLength > strlen(entry))
            {
                result = STAFString("STAFZip::handleAdd: The length of"
                                    " RELATIVETO [")
                         + excludePrefix
                         + "] exceeds the length of DIRECTORY ["
                         + entry
                         + "].\n";
                rc = kSTAFInvalidValue;

                break;
            }

            // save the char of entry at the prefixLength

            char c = *(entry + prefixLength);

            // truncate the entry to be a identical c string as excludeprefix

            entry[prefixLength] = kUTF8_NULL;

            // check if excludeprefix is a part of the entry name
            if (util.myStrCmp(entry, excludePrefix))
            {
                result = STAFString("STAFZip::handleAdd: RELATIVETO [")
                         + excludePrefix
                         + "] is not a leading sub-string of DIRECTORY ["
                         + entry
                         + "].\n";
                rc = kSTAFInvalidValue;

                break;
            }

            // restore the char of entry at the prefixLength

            *(entry + prefixLength) = c;
        }

        zf = new STAFZipFile(pData->fHandlePtr, file,
                             &rc, &resultBuffer, fileExist,
                             -1, -1, zipFileName);

        if (rc != kSTAFOk)
        {
            delete zf,
            result = resultBuffer;

            break;
        }

        if (STAFTrace::doTrace(kSTAFTraceServiceResult))
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("handleAdd_CP3")
                         + " entry ["
                         + entry
                         + "] prefixLength ["
                         + prefixLength
                         + "] recursive ["
                         + recursive
                         + "]");
        }
        
        // adding the entry to the zip archive

        if ((rc = zf->zipFile(entry, prefixLength, recursive, &resultBuffer)) !=
             kSTAFOk)
        {
            result = resultBuffer;
        }

        delete zf;
    }

    // file operation is done, let's close the file handle

    if (file != NULL)
    {
        fclose(file);
    }

    if (rc == kSTAFOk)
    {
        if (fileExist)
        {
            // If appending to existing archive, remove the original zip
            // archive since we are working on the backup copy

            if (remove(zipFile) != 0)
            {
                result = STAFString("STAFZip::handleAdd:"
                                    " Remove zipfile failed with OS RC ") +
                    errno + STAFString(": [") + zipFile +
                    STAFString("].  Failed when trying to remove the original "
                               "zipfile before replacing it with the updated "
                               "zipfile.\n");

                rc = kSTAFFileDeleteError;
            }

            // If the remove was successful, rename the backup copy to be the
            // zip archive name

            else if (rename(zipFileBackup, zipFile) != 0)
            {
                result = STAFString("STAFZip::handleAdd: "
                                    "Rename failed with OS RC ") +
                    errno + STAFString(": from [") + zipFileBackup +
                    STAFString("] to [") + zipFile +
                    STAFString("].  Failed when trying to rename the original "
                               "zipfile with the updated zipfile.\n");

                rc = kZIPGeneralZipError;
            }
        }
    }

    if (rc != kSTAFOk)
    {
        // An error occurred

        if (fileExist)
        {
            // If appending to existing zip archive, remove the backup copy
            // that we were working on and leave the original untouched

            remove(zipFileBackup);
        }
        else
        {
            // If this is a new archive that we created just remove it since
            // there was an error during zipping

            remove(zipFile);
        }
    }

    return STAFResultPtr(new STAFResult(rc, result), STAFResultPtr::INIT);
}


// Handles Unzip requests
STAFResultPtr handleUnzip(STAFServiceRequestLevel30 *pInfo,
                           STAFZipServiceData *pData, STAFString *mutexName)
{
    STAFString result;
    STAFString resultBuffer;

    char zipFile[MAXFILENAME + 1] = "";
    char fileName[MAXFILENAME + 1] = "";
    char toDir[MAXFILENAME + 1] = "";

    unsigned int toDirLength = 0;
    unsigned int fileLength = 0;
    int replace = 0;
    int restorePermission = 0;

    STAFZipFile *zf;
    STAFZipUtil util = STAFZipUtil(pData->fHandlePtr);

    STAFRC_t rc = kSTAFOk;

    FILE *file;       

    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "UNZIP", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fUnzipParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }


    // get "ZIPFILE" parameter value

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "ZIPFILE");
    if (resultPtr->rc != 0) return resultPtr;

    if (resultPtr->result.length() <= MAXFILENAME)
    {
        util.convertSTAFStringBuffer(resultPtr->result.toCurrentCodePage(), zipFile);
    }
    else
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                 STAFString("STAFZip::handleUnzip:"
                                            " ZIPFILE [")
                                 + zipFile
                                 + "] file name length exceeds "
                                 + MAXFILENAME
                                 + " charactors.\n"),
                                 STAFResultPtr::INIT);
    }

    resultPtr->result = resultPtr->result.replace(kUTF8_BSLASH, kUTF8_SLASH);

    // use zip file name to construct the mutex name

    *mutexName += resultPtr->result;    
    *mutexName = (*mutexName).toUpperCase();

    // obtain mutex semphore on the zipfile

    zipLock.request(*mutexName);

    // get "TODIRECTORY" parameter value

    resultPtr = resolveOp(pInfo, pData, parsedResult, "TODIRECTORY");
    if (resultPtr->rc != 0) return resultPtr;

    resultPtr->result = 
            STAFString(resultPtr->result.toCurrentCodePage()->buffer());

    resultPtr->result = resultPtr->result.replace(kUTF8_BSLASH, kUTF8_SLASH);
    toDirLength = resultPtr->result.length();

    if (toDirLength <= MAXFILENAME)
    {
        util.convertSTAFStringBuffer(resultPtr->result.toCurrentCodePage(), toDir);

        if (toDir[toDirLength - 1] != '/')
        {
            toDir[toDirLength] = '/';
            toDir[toDirLength + 1] = kUTF8_NULL;            
            toDirLength++;
        }
    }
    else
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                     STAFString("STAFZip::handleUnzip:"
                                                " TODIRECTORY [")
                                     + toDir
                                     + "] file name length exceeds "
                                     + MAXFILENAME
                                     + " charactors.\n"),
                                     STAFResultPtr::INIT);
    }

    // get "REPLACE" parameter value

    if (parsedResult->optionTimes("REPLACE"))
    {
        replace = 1;
    }

    // get "RESTOREPERMISSION" parameter value

    if (parsedResult->optionTimes("RESTOREPERMISSION"))
    {
        restorePermission = 1;
    }

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("handleUnzip_CP1")
                     +" zipFile ["
                     + zipFile
                     + "]");

    // open the ZIP archive for reading

    if ((file = fopen(zipFile, "rb")) == NULL) 
    {
        result = STAFString("STAFZip::handleUnzip_CP1.1: ")
                 + "ZipFile [" + zipFile + "] does not exist.\n";
        rc = kSTAFDoesNotExist;

        return STAFResultPtr(new STAFResult(rc, result),
                             STAFResultPtr::INIT);
    }

    zf = new STAFZipFile(pData->fHandlePtr, file, &rc, &resultBuffer, 1,
                         -1, -1, STAFString(zipFile));

    if (rc != kSTAFOk)
    {
        delete zf;
        fclose(file);

        return STAFResultPtr(new STAFResult(rc, resultBuffer),
                             STAFResultPtr::INIT);
    }

    unsigned int numFilesToUnzip = parsedResult->optionTimes("FILE");
    unsigned int numDirsToUnzip = parsedResult->optionTimes("DIRECTORY");

    if (numFilesToUnzip == 0 && numDirsToUnzip == 0)
    {
        if (STAFTrace::doTrace(kSTAFTraceServiceResult))
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("handleUnzip_CP2")
                         + " toDir ["
                         + toDir
                         + "] replace ["
                         + replace
                         + "] restorePermission ["
                         + restorePermission
                         + "]");
        }

        // if no FILE or DIRECTORY option is provided, unzip all files
        // in the archive

        if ((rc = zf->unzipFile(toDir, replace, restorePermission,
             &resultBuffer)) != kSTAFOk)
        {
            result = resultBuffer;
        }
    }
    else
    {
        if (numFilesToUnzip > 0)
        {
            // unzip the individual files one by one

            for (unsigned int i = 1; i <= numFilesToUnzip; i++)
            {
                // resolve variables

                STAFString thisFile = parsedResult->optionValue("FILE", i);
                resultPtr = resolveStr(pInfo, pData, thisFile);
                
                if (resultPtr->rc != 0)
                {
                    rc = resultPtr->rc;
                    result = resultPtr->result;
            
                    break;
                }

                thisFile = resultPtr->result;
                fileLength = thisFile.length();

                if (fileLength != 0)
                {
                    if (fileLength <= MAXFILENAME)
                    {
                        util.convertSTAFStringBuffer(
                            thisFile.toCurrentCodePage(), fileName);
                    }
                    else
                    {
                        result = STAFString("STAFZip::handleUnzip: FILE [") +
                            fileName + "] file name length exceeds " +
                            MAXFILENAME + " charactors.\n";
                        rc = kSTAFInvalidValue;

                        break;
                    }
                }

                if (STAFTrace::doTrace(kSTAFTraceServiceResult))
                {
                    STAFTrace::trace(
                        kSTAFTraceServiceResult,
                        STAFString("handleUnzip_CP3") +
                        " file [" + fileName + "] toDir [" +
                        toDir + "] replace [" + replace +
                        "] restorePermission [" + restorePermission + "]");
                }
                        
                // unzip the file entry

                if ((rc = zf->unzipFile(
                    fileName, toDir, replace, restorePermission,
                    &resultBuffer)) != kSTAFOk)
                {
                    result = resultBuffer;

                    // if fails, exit the loop

                    break;
                }
            } //end for
        }

        if ((numDirsToUnzip > 0) && (rc == kSTAFOk))
        {
            // unzip the individual dirs one by one

            for (unsigned int i = 1; i <= numDirsToUnzip; i++)
            {
                // resolve variables

                STAFString thisDirectory = parsedResult->optionValue(
                    "DIRECTORY", i);
                resultPtr = resolveStr(pInfo, pData, thisDirectory);

                if (resultPtr->rc != 0) 
                {
                    rc = resultPtr->rc;    
                    result = resultPtr->result;
            
                    break;
                }

                thisDirectory = resultPtr->result;
                fileLength = thisDirectory.length();

                if (fileLength != 0)
                {
                    if (fileLength <= MAXFILENAME)
                    {
                        util.convertSTAFStringBuffer(
                            thisDirectory.toCurrentCodePage(), fileName);
                    }
                    else
                    {
                        result = STAFString("STAFZip::handleUnzip:"
                                            " DIRECTORY [") + fileName +
                            "] dir name length exceeds " + MAXFILENAME +
                            " charactors.\n";

                        rc = kSTAFInvalidValue;

                        break;
                    }
                }

                if (STAFTrace::doTrace(kSTAFTraceServiceResult))
                {
                    STAFTrace::trace(
                        kSTAFTraceServiceResult,
                        STAFString("handleUnzip_CP3") + " dir [" + fileName +
                        "] toDir [" + toDir + "] replace [" + replace +
                        "] restorePermission [" + restorePermission + "]");
                }
                      
                // unzip the dir entry

                if ((rc = zf->unzipDir(
                    fileName, toDir, replace, restorePermission,
                    &resultBuffer)) != kSTAFOk)
                {
                    result = resultBuffer;

                    // if fails, exit the loop
                    break;
                }
            } // end for

        } // end if
    }

    delete zf;

    if (file != NULL)
    {
        fclose(file);
    }

    return STAFResultPtr(new STAFResult(rc, result),
                         STAFResultPtr::INIT);
}


// Handles Delete requests

STAFResultPtr handleDelete(STAFServiceRequestLevel30 *pInfo,
                           STAFZipServiceData *pData, STAFString *mutexName)
{
    STAFString result;
    STAFString resultBuffer;

    char zipFile[MAXFILENAME + 1] = "";
    char deleteFile[MAXFILENAME + 1] = "";
    char zipFileBackup[MAXFILENAME + 1] = "";

    STAFZipFile *zf;
    STAFZipUtil util = STAFZipUtil(pData->fHandlePtr);

    STAFRC_t rc = kSTAFOk;

    FILE *file;       

    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "DELETE", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fDeleteParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // get "ZIPFILE" parameter value

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "ZIPFILE");
    if (resultPtr->rc != 0) return resultPtr;

    if (resultPtr->result.length() <= MAXFILENAME)
    {
        util.convertSTAFStringBuffer(resultPtr->result.toCurrentCodePage(), zipFile);
    }
    else
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                            STAFString("STAFZip::handleDelete:"
                                            " ZIPFILE [")
                                            + zipFile
                                            + "] file name length exceeds "
                                            + MAXFILENAME
                                            + " charactors.\n"),
                                            STAFResultPtr::INIT);
    }
    
    resultPtr->result = resultPtr->result.replace(kUTF8_BSLASH, kUTF8_SLASH);

    // use zip file name to construct the mutex name

    *mutexName += resultPtr->result;    
    *mutexName = (*mutexName).toUpperCase();

    // obtain mutex semphore on the zipfile

    zipLock.request(*mutexName);

    int fileExist = util.checkFileExist(zipFile);

    if (!fileExist)
    {
        return STAFResultPtr(new STAFResult(
            kSTAFDoesNotExist,
            STAFString("STAFZip::handleDelete: Zip file does not exist [") +
            zipFile + "].\n"), STAFResultPtr::INIT);
    }        
        
    // backup the original zip archive

    strcpy(zipFileBackup, zipFile);
    strcat(zipFileBackup, ".ZIP");

    if (util.copyFile(zipFile, zipFileBackup) != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(
            kZIPGeneralZipError,
            STAFString("STAFZip::handleDelete: Can't create file [") +
            zipFile + ".ZIP].\n"), STAFResultPtr::INIT);
    }

    // open the ZIP archive for reading and writing

    if ((file = fopen(zipFileBackup, "r+b")) == NULL) 
    {
        return STAFResultPtr(new STAFResult(
            kSTAFDoesNotExist,
            STAFString("STAFZip::handleDelete_CP0.1: ZipFile [") +
            zipFileBackup + "] does not exist."), STAFResultPtr::INIT);
    }

    // loop to delete file entries from zip archive

    STAFInt64_t newsize = -1;

    unsigned int numFilesToDelete = parsedResult->optionTimes("FILE");

    for (unsigned int i = 1; i <= numFilesToDelete; i++)
    {
        // resolve variables

        STAFString thisFile = parsedResult->optionValue("FILE", i);
        resultPtr = resolveStr(pInfo, pData, thisFile);

        if (resultPtr->rc != 0) 
        {
            rc = resultPtr->rc;    
            result = resultPtr->result;
            
            break;
        }

        thisFile = resultPtr->result;

        if (thisFile.length() <= MAXFILENAME)
        {
            util.convertSTAFStringBuffer(thisFile.toCurrentCodePage(), deleteFile);
        }
        else
        {
            result = STAFString("STAFZip::handleDelete: FILE [")
                     + STAFString(deleteFile)
                     + "] file name length exceeds "
                     + MAXFILENAME
                     + " charactors.\n";
            rc = kSTAFInvalidValue;

            break;
        }

        // delete a file from zip archive

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("handleDelete_CP1")
                             + " zipFileBackup ["
                             + zipFileBackup
                             + "]");

        // open the backup zip archive

        zf = new STAFZipFile(pData->fHandlePtr, file,
                             &rc, &resultBuffer, 1,
                             newsize, -1, STAFString(zipFileBackup));

        if (rc != kSTAFOk)
        {
            delete zf;
            result = resultBuffer;

            break;
        }

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("handleDelete_CP2")
                         + " deleteFile [" + deleteFile + "]");

        // delete one entry from the zip archive

        if ((rc = zf->deleteFile(deleteFile, (STAFInt64_t*)&newsize,
                                 &resultBuffer)) != kSTAFOk)
        {
            delete zf;
            result = resultBuffer;

            break;
        }

        delete zf;
    } // end deleting loop

    // file operation is done, let's close the file handle

    if (file != NULL)
    {
        fclose(file);
    }

    // to reduce file size after deleting one or more file entries

    if (newsize > 0)
    {
        // change file size to the new size after deleting the file entry

        if ((rc = util.changeFileSize(zipFileBackup, newsize)) != 0)
        {
            result = STAFString(
                "STAFZip::handleDelete: Change file size fails: [") +
                zipFileBackup + STAFString("] to [") +
                STAFString(newsize) + STAFString("]");

            rc = kZIPGeneralZipError;
        }
    }

    if (rc == kSTAFOk && newsize > 0)
    {
        // Remove the original zip archive (since we are working on the
        // backup zip archive)

        if (remove(zipFile) != 0)
        {
            result = STAFString(
                "STAFZip::handleDelete: Remove zipfile failed with OS RC ") +
                errno + STAFString(": [") + zipFile + STAFString(
                    "].  Failed when trying to remove the original zipfile "
                    "so can replace it with the updated zipfile.");

            rc = kSTAFFileDeleteError;

            // Delete the backup zip archive since won't be able to use it

            remove(zipFileBackup);
        }
        else
        {
            // rename the backup zip archive to the original zip archive
            // since we are working on the backup zip archive

            if (rename(zipFileBackup, zipFile) != 0)
            {
                result = STAFString(
                    "STAFZip::handleDelete: Rename failed with OS RC ") +
                    errno + STAFString(": from [") + zipFileBackup +
                    STAFString("] to [") + zipFile + STAFString(
                        "].  Failed when trying to rename the original "
                        "zipfile with the updated zipfile.\n");

                rc = kZIPGeneralZipError;

                // Delete the backup zip archive since won't be able to use it

                remove(zipFileBackup);
            }
        }
    }
    else // either all entries are removed or remove failed
    {
        // remove the backup zip archive that we are working on

        remove(zipFileBackup);

        if (newsize == 0 && rc == kSTAFOk)
        {
            // If all files in the zip archive are removed, remove the zip
            // archive

            if ((rc = remove(zipFile)) != 0)
            {
                result = STAFString("STAFZip::handleDelete: "
                                "Remove file failed with OS RC ") +
                    errno + STAFString(": [") + STAFString(zipFile) +
                    STAFString("].  Failed when trying to remove the "
                               "original zipfile since all files in "
                               "the zipfile were removed.");

                rc = kSTAFFileDeleteError;
            }
        }
    }

    return STAFResultPtr(new STAFResult(rc, result),
                         STAFResultPtr::INIT);
}


// Handles List requests
STAFResultPtr handleList(STAFServiceRequestLevel30 *pInfo,
                           STAFZipServiceData *pData, STAFString *mutexName)
{
    STAFString result;

    char zipFile[MAXFILENAME + 1] = "";

    STAFZipFile *zf;
    STAFZipUtil util = STAFZipUtil(pData->fHandlePtr);

    STAFRC_t rc = kSTAFOk;

    FILE *file;       

    // Verify the requester has at least trust level 3

    VALIDATE_TRUST(3, pData->fShortName, "LIST", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        pData->fListParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // get "ZIPFILE" parameter value

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "ZIPFILE");
    if (resultPtr->rc != 0) return resultPtr;

    if (resultPtr->result.length() <= MAXFILENAME)
    {
        util.convertSTAFStringBuffer(resultPtr->result.toCurrentCodePage(), zipFile);
    }
    else
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue,
                                            STAFString("STAFZip::handleList:"
                                            " ZIPFILE [")
                                            + zipFile
                                            +"] file name length exceeds "
                                            + MAXFILENAME
                                            + " charactors.\n"),
                                            STAFResultPtr::INIT);
    }

    resultPtr->result = resultPtr->result.replace(kUTF8_BSLASH, kUTF8_SLASH);

    // use zip file name to construct the mutex name

    *mutexName += resultPtr->result;    
    *mutexName = (*mutexName).toUpperCase();

    // obtain mutex semphore on the zipfile

    zipLock.request(*mutexName);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("handleList_CP1")
                     + " zipFile ["
                     + zipFile
                     + "]");

    // open the ZIP archive for reading

    if ((file = fopen(zipFile, "rb")) == NULL) 
    {
        result = STAFString("STAFZip::handleList_CP1.1: ")
                 + "ZipFile [" + zipFile + "] does not exist.\n";
        rc = kSTAFDoesNotExist;

        return STAFResultPtr(new STAFResult(rc, result),
                             STAFResultPtr::INIT);
    }

    // open the Zip archive, don't create a new one

    zf = new STAFZipFile(pData->fHandlePtr, file, &rc, &result, 1,
                         -1, -1, STAFString(zipFile));

    if (rc != kSTAFOk)
    {
        delete zf;
        fclose(file);

        return STAFResultPtr(new STAFResult(rc, result),
                         STAFResultPtr::INIT);
    }

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("handleList_CP2"));

    STAFString buffer;

    rc = zf->listFile(&buffer, &result);

    delete zf;

    // file operation is done, let's close the file handle

    if (file != NULL)
    {
        fclose(file);
    }

    if (rc == kSTAFOk)
    {
        result = buffer;
    }

    return STAFResultPtr(new STAFResult(rc, result),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleHelp(STAFServiceRequestLevel30 *pInfo,
                         STAFZipServiceData *pData)
{
    // Verify the requester has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "HELP", pData->fLocalMachineName);

    // Return help text for the service
    
    return STAFResultPtr(new STAFResult(kSTAFOk, sHelpMsg),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleVersion(STAFServiceRequestLevel30 *pInfo,
                            STAFZipServiceData *pData)
{
    // Verify the requester has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "VERSION", pData->fLocalMachineName);

    return STAFResultPtr(new STAFResult(kSTAFOk, sVersionInfo),
                         STAFResultPtr::INIT);
}


STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo,
                        STAFZipServiceData *pData,
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
                         STAFZipServiceData *pData,
                         const STAFString &theString)
{
    return pData->fHandlePtr->submit(sLocal, sVar, sResStrResolve +
                                     STAFString(pInfo->requestNumber) +
                                     sString +
                                     pData->fHandlePtr->wrapData(theString));
}


void registerHelpData(STAFZipServiceData *pData, unsigned int errorNumber,
                      const STAFString &shortInfo, const STAFString &longInfo)
{
    static STAFString regString("REGISTER SERVICE %C ERROR %d INFO %C "
                                "DESCRIPTION %C");

    pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber,
        shortInfo.getImpl(), longInfo.getImpl()));
}


void unregisterHelpData(STAFZipServiceData *pData, unsigned int errorNumber)
{
    static STAFString regString("UNREGISTER SERVICE %C ERROR %d");

    pData->fHandlePtr->submit(sLocal, STAFString("HELP"), STAFHandle::
    formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber));
}


























