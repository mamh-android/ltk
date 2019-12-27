/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFConnectionProvider.h"
#include "STAFDynamicLibrary.h"

#define GET_ADDRESS(name, address) \
{\
    STAFRC_t rc = STAFDynamicLibraryGetAddress(library, name,\
                      reinterpret_cast<void **>(address), errorBuffer);\
    if (rc != 0) return rc;\
}


STAFRC_t STAFConnectionProviderLoad(STAFDynamicLibrary_t library,
                                    STAFConnectionProviderFunctionTable *funcs,
                                    STAFString_t *errorBuffer)
{
    if (library == 0) return kSTAFInvalidObject;
    if (funcs == 0) return kSTAFInvalidParm;

    GET_ADDRESS("STAFConnectionProviderConstruct", &funcs->provConstruct);
    GET_ADDRESS("STAFConnectionProviderStart",     &funcs->provStart);
    GET_ADDRESS("STAFConnectionProviderStop",      &funcs->provStop);
    GET_ADDRESS("STAFConnectionProviderDestruct",  &funcs->provDestruct);
    GET_ADDRESS("STAFConnectionProviderConnect",   &funcs->provConnect);
    GET_ADDRESS("STAFConnectionProviderGetMyNetworkIDs",
                &funcs->provGetMyNetworkIDs);
    GET_ADDRESS("STAFConnectionProviderGetOptions", &funcs->provGetOptions);
    GET_ADDRESS("STAFConnectionProviderGetProperty", &funcs->provGetProperty);
    GET_ADDRESS("STAFConnectionRead",              &funcs->connRead);
    GET_ADDRESS("STAFConnectionReadUInt",          &funcs->connReadUInt);
    GET_ADDRESS("STAFConnectionReadSTAFString",    &funcs->connReadSTAFString);
    GET_ADDRESS("STAFConnectionWrite",             &funcs->connWrite);
    GET_ADDRESS("STAFConnectionWriteUInt",         &funcs->connWriteUInt);
    GET_ADDRESS("STAFConnectionWriteSTAFString",   &funcs->connWriteSTAFString);
    GET_ADDRESS("STAFConnectionGetPeerNetworkIDs",
                &funcs->connGetPeerNetworkIDs);
    GET_ADDRESS("STAFConnectionDestruct",          &funcs->connDestruct);

    return kSTAFOk;
}
