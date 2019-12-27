/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ServiceManager
#define STAF_ServiceManager

#include <map>
#include <list>
#include "STAF.h"
#include "STAFString.h"
#include "STAFService.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFProc.h"

// STAFServiceManager - This class manages the set of STAF Services.  It allows
//                      you to add and delete services, retrieve services
//                      by name, and retrieve ordered or sequential lists of
//                      services.  Services include specialized types of
//                      services, such as Service Loaders and Authenticators.

class STAFServiceManager
{
public:

    // XXX: Why doesn't setting to gNoneString instead of "none" work?
    STAFServiceManager() : fDefaultAuthenticator("none")
    { /* Do Nothing */ }

    static bool kTraceEnabled;
    static bool kTraceDisabled;
    static bool kDefaultTraceState;

    static unsigned int kServiceLoadWaitTimeout;

    // Service management methods

    STAFRC_t add(STAFServicePtr theService);
    STAFRC_t addSLS(STAFServicePtr theService);
    STAFRC_t addAuthenticator(STAFServicePtr theService);
    STAFRC_t remove(STAFString name);
    STAFRC_t removeSLS(STAFServicePtr theService);
    STAFRC_t removeAuthenticator(const STAFString &name);
    STAFRC_t replace(STAFServicePtr theService, STAFString newName,
                     STAFString &errorBuffer);
    STAFRC_t rename(STAFString name, STAFString newName,
                    STAFString &errorBuffer);
    STAFRC_t get(STAFString name, STAFServicePtr &theService,
                 STAFString &errorBuffer,
                 bool addingService = false);
    STAFRC_t getAuthenticator(const STAFString &name, STAFServicePtr &theService);
    STAFRC_t verifyServiceName(const STAFString &name);

    // Service list methods

    typedef std::map<STAFString, STAFServicePtr> OrderedServiceList;
    typedef std::list<STAFServicePtr> ServiceList;
    typedef std::map<STAFString, STAFEventSemPtr> SLSLoadServiceMap;

    OrderedServiceList getOrderedServiceListCopy();
    ServiceList getServiceListCopy();
    ServiceList getSLSListCopy();
    OrderedServiceList getAuthenticatorMapCopy();

    // Default authenticator methods

    STAFString getDefaultAuthenticator();
    STAFRC_t setDefaultAuthenticator(const STAFString &name);

    // Service tracing methods

    typedef std::set<STAFString> ServiceStringList;
    typedef std::map<STAFString, bool> ServiceTraceStatusList;

    static void setDefaultTraceState(bool stateToSetTo);
    static bool getDefaultTraceState();
    static void setMaxServiceResultSize(unsigned int maxServiceResultSize);
    static unsigned int getMaxServiceResultSize();
    static void traceSetServices(const STAFString &services);
    static void traceServicesChange(const STAFString &services, bool stateToSetTo);
    static void traceServicesChangeAll(bool stateToSetTo);
    static void purgeUnregisteredServices();
    static ServiceStringList getTraceServices();
    static ServiceTraceStatusList getServiceTraceStatusList();
    static unsigned int doTraceService(const STAFString &service);

    ~STAFServiceManager() { /* Do Nothing */ }

private:

    // Don't allow copy construction or assignment
    STAFServiceManager(const STAFServiceManager &);
    STAFServiceManager &operator=(const STAFServiceManager &);

    STAFMutexSem fServiceListSem;
    static OrderedServiceList fOrderedServiceList;
    ServiceList fServiceList;

    static STAFMutexSem sServiceStringListSem;
    static ServiceStringList sServiceStringList;
    static ServiceTraceStatusList sServiceTraceStatusList;
    static STAFMutexSem sServiceTraceStatusListSem;

    STAFMutexSem fSLSListSem;
    ServiceList fSLSList;

    STAFMutexSem fSLSLoadServiceMapSem;
    SLSLoadServiceMap fSLSLoadServiceMap;

    STAFMutexSem fAuthenticatorMapSem;
    OrderedServiceList fAuthenticatorMap;
    STAFString fDefaultAuthenticator;
};

#endif
