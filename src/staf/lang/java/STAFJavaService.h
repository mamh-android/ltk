/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_JavaService
#define STAF_JavaService

enum JavaServiceIPCMessageType_t
{
    JAVA_SERVICE_JVMPING        = 0,
    JAVA_SERVICE_LOAD           = 1,
    JAVA_SERVICE_INIT           = 2,
    JAVA_SERVICE_ACCEPT_REQUEST = 3,
    JAVA_SERVICE_TERM           = 4,
    JAVA_SERVICE_DESTRUCT       = 5,
    JAVA_SERVICE_JVMEXIT        = 6,
    JAVA_SERVICE_JVMFIN         = 7
};

#endif
