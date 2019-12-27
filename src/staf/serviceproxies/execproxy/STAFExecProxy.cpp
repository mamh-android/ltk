/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFExecProxyLib.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAFUtil.h"
#include "STAFEventSem.h"
#include "STAFConnectionProvider.h"
#include "STAFServiceInterface.h"
#include <stdlib.h>
#include <list>
#include <vector>

STAFEventSemPtr gExitSemaphore;
STAFEventSemPtr *gExitSemaphorePtr = &gExitSemaphore;

STAFRC_t HandleRequest(const STAFConnectionProvider *provider,
                       STAFConnectionPtr &connection);
void HandleServiceConstruct(STAFConnectionPtr &connection);
void HandleServiceInit(STAFConnectionPtr &connection);
void HandleServiceRequest(STAFConnectionPtr &connection);
void HandleServiceTerm(STAFConnectionPtr &connection);
void HandleServiceDestruct(STAFConnectionPtr &connection);
void initConnection(char* serviceName);
unsigned int getLevel(STAFServiceLevelID levelID,
                       const char *levelString,
                       unsigned int stafMin,
                       unsigned int stafMax);

STAFDynamicLibrary_t fImplLib = 0;

STAFServiceGetLevelBounds_t fGetBounds;
unsigned int fLevelConstruct;
STAFServiceConstruct_t fConstruct;
unsigned int fLevelInit;
STAFServiceInit_t fInit;
unsigned int fLevelAcceptRequest;
STAFServiceAcceptRequest_t fAcceptRequest;
unsigned int fLevelTerm;
STAFServiceTerm_t fTerm;
unsigned int fLevelDestruct;
STAFServiceDestruct_t fDestruct;

STAFServiceHandle_t fServiceHandle;

static STAFString sIPCName("IPCNAME");

int main(int argc, char **argv)
{
    gExitSemaphore = STAFEventSemPtr(new STAFEventSem(),
                                     STAFEventSemPtr::INIT);

    initConnection(argv[1]);

    // Wait for signal to end

    gExitSemaphore->wait();

    return 0;
}

void initConnection(char* serviceName)
{
    try
    {
        // interprocess communication name
        STAFString ipcName = STAFString(serviceName);

        //option data to create a connection provider
        STAFStringConst_t optionData[] = { sIPCName.getImpl(),
                                           ipcName.getImpl() };
        STAFConnectionProviderConstructInfoLevel1 constructInfo =
        {
            kSTAFConnectionProviderInbound,
            1,
            optionData,
            &optionData[1]
        };

        STAFConnectionProvider *connProv =
            STAFConnectionProvider::create(ipcName, "STAFLIPC",
                                                 &constructInfo, 1);
        // when the connProv receives traffic, dispatch it to handleRequest
        connProv->start(HandleRequest);
    }
    catch (STAFException &se)
    {
        se.write("STAFExecProxy.initConnection()");
    }
    catch (...)
    {
        cout << "Caught unknown exception in STAFExecProxy.initConnection()"
             << endl;
    }
}

STAFRC_t HandleRequest(const STAFConnectionProvider *provider,
                       STAFConnectionPtr &connection)
{
    try
    {
        unsigned reqType = connection->readUInt();

        switch (reqType)
        {
            case STAFEXECPROXY_PING:
            {
                connection->writeUInt(kSTAFOk);
                connection->writeString(STAFString());
                break;
            }
            case STAFEXECPROXY_LOAD:
            {
                HandleServiceConstruct(connection);
                break;
            }
            case STAFEXECPROXY_INIT:
            {
                HandleServiceInit(connection);
                break;
            }
            case STAFEXECPROXY_ACCEPT_REQUEST:
            {
                HandleServiceRequest(connection);
                break;
            }
            case STAFEXECPROXY_TERM:
            {
                HandleServiceTerm(connection);
                break;
            }
            case STAFEXECPROXY_DESTRUCT:
            {
                HandleServiceDestruct(connection);
                gExitSemaphore->post();
                break;
            }
            default:
            {
                connection->writeUInt(kSTAFInvalidValue);
                connection->writeString(STAFString("Invalid STAFExecProxy ") +
                                        "request type: " + STAFString(reqType));
                break;
            }
        }
    }
    catch (STAFException &se)
    {
        se.write("STAFExecProxy.HandleRequest()");
    }
    catch (...)
    {
        cout << "Caught unknown exception in STAFExecProxy.HandleRequest()"
             << endl;
        gExitSemaphore->post();
    }

    return 0;
}

