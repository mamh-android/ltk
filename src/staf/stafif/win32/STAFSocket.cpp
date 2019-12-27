/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFSocket.h"
#include "STAF_iostream.h"
#include "STAFMutexSem.h"


unsigned int STAFSocketIsValidSocket(STAFSocket_t theSocket)
{
    return (theSocket != INVALID_SOCKET) ? 0 : 1;
}


STAFRC_t STAFSocketInit(STAFString_t *errorBuffer)
{
    static STAFMutexSem sem;
    static bool isInited = false;

    if (isInited) return kSTAFOk;

    STAFMutexSemLock lock(sem);

    if (isInited) return kSTAFOk;

    WORD version = MAKEWORD(1, 1);
    WSADATA wsaData = { 0 };

    int rc = WSAStartup(version, &wsaData);

    if (rc != 0)
    {
        STAFString error = STAFString("Error initializing WSA socket layer: "
                                      "WSAStartup() RC=") + rc;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    if (wsaData.wVersion != version)
    {
        STAFString error = STAFString("Error initializing WSA socket layer: "
                                      "Version mismatch, expected=") +
                           STAFString(version) + STAFString(", actual=") +
                           STAFString(wsaData.wVersion);
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    return kSTAFOk;
}


STAFRC_t STAFSocketClose(STAFSocket_t theSocket)
{
    STAFSocketInit(0);

    // XXX: We linger to ensure that the other side has a chance to read all the
    //      data.  In theory, we should make sure our protocol handles this
    //      situation, instead of having to rely on lingering.

    linger lingerData = { 1, 30 };

    setsockopt(theSocket, SOL_SOCKET, SO_LINGER, (char *)&lingerData,
               sizeof(lingerData));

    return (closesocket(theSocket) == 0) ? kSTAFOk : kSTAFCommunicationError;
}


int STAFSocketGetLastError()
{
    return WSAGetLastError();
}

STAFRC_t STAFSocketGetMyHostInfo(STAFString_t *hostname,
                                 STAFString_t *ipaddr,
                                 STAFString_t *errorBuffer)
{
    if (hostname == 0) return kSTAFInvalidParm;
    if (ipaddr == 0) return kSTAFInvalidParm;
    
    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

#ifdef STAF_USE_IPV6
 
    char buffer1[NI_MAXHOST] = { 0 };

    if (gethostname(buffer1, sizeof(buffer1)) == SOCKET_ERROR)
    {
        STAFString error = STAFString("Error getting hostname: gethostname()"
                                      "RC=") + errno;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    STAFString hostname1;
    
    hostname1 = STAFString(buffer1);

    struct addrinfo hints = { 0 };
    hints.ai_flags = 0; // default
    hints.ai_family = PF_UNSPEC;  // Any family is accepted
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0; // Any protocol is accepted

    struct addrinfo *result, *ressave;

    rc = getaddrinfo(buffer1, "6500", &hints, &result);

    if (rc != 0)
    {
        STAFString error = STAFString("Error getting address info: ") + buffer1;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    ressave = result;
        
    while (result)
    {
        if (result->ai_family == PF_INET6)
            break;
        
        if (result->ai_family == PF_INET)
            break;

        result = result->ai_next;
    }

    if (!result)
    {
        freeaddrinfo(ressave);

        STAFString error = STAFString("Error getting address info: no valid family");
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    // Get the hostname

    char buffer2[NI_MAXHOST] = { 0 };

    rc = getnameinfo(result->ai_addr, result->ai_addrlen, 
                         buffer2, sizeof(buffer2), 
                         NULL, 0, NI_NAMEREQD);

 
    if (rc != 0)
    {
        freeaddrinfo(ressave);

        STAFString error = STAFString("Error getting hostname: "
                                      "getnameinfo() RC=") + rc;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    STAFString hostname2;
    
    hostname2 = STAFString(buffer2);
 
    // Get the IP address

    STAFString_t ipAddress = 0;
    STAFString_t errorBuffer2 = 0;

    rc = STAFIPv6SocketGetPrintableAddressFromInAddr(
        result->ai_addr, result->ai_addrlen, &ipAddress, &errorBuffer2);

    freeaddrinfo(ressave);

    if (rc != kSTAFOk)
    {
        STAFString error(
            "Error getting printable IP address, "
            "STAFIPv6SocketGetPrintableAddressFromInAddr(), RC: " +
            STAFString(rc) + ", Info: " +
            STAFString(errorBuffer2, STAFString::kShallow));

        if (errorBuffer) *errorBuffer = error.adoptImpl();

        errorBuffer2 = 0;
        *ipaddr = STAFString("0.0.0.0", STAFString::kShallow).adoptImpl();

        return kSTAFCommunicationError;
    }
    
    *ipaddr = ipAddress;
    
#else // IPv4

    char buffer[256] = { 0 };

    if (gethostname(buffer, sizeof(buffer)) == SOCKET_ERROR)
    {
        STAFString error = STAFString("Error getting hostname: gethostname() "
                                      "RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
    
    struct hostent *host = gethostbyname(buffer);

    if (host == 0)
    {
        int errNum = WSAGetLastError();
        STAFString error = "";

        if (errNum == WSAHOST_NOT_FOUND)
        {
            error = STAFString("Unknown host name: ") + buffer +
                ", gethostbyname()";
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") + buffer +
                ", gethostbyname() RC=" + errNum;
        }

        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
    
    // Get the IP address

    STAFString_t ipAddress = 0;
    STAFString_t errorBuffer2 = 0;

    rc = STAFSocketGetPrintableAddressFromInAddr(
        (in_addr *)host->h_addr, &ipAddress, &errorBuffer2);

    if (rc != kSTAFOk)
    {
        STAFString error(
            "Error getting printable IP address, "
            "STAFSocketGetPrintableAddressAFromInAddr(), RC: " +
            STAFString(rc) + ", Info: " +
            STAFString(errorBuffer2, STAFString::kShallow));

        if (errorBuffer) *errorBuffer = error.adoptImpl();

        errorBuffer2 = 0;
        *ipaddr = STAFString("0.0.0.0", STAFString::kShallow).adoptImpl();

        return kSTAFCommunicationError;
    }
    
    *ipaddr = ipAddress;
    
    // Get the hostname

    STAFString hostname1(host->h_name);

    if (hostname1.count(kUTF8_PERIOD) > 2)
    {
        *hostname = hostname1.adoptImpl();
        return kSTAFOk;
    }

    int addr = *(int *)host->h_addr;

    host = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);

    if (host == 0)
    {
        STAFString error = STAFString("Error getting hostent structure: "
                                      "gethostbyaddr() RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    STAFString hostname2(host->h_name);
    
#endif

    if (hostname2.count(kUTF8_PERIOD) > 2)
    {
        *hostname = hostname2.adoptImpl();
        return kSTAFOk;
    }

    if (hostname1.length() > hostname2.length())
        *hostname = hostname1.adoptImpl();
    else
        *hostname = hostname2.adoptImpl();

    return kSTAFOk;
}


STAFRC_t STAFSocketGetInAddrByName(STAFStringConst_t name, in_addr *addr,
                                   STAFString_t *errorBuffer)
{
    if (name == 0) return kSTAFInvalidParm;
    if (addr == 0) return kSTAFInvalidParm;

    STAFString theName = name;
    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

    struct hostent *hostname =
                    gethostbyname(theName.toCurrentCodePage()->buffer());

    if (hostname == 0)
    {
        int errNum = WSAGetLastError();
        STAFString error = "";

        if (errNum == WSAHOST_NOT_FOUND)
        {
            error = STAFString("Unknown host name: ") +
                theName.toCurrentCodePage()->buffer() + ", gethostbyname()";
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname() RC=" + errNum;
        }

        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    addr->s_addr = *reinterpret_cast<unsigned long *>(hostname->h_addr);

    return kSTAFOk;
}


STAFRC_t STAFIPv6SocketGetNameByInAddr(sockaddr *addr, int addrlen, 
                                       STAFString_t *name,
                                       STAFString_t *errorBuffer)
{
#ifdef STAF_USE_IPV6
    if (addr == 0) return kSTAFInvalidParm;
    if (name == 0) return kSTAFInvalidParm;

    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

    char theHost[1025] = "";

    rc = getnameinfo(addr, addrlen, theHost, sizeof(theHost), 
                     NULL, 0, NI_NAMEREQD);

    if (rc != 0)
    {
        STAFString error = STAFString("Error getting hostname: "
                                      "getnameinfo() RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    *name = STAFString(theHost).adoptImpl();

#endif

    return kSTAFOk;
}


STAFRC_t STAFSocketGetNameByInAddr(in_addr *addr, STAFString_t *name,
                                   STAFString_t *errorBuffer)
{
    if (addr == 0) return kSTAFInvalidParm;
    if (name == 0) return kSTAFInvalidParm;

    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

    struct hostent *hostname =
                    gethostbyaddr(reinterpret_cast<char *>(&addr->s_addr),
                                  sizeof(addr->s_addr), AF_INET);
    if (hostname == 0)
    {
        STAFString error = STAFString("Error getting hostent structure: "
                                      "gethostbyaddr() RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    *name = STAFString(hostname->h_name).adoptImpl();

    return kSTAFOk;
}

STAFRC_t STAFIPv6SocketGetPrintableAddressFromInAddr(struct sockaddr *addr, 
                                                     int addrlen,
                                                     STAFString_t *ipaddr,
                                                     STAFString_t *errorBuffer)
{
#ifdef STAF_USE_IPV6

    if (addr == 0)   return kSTAFInvalidParm;
    if (ipaddr == 0) return kSTAFInvalidParm;

    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

    char theAddr[256] = "";

    rc = getnameinfo(addr, addrlen, theAddr, sizeof(theAddr), 
                         NULL, 0, NI_NUMERICHOST);

    if (rc != 0)
    {
        STAFString error = STAFString("Error getting printable IP address: "
                                      "getnameinfo() RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    *ipaddr = STAFString(theAddr).adoptImpl();

#endif

    return kSTAFOk;
}


STAFRC_t STAFSocketGetPrintableAddressFromInAddr(in_addr *addr,
                                                 STAFString_t *ipaddr,
                                                 STAFString_t *errorBuffer)
{
    if (addr == 0)   return kSTAFInvalidParm;
    if (ipaddr == 0) return kSTAFInvalidParm;

    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

    char *theAddr = inet_ntoa(*addr);

    if (theAddr == 0)
    {
        STAFString error = STAFString("Error getting printable IP address: "
                                      "inet_ntoa() RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    *ipaddr = STAFString(theAddr).adoptImpl();

    return kSTAFOk;
}


STAFRC_t STAFSocketSetBlockingMode(STAFSocket_t theSocket,
                                   STAFSocketBlockingMode_t blockingMode,
                                   STAFString_t *errorBuffer)
{
    STAFRC_t rc = STAFSocketInit(errorBuffer);

    if (rc != kSTAFOk) return rc;

    u_long nonBlocking = (blockingMode == kSTAFSocketBlocking) ? 0 : 1;
    int ioctlRC = ioctlsocket(theSocket, FIONBIO, (u_long *)&nonBlocking);

    if (ioctlRC != 0)
    {
        STAFString error = STAFString("Error performing ioctl on socket: "
                                      "ioctlsocket() RC=") + WSAGetLastError();
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    return kSTAFOk;
}
