/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFService.h"
#include "STAFTrace.h"
#include "STAFServiceManager.h"
#include "STAFProc.h"
#include "STAFThreadManager.h"


STAFServiceResult STAFService::submitRequest(
    const STAFServiceRequest &requestInfo)
{
    ServiceState serviceState = kNotReady;

    // First, lock the list.  If the service is accepting requests, add
    // ourself to the list of request threads.

    {
        STAFMutexSemLock lock(fRequestSem);
        serviceState = fServiceState;

        if (serviceState == kReady)
            fThreadList.insert(gThreadManagerPtr->getCurrentThreadID());
    }

    // Second, if we are accepting requests, pass the request on

    STAFServiceResult result;

    try
    {
        if (STAFTrace::doTrace(kSTAFTraceServiceRequest) &&
            STAFServiceManager::doTraceService(name()))
        {
            STAFString data(name() + " Service Request - Client: " +
                            requestInfo.fEndpoint + ", Handle: " +
                            STAFString(requestInfo.fHandle) + ", Process: " +
                            requestInfo.fHandleName + ", Request: " +
                            requestInfo.fRequest);

            STAFTrace::trace(kSTAFTraceServiceRequest, data);
        }

        if (serviceState != kReady)
        {
            result = kSTAFServiceNotAvailable;
        }
        else
        {
            result = acceptRequest(requestInfo);
        }
    }
    catch (...)
    {
        // We need to remove ourself from the list if we throw an exception
        // while processing the request

        STAFMutexSemLock lock(fRequestSem);
        fThreadList.erase(gThreadManagerPtr->getCurrentThreadID());

        throw;
    }

    // Third, lock the list and remove ourself from the list of
    // request threads

    {
        STAFMutexSemLock lock(fRequestSem);
        fThreadList.erase(gThreadManagerPtr->getCurrentThreadID());
    }

    // Trace any necessary data

    if (STAFTrace::doTrace(kSTAFTraceServiceResult) &&
        STAFServiceManager::doTraceService(name()))
    {
        STAFString serviceResult = ((result.fResult.length() == 0) ?
                                         STAFString("<No Result>") :
                                         STAFObject::unmarshall(
                                         result.fResult)->asFormattedString());

        unsigned int maxLength = STAFServiceManager::getMaxServiceResultSize();

        if ((maxLength > 0) &&
            (serviceResult.length(STAFString::kChar) > maxLength))
        {
            serviceResult = serviceResult.subString(0, maxLength,
                                                    STAFString::kChar);
            serviceResult = serviceResult + "...";
        }

        STAFString data(name() + " Service Result (" +
                        STAFString(result.fRC) +
                        ") - Client: " + requestInfo.fEndpoint + ", Handle: " +
                        STAFString(requestInfo.fHandle) + ", Process: " +
                        requestInfo.fHandleName + ", Request: " +
                        requestInfo.fRequest + ", Result: " + serviceResult);

        STAFTrace::trace(kSTAFTraceServiceResult, data);
    }
    else if (STAFTrace::doTrace(kSTAFTraceServiceComplete) &&
        STAFServiceManager::doTraceService(name()))
    {
        STAFString data(name() + " Service Complete (" +
                        STAFString(result.fRC) +
                        ") - Client: " + requestInfo.fEndpoint + ", Handle: " +
                        STAFString(requestInfo.fHandle) + ", Process: " +
                        requestInfo.fHandleName + ", Request: " +
                        requestInfo.fRequest + ", Result Length: " +
                        STAFString(result.fResult.length()));

        STAFTrace::trace(kSTAFTraceServiceComplete, data);
    }
    else if ((result.fRC != kSTAFOk) &&
             STAFTrace::doTrace(kSTAFTraceServiceError) &&
             STAFServiceManager::doTraceService(name()))
    {
        STAFString data(name() + " Service Non-Zero (" +
                        STAFString(result.fRC) +
                        ") Return Code - Client: " + requestInfo.fEndpoint +
                        ", Handle: " + STAFString(requestInfo.fHandle) +
                        ", Process: " + requestInfo.fHandleName +
                        ", Request: " + requestInfo.fRequest + ", Result: " +
                        ((result.fResult.length() == 0) ?
                             STAFString("<No Result>") :
                             STAFObject::unmarshall(
                                 result.fResult)->asFormattedString()));

        STAFTrace::trace(kSTAFTraceServiceError, data);
    }
    else if ((result.fRC == kSTAFAccessDenied) &&
             STAFTrace::doTrace(kSTAFTraceServiceAccessDenied) &&
             STAFServiceManager::doTraceService(name()))
    {
        STAFString data(name() + " Service Access Denied - Client: " +
                        requestInfo.fEndpoint + ", Handle: " +
                        STAFString(requestInfo.fHandle) + ", Process: " +
                        requestInfo.fHandleName + ", Request: " +
                        requestInfo.fRequest);

        STAFTrace::trace(kSTAFTraceServiceAccessDenied, data);
    }

    // Finally, return the result

    return result;
}

