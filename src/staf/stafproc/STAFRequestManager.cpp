/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFProc.h"
#include "STAFRequestManager.h"
#include "STAFService.h"
#include "STAFString.h"
#include "STAFError.h"
#include "STAFTrace.h"

static STAFRequestManager *pRequestManager = 0;

// Note that the maximum value for a request number must not exceed the
// maximum value for the STAFRequestNumber_t type (unsigned int), nor the
// maximum value for the JNI type (jint) used for the requestNumber in the
// STAFServiceInterfaceLevel30 RequestInfo Java class.

// XXX: If we changed the STAF JNI interface to use jlong instead of jint
//      for requestNumber, then could use UINT_MAX instead of INT_MAX

static STAFRequestNumber_t MAX_REQUEST_NUMBER = INT_MAX; // (e.g. 2147483647)
static STAFRequestNumber_t MIN_REQUEST_NUMBER = 1;

// Maximum number of active requests that can reside in the request map
// based on the range (minimum and maximum values) for request number values
static STAFRequestNumber_t MAX_ACTIVE_REQUESTS =
    MAX_REQUEST_NUMBER - MIN_REQUEST_NUMBER + 1;


STAFRequestManager::STAFRequestManager() : fNextRequest(MIN_REQUEST_NUMBER - 1),
                                           fResetCount(0),
                                           fTotalRequests(0)
{
    pRequestManager = this;
}


STAFServiceRequestPtr STAFRequestManager::getNewServiceRequest()
{
    STAFMutexSemLock requestLock(fRequestMapSem);
    STAFServiceRequest *serviceRequest = new STAFServiceRequest();

    // Determine the next available request number
    
    if (fNextRequest < MAX_REQUEST_NUMBER)
    {
        ++fNextRequest;
    }
    else
    {
        fNextRequest = MIN_REQUEST_NUMBER;
        fResetCount++;
    }
    
    if (fResetCount)
    {
        if (fRequestMap.size() < MAX_ACTIVE_REQUESTS)
        {
            // Find the next available request number (the next one that is
            // not in the request map)

            while (true)
            {
                if (fRequestMap.find(fNextRequest) == fRequestMap.end())
                    break;  // Request number is not found so it is available

                if (fNextRequest < MAX_REQUEST_NUMBER)
                {
                    ++fNextRequest;
                }
                else
                {
                    fNextRequest = MIN_REQUEST_NUMBER;
                    fResetCount++;
                }
            }
        }
        else
        {
            STAFString errorMsg = STAFString(
                "STAFRequestManager::getNewServiceRequest():  Exceeded the "
                "maximum number of active requests, ") + MAX_ACTIVE_REQUESTS +
                ", after resetting the request number " + fResetCount +
                " times";

            STAFTrace::trace(kSTAFTraceError, errorMsg);

            // XXX: We should not continue on because we're in a bad state
            // and we'll be replacing existing requests in the request map
            // We should probably return an error and have all the places
            // that call this method check if an error occurred and return
            // a new RC (MaximumRequestsExceeded).  But, since it is
            // extremely unlikely that there will ever be > 2G pending/
            // complete requests, we haven't done this yet.
        }
    }
    
    serviceRequest->fRequestNumber = fNextRequest;

    serviceRequest->fProcessingState = kSTAFServiceRequestPending;

    return STAFServiceRequestPtr(serviceRequest, STAFServiceRequestPtr::INIT);
}


STAFRC_t STAFRequestManager::add(STAFServiceRequestPtr &serviceRequest)
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    fRequestMap[serviceRequest->fRequestNumber] = serviceRequest;
    fTotalRequests++;

    return kSTAFOk;
}


STAFRC_t STAFRequestManager::requestCompleted(STAFRequestNumber_t request, 
                                              const STAFServiceResult &result) 
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return kSTAFRequestNumberNotFound;

    STAFServiceRequestPtr theRequest = (*theIterator).second;

    theRequest->fResult = result;
    theRequest->fProcessingState = kSTAFServiceRequestComplete;

    return kSTAFOk;
}


STAFRC_t STAFRequestManager::getResult(STAFRequestNumber_t request, 
                                       STAFServiceResult *result)
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    return doGetResult(request, result);
}


STAFRC_t STAFRequestManager::freeRequest(STAFRequestNumber_t request, 
                                         STAFServiceResult *result)
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return kSTAFRequestNumberNotFound;

    STAFRC_t rc = doGetResult(request, result);
    
    if (rc == kSTAFOk) 
        fRequestMap.erase(request);

    return rc;
}


STAFRC_t STAFRequestManager::deleteRequest(STAFRequestNumber_t request)                     
{
    // This method will remove the request whether or not it is complete

    STAFMutexSemLock requestLock(fRequestMapSem);

    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return kSTAFRequestNumberNotFound;

    fRequestMap.erase(request);

    return kSTAFOk;
}


STAFRC_t STAFRequestManager::getRequestData(STAFRequestNumber_t request, 
                                            STAFServiceRequestPtr &reqData)
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return kSTAFRequestNumberNotFound;

    reqData = (*theIterator).second;

    return kSTAFOk;
}


STAFRC_t STAFRequestManager::getStatus(STAFRequestNumber_t request, 
                                       STAFServiceRequestState_t  *status)
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    return doGetStatus(request, status);
}


bool STAFRequestManager::requestExists(STAFRequestNumber_t request)
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return false;

    return true;
}


STAFRequestManager::RequestMap STAFRequestManager::getRequestMapCopy()
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    return fRequestMap;
}


STAFRequestNumber_t STAFRequestManager::getRequestMapSize()
{
    STAFMutexSemLock requestLock(fRequestMapSem);

    return fRequestMap.size();
}


STAFUInt64_t STAFRequestManager::getTotalRequests()
{
    return fTotalRequests;
}


STAFRequestNumber_t STAFRequestManager::getMaxRequestNumber()
{
    return MAX_REQUEST_NUMBER;
}


STAFRequestNumber_t STAFRequestManager::getMinRequestNumber()
{
    return MIN_REQUEST_NUMBER;
}


STAFRequestNumber_t STAFRequestManager::getMaxActiveRequests()
{
    return MAX_ACTIVE_REQUESTS;
}


unsigned int STAFRequestManager::getResetCount()
{
    return fResetCount;
}


STAFRC_t STAFRequestManager::doGetStatus(STAFRequestNumber_t request, 
                                         STAFServiceRequestState_t  *status)
{
    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return kSTAFRequestNumberNotFound;

    *status = (*theIterator).second->fProcessingState;

    return kSTAFOk;
}


STAFRC_t STAFRequestManager::doGetResult(STAFRequestNumber_t request, 
                                         STAFServiceResult *result)
{
    RequestMap::iterator theIterator = fRequestMap.find(request);

    if (theIterator == fRequestMap.end())
        return kSTAFRequestNumberNotFound;

    STAFServiceRequestState_t reqStatus;
    STAFRC_t rc = doGetStatus(request, &reqStatus);

    if (rc != kSTAFOk) return rc;

    if (reqStatus != kSTAFServiceRequestComplete)
        return kSTAFRequestNotComplete;

    STAFServiceRequestPtr theRequest = (*theIterator).second;

    *result = theRequest->fResult;

    return kSTAFOk;
}
