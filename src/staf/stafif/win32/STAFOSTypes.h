/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_OSTypes
#define STAF_OSTypes

/* Disable "identifier truncated" message from VC++ */
 
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif
 
/* Define the STAF Operating System flag */

#define STAF_OS_TYPE_WIN32
#define STAF_CONFIG_NoSTDSetTerminate

// XXX: Define this flag to ask windows.h not to include additional headers

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/* Include Base Operating System types */

#include <windows.h>
#include <winbase.h>
#include <winnls.h>

/* The Platform SDK shipped with MSVC++ V6 didn't define this, but it is in */
/* later Platform SDKs, such as needed to build IA-64 support.              */

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

/* Include Socket types */
#ifdef STAF_USE_IPV6
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <winsock.h>
#endif 
#define SOCEINPROGRESS WSAEINPROGRESS
#define SOCEWOULDBLOCK WSAEWOULDBLOCK
#define SOCEINTR WSAEINTR
#define STAF_MSG_NOSIGNAL 0
typedef SOCKET STAFSocket_t;
typedef int STAFSocketLen_t;


/* Define necessary types */

typedef DWORD STAFProcessID_t;
typedef HANDLE STAFProcessHandle_t;
typedef HANDLE STAFUserID_t;
typedef unsigned int STAFThreadID_t;
typedef long STAFThreadSafeScalar_t;
// #define UINT64_MAX 18446744073709551615
// #define UINT64_MAX_LESS_FIRST_DIGIT 8446744073709551615

/* Define 64-bit numeric types */

typedef unsigned __int64 STAFUInt64_t;
typedef __int64 STAFInt64_t;

/* Include process definitions */
#include <process.h>

#define SYSLINK __stdcall

#endif
