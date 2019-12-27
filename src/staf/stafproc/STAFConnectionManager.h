/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ConnectionManager
#define STAF_ConnectionManager

#include "STAF.h"
#include "STAFString.h"
#include "STAFConnectionProvider.h"
#include "STAFTimestamp.h"

class STAFConnectionManager
{
public:

    STAFConnectionManager();

    typedef std::deque<STAFString> ConnectionProviderOptionList;
    typedef std::map<STAFString, STAFConnectionProviderPtr>
        ConnectionProviderMap;
    typedef std::deque<STAFConnectionProviderPtr> ConnectionProviderList;

    struct EndpointCacheData
    {
        EndpointCacheData()
        { /* Do Nothing */ }
        
        EndpointCacheData(const STAFString &aInterface)
            : interface(aInterface),
              createdTimestamp(STAFTimestamp::now())
        { /* Do Nothing */ }

        STAFString interface;
        STAFTimestamp createdTimestamp;
    };

    typedef std::map<STAFString, EndpointCacheData> EndpointCacheMap;

    STAFRC_t addConnectionProvider(
                 const STAFString &name,
                 const STAFString &library,
                 const ConnectionProviderOptionList &optionList,
                 STAFString &errorBuffer);

    STAFRC_t setDefaultConnectionProvider(const STAFString &name);
    STAFString getDefaultConnectionProvider();
    STAFRC_t enableAutoInterfaceCycling();
    STAFRC_t disableAutoInterfaceCycling();
    bool getAutoInterfaceCycling();
    
    ConnectionProviderList getConnectionProviderListCopy();

    STAFRC_t makeConnection(const STAFString &where,
                            STAFConnectionPtr &connection,
                            STAFString &errorBuffer);

    STAFRC_t makeConnection(const STAFString &where,
                            STAFConnectionProviderPtr &provider,
                            STAFConnectionPtr &connection,
                            STAFString &errorBuffer);

    STAFRC_t attemptConnection(const STAFString &interface,
                               const STAFString &endpoint,
                               STAFConnectionProviderPtr &provider,
                               STAFConnectionPtr &connection,
                               STAFString &errorBuffer);

    STAFRC_t addToEndpointCache(const STAFString &endpoint,
                                const STAFString &interface);

    STAFRC_t removeFromEndpointCache(const STAFString &endpoint);

    STAFRC_t purgeEndpointCache();

    STAFRC_t getCachedInterface(const STAFString &endpoint,
                                STAFString &interface);

    unsigned int getEndpointCacheSize();

    EndpointCacheMap getEndpointCacheMapCopy();

    ~STAFConnectionManager();

private:

    ConnectionProviderMap fConnProvMap;
    ConnectionProviderList fConnProvList;
    EndpointCacheMap fEndpointCacheMap;
    STAFString fDefaultConnProv;
    STAFMutexSem fEndpointCacheMapSem;
    STAFMutexSem fAutoInterfaceCyclingSem;
    bool fAutoInterfaceCycling;
};

#endif



