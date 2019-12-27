/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFConnectionProvider.h"
#include "STAFSocket.h"
#include "STAFUtil.h"
#include "STAFString.h"
#include "STAFEventSem.h"
#include "STAFTrace.h"
#include "STAFInternalUtil.h"
#include "STAFThreadManager.h"
#include "STAFFileSystem.h"
#include <set>
#include <vector>
#include <stdlib.h>
#include <string.h>

#ifdef STAF_USE_SSL
#include <openssl/crypto.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rc4.h>
#include <openssl/rand.h>
#endif

#ifdef STAF_OS_TYPE_WIN32
#include <conio.h>
#else
#include <termios.h>
#include <cstdio>
#endif

#ifdef STAF_USE_IPV6
#if defined(STAF_OS_TYPE_WIN32) && !defined(EADDRINUSE)
#define EADDRINUSE WSAEADDRINUSE
#endif
#if defined(IPV6_V6ONLY)
int openIPv6OnlySocket = IPV6_V6ONLY;
#else
int openIPv6OnlySocket = 0;
#endif
#endif

// XXX: Might want to look at adding someing like kSTAFConnectionError, instead
//      of using kSTAFBaseOSError in this code.  Same goes for Local IPC.

#define CATCH_STANDARD(functionString) \
catch (STAFException &e)\
{\
    if (errorBuffer)\
    {\
        *errorBuffer = getExceptionString(e, "STAFTCPConnProvider.cpp: "\
                                          functionString)\
                       .adoptImpl();\
    }\
}\
catch (...)\
{\
    if (errorBuffer)\
    {\
        STAFString error("STAFTCPConnProvider.cpp: Caught unknown "\
                         "exception in " functionString "()");\
        *errorBuffer = error.adoptImpl();\
    }\
}

