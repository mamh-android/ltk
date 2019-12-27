/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFProc.h"
#include "STAFException.h"
#include "STAFConnectionManager.h"
#include "STAFConnectionProvider.h"
#include "STAFThreadManager.h"
#include "STAFTrace.h"

STAFConnectionManager::STAFConnectionManager()
{
    fAutoInterfaceCycling = true;   // Enable by default
}


STAFRC_t STAFConnectionManager::addConnectionProvider(
    const STAFString &name,
    const STAFString &library,
    const ConnectionProviderOptionList &optionList,
    STAFString &errorBuffer)
{
    STAFConnectionProviderPtr theConnProv;

    try
    {
        // Check if connection provider with this name already exists

        if (fConnProvMap.find(name.toLowerCase()) != fConnProvMap.end())
        {
            errorBuffer = "Interface " + name.toLowerCase() + " already exists";
            return kSTAFAlreadyExists;
        }

        STAFConnectionProviderConstructInfoLevel1 constructInfo =
        { kSTAFConnectionProviderDuplex, 0 };

        STAFBuffer<STAFString> optionNames(new STAFString[optionList.size()],
                                           STAFBuffer<STAFString>::INIT,
                                           STAFBuffer<STAFString>::ARRAY);
        STAFBuffer<STAFString> optionValues(new STAFString[optionList.size()],
                                            STAFBuffer<STAFString>::INIT,
                                            STAFBuffer<STAFString>::ARRAY);

        STAFBuffer<STAFStringConst_t> optionNameImpls(
            new STAFStringConst_t[optionList.size()],
            STAFBuffer<STAFStringConst_t>::INIT,
            STAFBuffer<STAFStringConst_t>::ARRAY);
        STAFBuffer<STAFStringConst_t> optionValueImpls(
            new STAFStringConst_t[optionList.size()],
            STAFBuffer<STAFStringConst_t>::INIT,
            STAFBuffer<STAFStringConst_t>::ARRAY);

        for (int i = 0; i < optionList.size(); ++i)
        {
            unsigned int equalPos = optionList[i].find(kUTF8_EQUAL);

            optionNames[i] = optionList[i].subString(0, equalPos);
            optionValues[i] = (equalPos == STAFString::kNPos) ?
                                  STAFString() :
                                  optionList[i].subString(equalPos + 1);

            optionNameImpls[i] = optionNames[i].getImpl();
            optionValueImpls[i] = optionValues[i].getImpl();
        }

        constructInfo.numOptions = optionList.size();
        constructInfo.optionNames = optionNameImpls;
        constructInfo.optionValues = optionValueImpls;

        STAFConnectionProviderPtr theConnProv = 
            STAFConnectionProvider::createRefPtr(name, library,
                                                 &constructInfo, 1);

        // Check if a STAFTCP connection provider is being added and.
        // if so, make sure its port number is unique by checking if any
        // other STAFTCP connection provider is already using this port.

        STAFString theLibrary = theConnProv->getLibrary();

        if (theLibrary == "STAFTCP")
        {
            STAFString thePort = theConnProv->getProperty(
                kSTAFConnectionProviderPortProperty);

            for (ConnectionProviderList::iterator iter = fConnProvList.begin();
                 iter != fConnProvList.end(); ++iter)
            {
                if (theLibrary == (*iter)->getLibrary())
                {
                    if (thePort == (*iter)->getProperty(
                        kSTAFConnectionProviderPortProperty))
                    {
                        // Return an error as each STAFTCP connection provider
                        // must use a unique port number

                        if (thePort.find(kUTF8_AT) == 0)
                            thePort = thePort.subString(1);

                        errorBuffer = STAFString("Interface ") +
                            (*iter)->getName() + " is already using port " +
                            thePort;

                        return kSTAFInvalidValue;
                    }
                }
            }
        }

        // XXX: Hmmm... Not sure we should be doing this here
        // theConnProv->start(HandleRequest);

        fConnProvMap[name.toLowerCase()] = theConnProv;
        fConnProvList.push_back(theConnProv);

        if ((fDefaultConnProv == "") || (fDefaultConnProv == "local"))
            fDefaultConnProv = name;
    }
    catch (STAFException &se)
    {
        errorBuffer = se.getText();
        return se.getErrorCode();
    }

    return kSTAFOk;
}


STAFRC_t STAFConnectionManager::setDefaultConnectionProvider(
    const STAFString &name)
{
    ConnectionProviderMap::iterator iter = fConnProvMap.find(name.toLowerCase());

    if (iter == fConnProvMap.end())
        return kSTAFDoesNotExist;
    
    fDefaultConnProv = name;

    return kSTAFOk;
}


