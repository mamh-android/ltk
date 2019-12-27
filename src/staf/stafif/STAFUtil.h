/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Util
#define STAF_Util

#include "STAF.h"
#include "STAFString.h"
#include "STAFProcess.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Define min and max macros.  We've had conflicts using raw min() and max() */

#define STAF_MIN(a,b) ((a) <= (b) ? (a) : (b))
#define STAF_MAX(a,b) ((a) >= (b) ? (a) : (b))

/* Defines a structure to receive configuration information */
struct STAFConfigInfo
{
    STAFStringConst_t bootDrive;
    STAFStringConst_t osName;
    STAFStringConst_t osMajorVersion;
    STAFStringConst_t osMinorVersion;
    STAFStringConst_t osRevision;
    STAFUInt64_t physicalMemory;
    STAFStringConst_t exePath;
    STAFStringConst_t lineSeparator;
    STAFStringConst_t fileSeparator;
    STAFStringConst_t pathSeparator;
    STAFStringConst_t commandSeparator;
    STAFProcessStopMethod_t defaultProcessStopMethod;
    STAFProcessConsoleMode_t defaultProcessConsoleMode;
    unsigned int envVarCaseSensitive; /* 0=CaseInsensitive; 1=CaseSensitive */
    unsigned int numProcessors;
    STAFStringConst_t currentDrive;
    STAFProcessID_t maxPid; // Maximum value for a pid (Process ID)
};


/*********************************************************************/
/* STAFUtilGetConfigInfo - Retrieves OS configuration information    */
/*                                                                   */
/* Accepts: (Out) A pointer to a STAFConfigInfo structure            */
/*          (Out) A pointer to an error string                       */
/*          (Out) A pointer to an operating system return code       */
/*                                                                   */
/* Returns:  0, if successful                                        */
/*          >0, if unsuccessful (*errorBuffer and *osRC will be set) */
/*********************************************************************/
unsigned int STAFUtilGetConfigInfo(struct STAFConfigInfo *configInfo,
                                   STAFString_t *errorBuffer,
                                   unsigned int *osRC);


/*********************************************************************/
/* STAFUtilGetCurrentProcessCodePage - Obtains a string representing */
/*                                     the current process' code     */
/*                                     page                          */
/*                                                                   */
/* Accepts: (In)  A pointer to the buffer to hold the code page      */
/*                string (this should be large enough to hold the    */
/*                largest codepage string for the operating system;  */
/*                typically 128 bytes should be sufficient)          */
/*                                                                   */
/* Returns:  The pointer passed in                                   */
/*********************************************************************/
char *STAFUtilGetCurrentProcessCodePage(char *buffer);


/*********************************************************************/
/* STAFUtilGetPID - Returns the effective Process ID (PID)           */
/*                                                                   */
/* Accepts: Nothing                                                  */
/*                                                                   */
/* Returns:  The Process' ID                                         */
/*                                                                   */
/* Notes  :  1) This routine should be used instead of getpid(), as  */
/*              getpid() behaves strangely in a multi-threaded       */
/*              environment on Linux                                 */
/*********************************************************************/
STAFProcessID_t STAFUtilGetPID();


/*********************************************************************/
/* STAFUtilGetSystemMemory - Obtains a piece of system memory        */
/*                                                                   */
/* Accepts: (In)  The size of the memory needed                      */
/*          (Out) Pointer to an operating system return code         */
/*                                                                   */
/* Returns:  0, if unsuccessful (*osRC will be set)                  */
/*          >0, if successful (this is the address of the memory)    */
/*********************************************************************/
void *STAFUtilGetSystemMemory(unsigned long size, unsigned int *osRC);


/*********************************************************************/
/* STAFUtilFreeSystemMemory - Frees a piece of system memory         */
/*                                                                   */
/* Accepts: (In)  The pointer to the system memory to free           */
/*                                                                   */
/* Returns:  Nothing                                                 */
/*********************************************************************/
void STAFUtilFreeSystemMemory(void *);


/*********************************************************************/
/* STAFUtilSwapUInt - Byte reverses an unsigned int.  In other       */
/*                    words, it converts from little-endian or       */
/*                    visa-versa.                                    */
/*                                                                   */
/* Accepts: (In)  The unsigned int to byte reverse                   */
/*                                                                   */
/* Returns:  The byte-reversed unsigned int                          */
/*********************************************************************/
unsigned int STAFUtilSwapUInt(unsigned int theUInt);


