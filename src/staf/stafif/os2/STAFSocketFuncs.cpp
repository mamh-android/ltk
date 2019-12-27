/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAF_iostream.h"
#include "STAFSocketFuncs.h"
#include "STAFException.h"
#include "STAFString.h"

// From so32dll.dll

STAFSocket_t (* SYSLINK STAFSocketFuncs::accept)(STAFSocket_t, struct sockaddr *, int *) = 0;
int    (* SYSLINK STAFSocketFuncs::bind)(STAFSocket_t, struct sockaddr *, int) = 0;
int    (* SYSLINK STAFSocketFuncs::connect)(STAFSocket_t, struct sockaddr *, int) = 0;
int    (* SYSLINK STAFSocketFuncs::listen)(STAFSocket_t, int) = 0;
int    (* SYSLINK STAFSocketFuncs::recv)(STAFSocket_t, char *, int, int) = 0;
int    (* SYSLINK STAFSocketFuncs::send)(STAFSocket_t, char *, int, int) = 0;
int    (* SYSLINK STAFSocketFuncs::select)(int, fd_set *, fd_set *, fd_set *,
                                         struct timeval *) = 0;
int    (* SYSLINK STAFSocketFuncs::sock_errno)(void) = 0;
STAFSocket_t (* SYSLINK STAFSocketFuncs::socket)(int, int, int) = 0;
int    (* SYSLINK STAFSocketFuncs::soclose)(STAFSocket_t) = 0;

// From tcp32dll.dll

unsigned short   (* SYSLINK STAFSocketFuncs::bswap)(unsigned int short) = 0;
struct hostent * (* SYSLINK STAFSocketFuncs::gethostbyaddr)(const char *, int, int) = 0;
struct hostent * (* SYSLINK STAFSocketFuncs::gethostbyname)(const char *) = 0;
unsigned long    (* SYSLINK STAFSocketFuncs::inet_addr)(const char *) = 0;
unsigned long    (* SYSLINK STAFSocketFuncs::lswap)(unsigned long) = 0;

typedef int (* SYSLINK StartupFunc)(void);
static int (* SYSLINK hostid)(void) = 0;
static int (* SYSLINK ioctlSocket)(STAFSocket_t, int, caddr_t, int) = 0;

int STAFSocketFuncs::initSocketFuncs()
{
    int rc = 1;

    try
    {
        // Load funcs from so32dll.dll

        HMODULE lib = 0;
        char *error = 0;
        char so32DLL[] = "SO32DLL";
        APIRET rc = DosLoadModule((PSZ)error, sizeof(so32DLL), (PSZ)so32DLL, &lib);

        rc = DosQueryProcAddr(lib, 0, (PSZ)"accept", (PFN *)&accept);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"bind", (PFN *)&bind);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"connect", (PFN *)&connect);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"gethostid", (PFN *)&hostid);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"ioctl", (PFN *)&ioctlSocket);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"listen", (PFN *)&listen);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"recv", (PFN *)&recv);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"send", (PFN *)&send);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"bsdselect", (PFN *)&select);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"sock_errno", (PFN *)&sock_errno);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"socket", (PFN *)&socket);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"soclose", (PFN *)&soclose);

        StartupFunc sockStartup = 0;

        rc = DosQueryProcAddr(lib, 0, (PSZ)"sock_init", (PFN *)&sockStartup);

        // Load funcs from tcp32dll.dll

        char tcp32DLL[] = "TCP32DLL";
        rc = DosLoadModule((PSZ)error, sizeof(tcp32DLL), (PSZ)tcp32DLL, &lib);

        rc = DosQueryProcAddr(lib, 0, (PSZ)"bswap", (PFN *)&bswap);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"gethostbyaddr", (PFN *)&gethostbyaddr);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"gethostbyname", (PFN *)&gethostbyname);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"inet_addr", (PFN *)&inet_addr);
        rc = DosQueryProcAddr(lib, 0, (PSZ)"lswap", (PFN *)&lswap);

        // Now, initialize the socket functions

        rc = sockStartup();
    }
    catch (STAFException &e)
    {
        rc = 1;
        e.write();
    }
    catch (...)
    {
        rc = 1;
        cout << "Caught unknown exception" << endl;
    }

    return rc;
}


int STAFSocketFuncs::getRealHostName(STAFString &hostname)
{
    unsigned long hostaddr = STAFSocketFuncs::lswap(hostid());
    hostent *hostdata = STAFSocketFuncs::gethostbyaddr((char *)&hostaddr,
                                                       sizeof(hostaddr),
                                                       AF_INET);
    if (hostdata == 0)
    {
        cout << "Error on gethostbyaddr()" << endl;
        return 1;
    }

    hostname = STAFString(hostdata->h_name);

    return 0;
}


int STAFSocketFuncs::setBlockingMode(STAFSocket_t theSocket,
                                     STAFSocketFuncs::BlockingMode blockingMode)
{
    int nonBlocking = (blockingMode == kBlocking) ? 0 : 1;
    int ioctlRC = ioctlSocket(theSocket, FIONBIO, (char *)&nonBlocking,
                              sizeof(nonBlocking));
    if (ioctlRC < 0)
        return STAFSocketFuncs::sock_errno();

    return 0;
}

STAFRC_t STAFSocketFuncs::getInAddrByName(const char *name, in_addr *addr,
                                          unsigned int *osRC)
{
    // XXX: Need to make sure OS/2 has a per-thread gethostbyname() or if
    //      this needs to be changed to gethostbyname_r()

    struct hostent *hostname = gethostbyname(name);

    if (hostname == 0)
    {
        if (osRC) *osRC = STAFSocketFuncs::sock_errno();
        return kSTAFBaseOSError;
    }

    addr->s_addr = *reinterpret_cast<unsigned long *>(hostname->h_addr);

    return kSTAFOk;
}
