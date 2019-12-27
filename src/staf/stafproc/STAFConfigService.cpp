/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2012                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <deque>
#include "STAF.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFConfigService.h"
#include "STAFConfig.h"
#include "STAFConnectionManager.h"
#include "STAFDiagManager.h"
#include "STAFProcessService.h"
#include "STAFServiceManager.h"
#include "STAFThreadManager.h"
#include "STAFTrustManager.h"
#include "STAFTraceService.h"
#include "STAFTrace.h"

static STAFString sHelpMsg;

static const STAFString sIndent("    ");
static const STAFString sCommentDashLine(
    "# ---------------------------------------------------------------------");
static const STAFString sSpace(kUTF8_SPACE);
static const STAFString sContinuation(kUTF8_BSLASH);
static const STAFString sEqual(kUTF8_EQUAL);
static const STAFString sDoubleQuote(kUTF8_DQUOTE);
static const STAFString sEscapedDoubleQuote = STAFString(kUTF8_BSLASH) +
    STAFString(kUTF8_DQUOTE);
static const STAFString sCurrent("CURRENT");
static const STAFString sStartup("STARTUP");

// Maximum line length before adding a space and a continuation charcter
static int sMaxLineLength = 77;

static STAFString sDefaultSSLCACertificate;
static STAFString sDefaultSSLServerCertificate;
static STAFString sDefaultSSLServerKey;

typedef std::deque<STAFString> StringList;

// Local function prototypes
void addMachineNickname(StringList &cfgLines);
void addVariables(StringList &cfgLines, const STAFString upperVars);
void addOperationalParms(StringList &cfgLines);
void addInterfaces(StringList &cfgLines);
void addAuthenticators(StringList &cfgLines);
void addTrust(StringList &cfgLines);
void addNotifications(StringList &cfgLines);
void addServiceLoaderServices(StringList &cfgLines);
void addServices(StringList &cfgLines);
void addTracing(StringList &cfgLines);

// Add double quotes around an input string if it contains one or more
// spaces or double quotes and if the string contains any double quotes,
// escape them (e.g. " ==> \") 
static STAFString quoteString(const STAFString &input)
{
    unsigned int containsDoubleQuote = false;

    if (input.find(sDoubleQuote) != STAFString::kNPos)
        containsDoubleQuote = true;

    if (!containsDoubleQuote && (input.find(sSpace) == STAFString::kNPos))
    {
        // Return input string as no double quotes are needed
        return input;
    }

    if (!containsDoubleQuote)
    {
        // Return input string enclosed in double quotes
        return sDoubleQuote + input + sDoubleQuote;
    }
    else
    {
        // Escape double quotes in input string and enclose in double quotes
        return sDoubleQuote +
            input.replace(sDoubleQuote, sEscapedDoubleQuote) + sDoubleQuote;
    }
}

bool addHeader(StringList &cfgLines, const STAFString &title)
{
    if (cfgLines.size() != 0)
        cfgLines.push_back("");

    cfgLines.push_back(sCommentDashLine);
    cfgLines.push_back("# " + title);
    cfgLines.push_back(sCommentDashLine);
    cfgLines.push_back("");

    return false;  // Used to set first to false
}

STAFConfigService::STAFConfigService() : STAFService("CONFIG")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** CONFIG Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SAVE [FILE <Name>] [VARS <Current | Startup>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers
    
    fSaveParser.addOption("SAVE", 1,
                            STAFCommandParser::kValueNotAllowed);
    fSaveParser.addOption("FILE", 1,
                            STAFCommandParser::kValueRequired);
    fSaveParser.addOption("VARS", 1,
                            STAFCommandParser::kValueRequired);

    // Set default values for some seldom overridden STAFTCP connection
    // provider options (since connection providers don't currently have a
    // way to provide default values for their options).

    sDefaultSSLCACertificate = *gSTAFRootPtr + *gFileSeparatorPtr +
        "bin" + *gFileSeparatorPtr + "CAList.crt";
    sDefaultSSLServerCertificate = *gSTAFRootPtr + *gFileSeparatorPtr +
        "bin" + *gFileSeparatorPtr + "STAFDefault.crt";
    sDefaultSSLServerKey = *gSTAFRootPtr + *gFileSeparatorPtr +
            "bin" + *gFileSeparatorPtr + "STAFDefault.key";
}


STAFServiceResult STAFConfigService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();

    if      (action == "save") return handleSave(requestInfo);
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


