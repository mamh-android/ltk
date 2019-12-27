/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Trace
#define STAF_Trace

#include "STAF.h"
#include "STAFString.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum STAFTracePoint_e
{
    // No trace points
    kSTAFTraceNone                = 0x00000000,

    // Service Auditing
    kSTAFTraceServiceRequest      = 0x00000001,
    kSTAFTraceServiceResult       = 0x00000002,
    kSTAFTraceServiceError        = 0x00000004,
    kSTAFTraceServiceAccessDenied = 0x00000008,
    kSTAFTraceServiceManagement   = 0x00000010,
    kSTAFTraceRemoteRequests      = 0x00000020,

    // Unexpected/Error conditions
    kSTAFTraceError               = 0x00000100,

    // Registration Auditing
    kSTAFTraceRegistration        = 0x00000200,

    // Warning conditions
    kSTAFTraceWarning             = 0x00000400,

    // Info conditions
    kSTAFTraceInfo                = 0x00000800,

    // Deprecated options
    kSTAFTraceDeprecated          = 0x00001000,

    // Debug conditions
    kSTAFTraceDebug               = 0x00002000,

    // Service Complete
    kSTAFTraceServiceComplete     = 0x00004000,

    // All trace points
    kSTAFTraceAll                 = 0xFFFFFFFF
} STAFTracePoint_t;

typedef enum STAFTraceDestination_e
{
    kSTAFTraceToStdout        = 0,
    kSTAFTraceToStderr        = 1,
    kSTAFTraceToFile          = 2,
    kSTAFTraceToStdoutAndFile = 3,
    kSTAFTraceToStderrAndFile = 4
} STAFTraceDestination_t;

typedef enum STAFTraceFileMode_e
{
    kSTAFTraceFileReplace  = 0,
    kSTAFTraceFileAppend   = 1
} STAFTraceFileMode_t;


/****************************************************************************/
/* STAFTraceGetEnabledTracePoints - Retrieves the currently enabled trace   */
/*                                  points                                  */
/*                                                                          */
/* Accepts:  Nothing                                                        */
/*                                                                          */
/* Returns:  The currently enabled trace points                             */
/****************************************************************************/
STAFTracePoint_t STAFTraceGetEnabledTracePoints();


/****************************************************************************/
/* STAFTraceEnableTracePoints - Enables specified trace points              */
/*                                                                          */
/* Accepts: (IN)  The trace points to enable                                */
/*                                                                          */
/* Returns:  Nothing                                                        */
/****************************************************************************/
void STAFTraceEnableTracePoints(STAFTracePoint_t tracePoints);


/****************************************************************************/
/* STAFTraceDisableTracePoints - Disables specified trace points            */
/*                                                                          */
/* Accepts: (IN)  The trace points to disable                               */
/*                                                                          */
/* Returns:  Nothing                                                        */
/****************************************************************************/
void STAFTraceDisableTracePoints(STAFTracePoint_t tracePoints);


/****************************************************************************/
/* STAFTraceSetTraceDestination - Sets the destination for trace messages   */
/*                                                                          */
/* Accepts: (IN)  The trace destination                                     */
/*          (IN)  The file to trace to if destination is kSTAFTraceToFile   */
/*          (IN)  The file mode (replace or append) if destination is       */
/*                kSTAFTraceToFile (default is replace)                     */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, if successful                                         */
/*           other,   if unsuccessful                                       */
/****************************************************************************/
STAFRC_t STAFTraceSetTraceDestination(STAFTraceDestination_t traceDestination,
                                      STAFStringConst_t filename,
                                      STAFTraceFileMode_t traceFileMode,
                                      unsigned int *osRC);

/****************************************************************************/
/* STAFTraceGetTraceDestination - Gets the destination for trace messages   */
/*                                                                          */
/* Accepts: (OUT) Pointer to trace destination                              */
/*          (OUT) Pointer to trace file name                                */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, if successful                                         */
/*           other,   if unsuccessful                                       */
/*                                                                          */
/* Notes: 1) The trace file name will be non-NULL only if the trace         */
/*           destination is kSTAFTraceToFile                                */
/*        2) If the trace file name is non-NULL, you are responsible for    */
/*           calling STAFStringDestruct()                                   */
/****************************************************************************/
STAFRC_t STAFTraceGetTraceDestination(STAFTraceDestination_t *traceDestination,
                                      STAFString_t *filename,
                                      unsigned int *osRC);


/****************************************************************************/
/* STAFTraceGetTraceFileMode - Gets the trace file mode                     */
/* Accepts: (OUT) Pointer to trace file mode                                */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, if successful                                         */
/*           other,   if unsuccessful                                       */
/*                                                                          */
/****************************************************************************/
STAFRC_t STAFTraceGetTraceFileMode(STAFTraceFileMode_t *traceFileMode,
                                   unsigned int *osRC);


/****************************************************************************/
/* STAFTraceLogCStringMessage - Logs a trace message provided via a         */
/*                              NULL-terminated string                      */
/*                                                                          */
/* Accepts: (IN)  The trace point                                           */
/*          (IN)  Message                                                   */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, if successful                                         */
/*           other,   if unsuccessful                                       */
/****************************************************************************/
STAFRC_t STAFTraceLogCStringMessage(STAFTracePoint_t tracePoint,
                                    const char *message,
                                    unsigned int *osRC);


/****************************************************************************/
/* STAFTraceLogSTAFStringMessage - Logs a trace message provided via a      */
/*                                 STAFString                               */
/*                                                                          */
/* Accepts: (IN)  The trace point                                           */
/*          (IN)  Message                                                   */
/*          (OUT) Pointer to operating system return code                   */
/*                                                                          */
/* Returns:  kSTAFOk, if successful                                         */
/*           other,   if unsuccessful                                       */
/****************************************************************************/
STAFRC_t STAFTraceLogSTAFStringMessage(STAFTracePoint_t tracePoint,
                                       STAFStringConst_t message,
                                       unsigned int *osRC);

#ifdef __cplusplus
}

// Begin C++ language declarations

// STAFTrace - This class provides a thin wrapper around the C STAFTrace APIs.
//             It also provides the utility method doTrace(), which will
//             determine if a specific tracePoint would be traced before
//             actually trying to trace it.

class STAFTrace
{
public:

    static void traceOn(STAFTracePoint_t mask);
    static void traceOff(STAFTracePoint_t mask);
    static STAFTracePoint_t getTraceMask();

    static STAFRC_t setTraceDestination(STAFTraceDestination_t traceDestination,
                                        const STAFString &filename =
                                                          STAFString(),
                                        STAFTraceFileMode_t traceFileMode =
                                                       kSTAFTraceFileReplace,
                                        unsigned int *osRC = 0);
    static STAFTraceDestination_t getTraceDestination(STAFString &filenam);
    static STAFTraceFileMode_t getTraceFileMode();

    static bool doTrace(STAFTracePoint_t tracePoint);

    static STAFRC_t trace(STAFTracePoint_t tracePoint, const char *message);
    static STAFRC_t trace(STAFTracePoint_t tracePoint,
                          const STAFString &message);
};

// Now include inline definitions

inline STAFTracePoint_t operator|(const STAFTracePoint_t lhs,
                                  const STAFTracePoint_t rhs)
{
    return static_cast<STAFTracePoint_t>(static_cast<unsigned int>(lhs) |
                                         static_cast<unsigned int>(rhs));
}

#ifndef STAF_NATIVE_COMPILER
#include "STAFTraceInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
