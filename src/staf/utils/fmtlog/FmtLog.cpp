/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFTimestamp.h"
#include "STAFUtil.h"
#include "STAFCommandParser.h"
#include "STAFFileSystem.h"

// XXX: This utility steals ALL of its functionality from STAFLogService.cpp.
//      These structures and functions should be moved to a separate header so
//      that they can be shared by the service and this utility.  It would
//      probably also be a good idea to move this utility into the services/log
//      directory.

// Type definitions

struct LogRecord
{
    LogRecord() : recordFormatID(0), date(0), secondsPastMidnight(0),
                  logLevel(0), handle(0)
    { /* Do Nothing */ }

    LogRecord(unsigned int aDate, unsigned int seconds, unsigned int level,
              const STAFString &aMachine, const STAFString &aHandleName,
              STAFHandle_t aHandle, const STAFString(&aUser),
              const STAFString(&aEndpoint), const STAFString &aMessage)
        : recordFormatID(0), date(aDate), secondsPastMidnight(seconds),
          logLevel(level), machine(aMachine), handleName(aHandleName),
        handle(aHandle), user(aUser), endpoint(aEndpoint), message(aMessage)
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
};

enum ReadLogRecordRC
{
    kReadLogOk = 0,
    kReadLogEndOfFile = 1,
    kReadLogInvalidFormat = 2
};


// Prototypes

void printUsage();
void readUIntFromFile(istream &input, unsigned int &data,
                     unsigned int length = 4);
void readStringFromFile(istream &input, STAFString &inString);
unsigned int readLogRecordFromFile(istream &input, LogRecord &logRecord);
void writeLogRecordToString(STAFString &output, const LogRecord &logRecord,
                            const STAFString &separator,
                            const STAFString &endOfLine,
                            bool levelAsBits = false);
STAFString &convertLogLevelToString(unsigned int logLevel,
                                    bool levelAsBits = false);


// Some global variables

static STAFString sSpace(kUTF8_SPACE);
static STAFString sOldSep(kUTF8_VBAR);
static unsigned int sCurrRecordFormatID = 4;

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
static STAFString sUSER7Pretty("UseR7");
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

static STAFString sSlash(kUTF8_SLASH);
static STAFString sColon(kUTF8_COLON);
static const STAFString sSpecSeparator(sColon + sSlash + sSlash);
static const STAFString sUnauthenticatedUser = "none" +
    sSpecSeparator + "anonymous";

// Begin main

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    STAFString input;

    for (int i = 1; i < argc; ++i)
        input += sSpace + argv[i];

    STAFCommandParser parser;

    parser.addOption("FORMAT", 1, STAFCommandParser::kValueNotAllowed);
    parser.addOption("LOGFILE", 1, STAFCommandParser::kValueRequired);
    parser.addOption("NEWFILE", 1, STAFCommandParser::kValueRequired);
    parser.addOption("LEVELBITSTRING", 1, STAFCommandParser::kValueNotAllowed);
    parser.addOption("FIELDSEP", 1, STAFCommandParser::kValueRequired);
    parser.addOption("HELP", 1, STAFCommandParser::kValueNotAllowed);
    parser.addOptionGroup("FORMAT HELP", 1, 1);
    parser.addOptionNeed("FORMAT", "LOGFILE");
    parser.addOptionNeed("LOGFILE", "NEWFILE");

    STAFCommandParseResultPtr parsedResult = parser.parse(input);

    if (parsedResult->rc != 0)
    {
        cout << parsedResult->errorBuffer << endl;
        return parsedResult->rc;
    }

    if (parsedResult->optionTimes("HELP") != 0)
    {
        printUsage();
        return 1;
    }

    STAFString fieldSep(kUTF8_VBAR);

    if (parsedResult->optionTimes("FIELDSEP") != 0)
        fieldSep = parsedResult->optionValue("FIELDSEP");

    bool levelAsBits = (parsedResult->optionTimes("LEVELBITSTRING") != 0);

    // We use a blank end of line so that we don't have to figure out
    // what the appropriate thing really is.  We will just use "endl"

    STAFString endOfLine;
    STAFString infileName = parsedResult->optionValue("LOGFILE");
    STAFString outfileName = parsedResult->optionValue("NEWFILE");

    fstream infile(infileName.toCurrentCodePage()->buffer(),
                   ios::in | STAF_ios_binary);
    if (!infile)
    {
        cout << "Error opening Log File, " << infileName << endl;
        return 1;
    }

    fstream outfile(outfileName.toCurrentCodePage()->buffer(),
                    ios::out | STAF_ios_binary);
    if (!outfile)
    {
        cout << "Error opening Output File, " << outfileName << endl;
        return 1;
    }

    // Get the log file's entry and lock it

    STAFFSPath logfilePath(infileName);
    STAFFSEntryPtr logfileEntry;

    try
    {
        logfileEntry = logfilePath.getEntry();
    }
    catch (STAFException &se)
    {
        cout << "Error getting entry for Log File, " << infileName << endl;
        cout << "Error code: " << se.getErrorCode() << endl;
        cout << "Reason    : " << se.getText() << endl;
        return 1;
    }

    STAFFSEntryRLock logfileLock(logfileEntry);

    // Read each record

    unsigned int totalRecords = 0;
    LogRecord logRecord;

    while (!infile.eof())
    {
        // First, get the information from the log record

        unsigned int status = readLogRecordFromFile(infile, logRecord);

        if (status == kReadLogEndOfFile)
        {
            // Finish up with whatever needs to be done
            continue;
        }
        else if (status == kReadLogInvalidFormat)
        {
            cout << "Encountered unknown log record format." << endl
                 << "Exiting!" << endl;

            return 1;
        }

        ++totalRecords;

        STAFString recordString;

        writeLogRecordToString(recordString, logRecord, fieldSep, endOfLine,
                               levelAsBits);

        outfile << recordString << endl;
    }

    cout << "Formatted " << totalRecords << " record(s) to " << outfileName
         << endl;

    return 0;
}

