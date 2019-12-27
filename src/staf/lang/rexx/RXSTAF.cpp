/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <cstdio>
#include <string.h>
#include "STAF_rexx.h"
#include "RexxVar.h"
#include "STAF.h"

extern "C"
{

#if defined(STAF_OS_NAME_HPUX) && !defined(__ia64)
    void _main();
#endif

// These cause problems with REXX headers on AIX
//
// RexxFunctionHandler STAFLoadFuncs;
// RexxFunctionHandler RXSTAFRegister;
// RexxFunctionHandler RXSTAFUnRegister;
// RexxFunctionHandler RXSTAFSubmit;
// RexxFunctionHandler RXSTAFDropFuncs;

static char *NameTable[] =
{
    "STAFRegister",
    "STAFUnRegister",
    "STAFSubmit",
    "STAFDropFuncs"
};

static char *FunctionTable[] =
{
    "RXSTAFRegister",
    "RXSTAFUnRegister",
    "RXSTAFSubmit",
    "RXSTAFDropFuncs"
};


ULONG APIENTRY STAFLoadFuncs(UCHAR *, ULONG argc, RXSTRING *, PSZ,
                             RXSTRING *retstring)
{
    #if defined(STAF_OS_NAME_HPUX) && !defined(__ia64)
        _main();
    #endif

    if (argc != 0)
       return 40;

    for(int i = 0; i < (sizeof(FunctionTable) / sizeof(char *)); ++i)
    {
        RexxRegisterFunctionDll(NameTable[i], "RXSTAF", FunctionTable[i]);
    }

    retstring->strptr[0] = 0;
    retstring->strlength = 0;

    return 0;
}


// STAFRegister <Name> [, <Handle Var Name>]

ULONG APIENTRY RXSTAFRegister(UCHAR *, ULONG argc, RXSTRING *argv, PSZ,
                              RXSTRING *retstring)
{
    if ((argc < 1) || (argc > 2))
       return 40;

    STAFHandle_t handle = 0;
    unsigned int rc = 0;

    rc = STAFRegister(argv[0].strptr, &handle);

    if (rc == 0)
    {
        char *name = "STAFHandle";

        if (argc == 2) name = argv[1].strptr;

        RexxVar handleName(name);
        char handleVal[20];
        sprintf(handleVal, "%lu", handle);
        handleName = handleVal;
    }

    sprintf(retstring->strptr, "%lu", rc);
    retstring->strlength = strlen(retstring->strptr);

    return 0;
}


// STAFUnRegister [Handle]

ULONG APIENTRY RXSTAFUnRegister(UCHAR *, ULONG argc, RXSTRING *argv, PSZ,
                                RXSTRING *retstring)
{
    if (argc > 1)
       return 40;

    unsigned int rc = 0;
    STAFHandle_t handle = 0;

    if (argc == 1)
        sscanf(argv[0].strptr, "%lu", &handle);
    else
    {
        RexxVar theHandle("STAFHandle");
        sscanf(theHandle.Value(), "%lu", &handle);
    }

    rc = STAFUnRegister(handle);

    sprintf(retstring->strptr, "%lu", rc);
    retstring->strlength = strlen(retstring->strptr);

    return 0;
}


// STAFSubmit [Handle,] <Where>, <Service>, <Request> [, <Result Var Name>]

ULONG APIENTRY RXSTAFSubmit(UCHAR *, ULONG argc, RXSTRING *argv, PSZ,
                            RXSTRING *retstring)
{
    if ((argc < 3) || (argc > 5))
        return 40;

    unsigned int rc = 0;
    STAFHandle_t handle = 0;
    STAFSyncOption_t syncOption = kSTAFReqSync;
    char *result = 0;
    unsigned int resultLength = 0;

    if (argc > 3)
        sscanf(argv[0].strptr, "%lu", &handle);
    else
    {
        RexxVar theHandle("STAFHandle");
        sscanf(theHandle.Value(), "%lu", &handle);
    }
    
    RexxVar theSyncOption("STAFSyncOption");
    sscanf(theSyncOption.Value(), "%lu", &syncOption);    

    char *where = (argc == 3) ? argv[0].strptr : argv[1].strptr;
    char *service = (argc == 3) ? argv[1].strptr : argv[2].strptr;
    char *request = (argc == 3) ? argv[2].strptr : argv[3].strptr;
    unsigned int requestLength =  (unsigned int)((argc == 3) ?
                                  argv[2].strlength : argv[3].strlength);

    rc = STAFSubmit2(handle, syncOption, where, service, request, 
                     requestLength, &result, &resultLength);

    RexxVar stafResultName("STAFRESULT");
    RXSTRING rxResult = { 0 };

    if (result != 0)
    {
        rxResult.strptr = result;
        rxResult.strlength = resultLength;
    }

    stafResultName = rxResult;

    if (argc == 5)
    {
        RexxVar resultName(argv[4].strptr);
        resultName = rxResult;
    }

    if (result != 0) STAFFree(handle, result);

    sprintf(retstring->strptr, "%lu", rc);
    retstring->strlength = strlen(retstring->strptr);

    return 0;
}


ULONG APIENTRY RXSTAFDropFuncs(UCHAR *, ULONG argc, RXSTRING *, PSZ,
                               RXSTRING *retstring)
{
    if (argc != 0)
        return 40;

    for(int i = 0; i < (sizeof(FunctionTable) / sizeof(char *)); ++i)
    {
        RexxDeregisterFunction(NameTable[i]);
    }

    retstring->strptr[0] = 0;
    retstring->strlength = 0;

    return 0;
}


}  // End of extern "C"
