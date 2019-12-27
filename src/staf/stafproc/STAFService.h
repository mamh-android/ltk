/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Service
#define STAF_Service

#include <set>
#include <map>
#include "STAF.h"
#include "STAFServiceInterface.h"
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAFUtil.h"
#include "STAFMutexSem.h"
#include "STAFVariablePool.h"
#include "STAFTimestamp.h"

// STAFServiceResult - Encapsulates the data returned from service request

class STAFServiceResult
{
public:

    // This might be a good place to define an enumeration or some
    // constants for the servicePerformedRC and doShutdown

    STAFServiceResult(STAFRC_t rc,
                      const STAFString &result = STAFString(),
                      unsigned int doShutdown = 0)
        : fRC(rc), fResult(result), fDoShutdown(doShutdown)
    {
        /* Do Nothing */
    }

    STAFServiceResult() : fRC(kSTAFOk), fDoShutdown(0), fResult()
    {
        /* Do Nothing */
    }

    STAFRC_t fRC;
    unsigned int fDoShutdown;
    STAFString fResult;
};


typedef enum STAFServiceRequestState_e
{
    kSTAFServiceRequestPending = 0,
    kSTAFServiceRequestComplete = 1,
    kSTAFServiceRequestStateUnknown = 2
} STAFServiceRequestState_t;


// STAFServiceRequest - Encapsulates the data sent into a service

class STAFServiceRequest
{
public:

    STAFRequestNumber_t       fRequestNumber;
    STAFSyncOption_t          fSyncMode;
    STAFServiceRequestState_t fProcessingState;

    STAFTimestamp       fStartStamp;
    STAFTimestamp       fStopStamp;

    STAFString          fTargetMachine;
    STAFString          fTargetService;

    STAFString          fSTAFInstanceUUID;
    STAFString          fMachine;  
    STAFString          fMachineNickname;
    STAFHandle_t        fHandle;
    STAFString          fHandleName;
    STAFString          fRequest;
    STAFTrustLevel_t    fTrustLevel;
    bool                fIsLocalRequest;

    unsigned int        fDiagEnabled;

    STAFVariablePoolPtr fRequestVarPool;
    STAFVariablePoolPtr fSourceSharedVarPool;
    STAFVariablePool   *fLocalSharedVarPool;
    STAFVariablePool   *fLocalSystemVarPool;

    STAFString          fEndpoint;
    STAFString          fInterface;
    STAFString          fLogicalInterfaceID;
    STAFString          fPhysicalInterfaceID;
    STAFString          fPort;

    STAFString          fAuthenticator;
    STAFString          fUserIdentifier;
    STAFString          fAuthenticationData;
    STAFString          fDefaultAuthenticator;
    STAFString          fUser;

    STAFServiceResult   fResult;

};

typedef STAFRefPtr<STAFServiceRequest> STAFServiceRequestPtr;


// STAFService - An interface that encapsulates a service
//
// This class provides a generic front end to any service.  A particular
// service should be derived from this class.
//
// Facilities are provided to send a request to the service and to obtain the
// name of the service.
//
// Derived classes should override the acceptRequest() method and should
// also initialize STAFService with the correct name of the service.

class STAFService;  // Forward declaration needed for typdef

// STAFServicePtr is also defined in STAFProc.h
typedef STAFRefPtr<STAFService> STAFServicePtr;

class STAFService
{
public:

    enum ServiceState { kNotReady = 0, kInitializing, kReady, kTerminating,
                        kTerminated };

    enum UnregisterableState { kUnregisterable = 1, kNotUnregisterable = 0 };

    STAFService() : fIsUnregisterable(0), fServiceState(kNotReady),
                    fName("UNKNOWN"), fServiceType(kSTAFServiceTypeUnknown),
                    fLoadedBySLS("")
    { /* Do Nothing */ }

    STAFService(const STAFString &name,
                unsigned int isUnregisterable = kNotUnregisterable,
                STAFServiceType_t serviceType = kSTAFServiceTypeService,
                STAFString loadedBySLS = STAFString(""))
        : fIsUnregisterable(isUnregisterable), fServiceState(kNotReady),
          fName(name), fServiceType(serviceType), fLoadedBySLS(loadedBySLS)
    {
        fName.upperCase();
    }

    STAFServiceResult submitRequest(const STAFServiceRequest &requestInfo);

    const STAFString &name() const { return fName; }
    virtual STAFString getLibName() const { return STAFString("<Internal>"); }
    virtual STAFString getExecutable() const { return STAFString(""); }
    virtual STAFString getParameters() const { return STAFString(""); }
    virtual STAFObjectPtr getOptions() const { return STAFObject::createList(); }
    
    virtual STAFString info(unsigned int raw = 0) const = 0;
    ServiceState state();

    void setName(const STAFString &name);

    bool hasProperty(STAFString &name);
    STAFString getProperty(STAFString &name);
    // Returns old value, or empty string if no old value
    STAFString setProperty(STAFString &name, STAFString &value);

    virtual unsigned int isUnregisterable() const { return fIsUnregisterable; }

    STAFServiceType_t serviceType() const { return fServiceType; }
    STAFString getLoadedBySLS() const { return fLoadedBySLS; }

    STAFServiceResult initialize();
    STAFServiceResult terminate(unsigned int timeout = 50000);

    virtual ~STAFService() { /* Do Nothing */ }

#ifdef STAF_OS_NAME_ZOS
    // pthread_t is implementation-specific, and to use std::set, we need to
    // specify a comparison function as a template parameter for z/OS.
    // If we don't do this on z/OS, STAF will hang in STAFService.cpp at line:
    // fThreadList.erase(gThreadManagerPtr->getCurrentThreadID());
    struct ltthread
    {
        bool operator()(STAFThreadID_t t1, STAFThreadID_t t2) const
        {
            return strcmp(t1.__, t2.__) < 0;
        }
    };
    
    typedef std::set<STAFThreadID_t, ltthread> ThreadList;
#else
    typedef std::set<STAFThreadID_t> ThreadList;
#endif

protected:

    ThreadList fThreadList;
    STAFMutexSem fRequestSem;

private:

    typedef std::map<STAFString, STAFString> PropertyMap;

    // Don't allow copy construction or assignment
    STAFService(const STAFService &);
    STAFService &operator=(const STAFService &);

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo) = 0;
    virtual STAFServiceResult init();
    virtual STAFServiceResult term();

    unsigned int fIsUnregisterable;
    ServiceState fServiceState;
    STAFString fName;
    STAFServiceType_t fServiceType;

    // Indicates if the service was loaded by a service loader service.
    // "" indicates NOT loaded by a service loader service.
    // If the service was loaded by a service loader service, then it
    // will contain the registered name of the service loader service
    // that loaded it (e.g. STAFServiceLoader1)
    STAFString fLoadedBySLS;

    PropertyMap fPropertyMap;
};


STAF_EXCEPTION_DEFINITION(STAFServiceException, STAFException);
STAF_EXCEPTION_DEFINITION(STAFServiceCreateException, STAFServiceException);
STAF_EXCEPTION_DEFINITION(STAFServiceInitException, STAFServiceException);

#endif
