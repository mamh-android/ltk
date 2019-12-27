/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <vector>
#include <list>
#include <map>
#include "STAF_fstream.h"
#include "STAFString.h"
#include "STAFError.h"
#include "STAFException.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFEventSem.h"
#include "STAFRWSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFTimestamp.h"
#include "STAFUtil.h"
#include "STAFInternalUtil.h"
#include "STAFResPoolService.h"
#include "STAFFileSystem.h"

typedef STAFRefPtr<STAFCommandParser> STAFCommandParserPtr;

// Valid Request Types

enum RequestType
{
    kRandom = 0,
    kFirst = 1,
    kEntry = 2
};

static const int sMinimumPriority = 1;
static const int sMaximumPriority = 99;
static const int sDefaultPriority = 50;

// Resource Data - contains data for a entry in a resource pool

struct ResourceData
{
    ResourceData() : owned(0), garbageCollect(true)
    { /* Do Nothing */ }

    ResourceData(const STAFString &aEntry) :
        entry(aEntry), owned(0), garbageCollect(true)
    { /* Do Nothing */ }

    STAFString   entry;            // Entry value
    unsigned int owned;            // 0 means Available; 1 means Owned
    STAFString   orgUUID;          // Originating request's STAF UUID
    STAFString   orgMachine;       // Originating request's machine name
    STAFString   orgName;          // Originating request's handle name
    STAFHandle_t orgHandle;        // Originating request's handle
    STAFString   orgUser;          // Originating request's handle user
    STAFString   orgEndpoint;      // Originating request's endpoint
    STAFString   requestedTime;    // Time made request
    STAFString   acquiredTime;     // Time acquired the resource
    bool         garbageCollect;   // true means perform garbage collection
                                   // false means no garbage collection
};

// ResourceList --  Ordered list of entries in a resource pool

typedef std::vector<ResourceData> ResourceList;

// Request Data - contains data for a pending request for a resource

struct RequestData
{
    RequestData() : wakeup(new STAFEventSem(), STAFEventSemPtr::INIT),
                    retCode(kSTAFTimeout)
    {
        requestedTime = STAFTimestamp::now().asString();
        wakeup->reset();
        *garbageCollectedPtr = false;
        garbageCollect = true;
        priority = sDefaultPriority;
    }

    RequestData(const STAFString &aUUID, const STAFString &aMachine,
                const STAFString &aHandleName, const STAFHandle_t aHandle,
                const STAFString aUser, const STAFString aEndpoint,
                const bool aGarbageCollect,
                const RequestType aRequestType,
                const STAFString &aRequestedEntry,
                const int aPriority)
        : orgUUID(aUUID), orgMachine(aMachine), orgName(aHandleName),
          orgHandle(aHandle), orgUser(aUser), orgEndpoint(aEndpoint),
          wakeup(new STAFEventSem(), STAFEventSemPtr::INIT),
          retCode(kSTAFTimeout),
          garbageCollectedPtr(new bool, STAFRefPtr<bool>::INIT),
          garbageCollect(aGarbageCollect),
          requestType(aRequestType), requestedEntry(aRequestedEntry),
          priority(aPriority)
    {
        requestedTime = STAFTimestamp::now().asString();
        wakeup->reset();
        *garbageCollectedPtr = false;
    }

    STAFString      orgUUID;       // Originating request's STAF UUID
    STAFString      orgMachine;    // Originating request's machine name
    STAFString      orgName;       // Originating request's handle name
    STAFHandle_t    orgHandle;     // Originating request's STAF handle
    STAFString      orgUser;       // Originating request's user
    STAFString      orgEndpoint;   // Originating request's endpoint
    STAFString      requestedTime; // Originating request's date/time
    STAFEventSemPtr wakeup;        // Semaphore to wake up a pending request
    STAFRC_t        retCode;       // Return code indicating if request was
                                   //   successfully satisfied
    STAFString      resultBuffer;  // Entry obtained if retCode = 0
    STAFRefPtr<bool> garbageCollectedPtr;
    bool            garbageCollect;// true means perform garbage collection
                                   // false means no garbage collection
    RequestType     requestType;   // Request type
    STAFString      requestedEntry; // Entry, if requestType == kEntry
    int             priority;      // Priority for the request, default is 50
};

typedef STAFRefPtr<RequestData> RequestDataPtr;

// RequestList  --  Ordered list of pending requests for a pool

typedef std::list<RequestDataPtr> RequestList;

static const unsigned int sCurrFileFormat = 1;

// Resource Pool Data - contains all data for a resource pool

struct PoolData
{
    PoolData() : usedResources(0),
                 accessSem(new STAFMutexSem(), STAFMutexSemPtr::INIT)
    { /* Do nothing */ }

    PoolData(const STAFString &aPoolName, const STAFString &aPoolDescription)
        : poolName(aPoolName), poolDescription(aPoolDescription), 
          numResources(0), usedResources(0), 
          accessSem(new STAFMutexSem(), STAFMutexSemPtr::INIT)
    { fileFormat = sCurrFileFormat; }

    unsigned int    fileFormat;      // Format of the pool file
                                     //  "0" - REXX version in STAF 2.2 and <
                                     //   1  - C++ version in STAF 2.3 and later
    STAFString      poolName;        // Pool Name
    STAFString      poolDescription; // Pool Description
    unsigned int    numResources;    // Total # of entries in ResourceList
    unsigned int    usedResources;   // # of entries used in ResourceList
    ResourceList    resourceList;    // List of entries in a resource pool
    RequestList     requestList;     // List of pending requests
    STAFMutexSemPtr accessSem;       // Semaphore to control access to PoolData
};

typedef STAFRefPtr<PoolData> PoolDataPtr;


// PoolMap -- KEY:   Pool name in upper case, 
//            VALUE: Pointer to PoolData information

typedef std::map<STAFString, PoolDataPtr> PoolMap;

// List of STAF_NOTIFY UNREGISTER ONENDOFHANDLE requests to be submitted
// if removing all entries in a REMOVE request is succuessful

typedef std::vector<STAFString> NotifyUnregisterRequestList;

// Read/Write File Return Codes

enum ReadFileRC
{
    kReadorWriteOk = 0,
    kReadEndOfFile = 1,
    kReadInvalidFormat = 2,
    kFileOpenError = 3
};

// RESPOOL Service Data

struct ResPoolServiceData
{
    unsigned int  fDebugMode;            // Debug Mode flag
    STAFString    fShortName;            // Short service name
    STAFString    fName;                 // Registered service name
    STAFString    fLocalMachineName;     // Logical identifier for the local
                                         //   machine
    STAFString    fPoolDir;              // Pool Directory
    STAFHandlePtr fHandlePtr;            // Respool service's STAF handle
    STAFCommandParserPtr fCreateParser;  // RESPOOL CREATE command parser
    STAFCommandParserPtr fDeleteParser;  // RESPOOL DELETE command parser
    STAFCommandParserPtr fQueryParser;   // RESPOOL QUERY command parser
    STAFCommandParserPtr fRequestParser; // RESPOOL REQUEST command parser
    STAFCommandParserPtr fAddParser;     // RESPOOL ADD command parser
    STAFCommandParserPtr fRemoveParser;  // RESPOOL REMOVE command parser
    STAFCommandParserPtr fReleaseParser; // RESPOOL RELEASE command parser
    STAFCommandParserPtr fCancelParser;  // RESPOOL CANCEL command parser
    STAFCommandParserPtr fListParser;    // RESPOOL LIST command parser
    STAFCommandParserPtr 
        fSTAFCallbackParser; // RESPOOL STAF_CALLBACK command parser
    STAFCommandParserPtr fHelpParser;    // RESPOOL HELP command parser
    STAFCommandParserPtr fVersionParser; // RESPOOL VERSION command parser
    STAFCommandParserPtr fParmsParser;   // RESPOOL PARMS command parser

    STAFRWSemPtr         fPoolMapRWSem;  // Read/Write semaphore to control 
                                         //   access to the PoolMap
    PoolMap              fPoolMap;       // Map of all resource pools

    // Map Class Definitions for marshalled results
    STAFMapClassDefinitionPtr fPoolClass; 
    STAFMapClassDefinitionPtr fPoolInfoClass;
    STAFMapClassDefinitionPtr fSettingsClass;
    STAFMapClassDefinitionPtr fRequestClass;
    STAFMapClassDefinitionPtr fResourceClass;
    STAFMapClassDefinitionPtr fResourceOwnerClass;
};

typedef STAFRefPtr<ResPoolServiceData> ResPoolServiceDataPtr;

// Static Variables

static STAFString sHelpMsg;
static STAFString sLineSep;
static STAFString sPoolExt("rpl");
static const STAFString sVersionInfo("3.4.9");
static const STAFString sPool("POOL");
static const STAFString sDescription("DESCRIPTION");
static const STAFString sFirst("FIRST");
static const STAFString sTimeout("TIMEOUT");
static const STAFString sGarbageCollect("GARBAGECOLLECT");
static const STAFString sEntry("ENTRY");
static const STAFString sPriority("PRIORITY");
static const STAFString sForce("FORCE");
static const STAFString sHandle("HANDLE");
static const STAFString sName("NAME");
static const STAFString sMachine("MACHINE");
static const STAFString sDirectory("DIRECTORY");
static const STAFString sLeftCurlyBrace(kUTF8_LCURLY);
static const STAFString sSemiColon(kUTF8_SCOLON);
static const STAFString sColon(kUTF8_COLON);
static const STAFString sSlash(kUTF8_SLASH);
static const STAFString sSpecSeparator(sColon + sSlash + sSlash);
static const STAFString sLocal("local");
static const STAFString sHelp("help");
static const STAFString sVar("var");
static const STAFString sResStrResolve("RESOLVE REQUEST ");
static const STAFString sString(" STRING ");
static const STAFString sNo("No");
static const STAFString sYes("Yes");
static const STAFString sNotificationKey = "ResPoolEntry";

#ifdef STAF_OS_NAME_WIN32
static const STAFString sInvalidPoolNameCharacters("<>:\"/\\|?*");
static const STAFString sOS("Windows");
#else
static const STAFString sInvalidPoolNameCharacters("/");
static const STAFString sOS("Unix");
#endif

// Prototypes

static STAFResultPtr handleCreate(STAFServiceRequestLevel30 *, 
                                  ResPoolServiceData *);
static STAFResultPtr handleDelete(STAFServiceRequestLevel30 *,
                                  ResPoolServiceData *);
static STAFResultPtr handleQuery(STAFServiceRequestLevel30 *,
                                 ResPoolServiceData *);
static STAFResultPtr handleRequest(STAFServiceRequestLevel30 *,
                                   ResPoolServiceData *);
static STAFResultPtr handleRelease(STAFServiceRequestLevel30 *,
                                   ResPoolServiceData *);
static STAFResultPtr handleCancel(STAFServiceRequestLevel30 *,
                                  ResPoolServiceData *);
static STAFResultPtr handleAdd(STAFServiceRequestLevel30 *,
                               ResPoolServiceData *);
static STAFResultPtr handleRemove(STAFServiceRequestLevel30 *,
                                  ResPoolServiceData *);
static STAFResultPtr handleSTAFCallback(STAFServiceRequestLevel30 *,
                                        ResPoolServiceData *);
static STAFResultPtr handleList(STAFServiceRequestLevel30 *,
                                ResPoolServiceData *);
static STAFResultPtr handleHelp(STAFServiceRequestLevel30 *,
                                ResPoolServiceData *);
static STAFResultPtr handleVersion(STAFServiceRequestLevel30 *,
                                   ResPoolServiceData *);

static STAFResultPtr submitSTAFNotifyRegisterRequest(
    ResPoolServiceData *pData, STAFHandle_t handle, STAFString endpoint,
    STAFString uuid);

static STAFResultPtr submitSTAFNotifyUnregisterRequest(
    ResPoolServiceData *pData, STAFHandle_t handle, STAFString endpoint,
    STAFString uuid);

static STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo, 
                                ResPoolServiceData *pData,
                                const STAFString &theString);

static STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, 
                               ResPoolServiceData *pData,
                               STAFCommandParseResultPtr &parsedResult,
                               const STAFString &fOption,
                               unsigned int optionIndex = 1);

STAFResultPtr resolveOpLocal(ResPoolServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption,
                             unsigned int optionIndex = 1);

