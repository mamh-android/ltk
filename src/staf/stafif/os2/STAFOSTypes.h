/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_OSTypes
#define STAF_OSTypes

/* Define the STAF Operating System flag */

#define STAF_OS_TYPE_OS2

/* Include Base Operating System types */

#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSNLS
#define INCL_DOSNMPIPES
#define INCL_DOSQUEUES
#define INCL_DOSSESMGR
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#include <os2.h>

/* Include Socket types */

#define OS2
#define FD_SETSIZE 256
#include <sys\types.h>
#include <sys\socket.h>
#include <sys\ioctl.h>
#include <sys\select.h>
#include <sys\time.h>
#include <netdb.h>
#include <netinet\in.h>
#include <errno.h>

typedef int STAFSocket_t;
#define SOCEINPROGRESS EINPROGRESS
#define SOCEWOULDBLOCK EWOULDBLOCK
#define SOCEINTR EINTR
#define STAF_MSG_NOSIGNAL 0

/* Define necessary types */

// XXX: typedef HANDLE STAFEventSemHandle;
typedef PID STAFProcessID;
typedef ULONG STAFProcessHandle;

typedef unsigned int STAFThreadID_t;
typedef void STAFThreadFunctionReturn_t;
typedef void *STAFThreadFunctionData_t;
typedef int STAFThreadSafeScalar_t;

#define RETURN_FROM_STAF_THREAD(retcode)


/* Include process definitions */
#include <sys\process.h>

/* XXX: Include set_terminate() */
// #include <terminat.h>

/* XXX: For pgcc, can't use _cdecl #define SYSLINK _cdecl */
#define SYSLINK 

#endif