#define CATCH_STANDARD_TRACE(functionString) \
catch (STAFException &e)\
{\
    STAFTrace::trace(kSTAFTraceError,\
                     getExceptionString(e, "STAFTCPConnProvider.cpp: "\
                                        functionString));\
}\
catch (...)\
{\
    STAFTrace::trace(kSTAFTraceError,\
                     "STAFTCPConnProvider.cpp: Caught unknown exception "\
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

STAFRC_t STAFConnectionProviderGetMyNetworkIDs(
    STAFConnectionProvider_t provider,
    STAFStringConst_t *logicalID,STAFStringConst_t *physicalD,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetOptions(
    STAFConnectionProvider_t provider, STAFObject_t options,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetProperty(
    STAFConnectionProvider_t provider,
    STAFConnectionProviderProperty_t property, STAFStringConst_t *value,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionRead(
    STAFConnection_t connection, void *buffer,
    unsigned int readLength, STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionReadUInt(
    STAFConnection_t connection, unsigned int *uint,
    STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionReadSTAFString(
    STAFConnection_t connection, STAFString_t *stafString,
    STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionWrite(
    STAFConnection_t connection, void *buffer, unsigned int writeLength,
    STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionWriteUInt(
    STAFConnection_t connection, unsigned int uint,
    STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionWriteSTAFString(
    STAFConnection_t connection, STAFStringConst_t stafString,
    STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionGetPeerNetworkIDs(
    STAFConnection_t connection,
    STAFStringConst_t *logicalID, STAFStringConst_t *physicalD,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionDestruct(
    STAFConnection_t *connection, STAFString_t *errorBuffer);


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

struct STAFTCPConnectionProviderImpl : STAFConnectionProviderImpl
{
    STAFConnectionProviderMode_t mode;
    void *data;

    unsigned short port;
    unsigned short connectTimeout;
    STAFSocket_t serverSocket;
    STAFSocket_t serverSocketIPv6;
    
    STAFString logicalNetworkID;
    STAFString physicalNetworkID;

    STAFObjectPtr options;
    
    STAFString portProperty;
    STAFString isSecureProperty;

    STAFConnectionProviderNewConnectionFunc_t connFunc;

    STAFEventSemPtr syncSem;
    STAFConnectionProviderState_t state;
    STAFThreadManagerPtr threadManager;
    
    int family;

    STAFString secure;

#ifdef STAF_USE_SSL
    SSL_CTX *server_ctx;
    SSL_CTX *client_ctx;
    
    STAFString serverCertificate;
    STAFString serverKey;
    STAFString CACertificate;
#endif
};

struct STAFTCPConnectionImpl : STAFConnectionImpl
{
    STAFSocket_t clientSocket;

    STAFString logicalNetworkID;
    STAFString physicalNetworkID;

    unsigned int readWriteTimeout;

    // ???: Look at moving this buffer into the read/write functions.  It will
    //      make the read/write functions thread safe.  Just need to make sure
    //      it doesn't hurt performance.

    char buffer[4096];

#ifdef STAF_USE_SSL
    SSL *ssl;
    STAFString secure;
#endif
};

struct TCPConnectionData
{ 
    STAFConnectionProviderNewConnectionFunc_t connFunc;
    STAFTCPConnectionProviderImpl *provider;
    STAFTCPConnectionImpl *connection;
};

#ifdef STAF_USE_IPV6
class STAFNetworkAddress
{
public:

    STAFNetworkAddress()
    {
        buffer = NULL;
        current = NULL;
    };

    struct addrinfo **getBuffer()
    {
        return &buffer;
    };
    
    struct addrinfo *getCurrent()
    { 
        return current;
    };
    
    int next()
    {
        if (current == NULL)
        {
            current = buffer;
            return 1;
        }
        else
        {
            current = current->ai_next;

        
            if (current == NULL)
                return 0;
            else
                return 1;
        }
    };
         
    ~STAFNetworkAddress()
    {
        if (buffer != NULL)
            freeaddrinfo(buffer);
    };

private:

    struct addrinfo *current, *buffer;

};

#endif  //STAF_USE_IPV6


static unsigned short sDefaultSecurePort = 6550;
static unsigned short sDefaultNonSecurePort = 6500;
static unsigned short sDefaultConnectTimeout = 5000;

// These are the valid option names that can be specified (and these names
// are the names of the fields stored in the option map)
static STAFString sPort = STAFString("Port");
static STAFString sProtocol = STAFString("Protocol");
static STAFString sConnectTimeout = STAFString("ConnectTimeout");
static STAFString sServerCertificate = STAFString("SSL/ServerCertificate");
static STAFString sServerKey = STAFString("SSL/ServerKey");
static STAFString sCACertificate = STAFString("SSL/CACertificate");
static STAFString sSecure = STAFString("Secure");

// These are the valid values that can be specified for the PROTOCOL option
static STAFString sIPv4 = STAFString("IPv4");
static STAFString sIPv6 = STAFString("IPv6");
static STAFString sIPv4_IPv6 = STAFString("IPv4_IPv6");

// These are the valid values that can be specified for the SECURE option
static STAFString sYes = STAFString("Yes");
static STAFString sNo = STAFString("No");

#ifdef STAF_USE_IPV6
static void STAFIPv6TCPUpdateConnectionNetworkIDsFromInAddr(
    STAFConnection_t baseConn, sockaddr *addr, int addrlen)
{
    STAFTCPConnectionImpl *conn =
        static_cast<STAFTCPConnectionImpl *>(baseConn);

    STAFString_t ipAddress = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t ipAddrRC = 0;
    
    ipAddrRC = STAFIPv6SocketGetPrintableAddressFromInAddr(
        addr, addrlen, &ipAddress, &errorBuffer);
    
    if (ipAddrRC != kSTAFOk)
    {
        STAFString error(
            "Error getting printable IP address, "
            "STAFIPv6SocketGetPrintableAddressFromInAddr(), RC: " +
            STAFString(ipAddrRC) + ", Info: " +
            STAFString(errorBuffer, STAFString::kShallow));
        STAFTrace::trace(kSTAFTraceError, error);

        errorBuffer = 0;
        conn->physicalNetworkID = "0.0.0.0";
    }
    else
    {
        conn->physicalNetworkID = STAFString(ipAddress, STAFString::kShallow);
    }

    STAFString_t hostname = 0;
    STAFRC_t hostRC = 0;
    
    hostRC = STAFIPv6SocketGetNameByInAddr(
        addr, addrlen, &hostname, &errorBuffer);

    if (hostRC != kSTAFOk)
    {
        STAFString error(
            "Error getting hostname (for IP address " +
            conn->physicalNetworkID +
            "), STAFIPv6SocketGetNameByInAddr(), RC: " +
            STAFString(ipAddrRC) + ", Info: " +
            STAFString(errorBuffer, STAFString::kShallow));

        STAFTrace::trace(kSTAFTraceWarning, error);
        conn->logicalNetworkID = conn->physicalNetworkID;
    }
    else
    {
        conn->logicalNetworkID = STAFString(hostname, STAFString::kShallow);
    }
}
#else

static void STAFTCPUpdateConnectionNetworkIDsFromInAddr(
    STAFConnection_t baseConn, in_addr *addr)
{
    STAFTCPConnectionImpl *conn =
        static_cast<STAFTCPConnectionImpl *>(baseConn);

    STAFString_t ipAddress = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t ipAddrRC = STAFSocketGetPrintableAddressFromInAddr(
        addr, &ipAddress, &errorBuffer);

    if (ipAddrRC != kSTAFOk)
    {
        STAFString error(
            "Error getting printable IP address, "
            "STAFSocketGetPrintableAddressFromInAddr(), RC: " +
            STAFString(ipAddrRC) + ", Info: " +
            STAFString(errorBuffer, STAFString::kShallow));
        STAFTrace::trace(kSTAFTraceError, error);

        errorBuffer = 0;
        conn->physicalNetworkID = "0.0.0.0";
    }
    else
    {
        conn->physicalNetworkID = STAFString(ipAddress, STAFString::kShallow);
    }


    STAFString_t hostname = 0;
    STAFRC_t hostRC = STAFSocketGetNameByInAddr(addr, &hostname, &errorBuffer);

    if (hostRC != kSTAFOk)
    {
        STAFString error(
            "Error getting hostname (for IP address " +
            conn->physicalNetworkID + "), STAFSocketGetNameByInAddr(), RC: " +
            STAFString(ipAddrRC) + ", Info: " +
            STAFString(errorBuffer, STAFString::kShallow));

        STAFTrace::trace(kSTAFTraceWarning, error);
        conn->logicalNetworkID = conn->physicalNetworkID;
    }
    else
    {
        conn->logicalNetworkID = STAFString(hostname, STAFString::kShallow);
    }
}
#endif


static unsigned int STAFTCPConnectionThread(void *data)
{
    try
    {
        TCPConnectionData *connData =
            reinterpret_cast<TCPConnectionData *>(data);

        connData->connFunc(connData->provider, connData->connection,
                           &gFuncTable, connData->provider->data);

        delete connData;
    }
    CATCH_STANDARD_TRACE("STAFTCPConnectionThread");

    return 0;
}


#ifdef STAF_USE_IPV6
static unsigned int STAFTCPRunThreadIPv6(void *providerImpl)
{
    STAFTCPConnectionProviderImpl *provider =
        reinterpret_cast<STAFTCPConnectionProviderImpl *>(providerImpl);

    try
    {
        int maxSock, numSock = 0;

        int socks[2] = { 0 };

        if (provider->serverSocket > provider->serverSocketIPv6)
            maxSock = provider->serverSocket;
        else
            maxSock = provider->serverSocketIPv6;
        
        fd_set readSocks, readSocks0;

        FD_ZERO(&readSocks0);
            
        if (STAFUtilIsValidSocket(provider->serverSocket))
        {
            // Set the socket to be non-inheritable

            unsigned int osRC = 0;
            STAFSocket_t newSocket;

            if (STAFUtilGetNonInheritableSocket(provider->serverSocket,
                                                &newSocket, &osRC))
            {
                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("Error getting non-inheritable server socket:"
                               " IPv4, STAFUtilGetNonInheritableSocket(), "
                               "OS RC: ") + STAFString(osRC));
            }
            else
            {
                provider->serverSocket = newSocket;
            }

            FD_SET(provider->serverSocket, &readSocks0);
            
            socks[numSock] = provider->serverSocket;
            numSock++;
        }
            
        if (STAFUtilIsValidSocket(provider->serverSocketIPv6))
        {
            // Set the socket to be non-inheritable

            unsigned int osRC = 0;
            STAFSocket_t newSocket;

            if (STAFUtilGetNonInheritableSocket(provider->serverSocketIPv6,
                                                &newSocket, &osRC))
            {
                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("Error getting non-inheritable server socket:"
                               " IPv6, STAFUtilGetNonInheritableSocket(), "
                               "OS RC: ") + STAFString(osRC));
            }
            else
            {
                provider->serverSocketIPv6 = newSocket;
            }

            FD_SET(provider->serverSocketIPv6, &readSocks0);
            
            socks[numSock] = provider->serverSocketIPv6;
            numSock++;
        }

        provider->syncSem->post();

        while (provider->state == kSTAFConnectionProviderActive)
        {
            readSocks = readSocks0;

            if (select(maxSock + 1, &readSocks, NULL, NULL, NULL) < 0) 
            {
                if (provider->state == kSTAFConnectionProviderStopped)
                    break;

                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("Error selecting on server sockets, "
                               "socket RC: ") +
                    STAFString(STAFSocketGetLastError()));

                continue;
            }

            // Flag which indicates if need to close the client socket
            int closeClientSocket = 0;  // Don't need to close client socket

            for (int i = 0; i < numSock; i++)
            {
                if (FD_ISSET(socks[i], &readSocks) == 0) 
                {
                    continue;
                }

                STAFTCPConnectionImpl connImpl;
                struct sockaddr_storage clientAddress;
                STAFSocketLen_t clientAddressLength = sizeof(clientAddress);
                closeClientSocket = 0;  // Don't need to close socket

                // Need a try block within the for loop so that if an
                // exception occurs, this thread won't terminate so this
                // interface can continue accepting requests

                try
                {
                    do
                    {
                        connImpl.clientSocket = accept(
                            socks[i],
                            reinterpret_cast<struct sockaddr *>(&clientAddress),
                            &clientAddressLength);
                
                    } while (!STAFUtilIsValidSocket(connImpl.clientSocket) &&
                             (STAFSocketGetLastError() == SOCEINTR));

                    if (provider->state == kSTAFConnectionProviderStopped)
                        break;

                    if (!STAFUtilIsValidSocket(connImpl.clientSocket))
                    {
                        STAFTrace::trace(
                            kSTAFTraceError,
                            STAFString("Error accepting on server socket, "
                                       "socket RC: ") +
                            STAFString(STAFSocketGetLastError()));

                        if (STAFSocketGetLastError() == EIO)
                        {
                            // On z/OS, if get error 122 (EIO) then end up in
                            // a continuous loop logging this trace error
                            // message.  The basic cause of an EIO error is
                            // the TCP/IP stack being terminated and
                            // subsequently restarted.

                            // Close client socket to try to prevent this
                            STAFSocketClose(connImpl.clientSocket);
                        }
                        
                        continue;
                    }

                    unsigned int connectTimeout = provider->connectTimeout;
                    connImpl.readWriteTimeout = connectTimeout / 1000 * 24;

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
                                       "STAFUtilGetNonInheritableSocket(), "
                                       "OS RC: ") + STAFString(osRC));
                        continue;
                    }

                    connImpl.clientSocket = newSocket;

                    // Turn on the SO_KEEPALIVE socket option (which is turned
                    // off by default on Windows and most operarting systems)
                    // to make the socket send keepalive messages on the
                    // session so that if one side of the connection is
                    // terminated, the other side will be notified after the
                    // keepalive time which is 2 hours by default for most
                    // operating systems.  See Bugs #1559514 and #2978990.

#ifdef STAF_OS_TYPE_WIN32
                    // On Windows, the SO_KEEPALIVE optval takes a boolean
                    bool optVal = true;
#else
                    // On Unix, the SO_KEEPALIVE optval takes an int
                    int optVal = 1;
#endif
                    if (setsockopt(connImpl.clientSocket, SOL_SOCKET,
                                   SO_KEEPALIVE, (char*)&optVal,
                                   sizeof(optVal)) != 0)
                    {
                        STAFTrace::trace(
                            kSTAFTraceWarning,
                            STAFString("Error setting SO_KEEPALIVE option: "
                                       "setsockopt() RC=") +
                            STAFSocketGetLastError());
                    }

#ifdef STAF_USE_SSL
                    if (provider->secure.isEqualTo(
                        sYes, kSTAFStringCaseInsensitive))
                    {
                        int sslDebug = 0;  // Before SSL_new()

                        try
                        {
                            // Do server side SSL. 

                            connImpl.ssl = SSL_new(provider->server_ctx);
                            sslDebug = 1;  // Before SSL_set_fd()

                            if (connImpl.ssl == NULL)
                            {
                                // Close client socket so request won't hang
                                STAFSocketClose(connImpl.clientSocket);

                                STAFTrace::trace(kSTAFTraceWarning,
                                     STAFString(
                                         "Error getting server SSL object: ") +
                                     STAFString(
                                         ERR_error_string(ERR_get_error(),
                                                          NULL)));
                                continue;
                            }

                            // Set connection to SSL state 

                            SSL_set_fd(connImpl.ssl, connImpl.clientSocket);
                            sslDebug = 2;  // Before SSL_accept()

                            // Start the handshaking 

                            if (SSL_accept(connImpl.ssl) == -1)
                            {
                                // Close client socket so request won't hang
                                STAFSocketClose(connImpl.clientSocket);

                                SSL_free (connImpl.ssl);

                                // Log a trace warning

                                STAFTCPConnectionImpl myConn;
                                STAFIPv6TCPUpdateConnectionNetworkIDsFromInAddr(
                                    &myConn, (struct sockaddr*)&clientAddress,
                                    clientAddressLength);

                                STAFTrace::trace(
                                    kSTAFTraceWarning, STAFString(
                                        "Error in server SSL handshake "
                                        "originating from machine ") +
                                    myConn.logicalNetworkID +
                                    ".  A possible cause is a non-secure tcp "
                                    "interface submitted a request to a "
                                    "secure tcp interface which is not "
                                    "supported.  " + STAFString(
                                        ERR_error_string(ERR_get_error(),
                                                         NULL)));

                                continue;
                            }
                        }
                        catch (...)
                        {
                            try
                            {
                                // Close client socket so request won't hang
                                STAFSocketClose(connImpl.clientSocket);

                                if (connImpl.ssl != NULL)
                                    SSL_free(connImpl.ssl);
                            }
                            catch (...)
                            { /* Do nothing */ }

                            STAFString debugMsg = STAFString("SSL_new");

                            if (sslDebug == 1)
                                debugMsg = STAFString("SSL_set_fd");
                            else if (sslDebug == 2)
                                debugMsg = STAFString("SSL_accept");

                            STAFTrace::trace(
                                kSTAFTraceError, STAFString(
                                    "Caught unknown exception using SSL in "
                                    "STAFTCPRunThreadIPv6: ") +
                                debugMsg + "()");

                            continue;
                        }
                    }

                    connImpl.secure = provider->secure;
#endif
                    // Ok, let's perform the callback now

                    TCPConnectionData connData;

                    connData.connFunc   = provider->connFunc;
                    connData.provider   = provider;
                    connData.connection = new STAFTCPConnectionImpl(connImpl);

                    STAFIPv6TCPUpdateConnectionNetworkIDsFromInAddr(
                        connData.connection, (struct sockaddr*)&clientAddress,
                        clientAddressLength);

                    unsigned int dispatchRC = provider->threadManager->
                        dispatch(STAFTCPConnectionThread,
                                 new TCPConnectionData(connData));

                    if (dispatchRC != 0)
                    {
                        STAFTrace::trace(
                            kSTAFTraceError,
                            STAFString("STAFTCPRunThreadIPv6: "
                                       "Error dispatching a thread, RC: ") +
                            STAFString(dispatchRC));

                        STAFConnectionImpl *baseImpl = connData.connection;
                        STAFString_t errorBufferT = 0;
                        STAFConnectionDestruct(&baseImpl, &errorBufferT);

                        continue;
                    }
                }
                catch (STAFException &e)
                {
                    if (closeClientSocket)
                    {
                        try
                        {
                            // Close client socket so  request won't hang
                            STAFSocketClose(connImpl.clientSocket);
                        }
                        catch (...)
                        { /* Do nothing */ }
                    }

                    STAFTrace::trace(
                        kSTAFTraceError, getExceptionString(
                            e, "STAFTCPConnProvider.cpp: "
                            "STAFTCPRunThreadIPv6"));
                }                                              
                catch (...)
                {
                    if (closeClientSocket)
                    {
                        try
                        {
                            // Close client socket so request won't hang
                            STAFSocketClose(connImpl.clientSocket);
                        }
                        catch (...)
                        { /* Do nothing */ }
                    }

                    STAFTrace::trace(
                        kSTAFTraceError, "STAFTCPConnProvider.cpp: "
                        "Caught unknown exception in STAFTCPRunThreadIPv6()");
                }
            } // end for        
        } // end while
    }
    CATCH_STANDARD_TRACE("STAFTCPRunThreadIPv6:exitLoop");

    try
    {
        provider->syncSem->post();
    }
    CATCH_STANDARD_TRACE("STAFTCPRunThreadIPv6:post");

    return 0;
}
#else

static unsigned int STAFTCPRunThreadIPv4(void *providerImpl)
{
    STAFTCPConnectionProviderImpl *provider =
        reinterpret_cast<STAFTCPConnectionProviderImpl *>(providerImpl);

    try
    {
        provider->syncSem->post();
    }
    CATCH_STANDARD_TRACE("STAFTCPRunThreadIPv4: post");
    
    while (provider->state == kSTAFConnectionProviderActive)
    {
        STAFTCPConnectionImpl connImpl;
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
                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("Error accepting on server socket, "
                               "socket RC: ") +
                                    STAFString(STAFSocketGetLastError()));

                if (STAFSocketGetLastError() == EIO)
                {
                    // On z/OS, if get error 122 (EIO) then end up in
                    // a continuous loop logging this trace error
                    // message.  The basic cause of an EIO error is
                    // the TCP/IP stack being terminated and
                    // subsequently restarted.

                    // Close client socket to try to prevent this
                    STAFSocketClose(connImpl.clientSocket);
                }

                continue;
            }

            unsigned int connectTimeout = provider->connectTimeout;
            connImpl.readWriteTimeout = connectTimeout / 1000 * 24;

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
                    kSTAFTraceWarning,
                    STAFString("Error getting non-inheritable client socket, "
                               "STAFUtilGetNonInheritableSocket(), OS RC: ") +
                    STAFString(osRC));

                continue;
            }

            connImpl.clientSocket = newSocket;

            // Turn on the SO_KEEPALIVE socket option (which is turned
            // off by default on Windows and most operarting systems)
            // to make the socket send keepalive messages on the
            // session so that if one side of the connection is
            // terminated, the other side will be notified after the
            // keepalive time which is 2 hours by default for most
            // operating systems.  See Bugs #1559514 and #2978990.

#ifdef STAF_OS_TYPE_WIN32
            // On Windows, the SO_KEEPALIVE optval takes a boolean
            bool optVal = true;
#else
            // On Unix, the SO_KEEPALIVE optval takes an int
            int optVal = 1;
#endif
            if (setsockopt(connImpl.clientSocket, SOL_SOCKET, SO_KEEPALIVE,
                           (char*)&optVal, sizeof(optVal)) != 0)
            {
                STAFTrace::trace(
                    kSTAFTraceWarning,
                    STAFString("Error setting SO_KEEPALIVE option: "
                               "setsockopt() RC=") +
                    STAFSocketGetLastError());
            }
            
#ifdef STAF_USE_SSL
            if (provider->secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
            {
                int sslDebug = 0;  // Before SSL_new()

                try
                {
                    // Do server side SSL

                    connImpl.ssl = SSL_new(provider->server_ctx);
                    sslDebug = 1;  // Before SSL_set_fd()
                    
                    if (connImpl.ssl == NULL)
                    {
                        // Close the client socket so the request won't hang
                        STAFSocketClose(connImpl.clientSocket);

                        STAFTrace::trace(
                            kSTAFTraceWarning,
                            STAFString("Error getting server SSL object: ") +
                            STAFString(ERR_error_string(ERR_get_error(),
                                                        NULL)));
                        continue;
                    }
                    
                    // Set connection to SSL state 

                    SSL_set_fd(connImpl.ssl, connImpl.clientSocket);
                    sslDebug = 2;  // Before SSL_accept()

                    // Start the handshaking 

                    if (SSL_accept(connImpl.ssl) == -1)
                    {
                        // Close the client socket so the request won't hang
                        STAFSocketClose(connImpl.clientSocket);

                        SSL_free(connImpl.ssl);

                        // Log a trace warning for server SSL handshake error

                        STAFTCPConnectionImpl myConn;
                        STAFTCPUpdateConnectionNetworkIDsFromInAddr(
                            &myConn, &clientAddress.sin_addr);

                        STAFTrace::trace(
                            kSTAFTraceWarning, STAFString(
                                "Error in server SSL handshake originating "
                                "from machine ") +
                            myConn.logicalNetworkID + ".  A possible cause "
                            "is a non-secure tcp interface submitted a "
                            "request to a secure tcp interface which is not "
                            "supported.  " + STAFString(
                                ERR_error_string(ERR_get_error(), NULL)));

                        continue;
                    }
                }
                catch (...)
                {
                    try
                    {
                        // Close the client socket so the request won't hang
                        STAFSocketClose(connImpl.clientSocket);

                        if (connImpl.ssl != NULL)
                            SSL_free(connImpl.ssl);
                    }
                    catch (...)
                    { /* Do nothing */ }

                    STAFString debugMsg = STAFString("SSL_new");

                    if (sslDebug == 1)
                        debugMsg = STAFString("SSL_set_fd");
                    else if (sslDebug == 2)
                        debugMsg = STAFString("SSL_accept");

                    STAFTrace::trace(
                        kSTAFTraceError, STAFString(
                            "Caught unknown exception using SSL in "
                            "STAFTCPRunThreadIPv4: ") + debugMsg + "()");

                    continue;
                }
            }

            connImpl.secure = provider->secure;
#endif
            // Ok, let's perform the callback now

            TCPConnectionData connData;

            connData.connFunc   = provider->connFunc;
            connData.provider   = provider;
            connData.connection = new STAFTCPConnectionImpl(connImpl);

            STAFTCPUpdateConnectionNetworkIDsFromInAddr(
                connData.connection, &clientAddress.sin_addr);

            unsigned int dispatchRC = provider->threadManager->dispatch(
                STAFTCPConnectionThread, new TCPConnectionData(connData));

            if (dispatchRC != 0)
            {
                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("STAFTCPRunThreadIPv4: "
                               "Error dispatching a thread, RC: ") +
                    STAFString(dispatchRC));
                continue;
            }
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
                    e, "STAFTCPConnProvider.cpp: STAFTCPRunThreadIPv4"));
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

            STAFTrace::trace(
                kSTAFTraceError, "STAFTCPConnProvider.cpp: Caught unknown "
                "exception in STAFTCPRunThreadIPv4()");
        }
    } // end while loop

    try
    {
        provider->syncSem->post();
    }
    CATCH_STANDARD_TRACE("STAFTCPRunThreadIPv4: post end");

    return 0;
}
#endif

