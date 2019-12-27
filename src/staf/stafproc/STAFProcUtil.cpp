/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFProcUtil.h"
#include "STAFUtil.h"
#include "STAFException.h"
#include "STAFVariablePool.h"
#include "STAFProc.h"
#include "STAFHandleManager.h"
#include "STAFDiagManager.h"
#include "STAFCommandParser.h"
#include "STAFServiceManager.h"


bool isLocalMachine(const STAFString &machine, unsigned int fullCompare)
{
    static STAFString sLocal = "local";
    static STAFString sLocalLong = "local://local";

    if (fullCompare)
    {
        if (machine.isEqualTo(sLocal, kSTAFStringCaseInsensitive) ||
            machine.isEqualTo(sLocalLong, kSTAFStringCaseInsensitive) ||
            machine.isEqualTo(*gMachinePtr, kSTAFStringCaseInsensitive))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return machine.isEqualTo(sLocal, kSTAFStringCaseInsensitive);
}


STAFRC_t resolveUIntIfExists(STAFCommandParseResultPtr &parseResult,
                             const STAFString &optionName,
                             const STAFVariablePool *varPoolList[],
                             unsigned int varPoolListSize,
                             unsigned int &result,
                             STAFString &errorBuffer,
                             unsigned int defaultValue,
                             unsigned int minValue, unsigned int maxValue)
{
    if (parseResult->optionTimes(optionName) == 0)
        return kSTAFOk;

    return resolveUInt(parseResult->optionValue(optionName), optionName,
                       varPoolList, varPoolListSize,
                       result, errorBuffer,
                       defaultValue, minValue, maxValue);
}


STAFRC_t resolveUInt(const STAFString &unresolved,
                     const STAFVariablePool *varPoolList[],
                     unsigned int varPoolListSize,
                     unsigned int &result,
                     STAFString &errorBuffer,
                     unsigned int defaultValue,
                     unsigned int minValue, unsigned int maxValue)
{
    return resolveUInt(unresolved, STAFString(""),
                       varPoolList, varPoolListSize,
                       result, errorBuffer,
                       defaultValue, minValue, maxValue);
}


STAFRC_t resolveUInt(const STAFString &unresolved,
                     const STAFString &optionName,
                     const STAFVariablePool *varPoolList[],
                     unsigned int varPoolListSize,
                     unsigned int &result,
                     STAFString &errorBuffer,
                     unsigned int defaultValue,
                     unsigned int minValue, unsigned int maxValue)
{
    STAFString resultString;

    // Resolve any STAF variables in the string

    STAFRC_t rc = STAFVariablePool::resolve(unresolved, varPoolList,
                                            varPoolListSize, resultString);

    if (rc != kSTAFOk)
    {
        errorBuffer = resultString;

        return rc;
    }

    if (resultString.length() == 0)
    {
        result = defaultValue;
    }
    else
    {
        // Convert the resolved string to an unsigned integer

        return convertStringToUInt(
            resultString, optionName, result, errorBuffer,
            minValue, maxValue);
    }

    return rc;
}


STAFRC_t convertStringToUInt(const STAFString &theString,
                             const STAFString &optionName,
                             unsigned int &number,
                             STAFString &errorBuffer,
                             unsigned int minValue,
                             unsigned int maxValue)
{
    // Convert the string to an unsigned integer

    STAFString_t errorBufferT = 0;

    STAFRC_t rc = STAFUtilConvertStringToUInt(
        theString.getImpl(), optionName.getImpl(), &number,
        &errorBufferT, minValue, maxValue);

    if (rc != kSTAFOk)
        errorBuffer = STAFString(errorBufferT, STAFString::kShallow);

    return rc;
}


STAFRC_t resolveStringIfExists(
    STAFCommandParseResultPtr &parseResult, const STAFString &optionName,
    const STAFVariablePool *varPoolList[], unsigned int varPoolListSize,
    STAFString &result, STAFString &errorBuffer, bool ignoreVarResolveErrors)
{
    if (parseResult->optionTimes(optionName) == 0)
        return kSTAFOk;

    return resolveString(parseResult->optionValue(optionName), varPoolList,
                         varPoolListSize, result, errorBuffer,
                         ignoreVarResolveErrors);
}


STAFRC_t resolveString(const STAFString &unresolved,
                       const STAFVariablePool *varPoolList[],
                       unsigned int varPoolListSize, STAFString &result,
                       STAFString &errorBuffer, bool ignoreVarResolveErrors)
{
    STAFRC_t rc = STAFVariablePool::resolve(unresolved, varPoolList,
                                            varPoolListSize, result,
                                            ignoreVarResolveErrors);

    if (rc != kSTAFOk)
    {
        errorBuffer = result;
    }

    return rc;
}


STAFRC_t resolveDurationStringIfExists(
    STAFCommandParseResultPtr &parseResult, const STAFString &optionName,
    const STAFVariablePool *varPoolList[], unsigned int varPoolListSize,
    unsigned int &result, STAFString &errorBuffer, unsigned int defaultValue)
{
    // Check if the optionName exists and if it doesn't, return without
    // changing the value of result (e.g. don't set to defaultValue)

    if (parseResult->optionTimes(optionName) == 0)
        return kSTAFOk;

    // Resolve the value for the option

    STAFString resultString;

    STAFRC_t rc = resolveString(parseResult->optionValue(optionName),
                                varPoolList, varPoolListSize,
                                resultString, errorBuffer);

    if (rc != kSTAFOk) return rc;

    if (resultString.length() == 0)
    {
        // No value for the option was specified.
        // Default to the specified value.

        result = defaultValue;
    }
    else
    {
        // Check if the option value is a valid duration string and if
        // so, convert it to its numeric value in milliseconds

        return convertDurationStringToUInt(
            resultString, optionName, result, errorBuffer);
    }
    
    return rc;
}


STAFRC_t convertDurationStringToUInt(const STAFString &theString,
                                     const STAFString &optionName,
                                     unsigned int &number,
                                     STAFString &errorBuffer)
{
    // Check if the option value is a valid duration string and if so,
    // convert it to its numeric value in milliseconds

    STAFString_t errorBufferT = 0;

    STAFRC_t rc = STAFUtilConvertDurationString(
        theString.getImpl(), &number, &errorBufferT);

    if (rc != kSTAFOk)
    {
        errorBuffer = STAFString("Invalid value for the ") +
            optionName + " option: " + theString + " \n\n" +
            STAFString(errorBufferT, STAFString::kShallow);
    }

    return rc;
}


STAFRC_t validateTrust(unsigned int requiredTrustLevel,
                       const STAFString &service,
                       const STAFString &request,
                       const STAFServiceRequest &requestInfo,
                       STAFString &errorBuffer,
                       unsigned int localOnly)
{
    if (localOnly)
    {
        if (!requestInfo.fIsLocalRequest)
        {
            errorBuffer = service + " service's " + request + " request is "
                "only valid if submitted to the local machine"
                "\nLocal machine     : " + *gMachinePtr +
                "\nRequesting machine: " + requestInfo.fLogicalInterfaceID; 
            
            return kSTAFAccessDenied;
        }
    }

    if (requestInfo.fTrustLevel < requiredTrustLevel)
    {
        errorBuffer = STAFString("Trust level ") +
            STAFString(requiredTrustLevel) +
            " required for the " + service + " service's " + request +
            " request\nRequester has trust level " +
            STAFString(requestInfo.fTrustLevel) + " on machine " +
            *gMachinePtr + "\nRequesting machine: " + requestInfo.fInterface +
            gSpecSeparator + requestInfo.fLogicalInterfaceID +
            " (" + requestInfo.fInterface + gSpecSeparator +
            requestInfo.fPhysicalInterfaceID + ")" +
            "\nRequesting user   : " + requestInfo.fAuthenticator +
            gSpecSeparator + requestInfo.fUserIdentifier;

        return kSTAFAccessDenied;
    }

    return kSTAFOk;
}

STAFServiceResult submitAuthServiceRequest(const STAFServicePtr &service,
                                           const STAFString &request)
{
    static STAFString sLocal = "local";

    // This method uses the STAFProc handle to submit a local request to the
    // specified Authenticator service

    STAFServiceRequest requestInfo;

    requestInfo.fRequest = request;
    requestInfo.fMachine = sLocal; // Must be issued from local system
    requestInfo.fMachineNickname = *gMachineNicknamePtr;
    requestInfo.fHandle = gSTAFProcHandle; // A handle on the local system
    requestInfo.fHandleName = gHandleManagerPtr->name(gSTAFProcHandle);
    requestInfo.fRequestNumber = 0;
    requestInfo.fDiagEnabled = gDiagManagerPtr->getEnabled();
    requestInfo.fTrustLevel = 5;    // Authenticate requires trust level 5
    requestInfo.fAuthenticator = gNoneString;
    requestInfo.fUserIdentifier = gAnonymousString;
    requestInfo.fUser = gUnauthenticatedUser;
    requestInfo.fInterface = sLocal;
    requestInfo.fLogicalInterfaceID = sLocal;
    requestInfo.fPhysicalInterfaceID = sLocal;
    requestInfo.fEndpoint = sLocal + gSpecSeparator + sLocal;

    gHandleManagerPtr->variablePool(requestInfo.fHandle, 
                                    requestInfo.fRequestVarPool);

    requestInfo.fSourceSharedVarPool =
        STAFVariablePoolPtr(new STAFVariablePool, STAFVariablePoolPtr::INIT);
    requestInfo.fLocalSharedVarPool = *gSharedVariablePoolPtr;
    requestInfo.fLocalSystemVarPool = *gGlobalVariablePoolPtr;

    return service->submitRequest(requestInfo);
}

// This function checks if the specified authenticator requires a
// secure tcp connection by checking the "requiresecure" property
// of the authenticator

bool requiresSecureConnection(const STAFString &authenticator)
{
    static STAFString sYes = STAFString("Yes");
    static STAFString sNo = STAFString("No");
    static STAFString requireSecureProperty = STAFString("requiresecure"); 

    bool requireSecure = false;

    if (authenticator == gNoneString)
    {
        // Not authenticated so doesn't require a secure connection
        return requireSecure;
    }
    
    // Get the authenticator service's requiresecure property
                
    STAFServicePtr authService;

    STAFRC_t rc = gServiceManagerPtr->getAuthenticator(
        authenticator, authService);

    if (rc != kSTAFOk)
    {
        // Authenticator doesn't exist so doesn't require a secure connection
        return requireSecure;
    }

    if (!authService->hasProperty(requireSecureProperty))
    {
        // Determine if the authenticator service requires a secure tcp
        // connection and set the requiresecure property for the
        // authenticator service

        STAFServiceResult result = submitAuthServiceRequest(
            authService, "REQUIRESECURE");

        if (result.fRC == kSTAFOk)
            authService->setProperty(requireSecureProperty, result.fResult);
        else
            authService->setProperty(requireSecureProperty, sNo);
    }
                    
    if (authService->getProperty(requireSecureProperty).isEqualTo(
        sYes, kSTAFStringCaseInsensitive))
    {
        requireSecure = true;
    }

    return requireSecure;
}

