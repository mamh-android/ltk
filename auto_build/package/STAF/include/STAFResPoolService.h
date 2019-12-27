/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ResPoolService
#define STAF_ResPoolService

#ifdef __cplusplus
extern "C"
{
#endif
   
typedef enum STAFResPoolRC_e {     
    kSTAFResPoolNotEntryOwner = 4005,
    kSTAFResPoolHasPendingRequests = 4006,
    kSTAFResPoolNoEntriesAvailable = 4007,
    kSTAFResPoolCreatePoolPathError = 4008,
    kSTAFResPoolInvalidFileFormat = 4009,
    kSTAFResPoolEntryIsOwned = 4010,
    kSTAFResPoolNotRequester = 4011
} STAFResPool_t;

#ifdef __cplusplus
}
#endif

#endif
