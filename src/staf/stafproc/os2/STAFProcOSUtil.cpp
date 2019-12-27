/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFProcOSUtil.h"

unsigned int makePerformanceAdjustments(STAFPerfInfo perfInfo,
                                        STAFString &errorBuffer,
                                        unsigned int &osRC)
{
    unsigned long maxFH = 0;
    long fileHandleDelta = perfInfo.maxFiles;
    APIRET rc = DosSetRelMaxFH(&fileHandleDelta, &maxFH);

    if (rc != 0)
    {
        errorBuffer = "Error calling DosSetRelMaxFH";
        osRC = (unsigned int)rc;
        return 1;
    }

    return 0;
}


unsigned int STAFProcOSInit(STAFString &errorBuffer, unsigned int &osRC)
{
    return 0;
}


unsigned int STAFProcOSTerm(STAFString &errorBuffer, unsigned int &osRC)
{
    return 0;
}
