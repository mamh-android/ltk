/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXInvalidStartFunctionException extends STAXException
{
    public STAXInvalidStartFunctionException(String message)
    {
        super(message);
    }

    public STAXInvalidStartFunctionException(String message,
                                             STAXActionDefaultImpl action)
    {
        super(message, action);
    }
}
