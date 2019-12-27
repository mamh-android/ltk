/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFUtil.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFSimpleServices.h"
#include "STAFHandleManager.h"
#include "STAFTrace.h"

STAFString STAFPingService::sPing("ping");
STAFString STAFPingService::sPong("PONG");
STAFString STAFPingService::sHelpMsg;

STAFString STAFEchoService::sHelpMsg;

STAFString STAFDelayService::sHelpMsg;

STAFServiceResult STAFPingService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    if (STAFString(requestInfo.fRequest).strip().isEqualTo(
            sPing, kSTAFStringCaseInsensitive))
    {
        // Verify that the requesting machine/user has at least trust level 1

        IVALIDATE_TRUST(1, "PING");
        
        return STAFServiceResult(kSTAFOk, sPong);
    }
    else if (requestInfo.fRequest.subWord(0, 2).lowerCase() == "ping machine")
    {
        // PING MACHINE <Machine>

        IVALIDATE_TRUST(1, "PING");

        STAFString unresMachine = requestInfo.fRequest.subWord(2);

        if (unresMachine.length() == 0)
        {
            return STAFServiceResult(kSTAFInvalidRequestString,
                                     "Option, MACHINE, requires a value");
        }
        
        // Resolve any variables in the MACHINE option value

        DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
        STAFString errorBuffer;
        STAFString machine;
        STAFRC_t rc = RESOLVE_STRING(unresMachine, machine);
        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Submit a PING request to the specified machine

        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            machine, "PING", "PING");

        return STAFServiceResult(result->rc, result->result);
    }
    else if (requestInfo.fRequest.subWord(0, 1).lowerCase() == "help")
    {
        // Verify that the requesting machine/user has at least trust level 1

        IVALIDATE_TRUST(1, "HELP");

        return STAFServiceResult(kSTAFOk, sHelpMsg);
    }
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

STAFPingService::STAFPingService() : STAFService("PING")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** PING Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "PING [MACHINE <Machine>]" +
        *gLineSeparatorPtr +
        "HELP";
}

STAFString STAFPingService::info(unsigned int) const
{
    return name() + STAFString(": Internal");
}

STAFPingService::~STAFPingService()
{
    /* Do Nothing */
}


STAFServiceResult STAFEchoService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).toLowerCase();

    if (action == "echo")
    {
        // Verify that the requesting machine/user has at least trust level 2
 
        IVALIDATE_TRUST(2, "ECHO");

        STAFString echoString = requestInfo.fRequest.subWord(1);

        if (echoString.length() == 0)
        {
            return STAFServiceResult(kSTAFInvalidRequestString,
                                     "Option, ECHO, requires a value");
        }

        return STAFServiceResult(kSTAFOk, echoString);
    }
    else if (action == "help")
    {
        // Verify that the requesting machine/user has at least trust level 1

        IVALIDATE_TRUST(1, "HELP");

        return STAFServiceResult(kSTAFOk, sHelpMsg);
    }
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

STAFEchoService::STAFEchoService() : STAFService("ECHO")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** ECHO Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "ECHO <Message>" +
        *gLineSeparatorPtr +
        "HELP";
}

STAFString STAFEchoService::info(unsigned int) const
{
    return name() + STAFString(": Internal");
}

STAFEchoService::~STAFEchoService()
{
    // Do Nothing
}


