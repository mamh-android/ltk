/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXInvalidXMLElementException extends STAXException
{
    public STAXInvalidXMLElementException(String message)
    {
        super(message);
    }

    public STAXInvalidXMLElementException(String message,
                                          STAXActionDefaultImpl action)
    {
        super(message, action);
    }
}
