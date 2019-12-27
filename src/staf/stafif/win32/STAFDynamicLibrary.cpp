/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFDynamicLibrary.h"
#include "STAFUtilWin32.h"

struct STAFDynamicLibraryImplementation
{
    HINSTANCE fDynaLibHandle;
};


STAFRC_t STAFDynamicLibraryOpen(STAFDynamicLibrary_t *pDynaLib,
                                const char *name, STAFString_t *osMessage)
{
    STAFRC_t rc = kSTAFOk;

    if (pDynaLib == 0) return kSTAFInvalidObject;

    try
    {
        *pDynaLib = new STAFDynamicLibraryImplementation;
        STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;
        dynaLib.fDynaLibHandle = LoadLibrary(name);

        if (dynaLib.fDynaLibHandle == 0)
        {
            if (osMessage)
            {
                int systemErrorCode = GetLastError();
                STAFString_t systemErrorMsg = 0;
                
                STAFUtilWin32LookupSystemErrorMessage(
                    systemErrorCode, &systemErrorMsg);

                STAFString errorMsg = STAFString(name) +
                    ": Cannot open library file (aka module/DLL), OS RC " +
                    STAFString(systemErrorCode);

                STAFString systemErrorString = STAFString(
                    systemErrorMsg, STAFString::kShallow);

                if (systemErrorString.length() != 0)
                    errorMsg += ": " + STAFString(systemErrorString);

                *osMessage = errorMsg.adoptImpl();
            }

            delete *pDynaLib;
            return kSTAFBaseOSError;
        }
    }
    catch (...)
    {
        rc = kSTAFUnknownError;

        if (osMessage)
        {
            STAFString theError("Caught unknown exception");
            *osMessage = theError.adoptImpl();
        }
    }

    return rc;
}


STAFRC_t STAFDynamicLibraryGetAddress(STAFDynamicLibrary_t dynaLib,
                                      const char *name, void **address,
                                      STAFString_t *osMessage)
{
    if (dynaLib == 0) return kSTAFInvalidObject;
    if (address == 0) return kSTAFInvalidParm;

    *address = GetProcAddress(dynaLib->fDynaLibHandle, name);

    if (*address == 0)
    {
        if (osMessage)
        {
            int systemErrorCode = GetLastError();
            STAFString_t systemErrorMsg = 0;

            STAFUtilWin32LookupSystemErrorMessage(
                systemErrorCode, &systemErrorMsg);

            STAFString errorMsg = STAFString(name) +
                ": Error getting library address, OS RC " +
                STAFString(systemErrorCode);

            STAFString systemErrorString = STAFString(
                systemErrorMsg, STAFString::kShallow);

            if (systemErrorString.length() != 0)
                errorMsg += ": " + STAFString(systemErrorString);

            *osMessage = errorMsg.adoptImpl();
        }

        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}


STAFRC_t STAFDynamicLibraryClose(STAFDynamicLibrary_t *pDynaLib,
                                 STAFString_t *osMessage)
{
    if (pDynaLib == 0) return kSTAFInvalidObject;

    STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;

    unsigned int rc = FreeLibrary(dynaLib.fDynaLibHandle);

    delete *pDynaLib;
    *pDynaLib = 0;

    if (rc != 0)
    {
        if (osMessage)
        {
            int systemErrorCode = GetLastError();
            STAFString_t systemErrorMsg = 0;

            STAFUtilWin32LookupSystemErrorMessage(
                systemErrorCode, &systemErrorMsg);

            STAFString errorMsg =
                STAFString(": Error freeing library, OS RC ") +
                STAFString(systemErrorCode);

            STAFString systemErrorString = STAFString(
                systemErrorMsg, STAFString::kShallow);

            if (systemErrorString.length() != 0)
                errorMsg += ": " + STAFString(systemErrorString);

            *osMessage = errorMsg.adoptImpl();
        }

        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}
