/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ProcessInlImpl
#define STAF_ProcessInlImpl

#include "STAF.h"
#include "STAFProcess.h"

STAF_INLINE STAFRC_t STAFProcess::getProcessHandleFromID(
    STAFProcessID_t pid, STAFProcessHandle_t &procHandle, unsigned int &osRC)
{
    return STAFProcessGetHandleFromID(pid, &procHandle, &osRC);
}


STAF_INLINE bool STAFProcess::isRunning(STAFProcessHandle_t processHandle)
{
    unsigned int stillRunning = 0;
    STAFRC_t rc = STAFProcessIsRunning(processHandle, &stillRunning, 0);

    return (stillRunning == 1);
}


STAF_INLINE STAFRC_t STAFProcess::isValidAuthMode(
    STAFProcessAuthenticationMode_t authMode)
{
    return STAFProcessIsValidAuthMode(authMode);
}


STAF_INLINE STAFRC_t STAFProcess::isValidStopMethod(
    STAFProcessStopMethod_t stopMethod)
{
    return STAFProcessIsValidStopMethod(stopMethod);
}


STAF_INLINE STAFRC_t STAFProcess::registerForProcessTermination(
    STAFProcessID_t pid, STAFProcessHandle_t procHandle,
    STAFProcessEndCallbackLevel1 &callback)
{
    return STAFProcessRegisterEndCallback(pid, procHandle, &callback, 1);
}


STAF_INLINE STAFRC_t STAFProcess::startProcess(
    STAFProcessStartInfoLevel1 &startInfo, STAFProcessID_t &pid,
    STAFProcessHandle_t &procHandle, unsigned int &osRC)
{
    return STAFProcessStart(&pid, &procHandle, &startInfo, 1, &osRC);
}


STAF_INLINE STAFRC_t STAFProcess::startProcess2(
    STAFProcessStartInfoLevel1 &startInfo, STAFProcessID_t &pid,
    STAFProcessHandle_t &procHandle, unsigned int &osRC,
    STAFString &errorBuffer)
{
    STAFString_t theBuffer = 0;

    STAFRC_t rc = STAFProcessStart2(
        &pid, &procHandle, &startInfo, 1, &osRC, &theBuffer);

    if (theBuffer)
        errorBuffer = STAFString(theBuffer, STAFString::kShallow);

    return rc;
}


STAF_INLINE STAFRC_t STAFProcess::stopProcess(
    STAFProcessID_t pid, STAFProcessStopMethod_t stopMethod,
    unsigned int &osRC)
{
    return STAFProcessStop(pid, stopMethod, &osRC);
}


STAF_INLINE STAFRC_t STAFProcess::stopProcess2(
    STAFProcessID_t pid, STAFProcessStopMethod_t stopMethod,
    STAFProcessStopFlag_t stopFlag, unsigned int &osRC)
{
    return STAFProcessStop2(pid, stopMethod, stopFlag, &osRC);
}

#endif