static STAFResultPtr convertOptionStringToUInt(
    const STAFString &theString,
    const STAFString &optionName,
    unsigned int &number,
    const unsigned int minValue = 0,
    const unsigned int maxValue = UINT_MAX);

static void registerHelpData(ResPoolServiceData *pData,
                             unsigned int errorNumber,
                             const STAFString &shortInfo,
                             const STAFString &longInfo);

static void unregisterHelpData(ResPoolServiceData *pData,
                               unsigned int errorNumber);

void readUIntFromFile(istream &input, unsigned int &data,
                      unsigned int length = 4);

void writeUIntToFile(ostream &output, unsigned int data,
                     unsigned int length = 4);

void readStringFromFile(istream &input, STAFString &inString);

void writeStringToFile(ostream &output, STAFString &outString);

unsigned int readPoolFile(const STAFString &fileName, PoolData &poolData);

unsigned int writePoolFile(const STAFString &fileName, PoolData &poolData);


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

        ResPoolServiceData data;
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

        *pServiceHandle = new ResPoolServiceData(data);

        return kSTAFOk;
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                "STAFResPoolService.cpp: STAFServiceConstruct").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFResPoolService.cpp: STAFServiceConstruct: "
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

        ResPoolServiceData *pData =
            reinterpret_cast<ResPoolServiceData *>(serviceHandle);
        
        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);        

        retCode = STAFHandle::create(pData->fName, pData->fHandlePtr);
        
        if (retCode != kSTAFOk)
            return retCode;
        
        //CREATE options
        pData->fCreateParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fCreateParser->addOption("CREATE", 1,
                                       STAFCommandParser::kValueNotAllowed);
        pData->fCreateParser->addOption("POOL", 1,
                                       STAFCommandParser::kValueRequired);
        pData->fCreateParser->addOption("DESCRIPTION", 1,
                                       STAFCommandParser::kValueRequired);
        pData->fCreateParser->addOptionNeed("CREATE", "POOL");
        pData->fCreateParser->addOptionNeed("CREATE", "DESCRIPTION");
        
        //DELETE options
        pData->fDeleteParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fDeleteParser->addOption("DELETE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOption("POOL", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fDeleteParser->addOption("CONFIRM",  1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOption("FORCE",  1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fDeleteParser->addOptionNeed("DELETE", "POOL");
        pData->fDeleteParser->addOptionNeed("DELETE", "CONFIRM");
        
        //QUERY options
        pData->fQueryParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fQueryParser->addOption("QUERY", 1,
                                       STAFCommandParser::kValueNotAllowed);
        pData->fQueryParser->addOption("POOL", 1,
                                       STAFCommandParser::kValueRequired);
        pData->fQueryParser->addOptionNeed("QUERY", "POOL");

        //REQUEST options
        pData->fRequestParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fRequestParser->addOption("REQUEST", 1,
                                         STAFCommandParser::kValueNotAllowed);
        pData->fRequestParser->addOption("POOL", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fRequestParser->addOption("TIMEOUT", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fRequestParser->addOption("FIRST", 1,
                                         STAFCommandParser::kValueNotAllowed);
        pData->fRequestParser->addOption("RANDOM", 1,
                                         STAFCommandParser::kValueNotAllowed);
        pData->fRequestParser->addOption("ENTRY", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fRequestParser->addOption("RELEASE", 1,
                                         STAFCommandParser::kValueNotAllowed);
        pData->fRequestParser->addOption("PRIORITY", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fRequestParser->addOption("GARBAGECOLLECT", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fRequestParser->addOptionGroup("FIRST RANDOM ENTRY", 0, 1);
        pData->fRequestParser->addOptionNeed("REQUEST", "POOL");
        pData->fRequestParser->addOptionNeed("RELEASE", "ENTRY");

        //ADD options
        pData->fAddParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fAddParser->addOption("ADD", 1,
                                     STAFCommandParser::kValueNotAllowed);
        pData->fAddParser->addOption("POOL", 1,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOption("ENTRY", 0,
                                     STAFCommandParser::kValueRequired);
        pData->fAddParser->addOptionNeed("ADD", "POOL");
        pData->fAddParser->addOptionNeed("ADD", "ENTRY");

        //REMOVE options
        pData->fRemoveParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fRemoveParser->addOption("REMOVE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fRemoveParser->addOption("POOL", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fRemoveParser->addOption("ENTRY", 0,
                                        STAFCommandParser::kValueRequired);
        pData->fRemoveParser->addOption("CONFIRM", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fRemoveParser->addOption("FORCE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fRemoveParser->addOptionNeed("REMOVE", "POOL");
        pData->fRemoveParser->addOptionNeed("REMOVE", "ENTRY");
        pData->fRemoveParser->addOptionNeed("REMOVE", "CONFIRM");

        //RELEASE options
        pData->fReleaseParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        pData->fReleaseParser->addOption("RELEASE", 1,
                                         STAFCommandParser::kValueNotAllowed);
        pData->fReleaseParser->addOption("POOL", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fReleaseParser->addOption("ENTRY", 1,
                                         STAFCommandParser::kValueRequired);
        pData->fReleaseParser->addOption("FORCE", 1,
                                         STAFCommandParser::kValueNotAllowed);
        pData->fReleaseParser->addOptionNeed("RELEASE", "POOL");
        pData->fReleaseParser->addOptionNeed("RELEASE", "ENTRY");

        //LIST options
        pData->fListParser = STAFCommandParserPtr(new STAFCommandParser,
                                                  STAFCommandParserPtr::INIT);
        pData->fListParser->addOption("LIST", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("POOLS", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOption("SETTINGS", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fListParser->addOptionNeed("POOLS SETTINGS", "LIST");
        pData->fListParser->addOptionGroup("POOLS SETTINGS", 0, 1);
                                      
        // CANCEL REQUEST options
        pData->fCancelParser = STAFCommandParserPtr(
            new STAFCommandParser, STAFCommandParserPtr::INIT);
        pData->fCancelParser->addOption("CANCEL", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fCancelParser->addOption("POOL", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fCancelParser->addOption("FORCE", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fCancelParser->addOption("HANDLE", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fCancelParser->addOption("NAME", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fCancelParser->addOption("MACHINE", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fCancelParser->addOption("PRIORITY", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fCancelParser->addOption("ENTRY", 1,
                                        STAFCommandParser::kValueRequired);
        pData->fCancelParser->addOption("FIRST", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fCancelParser->addOption("LAST", 1,
                                        STAFCommandParser::kValueNotAllowed);
        pData->fCancelParser->addOptionNeed("CANCEL", "POOL");
        pData->fCancelParser->addOptionNeed(
            "POOL FORCE PRIORITY ENTRY FIRST LAST", "CANCEL");
        pData->fCancelParser->addOptionNeed("MACHINE HANDLE NAME", "FORCE");
        pData->fCancelParser->addOptionGroup("HANDLE NAME", 0, 1);
        pData->fCancelParser->addOptionGroup("FIRST LAST", 0, 1);

        //STAF_CALLBACK options
        pData->fSTAFCallbackParser = STAFCommandParserPtr(new 
            STAFCommandParser, STAFCommandParserPtr::INIT);
        pData->fSTAFCallbackParser->addOption("STAF_CALLBACK", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fSTAFCallbackParser->addOption("HANDLEDELETED", 1,
                                      STAFCommandParser::kValueNotAllowed);
        pData->fSTAFCallbackParser->addOption("HANDLE", 1,
                                      STAFCommandParser::kValueRequired);
        pData->fSTAFCallbackParser->addOption("UUID", 1,
                                      STAFCommandParser::kValueRequired);
        pData->fSTAFCallbackParser->addOption("MACHINE", 1,
                                      STAFCommandParser::kValueRequired);
        pData->fSTAFCallbackParser->addOption("KEY", 1,
                                      STAFCommandParser::kValueRequired);

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

        // PARMS        
        pData->fParmsParser = STAFCommandParserPtr(new STAFCommandParser,
                                                   STAFCommandParserPtr::INIT);
        pData->fParmsParser->addOption("DIRECTORY", 1,
                                       STAFCommandParser::kValueRequired);
        STAFCommandParseResultPtr parsedResult = 
            pData->fParmsParser->parse(pInfo->parms);

        if (parsedResult->rc != kSTAFOk) 
        {
            *pErrorBuffer = parsedResult->errorBuffer.adoptImpl();
            return parsedResult->rc;
        }

        if (parsedResult->optionTimes(sDirectory))
        {
            STAFResultPtr dirResult = resolveOpLocal(pData, parsedResult,
                                                     sDirectory);

            if (dirResult->rc != kSTAFOk)
            {
                *pErrorBuffer = dirResult->result.adoptImpl();
                return dirResult->rc;
            }

            pData->fPoolDir = dirResult->result;
        }
        
        // Construct the map class for general pool information output

        pData->fPoolClass = STAFMapClassDefinition::create(
            "STAF/Service/ResPool/Pool");

        pData->fPoolClass->addKey("poolName",    "Pool Name");
        pData->fPoolClass->addKey("description", "Description");
        
        // Construct the map class for listing service settings

        pData->fSettingsClass = STAFMapClassDefinition::create(
            "STAF/Service/ResPool/Settings");

        pData->fSettingsClass->addKey("directory",  "Directory");

        // Construct the map class for detailed pool information output

        pData->fPoolInfoClass = STAFMapClassDefinition::create(
            "STAF/Service/ResPool/PoolInfo");

        pData->fPoolInfoClass->addKey("description",  "Description");
        pData->fPoolInfoClass->addKey("requestList",  "Pending Requests");
        pData->fPoolInfoClass->addKey("resourceList", "Resources");

        // Construct map class for a pending request

        pData->fRequestClass = STAFMapClassDefinition::create(
            "STAF/Service/ResPool/Request");

        pData->fRequestClass->addKey("priority", "Priority");
        pData->fRequestClass->addKey("requestedTimestamp",
                                     "Date-Time Requested");
        pData->fRequestClass->addKey("requestedEntry", "Requested Entry");
        pData->fRequestClass->addKey("machine",    "Machine");
        pData->fRequestClass->addKey("handleName", "Handle Name");
        pData->fRequestClass->addKey("handle",     "Handle");
        pData->fRequestClass->addKey("user",       "User");
        pData->fRequestClass->addKey("endpoint",   "Endpoint");
        pData->fRequestClass->addKey("gc", "Perform Garbage Collection");

        // Construct map class for a resource owner

        pData->fResourceOwnerClass = STAFMapClassDefinition::create(
            "STAF/Service/ResPool/ResourceOwner");

        pData->fResourceOwnerClass->addKey("machine",    "Machine");
        pData->fResourceOwnerClass->addKey("handleName", "Handle Name");
        pData->fResourceOwnerClass->addKey("handle",     "Handle");
        pData->fResourceOwnerClass->addKey("user",       "User");
        pData->fResourceOwnerClass->addKey("endpoint",   "Endpoint");
        pData->fResourceOwnerClass->addKey("requestedTimestamp",
                                           "Date-Time Requested");
        pData->fResourceOwnerClass->addKey("acquiredTimestamp",
                                           "Date-Time Acquired");
        pData->fResourceOwnerClass->addKey("gc",
                                           "Perform Garbage Collection");
        
        // Construct map class for a resource

        pData->fResourceClass = STAFMapClassDefinition::create(
            "STAF/Service/ResPool/Resource");

        pData->fResourceClass->addKey("entry", "Entry");
        pData->fResourceClass->addKey("owner", "Owner");

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
            "CREATE  POOL <PoolName> DESCRIPTION <Pool Description>" +
            sLineSep +
            "DELETE  POOL <PoolName> CONFIRM [FORCE]" +
            sLineSep +
            "LIST    [POOLS | SETTINGS]" +
            sLineSep +
            "ADD     POOL <PoolName> ENTRY <Value> [ENTRY <Value>]..." +
            sLineSep +
            "REMOVE  POOL <PoolName> ENTRY <Value> [ENTRY <Value>]... CONFIRM [FORCE]" +
            sLineSep +
            "QUERY   POOL <PoolName>" +
            sLineSep +
            "REQUEST POOL <PoolName>" +
            sLineSep +
            "        [FIRST | RANDOM | ENTRY <Value> [RELEASE]] [PRIORITY <Number>]" +
            sLineSep +
            "        [TIMEOUT <Number>[s|m|h|d|w]] [GARBAGECOLLECT <Yes | No>]" +
            sLineSep +
            "RELEASE POOL <PoolName> ENTRY <Value> [FORCE]" +
            sLineSep +
            "CANCEL  POOL <PoolName>" +
            sLineSep +
            "        [FORCE [MACHINE <Machine>] [HANDLE <Handle #> | NAME <Handle Name>]]" +
            sLineSep +
            "        [ENTRY <Value>] [PRIORITY <Priority>] [FIRST | LAST]" +
            sLineSep +
            "VERSION" +
            sLineSep +
            "HELP";

        // Register Help Data
        registerHelpData(pData, kSTAFResPoolNotEntryOwner,
            STAFString("Not resource pool entry owner"),
            STAFString("You are not the owner of the entry you are trying to "
                       "RELEASE.  If a FORCE option is available for the "
                       "request, use it if you are sure that the correct "
                       "entry is specified."));

        registerHelpData(pData, kSTAFResPoolHasPendingRequests,
            STAFString("Resource pool has pending requests"),
            STAFString("The resource pool you are trying to DELETE has pending"
                       " requests.  If necessary, use the FORCE option."));

        registerHelpData(pData, kSTAFResPoolNoEntriesAvailable,
            STAFString("Resource pool has no entries available"),
            STAFString("The resource pool has no entries."));

        registerHelpData(pData, kSTAFResPoolCreatePoolPathError,
            STAFString("Error creating pool path"),
            STAFString("The directory specified by the DIRECTORY parameter "
                       "when registering the service or the default "
                       "directory could not be created."));

        registerHelpData(pData, kSTAFResPoolInvalidFileFormat,
            STAFString("Invalid pool file format"),
            STAFString("An error occurred reading the resource pool file "
                       "due to an error in the file format.  If you are "
                       "using the latest version of the Resource Pool "
                       "service, contact the STAF authors."));

        registerHelpData(pData, kSTAFResPoolEntryIsOwned,
            STAFString("Resource pool entry is owned"),
            STAFString("A resource pool entry you specified to REMOVE is "
                       "owned.  Use the FORCE option if you are sure that "
                       "the correct entry is specified."));

        registerHelpData(pData, kSTAFResPoolNotRequester,
            STAFString("Not pending requester"),
            STAFString("You cannot cancel a pending request you did not "
                       "submit unless you specify the FORCE option."));

        // Determine the pool directory

        STAFFSPath poolFilePath;

        if (pData->fPoolDir != STAFString())
        {
            // Set to pool directory using DIRECTORY parameter from RESPOOL
            // Service Configuration

            poolFilePath.setRoot(pData->fPoolDir);
        }
        else
        {   // Assign Default Pool Directory value if no DIRECTORY parameter
            // -  Use <pInfo->writeLocation>/<lower-case service name>
            
            poolFilePath.setRoot(pInfo->writeLocation);
            poolFilePath.addDir("service");
            poolFilePath.addDir(pData->fShortName.toLowerCase());
            pData->fPoolDir = poolFilePath.asString();
        }

        // Find all the pools (*.rpl files) in the pool directory
        
        STAFFSEntryType_t entryType = kSTAFFSFile;
        
        if (poolFilePath.exists())
        {
            STAFFSEnumPtr dirEnum = 
                poolFilePath.getEntry()->enumerate(kUTF8_STAR, sPoolExt, 
                                                   entryType);
            // Initialize each pool

            for (; dirEnum->isValid(); dirEnum->next())
            {
                STAFFSEntryPtr entry = dirEnum->entry();

                STAFString fileName = entry->path().asString();

                // Read the pool file and store its data in PoolData

                PoolData poolData;
                
                unsigned int status = readPoolFile(fileName, poolData);

                if (status == kReadEndOfFile)
                {
                    STAFString error( 
                        "STAFResPoolService.cpp: STAFServiceInit: "
                        "Invalid file contents in resource pool " + fileName);
                    cout << error << endl;
                    *pErrorBuffer = error.adoptImpl();
                    return kSTAFResPoolInvalidFileFormat;
                }
                else if (status == kReadInvalidFormat)
                {
                    STAFString error =
                       "STAFResPoolService.cpp: STAFServiceInit: " 
                       "Invalid file format (" + poolData.fileFormat;
                    error += ") in resource pool file " + fileName;
                    cout << error << endl;
                    *pErrorBuffer = error.adoptImpl();
                    return kSTAFResPoolInvalidFileFormat;
                }
                else if (status == kFileOpenError)
                {
                    STAFString error(
                       "STAFResPoolService.cpp: STAFServiceInit: "
                       "Error opening resource pool file " + fileName);
                    cout << error << endl;
                    *pErrorBuffer = error.adoptImpl();
                    return kSTAFFileOpenError;
                }

                // Add the pool data to the Pool Map

                pData->fPoolMap.insert(PoolMap::value_type(
                    poolData.poolName.toUpperCase(),
                    PoolDataPtr(new PoolData(poolData), PoolDataPtr::INIT)));
            }
        }
        else
        {
            // Create the pool directory
            try
            {
                STAFFSEntryPtr pooldir = 
                    poolFilePath.createDirectory(kSTAFFSCreatePath);
            }
            catch (...)
            { 
                STAFString error("STAFResPoolService.cpp: STAFServiceInit: "
                                 "Invalid ResPool Directory: " +
                                 pData->fPoolDir);
                cout << error << endl;
                *pErrorBuffer = error.adoptImpl();
                return kSTAFResPoolCreatePoolPathError;
            }
        }

        pData->fPoolMapRWSem = STAFRWSemPtr(new STAFRWSem, STAFRWSemPtr::INIT);
    }

    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                        "STAFResPoolService.cpp: STAFServiceInit").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFResPoolService.cpp: STAFServiceInit: "
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

        ResPoolServiceData *pData =
            reinterpret_cast<ResPoolServiceData *>(serviceHandle);

        STAFString request(pInfo->request);
        STAFString action = request.subWord(0, 1).toLowerCase();
        
        // Call functions for the request

        if (action == "create")
            result = handleCreate(pInfo, pData);
        else if (action == "delete")
            result = handleDelete(pInfo, pData);
        else if (action == "query")
            result = handleQuery(pInfo, pData);
        else if (action == "request")
            result = handleRequest(pInfo, pData);
        else if (action == "add")
            result = handleAdd(pInfo, pData);
        else if (action == "remove")
            result = handleRemove(pInfo, pData);
        else if (action == "release")
            result = handleRelease(pInfo, pData);
        else if (action == "cancel")
            result = handleCancel(pInfo, pData);
        else if (action == "list")
            result = handleList(pInfo, pData);
        else if (action == "staf_callback") 
            result = handleSTAFCallback(pInfo, pData);
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
            e, "STAFResPoolService.cpp: STAFServiceAcceptRequest").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFResPoolService.cpp: STAFServiceAcceptRequest: "
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
        
        ResPoolServiceData *pData =
            reinterpret_cast<ResPoolServiceData *>(serviceHandle);

        // Un-register Help Data

        unregisterHelpData(pData, kSTAFResPoolNotEntryOwner);
        unregisterHelpData(pData, kSTAFResPoolHasPendingRequests);
        unregisterHelpData(pData, kSTAFResPoolNoEntriesAvailable);
        unregisterHelpData(pData, kSTAFResPoolCreatePoolPathError);
        unregisterHelpData(pData, kSTAFResPoolInvalidFileFormat);
        unregisterHelpData(pData, kSTAFResPoolEntryIsOwned);
        unregisterHelpData(pData, kSTAFResPoolNotRequester);
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                        "STAFResPoolService.cpp: STAFServiceTerm").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFResPoolService.cpp: STAFServiceTerm: "
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
        ResPoolServiceData *pData =
            reinterpret_cast<ResPoolServiceData *>(*serviceHandle);

        delete pData;
        *serviceHandle = 0;

        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                    "STAFResPoolService.cpp: STAFServiceDestruct").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFResPoolService.cpp: STAFServiceDestruct: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


// Handles resource pool creation requests

STAFResultPtr handleCreate(STAFServiceRequestLevel30 *pInfo, 
                           ResPoolServiceData *pData)
{
    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "CREATE", pData->fLocalMachineName);

    // Parse the result

    STAFCommandParseResultPtr parsedResult =
        pData->fCreateParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;

    // Verify that the pool name does not contain any characters that can't be
    // used in a file name on the operating system.
    // We can't depend on fstream returning an error on some operating systems
    // like Windows XP because it doesn't always work correctly.
    // Also, by checking this here instead of relying on fstream, we can
    // provide a better error message.

    if (poolName.findFirstOf(sInvalidPoolNameCharacters) != STAFString::kNPos)
    {
        return STAFResultPtr(
            new STAFResult(
                kSTAFInvalidValue,
                STAFString("Invalid value for the POOL option: ") + poolName +
                STAFString("\nOn ") + sOS +
                STAFString(", a pool name cannot contain the following "
                           "characters: ") + sInvalidPoolNameCharacters), 
            STAFResultPtr::INIT);
    }

    // Set the poolDescription variable (resolve the pool description)

    resultPtr = resolveOp(pInfo, pData, parsedResult, sDescription);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolDescription = resultPtr->result;
 
    // Get a write lock on the Pool Map for the duration of this block

    STAFRWSemWLock wLock(*pData->fPoolMapRWSem);

    // Verify that the resource pool does not already exist in the Pool Map
    
    if ((pData->fPoolMap.find(poolName.toUpperCase())) != pData->fPoolMap.end())
    {
        return STAFResultPtr(new STAFResult(kSTAFAlreadyExists, poolName),
                             STAFResultPtr::INIT);
    }

    // Set the path for the resource pool file

    STAFFSPath poolFilePath;
    poolFilePath.setRoot(pData->fPoolDir);
    poolFilePath.setName(poolName);
    poolFilePath.setExtension(sPoolExt);

    PoolData poolData(poolName, poolDescription);

    // Write the pool data
    
    STAFString fileName = poolFilePath.asString();
    
    if (writePoolFile(fileName, poolData) != kReadorWriteOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileWriteError, fileName),
                             STAFResultPtr::INIT);
    }

    // Update in memory data structures

    pData->fPoolMap.insert(PoolMap::value_type(poolData.poolName.toUpperCase(),
                    PoolDataPtr(new PoolData(poolData), PoolDataPtr::INIT)));
    
    // Return an OK result

    return STAFResultPtr(new STAFResult(kSTAFOk, STAFString()),
                         STAFResultPtr::INIT);
}


// Handles resource pool deletion requests

STAFResultPtr handleDelete(STAFServiceRequestLevel30 *pInfo, 
                           ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

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
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;
    
    // Get a write lock on the Pool Map for the duration of this block.
    // Don't need a lock on Pool Data because I have a write lock on Pool Map.

    STAFRWSemWLock wLock(*pData->fPoolMapRWSem);

    // Make sure the resource pool is in the Pool Map and get a pointer to it

    PoolMap::iterator poolIterator;
    PoolDataPtr poolPtr;
    
    if ((poolIterator = pData->fPoolMap.find(poolName.toUpperCase())) == 
        pData->fPoolMap.end())
    {
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                             STAFResultPtr::INIT);
    }
    
    // Set a pointer to the resource pool to be deleted

    poolPtr = (*poolIterator).second;

    // Check if there are any pending requests and if FORCE is not specified.
    
    if ((poolPtr->requestList.size() != 0) && 
        (parsedResult->optionTimes(sForce) == 0))
    {
        return STAFResultPtr(new STAFResult(kSTAFResPoolHasPendingRequests,
                             poolName), STAFResultPtr::INIT);
    }
    
    // Now delete the resource pool file
    
    STAFFSPath poolFilePath;
    poolFilePath.setRoot(pData->fPoolDir);
    poolFilePath.setName(poolName);
    poolFilePath.setExtension(sPoolExt);
    
    try
    {
        poolFilePath.getEntry()->remove();
    }
    catch (STAFException &e)
    { 
        result = getExceptionString(e, 
                 "STAFResPoolSerice.cpp: Delete ").adoptImpl();
        return STAFResultPtr(new STAFResult(kSTAFFileDeleteError,
                             result), STAFResultPtr::INIT);                
    }
    
    // If there are any owned resources with garbage collection enabled,
    // delete the entries in the handle notification lists for garbage
    // collection

    for (unsigned int i = 0; i < poolPtr->resourceList.size(); i++)
    {
        if (poolPtr->resourceList[i].owned &&
            poolPtr->resourceList[i].garbageCollect)
        {
            submitSTAFNotifyUnregisterRequest(
                pData, poolPtr->resourceList[i].orgHandle,
                poolPtr->resourceList[i].orgEndpoint,
                poolPtr->resourceList[i].orgUUID);
        }
    }
    
    // Remove the resource pool from the Pool Map

    pData->fPoolMap.erase(poolIterator);

    // If there are any pending requests for this pool, wakeup each pending
    // requester with no resource specified.
  
    if (poolPtr->requestList.size() > 0)
    {
        RequestList::iterator iter;
        bool removedRequest = true;

        while (removedRequest)
        {
            removedRequest = false;

            for (iter = poolPtr->requestList.begin();
                 iter != poolPtr->requestList.end(); ++iter)
            {
                (*iter)->retCode = kSTAFDoesNotExist;
                (*iter)->resultBuffer = poolName;
                (*iter)->wakeup->post();

                // Break since can't continue iterating a list
                // that has been changed

                removedRequest = true;
                break;
            }
        }
    }
    
    // Return an Ok result

    return STAFResultPtr(new STAFResult(kSTAFOk, STAFString()),
                         STAFResultPtr::INIT);
}


// Handles resource pool add entry requests

STAFResultPtr handleAdd(STAFServiceRequestLevel30 *pInfo, 
                        ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "ADD", pData->fLocalMachineName);
    
    // Parse the result

    STAFCommandParseResultPtr parsedResult = 
        pData->fAddParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;

    // Get a write lock on the Pool Map for the duration of this block
    
    STAFRWSemWLock wLock(*pData->fPoolMapRWSem);

    // Make sure the resource pool is in the Pool Map and get a pointer to it

    PoolMap::iterator poolIterator;
    
    poolIterator = pData->fPoolMap.find(poolName.toUpperCase());

    if (poolIterator == pData->fPoolMap.end())
    {
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                             STAFResultPtr::INIT);
    }
        
    // Create a copy of the pool to update

    PoolDataPtr poolPtr = (*poolIterator).second;
    PoolData newPool = *(poolPtr);

    // Get each entry to be added and check if entry is already in the pool.       
    // If there are any duplicate entries to be added, return an error and
    // do not update the resource pool.

    unsigned int numEntriesToAdd = parsedResult->optionTimes(sEntry);
    unsigned int i;

    for (i = 1; i <= numEntriesToAdd; i++)
    {
        STAFString thisEntry = parsedResult->optionValue(sEntry, i);

        for (unsigned int j = 0; j < newPool.resourceList.size(); j++)
        {
            if (newPool.resourceList[j].entry == thisEntry)
            {
                 return STAFResultPtr(new STAFResult(kSTAFAlreadyExists,
                                                     thisEntry),
                                      STAFResultPtr::INIT);
            }
        }

        // Add to the end of the new pool's resource list if not a duplicate

        ResourceData resourceData(thisEntry);
        newPool.resourceList.push_back(resourceData);
        newPool.numResources++;
    }

    // Delete the old pool file and write the new pool file

    STAFFSPath poolFilePath;
    poolFilePath.setRoot(pData->fPoolDir);
    poolFilePath.setName(poolName);
    poolFilePath.setExtension(sPoolExt);

    try
    {
        poolFilePath.getEntry()->remove();
    }
    catch (STAFException &e)
    {
        result = getExceptionString(e,
                 "STAFResPoolService.cpp: Add ").adoptImpl();
        return STAFResultPtr(new STAFResult(kSTAFFileDeleteError, result),
                             STAFResultPtr::INIT);                
    }

    STAFString fileName = poolFilePath.asString();

    if (writePoolFile(fileName, newPool) != kReadorWriteOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileWriteError, fileName),
                             STAFResultPtr::INIT);
    }
    
    // Update the pool in memory

    poolPtr->resourceList = newPool.resourceList;
    poolPtr->numResources = newPool.numResources;

    // If there are any pending requests, check if any of the pending requests
    // any be satisfied by the entries just added.

    if (poolPtr->requestList.size() > 0)
    {
        bool removedRequest = true;
        RequestList::iterator iter;
        RequestDataPtr reqPtr;

        // For each resource entry added, iterate through the pending requests
        // and find the next requester whose request type is not kEntry and
        // tell the requester the resource he can have, wake the requester up,
        // and remove the satisfied pending request.

        for (i = poolPtr->numResources - numEntriesToAdd;
             i < poolPtr->numResources && removedRequest; i++)
        {
            removedRequest = false;
                
            for (iter = poolPtr->requestList.begin();
                 iter != poolPtr->requestList.end(); ++iter)
            {
                reqPtr = *iter;

                if (reqPtr->requestType != kEntry)
                {
                    // Assign a resource to the request

                    reqPtr->retCode = kSTAFOk;
                    reqPtr->resultBuffer = poolPtr->resourceList[i].entry;

                    // Update the resource entry's ownership information

                    poolPtr->resourceList[i].owned = 1;
                    poolPtr->usedResources++;
                    poolPtr->resourceList[i].orgUUID = reqPtr->orgUUID;
                    poolPtr->resourceList[i].orgMachine = reqPtr->orgMachine;
                    poolPtr->resourceList[i].orgName = reqPtr->orgName;
                    poolPtr->resourceList[i].orgHandle = reqPtr->orgHandle;
                    poolPtr->resourceList[i].orgUser = reqPtr->orgUser;
                    poolPtr->resourceList[i].orgEndpoint = reqPtr->orgEndpoint;
                    poolPtr->resourceList[i].requestedTime = 
                        reqPtr->requestedTime;
                    poolPtr->resourceList[i].acquiredTime = 
                        STAFTimestamp::now().asString();
                    poolPtr->resourceList[i].garbageCollect =
                        reqPtr->garbageCollect;

                    // Wakeup the requester

                    reqPtr->wakeup->post();
                    
                    // Remove the satisfied request from the pending
                    // request list

                    poolPtr->requestList.erase(iter);
                    
                    // Break since satisfied a pending request for this
                    // resource

                    removedRequest = true;
                    break;
                }
            }
        }
    }

    // Return an Ok result
    
    return STAFResultPtr(new STAFResult(kSTAFOk, result), STAFResultPtr::INIT);
}


// Handles resource pool remove entry requests

STAFResultPtr handleRemove(STAFServiceRequestLevel30 *pInfo, 
                           ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "REMOVE", pData->fLocalMachineName);
    
    // Parse the result

    STAFCommandParseResultPtr parsedResult = 
        pData->fRemoveParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;

    // Get a write lock on the Pool Map for the duration of this block

    STAFRWSemWLock wLock(*pData->fPoolMapRWSem);

    // Make sure the specified resource pool exists in Pool Map

    PoolMap::iterator poolIterator;
    
    poolIterator = pData->fPoolMap.find(poolName.toUpperCase());

    if (poolIterator == pData->fPoolMap.end())
    {  
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                             STAFResultPtr::INIT);
    }

    PoolDataPtr poolPtr = (*poolIterator).second;
    
    if (poolPtr->resourceList.size() == 0) 
    {
        return STAFResultPtr(new STAFResult(kSTAFResPoolNoEntriesAvailable,
                             poolName), STAFResultPtr::INIT);
    }

    unsigned int numEntriesToRemove = parsedResult->optionTimes(sEntry);
    
    NotifyUnregisterRequestList notifyUnregisterRequestList;

    // Create a copy of the pool to update

    PoolData newPool = *(poolPtr);

    // Get each of the entries to be removed; If an invalid entry is found,
    //  return with an appropriate error message and do not remove any other
    //  valid entries specified on the request.
   
    for (unsigned int i = 1; i <= numEntriesToRemove; i++)
    {
        STAFString thisEntry = parsedResult->optionValue(sEntry, i);
        bool entryExists = false;
        unsigned int resid;

        // Check if this entry is in the pool - set resid to its index

        for (unsigned int j = 0;
             j < newPool.resourceList.size() && !entryExists; j++)
        {
            if (thisEntry == newPool.resourceList[j].entry)
            {
                entryExists = true;
                resid = j;
            }
        }

        // Remove the entry if found and if owned with FORCE specified
        
        if (entryExists &&
            (!newPool.resourceList[resid].owned || 
             (parsedResult->optionTimes(sForce) != 0)))
        {
            if (newPool.resourceList[resid].owned)
            {
                newPool.usedResources--;

                // If garbage collection is to be performed, remove the
                // notification entry from the handle notification lists

                if (newPool.resourceList[resid].garbageCollect)
                {
                    // Generate STAF_NOTIFY UNREGISTER ONENDOFHANDLE request
                    // and save to be submitted the the local HANDLE service
                    // if the remove request is successful

                    STAFString request = "STAF_NOTIFY UNREGISTER "
                        "ONENDOFHANDLE " +
                        pData->fHandlePtr->wrapData(
                            STAFString(newPool.resourceList[resid].orgHandle)) +
                        " MACHINE " + pData->fHandlePtr->wrapData(
                            newPool.resourceList[resid].orgEndpoint) +
                        " UUID " + pData->fHandlePtr->wrapData(
                            newPool.resourceList[resid].orgUUID) +
                        " SERVICE " + pData->fHandlePtr->wrapData(
                            pData->fShortName) +
                        " KEY " + pData->fHandlePtr->wrapData(sNotificationKey);

                    notifyUnregisterRequestList.push_back(request);
                }
            }

            newPool.numResources--;
            newPool.resourceList.erase(newPool.resourceList.begin() + resid);
        }
        else if (!entryExists) 
        {
            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, thisEntry),
                                 STAFResultPtr::INIT);
        }
        else
        {
            //  Cannot remove pool entry if owned and FORCE not specified

            return STAFResultPtr(new STAFResult(
                kSTAFResPoolEntryIsOwned, thisEntry), STAFResultPtr::INIT);
        }
    } // end for each entry specified

    // Delete the old pool file and write the new pool file

    STAFFSPath poolFilePath;
    poolFilePath.setRoot(pData->fPoolDir);
    poolFilePath.setName(poolName);
    poolFilePath.setExtension(sPoolExt);

    try
    {
        poolFilePath.getEntry()->remove();
    }
    catch (STAFException &e)
    { 
        result = getExceptionString(e,
                             "STAFResPoolService.cpp: Remove ").adoptImpl();
        return STAFResultPtr(new STAFResult(kSTAFFileDeleteError, result),
                             STAFResultPtr::INIT);                
    }

    STAFString fileName = poolFilePath.asString();

    if (writePoolFile(fileName, newPool) != kReadorWriteOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileWriteError, fileName),
                             STAFResultPtr::INIT);
    }

    // Update the pool in memory

    poolPtr->resourceList = newPool.resourceList;
    poolPtr->numResources = newPool.numResources;
    poolPtr->usedResources = newPool.usedResources;

    // Submit the STAF_NOTIFY UNREGISTER requests to the local HANDLE service
    // for the owned resources (with garbage collection enabled) that were
    // removed so that notification entries will be removed from the handle
    // notification lists for garbage collection

    for (unsigned int j = 0; j < notifyUnregisterRequestList.size(); j++)
    {
        pData->fHandlePtr->submit(
            "local", "HANDLE", notifyUnregisterRequestList[j]);
    }

    // If there are any pending requests, remove any of the pending requests
    // that requested an entry that has just been removed from the resource
    // list (otherwise, the pending request will be stuck in the pending
    // request list until the entry is added).

    if (poolPtr->requestList.size() > 0)
    {
        // Iterate through the pending requests and find all pending
        // requests whose requestType is kEntry and whose requestedEntry
        // matches the entry which is being removed.  For each matching
        // pending request, remove the request from the pending request
        // list and assign the DoesNotExist return code and wake-up each
        // requester being removed.

        bool removedRequest = true;
        RequestList::iterator iter;
        RequestDataPtr reqPtr;

        while (removedRequest)
        {
            removedRequest = false;

            for (iter = poolPtr->requestList.begin(); 
                 iter != poolPtr->requestList.end() && !removedRequest;
                 ++iter)
            {
                reqPtr = *iter;

                if (reqPtr->requestType == kEntry)
                {
                    // Check if the requested entry matches an entry
                    // that was just deleted

                    for (unsigned int i = 1; i <= numEntriesToRemove; i++)
                    {
                        if (reqPtr->requestedEntry ==
                            parsedResult->optionValue(sEntry, i))
                        {
                            // Remove the satisfied request from the pending
                            // request list

                            poolPtr->requestList.erase(iter);

                            // Assign the return code and error message for
                            // the request

                            reqPtr->retCode = kSTAFDoesNotExist;

                            reqPtr->resultBuffer = STAFString("Entry ") +
                                reqPtr->requestedEntry + " does not exist";

                            // Wakeup the requester

                            reqPtr->wakeup->post();

                            // Break since can't continue iterating a list
                            // that has been changed

                            removedRequest = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Result an Ok result

    return STAFResultPtr(new STAFResult(kSTAFOk, result), STAFResultPtr::INIT);
}


// Handles resource pool request requests

STAFResultPtr handleRequest(STAFServiceRequestLevel30 *pInfo, 
                            ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 3

    VALIDATE_TRUST(3, pData->fShortName, "REQUEST", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fRequestParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;
    
    // Set the request type to random, first, or specified entry

    RequestType requestType = kRandom;  // Random is the default request type
    STAFString requestedEntry = STAFString("");

    if (parsedResult->optionTimes(sFirst) > 0)
    {
        requestType = kFirst;
    }
    else if (parsedResult->optionTimes(sEntry) > 0)
    {
        requestType = kEntry;
        
        resultPtr = resolveOp(pInfo, pData, parsedResult, sEntry);

        if (resultPtr->rc != 0) return resultPtr;

        requestedEntry = resultPtr->result;
    }

    unsigned int priority = sDefaultPriority;

    if (parsedResult->optionTimes(sPriority) > 0)
    {
        resultPtr = resolveOp(pInfo, pData, parsedResult, sPriority);

        if (resultPtr->rc != kSTAFOk) return resultPtr;

        // Convert resolved option string to an unsigned integer in range
        // 0 to 99

        resultPtr = convertOptionStringToUInt(
            resultPtr->result, sPriority, priority,
            sMinimumPriority, sMaximumPriority);

        if (resultPtr->rc != kSTAFOk) return resultPtr;
    }

    unsigned int timeout = STAF_EVENT_SEM_INDEFINITE_WAIT;

    if (parsedResult->optionTimes(sTimeout) > 0)
    {
        // Set the timeout variable (resolve the TIMEOUT option value and check if
        // it is a valid duration string and if so convert it to its numeric value
        // in milliseconds

        resultPtr = resolveOp(pInfo, pData, parsedResult, sTimeout);

        if (resultPtr->rc != 0) return resultPtr;

        STAFString_t errorBuffer = 0;

        STAFRC_t rc = STAFUtilConvertDurationString(
            resultPtr->result.getImpl(), &timeout, &errorBuffer);
        
        if (rc != kSTAFOk)
        {
            return STAFResultPtr(
                new STAFResult(
                    rc, STAFString("Invalid value for the TIMEOUT option: ") +
                    resultPtr->result + " \n\n" +
                    STAFString(errorBuffer, STAFString::kShallow)),
                STAFResultPtr::INIT);
        }
    }

    // Determine if garbage collection should be performed.

    bool garbageCollect = true;  // Default if GARBAGECOLLECT not specified

    if (parsedResult->optionTimes(sGarbageCollect) > 0)
    {
        // Set garbageCollect boolean (resolve the option value)

        STAFResultPtr resultPtr = resolveOp(
            pInfo, pData, parsedResult, sGarbageCollect);

        if (resultPtr->rc != 0) return resultPtr;

        STAFString gcValue = resultPtr->result;

        // If "YES", garbage collection will be performed when the handle
        // that requested the resource is deleted.  If "NO", no garbage
        // collection will be performed.

        if (gcValue.isEqualTo(sNo, kSTAFStringCaseInsensitive))
        {
            garbageCollect = false;
        }
        else if (gcValue.isEqualTo(sYes, kSTAFStringCaseInsensitive))
        {
            garbageCollect = true;
        }
        else
        {
            STAFString errMsg = STAFString("GARBAGECOLLECT value must be ") +
                "Yes or No.  Invalid value: " + gcValue;

            return STAFResultPtr(new STAFResult(kSTAFInvalidValue, errMsg),
                                 STAFResultPtr::INIT);
        }
    }

    // Determine if the RELEASE option was specified.

    bool release = false;

    if (parsedResult->optionTimes("RELEASE") > 0)
        release = true;

    // Start processing the request

    RequestDataPtr requestDataPtr;
    PoolMap::iterator poolIterator;
    PoolDataPtr poolPtr;
    
    // Get a read lock on the Pool Map for the duration of this block
    {
        STAFRWSemRLock rLock(*pData->fPoolMapRWSem);

        // Make sure the specified resource pool is in the PoolMap

        poolIterator = pData->fPoolMap.find(poolName.toUpperCase());

        if (poolIterator == pData->fPoolMap.end())
        {  
            return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                                 STAFResultPtr::INIT);
        }
    
        poolPtr = (*poolIterator).second;
        unsigned int i;
    
        // Lock the poolData semaphore for the duration of this block
        {
            STAFMutexSemLock lock(*poolPtr->accessSem);

            if (requestType == kEntry)
            {
                // Verify that the specified entry exists in the pool

                bool found = false;

                for (i = 0; i < poolPtr->resourceList.size(); i++)
                {
                    if (poolPtr->resourceList[i].entry == requestedEntry)
                        found = true;
                }

                if (!found)
                {
                    // Specified entry does not exist

                    return STAFResultPtr(
                        new STAFResult(
                            kSTAFDoesNotExist, STAFString("Entry ") +
                            requestedEntry + " does not exist"),
                        STAFResultPtr::INIT);
                }
            }
            else if (poolPtr->resourceList.size() == 0)
            {
                return STAFResultPtr(new STAFResult(
                    kSTAFResPoolNoEntriesAvailable, poolName),
                    STAFResultPtr::INIT);
            }
            
            if (garbageCollect)
            {
                // Register for notification when the handle ends
                
                STAFResultPtr resultPtr = submitSTAFNotifyRegisterRequest(
                    pData, pInfo->handle, pInfo->endpoint,
                    pInfo->stafInstanceUUID);

                if (resultPtr->rc != 0) return resultPtr;
            }
            
            // Check if there are any available resources

            bool isResourceAvailable = false;
            bool owner = false;
            unsigned int resid;

            if (requestType == kEntry)
            {
                // A specific entry was requested

                bool entryExists = false;

                // Check if the specified entry exists and is available

                for (i = 0; i < poolPtr->resourceList.size(); i++)
                {
                    if (poolPtr->resourceList[i].entry == requestedEntry)
                    {
                        if (!poolPtr->resourceList[i].owned)
                        {
                            isResourceAvailable = true;
                        }
                        else
                        {
                            // Resource is not available

                            if (release)
                            {
                                // Check if you are the owner of this entry

                                if ((poolPtr->resourceList[i].orgUUID ==
                                     pInfo->stafInstanceUUID) &&
                                    (poolPtr->resourceList[i].orgHandle ==
                                     pInfo->handle))
                                {
                                    owner = true; // You are the owner
                                    resid = i;
                                }
                            }
                        }

                        entryExists = true;
                        break;
                    }
                }

                if (!entryExists)
                {
                    return STAFResultPtr(
                        new STAFResult(
                            kSTAFDoesNotExist, STAFString("Entry ") +
                            requestedEntry + " does not exist"),
                        STAFResultPtr::INIT);
                }
            }
            else if ((poolPtr->numResources - poolPtr->usedResources) > 0)
            {
                // There are available resources and either the first
                // available entry or a random entry was requested
                
                unsigned int rIndex = rand() % (poolPtr->numResources -
                                                poolPtr->usedResources);

                // Find the first or Nth available resource in the list

                for (i = 0; i < poolPtr->resourceList.size(); i++)
                {
                    if ((!poolPtr->resourceList[i].owned) &&
                        ((requestType == kFirst) || !rIndex--))
                    {
                        isResourceAvailable = true;
                        break;
                    }
                }
            }
                
            if (release && !owner)
            {
                return STAFResultPtr(
                    new STAFResult(
                        kSTAFResPoolNotEntryOwner,
                        STAFString("The RELEASE option can only be used") +
                        "when requesting an entry that you already own"),
                    STAFResultPtr::INIT);
            }

            if (isResourceAvailable)
            {
                // Mark the resource as OWNED and return the entry info
                
                poolPtr->usedResources++;
                poolPtr->resourceList[i].owned = 1;
                poolPtr->resourceList[i].orgUUID = pInfo->stafInstanceUUID;
                poolPtr->resourceList[i].orgMachine = pInfo->machine;
                poolPtr->resourceList[i].orgName = pInfo->handleName;
                poolPtr->resourceList[i].orgHandle = pInfo->handle;
                poolPtr->resourceList[i].orgUser = pInfo->user;
                poolPtr->resourceList[i].orgEndpoint = pInfo->endpoint;
                STAFString currentTime = STAFTimestamp::now().asString();
                poolPtr->resourceList[i].requestedTime = currentTime;
                poolPtr->resourceList[i].acquiredTime = currentTime;
                poolPtr->resourceList[i].garbageCollect = garbageCollect;

                // Return the entry assigned to the request

                return STAFResultPtr(new STAFResult(kSTAFOk, 
                                     poolPtr->resourceList[i].entry),
                                     STAFResultPtr::INIT);
            }

            // Else no resources currently available; put on the pending list
    
            RequestData requestData(pInfo->stafInstanceUUID, pInfo->machine,
                                    pInfo->handleName, pInfo->handle,
                                    pInfo->user, pInfo->endpoint,
                                    garbageCollect,
                                    requestType, requestedEntry, priority);

            requestDataPtr = RequestDataPtr(new RequestData(requestData),
                                            RequestDataPtr::INIT);

            // Insert the request into the pending request list in
            // ascending order by priority/requestedTime
            
            RequestList::iterator iter;
            bool addedToList = false;

            for (iter = poolPtr->requestList.begin(); 
                 iter != poolPtr->requestList.end() && !addedToList;
                 ++iter)
            {
                if (priority < (*iter)->priority)
                {
                    poolPtr->requestList.insert(iter, requestDataPtr);
                    addedToList = true;
                }
            } // End for loop for iterating thru pending request list

            if (!addedToList)
            {
                // Add to the end of the pending request list

                poolPtr->requestList.push_back(requestDataPtr);
            }
            
            if (release && owner)
            {
                // Release this resource and determine the highest priority
                // pending request that can be satisfied and wake it up
                // to let it know it can have this resource now.

                if (poolPtr->resourceList[resid].garbageCollect)
                {
                    // Delete the notification from the handle notification list

                    submitSTAFNotifyUnregisterRequest(
                        pData, poolPtr->resourceList[resid].orgHandle,
                        poolPtr->resourceList[resid].orgEndpoint,
                        poolPtr->resourceList[resid].orgUUID);
                }

                // Mark the resource as available

                poolPtr->usedResources--;
                poolPtr->resourceList[resid].owned = 0;

                // Iterate through the pending requests.  Find the first pending
                // request that can be satisfied.  A pending request can be
                // satisfied if either:
                //   a) requestType != kEntry (e.g. kFirst or kRandom)
                //   or
                //   b) requestType == kEntry and the entry just released
                //      matches the requestedEntry.
                // If a pending request can be satisfied, tell this requester
                // the resource he can have.

                if (poolPtr->requestList.size() > 0)
                {
                    RequestDataPtr reqPtr;

                    for (iter = poolPtr->requestList.begin(); 
                         iter != poolPtr->requestList.end(); ++iter)
                    {
                        reqPtr = *iter;

                        if ((reqPtr->requestType != kEntry) ||
                           ((reqPtr->requestType == kEntry) &&
                            (reqPtr->requestedEntry ==
                             poolPtr->resourceList[resid].entry)))
                        {
                            // Assign the resource to the request

                            reqPtr->retCode = kSTAFOk;
                            reqPtr->resultBuffer = 
                                poolPtr->resourceList[resid].entry;

                            // Update the resource entry's ownership information

                            poolPtr->resourceList[resid].owned = 1;
                            poolPtr->usedResources++;
                            poolPtr->resourceList[resid].orgUUID = reqPtr->orgUUID;
                            poolPtr->resourceList[resid].orgMachine =
                                reqPtr->orgMachine;
                            poolPtr->resourceList[resid].orgName = reqPtr->orgName;
                            poolPtr->resourceList[resid].orgHandle =
                                reqPtr->orgHandle;
                            poolPtr->resourceList[resid].orgUser = reqPtr->orgUser;
                            poolPtr->resourceList[resid].orgEndpoint =
                                reqPtr->orgEndpoint;
                            poolPtr->resourceList[resid].requestedTime =
                                reqPtr->requestedTime;
                            poolPtr->resourceList[resid].acquiredTime =
                                STAFTimestamp::now().asString();
                            poolPtr->resourceList[resid].garbageCollect =
                                reqPtr->garbageCollect;

                            // Wakeup the requester

                            reqPtr->wakeup->post();

                            // Remove the satisfied request from the pending
                            // request list and break out of the loop

                            poolPtr->requestList.erase(iter);
                            break;
                        }
                    }
                }
            }

        } // End block for locking the PoolData access semaphore

    } // End block for putting a read lock on the PoolMap

    // Wait for the specified time for a resource to become available

    requestDataPtr->wakeup->wait(timeout);
            
    // Check if the handle that submitted this request was garbage
    // collected while we were waiting for a resource to become
    // available, and if so, return an error.
    // Save the cost of getting a semaphore lock by first checking if
    // the request was garbage collected

    if (*requestDataPtr->garbageCollectedPtr)
    {
        // Lock the poolData semaphore for the duration of this block

        STAFMutexSemLock lock(*poolPtr->accessSem);

        if (*requestDataPtr->garbageCollectedPtr)
        {
            return STAFResultPtr(
                new STAFResult(
                    kSTAFRequestCancelled,
                    "The handle that submitted this request no longer exists"),
                STAFResultPtr::INIT);
        }
    }

    // If request's return code is not 0 (e.g. timed out or if a specific
    // entry was requested but it no longer exists in the resource list,
    // or if the request was cancelled), remove the request from the list.
    // Save the cost of getting a semaphore lock by first checking if the
    // request failed.

    if (requestDataPtr->retCode != kSTAFOk)
    {
        // Lock the poolData semaphore for the duration of this block

        STAFMutexSemLock lock(*poolPtr->accessSem);

        if (requestDataPtr->retCode != kSTAFOk)
        {
            poolPtr->requestList.remove(requestDataPtr);

            if (garbageCollect)
            {
                // Delete the notification from the handle notification list

                submitSTAFNotifyUnregisterRequest(
                    pData, pInfo->handle, pInfo->endpoint,
                    pInfo->stafInstanceUUID);
            }
        }
    }

    // Return the return code and result assigned to the request

    return STAFResultPtr(new STAFResult(requestDataPtr->retCode,
                         requestDataPtr->resultBuffer), STAFResultPtr::INIT);
}                    


// Handles resource pool release entry requests

STAFResultPtr handleRelease(STAFServiceRequestLevel30 *pInfo, 
                            ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 3

    VALIDATE_TRUST(3, pData->fShortName, "RELEASE", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fReleaseParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;

    // Set the releaseEntry variable (don't resolve)

    STAFString releaseEntry = parsedResult->optionValue(sEntry);
   
    // Get a read lock on the Pool Map for the duration of this block

    STAFRWSemRLock rLock(*pData->fPoolMapRWSem);

    // Make sure that the resource pool is in pData->poolMap

    PoolMap::iterator poolIterator;
    PoolDataPtr poolPtr;

    poolIterator = pData->fPoolMap.find(poolName.toUpperCase());

    if (poolIterator == pData->fPoolMap.end())
    {  
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                             STAFResultPtr::INIT);
    }
    
    poolPtr = (*poolIterator).second;
    
    // Lock the poolData semaphore for the duration of this block
    
    STAFMutexSemLock lock(*poolPtr->accessSem);
    
    // Find the entry in the resource pool

    bool entryExists = false;
    unsigned int resid;

    for (unsigned int i = 0; i < poolPtr->resourceList.size(); i++)
    {
        if (releaseEntry == poolPtr->resourceList[i].entry)
        {
            entryExists = true;
            resid = i;
            break;
        }
    }

    // If the entry is not in the resource pool, return an error

    if (!entryExists)
    { 
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, releaseEntry),
                             STAFResultPtr::INIT);
    }

    // Check if the entry requested to be released is owned

    if (poolPtr->resourceList[resid].owned)
    {
        // Check if you are the owner

        unsigned int owner = 0;   // 0 means not the owner

        if ((poolPtr->resourceList[resid].orgUUID == pInfo->stafInstanceUUID) &&
            (poolPtr->resourceList[resid].orgHandle == pInfo->handle))
        {
            owner = 1;            // 1 means you are the owner
        }

        // If you are not the owner and FORCE is specified, need trust level 4

        unsigned int force = parsedResult->optionTimes(sForce);
        
        if (!owner && force)
        {
            // Verify the requester has at least trust level 4

            VALIDATE_TRUST2(4, pData->fShortName, "RELEASE FORCE",
                            pData->fLocalMachineName);
        }

        // To release, you must be the owner or FORCE must be specified
        
        if (owner || force)
        {
            // Release this resource and determine the highest priority
            // pending request that can be satisfied and wake it up
            // to let it know it can have this resource now.

            if (poolPtr->resourceList[resid].garbageCollect)
            {
                // Delete the notification from the handle notification list

                submitSTAFNotifyUnregisterRequest(
                    pData, poolPtr->resourceList[resid].orgHandle,
                    poolPtr->resourceList[resid].orgEndpoint,
                    poolPtr->resourceList[resid].orgUUID);
            }

            // Mark the resource as available

            poolPtr->usedResources--;
            poolPtr->resourceList[resid].owned = 0;

            // Iterate through the pending requests.  Find the first pending
            // request that can be satisfied.  A pending request can be
            // satisfied if either:
            //   a) requestType != kEntry (e.g. kFirst or kRandom)
            //   or
            //   b) requestType == kEntry and the entry just released
            //      matches the requestedEntry.
            // If a pending request can be satisfied, tell this requester
            // the resource he can have.

            if (poolPtr->requestList.size() > 0)
            {
                RequestList::iterator iter;
                RequestDataPtr reqPtr;

                for (iter = poolPtr->requestList.begin(); 
                     iter != poolPtr->requestList.end(); ++iter)
                {
                    reqPtr = *iter;

                    if ((reqPtr->requestType != kEntry) ||
                       ((reqPtr->requestType == kEntry) &&
                        (reqPtr->requestedEntry ==
                         poolPtr->resourceList[resid].entry)))
                    {
                        // Assign the resource to the request

                        reqPtr->retCode = kSTAFOk;
                        reqPtr->resultBuffer = 
                            poolPtr->resourceList[resid].entry;
                        
                        // Update the resource entry's ownership information
                        
                        poolPtr->resourceList[resid].owned = 1;
                        poolPtr->usedResources++;
                        poolPtr->resourceList[resid].orgUUID = reqPtr->orgUUID;
                        poolPtr->resourceList[resid].orgMachine =
                            reqPtr->orgMachine;
                        poolPtr->resourceList[resid].orgName = reqPtr->orgName;
                        poolPtr->resourceList[resid].orgHandle =
                            reqPtr->orgHandle;
                        poolPtr->resourceList[resid].orgUser = reqPtr->orgUser;
                        poolPtr->resourceList[resid].orgEndpoint =
                            reqPtr->orgEndpoint;
                        poolPtr->resourceList[resid].requestedTime =
                            reqPtr->requestedTime;
                        poolPtr->resourceList[resid].acquiredTime =
                            STAFTimestamp::now().asString();
                        poolPtr->resourceList[resid].garbageCollect =
                            reqPtr->garbageCollect;
                
                        // Wakeup the requester

                        reqPtr->wakeup->post();

                        // Remove the satisfied request from the pending
                        // request list and break out of the loop

                        poolPtr->requestList.erase(iter);
                        break;
                    }
                }
            }
        }
        else
        {
            return STAFResultPtr(new STAFResult(kSTAFResPoolNotEntryOwner, 
                                 releaseEntry), STAFResultPtr::INIT);
        }
    } // end if entry was owned

    // Return an Ok result

    return STAFResultPtr(new STAFResult(kSTAFOk, result), 
                         STAFResultPtr::INIT);
}


// Handles cancelling a pending request for a resource pool

STAFResultPtr handleCancel(STAFServiceRequestLevel30 *pInfo, 
                           ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 3

    VALIDATE_TRUST(3, pData->fShortName, "CANCEL", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        pData->fCancelParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;
    
    // Determine if the FORCE option is specified

    unsigned int force = parsedResult->optionTimes(sForce);

    // Default machine, handle, and handle name to that of the
    // originating requester who is submitting the CANCEL request

    STAFString orgMachine = pInfo->machine;
    STAFHandle_t orgHandle = pInfo->handle;
    STAFString orgHandleName = pInfo->handleName;
    
    if (force)
    {
        // Determine if the MACHINE option is specified

        if (parsedResult->optionTimes(sMachine) > 0)
        {
            resultPtr = resolveOp(pInfo, pData, parsedResult, sMachine);

            if (resultPtr->rc != 0) return resultPtr;
        
            orgMachine = resultPtr->result;
        }

        // Determine if the HANDLE or NAME option is specified

        if (parsedResult->optionTimes(sHandle) > 0)
        {
            resultPtr = resolveOp(pInfo, pData, parsedResult, sHandle);

            if (resultPtr->rc != kSTAFOk) return resultPtr;

            // Convert resolved option string to an unsigned integer in range
            // 1 to UINT_MAX
        
            unsigned int handle;

            resultPtr = convertOptionStringToUInt(
                resultPtr->result, sHandle, handle, 1);

            if (resultPtr->rc != kSTAFOk) return resultPtr;

            orgHandle = handle;
            orgHandleName = "";  // Indicates not to check for a match
        }
        else if (parsedResult->optionTimes(sName) > 0)
        {
            resultPtr = resolveOp(pInfo, pData, parsedResult, sName);

            if (resultPtr->rc != 0) return resultPtr;
        
            orgHandleName = resultPtr->result;
            orgHandle = 0;  // Indicates not to check for a match
        }
    }

    // Determine if the ENTRY option is specified

    STAFString requestedEntry;
    bool matchEntry = false;

    if (parsedResult->optionTimes(sEntry) > 0)
    {
        matchEntry = true;
        
        resultPtr = resolveOp(pInfo, pData, parsedResult, sEntry);

        if (resultPtr->rc != kSTAFOk) return resultPtr;

        requestedEntry = resultPtr->result;
    }

    // Determine if the PRIORITY option is specified

    unsigned int priority = sDefaultPriority;
    bool matchPriority = false;

    if (parsedResult->optionTimes(sPriority) > 0)
    {
        matchPriority = true;

        resultPtr = resolveOp(pInfo, pData, parsedResult, sPriority);

        if (resultPtr->rc != kSTAFOk) return resultPtr;

        // Convert resolved option string to an unsigned integer in range
        // 0 to 99

        resultPtr = convertOptionStringToUInt(
            resultPtr->result, sPriority, priority,
            sMinimumPriority, sMaximumPriority);

        if (resultPtr->rc != kSTAFOk) return resultPtr;
    }

    // Determine if the FIRST or LAST option is specified
    // The default is to find the last entry in the Pending Requests List
    // that matches the criteria.  The FIRST option can be specified to
    // find the first entry in the Pending Requests List that matches.

    bool first = false;   // Last is the default

    if (parsedResult->optionTimes(sFirst) > 0)
        first = true;

    // Get a read lock on the Pool Map for the duration of this block

    STAFRWSemRLock rLock(*pData->fPoolMapRWSem);

    // Make sure that the resource pool is in pData->poolMap

    PoolMap::iterator poolIterator;
    PoolDataPtr poolPtr;

    poolIterator = pData->fPoolMap.find(poolName.toUpperCase());

    if (poolIterator == pData->fPoolMap.end())
    {  
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                             STAFResultPtr::INIT);
    }
    
    poolPtr = (*poolIterator).second;

    // Lock the poolData semaphore for the duration of this block
    
    STAFMutexSemLock lock(*poolPtr->accessSem);
    
    // Check if a pending request entry matches all the following conditions:
    // - A handle# is not specified or it matches and
    // - A handle name is not specified or it matches (case-insensitive)
    // - A machine is not specified or it matches (case-insensitive)
    // - A entry is not specified or it matches (case-sensitive)
    // - A priority is not specified or it matches

    bool requestFound = false;
    RequestDataPtr reqPtr;
    
    if (first)
    {
        // The FIRST option was specified, so iterate forward in the Pending
        // Requests list to find the first entry that matches the criteria
        
        for (RequestList::iterator iter = poolPtr->requestList.begin();
             iter != poolPtr->requestList.end() && !requestFound; ++iter)
        {                                          
            reqPtr = *iter;

            if (((orgHandle == 0) || (reqPtr->orgHandle == orgHandle)) &&
                ((orgHandleName == "") || (reqPtr->orgName.isEqualTo(
                    orgHandleName, kSTAFStringCaseInsensitive))) &&
                ((orgMachine == "") || (reqPtr->orgMachine.isEqualTo(
                    orgMachine, kSTAFStringCaseInsensitive))) &&
                ((!matchEntry) || (reqPtr->requestedEntry == requestedEntry)) &&
                ((!matchPriority) || (reqPtr->priority == priority)))
            {
                requestFound = true;
            }
        }
    }
    else
    {
        // The LAST option is specified (or neither the FIRST or LAST option
        // was specified), so do a reverse iterate to find the last entry in
        // the Pending Requests list that matches the criteria

        for (RequestList::reverse_iterator riter = poolPtr->requestList.rbegin();
             riter != poolPtr->requestList.rend() && !requestFound; ++riter)
        {
            reqPtr = *riter;

            if (((orgHandle == 0) || (reqPtr->orgHandle == orgHandle)) &&
                ((orgHandleName == "") || (reqPtr->orgName.isEqualTo(
                    orgHandleName, kSTAFStringCaseInsensitive))) &&
                ((orgMachine == "") || (reqPtr->orgMachine.isEqualTo(
                    orgMachine, kSTAFStringCaseInsensitive))) &&
                ((!matchEntry) || (reqPtr->requestedEntry == requestedEntry)) &&
                ((!matchPriority) || (reqPtr->priority == priority)))
            {
                requestFound = true;
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

        if (matchEntry)
            noMatchMessage = noMatchMessage + " RequestedEntry=" +
                             requestedEntry;

        if (matchPriority)
            noMatchMessage = noMatchMessage + " Priority=" + priority;

        return STAFResultPtr(
            new STAFResult(kSTAFDoesNotExist, noMatchMessage),
            STAFResultPtr::INIT);
    }

    // Check if you are the requester of the matching pending request entry

    unsigned int requester = 0;   // 0 means not the requester

    if ((reqPtr->orgUUID == pInfo->stafInstanceUUID) &&
        (reqPtr->orgHandle == pInfo->handle))
    {
        requester = 1;    // 1 means you are the requester
    }

    // If you are not the requester and FORCE is specified, need trust level 4

    if (!requester && force)
    {
        // Verify the requester has at least trust level 4

        VALIDATE_TRUST2(4, pData->fShortName, "CANCEL FORCE",
                        pData->fLocalMachineName);
    }

    // To cancel the request, you must be the requester or the FORCE option
    // must be specified

    if (!requester && !force)
    {
        return STAFResultPtr(
            new STAFResult(
                kSTAFResPoolNotRequester,
                "Cannot cancel a request you did not submit "
                "unless you specify the FORCE option"),
            STAFResultPtr::INIT);
    }
    
    // Tell its requester the request was cancelled and wake it up

    reqPtr->retCode = kSTAFRequestCancelled;
    reqPtr->resultBuffer = "The request was cancelled by a RESPOOL CANCEL "
        "request";
    reqPtr->wakeup->post();

    return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
}


STAFResultPtr handleList(STAFServiceRequestLevel30 *pInfo, 
                         ResPoolServiceData *pData)
{    
    // Verify the requester has at least trust level 2

    VALIDATE_TRUST(2, pData->fShortName, "LIST", pData->fLocalMachineName);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fListParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("SETTINGS"))
    {
        // LIST SETTINGS

        // Create a marshalled map containing settings for the service
        
        mc->setMapClassDefinition(pData->fSettingsClass->reference());

        // Write the service settings to a map

        STAFObjectPtr outputMap = pData->fSettingsClass->createInstance();
        outputMap->put("directory", pData->fPoolDir);

        mc->setRootObject(outputMap);
    }
    else
    {
        // LIST POOLS (This is the default if only LIST is specified)

        // Create a marshalled list of maps containing pool information

        mc->setMapClassDefinition(pData->fPoolClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();

        // Get a read lock on the Pool Map for the duration of this block

        STAFRWSemRLock rLock(*pData->fPoolMapRWSem);

        // Write all the pool names to the result variable

        PoolMap::iterator poolIterator;

        for(poolIterator = pData->fPoolMap.begin(); 
            poolIterator != pData->fPoolMap.end(); ++poolIterator)
        {
            STAFObjectPtr poolInfoMap = pData->fPoolClass->createInstance();
            poolInfoMap->put("poolName", poolIterator->second->poolName);
            poolInfoMap->put("description", poolIterator->second->poolDescription);
            outputList->append(poolInfoMap);
        }           

        mc->setRootObject(outputList);
    }

    // Return an Ok result and the marshalled output for a list of the pools

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()),
                         STAFResultPtr::INIT);
}


// Handles resource pool query requests

STAFResultPtr handleQuery(STAFServiceRequestLevel30 *pInfo, 
                          ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;

    // Verify the requester has at least trust level 2

    VALIDATE_TRUST(2, pData->fShortName, "QUERY", pData->fLocalMachineName);
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = pData->fQueryParser->parse(
        pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    // Set the poolName variable (resolve the pool name)

    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);

    if (resultPtr->rc != 0) return resultPtr;

    STAFString poolName = resultPtr->result;

    // Create a marshalled map of general pool information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(pData->fPoolInfoClass->reference());
    mc->setMapClassDefinition(pData->fRequestClass->reference());
    mc->setMapClassDefinition(pData->fResourceClass->reference());
    mc->setMapClassDefinition(pData->fResourceOwnerClass->reference());

    STAFObjectPtr poolInfoMap = pData->fPoolInfoClass->createInstance();

    // Get a read lock on the Pool Map for the duration of this block

    STAFRWSemRLock rLock(*pData->fPoolMapRWSem);

    // Make sure the resource pool is in the Pool Map and get a pointer to it

    PoolMap::iterator poolIterator;
    PoolDataPtr poolPtr;

    poolIterator = pData->fPoolMap.find(poolName.toUpperCase());

    if (poolIterator == pData->fPoolMap.end())
    {  
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
                             STAFResultPtr::INIT);
    }

    // Set a pointer to the resource pool being queried

    poolPtr = (*poolIterator).second;
    
    poolInfoMap->put("description", poolPtr->poolDescription);

    // Create an empty list object for marshalled list of pending requests

    STAFObjectPtr requestList = STAFObject::createList();
    
    // Append an entry to requestList for each pending request

    if (poolPtr->requestList.size() > 0)
    {
        RequestList::iterator iList;

        for (iList = poolPtr->requestList.begin(); 
             iList != poolPtr->requestList.end(); ++iList)
        {
            // Add an entry in a marshalling context as a map to requestList

            STAFObjectPtr requestMap = pData->fRequestClass->createInstance();

            requestMap->put("machine", (*iList)->orgMachine);
            requestMap->put("handleName", (*iList)->orgName);
            requestMap->put("handle", STAFString((*iList)->orgHandle));
            requestMap->put("user", (*iList)->orgUser);
            requestMap->put("endpoint", (*iList)->orgEndpoint);
            requestMap->put("requestedTimestamp", (*iList)->requestedTime);
            requestMap->put("priority", (*iList)->priority);

            if ((*iList)->garbageCollect)
                requestMap->put("gc", sYes);
            else
                requestMap->put("gc", sNo);

            if ((*iList)->requestType == kEntry)
                requestMap->put("requestedEntry", (*iList)->requestedEntry);
            else
                requestMap->put("requestedEntry", STAFObject::createNone());

            requestList->append(requestMap);
        }
    }

    poolInfoMap->put("requestList", requestList);
      
    // Create an empty list object for the marshalled list of the pool's
    // resource entries

    STAFObjectPtr resourceList = STAFObject::createList();

    // Append an entry to resourceList for each resource

    for (unsigned int i = 0; i < poolPtr->resourceList.size(); i++)
    {
        // Create a map of information about each resource

        STAFObjectPtr resourceMap = pData->fResourceClass->createInstance();

        resourceMap->put("entry", poolPtr->resourceList[i].entry);

        if (!poolPtr->resourceList[i].owned)
        {
            resourceMap->put("owner", STAFObject::createNone());
        }
        else
        {
            // Create a map of information about the resource owner

            STAFObjectPtr resourceOwnerMap =
                pData->fResourceOwnerClass->createInstance();

            resourceOwnerMap->put("machine",
                                  poolPtr->resourceList[i].orgMachine);
            resourceOwnerMap->put("handleName",
                                  poolPtr->resourceList[i].orgName);
            resourceOwnerMap->put("handle", STAFString(
                                  poolPtr->resourceList[i].orgHandle));
            resourceOwnerMap->put("user", poolPtr->resourceList[i].orgUser);
            resourceOwnerMap->put("endpoint",
                                  poolPtr->resourceList[i].orgEndpoint);
            resourceOwnerMap->put("requestedTimestamp",
                                  poolPtr->resourceList[i].requestedTime);
            resourceOwnerMap->put("acquiredTimestamp",
                                  poolPtr->resourceList[i].acquiredTime);

            if (poolPtr->resourceList[i].garbageCollect)
                resourceOwnerMap->put("gc", "Yes");
            else
                resourceOwnerMap->put("gc", "No");

            resourceMap->put("owner", resourceOwnerMap);
        }

        // Add the entry in a marshalling context as a map to resourceList

        resourceList->append(resourceMap);
    }
    
    poolInfoMap->put("resourceList", resourceList);

    // Set the marshalling context's root object

    mc->setRootObject(poolInfoMap);

    // Return the marshalled result of the query

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()), 
                         STAFResultPtr::INIT);
}


STAFResultPtr handleSTAFCallback(STAFServiceRequestLevel30 *pInfo, 
                                 ResPoolServiceData *pData)
{
    STAFString result;
    STAFRC_t rc = kSTAFOk;
    
    // Don't check the TRUST level, but make sure the requesting handle
    // is the STAFProc handle

    if (pInfo->handle != 1)
    {
        return STAFResultPtr(
            new STAFResult(
                kSTAFAccessDenied, 
                "This request is only valid when submitted by STAFProc"),
            STAFResultPtr::INIT);
    }
    
    STAFCommandParseResultPtr parsedResult = 
        pData->fSTAFCallbackParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(
            new STAFResult(kSTAFInvalidRequestString,
                           parsedResult->errorBuffer),
            STAFResultPtr::INIT);
    }
  
    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "HANDLE");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString handle = resultPtr->result;
    
    resultPtr = resolveOp(pInfo, pData, parsedResult, "MACHINE");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString machine = resultPtr->result;
    
    resultPtr = resolveOp(pInfo, pData, parsedResult, "UUID");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString uuid = resultPtr->result;
    
    resultPtr = resolveOp(pInfo, pData, parsedResult, "KEY");

    if (resultPtr->rc != 0) return resultPtr;

    STAFString key = resultPtr->result;

    // Get a read lock on the Pool Map for the duration of this block

    STAFRWSemRLock rLock(*pData->fPoolMapRWSem);

    PoolMap::iterator poolIter;

    // For each pool, remove any pending requests that were requested by
    // this garbage collected handle and remove any resources that are owned
    // by this garbage collected handle

    for(poolIter = pData->fPoolMap.begin();
        poolIter != pData->fPoolMap.end(); ++poolIter)
    {

        STAFMutexSemLock lock(*poolIter->second->accessSem);
            
        // First, remove any pending requests for this pool that were
        // requested by this garbage collected handle that no longer exists 

        if (poolIter->second->requestList.size() > 0)
        {
            bool removedRequest = true;

            while (removedRequest)
            {
                removedRequest = false;

                for (RequestList::iterator iter =
                     poolIter->second->requestList.begin();
                     iter != poolIter->second->requestList.end(); ++iter)
                {
                    // Check if this garbage collected handle was the requester
                    // (if it's supposed to be garbage collected)

                    if (((*iter)->garbageCollect) &&
                        (STAFString((*iter)->orgHandle) == handle) &&
                        (STAFString((*iter)->orgUUID) == uuid))
                    {
                        // The requester for this entry in the request list
                        // no longer exists, so remove this pending request

                        poolIter->second->requestList.erase(iter);
                        
                        // Set the flag indicating that the pending request
                        // has been garbage collected and wake up the
                        // requester

                        *(*iter)->garbageCollectedPtr = true;
                        (*iter)->wakeup->post();
                        
                        // Break since can't continue iterating a list
                        // that has been changed

                        removedRequest = true;
                        break;
                    }
                }
            }
        }

        // Second, release any resources in this pool that are owned by this
        // garbage collected handle that no longer exist and then determine
        // the pending request(s) that can be satisfied and wake them up.

        for (unsigned int resid = 0; resid < poolIter->second->numResources;
             ++resid)
        {
            if ((poolIter->second->resourceList.size() != 0) &&
                (poolIter->second->resourceList[resid].owned))
            {
                // Check if this garbage collected handle is the owner of the
                // resource (if it's supposed to be garbage collected)

                if ((poolIter->second->resourceList[resid].garbageCollect) &&
                    (STAFString(poolIter->second->resourceList[resid].
                                orgHandle) == handle) &&
                    (STAFString(poolIter->second->resourceList[resid].
                                orgUUID) == uuid))
                {
                    // The owner of this resource no longer exists
                        
                    // Mark the resource as available
    
                    poolIter->second->usedResources--;
                    poolIter->second->resourceList[resid].owned = 0;

                    // Iterate through the pending requests.  Find the first
                    // pending request that can be satisfied.
                    // A pending request can be satisfied if either:
                    //   a) requestType != kEntry (e.g. kFirst or kRandom)
                    //   or
                    //   b) requestType == kEntry and the entry just released
                    //      matches the requestedEntry.
                    // If a pending request can be satisfied, tell this
                    // requester the resource he can have.

                    if (poolIter->second->requestList.size() > 0)
                    {
                        RequestList::iterator iter;
                        RequestDataPtr reqPtr;

                        for (iter = poolIter->second->requestList.begin(); 
                             iter != poolIter->second->requestList.end(); ++iter)
                        {
                            reqPtr = *iter;

                            if ((reqPtr->requestType != kEntry) ||
                               ((reqPtr->requestType == kEntry) &&
                                (reqPtr->requestedEntry ==
                                 poolIter->second->resourceList[resid].entry)))
                            {
                                // Assign the resource to the request

                                reqPtr->retCode = kSTAFOk;
                                reqPtr->resultBuffer = 
                                    poolIter->second->resourceList[resid].entry;

                                // Update the resource entry's ownership info

                                poolIter->second->resourceList[resid].owned = 1;
                                poolIter->second->usedResources++;
                                poolIter->second->resourceList[resid].orgUUID =
                                    reqPtr->orgUUID;
                                poolIter->second->resourceList[resid].orgMachine =
                                    reqPtr->orgMachine;
                                poolIter->second->resourceList[resid].orgName =
                                    reqPtr->orgName;
                                poolIter->second->resourceList[resid].orgHandle =
                                    reqPtr->orgHandle;
                                poolIter->second->resourceList[resid].orgUser =
                                    reqPtr->orgUser;
                                poolIter->second->resourceList[resid].orgEndpoint =
                                    reqPtr->orgEndpoint;
                                poolIter->second->resourceList[resid].requestedTime =
                                    reqPtr->requestedTime;
                                poolIter->second->resourceList[resid].acquiredTime =
                                    STAFTimestamp::now().asString();
                                poolIter->second->resourceList[resid].garbageCollect =
                                    reqPtr->garbageCollect;

                                // Wakeup the requester

                                reqPtr->wakeup->post();

                                // Remove the satisfied request from the pending
                                // request list and break out of the loop

                                poolIter->second->requestList.erase(iter);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return STAFResultPtr(new STAFResult(kSTAFOk, result), STAFResultPtr::INIT);
}


STAFResultPtr handleHelp(STAFServiceRequestLevel30 *pInfo, 
                         ResPoolServiceData *pData)
{
    // Verify the requester has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "HELP", pData->fLocalMachineName);
    
    // Return help text

    return STAFResultPtr(new STAFResult(kSTAFOk, sHelpMsg),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleVersion(STAFServiceRequestLevel30 *pInfo, 
                            ResPoolServiceData *pData)
{    
    // Verify the requester has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "VERSION", pData->fLocalMachineName);
    
    return STAFResultPtr(new STAFResult(kSTAFOk, sVersionInfo), 
                         STAFResultPtr::INIT);
}


STAFResultPtr submitSTAFNotifyRegisterRequest(ResPoolServiceData *pData,
                                              STAFHandle_t handle,
                                              STAFString endpoint,
                                              STAFString uuid)
{
    // Submit a STAF_NOTIFY REGISTER request to the local HANDLE service to
    // add the notification

    STAFString request = "STAF_NOTIFY REGISTER ONENDOFHANDLE " +
        pData->fHandlePtr->wrapData(STAFString(handle)) +
        " MACHINE " + pData->fHandlePtr->wrapData(endpoint) +
        " UUID " + pData->fHandlePtr->wrapData(uuid) +
        " SERVICE " + pData->fHandlePtr->wrapData(pData->fShortName) +
        " KEY " + pData->fHandlePtr->wrapData(sNotificationKey);

    STAFResultPtr resultPtr = pData->fHandlePtr->submit(
        "local", "HANDLE", request);
    
    if (resultPtr->rc == kSTAFOk)
    {
        return resultPtr;
    }
    else
    {
        return STAFResultPtr(
            new STAFResult(
                resultPtr->rc,
                STAFString("An error occurred when the ") + pData->fShortName +
                " service on machine '" + pData->fLocalMachineName +
                "' attempted to register for garbage collection notification"
                " on endpoint '" + endpoint + "' for handle '" +
                STAFString(handle) + "'.  Reason: " + resultPtr->result),
            STAFResultPtr::INIT);
    }
}


STAFResultPtr submitSTAFNotifyUnregisterRequest(ResPoolServiceData *pData,
                                       STAFHandle_t handle,
                                       STAFString endpoint,
                                       STAFString uuid)
{
    // Submit a STAF_NOTIFY UNREGISTER request to the local HANDLE service to
    // delete the notification

    STAFString request = "STAF_NOTIFY UNREGISTER ONENDOFHANDLE " +
        pData->fHandlePtr->wrapData(STAFString(handle)) +
        " MACHINE " + pData->fHandlePtr->wrapData(endpoint) +
        " UUID " + pData->fHandlePtr->wrapData(uuid) +
        " SERVICE " + pData->fHandlePtr->wrapData(pData->fShortName) +
        " KEY " + pData->fHandlePtr->wrapData(sNotificationKey);

    STAFResultPtr resultPtr = pData->fHandlePtr->submit(
        "local", "HANDLE", request);
    
    if (resultPtr->rc == kSTAFOk)
    {
        return resultPtr;
    }
    else
    {
        return STAFResultPtr(
            new STAFResult(
                resultPtr->rc,
                STAFString("An error occurred when the ") + pData->fShortName +
                " service on machine '" + pData->fLocalMachineName +
                "' attempted to unregister for garbage collection notification"
                " on endpoint '" + endpoint + "' for handle '" +
                STAFString(handle) + "'.  Reason: " + resultPtr->result),
            STAFResultPtr::INIT);
    }
}


STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
                         ResPoolServiceData *pData, 
                         const STAFString &theString)
{
    return pData->fHandlePtr->submit(sLocal, sVar, sResStrResolve +
                                     STAFString(pInfo->requestNumber) +
                                     sString +
                                     pData->fHandlePtr->wrapData(theString));
}


STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, 
                        ResPoolServiceData *pData,
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


STAFResultPtr resolveOpLocal(ResPoolServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption, 
                             unsigned int optionIndex)
{
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


void registerHelpData(ResPoolServiceData *pData, unsigned int errorNumber,
                      const STAFString &shortInfo, const STAFString &longInfo)
{
    static STAFString regString("REGISTER SERVICE %C ERROR %d INFO %C "
                                "DESCRIPTION %C");

    pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber,
        shortInfo.getImpl(), longInfo.getImpl()));
}


void unregisterHelpData(ResPoolServiceData *pData, unsigned int errorNumber)
{
    static STAFString regString("UNREGISTER SERVICE %C ERROR %d");

    pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber));
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


// Read a resource pool file and store its contents in PoolData.
// Currently, there are two formats of resource pool files supported:
// - 1 is the current binary format used by the C++ Respool Service in STAF 2.3
//   and later
// - "0" is the old text format used by the REXX Respool Service in STAF 2.2
//   and earlier

unsigned int readPoolFile(const STAFString &fileName, PoolData &poolData)
{
    unsigned int rc = kReadorWriteOk;

    // Open the pool file (in binary mode)

    fstream poolfile(fileName.toCurrentCodePage()->buffer(),
                     ios::in | STAF_ios_binary);

    if (!poolfile) return kFileOpenError;
    
    // Check the format of the file to determine how to read the file

    readUIntFromFile(poolfile, poolData.fileFormat);

    if (poolfile.eof()) return kReadEndOfFile;

    if (poolData.fileFormat == sCurrFileFormat)
    {
        // This is the current binary format used by the C++ Respool Service

        readStringFromFile(poolfile, poolData.poolName);
        readStringFromFile(poolfile, poolData.poolDescription);
        readUIntFromFile(poolfile, poolData.numResources);
        
        STAFString entry;
        
        for (unsigned int i = 0; i < poolData.numResources; i++) 
        {
            readStringFromFile(poolfile, entry);
            
            ResourceData resourceData(entry);
            poolData.resourceList.push_back(resourceData);
        }
    }
    else
    {
        // Check if has the old text format used by the Rexx Respool Service

        // Close and reopen the pool file (in text mode)

        poolfile.close();
        poolfile.open(fileName.toCurrentCodePage()->buffer(), ios::in);
        if (!poolfile) return kFileOpenError;

        STAFString line;
        char poolLine[1024];

        // Read pool file format line

        poolLine[0] = 0;
        poolfile.getline(poolLine, 1024);
        
        if (poolfile.eof()) return kReadEndOfFile;
        
        STAFString line1(poolLine);
        STAFString fileFormatString = line1;

        // Validate file format and process based on file format

        if (fileFormatString != STAFString("0"))
        {   
            // Unknown/invalid file format
            return kReadInvalidFormat;
        }
       
        // A String "0" format was used by the old REXX Respool service

        poolData.fileFormat = 0;

        // Read pool name line
        poolLine[0] = 0;
        poolfile.getline(poolLine, 1024);

        if (poolfile.eof()) return kReadEndOfFile;

        STAFString line2(poolLine);
        poolData.poolName = line2;
        
        // Read pool description line
        poolLine[0] = 0;
        poolfile.getline(poolLine, 1024);
            
        if (poolfile.eof()) return kReadEndOfFile;
           
        STAFString line3(poolLine);
        poolData.poolDescription = line3;

        // Read resource entry lines
        poolLine[0] = 0;
        poolfile.getline(poolLine, 1024);
 
        unsigned int i = 0;

        for (i = 0; !poolfile.eof() && !rc; i++)
        {
            STAFString entry(poolLine);
            ResourceData resourceData(entry);
            poolData.resourceList.push_back(resourceData);
            
            // Get next line
            poolLine[0] = 0;
            poolfile.getline(poolLine, 1024);       
        }

        poolData.numResources = i;
    }

    return rc;
}


// Write the resource pool file.  Note that pool files are always written
// in the current format.

unsigned int writePoolFile(const STAFString &fileName, PoolData &poolData)
{
    // Open the pool file

    fstream poolfile(fileName.toCurrentCodePage()->buffer(),
                    ios::out | STAF_ios_binary);

    if (!poolfile) return kFileOpenError;
    
    // Write to the pool file

    writeUIntToFile(poolfile, sCurrFileFormat);
    writeStringToFile(poolfile, poolData.poolName);
    writeStringToFile(poolfile, poolData.poolDescription);
    unsigned int numResources = poolData.resourceList.size();
    writeUIntToFile(poolfile, numResources);
    
    for (unsigned int i = 0; i < numResources; i++) 
    {
        writeStringToFile(poolfile, poolData.resourceList[i].entry);
    }

    return kReadorWriteOk;
}

