/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
 
#include "STAF.h"
#include <fcntl.h>
#include <arpa/nameser.h>
#include <resolv.h>

#ifdef STAF_OS_NAME_FREEBSD
#include <netdb.h>
#endif

#include "STAFSocket.h"
#include "STAFThreadManager.h"
#include "STAF_iostream.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST  1025
#endif

unsigned int STAFSocketIsValidSocket(STAFSocket_t theSocket)
{
    return (theSocket < 0) ? 1 : 0;
}


STAFRC_t STAFSocketInit(STAFString_t *errorBuffer)
{
    return kSTAFOk;
}


STAFRC_t STAFSocketClose(STAFSocket_t theSocket)
{
    return (close(theSocket) == 0) ? kSTAFOk : kSTAFCommunicationError;
}


int STAFSocketGetLastError()
{
    return errno;
}


STAFRC_t STAFSocketGetMyHostInfo(STAFString_t *hostname,
                                 STAFString_t *ipaddr,
                                 STAFString_t *errorBuffer)
{
    if (hostname == 0) return kSTAFInvalidParm;
    if (ipaddr == 0) return kSTAFInvalidParm;
 
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

    //if (hostname1.count(kUTF8_PERIOD) > 2)
    //{
    //    *hostname = hostname1.adoptImpl();
    //    return kSTAFOk;
    //}

    struct addrinfo hints = { 0 };
    hints.ai_flags = 0; // default
    hints.ai_family = PF_UNSPEC;  // Any family is accepted
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0; // Any protocol is accepted

    struct addrinfo *result, *ressave;

    int rc = getaddrinfo(buffer1, "6500", &hints, &result);

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
        STAFString error = STAFString("Error getting hostname: gethostname()"
                                      "RC=") + errno;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    struct hostent *host = gethostbyname(buffer);

    if (host == 0)
    {
        STAFString error = "";

        if (h_errno == HOST_NOT_FOUND)
        {
            error = STAFString("Unknown host name: ") + buffer +
                ", gethostbyname()";
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") +
                buffer + ", gethostbyname() RC=" + h_errno;
        }

        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    // Get the IP address
    
    STAFString_t ipAddress = 0;
    STAFString_t errorBuffer2 = 0;

    int rc = STAFSocketGetPrintableAddressFromInAddr(
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
                                      "gethostbyaddr() RC=") + h_errno;
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

#if defined(STAF_GETHOSTBYNAME_R_6PARM)
    struct hostent hostent_data = { 0 };
    struct hostent *hostname = &hostent_data;
    char hostdata_buff[2048] = { 0 };
    struct hostent *hostname2 = 0;
    int host_error = 0;

    int rc = gethostbyname_r(theName.toCurrentCodePage()->buffer(), hostname,
                             hostdata_buff, sizeof(hostdata_buff), &hostname2,
                             &host_error);

    if ((rc != 0) || (hostname == NULL) || (hostname2 == NULL))
    {
        STAFString error = "";

        if ((rc == EAGAIN) || ((rc == 0) && (host_error == TRY_AGAIN)))
        {
            // If get a "Try Again" error, either rc = EAGAIN (11) or
            // host_error = TRY_AGAIN (2), retry up to 20 times
            // calling gethostbyname_r() until successful.
            // Note that 20 retries was randomly selected, as well as
            // the 1 second delay.  With a 1 second delay between retries,
            // I've seen it take up to 4 retries to be successful.
            // We could make the max retries and/or delay configurable.

            int retryCount = 0;

            for (; retryCount < 20 &&
                  ((rc == EAGAIN) || ((rc == 0) && (host_error == TRY_AGAIN)));
                  retryCount++)
            {
                // Delay for 1 second (1000 ms) before retrying
                STAFThreadManager::sleepCurrentThread(1000);

                host_error = 0;
                rc = gethostbyname_r(
                    theName.toCurrentCodePage()->buffer(), hostname,
                    hostdata_buff, sizeof(hostdata_buff), &hostname2,
                    &host_error);
            }

            if ((rc != 0) || (hostname == NULL) || (hostname2 == NULL))
            {
                error = STAFString(
                    "Error getting hostent structure for host name: ") +
                    theName.toCurrentCodePage()->buffer() +
                    ", gethostbyname_r() ";

                if (rc != 0)
                    error = error + "rc=" + rc;
                else
                    error = error + "host_error=" + host_error;

                error = error + " after retrying " + retryCount + " times";
            }
        }
        else if (rc != 0)
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname_r() rc=" + rc;
        }
        else if (host_error == HOST_NOT_FOUND)
        {
            error = STAFString("Unknown host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname_r()";
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname_r() host_error=" + host_error;
        }

        if (error.length() != 0)
        {
            if (errorBuffer) *errorBuffer = error.adoptImpl();
            return kSTAFCommunicationError;
        }
    }
#elif defined(STAF_GETHOSTBYNAME_R_5PARM)
    struct hostent hostent_data = { 0 };
    struct hostent *hostname = &hostent_data;
    char hostdata_buff[2048] = { 0 };
    int host_error = 0;

    gethostbyname_r(theName.toCurrentCodePage()->buffer(), hostname,
                    hostdata_buff, sizeof(hostdata_buff), &host_error);

    if (host_error != 0)
    {
        STAFString error = "";

        if (host_error == HOST_NOT_FOUND)
        {
            error = STAFString("Unknown host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname_r()";
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname_r() RC=" + host_error;
        }
        
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
#elif defined(STAF_GETHOSTBYNAME_R_3PARM)
    struct hostent_data hostname_hostent_data = { 0 };
    struct hostent hostname_data = { 0 };
    struct hostent *hostname = &hostname_data;
    int rc = gethostbyname_r(theName.toCurrentCodePage()->buffer(), hostname,
                             &hostname_hostent_data);
    
    if (rc != 0)
    {
        STAFString error = STAFString(
            "Error getting hostent structure for host name: ") +
            theName.toCurrentCodePage()->buffer() +
            ", gethostbyname_r() RC=" + rc;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
#elif defined(STAF_NO_GETHOSTBYNAME_R)
    struct hostent *hostname =
                    gethostbyname(theName.toCurrentCodePage()->buffer());

    if (hostname == 0)
    {
        STAFString error = "";

        if (h_errno == HOST_NOT_FOUND)
        {
            error = STAFString("Unknown host name: ") +
                theName.toCurrentCodePage()->buffer() + ", gethostbyname()";
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure for host name: ") +
                theName.toCurrentCodePage()->buffer() +
                ", gethostbyname() RC=" + h_errno;
        }

        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
#else
#error Undefined format for thread-safe gethostbyname()
#endif

    addr->s_addr = *reinterpret_cast<unsigned int *>(hostname->h_addr);

    return kSTAFOk;
}


STAFRC_t STAFIPv6SocketGetNameByInAddr(sockaddr *addr, int addrlen,
                                   STAFString_t *name,
                                   STAFString_t *errorBuffer)
{
#ifdef STAF_USE_IPV6

    if (addr == 0) return kSTAFInvalidParm;
    if (name == 0) return kSTAFInvalidParm;

    char theHost[1025] = "";

    int rc = getnameinfo(addr, addrlen, theHost, sizeof(theHost), 
                         NULL, 0, NI_NAMEREQD);

    if (rc != 0)
    {
        STAFString error = STAFString("Error getting hostname: "
                                      "getnameinfo() RC=") + rc;
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

#if defined(STAF_GETHOSTBYADDR_R_8PARM)
    struct hostent hostent_data = { 0 };
    struct hostent *hostname = &hostent_data;
    char hostdata_buff[2048] = { 0 };
    struct hostent *hostname2 = 0;
    int host_error = 0;

    // int rc = gethostbyaddr_r(reinterpret_cast<char *>(&addr->s_addr),
    int rc = gethostbyaddr_r(&addr->s_addr,
                             sizeof(addr->s_addr), AF_INET,
                             hostname, hostdata_buff, sizeof(hostdata_buff),
                             &hostname2, &host_error);
    
    if ((host_error != 0) || (rc != 0) || (hostname2 == NULL))
    {
        STAFString error = "";

        if ((rc == EAGAIN) || (host_error == TRY_AGAIN))
        {
            // If get a "Try Again" error, either rc = EAGAIN (11) or
            // host_error = TRY_AGAIN (2), retry up to 20 times
            // calling gethostbyaddr_r() until successful.
            // We could make the max retries and/or delay configurable.

            int retryCount = 0;

            for (; retryCount < 20 && ((rc == EAGAIN) ||
                                       (host_error == TRY_AGAIN));
                  retryCount++)
            {
                // Delay for 1 second (1000 ms) before retrying
                STAFThreadManager::sleepCurrentThread(1000);

                host_error = 0;
                rc = gethostbyaddr_r(
                    &addr->s_addr, sizeof(addr->s_addr), AF_INET,
                    hostname, hostdata_buff, sizeof(hostdata_buff),
                    &hostname2, &host_error);
            }

            if ((host_error != 0) || (rc != 0))
            {
                error = STAFString(
                    "Error getting hostent structure: gethostbyaddr_r() ");

                if (host_error != 0)
                    error = error + "host_error=" + host_error;
                else
                    error = error + "rc=" + rc;

                error = error + " after retrying " + retryCount + " times";
            }
        }
        else
        {
            error = STAFString(
                "Error getting hostent structure: gethostbyaddr_r() ");

            if (host_error != 0)
                error = error + "host_error=" + host_error;
            else if (rc != 0)
                error = error + "rc=" + rc;
            else
                error = error + "returned NULL hostname";
        }

        if (error.length() != 0)
        {
            if (errorBuffer) *errorBuffer = error.adoptImpl();
            return kSTAFCommunicationError;
        }
    }
#elif defined(STAF_GETHOSTBYADDR_R_7PARM)
    struct hostent hostent_data = { 0 };
    struct hostent *hostname = &hostent_data;
    char hostdata_buff[2048] = { 0 };
    int host_error = 0;

    struct hostent *hostname2 = 
        gethostbyaddr_r(reinterpret_cast<char *>(&addr->s_addr),
                        sizeof(addr->s_addr), AF_INET, hostname, 
                        hostdata_buff, sizeof(hostdata_buff), &host_error);

    if (host_error != 0)
    {
        STAFString error = STAFString("Error getting hostent structure: "
                                      "gethostbyaddr_r() RC=") + host_error;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
#elif defined(STAF_GETHOSTBYADDR_R_5PARM)
    struct hostent_data hostname_hostent_data = { 0 };
    struct hostent hostname_data = { 0 };
    struct hostent *hostname = &hostname_data;

    int rc = gethostbyaddr_r(reinterpret_cast<char *>(&addr->s_addr),
                             sizeof(addr->s_addr), AF_INET,
                             hostname, &hostname_hostent_data);

    if (rc != 0)
    {
        STAFString error = STAFString("Error getting hostent structure: "
                                      "gethostbyaddr_r() RC=") + rc;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
#elif defined(STAF_NO_GETHOSTBYADDR_R)
    struct hostent *hostname = 
        gethostbyaddr(reinterpret_cast<char *>(&addr->s_addr), 
                      sizeof(addr->s_addr),
                      AF_INET);
                      
    if (hostname == 0)
    {
        STAFString error = STAFString("Error getting hostent structure: "
                                      "gethostbyaddr() RC=") + h_errno;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }
#else
#error Undefined format for thread-safe gethostbyaddr()
#endif

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
                                      "getnameinfo() RC=") + errno;
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

    char addrBuffer[32] = { 0 };
    const char *theAddr = inet_ntop(AF_INET, addr, addrBuffer,
                                    sizeof(addrBuffer));
    if (theAddr == 0)
    {
        STAFString error = STAFString("Error getting printable IP address: "
                                      "inet_ntop() RC=") + errno;
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
    int fd_flags = fcntl(theSocket, F_GETFL, 0);

    if (fd_flags == -1)
    {
        STAFString error = STAFString("Error getting file descriptor flags: "
                                      "fcntl() RC=") + errno;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    if (blockingMode == kSTAFSocketBlocking)
        fd_flags &= ~O_NONBLOCK;
    else
        fd_flags |= O_NONBLOCK;

    if (fcntl(theSocket, F_SETFL, fd_flags) == -1)
    {
        STAFString error = STAFString("Error setting socket flags: "
                                      "fcntl() RC=") + errno;
        if (errorBuffer) *errorBuffer = error.adoptImpl();
        return kSTAFCommunicationError;
    }

    return kSTAFOk;
}
