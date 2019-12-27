/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DynamicLibraryInlImpl
#define STAF_DynamicLibraryInlImpl

#include "STAF.h"
#include "STAFDynamicLibrary.h"
#include "STAFException.h"

STAF_INLINE STAFDynamicLibrary::STAFDynamicLibrary(const char *libraryName)
    : fDynaLibImpl(0)
{
    STAFString_t error = 0; 
    STAFRC_t rc = STAFDynamicLibraryOpen(&fDynaLibImpl, libraryName,
                                         &error);
    if (rc)
    {
        STAFString errorMsg("STAFDynamicLibraryOpen");

        if (rc == kSTAFBaseOSError)
            errorMsg += ": " + STAFString(error, STAFString::kShallow);

        STAFException se(errorMsg.toCurrentCodePage()->buffer(), rc);
        THROW_STAF_EXCEPTION(se);
    }
}


STAF_INLINE void *STAFDynamicLibrary::getAddress(const char *name)
{
    STAFString_t error = 0; 
    void *address = 0;
    STAFRC_t rc = STAFDynamicLibraryGetAddress(fDynaLibImpl, name, &address,
                                               &error);
    if (rc)
    {
        STAFString errorMsg("STAFDynamicLibraryGetAddress");

        if (rc == kSTAFBaseOSError)
            errorMsg += ": " + STAFString(error, STAFString::kShallow);

        STAFException se(errorMsg.toCurrentCodePage()->buffer(), rc);
        THROW_STAF_EXCEPTION(se);
    }

    return address;
}


STAF_INLINE STAFDynamicLibrary::~STAFDynamicLibrary()
{
  STAFString_t error = 0;
  unsigned int osRC = 0;
  STAFRC_t rc = STAFDynamicLibraryClose(&fDynaLibImpl, &error);

  if (rc == kSTAFBaseOSError) STAFStringDestruct(&error, &osRC);
}

#endif