STAFServiceResult STAFConfigService::handleSave(
    const STAFServiceRequest &requestInfo)
{
    // Requires at least trust level 3 unless a default auth password is set,
    // then it requires trust level 5 because the output will include a
    // a password.

    if (STAFProcessService::getDefaultAuthPassword().length() == 0)
    {
        IVALIDATE_TRUST(3, "SAVE");
    }
    else
    {
        IVALIDATE_TRUST(5, "SAVE");
    }
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = fSaveParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFRC_t rc = 0;
    STAFString fileName("");

    if (parsedResult->optionTimes("FILE") > 0)
    {
        rc = RESOLVE_STRING_OPTION("FILE", fileName);

        if (rc) return STAFServiceResult(rc, errorBuffer);
        
        if (fileName.length() == 0)
        {
            return STAFServiceResult(kSTAFInvalidValue,
                                     "FILE must specify a valid file name");
        }

        // Verify that the file name does not exist
        
        STAFFSPath path(fileName);

        try
        {
            if (path.exists())
                return STAFServiceResult(
                    kSTAFAlreadyExists, "File " + fileName + " already exists");
        }
        catch (STAFBaseOSErrorException &e)
        {
            STAFString errMsg = "Error checking if file " + fileName +
                "exists" + *gLineSeparatorPtr +
                e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, errMsg);
        }
    }
    
    // Get the value of the VARS option and verify it is valid

    STAFString upperVars = sCurrent;  // This is the default if not specified
    STAFString vars;

    if (parsedResult->optionTimes("VARS") > 0)
    {
        rc = RESOLVE_STRING_OPTION("VARS", vars);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        upperVars = vars.toUpperCase();

        if ((upperVars != sCurrent) && (upperVars != sStartup))
        {
            STAFString errMsg = "Invalid VARS value: " + vars +
                *gLineSeparatorPtr +
                "The value for the VARS option must be " + sCurrent + " or " +
                sStartup + " (case-insensitive)";

            return STAFServiceResult(kSTAFInvalidValue, errMsg); 
        }
    }
    
    // Generate STAF configuration data from the current STAF runtime 
    // configuration
    
    StringList cfgLines;

    addHeader(cfgLines, "STAF Configuration File");
    addMachineNickname(cfgLines);
    addTracing(cfgLines);
    addVariables(cfgLines, upperVars);
    addOperationalParms(cfgLines);
    addInterfaces(cfgLines);
    addAuthenticators(cfgLines);
    addTrust(cfgLines);
    addNotifications(cfgLines);
    addServiceLoaderServices(cfgLines);
    addServices(cfgLines);

    // If the FILE option was specified, write the configuration lines to the
    // specified file.  Otherwise, return the configuration lines in the
    // result.

    if (fileName.length() > 0)
    {
        // Write the configuration file contents to the specified file

        ofstream outFile(fileName.toCurrentCodePage()->buffer(), ios::out); 
        
        if (!outFile)
        {
            return STAFServiceResult(
                kSTAFFileOpenError, "Unable to open file " + fileName);
        }

        StringList::iterator iter;

        for (iter = cfgLines.begin(); iter != cfgLines.end(); ++iter)
        {
            outFile << *iter << endl;
        }

        outFile.close();

        return STAFServiceResult(kSTAFOk);
    }
    else
    {
        // Concatenate each line (adding line endings) to create a string
        // containing the configuration file contents

        STAFString result("");
        StringList::iterator iter;

        for (iter = cfgLines.begin(); iter != cfgLines.end(); ++iter)
        {
            result += *iter + *gLineSeparatorPtr;
        }

        return STAFServiceResult(kSTAFOk, result);
    }

    return STAFServiceResult(kSTAFOk);
}

// Add the MACHINENICKNAME configuration statement if needed
void addMachineNickname(StringList &cfgLines)
{
    // Create a MACHINENICKNAME configuration statement if the value of
    // gMachineNickname is different from the value of gMachine.

    if (*gMachineNicknamePtr != *gMachinePtr)
    {
        STAFString line = "MACHINENICKNAME " + *gMachineNicknamePtr;
        cfgLines.push_back(line);
    }
}

// Add the INTERFACE configuration statements and add a SET DEFAULTINTEFACE
// configuration statement if needed
void addInterfaces(StringList &cfgLines)
{
    // Create an INTERFACE configuration statement for each network interface
    // (not including the "local" interface) in the map returned by
    // gConnectionManagerPtr->getConnectionProviderListCopy().
    // Include any non-default options for each interface.

    // INTERFACE <Name> LIBRARY <Implementation Library> [OPTION <Name[=value]>]...
    
    bool first = true;
    STAFString firstInterface = "local"; // Default if no other interfaces

    STAFConnectionManager::ConnectionProviderList connProvList =
        gConnectionManagerPtr->getConnectionProviderListCopy();
    
    for (STAFConnectionManager::ConnectionProviderList::iterator
         iter = connProvList.begin(); iter != connProvList.end(); ++iter)
    {
        if ((*iter)->getName() == "local")
        {
            // Skip the "local" interface
            continue;
        }

        if (first)
        {
            first = addHeader(cfgLines, "Interfaces (Connection Providers)");
            firstInterface = (*iter)->getName();
        }
        else
        {
            cfgLines.push_back("");
        }

        STAFString library = (*iter)->getLibrary();

        STAFString line = "INTERFACE " + (*iter)->getName() +
            " LIBRARY " + library;
        
        // Get its options
        
        STAFObjectPtr options;
        (*iter)->getOptions(options);

        STAFObjectIteratorPtr iterPtr;

        for (iterPtr = options->keyIterator(); iterPtr->hasNext();)
        {
            STAFString key = iterPtr->next()->asString();
            STAFString value = options->get(key)->asString();

            // The following options for a ssl interface are seldom overridden,
            // so only want to include them if the default values are not used

            if (library == "STAFTCP")
            {
                if (key.isEqualTo("SSL/CACertificate",
                                  kSTAFStringCaseInsensitive))
                {
                    if (value == sDefaultSSLCACertificate)
                        continue; // Skip
                }
                else if (key.isEqualTo("SSL/ServerCertificate",
                                       kSTAFStringCaseInsensitive))
                {
                    if (value == sDefaultSSLServerCertificate)
                        continue; // Skip
                }
                else if (key.isEqualTo("SSL/ServerKey",
                                       kSTAFStringCaseInsensitive))
                {
                    if (value == sDefaultSSLServerKey)
                        continue; // Skip
                }
            }

            STAFString option = quoteString(key + sEqual + value);

            STAFString optionStr = "OPTION " + option;

            if (line.length() + optionStr.length() < sMaxLineLength)
            {
                line += " " + optionStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line);
                line = sIndent + optionStr;
            }
        }

        cfgLines.push_back(line);
    }

    // Get rid of connection provider references
    connProvList = STAFConnectionManager::ConnectionProviderList();

    // SET DEFAULTINTERFACE (after registering interfaces)

    STAFString defaultInterface = 
        gConnectionManagerPtr->getDefaultConnectionProvider();

    // If the default interface is not the first interface, add a
    // SET DEFAULTINTERFACE line

    if (!defaultInterface.isEqualTo(firstInterface,
                                    kSTAFStringCaseInsensitive))
    {
        cfgLines.push_back("");
        STAFString line = "SET DEFAULTINTERFACE " + defaultInterface;
        cfgLines.push_back(line);
    }
}

