/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;

/**
 * This is a generic Exception for the Namespace service.
 */
public class NSException extends Exception
{
    /**
     * Constructor
     */
    public NSException()
    {
        super();
    }

    /**
     * Constructor
     * @param message
     */
    public NSException(String message)
    {
        super(message);
    }
}
