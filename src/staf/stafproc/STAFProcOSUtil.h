/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ProcOSUtil
#define STAF_ProcOSUtil

#include "STAFString.h"

// Defines a structor for passing performance parameters
struct STAFPerfInfo
{
    unsigned int maxFiles;
};

// Sets system configuration appropriate with performance data
// If successful, returns 0
// If unsuccessful, retruns >0, errorBuffer will be set with a descriptive
// string, and osRC will be set to an error code relating to the message in
// errorBuffer.
unsigned int makePerformanceAdjustments(STAFPerfInfo perfInfo,
                                        STAFString &errorBuffer,
                                        unsigned int &osRC);

// Performs OS Specific Initialization
unsigned int STAFProcOSInit(STAFString &errorBuffer, unsigned int &osRC);

// Performs Lock On Data Directory
unsigned int STAFProcOSLockDataDir(STAFString &errorBuffer, unsigned int &osRC);


// Performs OS Specific Termination
unsigned int STAFProcOSTerm(STAFString &errorBuffer, unsigned int &osRC);

#endif
