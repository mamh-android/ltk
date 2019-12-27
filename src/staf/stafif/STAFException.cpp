/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFException.h"
#include "STAFTrace.h"

void STAFException::trace(const char *caughtWhere)
{
    STAFException::trace(kSTAFTraceError, caughtWhere);
}

void STAFException::trace(unsigned int tracePoint, const char *caughtWhere)
{
    char errorMsg[1080] = { 0 };

    if (caughtWhere)
    {
        if (getLocation()[0] != 0)
        {
            sprintf(errorMsg, "Caught STAFException in %s, "
                    "Exception: %s, Location: %s, Text: %s, Error code: %d",
                    caughtWhere, getName(), getLocation(), getText(),
                    getErrorCode());
        }
        else
        {
            sprintf(errorMsg, "Caught STAFException in %s, "
                    "Exception: %s, Text: %s, Error code: %d",
                    caughtWhere, getName(), getText(), getErrorCode());
        }
    }
    else
    {
        if (getLocation()[0] != 0)
        {
            sprintf(errorMsg, "Caught STAFException, "
                    "Exception: %s, Location: %s, Text: %s, Error code: %d",
                    getName(), getLocation(), getText(), getErrorCode());
        }
        else
        {
            sprintf(errorMsg, "Caught STAFException, "
                    "Exception: %s, Text: %s, Error code: %d",
                    getName(), getText(), getErrorCode());
        }
    }

    STAFTrace::trace(static_cast<STAFTracePoint_t>(tracePoint), errorMsg);
}
