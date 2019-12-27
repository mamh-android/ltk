/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFTimestamp.h"
#include "STAFMutexSem.h"

static STAFMutexSem sMutexSem;

STAFRC_t STAFThreadSafeLocalTimeString(char *buffer, unsigned int bufSize,
                                       const char *format, time_t theTime,
                                       unsigned int *osRC)
{
    try
    {
        STAFMutexSemLock semLock(sMutexSem);

        int numChars = strftime(buffer, bufSize, format, localtime(&theTime));

        if ((numChars == 0) && (osRC)) *osRC = errno;

        return (numChars == 0) ? kSTAFBaseOSError : kSTAFOk;
    }
    catch (STAFException &se)
    {
        if (osRC) *osRC = se.getErrorCode();
        return kSTAFUnknownError;
    }
    catch (...)
    {
        return kSTAFUnknownError;
    }
}


STAFRC_t STAFThreadSafeLocalTime(struct tm *theTM, time_t theTime,
                                 unsigned int *osRC)
{
    try
    {
        STAFMutexSemLock semLock(sMutexSem);

        *theTM = *localtime(&theTime);
    }
    catch (STAFException &se)
    {
        if (osRC) *osRC = se.getErrorCode();
        return kSTAFUnknownError;
    }
    catch (...)
    {
        return kSTAFUnknownError;
    }

    return kSTAFOk;
}
