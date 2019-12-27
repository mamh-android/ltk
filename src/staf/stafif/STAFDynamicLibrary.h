/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DynamicLibrary
#define STAF_DynamicLibrary

#include "STAFString.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

typedef struct STAFDynamicLibraryImplementation *STAFDynamicLibrary_t;

/***********************************************************************/
/* STAFDynamicLibraryOpen - Creates a STAF Mutex sempahore             */
/*                                                                     */
/* Accepts: (Out) Pointer to a dynamic library                         */
/*          (In)  The name of the dynamic library                      */
/*          (Out) Pointer to STAFString_t (if RC=kSTAFBaseOSError,     */
/*                this will contain an OS dependent message, in which  */
/*                case it must be freed with STAFStringDestruct().     */ 
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFDynamicLibraryOpen(STAFDynamicLibrary_t *pDynaLib,
                                const char *name, STAFString_t *osMessage);

/***********************************************************************/
/* STAFDynamicLibraryGetAddress - Retrieves an address from a dynamic  */
/*                                library by name.                     */
/*                                                                     */
/* Accepts: (In)  A dynamic library                                    */
/*          (In)  The name of the address to get                       */
/*          (Out) A pointer to the address to be set                   */
/*          (Out) Pointer to STAFString_t (if RC=1, this will contain  */
/*                an OS dependent message, in which case it must be    */
/*                with STAFStringDestruct().                           */ 
/*                an OS dependent message. This must be freed with     */
/*                freed with STAFStringDestruct().                     */ 
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFDynamicLibraryGetAddress(STAFDynamicLibrary_t dynaLib,
                                      const char *name, void **address,
                                      STAFString_t *osMessage);

/***********************************************************************/
/* STAFDynamicLibraryClose - Closes a Dynamic Library                  */
/*                                                                     */
/* Accepts: (In)  Pointer to a dynamic library                         */
/*          (Out) Pointer to STAFString_t (if RC=1, this will contain  */
/*                an OS dependent message, in which case it must be    */
/*                freed with STAFStringDestruct().                     */ 
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFDynamicLibraryClose(STAFDynamicLibrary_t *pDynaLib,
                                 STAFString_t *osMessage);

/* End C language definitions */

#ifdef __cplusplus
}

// Begin C++ language definitions

#include "STAFException.h"

// STAFDynamicLibrary - This class provides a C++ wrapper around the STAF
//                      Dynamic Library C APIs.

class STAFDynamicLibrary
{
public:

    STAFDynamicLibrary(const char *libraryName);

    void *getAddress(const char *name);

    ~STAFDynamicLibrary();

private:

    // Don't allow copy or assignment
    STAFDynamicLibrary(const STAFDynamicLibrary &);
    STAFDynamicLibrary &operator=(const STAFDynamicLibrary &);

    STAFDynamicLibrary_t fDynaLibImpl;
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFDynamicLibraryInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
