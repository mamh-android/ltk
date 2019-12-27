/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <deque>
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFString.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFException.h"
#include "STAFProcessService.h"
#include "STAFProcess.h"
#include "STAFHandleManager.h"
#include "STAFVariablePool.h"
#include "STAFUtil.h"
#include "STAFThreadManager.h"
#include "STAFTrace.h"
#include "STAFInternalProcess.h"

STAFProcessAuthenticationMode_t
    STAFProcessService::fAuthMode = kSTAFProcessAuthDisabled;
STAFProcessStopMethod_t
    STAFProcessService::fDefaultStopMethod = kSTAFProcessStopWithSigKillAll;
STAFProcessDisabledAuthAction_t
    STAFProcessService::fAuthAction = kSTAFProcessDisabledAuthIgnore;
STAFProcessConsoleFocus_t
    STAFProcessService::fDefaultConsoleFocus = kSTAFProcessBackground;
STAFString STAFProcessService::fDefaultAuthUsername;
STAFString STAFProcessService::fDefaultAuthPassword;
STAFString STAFProcessService::fDefaultShellCommand;
STAFString STAFProcessService::fDefaultSameConsoleShell;
STAFString STAFProcessService::fDefaultNewConsoleShell;
STAFString STAFProcessService::fTempDirectory;
STAFEventSem STAFProcessService::fNotifySem;
STAFProcessService::ProcessPtr STAFProcessService::fNotifyProcess;
unsigned int STAFProcessService::fEnvVarCaseSensitive = 1;
STAFProcessID_t STAFProcessService::fMaxPid;

static STAFString sHelpMsg;
static STAFMutexSem sDefaultStopUsingSem;
static STAFMutexSem sDefaultConsoleModeSem;
static STAFMutexSem sDefaultConsoleFocusSem;
static STAFMutexSem sProcessAuthModeSem;
static STAFMutexSem sDefaultAuthUsernameSem;
static STAFMutexSem sDefaultAuthPasswordSem;
static STAFMutexSem sDefaultAuthDisabledActionSem;
static STAFMutexSem sDefaultShellSem;
static STAFMutexSem sDefaultNewConsoleShellSem;
static STAFMutexSem sDefaultSameConsoleShellSem;
static STAFMutexSem sTempDirectorySem;

static const STAFString sNew = "New";
static const STAFString sSame = "Same";

static const unsigned int sMaxReadAttempts = 10;
static const unsigned int sReadRetryDelay = 500;  // 1/2 second (500ms)

static const STAFString sHOSTNAME_ERROR = "Error resolving host name";

// Note: This is a mild hack to allow the callback routine to work.
// In a perfect world, this class (and all the Service classes) would
// be Singletons and I would create a more generic callback mechanism.

static STAFProcessService *processService = 0;

STAFServiceResult readFileIntoString(const STAFString &file,
                                     unsigned int maxFileSize)
{
    STAFString errMsg = "";
    unsigned fileLength = 0;
    
    try
    {
        fstream inFile(file.toCurrentCodePage()->buffer(),
                       ios::in | STAF_ios_binary);

        if (!inFile) return STAFServiceResult(kSTAFFileOpenError, file);

        // Figure out how big the file is

        inFile.seekg(0, ios::end);
        fileLength = (unsigned int)inFile.tellg();
        
        if (fileLength == 0)
        {
            inFile.close();
            return STAFServiceResult(kSTAFOk, STAFString(""));
        }

        // Determine if the file size exceeds the maximum allowed size
    
        if ((maxFileSize != 0) && (fileLength > maxFileSize))
        {
            inFile.close();

            return STAFServiceResult(
                kSTAFMaximumSizeExceeded,
                STAFString("File '") + file + "' size is " + fileLength +
                " bytes which exceeds " + STAFString(maxFileSize) +
                " bytes, the maximum return file size allowed");
        }

        // Allocate memory for the output buffer

        STAFRefPtr<char> buffer(new char[fileLength], STAFRefPtr<char>::INIT,
                                STAFRefPtr<char>::ARRAY);

        // Read in the entire file into the buffer

        inFile.seekg(0, ios::beg);
        inFile.read(buffer, fileLength);
        
        // Verify if the file read was successful

        if (inFile.good() || (inFile.eof() && (inFile.gcount() == fileLength)))
        {
            inFile.close();
            return STAFServiceResult(kSTAFOk, STAFString(buffer, fileLength));
        }

        // The read failed.  Retry the read up to sMaxReadAttempts times with a
        // delay between each attempt.
        
        int readAttempt = 1;

        for (; !inFile.good() && readAttempt <= sMaxReadAttempts;)
        {
            if (inFile.fail())
            {
                // Recoverable read error

                // Log a warning tracepoint message

                STAFString warningMsg(
                    "STAFProcessService::readFileIntoString(): - "
                    "Read attempt #" + STAFString(readAttempt) +
                    " failed for file " + file + ", fileLength: " +
                    STAFString(fileLength) + " bytes");

                STAFTrace::trace(kSTAFTraceWarning, warningMsg);

                // Delay and retry read after clearing any error flags and
                // repositioning the file pointer

                STAFThreadManager::sleepCurrentThread(sReadRetryDelay);
            
                inFile.clear();
            
                if (readAttempt == sMaxReadAttempts)
                {
                    // Before the last read attempt, try closing the file and
                    // reopening it first to see if that fixes the problem

                    inFile.close();
                    inFile.open(file.toCurrentCodePage()->buffer(),
                                ios::in | STAF_ios_binary);
                }

                readAttempt++;
                inFile.seekg(0, ios::beg);
                inFile.read(buffer, fileLength);
            }
            else if (inFile.bad())
            {
                // Unrecoverable read error.
                break;
            }
        }

        if (!inFile.good())
        {
            errMsg = STAFString("Unrecoverable read error (after attempting "
                                "read ") + STAFString(readAttempt) + " times";
        }
        
        inFile.close();
    }
    catch (STAFException &se)
    {
        errMsg = "RC: " + STAFString(se.getErrorCode()) +
            ", Text: " + se.getText();
    }
    catch (std::bad_alloc)
    {
        errMsg = "Caught bad_alloc exception - Out of memory?";
    }
    catch (...)
    {
        // No additional info about the error is available

        errMsg = "Caught unknown exception";
    }

    return STAFServiceResult(
        kSTAFFileReadError,
        STAFString("STAFProcessService::readFileIntoString(): "
                   "Error reading file: ") + file +
        ", fileLength: " + STAFString(fileLength) + " bytes, " + errMsg);
}


STAFServiceResult deleteTempFile(const STAFString &tempFileName)
{
    STAFRC_t rc = kSTAFBaseOSError;
    unsigned int osRC = 0;
    STAFString errorBuffer = STAFString("");
    unsigned int retryAttempts = 10;

    // Retry to delete the temporary file up to 10 times until successful
    // because sometimes the file is still in use (OSRC=32 on Windows)
    for (unsigned int i = 1; (rc != kSTAFOk) && (i <= retryAttempts); ++i)
    {
        try
        {
            rc = STAFFSPath(tempFileName).getEntry()->remove(&osRC);

            if (rc != kSTAFOk)
            {
                if (i == retryAttempts)
                {
                    errorBuffer =
                        STAFString("STAFProcessService::deleteTempFile()"
                                   " - Failed for " + tempFileName +
                                   " with OSRC " + osRC);

                    STAFTrace::trace(kSTAFTraceWarning, errorBuffer);
                }
                else
                {
                    // Sleep a short time before retrying to delete the file
                    STAFThreadManager::sleepCurrentThread(500);
                }
            }
        }
        catch (STAFException &se)
        {
            // Don't retry deleting the temp file
            rc = kSTAFBaseOSError;
            errorBuffer = STAFString("STAFProcessService::deleteTempFile()"
                                     " - Failed for " + tempFileName +
                                     " Text: " + se.getText());
            STAFTrace::trace(kSTAFTraceWarning, errorBuffer);
        }
        catch (...)
        {
            // Don't retry deleting the temp file
            rc = kSTAFBaseOSError;
            errorBuffer = STAFString("STAFProcessService::deleteTempFile()"
                                     " - Caught unknown exception deleting " +
                                     tempFileName);
            STAFTrace::trace(kSTAFTraceWarning, errorBuffer);
        }
    }

    return STAFServiceResult(rc, errorBuffer);
}

/***********************************************************************/
/* replaceEndpointSystemId - Replaces the system identifier in the     */
/*                           input endpoint with the input system      */
/*                           identifier and returns the new endpoint.  */
/*                           If the input system identifier is the     */
/*                           same as the endpoint's system identifier, */
/*                           then it returns "" for the new endpoint.  */
/* The syntax for an endpoint is:                                      */
/*    [<Interface>://]<System Identifier>[@<Port>]                     */
/* Note the <System Identifier> could be a host name or an IP address  */
/*                                                                     */
/* Accepts: (In)  A STAFString which is an endpoint                    */
/*          (In)  A STAFString which is a system identifier            */
/*                                                                     */
/* Returns: a new endpoint or ""                                       */
/***********************************************************************/

STAFString replaceEndpointSystemId(const STAFString endpoint,
                                   const STAFString systemId)
{
    STAFString theInterface = "";
    STAFString theSystemId = "";
    STAFString thePort = "";
    STAFString newEndpoint = "";

    // Split the endpoint into its interface and system identifier

    unsigned int sepPos = endpoint.find(gSpecSeparator);

    if (sepPos != STAFString::kNPos)
    {
        theInterface = endpoint.subString(0, sepPos + gSpecSeparator.length());
        theSystemId = endpoint.subString(sepPos + gSpecSeparator.length());
    }
    else
    {
        theSystemId = endpoint;
    }
    
    // Remove the port (if present) from the system identifier

    sepPos = theSystemId.find(kUTF8_AT);

    if (sepPos != STAFString::kNPos)
    {
        // If the characters following the "@" are numeric, then assume
        // it's a valid port and strip the @ and the port number from
        // the system indentifier.

        STAFString port = theSystemId.subString(sepPos + 1);
        
        if (port.isDigits())
        {
            thePort = theSystemId.subString(sepPos);
            theSystemId = theSystemId.subString(0, sepPos);
        }
    }
    
    if (!theSystemId.isEqualTo(systemId, kSTAFStringCaseInsensitive))
        newEndpoint = theInterface + systemId + thePort;
    
    return newEndpoint;
}