#ifdef STAF_USE_SSL

// This flag indicates if the SSL threads have been initialized as only need
// to do once (and there could be multiple ssl interfaces registered
static int init_ssl_threads = 0;

// This is a pointer to an array of mutexes needed by the OpenSSL locking
// callback function
#ifdef STAF_OS_TYPE_WIN32
static HANDLE *lock_cs;
#else
static pthread_mutex_t *lock_cs;
#endif

/*
 * The locking callback function is needed to perform locking on shared data
 * structures used by OpenSSL whenever multiple threads use OpenSSL.
 * This function must be able to handle up to CRYPTO_num_locks() different
 * mutex locks. It sets the n-th lock if mode & CRYPTO_LOCK, and releases it
 * otherwise.  We define one locking callback function for Windows and another
 * for Unix operating systems.
 */
#ifdef STAF_OS_TYPE_WIN32
static void STAF_SSL_Locking_Callback(int mode, int type,
                                      const char *file, int line)
{
    if (mode & CRYPTO_LOCK)
        WaitForSingleObject(lock_cs[type], INFINITE);
    else
        ReleaseMutex(lock_cs[type]);
}

#else
static void STAF_SSL_Locking_Callback(int mode, int type,
                                      const char *file, int line)
{
    if (mode & CRYPTO_LOCK)
        pthread_mutex_lock(&(lock_cs[type]));
    else
        pthread_mutex_unlock(&(lock_cs[type]));
}

/*
 * This callback function returns a thread ID.  It isn't needed on Windows
 * nor on platforms where getpid() returns a different ID for each thread.
 */
static unsigned long STAF_SSL_ThreadID_Callback()
{
    return (unsigned long)pthread_self();
}
#endif

/*
 * This function assigns the callback functions needed for multi-threaded
 * applications that use OpenSSL so that they don't randomly crash.
 */
static void STAF_SSL_Thread_Setup()
{
    if (init_ssl_threads)
        return;
    
    init_ssl_threads = 1;
    
    // The CRYPTO_set_locking_callback() method requires an array of mutexes
    // that is needed for locking access to global shared data structure
    // used by OpenSSL

#   ifdef STAF_OS_TYPE_WIN32
        lock_cs = (HANDLE *)OPENSSL_malloc(
            CRYPTO_num_locks() * sizeof(HANDLE));
#   else
        lock_cs = (pthread_mutex_t *)OPENSSL_malloc(
            CRYPTO_num_locks() * sizeof(pthread_mutex_t));
#   endif

    for (int i = 0; i < CRYPTO_num_locks(); i++)
    {
#       ifdef STAF_OS_TYPE_WIN32
            lock_cs[i] = CreateMutex(NULL, FALSE, NULL);
#       else
            pthread_mutex_init(&(lock_cs[i]), NULL);
#       endif
    }

    // Assign callback functions needed for multi-threaded applications
    // that use OpenSSL

    CRYPTO_set_locking_callback(STAF_SSL_Locking_Callback);

    // If Unix, assign a callback function for returning a thread id

#   ifndef STAF_OS_TYPE_WIN32
        CRYPTO_set_id_callback(STAF_SSL_ThreadID_Callback);
#   endif
}

static void STAF_SSL_Thread_Cleanup()
{
    if (!init_ssl_threads)
        return;

    init_ssl_threads = 0;
            
    // Remove the SSL locking callback function

    CRYPTO_set_locking_callback(NULL);

    // If Unix, Remove the SSL thread id callback function

#   ifndef STAF_OS_TYPE_WIN32
        CRYPTO_set_id_callback(NULL);
#   endif

    // Delete the mutexes used for locking by OpenSSL

    for (int i = 0; i < CRYPTO_num_locks(); i++)
    {
#       ifdef STAF_OS_TYPE_WIN32
            CloseHandle(lock_cs[i]);
#       else
            pthread_mutex_destroy(&(lock_cs[i]));
#       endif
    }

    OPENSSL_free(lock_cs);
}


int password_cb(char *buf, int size, int rwflag, void *userdata)
{

    cout << "Please enter password for your private key file:" << endl;
    
#ifdef STAF_OS_TYPE_WIN32

    int i = 0;
    do 
    {
        buf[i] = getch();
        i++;
    
    } while (buf[i-1] != '\r' && buf[i-1] != '\n' && i < size - 2);

    if (buf[i-1] == '\r' || buf[i-1] == '\n')
        i--;

    buf[i] = '\0';
#else   
    char *passwd;
    
    passwd = getpass(" ");
    memcpy(buf, passwd, strlen(passwd));
    *(buf+strlen(passwd)+1) = 0;
#endif

    return strlen(buf);
}

