/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2010                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXExceedsMaxThreadsException extends STAXException
{
    public STAXExceedsMaxThreadsException(String message)
    {
        super(message);
    }

    public STAXExceedsMaxThreadsException(String message,
                                          STAXActionDefaultImpl action)
    {
        super(message, action);
    }
}
