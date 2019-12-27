/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_UtilWin32
#define STAF_UtilWin32

#include "STAF.h"
#include "STAFString.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Define Windows Operating System Types
typedef enum STAFWinType_e
{
    kSTAFWin95        = 0x0001,
    kSTAFWin98        = 0x0002,
    kSTAFWinMe        = 0x0004,
    kSTAFWin9XUnknown = 0x0008,
    kSTAFWinNT        = 0x0010,
    kSTAFWin2K        = 0x0020,
    kSTAFWinXP        = 0x0040,
    kSTAFWinNTUnknown = 0x0080,
    kSTAFWin32Unknown = 0x0100,
    kSTAFWin2K3       = 0x0200,
    kSTAFWinVista     = 0x0400,
    kSTAFWinSrv2008   = 0x0800,
    kSTAFWin7         = 0x1000,
    kSTAFWinSrv2008R2 = 0x2000,
    kSTAFWin8         = 0x4000,
    kSTAFWinSrv2012   = 0x8000,

    // Define common combinations that can be used as a mask to
    // determine if the Windows type matches.
    kSTAFWin9X        = (0x0001 | 0x0002 | 0x0004 | 0x0008),
    kSTAFWinNTPlus    = (0x0010 | 0x0020 | 0x0040 | 0x0200 | 0x0400 | 0x0800 |
                         0x1000 | 0x2000 | 0x4000 | 0x8000 | 0x0080),
    kSTAFWinNT2000    = (0x0010 | 0x0020),  
    kSTAFWin2KPlus    = (0x0020 | 0x0040 | 0x0200 | 0x0400 | 0x0800 | 0x1000 |
                         0x2000 | 0x4000 | 0x8000 | 0x0080),
    kSTAFWinXPPlus    = (0x0040 | 0x0200 | 0x0400 | 0x0800 | 0x1000 | 0x2000 |
                         0x4000 | 0x8000 | 0x0080),
    kSTAFWin2K3Plus   = (0x0200 | 0x0400 | 0x0800 | 0x1000 | 0x2000 | 0x4000 |
                         0x8000 | 0x0080),
    kSTAFWinVistaPlus = (0x0400 | 0x0800 | 0x1000 | 0x2000 | 0x4000 | 0x8000 |
                         0x0080),
    kSTAFWinSrv2008Plus = (0x0800 | 0x1000 | 0x2000 | 0x4000 | 0x8000 | 0x0080)

} STAFWinType_t;

/*********************************************************************/
/* STAFUtilWin32GetWinType - Gets the Windows type                   */
/*                                                                   */
/* Returns:  the Windows type (e.g. kSTAFWin95, kSTAFWin2K, etc.)    */
/*********************************************************************/
unsigned int STAFUtilWin32GetWinType();

/*********************************************************************/
/* STAFUtilWin32CreateNullSD - Creates a null Security Descriptor    */
/*                                                                   */
/* Accepts: (In)  A pointer to a security descriptor                 */
/*                                                                   */
/* Returns:  0, if successful                                        */
/*          >0, if unsuccessful                                      */
/*********************************************************************/
unsigned int STAFUtilWin32CreateNullSD(PSECURITY_DESCRIPTOR *sd);

/*********************************************************************/
/* STAFUtilWin32DeleteNullSD - Deletes a null Security Descriptor    */
/*                                                                   */
/* Accepts: (In)  A pointer to a security descriptor                 */
/*                                                                   */
/* Returns:  0                                                       */
/*********************************************************************/
unsigned int STAFUtilWin32DeleteNullSD(PSECURITY_DESCRIPTOR *sd);

/*******************************************************************************/
/* STAFUtilWin32LookupSystemErrorMessage - This function looks up the error    */
/* message for the specified Windows system error code.                        */
/*                                                                             */
/* Accepts: (IN)  System error code                                            */
/*          (OUT) Error message                                                */
/*                                                                             */
/* Returns: Nothing                                                            */
/*******************************************************************************/
void STAFUtilWin32LookupSystemErrorMessage(unsigned int errorCode,
                                           STAFString_t *errorMsg); 

#ifdef __cplusplus
}
#endif

#endif