STAFString STAFConnectionManager::getDefaultConnectionProvider()
{
    return fDefaultConnProv;
}


STAFRC_t STAFConnectionManager::enableAutoInterfaceCycling()
{
    STAFMutexSemLock lock(fAutoInterfaceCyclingSem);
    fAutoInterfaceCycling = true;
    return kSTAFOk;
}


STAFRC_t STAFConnectionManager::disableAutoInterfaceCycling()
{
    STAFMutexSemLock lock(fAutoInterfaceCyclingSem);
    fAutoInterfaceCycling = false;
    return kSTAFOk;
}


bool STAFConnectionManager::getAutoInterfaceCycling()
{
    STAFMutexSemLock lock(fAutoInterfaceCyclingSem);
    return fAutoInterfaceCycling;
}


STAFConnectionManager::ConnectionProviderList
    STAFConnectionManager::getConnectionProviderListCopy()
{
    return fConnProvList;
}


STAFRC_t STAFConnectionManager::makeConnection(const STAFString &where,
                                               STAFConnectionPtr &connection,
                                               STAFString &errorBuffer)
{
    STAFConnectionProviderPtr notUsed;

    return makeConnection(where, notUsed, connection, errorBuffer);
}


STAFRC_t STAFConnectionManager::makeConnection(
    const STAFString &where, STAFConnectionProviderPtr &provider,
    STAFConnectionPtr &connection, STAFString &errorBuffer)
{
    static STAFString sLocal("local");
    STAFRC_t rc = kSTAFOk;
    STAFString endpoint = where;
    STAFString connProvName = fDefaultConnProv;
    bool interfaceSpecified = true;
    bool useCachedEndpoint = false;

    // Check if an interface was specified in the 'where' value

    unsigned int sepPos = where.find(gSpecSeparator);

    if (sepPos != STAFString::kNPos)
    {
        connProvName = where.subString(0, sepPos);
        endpoint = where.subString(sepPos + gSpecSeparator.length());
    }
    else if (where.toLowerCase() == sLocal)
    {
        connProvName = sLocal;
    }
    else if (fAutoInterfaceCycling)
    {
        interfaceSpecified = false;

        // If the endpoint is found in the Endpoint Cache Map, assign the
        // connection provider (aka interface) name cached for the endpoint

        STAFString interface;

        if (getCachedInterface(endpoint, connProvName) == kSTAFOk)
        {
            useCachedEndpoint = true;
        }
    }

    // Make sure connection provider exists and get the connection provider

    ConnectionProviderMap::iterator cpIter = fConnProvMap.find(
        connProvName.toLowerCase());

    if (cpIter == fConnProvMap.end())
    {
        errorBuffer = "No such interface: " + connProvName +
            ", Endpoint: " + where;
        return kSTAFNoPathToMachine;
    }

    provider = cpIter->second;

    // Attempt to connect to the endpoint using this connection provider

    rc = attemptConnection(connProvName, endpoint, provider, connection,
                           errorBuffer);

    if ((rc != kSTAFOk) && useCachedEndpoint)
    {
        removeFromEndpointCache(endpoint);
    }

    if ((rc == kSTAFOk) || interfaceSpecified || !fAutoInterfaceCycling)
    {
        // Return because the connection worked or because the connection
        // failed and an explicit interface was specified or automatic
        // interface cycling is not enabled

        return rc;
    }

    // Don't overwrite the error message from the first attempt, so that the
    // rc and error message from the 1st connect attempt are returned if still
    // can't connect using other connection providers.

    STAFString errorBuffer2 = "";
    
    // Iterate through the remaining connection providers and try to connect
    // to the endpoint using another connection provider

    for (STAFConnectionManager::ConnectionProviderList::iterator
         iter = fConnProvList.begin(); iter != fConnProvList.end(); ++iter)
    {
        STAFString interface = (*iter)->getName();

        if ((interface == connProvName) || (interface == sLocal))
        {
            // Skip the connProvName that already we tried and skip the local
            // connection provider
            continue;
        }
        
        if (attemptConnection(interface, endpoint, *iter, connection, 
                              errorBuffer2) == kSTAFOk)
        {
            // Connection worked - Assign the provider, add to the endpoint
            // cache and exit

            provider = *iter;
            addToEndpointCache(endpoint, interface);
            return kSTAFOk;
        }
    }

    return rc;
}


