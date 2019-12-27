/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <dll.h>
#include "STAFDynamicLibrary.h"
#include "STAFMutexSem.h"
#include "STAFString.h"

#define STRINGIFY_X(a) #a
#define STRINGIFY(a) STRINGIFY_X(a) 

struct STAFDynamicLibraryImplementation
{
    dllhandle *fDynaLibHandle;
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

        dynaLib.fDynaLibHandle = dllload(theName.toCurrentCodePage()->buffer());
        if (dynaLib.fDynaLibHandle == NULL)
        {
            if (osMessage)
            {
                const char *error = strerror(errno);
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

    // Look for a function symbol first, if that fails look for a variable symbol.
    if ((*address = (void *)dllqueryfn(dynaLib->fDynaLibHandle, name)) == NULL)
        *address = dllqueryvar(dynaLib->fDynaLibHandle, name);

    if (*address == NULL)
    {
        const char *error = strerror(errno);

        if (error != NULL)
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

    int rc = dllfree(dynaLib.fDynaLibHandle);

    delete *pDynaLib;
    *pDynaLib = 0;

    if (rc != 0)
    {
        if (osMessage)
        {
            const char *error = strerror(errno);
            STAFString theError(error);
            *osMessage = theError.adoptImpl();
        }

        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}
