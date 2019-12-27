/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFProcOSUtil.h"
#include "STAFProc.h"
#include "STAFUtilWin32.h"

static HANDLE sSTAFProcAtomHandle = 0;
static HANDLE sDataDirAtomHandle = 0;

unsigned int makePerformanceAdjustments(STAFPerfInfo, STAFString &,
                                        unsigned int &)
{
    return 0;
}


BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
    if ((ctrlType == CTRL_C_EVENT) || (ctrlType == CTRL_BREAK_EVENT))
    {
        (*gShutdownSemaphorePtr)->post();
        return TRUE;
    }

    return FALSE;
}


unsigned int STAFProcOSInit(STAFString &errorBuffer, unsigned int &osRC)
{
    STAFString STAFAtom;

    STAFString globalStr = "";

    if (STAFUtilWin32GetWinType() & kSTAFWin2KPlus)
    {
        // To support Terminal Server and Remote Desktop Connection,
        // need to prepend Global\\ for Win2K and above.  If Global\\ is
        // not specified, when a user logs into TS or RDC (and they are
        // already logged in and running STAFProc with the same STAF instance
        // name), the call to CreateEvent will succeed but GetLastError will be
        // 0 instead of ERROR_ALREADY_EXISTS, and we will incorrectly continue
        // with STAFProc initialization and kill the local interface for the
        // existing instance of STAFProc.

        globalStr = "Global\\";
    }

    STAFAtom = globalStr + *gSTAFInstanceNamePtr + "/Started/Atom" + 
               STAFString(kUTF8_NULL);

    sSTAFProcAtomHandle = CreateEvent(
        0, TRUE, FALSE, STAFAtom.toCurrentCodePage()->buffer());

    if (sSTAFProcAtomHandle == 0)
    {
        osRC = GetLastError();
        errorBuffer = "Error getting STAFProc atom";
        return 1;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        osRC = GetLastError();
        errorBuffer = "STAFProc already started";
        CloseHandle(sSTAFProcAtomHandle);
        return 1;
    }

    BOOL rc = SetConsoleCtrlHandler(CtrlHandler, TRUE);

    if (rc == FALSE)
    {
        osRC = GetLastError();
        errorBuffer = "Error setting console handlers";
        CloseHandle(sSTAFProcAtomHandle);
        CloseHandle(sDataDirAtomHandle);
        return 1;
    }

    return 0;
}


unsigned int STAFProcOSLockDataDir(STAFString &errorBuffer, unsigned int &osRC)
{
    STAFString DataDirAtom;
    
    DataDirAtom = *gSTAFWriteLocationPtr + "/DataDir/Atom" + 
                  STAFString(kUTF8_NULL);

    DataDirAtom = DataDirAtom.replace(kUTF8_BSLASH, kUTF8_HYPHEN);
    
    sDataDirAtomHandle = CreateEvent(
        0, TRUE, FALSE, DataDirAtom.toCurrentCodePage()->buffer());

    if (sDataDirAtomHandle == 0)
    {
        osRC = GetLastError();
        errorBuffer = "Error obtaining a lock on data directory " +
            *gSTAFWriteLocationPtr +
            " because cannot get the data directory atom";
        CloseHandle(sSTAFProcAtomHandle);
        return 1;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        osRC = GetLastError();
        errorBuffer = "Error obtaining a lock on data directory " +
            *gSTAFWriteLocationPtr +
            " because this DATADIR is being used by another instance "
            "of STAFProc";
        CloseHandle(sSTAFProcAtomHandle);
        return 1;
    }

    return 0;
}


unsigned int STAFProcOSTerm(STAFString &errorBuffer, unsigned int &osRC)
{
    if (sSTAFProcAtomHandle != 0) CloseHandle(sSTAFProcAtomHandle);
    if (sDataDirAtomHandle != 0) CloseHandle(sDataDirAtomHandle);
    return 0;
}

