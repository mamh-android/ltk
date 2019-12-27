/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_LogService
#define STAF_LogService

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum STAFLogError_e
{
    kSTAFLogInvalidLevel = 4004,
    kSTAFLogInvalidFileFormat = 4007,
    kSTAFLogPurgeFailure = 4008,
    kSTAFLogExceededDefaultMaxRecords = 4010
} STAFLogError_t;


#ifdef __cplusplus
}
#endif

#endif
