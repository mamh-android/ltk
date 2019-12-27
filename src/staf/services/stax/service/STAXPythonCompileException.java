/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXPythonCompileException extends STAXException
{
    public STAXPythonCompileException(String message)
    {
        super(message);
    }

    public STAXPythonCompileException(String message,
                                      STAXActionDefaultImpl action)
    {
        super(message, action);
    }
}