/*********************************************************************/
/* STAFUtilConvertNativeUIntToLE - Converts an unsigned in the       */
/*                                 platform's native format to       */
/*                                 little-endian format              */
/*                                                                   */
/* Accepts: (In)  The unsigned int to convert to little-endian       */
/*                                                                   */
/* Returns:  The little-endian version of the unsigned int           */
/*********************************************************************/
unsigned int STAFUtilConvertNativeUIntToLE(unsigned int theUInt);


/*********************************************************************/
/* STAFUtilConvertLEUIntToNative - Converts an unsigned in           */
/*                                 little-endian format to the       */
/*                                 platform's native format          */
/*                                                                   */
/* Accepts: (In)  The unsigned int to convert to little-endian       */
/*                                                                   */
/* Returns:  The little-endian version of the unsigned int           */
/*********************************************************************/
unsigned int STAFUtilConvertLEUIntToNative(unsigned int theUInt);


/*********************************************************************/
/* STAFUtilIsValidSocket - Determines if a socket descriptor,        */
/*                         received from socket() or accept() is     */
/*                         valid                                     */
/*                                                                   */
/* Accepts: (In)  The socket to check                                */
/*                                                                   */
/* Returns:  0, if the socket is invalid                             */
/*          >0, if the socket is valid                               */
/*********************************************************************/
unsigned int STAFUtilIsValidSocket(STAFSocket_t theSocket);


/*********************************************************************/
/* STAFUtilGetNonInheritableSocket - Provides a new socket that is   */
/*   a identical to the old socket except the new socket cannot be   */
/*   inherited by new processes.                                     */
/*                                                                   */
/* Accepts: (In)  The socket to duplicate                            */
/*          (Out) The new socket (with inheritHandle set to FALSE)   */
/*          (Out) A pointer to an operating system return code       */
/*                                                                   */
/* Returns:  0, if successful                                        */
/*          >0, if unsuccessful (*osRC will be set)                  */
/*********************************************************************/
unsigned int STAFUtilGetNonInheritableSocket(STAFSocket_t oldSocket,
                                             STAFSocket_t *newSocket,
                                             unsigned int *osRC);


/*********************************************************************/
/* STAFUtilFormatString - Generates a string based on a format       */
/*                        string, ala printf().  This is generally   */
/*                        used to format STAF request strings.       */
/*                                                                   */
/* Accepts: (In)  The format string                                  */
/*          (Out) A pointer to the output string                     */
/*          (In)  All data indicated in the format string            */
/*                                                                   */
/* Returns:  Standard return codes                                   */
/*                                                                   */
/* Notes  :  1) The caller is responsible for destructing the        */
/*              output string                                        */
/*********************************************************************/
/* Valid format string specifiers:                                   */
/*                                                                   */
/* %d - an unsigned integer                                          */
/* %s - a STAFString_t                                               */
/* %C - a STAFString_t which will be formatted in colon-length-colon */
/*      delimited format                                             */
/* %% - a percent sign                                               */
/*                                                                   */
/* Any other %<char> is simply ignored (and not copied)              */
/*********************************************************************/
unsigned int STAFUtilFormatString(STAFStringConst_t formatString,
                                  STAFString_t *outputString, ...);


/*********************************************************************/
/* STAFUtilFormatString2 - Generates a string based on a format      */
/*                         string, ala printf().  This is generally  */
/*                         used to format STAF request strings.      */
/*                                                                   */
/* Accepts: (In)  The format string                                  */
/*          (Out) A pointer to the output string                     */
/*          (In)  A variable argument list                           */
/*                                                                   */
/* Returns:  Standard return codes                                   */
/*                                                                   */
/* Notes  :  1) The caller is responsible for destructing the        */
/*              output string                                        */
/*           2) Valid format strings are the same as defined for     */
/*              STAFUtilFormatString()                               */
/*********************************************************************/
unsigned int STAFUtilFormatString2(STAFStringConst_t formatString,
                                   STAFString_t *outputString, va_list args);