/* XXX: Not currently used
unsigned int VerifyCertificateHostname(X509* cert, STAFString hostname)
{
    int extcount, i, j, ok=0;
    char name[256];
    X509_NAME *subj;
    const char *extstr;
    CONF_VALUE *nval;
    const unsigned char *data;
    X509_EXTENSION *ext;
    X509V3_EXT_METHOD *meth;
    STACK_OF(CONF_VALUE) *val;
    
    if ((extcount = X509_get_ext_count(cert)) > 0)
    {
        for (i=0; !ok && i < extcount; i++)
        {
            ext = X509_get_ext(cert, i);
            extstr = OBJ_nid2sn(OBJ_obj2nid(X509_EXTENSION_get_object(ext)));
            
            if (STAFString(extstr) != "subjectAltName")
            {
                if (!(meth = X509V3_EXT_get(ext))) break;
                data = ext->value->data;
                
                // XXX:  If uncomment this method, need to fix the following
                // line to resolve the following error "invalid conversion
                // from 'const unsigned char**' to 'unsigned char**'" that
                // you can get using gcc 4.1.2 or later
                val = meth->i2v(meth, meth->d2i(0, &data, ext->value->length), 0);
                for (j = 0; j < sk_CONF_VALUE_num(val); j++)
                {
                    nval = sk_CONF_VALUE_value(val, j);

                    if (STAFString(nval->name) != "DNS" && STAFString(nval->value) != hostname)
                    {
                        ok = 1;
                        break;
                    }
                }
            }
        }
    }
    
    if (!ok && (subj = X509_get_subject_name(cert)) &&
        X509_NAME_get_text_by_NID(subj, NID_commonName, name, sizeof(name)) > 0)
    {
        name[sizeof(name) - 1] = '\0';
        if (STAFString(name) != hostname) ok = 1;
    }
    
    return ok;
}
*/
#endif


#define HANDLE_START_ERROR(string, function) \
{\
    STAFString theError = STAFString(string) + STAFString(", " function \
                                                          " RC=") + \
                          STAFString(STAFSocketGetLastError()); \
\
    if (errorBuffer) *errorBuffer = theError.adoptImpl();\
    return kSTAFBaseOSError;\
}

