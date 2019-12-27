/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <map>
#include <list>
#include "STAFLogService.h"
#include "STAF_fstream.h"
#include "STAF_iostream.h"
#include "STAFString.h"
#include "STAFException.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFTimestamp.h"
#include "STAFUtil.h"
#include "STAFInternalUtil.h"
#include "STAFFileSystem.h"
#include "STAFRWSem.h"

//#define STAF_DO_TIMING
#include "STAFTiming.h"

// Note: The handleXXX functions for the Log service behave slightly differently
//       from other services.  They parse the input before checking the trust
//       of the requester.  This is necessary since requests may be forwarded
//       from another system.  These requests may contain RMTMACHINE options,
//       which specify the originating system.

// Type definitions

// The STAFLogFileLocks class represents the necessary locks needed to access
// a specific log file.  The static acquireLocks() method is used to obtain
// access to the appropriate locks.
//
// Internally, this class maintains a map of all the locks for "active" logs.
// An "active" log is one for which there is a request currently being
// processed.  Once no more requests are working on a log, the log is no
// longer "active" and is removed from the map.  This is handled by maintaining
// a "count" of all requests using the locks for a given log.  When the lock
// structure is deleted, the count is decremented, and, if it is zero, the
// lock structure is removed from the map.
//
// This internal mechanism is used so that the map of log locks doesn't continue
// to grow, which could lead to resource exhaustion (more likely OS semaphores
// than memory).  With this, the map stays very small, which also aids in
// performance (of lookups).

class STAFLogFileLocks;
typedef STAFRefPtr<STAFLogFileLocks> STAFLogFileLocksPtr;

class STAFLogFileLocks
{
public:

    // This is the method used to get access to the locks for a given
    // log file.
    static STAFLogFileLocksPtr acquireLocks(const STAFString &logFile);

    // DEBUG
    static void dumpLockData();

    // The logAccess sem allows general access to the log.
    // A read lock should be obtained for any general read/write operation.
    // A write lock should be obtainef for any destructive type of change, such
    // as a purge/delete
    STAFRWSemPtr logAccess;

    // The recordAccess sem allows access to individual records.  You should
    // acquire this sem before attempting to read or write any log records
    // from/to the file.
    // Note: You should already have a logAccess lock before acquiring this lock
    STAFMutexSemPtr recordAccess;

    ~STAFLogFileLocks()
    {
        releaseLocks(logFile);
    }

private:

    // This is private so that users must go through acquireLocks()
    STAFLogFileLocks(STAFRWSemPtr theLogAccess,
                     STAFMutexSemPtr theRecordAccess,
                     const STAFString &theLogFile) :
        logAccess(theLogAccess),
        recordAccess(theRecordAccess),
        logFile(theLogFile)
    { /* Do nothing */ }

    STAFString logFile;

    // This is private so that is only called by the destructor
    static void releaseLocks(const STAFString &logFile);

    // This structure holds the real semaphores and access count
    struct LogLocks
    {
        LogLocks() : logAccess(new STAFRWSem, STAFRWSemPtr::INIT),
                     recordAccess(new STAFMutexSem, STAFMutexSemPtr::INIT),
                     count(1)
        { /* Do nothing */ }

        STAFRWSemPtr logAccess;
        STAFMutexSemPtr recordAccess;
        unsigned int count;
    };

    typedef std::map<STAFString, LogLocks> LogLocksMap;

    static STAFMutexSem logLocksMutex;
    static std::map<STAFString, LogLocks> logLocks;
};

STAFMutexSem STAFLogFileLocks::logLocksMutex;
STAFLogFileLocks::LogLocksMap STAFLogFileLocks::logLocks;

void STAFLogFileLocks::dumpLockData()
{
    STAFMutexSemLock lock(logLocksMutex);

    for (LogLocksMap::iterator iter = logLocks.begin();
         iter != logLocks.end();
         ++iter)
    {
        cout << iter->first << ": " << iter->second.count << endl;
    }
}

STAFLogFileLocksPtr STAFLogFileLocks::acquireLocks(const STAFString &logFile)
{
    STAFString logFileName = logFile.toLowerCase();
    STAFMutexSemLock lock(logLocksMutex);

    LogLocksMap::iterator iter = logLocks.find(logFileName);

    if (iter != logLocks.end())
    {
        ++iter->second.count;

        return STAFLogFileLocksPtr(new STAFLogFileLocks(
                                           iter->second.logAccess,
                                           iter->second.recordAccess,
                                           logFileName),
                                   STAFLogFileLocksPtr::INIT);
    }
    else
    {
        LogLocks theLogLocks;
        logLocks[logFileName] = theLogLocks;

        return STAFLogFileLocksPtr(new STAFLogFileLocks(
                                           theLogLocks.logAccess,
                                           theLogLocks.recordAccess,
                                           logFileName),
                                   STAFLogFileLocksPtr::INIT);
    }
}

void STAFLogFileLocks::releaseLocks(const STAFString &logFile)
{
    STAFMutexSemLock lock(logLocksMutex);
    LogLocksMap::iterator iter = logLocks.find(logFile);

    if ((iter != logLocks.end()) && (--iter->second.count == 0))
    {
        logLocks.erase(iter);
    }
}

typedef STAFRefPtr<STAFCommandParser> STAFCommandParserPtr;

struct LogServiceData
{
    unsigned int fDebugMode;               // Not used, currently
    STAFString fName;                      // Registered service name
    STAFString fShortName;                 // Short service name
    STAFString fResolveLogMaskString;      // String for resolve log mask
    STAFString fResolveMessageString;      // String for resolve message var
    STAFString fRoot;                      // Root of log directory
    STAFString fRemoteLogServer;           // Name of remote log server
    STAFString fRemoteLogService;          // Name of remote log service
    STAFHandlePtr fHandle;                 // Log service's STAF handle
    unsigned int fDefaultResolveMessage;   // Default for resolving messages
    unsigned int fMaxRecordSize;           // Maximum log record size
    unsigned int fDefaultMaxQueryRecords;  // Default maximum records to return
                                           // on a generic QUERY request
    unsigned int fUseResolveMessageVar;    // Honor S/S/Log/ResolveMessage
    unsigned int fRLogMode;                // Are we in RLog mode?
    STAFCommandParserPtr fParmsParser;
    STAFCommandParserPtr fLogParser;
    STAFCommandParserPtr fQueryParser;
    STAFCommandParserPtr fListParser;
    STAFCommandParserPtr fDeleteParser;
    STAFCommandParserPtr fPurgeParser;
    STAFCommandParserPtr fSetParser;
    STAFString fDefaultAuthenticator;      // Default Authenticator
    STAFString fLocalMachineName;

    // Map Class Definitions for marshalled results
    STAFMapClassDefinitionPtr fLogRecordClass;
    STAFMapClassDefinitionPtr fLogRecordLongClass;
    STAFMapClassDefinitionPtr fQueryStatsClass;
    STAFMapClassDefinitionPtr fPurgeStatsClass;
    STAFMapClassDefinitionPtr fListLocalSettingsClass;
    STAFMapClassDefinitionPtr fListRemoteSettingsClass;
    STAFMapClassDefinitionPtr fListLogsClass;
};


struct LogRecord
{
    LogRecord() : recordFormatID(0), date(0), secondsPastMidnight(0),
                  logLevel(0), handle(0), recordNumber(0)
    { /* Do Nothing */ }

    LogRecord(unsigned int aDate, unsigned int seconds, unsigned int level,
              const STAFString &aMachine, const STAFString &aHandleName,
              STAFHandle_t aHandle, const STAFString(&aUser),
              const STAFString(&aEndpoint), const STAFString &aMessage)
        : recordFormatID(0), date(aDate), secondsPastMidnight(seconds),
          logLevel(level), machine(aMachine), handleName(aHandleName),
          handle(aHandle), user(aUser), endpoint(aEndpoint), message(aMessage),
          recordNumber(0)
    { /* Do Nothing */ }

    unsigned int recordFormatID;
    unsigned int date;
    unsigned int secondsPastMidnight;
    unsigned int logLevel;
    STAFString machine;
    STAFString handleName;
    STAFHandle_t handle;
    STAFString user;
    STAFString endpoint;
    STAFString message;

    // Note: Record number is not stored in the logfile
    unsigned int recordNumber;
};


typedef std::deque<STAFString> StringList;
typedef std::deque<STAFHandle_t> HandleList;
typedef struct
{
    unsigned int date;
    unsigned int seconds;
} LogTimestamp;

struct LogRecordFilter
{
    LogRecordFilter() : useLevelMask(false), useFrom(false), useAfter(false),
                        useBefore(false), useTo(false)
    { /* Do Nothing */ }

    StringList contains;
    StringList cscontains;
    StringList startswith;
    StringList csstartswith;
    StringList qMachines;
    StringList names;
    StringList users;
    StringList endpoints;
    HandleList qHandles;
    bool useLevelMask;
    unsigned int levelMask;
    bool useFrom;
    LogTimestamp fromTimestamp;
    bool useAfter;
    LogTimestamp afterTimestamp;
    bool useBefore;
    LogTimestamp beforeTimestamp;
    bool useTo;
    LogTimestamp toTimestamp;
};


enum ReadLogRecordRC
{
    kReadLogOk = 0,
    kReadLogEndOfFile = 1,
    kReadLogInvalidFormat = 2
};

struct LogStats
{
    unsigned int fatal;
    unsigned int error;
    unsigned int warning;
    unsigned int info;
    unsigned int trace;
    unsigned int trace2;
    unsigned int trace3;
    unsigned int debug;
    unsigned int debug2;
    unsigned int debug3;
    unsigned int start;
    unsigned int stop;
    unsigned int pass;
    unsigned int fail;
    unsigned int status;
    unsigned int user1;
    unsigned int user2;
    unsigned int user3;
    unsigned int user4;
    unsigned int user5;
    unsigned int user6;
    unsigned int user7;
    unsigned int user8;
};

// Some global variables

static STAFString sHelpMsg;
static STAFString sLineSep;
static const STAFString sVersionInfo("3.4.0");
static const STAFString sZeroOne("01");
static const STAFString sOne("1");
static const STAFString sLogExt("log");
static const STAFString sTmpExt("tmp");
static const STAFString sLeftCurly(kUTF8_LCURLY);
static const STAFString sSpace(kUTF8_SPACE);
static const STAFString sEqual(kUTF8_EQUAL);
static const STAFString sSlash(kUTF8_SLASH);
static const STAFString sColon(kUTF8_COLON);
static const STAFString sSpecSeparator(sColon + sSlash + sSlash);
static const STAFString sTimestampSeps("-@");
static const STAFString sLocal("LOCAL");
static const STAFString sVar("VAR");
static const STAFString sResStrResolve("RESOLVE REQUEST ");
static const STAFString sString(" STRING ");
static const STAFString sTrust("TRUST");
static const STAFString sMisc("MISC");
static const STAFString sRESOLVE("RESOLVE");
static const STAFString sEverythingLogMask("11111111111111111111111111111111");
static const STAFString sOldSep(kUTF8_VBAR);
static const STAFString sEOLString("RESOLVE STRING {STAF/Config/Sep/Line}");
static const STAFString sResMachineString("RESOLVE STRING {STAF/Config/Machine}");
static const STAFString sDefAuthString(
    "RESOLVE STRING {STAF/Config/DefaultAuthenticator}");
static const STAFString sListDots("........................................");
static const STAFString sSizeEquals("Size=");
static const STAFString sGetMachine("GET MACHINE ");
static const STAFString sGetUser(" USER ");
static const STAFString sMachine("MACHINE ");
static const STAFString sLOG("LOG");
static const STAFString sQUERY("QUERY");
static const STAFString sLIST("LIST");
static const STAFString sPURGE("PURGE");
static const STAFString sDELETE("DELETE");
static const STAFString sHELP("HELP");
static const STAFString sSET("SET");
static const STAFString sVERSION("VERSION");
static const STAFString sGLOBAL("GLOBAL");
static const STAFString sMACHINE("MACHINE");
static const STAFString sMACHINES("MACHINES");
static const STAFString sHANDLE("HANDLE");
static const STAFString sHANDLES("HANDLES");
static const STAFString sLOGNAME("LOGNAME");
static const STAFString sLEVEL("LEVEL");
static const STAFString sMESSAGE("MESSAGE");
static const STAFString sRESOLVEMESSAGE("RESOLVEMESSAGE");
static const STAFString sNORESOLVEMESSAGE("NORESOLVEMESSAGE");
static const STAFString sCONTAINS("CONTAINS");
static const STAFString sCSCONTAINS("CSCONTAINS");
static const STAFString sSTARTSWITH("STARTSWITH");
static const STAFString sCSSTARTSWITH("CSSTARTSWITH");
static const STAFString sQMACHINE("QMACHINE");
static const STAFString sQHANDLE("QHANDLE");
static const STAFString sNAME("NAME");
static const STAFString sUSER("USER");
static const STAFString sENDPOINT("ENDPOINT");
static const STAFString sLEVELMASK("LEVELMASK");
static const STAFString sFROM("FROM");
static const STAFString sAFTER("AFTER");
static const STAFString sFROMRECORD("FROMRECORD");
static const STAFString sTORECORD("TORECORD");
static const STAFString sBEFORE("BEFORE");
static const STAFString sTO("TO");
static const STAFString sLEVELBITSTRING("LEVELBITSTRING");
static const STAFString sFIRST("FIRST");
static const STAFString sLAST("LAST");
static const STAFString sALL("ALL");
static const STAFString sSTATS("STATS");
static const STAFString sTOTAL("TOTAL");
static const STAFString sLONG("LONG");
static const STAFString sCONFIRMALL("CONFIRMALL");
static const STAFString sDIRECTORY("DIRECTORY");
static const STAFString sRMTMACHINE("RMTMACHINE");
static const STAFString sRMTNICKNAME("RMTNICKNAME");
static const STAFString sRMTHANDLE("RMTHANDLE");
static const STAFString sRMTNAME("RMTNAME");
static const STAFString sRMTUSER("RMTUSER");
static const STAFString sRMTMACH("RMTMACH");
static const STAFString sMAXRECORDSIZE("MAXRECORDSIZE");
static const STAFString sDEFAULTMAXQUERYRECORDS("DEFAULTMAXQUERYRECORDS");
static const STAFString sENABLERESOLVEMESSAGEVAR("ENABLERESOLVEMESSAGEVAR");
static const STAFString sDISABLERESOLVEMESSAGEVAR("DISABLERESOLVEMESSAGEVAR");
static const STAFString sSETTINGS("SETTINGS");
static const STAFString sENABLEREMOTELOGGING("ENABLEREMOTELOGGING");
static const STAFString sREMOTELOGSERVER("REMOTELOGSERVER");
static const STAFString sREMOTELOGSERVICE("REMOTELOGSERVICE");
static const STAFString sTODAY("TODAY");

static const STAFString sFATAL("FATAL");
static const STAFString sERROR("ERROR");
static const STAFString sWARNING("WARNING");
static const STAFString sINFO("INFO");
static const STAFString sTRACE("TRACE");
static const STAFString sTRACE2("TRACE2");
static const STAFString sTRACE3("TRACE3");
static const STAFString sDEBUG("DEBUG");
static const STAFString sDEBUG2("DEBUG2");
static const STAFString sDEBUG3("DEBUG3");
static const STAFString sSTART("START");
static const STAFString sSTOP("STOP");
static const STAFString sPASS("PASS");
static const STAFString sFAIL("FAIL");
static const STAFString sSTATUS("STATUS");
static const STAFString sUSER1("USER1");
static const STAFString sUSER2("USER2");
static const STAFString sUSER3("USER3");
static const STAFString sUSER4("USER4");
static const STAFString sUSER5("USER5");
static const STAFString sUSER6("USER6");
static const STAFString sUSER7("USER7");
static const STAFString sUSER8("USER8");

static STAFString sFATALPretty("Fatal");
static STAFString sERRORPretty("Error");
static STAFString sWARNINGPretty("Warning");
static STAFString sINFOPretty("Info");
static STAFString sTRACEPretty("Trace");
static STAFString sTRACE2Pretty("Trace2");
static STAFString sTRACE3Pretty("Trace3");
static STAFString sDEBUGPretty("Debug");
static STAFString sDEBUG2Pretty("Debug2");
static STAFString sDEBUG3Pretty("Debug3");
static STAFString sSTARTPretty("Start");
static STAFString sSTOPPretty("Stop");
static STAFString sPASSPretty("Pass");
static STAFString sFAILPretty("Fail");
static STAFString sSTATUSPretty("Status");
static STAFString sUSER1Pretty("User1");
static STAFString sUSER2Pretty("User2");
static STAFString sUSER3Pretty("User3");
static STAFString sUSER4Pretty("User4");
static STAFString sUSER5Pretty("User5");
static STAFString sUSER6Pretty("User6");
static STAFString sUSER7Pretty("User7");
static STAFString sUSER8Pretty("User8");
static STAFString sUNKNOWNPretty("Unknown");