// Add the configuration statements for any Authenticators and add a
// SET DEFAULTAUTHENTICATOR configuration statement if needed
void addAuthenticators(StringList &cfgLines)
{
    // Create a AUTHENTICATOR configuration statement for each authenticator
    // service.

    // Syntax:
    // AUTHENTICATOR <Name> LIBRARY <Implementation library>
    //     [EXECUTE <Executable>]
    //     [OPTION <Name[=Value]>]... [PARMS <Parameters>]

    STAFServiceManager::OrderedServiceList authenticatorMap =
        gServiceManagerPtr->getAuthenticatorMapCopy();

    STAFServiceManager::OrderedServiceList::iterator iter;
    bool first = true;
    STAFString firstAuthenticator("none"); // Default if no authenticators

    for (iter = authenticatorMap.begin(); iter != authenticatorMap.end();
         iter++)
    {
        if (first)
        {
            first = addHeader(cfgLines, "Authenticators");
            firstAuthenticator = iter->first;
        }
        else
        {
            cfgLines.push_back("");
        }

        STAFString line = "AUTHENTICATOR " + iter->first +
            " LIBRARY " + iter->second->getLibName();
        
        STAFString exec = iter->second->getExecutable();

        if (exec.length() != 0)
        {
            STAFString execStr = "EXECUTE " + quoteString(exec);

            if (line.length() + execStr.length() < sMaxLineLength)
            {
                line += " " + execStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line); 
                line = sIndent + execStr;
            }    
        }
        
        // Get any options for the authenticator

        STAFObjectPtr options = iter->second->getOptions();
        
        for (STAFObjectIteratorPtr iterPtr = options->iterate();
             iterPtr->hasNext();)
        {
            STAFString option = quoteString(iterPtr->next()->asString());

            STAFString optionStr = "OPTION " + option;

            if (line.length() + optionStr.length() < sMaxLineLength)
            {
                line += " " + optionStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line); 
                line = sIndent + optionStr;
            }
        }
        
        // Get any parameters for the external service
        
        STAFString parms = iter->second->getParameters();
        
        if (parms.length() != 0)
        {
            // PARMS will contain spaces if multiple parameters are specified
            // so enclose in double quotes, escaping any double quotes in the
            // parms value

            STAFString parmsStr = STAFString("PARMS ") + quoteString(parms);
            
            if (line.length() + parmsStr.length() < sMaxLineLength)
            {
                line += " " + STAFString(parmsStr);
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line);
                line = sIndent + STAFString(parmsStr);
            }
        }
        
        cfgLines.push_back(line);
    }

    // Get rid of connection provider references
    authenticatorMap = STAFServiceManager::OrderedServiceList();
    
    // SET DEFAULTAUTHENTICATOR (after registering authenticators):

    STAFString defaultAuthenticator =
        gServiceManagerPtr->getDefaultAuthenticator();

    // If the default authenticator is not the first authenticator, add
    // a SET DEFAULTAUTHENTICATOR line

    if (!defaultAuthenticator.isEqualTo(firstAuthenticator,
                                        kSTAFStringCaseInsensitive))
    {
        cfgLines.push_back("");
        STAFString line = "SET DEFAULTAUTHENTICATOR " + defaultAuthenticator;
        cfgLines.push_back(line);
    }
}