void printUsage()
{
    cout << "Usage: FmtLog FORMAT LOGFILE <Log File> NEWFILE <New File>" << endl
         << "              [LEVELBITSTRING] [FIELDSEP <Field Separator>]"
         << endl;
}


void writeLogRecordToString(STAFString &output, const LogRecord &logRecord,
                            const STAFString &sep, const STAFString &endOfLine,
                            bool levelAsBits)
{
    unsigned int year = logRecord.date / 10000;
    unsigned int month = (logRecord.date % 10000) / 100;
    unsigned int day = logRecord.date % 100;
    unsigned int hour = logRecord.secondsPastMidnight / 3600;
    unsigned int minute = (logRecord.secondsPastMidnight % 3600) / 60;
    unsigned int second = logRecord.secondsPastMidnight % 60;
    STAFTimestamp theTimestamp(year, month, day, hour, minute, second);

    output += theTimestamp.asString();
    output += sep;
    output += logRecord.machine;
    output += sep;
    output += STAFString(logRecord.handle);
    output += sep;
    output += logRecord.handleName;
    output += sep;
    output += logRecord.user;
    output += sep;
    output += logRecord.endpoint;
    output += sep;
    output += convertLogLevelToString(logRecord.logLevel, levelAsBits);
    output += sep;
    output += logRecord.message;
    output += endOfLine;
}


unsigned int readLogRecordFromFile(istream &logfile, LogRecord &logRecord)
{
    unsigned int totalLength = 0;

    readUIntFromFile(logfile, logRecord.recordFormatID, 1);

    if (logfile.eof()) return kReadLogEndOfFile;

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
        logRecord.endpoint = "tcp://" + logRecord.machine;
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
        logRecord.handleName =
            dataString.subString(sepLoc2 + 1, sepLoc3 - sepLoc2 - 1);
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
        logRecord.handleName =
            dataString.subString(sepLoc2 + 1, sepLoc3 - sepLoc2 - 1);
        logRecord.message = dataString.subString(sepLoc3 + 5);

        logRecord.logLevel = *(reinterpret_cast<unsigned int *>(
                               const_cast<char *>(dataString.buffer()
                                                  + sepLoc3 + 1)));

        // Fixup log level from Rexx's big-endian to native format

        logRecord.logLevel =
            STAFUtilConvertLEUIntToNative(STAFUtilSwapUInt(logRecord.logLevel));

        logRecord.user = sUnauthenticatedUser;
        logRecord.endpoint = "tcp" + sSpecSeparator + logRecord.machine;
    }
    else
    {
        return kReadLogInvalidFormat;
    }

    return kReadLogOk;
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
