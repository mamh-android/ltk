/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_RequestManager
#define STAF_RequestManager

#include <map>
#include <vector>
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAFOSTypes.h"
#include "STAF.h"
#include "STAFProc.h"
#include "STAFError.h"
#include "STAFMutexSem.h"
#include "STAFTimestamp.h"
#include "STAFService.h"

// STAFRequestManager - This class manages all the requests used by
//                      STAFProc.    

class STAFRequestManager
{
public:

    typedef std::map<STAFRequestNumber_t, STAFServiceRequestPtr> RequestMap;

    STAFRequestManager();

    STAFServiceRequestPtr getNewServiceRequest();
    STAFRC_t add(STAFServiceRequestPtr &serviceRequest);
    STAFRC_t requestCompleted(STAFRequestNumber_t request, 
                              const STAFServiceResult &result);
    STAFRC_t getResult(STAFRequestNumber_t request, STAFServiceResult *result);
    STAFRC_t freeRequest(STAFRequestNumber_t request, STAFServiceResult *result);
    STAFRC_t deleteRequest(STAFRequestNumber_t request);
    STAFRC_t getRequestData(STAFRequestNumber_t request,
                            STAFServiceRequestPtr &serviceRequest);
    STAFRC_t getStatus(STAFRequestNumber_t request,
                       STAFServiceRequestState_t *status);
    bool requestExists(STAFRequestNumber_t request);
    RequestMap getRequestMapCopy();

    STAFRequestNumber_t getRequestMapSize();
    STAFUInt64_t getTotalRequests();
    STAFRequestNumber_t getMaxRequestNumber();
    STAFRequestNumber_t getMinRequestNumber();
    STAFRequestNumber_t getMaxActiveRequests();
    unsigned int getResetCount();

private:

    // Don't allow copy construction or assignment
    STAFRequestManager(const STAFRequestManager &);
    STAFRequestManager &operator=(const STAFRequestManager &);

    STAFRC_t doGetResult(STAFRequestNumber_t request, STAFServiceResult *result);
    STAFRC_t doGetStatus(STAFRequestNumber_t request,
                         STAFServiceRequestState_t *status);

    STAFRequestNumber_t fNextRequest;
    STAFMutexSem fRequestMapSem;
    RequestMap fRequestMap;

    // Contains the total number of STAF requests that have been submitted
    STAFUInt64_t fTotalRequests;

    // Contains the number of times the maximum value for the request number
    // has been reached and it has been reset back to 1
    unsigned int fResetCount;
};

#endif