static STAFString sFATALBits  ("00000000000000000000000000000001");
static STAFString sERRORBits  ("00000000000000000000000000000010");
static STAFString sWARNINGBits("00000000000000000000000000000100");
static STAFString sINFOBits   ("00000000000000000000000000001000");
static STAFString sTRACEBits  ("00000000000000000000000000010000");
static STAFString sTRACE2Bits ("00000000000000000000000000100000");
static STAFString sTRACE3Bits ("00000000000000000000000001000000");
static STAFString sDEBUGBits  ("00000000000000000000000010000000");
static STAFString sDEBUG2Bits ("00000000000000000000000100000000");
static STAFString sDEBUG3Bits ("00000000000000000000001000000000");
static STAFString sSTARTBits  ("00000000000000000000010000000000");
static STAFString sSTOPBits   ("00000000000000000000100000000000");
static STAFString sPASSBits   ("00000000000000000001000000000000");
static STAFString sFAILBits   ("00000000000000000010000000000000");
static STAFString sSTATUSBits ("00000000000000000100000000000000");
static STAFString sUSER1Bits  ("00000001000000000000000000000000");
static STAFString sUSER2Bits  ("00000010000000000000000000000000");
static STAFString sUSER3Bits  ("00000100000000000000000000000000");
static STAFString sUSER4Bits  ("00001000000000000000000000000000");
static STAFString sUSER5Bits  ("00010000000000000000000000000000");
static STAFString sUSER6Bits  ("00100000000000000000000000000000");
static STAFString sUSER7Bits  ("01000000000000000000000000000000");
static STAFString sUSER8Bits  ("10000000000000000000000000000000");
static STAFString sUNKNOWNBits("00000000000000000000000000000000");

static const STAFString sUnauthenticatedUser = "none" +
    sSpecSeparator + "anonymous";

static const unsigned int sCurrRecordFormatID = 4;


// Prototypes

