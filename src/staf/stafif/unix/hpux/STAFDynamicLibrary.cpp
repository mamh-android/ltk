/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <dl.h>
#include "STAFDynamicLibrary.h"
#include "STAFMutexSem.h"
#include "STAFString.h"
#include <map>

#define STRINGIFY_X(a) #a
#define STRINGIFY(a) STRINGIFY_X(a) 

struct STAFDynamicLibraryImplementation
{
    shl_t fDynaLibHandle;
    STAFString fName;
};

typedef std::map<STAFString, unsigned int> STAFLibMap;
STAFLibMap fLibRefCountMap;
STAFMutexSem fLibSem;

STAFRC_t STAFDynamicLibraryOpen(STAFDynamicLibrary_t *pDynaLib,
                                const char *name, STAFString_t *osMessage)
{
    STAFRC_t rc = kSTAFOk;

    if (pDynaLib == 0) return kSTAFInvalidObject;

    try
    {
        *pDynaLib = new STAFDynamicLibraryImplementation;
        STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;

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

        dynaLib.fName = theName;

        dynaLib.fDynaLibHandle = shl_load(theName.toCurrentCodePage()->buffer(),
                                          BIND_IMMEDIATE | DYNAMIC_PATH, 0);
    
        if (dynaLib.fDynaLibHandle == 0)
        {
            if (osMessage) *osMessage = STAFString(errno).adoptImpl();

            delete *pDynaLib;
            return kSTAFBaseOSError;
        }
        else
        {
            STAFMutexSemLock libLock(fLibSem);

            if (fLibRefCountMap.find(theName) == fLibRefCountMap.end())
            {
                fLibRefCountMap[theName] = 0;
            }

            fLibRefCountMap[theName] = fLibRefCountMap[theName]++;
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

    int rc = shl_findsym(&dynaLib->fDynaLibHandle, name, TYPE_UNDEFINED,
                         address);

    if (rc != 0)
    {
        if (osMessage) *osMessage = STAFString(errno).adoptImpl();

        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}



STAFRC_t STAFDynamicLibraryClose(STAFDynamicLibrary_t *pDynaLib,
                                 STAFString_t *osMessage)
{
    if (pDynaLib == 0) return kSTAFInvalidObject;

    STAFDynamicLibraryImplementation &dynaLib = **pDynaLib;

    STAFString theName(dynaLib.fName);

    STAFMutexSemLock libLock(fLibSem);

    if (fLibRefCountMap.find(theName) != fLibRefCountMap.end())
    {
        if (--fLibRefCountMap[theName] != 0)
        {
            return kSTAFOk;
        }
    }

    int rc = shl_unload(dynaLib.fDynaLibHandle);

    delete *pDynaLib;
    *pDynaLib = 0;

    if (rc != 0)
    {
        if (osMessage) *osMessage = STAFString(errno).adoptImpl();

        return kSTAFBaseOSError;
    }

    return kSTAFOk;
}
