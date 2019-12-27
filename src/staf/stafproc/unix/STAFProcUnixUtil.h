/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ProcUnixUtil
#define STAF_ProcUnixUtil

#include "STAFString.h"

// Note: These functions are actually defined in STAFProcOSUtil.cpp


// this macro takes the mode of a file (member of a
// struct stat) and determines whether the file has
// execute permissions or not. the naming convention
// of this macro follows the naming convention of o-
// ther macros used within sys/stat.h.

#define S_ISEXE(mode) (mode & 0x49)

// this function returns the path of "file" as found in
// the PATH environment variable and returns it in the 
// "path" string. the path will have no terminating "/"
// and if a path for the file is not found, an error me-
// ssage will be returned in "path" instead. on success
// a 1 is returned, and on failure a 0 is returned.

int getFilePath(const STAFString &file, STAFString &path);

#endif
