/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_OSTypes
#define STAF_OSTypes

/* To avoid namespace pollution it might be best to have
 * #define _POSIX_C_SOURCE
 * before all of the headers.  This will also help reduce future portability
 * issues as it ensures that all the POSIX/ANSI types and functions are
 * defined, and only the POSIX/ANSI types and functions are defined (for those
 * POSIX/ANSI headers).  Should not affect non-POSIX headers in the slightest.
 *
 * pwilloug@uk.ibm.com
 * */
 
#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

// Include stdlib.h instead of cstdlib so this header file can be included
// in a STAF service compiled using C or C++.
#include <stdlib.h>

#include <strings.h>

/* TCP/IP comms headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* POSIX Thread support header */
#include <pthread.h>

#define STAF_OS_TYPE_UNIX

#define SYSLINK

#ifdef STAF_OS_NAME_UNIXWARE
  #define STAF_FORK fork1
  #define STAF_Config_NoSTDIOStreamSupport
  #define STAF_Config_NoSTDFStreamSupport
#else
  #define STAF_FORK fork
#endif

typedef int STAFProcessID_t;
typedef int STAFProcessHandle_t;
typedef uid_t STAFUserID_t;
typedef gid_t STAFGroupID_t;

/* Setup Socket data */
typedef int STAFSocket_t;

#if defined(STAF_OS_NAME_HPUX)
     typedef int STAFSocketLen_t;
#else
     typedef socklen_t STAFSocketLen_t;
#endif

#define SOCEINPROGRESS EINPROGRESS
#define SOCEWOULDBLOCK EWOULDBLOCK
#define SOCEINTR EINTR
#define SOCKET_ERROR -1

#ifdef STAF_OS_NAME_LINUX
  #ifndef MSG_NOSIGNAL
    #define STAF_MSG_NOSIGNAL 0x4000
  #else
    #define STAF_MSG_NOSIGNAL MSG_NOSIGNAL
  #endif
#else
  #define STAF_MSG_NOSIGNAL 0
#endif


/* Setup Thread types */

typedef pthread_t STAFThreadID_t;
typedef int STAFThreadSafeScalar_t;

/* Define 64-bit numeric types */

#if defined(STAF_OS_NAME_AIX) || defined(STAF_OS_NAME_SOLARIS) || defined(STAF_OS_NAME_HPUX)
    typedef uint64_t STAFUInt64_t;
    typedef int64_t STAFInt64_t;
// #define UINT64_MAX_LESS_FIRST_DIGIT 8446744073709551615U
#else
    // Other Unix's such as Linux, z/OS, Mac, FreeBSD
    typedef unsigned long long STAFUInt64_t;
    typedef long long STAFInt64_t;
// #define UINT64_MAX 18446744073709551615ULL
// #define UINT64_MAX_LESS_FIRST_DIGIT 8446744073709551615ULL
#endif

#ifdef STAF_OS_NAME_ZOS
#include "STAF_iostream.h"

inline ostream &operator<<(ostream &theStream, const STAFThreadID_t theThread)
{
    unsigned long *ptid = reinterpret_cast<unsigned long *>(
                          const_cast<char *>(&theThread.__[4]));
    return theStream << *ptid;
}

inline operator<(const STAFThreadID_t lhs, const STAFThreadID_t rhs)
{
    unsigned long *ltid = reinterpret_cast<unsigned long*>(
                          const_cast<char *>(&lhs.__[4]));
    unsigned long *rtid = reinterpret_cast<unsigned long*>(
                          const_cast<char *>(&rhs.__[4]));
    return (*ltid < *rtid);
}

/* On z/OS the pthread_xxx() calls don't return the error code like other */
/* unices.  Instead, they return non-zero and set errno.  These macros    */
/* make the z/OS pthread_xxx() calls act like the other unices.           */

#define pthread_create_staf_zos(thread, attr, func, data) \
(pthread_create(thread, attr, func, data) == 0 ? 0 : errno)
#define pthread_create(thread, attr, func, data) \
pthread_create_staf_zos(thread, attr, func, data)

#define pthread_attr_init_staf_zos(attr) \
(pthread_attr_init(attr) == 0 ? 0 : errno)
#define pthread_attr_init(attr) pthread_attr_init_staf_zos(attr)

#define pthread_attr_setweight_np_staf_zos(attr, value) \
(pthread_attr_setweight_np(attr, value) == 0 ? 0 : errno)
#define pthread_attr_setweight_np(attr, value) \
pthread_attr_setweight_np_staf_zos(attr, value)

#define pthread_attr_setsynctype_np_staf_zos(attr, value) \
(pthread_attr_setsynctype_np(attr, value) == 0 ? 0 : errno)
#define pthread_attr_setsynctype_np(attr, value) \
pthread_attr_setsynctype_np_staf_zos(attr, value)

#define pthread_mutex_init_staf_zos(mutex, attr) \
(pthread_mutex_init(mutex, attr) == 0 ? 0 : errno)
#define pthread_mutex_init(mutex, attr) pthread_mutex_init_staf_zos(mutex, attr)

#define pthread_mutex_lock_staf_zos(mutex) \
(pthread_mutex_lock(mutex) == 0 ? 0 : errno)
#define pthread_mutex_lock(mutex) pthread_mutex_lock_staf_zos(mutex)

#define pthread_mutex_unlock_staf_zos(mutex) \
(pthread_mutex_unlock(mutex) == 0 ? 0 : errno)
#define pthread_mutex_unlock(mutex) pthread_mutex_unlock_staf_zos(mutex)

#define pthread_mutex_destroy_staf_zos(mutex) \
(pthread_mutex_destroy(mutex) == 0 ? 0 : errno)
#define pthread_mutex_destroy(mutex) pthread_mutex_destroy_staf_zos(mutex)

#define pthread_cond_init_staf_zos(cond, attr) \
(pthread_cond_init(cond, attr) == 0 ? 0 : errno)
#define pthread_cond_init(cond, attr) pthread_cond_init_staf_zos(cond, attr)

#define pthread_cond_signal_staf_zos(cond) \
(pthread_cond_signal(cond) == 0 ? 0 : errno)
#define pthread_cond_signal(cond) pthread_cond_signal_staf_zos(cond)

#define pthread_cond_broadcast_staf_zos(cond) \
(pthread_cond_broadcast(cond) == 0 ? 0 : errno)
#define pthread_cond_broadcast(cond) pthread_cond_broadcast_staf_zos(cond)

#define pthread_cond_timedwait_staf_zos(cond, mutex, timespec) \
(pthread_cond_timedwait(cond, mutex, timespec) == 0 ? 0 : errno)
#define pthread_cond_timedwait(cond, mutex, timespec) \
pthread_cond_timedwait_staf_zos(cond, mutex, timespec)

#define pthread_cond_destroy_staf_zos(cond) \
(pthread_cond_destroy(cond) == 0 ? 0 : errno)
#define pthread_cond_destroy(cond) pthread_cond_destroy_staf_zos(cond)

#endif

#endif
