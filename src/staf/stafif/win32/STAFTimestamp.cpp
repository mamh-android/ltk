/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFTimestamp.h"

struct STAFRelativeTimeImpl
{
    DWORD theTime;
};


STAFRC_t STAFTimestampGetRelativeTime(STAFRelativeTime_t *currRelTime,
                                       unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (currRelTime == 0) return kSTAFInvalidParm;

    try
    {
        *currRelTime = new STAFRelativeTimeImpl;

        (*currRelTime)->theTime = GetTickCount();
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

    if (rhs->theTime > lhs->theTime)
        *diffInMillis = (0xFFFFFFFF - rhs->theTime) + lhs->theTime;
    else
        *diffInMillis = lhs->theTime - rhs->theTime;

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

