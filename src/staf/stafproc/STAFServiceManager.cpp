/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFServiceManager.h"
#include "STAFDiagManager.h"
#include "STAFProc.h"

STAFServiceManager::ServiceStringList STAFServiceManager::sServiceStringList;
STAFMutexSem STAFServiceManager::sServiceStringListSem;
STAFServiceManager::ServiceTraceStatusList STAFServiceManager::sServiceTraceStatusList;
STAFMutexSem STAFServiceManager::sServiceTraceStatusListSem;
STAFServiceManager::OrderedServiceList STAFServiceManager::fOrderedServiceList;

// Assign a timeout for waiting for a service to be loaded (30 minutes)
unsigned int STAFServiceManager::kServiceLoadWaitTimeout = 1800000;

bool STAFServiceManager::kTraceEnabled = true;
bool STAFServiceManager::kTraceDisabled= false;
bool STAFServiceManager::kDefaultTraceState = kTraceEnabled;
unsigned int fMaxServiceResultSize = 0;

STAFRC_t STAFServiceManager::add(STAFServicePtr theService)
{
    STAFString serviceName(theService->name());

    {   // Extra block so that lock is released when the block is exited

        // Return an error if the name of the service to be added is already
        // the name of an authenticator

        STAFMutexSemLock authenticatorLock(fAuthenticatorMapSem);

        if (fAuthenticatorMap.find(serviceName) != fAuthenticatorMap.end())
        {
            return kSTAFAlreadyExists;
        }
    }

    {
        STAFMutexSemLock serviceLock(fServiceListSem);

        if (fOrderedServiceList.find(serviceName) != fOrderedServiceList.end())
        {
            return kSTAFAlreadyExists;
        }

        fOrderedServiceList[serviceName] = theService;
        fServiceList.push_back(theService);
    }

    {
        STAFMutexSemLock serviceLock(sServiceTraceStatusListSem);

        sServiceTraceStatusList[serviceName] = kDefaultTraceState;
    }

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::addSLS(STAFServicePtr theService)
{
    STAFMutexSemLock serviceLock(fSLSListSem);

    fSLSList.push_back(theService);

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::addAuthenticator(STAFServicePtr theService)
{
    STAFString serviceName(theService->name());

    // Verify that the authenticator name is not the name of a service

    {   // Extra block so that lock is released when the block is exited

        STAFMutexSemLock serviceLock(fServiceListSem);

        if (fOrderedServiceList.find(serviceName) != fOrderedServiceList.end())
        {
            return kSTAFAlreadyExists;
        }
    }

    STAFMutexSemLock authenticatorLock(fAuthenticatorMapSem);

    if (fAuthenticatorMap.find(serviceName) != fAuthenticatorMap.end())
    {
        return kSTAFAlreadyExists;
    }

    fAuthenticatorMap[serviceName] = theService;

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::remove(STAFString name)
{
    name.upperCase();

    STAFMutexSemLock serviceLock(fServiceListSem);

    if (fOrderedServiceList.find(name) == fOrderedServiceList.end())
        return kSTAFDoesNotExist;

    ServiceList::iterator listIter;

    for(listIter = fServiceList.begin(); listIter != fServiceList.end();
        ++listIter)
    {
        if ((*listIter)->name() == name)
        {
            // Don't allow removing STAF internal services
            if (! (*listIter)->isUnregisterable())
                return kSTAFServiceNotUnregisterable;
            else
                fOrderedServiceList.erase(name);

            fServiceList.erase(listIter);
            break;
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::removeSLS(STAFServicePtr theService)
{
    STAFMutexSemLock serviceLock(fSLSListSem);

    // XXX: This should be done with some form of erase_if or remove_if

    ServiceList::iterator listIter;

    for(listIter = fSLSList.begin(); listIter != fSLSList.end();
        ++listIter)
    {
        if ((*listIter) == theService)
        {
            fSLSList.erase(listIter);
            break;
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::removeAuthenticator(const STAFString &name)
{
    STAFString upperName = name.toUpperCase();

    STAFMutexSemLock serviceLock(fAuthenticatorMapSem);

    if (fAuthenticatorMap.find(upperName) == fAuthenticatorMap.end())
        return kSTAFDoesNotExist;

    fAuthenticatorMap.erase(upperName);

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::replace(STAFServicePtr theService,
                                     STAFString newName,
                                     STAFString &errorBuffer)
{
    STAFServicePtr service;
    STAFString name = theService->name();
    STAFString upperName = name.upperCase();

    {
        STAFMutexSemLock serviceLock(fServiceListSem);

        // Make sure that service being replaced already exists
        if (fOrderedServiceList.find(upperName) == fOrderedServiceList.end())
        {
            errorBuffer = name;
            return kSTAFDoesNotExist;
        }

        // If the existing service is being renamed, make sure no service
        //  already exists with this new name.
        if (newName != STAFString())
        {
            if (fOrderedServiceList.find(newName.toUpperCase()) !=
                fOrderedServiceList.end())
            {
                errorBuffer = newName;
                return kSTAFAlreadyExists;
            }

            STAFMutexSemLock authenticatorLock(fAuthenticatorMapSem);

            if (fAuthenticatorMap.find(newName.toUpperCase()) !=
                fAuthenticatorMap.end())
            {
                errorBuffer = newName;
                return kSTAFAlreadyExists;
            }
        }

        ServiceList::iterator listIter;

        for(listIter = fServiceList.begin(); listIter != fServiceList.end();
            ++listIter)
        {
            if ((*listIter)->name() == upperName)
            {
                // Don't allow removing STAF internal services
                if (! (*listIter)->isUnregisterable())
                {
                    errorBuffer = name;
                    return kSTAFServiceNotUnregisterable;
                }

                service = *listIter;

                fOrderedServiceList.erase(upperName);

                if (newName == STAFString())
                    fServiceList.erase(listIter);

                break;
            }
        }

        // Add the new service
        fOrderedServiceList[theService->name()] = theService;
        fServiceList.push_back(theService);

        if (newName != STAFString())
        {
            // Change the old service's name and add back to the service list
            service->setName(newName);
            fOrderedServiceList[newName.toUpperCase()] = service;
        }
    }

    if (newName == STAFString())
    {
        // Terminate the service that was replaced
        STAFServiceResult result = service->terminate();
    }

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::rename(STAFString name, STAFString newName,
                                    STAFString &errorBuffer)
{
    STAFServicePtr service;
    STAFString upperName = name.toUpperCase();

    STAFMutexSemLock serviceLock(fServiceListSem);

    // Make sure that service being rename already exists
    if (fOrderedServiceList.find(upperName) == fOrderedServiceList.end())
    {
        errorBuffer = name;
        return kSTAFDoesNotExist;
    }

    // Make sure no service already exists with this new name.
    if (fOrderedServiceList.find(newName.toUpperCase()) !=
        fOrderedServiceList.end())
    {
        errorBuffer = newName;
        return kSTAFAlreadyExists;
    }

    {   // Extra block so that lock is released when the block is exited

        STAFMutexSemLock authenticatorLock(fAuthenticatorMapSem);

        if (fAuthenticatorMap.find(newName.toUpperCase()) !=
            fAuthenticatorMap.end())
        {
            errorBuffer = newName;
            return kSTAFAlreadyExists;
        }
    }

    ServiceList::iterator listIter;

    for(listIter = fServiceList.begin(); listIter != fServiceList.end();
        ++listIter)
    {
        if ((*listIter)->name() == upperName)
        {
            // Don't allow renaming STAF internal services
            if (! (*listIter)->isUnregisterable())
            {
                errorBuffer = name;
                return kSTAFServiceNotUnregisterable;
            }

            service = *listIter;

            fOrderedServiceList.erase(name);

            // Change the old service's name & add back to the service list
            service->setName(newName);
            fOrderedServiceList.erase(upperName);
            fOrderedServiceList[newName.toUpperCase()] = service;

            break;
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFServiceManager::get(STAFString name, STAFServicePtr &theService,
                                 STAFString &errorBuffer, bool addingService)
{
    name.upperCase();

    {   // Extra block so that lock is released when the block is exited

        // If the service exists, get the service from the service map

        STAFMutexSemLock serviceLock(fServiceListSem);

        if (fOrderedServiceList.find(name) != fOrderedServiceList.end())
        {
            theService = fOrderedServiceList[name];
            return kSTAFOk;
        }
    }

    if (addingService)
    {
        // Check if the name of the service to be added is already the name
        // of an authenticator

        STAFMutexSemLock authenticatorLock(fAuthenticatorMapSem);

        if (fAuthenticatorMap.find(name) != fAuthenticatorMap.end())
        {
            return kSTAFOk;
        }
    }
    else
    {
        // Service was not found.
        // If we are not adding a new service, call the SLSs

        // Note: We can't lock fSLSListSem before looping through all SLSs
        // because a SLS could submit a request to a service that needs to be
        // loaded by another SLS which would cause a deadlock (e.g. the
        // HttpServiceLoader may submit a request to the ZIP service which is
        // loaded by the DefaultSLS).  We don't need to lock fSLSListSem
        // because we don't currently allow a SLS to be dynamically added or
        // removed.

        // Don't allow a service to be loaded multiple times simultaneously.
        // This is done by managing a map of services currently being loaded
        // by service loader services whose keys are the names of services
        // being loaded and whose values are event semaphore pointers.  When
        // attempting to load a service, if the service isn't in the
        // fSLSLoadServiceMap, a new event semaphore for the service is
        // created and added to the map and any other requests to load this
        // service will wait on this event semaphore.
        
        STAFEventSemPtr eventSemPtr;
        bool serviceBeingLoaded = false;

        {
            STAFMutexSemLock lock(fSLSLoadServiceMapSem);

            if (fSLSLoadServiceMap.find(name) != fSLSLoadServiceMap.end())
            {
                serviceBeingLoaded = true;
                eventSemPtr = fSLSLoadServiceMap[name];
            }
            else
            {
                eventSemPtr = STAFEventSemPtr(
                    new STAFEventSem, STAFEventSemPtr::INIT);
                fSLSLoadServiceMap[name] = eventSemPtr;
            }
        }

        if (serviceBeingLoaded)
        {
            // Wait for up to maximum wait time while the service is loaded

            eventSemPtr->wait(kServiceLoadWaitTimeout);
        
            // Check whether the service was loaded while we were waiting

            STAFMutexSemLock serviceLock(fServiceListSem);

            if (fOrderedServiceList.find(name) != fOrderedServiceList.end())
            {
                theService = fOrderedServiceList[name];
                return kSTAFOk;
            }
            else
            {
                errorBuffer = STAFString(
                    " service could not be loaded by a service loader "
                    "service (Timed out)");
                return kSTAFDoesNotExist;
            }
        }

        // Loop through all SLSs to see if any can load the service

        ServiceList::iterator listIter;
        bool serviceLoaded = false;

        for(listIter = fSLSList.begin(); listIter != fSLSList.end();
            ++listIter)
        {
            // Submit LOAD SERVICE request to a service loader service (SLS)

            STAFServiceRequest requestInfo;

            requestInfo.fMachine = "local";
            requestInfo.fMachineNickname = *gMachineNicknamePtr;
            requestInfo.fHandle = gSTAFProcHandle;
            requestInfo.fHandleName = "ServiceLoader";   // XXX
            requestInfo.fRequest = "LOAD SERVICE " + name;
            requestInfo.fRequestNumber = 0;
            requestInfo.fDiagEnabled = gDiagManagerPtr->getEnabled();
            requestInfo.fTrustLevel = 5;    // XXX
            requestInfo.fAuthenticator = gNoneString;
            requestInfo.fUserIdentifier = gAnonymousString;
            requestInfo.fUser = gUnauthenticatedUser;
            requestInfo.fInterface = "local";
            requestInfo.fEndpoint = "local" + gSpecSeparator + "local";
            requestInfo.fPhysicalInterfaceID = "local";

            STAFServiceResult serviceResult =
                              (*listIter)->submitRequest(requestInfo);

            if (serviceResult.fRC != kSTAFOk)
            {
                errorBuffer =  STAFString(" service could not be loaded by ") +
                               (*listIter)->name() +
                               STAFString(" (") + (*listIter)->getLibName() +
                               STAFString("): RC: ") + serviceResult.fRC +
                               STAFString(" Result: ") + serviceResult.fResult;
            }
            else
            {
                STAFMutexSemLock serviceLock(fServiceListSem);

                if (fOrderedServiceList.find(name) !=
                    fOrderedServiceList.end())
                {
                    theService = fOrderedServiceList[name];
                    serviceLoaded = true;
                    break;
                }
            }
        }

        // Post EventSemPtr for the service and remove it from the map

        {
            STAFMutexSemLock lock(fSLSLoadServiceMapSem);

            fSLSLoadServiceMap.erase(name);
            eventSemPtr->post();
        }

        if (serviceLoaded) return kSTAFOk;
    }

    return kSTAFDoesNotExist;
}


STAFRC_t STAFServiceManager::getAuthenticator(const STAFString &name,
                                              STAFServicePtr &theService)
{
    STAFString upperName = name.toUpperCase();

    {
        STAFMutexSemLock serviceLock(fAuthenticatorMapSem);

        if (fAuthenticatorMap.find(upperName) != fAuthenticatorMap.end())
        {
            theService = fAuthenticatorMap[upperName];
            return kSTAFOk;
        }
    }

    return kSTAFDoesNotExist;
}


STAFServiceManager::OrderedServiceList STAFServiceManager::
                                       getOrderedServiceListCopy()
{
    STAFMutexSemLock serviceLock(fServiceListSem);

    return fOrderedServiceList;
}


STAFServiceManager::ServiceList STAFServiceManager::getServiceListCopy()
{
    STAFMutexSemLock serviceLock(fServiceListSem);

    return fServiceList;
}


STAFServiceManager::ServiceList STAFServiceManager::getSLSListCopy()
{
    STAFMutexSemLock serviceLock(fSLSListSem);

    return fSLSList;
}


STAFServiceManager::OrderedServiceList STAFServiceManager::
                                       getAuthenticatorMapCopy()
{
    STAFMutexSemLock serviceLock(fAuthenticatorMapSem);

    return fAuthenticatorMap;
}


STAFString STAFServiceManager::getDefaultAuthenticator()
{
    return fDefaultAuthenticator;
}


STAFRC_t STAFServiceManager::setDefaultAuthenticator(const STAFString &name)
{
    // Verify that the authenticator name specified is registered

    STAFString upperAuthName = name.toUpperCase();

    {
        STAFMutexSemLock serviceLock(fAuthenticatorMapSem);

        if (fAuthenticatorMap.find(upperAuthName) == fAuthenticatorMap.end())
        {
            return kSTAFDoesNotExist;
        }
    }

    fDefaultAuthenticator = name;

    return kSTAFOk;
}


// This method is called to verify that the name specified for a service
// to be added is a valid name.  Returns 0 to indicate the service name is
// valid if none of the following conditions are true:
// 1) Service name (upper-case) = "NONE"
// 2) Service name (upper-case) begins with "STAF".  This is only valid
//    for Service Loader services whose names are generated by STAF (and
//    thus, their names are not verified
// 3) Service name contains certain special characters, such that the name
//    cannot be used as a directory name to store data for the service.

STAFRC_t STAFServiceManager::verifyServiceName(const STAFString &name)
{
    STAFString upperName = name.toUpperCase();

    if ((upperName == "NONE") ||
        (upperName.length() >= 4 && upperName.subString(0,4) == "STAF") ||
        (upperName.findFirstOf("~!#$%^&*()+={[}]|;:?/<>\\") !=
         STAFString::kNPos))
    {
        return kSTAFInvalidValue;
    }

    return kSTAFOk;
}

// pass in kTraceEnabled to enable the services, or kTraceDisabled to disable
void STAFServiceManager::traceServicesChange(const STAFString &services,
    bool stateToSetTo)
{
    STAFMutexSemLock lock(sServiceTraceStatusListSem);

    for (int i = 0; i < services.numWords(); ++i)
    {
        STAFString serviceName = services.subWord(i, 1).upperCase();

        //XXX: if CHECK option is added, need to add a check here
        // to make sure the service isn't already enabled
        sServiceTraceStatusList[serviceName] = stateToSetTo;
    }
}

// pass in kTraceEnabled to enable the services, or kTraceDisabled to disable
void STAFServiceManager::traceServicesChangeAll(bool stateToSetTo)
{
    STAFMutexSemLock lock(sServiceTraceStatusListSem);

    ServiceTraceStatusList::iterator serviceTraceStatusListIter;

    for(serviceTraceStatusListIter = sServiceTraceStatusList.begin();
        serviceTraceStatusListIter != sServiceTraceStatusList.end();
        serviceTraceStatusListIter++)
    {
        serviceTraceStatusListIter->second = stateToSetTo;
    }
}

void STAFServiceManager::traceSetServices(const STAFString &services)
{
    traceServicesChangeAll(kTraceDisabled);
    traceServicesChange(services, kTraceEnabled);
}

void STAFServiceManager::purgeUnregisteredServices()
{
    ServiceTraceStatusList::iterator serviceTraceStatusListIter;

    STAFMutexSemLock lock(sServiceTraceStatusListSem);

    STAFString listToDelete = "";
    for (serviceTraceStatusListIter = sServiceTraceStatusList.begin();
        serviceTraceStatusListIter != sServiceTraceStatusList.end();
        serviceTraceStatusListIter++)
    {
        if (fOrderedServiceList.find(serviceTraceStatusListIter->first)
                == fOrderedServiceList.end())
        {
            listToDelete += serviceTraceStatusListIter->first + " ";
        }

    }
    for(int i = 0; i < listToDelete.numWords(); i++)
    {
        sServiceTraceStatusList.erase(listToDelete.subWord(i, 1));
    }
}

unsigned int STAFServiceManager::doTraceService(const STAFString &service)
{
    STAFMutexSemLock lock(sServiceTraceStatusListSem);

    if (sServiceTraceStatusList.size() == 0) return 1;

    if (sServiceTraceStatusList.find(service.toUpperCase()) !=
        sServiceTraceStatusList.end())
    {
        // Service found in the trace status list - Check if enabled

        if (sServiceTraceStatusList[service.toUpperCase()])
            return 1;
        else
            return 0;
    }
    
    // Service not found in the trace status list

    return 0;
}

STAFServiceManager::ServiceStringList STAFServiceManager::getTraceServices()
{
    ServiceStringList allTraceServices;
    ServiceTraceStatusList::iterator serviceTraceStatusListIter;

    STAFMutexSemLock lock(sServiceTraceStatusListSem);

    for (serviceTraceStatusListIter = sServiceTraceStatusList.begin();
        serviceTraceStatusListIter != sServiceTraceStatusList.end();
        serviceTraceStatusListIter++)
    {
       allTraceServices.insert(serviceTraceStatusListIter->first);
    }

    return allTraceServices;
}

STAFServiceManager::ServiceTraceStatusList STAFServiceManager::getServiceTraceStatusList()
{
    return sServiceTraceStatusList;
}

void STAFServiceManager::setDefaultTraceState(bool stateToSetTo)
{
    kDefaultTraceState = stateToSetTo;
}
bool STAFServiceManager::getDefaultTraceState()
{
    return kDefaultTraceState;
}

void STAFServiceManager::setMaxServiceResultSize(unsigned int maxServiceResultSize)
{
    fMaxServiceResultSize = maxServiceResultSize;
}
unsigned int STAFServiceManager::getMaxServiceResultSize()
{
    return fMaxServiceResultSize;
}