static STAFResultPtr handleLog(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handleQuery(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handlePurge(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handleList(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handleDelete(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handleSet(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handleHelp(STAFServiceRequestLevel30 *, LogServiceData *);
static STAFResultPtr handleRemoteLog(STAFServiceRequestLevel30 *,
                                     LogServiceData *);
static STAFResultPtr handleRemoteLogGeneral(STAFServiceRequestLevel30 *,
                                            LogServiceData *);

static bool isValidLogLevel(const STAFString &levelString,
                            unsigned int &outputLevel);

static bool convertLogLevelToUInt(const STAFString &levelString,
                                  unsigned int &outputLevel);

static STAFString &convertLogLevelToString(unsigned int logLevel,
                                           bool levelAsBits = false);

static bool convertLogMaskToUInt(const STAFString &logmaskString,
                                 unsigned int &logMask);

void readUIntFromFile(istream &input, unsigned int &data,
                     unsigned int length = 4);

void writeUIntToFile(ostream &output, unsigned int data,
                     unsigned int length = 4);

void readStringFromFile(istream &input, STAFString &inString);

void writeStringToFile(ostream &output, const STAFString &outString);

STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData,
                         const STAFString &theString);

STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData,
                        STAFCommandParseResultPtr &parsedResult,
                        const STAFString &fOption,
                        unsigned int optionIndex = 1);

STAFResultPtr resolveOpLocal(LogServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption,
                             unsigned int optionIndex = 1);

static STAFResultPtr convertStringToUInt(
    const STAFString &theString,
    unsigned int &number,
    const unsigned int minValue = 0,
    const unsigned int maxValue = UINT_MAX);

static STAFResultPtr convertOptionStringToUInt(
    const STAFString &theString,
    const STAFString &optionName,
    unsigned int &number,
    const unsigned int minValue = 0,
    const unsigned int maxValue = UINT_MAX);

bool generateQueryPurgeDeleteLogFilePath(STAFFSPath &logfilePath,
     STAFResultPtr &errorResult, STAFServiceRequestLevel30 *pInfo,
     LogServiceData *pData, STAFCommandParseResultPtr &parsedResult);

bool updateQueryPurgeLogFilter(LogRecordFilter &logFilter,
     STAFResultPtr &errorResult, STAFServiceRequestLevel30 *pInfo,
     LogServiceData *pData, STAFCommandParseResultPtr &parsedResult);

unsigned int readLogRecordFromFile(istream &input, LogRecord &logRecord,
                                   unsigned int recordNumber);

void writeLogRecordToFile(ostream &output, LogRecord &logRecord);

void addLogRecordToList(STAFObjectPtr &logList,
                        STAFMapClassDefinitionPtr &logRecordClass,
                        const LogRecord &logRecord, bool levelAsBits,
                        bool longFormat);

void printLogRecord(const LogRecord &logRecord);

bool logRecordMatchesFilter(const LogRecord &logRecord,
                            const LogRecordFilter &logFilter,
                            const STAFString &defaultAuthenticator);

void updateLogStats(LogStats &logStats, unsigned int logLevel);

void addLogStatsToMap(STAFObjectPtr &queryStatsMap, const LogStats &logStats);

static void registerHelpData(LogServiceData *pData, unsigned int errorNumber,
                             const STAFString &shortInfo,
                             const STAFString &longInfo);

static void unregisterHelpData(LogServiceData *pData, unsigned int errorNumber);


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
        LogServiceData data;

        data.fDebugMode = 0;
        data.fRLogMode = 0;
        data.fMaxRecordSize = 100000;
        data.fDefaultMaxQueryRecords = 100;
        data.fDefaultResolveMessage = 0;
        data.fUseResolveMessageVar = 0;
        data.fRemoteLogService = sLOG;
        data.fShortName = STAFString(pInfo->name).toUpperCase();
        data.fName = "STAF/Service/";
        data.fName += pInfo->name;
        data.fResolveLogMaskString = sLeftCurly + data.fName + "/Mask}";
        data.fResolveMessageString = sLeftCurly + data.fName +
                                     "/ResolveMessage}";

        // Walk through and verify the config options
        // Note: The log service does not currently have any options

        for (unsigned int i = 0; i < pInfo->numOptions; ++i)
        {
            STAFString optionError("Invalid option, ");
            optionError += pInfo->pOptionName[i];
            *pErrorBuffer = optionError.adoptImpl();
            return kSTAFServiceConfigurationError;
        }

        // Setup parsers

        // PARMS parser

        data.fParmsParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        data.fParmsParser->addOption(
            "DIRECTORY",        1, STAFCommandParser::kValueRequired);
        data.fParmsParser->addOption(
            "ENABLEREMOTELOGGING", 1, STAFCommandParser::kValueNotAllowed);
        data.fParmsParser->addOption(
            "REMOTELOGSERVER",  1, STAFCommandParser::kValueRequired);
        data.fParmsParser->addOption(
            "REMOTELOGSERVICE", 1, STAFCommandParser::kValueRequired);
        data.fParmsParser->addOption(
            "MAXRECORDSIZE",    1, STAFCommandParser::kValueRequired);
        data.fParmsParser->addOption(
            "DEFAULTMAXQUERYRECORDS", 1, STAFCommandParser::kValueRequired);
        data.fParmsParser->addOption(
            "RESOLVEMESSAGE",   1, STAFCommandParser::kValueNotAllowed);
        data.fParmsParser->addOption(
            "NORESOLVEMESSAGE", 1, STAFCommandParser::kValueNotAllowed);
        data.fParmsParser->addOption(
            "ENABLERESOLVEMESSAGEVAR",  1, STAFCommandParser::kValueNotAllowed);
        data.fParmsParser->addOption(
            "DISABLERESOLVEMESSAGEVAR", 1, STAFCommandParser::kValueNotAllowed);

        data.fParmsParser->addOptionGroup(
            "NORESOLVEMESSAGE RESOLVEMESSAGE", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLERESOLVEMESSAGEVAR " "DISABLERESOLVEMESSAGEVAR", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING DIRECTORY", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING MAXRECORDSIZE", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING DEFAULTMAXQUERYRECORDS", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING RESOLVEMESSAGE", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING NORESOLVEMESSAGE", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING ENABLERESOLVEMESSAGEVAR", 0, 1);
        data.fParmsParser->addOptionGroup(
            "ENABLEREMOTELOGGING DISABLERESOLVEMESSAGEVAR", 0, 1);

        data.fParmsParser->addOptionNeed("REMOTELOGSERVER REMOTELOGSERVICE",
                                         "ENABLEREMOTELOGGING");
        data.fParmsParser->addOptionNeed("ENABLEREMOTELOGGING",
                                         "REMOTELOGSERVER");

        // LOG parser

        data.fLogParser = STAFCommandParserPtr(new STAFCommandParser,
                                               STAFCommandParserPtr::INIT);
        data.fLogParser->addOption(
            "LOG",              1, STAFCommandParser::kValueNotAllowed);
        data.fLogParser->addOption(
            "GLOBAL",           1, STAFCommandParser::kValueNotAllowed);
        data.fLogParser->addOption(
            "MACHINE",          1, STAFCommandParser::kValueNotAllowed);
        data.fLogParser->addOption(
            "HANDLE",           1, STAFCommandParser::kValueNotAllowed);
        data.fLogParser->addOption(
            "LOGTYPE",          1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "LOGNAME",          1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "LEVEL",            1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "MESSAGE",          1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "RESOLVEMESSAGE",   1, STAFCommandParser::kValueNotAllowed);
        data.fLogParser->addOption(
            "NORESOLVEMESSAGE", 1, STAFCommandParser::kValueNotAllowed);
        data.fLogParser->addOption(
            "RMTMACHINE",       1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "RMTNICKNAME",      1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "RMTNAME",          1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "RMTHANDLE",        1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "RMTUSER",          1, STAFCommandParser::kValueRequired);
        data.fLogParser->addOption(
            "RMTMACH",          1, STAFCommandParser::kValueRequired);
        
        data.fLogParser->addOptionGroup("LOG", 1, 1);
        data.fLogParser->addOptionGroup("LEVEL", 1, 1);
        data.fLogParser->addOptionGroup("MESSAGE", 1, 1);
        data.fLogParser->addOptionGroup("GLOBAL MACHINE HANDLE LOGTYPE", 1, 1);
        data.fLogParser->addOptionGroup("LOGNAME", 1, 1);
        data.fLogParser->addOptionGroup("RESOLVEMESSAGE NORESOLVEMESSAGE",
                                        0, 1);

        data.fLogParser->addOptionNeed("RMTMACHINE", "RMTUSER");
        data.fLogParser->addOptionNeed(
            "RMTNAME RMTHANDLE RMTUSER RMTMACH RMTNICKNAME", "RMTMACHINE");
        
        // QUERY parser

        data.fQueryParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        data.fQueryParser->addOption(
            "QUERY",            1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "GLOBAL",           1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "MACHINE",          1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "HANDLE",           1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "LOGNAME",          1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "LEVELMASK",        1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "QMACHINE",         0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "QHANDLE",          0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "NAME",             0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "USER",             0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "ENDPOINT",         0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "FIRST",            1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "LAST",             1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "ALL",              1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "TOTAL",            1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "STATS",            1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "LONG",            1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "CONTAINS",         0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "CSCONTAINS",       0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "STARTSWITH",       0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "CSSTARTSWITH",     0, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "LEVELBITSTRING",   1, STAFCommandParser::kValueNotAllowed);
        data.fQueryParser->addOption(
            "FROM",             1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "AFTER",            1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "BEFORE",           1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "TO",               1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "FROMRECORD",       1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "TORECORD",         1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "RMTMACHINE",       1, STAFCommandParser::kValueRequired);
        data.fQueryParser->addOption(
            "RMTUSER",          1, STAFCommandParser::kValueRequired);

        data.fQueryParser->addOptionGroup("QUERY", 1, 1);
        data.fQueryParser->addOptionGroup("GLOBAL MACHINE", 1, 1);
        data.fQueryParser->addOptionGroup("LOGNAME", 1, 1);
        data.fQueryParser->addOptionGroup("FROM AFTER", 0, 1);
        data.fQueryParser->addOptionGroup("BEFORE TO", 0, 1);
        data.fQueryParser->addOptionGroup("FIRST LAST ALL", 0, 1);
        data.fQueryParser->addOptionGroup("TOTAL STATS LONG", 0, 1);

        data.fQueryParser->addOptionNeed("HANDLE", "MACHINE");
        data.fQueryParser->addOptionNeed("RMTMACHINE", "RMTUSER");
        data.fQueryParser->addOptionNeed("RMTUSER", "RMTMACHINE");
        
        // PURGE parser

        data.fPurgeParser = STAFCommandParserPtr(new STAFCommandParser,
                                                 STAFCommandParserPtr::INIT);
        data.fPurgeParser->addOption(
            "PURGE",            1, STAFCommandParser::kValueNotAllowed);
        data.fPurgeParser->addOption(
            "GLOBAL",           1, STAFCommandParser::kValueNotAllowed);
        data.fPurgeParser->addOption(
            "MACHINE",          1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "HANDLE",           1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "LOGNAME",          1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "LEVELMASK",        1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "QMACHINE",         0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "QHANDLE",          0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "NAME",             0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "USER",             0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "ENDPOINT",         0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "FIRST",            1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "LAST",             1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "CONTAINS",         0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "CSCONTAINS",       0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "STARTSWITH",       0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "CSSTARTSWITH",     0, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "FROM",             1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "AFTER",            1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "BEFORE",           1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "TO",               1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "FROMRECORD",       1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "TORECORD",         1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "RMTMACHINE",       1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "RMTUSER",          1, STAFCommandParser::kValueRequired);
        data.fPurgeParser->addOption(
            "CONFIRM",          1, STAFCommandParser::kValueNotAllowed);
        data.fPurgeParser->addOption(
            "CONFIRMALL",       1, STAFCommandParser::kValueNotAllowed);

        data.fPurgeParser->addOptionGroup("PURGE", 1, 1);
        data.fPurgeParser->addOptionGroup("GLOBAL MACHINE", 1, 1);
        data.fPurgeParser->addOptionGroup("LOGNAME", 1, 1);
        data.fPurgeParser->addOptionGroup("CONFIRM CONFIRMALL", 1, 1);
        data.fPurgeParser->addOptionGroup("FROM AFTER", 0, 1);
        data.fPurgeParser->addOptionGroup("BEFORE TO", 0, 1);
        data.fPurgeParser->addOptionGroup("FIRST LAST", 0, 1);

        data.fPurgeParser->addOptionNeed("HANDLE", "MACHINE");
        data.fPurgeParser->addOptionNeed("RMTMACHINE", "RMTUSER");
        data.fPurgeParser->addOptionNeed("RMTUSER", "RMTMACHINE");

        // LIST parser

        data.fListParser = STAFCommandParserPtr(new STAFCommandParser,
                                                STAFCommandParserPtr::INIT);
        data.fListParser->addOption(
            "LIST",             1, STAFCommandParser::kValueNotAllowed);
        data.fListParser->addOption(
            "GLOBAL",           1, STAFCommandParser::kValueNotAllowed);
        data.fListParser->addOption(
            "MACHINES",         1, STAFCommandParser::kValueNotAllowed);
        data.fListParser->addOption(
            "MACHINE",          1, STAFCommandParser::kValueRequired);
        data.fListParser->addOption(
            "HANDLES",          1, STAFCommandParser::kValueNotAllowed);
        data.fListParser->addOption(
            "HANDLE",           1, STAFCommandParser::kValueRequired);
        data.fListParser->addOption(
            "RMTMACHINE",       1, STAFCommandParser::kValueRequired);
        data.fListParser->addOption(
            "RMTUSER",          1, STAFCommandParser::kValueRequired);
        data.fListParser->addOption(
            "SETTINGS",         1, STAFCommandParser::kValueNotAllowed);

        data.fListParser->addOptionGroup("LIST", 1, 1);
        data.fListParser->addOptionGroup("GLOBAL MACHINES MACHINE SETTINGS",
                                         1, 1);
        data.fListParser->addOptionGroup("HANDLES HANDLE", 0, 1);

        data.fListParser->addOptionNeed("HANDLES HANDLE", "MACHINE");
        data.fListParser->addOptionNeed("RMTMACHINE", "RMTUSER");
        data.fListParser->addOptionNeed("RMTUSER", "RMTMACHINE");


        // DELETE parser

        data.fDeleteParser = STAFCommandParserPtr(new STAFCommandParser,
                                                  STAFCommandParserPtr::INIT);
        data.fDeleteParser->addOption(
            "DELETE",           1, STAFCommandParser::kValueNotAllowed);
        data.fDeleteParser->addOption(
            "GLOBAL",           1, STAFCommandParser::kValueNotAllowed);
        data.fDeleteParser->addOption(
            "MACHINE",          1, STAFCommandParser::kValueRequired);
        data.fDeleteParser->addOption(
            "HANDLE",           1, STAFCommandParser::kValueRequired);
        data.fDeleteParser->addOption(
            "LOGNAME",          1, STAFCommandParser::kValueRequired);
        data.fDeleteParser->addOption(
            "CONFIRM",          1, STAFCommandParser::kValueNotAllowed);
        data.fDeleteParser->addOption(
            "RMTMACHINE",       1, STAFCommandParser::kValueRequired);
        data.fDeleteParser->addOption(
            "RMTUSER",          1, STAFCommandParser::kValueRequired);

        data.fDeleteParser->addOptionGroup("DELETE", 1, 1);
        data.fDeleteParser->addOptionGroup("GLOBAL MACHINE", 1, 1);
        data.fDeleteParser->addOptionGroup("LOGNAME", 1, 1);
        data.fDeleteParser->addOptionGroup("CONFIRM", 1, 1);

        data.fDeleteParser->addOptionNeed("HANDLE", "MACHINE");
        data.fDeleteParser->addOptionNeed("RMTMACHINE", "RMTUSER");
        data.fDeleteParser->addOptionNeed("RMTUSER", "RMTMACHINE");

        // SET parser

        data.fSetParser = STAFCommandParserPtr(new STAFCommandParser,
                                               STAFCommandParserPtr::INIT);
        data.fSetParser->addOption(
            "SET",              1, STAFCommandParser::kValueNotAllowed);
        data.fSetParser->addOption(
            "MAXRECORDSIZE",    1, STAFCommandParser::kValueRequired);
        data.fSetParser->addOption(
            "DEFAULTMAXQUERYRECORDS", 1, STAFCommandParser::kValueRequired);
        data.fSetParser->addOption(
            "NORESOLVEMESSAGE", 1, STAFCommandParser::kValueNotAllowed);
        data.fSetParser->addOption(
            "RESOLVEMESSAGE",   1, STAFCommandParser::kValueNotAllowed);
        data.fSetParser->addOption(
            "ENABLERESOLVEMESSAGEVAR",  1, STAFCommandParser::kValueNotAllowed);
        data.fSetParser->addOption(
            "DISABLERESOLVEMESSAGEVAR", 1, STAFCommandParser::kValueNotAllowed);

        data.fSetParser->addOptionGroup("NORESOLVEMESSAGE RESOLVEMESSAGE",
                                        0, 1);
        data.fSetParser->addOptionGroup("ENABLERESOLVEMESSAGEVAR "
                                        "DISABLERESOLVEMESSAGEVAR",
                                        0, 1);
 
        // Construct the map class for the marshalled QUERY log-record output

        data.fLogRecordClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/LogRecord");

        data.fLogRecordClass->addKey("timestamp",  "Date-Time");
        data.fLogRecordClass->addKey("level",      "Level");
        data.fLogRecordClass->addKey("message",    "Message");

        // Construct the map class for the marshalled QUERY LONG log-record
        // output

        data.fLogRecordLongClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/LogRecordLong");

        data.fLogRecordLongClass->addKey("recordNumber", "Record #");
        data.fLogRecordLongClass->setKeyProperty(
            "recordNumber", "display-short-name", "R#"); 
        data.fLogRecordLongClass->addKey("timestamp",  "Date-Time");
        data.fLogRecordLongClass->addKey("machine",    "Machine");
        data.fLogRecordLongClass->addKey("handle",     "Handle");
        data.fLogRecordLongClass->setKeyProperty(
            "handle", "display-short-name", "H#"); 
        data.fLogRecordLongClass->addKey("handleName", "Handle Name");
        data.fLogRecordLongClass->setKeyProperty(
            "handleName", "display-short-name", "Name"); 
        data.fLogRecordLongClass->addKey("user",       "User");
        data.fLogRecordLongClass->addKey("endpoint",   "Endpoint");
        data.fLogRecordLongClass->addKey("level",      "Level");
        data.fLogRecordLongClass->addKey("message",    "Message");

        // Construct the map class for the marshalled QUERY STATS output

        data.fQueryStatsClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/QueryStats");

        data.fQueryStatsClass->addKey("fatal",   sFATALPretty);
        data.fQueryStatsClass->addKey("error",   sERRORPretty);
        data.fQueryStatsClass->addKey("warning", sWARNINGPretty);
        data.fQueryStatsClass->addKey("info",    sINFOPretty);
        data.fQueryStatsClass->addKey("trace",   sTRACEPretty);
        data.fQueryStatsClass->addKey("trace2",  sTRACE2Pretty);
        data.fQueryStatsClass->addKey("trace3",  sTRACE3Pretty);
        data.fQueryStatsClass->addKey("debug",   sDEBUGPretty);
        data.fQueryStatsClass->addKey("debug2",  sDEBUG2Pretty);
        data.fQueryStatsClass->addKey("debug3",  sDEBUG3Pretty);
        data.fQueryStatsClass->addKey("start",   sSTARTPretty);
        data.fQueryStatsClass->addKey("stop",    sSTOPPretty);
        data.fQueryStatsClass->addKey("pass",    sPASSPretty);
        data.fQueryStatsClass->addKey("fail",    sFAILPretty);
        data.fQueryStatsClass->addKey("status",  sSTATUSPretty);
        data.fQueryStatsClass->addKey("user1",   sUSER1Pretty);
        data.fQueryStatsClass->addKey("user2",   sUSER2Pretty);
        data.fQueryStatsClass->addKey("user3",   sUSER3Pretty);
        data.fQueryStatsClass->addKey("user4",   sUSER4Pretty);
        data.fQueryStatsClass->addKey("user5",   sUSER5Pretty);
        data.fQueryStatsClass->addKey("user6",   sUSER6Pretty);
        data.fQueryStatsClass->addKey("user7",   sUSER7Pretty);
        data.fQueryStatsClass->addKey("user8",   sUSER8Pretty);

        // Construct the map class for the marshalled PURGE request output

        data.fPurgeStatsClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/PurgeStats");

        data.fPurgeStatsClass->addKey("purgedRecords", "Purged Records");
        data.fPurgeStatsClass->addKey("totalRecords",  "Total Records");
        
        // Construct the map class for the marshalled LIST SETTINGS output
        // for local logging

        data.fListLocalSettingsClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/LocalSettings");

        data.fListLocalSettingsClass->addKey("loggingMode", "Logging Mode");
        data.fListLocalSettingsClass->addKey("directory",   "Directory");
        data.fListLocalSettingsClass->addKey("maxRecordSize",
                                             "Max Record Size");
        data.fListLocalSettingsClass->addKey("defaultMaxQueryRecords",
                                             "Default Max Query Records");
        data.fListLocalSettingsClass->addKey("resolveMessage",
                                             "Resolve Message");
        data.fListLocalSettingsClass->addKey("resolveMessageVar",
                                             "Resolve Message Var");

        // Construct the map class for the marshalled LIST SETTINGS output
        // for remote logging

        data.fListRemoteSettingsClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/RemoteSettings");

        data.fListRemoteSettingsClass->addKey("loggingMode", "Logging Mode");
        data.fListRemoteSettingsClass->addKey("remoteLogServer",
                                              "Remote Log Server");
        data.fListRemoteSettingsClass->addKey("remoteLogService",
                                              "Remote Log Service");

        // Construct map class for marshalled LIST logs/machines/handles output

        data.fListLogsClass = STAFMapClassDefinition::create(
            "STAF/Service/Log/ListLogs");

        data.fListLogsClass->addKey("logName",   "Log Name");
        data.fListLogsClass->addKey("timestamp", "Date-Time");
        data.fListLogsClass->addKey("upperSize", "U-Size");
        data.fListLogsClass->addKey("size",      "L-Size");
        
        // Set service handle

        *pServiceHandle = new LogServiceData(data);

        return kSTAFOk;
    }
    catch (STAFException &e)
    { *pErrorBuffer = getExceptionString(e,
                      "STAFLogService.cpp: STAFServiceConstruct").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFLogService.cpp: STAFServiceConstruct: "
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

        LogServiceData *pData =
            reinterpret_cast<LogServiceData *>(serviceHandle);
        
        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);

        retCode = STAFHandle::create(pData->fName, pData->fHandle);
        
        if (retCode != kSTAFOk)
            return retCode;

        STAFCommandParseResultPtr parsedResult =
            pData->fParmsParser->parse(pInfo->parms);

        if (parsedResult->rc != kSTAFOk)
        {
            *pErrorBuffer = parsedResult->errorBuffer.adoptImpl();
            return parsedResult->rc;
        }

        if (parsedResult->optionTimes(sENABLEREMOTELOGGING) != 0)
        {
            pData->fRLogMode = 1;

            STAFResultPtr serverResult = resolveOpLocal(pData, parsedResult,
                                                        sREMOTELOGSERVER);
            if (serverResult->rc != kSTAFOk)
            {
                *pErrorBuffer = serverResult->result.adoptImpl();
                return serverResult->rc;
            }

            pData->fRemoteLogServer = serverResult->result;

            STAFResultPtr serviceResult = resolveOpLocal(pData, parsedResult,
                                                         sREMOTELOGSERVICE);
            if (serviceResult->rc != kSTAFOk)
            {
                *pErrorBuffer = serviceResult->result.adoptImpl();
                return serviceResult->rc;
            }

            if (serviceResult->result.length() != 0)
                pData->fRemoteLogService = serviceResult->result;
        }

        STAFResultPtr dirResult = resolveOpLocal(pData, parsedResult,
                                                 sDIRECTORY);
        if (dirResult->rc != kSTAFOk)
        {
            *pErrorBuffer = dirResult->result.adoptImpl();
            return dirResult->rc;
        }

        STAFFSPath logPath;

        if (dirResult->result.length() == 0)
        {
            logPath.setRoot(pInfo->writeLocation);
            logPath.addDir("service");
            logPath.addDir(pData->fShortName.toLowerCase());
            pData->fRoot = logPath.asString();
        }
        else
        {
            pData->fRoot = dirResult->result;
            logPath.setRoot(pData->fRoot);
        }

        // Create the log data directory if it doesn't already exist to
        // verify the log data directory name.

        if (!logPath.exists())
        {
            try
            {
                STAFFSEntryPtr logdir = 
                    logPath.createDirectory(kSTAFFSCreatePath);
            }
            catch (...)
            { 
                STAFString error("STAFLogService.cpp: STAFServiceInit: "
                                 "Invalid Log Directory: " + pData->fRoot);
                cout << error << endl;
                *pErrorBuffer = error.adoptImpl();
                return kSTAFServiceConfigurationError;
            }
        }
        
        if (parsedResult->optionTimes(sMAXRECORDSIZE) != 0)
        {
            STAFResultPtr maxResult = resolveOpLocal(pData, parsedResult,
                                                     sMAXRECORDSIZE);

            if (maxResult->rc != kSTAFOk)
            {
                *pErrorBuffer = maxResult->result.adoptImpl();
                return maxResult->rc;
            }

            // Convert resolved option string to an unsigned integer in range
            // 0 to UINT_MAX

            maxResult = convertOptionStringToUInt(
                maxResult->result, sMAXRECORDSIZE, pData->fMaxRecordSize);

            if (maxResult->rc != kSTAFOk)
            {
                *pErrorBuffer = maxResult->result.adoptImpl();
                return maxResult->rc;
            }
        }

        if (parsedResult->optionTimes(sDEFAULTMAXQUERYRECORDS) != 0)
        {
            STAFResultPtr maxResult = resolveOpLocal(
                pData, parsedResult, sDEFAULTMAXQUERYRECORDS);

            if (maxResult->rc != kSTAFOk)
            {
                *pErrorBuffer = maxResult->result.adoptImpl();
                return maxResult->rc;
            }
            
            // Convert resolved option string to an unsigned integer in range
            // 0 to UINT_MAX

            maxResult = convertOptionStringToUInt(
                maxResult->result, sDEFAULTMAXQUERYRECORDS,
                pData->fDefaultMaxQueryRecords);

            if (maxResult->rc != kSTAFOk)
            {
                *pErrorBuffer = maxResult->result.adoptImpl();
                return maxResult->rc;
            }
        }

        if (parsedResult->optionTimes(sRESOLVEMESSAGE) != 0)
            pData->fDefaultResolveMessage = 1;
        else if (parsedResult->optionTimes(sNORESOLVEMESSAGE) != 0)
            pData->fDefaultResolveMessage = 0;

        if (parsedResult->optionTimes(sENABLERESOLVEMESSAGEVAR) != 0)
            pData->fUseResolveMessageVar = 1;
        else if (parsedResult->optionTimes(sDISABLERESOLVEMESSAGEVAR) != 0)
            pData->fUseResolveMessageVar = 0;

        // Get the default authenticator

        STAFResultPtr resResult = pData->fHandle->submit(
            sLocal, sVar, sDefAuthString);

        if (resResult->rc != kSTAFOk)
        {
            *pErrorBuffer = resResult->result.adoptImpl();
            return resResult->rc;
        }

        pData->fDefaultAuthenticator = resResult->result;
        
        // Get the machine name for the local machine

        resResult = pData->fHandle->submit(sLocal, sVar, sResMachineString);

        if (resResult->rc != kSTAFOk)
        {
            *pErrorBuffer = resResult->result.adoptImpl();
            return resResult->rc;
        }

        pData->fLocalMachineName = resResult->result;
         
        // Get the line separator

        resResult = pData->fHandle->submit(sLocal, sVar, sEOLString);

        if (resResult->rc != kSTAFOk)
        {
            *pErrorBuffer = resResult->result.adoptImpl();
            return resResult->rc;
        }

        sLineSep = resResult->result;

        // Assign the help text string for the service

        sHelpMsg = STAFString("*** ") + pData->fShortName + " Service Help ***" +
            sLineSep + sLineSep +
            "LOG    <GLOBAL | MACHINE | HANDLE> LOGNAME <Logname> LEVEL <Level>" +
            sLineSep +
            "       MESSAGE <Message> [RESOLVEMESSAGE | NORESOLVEMESSAGE]" +
            sLineSep + sLineSep +
            "QUERY  <GLOBAL | MACHINE <Machine> [HANDLE <Handle>]> LOGNAME <Logname>" +
            sLineSep +
            "       [LEVELMASK <Mask>] [QMACHINE <Machine>]... " +
            "[QHANDLE <Handle>]..." +
            sLineSep +
            "       [NAME <Name>]... [USER <User>]... [ENDPOINT <Endpoint>]..." +
            sLineSep +
            "       [CONTAINS <String>]... [CSCONTAINS <String>]..." +
            sLineSep +
            "       [STARTSWITH <String>]... [CSSTARTSWITH <String>]..." +
            sLineSep +
            "       [FROM <Timestamp> | AFTER <Timestamp>]" +
            sLineSep +
            "       [BEFORE <Timestamp> | TO <Timestamp>]" +
            sLineSep +
            "       [FROMRECORD <Num>] [TORECORD <Num>]" +
            sLineSep +
            "       [FIRST <Num> | LAST <Num> | ALL] [TOTAL | STATS | LONG]" +
            sLineSep +
            "       [LEVELBITSTRING]" +
            sLineSep + sLineSep +
            "LIST   GLOBAL | MACHINES | MACHINE <Machine> [HANDLES | HANDLE <Handle>] |" +
            sLineSep +
            "       SETTINGS" +
            sLineSep + sLineSep +
            "DELETE <GLOBAL | MACHINE <Machine> [HANDLE <Handle>]>" +
            sLineSep +
            "       LOGNAME <Logname> CONFIRM" +
            sLineSep + sLineSep +
            "PURGE  <GLOBAL | MACHINE <Machine> [HANDLE <Handle>]> LOGNAME <Logname>" +
            sLineSep +
            "       CONFIRM | CONFIRMALL" +
            sLineSep +
            "       [LEVELMASK <Mask>] [QMACHINE <Machine>]... [QHANDLE <Handle>]..." +
            sLineSep +
            "       [NAME <Name>]... [USER <User>]... [ENDPOINT <Endpoint>]..." +
            sLineSep +
            "       [CONTAINS <String>]... [CSCONTAINS <String>]..." +
            sLineSep +
            "       [STARTSWITH <String>]... [CSSTARTSWITH <String>]..." +
            sLineSep +
            "       [FROM <Timestamp> | AFTER <Timestamp>]" +
            sLineSep +
            "       [BEFORE <Timestamp> | TO <Timestamp>]" +
            sLineSep +
            "       [FROMRECORD <Num>] [TORECORD <Num>]" +
            sLineSep +
            "       [FIRST <Num> | LAST <Num>]" +
            sLineSep + sLineSep +
            "SET    [MAXRECORDSIZE <Size>] [DEFAULTMAXQUERYRECORDS <Number>]" +
            sLineSep +
            "       [ENABLERESOLVEMESSAGEVAR | DISABLERESOLVEMESSAGEVAR]" +
            sLineSep +
            "       [RESOLVEMESSAGE | NORESOLVEMESSAGE]" +
            sLineSep + sLineSep +
            "VERSION" +
            sLineSep + sLineSep +
            "HELP";

        // Register help for the error codes for this service

        registerHelpData(pData, kSTAFLogInvalidLevel,
            STAFString("Invalid level"),
            STAFString("An invalid logging level was specified.  "
                       "See the STAF User's Guide for a complete list of "
                       "logging levels."));

        registerHelpData(pData, kSTAFLogInvalidFileFormat,
            STAFString("Invalid file format"),
            STAFString("An invalid/unknown record format was encountered "
                       "while reading the log file"));

        registerHelpData(pData, kSTAFLogPurgeFailure,
            STAFString("Unable to purge all log records"),
            STAFString("Your purge criteria selected every record in the log "
                       "file.  Use CONFIRMALL instead of CONFIRM if you "
                       "really want to delete every record (or submit a "
                       "DELETE request).  Or, modify your purge criteria."));

        registerHelpData(pData, kSTAFLogExceededDefaultMaxRecords,
            STAFString("Exceeded default maximum query records"),
            STAFString("Your query criteria selected more records than "
                       "allowed by the DefaultMaxQueryRecords setting.  "
                       "Use the FIRST <Num> or LAST <Num> option to specify "
                       "the number of records or the ALL option if you "
                       "really want all of the records."));
    }
    catch (STAFException &e)
    {
        *pErrorBuffer = getExceptionString(e,
                        "STAFLogService.cpp: STAFServiceInit").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFLogService.cpp: STAFServiceInit: "
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
        STAFResultPtr result(new STAFResult(), STAFResultPtr::INIT);

        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

        LogServiceData *pData =
            reinterpret_cast<LogServiceData *>(serviceHandle);

        STAFString request(pInfo->request);
        STAFString action = request.subWord(0, 1).toUpperCase();

        if (pData->fRLogMode != 0)
        {
            if (action == sLOG) result = handleRemoteLog(pInfo, pData);
            else if (action == sVERSION)
            {
                result = STAFResultPtr(new STAFResult(kSTAFOk, sVersionInfo),
                                       STAFResultPtr::INIT);
            }
            else if ((action == sLIST) &&
                     (request.subWord(1, 2).toUpperCase() == sSETTINGS))
            {
                result = handleList(pInfo, pData);
            }
            else result = handleRemoteLogGeneral(pInfo, pData);
        }
        else if (action == sLOG)    result = handleLog(pInfo, pData);
        else if (action == sQUERY)  result = handleQuery(pInfo, pData);
        else if (action == sLIST)   result = handleList(pInfo, pData);
        else if (action == sDELETE) result = handleDelete(pInfo, pData);
        else if (action == sPURGE)  result = handlePurge(pInfo, pData);
        else if (action == sSET)    result = handleSet(pInfo, pData);
        else if (action == sHELP)   result = handleHelp(pInfo, pData);
        else if (action == sVERSION)
        {
            result = STAFResultPtr(new STAFResult(kSTAFOk, sVersionInfo),
                                   STAFResultPtr::INIT);
        }
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
            e, "STAFLogService.cpp: STAFServiceAcceptRequest").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFLogService.cpp: STAFServiceAcceptRequest: "
                         "Caught unknown exception");
        *pResultBuffer = error.adoptImpl();
    }

    return retCode;
}


STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle, void *pTermInfo,
                         unsigned int termLevel, STAFString_t *pErrorBuffer)
{
    if (termLevel != 0) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    try
    {
        retCode = kSTAFOk;

        LogServiceData *pData = 
            reinterpret_cast<LogServiceData *>(serviceHandle);

        // Un-register Help Data

        unregisterHelpData(pData, kSTAFLogInvalidLevel);
        unregisterHelpData(pData, kSTAFLogInvalidFileFormat);
        unregisterHelpData(pData, kSTAFLogPurgeFailure);
    }
    catch (STAFException &e)
    {
        *pErrorBuffer = getExceptionString(e,
                        "STAFLogService.cpp: STAFServiceTerm").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFLogService.cpp: STAFServiceTerm: "
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
        LogServiceData *pData =
            reinterpret_cast<LogServiceData *>(*serviceHandle);
        delete pData;
        *serviceHandle = 0;

        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    {
        *pErrorBuffer = getExceptionString(e,
                        "STAFLogService.cpp: STAFServiceDestruct").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFLogService.cpp: STAFServiceDestruct: "
                         "Caught unknown exception");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


STAFResultPtr handleLog(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData)
{
    // XXX: This routine in particular has serious issues when we add the
    //      ability to resolve remote handle variables.  It relies on functions
    //      which assume that the requestor is in the pInfo structure, which it
    //      may not be if we are a remote log server.

    INIT_TIMES();
    RECORD_TIME("Parsing");

    STAFCommandParseResultPtr parsedResult =
        pData->fLogParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    RECORD_TIME("Check trust");

    // Verify that the requesting machine/user has at least trust level 3

    if (parsedResult->optionTimes(sRMTMACHINE) == 0)
    {
        VALIDATE_TRUST(3, pData->fShortName, "LOG", pData->fLocalMachineName);
    }
    else
    {
        // Verify that the remote log machine/user has at least trust level 3

        STAFString trustRequest(
            sGetMachine + parsedResult->optionValue(sRMTMACHINE) +
            sGetUser + parsedResult->optionValue(sRMTUSER));

        STAFResultPtr trustResult = pData->fHandle->submit(
            sLocal, sTrust, trustRequest);

        if (trustResult->rc != kSTAFOk)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 3 failed.  STAF local TRUST ") + trustRequest +
                " failed with RC: " + trustResult->rc +
                ", Result: " + trustResult->result;

            return STAFResultPtr(
                new STAFResult(trustResult->rc, errorMsg),
                STAFResultPtr::INIT);
        }

        try
        {
            unsigned int trustLevel = trustResult->result.asUInt();
            
            VALIDATE_TRUST3(3, pData->fShortName, "LOG",
                            pData->fLocalMachineName, trustLevel,
                            parsedResult->optionValue(sRMTMACHINE), "",
                            parsedResult->optionValue(sRMTUSER));
        }
        catch (STAFException)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 3 failed.  STAF local TRUST ") + trustRequest +
                " did not return a numeric trust level, Result: " +
                trustResult->result;

            return STAFResultPtr(
                new STAFResult(kSTAFAccessDenied, errorMsg),
                STAFResultPtr::INIT);
        }
    }
    
    // Get log level and log mask and see if we should log

    RECORD_TIME("Check log level");

    STAFResultPtr levelResult = resolveOp(pInfo, pData, parsedResult, sLEVEL);

    if (levelResult->rc != kSTAFOk) return levelResult;

    unsigned int logLevel = 0;

    if (!isValidLogLevel(levelResult->result, logLevel))
    {
        return STAFResultPtr(new STAFResult(kSTAFLogInvalidLevel,
                                            levelResult->result),
                             STAFResultPtr::INIT);
    }

    STAFResultPtr logmaskResult = resolveStr(pInfo, pData,
                                             pData->fResolveLogMaskString);
    if ((logmaskResult->rc != kSTAFOk) &&
        (logmaskResult->rc != kSTAFVariableDoesNotExist))
    {
        return logmaskResult;
    }

    if (logmaskResult->rc == kSTAFVariableDoesNotExist)
        logmaskResult->result = sEverythingLogMask;

    unsigned int logMask = 0;

    if (!convertLogMaskToUInt(logmaskResult->result, logMask))
    {
        return STAFResultPtr(new STAFResult(kSTAFLogInvalidLevel,
                                            logmaskResult->result),
                             STAFResultPtr::INIT);
    }

    if (!(logLevel & logMask))
        return STAFResultPtr(new STAFResult, STAFResultPtr::INIT);

    // Get logname and message

    RECORD_TIME("Get logname and message");

    STAFResultPtr lognameResult = resolveOp(pInfo, pData, parsedResult,
                                            sLOGNAME);

    if (lognameResult->rc != kSTAFOk) return lognameResult;

    // if the user specifies to resolve messages or
    // STAF/Service/Log/ResolveMessage is enabled and set or
    // the default is to resolve messages and the user didn't say not to
    // then resolve the message

    STAFString message = parsedResult->optionValue(sMESSAGE);
    unsigned int resolveMessage = pData->fDefaultResolveMessage;

    if (parsedResult->optionTimes(sRESOLVEMESSAGE) != 0)
    {
        resolveMessage = 1;
    }
    else if (parsedResult->optionTimes(sNORESOLVEMESSAGE) != 0)
    {
        resolveMessage = 0;
    }
    else if (pData->fUseResolveMessageVar != 0)
    {
        STAFResultPtr resolveResult = resolveStr(pInfo, pData,
                                                 pData->fResolveMessageString);
        if (resolveResult->rc == kSTAFOk)
        {
            try
            {
                if (resolveResult->result.asUInt() != 0)
                    resolveMessage = 1;
                else
                    resolveMessage = 0;
            }
            catch (STAFException)
            {
                STAFString errorMsg = STAFString("The resolved value for ") +
                    pData->fResolveMessageString + " must be 0 or 1.  "
                    "Invalid value: " + resolveResult->result;

                return STAFResultPtr(
                    new STAFResult(kSTAFInvalidValue, errorMsg),
                    STAFResultPtr::INIT);
            }
        }
        else if (resolveResult->rc != kSTAFVariableDoesNotExist)
        {
            return resolveResult;
        }
    }

    if (resolveMessage != 0)
    {
        STAFResultPtr messageResult = resolveOp(
            pInfo, pData, parsedResult, sMESSAGE);

        if (messageResult->rc != kSTAFOk) return messageResult;

        message = messageResult->result;
    }

    if (message.length(STAFString::kChar) > pData->fMaxRecordSize)
        message = message.subString(0, pData->fMaxRecordSize, STAFString::kChar);

    // Check for remote logging info

    RECORD_TIME("Check for remote");

    bool isRemote = false;
    STAFString remoteMachine;
    STAFString remoteHandleName;
    STAFString remoteHandle;
    STAFString remoteUser;
    STAFString remoteLogicalMachine;
    STAFString remoteNickname;

    if (parsedResult->optionTimes(sRMTMACHINE) != 0)
    {
        isRemote = true;
        remoteMachine = parsedResult->optionValue(sRMTMACHINE);
        remoteHandleName = parsedResult->optionValue(sRMTNAME);
        remoteHandle = parsedResult->optionValue(sRMTHANDLE);
        remoteUser = parsedResult->optionValue(sRMTUSER);
        remoteLogicalMachine = parsedResult->optionValue(sRMTMACH);
        remoteNickname = parsedResult->optionValue(sRMTNICKNAME);
    }

    // Setup/Create the path

    RECORD_TIME("Create path");

    STAFFSPath logfilePath;

    logfilePath.setRoot(pData->fRoot);

    if (parsedResult->optionTimes(sGLOBAL) != 0)
    {
        logfilePath.addDir(sGLOBAL);
    }
    else
    {
        logfilePath.addDir(sMACHINE);

        if (isRemote)
            logfilePath.addDir(remoteNickname);
        else
            logfilePath.addDir(pInfo->machineNickname);

        if (parsedResult->optionTimes(sMACHINE) != 0)
        {
            logfilePath.addDir(sGLOBAL);
        }
        else
        {
            logfilePath.addDir(sHANDLE);

            if (isRemote)
                logfilePath.addDir(remoteHandle);
            else
                logfilePath.addDir(pInfo->handle);
        }
    }

    if (!logfilePath.exists())
    {
        try
        {
            // XXX: Don't want exceptions here
            STAFFSEntryPtr logdir =
                           logfilePath.createDirectory(kSTAFFSCreatePath);
        }
        catch (...)
        { /* Do Nothing */ }
    }

    logfilePath.setName(lognameResult->result);
    logfilePath.setExtension(sLogExt);

    // Must get lock before opening file to avoid log corruption problem
    // with multiple simultaneous read/writes to same log.

    // Get a write lock on the global Read/Write Semaphore

    RECORD_TIME("Get and lock entry");

    STAFLogFileLocksPtr logLocks =
        STAFLogFileLocks::acquireLocks(logfilePath.asString());
    STAFRWSemRLock accessLock(*logLocks->logAccess);
    STAFMutexSemLock recordLock(*logLocks->recordAccess);

    // Open the log file

    RECORD_TIME("Open log file");

    fstream logfile(logfilePath.asString().toCurrentCodePage()->buffer(),
                    ios::out | ios::app | STAF_ios_binary);

    if (!logfile)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileOpenError,
                                            logfilePath.asString()),
                             STAFResultPtr::INIT);
    }
    
    // Write the log file data

    RECORD_TIME("Write log record");

    STAFTimestamp currTimestamp;

    LogRecord logRecord(currTimestamp.asDateString().asUInt(),
                        currTimestamp.asSecondsPastMidnight(), logLevel,
                        pInfo->machine, pInfo->handleName, pInfo->handle,
                        pInfo->user, pInfo->endpoint, message);

    if (isRemote)
    {
        logRecord.machine = remoteLogicalMachine;
        logRecord.handle = remoteHandle.asUIntWithDefault(0);
        logRecord.handleName = remoteHandleName;
        logRecord.user = remoteUser;
        logRecord.endpoint = remoteMachine;
    }

    writeLogRecordToFile(logfile, logRecord);

    RECORD_TIME("Log complete (except for file close)");
    OUTPUT_TIMES();

    return STAFResultPtr(new STAFResult, STAFResultPtr::INIT);
}