/*********************************************************************/
/* STAFUtilCreateTempFile - Creates a 0 length temporary file        */
/*                                                                   */
/* Accepts: (In)  A pointer to a string containing the name of the   */
/*                directory to store the temporary file in           */
/*          (In)  A pointer to a string containing the suffix for    */
/*                the temporary file name                            */
/*          (In)  The request number (used in generating a unique    */
/*                random seed)                                       */
/*          (Out) A pointer to a string containing the name of the   */
/*                temporary file created                             */
/*          (Out) A pointer to an error string                       */
/*          (Out) A pointer to an operating system return code       */
/*                                                                   */
/* Returns:  0, if successful                                        */
/*          >0, if unsuccessful (*errorBuffer and *osRC will be set) */
/*                                                                   */
/* Notes  :  1) The caller is responsible for destructing the output */
/*              string containing the name of the temporary file     */
/*********************************************************************/
STAFRC_t STAFUtilCreateTempFile(STAFStringConst_t prefix,
                                STAFStringConst_t suffix,
                                unsigned int requestNumber,
                                STAFString_t *tempFileName,
                                STAFString_t *errorBuffer,
                                unsigned int *osRC);


/*********************************************************************/
/* STAFUtilStripPortFromEndpoint - Removes @<Port> from the end of   */
/*     an endpoint if present.                                       */
/*                                                                   */
/* Accepts: (In/Out)  A pointer to a string containing the endpoint  */
/*                    with format:                                   */
/*                      [<Interface>://<Machine Identifier>[@<Port>] */
/*          (Out)     A pointer to a string containing the stripped  */
/*                    endpoint with format:                          */
/*                      [<Interface>://<Machine Identifier>          */
/*                                                                   */
/* Returns:  0                                                       */
/* Notes  :  1) The caller is responsible for destructing the output */
/*              string containing the stripped endpoint              */
/*********************************************************************/
STAFRC_t STAFUtilStripPortFromEndpoint(STAFStringConst_t endpoint,
                                       STAFString_t *strippedEndpoint);


/*********************************************************************/
/* STAFUtilValidateTrust - Verifies that the actual trust level is   */
/*     not less than the required trust level.                       */
/*                                                                   */
/* Accepts: (In)  The actual trust level of the requesting machine   */
/*          (In)  The required trust level for the service request   */
/*          (In)  A pointer to a string containing the name of the   */
/*                service                                            */
/*          (In)  A pointer to a string containing the request string*/
/*          (In)  A pointer to a string containing the name of the   */
/*                local machine (it's STAF/Config/Machine value)     */
/*          (In)  A pointer to a string containing the endpoint of   */
/*                the requesting machine                             */
/*          (In)  A pointer to a string containing the physical      */
/*                interface identifier of the requesting machine     */
/*          (In)  A pointer to a string containing the user of the   */
/*                requesting machine                                 */
/*          (Out) A pointer to an error string                       */
/*                                                                   */
/* Returns:  0,  if successful                                       */
/*           25, if unsuccessful (*errorBuffer will be set)          */
/*********************************************************************/
STAFRC_t STAFUtilValidateTrust(unsigned int actualTrustLevel,
                               unsigned int requiredTrustLevel,
                               STAFStringConst_t service,
                               STAFStringConst_t request,
                               STAFStringConst_t localMachine,
                               STAFStringConst_t requestingEndpoint,
                               STAFStringConst_t physicalInterfaceID,
                               STAFStringConst_t requestingUser,
                               STAFString_t *errorBuffer);


/*********************************************************************/
/* STAFUtilConvertStringToUInt - Converts a number expressed as a    */
/*   string to an unsigned integer within a specified range (that    */
/*   defaults to range 0 to UINT_MAX if not specified)               */
/*                                                                   */
/* Accepts: (In)  The string to be converted to an unsigned integer  */
/*          (In)  Option name string.  Set to blank if the string    */
/*                is not the value for an option.                    */
/*                This is used only in the error message.            */
/*          (Out) The unsigned integer value                         */
/*          (Out) A pointer to an error string.                      */
/*          (In)  The minimum value allowed (cannot be < 0)          */
/*                The default is 0.                                  */
/*          (Out) The maximum value allowed (cannot be > UINT_MAX    */
/*                (e.g. 4294967295).  The default is UINT_MAX.       */
/*                                                                   */
/* Returns:  0,  if successful                                       */
/*           47  if unsuccessful (*errorBuffer will be set)          */
/*********************************************************************/
STAFRC_t STAFUtilConvertStringToUInt(STAFStringConst_t theString,
                                     STAFStringConst_t optionName,
                                     unsigned int *theUInt,
                                     STAFString_t *errorBuffer,
                                     unsigned int minValue = 0,
                                     unsigned int maxValue = UINT_MAX);