// Add the configuration statements for any Operational Parameters that are
// not set to their default value.
void addOperationalParms(StringList &cfgLines)
{
    bool first = true;
    STAFString line;

    // CONNECTATTEMPTS

    if (gConnectionAttempts != gDefaultConnectionAttempts)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET CONNECTATTEMPTS " + STAFString(gConnectionAttempts);
        cfgLines.push_back(line);
    }

    // CONNECTRETRYDELAY

    if (gConnectionRetryDelay != gDefaultConnectionRetryDelay)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET CONNECTRETRYDELAY " + STAFString(gConnectionRetryDelay);
        cfgLines.push_back(line);
    }

    // DATADIR:

    if (*gSTAFWriteLocationPtr != gDefaultSTAFWriteLocation)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DATADIR " + quoteString(*gSTAFWriteLocationPtr);
        cfgLines.push_back(line);
    }

    // DEFAULTAUTHDISABLEDACTION - Default is Ignore
    
    if (STAFProcessService::getDefaultDisabledAuthAction() !=
        kSTAFProcessDisabledAuthIgnore)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTAUTHDISABLEDACTION " +
            STAFProcessService::getDefaultDisabledAuthActionAsString();
        cfgLines.push_back(line);
    }

    // Set the DEFAULTAUTHENTICATOR (if needed) after adding the
    // authenticators because the default authenticator must already exist

    // DEFAULTAUTHPASSWORD - The default is an empty string

    STAFString password = STAFProcessService::getDefaultAuthPassword();
    
    if (password.length() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTAUTHPASSWORD " + quoteString(password);
        cfgLines.push_back(line);
    }

    // DEFAULTAUTHUSERNAME - The default is an empty string

    STAFString username = STAFProcessService::getDefaultAuthUsername();
    
    if (username.length() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTAUTHUSERNAME " + quoteString(username);
        cfgLines.push_back(line);
    }

    // DEFAULTFOCUS - The default focus is Background

    if (STAFProcessService::getDefaultConsoleFocus() !=
        kSTAFProcessBackground)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTFOCUS " +
            STAFProcessService::getDefaultConsoleFocusAsString();
        cfgLines.push_back(line);
    }

    // Set the DEFAULTINTERFACE (if needed) after adding the interfaces
    // because the default interface must already exist

    // DEFAULTNEWCONSOLE | DEFAULTSAMECONSOLE

    if (gDefaultConsoleMode != gDefaultDefaultConsoleMode)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        if (gDefaultConsoleMode == kSTAFProcessNewConsole)
            line = "SET DEFAULTNEWCONSOLE";
        else if (gDefaultConsoleMode == kSTAFProcessSameConsole)
            line = "SET DEFAULTSAMECONSOLE";

        cfgLines.push_back(line);
    }

    // DEFAULTNEWCONSOLESHELL - The default is an empty string

    STAFString shell = STAFProcessService::getDefaultNewConsoleShell();
    
    if (shell.length() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTNEWCONSOLESHELL " + quoteString(shell);
        cfgLines.push_back(line);
    }

    // DEFAULTSAMECONSOLESHELL - The default is an empty string

    shell = STAFProcessService::getDefaultSameConsoleShell();
    
    if (shell.length() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTSAMECONSOLESHELL " + quoteString(shell);
        cfgLines.push_back(line);
    }

    // DEFAULTSHELL - The default is an empty string

    shell = STAFProcessService::getDefaultShellCommand();
    
    if (shell.length() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTSHELL " + quoteString(shell);
        cfgLines.push_back(line);
    }

    // DEFAULTSTOPUSING

    if (STAFProcessService::getDefaultStopMethod() != gDefaultProcessStopMethod)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET DEFAULTSTOPUSING " +
            STAFProcessService::getDefaultStopMethodAsString();
        cfgLines.push_back(line);
    }

    // ENABLEDIAGS - The default is disabled

    if (gDiagManagerPtr->getEnabled())
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET ENABLEDIAGS";
        cfgLines.push_back(line);
    }

    // HANDLEGCINTERVAL

    if (gHandleGCInterval != gDefaultHandleGCInterval)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET HANDLEGCINTERVAL " + STAFString(gHandleGCInterval);
        cfgLines.push_back(line);
    }

    // INITIALTHREADS

    if (gNumInitialThreads != gDefaultNumInitialThreads)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET INITIALTHREADS " + STAFString(gNumInitialThreads);
        cfgLines.push_back(line);
    }

    // INTERFACECYCLING - The default is enabled

    if (!gConnectionManagerPtr->getAutoInterfaceCycling())
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET INTERFACECYCLING DISABLED";
        cfgLines.push_back(line);
    }

    // MAXQUEUESIZE

    if (gMaxQueueSize != gDefaultMaxQueueSize)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET MAXQUEUESIZE " + STAFString(gMaxQueueSize);
        cfgLines.push_back(line);
    }

    // MAXRETURNFILESIZE
    
    if (gMaxReturnFileSize != gDefaultMaxReturnFileSize)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET MAXRETURNFILESIZE " + STAFString(gMaxReturnFileSize);
        cfgLines.push_back(line);
    }
    
    // PROCESSAUTHMODE - The default is disabled

    if (STAFProcessService::getAuthMode() != kSTAFProcessAuthDisabled)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET PROCESSAUTHMODE " +
            STAFProcessService::getAuthModeAsString();
        cfgLines.push_back(line);
    }

    // RESULTCOMPATIBILITYMODE

    if (gResultCompatibilityMode != gDefaultResultCompatibilityMode)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        STAFString mode;

        if (gResultCompatibilityMode == kSTAFResultCompatibilityVerbose)
            mode = "Verbose";
        else
            mode = "None";

        line = "SET RESULTCOMPATIBILITYMODE " + mode;
        cfgLines.push_back(line);
    }

    // STRICTFSCOPYTRUST

    if (gStrictFSCopyTrust &&
        (gStrictFSCopyTrust != gDefaultStrictFSCopyTrust))
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET STRICTFSCOPYTRUST";
        cfgLines.push_back(line);
    }

    // THREADGROWTHDELTA - The default is 1

    unsigned int growthDelta = gThreadManagerPtr->getGrowthDelta();

    if (growthDelta != 1)
    {
        if (first)
            first = addHeader(cfgLines, "Operational Parameters");

        line = "SET THREADGROWTHDELTA " + STAFString(growthDelta);
        cfgLines.push_back(line);
    }
}

