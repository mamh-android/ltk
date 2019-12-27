/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_InternalUtil
#define STAF_InternalUtil

#ifdef __cplusplus

#include "STAFString.h"

inline STAFString getExceptionString(STAFException &e,
                                     const char *caughtWhere = 0)
{
    STAFString result;

    if (caughtWhere)
    {
        result += STAFString("In ") + caughtWhere + STAFString(", ");
    }

    result += STAFString("Name: ") + e.getName();
    result += STAFString(", Location: ") + e.getLocation();
    result += STAFString(", Text: ") + e.getText();
    result += STAFString(", Error code: ") + e.getErrorCode();

    return result;
}

#endif

#endif