/*********************************************************************/
/* STAFUtilConvertStringToUInt64 - Converts a number expressed as a  */
/*   string to an unsigned integer within a specified range (that    */
/*   defaults to range 0 to UINT64_MAX if not specified)             */
/*                                                                   */
/* Accepts: (In)  The string to be converted to an unsigned integer  */
/*          (In)  Option name string.  Set to blank if the string    */
/*                is not the value for an option.                    */
/*                This is used only in the error message.            */
/*          (Out) The unsigned integer (64-bit) value                */
/*          (Out) A pointer to an error string.                      */
/*          (In)  The minimum value allowed (cannot be < 0)          */
/*                The default is 0.                                  */
/*          (Out) The maximum value allowed (cannot be > UINT64_MAX  */
/*                (e.g. 18446744073709551615).  The default is       */
/*                UINT64_MAX.                                        */
/*                                                                   */
/* Returns:  0,  if successful                                       */
/*           47  if unsuccessful (*errorBuffer will be set)          */
/*********************************************************************/
/* XXX: Commented out until get UINT64_MAX working on Solaris
STAFRC_t STAFUtilConvertStringToUInt64(STAFStringConst_t theString,
                                       STAFStringConst_t optionName,
                                       STAFUInt64_t *theUInt,
                                       STAFString_t *errorBuffer,
                                       STAFUInt64_t minValue = 0,
                                       STAFUInt64_t maxValue = UINT64_MAX);
*/                                       


/*********************************************************************/
/* STAFUtilConvertDurationString - Converts the time duration        */
/*   expressed as a string to a numeric value in milliseconds.       */
/*                                                                   */
/* Accepts: (In)  The duration string                                */
/*                The duration string may be expressed in            */
/*                milliseconds, seconds, minutes, hours, days, or    */
/*                weeks.  Its format is:                             */
/*                  <Number>[<Type>]                                 */
/*                where <Number> is an integer >= 0 and <Type>, if   */
/*                specified, is one of the following:                */
/*                s (for seconds), m (for minutes), h (for hours),   */
/*                d (for days), or w (for weeks). For example:       */
/*                - 100 specifies 100 milliseconds,                  */
/*                - 10s specifies 10 seconds,                        */
/*                - 5m specifies 5 minutes,                          */
/*                - 2h specifies 2 hours,                            */
/*                - 1d specifies 1 day,                              */
/*                - 1w specifies 1 week                              */
/*          (Out) The numeric duration value in milliseconds         */
/*          (Out) A pointer to an error string                       */
/*                                                                   */
/* Returns:  0,  if successful                                       */
/*           47  if unsuccessful (*errorBuffer will be set)          */
/*********************************************************************/
STAFRC_t STAFUtilConvertDurationString(STAFStringConst_t durationString,
                                       unsigned int *duration,
                                       STAFString_t *errorBuffer);

/*********************************************************************/
/* STAFUtilConvertSizeString - Converts a size of a file expressed   */
/*   as a string to a numeric value in bytes.                        */
/*                                                                   */
/* Accepts: (In)  The size string                                    */
/*                The size string may be expressed in bytes,         */
/*                kilobytes, or megabytes.  Its format is:           */
/*                  <Number>[<Type>]                                 */
/*                where <Number> is an integer >= 0 and <Type>, if   */
/*                specified, is one of the following:                */
/*                k (for kilobytes) or m (for megabytes).            */
/*                For example:                                       */
/*                - 100000 specifies 100,000 bytes,                  */
/*                - 500k specifies 500 kilobytes (or 512,000 bytes), */
/*                - 5m specifies 5 megabytes (or 5,242,880 bytes)    */
/*          (Out) The numeric size value in bytes                    */
/*          (Out) A pointer to an error string                       */
/*                                                                   */
/* Returns:  0,  if successful                                       */
/*           47  if unsuccessful (*errorBuffer will be set)          */
/*********************************************************************/
STAFRC_t STAFUtilConvertSizeString(STAFStringConst_t sizeString,
                                   unsigned int *size,
                                   STAFString_t *errorBuffer);


#ifdef __cplusplus

/***********************************************************************/
/* VALIDATE_TRUST - Macro to verify that the actual trust level is     */
/*     not less than the required trust level.  This is what most of   */
/*     the STAF C++ external services use to verify trust.             */
/*     It assumes the service uses the following variable names:       */
/*     - pInfo: contains the service request information               */
/***********************************************************************/
#define VALIDATE_TRUST(requiredTrustLevel, service, request, localMachine)\
STAFString_t stafUtilTrustErrorBuffer = 0;\
STAFRC_t stafUtilTrustRC = STAFUtilValidateTrust(\
    pInfo->trustLevel, requiredTrustLevel, STAFString(service).getImpl(),\
    STAFString(request).getImpl(), localMachine.getImpl(),\
    pInfo->endpoint, pInfo->physicalInterfaceID, pInfo->user,\
    &stafUtilTrustErrorBuffer);\