// Add the configuration statements for any Service Loader Services
void addServiceLoaderServices(StringList &cfgLines)
{
    // Create a SERVICELOADER configuration statement for each service loader
    // Syntax:
    // SERVICELOADER LIBRARY <Implementation library> [EXECUTE <Executable>]
    //     [OPTION <Name[=Value]>]... [PARMS <Parameters>]

    STAFServiceManager::ServiceList serviceLoaderList =
        gServiceManagerPtr->getSLSListCopy();

    STAFServiceManager::ServiceList::iterator iter;
    bool first = true;

    for (iter = serviceLoaderList.begin(); iter != serviceLoaderList.end();
         iter++)
    {
        if (first)
            first = addHeader(cfgLines, "Service Loader Service Registrations");
        else
            cfgLines.push_back("");

        STAFString line = "SERVICELOADER LIBRARY " + (*iter)->getLibName();

        STAFString exec = (*iter)->getExecutable();

        if (exec.length() != 0)
        {
            STAFString execStr = "EXECUTE " + quoteString(exec);

            if (line.length() + execStr.length() < sMaxLineLength)
            {
                line += " " + execStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line); 
                line = sIndent + execStr;
            }    
        }

        // Get any options for the service loader service

        STAFObjectPtr options = (*iter)->getOptions();

        for (STAFObjectIteratorPtr iterPtr = options->iterate();
             iterPtr->hasNext();)
        {
            STAFString option = quoteString(iterPtr->next()->asString());

            STAFString optionStr = "OPTION " + option;

            if (line.length() + optionStr.length() < sMaxLineLength)
            {
                line += " " + optionStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line); 
                line = sIndent + optionStr;
            }
        }
        
        // Get any parameters for the service loader service
        
        STAFString parms = (*iter)->getParameters();

        if (parms.length() != 0)
        {
            STAFString parmsStr = STAFString("PARMS ") + quoteString(parms);

            if (line.length() + parmsStr.length() < sMaxLineLength)
            {
                line += " " + parmsStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line);
                line = sIndent + parmsStr;
            }
        }
        
        cfgLines.push_back(line);
    }

    // Get rid of service loader references
    serviceLoaderList = STAFServiceManager::ServiceList();
}

// Add the configuration statements for any registered External Services
void addServices(StringList &cfgLines)
{
    // Create a SERVICE configuration statement for each external or delegated
    // service in the service list, unless the service was loaded by a service
    // loader service.

    // Note:  The gServiceManagerPtr->getServiceListCopy() method returns a
    // list of the service in the order they were added.  We need to add the
    // services in that same order.

    // Syntax:
    //
    // SERVICE <Name> LIBRARY <Implementation library> [EXECUTE <Executable>]
    //     [OPTION <Name[=Value]>]... [PARMS <Parameters>]
    // or
    // SERVICE <Name> DELEGATE <Machine> [TONAME <Remote Service Name>]
    
    STAFServiceManager::ServiceList serviceList =
        gServiceManagerPtr->getServiceListCopy();
    STAFServiceManager::ServiceList::iterator iter;
    bool first = true;

    for (iter = serviceList.begin(); iter != serviceList.end(); iter++)
    {
        // Get the library to determine the type of service (e.g. Internal,
        // External, Delegated)

        STAFString library = (*iter)->getLibName();

        if ((library == "<Internal>") || ((*iter)->getLoadedBySLS() != ""))
        {
            // Ignore internal services and ignore services loaded by a
            // service loader service
            continue;
        }

        if (first)
            first = addHeader(cfgLines, "Service Registrations");
        else
            cfgLines.push_back("");

        STAFString line = "SERVICE " + (*iter)->name();

        if (library == "<Delegated>")
        {
            // Delegated service syntax:
            //
            // SERVICE <Name> DELEGATE <Machine> [TONAME <Remote Service Name>]

            // Get the name of the machine and the service name that the
            // service was delegated to from the service info() method
            // which returns the following string for delegated services:
            // <serviceName>: Delegated to <toName> on <machine>

            STAFString info = (*iter)->info();
            STAFString toServiceString(": Delegated to ");
            STAFString toMachineString(" on ");

            unsigned int index1 = info.find(toServiceString);
            unsigned int index2 = info.find(toMachineString);

            if (index1 != STAFString::kNPos && index2 != STAFString::kNPos)
            {
                STAFString toService = info.subString(0, index1);
                STAFString toMachine = info.subString(
                    index2 + toMachineString.length());

                line += " DELEGATE " + toMachine;

                if (toService != (*iter)->name())
                {
                    // Add the TONAME option
                    line += " TONAME " + toService;
                }
            }
            else
            {
                // Cannot get delegated service information so log a warning
                // tracepoint message and ignore

                // Log a warning tracepoint message

                STAFString warningMsg = STAFString(
                    "STAFConfigService::addServices() - Error ") +
                    "adding a SERVICE line for delegated service " +
                    (*iter)->name() + " because its info() method " +
                    "returned a string in an unknown format.  info()=" +
                    (*iter)->info();

                STAFTrace::trace(kSTAFTraceWarning, warningMsg);

                continue;
            }
        }
        else
        {
            // External service

            line += " LIBRARY " + library;
        }
        
        STAFString exec = (*iter)->getExecutable();

        if (exec.length() != 0)
        {
            STAFString execStr = "EXECUTE " + quoteString(exec);

            if (line.length() + execStr.length() < sMaxLineLength)
            {
                line += " " + execStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line); 
                line = sIndent + execStr;
            }    
        }
        
        // Get any options for the external service

        STAFObjectPtr options = (*iter)->getOptions();
        
        for (STAFObjectIteratorPtr iterPtr = options->iterate();
             iterPtr->hasNext();)
        {
            STAFString option = quoteString(iterPtr->next()->asString());

            STAFString optionStr = "OPTION " + option;

            if (line.length() + optionStr.length() < sMaxLineLength)
            {
                line += " " + optionStr;
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line); 
                line = sIndent + optionStr;
            }
        }
        
        // Get any parameters for the external service
        
        STAFString parms = (*iter)->getParameters();
        
        if (parms.length() != 0)
        {
            // PARMS will contain spaces if multiple parameters are specified
            // so enclose in double quotes, escaping any double quotes in the
            // parms value

            STAFString parmsStr = STAFString("PARMS ") + quoteString(parms);
            
            if (line.length() + parmsStr.length() < sMaxLineLength)
            {
                line += " " + STAFString(parmsStr);
            }
            else
            {
                line += " " + sContinuation;
                cfgLines.push_back(line);
                line = sIndent + STAFString(parmsStr);
            }
        }
        
        cfgLines.push_back(line);
    }

    // Get rid of service list references
    serviceList = STAFServiceManager::ServiceList();
}