STAFServiceResult STAFDelayService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).toLowerCase();

    if (action == "delay")
    {
        // Verify that the requesting machine/user has at least trust level 2

        IVALIDATE_TRUST(2, "DELAY");

        STAFString delayString = requestInfo.fRequest.subWord(1);

        if (delayString.length() == 0)
        {
            return STAFServiceResult(kSTAFInvalidRequestString,
                                     "Option, DELAY, requires a value");
        }

        STAFString_t errorBuffer = 0;
        unsigned int delay = 0;

        STAFRC_t rc = STAFUtilConvertDurationString(
            delayString.getImpl(), &delay, &errorBuffer);
        
        if (rc != kSTAFOk)
        {
            return STAFServiceResult(
                rc, STAFString("Invalid value for the DELAY option: ") +
                delayString + " \n\n" +
                STAFString(errorBuffer, STAFString::kShallow));
        }

        // Register for notification so that if the handle is deleted and
        // garbage collected before the delay timeout expires, the DELAY
        // request will be cancelled.  Set the key in the notification
        // request to be the originating request number.
  
        STAFServiceResult serviceResult = gHandleManagerPtr->addNotification(
            requestInfo.fHandle, requestInfo.fEndpoint, requestInfo.fMachine,
            requestInfo.fSTAFInstanceUUID, name(),
            STAFString(requestInfo.fRequestNumber));

        if (serviceResult.fRC != kSTAFOk)
        {
            // Could return an error if addNotification() fails, but decided
            // to just to log a trace warning message instead

            if (STAFTrace::doTrace(kSTAFTraceDebug))
            {
                STAFString msg = STAFString(
                    "STAFDelayService: An error occurred when the DELAY "
                    "service attempted to register for garbage colllection"
                    " notification on endpoint '") + requestInfo.fEndpoint +
                    "', RC: " + STAFString(serviceResult.fRC) +
                    ", Reason: " + serviceResult.fResult;
                STAFTrace::trace(kSTAFTraceDebug, msg);
            }
        }

        // Add an entry to the DelayRequestMap where the key is the
        // requester's:
        //
        //   request#|handle#|STAFInstanceUUID
        //
        // and the value is an event semaphore pointer

        STAFString key = STAFString(requestInfo.fRequestNumber) + "|" +
            STAFString(requestInfo.fHandle) + "|" +
            requestInfo.fSTAFInstanceUUID;
        
        // Create an event semaphore pointer in the Reset state

        STAFEventSemPtr waiterSem(new STAFEventSem(), STAFEventSemPtr::INIT);

        {
            STAFMutexSemLock lock(fDelayRequestMapSem);
            fDelayRequestMap[key] = waiterSem;
        }

        // Wait for specified delay timeout

        waiterSem->wait(delay);

        // Check the state (reset or posted) of the event semaphore as this
        // indicates whether we delayed for the entire timeout or not

        STAFMutexSemLock lock(fDelayRequestMapSem);
        
        if (waiterSem->query() == kSTAFEventSemReset)
        {
            // The DELAY request completed successfully after delaying for the
            // specified delay timeout

            // Remove the request from the DelayRequestMap and delete
            // the notfication from the handle gc notification list

            fDelayRequestMap.erase(key);

            gHandleManagerPtr->deleteNotification(
                requestInfo.fHandle, requestInfo.fEndpoint, 
                requestInfo.fMachine, requestInfo.fSTAFInstanceUUID,
                name(), STAFString(requestInfo.fRequestNumber));

            return STAFServiceResult(kSTAFOk);
        }
        else
        {
            // The handle that submitted the DELAY request was deleted and
            // garbage collection has occurred (as the waiterSem event
            // semaphore was posted by our callback), so return a
            // RequestCancelled error

            return STAFServiceResult(
                kSTAFRequestCancelled,
                "The handle that submitted this request no longer exists");
        }
    }
    else if (action == "staf_callback")
    {
        // Don't check the TRUST level, but make sure the requesting handle
        // is the STAFProc handle

        if (requestInfo.fHandle != 1)
        {
            return STAFServiceResult(
                kSTAFAccessDenied,
                "This request is only valid when submitted by STAFProc");
        }

        STAFCommandParseResultPtr parsedResult =
            fSTAFCallbackParser.parse(requestInfo.fRequest);

        if (parsedResult->rc != kSTAFOk)
        {
            return STAFServiceResult(
                kSTAFInvalidRequestString, parsedResult->errorBuffer);
        }

        // Check if the request#|handle#|STAFInstanceUUID provided in the
        // callback request exists in the DelayRequestMap

        STAFString key = parsedResult->optionValue("KEY") + "|" +
            parsedResult->optionValue("HANDLE") + "|" +
            parsedResult->optionValue("UUID");

        STAFMutexSemLock lock(fDelayRequestMapSem);

        if (fDelayRequestMap.find(key) != fDelayRequestMap.end())
        {
            // Key was found in DelayRequestMap, so post its event semaphore
            // to indicate that the request should be cancelled, and remove
            // this entry from the DelayRequestMap

            fDelayRequestMap[key]->post();
            fDelayRequestMap.erase(key);
        }

        return STAFServiceResult(kSTAFOk);
    }
    else if (action == "help")
    {
        // Verify that the requesting machine/user has at least trust level 1

        IVALIDATE_TRUST(1, "HELP");

        return STAFServiceResult(kSTAFOk, sHelpMsg);
    }
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

STAFDelayService::STAFDelayService() : STAFService("DELAY")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** DELAY Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DELAY <Number>[s|m|h|d|w]" +
        *gLineSeparatorPtr +
        "HELP";

    // Set up STAF_CALLBACK parser

    // Note that STAF_CALLBACK is an undocumented request command that can
    // only be submitted by STAFProc
 
    fSTAFCallbackParser.addOption("STAF_CALLBACK",  1, 
        STAFCommandParser::kValueNotAllowed);
    fSTAFCallbackParser.addOption("HANDLEDELETED", 1, 
        STAFCommandParser::kValueNotAllowed);
    fSTAFCallbackParser.addOption("HANDLE", 1, 
        STAFCommandParser::kValueRequired);
    fSTAFCallbackParser.addOption("MACHINE", 1, 
        STAFCommandParser::kValueRequired);
    fSTAFCallbackParser.addOption("UUID", 1, 
        STAFCommandParser::kValueRequired);
    fSTAFCallbackParser.addOption("KEY", 1, 
        STAFCommandParser::kValueRequired);
}

STAFString STAFDelayService::info(unsigned int) const
{
    return name() + STAFString(": Internal");
}

STAFDelayService::~STAFDelayService()
{
    /* Do Nothing */
}