if (stafUtilTrustRC != kSTAFOk)\
    return STAFResultPtr(new STAFResult(kSTAFAccessDenied,\
        STAFString(stafUtilTrustErrorBuffer, STAFString::kShallow)),\
        STAFResultPtr::INIT);
                         

/***********************************************************************/
/* VALIDATE_TRUST2- Macro to verify that the actual trust level is     */
/*     not less than the required trust level.  This is just like      */
/*     the VALIDATE_TRUST macro except it uses different names for     */
/*     the variables it defines so that it can be run in the same      */
/*     function as where you ran VALIDATE_TRUST.  This can be useful   */
/*     if you want to first check if a request has a trust level       */
/*     but then you find that an option like FORCE was specified that  */
/*     requires a higher trust level so you can then use this macro.   */
/*     It assumes the service uses the following variable names:       */
/*     - pInfo: contains the service request information               */
/***********************************************************************/
#define VALIDATE_TRUST2(requiredTrustLevel, service, request, localMachine)\
STAFString_t stafUtilTrustErrorBuffer = 0;\
STAFRC_t stafUtilTrustRC = STAFUtilValidateTrust(\
    pInfo->trustLevel, requiredTrustLevel, STAFString(service).getImpl(),\
    STAFString(request).getImpl(), localMachine.getImpl(),\
    pInfo->endpoint, pInfo->physicalInterfaceID, pInfo->user,\
    &stafUtilTrustErrorBuffer);\
if (stafUtilTrustRC != kSTAFOk)\
    return STAFResultPtr(new STAFResult(kSTAFAccessDenied,\
        STAFString(stafUtilTrustErrorBuffer, STAFString::kShallow)),\
        STAFResultPtr::INIT);
                        

/***********************************************************************/
/* VALIDATE_TRUST3 - Macro to verify that the actual trust level is    */
/*     not less than the required trust level.  This is what a STAF    */
/*     C++ external service can use to verify trust if can't use the   */
/*     pInfo variable (or a different named variable) to contain the   */
/*     service request information.  This macro can be used if         */
/*     individual STAFStrings are used to contain the requesting       */
/*     machine's trust level, endpoint, physicalInterfaceID, and/or    */
/*     user values.                                                    */
/***********************************************************************/
#define VALIDATE_TRUST3(requiredTrustLevel, service, request, localMachine,\
                        actualTrustLevel, endpoint, physicalInterfaceID, user)\
STAFString_t stafUtilTrustErrorBuffer = 0;\
STAFRC_t stafUtilTrustRC = STAFUtilValidateTrust(\
    actualTrustLevel, requiredTrustLevel, STAFString(service).getImpl(),\
    STAFString(request).getImpl(), localMachine.getImpl(),\
    STAFString(endpoint).getImpl(), STAFString(physicalInterfaceID).getImpl(),\
    STAFString(user).getImpl(), &stafUtilTrustErrorBuffer);\
if (stafUtilTrustRC != kSTAFOk)\
    return STAFResultPtr(new STAFResult(kSTAFAccessDenied,\
        STAFString(stafUtilTrustErrorBuffer, STAFString::kShallow)),\
        STAFResultPtr::INIT);
        

/***********************************************************************/
/* VALIDATE_TRUST4 - Macro to verify that the actual trust level is    */
/*     not less than the required trust level.  This is what a STAF    */
/*     C++ external service can use to verify trust if it's not using  */
/*     the standard name of pInfo to contain the service request       */
/*     information.                                                    */
/*     - info: contains the service request information                */
/***********************************************************************/
#define VALIDATE_TRUST4(requiredTrustLevel, service, request, localMachine,\
                        info)\
STAFString_t stafUtilTrustErrorBuffer = 0;\
STAFRC_t stafUtilTrustRC = STAFUtilValidateTrust(\
    info->trustLevel, requiredTrustLevel, STAFString(service).getImpl(),\
    STAFString(request).getImpl(), localMachine.getImpl(),\
    info->endpoint, info->physicalInterfaceID, info->user,\
    &stafUtilTrustErrorBuffer);\
if (stafUtilTrustRC != kSTAFOk)\
    return STAFResultPtr(new STAFResult(kSTAFAccessDenied,\
         STAFString(stafUtilTrustErrorBuffer, STAFString::kShallow)),\
         STAFResultPtr::INIT);
}

#endif

#endif
