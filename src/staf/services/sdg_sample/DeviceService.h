/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DeviceService
#define STAF_DeviceService

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum STAFDeviceError_e
{
    // add service-specific return codes here
    kDeviceInvalidSerialNumber = 4001
} STAFDeviceError_t;


#ifdef __cplusplus
}
#endif

#endif