STAFResultPtr handleQuery(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData)
{
    STAFCommandParseResultPtr parsedResult = 
        pData->fQueryParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    if (parsedResult->optionTimes(sRMTMACHINE) == 0)
    {
        // Verify that the requesting machine/user has at least trust level 2

        VALIDATE_TRUST(2, pData->fShortName, "QUERY", pData->fLocalMachineName);
    }
    else
    {
        // Verify that the remote log machine/user has at least trust level 2

        STAFString trustRequest(
            sGetMachine + parsedResult->optionValue(sRMTMACHINE) +
            sGetUser + parsedResult->optionValue(sRMTUSER));

        STAFResultPtr trustResult = pData->fHandle->submit(
            sLocal, sTrust, trustRequest);

        if (trustResult->rc != kSTAFOk)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 2 failed.  STAF local TRUST ") + trustRequest +
                " failed with RC: " + trustResult->rc +
                ", Result: " + trustResult->result;

            return STAFResultPtr(
                new STAFResult(trustResult->rc, errorMsg),
                STAFResultPtr::INIT);
        }

        try
        {
            unsigned int trustLevel = trustResult->result.asUInt();
            
            VALIDATE_TRUST3(2, pData->fShortName, "QUERY",
                            pData->fLocalMachineName, trustLevel,
                            parsedResult->optionValue(sRMTMACHINE), "",
                            parsedResult->optionValue(sRMTUSER));
        }
        catch (STAFException)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 2 failed.  STAF local TRUST ") + trustRequest +
                " did not return a numeric trust level, Result: " +
                trustResult->result;

            return STAFResultPtr(
                new STAFResult(kSTAFAccessDenied, errorMsg),
                STAFResultPtr::INIT);
        }
    }
    
    // Build up the log filter criteria

    STAFResultPtr errorResult;
    LogRecordFilter logFilter;
    LogStats logStats = { 0 };

    if (!updateQueryPurgeLogFilter(logFilter, errorResult, pInfo, pData,
                                   parsedResult))
    {
        return errorResult;
    }

    // See if they specified LEVELBITSTRING or STATS or TOTAL

    bool levelAsBits = (parsedResult->optionTimes(sLEVELBITSTRING) > 0) ?
                       true : false;
    bool onlyStats = (parsedResult->optionTimes(sSTATS) > 0) ? true : false;
    bool onlyTotal = (parsedResult->optionTimes(sTOTAL) > 0) ? true : false;
    bool longFormat = (parsedResult->optionTimes(sLONG) > 0) ? true : false;

    unsigned int first = 0;
    unsigned int last = 0;
    bool all = false;

    // Check if they specified FIRST or LAST or ALL

    if (parsedResult->optionTimes(sFIRST))
    {
        STAFResultPtr result = resolveOp(
            pInfo, pData, parsedResult, sFIRST);

        if (result->rc != kSTAFOk) return result;
        
        // Convert resolved option string to an unsigned integer in range 0
        // to UINT_MAX

        result = convertOptionStringToUInt(result->result, sFIRST, first);

        if (result->rc != kSTAFOk) return result;
    }
    else if (parsedResult->optionTimes(sLAST))
    {
        STAFResultPtr result = resolveOp(
            pInfo, pData, parsedResult, sLAST);

        if (result->rc != kSTAFOk) return result;

        // Convert resolved option string to an unsigned integer in range 0
        // to UINT_MAX

        result = convertOptionStringToUInt(result->result, sLAST, last);

        if (result->rc != kSTAFOk) return result;
    }
    else if (parsedResult->optionTimes(sALL))
    {
        all = true;
    }

    bool useFromRecord = false;
    unsigned int fromRecord = 0;
    bool useToRecord = false;
    unsigned int toRecord = 0;

    if (parsedResult->optionTimes(sFROMRECORD) != 0)
    {
        STAFResultPtr result = resolveOp(
            pInfo, pData, parsedResult, sFROMRECORD);

        if (result->rc != kSTAFOk) return result;

        // Convert resolved option string to an unsigned integer in range 1
        // to UINT_MAX

        result = convertOptionStringToUInt(
            result->result, sFROMRECORD, fromRecord, 1);

        if (result->rc != kSTAFOk) return result;

        useFromRecord = true;
    }

    if (parsedResult->optionTimes(sTORECORD) != 0)
    {
        STAFResultPtr result = resolveOp(
            pInfo, pData, parsedResult, sTORECORD);

        if (result->rc != kSTAFOk) return result;
        
        // Convert resolved option string to an unsigned integer in range 1
        // to UINT_MAX

        result = convertOptionStringToUInt(
            result->result, sTORECORD, toRecord, 1);

        if (result->rc != kSTAFOk) return result;

        useToRecord = true;
    }

    if (useToRecord && useFromRecord && (fromRecord > toRecord))
    {
        STAFString errorMsg = STAFString("FROMRECORD value must be <= ") +
            "TORECORD value.  FROMRECORD: " + STAFString(fromRecord) +
            ", TORECORD: " + STAFString(toRecord);
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue, errorMsg),
                             STAFResultPtr::INIT);

    }

    // Get the log file path

    STAFFSPath logfilePath;

    if (!generateQueryPurgeDeleteLogFilePath(logfilePath, errorResult,
                                             pInfo, pData, parsedResult))
    {
        return errorResult;
    }

    // Open the log file

    if (!logfilePath.exists())
    {
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist,
                                            logfilePath.name()),
                             STAFResultPtr::INIT);
    }

    fstream logfile(logfilePath.asString().toCurrentCodePage()->buffer(),
                    ios::in | STAF_ios_binary);

    if (!logfile)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileOpenError,
                                            logfilePath.asString()),
                             STAFResultPtr::INIT);
    }
    
    // Determine if should return an error and limit number of records if the
    // number of records matching the query criteria exceeds the default
    // maximum query records.

    bool limitRecords = false;

    if ((pData->fDefaultMaxQueryRecords != 0) && (!all) &&
         (first == 0) && (last == 0) && (!onlyTotal) && (!onlyStats))
    {
        limitRecords = true;
        last = pData->fDefaultMaxQueryRecords;
    }

    STAFRC_t rc = kSTAFOk;

    // Create the marshalling context
    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    // Create an empty list object for the marshalled list of log records
    STAFObjectPtr logList = STAFObject::createList();

    // Set the log record map class definition pointer

    STAFMapClassDefinitionPtr mapClassPtr = pData->fLogRecordClass->
        reference();

    if (longFormat)
        mapClassPtr = pData->fLogRecordLongClass->reference();
    
    // Get a read lock on the global Read/Write Semaphore

    STAFLogFileLocksPtr logLocks =
        STAFLogFileLocks::acquireLocks(logfilePath.asString());
    STAFRWSemRLock accessLock(*logLocks->logAccess);

    // Read each record

    const int RECORDS_PER_LOCK = 100;
    unsigned int currFirstRecords = 0;
    std::list<LogRecord> lastList;
    STAFString result = STAFString();
    unsigned int totalRecords = 0;
    LogRecord logRecord;
    
    // Record number counter (used to compare with fromRecord and toRecord)
    // and to know which record number is currently being processed.
    unsigned int recordNum = 0;

    // Flag to use to indicate if don't need to process any more records
    bool done = false;

    while (!logfile.eof() && !done)
    {
        STAFMutexSemLock recordLock(*logLocks->recordAccess);

        for (int recordCount = 0;
             (recordCount < RECORDS_PER_LOCK) && !logfile.eof();
             ++recordCount)
        {
            recordNum++;

            // First, get the information from the log record

            unsigned int status = readLogRecordFromFile(
                logfile, logRecord, recordNum);

            if (status == kReadLogEndOfFile)
            {
                // Finish up with whatever needs to be done
                continue;
            }
            else if (status == kReadLogInvalidFormat)
            {
                // Clean/Finish up what has been done

                return STAFResultPtr(new STAFResult(kSTAFLogInvalidFileFormat,
                                     STAFString(logRecord.recordFormatID)),
                                     STAFResultPtr::INIT);
            }

            // Now, make sure this record meets the selection criteria

            if (useFromRecord)
            {
                if (recordNum < fromRecord)
                {
                    // Ignore this log record
                    continue;
                }
            }

            if (useToRecord)
            {
                if (recordNum > toRecord)
                {
                    // Don't need to process any more log records
                    done = true;
                    break;
                }
            }

            // XXX: Might want to make a change to stop once we reach the
            //      BEFORE / TO date

            if (!logRecordMatchesFilter(logRecord, logFilter,
                                        pData->fDefaultAuthenticator))
            {
                continue;
            }

            // Now, do whatever we are supposed to with the record

            /* DEBUG: printLogRecord(logRecord); */

            if (last > 0)
            {
                // Make sure we only keep 'last' number of records
                if (lastList.size() == last)
                {
                    lastList.pop_front();

                    // ???-CVR: This check seems odd.  The user seems to have
                    //          explicitly told us how many records they want,
                    //          but if they've also told us some maximum
                    //          number of records, then we return an error.
                    //          Seems like we need to see if they've actually
                    //          exceeded the limit before returning this error.

                    if (limitRecords) rc = kSTAFLogExceededDefaultMaxRecords;
                }

                lastList.push_back(logRecord);
            }
            else
            {
                if (onlyTotal)
                {
                    ++totalRecords;
                }
                else if (onlyStats)
                {
                    updateLogStats(logStats, logRecord.logLevel);
                }
                else
                {
                    addLogRecordToList(logList, mapClassPtr, logRecord,
                                       levelAsBits, longFormat);
                }

                if ((first > 0) && (++currFirstRecords >= first))
                {
                    // Don't need to process any more log records
                    done = true;
                    break;
                }
            }
        }
    }

    // If we were doing the "last" set of things, we now need to generate
    // the output or update the stats

    if (last > 0)
    {
        for (std::list<LogRecord>::iterator iter = lastList.begin();
             iter != lastList.end(); ++iter)
        {
            if (onlyTotal) ++totalRecords;
            else if (onlyStats)
            {
                updateLogStats(logStats, (*iter).logLevel);
            }
            else
            {
                addLogRecordToList(logList, mapClassPtr, (*iter),
                                   levelAsBits, longFormat);
            }
        }
    }

    if (onlyTotal)
    {
        result = STAFString(totalRecords);
    }
    else if (onlyStats)
    {
        // Create a map object for the marshalled map of log stats

        STAFObjectPtr queryStatsMap =
            pData->fQueryStatsClass->createInstance();

        addLogStatsToMap(queryStatsMap, logStats);
        
        // Set the marshalling context map class definitions and root object

        mc->setMapClassDefinition(pData->fQueryStatsClass->reference());
        mc->setRootObject(queryStatsMap);

        result = mc->marshall();
    }
    else
    {
        // Set the marshalling context map class definitions and root object

        mc->setMapClassDefinition(mapClassPtr->reference());
        mc->setRootObject(logList);

        result = mc->marshall();
    }

    return STAFResultPtr(new STAFResult(rc, result), STAFResultPtr::INIT);
}


STAFResultPtr handleList(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData)
{
    STAFCommandParseResultPtr parsedResult =
        pData->fListParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // Verify that the requesting machine/user has at least trust level 2

    if (parsedResult->optionTimes(sRMTMACHINE) == 0)
    {
        VALIDATE_TRUST(2, pData->fShortName, "LIST", pData->fLocalMachineName);
    }
    else
    {
        // Verify that the remote log machine/user has at least trust level 2

        STAFString trustRequest(
            sGetMachine + parsedResult->optionValue(sRMTMACHINE) +
            sGetUser + parsedResult->optionValue(sRMTUSER));

        STAFResultPtr trustResult = pData->fHandle->submit(
            sLocal, sTrust, trustRequest);

        if (trustResult->rc != kSTAFOk)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 2 failed.  STAF local TRUST ") + trustRequest +
                " failed with RC: " + trustResult->rc +
                ", Result: " + trustResult->result;

            return STAFResultPtr(
                new STAFResult(trustResult->rc, errorMsg),
                STAFResultPtr::INIT);
        }

        try
        {
            unsigned int trustLevel = trustResult->result.asUInt();
            
            VALIDATE_TRUST3(2, pData->fShortName, "LIST",
                            pData->fLocalMachineName, trustLevel,
                            parsedResult->optionValue(sRMTMACHINE), "",
                            parsedResult->optionValue(sRMTUSER));
        }
        catch (STAFException)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 2 failed.  STAF local TRUST ") + trustRequest +
                " did not return a numeric trust level, Result: " +
                trustResult->result;

            return STAFResultPtr(
                new STAFResult(kSTAFAccessDenied, errorMsg),
                STAFResultPtr::INIT);
        }
    }
    
    // Create the marshalling context

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    // Generate output based on the LIST options specified

    if (parsedResult->optionTimes(sSETTINGS) != 0)
    {
        STAFString result;

        if (pData->fRLogMode != 0)
        {
            // Create a map object for the remote log settings

            STAFObjectPtr listRemoteSettingsMap =
                pData->fListRemoteSettingsClass->createInstance();

            listRemoteSettingsMap->put("loggingMode", "Remote");
            listRemoteSettingsMap->put("remoteLogServer",
                                       pData->fRemoteLogServer);
            listRemoteSettingsMap->put("remoteLogService",
                                       pData->fRemoteLogService);

            // Set marshalling context map class definitions and root object

            mc->setMapClassDefinition(
                pData->fListRemoteSettingsClass->reference());
            mc->setRootObject(listRemoteSettingsMap);

            result = mc->marshall();
        }
        else
        {
            // Create a map object for the local log settings

            STAFObjectPtr listLocalSettingsMap = 
                pData->fListLocalSettingsClass->createInstance();

            listLocalSettingsMap->put("loggingMode", "Local");
            listLocalSettingsMap->put("directory", pData->fRoot);
            listLocalSettingsMap->put(
                "maxRecordSize", STAFString(pData->fMaxRecordSize));
            listLocalSettingsMap->put(
                "defaultMaxQueryRecords",
                STAFString(pData->fDefaultMaxQueryRecords));

            if (pData->fDefaultResolveMessage != 0)
                listLocalSettingsMap->put("resolveMessage", "Enabled");
            else
                listLocalSettingsMap->put("resolveMessage", "Disabled");

            if (pData->fUseResolveMessageVar != 0)
                listLocalSettingsMap->put("resolveMessageVar", "Enabled");
            else
                listLocalSettingsMap->put("resolveMessageVar", "Disabled");

            // Set marshalling context map class definitions and root object

            mc->setMapClassDefinition(
                pData->fListLocalSettingsClass->reference());
            mc->setRootObject(listLocalSettingsMap);

            result = mc->marshall();
        }

        return STAFResultPtr(new STAFResult(kSTAFOk, result),
                             STAFResultPtr::INIT);
    }

    STAFFSPath path;
    STAFFSEntryType_t entryType = kSTAFFSFile;
    STAFString extPattern = kUTF8_STAR;
    unsigned int globalTimes = parsedResult->optionTimes(sGLOBAL);
    unsigned int machinesTimes = parsedResult->optionTimes(sMACHINES);
    unsigned int machineTimes = parsedResult->optionTimes(sMACHINE);
    unsigned int handlesTimes = parsedResult->optionTimes(sHANDLES);
    unsigned int handleTimes = parsedResult->optionTimes(sHANDLE);

    path.setRoot(pData->fRoot);

    if (globalTimes != 0)
    {
        path.addDir(sGLOBAL);
        extPattern = sLogExt;
    }
    else if (machinesTimes != 0)
    {
        entryType = kSTAFFSDirectory;
        path.addDir(sMACHINE);
    }
    else  // MACHINE was specified
    {
        STAFResultPtr machineValueResult = resolveOp(pInfo, pData, parsedResult,
                                                     sMACHINE);

        if (machineValueResult->rc != kSTAFOk) return machineValueResult;

        path.addDir(sMACHINE);
        path.addDir(machineValueResult->result);

        if (handlesTimes != 0)
        {
            entryType = kSTAFFSDirectory;
            path.addDir(sHANDLE);
        }
        else if (handleTimes != 0)
        {
            STAFResultPtr handleValueResult = resolveOp(pInfo, pData,
                                                        parsedResult, sHANDLE);

            if (handleValueResult->rc != kSTAFOk) return handleValueResult;

            path.addDir(sHANDLE);
            path.addDir(handleValueResult->result);
            extPattern = sLogExt;
        }
        else  // Only MACHINE was specified
        {
            path.addDir(sGLOBAL);
            extPattern = sLogExt;
        }
    }
    
    // Set marshalling context map class definitions

    mc->setMapClassDefinition(pData->fListLogsClass->reference());

    // Create a empty list object for the marshalled list of logs, machines,
    // or handles

    STAFObjectPtr outputList = STAFObject::createList();
    
    if (path.exists())
    {
        STAFFSEntryPtr dirEntry;

        try
        {
            dirEntry = path.getEntry();
        }
        catch (STAFException &e)
        {
            STAFString msg = "STAFLogService::handleList: Error getting "
                "entry for directory " + path.asString();
            STAFString result = getExceptionString(
                e, msg.toCurrentCodePage()->buffer()).adoptImpl();

            return STAFResultPtr(new STAFResult(kSTAFFileOpenError, result),
                                 STAFResultPtr::INIT);                
        }
        
        STAFFSEnumPtr dirEnum = dirEntry->enumerate(
            kUTF8_STAR, extPattern, entryType);

        for (; dirEnum->isValid(); dirEnum->next())
        {
            STAFFSEntryPtr entry = dirEnum->entry();
            STAFString data;

            if (machinesTimes || handlesTimes)
            {
                // Don't strip of the extension, because it could be
                // part of the machine name if using long names

                outputList->append(
                    entry->path().setRoot().clearDirList().asString());
            }
            else
            {
                STAFObjectPtr logMap = pData->fListLogsClass->createInstance();
                
                logMap->put("logName", 
                entry->path().setRoot().setExtension()
                       .clearDirList().asString());
                logMap->put("timestamp", entry->modTime().asString());
                logMap->put("upperSize", entry->size().first);
                logMap->put("size", entry->size().second);

                outputList->append(logMap);
            }
        }
    }

    // Set the root object for the marshalling context

    mc->setRootObject(outputList);

    // Set the result to the marshalled output for a list of log records

    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleDelete(STAFServiceRequestLevel30 *pInfo,
                           LogServiceData *pData)
{
    STAFCommandParseResultPtr parsedResult = 
        pData->fDeleteParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // Verify the requesting machine/user has at least trust level 4

    if (parsedResult->optionTimes(sRMTMACHINE) == 0)
    {
        VALIDATE_TRUST(4, pData->fShortName, "DELETE",
                       pData->fLocalMachineName);
    }
    else
    {
        // Verify the remote log machine/user has at least trust level 4

        STAFString trustRequest(
            sGetMachine + parsedResult->optionValue(sRMTMACHINE) +
            sGetUser + parsedResult->optionValue(sRMTUSER));

        STAFResultPtr trustResult = pData->fHandle->submit(
            sLocal, sTrust, trustRequest);

        if (trustResult->rc != kSTAFOk)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 4 failed.  STAF local TRUST ") + trustRequest +
                " failed with RC: " + trustResult->rc +
                ", Result: " + trustResult->result;

            return STAFResultPtr(
                new STAFResult(trustResult->rc, errorMsg),
                STAFResultPtr::INIT);
        }

        try
        {
            unsigned int trustLevel = trustResult->result.asUInt();
            
            VALIDATE_TRUST3(4, pData->fShortName, "DELETE",
                            pData->fLocalMachineName, trustLevel,
                            parsedResult->optionValue(sRMTMACHINE), "",
                            parsedResult->optionValue(sRMTUSER));
        }
        catch (STAFException)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 4 failed.  STAF local TRUST ") + trustRequest +
                " did not return a numeric trust level, Result: " +
                trustResult->result;

            return STAFResultPtr(
                new STAFResult(kSTAFAccessDenied, errorMsg),
                STAFResultPtr::INIT);
        }
    }
    
    STAFFSPath logfilePath;
    STAFResultPtr errorResult;

    if (!generateQueryPurgeDeleteLogFilePath(logfilePath, errorResult,
                                             pInfo, pData, parsedResult))
    {
        return errorResult;
    }

    if (!logfilePath.exists())
    {
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist,
                                            logfilePath.name()),
                             STAFResultPtr::INIT);
    }

    STAFFSEntryPtr logfileEntry;

    try
    {
        logfileEntry = logfilePath.getEntry();
    }
    catch (STAFException &e)
    {
        STAFString msg = "STAFLogService::handleDelete: Error getting "
            "entry for logfile " + logfilePath.asString();
        STAFString result = getExceptionString(
            e, msg.toCurrentCodePage()->buffer()).adoptImpl();

        return STAFResultPtr(new STAFResult(kSTAFFileDeleteError, result),
                             STAFResultPtr::INIT);                
    }

    // Get a write lock on the global Read/Write Semaphore

    STAFLogFileLocksPtr logLocks =
        STAFLogFileLocks::acquireLocks(logfilePath.asString());
    STAFRWSemWLock accessLock(*logLocks->logAccess);
    
    logfileEntry->remove();

    return STAFResultPtr(new STAFResult, STAFResultPtr::INIT);
}


