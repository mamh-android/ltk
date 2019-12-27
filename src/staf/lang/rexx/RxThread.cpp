/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <string>
#include <map>
#include "STAF_fstream.h"
#include <cstdio>
#include <string.h>
#include "STAF_rexx.h"
#include "STAFUtil.h"
#include "RexxVar.h"
#include "STAFEventSem.h"
#include "STAFMutexSem.h"
#include "RxThread.h"
#include "STAFThread.h"

extern "C"
{

#ifdef STAF_OS_NAME_HPUX
    void _main();
#endif

// These cause problems with REXX headers on AIX
//
// RexxFunctionHandler RxThreadLoadFuncs;
// RexxFunctionHandler RxThreadCreateTokenImage;
// RexxFunctionHandler RxThreadStart;
// RexxFunctionHandler RxThreadValue;
// RexxFunctionHandler RxThreadDropFuncs;

static unsigned int rexxThread(void *threadInfo);

struct RxThreadInfo
{
    RXSTRING image;
    RXSTRING function;
    RXSTRING data;
    STAFEventSem synch;
};

static RexxVarListList gRexxVarListList;
static STAFMutexSem gRexxVarListListSem;

static char *NameTable[] =
{
    "RxThreadStart",
    "RxThreadCreateTokenImage",
    "RxThreadValue",
    "RxThreadDropFuncs"
};

static char *FunctionTable[] =
{
    "RxThreadStart",
    "RxThreadCreateTokenImage",
    "RxThreadValue",
    "RxThreadDropFuncs"
};


ULONG APIENTRY RxThreadLoadFuncs(UCHAR *, ULONG argc, RXSTRING *, PSZ,
                                 RXSTRING *retstring)
{
    #ifdef STAF_OS_NAME_HPUX
        _main();
    #endif

    if (argc != 0)
       return 40;

    for(int i = 0; i < (sizeof(FunctionTable) / sizeof(char *)); ++i)
    {
        RexxRegisterFunctionDll(NameTable[i], "RxThread", FunctionTable[i]);
    }

    retstring->strptr[0] = 0;
    retstring->strlength = 0;

    return 0;
}


// Rexx Syntax: rc = RxThreadCreateTokenImage(filename, imageVarName)

ULONG APIENTRY RxThreadCreateTokenImage(UCHAR *, ULONG argc, RXSTRING *argv,
                                        PSZ, RXSTRING *retstring)
{
    if (argc != 2) return 40;

    SHORT rexxRC = 0;
    LONG rc = 0;
    RXSTRING arg = { 3, "//T" };
    RXSTRING result = { 0 };
    RXSTRING inStore[2] = { 0 };

    // First open the file

    fstream sourceFile(argv[0].strptr, std::ios::binary | std::ios::in);

    if (!sourceFile)
    {
        retstring->strptr[0] = '1';
        retstring->strlength = 1;
        return 0;
    }

    // Figure out how big it is

    sourceFile.seekg(0, std::ios::end);
    unsigned int fileLength = (unsigned int)sourceFile.tellg();

    // Initialize the source buffer

    inStore[0].strlength = fileLength;
    inStore[0].strptr = new char[fileLength];
    sourceFile.seekg(0, std::ios::beg);
    sourceFile.read(inStore[0].strptr, fileLength);

    // Now call RexxStart to tokenize the source

    rc = RexxStart(1, &arg, argv[0].strptr, inStore, 0, RXCOMMAND, 0, &rexxRC,
                   &result);

    // Free up the source image

    delete [] inStore[0].strptr;

    if (rc != 0)
    {
        retstring->strptr[0] = '2';
        retstring->strlength = 1;
        return 0;
    }

    // Copy the image into the users variable

    RexxVar tokenImage(argv[1].strptr);

    tokenImage = inStore[1];

    // Free up the sytem memory for the tokenized image

    STAFUtilFreeSystemMemory(inStore[1].strptr);

    retstring->strptr[0] = '0';
    retstring->strlength = 1;

    return 0;
}


// Rexx Syntax: tid = RxThreadStart(image, function [, data])

ULONG APIENTRY RxThreadStart(UCHAR *, ULONG argc, RXSTRING *argv, PSZ,
                             RXSTRING *retstring)
{
    if ((argc < 2) || (argc > 3)) return 40;

    unsigned int rc = 0;
    RxThreadInfo *pRxThreadInfo = 0;

    try
    {
        // Get a new thread info structure and initialize it

        pRxThreadInfo = new RxThreadInfo;

        pRxThreadInfo->image = argv[0];
        pRxThreadInfo->function = argv[1];

        if (argc == 3) pRxThreadInfo->data = argv[2];
        else
        {
            pRxThreadInfo->data.strptr = 0;
            pRxThreadInfo->data.strlength = 0;
        }

        // Start the thread
        // Note: The actual starting of the Rexx program occurs in the
        //       rexxThread function

        STAFThreadID_t threadID = 0;
        rc = STAFThreadStart(&threadID, rexxThread, pRxThreadInfo, 0, 0);

        // Set the return code to the thread's tid

        rc = threadID;

        // Wait for the thread to signal us that is is done copying
        // info from the thread info structure

        if (pRxThreadInfo->synch.wait() != 0)
        {
            // We timed out waiting for the thread to signal us
            // So, let's kill it and return an error to the client

            rc = 0;
            // XXX:
            // rxThread.stop();
        }
    }
    catch (...)
    {
    }

    // Now free up the thread info structure

    delete pRxThreadInfo;

    // Set the REXX return code

    sprintf(retstring->strptr, "%lu", rc);
    retstring->strlength = strlen(retstring->strptr);

    return 0;
}


unsigned int rexxThread(void *threadInfo)
{
    RxThreadInfo *pRxThreadInfo = (RxThreadInfo *)threadInfo;
    RXSTRING image = { 0 };
    RXSTRING function = { 0 };
    RXSTRING data = { 0 };

    try
    {
        // Copy the image data from the thread info structure into
        // heap memory

        image.strlength = pRxThreadInfo->image.strlength;
        image.strptr = new char[image.strlength + 1];
        memcpy(image.strptr, pRxThreadInfo->image.strptr,
               (size_t)image.strlength);
        image.strptr[image.strlength] = 0;

        // Copy the function data from the thread info structure into
        // heap memory

        function.strlength = pRxThreadInfo->function.strlength;
        function.strptr = new char[function.strlength + 1];
        memcpy(function.strptr, pRxThreadInfo->function.strptr,
               (size_t)function.strlength);
        function.strptr[function.strlength] = 0;

        // If necessary, copy the function data from the thread info
        // structure into heap memory

        if (pRxThreadInfo->data.strptr != 0)
        {
            data.strlength = pRxThreadInfo->data.strlength;
            data.strptr = new char[data.strlength + 1];
            memcpy(data.strptr, pRxThreadInfo->data.strptr,
                   (size_t)data.strlength);
            data.strptr[data.strlength] = 0;
        }

        // Let RxThreadStart know we are done with its thread info data

        pRxThreadInfo->synch.post();

        // Initialize some variables needed by RexxStart

        RXSTRING arg[2] = { 0 };
        SHORT rexxRC = 0;
        LONG rc = 0;
        RXSTRING rexxResult = { 0 };

        // Set the first Rexx argument to our tid

        char tidString[32] = { 0 };

        sprintf(tidString, "%lu", STAFThreadCurrentThreadID());

        arg[0].strptr = tidString;
        arg[0].strlength = strlen(tidString);

        // Set the second Rexx argrument to whatever the user passed in

        arg[1] = data;

        // Set up the inStore array to point at our tokenized image

        RXSTRING inStore[2] = { 0 };
        inStore[1] = image;

        // Now call RexxStart and free the result if necessary
        // We don't currently do anything with the return code.

        rc = RexxStart(2, arg, function.strptr, inStore, 0, RXFUNCTION, 0,
                       &rexxRC, &rexxResult);

        if (rexxResult.strptr != 0)
            STAFUtilFreeSystemMemory(rexxResult.strptr);
    }
    catch (...)
    {
    }

    // Free our heap memory

    delete [] image.strptr;
    delete [] function.strptr;
    delete [] data.strptr;

    return 0;
}

// Rexx Syntax: value = RxThreadValue(pool, name [, value])

ULONG APIENTRY RxThreadValue(UCHAR *, ULONG argc, RXSTRING *argv, PSZ,
                             RXSTRING *retstring)
{
    if ((argc < 2) || (argc > 3)) return 40;

    std::string listName(argv[0].strptr, (unsigned int)argv[0].strlength);
    std::string name(argv[1].strptr, (unsigned int)argv[1].strlength);
    std::string value;
    STAFMutexSemLock lock(gRexxVarListListSem);

    if (argc == 2)
    {
        // Get the value

        value = gRexxVarListList[listName][name];
    }
    else
    {
        value = std::string(argv[2].strptr, (unsigned int)argv[2].strlength);

        gRexxVarListList[listName][name] = value;
    }

    unsigned int osRC = 0;

    if (value.length() > 256)
    {
        // This memory will be freed by the REXX interpreter

        void *theMem = STAFUtilGetSystemMemory(value.length(), &osRC);
        if (theMem != 0) retstring->strptr = (char *)theMem;
    }

    if (osRC == 0)
    {
        if (value.length() != 0)
            memcpy(retstring->strptr, value.data(), value.length());
        retstring->strlength = value.length();
    }

    return 0;
}


ULONG APIENTRY RxThreadDropFuncs(UCHAR *, ULONG argc, RXSTRING *, PSZ,
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
