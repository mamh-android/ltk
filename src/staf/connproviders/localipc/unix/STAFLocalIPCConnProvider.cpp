/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <fcntl.h>
#include <sys/un.h>
#include "STAFConnectionProvider.h"
#include "STAFSocket.h"
#include "STAFUtil.h"
#include "STAFString.h"
#include "STAFEventSem.h"
#include "STAFTrace.h"
#include "STAFInternalUtil.h"
#include "STAFThreadManager.h"
#include <set>
#include <stdlib.h>

// XXX: Might want to look at adding someing like kSTAFConnectionError, instead
//      of using kSTAFBaseOSError in this code.  Same goes for Local IPC.

#define CATCH_STANDARD(functionString) \
catch (STAFException &e)\
{\
    if (errorBuffer)\
    {\
        *errorBuffer = getExceptionString(e, "STAFLocalIPCConnProvider.cpp: "\
                                          functionString)\
                       .adoptImpl();\
    }\
}\
catch (...)\
{\
    if (errorBuffer)\
    {\
        STAFString error("STAFLocalIPCConnProvider.cpp: Caught unknown "\
                         "exception in " functionString "()");\
        *errorBuffer = error.adoptImpl();\
    }\
}

#define CATCH_STANDARD_TRACE(functionString) \
catch (STAFException &e)\
{\
    STAFTrace::trace(kSTAFTraceError,\
                     getExceptionString(e, "STAFLocalIPCConnProvider.cpp: "\
                                        functionString));\
}\
catch (...)\
{\
    STAFTrace::trace(kSTAFTraceError,\
                     "STAFLocalIPCConnProvider.cpp: Caught unknown exception "\
                     "in " functionString "()");\
}

#ifdef STAF_OS_NAME_HPUX
extern "C"
{
    void _main();
}
#endif

// Prototypes