STAFResultPtr handlePurge(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData)
{
    STAFCommandParseResultPtr parsedResult = 
        pData->fPurgeParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // Verify that the requesting machine/user has at least trust level 4

    if (parsedResult->optionTimes(sRMTMACHINE) == 0)
    {
        VALIDATE_TRUST(4, pData->fShortName, "PURGE", pData->fLocalMachineName);
    }
    else
    {
        // Verify that the remote log machine/user has at least trust level 4

        STAFString trustRequest(
            sGetMachine + parsedResult->optionValue(sRMTMACHINE) +
            sGetUser + parsedResult->optionValue(sRMTUSER));

        STAFResultPtr trustResult = pData->fHandle->submit(
            sLocal, sTrust, trustRequest);

        if (trustResult->rc != kSTAFOk)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 4 failed.  STAF local TRUST ") + trustRequest +
                " failed with RC: " + trustResult->rc +
                ", Result: " + trustResult->result;

            return STAFResultPtr(
                new STAFResult(trustResult->rc, errorMsg),
                STAFResultPtr::INIT);
        }

        try
        {
            unsigned int trustLevel = trustResult->result.asUInt();
            
            VALIDATE_TRUST3(4, pData->fShortName, "PURGE",
                            pData->fLocalMachineName, trustLevel,
                            parsedResult->optionValue(sRMTMACHINE), "",
                            parsedResult->optionValue(sRMTUSER));
        }
        catch (STAFException)
        {
            STAFString errorMsg = STAFString(
                "Verifying that the remote log machine/user has at least "
                "trust level 4 failed.  STAF local TRUST ") + trustRequest +
                " did not return a numeric trust level, Result: " +
                trustResult->result;

            return STAFResultPtr(
                new STAFResult(kSTAFAccessDenied, errorMsg),
                STAFResultPtr::INIT);
        }
    }

    // See if they specified the CONFIRMALL option

    bool confirmAll = false;

    if (parsedResult->optionTimes(sCONFIRMALL) != 0)
    {
        confirmAll = true;
    }

    // Build up the log filter criteria

    STAFResultPtr errorResult;
    LogRecordFilter logFilter;
    LogStats logStats = { 0 };

    if (!updateQueryPurgeLogFilter(logFilter, errorResult, pInfo, pData,
                                   parsedResult))
    {
        return errorResult;
    }

    unsigned int first = 0;
    unsigned int last = 0;
    STAFResultPtr result;

    if (parsedResult->optionTimes(sFIRST) != 0)
    {
        // Resolve any STAF variables in the FIRST option value

        result = resolveOp(pInfo, pData, parsedResult, sFIRST);

        if (result->rc != kSTAFOk) return result;

        // Convert resolved option string to an unsigned integer in range
        // 0 to UINT_MAX
    
        result = convertOptionStringToUInt(result->result, sFIRST, first);

        if (result->rc != kSTAFOk) return result;
    }
    else if (parsedResult->optionTimes(sLAST) != 0)
    {
        // Resolve any STAF varaibles in the LAST option value

        result = resolveOp(pInfo, pData, parsedResult, sLAST);

        if (result->rc != kSTAFOk) return result;
    
        // Convert resolved option string to an unsigned integer in range
        // 0 to UINT_MAX

        result = convertOptionStringToUInt(result->result, sLAST, last);

        if (result->rc != kSTAFOk) return result;
    }
    
    bool useFromRecord = false;
    unsigned int fromRecord = 0;
    bool useToRecord = false;
    unsigned int toRecord = 0;
    
    if (parsedResult->optionTimes(sFROMRECORD) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sFROMRECORD);

        if (result->rc != kSTAFOk) return result;

        // Convert resolved option string to an unsigned integer in range
        // 1 to UINT_MAX

        result = convertOptionStringToUInt(
            result->result, sFROMRECORD, fromRecord, 1);

        if (result->rc != kSTAFOk) return result;

        useFromRecord = true;
    }

    if (parsedResult->optionTimes(sTORECORD) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sTORECORD);

        if (result->rc != kSTAFOk) return result;
        
        // Convert resolved option string to an unsigned integer in range
        // 1 to UINT_MAX

        result = convertOptionStringToUInt(
            result->result, sTORECORD, toRecord, 1);

        if (result->rc != kSTAFOk) return result;
        
        useToRecord = true;
    }

    if (useToRecord && useFromRecord && (fromRecord > toRecord))
    {
        STAFString errorMsg = STAFString("FROMRECORD value must be <= ") +
            "TORECORD value.  FROMRECORD: " + STAFString(fromRecord) +
            ", TORECORD: " + STAFString(toRecord);
        return STAFResultPtr(new STAFResult(kSTAFInvalidValue, errorMsg),
                             STAFResultPtr::INIT);
    }

    // Get the log file path

    STAFFSPath logfilePath;

    if (!generateQueryPurgeDeleteLogFilePath(logfilePath, errorResult,
                                             pInfo, pData, parsedResult))
    {
        return errorResult;
    }

    // See if the log file exists

    if (!logfilePath.exists())
    {
        return STAFResultPtr(new STAFResult(kSTAFDoesNotExist,
                                            logfilePath.name()),
                             STAFResultPtr::INIT);
    }
    
    STAFFSEntryPtr logfileEntry;

    try
    {
        logfileEntry = logfilePath.getEntry();
    }
    catch (STAFException &e)
    {
        STAFString msg = "STAFLogService::handlePurge: Error getting "
            "entry for logfile " + logfilePath.asString();
        STAFString result = getExceptionString(
            e, msg.toCurrentCodePage()->buffer()).adoptImpl();

        return STAFResultPtr(new STAFResult(kSTAFFileOpenError, result),
                             STAFResultPtr::INIT);                
    }

    // Get a write lock on the global Read/Write Semaphore

    STAFLogFileLocksPtr logLocks =
        STAFLogFileLocks::acquireLocks(logfilePath.asString());
    STAFRWSemWLock accessLock(*logLocks->logAccess);

    // Now copy the log file to a temporary file

    STAFFSPath tempfilePath = logfilePath;

    tempfilePath.setExtension(sTmpExt);

    try
    {
        logfileEntry->copy(tempfilePath.asString());
    }
    catch (STAFException &e)
    {
        STAFString msg = "STAFLogService::handlePurge: Error copying "
            "logfile " + logfilePath.asString() + " to temporary file " +
            tempfilePath.asString();
        STAFString result = getExceptionString(
            e, msg.toCurrentCodePage()->buffer()).adoptImpl();

        return STAFResultPtr(new STAFResult(kSTAFFileWriteError, result),
                             STAFResultPtr::INIT);                
    }

    // Open the log file and temporary log file
    // Note: Notice the log file is the output file and temporary file is
    //       the input file
    // Note: We don't need to lock the temporary file since only one PURGE
    //       can occur at any given time

    fstream logfile(logfilePath.asString().toCurrentCodePage()->buffer(),
                    ios::out | STAF_ios_binary | ios::trunc);

    if (!logfile)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileOpenError,
                                            logfilePath.asString()),
                             STAFResultPtr::INIT);
    }


    fstream tempfile(tempfilePath.asString().toCurrentCodePage()->buffer(),
                     ios::in | STAF_ios_binary);

    if (!tempfile)
    {
        return STAFResultPtr(new STAFResult(kSTAFFileOpenError,
                                            tempfilePath.asString()),
                             STAFResultPtr::INIT);
    }

    // Read each record

    unsigned int currFirstRecords = 0;
    std::list<LogRecord> lastList;
    unsigned int writtenRecords = 0;
    LogRecord logRecord;
    
    // Record number counter (used to compare with fromRecord and toRecord)
    unsigned int recordNum = 0;

    while (!tempfile.eof())
    {
        // First, get the information from the log record

        unsigned int status = readLogRecordFromFile(
            tempfile, logRecord, recordNum + 1);

        if (status == kReadLogEndOfFile)
        {
            // Finish up with whatever needs to be done
            continue;
        }
        else if (status == kReadLogInvalidFormat)
        {
            // Clean/Finish up what has been done

            return STAFResultPtr(new STAFResult(kSTAFLogInvalidFileFormat,
                                 STAFString(logRecord.recordFormatID)),
                                 STAFResultPtr::INIT);
        }

        recordNum++;

        // Now, make sure this record meets the selection criteria

        bool match = true;

        if (useFromRecord)
        {
            if (recordNum < fromRecord)
            {
                // Ignore this log record
                match = false;
            }
        }

        if (useToRecord)
        {
            if (recordNum > toRecord)
            {
                match = false;
            }
        }

        // See if this record meets the selection criteria

        if (!match || (match && !logRecordMatchesFilter(
            logRecord, logFilter, pData->fDefaultAuthenticator)))
        {
            // If it doesn't match then, it isn't purged so we write it to
            // the file
            writeLogRecordToFile(logfile, logRecord);
            ++writtenRecords;
        }
        else
        {
            // If it does match, then handle first and last issues
            // Note: If first/last is not specified then the record is purged
            //       and won't be written

            if (last > 0)
            {
                // Make sure we only purge 'last' number of records
                if (lastList.size() == last)
                {
                    writeLogRecordToFile(logfile, lastList.front());
                    lastList.pop_front();
                    ++writtenRecords;
                }

                lastList.push_back(logRecord);
            }
            else if ((first > 0) && (++currFirstRecords > first))
            {
                writeLogRecordToFile(logfile, logRecord);
                ++writtenRecords;
            }
        }
    }

    unsigned int totalRecords = recordNum;

    // Make sure we didn't purge the whole file if didn't specify the
    // CONFIRMALL option and if there were any records in the log file to
    // begin with

    if ((writtenRecords == 0) && (totalRecords > 0) && !confirmAll)
    {
        // If so, copy the temp file back and return an error

        try
        {
            tempfilePath.getEntry()->copy(logfilePath.asString());
        }
        catch (STAFException &e)
        {
            STAFString msg = "STAFLogService::handlePurge: Error copying "
                "temporary file " + tempfilePath.asString() + " back to "
                "logfile " + logfilePath.asString();
            STAFString result = getExceptionString(
                e, msg.toCurrentCodePage()->buffer()).adoptImpl();
            return STAFResultPtr(new STAFResult(kSTAFLogPurgeFailure, result),
                                 STAFResultPtr::INIT);                
        }

        STAFString msg = "Your purge criteria selected every record in the "
            "log file.  Use CONFIRMALL instead of CONFIRM if you really want "
            "to delete every record (or submit a DELETE request).  Or, "
            "modify your purge criteria if you don't want to delete every "
            "record.";

        return STAFResultPtr(new STAFResult(kSTAFLogPurgeFailure, msg),
                     STAFResultPtr::INIT);
    }
    
    // Remove the temporary file
    tempfile.close();

    try
    {
        tempfilePath.getEntry()->remove();
    }
    catch (STAFException)
    {
        // Ignore
    }

    // Create a map object for the marshalled map of log purge stats

    STAFObjectPtr purgeStatsMap = pData->fPurgeStatsClass->createInstance();

    purgeStatsMap->put("purgedRecords",
                     STAFString(totalRecords - writtenRecords));
    purgeStatsMap->put("totalRecords", STAFString(totalRecords));

    // Create the marshalling context and set map class definitions and
    // its root object

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    mc->setMapClassDefinition(pData->fPurgeStatsClass->reference());
    mc->setRootObject(purgeStatsMap);

    // Set the result to the marshalled output for a map of purge statistics
    
    return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleSet(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData)
{
    // Verify that the requesting machine/user has at least trust level 5

    VALIDATE_TRUST(5, pData->fShortName, "SET", pData->fLocalMachineName);
    
    STAFString result;

    STAFCommandParseResultPtr parsedResult = 
        pData->fSetParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    if (parsedResult->optionTimes(sMAXRECORDSIZE) != 0)
    {
        STAFResultPtr maxResult = resolveOp(pInfo, pData, parsedResult,
                                            sMAXRECORDSIZE);

        if (maxResult->rc != kSTAFOk) return maxResult;
        
        // Convert resolved option string to an unsigned integer in range
        // 0 to UINT_MAX

        maxResult = convertOptionStringToUInt(
            maxResult->result, sMAXRECORDSIZE, pData->fMaxRecordSize);

        if (maxResult->rc != kSTAFOk) return maxResult;
    }

    if (parsedResult->optionTimes(sDEFAULTMAXQUERYRECORDS) != 0)
    {
        STAFResultPtr maxResult = resolveOp(
            pInfo, pData, parsedResult, sDEFAULTMAXQUERYRECORDS);

        if (maxResult->rc != kSTAFOk) return maxResult;
        
        // Convert resolved option string to an unsigned integer in range
        // 0 to UINT_MAX

        maxResult = convertOptionStringToUInt(
            maxResult->result, sDEFAULTMAXQUERYRECORDS,
            pData->fDefaultMaxQueryRecords);

        if (maxResult->rc != kSTAFOk) return maxResult;
    }

    if (parsedResult->optionTimes(sRESOLVEMESSAGE) != 0)
    {
        pData->fDefaultResolveMessage = 1;
    }
    else if (parsedResult->optionTimes(sNORESOLVEMESSAGE) != 0)
    {
        pData->fDefaultResolveMessage = 0;
    }

    if (parsedResult->optionTimes(sENABLERESOLVEMESSAGEVAR) != 0)
    {
        pData->fUseResolveMessageVar = 1;
    }
    else if (parsedResult->optionTimes(sDISABLERESOLVEMESSAGEVAR) != 0)
    {
        pData->fUseResolveMessageVar = 0;
    }

    return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
}


STAFResultPtr handleHelp(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData)
{
    // Verify that the requesting machine/user has at least trust level 1

    VALIDATE_TRUST(1, pData->fShortName, "HELP", pData->fLocalMachineName);

    // Return the help information for the service

    return STAFResultPtr(new STAFResult(kSTAFOk, sHelpMsg),
                         STAFResultPtr::INIT);
}


STAFResultPtr handleRemoteLog(STAFServiceRequestLevel30 *pInfo,
                              LogServiceData *pData)
{
    // Verify that the requesting machine/user has at least trust level 1 in
    // order to perform a remote LOG request

    VALIDATE_TRUST(1, pData->fShortName, "LOG", pData->fLocalMachineName);

    STAFCommandParseResultPtr parsedResult =
        pData->fLogParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
                             parsedResult->errorBuffer), STAFResultPtr::INIT);
    }

    // Get log level and log mask and see if we should log

    STAFResultPtr levelResult = resolveOp(pInfo, pData, parsedResult, sLEVEL);

    if (levelResult->rc != kSTAFOk) return levelResult;

    unsigned int logLevel = 0;

    if (!isValidLogLevel(levelResult->result, logLevel))
    {
        return STAFResultPtr(new STAFResult(kSTAFLogInvalidLevel,
                                            levelResult->result),
                             STAFResultPtr::INIT);
    }

    STAFResultPtr logmaskResult = resolveStr(pInfo, pData,
                                             pData->fResolveLogMaskString);
    if ((logmaskResult->rc != kSTAFOk) &&
        (logmaskResult->rc != kSTAFVariableDoesNotExist))
    {
        return logmaskResult;
    }

    if (logmaskResult->rc == kSTAFVariableDoesNotExist)
        logmaskResult->result = sEverythingLogMask;

    unsigned int logMask = 0;

    if (!convertLogMaskToUInt(logmaskResult->result, logMask))
    {
        return STAFResultPtr(new STAFResult(kSTAFLogInvalidLevel,
                                            logmaskResult->result),
                             STAFResultPtr::INIT);
    }

    if (!(logLevel & logMask))
        return STAFResultPtr(new STAFResult, STAFResultPtr::INIT);

    // Now pass the request on to the remote log server

    STAFString newRequest(pInfo->request);

    newRequest += sSpace;
    newRequest += sRMTMACHINE;
    newRequest += sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->endpoint);
    newRequest += sSpace;
    newRequest += sRMTNAME;
    newRequest += sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->handleName);
    newRequest += sSpace;
    newRequest += sRMTHANDLE;
    newRequest += sSpace;
    newRequest += STAFString(pInfo->handle);
    newRequest += sSpace + sRMTUSER + sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->user);
    newRequest += sSpace + sRMTMACH + sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->machine);
    newRequest += sSpace + sRMTNICKNAME + sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->machineNickname);

    return pData->fHandle->submit(pData->fRemoteLogServer,
                                  pData->fRemoteLogService, newRequest);
}


STAFResultPtr handleRemoteLogGeneral(STAFServiceRequestLevel30 *pInfo,
                                     LogServiceData *pData)
{
    // Verify that the requesting machine/user has at least trust level 1
    // in order to submit a request to the remote log service

    STAFString request = STAFString(pInfo->request).subWord(0, 1).toUpperCase();
    
    VALIDATE_TRUST(1, pData->fShortName, request, pData->fLocalMachineName);
    
    // Pass the request on to the remote log server

    STAFString newRequest(pInfo->request);

    newRequest += sSpace;
    newRequest += sRMTMACHINE;
    newRequest += sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->machine);
    newRequest += sSpace + sRMTUSER + sSpace;
    newRequest += pData->fHandle->wrapData(pInfo->user);
    
    return pData->fHandle->submit(pData->fRemoteLogServer,
                                  pData->fRemoteLogService, newRequest);
}


STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
                         LogServiceData *pData, const STAFString &theString)
{
    return pData->fHandle->submit(
        sLocal, sVar, sResStrResolve + STAFString(pInfo->requestNumber) +
        sString + pData->fHandle->wrapData(theString));
}


STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo, LogServiceData *pData,
                        STAFCommandParseResultPtr &parsedResult,
                        const STAFString &fOption, unsigned int optionIndex)
{
    // ???: Would const STAFString & work here?
    STAFString optionValue = parsedResult->optionValue(fOption, optionIndex);

    if (optionValue.find(sLeftCurly) == STAFString::kNPos)
    {
        return STAFResultPtr(new STAFResult(kSTAFOk, optionValue),
                             STAFResultPtr::INIT);
    }

    return resolveStr(pInfo, pData, optionValue);
}


