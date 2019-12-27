/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFTimestamp.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFRefPtr.h"
#include "STAFTrace.h"
#include "STAFThread.h"
#include "STAFMutexSem.h"

typedef STAFRefPtr<ofstream> STAFTraceOutput;

static STAFTraceDestination_t sDestination = kSTAFTraceToStdout;
static STAFTraceFileMode_t sTraceFileMode = kSTAFTraceFileReplace;
static STAFTraceOutput sTraceOutput;
static STAFString sTraceFile;

// Turn on tracing by default for Error and Deprecated tracepoints
static STAFTracePoint_t sMask = static_cast<STAFTracePoint_t>(
    static_cast<unsigned int>(kSTAFTraceError) |
    static_cast<unsigned int>(kSTAFTraceDeprecated));
static STAFMutexSem sTraceSem;

STAFTracePoint_t STAFTraceGetEnabledTracePoints()
{
    return sMask;
}


void STAFTraceEnableTracePoints(STAFTracePoint_t tracePoints)
{
    sMask = static_cast<STAFTracePoint_t>(
        static_cast<unsigned int>(sMask) |
        static_cast<unsigned int>(tracePoints));
}


void STAFTraceDisableTracePoints(STAFTracePoint_t tracePoints)
{
    sMask = static_cast<STAFTracePoint_t>(
        static_cast<unsigned int>(sMask) &
        ~static_cast<unsigned int>(tracePoints));
}


STAFRC_t STAFTraceSetTraceDestination(STAFTraceDestination_t traceDestination,
                                      STAFStringConst_t filename,
                                      STAFTraceFileMode_t traceFileMode,
                                      unsigned int *osRC)
{
    try
    {
        if ((traceDestination == kSTAFTraceToStdout) ||
            (traceDestination == kSTAFTraceToStderr))
        {
            sDestination = traceDestination;
            sTraceOutput = STAFTraceOutput();
        }
        else
        {
            if (traceFileMode == kSTAFTraceFileAppend)
            {
                sTraceOutput = STAFTraceOutput(new
                    ofstream(STAFString(filename).toCurrentCodePage()->buffer(),
                             ios::out | ios::ate | ios::app),
                    STAFTraceOutput::INIT);
            }
            else
            {
                sTraceOutput = STAFTraceOutput(new
                    ofstream(STAFString(filename).toCurrentCodePage()->buffer(),
                             ios::out | ios::ate),
                    STAFTraceOutput::INIT);
            }

            if (traceDestination == kSTAFTraceToStdoutAndFile)
                sDestination = kSTAFTraceToStdoutAndFile;
            else if (traceDestination == kSTAFTraceToStderrAndFile)
                sDestination = kSTAFTraceToStderrAndFile;
            else
                sDestination = kSTAFTraceToFile;

            sTraceFileMode = traceFileMode;
            sTraceFile = filename;
        }
    }
    catch (...)
    {
        return kSTAFUnknownError;
    }

    return kSTAFOk;
}


STAFRC_t STAFTraceGetTraceDestination(STAFTraceDestination_t *traceDestination,
                                      STAFString_t *filename,
                                      unsigned int *osRC)
{
    try
    {
        if (traceDestination == 0) return kSTAFInvalidValue;

        *traceDestination = sDestination;

        if ((sDestination >= kSTAFTraceToFile) && (filename != 0))
        {
            *filename = STAFString(sTraceFile).adoptImpl();
        }
    }
    catch (...)
    {
        return kSTAFUnknownError;
    }

    return kSTAFOk;
}


STAFRC_t STAFTraceGetTraceFileMode(STAFTraceFileMode_t *traceFileMode,
                                   unsigned int *osRC)
{
    try
    {
        *traceFileMode = sTraceFileMode;
    }
    catch (...)
    {
        return kSTAFUnknownError;
    }

    return kSTAFOk;
}


STAFRC_t STAFTraceLogCStringMessage(STAFTracePoint_t tracePoint,
                                    const char *message,
                                    unsigned int *osRC)
{
    try
    {
        return STAFTraceLogSTAFStringMessage(tracePoint,
                                             STAFString(message).getImpl(),
                                             osRC);
    }
    catch (...)
    {
        // Do nothing
    }

    return kSTAFUnknownError;
}


STAFRC_t STAFTraceLogSTAFStringMessage(STAFTracePoint_t tracePoint,
                                       STAFStringConst_t message,
                                       unsigned int *osRC)
{
    try
    {
        if ((tracePoint == 0) || ((tracePoint & sMask) != tracePoint))
            return kSTAFOk;
        
        STAFMutexSemLock lock(sTraceSem);

        if ((sDestination == kSTAFTraceToStdout) ||
            (sDestination == kSTAFTraceToStdoutAndFile))
        {
            cout << STAFTimestamp::now().asString() << ";"
                 << STAFThreadCurrentThreadID() << ";"
                 << hex << setw(8) << setfill('0') << tracePoint << dec
                 << ";" << STAFHandle::maskPrivateData(STAFString(message))
                 << endl;
        }
        else if ((sDestination == kSTAFTraceToStderr) ||
                 (sDestination == kSTAFTraceToStderrAndFile))
        {
            cerr << STAFTimestamp::now().asString() << ";"
                 << STAFThreadCurrentThreadID() << ";"
                 << hex << setw(8) << setfill('0') << tracePoint << dec
                 << ";" << STAFHandle::maskPrivateData(STAFString(message))
                 << endl;
        }
        
        if (sDestination >= kSTAFTraceToFile)
        {
            (*sTraceOutput) << STAFTimestamp::now().asString() << ";"
                            << STAFThreadCurrentThreadID() << ";"
                            << hex << setw(8) << setfill('0') << tracePoint
                            << dec << ";"
                            << STAFHandle::maskPrivateData(STAFString(message))
                            << endl;
        }
    }
    catch (...)
    {
        return kSTAFUnknownError;
    }

    return kSTAFOk;
}
