/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFDynamicLibrary.h"

struct STAFDynamicLibraryImplementation
{
    HMODULE fDynaLibHandle;
};


unsigned int STAFDynamicLibraryOpen(STAFDynamicLibrary_t *pDynaLib,
                                    const char *name, STAFString_t *osMessage)
{
    unsigned int rc = 0;

    if (pDynaLib == 0) return 2;

    try
    {
        *pDynaLib = new STAFDynamicLibraryImplementation;
        STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;
        char error[256] = 0;
        rc = DosLoadModule((PSZ)error, sizeof(error), (PSZ)name,
                           &dynaLib.fDynaLibHandle);
        if (rc != 0)
        {
            if (osMessage)
            {
                STAFString message(rc);
                message += STAFString(":") + error;
                *osMessage = message.adoptImpl();
            }

            delete *pDynaLib;
            return 1;
        }
    }
    catch (...)
    { rc = 1; if (osRC) *osRC = 0xFFFFFFFF; }

    return rc;
}


unsigned int STAFDynamicLibraryGetAddress(STAFDynamicLibrary_t dynaLib,
                                          const char *name, void **address,
                                          STAFString_t *osMessage)
{
    if (dynaLib == 0) return 2;

    unsigned int rc = DosQueryProcAddr(dynaLib->fDynaLibHandle, 0, name,
                                       address);
    if (rc != 0)
    {
        if (osMessage) *osMessage = STAFString(rc).adoptImpl();
        return 1;
    }

    return 0;
}


unsigned int STAFDynamicLibraryClose(STAFDynamicLibrary_t *pDynaLib,
                                     STAFString_t *osMessage)
{
    if (pDynaLib == 0) return 2;

    STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;

    unsigned int rc = DosFreeModule(dynaLib.fDynaLibHandle);

    delete *pDynaLib;
    *pDynaLib = 0;

    if (rc != 0)
    {
        if (osMessage) *osMessage = STAFString(rc).adoptImpl();
        return 1;
    }

    return 0;
}