STAFResultPtr resolveOpLocal(LogServiceData *pData,
                             STAFCommandParseResultPtr &parsedResult,
                             const STAFString &fOption,
                             unsigned int optionIndex)
{
    // ???: Would const STAFString & work here?
    STAFString optionValue = parsedResult->optionValue(fOption, optionIndex);

    if (optionValue.find(sLeftCurly) == STAFString::kNPos)
    {
        return STAFResultPtr(new STAFResult(kSTAFOk, optionValue),
                             STAFResultPtr::INIT);
    }

    return pData->fHandle->submit(sLocal, sVar, sResStrResolve +
                                     sString +
                                     pData->fHandle->wrapData(optionValue));
}


STAFResultPtr convertStringToUInt(const STAFString &theString,
                                  unsigned int &number,
                                  const unsigned int minValue,
                                  const unsigned int maxValue)
{
    return convertOptionStringToUInt(theString, "", number,
                                     minValue, maxValue);
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


bool isValidLogLevel(const STAFString &levelString, unsigned int &outputLevel)
{
    if (levelString.findFirstNotOf(sZeroOne) == STAFString::kNPos)
    {
        // If we are here, the level string is all zeros and ones
        // If the level string has more than 32 characters or has more than
        // one "1" then it is invalid

        if (levelString.length(STAFString::kChar) > 32) return false;
        if (levelString.count(sOne) != 1) return false;

        unsigned int index = levelString.find(sOne, 0, STAFString::kChar);

        outputLevel = 1;
        outputLevel <<= (levelString.length(STAFString::kChar) - index - 1);
    }
    else
    {
        if (!convertLogLevelToUInt(levelString, outputLevel)) return false;
    }

    if ((outputLevel > 0x00004000) && (outputLevel < 0x01000000)) return false;

    return true;
}


// returns true if the level string is understood
// returns false if the level string is bad

bool convertLogLevelToUInt(const STAFString &levelString,
                           unsigned int &outputLevel)
{
    STAFString upperLevel = levelString.toUpperCase();

    if (upperLevel == sFATAL)        outputLevel = 0x00000001;
    else if (upperLevel == sERROR)   outputLevel = 0x00000002;
    else if (upperLevel == sWARNING) outputLevel = 0x00000004;
    else if (upperLevel == sINFO)    outputLevel = 0x00000008;
    else if (upperLevel == sTRACE)   outputLevel = 0x00000010;
    else if (upperLevel == sTRACE2)  outputLevel = 0x00000020;
    else if (upperLevel == sTRACE3)  outputLevel = 0x00000040;
    else if (upperLevel == sDEBUG)   outputLevel = 0x00000080;
    else if (upperLevel == sDEBUG2)  outputLevel = 0x00000100;
    else if (upperLevel == sDEBUG3)  outputLevel = 0x00000200;
    else if (upperLevel == sSTART)   outputLevel = 0x00000400;
    else if (upperLevel == sSTOP)    outputLevel = 0x00000800;
    else if (upperLevel == sPASS)    outputLevel = 0x00001000;
    else if (upperLevel == sFAIL)    outputLevel = 0x00002000;
    else if (upperLevel == sSTATUS)  outputLevel = 0x00004000;
    else if (upperLevel == sUSER1)   outputLevel = 0x01000000;
    else if (upperLevel == sUSER2)   outputLevel = 0x02000000;
    else if (upperLevel == sUSER3)   outputLevel = 0x04000000;
    else if (upperLevel == sUSER4)   outputLevel = 0x08000000;
    else if (upperLevel == sUSER5)   outputLevel = 0x10000000;
    else if (upperLevel == sUSER6)   outputLevel = 0x20000000;
    else if (upperLevel == sUSER7)   outputLevel = 0x40000000;
    else if (upperLevel == sUSER8)   outputLevel = 0x80000000;
    else return false;

    return true;
}


STAFString &convertLogLevelToString(unsigned int logLevel, bool levelAsBits)
{
    if (logLevel == 0x00000001)
        return levelAsBits ? sFATALBits : sFATALPretty;
    else if (logLevel == 0x00000002)
        return levelAsBits ? sERRORBits : sERRORPretty;
    else if (logLevel == 0x00000004)
        return levelAsBits ? sWARNINGBits :  sWARNINGPretty;
    else if (logLevel == 0x00000008)
        return levelAsBits ? sINFOBits : sINFOPretty;
    else if (logLevel == 0x00000010)
        return levelAsBits ? sTRACEBits : sTRACEPretty;
    else if (logLevel == 0x00000020)
        return levelAsBits ? sTRACE2Bits : sTRACE2Pretty;
    else if (logLevel == 0x00000040)
        return levelAsBits ? sTRACE3Bits : sTRACE3Pretty;
    else if (logLevel == 0x00000080)
        return levelAsBits ? sDEBUGBits : sDEBUGPretty;
    else if (logLevel == 0x00000100)
        return levelAsBits ? sDEBUG2Bits : sDEBUG2Pretty;
    else if (logLevel == 0x00000200)
        return levelAsBits ? sDEBUG3Bits : sDEBUG3Pretty;
    else if (logLevel == 0x00000400)
        return levelAsBits ? sSTARTBits : sSTARTPretty;
    else if (logLevel == 0x00000800)
        return levelAsBits ? sSTOPBits : sSTOPPretty;
    else if (logLevel == 0x00001000)
        return levelAsBits ? sPASSBits : sPASSPretty;
    else if (logLevel == 0x00002000)
        return levelAsBits ? sFAILBits : sFAILPretty;
    else if (logLevel == 0x00004000)
        return levelAsBits ? sSTATUSBits : sSTATUSPretty;
    else if (logLevel == 0x01000000)
        return levelAsBits ? sUSER1Bits : sUSER1Pretty;
    else if (logLevel == 0x02000000)
        return levelAsBits ? sUSER2Bits : sUSER2Pretty;
    else if (logLevel == 0x04000000)
        return levelAsBits ? sUSER3Bits : sUSER3Pretty;
    else if (logLevel == 0x08000000)
        return levelAsBits ? sUSER4Bits : sUSER4Pretty;
    else if (logLevel == 0x10000000)
        return levelAsBits ? sUSER5Bits : sUSER5Pretty;
    else if (logLevel == 0x20000000)
        return levelAsBits ? sUSER6Bits : sUSER6Pretty;
    else if (logLevel == 0x40000000)
        return levelAsBits ? sUSER7Bits : sUSER7Pretty;
    else if (logLevel == 0x80000000)
        return levelAsBits ? sUSER8Bits : sUSER8Pretty;
    else
        return levelAsBits ? sUNKNOWNBits : sUNKNOWNPretty;
}


bool convertLogMaskToUInt(const STAFString &logmaskString,
                          unsigned int &logmaskOutput)
{
    logmaskOutput = 0;

    if (logmaskString.findFirstNotOf(sZeroOne) == STAFString::kNPos)
    {
        // If we are here, the level string is all zeros and ones
        // If the level string has more than 32 characters then it is invalid

        if (logmaskString.length(STAFString::kChar) > 32) return false;

        for (int i = 0; i < logmaskString.length(STAFString::kChar); ++i)
        {
            if (logmaskString.subString(i, 1, STAFString::kChar) == sOne)
            {
                logmaskOutput |=
                    1 << (logmaskString.length(STAFString::kChar) - i - 1);
            }
        }
    }
    else
    {
        for (int i = 0; i < logmaskString.numWords(); ++i)
        {
            unsigned int thisLevel = 0;

            if (convertLogLevelToUInt(logmaskString.subWord(i, 1), thisLevel))
                logmaskOutput |= thisLevel;
            else
                return false;
        }
    }

    return true;
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


void writeStringToFile(ostream &output, const STAFString &outString)
{
    unsigned int stringLength = outString.length();

    writeUIntToFile(output, stringLength);
    output.write(outString.buffer(), stringLength);
}


// returns true if the logfile path was generated successfully
// returns false (and errorResult is set) if there was an error generating
//     the logfile path

bool generateQueryPurgeDeleteLogFilePath(STAFFSPath &logfilePath,
     STAFResultPtr &errorResult, STAFServiceRequestLevel30 *pInfo,
     LogServiceData *pData, STAFCommandParseResultPtr &parsedResult)
{
    // Get logname

    STAFResultPtr lognameResult = resolveOp(pInfo, pData, parsedResult,
                                            sLOGNAME);

    if (lognameResult->rc != kSTAFOk)
    {
        errorResult = lognameResult;
        return false;
    }

    // Setup the path

    logfilePath.setRoot(pData->fRoot);

    if (parsedResult->optionTimes(sGLOBAL) != 0)
    {
        // They specified a GLOBAL log file

        logfilePath.addDir(sGLOBAL);
    }
    else
    {
        // They specified at least a MACHINE log file

        STAFResultPtr machineResult = resolveOp(pInfo, pData, parsedResult,
                                                sMACHINE);

        if (machineResult->rc != kSTAFOk)
        {
            errorResult = machineResult;
            return false;
        }

        logfilePath.addDir(sMACHINE);
        logfilePath.addDir(machineResult->result);

        if (parsedResult->optionTimes(sHANDLE) != 0)
        {
            // They specified a HANDLE log file

            STAFResultPtr handleResult = resolveOp(pInfo, pData, parsedResult,
                                                   sHANDLE);

            if (handleResult->rc != kSTAFOk)
            {
                errorResult = handleResult;
                return false;
            }

            logfilePath.addDir(sHANDLE);
            logfilePath.addDir(handleResult->result);
        }
        else
        {
            logfilePath.addDir(sGLOBAL);
        }
    }

    logfilePath.setName(lognameResult->result);
    logfilePath.setExtension(sLogExt);

    return true;
}


unsigned int readLogRecordFromFile(istream &logfile, LogRecord &logRecord,
                                   unsigned int recordNum)
{
    unsigned int totalLength = 0;

    readUIntFromFile(logfile, logRecord.recordFormatID, 1);

    if (logfile.eof()) return kReadLogEndOfFile;

    logRecord.recordNumber = recordNum;

    if (logRecord.recordFormatID == sCurrRecordFormatID)
    {
        // This format added endpoint to the log record

        readUIntFromFile(logfile, logRecord.date);
        readUIntFromFile(logfile, logRecord.secondsPastMidnight, 3);
        readUIntFromFile(logfile, logRecord.logLevel);
        readUIntFromFile(logfile, logRecord.handle);
        readStringFromFile(logfile, logRecord.machine);
        readStringFromFile(logfile, logRecord.handleName);
        readStringFromFile(logfile, logRecord.user);
        readStringFromFile(logfile, logRecord.endpoint);
        readStringFromFile(logfile, logRecord.message);
    }
    else if (logRecord.recordFormatID == 3)
    {
        // This format added user to the log record for Feature #627135

        readUIntFromFile(logfile, logRecord.date);
        readUIntFromFile(logfile, logRecord.secondsPastMidnight, 3);
        readUIntFromFile(logfile, logRecord.logLevel);
        readUIntFromFile(logfile, logRecord.handle);
        readStringFromFile(logfile, logRecord.machine);
        readStringFromFile(logfile, logRecord.handleName);
        readStringFromFile(logfile, logRecord.user);
        readStringFromFile(logfile, logRecord.message);
        logRecord.endpoint = "tcp" + sSpecSeparator + logRecord.machine;
    }
    else if (logRecord.recordFormatID == 2)
    {
        readUIntFromFile(logfile, logRecord.date);
        readUIntFromFile(logfile, logRecord.secondsPastMidnight, 3);
        readUIntFromFile(logfile, logRecord.logLevel);
        readUIntFromFile(logfile, logRecord.handle);
        readStringFromFile(logfile, logRecord.machine);
        readStringFromFile(logfile, logRecord.handleName);
        readStringFromFile(logfile, logRecord.message);
        logRecord.user = sUnauthenticatedUser;
        logRecord.endpoint = "tcp" + sSpecSeparator + logRecord.machine;
    }
    else if (logRecord.recordFormatID == 1)
    {
        // This is the format used by the last couple of implementations
        // of the Log service written in Rexx

        readUIntFromFile(logfile, logRecord.date);
        readUIntFromFile(logfile, logRecord.secondsPastMidnight, 3);
        readUIntFromFile(logfile, logRecord.logLevel);
        readUIntFromFile(logfile, totalLength);

        // Get a buffer and read the rest of the data

        STAFRefPtr<char> data = STAFRefPtr<char>(new char[totalLength],
                                                 STAFRefPtr<char>::INIT);
        logfile.read(data, totalLength);

        STAFString dataString(data, totalLength, STAFString::kCurrent);

        // Find the separation points for the other fields

        unsigned int sepLoc1 = dataString.find(sOldSep);
        unsigned int sepLoc2 = dataString.find(sOldSep, sepLoc1 + 1);
        unsigned int sepLoc3 = dataString.find(sOldSep, sepLoc2 + 1);

        logRecord.machine = dataString.subString(0, sepLoc1);
        logRecord.handle = dataString.subString(
            sepLoc1 + 1, sepLoc2 - sepLoc1 - 1).asUIntWithDefault(0);
        logRecord.handleName = dataString.subString(
            sepLoc2 + 1, sepLoc3 - sepLoc2 - 1);
        logRecord.message = dataString.subString(sepLoc3 + 1);

        logRecord.user = sUnauthenticatedUser;
        logRecord.endpoint = "tcp" + sSpecSeparator + logRecord.machine;
    }
    else if (logRecord.recordFormatID == 0)
    {
        // This is the format used by the first few implementations of the
        // log service written in Rexx

        readUIntFromFile(logfile, totalLength);
        readUIntFromFile(logfile, logRecord.date);
        readUIntFromFile(logfile, logRecord.secondsPastMidnight, 3);

        // Remove date and time length from totalLength

        totalLength -= 7;

        // Get a buffer and read the rest of the data

        STAFRefPtr<char> data = STAFRefPtr<char>(new char[totalLength],
                                                 STAFRefPtr<char>::INIT);
        logfile.read(data, totalLength);

        STAFString dataString(data, totalLength, STAFString::kCurrent);

        // Find the separation points for the other fields

        unsigned int sepLoc1 = dataString.find(sOldSep);
        unsigned int sepLoc2 = dataString.find(sOldSep, sepLoc1 + 1);
        unsigned int sepLoc3 = dataString.find(sOldSep, sepLoc2 + 1);

        logRecord.machine = dataString.subString(0, sepLoc1);
        logRecord.handle = dataString.subString(
            sepLoc1 + 1, sepLoc2 - sepLoc1 - 1).asUIntWithDefault(0);
        logRecord.handleName = dataString.subString(
            sepLoc2 + 1, sepLoc3 - sepLoc2 - 1);
        logRecord.message = dataString.subString(sepLoc3 + 5);

        logRecord.logLevel = *(reinterpret_cast<unsigned int *>(
                               const_cast<char *>(dataString.buffer()
                                                  + sepLoc3 + 1)));

        // Fixup log level from Rexx's big-endian to native format

        logRecord.logLevel = STAFUtilConvertLEUIntToNative(
            STAFUtilSwapUInt(logRecord.logLevel));

        logRecord.user = sUnauthenticatedUser;
        logRecord.endpoint = "tcp" + sSpecSeparator + logRecord.machine;
    }
    else
    {
        return kReadLogInvalidFormat;
    }

    return kReadLogOk;
}


void writeLogRecordToFile(ostream &logfile, LogRecord &logRecord)
{
    writeUIntToFile(logfile, sCurrRecordFormatID, 1);
    writeUIntToFile(logfile, logRecord.date);
    writeUIntToFile(logfile, logRecord.secondsPastMidnight, 3);
    writeUIntToFile(logfile, logRecord.logLevel);
    writeUIntToFile(logfile, logRecord.handle);
    writeStringToFile(logfile, logRecord.machine);
    writeStringToFile(logfile, logRecord.handleName);
    writeStringToFile(logfile, logRecord.user);
    writeStringToFile(logfile, logRecord.endpoint);
    writeStringToFile(logfile, STAFHandle::maskPrivateData(logRecord.message));
}


// Add a log record in a marshalling context as a map to the logList

void addLogRecordToList(STAFObjectPtr &logList,
                        STAFMapClassDefinitionPtr &logRecordClass, 
                        const LogRecord &logRecord, bool levelAsBits,
                        bool longFormat)
{
    unsigned int year = logRecord.date / 10000;
    unsigned int month = (logRecord.date % 10000) / 100;
    unsigned int day = logRecord.date % 100;
    unsigned int hour = logRecord.secondsPastMidnight / 3600;
    unsigned int minute = (logRecord.secondsPastMidnight % 3600) / 60;
    unsigned int second = logRecord.secondsPastMidnight % 60;
    STAFTimestamp theTimestamp(year, month, day, hour, minute, second);

    STAFObjectPtr logRecordMap = logRecordClass->createInstance();

    logRecordMap->put("timestamp", theTimestamp.asString());
    logRecordMap->put("level", convertLogLevelToString(
        logRecord.logLevel, levelAsBits));
    logRecordMap->put("message", logRecord.message);

    if (longFormat)
    {
        logRecordMap->put("recordNumber", STAFString(logRecord.recordNumber));
        logRecordMap->put("machine", logRecord.machine);
        logRecordMap->put("handle", STAFString(logRecord.handle));
        logRecordMap->put("handleName", logRecord.handleName);
        logRecordMap->put("user", logRecord.user);
        logRecordMap->put("endpoint", logRecord.endpoint);
    }

    logList->append(logRecordMap);
}


void printLogRecord(const LogRecord &logRecord)
{
    cout << "Record #: " << logRecord.recordNumber
         << ", Record ID: " << logRecord.recordFormatID
         << ", Date: " << logRecord.date
         << ", Seconds: " << logRecord.secondsPastMidnight
         << ", Level: 0x" << hex << logRecord.logLevel << dec << endl
         << "Machine: " << logRecord.machine
         << ", HandleName: " << logRecord.handleName
         << " ,Handle: " << logRecord.handle
         << ", User: " << logRecord.user
         << ", Endpoint: " << logRecord.endpoint
         << endl << "Message: " << logRecord.message << endl << endl;
}


bool logRecordMatchesFilter(const LogRecord &logRecord,
                            const LogRecordFilter &logFilter,
                            const STAFString &defaultAuthenticator)
{
    // Check if the record matches the specified date(s)

    if (logFilter.useFrom &&
        ((logRecord.date < logFilter.fromTimestamp.date) ||
         ((logRecord.date == logFilter.fromTimestamp.date) &&
          (logRecord.secondsPastMidnight < logFilter.fromTimestamp.seconds))))
    {
        return false;
    }

    if (logFilter.useAfter &&
        ((logRecord.date < logFilter.afterTimestamp.date) ||
         ((logRecord.date == logFilter.afterTimestamp.date) &&
          (logRecord.secondsPastMidnight <= logFilter.afterTimestamp.seconds))))
    {
        return false;
    }

    if (logFilter.useBefore &&
        ((logRecord.date > logFilter.beforeTimestamp.date) ||
         ((logRecord.date == logFilter.beforeTimestamp.date) &&
          (logRecord.secondsPastMidnight >= logFilter.beforeTimestamp.seconds))))
    {
        return false;
    }

    if (logFilter.useTo &&
        ((logRecord.date > logFilter.toTimestamp.date) ||
         ((logRecord.date == logFilter.toTimestamp.date) &&
          (logRecord.secondsPastMidnight > logFilter.toTimestamp.seconds))))
    {
        return false;
    }

    if (logFilter.useLevelMask && !(logFilter.levelMask & logRecord.logLevel))
        return false;

    // Check if the record matches at least one of the specified QMACHINE(s)

    StringList::const_iterator stringIter;
    bool foundMatch = false;

    for (stringIter = logFilter.qMachines.begin();
         !foundMatch && (stringIter != logFilter.qMachines.end()); ++stringIter)
    {
        if (logRecord.machine.isEqualTo(*stringIter,
                                        kSTAFStringCaseInsensitive))
        {
            foundMatch = true;
        }
    }

    if ((logFilter.qMachines.size() != 0) && !foundMatch)
        return false;

    // Check if the record matches at least one of ths specified handle NAME(s)

    foundMatch = false;

    for (stringIter = logFilter.names.begin();
         !foundMatch && (stringIter != logFilter.names.end()); ++stringIter)
    {
        if (logRecord.handleName.isEqualTo(*stringIter,
                                           kSTAFStringCaseInsensitive))
        {
            foundMatch = true;
        }
    }

    if ((logFilter.names.size() != 0) && !foundMatch)
        return false;

    // Check if the record matches at least one of the specified ENDPOINT(s)

    foundMatch = false;

    for (stringIter = logFilter.endpoints.begin();
         !foundMatch && (stringIter != logFilter.endpoints.end());
         ++stringIter)
    {
        if (logRecord.endpoint.isEqualTo(*stringIter,
                                         kSTAFStringCaseInsensitive))
        {
            foundMatch = true;
        }
    }

    if ((logFilter.endpoints.size() != 0) && !foundMatch)
        return false;
    
    /*  XXX: This used to do a case-insensitive match on authenticator
             and a case-sensitive match on user identifer and match
             using wildcards.  We decided to remove this until we came
             up with a more generic matching scheme to use for all
             STAF requests that also allows some sort of escaping in
             case someone really wanted to match on a * or ? characater
             
    // Change authenticator in logRecord's user to lower-case

    STAFString lowerLogRecordUser = logRecord.user;

    unsigned int sepIndex = lowerLogRecordUser.find(sSpecSeparator);

    if (sepIndex != STAFString::kNPos)
    {
        lowerLogRecordUser = 
           logRecord.user.subString(0, sepIndex).toLowerCase() +
           logRecord.user.subString(sepIndex);
    }
    
    // Check if the record matches at least one USER option

    foundMatch = false;

    for (stringIter = logFilter.users.begin();
         !foundMatch && (stringIter != logFilter.users.end()); ++stringIter)
    {
        STAFString lowerUser = *stringIter;

        // Check if authenticator was specified in the user value

        sepIndex = lowerUser.find(sSpecSeparator);

        if (sepIndex == STAFString::kNPos)
        {
            // No authenticator specified - get default authenticator
            // and change authenticator to lower-case
            lowerUser = defaultAuthenticator.toLowerCase() + sSpecSeparator +
                        lowerUser;
        }
        else
        {
            // User specified in form of Authenticator://UserIdentifier
            // Change authenticator to lower-case.
            lowerUser = lowerUser.subString(0, sepIndex).toLowerCase() +
                        lowerUser.subString(sepIndex);
        }

        if (lowerLogRecordUser.matchesWildcards(lowerUser))
            foundMatch = true;
    }
    */

    // Check if the record matches at least one of the specified USER(s)

    foundMatch = false;

    for (stringIter = logFilter.users.begin();
         !foundMatch && (stringIter != logFilter.users.end());
         ++stringIter)
    {
        if (logRecord.user.isEqualTo(*stringIter,
                                     kSTAFStringCaseInsensitive))
        {
            foundMatch = true;
        }
    }

    if ((logFilter.users.size() != 0) && !foundMatch)
        return false;

    // Check if matches at least one CONTAINS option

    STAFString lowerMessage = logRecord.message.toLowerCase();
    foundMatch = false;

    for (stringIter = logFilter.contains.begin();
         !foundMatch && (stringIter != logFilter.contains.end()); ++stringIter)
    {
        if (lowerMessage.find(*stringIter) != STAFString::kNPos)
            foundMatch = true;
    }

    if ((logFilter.contains.size() != 0) && !foundMatch)
        return false;

    // Check if matches at least one CSCONTAINS option

    STAFString message = logRecord.message;
    foundMatch = false;

    for (stringIter = logFilter.cscontains.begin();
         !foundMatch && (stringIter != logFilter.cscontains.end()); ++stringIter)
    {
        if (message.find(*stringIter) != STAFString::kNPos)
            foundMatch = true;
    }

    if ((logFilter.cscontains.size() != 0) && !foundMatch)
        return false;

    // Check if matches at least one STARTSWITH or CSSTARTSWITH option

    foundMatch = false;

    for (stringIter = logFilter.startswith.begin();
         !foundMatch && (stringIter != logFilter.startswith.end());
         ++stringIter)
    {
        if (lowerMessage.find(*stringIter) == 0)
            foundMatch = true;
    }

    for (stringIter = logFilter.csstartswith.begin();
         !foundMatch && (stringIter != logFilter.csstartswith.end());
         ++stringIter)
    {
        if (message.find(*stringIter) == 0)
            foundMatch = true;
    }

    if ((logFilter.startswith.size() != 0 || logFilter.csstartswith.size() != 0)
        && !foundMatch)
        return false;

    // Check if matches at least one QHANDLE option

    foundMatch = false;

    for (HandleList::const_iterator hIter = logFilter.qHandles.begin();
         !foundMatch && (hIter != logFilter.qHandles.end()); ++hIter)
    {
      if (*hIter == logRecord.handle)
          foundMatch = true;
    }

    if ((logFilter.qHandles.size() != 0) && !foundMatch)
        return false;

    return true;
}


// returns true if the log filter was updated successfully
// returns false (and result is set) if there was an error updating
//         the log filter

bool updateQueryPurgeLogFilter(LogRecordFilter &logFilter,
     STAFResultPtr &result, STAFServiceRequestLevel30 *pInfo,
     LogServiceData *pData, STAFCommandParseResultPtr &parsedResult)
{
    int i = 0;

    for (i = 1; i <= parsedResult->optionTimes(sCONTAINS); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sCONTAINS, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.contains.push_back(result->result.toLowerCase());
    }

    for (i = 1; i <= parsedResult->optionTimes(sCSCONTAINS); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sCSCONTAINS, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.cscontains.push_back(result->result);
    }

    for (i = 1; i <= parsedResult->optionTimes(sSTARTSWITH); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sSTARTSWITH, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.startswith.push_back(result->result.toLowerCase());
    }

    for (i = 1; i <= parsedResult->optionTimes(sCSSTARTSWITH); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sCSSTARTSWITH, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.csstartswith.push_back(result->result);
    }

    for (i = 1; i <= parsedResult->optionTimes(sQMACHINE); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sQMACHINE, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.qMachines.push_back(result->result);
    }

    for (i = 1; i <= parsedResult->optionTimes(sNAME); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sNAME, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.names.push_back(result->result);
    }

    for (i = 1; i <= parsedResult->optionTimes(sUSER); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sUSER, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.users.push_back(result->result);
    }

    for (i = 1; i <= parsedResult->optionTimes(sENDPOINT); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sENDPOINT, i);

        if (result->rc != kSTAFOk) return false;

        logFilter.endpoints.push_back(result->result);
    }

    for (i = 1; i <= parsedResult->optionTimes(sQHANDLE); ++i)
    {
        result = resolveOp(pInfo, pData, parsedResult, sQHANDLE, i);

        if (result->rc != kSTAFOk) return false;
        
        // Convert resolved option string to an unsigned integer in range
        // 1 to UINT_MAX

        unsigned int qHandle;

        result = convertOptionStringToUInt(
            result->result, sQHANDLE, qHandle, 1);

        if (result->rc != kSTAFOk) return false;

        logFilter.qHandles.push_back(qHandle);
    }

    if (parsedResult->optionTimes(sLEVELMASK) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sLEVELMASK);

        if (result->rc != kSTAFOk) return false;

        if (!convertLogMaskToUInt(result->result, logFilter.levelMask))
        {
            result->rc = kSTAFLogInvalidLevel;
            return false;
        }

        logFilter.useLevelMask = true;
    }

    if (parsedResult->optionTimes(sFROM) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sFROM);

        if (result->rc != kSTAFOk) return false;

        // Replace case-insensitive occurrence of TODAY with current date
        STAFString timestampString = result->result.toUpperCase().replace(
            sTODAY, (STAFTimestamp::now().asString()).subString(0, 8));

        if (!STAFTimestamp::isValidTimestampString(timestampString,
                                                   sTimestampSeps))
        {
            result->rc = kSTAFInvalidValue;
            return false;
        }

        STAFTimestamp from(timestampString, sTimestampSeps);

        logFilter.useFrom = true;
        logFilter.fromTimestamp.date = from.asDateString().asUInt();
        logFilter.fromTimestamp.seconds = from.asSecondsPastMidnight();
    }

    if (parsedResult->optionTimes(sAFTER) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sAFTER);

        if (result->rc != kSTAFOk) return false;

        // Replace case-insensitive occurrence of TODAY with current date
        STAFString timestampString = result->result.toUpperCase().replace(
            sTODAY, (STAFTimestamp::now().asString()).subString(0, 8));

        if (!STAFTimestamp::isValidTimestampString(timestampString,
                                                   sTimestampSeps))
        {
            result->rc = kSTAFInvalidValue;
            return false;
        }

        STAFTimestamp after(timestampString, sTimestampSeps);

        logFilter.useAfter = true;
        logFilter.afterTimestamp.date = after.asDateString().asUInt();
        logFilter.afterTimestamp.seconds = after.asSecondsPastMidnight();
    }

    if (parsedResult->optionTimes(sBEFORE) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sBEFORE);

        if (result->rc != kSTAFOk) return false;

        // Replace case-insensitive occurrence of TODAY with current date
        STAFString timestampString = result->result.toUpperCase().replace(
            sTODAY, (STAFTimestamp::now().asString()).subString(0, 8));

        if (!STAFTimestamp::isValidTimestampString(timestampString,
                                                   sTimestampSeps))
        {
            result->rc = kSTAFInvalidValue;
            return false;
        }

        STAFTimestamp before(timestampString, sTimestampSeps);

        logFilter.useBefore = true;
        logFilter.beforeTimestamp.date = before.asDateString().asUInt();
        logFilter.beforeTimestamp.seconds = before.asSecondsPastMidnight();
    }

    if (parsedResult->optionTimes(sTO) != 0)
    {
        result = resolveOp(pInfo, pData, parsedResult, sTO);

        if (result->rc != kSTAFOk) return false;

        // Replace case-insensitive occurrence of TODAY with current date
        STAFString timestampString = result->result.toUpperCase().replace(
            sTODAY, (STAFTimestamp::now().asString()).subString(0, 8));

        if (!STAFTimestamp::isValidTimestampString(timestampString,
                                                   sTimestampSeps))
        {
            result->rc = kSTAFInvalidValue;
            return false;
        }

        STAFTimestamp to(timestampString, sTimestampSeps);

        logFilter.useTo = true;
        logFilter.toTimestamp.date = to.asDateString().asUInt();
        logFilter.toTimestamp.seconds = to.asSecondsPastMidnight();
    }

    return true;
}