STAFProcessService::STAFProcessService() : STAFService("PROCESS")
{
    processService = this;

    // Assign the help text string for the service

    sHelpMsg = STAFString("*** PROCESS Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "START [SHELL [<Shell>]] COMMAND <Command> [PARMS <Parms>] [WORKDIR <Directory>]" +
        *gLineSeparatorPtr +
        "      [VAR <Variable>=<Value>]... [ENV <Variable>=<Value>]... [USEPROCESSVARS]" +
        *gLineSeparatorPtr +
        "      [WORKLOAD <Name>] [TITLE <Title>] [WAIT [<Number>[s|m|h|d|w]] | ASYNC]" +
        *gLineSeparatorPtr +
        "      [STOPUSING <Method>] [STATICHANDLENAME <Name>]" +
        *gLineSeparatorPtr +
        "      [NEWCONSOLE | SAMECONSOLE] [FOCUS <Background | Foreground | Minimized>]" +
        *gLineSeparatorPtr +
        "      [USERNAME <User Name> [PASSWORD <Password>]]" +
        *gLineSeparatorPtr +
        "      [DISABLEDAUTHISERROR | IGNOREDISABLEDAUTH] " +
        *gLineSeparatorPtr +
        "      [STDIN <File>] [STDOUT <File> | STDOUTAPPEND <File>]" +
        *gLineSeparatorPtr +
        "      [STDERR <File> | STDERRAPPEND <File> | STDERRTOSTDOUT]" +
        *gLineSeparatorPtr +
        "      [RETURNSTDOUT] [RETURNSTDERR] [RETURNFILE <File>]..." +
        *gLineSeparatorPtr +
        "      [NOTIFY ONEND [HANDLE <Handle> | NAME <Name>]" +
        *gLineSeparatorPtr +
        "      [MACHINE <Machine>] [PRIORITY <Priority>] [KEY <Key>]]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "STOP  <ALL CONFIRM | WORKLOAD <Name> | HANDLE <Handle>> [USING <Method>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "KILL  PID <Pid> CONFIRM [USING <Method>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST  [HANDLES] [RUNNING] [COMPLETED] [WORKLOAD <Name>] [LONG]" +
        *gLineSeparatorPtr +
        "LIST  SETTINGS" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "QUERY HANDLE <Handle>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "FREE  <ALL | WORKLOAD <Name> | HANDLE <Handle>>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "NOTIFY REGISTER   ONENDOFHANDLE <Handle> [HANDLE <Handle> | NAME <Name>]" +
        *gLineSeparatorPtr +
        "                  [MACHINE <Machine>] [PRIORITY <Priority>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "NOTIFY UNREGISTER ONENDOFHANDLE <Handle> [HANDLE <Handle> | NAME <Name>]" +
        *gLineSeparatorPtr +
        "                  [MACHINE <Machine>] [PRIORITY <Priority>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "NOTIFY LIST       ONENDOFHANDLE <Handle>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SET   [DEFAULTSTOPUSING <Method>] [DEFAULTCONSOLE <New | Same>]" +
        *gLineSeparatorPtr +
        "      [DEFAULTFOCUS <Background | Foreground | Minimized>] " +
        *gLineSeparatorPtr +
        "      [PROCESSAUTHMODE <Auth Mode>] " +
        *gLineSeparatorPtr +
        "      [DEFAULTAUTHUSERNAME <User Name>] [DEFAULTAUTHPASSWORD <Password>] " +
        *gLineSeparatorPtr +
        "      [DEFAULTAUTHDISABLEDACTION <Error | Ignore>] [DEFAULTSHELL <Shell>]" +
        *gLineSeparatorPtr +
        "      [DEFAULTNEWCONSOLESHELL <Shell>] [DEFAULTSAMECONSOLESHELL <Shell>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // START options

    fStartParser.addOption("START",             1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("WORKLOAD",          1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("COMMAND",           1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("PARMS",             1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("SHELL",             1,
        STAFCommandParser::kValueAllowed);
    fStartParser.addOption("WORKDIR",           1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("TITLE",             1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("WAIT",              1,
        STAFCommandParser::kValueAllowed);
    fStartParser.addOption("ASYNC",             1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("VAR",               0,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("ENV",               0,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("USEPROCESSVARS",    0,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("NOTIFY",            1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("ONEND",             1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("NAME",              1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("HANDLE",            1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("MACHINE",           1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("PRIORITY",          1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("KEY",          1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STOPUSING",         1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("NEWCONSOLE",        1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("SAMECONSOLE",       1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("FOCUS",             1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("USERNAME",          1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("PASSWORD",          1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STDIN",             1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STDOUT",            1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STDERR",            1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STDOUTAPPEND",      1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STDERRAPPEND",      1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("STDERRTOSTDOUT", 0,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("DISABLEDAUTHISERROR", 0,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("IGNOREDISABLEDAUTH",  0,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("STATICHANDLENAME",  1,
        STAFCommandParser::kValueRequired);
    fStartParser.addOption("RETURNSTDOUT",      1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("RETURNSTDERR",      1,
        STAFCommandParser::kValueNotAllowed);
    fStartParser.addOption("RETURNFILE",        0,
        STAFCommandParser::kValueRequired);

    // START option groups

    fStartParser.addOptionGroup("COMMAND", 1, 1);
    fStartParser.addOptionGroup("WAIT ASYNC",  0, 1);
    fStartParser.addOptionGroup("HANDLE NAME", 0, 1);
    fStartParser.addOptionGroup("NEWCONSOLE SAMECONSOLE", 0, 1);
    fStartParser.addOptionGroup("DISABLEDAUTHISERROR IGNOREDISABLEDAUTH", 0, 1);
    fStartParser.addOptionGroup("STDOUT STDOUTAPPEND", 0, 1);
    fStartParser.addOptionGroup("STDERR STDERRAPPEND STDERRTOSTDOUT", 0, 1);

    // START option needs

    fStartParser.addOptionNeed("NOTIFY", "ONEND");
    fStartParser.addOptionNeed("PASSWORD", "USERNAME");
    fStartParser.addOptionNeed("NAME HANDLE MACHINE PRIORITY KEY", "NOTIFY");
    fStartParser.addOptionNeed("STDERRTOSTDOUT", "STDOUT STDOUTAPPEND RETURNSTDOUT");

    // QUERY options

    fQueryParser.addOption("QUERY",    1,
        STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption("FREE",    1,
        STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption("WORKLOAD", 1,
        STAFCommandParser::kValueRequired);
    fQueryParser.addOption("HANDLE",   1,
        STAFCommandParser::kValueRequired);

    // QUERY groups

    fQueryParser.addOptionGroup("HANDLE", 1, 1);

    // FREE options

    fFreeParser.addOption("FREE",    1,
        STAFCommandParser::kValueNotAllowed);
    fFreeParser.addOption("WORKLOAD", 1,
        STAFCommandParser::kValueRequired);
    fFreeParser.addOption("HANDLE",   1,
        STAFCommandParser::kValueRequired);
    fFreeParser.addOption("ALL", 1,
        STAFCommandParser::kValueNotAllowed);

    // FREE groups

    fFreeParser.addOptionGroup("HANDLE WORKLOAD ALL", 1, 1);

    // STOP options

    fStopParser.addOption("STOP",     1,
        STAFCommandParser::kValueNotAllowed);
    fStopParser.addOption("WORKLOAD", 1,
        STAFCommandParser::kValueRequired);
    fStopParser.addOption("HANDLE",   1,
        STAFCommandParser::kValueRequired);
    fStopParser.addOption("ALL",      1,
        STAFCommandParser::kValueNotAllowed);
    fStopParser.addOption("CONFIRM",  1,
        STAFCommandParser::kValueNotAllowed);
    fStopParser.addOption("USING",    1,
        STAFCommandParser::kValueRequired);

    // STOP needs

    fStopParser.addOptionNeed("ALL", "CONFIRM");
    fStopParser.addOptionNeed("CONFIRM", "ALL");
    
    // STOP groups

    fStopParser.addOptionGroup("HANDLE WORKLOAD ALL", 1, 1);

    // KILL options

    fKillParser.addOption(
        "KILL", 1, STAFCommandParser::kValueNotAllowed);
    fKillParser.addOption(
        "PID",  1, STAFCommandParser::kValueRequired);
    fKillParser.addOption(
        "CONFIRM", 1, STAFCommandParser::kValueNotAllowed);
    fKillParser.addOption("USING",    1,
        STAFCommandParser::kValueRequired);

    // KILL needs

    fKillParser.addOptionGroup("PID", 1, 1);
    fKillParser.addOptionGroup("CONFIRM", 1, 1);

    // NOTIFY registration options

    fNotifyRegistrationParser.addOption("NOTIFY",        1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyRegistrationParser.addOption("REGISTER",      1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyRegistrationParser.addOption("UNREGISTER",    1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyRegistrationParser.addOption("ONENDOFHANDLE", 1,
        STAFCommandParser::kValueRequired);
    fNotifyRegistrationParser.addOption("NAME",          1,
        STAFCommandParser::kValueRequired);
    fNotifyRegistrationParser.addOption("HANDLE",        1,
        STAFCommandParser::kValueRequired);
    fNotifyRegistrationParser.addOption("MACHINE",       1,
        STAFCommandParser::kValueRequired);
    fNotifyRegistrationParser.addOption("PRIORITY",      1,
        STAFCommandParser::kValueRequired);

    // NOTIFY registration groups

    fNotifyRegistrationParser.addOptionGroup("ONENDOFHANDLE", 1, 1);
    fNotifyRegistrationParser.addOptionGroup("HANDLE NAME",   0, 1);

    // NOTIFY LIST options

    fNotifyListParser.addOption("NOTIFY",        1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyListParser.addOption("LIST",          1,
        STAFCommandParser::kValueNotAllowed);
    fNotifyListParser.addOption("ONENDOFHANDLE", 1,
        STAFCommandParser::kValueRequired);

    // NOTIFY LIST groups

    fNotifyListParser.addOptionGroup("ONENDOFHANDLE", 1, 1);

    // LIST options
    fListParser.addOption("LIST",       1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("HANDLES",       1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("WORKLOAD",   1,
        STAFCommandParser::kValueRequired);
    fListParser.addOption("COMPLETED", 1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("RUNNING", 1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("LONG", 1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SETTINGS", 1,
        STAFCommandParser::kValueNotAllowed);

    // LIST groups
    fListParser.addOptionGroup("WORKLOAD", 0, 1);

    // You can list the operational settings for the Process service OR
    // you can list processes
    fListParser.addOptionGroup("SETTINGS HANDLES", 0, 1);
    fListParser.addOptionGroup("SETTINGS WORKLOAD", 0, 1);
    fListParser.addOptionGroup("SETTINGS COMPLETED", 0, 1);
    fListParser.addOptionGroup("SETTINGS RUNNING", 0, 1);
    fListParser.addOptionGroup("SETTINGS LONG", 0, 1);

    // set options

    fSetParser.addOption("SET", 1,
                         STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("DEFAULTSTOPUSING", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTCONSOLE", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTFOCUS", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("PROCESSAUTHMODE", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHUSERNAME", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHPASSWORD", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTAUTHDISABLEDACTION", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTSHELL", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTNEWCONSOLESHELL", 1,
                         STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULTSAMECONSOLESHELL", 1,
                         STAFCommandParser::kValueRequired);

    // Construct map class for process completion information

    fCompletionMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/CompletionInfo");

    fCompletionMapClass->addKey("rc",       "Return Code");
    fCompletionMapClass->addKey("key",      "Key");
    fCompletionMapClass->addKey("fileList", "Files");
    
    // Construct map class for returned file information

    fReturnFileMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/ReturnFileInfo");

    fReturnFileMapClass->addKey("rc",   "Return Code");
    fReturnFileMapClass->addKey("data", "Data");

    // Construct map class for querying process information

    fProcessInfoMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/ProcessInfo");

    fProcessInfoMapClass->addKey("handle",         "Handle");
    fProcessInfoMapClass->addKey("handleName",     "Handle Name");
    fProcessInfoMapClass->addKey("title",          "Title");
    fProcessInfoMapClass->addKey("workload",       "Workload");
    fProcessInfoMapClass->addKey("shell",          "Shell");
    fProcessInfoMapClass->addKey("command",        "Command");
    fProcessInfoMapClass->addKey("parms",          "Parms");
    fProcessInfoMapClass->addKey("workdir",        "Workdir");
    fProcessInfoMapClass->addKey("focus",          "Focus");
    fProcessInfoMapClass->addKey("userName",       "User Name");
    fProcessInfoMapClass->addKey("key",            "Key");
    fProcessInfoMapClass->addKey("pid",            "PID");
    fProcessInfoMapClass->addKey("startMode",      "Start Mode");
    fProcessInfoMapClass->addKey("startTimestamp", "Start Date-Time");
    fProcessInfoMapClass->addKey("endTimestamp",   "End Date-Time");
    fProcessInfoMapClass->addKey("rc",             "Return Code");

    // Construct map class for listing processes information

    fListProcessMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/ProcessListInfo");

    fListProcessMapClass->addKey("handle", "Handle");
    fListProcessMapClass->setKeyProperty(
        "handle", "display-short-name", "H#");
    fListProcessMapClass->addKey("command", "Command");
    fListProcessMapClass->addKey("startTimestamp", "Start Date-Time");
    fListProcessMapClass->setKeyProperty(
        "startTimestamp", "display-short-name", "Start D-T");
    fListProcessMapClass->addKey("endTimestamp", "End Date-Time");
    fListProcessMapClass->setKeyProperty(
        "endTimestamp", "display-short-name", "End D-T");
    fListProcessMapClass->addKey("rc", "Return Code");
    fListProcessMapClass->setKeyProperty(
        "rc", "display-short-name", "RC");

    // Construct map class for listing processes information

    fListLongProcessMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/ProcessListLong");

    fListLongProcessMapClass->addKey("handle", "Handle");
    fListLongProcessMapClass->setKeyProperty(
        "handle", "display-short-name", "H#");
    fListLongProcessMapClass->addKey("workload", "Workload");
    fListLongProcessMapClass->addKey("command", "Command");
    fListLongProcessMapClass->addKey("pid", "PID");
    fListLongProcessMapClass->addKey("startTimestamp", "Start Date-Time");
    fListLongProcessMapClass->setKeyProperty(
        "startTimestamp", "display-short-name", "Start D-T");
    fListLongProcessMapClass->addKey("endTimestamp", "End Date-Time");
    fListLongProcessMapClass->setKeyProperty(
        "endTimestamp", "display-short-name", "End D-T");
    fListLongProcessMapClass->addKey("rc", "Return Code");
    fListLongProcessMapClass->setKeyProperty(
        "rc", "display-short-name", "RC");

    // Construct map class for listing operational settings information

    fSettingsMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/Settings");

    fSettingsMapClass->addKey("defaultStopUsing", "Default Stop Using Method");
    fSettingsMapClass->addKey("defaultConsoleMode", "Default Console Mode");
    fSettingsMapClass->addKey("defaultFocus", "Default Focus");
    fSettingsMapClass->addKey("processAuthMode", "Process Auth Mode");
    fSettingsMapClass->addKey("defaultAuthUsername", "Default Auth Username");
    fSettingsMapClass->addKey("defaultAuthPassword", "Default Auth Password");
    fSettingsMapClass->addKey("defaultAuthDisabledAction", 
                              "Default Auth Disabled Action");
    fSettingsMapClass->addKey("defaultShell", "Default Shell");
    fSettingsMapClass->addKey("defaultNewConsoleShell",
                              "Default New Console Shell");
    fSettingsMapClass->addKey("defaultSameConsoleShell",
                              "Default Same Console Shell");

    // Construct map class for freed process information

    fFreeMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/FreeInfo");

    fFreeMapClass->addKey("freedProcesses", "Freed Processes");
    fFreeMapClass->addKey("totalProcesses", "Total Processes");
    
    // Construct map class for stopped process information

    fStopMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/StopInfo");

    fStopMapClass->addKey("stoppedProcesses", "Stopped Processes");
    fStopMapClass->addKey("totalProcesses",   "Total Processes");

    // Construct map class for notify list information

    fNotifieeMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Process/Notifiee");

    fNotifieeMapClass->addKey("priority",   "Priority");
    fNotifieeMapClass->setKeyProperty("priority", "display-short-name", "P");
    fNotifieeMapClass->addKey("machine",    "Machine");
    fNotifieeMapClass->addKey("notifyBy",   "Notify By");
    fNotifieeMapClass->addKey("notifiee",   "Notifiee");

    // Set the default stop method, the environment variable case
    // sensitive flag, and the maximum pid

    STAFConfigInfo configInfo;
    STAFString_t errorBufferT = 0;
    unsigned int osRC = 0;

    if (STAFUtilGetConfigInfo(&configInfo, &errorBufferT, &osRC) != kSTAFOk)
    {
        STAFString errorMsg = STAFString(errorBufferT, STAFString::kShallow) +
            ", RC: " + STAFString(osRC);
        STAFTrace::trace(
            kSTAFTraceError, errorMsg.toCurrentCodePage()->buffer());
    }
    else
    {
        STAFProcessService::fDefaultStopMethod =
            configInfo.defaultProcessStopMethod;
        STAFProcessService::fEnvVarCaseSensitive =
            configInfo.envVarCaseSensitive;
        STAFProcessService::fMaxPid = configInfo.maxPid;
    }

    // Set the temporary directory used for storing temporary stdout/stderr
    // files when starting a process if RETURNSTDOUT/RETURNSTDERR option is
    // specified, but not a STDOUT/STDERR option
    STAFProcessService::setTempDirectory(
        *gSTAFWriteLocationPtr + *gFileSeparatorPtr + "tmp");
}


STAFProcessService::~STAFProcessService()
{
    /* Do Nothing */
}


STAFString STAFProcessService::info(unsigned int) const
{
    return name() + ": Internal";
}


void STAFProcessService::processTerminationCallback(
         STAFProcessID_t pid, STAFProcessHandle_t procHandle,
         unsigned int retCode, void *)
{
    processService->handleProcessTermination(pid, retCode);
}


unsigned int STAFProcessService::sendNotificationCallback(void *)
{
    try
    {
        processService->sendNotification();
    }
    catch (STAFException &se)
    {
        se.trace("STAFProcessService::sendNotificationCallback()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in "
            "STAFProcessService::sendNotificationCallback()");
    }

    return 0;
}


STAFString STAFProcessService::getTempDirectory()
{
    return fTempDirectory;
}

STAFRC_t STAFProcessService::setTempDirectory(const STAFString &tempDirectory)
{
    // Get exclusive access to fTempDirectory
    STAFMutexSemLock lock(sTempDirectorySem);

    fTempDirectory = tempDirectory;

    return kSTAFOk;
}


STAFRC_t STAFProcessService::getStopMethodFromString(
    STAFProcessStopMethod_t &stopMethod,
    const STAFString &methodString)
{
    static STAFString sSigKill("SIGKILL");
    static STAFString sSigKillAll("SIGKILLALL");
    static STAFString sSigTerm("SIGTERM");
    static STAFString sSigTermAll("SIGTERMALL");
    static STAFString sSigInt("SIGINT");
    static STAFString sSigIntAll("SIGINTALL");
    static STAFString sWM_CLOSE("WM_CLOSE");
    STAFString upperMode = methodString.toUpperCase();

    if (upperMode == sSigKill)
        stopMethod = kSTAFProcessStopWithSigKill;
    else if (upperMode == sSigKillAll)
        stopMethod = kSTAFProcessStopWithSigKillAll;
    else if (upperMode == sSigTerm)
        stopMethod = kSTAFProcessStopWithSigTerm;
    else if (upperMode == sSigTermAll)
        stopMethod = kSTAFProcessStopWithSigTermAll;
    else if (upperMode == sSigInt)
        stopMethod = kSTAFProcessStopWithSigInt;
    else if (upperMode == sSigIntAll)
        stopMethod = kSTAFProcessStopWithSigIntAll;
    else if (upperMode == sWM_CLOSE)
        stopMethod = kSTAFProcessStopWithWM_CLOSE;
    else return kSTAFInvalidValue;

    return kSTAFOk;
}


STAFRC_t STAFProcessService::setDefaultStopMethod(
    STAFProcessStopMethod_t stopMethod)
{
    if (STAFProcess::isValidStopMethod(stopMethod) != kSTAFOk)
        return kSTAFInvalidValue;

    // Get exclusive access to fDefaultStopMethod
    STAFMutexSemLock lock(sDefaultStopUsingSem);
    fDefaultStopMethod = stopMethod;

    return kSTAFOk;
}


STAFProcessStopMethod_t STAFProcessService::getDefaultStopMethod()
{
    return fDefaultStopMethod;
}


STAFString STAFProcessService::getDefaultStopMethodAsString()
{
    static STAFString sSigKill("SigKill");
    static STAFString sSigKillAll("SigKillAll");
    static STAFString sSigTerm("SigTerm");
    static STAFString sSigTermAll("SigTermAll");
    static STAFString sSigInt("SigInt");
    static STAFString sSigIntAll("SigIntAll");
    static STAFString sWM_CLOSE("WM_Close");
    STAFString stopMethodString;
    
    if (fDefaultStopMethod == kSTAFProcessStopWithSigKill)
        return sSigKill;
    else if (fDefaultStopMethod == kSTAFProcessStopWithSigKillAll)
        return sSigKillAll;
    else if (fDefaultStopMethod == kSTAFProcessStopWithSigTerm)
        return sSigTerm;
    else if (fDefaultStopMethod == kSTAFProcessStopWithSigTermAll)
        return sSigTermAll;
    else if (fDefaultStopMethod == kSTAFProcessStopWithSigInt)
        return sSigInt;
    else if (fDefaultStopMethod == kSTAFProcessStopWithSigIntAll)
        return sSigIntAll;
    else // fDefaultStopMethod == kSTAFProcessStopWithWM_CLOSE
        return sWM_CLOSE;
}


STAFRC_t STAFProcessService::getConsoleFocusFromString(
    STAFProcessConsoleFocus_t &consoleFocus,
    const STAFString &focusString)
{
    static STAFString sBackground("BACKGROUND");
    static STAFString sForeground("FOREGROUND");
    static STAFString sMinimized("MINIMIZED");
    STAFString upperFocus = focusString.toUpperCase();

    if (upperFocus == sBackground)
        consoleFocus = kSTAFProcessBackground;
    else if (upperFocus == sForeground)
        consoleFocus = kSTAFProcessForeground;
    else if (upperFocus == sMinimized)
        consoleFocus = kSTAFProcessMinimized;
    else return kSTAFInvalidValue;

    return kSTAFOk;
}


STAFRC_t STAFProcessService::setDefaultConsoleFocus(
    STAFProcessConsoleFocus_t consoleFocus)
{
    // Get exclusive access to fDefaultConsoleFocus
    STAFMutexSemLock lock(sDefaultConsoleFocusSem);
    fDefaultConsoleFocus = consoleFocus;

    return kSTAFOk;
}


STAFProcessConsoleFocus_t STAFProcessService::getDefaultConsoleFocus()
{
    return fDefaultConsoleFocus;
}


STAFString STAFProcessService::getDefaultConsoleFocusAsString()
{
    return getConsoleFocusAsString(fDefaultConsoleFocus);
}


STAFString STAFProcessService::getConsoleFocusAsString(
    STAFProcessConsoleFocus_t consoleFocus)
{
    static STAFString sBackground("Background");
    static STAFString sForeground("Foreground");
    static STAFString sMinimized("Minimized");
    
    if (consoleFocus == kSTAFProcessBackground)
        return sBackground;
    else if (consoleFocus == kSTAFProcessForeground)
        return sForeground;
    else // consoleFocus == kSTAFProcessMinimized
        return sMinimized;
}


STAFRC_t STAFProcessService::getAuthModeFromString(
    STAFProcessAuthenticationMode_t &authMode, const STAFString &modeString)
{
    static STAFString sDisabled("DISABLED");
    static STAFString sNone("NONE");
    static STAFString sWindows("WINDOWS");
    STAFString upperMode = modeString.toUpperCase();

    if      (upperMode == sDisabled) authMode = kSTAFProcessAuthDisabled;
    else if (upperMode == sNone)     authMode = kSTAFProcessAuthNone;
    else if (upperMode == sWindows)  authMode = kSTAFProcessAuthWindows;
    else return kSTAFInvalidValue;

    return kSTAFOk;
}


STAFRC_t STAFProcessService::setAuthMode(
    STAFProcessAuthenticationMode_t authMode, unsigned int &osRC)
{
    if (STAFProcess::isValidAuthMode(authMode) != kSTAFOk)
        return kSTAFInvalidValue;

    // Get exclusive access to fAuthMode
    STAFMutexSemLock lock(sProcessAuthModeSem);
    fAuthMode = authMode;

    return kSTAFOk;
}


STAFProcessAuthenticationMode_t STAFProcessService::getAuthMode()
{
    return fAuthMode;
}


STAFString STAFProcessService::getAuthModeAsString()
{
    static STAFString sDisabled("Disabled");
    static STAFString sNone("None");
    static STAFString sWindows("Windows");

    if (fAuthMode == kSTAFProcessAuthDisabled)
        return sDisabled;
    else if (fAuthMode == kSTAFProcessAuthNone)
        return sNone;
    else // fAuthMode == kSTAFProcesAuthWindows
        return sWindows;
}


STAFRC_t STAFProcessService::getDefaultDisabledAuthActionFromString(
    STAFProcessDisabledAuthAction_t &authAction, const STAFString &actionString)
{
    static STAFString sIgnore("IGNORE");
    static STAFString sError("ERROR");

    STAFString upperAction = actionString.toUpperCase();

    if (upperAction == sIgnore) authAction = kSTAFProcessDisabledAuthIgnore;
    else if (upperAction == sError) authAction = kSTAFProcessDisabledAuthError;
    else return kSTAFInvalidValue;

    return kSTAFOk;
}


STAFRC_t STAFProcessService::setDefaultDisabledAuthAction(
    STAFProcessDisabledAuthAction_t authAction)
{
    // Get exclusive access to fAuthAction
    STAFMutexSemLock lock(sDefaultAuthDisabledActionSem);

    fAuthAction = authAction;
    return kSTAFOk;
}


STAFProcessDisabledAuthAction_t
STAFProcessService::getDefaultDisabledAuthAction()
{
    return fAuthAction;
}


STAFString STAFProcessService::getDefaultDisabledAuthActionAsString()
{
    static STAFString sIgnore("Ignore");
    static STAFString sError("Error");

    if (fAuthAction == kSTAFProcessDisabledAuthIgnore)
        return sIgnore;
    else // fAuthAction == kSTAFProcessDisabledAuthError
        return sError;
}


STAFRC_t STAFProcessService::setDefaultAuthUsername(const STAFString &username)
{
    // Get exclusive access to fDefaultAuthUsername
    STAFMutexSemLock lock(sDefaultAuthUsernameSem);
    fDefaultAuthUsername = username;
    return kSTAFOk;
}


STAFString STAFProcessService::getDefaultAuthUsername()
{
    return fDefaultAuthUsername;
}


STAFRC_t STAFProcessService::setDefaultAuthPassword(const STAFString &password)
{
    // Get exclusive access to fDefaultAuthPassword
    STAFMutexSemLock lock(sDefaultAuthPasswordSem);
    fDefaultAuthPassword = password;
    return kSTAFOk;
}


STAFString STAFProcessService::getDefaultAuthPassword()
{
    return fDefaultAuthPassword;
}


STAFRC_t isShellCommandValid(const STAFString &shellCommand)
{
    return STAFProcessValidateShellSubstitutionChars(shellCommand);
}


STAFRC_t STAFProcessService::setDefaultShellCommand(
    const STAFString &shellCommand)
{
    if ((shellCommand.length() != 0) && 
        (isShellCommandValid(shellCommand) != kSTAFOk))
    {
        return kSTAFInvalidValue;
    }

    // Get exclusive access to fDefaultShellCommand
    STAFMutexSemLock lock(sDefaultShellSem);
    fDefaultShellCommand = shellCommand;

    return kSTAFOk;
}


STAFString STAFProcessService::getDefaultShellCommand()
{
    return fDefaultShellCommand;
}


STAFRC_t STAFProcessService::setDefaultSameConsoleShell(
    const STAFString &shellCommand)
{
    if ((shellCommand.length() != 0) && 
        (isShellCommandValid(shellCommand) != kSTAFOk))
    {
        return kSTAFInvalidValue;
    }

    // Get exclusive access to fDefaultSameConsoleShell
    STAFMutexSemLock lock(sDefaultSameConsoleShellSem);
    fDefaultSameConsoleShell = shellCommand;

    return kSTAFOk;
}


STAFString STAFProcessService::getDefaultSameConsoleShell()
{
    return fDefaultSameConsoleShell;
}


STAFRC_t STAFProcessService::setDefaultNewConsoleShell(
    const STAFString &shellCommand)
{
    if ((shellCommand.length() != 0) && 
        (isShellCommandValid(shellCommand) != kSTAFOk))
    {
        return kSTAFInvalidValue;
    }

    // Get exclusive access to fDefaultNewConsoleShell
    STAFMutexSemLock lock(sDefaultNewConsoleShellSem);
    fDefaultNewConsoleShell = shellCommand;

    return kSTAFOk;
}


STAFString STAFProcessService::getDefaultNewConsoleShell()
{
    return fDefaultNewConsoleShell;
}


void STAFProcessService::handleProcessTermination(STAFProcessID_t pid,
                                                  unsigned int retCode)
{
    try
    {
        ProcessList::iterator iter;

        STAFMutexSemLock processLock(fProcessListSem);

        for (iter = fProcessList.begin(); (iter != fProcessList.end()) &&
             (iter->second->pid != pid) ||
             (iter->second->state != kRunning); iter++)
        { /* Do Nothing */ }

        if (iter == fProcessList.end())
        {
            STAFTrace::trace(kSTAFTraceError, "PID was not in the list in "
                             "handleProcessTermination()");
            return;
        }

        ProcessPtr process = iter->second;

        process->RC = retCode;
        process->endStamp = STAFTimestamp::now();
        process->state = kComplete;

        gHandleManagerPtr->unRegister(process->handle, process->pid);

        if (process->notify) process->notify->post();

        // Send out the notification list

        fNotifySem.reset();
        fNotifyProcess = process;

        unsigned int dispatchRC = gThreadManagerPtr->dispatch(
            sendNotificationCallback, 0);

        if (dispatchRC != 0)
        {
            // XXX: Could potentially add a retry in the future (e.g. put
            //      on a queue and retry later)

            STAFTrace::trace(
                kSTAFTraceError,
                "In STAFProcessService::handleProcessTermination(): "
                "Error dispatching a thread, RC: " + dispatchRC);
            return;
        }

        fNotifySem.wait();
        fNotifyProcess = ProcessPtr();

    }
    catch (STAFException &se)
    {
        se.trace("STAFProcessService::handleProcessTermination()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in "
            "STAFProcessService::handleProcessTermination()");
    }
}

void STAFProcessService::sendNotification()
{
    ProcessPtr process(fNotifyProcess);

    fNotifySem.post();

    STAFString type("STAF/Process/End");

    // Create the message to be sent.  The message is a marshalled map
    // containing the process completion information.

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    STAFObjectPtr messageMap = STAFObject::createMap();
    messageMap->put("handle", STAFString(process->handle));
    messageMap->put("endTimestamp", process->endStamp.asString());
    messageMap->put("rc", STAFString(process->RC));
    messageMap->put("key", process->key);

    STAFObjectPtr fileList = STAFObject::createList();

    if (process->retFileList.size() != 0)
    {
        for (STAFProcessService::Process::FileList::iterator iter =
                 process->retFileList.begin();
             iter != process->retFileList.end();
             ++iter)
        {
            STAFServiceResult result = readFileIntoString(
                *iter, process->maxReturnFileSize);

            STAFObjectPtr retFileMap = STAFObject::createMap();
            retFileMap->put("rc", STAFString(result.fRC));
            retFileMap->put("data", STAFString(result.fResult));

            fileList->append(retFileMap);
        }
    }

    messageMap->put("fileList", fileList);

    mc->setRootObject(messageMap);

    // Create a string to assign to the message by marshalling the
    // process completion map

    STAFString message;

    try
    {
        message = mc->marshall();
    }
    catch (...)
    {
        // Catch an unexpected errors, such a memory allocation error.
        // If the maximum file size previously specified is > 1M and
        // the returned file data exceeds 1M, return an error for the
        // file rc instead of returning the large file data string
        // and retry marshalling the process completion data.

        bool retryMarshall = false;

        if ((process->retFileList.size() != 0) &&
            ((process->maxReturnFileSize == 0) ||  // No maximum file size
             (process->maxReturnFileSize > 1048576)))
        {
            fileList = STAFObject::createList();

            for (STAFProcessService::Process::FileList::iterator iter =
                     process->retFileList.begin();
                 iter != process->retFileList.end(); ++iter)
            {
                STAFServiceResult result = readFileIntoString(
                    *iter, 1048576);

                if (result.fRC == kSTAFMaximumSizeExceeded)
                {
                    retryMarshall = true;
                    result.fResult = result.fResult +
                        "  Note: An error occurred marshalling the large "
                        "file data, so reducde the maximum return file "
                        "size to 1M.";
                }

                STAFObjectPtr retFileMap = STAFObject::createMap();
                retFileMap->put("rc", STAFString(result.fRC));
                retFileMap->put("data", STAFString(result.fResult));

                fileList->append(retFileMap);
            }
        }

        if (retryMarshall)
        {
            // Retry marshalling using the new file list we just created

            messageMap->put("fileList", fileList);
            mc->setRootObject(messageMap);
            message = mc->marshall();
        }
        else
        {
            // Delete the temporary StdOut/StdErr files if used and available for
            // deletion.
            deleteTempFiles(*process);

            throw;
        }
    }

    process->notificationList->sendNotification(type, message);

    // Delete the temporary StdOut/StdErr files if used and available for
    // deletion.
    deleteTempFiles(*process);
}


STAFServiceResult STAFProcessService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    // select word 1 as the action and the rest of the request as rest

    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();

    if      (action == "start") return handleStart(requestInfo);
    else if (action == "stop")  return handleStop(requestInfo);
    else if (action == "query") return handleQuery(requestInfo);
    else if (action == "free")  return handleFree(requestInfo);
    else if (action == "list") return handleList(requestInfo);
    else if (action == "notify")
    {
        STAFString subAction = requestInfo.fRequest.subWord(1, 1).lowerCase();

        if (subAction == "register")
            return handleNotifyRegistration(1, requestInfo);
        else if (subAction == "unregister")
            return handleNotifyRegistration(0, requestInfo);
        else if (subAction == "list")
            return handleNotifyList(requestInfo);
        else
        {
            STAFString errMsg;

            if (subAction.length() == 0)
            {
                errMsg = "You must have at least 1, but no more than 1 of "
                    "the option(s), REGISTER UNREGISTER LIST";
            }
            else
            {
                errMsg = STAFString("'") +
                    requestInfo.fRequest.subWord(0, 1) + " " +
                    requestInfo.fRequest.subWord(1, 1) +
                    "' is not a valid command request for the " + name() +
                    " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
                    sHelpMsg;
            }

            return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
        }
    }
    else if (action == "set") return handleSet(requestInfo);
    else if (action == "kill")  return handleKill(requestInfo);
    else if (action == "help") return handleHelp(requestInfo);
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


STAFServiceResult STAFProcessService::handleStart(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "START");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fStartParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFString errorBuffer;

    ProcessSync startMode = kAsync;

    if (parsedResult->optionTimes("WAIT") != 0)
    {
        startMode = kWait;
    }

    STAFProcessStartInfoLevel1 startData = { 0 };
    ProcessPtr process(new Process(), ProcessPtr::INIT);
    unsigned int doNotify = parsedResult->optionTimes("NOTIFY");
    unsigned int useName = parsedResult->optionTimes("NAME");
    unsigned int priority = 5;
    STAFHandle_t theHandle = requestInfo.fHandle;
    STAFString machine = requestInfo.fEndpoint;

    STAFString name;
    STAFString password = "";
    STAFString stdInp = "";
    STAFString stdOut = "";
    STAFString stdErr = "";
    STAFString stopMethodString;
    STAFString focusString = "";
    unsigned int timeout = 0;
    STAFString staticHandleName;

    if (parsedResult->optionTimes("STATICHANDLENAME") != 0)
        process->handleType = kStatic;
    else
        process->handleType = kPending;

    if (parsedResult->optionTimes("SHELL") != 0)
        process->processType = kShell;
    else
        process->processType = kCommand;

    process->authenticator = requestInfo.fAuthenticator;
    process->userIdentifier = requestInfo.fUserIdentifier;

    // Set up variable pool

    STAFVariablePoolPtr varPool(new STAFVariablePool,
                                STAFVariablePoolPtr::INIT);

    if (parsedResult->optionTimes("VAR") != 0)
    {
        for (int i = 1; i <= parsedResult->optionTimes("VAR"); ++i)
        {
            STAFString nameAndValue = parsedResult->optionValue("VAR", i);
            unsigned int equalPos = nameAndValue.find(kUTF8_EQUAL);

            if (equalPos == STAFString::kNPos)
            {
                return STAFServiceResult(
                    kSTAFInvalidValue,
                    STAFString("The value for a VAR option must have format"
                               " Variable=Value.  Invalid value: ") +
                    nameAndValue);
            }

            varPool->set(nameAndValue.subString(0, equalPos),
                         nameAndValue.subString(equalPos +
                         nameAndValue.sizeOfChar(equalPos)));
        }
    }

    // Create a STAF handle variable for this PROCESS START request that
    // contains the endpoint of the originating machine
    varPool->set("STAF/Service/Process/OrgEndpoint", requestInfo.fEndpoint);

    // Set up the master variable pool list and size
    //
    // Note: This is normally handled by a macro, but we need to handle it
    //       specially here due to the use of the USEPROCESSVARS pool

    const STAFVariablePool *masterVarPoolList[] =
    {
        varPool,
        requestInfo.fRequestVarPool,
        requestInfo.fSourceSharedVarPool,
        requestInfo.fLocalSharedVarPool,
        requestInfo.fLocalSystemVarPool
    };

    const STAFVariablePool **varPoolList = masterVarPoolList;
    unsigned int varPoolListSize =
                 sizeof(masterVarPoolList) / sizeof(const STAFVariablePool *);

    // If USEPROCESSVARS isn't set, then adjust the varPoolList[Size] being used

    if (parsedResult->optionTimes("USEPROCESSVARS") == 0)
    {
        varPoolList = &masterVarPoolList[1];
        --varPoolListSize;
    }

    // Resolve the rest of the options

    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "WORKLOAD", process->workload);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "SHELL", process->shellCommand);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "COMMAND", process->command);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "WORKDIR", process->workdir);
    
    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "PARMS", process->parms);
    
    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "TITLE", process->title);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_UINT_OPTION("PRIORITY", priority);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "KEY", process->key);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
        "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS("NAME", name);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS("MACHINE", machine);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_DEFAULT_DURATION_OPTION(
        "WAIT", timeout, STAF_EVENT_SEM_INDEFINITE_WAIT);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "STOPUSING", stopMethodString);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS("FOCUS", focusString);
    
    if (rc) return STAFServiceResult(rc, errorBuffer);

    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "USERNAME", process->username);

    if (rc) return STAFServiceResult(rc, errorBuffer);
        
    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
            "PASSWORD", password);
    
    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (parsedResult->optionTimes("STDIN") != 0)
    {
        startData.stdinMode = kSTAFProcessIOReadFile;
        rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
            "STDIN", stdInp);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }

    if (parsedResult->optionTimes("STDOUT") != 0)
    {
        startData.stdoutMode = kSTAFProcessIOReplaceFile;
        rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
            "STDOUT", stdOut);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }
    else if (parsedResult->optionTimes("STDOUTAPPEND") != 0)
    {
        startData.stdoutMode = kSTAFProcessIOAppendFile;
        rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
            "STDOUTAPPEND", stdOut);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }

    if (stdOut.length() != 0)
    {
        STAFFSPath stdoutPath(stdOut);
        
        // Assign the parent directory path for the Stdout file
        
        STAFFSPath thePath;
        thePath.setRoot(stdoutPath.root());
        
        for (unsigned int i = 0; i < stdoutPath.numDirs(); ++i)
        {
            thePath.addDir(stdoutPath.dir(i));
        }

        // Create the parent directory path for the Stdout file if it
        // doesn't already exist

        try
        {
            if (!thePath.exists())
            {
                unsigned int osRC = 0;
                STAFString dirToCreate = thePath.asString();
                rc = STAFFSCreateDirectory(
                    dirToCreate.getImpl(), kSTAFFSCreatePath, &osRC);

                if ((rc != kSTAFOk) && (rc != kSTAFAlreadyExists))
                {
                    STAFString result = STAFString(
                        "Could not create STDOUT directory (") +
                        dirToCreate + "), OS RC: " + STAFString(osRC);
                    return STAFServiceResult(rc, result);
                }
            }
        }
        catch (STAFException &se)
        {
            STAFString result = STAFString(
                "Error checking if the STDOUT directory exists (") +
                thePath.asString() + ") " + se.getText();

            return STAFServiceResult(se.getErrorCode(), result);
        }
        catch (...)
        {
            return STAFServiceResult(
                kSTAFBaseOSError,
                STAFString("Unknown error checking if the STDOUT directory "
                           " exists (") + thePath.asString() + ")");
        }
    }

    if (parsedResult->optionTimes("STDERR") != 0)
    {
        startData.stderrMode = kSTAFProcessIOReplaceFile;
        rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
            "STDERR", stdErr);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }
    else if (parsedResult->optionTimes("STDERRAPPEND") != 0)
    {
        startData.stderrMode = kSTAFProcessIOAppendFile;
        rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
            "STDERRAPPEND", stdErr);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }

    if (stdErr.length() != 0)
    {
        STAFFSPath stderrPath(stdErr);

        // Assign the parent directory path for the Stderr file

        STAFFSPath thePath;
        thePath.setRoot(stderrPath.root());
        
        for (unsigned int i = 0; i < stderrPath.numDirs(); ++i)
        {
            thePath.addDir(stderrPath.dir(i));
        }

        // Create the parent directory path for the Stderr file if it
        // doesn't already exist

        try
        {
            if (!thePath.exists())
            {
                unsigned int osRC = 0;
                STAFString dirToCreate = thePath.asString();
                rc = STAFFSCreateDirectory(
                    dirToCreate.getImpl(), kSTAFFSCreatePath, &osRC);

                if ((rc != kSTAFOk) && (rc != kSTAFAlreadyExists))
                {
                    STAFString result = STAFString(
                        "Could not create STDERR directory (") +
                        dirToCreate + "), OS RC: " + STAFString(osRC);
                    return STAFServiceResult(rc, result);
                }
            }
        }
        catch (STAFException &se)
        {
            STAFString result = STAFString(
                "Error checking if the STDERR directory exists (") +
                thePath.asString() + ") " + se.getText();

            return STAFServiceResult(se.getErrorCode(), result);
        }
        catch (...)
        {
            return STAFServiceResult(
                kSTAFBaseOSError,
                STAFString("Unknown error checking if the STDERR directory "
                           " exists (") + thePath.asString() + ")");
        }
    }

    // Make sure files specified for stdout and stderr are not the same

    if ((stdOut.length() != 0) && (stdErr.length() != 0))
    {
        try
        {
            // First, convert stdOut and stdErr to their full long path names
            // (e.g. with correct file separators, correct case if Windows,
            // no unnecessary trailing slashes, etc), so can better check
            // if the file names are for the same file
            
            STAFFSPath stdoutPath(stdOut);
            STAFString stdoutName = stdoutPath.setRoot(
                stdoutPath.root()).asString();

            STAFFSPath stderrPath(stdErr);
            STAFString stderrName = stderrPath.setRoot(
                stderrPath.root()).asString();

            if (STAFFileSystem::comparePaths(stdoutName, stderrName) ==
                kSTAFFSSamePath)
            {
                return STAFServiceResult(
                    kSTAFInvalidValue,
                    STAFString("The same file cannot be specified for "
                               "stdout, '") + stdOut + "', and stderr, '" +
                    stdErr + "'.  To write stdout and stderr to the same "
                    "file, use the STDERRTOSTDOUT option instead of the "
                    "STDERR option.");
            }
        }
        catch (STAFException &e)
        {
            return STAFServiceResult(
                e.getErrorCode(),
                STAFString("Unexpected exception while checking if the "
                           "same file was specified STDOUT and STDERR.  ") +
                e.getText());
        }
    }
    
    rc = RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(
        "STATICHANDLENAME", staticHandleName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    bool returnFileFlag = false;

    if (parsedResult->optionTimes("RETURNSTDOUT") != 0)
    {
        returnFileFlag = true;

        if (stdOut.length() == 0)
        {
            // STDOUT name not specified so create a temp file.
            // Use Replace File mode by default.

            startData.stdoutMode = kSTAFProcessIOReplaceFile;

            // Get a temporary file name

            STAFString_t tempFileNameT;
            STAFString_t errorBufferT;
            unsigned int osRC = 0;

            rc = STAFUtilCreateTempFile(
                fTempDirectory.getImpl(), STAFString("out").getImpl(),
                requestInfo.fRequestNumber, &tempFileNameT, &errorBufferT, &osRC);

            if (rc)
            {
                return STAFServiceResult(
                    rc, STAFString(errorBufferT, STAFString::kShallow) +
                    " for STDOUT, OSRC: " + osRC);
            }

            stdOut = STAFString(tempFileNameT, STAFString::kShallow);

            TempFileInfo tempFileInfo;
            tempFileInfo.name = stdOut;

            if (startMode == kWait)
            {
                // Delete the temp file after waiting for the process to end
                tempFileInfo.deleteFlag = false;
            }
            else
            {
                // Delete the temp file after the queue has been updated
                tempFileInfo.deleteFlag = true;
            }

            process->stdoutTempFileInfo = tempFileInfo;
        }

        process->retFileList.push_back(stdOut);
    }

    if (parsedResult->optionTimes("RETURNSTDERR") != 0)
    {
        returnFileFlag = true;

        if (stdErr.length() == 0)
        {
            // STDERR name not specified so create a temp file.
            // Use Replace File mode by default.

            startData.stderrMode = kSTAFProcessIOReplaceFile;

            // Get a temporary file name

            STAFString_t tempFileNameT;
            STAFString_t errorBufferT;
            unsigned int osRC = 0;

            rc = STAFUtilCreateTempFile(
                fTempDirectory.getImpl(), STAFString("err").getImpl(),
                requestInfo.fRequestNumber, &tempFileNameT, &errorBufferT, &osRC);

            if (rc)
            {
                // Delete temp file created for Stdout before returning
                if (process->stdoutTempFileInfo.name.length() != 0)
                    deleteTempFile(process->stdoutTempFileInfo.name);

                return STAFServiceResult(
                    rc, STAFString(errorBufferT, STAFString::kShallow) +
                    " for STDERR, OSRC: " + osRC);
            }

            stdErr = STAFString(tempFileNameT, STAFString::kShallow);

            TempFileInfo tempFileInfo;
            tempFileInfo.name = stdErr;

            if (startMode == kWait)
            {
                // Delete the temp file after waiting for the process to end
                tempFileInfo.deleteFlag = false;
            }
            else
            {
                // Delete the temp file after the queue has been updated
                tempFileInfo.deleteFlag = true;
            }

            process->stderrTempFileInfo = tempFileInfo;
        }

        process->retFileList.push_back(stdErr);
    }

    if (parsedResult->optionTimes("STDERRTOSTDOUT") != 0)
        startData.stderrMode = kSTAFProcessIOStdout;

    if (parsedResult->optionTimes("RETURNFILE") > 0)
    {
        returnFileFlag = true;
    
        for (int fileIndex = 1;
             !rc && (fileIndex <= parsedResult->optionTimes("RETURNFILE"));
             ++fileIndex)
        {
            STAFString returnFile;

            rc = RESOLVE_INDEXED_STRING_OPTION_IGNORE_ERRORS(
                "RETURNFILE", fileIndex, returnFile);

            if (!rc) process->retFileList.push_back(returnFile);
        }
    }

    if (returnFileFlag)
    {
        // Determine the maximum return file size

        // Check if variable STAF/MaxReturnFileSize was set to a non-zero
        // value.  Resolve this variable using only the originating handle's
        // pool associated with this request so create a variable pool that
        // only contains the request variable pool.

        const STAFVariablePool *theVarPoolList[] =
        {
            requestInfo.fRequestVarPool,
        };

        unsigned int theVarPoolListSize = sizeof(theVarPoolList) /
            sizeof(const STAFVariablePool *);

        STAFString sizeString;

        STAFRC_t maxSizeRC = STAFVariablePool::resolve(
                "{STAF/MaxReturnFileSize}",
                theVarPoolList, theVarPoolListSize, sizeString);
        
        if (maxSizeRC == kSTAFOk)
        {
            // Variable STAF/MaxReturnFileSize exists

            // Verify if its size value is valid and convert it to bytes
            // if needed

            STAFString_t errorBuffer = 0;
            unsigned int maxSize = 0; // 0 means no size limit

            STAFRC_t rc = STAFUtilConvertSizeString(
                sizeString.getImpl(), &maxSize, &errorBuffer);

            if (rc != kSTAFOk)
            {
                // Delete the temporary StdOut/StdErr files if used before
                // returning
                deleteTempFiles(*process, true);

                STAFString errMsg = STAFString(
                    "Variable STAF/MaxReturnFileSize in the originating "
                    "handle's variable pool for this request is set to "
                    "an invalid value: ") + sizeString + " \n" +
                    STAFString(errorBuffer, STAFString::kShallow);

                return STAFServiceResult(kSTAFInvalidValue, errMsg);
            }

            if (maxSize != 0)
            {
                if ((gMaxReturnFileSize == 0) ||
                    (maxSize < gMaxReturnFileSize))
                {
                    // Assign the maximum file size based on the
                    // STAF/MaxReturnFileSize variable
                    process->maxReturnFileSize = maxSize;
                }
            }
        }
    }

    if (!rc && process->shellCommand.length() != 0)
    {
        if (isShellCommandValid(process->shellCommand) != kSTAFOk)
        {
            rc = kSTAFInvalidValue;
            errorBuffer = STAFString("Invalid shell command: ") +
                process->shellCommand;
        }
    }

    if (!rc)
    {
        if (stopMethodString.length() > 0)
        {
            rc = getStopMethodFromString(process->stopMethod, stopMethodString);

            if (rc)
            {
                rc = kSTAFInvalidValue;
                errorBuffer = STAFString("Invalid stop using method: ") +
                    stopMethodString;
            }

            if (STAFProcess::isValidStopMethod(process->stopMethod) != kSTAFOk)
            {
                rc = kSTAFInvalidValue;
                errorBuffer = STAFString("Invalid stop using method: ") +
                    stopMethodString;
            }
        }
        else
        {
            process->stopMethod = STAFProcessService::getDefaultStopMethod();
        }
    }
    
    if (!rc)
    {
        if (focusString.length() > 0)
        {
            rc = getConsoleFocusFromString(process->consoleFocus, focusString);

            if (rc)
            {
                rc = kSTAFInvalidValue;
                errorBuffer = STAFString("Invalid focus: ") + focusString +
                    "\nValid values are Minimized, Background, or Foreground.";
            }
        }
        else
        {
            process->consoleFocus = STAFProcessService::getDefaultConsoleFocus();
        }
    }

    if (rc)
    {
        // Delete the temporary StdOut/StdErr files if used before returning
        deleteTempFiles(*process, true);

        return STAFServiceResult(rc, errorBuffer);
    }

    process->startStamp = STAFTimestamp::now();
    process->state = kRunning;
    process->notificationList = STAFNotificationListPtr(
                                new STAFNotificationList(),
                                STAFNotificationListPtr::INIT);
    if (doNotify)
    {
        // Assign an alternate endpoint containing the requesting machine's
        // physical interface ID (if different from "machine")

        STAFString alternateEndpoint = replaceEndpointSystemId(
            machine, requestInfo.fPhysicalInterfaceID);

        // Register to be notified (by name or handle) when the process
        // completes

        if (useName)
        {
            process->notificationList->reg(
                machine, alternateEndpoint, name, priority);
        }
        else
        {
            process->notificationList->reg(
                machine, alternateEndpoint, theHandle, priority);
        }
    }

    if (startMode == kWait)
        process->notify = STAFEventSemPtr(new STAFEventSem(),
                                          STAFEventSemPtr::INIT);

    // Create the environment data from the global environment map.
    // Need to create even if no environment variables are being added or
    // changed because if a process is started as another user on Windows
    // and then if a process is started as the user who was logged on when
    // STAFProc was started, the USERPROFILE environment shows the wrong
    // value if set startData->environment = 0 instead of creating the
    // environment data.

    // First, create a STAFProcessEnvMap from the global environment map so
    // that the key (the environment variable name) can be case-insensitive
    // if designated by the operating system.

    STAFProcessEnvMap envMap;

    for (STAFEnvMap::iterator emIter = gEnvMapPtr->begin();
         emIter != gEnvMapPtr->end(); ++emIter)
    {
        STAFString envName = emIter->first;
        STAFString envNameKey = envName;
            
        if (!fEnvVarCaseSensitive)
            envNameKey.upperCase();

        STAFString envValue = emIter->second;

        envMap[envNameKey] = STAFProcessEnvData(envName, envValue);
    }

    // Add all user specified environment variables, if any, to the envMap
    // and create a list of the user environment variables

    std::deque<STAFString> userEnvList;

    for (int i = 1; i <= parsedResult->optionTimes("ENV"); ++i)
    {
        STAFString envVar;

        rc = RESOLVE_INDEXED_STRING_OPTION_IGNORE_ERRORS("ENV", i, envVar);

        if (rc)
        {
            // Delete the temporary StdOut/StdErr files if used
            deleteTempFiles(*process, true);

            return STAFServiceResult(rc, errorBuffer);
        }

        unsigned int equalPos = envVar.find(kUTF8_EQUAL);
        STAFString envName(envVar.subString(0, equalPos));
        STAFString envNameKey = envName;
        STAFString envValue;

        if (equalPos != STAFString::kNPos)
            envValue = envVar.subString(equalPos + 1);

        if (!fEnvVarCaseSensitive)
            envNameKey.upperCase();

        // Retain original env variable name if already exists

        if (envMap.find(envNameKey) != envMap.end())
            envMap[envNameKey].envValue = envValue;
        else
            envMap[envNameKey] = STAFProcessEnvData(envName, envValue);

        // Store only the user specified variables here for use by
        // CreateProcessAsUser() called in Win32's STAFProcess.

        userEnvList.push_back(envVar);
    }

    if (process->handleType == kStatic)
    {
        // Allocate the handle and add the STAF_STATIC_HANDLE environment
        // variable to the envMap

        STAFServiceResult result = gHandleManagerPtr->addAndGetStaticHandle(
            process->handle, staticHandleName, varPool);

        if (result.fRC)
        {
            // Delete the temporary StdOut/StdErr files if used before returning
            deleteTempFiles(*process, true);

            return result;
        }

        envMap["STAF_STATIC_HANDLE"] = STAFProcessEnvData(
            "STAF_STATIC_HANDLE", process->handle);

        userEnvList.push_back(
            STAFString("STAF_STATIC_HANDLE=") + process->handle);
    }

    // Now, walk through the list and combine the entries back into
    // Name=Value form, get the current code page representation and
    // figure out how big a buffer we need

    int bufSize = 0;
    std::deque<STAFStringBufferPtr> envList;

    for (STAFProcessEnvMap::iterator iter2 = envMap.begin();
        iter2 != envMap.end(); ++iter2)
    {
        STAFProcessEnvData envData = iter2->second;
        STAFString envCombo = envData.envName + kUTF8_EQUAL +
            envData.envValue;
        STAFStringBufferPtr envComboPtr = envCombo.toCurrentCodePage();

        bufSize += envComboPtr->length() + 1;  // Add one for the null byte
        envList.push_back(envComboPtr);
    }

    // Allocate the buffer

    bufSize += 1;  // Add one for the trailing null byte
    char *envBuf = new char[bufSize];

    // Walk the list and add the entries to the buffer

    bufSize = 0;

    for (std::deque<STAFStringBufferPtr>::iterator iter = envList.begin();
         iter != envList.end(); ++iter)
    {
        memcpy(envBuf + bufSize, (*iter)->buffer(), (*iter)->length());
        envBuf[bufSize + (*iter)->length()] = 0;
        bufSize += (*iter)->length() + 1;
    }

    // Add the trailing null byte

    envBuf[bufSize] = 0;

    startData.environment = (char *)envBuf;

    // Now set up the user environment array

    STAFRefPtr<STAFStringConst_t> userEnvArray = STAFRefPtr<STAFStringConst_t>
        (new STAFStringConst_t[userEnvList.size()],
         STAFRefPtr<STAFStringConst_t>::INIT);

    for (unsigned int userEnvIndex = 0; userEnvIndex < userEnvList.size();
         ++userEnvIndex)
    {
        userEnvArray[userEnvIndex] = userEnvList[userEnvIndex].getImpl();
    }
    
    startData.userEnvList = userEnvArray;
    startData.userEnvCount = userEnvList.size();

    // Set the console mode for the process

    if (parsedResult->optionTimes("NEWCONSOLE") != 0)
        startData.consoleMode = kSTAFProcessNewConsole;
    else if (parsedResult->optionTimes("SAMECONSOLE") != 0)
        startData.consoleMode = kSTAFProcessSameConsole;
    else
        startData.consoleMode = gDefaultConsoleMode;

    // Set the console focus for the process

    startData.consoleFocus = process->consoleFocus;

    STAFString defaultShellCommand;
    STAFString defaultSameConsoleShell;
    STAFString defaultNewConsoleShell;

    // Set the shell command for the process

    if (process->processType == kCommand)
    {
        startData.commandType = kSTAFProcessCommand;
        startData.shellCommand = 0;
        process->shellCommand = "<No Shell>";
    }
    else
    {   // process->processType == kShell
        startData.commandType = kSTAFProcessShell;
        if (process->shellCommand.length() != 0)
        {
            startData.shellCommand = process->shellCommand.getImpl();
        }
        else
        {
             // Assign the default shell command
             defaultShellCommand = STAFProcessService::getDefaultShellCommand();

             if (startData.consoleMode == kSTAFProcessSameConsole)
             {
                 defaultSameConsoleShell = STAFProcessService::getDefaultSameConsoleShell();

                 if (defaultSameConsoleShell.length() != 0)
                     startData.shellCommand = defaultSameConsoleShell.getImpl();
                 else if (defaultShellCommand.length() != 0)
                     startData.shellCommand = defaultShellCommand.getImpl();
                 else
                     startData.shellCommand = 0;
             }
             else
             {
                 // startData.consoleMode = kSTAFProcessNewConsole
                 defaultNewConsoleShell = STAFProcessService::getDefaultNewConsoleShell();

                 if (defaultNewConsoleShell.length() != 0)
                     startData.shellCommand = defaultNewConsoleShell.getImpl();
                 else if (defaultShellCommand.length() != 0)
                     startData.shellCommand = defaultShellCommand.getImpl();
                 else
                     startData.shellCommand = 0;
             }

             // Assign shell command so query a process handle shows it
             if (startData.shellCommand == 0)
                 process->shellCommand = "<Default Shell>";
             else
                 process->shellCommand = startData.shellCommand;
        }
    }

    STAFString commandNoPrivacyDelimiters = STAFHandle::removePrivacyDelimiters(
        process->command);

    startData.command = commandNoPrivacyDelimiters.getImpl();
    
    STAFString parmsNoPrivacyDelimiters;

    if (process->parms.length() != 0)
    {
        parmsNoPrivacyDelimiters = STAFHandle::removePrivacyDelimiters(
            process->parms);
        startData.parms = parmsNoPrivacyDelimiters.getImpl();
    }
    else
    {
        startData.parms = 0;
    }

    startData.title = (process->title.length() != 0) ?
        process->title.getImpl() : 0;

    startData.workdir = (process->workdir.length() != 0) ?
        process->workdir.getImpl() : 0;

    startData.workload = (process->workload.length() != 0) ?
        process->workload.getImpl() : 0;

    STAFString defaultUsername;
    STAFString defaultPassword;

    if (process->username.length() != 0)
        startData.username = process->username.getImpl();
    else
    {
        defaultUsername = STAFProcessService::getDefaultAuthUsername();
        if (defaultUsername.length() != 0)
            startData.username = defaultUsername.getImpl();
        else
            startData.username = 0;
    }

    STAFString passwordNoPrivacyDelimiters;

    if (password.length() != 0)
    {
        passwordNoPrivacyDelimiters = STAFHandle::removePrivacyDelimiters(
            password);
        startData.password = passwordNoPrivacyDelimiters.getImpl();
    }
    else
    {
        defaultPassword = STAFProcessService::getDefaultAuthPassword();

        if (defaultPassword.length() != 0)
        {
            defaultPassword = STAFHandle::removePrivacyDelimiters(
                defaultPassword);
            startData.password = defaultPassword.getImpl();
        }
        else
        {
            startData.password = 0;
        }
    }

    startData.stdinRedirect  = stdInp.getImpl();
    startData.stdoutRedirect = stdOut.getImpl();
    startData.stderrRedirect = stdErr.getImpl();

    if (parsedResult->optionTimes("DISABLEDAUTHISERROR"))
        startData.disabledAuthAction = kSTAFProcessDisabledAuthError;
    else if (parsedResult->optionTimes("IGNOREDISABLEDAUTH"))
        startData.disabledAuthAction = kSTAFProcessDisabledAuthIgnore;
    else
        startData.disabledAuthAction = STAFProcessService::getDefaultDisabledAuthAction();

    startData.authMode = STAFProcessService::getAuthMode();

    STAFProcessEndCallbackLevel1 callback = { processTerminationCallback, 0 };

    startData.callback = &callback;

    unsigned int osRC = 0;
    STAFString errorBuffer2;

    // This block is here to make sure that the process is started and on
    // the list before processTerminationCallback tries to remove it from
    // the list.  We also get the HandleManager's lock so that we atomically
    // start the process and add the pending handle.

    {
        STAFMutexSemLock processLock(fProcessListSem);
        STAFMutexSemLock handleLock(gHandleManagerPtr->getHandleManagerSem());

        rc = STAFProcess::startProcess2(
            startData, process->pid, process->procHandle, osRC, errorBuffer2);

        if (rc == kSTAFOk)
        {
            if (process->handleType == kPending)
            {
                STAFServiceResult result =
                    gHandleManagerPtr->addAndGetPendingHandle(
                        process->handle, process->pid, process->procHandle,
                        varPool);

                if (result.fRC)
                {
                    rc = result.fRC;
                    errorBuffer2 = result.fResult;
                }
            }
            else if (process->handleType == kStatic)
            {
                // Update the pid/procHandle for this static handle that was
                // created by specifying the STATICHANDLENAME option so that
                // the pid won't be 0 when submitting a LIST/QUERY
                // request to the HANDLE service

                gHandleManagerPtr->updateStaticHandle(
                    process->handle, process->pid, process->procHandle);
            }

            if (rc == kSTAFOk)
                fProcessList[process->handle] = process;
        }
    }

    if (rc != kSTAFOk)
    {
        // The process failed to start, but Stdout/Stderr files (temporary or
        // not) may have been created and should be deleted before returning
        try
        {
            if (stdOut.length() != 0)
                STAFFSPath(stdOut).getEntry()->remove(&osRC);

            if (stdErr.length() != 0)
                STAFFSPath(stdErr).getEntry()->remove(&osRC);
        }
        catch (...)
        { /* Ignore any errors */ }
    }

    delete [] envBuf;

    if ((rc != kSTAFOk) && (process->handleType == kStatic))
        gHandleManagerPtr->removeStaticHandle(process->handle);

    if (rc != kSTAFOk)
    {
        STAFString errorMsg = errorBuffer2;
        return STAFServiceResult(rc, errorMsg);
    }

    STAFString result(process->handle);

    if (startMode == kWait)
    {
        process->notify->wait(timeout);

        if (process->notify->query())
        {
            // We didn't time out waiting for the process to end
            // So remove the process from the list, remove the handle,
            // and return the RC

            STAFMutexSemLock processLock(fProcessListSem);

            if (process->handleType == kPending)
            {
                gHandleManagerPtr->removePendingHandle(process->handle,
                                                       process->pid);
            }
            else
            {
                gHandleManagerPtr->removeStaticHandle(process->handle);
            }

            fProcessList.erase(process->handle);

            // Create the marshalling context and set its map class definitions

            STAFObjectPtr mc = STAFObject::createMarshallingContext();

            mc->setMapClassDefinition(fCompletionMapClass->reference());
            mc->setMapClassDefinition(fReturnFileMapClass->reference());

            // Create a map to contain the completion information

            STAFObjectPtr completionMap =
                fCompletionMapClass->createInstance();

            completionMap->put("rc", STAFString(process->RC));

            if (process->key.length() != 0)
                completionMap->put("key", process->key);
            else
                completionMap->put("key", STAFObject::createNone());

            // Now, tack on any files that the user wanted returned

            STAFObjectPtr returnFileList = STAFObject::createList();

            if (process->retFileList.size() != 0)
            {
                for (STAFProcessService::Process::FileList::iterator iter =
                         process->retFileList.begin();
                     iter != process->retFileList.end(); ++iter)
                {
                    STAFServiceResult fileResult = readFileIntoString(
                        *iter, process->maxReturnFileSize);

                    STAFObjectPtr returnFileMap = 
                        fReturnFileMapClass->createInstance();

                    returnFileMap->put(
                        "rc", STAFString(fileResult.fRC));
                    returnFileMap->put(
                        "data", STAFString(fileResult.fResult));

                    returnFileList->append(returnFileMap);
                }
            }

            completionMap->put("fileList", returnFileList);

            mc->setRootObject(completionMap);

            try
            {
                result = mc->marshall();
            }
            catch (...)
            {
                // Catch an unexpected errors, such a memory allocation error.
                // If the maximum file size previously specified is > 1M and
                // the returned file data exceeds 1M, return an error for the
                // file rc instead of returning the large file data string
                // and retry marshalling the process completion data.

                bool retryMarshall = false;

                if ((process->retFileList.size() != 0) &&
                    ((process->maxReturnFileSize == 0) ||  // No max file size
                     (process->maxReturnFileSize > 1048576)))
                {
                    returnFileList = STAFObject::createList();

                    for (STAFProcessService::Process::FileList::iterator iter =
                             process->retFileList.begin();
                         iter != process->retFileList.end(); ++iter)
                    {
                        STAFServiceResult fileResult = readFileIntoString(
                            *iter, 1048576);

                        if (fileResult.fRC == kSTAFMaximumSizeExceeded)
                        {
                            retryMarshall = true;
                            fileResult.fResult = fileResult.fResult +
                                "  Note: An error occurred marshalling the "
                                "large file data, so reduced the maximum "
                                "return file size to 1M.";
                        }

                        STAFObjectPtr returnFileMap = 
                            fReturnFileMapClass->createInstance();

                        returnFileMap->put(
                            "rc", STAFString(fileResult.fRC));
                        returnFileMap->put(
                            "data", STAFString(fileResult.fResult));

                        returnFileList->append(returnFileMap);
                    }
                }

                if (retryMarshall)
                {
                    // Retry marshalling using the new file list just created

                    completionMap->put("fileList", returnFileList);
                    mc->setRootObject(completionMap);
                    result = mc->marshall();
                }
                else
                {
                    deleteTempFiles(*process, true);
                    throw;
                }
            }

            // Delete the temporary StdOut/Stderr files if used, with the
            // force argument set to true so it doesn't check the deleteFlags
            deleteTempFiles(*process, true);
        }
        else
        {
            // STAF timed out waiting for the process to end

            rc = kSTAFTimeout;

            // XXX: Is there a timing issue where the notify could have
            // happened before this lock such that the temp files don't
            // get deleted?

            // Set the delete flag for the temporary Stdout/Stderr files
            // if used so that the temp files will be deleted in the
            // sendNotfication() method.

            STAFMutexSemLock lock(*process->tempFileMutex);

            if (process->stdoutTempFileInfo.name.length() != 0)
                process->stdoutTempFileInfo.deleteFlag = true;

            if (process->stderrTempFileInfo.name.length() != 0)
                process->stderrTempFileInfo.deleteFlag = true;
        }
    }

    return STAFServiceResult(rc, result);
}


STAFServiceResult STAFProcessService::handleStop(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "STOP");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fStopParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString stopMethodString;
    STAFString errorBuffer;
    STAFProcessStopMethod_t stopMethod;
    bool userSpecifiedStopMethod = false;

    // Resolve the HANDLE option (if present) and verify that it can be
    // converted to an unsigned integer in the range for valid handle numbers

    STAFHandle_t theHandle = 0;

    STAFRC_t rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
        "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);
    
    // Resolve the USING option (if present) and verify that it specifies
    // a valid stop using method

    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("USING", stopMethodString);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (stopMethodString.length() > 0)
    {
        rc = STAFProcessService::getStopMethodFromString(
            stopMethod, stopMethodString);

        if (rc)
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                STAFString("Invalid value for stop using method: ") +
                stopMethodString);
        }
        
        if (STAFProcess::isValidStopMethod(stopMethod) != kSTAFOk)
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                STAFString("Invalid stop using method: ") + stopMethodString);
        }       

        userSpecifiedStopMethod = true;
    }

    STAFString result;
    STAFMutexSemLock processLock(fProcessListSem);

    if (parsedResult->optionTimes("HANDLE") != 0)
    {
        if (fProcessList.find(theHandle) == fProcessList.end())
        {
            return STAFServiceResult(
                kSTAFHandleDoesNotExist,
                STAFString("The specified handle does not exist: ") +
                theHandle);
        }

        ProcessPtr process = fProcessList[theHandle];

        if (process->state == kComplete)
            return STAFServiceResult(kSTAFProcessAlreadyComplete, "");

        unsigned int osRC = 0;

        rc = STAFProcess::stopProcess(process->pid,
             userSpecifiedStopMethod ? stopMethod : process->stopMethod, osRC);

        if (rc == kSTAFBaseOSError)
            return STAFServiceResult(rc, STAFString(osRC));
        else if (rc != kSTAFOk)
            return rc;

        gHandleManagerPtr->unRegister(theHandle, process->pid);

    }
    else
    {
        int stopAll = parsedResult->optionTimes("ALL");
        STAFString workload;
        STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("WORKLOAD", workload);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        workload.lowerCase();

        // Create the marshalling context and set its map class definitions

        STAFObjectPtr mc = STAFObject::createMarshallingContext();

        mc->setMapClassDefinition(fStopMapClass->reference());

        // Iterate through the processes

        ProcessList::iterator iter;
        int countTotal = fProcessList.size();
        int countStopped = 0;

        for (iter = fProcessList.begin(); iter != fProcessList.end(); ++iter)
        {
            ProcessPtr process = iter->second;

            if ((stopAll != 0) || (process->workload.toLowerCase() ==
                                   workload))
            {
                // Note: When stopping ALL or a workload, it is not an error
                //       to stop a kComplete process.  The implication is
                //       that only kRunning processes will be stopped.

                unsigned int osRC = 0;

                if (process->state == kRunning)
                {
                    rc = STAFProcess::stopProcess(
                         process->pid,
                         userSpecifiedStopMethod ?
                             stopMethod :
                             process->stopMethod,
                         osRC);
                    ++countStopped;
                }

                if (rc == kSTAFBaseOSError)
                    return STAFServiceResult(rc, STAFString(osRC));
                else if (rc != kSTAFOk)
                    return rc;

                gHandleManagerPtr->unRegister(process->handle, process->pid);
            }

        }  // end while iter valid

        // Create a map containing # processes stopped and the total processes

        STAFObjectPtr stopInfoMap = fStopMapClass->createInstance();

        stopInfoMap->put("stoppedProcesses", STAFString(countStopped));
        stopInfoMap->put("totalProcesses",  STAFString(countTotal));

        mc->setRootObject(stopInfoMap);
        result = mc->marshall();

    }  // end if ALL or WORKLOAD

    return STAFServiceResult(kSTAFOk, result);
}


STAFServiceResult STAFProcessService::handleKill(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "KILL");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fKillParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    
    //  Get the PID

    // Note:  The maximum PID on Windows is UINT_MAX and the maximum PID on
    // Unix is INT_MAX because those are the maximum values for the
    // STAFProcessID_t type

    unsigned int maxPID = (unsigned int)STAFProcessService::fMaxPid;
    unsigned int pidUInt;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE("PID", pidUInt, 0, maxPID);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFProcessID_t thePid = static_cast<STAFProcessID_t>(pidUInt);
    
    if (thePid == gSTAFProcPID)
    {
        return STAFServiceResult(
            kSTAFInvalidValue,
            "You are not allowed to specify the pid for the STAFProc process."
            "   Instead, use the SHUTDOWN service to shutdown STAFProc.");
    }

    // Get the method to use to kill the process, if provided

    STAFString stopMethodString;
    STAFProcessStopMethod_t stopMethod;
    bool userSpecifiedStopMethod = false;

    if (parsedResult->optionTimes("USING") > 0)
    {
        rc = RESOLVE_OPTIONAL_STRING_OPTION("USING", stopMethodString);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::getStopMethodFromString(
            stopMethod, stopMethodString);

        if (rc)
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                STAFString("Invalid value for stop using method: ") +
                stopMethodString);
        }

        userSpecifiedStopMethod = true;
    }

    // If the USING option was not specified, use the default stop method
    // for the PROCESS service

    if (!userSpecifiedStopMethod)
        stopMethod = STAFProcessService::getDefaultStopMethod();

    unsigned int osRC = 0;

    rc = STAFProcess::stopProcess2(
        thePid, stopMethod, kSTAFProcessKillRequest, osRC);

    if (rc == kSTAFOk)
        return STAFServiceResult(kSTAFOk);
    else if (rc == kSTAFBaseOSError)
        return STAFServiceResult(rc, STAFString(osRC));
    else
    {
        if (rc == kSTAFDoesNotExist)
        {
            return STAFServiceResult(
                rc, STAFString("The specified pid does not exist: ") +
                STAFString(thePid));
        }
        else
        {
            return STAFServiceResult(rc);
        }
    }
}


STAFServiceResult STAFProcessService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    ProcessList::iterator iter;

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("SETTINGS") > 0)
    {
        // LIST SETTINGS

        // Create a marshalled map containing operational settings information
        // for the PROCESS service

        mc->setMapClassDefinition(fSettingsMapClass->reference());
        STAFObjectPtr settingsMap = fSettingsMapClass->createInstance();

        settingsMap->put("defaultStopUsing", 
                         STAFProcessService::getDefaultStopMethodAsString());

        if (gDefaultConsoleMode == kSTAFProcessNewConsole)
            settingsMap->put("defaultConsoleMode", "New");
        else
            settingsMap->put("defaultConsoleMode", "Same");

        settingsMap->put("defaultFocus", 
                         STAFProcessService::getDefaultConsoleFocusAsString());

        settingsMap->put("processAuthMode",
                         STAFProcessService::getAuthModeAsString());

        STAFString username = STAFProcessService::getDefaultAuthUsername();
        
        if (username != STAFString(""))
            settingsMap->put("defaultAuthUsername", username);

        if (STAFProcessService::getDefaultAuthPassword() != STAFString(""))
            settingsMap->put("defaultAuthPassword", "*******");

        settingsMap->put("defaultAuthDisabledAction",
                         STAFProcessService::
                         getDefaultDisabledAuthActionAsString());

        STAFString shell = STAFProcessService::getDefaultShellCommand();

        if (shell != STAFString(""))
            settingsMap->put("defaultShell", shell);

        shell = STAFProcessService::getDefaultNewConsoleShell();

        if (shell != STAFString(""))
            settingsMap->put("defaultNewConsoleShell", shell);

        shell = STAFProcessService::getDefaultSameConsoleShell();

        if (shell != STAFString(""))
            settingsMap->put("defaultSameConsoleShell", shell);

        mc->setRootObject(settingsMap);

        return STAFServiceResult(kSTAFOk, mc->marshall());
    }

    // List processes

    // LIST  [HANDLES] [RUNNING] [COMPLETED] [WORKLOAD <Name>] [LONG]

    STAFMutexSemLock processLock(fProcessListSem);

    STAFString workload;
    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("WORKLOAD", workload);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    workload.lowerCase();

    unsigned int listCompleted = (parsedResult->optionTimes("COMPLETED") != 0);
    unsigned int listRunning = (parsedResult->optionTimes("RUNNING") != 0);

    if ((listCompleted == 0) && (listRunning == 0))
    {
        listCompleted = 1;
        listRunning = 1;
    }

    bool listLong = (parsedResult->optionTimes("LONG") != 0);

    // Set the map class definition(s) for the marshalling context
    
    if (!listLong)
        mc->setMapClassDefinition(fListProcessMapClass->reference());
    else
        mc->setMapClassDefinition(fListLongProcessMapClass->reference());

    // Create an empty list object for marshalled list of pending requests

    STAFObjectPtr processList = STAFObject::createList();

    unsigned int listAll = (parsedResult->optionTimes("WORKLOAD") == 0);

    // Iterate through the process list

    for (iter = fProcessList.begin(); iter != fProcessList.end(); iter++)
    {

        ProcessPtr process = iter->second;

        if (((process->workload.toLowerCase() == workload) || listAll != 0)
           && (((process->state==kComplete) && (listCompleted != 0))
              ||((process->state==kRunning) && (listRunning != 0))))
        {
            // Add an entry to processList

            STAFObjectPtr processMap;

            if (!listLong)
            {
                processMap = fListProcessMapClass->createInstance();
            }
            else
            {
                processMap = fListLongProcessMapClass->createInstance();

                // Assign fields that are not in the "short" list format
                if (process->workload.length() != 0)
                    processMap->put("workload", process->workload);

                processMap->put("pid", process->pid);
            }
            
            processMap->put("handle", process->handle);
            processMap->put("command",
                            STAFHandle::maskPrivateData(process->command));
            processMap->put("startTimestamp", process->startStamp.asString());

            if (process->state == kComplete)
            {
                processMap->put("endTimestamp", process->endStamp.asString());
                processMap->put("rc", STAFString(process->RC));
            }
            else
            {
                processMap->put("endTimestamp", STAFObject::createNone());
                processMap->put("rc", STAFObject::createNone());
            }

            processList->append(processMap);
        }
    }

    // Set the marshalling context's root object

    mc->setRootObject(processList);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFProcessService::handleSet(
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

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFRC_t rc = 0;
    
    if (parsedResult->optionTimes("DEFAULTSTOPUSING") > 0)
    {
        STAFString stopString;
        STAFProcessStopMethod_t stopMethod;

        rc = RESOLVE_STRING_OPTION("DEFAULTSTOPUSING", stopString);

        if (rc) return STAFServiceResult(rc, errorBuffer);
        
        rc = STAFProcessService::getStopMethodFromString(stopMethod,
                                                         stopString);
        if (rc)
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "DEFAULTSTOPUSING contains an invalid method: " + stopString);
        }

        rc = STAFProcessService::setDefaultStopMethod(stopMethod);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTSTOPUSING method to " + stopString);
        }
    }

    if (parsedResult->optionTimes("PROCESSAUTHMODE") > 0)
    {
        STAFString modeString;
        STAFProcessAuthenticationMode_t mode;
        unsigned int osRC = 0;

        rc = RESOLVE_STRING_OPTION("PROCESSAUTHMODE", modeString);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::getAuthModeFromString(mode, modeString);

        if (rc)
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "PROCESSAUTHMODE contains an invalid mode: " + modeString);
        }

        rc = STAFProcessService::setAuthMode(mode, osRC);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting PROCESSAUTHMODE to " + modeString);
        }
    }

    if (parsedResult->optionTimes("DEFAULTAUTHUSERNAME") > 0)
    {
        STAFString username;

        rc = RESOLVE_STRING_OPTION("DEFAULTAUTHUSERNAME", username);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::setDefaultAuthUsername(username);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTAUTHUSERNAME to " + username);
        }
    }
    
    if (parsedResult->optionTimes("DEFAULTAUTHPASSWORD") > 0)
    {
        STAFString password;

        rc = RESOLVE_STRING_OPTION("DEFAULTAUTHPASSWORD", password);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::setDefaultAuthPassword(password);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTAUTHPASSWORD");
        }
    }

    if (parsedResult->optionTimes("DEFAULTAUTHDISABLEDACTION") > 0)
    {
        STAFString actionString;
        STAFProcessDisabledAuthAction_t action;

        rc = RESOLVE_STRING_OPTION("DEFAULTAUTHDISABLEDACTION", actionString);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::getDefaultDisabledAuthActionFromString(
             action, actionString);

        if (rc)
        {
            return STAFServiceResult(
                kSTAFInvalidValue, "DEFAULTAUTHDISABLEDACTION contains an "
                "invalid value: " + actionString);
        }

        rc = STAFProcessService::setDefaultDisabledAuthAction(action);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTAUTHDISABLEDACTION to " +
                actionString);
        }
    }

    if (parsedResult->optionTimes("DEFAULTCONSOLE") > 0)
    {
        // SET DEFAULTCONSOLE

        STAFString defaultConsole;
        rc = RESOLVE_STRING_OPTION("DEFAULTCONSOLE", defaultConsole);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        if (defaultConsole.isEqualTo(sNew, kSTAFStringCaseInsensitive))
        {
            // Get exclusive access to gDefaultConsoleMode

            STAFMutexSemLock lock(sDefaultConsoleModeSem);
            gDefaultConsoleMode = kSTAFProcessNewConsole;
        }
        else if (defaultConsole.isEqualTo(sSame, kSTAFStringCaseInsensitive))
        {
            // Get exclusive access to gDefaultConsoleMode

            STAFMutexSemLock lock(sDefaultConsoleModeSem);
            gDefaultConsoleMode = kSTAFProcessSameConsole;
        }
        else
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "DEFAULTCONSOLE value must be NEW or SAME.  "
                "Invalid value: " + defaultConsole);
        }
    }

    if (parsedResult->optionTimes("DEFAULTFOCUS") > 0)
    {
        // SET DEFAULTFOCUS

        STAFString focusString;
        STAFProcessConsoleFocus_t focus;

        rc = RESOLVE_STRING_OPTION("DEFAULTFOCUS", focusString);

        if (rc) return STAFServiceResult(rc, errorBuffer);
        
        rc = STAFProcessService::getConsoleFocusFromString(focus, focusString);

        if (rc)
        {
            return STAFServiceResult(
                kSTAFInvalidValue,
                "DEFAULTFOCUS value must be Background, Foreground, or "
                "Minimized.  Invalid value: " + focusString);
        }

        STAFProcessService::setDefaultConsoleFocus(focus);
    }

    if (parsedResult->optionTimes("DEFAULTSHELL") > 0)
    {
        STAFString shellCommand;

        rc = RESOLVE_STRING_OPTION("DEFAULTSHELL", shellCommand);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::setDefaultShellCommand(shellCommand);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTSHELL to " + shellCommand);
        }
    }

    if (parsedResult->optionTimes("DEFAULTSAMECONSOLESHELL") > 0)
    {
        STAFString shellCommand;

        rc = RESOLVE_STRING_OPTION("DEFAULTSAMECONSOLESHELL", shellCommand);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::setDefaultSameConsoleShell(shellCommand);

        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTSAMECONSOLESHELL to " + shellCommand);
        }
    }

    if (parsedResult->optionTimes("DEFAULTNEWCONSOLESHELL") > 0)
    {
        STAFString shellCommand;

        rc = RESOLVE_STRING_OPTION("DEFAULTNEWCONSOLESHELL", shellCommand);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = STAFProcessService::setDefaultNewConsoleShell(shellCommand);
        
        if (rc)
        {
            return STAFServiceResult(
                rc, "Error setting DEFAULTNEWCONSOLESHELL to " + shellCommand);
        }
    }

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFProcessService::handleQuery(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "QUERY");

    ProcessList::iterator iter;

    STAFMutexSemLock processLock(fProcessListSem);

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fQueryParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFHandle_t handle;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "HANDLE", handle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);
    
    if (fProcessList.find(handle) == fProcessList.end())
    {
        return STAFServiceResult(
            kSTAFHandleDoesNotExist,
            STAFString("The specified handle does not exist: ") + handle);
    }

    // Create the marshalling context and set its map class definitions

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fProcessInfoMapClass->reference());
    
    ProcessPtr process = fProcessList[handle];

    // Create a map entry with all the process information

    STAFObjectPtr queryMap = fProcessInfoMapClass->createInstance();
    
    queryMap->put("handle", STAFString(handle));

    if (gHandleManagerPtr->name(handle).length() == 0)
        queryMap->put("handleName", STAFObject::createNone());
    else
        queryMap->put("handleName", gHandleManagerPtr->name(handle));

    if (process->title.length() == 0)
        queryMap->put("title", STAFObject::createNone());
    else
        queryMap->put("title", process->title);

    if (process->workload == "")
        queryMap->put("workload", STAFObject::createNone());
    else
        queryMap->put("workload", process->workload);

    if (process->shellCommand == "<No Shell>")
        queryMap->put("shell", STAFObject::createNone());
    else
        queryMap->put("shell", process->shellCommand);

    queryMap->put("command", STAFHandle::maskPrivateData(process->command));

    if (process->parms.length() == 0)
        queryMap->put("parms", STAFObject::createNone());
    else
        queryMap->put("parms", STAFHandle::maskPrivateData(process->parms));

    if (process->workdir.length() == 0)
        queryMap->put("workdir", STAFObject::createNone());
    else
        queryMap->put("workdir", process->workdir);

    queryMap->put("focus", getConsoleFocusAsString(process->consoleFocus));

    if (process->username.length() == 0)
        queryMap->put("userName", STAFObject::createNone());
    else
        queryMap->put("userName", process->username);

    if (process->key.length() == 0)
        queryMap->put("key", STAFObject::createNone());
    else
        queryMap->put("key", process->key);

    queryMap->put("pid", STAFString(process->pid));

    if (process->notify)
        queryMap->put("startMode", "Wait");
    else
        queryMap->put("startMode", "Async");

    queryMap->put("startTimestamp", process->startStamp.asString());

    if (process->state == kComplete)
    {
        queryMap->put("endTimestamp", process->endStamp.asString());
        queryMap->put("rc", STAFString(process->RC));
    }
    else
    {
        queryMap->put("endTimestamp", STAFObject::createNone());
        queryMap->put("rc", STAFObject::createNone());
    }

    // Set the marshalling context's root object

    mc->setRootObject(queryMap);
    
    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFProcessService::handleFree(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "FREE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fFreeParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString result;

    STAFMutexSemLock processLock(fProcessListSem);

    if (parsedResult->optionTimes("HANDLE") != 0)
    {
        STAFHandle_t handle;

        STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
            "HANDLE", handle, gHandleManagerPtr->getMinHandleNumber(),
            gHandleManagerPtr->getMaxHandleNumber());

        if (rc) return STAFServiceResult(rc, errorBuffer);

        if (fProcessList.find(handle) == fProcessList.end())
        {
            return STAFServiceResult(
                kSTAFHandleDoesNotExist,
                STAFString("The specified handle does not exist: ") + handle);
        }

        if (fProcessList[handle]->state == kRunning)
            return STAFServiceResult(kSTAFProcessNotComplete, "");

        if (fProcessList[handle]->handleType == kPending)
        {
            gHandleManagerPtr->removePendingHandle(handle,
                                                   fProcessList[handle]->pid);
        }
        else
        {
            gHandleManagerPtr->removeStaticHandle(handle);
        }

        fProcessList.erase(handle);
    }
    else
    {
        int freeAll = parsedResult->optionTimes("ALL");
        STAFString workload;
        STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("WORKLOAD", workload);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        workload.lowerCase();

        // Create the marshalling context and set its map class definitions

        STAFObjectPtr mc = STAFObject::createMarshallingContext();

        mc->setMapClassDefinition(fFreeMapClass->reference());

        ProcessList::iterator iter;
        int countTotal = fProcessList.size();
        int countFreed = 0;

        for (int freedOne = 1; freedOne == 1;)
        {
            for (freedOne = 0, iter = fProcessList.begin();
                 !freedOne && iter != fProcessList.end();)
            {
                ProcessPtr process = iter->second;

                if ((freeAll != 0) || (process->workload.toLowerCase() ==
                    workload))
                {
                    // Note: When freeing ALL or a workload, it is not an error
                    //       to free a kRunning process.  The implication is
                    //       that only kComplete processes will be freed.

                    if (process->state == kComplete)
                    {
                        if (process->handleType == kPending)
                        {
                            gHandleManagerPtr->removePendingHandle(
                                process->handle, process->pid);
                        }
                        else
                        {
                            gHandleManagerPtr->removeStaticHandle(
                                process->handle);
                        }

                        fProcessList.erase(process->handle);
                        freedOne = 1;
                        ++countFreed;
                        iter = fProcessList.begin();
                    }
                }

                if (!freedOne) iter++;

            }  // end while iter valid or not freed one
        }  // end while we can still free more

        // Create a amp containing # processes freed and the total processes

        STAFObjectPtr freeInfoMap = fFreeMapClass->createInstance();

        freeInfoMap->put("freedProcesses", STAFString(countFreed));
        freeInfoMap->put("totalProcesses",  STAFString(countTotal));

        mc->setRootObject(freeInfoMap);
        result = mc->marshall();

    }  // end if free ALL or WORKLOAD

    return STAFServiceResult(kSTAFOk, result);
}