STAFRC_t STAFConnectionProviderConstruct(STAFConnectionProvider_t *baseProvider,
                                         void *constructInfo,
                                         unsigned int constructInfoLevel,
                                         STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidParm;
    if (constructInfoLevel != 1) return kSTAFInvalidAPILevel;

    STAFConnectionProviderConstructInfoLevel1 *cpInfo =
        reinterpret_cast<STAFConnectionProviderConstructInfoLevel1 *>(
            constructInfo);

    STAFRC_t rc = kSTAFOk;

    try
    {
        STAFTCPConnectionProviderImpl tcpData;

        tcpData.mode          = cpInfo->mode;
        tcpData.syncSem       = STAFEventSemPtr(new STAFEventSem,
                                                STAFEventSemPtr::INIT);
        tcpData.state         = kSTAFConnectionProviderStopped;
        tcpData.port          = sDefaultNonSecurePort;
        tcpData.connectTimeout = sDefaultConnectTimeout;
        tcpData.family        = PF_INET;
#ifdef STAF_USE_IPV6
        tcpData.family        = PF_UNSPEC;
#endif
        tcpData.secure        = sNo;
#ifdef STAF_USE_SSL

        // Get the STAF configuration information (to get the STAFRoot)

        STAFConfigInfo configInfo;
        unsigned int osRC = 0;

        rc = STAFUtilGetConfigInfo(&configInfo, errorBuffer, &osRC);

        if (rc != kSTAFOk)
        {
            if (errorBuffer)
            {
                STAFString errorMsg = "STAFUtilGetConfigInfo: " + 
                    STAFString(*errorBuffer) +
                    ", OS rc: " + STAFString(osRC);
                *errorBuffer = errorMsg.adoptImpl();
            }

            return rc;
        }

        // Assign the default locations for the files needed for a secure
        // TCP connection provider.

        tcpData.serverCertificate = STAFString(configInfo.exePath) +
            configInfo.fileSeparator + "bin" + configInfo.fileSeparator +
             "STAFDefault.crt";
        tcpData.serverKey = STAFString(configInfo.exePath) +
             configInfo.fileSeparator + "bin" + configInfo.fileSeparator +
            "STAFDefault.key";
        tcpData.CACertificate = STAFString(configInfo.exePath) +
            configInfo.fileSeparator + "bin" + configInfo.fileSeparator +
            "CAList.crt";
#endif
        tcpData.serverSocket  = -1;
        tcpData.serverSocketIPv6 = -1;
        tcpData.threadManager =
            STAFThreadManagerPtr(new STAFThreadManager,
                                 STAFThreadManagerPtr::INIT);

        int isPortSet = 0;

        for (unsigned int i = 0; i < cpInfo->numOptions; ++i)
        {
            STAFString thisOption = STAFString(cpInfo->optionNames[i]);

            if (thisOption.isEqualTo(sPort, kSTAFStringCaseInsensitive))
            {
                // Convert port value to a number in range 0 to USHRT_MAX

                unsigned int port;
                STAFString_t errorBufferT = 0;

                rc = STAFUtilConvertStringToUInt(
                    cpInfo->optionValues[i], sPort.getImpl(), &port,
                    &errorBufferT, 0, USHRT_MAX);

                if (rc != kSTAFOk)
                {
                    if (errorBuffer) *errorBuffer = errorBufferT; 

                    return rc;
                }

                tcpData.port = (unsigned short)port;
                isPortSet = 1;
            }
            else if (thisOption.isEqualTo(sConnectTimeout,
                                          kSTAFStringCaseInsensitive))
            {
                // Convert connectTimeout value to a number in range
                // 0 to USHRT_MAX

                unsigned int connectTimeout;
                STAFString_t errorBufferT = 0;

                rc = STAFUtilConvertStringToUInt(
                    cpInfo->optionValues[i], sConnectTimeout.getImpl(),
                    &connectTimeout, &errorBufferT, 0, USHRT_MAX);

                if (rc != kSTAFOk)
                {
                    if (errorBuffer) *errorBuffer = errorBufferT; 

                    return rc;
                }

                tcpData.connectTimeout = (unsigned short)connectTimeout;
            }
            else if (thisOption.isEqualTo(sSecure,
                                          kSTAFStringCaseInsensitive))
            {
                STAFString secure = cpInfo->optionValues[i];

                if (secure.isEqualTo(sNo, kSTAFStringCaseInsensitive))
                {
                    tcpData.secure = sNo;
                }
#ifdef STAF_USE_SSL
                else if (secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
                {
                    tcpData.secure = sYes;
                }
#endif
                else
                {
                    if (errorBuffer)
                    {
                        *errorBuffer = STAFString(
                            "SECURE must be set to No"
#ifdef STAF_USE_SSL
                            " or Yes"
#endif
                            ).adoptImpl();
                    }

                    return kSTAFInvalidValue;
                }
            }
#ifdef STAF_USE_SSL 
            else if (thisOption.isEqualTo(sServerCertificate,
                                          kSTAFStringCaseInsensitive))
            {
                tcpData.serverCertificate = cpInfo->optionValues[i];
            }
            else if (thisOption.isEqualTo(sServerKey,
                                          kSTAFStringCaseInsensitive))
            {
                tcpData.serverKey = cpInfo->optionValues[i];
            }
            else if (thisOption.isEqualTo(sCACertificate,
                                          kSTAFStringCaseInsensitive))
            {
                tcpData.CACertificate = cpInfo->optionValues[i];
            }
#endif
            else if (thisOption.isEqualTo(sProtocol, kSTAFStringCaseInsensitive))
            {
                STAFString protocol;
                protocol = cpInfo->optionValues[i];

                if (protocol.isEqualTo(sIPv4, kSTAFStringCaseInsensitive))
                {
                    tcpData.family = PF_INET;
                }
#ifdef STAF_USE_IPV6
                else if (protocol.isEqualTo(sIPv6, kSTAFStringCaseInsensitive))
                {
                    tcpData.family = PF_INET6;
                }
                else if (protocol.isEqualTo(sIPv4_IPv6,
                                            kSTAFStringCaseInsensitive))
                {
                    tcpData.family = PF_UNSPEC;
                }
#endif
                else
                {
                    if (errorBuffer)
                    {
                        *errorBuffer = STAFString(
                            "PROTOCOL must be set to IPv4"
#ifdef STAF_USE_IPV6
                            ", IPv6, or IPv4_IPv6"
#endif
                            ).adoptImpl();
                    }

                    return kSTAFInvalidValue;
                }
            }
            else
            {
                if (errorBuffer)
                {
                    *errorBuffer = STAFString(
                        "Invalid option: " + STAFString(thisOption)).
                        adoptImpl();
                }
                return kSTAFInvalidValue;
            }
        }

#ifdef STAF_USE_SSL
        if (!isPortSet)
        {
            if (tcpData.secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
            {
                tcpData.port = sDefaultSecurePort;
            }
            else
            {
                tcpData.port = sDefaultNonSecurePort;
            }                
        }
#endif
        
        // Add each option to a map.

        tcpData.options = STAFObject::createMap();
        tcpData.options->put(sPort, STAFString(tcpData.port));
        tcpData.options->put(sConnectTimeout,
                             STAFString(tcpData.connectTimeout));
        tcpData.options->put(sSecure, tcpData.secure);
#ifdef STAF_USE_SSL
        if (tcpData.secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
        {
            // Verify that the specified serverCertificate file exists

            STAFFSPath entryPath(tcpData.serverCertificate);

            if (!entryPath.exists())
            {
                if (errorBuffer)
                {
                    *errorBuffer = STAFString(
                        "SSL/ServerCertificate file " +
                        tcpData.serverCertificate + " does not exist").
                        adoptImpl();
                }

                return kSTAFInvalidValue;
            }
            
            // Verify that the specified serverKey file exists

            entryPath = STAFFSPath(tcpData.serverKey);

            if (!entryPath.exists())
            {
                if (errorBuffer)
                {
                    *errorBuffer = STAFString(
                        "SSL/ServerKey file " + tcpData.serverKey +
                        " does not exist").adoptImpl();
                }

                return kSTAFInvalidValue;
            }

            // Verify that the specified CACertificate file exists

            entryPath = STAFFSPath(tcpData.CACertificate);

            if (!entryPath.exists())
            {
                if (errorBuffer)
                {
                    *errorBuffer = STAFString(
                        "SSL/CACertificate file " + tcpData.CACertificate +
                        " does not exist").adoptImpl();
                }

                return kSTAFInvalidValue;
            }

            tcpData.options->put(sServerCertificate, tcpData.serverCertificate);
            tcpData.options->put(sServerKey, tcpData.serverKey);
            tcpData.options->put(sCACertificate, tcpData.CACertificate);
        }
#endif

#ifdef STAF_USE_IPV6
        if (tcpData.family == PF_INET)
            tcpData.options->put(sProtocol, sIPv4);
        else if (tcpData.family == PF_INET6)
            tcpData.options->put(sProtocol, sIPv6);
        else // if (tcpData.family == PF_UNSPEC)
            tcpData.options->put(sProtocol, sIPv4_IPv6);        
#else
        tcpData.options->put(sProtocol, sIPv4);
#endif

        // Setup property values

        tcpData.portProperty = STAFString(kUTF8_AT) + tcpData.port;
        tcpData.isSecureProperty = STAFString("0");

#ifdef STAF_USE_SSL
        if (tcpData.secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
        {
            tcpData.isSecureProperty = STAFString("1");
        }
#endif

        // Get logical identifier (the host name) and physical identifier (the
        // IP address)

        STAFString_t machineNameImpl = 0;
        STAFString_t ipAddressImpl = 0;
        STAFString_t errorBuffer2 = 0;
        STAFRC_t hostRC = STAFSocketGetMyHostInfo(&machineNameImpl,
                                                  &ipAddressImpl,
                                                  &errorBuffer2);
        if (hostRC != 0)
        {
            STAFString error(
                "Could not determine logical/physical identifier."
                "Error code: " + STAFString(hostRC) +
                STAFString("  Reason: ") +
                STAFString(errorBuffer2, STAFString::kShallow));

            if (errorBuffer) *errorBuffer = error.adoptImpl();

            errorBuffer2 = 0;

            //XXX: Is this the right RC?
            return kSTAFInvalidValue;
        }

        tcpData.logicalNetworkID = STAFString(machineNameImpl);
        tcpData.physicalNetworkID = STAFString(ipAddressImpl);
        
        rc = STAFSocketInit(errorBuffer);

        if (rc != kSTAFOk) return rc;

        *baseProvider = new STAFTCPConnectionProviderImpl(tcpData);

        STAFTCPConnectionProviderImpl **provider =
            reinterpret_cast<STAFTCPConnectionProviderImpl **>(baseProvider);

#ifdef STAF_USE_SSL
        if (tcpData.secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
        {
            // generate random seed

            int seed_int[100]; 
            srand( (unsigned)time( NULL ) );
            for( int j = 0;   j < 100; j++ )
                seed_int[j] = rand();
            RAND_seed(seed_int, sizeof(seed_int));

            // SSL preliminaries. We keep the certificate and key with the context. 

            OpenSSL_add_all_algorithms();   
            SSL_load_error_strings();
            SSL_library_init();

            // Prepare server side SSL context

            (*provider)->server_ctx = SSL_CTX_new(TLSv1_server_method());

            if (!(*provider)->server_ctx)
                HANDLE_START_ERROR(ERR_reason_error_string(ERR_get_error()), 
                                   "SSL_CTX_new()");

            if ((*provider)->serverCertificate.length() != 0)
            {
                char name[256] = "";
 
                memcpy(name, 
                       (*provider)->serverCertificate.buffer(), 
                       (*provider)->serverCertificate.length());

                if(SSL_CTX_use_certificate_chain_file((*provider)->server_ctx,
                                                       name) <= 0)
                    HANDLE_START_ERROR(ERR_error_string(ERR_get_error(), NULL), 
                                       "SSL_CTX_use_certificate_file()");            
            }

            if ((*provider)->serverKey.length() != 0)
            {
                char name[256] = "";

                memcpy(name, 
                       (*provider)->serverKey.buffer(), 
                       (*provider)->serverKey.length());
    
                SSL_CTX_set_default_passwd_cb((*provider)->server_ctx,
                                              password_cb);

                if (SSL_CTX_use_PrivateKey_file((*provider)->server_ctx, 
                                                name,
                                                SSL_FILETYPE_PEM) <= 0)
                    HANDLE_START_ERROR(ERR_error_string(ERR_get_error(), NULL), 
                                       "SSL_CTX_use_PrivateKey_file()");
            }

            if ((*provider)->serverKey.length() != 0 ||
                (*provider)->serverCertificate.length() != 0)
            {
                if (!SSL_CTX_check_private_key((*provider)->server_ctx))
                    HANDLE_START_ERROR(ERR_error_string(ERR_get_error(), NULL), 
                                       "SSL_CTX_check_private_key()");
            }

            if (!SSL_CTX_set_cipher_list((*provider)->server_ctx,"AES128-SHA"))
                HANDLE_START_ERROR(ERR_error_string(ERR_get_error(), NULL), 
                                   "SSL_CTX_set_cipher_list()");

            // Prepare client side SSL context

            (*provider)->client_ctx = SSL_CTX_new(TLSv1_client_method());

            if (!(*provider)->client_ctx)
                HANDLE_START_ERROR(ERR_error_string(ERR_get_error(), NULL), 
                                   "SSL_CTX_new()");

            SSL_CTX_set_verify((*provider)->client_ctx, SSL_VERIFY_PEER, NULL);   

            char name[256] = "";

            memcpy(name, 
                   (*provider)->CACertificate.buffer(), 
                   (*provider)->CACertificate.length());

            if (SSL_CTX_load_verify_locations((*provider)->client_ctx, name, 
                                              NULL) <= 0)
                HANDLE_START_ERROR(ERR_error_string(ERR_get_error(), NULL), 
                                   "SSL_CTX_load_verify_locations()");

            // Perform SSL Thread Setup and assign callback functions needed for
            // multi-threaded applications that use OpenSSL
            
            STAF_SSL_Thread_Setup();
        }
#endif

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderConstruct");

    return kSTAFUnknownError;
}

 
#ifdef STAF_OS_TYPE_WIN32
#define SET_SOCKET_IPv6_ONLY \
            ;
#else
#define SET_SOCKET_IPv6_ONLY \
        if (openIPv6OnlySocket)\
        {\
            int on = 1;\
            if (setsockopt(provider->serverSocketIPv6, IPPROTO_IPV6, \
                           openIPv6OnlySocket, &on, sizeof(on)) < 0) \
            {\
                HANDLE_START_ERROR("Error setting to IP6 only: IPv6", \
                                   "socket()");\
            }\
        }
#endif

STAFRC_t STAFConnectionProviderStartIPv4(STAFConnectionProvider_t baseProvider,
                                         STAFString_t *errorBuffer)
{
    STAFTCPConnectionProviderImpl *provider =
        static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

    provider->serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    if (!STAFUtilIsValidSocket(provider->serverSocket))
        HANDLE_START_ERROR("No socket available", "socket()");

    // Set the socket to be non-inheritable

    unsigned int osRC = 0;
    STAFSocket_t newSocket;
    
    if (STAFUtilGetNonInheritableSocket(provider->serverSocket,
                                        &newSocket, &osRC))
    {
        HANDLE_START_ERROR("Error getting non-inheritable server socket",
                           "STAFUtilGetNonInheritableSocket()");
    }

    provider->serverSocket = newSocket;

    // Turn on the SO_REUSEADDR socket option to get rid of the pesky
    // "Address already in use" error message

    int on = 1;

    if (setsockopt(provider->serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   (char*)&on, sizeof(on)) < 0)
    {
        HANDLE_START_ERROR("Error setting server socket to reuse address",
                           "setsockopt()");
    }

    struct sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(provider->port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int bindRC = bind(provider->serverSocket,
                      reinterpret_cast<struct sockaddr *>(&serverAddress),
                      sizeof(serverAddress));

    if (bindRC != 0)
        HANDLE_START_ERROR("Error binding server socket", "bind()");

    int listenRC = listen(provider->serverSocket, SOMAXCONN);

    if (listenRC != 0)
        HANDLE_START_ERROR("Error listening on server socket", "listen()");

    return kSTAFOk;
        
}

STAFRC_t STAFConnectionProviderStartIPv6(STAFConnectionProvider_t baseProvider,
                                         STAFString_t *errorBuffer)
{
#ifdef STAF_USE_IPV6

    STAFTCPConnectionProviderImpl *provider =
        static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

    struct addrinfo hints = { 0 };
    hints.ai_flags = AI_PASSIVE; // Any address is accepted
    hints.ai_family = PF_UNSPEC;  // Any family is accepted
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0; // Any protocol is accepted

    struct addrinfo *resipv4 = NULL, *resipv6 = NULL;

    STAFNetworkAddress na;

    int rc = getaddrinfo(NULL, 
                         (STAFString(provider->port) + 
                          STAFString(kUTF8_NULL)).buffer(),
                         &hints, na.getBuffer());

    if (rc != 0)
        HANDLE_START_ERROR("No address info available", "getaddrinfo()");

    while (na.next())
    {
        if (na.getCurrent()->ai_family == PF_INET6)
        {
            resipv6 = na.getCurrent();
        }
        else if (na.getCurrent()->ai_family == PF_INET)
        {
            resipv4 = na.getCurrent();
        }
    }

    // On Solaris, an IPv6 socket will accept IPv4 calls, and give them as 
    // mapped addresses. However, if an IPv4 socket is also listening on 
    // all interfaces, calls are directed to the appropriate socket.

    // On (some versions of) Linux (kernel earlier than 2.4.20), an IPv6 
    // socket will accept IPv4 calls, and give them as mapped addresses, 
    // but an attempt also to listen on an IPv4 socket on all interfaces 
    // causes an error. Kernel later than 2.4.20, it has the IPV6_V6ONLY
    // socket option.

    // On OpenBSD, an IPv6 socket will not accept IPv4 calls. You have to 
    // set up two sockets if you want to accept both kinds of call.

    // FreeBSD is like OpenBSD, but it has the IPV6_V6ONLY socket option, 
    // which can be turned off, to make it behave like the versions of 
    // Linux described above.

    // Windows is like OpenBSD.

    // IPv6 only, when connection provider working in a IPv4 and IPv6 mixed
    // mode, handle IPv6 first

    if (provider->family == PF_INET6 || provider->family == PF_UNSPEC)
    {
        if (resipv6 == NULL)
            HANDLE_START_ERROR("No supported IPv6 address", "resipv6");

        provider->serverSocketIPv6 = socket(resipv6->ai_family, 
                                            resipv6->ai_socktype,
                                            resipv6->ai_protocol);

        if (!STAFUtilIsValidSocket(provider->serverSocketIPv6))
            HANDLE_START_ERROR("No socket available: IPv6", "socket()");

        // Set the socket to be non-inheritable

        unsigned int osRC = 0;
        STAFSocket_t newSocket;

        if (STAFUtilGetNonInheritableSocket(provider->serverSocketIPv6,
                                            &newSocket, &osRC))
        {
            HANDLE_START_ERROR("Error getting non-inheritable server socket:"
                               " IPv6", "STAFUtilGetNonInheritableSocket()");
        }

        provider->serverSocketIPv6 = newSocket;
        
        // Turn on the SO_REUSEADDR socket option to get rid of the pesky
        // "Address already in use" error message

        int on = 1;

        if (setsockopt(provider->serverSocketIPv6, SOL_SOCKET, SO_REUSEADDR,
                       (char*)&on, sizeof(on)) < 0)
        {
            HANDLE_START_ERROR(
                "Error setting IPv6 server socket to reuse address",
                "setsockopt()");
        }

        SET_SOCKET_IPv6_ONLY;

        int bindRC = bind(provider->serverSocketIPv6,
                          resipv6->ai_addr,
                          resipv6->ai_addrlen);

        if (bindRC != 0)
            HANDLE_START_ERROR("Error binding server socket: IPv6", 
                               "bind()");

        int listenRC = listen(provider->serverSocketIPv6, SOMAXCONN);
        if (listenRC != 0)
            HANDLE_START_ERROR("Error listening on server socket : IPv6", 
                               "listen()");
    }

    // IPv4 only
        
    if (provider->family == PF_INET || provider->family == PF_UNSPEC)
    {
        if (resipv4 == NULL)
            HANDLE_START_ERROR("No supported IPv4 address", "resipv4");
        
        provider->serverSocket = socket(resipv4->ai_family, 
                                        resipv4->ai_socktype,
                                        resipv4->ai_protocol);

        if (!STAFUtilIsValidSocket(provider->serverSocket))
            HANDLE_START_ERROR("No socket available", "socket()");
                
        // Set the socket to be non-inheritable

        unsigned int osRC = 0;
        STAFSocket_t newSocket;

        if (STAFUtilGetNonInheritableSocket(provider->serverSocket,
                                            &newSocket, &osRC))
        {
            HANDLE_START_ERROR("Error getting non-inheritable server socket: "
                               "IPv4", "STAFUtilGetNonInheritableSocket()");
        }

        provider->serverSocket = newSocket;

        // Turn on the SO_REUSEADDR socket option to get rid of the pesky
        // "Address already in use" error message

        int on = 1;

        if (setsockopt(provider->serverSocket, SOL_SOCKET, SO_REUSEADDR,
                       (char*)&on, sizeof(on)) < 0)
        {
            HANDLE_START_ERROR(
                "Error setting IPv4 server socket to reuse address",
                "setsockopt()");
        }

        int bindRC = bind(provider->serverSocket,
                          resipv4->ai_addr,
                          resipv4->ai_addrlen);
                    
        if (bindRC != 0)
        {
            if (STAFSocketGetLastError() == EADDRINUSE &&
                STAFUtilIsValidSocket(provider->serverSocketIPv6))
            {
                // Since IPv6 socket is already opened, assume it 
                // also accepts IPv4 traffic, so we close the IPv4
                // socket and stop binding.
                    
                STAFSocketClose(provider->serverSocket);
                provider->serverSocket = -1;
            }
            else
                HANDLE_START_ERROR("Error binding server socket", 
                                   "bind()");
        }
        else
        {
            int listenRC = listen(provider->serverSocket, SOMAXCONN);

            if (listenRC != 0)
                HANDLE_START_ERROR("Error listening on server socket", 
                                   "listen()");
        }
    }

#endif

    return kSTAFOk;
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

        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

        provider->connFunc = cpInfo->newConnectionFunc;
        provider->data = cpInfo->data;

        STAFRC_t rc;

#ifdef STAF_USE_IPV6
        rc = STAFConnectionProviderStartIPv6(provider, errorBuffer);
#else 
        rc = STAFConnectionProviderStartIPv4(provider, errorBuffer);
#endif

        if (rc != kSTAFOk)
            return rc;

        // Ok, the provider is now ready

        provider->syncSem->reset();
        provider->state = kSTAFConnectionProviderActive;

        #ifdef STAF_USE_IPV6
        rc = provider->threadManager->dispatch(STAFTCPRunThreadIPv6, provider);
#else
        rc = provider->threadManager->dispatch(STAFTCPRunThreadIPv4, provider);
#endif
        if (rc != kSTAFOk)
        {
            STAFString theError = STAFString(
                "STAFConnectionProviderStart: Error dispatching a thread");

            if (errorBuffer) *errorBuffer = theError.adoptImpl();

            return rc;
        }

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
        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

        provider->state = kSTAFConnectionProviderStopped;
        
        // Connect to the server socket to wake up the run thread

        provider->syncSem->reset();
        
        STAFString host = STAFString(provider->logicalNetworkID);
        unsigned short port = provider->port;
        STAFSocket_t shutdownSocket;
        int rc = 0;

#ifdef STAF_USE_IPV6
        struct addrinfo hints = { 0 };
        hints.ai_flags = 0; // default
        hints.ai_family = PF_UNSPEC;  // Any family is accepted
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0; // Any protocol is accepted

        STAFNetworkAddress na;

        rc = getaddrinfo((host + STAFString(kUTF8_NULL)).buffer(), 
                         (STAFString(port) + STAFString(kUTF8_NULL)).buffer(),
                         &hints, na.getBuffer());

        if (rc == 0)
        {
            while (na.next())
            {
                if (na.getCurrent()->ai_family == PF_INET6 &&
                    (provider->family == PF_INET6 ||
                     provider->family == PF_UNSPEC))
                    break;

                if (na.getCurrent()->ai_family == PF_INET &&
                    (provider->family == PF_INET ||
                     provider->family == PF_UNSPEC))
                    break;
            }

            if (na.getCurrent() != NULL)
            {
                shutdownSocket = socket(
                    na.getCurrent()->ai_family, na.getCurrent()->ai_socktype,
                    na.getCurrent()->ai_protocol);
            }
            else
            {
                rc = 1;
            }
        }
#else //IPv4
        struct sockaddr_in serverAddress = { 0 };
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);

        if (host.findFirstNotOf("1234567890.") != STAFString::kNPos)
        {
            // This is a hostname

            unsigned int osRC = 0;

            rc = STAFSocketGetInAddrByName(
                host.getImpl(), &serverAddress.sin_addr, errorBuffer);
        }
        else
        {
            // This is an IP address

            serverAddress.sin_addr.s_addr =
                inet_addr(host.toCurrentCodePage()->buffer());
        }
        
        shutdownSocket = socket(PF_INET, SOCK_STREAM, 0);
#endif

        if ((rc == 0) && STAFUtilIsValidSocket(shutdownSocket))
        {
            int connectRC = 0;

#ifdef STAF_USE_IPV6
            connectRC = connect(
                shutdownSocket,
                na.getCurrent()->ai_addr, na.getCurrent()->ai_addrlen);
#else
            connectRC = connect(
                shutdownSocket,            
                reinterpret_cast<struct sockaddr *>(&serverAddress),
                sizeof(serverAddress));
#endif
        
            STAFSocketClose(shutdownSocket);
        }
        
        if (provider->syncSem->wait(10000) != 0)
        {
            STAFTrace::trace(
                kSTAFTraceWarning,
                STAFString("STAFTCPConnectionProviderStop - Timed out "
                           "waiting for run thread to wake up"));
        }

        // Release resources acquired when starting provider

        STAFSocketClose(provider->serverSocket);
#ifdef STAF_USE_IPV6
        STAFSocketClose(provider->serverSocketIPv6);
#endif

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
        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(*baseProvider);

        if (provider->state != kSTAFConnectionProviderStopped)
        {
            provider->state = kSTAFConnectionProviderStopped;

            // XXX: Should we check the RC?
            STAFSocketClose(provider->serverSocket);
#ifdef STAF_USE_IPV6
            STAFSocketClose(provider->serverSocketIPv6);
#endif
        }

#ifdef STAF_USE_SSL
        if (provider->secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
        {
            // Free SSL context

            SSL_CTX_free(provider->server_ctx);
            SSL_CTX_free(provider->client_ctx);

            // Free SSL Thread callbacks and mutexes used by the locking
            // callback function

            STAF_SSL_Thread_Cleanup();
        }
#endif

        delete provider;
        provider = 0;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderDestruct");

    return kSTAFUnknownError;
}


#ifdef STAF_USE_SSL
/****************************************************************************
 * The STAF_SSL_connect method calls SSL_connect() which initiates a SSL
 * handshake with a server. To prevent the call to SSL_connect() from hanging
 * in some situations, we have to call it using a non-blocking socket.
 * When a non-blocking socket is used, SSL_connect() will also return when it
 * could not satisfy the needs of SSL_connect() to continue the handshake.
 * In this case, a call to SSL_get_error() with the return value will yield
 * SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE in which case we need to call
 * select() to wait for data to be available up until the timeout is exceeded.
 * If the select() is successful, then we can retry calling SSL_connect().
 * If the select() times out or fails, an error is returned.
 *
 * IMPORTANT:  The socket provided must be in non-blocking mode.
 *                 
 * Input:  socket  - TCP socket descriptor
 *         ssl     - SSL object pointer
 *         timeout - connection timeout in seconds
 *
 * Returns:  0 if the SSL_connect is successful,
 *          -1 if the SSL_connect failed,
 *          -2 if the SSL_connect timed out                     
 ***************************************************************************/
int STAF_SSL_connect(int socket, SSL *ssl, unsigned int timeout)
{
    //  Set the connect timeout based on the timeout value provided in seconds
    timeval connect_timeout = { timeout / 1000, (timeout % 1000) * 1000 };
                
    // In a do while loop, call SSL_connect().  If it succeeds then the SSL
    // handshake is done.  If it fails with an error other than
    // SSL_ERROR_WANT_READ/WRITE, it's done.  If it fails with
    // SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE, call select() to wait for
    // data to become available, up to the specified connect timeout.  If
    // select() is successful, indicating data is now available, continue
    // looping so that SSL_connect() is called again.  If the select() times
    // out or fails, return an error.

    int rc = 0;

    do
    {
        rc = SSL_connect(ssl);

        if ((rc == 1) && SSL_is_init_finished(ssl))
        {
            // SSL_connect() handshake completed successfully
            return 0;
        }

        switch (SSL_get_error(ssl, rc))
        {
            case SSL_ERROR_NONE:
                // No error, we should have a connection, check again
                if ((rc == 1) && SSL_is_init_finished(ssl))
                {
                    // SSL_connect() handshake completed succesfully
                    return 0;
                }
                break;

            case SSL_ERROR_SYSCALL:
                // Explicitly check for SOCEWOULDBLOCK (Resource temporarily
                // unavailable) since it doesn't get converted to an
                // SSL_ERROR_WANT_READ/WRITE on some platforms.
                // If SSL_connect failed outright though, don't bother checking
                // more.  This can happen if the socket gets closed during the
                // handshake.

                if ((STAFSocketGetLastError() == SOCEWOULDBLOCK) &&
                    (rc == -1) && (SSL_want_read(ssl) || SSL_want_write(ssl)))
                {
                    // Wait for more activity
                    // Falls through to SSL_ERROR_WANT_WRITE case
                }
                else
                {
                    return -1;  // Return an error
                }

             case SSL_ERROR_WANT_READ:
                 // Falls through to SSL_ERROR_WANT_WRITE case
             case SSL_ERROR_WANT_WRITE:
                // No error, just wait until we have data or we time out

                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(socket, &fds);

                rc = select(socket + 1, &fds, NULL, NULL, &connect_timeout);

                if (rc == 0)
                    return -2;  // Select timed out
                else if (rc < 0)
                    return -1;  // Select failed

                break;

            case SSL_ERROR_ZERO_RETURN:
                // Failed because the client shutdown the connection
                return -1;  // Return an error
            
            default:
                // An unknown error occurred
                return -1;  // Return an error

        } // end switch
    } while (!SSL_is_init_finished(ssl));
    
    return -1;  // Return an error
}
#endif


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
    
    STAFTCPConnectionImpl connImpl;

    // Flag which indicates if need to close the client socket
    int closeClientSocket = 0;  // Don't need to close client socket

    try
    {
        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

        STAFString host = cpInfo->endpoint;
        unsigned short port = provider->port;

        // Extract out the port if specified.  Note, because IPv6 uses colons
        // in IP addresses, we use '@' to separate the host/IP address from
        // the port, instead of the more conventional ':'.

        unsigned int atPos = host.find(kUTF8_AT);

        if (atPos != STAFString::kNPos)
        {
            STAFString portString = host.subString(atPos + 1);

            // Convert port string to a number in range 0 to USHRT_MAX

            unsigned int portNumber;
            STAFString_t errorBufferT = 0;

            if (STAFUtilConvertStringToUInt(
                portString.getImpl(), STAFString("").getImpl(), &portNumber,
                &errorBufferT, 0, USHRT_MAX) == kSTAFOk)
            {
                // Successfully converted the port string to a number, so
                // assign the port and host

                port = portNumber;
                host = host.subString(0, atPos);
            }
        }

#ifdef STAF_USE_IPV6
        struct addrinfo hints = { 0 };
        hints.ai_flags = 0; // default
        hints.ai_family = PF_UNSPEC;  // Any family is accepted
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0; // Any protocol is accepted

        STAFNetworkAddress na;

        int rc = getaddrinfo((host + STAFString(kUTF8_NULL)).buffer(), 
                             (STAFString(port) + 
                              STAFString(kUTF8_NULL)).buffer(),
                             &hints, na.getBuffer());

        if (rc != 0)
        {
            STAFString error = STAFString(
                "Error getting address info: ") + host;
            if (errorBuffer) *errorBuffer = error.adoptImpl();
            return kSTAFCommunicationError;
        }

        while (na.next())
        {
            if (na.getCurrent()->ai_family == PF_INET6 &&
                (provider->family == PF_INET6 ||
                 provider->family == PF_UNSPEC))
                break;

            if (na.getCurrent()->ai_family == PF_INET &&
                (provider->family == PF_INET ||
                 provider->family == PF_UNSPEC))
                break;
        }

        if (na.getCurrent() == NULL)
        {
            STAFString error = STAFString(
                "Error getting IPv4 or IPv6 address info: ") + host;
            if (errorBuffer) *errorBuffer = error.adoptImpl();
            return kSTAFCommunicationError;
        }

        connImpl.clientSocket = socket(na.getCurrent()->ai_family,
                                       na.getCurrent()->ai_socktype,
                                       na.getCurrent()->ai_protocol);
#else //IPv4

        struct sockaddr_in serverAddress = { 0 };
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);

        if (host.findFirstNotOf("1234567890.") != STAFString::kNPos)
        {
            // This is a hostname

            unsigned int osRC = 0;

            STAFRC_t rc = STAFSocketGetInAddrByName(host.getImpl(),
                                                    &serverAddress.sin_addr,
                                                    errorBuffer);

            if (rc != kSTAFOk)
            {
                STAFString error = STAFString("Error resolving host name: ");

                if (errorBuffer)
                {
                    error += STAFString(*errorBuffer, STAFString::kShallow);
                    *errorBuffer = error.adoptImpl();
                }

                return kSTAFCommunicationError;
            }
        }
        else
        {
            // This is an IP address

            if (host.count(kUTF8_PERIOD) != 3)
            {
                STAFString error = STAFString("Invalid IP Address: ") + host;
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFCommunicationError;
            }

            serverAddress.sin_addr.s_addr =
                inet_addr(host.toCurrentCodePage()->buffer());
        }

        connImpl.clientSocket = socket(PF_INET, SOCK_STREAM, 0);
#endif

        if (!STAFUtilIsValidSocket(connImpl.clientSocket))
        {
            STAFString error = STAFString(
                "Error creating socket: socket() RC=") +
                STAFSocketGetLastError();
            if (errorBuffer) *errorBuffer = error.adoptImpl();
            return kSTAFCommunicationError;
        }

        closeClientSocket = 1;  // Need to close client socket if error

        // Set the socket to be non-inheritable
        
        unsigned int osRC = 0;
        STAFSocket_t newSocket;

        unsigned int connectTimeout = provider->connectTimeout;
        connImpl.readWriteTimeout = connectTimeout / 1000 * 24;

        if (STAFUtilGetNonInheritableSocket(connImpl.clientSocket,
                                            &newSocket, &osRC))
        {
            STAFString error = STAFString(
                "Error getting non-inheritable socket, "
                "STAFUtilGetNonInheritableSocket(), OS RC: ") +
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
        
        // Turn on the SO_KEEPALIVE socket option (which is turned off by
        // default on Windows and most operarting systems) to make the
        // socket send keepalive messages on the session so that if one
        // side of the connection is terminated, the other side will be
        // notified after the keepalive time which is 2 hours by default
        // for most operating systems.  See Bugs #1559514 and #2978990.

#ifdef STAF_OS_TYPE_WIN32
        // On Windows, the SO_KEEPALIVE optval takes a boolean
        bool optVal = true;
#else
        // On Unix, the SO_KEEPALIVE optval takes an int
        int optVal = 1;
#endif
        if (setsockopt(connImpl.clientSocket, SOL_SOCKET, SO_KEEPALIVE,
                       (char*)&optVal, sizeof(optVal)) != 0)
        {
            STAFString error = STAFString(
                "Error setting SO_KEEPALIVE option: setsockopt() RC=") +
                STAFSocketGetLastError();

            if (errorBuffer) *errorBuffer = error.adoptImpl();

            STAFSocketClose(connImpl.clientSocket);
            return kSTAFCommunicationError;
        }

        // Set non-blocking mode for the socket

        int modeRC = STAFSocketSetBlockingMode(connImpl.clientSocket,
                                               kSTAFSocketNonBlocking,
                                               errorBuffer);
        if (modeRC != kSTAFOk)
        {
            STAFString error = STAFString(
                "Error setting socket to non-blocking mode:");

            if (errorBuffer)
            {
                error += STAFString(*errorBuffer, STAFString::kShallow);
                *errorBuffer = error.adoptImpl();
            }

            STAFSocketClose(connImpl.clientSocket);
            return kSTAFCommunicationError;
        }

        int connectRC = 0;

#ifdef STAF_USE_IPV6
        connectRC = connect(connImpl.clientSocket,            
                            na.getCurrent()->ai_addr, 
                            na.getCurrent()->ai_addrlen);
#else
        connectRC = connect(connImpl.clientSocket,            
            reinterpret_cast<struct sockaddr *>(&serverAddress),
            sizeof(serverAddress));
#endif

        if ((connectRC < 0) &&
            (STAFSocketGetLastError() != SOCEINPROGRESS) &&
            (STAFSocketGetLastError() != SOCEWOULDBLOCK) &&
            (STAFSocketGetLastError() != 0))
        {
            STAFString error = STAFString(
                "Error connecting to endpoint: connect() RC=") +
                STAFSocketGetLastError();

            if (errorBuffer) *errorBuffer = error.adoptImpl();
            STAFSocketClose(connImpl.clientSocket);

            return kSTAFCommunicationError;
        }

        unsigned int timeout = provider->connectTimeout;

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
                STAFString error = STAFString(
                    "Error connecting to endpoint: select() RC=") +
                    STAFSocketGetLastError();

                if (errorBuffer) *errorBuffer = error.adoptImpl();
                STAFSocketClose(connImpl.clientSocket);

                return kSTAFCommunicationError;
            }
            else if (selectRC == 0)
            {
                STAFString error = STAFString(
                    "Timed out connecting to endpoint: select() timeout");

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
                    "Error performing test read on connected endpoint: recv()"
                    " RC=") + STAFSocketGetLastError();

                if (errorBuffer) *errorBuffer = error.adoptImpl();
                STAFSocketClose(connImpl.clientSocket);

                return kSTAFCommunicationError;
            }
        }
  
#ifdef STAF_USE_IPV6
        STAFIPv6TCPUpdateConnectionNetworkIDsFromInAddr(
            &connImpl, na.getCurrent()->ai_addr, na.getCurrent()->ai_addrlen);
#else
        STAFTCPUpdateConnectionNetworkIDsFromInAddr(
            &connImpl, &serverAddress.sin_addr);
#endif

#ifdef STAF_USE_SSL
        if (provider->secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
        {
            int sslDebug = 0;  // Before SSL_new()

            try
            {
                // Now we have a TCP connection. Start SSL negotiation. 
 
                connImpl.ssl = SSL_new(provider->client_ctx);
                sslDebug = 1;  // Before SSL_set_fd()

                if (connImpl.ssl == NULL)
                {
                    STAFSocketClose(connImpl.clientSocket);

                    STAFString error = 
                        STAFString("Error getting client SSL object: ") +
                        STAFString(ERR_error_string(ERR_get_error(), NULL));
             
                    if (errorBuffer) *errorBuffer = error.adoptImpl(); 
 
                    return kSTAFCommunicationError;
                }
 
                SSL_set_fd(connImpl.ssl, connImpl.clientSocket);
                sslDebug = 2;  // Before SSL_connect()

                int ssl_err = STAF_SSL_connect(
                    connImpl.clientSocket, connImpl.ssl, connectTimeout);

                if (ssl_err < 0)
                {
                    STAFSocketClose(connImpl.clientSocket);
                    SSL_free(connImpl.ssl);

                    STAFString error;

                    if (ssl_err == -2)
                    {
                        // SSL_connect timed out
                        error = STAFString("Client SSL handshake timed out");
                    }
                    else
                    {
                        error = STAFString(
                            "Error in client SSL handshake.  A possible "
                            "cause is an invalid interface/port combination "
                            "in the endpoint (e.g. a secure tcp interface, "
                            "but a port for a non-secure interface)");
                    }
             
                    if (errorBuffer) *errorBuffer = error.adoptImpl();
 
                    return kSTAFCommunicationError;
                }

                sslDebug = 3;  // Before SSL_get_peer_certificate()
                X509 *cert;
 
                if ((cert = SSL_get_peer_certificate(connImpl.ssl)) == NULL)
                {
                    STAFSocketClose(connImpl.clientSocket);
                    SSL_free(connImpl.ssl);

                    STAFString error = 
                        STAFString("Error in getting server certificate: ") +
                        STAFString(ERR_error_string(ERR_get_error(), NULL));
             
                    if (errorBuffer) *errorBuffer = error.adoptImpl();
 
                    return kSTAFCommunicationError;
                }

                /* Uncomment below once we can enforce all certificates 
                   use long hostname in the common name field.
            
                // To deal with man-in-the-middle attack, server certificate needs
                // to use hostname in the common name field when requesting 
                // CA's signature
         
                if (VerifyCertificateHostname(cert, host))
                {
                    STAFSocketClose(connImpl.clientSocket);
                    SSL_free(connImpl.ssl);
                    
                    STAFString error = 
                        STAFString("Certificate doesn't belong to server: ") +
                        STAFString(ERR_error_string(ERR_get_error(), NULL));
             
                    if (errorBuffer) *errorBuffer = error.adoptImpl();
 
                    return kSTAFCommunicationError;
                }
                */ 
 
                sslDebug = 4; // Before X509_free()
                X509_free(cert);  
            }
            catch (...)
            {
                try
                {
                    // Close client socket so request won't hang
                    STAFSocketClose(connImpl.clientSocket);

                    if (connImpl.ssl != NULL)
                        SSL_free(connImpl.ssl);
                }
                catch (...)
                { /* Do nothing */ }

                STAFString debugMsg = STAFString("SSL_new");

                if (sslDebug == 1)
                    debugMsg = STAFString("SSL_set_fd");
                else if (sslDebug == 2)
                    debugMsg = STAFString("SSL_connect");
                else if (sslDebug == 3)
                    debugMsg = STAFString("SSL_get_peer_certificate");
                else if (sslDebug == 4)
                    debugMsg = STAFString("X509_free");

                STAFString error = 
                    STAFString("Caught unknown exception using SSL, ") + 
                    debugMsg + "()";

                if (errorBuffer) *errorBuffer = error.adoptImpl(); 

                return kSTAFCommunicationError;
            }
        }

        connImpl.secure = provider->secure;
#endif
        // Set the socket blocking mode to blocking

        modeRC = STAFSocketSetBlockingMode(connImpl.clientSocket,
                                           kSTAFSocketBlocking, errorBuffer);
        if (modeRC != 0)
        {
            STAFString error = STAFString(
                "Error setting socket to blocking mode:");

            if (errorBuffer)
            {
                error += STAFString(*errorBuffer, STAFString::kShallow);
                *errorBuffer = error.adoptImpl();
            }

            STAFSocketClose(connImpl.clientSocket);

            return kSTAFCommunicationError;
        }

        *connection = new STAFTCPConnectionImpl(connImpl);

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        if (errorBuffer)
        {
            *errorBuffer = getExceptionString(
                e, "STAFTCPConnProvider.cpp").adoptImpl();
        }
    }
    catch (...)
    {
        if (errorBuffer)
        {
            STAFString error(
                "STAFTCPConnProvider.cpp: Caught unknown exception");
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
    STAFStringConst_t *logicalID, STAFStringConst_t *physicalID,
    STAFString_t *errorBuffer)
{
    if (baseProvider   == 0) return kSTAFInvalidObject;
    if (logicalID  == 0) return kSTAFInvalidParm;
    if (physicalID == 0) return kSTAFInvalidParm;

    try
    {
        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

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
        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

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
        STAFTCPConnectionProviderImpl *provider =
            static_cast<STAFTCPConnectionProviderImpl *>(baseProvider);

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


int STAFRead(int socket, char *buffer, int len, bool isSecure,
             bool doTimeout, int timeout, void *sslst = NULL)
{
    if (doTimeout)
    {
        fd_set fds;
        int n;
        struct timeval tv;

        // Set up the file descriptor set
        FD_ZERO(&fds);
        FD_SET(socket, &fds);

        // set up the struct timeval for the timeout
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        // Wait until timeout or data received
        n = select(socket + 1, &fds, NULL, NULL, &tv);

        if (n == 0)
            return -2; // timeout
        else if (n < 0)
            return n;  // error (-1)
    }

    if (isSecure)
    {
        int rc = 0;

#ifdef STAF_USE_SSL
        ssl_st *ssl = reinterpret_cast<ssl_st *>(sslst);
        rc = SSL_read(ssl, buffer, len);
#endif
        return rc;
    }
    else
    {
        return recv(socket, buffer, len, STAF_MSG_NOSIGNAL);
    }
}


STAFRC_t STAFConnectionRead(STAFConnection_t baseConnection, void *buffer,
                            unsigned int readLength,
                            STAFString_t *errorBuffer,
                            bool doTimeout)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if ((buffer == 0) && (readLength != 0)) return kSTAFInvalidParm;

    bool isSecure = false;

    try
    {
        STAFTCPConnectionImpl *connection =
            static_cast<STAFTCPConnectionImpl *>(baseConnection);

        int rc = 0;

        for(unsigned current = 0; current < readLength; current += rc)
        {
            int recvSize = (int)STAF_MIN((size_t)(readLength - current), 
                                         sizeof(connection->buffer));

#ifdef STAF_USE_SSL
            isSecure = connection->secure.isEqualTo(sYes,
                                                    kSTAFStringCaseInsensitive);
#endif

            if (isSecure)
            {
#ifdef STAF_USE_SSL
                // DATA EXCHANGE - Receive message and send reply.

                rc = STAFRead(connection->clientSocket,
                              connection->buffer,
                              recvSize,
                              isSecure,
                              doTimeout,
                              connection->readWriteTimeout,
                              connection->ssl);

                if (rc < 0)
                {
                    STAFString error;

                    if (rc == -2) // Timeout
                    {
                        error = STAFString(
                            "select() timeout: SSL_read() RC=") +
                            STAFSocketGetLastError() +
                            STAFString(" SSL error: ") +
                            STAFString(ERR_error_string(ERR_get_error(),
                            NULL));
                    }
                    else
                    {
                        error = STAFString(
                            "Error reading from socket: SSL_read() RC=") +
                            STAFSocketGetLastError() +
                            STAFString(" SSL error: ") +
                            STAFString(ERR_error_string(ERR_get_error(),
                            NULL));
                    }

                    if (errorBuffer) *errorBuffer = error.adoptImpl();
                    return kSTAFCommunicationError;
                }
#endif
            }
            else
            {
                do
                {
                    rc = STAFRead(connection->clientSocket,
                                  connection->buffer,
                                  recvSize,
                                  isSecure,
                                  doTimeout,
                                  connection->readWriteTimeout);
                }
                while ((rc < 0) && (STAFSocketGetLastError() == SOCEINTR));

                if (rc < 0)
                {
                    STAFString error;

                    if (rc == -2) // Timeout
                    {
                        error = STAFString(
                            "select() timeout: recv() RC=") +
                            STAFSocketGetLastError();
                    }
                    else
                    {
                        error = STAFString(
                            "Error reading from socket: recv() RC=") +
                            STAFSocketGetLastError();
                    }

                    if (errorBuffer) *errorBuffer = error.adoptImpl();
                    return kSTAFCommunicationError;
                }
            }

            if (rc == 0)
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


int STAFWrite(int socket, char *buffer, int len, bool isSecure,
              bool doTimeout, int timeout, void *sslst = NULL)
{
    if (doTimeout)
    {
        fd_set fds;
        int n;
        struct timeval tv;

        // Set up the file descriptor set
        FD_ZERO(&fds);
        FD_SET(socket, &fds);

        // set up the struct timeval for the timeout
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        // Wait until timeout or data received
        n = select(socket + 1, NULL, &fds, NULL, &tv);

        if (n == 0)
            return -2; // timeout
        else if (n < 0)
            return n;  // error (-1)
    }

    if (isSecure)
    {
        int rc = 0;

#ifdef STAF_USE_SSL
        ssl_st *ssl = reinterpret_cast<ssl_st *>(sslst);
        rc = SSL_write(ssl, buffer, len);
#endif
        return rc;
    }
    else
    {
        return send(socket, buffer, len, 0);
    }
}


STAFRC_t STAFConnectionWrite(STAFConnection_t baseConnection, void *buffer,
                             unsigned int writeLength,
                             STAFString_t *errorBuffer, bool doTimeout)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if ((buffer == 0) && (writeLength != 0)) return kSTAFInvalidParm;

    bool isSecure = false;

    try
    {
        STAFTCPConnectionImpl *connection =
            static_cast<STAFTCPConnectionImpl *>(baseConnection);

        int rc = 0;

        for(unsigned int current = 0; current < writeLength; current += rc)
        {
            int sendSize = (int)STAF_MIN((size_t)(writeLength - current), 
                                         sizeof(connection->buffer));
            memcpy(connection->buffer, (char *)buffer + current, sendSize);

#ifdef STAF_USE_SSL
            isSecure = connection->secure.isEqualTo(sYes,
                                                    kSTAFStringCaseInsensitive);
#endif

            if (isSecure)
            {
#ifdef STAF_USE_SSL
                // DATA EXCHANGE - Receive message and send reply.

                rc = STAFWrite(connection->clientSocket,
                               connection->buffer,
                               sendSize,
                               isSecure,
                               doTimeout,
                               connection->readWriteTimeout,
                               connection->ssl);

                if (rc < 0)
                {
                    STAFString error;

                    if (rc == -2) // Timeout
                    {
                        error = STAFString(
                            "select() timeout: SSL_write() RC=") +
                            STAFSocketGetLastError() +
                            STAFString(" SSL error: ") +
                            STAFString(ERR_error_string(ERR_get_error(),
                            NULL));
                    }
                    else
                    {
                        error = STAFString(
                            "Error writing to SSL: SSL_write() RC=") +
                            STAFSocketGetLastError() +
                            STAFString(" SSL error: ") +
                            STAFString(ERR_error_string(ERR_get_error(), NULL));
                    }

                    if (errorBuffer) *errorBuffer = error.adoptImpl();
                    return kSTAFCommunicationError;
                }
#endif
            }
            else
            {
                do
                {
                    rc = STAFWrite(connection->clientSocket,
                                  connection->buffer,
                                  sendSize,
                                  isSecure,
                                  doTimeout,
                                  connection->readWriteTimeout);
                }
                while ((rc < 0) && (STAFSocketGetLastError() == SOCEINTR));

                if (rc < 0)
                {
                    STAFString error;

                    if (rc == -2) // Timeout
                    {
                        error = STAFString(
                            "select() timeout: send() RC=") +
                            STAFSocketGetLastError();
                    }
                    else
                    {
                        error = STAFString(
                            "Error writing to socket: send() RC=") +
                            STAFSocketGetLastError();
                    }

                    if (errorBuffer) *errorBuffer = error.adoptImpl();
                    return kSTAFCommunicationError;
                }
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


STAFRC_t STAFConnectionGetPeerNetworkIDs(STAFConnection_t baseConnection,
                                         STAFStringConst_t *logicalID,
                                         STAFStringConst_t *physicalID,
                                         STAFString_t *errorBuffer)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if (logicalID  == 0) return kSTAFInvalidParm;
    if (physicalID == 0) return kSTAFInvalidParm;

    try
    {
        STAFTCPConnectionImpl *connection =
            static_cast<STAFTCPConnectionImpl *>(baseConnection);

        // XXX: May need to have this routine return STAFString_t instead of
        //      STAFStringConst_t in case they try to use the string after the
        //      connection goes away.

        *logicalID = connection->logicalNetworkID.getImpl();
        *physicalID = connection->physicalNetworkID.getImpl();

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
        STAFTCPConnectionImpl *connection =
            static_cast<STAFTCPConnectionImpl *>(*baseConnection);

        STAFSocketClose(connection->clientSocket);

#ifdef STAF_USE_SSL
        // Clean up.
        if (connection->secure.isEqualTo(sYes, kSTAFStringCaseInsensitive))
            SSL_free(connection->ssl);
#endif

        delete connection;
        connection = 0;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionDestruct");

    return kSTAFUnknownError;
}