void updateLogStats(LogStats &logStats, unsigned int logLevel)
{
    if (logLevel == 0x00000001)      ++logStats.fatal;
    else if (logLevel == 0x00000002) ++logStats.error;
    else if (logLevel == 0x00000004) ++logStats.warning;
    else if (logLevel == 0x00000008) ++logStats.info;
    else if (logLevel == 0x00000010) ++logStats.trace;
    else if (logLevel == 0x00000020) ++logStats.trace2;
    else if (logLevel == 0x00000040) ++logStats.trace3;
    else if (logLevel == 0x00000080) ++logStats.debug;
    else if (logLevel == 0x00000100) ++logStats.debug2;
    else if (logLevel == 0x00000200) ++logStats.debug3;
    else if (logLevel == 0x00000400) ++logStats.start;
    else if (logLevel == 0x00000800) ++logStats.stop;
    else if (logLevel == 0x00001000) ++logStats.pass;
    else if (logLevel == 0x00002000) ++logStats.fail;
    else if (logLevel == 0x00004000) ++logStats.status;
    else if (logLevel == 0x01000000) ++logStats.user1;
    else if (logLevel == 0x02000000) ++logStats.user2;
    else if (logLevel == 0x04000000) ++logStats.user3;
    else if (logLevel == 0x08000000) ++logStats.user4;
    else if (logLevel == 0x10000000) ++logStats.user5;
    else if (logLevel == 0x20000000) ++logStats.user6;
    else if (logLevel == 0x40000000) ++logStats.user7;
    else if (logLevel == 0x80000000) ++logStats.user8;
}


// Add log stats in a marshalling context as a map to the logList

void addLogStatsToMap(STAFObjectPtr &queryStatsMap, const LogStats &logStats)
{
    queryStatsMap->put("fatal",   STAFString(logStats.fatal));
    queryStatsMap->put("error",   STAFString(logStats.error));
    queryStatsMap->put("warning", STAFString(logStats.warning));
    queryStatsMap->put("info",    STAFString(logStats.info));
    queryStatsMap->put("trace",   STAFString(logStats.trace));
    queryStatsMap->put("trace2",  STAFString(logStats.trace2));
    queryStatsMap->put("trace3",  STAFString(logStats.trace3));
    queryStatsMap->put("debug",   STAFString(logStats.debug));
    queryStatsMap->put("debug2",  STAFString(logStats.debug2));
    queryStatsMap->put("debug3",  STAFString(logStats.debug3));
    queryStatsMap->put("start",   STAFString(logStats.start));
    queryStatsMap->put("stop",    STAFString(logStats.stop));
    queryStatsMap->put("pass",    STAFString(logStats.pass));
    queryStatsMap->put("fail",    STAFString(logStats.fail));
    queryStatsMap->put("status",  STAFString(logStats.status));
    queryStatsMap->put("user1",   STAFString(logStats.user1));
    queryStatsMap->put("user2",   STAFString(logStats.user2));
    queryStatsMap->put("user3",   STAFString(logStats.user3));
    queryStatsMap->put("user4",   STAFString(logStats.user4));
    queryStatsMap->put("user5",   STAFString(logStats.user5));
    queryStatsMap->put("user6",   STAFString(logStats.user6));
    queryStatsMap->put("user7",   STAFString(logStats.user7));
    queryStatsMap->put("user8",   STAFString(logStats.user8));
}


void registerHelpData(LogServiceData *pData, unsigned int errorNumber,
                      const STAFString &shortInfo, const STAFString &longInfo)
{
    static STAFString regString("REGISTER SERVICE %C ERROR %d INFO %C "
                                "DESCRIPTION %C");

    pData->fHandle->submit(sLocal, sHELP, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber,
        shortInfo.getImpl(), longInfo.getImpl()));
}


void unregisterHelpData(LogServiceData *pData, unsigned int errorNumber)
{
    static STAFString regString("UNREGISTER SERVICE %C ERROR %d");

    pData->fHandle->submit(sLocal, sHELP, STAFHandle::formatString(
        regString.getImpl(), pData->fShortName.getImpl(), errorNumber));
}