STAFRC_t STAFConnectionProviderConstruct(STAFConnectionProvider_t *provider,
                                         void *constructInfo,
                                         unsigned int constructInfoLevel,
                                         STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderStart(STAFConnectionProvider_t provider,
                                     void *startInfo,
                                     unsigned int startInfoLevel,
                                     STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderStop(STAFConnectionProvider_t provider,
                                    void *stopInfo,
                                    unsigned int stopInfoLevel,
                                    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderDestruct(STAFConnectionProvider_t *provider,
                                        void *destructInfo,
                                        unsigned int destructInfoLevel,
                                        STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderConnect(STAFConnectionProvider_t provider,
                                       STAFConnection_t *connection,
                                       void *connectInfo,
                                       unsigned int connectInfoLevel,
                                       STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetMyNetworkIDs(STAFConnectionProvider_t provider,
                                               STAFStringConst_t *logicalID,
                                               STAFStringConst_t *physicalD,
                                               STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetOptions(
    STAFConnectionProvider_t provider,
    STAFObject_t *options,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetProperty(
    STAFConnectionProvider_t provider,
    STAFConnectionProviderProperty_t property, STAFStringConst_t *value,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionRead(STAFConnection_t connection, void *buffer,
                            unsigned int readLength, STAFString_t *errorBuffer,
                            bool doTimeout);

STAFRC_t STAFConnectionReadUInt(STAFConnection_t connection, unsigned int *uint,
                                STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionReadSTAFString(STAFConnection_t connection,
                                      STAFString_t *stafString,
                                      STAFString_t *errorBuffer,
                                      bool doTimeout);

STAFRC_t STAFConnectionWrite(STAFConnection_t connection, void *buffer,
                             unsigned int writeLength,
                             STAFString_t *errorBuffer,
                             bool doTimeout);

STAFRC_t STAFConnectionWriteUInt(STAFConnection_t connection, unsigned int uint,
                                 STAFString_t *errorBuffer,
                                 bool doTimeout);

STAFRC_t STAFConnectionWriteSTAFString(STAFConnection_t connection,
                                       STAFStringConst_t stafString,
                                       STAFString_t *errorBuffer,
                                       bool doTimeout);

STAFRC_t STAFConnectionGetPeerNetworkIDs(STAFConnection_t connection,
                                         STAFStringConst_t *logicalID,
                                         STAFStringConst_t *physicalD,
                                         STAFString_t *errorBuffer);

STAFRC_t STAFConnectionDestruct(STAFConnection_t *connection,
                                STAFString_t *errorBuffer);


static STAFConnectionProviderFunctionTable gFuncTable =
{
    STAFConnectionProviderConstruct,
    STAFConnectionProviderStart,
    STAFConnectionProviderStop,
    STAFConnectionProviderDestruct,
    STAFConnectionProviderConnect,
    STAFConnectionProviderGetMyNetworkIDs,
    STAFConnectionProviderGetOptions,
    STAFConnectionProviderGetProperty,
    STAFConnectionRead,
    STAFConnectionReadUInt,
    STAFConnectionReadSTAFString,
    STAFConnectionWrite,
    STAFConnectionWriteUInt,
    STAFConnectionWriteSTAFString,
    STAFConnectionGetPeerNetworkIDs,
    STAFConnectionDestruct
};

struct STAFLocalConnectionProviderImpl : STAFConnectionProviderImpl
{
    STAFConnectionProviderMode_t mode;
    void *data;

    STAFString ipcName;

    // Fully-qualified socket path, e.g. "/tmp/STAFIPC_" + ipcName
    STAFString socketPath;

    STAFSocket_t serverSocket;
    
    STAFString logicalNetworkID;
    STAFString physicalNetworkID;

    STAFObjectPtr options;

    STAFString portProperty;
    STAFString isSecureProperty;

    STAFConnectionProviderNewConnectionFunc_t connFunc;

    STAFEventSemPtr syncSem;
    STAFConnectionProviderState_t state;
    STAFThreadManagerPtr threadManager;
};

struct STAFLocalConnectionImpl : STAFConnectionImpl
{
    STAFSocket_t clientSocket;

    // XXX: HACK HACK
    //      Due to an unknown bug on Linux, these strings need to be here
    //      to prevent corruption of the corresponding tcp/ip connection
    //      provider.  It appears that some form of copy
    //      assignment/construction operator from this structure replaces
    //      one for the tcp/ip version and the buffer below zeros out the
    //      strings in the corresponding spot in the tcp/ip provider.

    STAFString dummyLogicalNetworkID;
    STAFString dummyPhysicalNetworkID;

    // ???: Look at moving this buffer into the read/write functions.  It will
    //      make the read/write functions thread safe.  Just need to make sure
    //      it doesn't hurt performance.

    char buffer[4096];
};

struct LocalConnectionData
{
    STAFConnectionProviderNewConnectionFunc_t connFunc;
    STAFLocalConnectionProviderImpl *provider;
    STAFLocalConnectionImpl *connection;
};


unsigned int STAFLocalIPCConnectionThread(void *data)
{
    try
    {
        LocalConnectionData *connData =
            reinterpret_cast<LocalConnectionData *>(data);

        connData->connFunc(connData->provider, connData->connection,
                           &gFuncTable, connData->provider->data);

        delete connData;
    }
    CATCH_STANDARD_TRACE("STAFLocalIPCConnectionThread");

    return 0;
}


unsigned int STAFTCPRunThread(void *providerImpl)
{
    STAFLocalConnectionProviderImpl *provider =
        reinterpret_cast<STAFLocalConnectionProviderImpl *>(providerImpl);

    try
    {
        provider->syncSem->post();
    }
    CATCH_STANDARD_TRACE("STAFTCPRunThread: post");

    while (provider->state == kSTAFConnectionProviderActive)
    {
        STAFLocalConnectionImpl connImpl;
        struct sockaddr_in clientAddress = { 0 };
        STAFSocketLen_t clientAddressLength = sizeof(clientAddress);
        
        // Flag which indicates if need to close the client socket
        int closeClientSocket = 0;  // Don't need to close client socket

        // Need a try block within the while loop so that if an exception
        // occurs, this thread won't terminate so this interface can continue
        // accepting requests

        try
        {
            do
            {
                connImpl.clientSocket = accept(
                    provider->serverSocket,
                    reinterpret_cast<struct sockaddr *>(&clientAddress),
                    &clientAddressLength);
            } while (!STAFUtilIsValidSocket(connImpl.clientSocket) &&
                     (STAFSocketGetLastError() == SOCEINTR) &&
                     (provider->state != kSTAFConnectionProviderStopped));

            if (provider->state == kSTAFConnectionProviderStopped)
                break;

            if (!STAFUtilIsValidSocket(connImpl.clientSocket))
            {
                STAFTrace::trace(kSTAFTraceError,
                                 STAFString("Error accepting on server socket"
                                            ", socket RC: ") +
                                 STAFString(STAFSocketGetLastError()));
                continue;
            }

            closeClientSocket = 1;  // Need to close client socket

            // Set the socket to be non-inheritable

            unsigned int osRC = 0;
            STAFSocket_t newSocket;

            if (STAFUtilGetNonInheritableSocket(connImpl.clientSocket,
                                                &newSocket, &osRC))
            {
                // Close the client socket so the request won't hang
                STAFSocketClose(connImpl.clientSocket);

                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("Error getting non-inheritable socket, "
                               "STAFUtilGetNonInheritableSocket(), OS RC: ") +
                    STAFString(osRC));

                continue;
            }

            connImpl.clientSocket = newSocket;

            // Ok, let's perform the callback now

            LocalConnectionData connData;

            connData.connFunc   = provider->connFunc;
            connData.provider   = provider;
            connData.connection = new STAFLocalConnectionImpl(connImpl);

            provider->threadManager->dispatch(
                STAFLocalIPCConnectionThread,
                new LocalConnectionData(connData));
        }
        catch (STAFException &e)
        {
            if (closeClientSocket)
            {
                try
                {
                    // Close the client socket so the request won't hang
                    STAFSocketClose(connImpl.clientSocket);
                }
                catch (...)
                { /* Do nothing */ }
            }

            STAFTrace::trace(
                kSTAFTraceError, getExceptionString(
                    e, "STAFLocalIPCConnProvider.cpp: STAFLocalIPCRunThread"));
        }                                              
        catch (...)
        {
            if (closeClientSocket)
            {
                try
                {
                    // Close the client socket so the request won't hang
                    STAFSocketClose(connImpl.clientSocket);
                }
                catch (...)
                { /* Do nothing */ }
            }

            STAFTrace::trace(kSTAFTraceError,
                             "STAFLocalIPConnProvider.cpp: Caught unknown "
                             "exception in STAFLocalIPCRunThread()");
        }
    } // end while loop

    try
    {
        provider->syncSem->post();
    }
    CATCH_STANDARD_TRACE("STAFTCPRunThread: post end");

    return 0;
}


STAFRC_t STAFConnectionProviderConstruct(STAFConnectionProvider_t *provider,
                                         void *constructInfo,
                                         unsigned int constructInfoLevel,
                                         STAFString_t *errorBuffer)
{
    if (provider == 0) return kSTAFInvalidParm;
    if (constructInfoLevel != 1) return kSTAFInvalidAPILevel;

    STAFConnectionProviderConstructInfoLevel1 *cpInfo =
        reinterpret_cast<STAFConnectionProviderConstructInfoLevel1 *>(
            constructInfo);

    try
    {
        STAFLocalConnectionProviderImpl lipcData;

        lipcData.mode          = cpInfo->mode;
        lipcData.syncSem       = STAFEventSemPtr(
            new STAFEventSem, STAFEventSemPtr::INIT);
        lipcData.state         = kSTAFConnectionProviderStopped;
        lipcData.serverSocket  = 0;

        if (cpInfo->mode != kSTAFConnectionProviderOutbound)
        {
            lipcData.threadManager =
                STAFThreadManagerPtr(new STAFThreadManager,
                                     STAFThreadManagerPtr::INIT);
        }

        // If the STAF_INSTANCE_NAME environment variable exists, set the
        // ipcName to its value, otherwise set it to "STAF".
        
        lipcData.ipcName = "STAF";

        if (getenv("STAF_INSTANCE_NAME") != NULL)
        {
            lipcData.ipcName = STAFString(getenv("STAF_INSTANCE_NAME"));

            if (lipcData.ipcName.length() == 0)
                lipcData.ipcName = STAFString("STAF");
        }
        
        // If the IPCName option was specified for the connection provider,
        // append its value to the ipcName.

        for (unsigned int i = 0; i < cpInfo->numOptions; ++i)
        {
            STAFString thisOption = STAFString(
                cpInfo->optionNames[i]).upperCase();

            if (thisOption == "IPCNAME")
            {
                lipcData.ipcName = lipcData.ipcName + cpInfo->optionValues[i];
            }
            else
            {
                if (errorBuffer)
                {
                    *errorBuffer =
                        STAFString(cpInfo->optionNames[i]).adoptImpl();
                }
                return kSTAFInvalidValue;
            }
        }

        // If the STAF_TEMP_DIR environment variable exists, set tempDir
        // to its value, otherwise set it to "/tmp"

        STAFString tempDir("/tmp");

        if (getenv("STAF_TEMP_DIR") != NULL)
        {
            tempDir = STAFString(getenv("STAF_TEMP_DIR"));

            if (tempDir.length() == 0)
                tempDir = "/tmp";
        }

        lipcData.socketPath = tempDir + STAFString("/STAFIPC_") +
            lipcData.ipcName;

        // Add each option to a map.

        lipcData.options = STAFObject::createMap();
        lipcData.options->put("IPCName", lipcData.ipcName);

        // Setup property values

        lipcData.portProperty = STAFString();
        lipcData.isSecureProperty = STAFString("0");

        // Assign logical and physical identifiers
        lipcData.logicalNetworkID = STAFString("local");
        lipcData.physicalNetworkID = STAFString("local");

        STAFRC_t rc = STAFSocketInit(errorBuffer);

        if (rc != kSTAFOk) return rc;

        *provider = new STAFLocalConnectionProviderImpl(lipcData);

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderConstruct");

    return kSTAFUnknownError;
}


#define HANDLE_START_ERROR(string, function) \
{\
    STAFString theError = STAFString(string) + STAFString(", " function \
                                                          " osRC=") + \
                          STAFString(STAFSocketGetLastError()); \
\
    if (errorBuffer) *errorBuffer = theError.adoptImpl();\
    return kSTAFBaseOSError;\
}

STAFRC_t STAFConnectionProviderStart(STAFConnectionProvider_t baseProvider,
                                     void *startInfo,
                                     unsigned int startInfoLevel,
                                     STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (startInfoLevel != 1) return kSTAFInvalidAPILevel;

    STAFConnectionProviderStartInfoLevel1 *cpInfo =
        reinterpret_cast<STAFConnectionProviderStartInfoLevel1 *>(startInfo);

    try
    {
        if (cpInfo->newConnectionFunc == 0) return kSTAFInvalidValue;

        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        provider->connFunc = cpInfo->newConnectionFunc;
        provider->data = cpInfo->data;
        provider->serverSocket = socket(PF_UNIX, SOCK_STREAM, 0);

        if (!STAFUtilIsValidSocket(provider->serverSocket))
            HANDLE_START_ERROR("No socket available", "socket()");

        // Set the socket to be non-inheritable to ensure that the file
        // descriptor is closed when we fork() and exec().

        unsigned int osRC = 0;
        STAFSocket_t newSocket;

        if (STAFUtilGetNonInheritableSocket(provider->serverSocket,
                                            &newSocket, &osRC))
        {
            HANDLE_START_ERROR("Error getting non-inheritable server socket",
                               "STAFUtilGetNonInheritableSocket()");
        }

        provider->serverSocket = newSocket;

        struct sockaddr_un serverAddress = { 0 };

        serverAddress.sun_family = AF_UNIX;

        strcpy(serverAddress.sun_path,
               provider->socketPath.toCurrentCodePage()->buffer());

        #if defined(STAF_OS_NAME_AIX) || defined(STAF_OS_NAME_AIX64)
        // Assign length of null-terminated socket name on AIX to prevent
        // bind() failing with errno EINVAL (e.g. 22 or 3490) on IBM i
        // (when compiled on AIX 5.3).
        // This is equivalent to using:  int sunLen = SUN_LEN(&serverAddress);
        int sunLen = offsetof(struct sockaddr_un, sun_path) +
            strlen(provider->socketPath.toCurrentCodePage()->buffer());
        #else
        int sunLen = sizeof(serverAddress);
        #endif

        // We attempt to remove the socket path in case it was left around
        // from a previous invocation

        unlink(serverAddress.sun_path);

        int bindRC = bind(provider->serverSocket,
                          reinterpret_cast<struct sockaddr *>(&serverAddress),
                          sunLen);

        if (bindRC != 0)
        {
            int theRC = STAFSocketGetLastError();

            STAFString theError = STAFString(
                "Error binding server socket, bind() osRC=") +
                STAFString(theRC);

            if (theRC == EACCES)
            {
                theError += " - The socket file is protected, "
                    "and the current user has inadequate permission to "
                    "access it.";
            }
            else if (theRC == EADDRINUSE)
            {
                theError += " - The socket file is already in use.";
            }
            
            theError += "  This error occurs if STAF was not shut down "
                "properly using the SHUTDOWN service or if STAF is still "
                "in the process of shutting down.  To resolve, log on as "
                "the superuser and remove socket file " +
                STAFString(serverAddress.sun_path) + " and retry.";

            if (errorBuffer) *errorBuffer = theError.adoptImpl();

            return kSTAFBaseOSError;
        }
        
        int listenRC = listen(provider->serverSocket, SOMAXCONN);

        if (listenRC != 0)
            HANDLE_START_ERROR("Error listening on server socket", "listen()");

        // Ok, the provider is now ready

        provider->syncSem->reset();
        provider->state = kSTAFConnectionProviderActive;
        provider->threadManager->dispatch(STAFTCPRunThread, provider);

        // XXX: Might want to time out on this one.

        provider->syncSem->wait();

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderStart");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderStop(STAFConnectionProvider_t baseProvider,
                                    void *stopInfo,
                                    unsigned int stopInfoLevel,
                                    STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (stopInfoLevel != 0) return kSTAFInvalidAPILevel;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        provider->state = kSTAFConnectionProviderStopped;

        provider->syncSem->reset();

        // Wake up the run thread and release resources acquired when
        // starting provider
        
        STAFSocketClose(provider->serverSocket);
        
        if (provider->syncSem->wait(10000) != 0)
        {
            STAFTrace::trace(
                kSTAFTraceWarning,
                STAFString("STAFLocalIPCConnectionProviderStop - Timed out "
                           "waiting for run thread to wake up"));
        }

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderStop");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderDestruct(STAFConnectionProvider_t *baseProvider,
                                        void *destructInfo,
                                        unsigned int destructInfoLevel,
                                        STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (*baseProvider == 0) return kSTAFInvalidObject;
    if (destructInfoLevel != 0) return kSTAFInvalidAPILevel;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(*baseProvider);
        
        // Delete the /tmp/STAFIPC_<STAFInstanceName> temporary socket file

        unlink((provider->socketPath + STAFString(kUTF8_NULL)).
               toCurrentCodePage()->buffer());

        // Close the socket if needed and delete the connection provider

        if (provider->state != kSTAFConnectionProviderStopped)
        {
            provider->state = kSTAFConnectionProviderStopped;

            // XXX: Should we check the RC?
            STAFSocketClose(provider->serverSocket);
        }

        delete provider;

        provider = 0;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderDestruct");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderConnect(STAFConnectionProvider_t baseProvider,
                                       STAFConnection_t *connection,
                                       void *connectInfo,
                                       unsigned int connectInfoLevel,
                                       STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (connectInfoLevel != 1) return kSTAFInvalidAPILevel;
    if (connection == 0) return kSTAFInvalidParm;

    STAFConnectionProviderConnectInfoLevel1 *cpInfo =
        reinterpret_cast<STAFConnectionProviderConnectInfoLevel1 *>(connectInfo);

    STAFLocalConnectionImpl connImpl;

    // Flag which indicates if need to close the client socket
    int closeClientSocket = 0;  // Don't need to close client socket

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        STAFString host = cpInfo->endpoint;
        struct sockaddr_un serverAddress = { 0 };

        serverAddress.sun_family = AF_UNIX;
        strcpy(serverAddress.sun_path,
               provider->socketPath.toCurrentCodePage()->buffer());

        #if defined(STAF_OS_NAME_AIX) || defined(STAF_OS_NAME_AIX64)
        // Assign length of null-terminated socket name on AIX to prevent
        // connect() failing on IBM i (when compiled on AIX 5.3).
        // This is equivalent to using:  int sunLen = SUN_LEN(&serverAddress);
        int sunLen = offsetof(struct sockaddr_un, sun_path) +
            strlen(provider->socketPath.toCurrentCodePage()->buffer());
        #else
        int sunLen = sizeof(serverAddress);
        #endif

        connImpl.clientSocket = socket(PF_UNIX, SOCK_STREAM, 0);

        if (!STAFUtilIsValidSocket(connImpl.clientSocket))
        {
            STAFString error = STAFString(
                "Error creating socket for path '") + provider->socketPath +
                "', socket(), osRC=" + STAFSocketGetLastError();
            if (errorBuffer) *errorBuffer = error.adoptImpl();
            return kSTAFCommunicationError;
        }

        closeClientSocket = 1;  // Need to close client socket if error

        // Set the socket to be non-inheritable

        unsigned int osRC = 0;
        STAFSocket_t newSocket;

        if (STAFUtilGetNonInheritableSocket(connImpl.clientSocket,
                                            &newSocket, &osRC))
        {
            STAFString error = STAFString(
                "Error getting non-inheritable socket, "
                "STAFUtilGetNonInheritableSocket(), osRC=") +
                STAFString(osRC);

            if (errorBuffer)
            {
                error += STAFString(*errorBuffer, STAFString::kShallow);
                *errorBuffer = error.adoptImpl();
            }

            STAFSocketClose(connImpl.clientSocket);

            return kSTAFCommunicationError;
        }

        connImpl.clientSocket = newSocket;
        
        int modeRC = STAFSocketSetBlockingMode(connImpl.clientSocket,
                                               kSTAFSocketNonBlocking,
                                               errorBuffer);
        if (modeRC != kSTAFOk)
        {
            STAFString error = STAFString("Error setting socket to non-blocking "
                                          "mode:");
            if (errorBuffer)
            {
                error += STAFString(*errorBuffer, STAFString::kShallow);
                *errorBuffer = error.adoptImpl();
            }

            STAFSocketClose(connImpl.clientSocket);

            return kSTAFCommunicationError;
        }

        int connectRC = connect(
            connImpl.clientSocket,
            reinterpret_cast<struct sockaddr *>(&serverAddress),
            sunLen);

        if ((connectRC < 0) &&
            (STAFSocketGetLastError() == ECONNREFUSED))
        {
            // On Solaris Opteron the connect request intermittently (usually
            // under heavy load) will return ECONNREFUSED.  In this case we
            // will retry the connect request up to 5 times (sleeping 100ms
            // between every request).

            int maxRetry = 5;

            for (int i = 0; i < maxRetry; i++)
            {
                if ((connectRC < 0) &&
                    (STAFSocketGetLastError() == ECONNREFUSED))
                {
                    STAFSocketClose(connImpl.clientSocket);

                    connImpl.clientSocket = socket(PF_UNIX, SOCK_STREAM, 0);

                    modeRC = STAFSocketSetBlockingMode(connImpl.clientSocket,
                                                       kSTAFSocketNonBlocking,
                                                       errorBuffer);

                    STAFThreadManager::sleepCurrentThread(100);

                    connectRC = connect(
                        connImpl.clientSocket,
                        reinterpret_cast<struct sockaddr *>(&serverAddress),
                        sunLen);
                }
                else
                {
                    break;
                }
            }
        }

        if ((connectRC < 0) &&
            (STAFSocketGetLastError() != SOCEINPROGRESS) &&
            (STAFSocketGetLastError() != SOCEWOULDBLOCK) &&
            (STAFSocketGetLastError() != 0))
        {
            STAFString error = STAFString(
                "Error connecting to endpoint '") + host +
                "', connect(), osRC=" +
                STAFSocketGetLastError();

            if (STAFSocketGetLastError() == ENOENT)
            {
                // ENOENT - Error: Socket file not found

                STAFString tempDir("/tmp");

                if (getenv("STAF_TEMP_DIR") != NULL)
                {
                    tempDir = STAFString(getenv("STAF_TEMP_DIR"));

                    if (tempDir.length() == 0)
                        tempDir = "/tmp";
                }

                error += STAFString(
                    ". There is no STAFProc instance running with "
                    "STAF_INSTANCE_NAME=") + provider->ipcName +
                    " and STAF_TEMP_DIR=" + tempDir + " on the local machine."
                    " Verify that STAFProc is running and set these "
                    "environment variables to the values that STAFProc is "
                    "using. Or, if STAFProc is running with the same values, "
                    "socket file '" + provider->socketPath +
                    "' may have been deleted.";
            }

            if (errorBuffer) *errorBuffer = error.adoptImpl();
            STAFSocketClose(connImpl.clientSocket);
            return kSTAFCommunicationError;
        }

        // XXX: this timeout needs to be passed somehow

        unsigned int timeout = 5000;

        if (connectRC < 0)
        {
            fd_set writeSocks;
            FD_ZERO(&writeSocks);
            FD_SET(connImpl.clientSocket, &writeSocks);
            timeval theTimeout = { timeout / 1000, (timeout % 1000) * 1000 };
            int selectRC = select(connImpl.clientSocket + 1, 0,
                                  &writeSocks, 0,
                                  (timeout == 0) ? 0 : &theTimeout);
            if (selectRC < 0)
            {
                STAFString error = STAFString("Error connecting to endpoint: "
                                              "select(), osRC=") +
                                   STAFSocketGetLastError();

                if (errorBuffer) *errorBuffer = error.adoptImpl();
                STAFSocketClose(connImpl.clientSocket);
                return kSTAFCommunicationError;
            }
            else if (selectRC == 0)
            {
                STAFString error = STAFString("Timed out connecting to "
                                              "endpoint: select() timeout");
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                STAFSocketClose(connImpl.clientSocket);
                return kSTAFCommunicationError;
            }

            // On AIX, select() returns successfully if the target system exists.
            // This is true, even if the connection to the particular port is
            // refused.  So, we put in a zero-byte read to make sure that the
            // socket is actually connected successfully.
            // On Windows, we need to check if errno is SOCEWOULDBLOCK

            int recvRC = 0;

            do
            {
                recvRC = recv(connImpl.clientSocket, 0, 0, STAF_MSG_NOSIGNAL);
            }
            while ((recvRC < 0) && (STAFSocketGetLastError() == SOCEINTR));

            if ((recvRC < 0) && (STAFSocketGetLastError() != SOCEWOULDBLOCK))
            {
                STAFString error = STAFString(
                    "Error performing test read on connected endpoint: "
                    "recv(), osRC=") + STAFSocketGetLastError();
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                STAFSocketClose(connImpl.clientSocket);
                return kSTAFCommunicationError;
            }
        }

        modeRC = STAFSocketSetBlockingMode(connImpl.clientSocket,
                                           kSTAFSocketBlocking, errorBuffer);
        if (modeRC != 0)
        {
            STAFString error = STAFString("Error setting socket to blocking "
                                          "mode:");
            if (errorBuffer)
            {
                error += STAFString(*errorBuffer, STAFString::kShallow);
                *errorBuffer = error.adoptImpl();
            }

            STAFSocketClose(connImpl.clientSocket);

            return kSTAFCommunicationError;
        }

        *connection = new STAFLocalConnectionImpl(connImpl);

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        if (errorBuffer)
        {
            *errorBuffer = getExceptionString(
                e, "unix/STAFLocalIPCConnProvider.cpp").adoptImpl();
        }
    }
    catch (...)
    {
        if (errorBuffer)
        {
            STAFString error(
                "unix/STAFLocalIPCConnProvider.cpp: Caught unknown exception");
            *errorBuffer = error.adoptImpl();
        }
    }

    if (closeClientSocket)
    {
        try
        {
            // Need to close the client socket so the request won't hang
            STAFSocketClose(connImpl.clientSocket);
        }
        catch (...)
        {
            // No nothing
        }
    }

    return kSTAFCommunicationError;
}


STAFRC_t STAFConnectionProviderGetMyNetworkIDs(
    STAFConnectionProvider_t baseProvider,
    STAFStringConst_t *logicalID,
    STAFStringConst_t *physicalID,
    STAFString_t *errorBuffer)
{
    if (baseProvider   == 0) return kSTAFInvalidObject;
    if (logicalID  == 0) return kSTAFInvalidParm;
    if (physicalID == 0) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        *logicalID = (STAFStringConst_t)provider->logicalNetworkID.getImpl();
        *physicalID = (STAFStringConst_t)provider->physicalNetworkID.getImpl();

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderGetMyNetworkIDs");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderGetOptions(
    STAFConnectionProvider_t baseProvider,
    STAFObject_t *options,
    STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (options  == 0) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        STAFObjectConstructReference(options, provider->options->getImpl());

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderGetOptions");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderGetProperty(
    STAFConnectionProvider_t baseProvider,
    STAFConnectionProviderProperty_t property, STAFStringConst_t *value,
    STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (value    == 0) return kSTAFInvalidParm;
    
    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        if (property == kSTAFConnectionProviderPortProperty)
            *value = (STAFStringConst_t)provider->portProperty.getImpl();
        else if (property == kSTAFConnectionProviderIsSecureProperty)
            *value = (STAFStringConst_t)provider->isSecureProperty.getImpl();
        else
            return kSTAFInvalidValue;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderGetProperty");

    return kSTAFUnknownError;
}

int STAFRead(int socket, char *buffer, int len, bool doTimeout)
{
    if (doTimeout)
    {
        fd_set fds;
        int n;

        // Set up the file descriptor set
        FD_ZERO(&fds);
        FD_SET(socket, &fds);
        struct timeval tv = { 120, 0 };

        // Wait until timeout or data received
        n = select(socket + 1, &fds, NULL, NULL, &tv);

        if (n == 0)
            return -2; // timeout
        else if (n < 0)
            return n;  // error (-1)
    }

    return recv(socket, buffer, len, STAF_MSG_NOSIGNAL);
}


STAFRC_t STAFConnectionRead(STAFConnection_t baseConnection, void *buffer,
                            unsigned int readLength,
                            STAFString_t *errorBuffer, bool doTimeout)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if ((buffer == 0) && (readLength != 0)) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionImpl *connection =
            static_cast<STAFLocalConnectionImpl *>(baseConnection);

        int rc = 0;

        for(unsigned current = 0; current < readLength; current += rc)
        {
            int recvSize = (int)STAF_MIN((size_t)(readLength - current), 
                                         sizeof(connection->buffer));

            do
            {
                rc = STAFRead(connection->clientSocket,
                              connection->buffer,
                              recvSize,
                              doTimeout);
            }
            while ((rc < 0) && (STAFSocketGetLastError() == SOCEINTR));

            if (rc < 0)
            {
                STAFString error;

                if (rc == -2) // Timeout
                {
                    error = STAFString("select() timeout: recv() osRC=") +
                        STAFSocketGetLastError();
                }
                else
                {
                    error = STAFString(
                        "Error reading from socket: recv() osRC=") +
                        STAFSocketGetLastError();
                }

                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFCommunicationError;
            }
            else if (rc == 0)
            {
                STAFString error = STAFString("Error reading from socket: "
                                              "other side closed socket");
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFCommunicationError;
            }
            else
            {
                memcpy((char *)buffer + current, connection->buffer, rc);
            }
        }

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionRead");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionReadUInt(STAFConnection_t connection,
                                unsigned int *uint, STAFString_t *errorBuffer,
                                bool doTimeout)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (uint == 0) return kSTAFInvalidParm;

    STAFRC_t rc = STAFConnectionRead(connection, uint, sizeof(unsigned int),
                                     errorBuffer, doTimeout);

    if (rc == kSTAFOk) *uint = STAFUtilConvertLEUIntToNative(*uint);

    return rc;
}


STAFRC_t STAFConnectionReadSTAFString(STAFConnection_t connection,
                                      STAFString_t *stafString,
                                      STAFString_t *errorBuffer,
                                      bool doTimeout)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (stafString == 0) return kSTAFInvalidParm;

    try
    {
        // First, get the size of the string

        unsigned int size = 0;
        STAFRC_t rc = STAFConnectionReadUInt(connection, &size, errorBuffer,
                                             doTimeout);

        if (rc != kSTAFOk) return rc;

        // Next read in the actual UTF-8 data

        char *inputData = new char[size];

        rc = STAFConnectionRead(connection, (void *)inputData, size,
                                errorBuffer, doTimeout);

        if (rc != kSTAFOk)
        {
            delete [] inputData;
            return rc;
        }

        // Now, create the actual STAFString

        unsigned int osRC = 0;

        rc = STAFStringConstruct(stafString, inputData, size, &osRC);

        if ((rc == kSTAFBaseOSError) && (errorBuffer != 0))
            *errorBuffer = STAFString(osRC).adoptImpl();

        delete [] inputData;

        return rc;
    }
    CATCH_STANDARD("STAFConnectionReadSTAFString");

    return kSTAFUnknownError;
}


int STAFWrite(int socket, char *buffer, int len, bool doTimeout)
{
    if (doTimeout)
    {
        fd_set fds;
        int n;

        // Set up the file descriptor set
        FD_ZERO(&fds);
        FD_SET(socket, &fds);
        struct timeval tv = { 120, 0 };

        // Wait until timeout or data received
        n = select(socket + 1, NULL, &fds, NULL, &tv);

        if (n == 0)
            return -2; // timeout
        else if (n < 0)
            return n;  // error (-1)
    }

    return send(socket, buffer, len, 0);
}


STAFRC_t STAFConnectionWrite(STAFConnection_t baseConnection, void *buffer,
                             unsigned int writeLength,
                             STAFString_t *errorBuffer,
                             bool doTimeout)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if ((buffer == 0) && (writeLength != 0)) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionImpl *connection =
            static_cast<STAFLocalConnectionImpl *>(baseConnection);

        int rc = 0;

        for(unsigned int current = 0; current < writeLength; current += rc)
        {
            int sendSize = (int)STAF_MIN((size_t)(writeLength - current), 
                                         sizeof(connection->buffer));
            memcpy(connection->buffer, (char *)buffer + current, sendSize);

            do
            {
                rc = STAFWrite(connection->clientSocket,
                               connection->buffer,
                               sendSize,
                               doTimeout);
            }
            while ((rc < 0) && (STAFSocketGetLastError() == SOCEINTR));

            if (rc < 0)
            {
                STAFString error;

                if (rc == -2) // Timeout
                {
                    error = STAFString(
                        "select() timeout: send() osRC=") +
                        STAFSocketGetLastError();
                }
                else
                {
                    error = STAFString(
                        "Error writing to socket: send() osRC=") +
                        STAFSocketGetLastError();
                }

                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFCommunicationError;
            }
        }

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionWrite");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionWriteUInt(STAFConnection_t connection,
                                 unsigned int uint, STAFString_t *errorBuffer,
                                 bool doTimeout)
{
    unsigned int leUInt = STAFUtilConvertNativeUIntToLE(uint);

    return STAFConnectionWrite(connection, &leUInt, sizeof(unsigned int),
                               errorBuffer, doTimeout);
}


STAFRC_t STAFConnectionWriteSTAFString(STAFConnection_t connection,
                                       STAFStringConst_t stafString,
                                       STAFString_t *errorBuffer,
                                       bool doTimeout)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (stafString == 0) return kSTAFInvalidObject;

    try
    {
        unsigned int osRC = 0;
        unsigned int length = 0;
        const char *buffer = 0;
        STAFRC_t rc = STAFStringGetBuffer(stafString, &buffer, &length, &osRC);

        if ((rc == kSTAFBaseOSError) && (errorBuffer != 0))
        {
            *errorBuffer = STAFString(osRC).adoptImpl();
            return rc;
        }

        rc = STAFConnectionWriteUInt(connection, length, errorBuffer, doTimeout);

        if (rc == kSTAFOk)
        {
            rc = STAFConnectionWrite(connection, const_cast<char *>(buffer),
                                     length, errorBuffer, doTimeout);
        }

        return rc;
    }
    CATCH_STANDARD("STAFConnectionWriteSTAFString");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionGetPeerNetworkIDs(STAFConnection_t connection,
                                         STAFStringConst_t *logicalID,
                                         STAFStringConst_t *physicalID,
                                         STAFString_t *errorBuffer)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (logicalID  == 0) return kSTAFInvalidParm;
    if (physicalID == 0) return kSTAFInvalidParm;

    try
    {
        static STAFString sLocalString("local");

        *logicalID = sLocalString.getImpl();
        *physicalID = sLocalString.getImpl();

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionGetPeerNetworkIDs");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionDestruct(STAFConnection_t *baseConnection,
                                STAFString_t *errorBuffer)
{
    if (baseConnection == 0) return kSTAFInvalidParm;
    if (*baseConnection == 0) return kSTAFInvalidObject;

    try
    {
        STAFLocalConnectionImpl *connection =
            static_cast<STAFLocalConnectionImpl *>(*baseConnection);

        STAFSocketClose(connection->clientSocket);

        delete connection;

        connection = 0;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionDestruct");

    return kSTAFUnknownError;
}