STAFRC_t STAFConnectionManager::attemptConnection(
    const STAFString &interface, const STAFString &endpoint,
    STAFConnectionProviderPtr &provider, STAFConnectionPtr &connection,
    STAFString &errorBuffer)
{
    STAFRC_t rc = kSTAFOk;

    // Attempt to make a connection up to the number of times specified by
    // gConnectionAttempts until a connection is made.

    for (int attempt = 1; attempt <= gConnectionAttempts; ++attempt)
    {
        rc = kSTAFOk;

        try
        {
            connection = provider->connect(endpoint);
        }
        catch (STAFException &e)
        {
            rc = kSTAFNoPathToMachine;
            errorBuffer = e.getText() + STAFString(": ") +
                STAFString(e.getErrorCode()) + ", Endpoint: " +
                interface + gSpecSeparator + endpoint;
        }
        catch (...)
        {
            rc = kSTAFUnknownError;
        }

        if (rc == kSTAFOk) return rc;  // Worked, so don't retry

        int delayTime = 0;

        // Sleep (except after last connection attempt)
        if (attempt < gConnectionAttempts)
        {
            // Sleep a random number of milliseconds between 1 and the
            // gConnectionRetryDelay to reduce the # of connection attempts
            // needed when getting RC 16 recv: 111 errors that occur when the
            // queue for pending socket requests is full (Bug #915342)

            delayTime = rand() % gConnectionRetryDelay;
            STAFThreadManager::sleepCurrentThread(delayTime);
        }

        STAFTrace::trace(
            kSTAFTraceWarning,
            "STAFConnectionManager::makeConnection - Attempt #" +
            STAFString(attempt) + " of " + gConnectionAttempts +
            " (Delay " + delayTime + " milliseconds), RC: " + rc +
            ", Result: " + errorBuffer);
    }

    return rc;
}


STAFRC_t STAFConnectionManager::addToEndpointCache(const STAFString &endpoint,
                                                   const STAFString &interface)
{
    // Don't add to cache if using the default connection provider since that
    // will be the first one tried anyway

    if (interface == fDefaultConnProv) return kSTAFOk;

    // Add the endpoint to the endpoint cache map

    STAFMutexSemLock cacheLock(fEndpointCacheMapSem);

    EndpointCacheData endpointData(interface);
    fEndpointCacheMap[endpoint] = endpointData;

    return kSTAFOk;
}


STAFRC_t STAFConnectionManager::removeFromEndpointCache(
    const STAFString &endpoint)
{
    // Remove endpoint from the endpoint cache map

    STAFMutexSemLock cacheLock(fEndpointCacheMapSem);

    if (fEndpointCacheMap.find(endpoint) != fEndpointCacheMap.end())
    {
        fEndpointCacheMap.erase(endpoint);
        return kSTAFOk;
    }
    else
    {
        return kSTAFDoesNotExist;
    }
}


STAFRC_t STAFConnectionManager::purgeEndpointCache()
{
    // Remove all entries from the endpoint cache map

    STAFMutexSemLock cacheLock(fEndpointCacheMapSem);

    fEndpointCacheMap = EndpointCacheMap();

    return kSTAFOk;
}

// STAFConnectionManager::getEndpointCacheData
//
// Description:
//   Get the cached information for an endpoint
//
// Parameters:
//   endpoint  - endpoint (Input)
//   interface - the interface (aka connection provider name) cached for the
//               specified endpoint (Output)
//
// Returns:
//   rc 0 if the endpoint was cached
//   Non-zero rc if the specified endpoint did not exist in the cache

STAFRC_t STAFConnectionManager::getCachedInterface(
    const STAFString &endpoint, STAFString &interface)
{
    STAFRC_t rc = kSTAFOk;

    STAFMutexSemLock cacheLock(fEndpointCacheMapSem);

    if (fEndpointCacheMap.find(endpoint) != fEndpointCacheMap.end())
    {
        EndpointCacheData endpointData = fEndpointCacheMap[endpoint];
        interface = endpointData.interface;
    }
    else
    {
        rc = kSTAFDoesNotExist;
    }

    return rc;
}


unsigned int STAFConnectionManager::getEndpointCacheSize()
{
    STAFMutexSemLock cacheLock(fEndpointCacheMapSem);

    return fEndpointCacheMap.size();
}


STAFConnectionManager::EndpointCacheMap STAFConnectionManager::
    getEndpointCacheMapCopy()
{
    STAFMutexSemLock cacheLock(fEndpointCacheMapSem);

    return fEndpointCacheMap;
}


STAFConnectionManager::~STAFConnectionManager()
{
    /* Do Nothing */
}
