/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <sys/time.h>
#include <cstdlib>
#include "STAFTimestamp.h"

struct STAFRelativeTimeImpl
{
    struct timeval theTime;
};


STAFRC_t STAFTimestampGetRelativeTime(STAFRelativeTime_t *currRelTime,
                                       unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (currRelTime == 0) return kSTAFInvalidParm;

    try
    {
        struct timeval theTime = { 0 };
        int rc2 = gettimeofday(&theTime, 0);

        if (rc2 != 0)
        {
            if (osRC) *osRC = rc2;
            return kSTAFBaseOSError;
        }

        *currRelTime = new STAFRelativeTimeImpl;

        (*currRelTime)->theTime = theTime;
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


STAFRC_t STAFTimestampGetRelativeTimeDifference(const STAFRelativeTime_t lhs,
                                                const STAFRelativeTime_t rhs,
                                                unsigned int *diffInMillis)
{
    if ((lhs == 0) || (rhs == 0)) return kSTAFInvalidObject;
    if (diffInMillis == 0) return kSTAFInvalidParm;

    *diffInMillis = ((lhs->theTime.tv_sec - rhs->theTime.tv_sec) * 1000) +
                    ((lhs->theTime.tv_usec - rhs->theTime.tv_usec + 500) / 1000);

    return kSTAFOk;
}


STAFRC_t STAFTimestampFreeRelativeTime(STAFRelativeTime_t *relTime)
{
    STAFRC_t rc = kSTAFOk;

    if (relTime == 0) return kSTAFInvalidParm;

    try
    {
        delete *relTime;
        *relTime = 0;
    }
    catch (...)
    { rc = kSTAFUnknownError; }

    return rc;
}