void STAFService::setName(const STAFString &name)
{
    STAFMutexSemLock lock(fRequestSem);
    fName = name.toUpperCase();
}

STAFService::ServiceState STAFService::state()
{
    STAFMutexSemLock lock(fRequestSem);
    return fServiceState;
}

bool STAFService::hasProperty(STAFString &name)
{
    return fPropertyMap.find(name) != fPropertyMap.end();
}

STAFString STAFService::getProperty(STAFString &name)
{
    return hasProperty(name) ? fPropertyMap[name] : "";
}

STAFString STAFService::setProperty(STAFString &name, STAFString &value)
{
    STAFString tempValue = getProperty(name);

    fPropertyMap[name] = value;

    return tempValue;
}

STAFServiceResult STAFService::initialize()
{
    {
        STAFMutexSemLock lock(fRequestSem);

        if (fServiceState == kNotReady)
            fServiceState = kInitializing;
        else if (fServiceState == kReady)
            return kSTAFOk;
        else
            return kSTAFServiceNotAvailable;
    }

    STAFTrace::trace(kSTAFTraceServiceManagement,
                     "Service " + name() + ": Initializing");

    STAFServiceResult result = init();

    if (!result.fRC)
    {
        STAFMutexSemLock lock(fRequestSem);
        fServiceState = kReady;
    }

    return result;
}


STAFServiceResult STAFService::init()
{
    return STAFServiceResult(kSTAFOk, STAFString(""));
}


STAFServiceResult STAFService::terminate(unsigned int timeout)
{
    {
        STAFMutexSemLock lock(fRequestSem);

        if (fServiceState == kReady)
            fServiceState = kTerminating;
        else if (fServiceState == kTerminated)
            return kSTAFOk;
        else
            return kSTAFServiceNotAvailable;
    }

    STAFTrace::trace(kSTAFTraceServiceManagement,
                     "Service " + name() + ": Terminating");

    const unsigned int oneSecond = 1000;

    for(unsigned int remainingTimeout = timeout;
        (remainingTimeout != 0) && (fThreadList.size() != 0);
        remainingTimeout -= STAF_MIN(oneSecond, remainingTimeout))
    {
        gThreadManagerPtr->sleepCurrentThread(STAF_MIN(oneSecond,
                                              remainingTimeout));
    }

    STAFServiceResult result = term();

    {
        STAFMutexSemLock lock(fRequestSem);

        if (fThreadList.size() != 0)
        {
            // loop through and kill all the threads
            ThreadList::iterator listIter;

            for(listIter = fThreadList.begin(); listIter != fThreadList.end();
                ++listIter)
            { /* Do Nothing */ }
            // XXX:    STAFThread(*listIter).stop();
        }

        fServiceState = kTerminated;
    }

    return result;
}


STAFServiceResult STAFService::term()
{
    return STAFServiceResult(kSTAFOk, STAFString(""));
}
