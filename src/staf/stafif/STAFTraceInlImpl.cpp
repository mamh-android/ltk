/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_TraceInlImpl
#define STAF_TraceInlImpl

#include "STAF.h"
#include "STAFTrace.h"

STAF_INLINE void STAFTrace::traceOn(STAFTracePoint_t mask)
{
    STAFTraceEnableTracePoints(mask);
}


STAF_INLINE void STAFTrace::traceOff(STAFTracePoint_t mask)
{
    STAFTraceDisableTracePoints(mask);
}


STAF_INLINE STAFTracePoint_t STAFTrace::getTraceMask()
{
    return STAFTraceGetEnabledTracePoints();
}


STAF_INLINE STAFRC_t
STAFTrace::setTraceDestination(STAFTraceDestination_t traceDestination,
                               const STAFString &filename,
                               STAFTraceFileMode_t traceFileMode,
                               unsigned int *osRC)
{
    return STAFTraceSetTraceDestination(traceDestination,
                                        filename.getImpl(),
                                        traceFileMode,
                                        osRC);
}


STAF_INLINE STAFTraceDestination_t
STAFTrace::getTraceDestination(STAFString &filename)
{
    STAFTraceDestination_t traceDestination = kSTAFTraceToStdout;
    STAFString_t filenameImpl = 0;
    STAFRC_t rc = STAFTraceGetTraceDestination(&traceDestination,
                                               &filenameImpl,
                                               0);

    if (filenameImpl != 0)
        filename = STAFString(filenameImpl, STAFString::kShallow);

    return traceDestination;
}


STAF_INLINE STAFTraceFileMode_t
STAFTrace::getTraceFileMode()
{
    STAFTraceFileMode_t traceFileMode = kSTAFTraceFileReplace;

    STAFRC_t rc = STAFTraceGetTraceFileMode(&traceFileMode, 0);

    return traceFileMode;
}


STAF_INLINE bool STAFTrace::doTrace(STAFTracePoint_t tracePoint)
{
    return ((tracePoint != 0) &&
            ((tracePoint & STAFTraceGetEnabledTracePoints()) == tracePoint));
}

STAF_INLINE STAFRC_t STAFTrace::trace(STAFTracePoint_t tracePoint,
                                      const char *message)
{
    return STAFTraceLogCStringMessage(tracePoint, message, 0);
}


STAF_INLINE STAFRC_t STAFTrace::trace(STAFTracePoint_t tracePoint,
                                      const STAFString &message)
{
    return STAFTraceLogSTAFStringMessage(tracePoint, message.getImpl(), 0);
}

#endif