// Add the configuration statements for Trust
void addTrust(StringList &cfgLines)
{
    bool first = true;
    STAFString line;

    // Add a default trust line if not using default trust level 3
    // Syntax:  TRUST LEVEL <Level> DEFAULT
    
    unsigned int defaultTrustLevel =
        gTrustManagerPtr->getDefaultTrusteeLevel();
    
    if (defaultTrustLevel != 3)
    {
        if (first)
            first = addHeader(cfgLines, "Trust Levels");

        line = "TRUST LEVEL " + STAFString(defaultTrustLevel) + " DEFAULT";
        cfgLines.push_back(line);
    }

    // Create a machine trust configuration line for each entry in the
    // Machine Trust Map.
    // Syntax:  TRUST LEVEL <Level> MACHINE <Machine>

    STAFTrustManager::TrustMap trustMap = 
        gTrustManagerPtr->getMachineTrustMapCopy();
    STAFTrustManager::TrustMap::iterator iter;

    if (trustMap.size() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Trust Levels");
        else
            cfgLines.push_back("");

        for (iter = trustMap.begin(); iter != trustMap.end(); iter++)
        {
            line = "TRUST LEVEL " + STAFString(iter->second.trustLevel) +
                " MACHINE " + iter->second.group + gSpecSeparator +
                iter->second.entity;
            cfgLines.push_back(line);
        }
    }

    // Create a user trust configuration line for each entry in the
    // User Trust Map.
    // Syntax:  TRUST LEVEL <Level> MACHINE <Machine>

    trustMap = gTrustManagerPtr->getUserTrustMapCopy();

    if (trustMap.size() != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Trust Levels");
        else
            cfgLines.push_back("");

        for (iter = trustMap.begin(); iter != trustMap.end(); iter++)
        {
            line = "TRUST LEVEL " +  STAFString(iter->second.trustLevel) +
                " USER " + quoteString(iter->second.group + gSpecSeparator +
                                       iter->second.entity);
            cfgLines.push_back(line);
        }
    }
}

