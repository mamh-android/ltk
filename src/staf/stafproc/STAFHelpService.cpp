/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFHelpService.h"
#include "STAFUtil.h"

// REGISTER SERVICE <Name> ERROR <#> INFO <String> DESCRIPTION <String>
//
// UNREGISTER SERVICE <Name> ERROR <#>
//
// [SERVICE <Name>] ERROR <#>
//
// LIST [SERVICE <Name>] [ERRORS]
//
// HELP

struct HelpMessageInfo
{
    STAFRC_t rc;
    const char *info;
    const char *description;
};

static const STAFString sErrorInfoClassName = "STAF/Service/Help/ErrorInfo";
static const STAFString sErrorDetailsClassName =
    "STAF/Service/Help/ErrorDetails";
static const STAFString sServiceErrorClassName = 
    "STAF/Service/Help/ServiceError";
static STAFString sHelpMsg;

STAFMapClassDefinitionPtr fErrorInfoClass;
STAFMapClassDefinitionPtr fErrorDetailsClass;
STAFMapClassDefinitionPtr fServiceErrorClass;
 
STAFHelpService::STAFHelpService() : STAFService("HELP")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** HELP Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "REGISTER   SERVICE <Name> ERROR <Number> INFO <String> "
        "DESCRIPTION <String>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "UNREGISTER SERVICE <Name> ERROR <Number>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "[SERVICE <Name>] ERROR <Number>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST SERVICES | [SERVICE <Name>] ERRORS" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP" +
        *gLineSeparatorPtr;

    // Create the command request parsers

    fRegParser.addOption("REGISTER",    1, STAFCommandParser::kValueNotAllowed);
    fRegParser.addOption("UNREGISTER",  1, STAFCommandParser::kValueNotAllowed);
    fRegParser.addOption("SERVICE",     1, STAFCommandParser::kValueRequired);
    fRegParser.addOption("ERROR",       1, STAFCommandParser::kValueRequired);
    fRegParser.addOption("INFO",        1, STAFCommandParser::kValueRequired);
    fRegParser.addOption("DESCRIPTION", 1, STAFCommandParser::kValueRequired);

    fRegParser.addOptionGroup("REGISTER UNREGISTER", 0, 1);
    fRegParser.addOptionGroup("UNREGISTER INFO", 0, 1);
    fRegParser.addOptionGroup("UNREGISTER DESCRIPTION", 0, 1);

    fRegParser.addOptionNeed("REGISTER", "INFO");
    fRegParser.addOptionNeed("REGISTER", "DESCRIPTION");

    fListParser.addOption("LIST",     1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SERVICE",  1, STAFCommandParser::kValueRequired);
    fListParser.addOption("ERRORS",   1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SERVICES", 1, STAFCommandParser::kValueNotAllowed);

    fListParser.addOptionGroup("SERVICES ERRORS", 0, 1);
    fListParser.addOptionGroup("SERVICE SERVICES", 0, 1);

    fErrorParser.addOption("SERVICE", 1, STAFCommandParser::kValueRequired);
    fErrorParser.addOption("ERROR",   1, STAFCommandParser::kValueRequired);

    fErrorParser.addOptionNeed("SERVICE", "ERROR");

    // Construct map class for a error info

    fErrorInfoClass = STAFMapClassDefinition::create(sErrorInfoClassName);
    fErrorInfoClass->addKey("returnCode", "Return Code");
    fErrorInfoClass->addKey("description", "Description");

    // Construct map class for a detailed error information for a single error

    fErrorDetailsClass = STAFMapClassDefinition::create(
        sErrorDetailsClassName);
    fErrorDetailsClass->addKey("description", "Description");
    fErrorDetailsClass->addKey("details", "Details");

    // Construct map class for a service error info

    fServiceErrorClass = STAFMapClassDefinition::create(
        sServiceErrorClassName);
    fServiceErrorClass->addKey("service", "Service");
    fServiceErrorClass->addKey("description", "Description");
    fServiceErrorClass->addKey("details",     "Details");

    HelpMessageInfo helpMsgs[] =
    {
        { kSTAFOk, "No error",
          "No error" },
        { kSTAFInvalidAPI, "Invalid API",
          "This indicates that a process has tried to call an invalid "
          "internal STAF API.  If this error occurs, "
          "report it to the authors.{STAF/Config/Sep/Line}" },
        { kSTAFUnknownService, "Unknown service",
          "You have tried to submit a request to a service that is "
          "unknown to STAFProc.  Verify that you "
          "have correctly registered the service.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidHandle, "Invalid handle",
          "You are passing an invalid handle to a STAF API.  Ensure that "
          "you are using the handle you received "
          "when you registered with STAF.{STAF/Config/Sep/Line}" },
        { kSTAFHandleAlreadyExists, "Handle already exists",
          "This indicates an internal STAF error. If this error occurs, "
          "report it to the authors."
          "{STAF/Config/Sep/Line}" },
        { kSTAFHandleDoesNotExist, "Handle does not exist",
          "You are trying to perform an operation on a handle that does "
          "not exist.  For example, you may be "
          "trying to stop a process, but you are specifying the "
          "wrong handle.{STAF/Config/Sep/Line}" },
        { kSTAFUnknownError, "Unknown error",
          "An unknown error has occurred.  This error is usually an "
          "indication of an internal STAF error.  "
          "If this error occurs, report it the authors."
          "{STAF/Config/Sep/Line}" },
        { kSTAFInvalidRequestString, "Invalid request string",
          "You have submitted an improperly formatted request to a "
          "service.  See the appropriate section "
          "in STAF User's Guide for the syntax of the service's "
          "requests, or contact the provider of "
          "the service.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidServiceResult, "Invalid service result",
          "This indicates an internal error with the service to which a "
          "request was submitted.  If this error "
          "occurs, report it to the authors and the service "
          "provider.{STAF/Config/Sep/Line}" },
        { kSTAFREXXError, "REXX Error",
          "This indicates an internal error in an external Rexx "
          "service.  If this error occurs, report "
          "it to the authors and the service provider."
          "{STAF/Config/Sep/Line}" },
        { kSTAFBaseOSError, "Base operating system error",
          "This indicates that a base operating system error was "
          "encountered.  The actual base operating system error code, "
          "and possibly additional information about the error, will be "
          "returned in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFProcessAlreadyComplete, "Process already complete",
          "You are trying to stop a specific process that has either "
          "already been stopped or has finished "
          "execution on its own.{STAF/Config/Sep/Line}" },
        { kSTAFProcessNotComplete, "Process not complete",
          "You are trying to free process information for a process that "
          "is still executing.{STAF/Config/Sep/Line}" },
        { kSTAFVariableDoesNotExist, "Variable does not exist",
          "You are trying to resolve, get or delete a variable that does "
          "not exist.{STAF/Config/Sep/Line}" },
        { kSTAFUnResolvableString, "Unresolvable string",
          "You have requested to resolve a string that cannot be resolved.  "
          "This indicates that you have "
          "exceeded the resolution depth of the VAR service.  "
          "The most common cause of this is recursive "
          "variables definitions.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidResolveString, "Invalid resolve string",
          "The string you requested to be resolved has a non-matching left "
          "or right curly brace.  Ensure that all "
          "variable references have both left and right "
          "curly braces.{STAF/Config/Sep/Line}" },
        { kSTAFNoPathToMachine, "No path to endpoint",
          "This indicates that STAFProc was not able to submit the request"
          "{STAF/Config/Sep/Line}"
          "to the requested endpoint (i.e. target machine).  This error usually"
          "{STAF/Config/Sep/Line}"
          "indicates one or more of the following:"
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}"
          "1. STAFProc is not running on the target machine."
          "{STAF/Config/Sep/Line}"
          "2. The requested endpoint is not valid."
          "{STAF/Config/Sep/Line}"
          "3. The network interface or port for the requested endpoint is not supported."
          "{STAF/Config/Sep/Line}"
          "4. A firewall is blocking communication via the port for the requested"
          "{STAF/Config/Sep/Line}"
          "   endpoint."
          "{STAF/Config/Sep/Line}"
          "5. A secure network interface is being used to communicate to a machine that"
          "{STAF/Config/Sep/Line}"
          "   doesn't have a secure network interface configured with the same"
          "{STAF/Config/Sep/Line}"
          "   certificate."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}"
          "Alternatively, you may need to increase your CONNECTTIMEOUT value for the"
          "{STAF/Config/Sep/Line}"
          "network interface and/or increase your CONNECTATTEMPTS value in your"
          "{STAF/Config/Sep/Line}"
          "STAF.cfg file.{STAF/Config/Sep/Line}" },
        { kSTAFFileOpenError, "File open error",
          "This indicates that there was an error opening the requested "
          "file.  Some possible explanations are that "
          "the file/path does not exist, contains "
          "invalid characters, or is locked.{STAF/Config/Sep/Line}" },
        { kSTAFFileReadError, "File read error",
          "This indicates that there was an error while trying to read "
          "data from a file.{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}"
          "Note: Additional information regarding which file could not be "
          "read may be provided in the result passed back "
          "from the submit call.{STAF/Config/Sep/Line}" },
        { kSTAFFileWriteError, "File write error",
          "This indicates that there was an error while trying to write "
          "data to a file.{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: "
          "Additional information regarding which file could not be written to "
          "may be provided in the result passed back from "
          "the submit call.{STAF/Config/Sep/Line}" },
        { kSTAFFileDeleteError, "File delete error",
          "This indicates that there was an error while trying to delete "
          "a file or directory.{STAF/Config/Sep/Line}"
          "{STAF/Config/Sep/Line}Note: Additional information regarding which "
          "file or directory could not be deleted may be "
          "provided in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFNotRunning, "STAF not running",
          "This indicates that STAFProc is not running on the local machine"
          "{STAF/Config/Sep/Line}"
          "with the same STAF_INSTANCE_NAME (and/or the same STAF_TEMP_DIR if on a Unix"
          "{STAF/Config/Sep/Line}"
          "machine).{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}"
          "Notes:{STAF/Config/Sep/Line}"
          "1. If the STAF_INSTANCE_NAME environment variable is not set, it defaults"
          "{STAF/Config/Sep/Line}"
          "   to \"STAF\".{STAF/Config/Sep/Line}"
          "2. On Unix, if the STAF_TEMP_DIR environment variable is not set, it defaults"
          "{STAF/Config/Sep/Line}"
          "   to \"/tmp\".  This environment variable is not used on Windows."
          "{STAF/Config/Sep/Line}"
          "3. This error can also occur when submitting a request using the local IPC"
          "{STAF/Config/Sep/Line}"
          "   interface on a Unix machine if the socket file that the local interface"
          "{STAF/Config/Sep/Line}"
          "   uses has been inadvertently deleted."
          "{STAF/Config/Sep/Line}"
          "4. To get more information on this error, set special environment variable"
          "{STAF/Config/Sep/Line}"
          "   STAF_DEBUG_21=1 and resubmit your local STAF service request."
          "{STAF/Config/Sep/Line}" },
        { kSTAFCommunicationError, "Communication error",
          "This indicates an error transmitting data across the network, "
          "or to the local STAF process.  For "
          "example, you would receive this error if STAFProc.exe was "
          "terminated in the middle of a service "
          "request, or if a bridge went down in the "
          "middle of a remote service request.  This can also indicate that "
          "the requested endpoint is not valid (e.g. it has an invalid "
          "network interface and port combination such as a non-secure tcp "
          "interface with the port for a secure ssl interface)."
          "{STAF/Config/Sep/Line}" },
        { kSTAFTrusteeDoesNotExist, "Trusteee does not exist",
          "You have requested to delete a trustee, and the trustee does "
          "not exist.  Verify that you have specified "
          "the correct trustee.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidTrustLevel, "Invalid trust level",
          "You have attempted to set a machine or default trust level to "
          "an invalid level.  The valid trust levels "
          "are from zero to five.{STAF/Config/Sep/Line}" },
        { kSTAFAccessDenied, "Insufficient trust level",
          "You have submitted a request for which you do not have the "
          "required trust level to perform the request."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}"
          "Note: Additional information regarding the required trust "
          "level may be provided in the result passed back from the "
          "submit call.{STAF/Config/Sep/Line}" },
        { kSTAFRegistrationError, "Registration error",
          "This indicates an error with the configuration of an external "
          "service.  If this error occurs, report it "
          "the service's authors.{STAF/Config/Sep/Line}" },
        { kSTAFServiceConfigurationError, "Service configuration error",
          "This indicates an error with the configuration of an external "
          "service.  One possible explanation is that the LIBRARY you "
          "specified when configuring the service does not exist.  Or, "
          "if you specified the EXECUTE option, verify that the executable"
          "exists and has the execute permission.  Or, if you specified "
          "the PARMS option, verify that all of the service configuration "
          "are valid.  Consult the appropriate documentation for the "
          "service to verify whether you have configured the service "
          "properly, or contact the service provider.{STAF/Config/Sep/Line}" },
        { kSTAFQueueFull, "Queue full",
          "This indicates that you are trying to queue a message to a "
          "handle's queue, but the queue is full.  "
          "The maximum queue size can be increased by using the "
          "MAXQUEUESIZE statement in the STAF "
          "Configuration  File.{STAF/Config/Sep/Line}" },
        { kSTAFNoQueueElement, "No queue element",
          "This indicates that you tried to GET or PEEK a particular "
          "element in a queue, but no such element "
          "exists, or the queue is empty.{STAF/Config/Sep/Line}" },
        { kSTAFNotifieeDoesNotExist, "Notifiee does not exist",
          "This indicates that you are trying to remove a message "
          "notification for a machine/process/priority "
          "combination which does not exist in the notification "
          "list.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidAPILevel, "Invalid API level",
          "This indicates that a process has tried to call an invalid "
          "level of an internal STAF API.  If this "
          "error occurs, report it to the authors.{STAF/Config/Sep/Line}" },
        { kSTAFServiceNotUnregisterable, "Service not unregisterable",
          "This indicates that you are trying to unregister a service "
          "that is not unregisterable."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Internal "
          "services are not unregisterable.{STAF/Config/Sep/Line}" },
        { kSTAFServiceNotAvailable, "Service not available",
          "This indicates that the service you requested is not currently able "
          "to accept requests.  The service may be in "
          "the process of initializing or terminating.{STAF/Config/Sep/Line}" },
        { kSTAFSemaphoreDoesNotExist, "Semaphore does not exist",
          "This indicates that you are trying to release, query, or "
          "delete a semaphore that does not exist."
          "{STAF/Config/Sep/Line}" },
        { kSTAFNotSemaphoreOwner, "Not semaphore owner",
          "This indicates that you are trying to release a semaphore for "
          "which your process is not the current "
          "owner.{STAF/Config/Sep/Line}" },
        { kSTAFSemaphoreHasPendingRequests, "Semaphore has pending requests",
          "This indicates that you are trying to delete either a mutex "
          "semaphore that is currently owned or an "
          "event semaphore that has waiting processes."
          "{STAF/Config/Sep/Line}" },
        { kSTAFTimeout, "Timeout",
          "This indicates that you submitted a request with a timeout "
          "value and the request did not complete "
          "within the requested time.{STAF/Config/Sep/Line}" },
        { kSTAFJavaError, "Java error",
          "This indicates an error performing a Java native method call.  "
          "A description of the error will be "
          "returned in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFConverterError, "Converter error",
          "This indicates an error performing a codepage conversion.  The "
          "most likely cause of this error is that "
          "STAF was not properly installed.  However, it is "
          "possible that you are currently using a "
          "codepage that was not present or specified "
          "during STAF installation.{STAF/Config/Sep/Line}" },
        { kSTAFMoveError, "Move error",
          "This indicates that there was an error while trying to move "
          "a file or directory.{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}"
          "Note: Additional information regarding the error may be "
          "provided in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFInvalidObject, "Invalid object",
          "This indicates that an invalid object was specified to a STAF "
          "API.  If you receive this return code via "
          "a standard STAFSubmit call, report it to the "
          "authors and the service provider.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidParm, "Invalid parm",
          "This indicates that an invalid parameter was specified to a "
          "STAF API.  If you receive this return code "
          "via a standard STAFSubmit call, report it to the "
          "authors and the service provider."
          "{STAF/Config/Sep/Line}" },
        { kSTAFRequestNumberNotFound, "Request number not found",
          "This indicates that the specified request number was not found.  "
          "The specified request number may be "
          "invalid, or the request's information may no longer be "
          "available from the Service Service (for "
          "example, if the SERVICE FREE command had "
          "previously been issued for the request number)."
          "{STAF/Config/Sep/Line}" },
        { kSTAFInvalidAsynchOption, "Invalid asynchronous option",
          "This indicates that an invalid asynchronous submit option was "
          "specified.{STAF/Config/Sep/Line}" },
        { kSTAFRequestNotComplete, "Request not complete",
          "This indicates that the specified request is not complete.  "
          "This error code would be returned, for "
          "example, if you requested the result of a request which "
          "has not yet completed."
          "{STAF/Config/Sep/Line}" },
        { kSTAFProcessAuthenticationDenied, "Process authentication denied",
          "This indicates that the userid/password you specified could not be "
          "authenticated.  The userid/password may not "
          "be valid or authentication may be disabled."
          "{STAF/Config/Sep/Line}" },
        { kSTAFInvalidValue, "Invalid value",
          "This indicates that an invalid value was specified.  This is "
          "closely related to the Invalid Request "
          "String return code, but indicates that a specific "
          "value in the request is invalid.  For "
          "example, you may not have specified a number "
          "where a number was expected.{STAF/Config/Sep/Line}"
          "{STAF/Config/Sep/Line}Note: Additional information regarding the "
          "invalid value may be provided in the result "
          "passed back from the submit call.{STAF/Config/Sep/Line}" },
        { kSTAFDoesNotExist, "Does not exist",
          "This indicates that the item you specified does not exist."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Additional "
          "information regarding the item which could not be found "
          "may be provided in the result passed back "
          "from the submit call.{STAF/Config/Sep/Line}" },
        { kSTAFAlreadyExists, "Already exists",
          "This indicates that the item you specified already exists."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Additional "
          "information regarding the item which already exists may "
          "be provided in the result passed back "
          "from the submit call.{STAF/Config/Sep/Line}" },
        { kSTAFDirectoryNotEmpty, "Directory Not Empty",
          "This indicates that you have tried to delete a directory, but that "
          "directory is not empty.{STAF/Config/Sep/Line}"
          "{STAF/Config/Sep/Line}Note: Additional information specifying the "
          "directory which could not be deleted may be "
          "provided in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFDirectoryCopyError, "Directory Copy Error",
          "This indicates that you have tried to copy a directory, but that "
          "errors{STAF/Config/Sep/Line}occurred during the copy."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Additional "
          "information specifying the entries which could not be "
          "copied may be provided in the result "
          "passed back from the submit call.{STAF/Config/Sep/Line}" },
        { kSTAFDiagnosticsNotEnabled, "Diagnostics Not Enabled",
          "This indicates that you tried to record diagnostics data, but "
          "diagnostics have not been enabled.  You must "
          "enable diagnostics before you can record "
          "diagnostics data.{STAF/Config/Sep/Line}" },
        { kSTAFHandleAuthenticationDenied, "Handle Authentication Denied",
          "This indicates that the user, credentials, and/or authenticator "
          "you specified could not be authenticated. "
          "The user/credentials may not be valid or the "
          "authenticator may not be registered.{STAF/Config/Sep/Line}"
          "{STAF/Config/Sep/Line}Note: Additional information specifying why "
          "authentication was denied may be provided in "
          "the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFHandleAlreadyAuthenticated, "Handle Already Authenticated",
          "This indicates that the handle is already authenticated. "
          "The handle must be unauthenticated in order "
          "to be authenticated.{STAF/Config/Sep/Line}" },
        { kSTAFInvalidSTAFVersion, "Invalid STAF Version",
          "This indicates that the version of STAF (or the version of a "
          "STAF service) is lower than the minimum required version."
          "{STAF/Config/Sep/Line}" },
        { kSTAFRequestCancelled, "Request Cancelled",
          "This indicates that the request has been cancelled."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Additional "
          "information specifying why the request was cancelled may be "
          "provided in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFCreateThreadError, "Create Thread Error",
          "This indicates that a problem occurred creating a new thread. "
          "One possible explanation is that there's not enough memory "
          "available to create a new thread."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Additional "
          "information specifying why creating a new thread failed may be "
          "provided in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFMaximumSizeExceeded, "Maximum Size Exceeded",
          "This indicates that the size of a file exceeded the maximum "
          "size allowed (e.g. per the MAXRETURNFILESIZE operational "
          "parameter or per the MAXRETURNFILESIZE setting for the STAX "
          "service). A maximum file size is usually set to prevent the "
          "creation of result strings that require more memory than is "
          "available which can cause errors or crashes."
          "{STAF/Config/Sep/Line}{STAF/Config/Sep/Line}Note: Additional "
          "information specifying why this error occurred may be "
          "provided in the result passed back from the submit call."
          "{STAF/Config/Sep/Line}" },
        { kSTAFMaximumHandlesExceeded, "Maximum Handles Exceeded",
          "This indicates that a new handle could not be created/"
          "registered because the maximum number of active handles "
          "allowed by STAF has been exceeded.  You need to delete one or "
          "more handles that are no longer being used.  The Handle "
          "service's LIST HANDLES SUMMARY request provides information "
          "on the maximum number of active STAF handles and this may be "
          "helpful in better understanding why this error occurred." },
        { kSTAFNotRequester, "Not Pending Requester",
          "You cannot cancel a pending request your handle did not submit "
          "unless you specify the FORCE option." }
    };

    for (int i = 0; i < (sizeof(helpMsgs) / sizeof(HelpMessageInfo)); ++i)
    {
        ServiceHelpData helpData(helpMsgs[i].info, helpMsgs[i].description);
        fErrorMap[helpMsgs[i].rc]["INTERNAL"] = helpData;
        fServiceMap["INTERNAL"][helpMsgs[i].rc] = helpData;
    }
}


