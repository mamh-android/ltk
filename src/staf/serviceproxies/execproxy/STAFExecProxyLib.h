/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_STAFEXECPROXY
#define STAF_STAFEXECPROXY

enum STAFExecProxyIPCMessageType_t
{
    STAFEXECPROXY_PING            = 0,
    STAFEXECPROXY_LOAD            = 1,
    STAFEXECPROXY_INIT            = 2,
    STAFEXECPROXY_ACCEPT_REQUEST  = 3,
    STAFEXECPROXY_TERM            = 4,
    STAFEXECPROXY_DESTRUCT        = 5,
    STAFEXECPROXY_EXIT            = 6
};

#endif

