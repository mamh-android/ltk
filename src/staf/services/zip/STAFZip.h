/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIP
#define STAF_ZIP

#define MAXFILENAME (8192)
#define MAXMESSAGE (16384)
#define uLong unsigned long
#define uInt unsigned int
#define BUFREADCOMMENT (0x400)
#define Z_BUFSIZE (16384)


#ifdef STAF_OS_TYPE_WIN32
#define CASESENSITIVITYDEFAULTVALUE (0)
#endif

#ifdef STAF_OS_TYPE_UNIX
#define CASESENSITIVITYDEFAULTVALUE (1)
#endif


#ifdef __cplusplus

// On Windows, need to declare fseeki64 and ftelli64 functions since using
// using MS Visual Studio V6.0

#ifdef STAF_OS_TYPE_WIN32
extern "C" int __cdecl _fseeki64(FILE *, STAFInt64_t, int);
extern "C" STAFInt64_t __cdecl _ftelli64(FILE *);
#endif

extern "C"
{
#endif

typedef enum STAFZIPError_e
{
    // add service-specific return codes here

    kZIPGeneralZipError = 4001,
    kZIPNotEnoughMemory = 4002,
    kZIPChangeFileSizeError = 4003,
    kZIPErrorCreatingDir = 4004,
    kZIPInvalidZipFile = 4005,
    kZIPBadCRC = 4006,
    kZIPInvalidOwnerGroup = 4007,
    kZIPInvalidFileMode = 4008

} STAFZIPError_t;


#ifdef __cplusplus
}
#endif


#endif