void HandleServiceConstruct(STAFConnectionPtr &connection)
{
    STAFString serviceName   = connection->readString();
    STAFString execInfo = connection->readString();
    STAFString libraryInfo = connection->readString();
    STAFString writeLocation = connection->readString();

    STAFServiceType_t serviceType =
        static_cast<STAFServiceType_t>(connection->readUInt());

    unsigned int numOptions = connection->readUInt();

    STAFString_t *optionNames = new STAFString_t[numOptions];
    STAFString_t *optionValues = new STAFString_t[numOptions];

    for (unsigned int k = 0; k < numOptions; ++k)
    {
        STAFString optionName = connection->readString();
        STAFString optionValue = connection->readString();

        optionNames[k] = optionName.adoptImpl();
        optionValues[k] = optionValue.adoptImpl();
    }

    STAFString_t errorBuffer = 0;
    STAFRC_t rc =
        STAFDynamicLibraryOpen(&fImplLib,
                               libraryInfo.toCurrentCodePage()->buffer(),
                               &errorBuffer);

    STAFString result = STAFString(errorBuffer, STAFString::kShallow);

    if (rc)
    {
        result = STAFString("Error initializing service, ")
            + serviceName + STAFString(", Result: ")+ result;

        connection->writeUInt(rc);
        connection->writeString(result);

        gExitSemaphore->post();
    }

    // Obtain service's interface addresses and levels

    rc = STAFDynamicLibraryGetAddress(fImplLib,
                                      "STAFServiceGetLevelBounds",
                                      reinterpret_cast<void **>(&fGetBounds),
                                      &errorBuffer);

    fLevelConstruct = getLevel(kServiceInfo, "ServiceInfo", 30, 30);
    rc = STAFDynamicLibraryGetAddress(fImplLib,
                                      "STAFServiceConstruct",
                                      reinterpret_cast<void **>(&fConstruct),
                                      &errorBuffer);

    fLevelInit = getLevel(kServiceInit, "ServiceInit", 30, 30);
    rc = STAFDynamicLibraryGetAddress(fImplLib,
                                      "STAFServiceInit",
                                      reinterpret_cast<void **>(&fInit),
                                      &errorBuffer);

    fLevelAcceptRequest = getLevel(kServiceAcceptRequest,
                                   "ServiceAcceptRequest", 30, 30);
    rc = STAFDynamicLibraryGetAddress(fImplLib,
                                      "STAFServiceAcceptRequest",
                                      reinterpret_cast<void **>(&fAcceptRequest),
                                      &errorBuffer);

    fLevelTerm = getLevel(kServiceTerm, "ServiceTerm", 0, 0);
    rc = STAFDynamicLibraryGetAddress(fImplLib,
                                      "STAFServiceTerm",
                                      reinterpret_cast<void **>(&fTerm),
                                      &errorBuffer);

    fLevelDestruct = getLevel(kServiceDestruct, "ServiceDestruct", 0, 0);
    rc = STAFDynamicLibraryGetAddress(fImplLib,
                                      "STAFServiceDestruct",
                                      reinterpret_cast<void **>(&fDestruct),
                                      &errorBuffer);

    // Construct service

    unsigned int constructRC = kSTAFUnknownError;
    STAFString_t constructErrorBuffer = 0;

    if (fLevelConstruct == 30)
    {
        STAFServiceInfoLevel30 info = { serviceName.getImpl(),
                                        execInfo.getImpl(),
                                        writeLocation.getImpl(),
                                        serviceType,
                                        numOptions,
                                        optionNames,
                                        optionValues };

        constructRC = fConstruct(&fServiceHandle, &info, 30,
                                 &constructErrorBuffer);
    }

    connection->writeUInt(constructRC);
    connection->writeString(STAFString(constructErrorBuffer));
    
    if (constructRC != kSTAFOk)
    {
        gExitSemaphore->post();
    }
}