// Add the configuration statements for NOTIFY ONSTART and NOTIFY ONSHUTDOWN
// Syntax:
// NOTIFY <ONSTART | ONSHUTDOWN> MACHINE <Machine>
//        [PRIORITY <Priority>] <NAME <Name> | HANDLE <Handle>>
void addNotifications(StringList &cfgLines)
{
    // Add a NOTIFY ONSTART configuration statement for each entry in the
    // list returned by gNotifyOnStartPtr::getNotifieeListCopy().
    
    STAFNotificationList::NotifieeList notifieeList(
        gNotifyOnStartPtr->getNotifieeListCopy());

    STAFNotificationList::NotifieeList::iterator iter;
    bool first = true;

    for (iter = notifieeList.begin(); iter != notifieeList.end(); iter++)
    {
        if (first)
            first = addHeader(cfgLines, "Notifications");
        else
            cfgLines.push_back("");

        STAFNotificationList::Notifiee notifiee = *iter;

        STAFString line = "NOTIFY ONSTART MACHINE " +
            quoteString(notifiee.machine);

        // The default priority is 5

        if (notifiee.priority != 5)
            line += " PRIORITY " + STAFString(notifiee.priority);

        if (notifiee.nameOrHandle == STAFNotificationList::Notifiee::kName)
            line += " NAME " + quoteString(notifiee.process);
        else
            line += " HANDLE " + STAFString(notifiee.handle);
        
        cfgLines.push_back(line);
    }

    // Add a NOTIFY ONSHUTDOWN configuration statement for each entry in
    // the list returned by gNotifyOnShutdownPtr::getNotifieeListCopy().

    notifieeList = gNotifyOnShutdownPtr->getNotifieeListCopy();
    
    for (iter = notifieeList.begin(); iter != notifieeList.end(); iter++)
    {
        if (first)
            first = addHeader(cfgLines, "Notifications");
        else
            cfgLines.push_back("");

        STAFNotificationList::Notifiee notifiee = *iter;

        STAFString line = "NOTIFY ONSHUTDOWN MACHINE " +
            quoteString(notifiee.machine);

        // The default priority is 5

        if (notifiee.priority != 5)
            line += " PRIORITY " + STAFString(notifiee.priority);

        if (notifiee.nameOrHandle == STAFNotificationList::Notifiee::kName)
            line += " NAME " + quoteString(notifiee.process);
        else
            line += " HANDLE " + STAFString(notifiee.handle);

        cfgLines.push_back(line);
    }
}

// Add the configuration statements for TRACE SET DEFAULTSERVICESTATE Disabled
// if the trace default service state is not Enabled (the default)
void addTraceDefaultServiceState(StringList &cfgLines)
{
    bool first = true;

    // Check if the Default Trace State is not Enabled (its default state) and
    // if not, add line TRACE SET DEFAULTSERVICESTATE Disabled

    if (STAFServiceManager::getDefaultTraceState() !=
        STAFServiceManager::kTraceEnabled)
    {
        cfgLines.push_back("");
        STAFString line = "TRACE SET DEFAULTSERVICESTATE Disabled";
        cfgLines.push_back(line);
    }
}

// Add the configuration statements for TRACE service settings
// Syntax:
//   TRACE SET DESTINATION TO < [STDOUT | STDERR] [FILE <File name> [APPEND]] >
//   TRACE SET DEFAULTSERVICESTATE <Enabled | Disabled>   
//   TRACE ENABLE ALL SERVICES
//   TRACE DISABLE SERVICES <Service list>
//   TRACE SET MAXSERVICERESULTSIZE <Number>[k|m]
//   TRACE ENABLE TRACEPOINTS <Trace point list>
void addTracing(StringList &cfgLines)
{
    bool first = true;

    // Check if the trace destination is not STDOUT (the default) and if not,
    // add line TRACE SET DESTINATION TO < [STDOUT | STDERR] [FILE <File name> [APPEND]]>

    STAFString traceFilename;

    STAFTraceDestination_e traceDestination = 
        STAFTrace::getTraceDestination(traceFilename);

    if (traceDestination != kSTAFTraceToStdout)
    {
        if (first)
            first = addHeader(cfgLines, "Tracing");

        STAFString line = "TRACE SET DESTINATION TO ";
        
        if (traceDestination == kSTAFTraceToStderr)
        {
            line += "STDERR";
        }
        else
        {
            // Trace destination is a file (and may be to stdout or stderr)

            if (traceDestination == kSTAFTraceToStdoutAndFile)
                line += "STDOUT ";
            else if (traceDestination == kSTAFTraceToStderrAndFile)
                line += "STDERR ";

            line += "FILE " + traceFilename;

            STAFTraceFileMode_e traceFileMode = STAFTrace::getTraceFileMode();

            if (traceFileMode == kSTAFTraceFileAppend)
                line += " APPEND";
        }
        
        cfgLines.push_back(line);
    }
    
    // Check if MAXSERVICERESULTSIZE != 0 (the default), and if not, add line 
    // TRACE SET MAXSERVICERESULTSIZE <Number>

    unsigned int maxServiceResultSize =
        STAFServiceManager::getMaxServiceResultSize();

    if (maxServiceResultSize != 0)
    {
        if (first)
            first = addHeader(cfgLines, "Tracing");

        STAFString line = "TRACE SET MAXSERVICERESULTSIZE " +
            STAFString(maxServiceResultSize);

        cfgLines.push_back(line);
    }

    // Check if the Default Trace State is not Enabled (its default state) and
    // if not, add line TRACE SET DEFAULTSERVICESTATE Disabled

    if (STAFServiceManager::getDefaultTraceState() !=
        STAFServiceManager::kTraceEnabled)
    {
        if (first)
            first = addHeader(cfgLines, "Tracing");

        STAFString line = "TRACE SET DEFAULTSERVICESTATE Disabled";
        
        cfgLines.push_back(line);
    }
    
    // For any services whose trace mode is not the default trace state,
    // set its tracing state.

    bool allServicesEnabled = true;
    bool allServicesDisabled = true;

    STAFString enabledServices("");
    STAFString disabledServices("");

    STAFServiceManager::ServiceTraceStatusList serviceTraceStatusList =
        STAFServiceManager::getServiceTraceStatusList();
    STAFServiceManager::OrderedServiceList allServiceList =
         gServiceManagerPtr->getOrderedServiceListCopy();
    STAFServiceManager::ServiceTraceStatusList::iterator traceStatusIter;

    for (traceStatusIter = serviceTraceStatusList.begin();
         traceStatusIter != serviceTraceStatusList.end(); traceStatusIter++)
    {
        if (traceStatusIter->second == STAFServiceManager::kTraceEnabled)
        {
            allServicesDisabled = false;
            enabledServices += STAFString(traceStatusIter->first) + " ";
        }
        else if (traceStatusIter->second == STAFServiceManager::kTraceDisabled)
        {
            allServicesEnabled = false;
            disabledServices += STAFString(traceStatusIter->first) + " ";
        }
    }

    if (enabledServices.length() > 0)
    {
        if (first)
            first = addHeader(cfgLines, "Tracing");
        else
            cfgLines.push_back("");

        STAFString line = "TRACE ENABLE ";

        if (allServicesEnabled)
            line += "ALL SERVICES";
        else
            line += "SERVICES " + quoteString(enabledServices.strip());

        cfgLines.push_back(line);
    }
    
    if (disabledServices.length() > 0)
    {
        if (first)
            first = addHeader(cfgLines, "Tracing");
        else
            cfgLines.push_back("");

        STAFString line = "TRACE DISABLE ";

        if (allServicesDisabled)
            line += "ALL SERVICES";
        else
            line += "SERVICES " + quoteString(disabledServices.strip());

        cfgLines.push_back(line);
    }

    // Check if any of the tracepoints are enabled (all are disabled by
    // default) and if so, add line TRACE ENABLE TRACEPOINTS <Tracepoint list>

    STAFString enabledTracepoints("");
    STAFTraceService::STAFTracepointMap::iterator tracepointMapIter;

    for (tracepointMapIter = STAFTraceService::kSTAFTracepointMap.begin();
         tracepointMapIter != STAFTraceService::kSTAFTracepointMap.end();
         tracepointMapIter++)
    {
        if (STAFTrace::doTrace(tracepointMapIter->first))
            enabledTracepoints += tracepointMapIter->second + " ";
    }

    if (enabledTracepoints.length() > 0)
    {
        if (first)
            first = addHeader(cfgLines, "Tracing");
        else
            cfgLines.push_back("");

        STAFString line = "TRACE ENABLE TRACEPOINTS " +
            quoteString(enabledTracepoints.strip());

        cfgLines.push_back(line);
    }
}


