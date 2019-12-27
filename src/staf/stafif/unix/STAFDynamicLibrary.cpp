/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <dlfcn.h>
#include "STAFDynamicLibrary.h"
#include "STAFMutexSem.h"
#include "STAFString.h"

#define STRINGIFY_X(a) #a
#define STRINGIFY(a) STRINGIFY_X(a) 

// XXX: We don't return anything useful via osRC here, as the dl() calls
//      don't give meaningful return codes.  We might want to consider
//      opening up these interfaces to allow the return of strings, so that
//      we could return the result of dlerror().

struct STAFDynamicLibraryImplementation
{
    void *fDynaLibHandle;
};


STAFMutexSem sDLErrorSem;

STAFRC_t STAFDynamicLibraryOpen(STAFDynamicLibrary_t *pDynaLib,
                                const char *name, STAFString_t *osMessage)
{
    STAFRC_t rc = kSTAFOk;

    if (pDynaLib == 0) return kSTAFInvalidObject;

    try
    {
        *pDynaLib = new STAFDynamicLibraryImplementation;
        STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;
        STAFMutexSemLock lock(sDLErrorSem);

        STAFString theName(name);
        STAFString thePrefix(STRINGIFY(STAF_SHARED_LIB_PREFIX));
        STAFString theSuffix(STRINGIFY(STAF_SHARED_LIB_SUFFIX));

        if (theName.find(kUTF8_SLASH) == STAFString::kNPos)
        {
            if (theName.find(thePrefix) != 0)
                theName = thePrefix + theName;

            if (theName.find(theSuffix) !=
                (theName.length() - theSuffix.length()))
            {
                theName = theName + theSuffix;
            }
        }

        // Changed to use RTLD_LOCAL, instead of RTLD_GLOBAL, on dlopen()
        // to fix a problem on Linux, Solaris, and FreeBSD, where a function
        // from the wrong connetion provider would sometimes be called (e.g.
        // from the local connprovider instead of from the TCP connprovider)
        // which resulted in problems using secure TCP since the functions
        // are different.
        dynaLib.fDynaLibHandle = dlopen(theName.toCurrentCodePage()->buffer(),
                                        RTLD_NOW | RTLD_LOCAL);

        if (dynaLib.fDynaLibHandle == 0)
        {
            if (osMessage)
            {
                const char *error = dlerror();
                STAFString theError(error);
                *osMessage = theError.adoptImpl();
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

    STAFMutexSemLock lock(sDLErrorSem);

    *address = dlsym(dynaLib->fDynaLibHandle, name);

    if (*address == 0)
    {
        const char *error = dlerror();

        if (error != 0)
        {
            if (osMessage)
            {
                STAFString theError(error);
                *osMessage = theError.adoptImpl();
            }

            return kSTAFBaseOSError;
        }
    }

    return kSTAFOk;
}



STAFRC_t STAFDynamicLibraryClose(STAFDynamicLibrary_t *pDynaLib,
                                 STAFString_t *osMessage)
{
    if (pDynaLib == 0) return kSTAFInvalidObject;

    STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;

    STAFMutexSemLock lock(sDLErrorSem);

    int rc = dlclose(dynaLib.fDynaLibHandle);

    delete *pDynaLib;
    *pDynaLib = 0;

    if (rc != 0)
    {
        if (osMessage)
        {
            const char *error = dlerror();
            STAFString theError(error);
            *osMessage = theError.adoptImpl();
        }

        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}