void HandleServiceInit(STAFConnectionPtr &connection)
{
    STAFString serviceName   = connection->readString();
    STAFString parms = connection->readString();
    STAFString writeLocation = connection->readString();

    STAFString_t errorBuffer = 0;
    STAFRC_t rc = kSTAFUnknownError;

    if (fLevelInit == 30)
    {
        STAFServiceInitLevel30 initData = { parms.getImpl(),
                                            writeLocation.getImpl() };
        rc = fInit(fServiceHandle, &initData, 30, &errorBuffer);
    }

    STAFString result = STAFString(errorBuffer, STAFString::kShallow);

    if (rc) result = STAFString("Error initializing service, ")
                     + serviceName + STAFString(", Result: ")+ result;

    connection->writeUInt(rc);
    connection->writeString(result);

    if (rc != kSTAFOk)
    {
        gExitSemaphore->post();
    }
}

void HandleServiceTerm(STAFConnectionPtr &connection)
{
    STAFString serviceName   = connection->readString();

    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fTerm(fServiceHandle, 0, 0, &errorBuffer);

    STAFString result = STAFString(errorBuffer, STAFString::kShallow);

    if (rc) result = STAFString("Error initializing service, ")
                     + serviceName + STAFString(", Result: ")+ result;

    connection->writeUInt(rc);
    connection->writeString(result);
}

void HandleServiceDestruct(STAFConnectionPtr &connection)
{
    STAFString serviceName   = connection->readString();

    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fDestruct(&fServiceHandle, 0, 0, &errorBuffer);

    STAFString result = STAFString(errorBuffer, STAFString::kShallow);

    if (rc) result = STAFString("Error destructing service, ")
                     + serviceName + STAFString(", Result: ")+ result;

    connection->writeUInt(rc);
    connection->writeString(result);
}