// Get the configuration statements for system and shared variables
// Syntax:  SET [SYSTEM | SHARED] VAR <Name=Value> [VAR <Name=Value>] ...
void addVariables(StringList &cfgLines, const STAFString upperVars)
{
    if (upperVars == sCurrent)
    {
        // Add configuration statements for the current system and shared
        // variables

        STAFVariablePool::VariableMap varMap =
            ((STAFVariablePool*)*gGlobalVariablePoolPtr)->getVariableMapCopy();

        // Add a SET SYSTEM VAR configuration statement for each local system
        // variable (not including system variables whose name begins with
        // "STAF/Config/" or "STAF/Env/" or whose name is "STAF/DataDir" or
        // "STAF/Version") as these system variables are set by STAFProc.

        STAFVariablePool::VariableMap::iterator iter;
        bool first = true;

        for (iter = varMap.begin(); iter != varMap.end(); iter++)
        {
            const STAFVariablePool::Variable &var = iter->second;

            if ((var.name == "STAF/DataDir") ||
                (var.name == "STAF/Version") ||
                var.name.startsWith("STAF/Config/") ||
                var.name.startsWith("STAF/Env/"))
            {
                continue;  // Skip
            }
        
            if (first)
                first = addHeader(cfgLines, "Variables");

            STAFString line = "SET SYSTEM VAR " +
                quoteString(var.name + sEqual + var.value);

            cfgLines.push_back(line);
        }

        // Add a SET SHARED VAR configuration statement for each local shared
        // variable.

        varMap = ((STAFVariablePool*)*gSharedVariablePoolPtr)->getVariableMapCopy();
        first = true;

        for (iter = varMap.begin(); iter != varMap.end(); iter++)
        {
            if (first)
            {
                first = false;
                cfgLines.push_back("");
            }

            const STAFVariablePool::Variable &var = iter->second;

            STAFString line = "SET SHARED VAR " +
                quoteString(var.name + sEqual + var.value);

            cfgLines.push_back(line);
        }
    }
    else if (upperVars == sStartup)
    {
        // Add configuration statements for the SET VAR lines in the STAF cfg
        // file at startup

        std::deque<STAFString> setVarLines = getSetVarLines();

        bool first = true;
        StringList::iterator iter;

        for (iter = setVarLines.begin(); iter != setVarLines.end(); ++iter)
        {
            if (first)
                first = addHeader(cfgLines, "Variables");

            cfgLines.push_back(*iter);
        }
    }
}

STAFServiceResult STAFConfigService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}


STAFString STAFConfigService::info(unsigned int) const
{
    return name() + ": Internal";
}

STAFConfigService::~STAFConfigService()
{
    /* Do Nothing */
}