STAFString STAFHelpService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFHelpService::~STAFHelpService()
{
    /* Do Nothing */
}


STAFServiceResult STAFHelpService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();
 
    if (action == "register")
        return handleRegister(true, requestInfo);
    else if (action == "unregister")
        return handleRegister(false, requestInfo);
    else if (action == "list")
        return handleList(requestInfo);
    else if ((action == "service") || (action == "error"))
        return handleError(requestInfo);
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



STAFServiceResult STAFHelpService::handleRegister(
    bool isRegister, const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine and that
    // the requesting machine/user has at least trust level 3

    if (isRegister)
    {
        IVALIDATE_LOCAL_TRUST(3, "REGISTER");
    }
    else
    {
        IVALIDATE_LOCAL_TRUST(3, "UNREGISTER");
    }
 
    // Parse the request

    STAFCommandParseResultPtr parsedResult = fRegParser.parse(
        requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString info = parsedResult->optionValue("INFO");
    STAFString description = parsedResult->optionValue("DESCRIPTION");
    STAFString service;
    STAFString errorBuffer;
    unsigned int errorNumber;
    STAFRC_t rc = RESOLVE_OPTIONAL_UINT_OPTION("ERROR", errorNumber);

    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("SERVICE", service);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString upperService = service.toUpperCase();

    if (isRegister)
    {
        ServiceHelpData helpData = ServiceHelpData(info, description);
        STAFMutexSemLock lock(fMapSem);

        fErrorMap[errorNumber][upperService] = helpData;
        fServiceMap[upperService][errorNumber] = helpData;
    }
    else
    {
        STAFMutexSemLock lock(fMapSem);

        if (fErrorMap.find(errorNumber) == fErrorMap.end())
            return STAFServiceResult(kSTAFDoesNotExist, errorNumber);

        if (fErrorMap[errorNumber].find(upperService) ==
            fErrorMap[errorNumber].end())
        {
            return STAFServiceResult(kSTAFDoesNotExist, service);
        }

        fErrorMap[errorNumber].erase(upperService);
        fServiceMap[upperService].erase(errorNumber);
    }

    return kSTAFOk;
}


STAFServiceResult STAFHelpService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListParser.parse(
        requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString service = "INTERNAL";
    STAFString errorBuffer;
    STAFString result;
    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("SERVICE", service);

    if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);

    STAFString upperService = service.toUpperCase();
    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    STAFObjectPtr outputList = STAFObject::createList();
    
    STAFMutexSemLock lock(fMapSem);

    if (parsedResult->optionTimes("SERVICES") != 0)
    {
        // Create a marshalled list of strings containing the service names
        
        HelpServiceMap::iterator iter;

        for (iter = fServiceMap.begin(); iter != fServiceMap.end(); ++iter)
        {
            if (iter->first != "INTERNAL")
                outputList->append(STAFString(iter->first));
        }
    }
    else
    {
        if (fServiceMap.find(upperService) == fServiceMap.end())
            return STAFServiceResult(kSTAFDoesNotExist, service);

        // Create a marshalled list of maps containing the error information

        mc->setMapClassDefinition(fErrorInfoClass->reference());
        
        HelpErrorToInfoMap::iterator iter;

        for (iter = fServiceMap[upperService].begin();
             iter != fServiceMap[upperService].end(); ++iter)
        {
            STAFObjectPtr errorInfoMap = fErrorInfoClass->createInstance();
            errorInfoMap->put("returnCode", STAFString(iter->first));
            errorInfoMap->put("description", STAFString(iter->second.info));
            outputList->append(errorInfoMap);
        }

        if (upperService == "INTERNAL")
        {
            STAFObjectPtr errorInfoMap = fErrorInfoClass->createInstance();
            errorInfoMap->put("returnCode", STAFString("4000+"));
            errorInfoMap->put(
                "description", STAFString("Service specific errors"));
            outputList->append(errorInfoMap);
        }
    }

    mc->setRootObject(outputList);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFHelpService::handleError(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "ERROR");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fErrorParser.parse(requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString service;
    STAFString errorBuffer;
    unsigned int errorNumber;
    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("SERVICE", service);

    if (!rc) rc = RESOLVE_OPTIONAL_UINT_OPTION("ERROR", errorNumber);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFString upperService = service.toUpperCase();
    STAFMutexSemLock lock(fMapSem);

    if (service.length() != 0)
    {
        if (fServiceMap.find(upperService) == fServiceMap.end())
            return STAFServiceResult(kSTAFDoesNotExist, service);

        if (fServiceMap[upperService].find(errorNumber) ==
            fServiceMap[upperService].end())
        {
            return STAFServiceResult(kSTAFDoesNotExist, errorNumber);
        }
    }

    if (fErrorMap.find(errorNumber) == fErrorMap.end())
        return STAFServiceResult(kSTAFDoesNotExist, errorNumber);

    // Create a marshalled map containing help information for the error

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    STAFObjectPtr outputList = STAFObject::createList();
    
    HelpServiceToInfoMap::iterator iter;
    
    if ((upperService.length() != 0) || (errorNumber < 4000))
    {
        ServiceHelpData helpData;

        if (upperService.length() == 0)
            upperService = "INTERNAL";

        helpData = fErrorMap[errorNumber][upperService];
        
        STAFString info;
        STAFString detail;
        STAFRC_t rc = RESOLVE_STRING(helpData.info, info);
        if (!rc) rc = RESOLVE_STRING(helpData.detail, detail);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Create a marshalled map containing the error information

        mc->setMapClassDefinition(fErrorDetailsClass->reference());

        STAFObjectPtr errorMap = fErrorDetailsClass->createInstance();
        errorMap->put("description", info);
        errorMap->put("details", detail);

        mc->setRootObject(errorMap);
    }
    else
    {
        mc->setMapClassDefinition(fServiceErrorClass->reference());

        for(iter = fErrorMap[errorNumber].begin();
            iter != fErrorMap[errorNumber].end(); ++iter)
        {
            STAFString info;
            STAFString detail;
            STAFRC_t rc = RESOLVE_STRING(iter->second.info, info);
            if (!rc) rc = RESOLVE_STRING(iter->second.detail, detail);
            if (rc) return STAFServiceResult(rc, errorBuffer);

            // Create a marshalled list of maps contains the error information

            STAFObjectPtr errorMap = fServiceErrorClass->createInstance();
            errorMap->put("service", STAFString(iter->first));
            errorMap->put("description", info);
            errorMap->put("details", detail);

            outputList->append(errorMap);
        }

        mc->setRootObject(outputList);
    }

    return STAFServiceResult(kSTAFOk, mc->marshall());
}

STAFServiceResult STAFHelpService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    // Return the help text

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