void HandleServiceRequest(STAFConnectionPtr &connection)
{
    // Note: The request type has already been read
    unsigned int totalLength = connection->readUInt();
    STAFBuffer<char> buffer(new char[totalLength], STAFBuffer<char>::INIT,
                            STAFBuffer<char>::ARRAY);
    unsigned int *uintBuffer =
        reinterpret_cast<unsigned int *>((char *)buffer);

    // IMPORTANT:  Increase the numFields value if add a field to the
    //             ServiceRequest class for a new STAFServiceInterfaceLevel.
    //             Note:  This value is 2 less than the numFields value in
    //                    STAFExecProxyLib.cpp.

    unsigned int numFields = 14;  // # of fields in buffer

    // uintBuffer[0] = Service name length
    // uintBuffer[1] = Handle
    // uintBuffer[2] = Trust level
    // uintBuffer[3] = Machine length
    // uintBuffer[4] = Machine Nickname length
    // uintBuffer[5] = Handle name length
    // uintBuffer[6] = Request length
    // uintBuffer[7] = Diagnostics Enabled Flag
    // uinrBuffer[8] = Request Number
    // uintBuffer[9] = userLength;
    // uintBuffer[10] = endpointLength;
    // uintBuffer[11] = STAF Instance UUID length
    // uintBuffer[12] = Is Local Request Flag
    // uintBuffer[13] = Physical Interface ID length
    
    connection->read(buffer, totalLength);
    
    char *serviceNameBuffer = buffer + (numFields * sizeof(unsigned int));
    char *machineBuffer = serviceNameBuffer + uintBuffer[0];
    char *machineNicknameBuffer = machineBuffer +  uintBuffer[3];
    char *handleNameBuffer = machineNicknameBuffer + uintBuffer[4];
    char *requestBuffer = handleNameBuffer + uintBuffer[5];
    char *userBuffer = requestBuffer + uintBuffer[6];
    char *endpointBuffer = userBuffer + uintBuffer[9];
    char *stafInstanceUUIDBuffer = endpointBuffer + uintBuffer[10];
    char *physicalInterfaceIDBuffer = stafInstanceUUIDBuffer + uintBuffer[11];

    if (serviceNameBuffer == 0 || machineBuffer == 0 ||
        machineNicknameBuffer == 0 || handleNameBuffer == 0 ||
        requestBuffer == 0 || userBuffer == 0 || 
        endpointBuffer == 0 || stafInstanceUUIDBuffer == 0 ||
        physicalInterfaceIDBuffer == 0)
    {
        cout << "Memory allocation failure in "
             << "STAFService.HandleServiceRequest()" << endl;
        /* XXX: Shouldn't be a kSTAFJavaError - it's a Perl error */
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Memory allocation failure in "
                                "STAFService.HandleServiceRequest");
        return;
    }

    STAFString serviceName(serviceNameBuffer, uintBuffer[0],
                           STAFString::kUTF8);
    unsigned int handle = uintBuffer[1];
    unsigned int trustLevel = uintBuffer[2];
    STAFString machine(machineBuffer, uintBuffer[3], STAFString::kUTF8);
    STAFString machineNickname(machineNicknameBuffer, uintBuffer[4],
                               STAFString::kUTF8);
    STAFString handleName(handleNameBuffer, uintBuffer[5],
                          STAFString::kUTF8);
    STAFString request(requestBuffer, uintBuffer[6], STAFString::kUTF8);
    unsigned int diagEnabled = uintBuffer[7];
    unsigned int requestNumber = uintBuffer[8];
    STAFString user(userBuffer, uintBuffer[9], STAFString::kUTF8);
    STAFString endpoint(endpointBuffer, uintBuffer[10], STAFString::kUTF8);
    STAFString stafInstanceUUID(stafInstanceUUIDBuffer, uintBuffer[11],
                                STAFString::kUTF8);
    unsigned int isLocalRequest = uintBuffer[12];
    STAFString physicalInterfaceID(physicalInterfaceIDBuffer, uintBuffer[13],
                                   STAFString::kUTF8);

    STAFString_t resultBuffer = 0;
    STAFRC_t rc = kSTAFUnknownError;
    
    if (fLevelAcceptRequest == 30)
    {
        STAFServiceRequestLevel30 requestData = {
            stafInstanceUUID.getImpl(),
            machine.getImpl(),
            machineNickname.getImpl(),
            handleName.getImpl(),
            handle,
            trustLevel,
            isLocalRequest,
            diagEnabled,
            request.getImpl(),
            requestNumber,
            user.getImpl(),
            endpoint.getImpl(),
            physicalInterfaceID.getImpl()
        };

        rc = fAcceptRequest(fServiceHandle, &requestData, 30, &resultBuffer);
    }

    connection->writeUInt(rc);
    connection->writeString(STAFString(resultBuffer,
                                       STAFString::kShallow));
}

unsigned int getLevel(STAFServiceLevelID levelID,
                       const char *levelString,
                       unsigned int stafMin,
                       unsigned int stafMax)
{
    unsigned int serviceMin = 0;
    unsigned int serviceMax = 0;
    unsigned int rc = 0;

    // Get the service's supported levels

    if ((rc = fGetBounds(levelID, &serviceMin, &serviceMax) != 0))
    {
        STAFString message(STAFString("Error getting level bounds for ") +
                           levelString);
        cout << message << endl;
    }

    // Check for non-overlapping ranges

    if ((serviceMin > stafMax) || (stafMin > serviceMax))
    {
        STAFString message(levelString);
        message += STAFString(" level validation failure, RC = " + rc) +
                   ".  Service supports "        +
                   serviceMin + ":" + serviceMax +
                   ".  STAF supports " + stafMin +
                   ":" + stafMax + ".";
        cout << message << endl;
    }

    return STAF_MIN(serviceMax, stafMax);
}