STAFServiceResult STAFProcessService::handleNotifyRegistration(
    unsigned int isRegister, const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    if (isRegister)
    {
        IVALIDATE_TRUST(3, "NOTIFY REGISTER");
    }
    else
    {
        IVALIDATE_TRUST(3, "NOTIFY UNREGISTER");
    }

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fNotifyRegistrationParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    unsigned int priority      = 5;
    STAFString   machine = requestInfo.fEndpoint;
    STAFHandle_t theHandle = requestInfo.fHandle;
    STAFString   name;
    unsigned int useName = 0;
    STAFString   errorBuffer;

    // Resolve the required ONENDOFHANDLE option and verify that it can be
    // converted to an unsigned integer in the range for valid handle handles

    STAFHandle_t processHandle = 0;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "ONENDOFHANDLE", processHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Resolve the MACHINE option (if present)

    rc = RESOLVE_OPTIONAL_STRING_OPTION("MACHINE", machine);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Resolve the PRIORITY option (if present) and verify that it can be
    // converted to an unsigned integer in the range of 0 to UINT_MAX

    rc = RESOLVE_OPTIONAL_UINT_OPTION("PRIORITY", priority);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Check if the HANDLE option (handle#) or the NAME option (handle name)
    // is specified

    if (parsedResult->optionTimes("HANDLE") > 0)
    {
        // Resolve the HANDLE option (if present) and verify that it can be
        // converted to an unsigned integer in the range for valid handle numbers
        
        rc = RESOLVE_UINT_OPTION_RANGE(
        "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }
    else if (parsedResult->optionTimes("NAME") > 0)
    {
        useName = 1;

        // Resolve the NAME option

        if (!rc) rc = RESOLVE_STRING_OPTION("NAME", name);

        if (rc) return STAFServiceResult(rc, errorBuffer);
    }

    STAFMutexSemLock processLock(fProcessListSem);

    if (fProcessList.find(processHandle) == fProcessList.end())
    {
        return STAFServiceResult(
            kSTAFHandleDoesNotExist,
            STAFString("The specified handle does not exist: ") +
            processHandle);
    }

    ProcessPtr process = fProcessList[processHandle];

    if (process->state == kComplete)
        return STAFServiceResult(kSTAFProcessAlreadyComplete);

    if (isRegister)
    {
        if (useName)
            rc = process->notificationList->reg(machine, name, priority);
        else
            rc = process->notificationList->reg(machine, theHandle, priority);
    }
    else
    {
        if (useName)
        {
            rc = process->notificationList->unregister(machine, name,
                                                       priority);
        }
        else
        {
            rc = process->notificationList->unregister(machine, theHandle,
                                                       priority);
        }
    }

    return rc;
}


STAFServiceResult STAFProcessService::handleNotifyList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "NOTIFY LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fNotifyListParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;

    // Resolve the required ONENDOFHANDLE option and verify that it can be
    // converted to an unsigned integer in the range for valid handle handles

    STAFHandle_t processHandle = 0;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "ONENDOFHANDLE", processHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    ProcessPtr process;

    // Just lock the list long enough to get a pointer to the process

    {
        STAFMutexSemLock processLock(fProcessListSem);

        if (fProcessList.find(processHandle) == fProcessList.end())
        {
            return STAFServiceResult(
                kSTAFHandleDoesNotExist,
                STAFString("The specified handle does not exist: ") +
                processHandle);
        }

        process = fProcessList[processHandle];
    }

    // Create the marshalling context and set its map class definitions

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    
    mc->setMapClassDefinition(fNotifieeMapClass->reference());
    
    // Create an empty list object for the resulting list of notifiees

    STAFObjectPtr outputList = STAFObject::createList();

    // Get the list of notifiees

    STAFNotificationList::NotifieeList notifieeList(
        process->notificationList->getNotifieeListCopy());

    // Iterate thru the list of notifiees

    STAFNotificationList::NotifieeList::iterator iter;

    for (iter = notifieeList.begin(); iter != notifieeList.end(); iter++)
    {
        STAFNotificationList::Notifiee notifiee = *iter;

        // Create a map of information about each notifiee

        STAFObjectPtr notifieeMap = fNotifieeMapClass->createInstance();
        notifieeMap->put("priority", STAFString(notifiee.priority));
        notifieeMap->put("machine", notifiee.machine);

        if (notifiee.nameOrHandle == STAFNotificationList::Notifiee::kName)
        {
            notifieeMap->put("notifyBy", "Name");
            notifieeMap->put("notifiee", notifiee.process);
        }
        else
        {
            notifieeMap->put("notifyBy", "Handle");
            notifieeMap->put("notifiee", STAFString(notifiee.handle));
        }

        outputList->append(notifieeMap);
    }

    mc->setRootObject(outputList);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFProcessService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    // Return the help text for the service

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}


STAFServiceResult STAFProcessService::deleteTempFiles(
    Process &process, bool forceFlag)
{
    STAFServiceResult result = STAFServiceResult(kSTAFOk);
    STAFServiceResult deleteResult;

    STAFMutexSemLock lock(*process.tempFileMutex);

    if ((process.stdoutTempFileInfo.name.length() != 0) &&
        (forceFlag || process.stdoutTempFileInfo.deleteFlag))
    {
        // Set flag to prevent this file from being deleted again
        process.stdoutTempFileInfo.deleteFlag = false;

        deleteResult = deleteTempFile(process.stdoutTempFileInfo.name);

        if (deleteResult.fRC != kSTAFOk)
        {
            result = deleteResult;
        }
    }

    if ((process.stderrTempFileInfo.name.length() != 0) &&
        (forceFlag || process.stderrTempFileInfo.deleteFlag))
    {
        // Set flag to prevent this file from being deleted again
        process.stderrTempFileInfo.deleteFlag = false;

        deleteResult = deleteTempFile(process.stderrTempFileInfo.name);

        if ((deleteResult.fRC != kSTAFOk) &&
            (result.fRC == kSTAFOk))
        {
            result = deleteResult;
        }
    }

    return result;
}
