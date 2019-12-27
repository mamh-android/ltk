/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_UnixUtil
#define STAF_UnixUtil

#include "STAFString.h"

/*****************************************************************************/
/* S_ISEXE - This macro takes the mode of a file (member of a struct stat)   */
/*           and determines whether the file has execute permissions or not. */
/*           The naming convention of this macro follows the naming          */
/*           convention of other macros used within sys/stat.h.              */
/*****************************************************************************/
#define S_ISEXE(mode) (mode & 0x49)


/*****************************************************************************/
/* STAFUtilUnixGetFilePath - Determines the path of a file as it is found in */
/*                           the PATH environment variable.  The path        */
/*                           returned will have no terminating "/", and, if  */
/*                           the file is not found in the PATH, an error     */
/*                           message will returned in the path string        */
/*                           instead.                                        */
/*                                                                           */
/* Accepts: (IN)  The file to find in the PATH                               */
/*          (OUT) Pointer to the path of the file                            */
/*          (OUT) Pointer to operating system return code                    */
/*                                                                           */
/* Returns:  0, if successful                                                */
/*          >1, if unsuccessul                                               */
/*                                                                           */
/*                                                                           */
/* Notes: 1) You are responsible for destructing the string returned for the */
/*           path                                                            */
/*****************************************************************************/
STAFRC_t STAFUtilUnixGetFilePath(STAFStringConst_t file, STAFString_t *path,
                                 unsigned int *osRC);

#endif
