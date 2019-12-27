/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

/**
 * This is an exception class that is used/thrown by the <code>STAFHandle</code>
 * class. The <code>rc</code> allows you to access the actual return code
 * issued by STAF. You can use the standard Throwable method
 * <code>GetMessage</code> to retrieve any extra data regarding the exception.
 */
public class STAFException extends Exception
{
    /**
     *  Class constructor that creates a new <code>STAFException</code> instance
     *  assigning STAF return code 0 with no additional information regarding
     *  the exception.
     */ 
    public STAFException() { super(); rc = 0; }

    /**
     * Class constructor that creates a new <code>STAFException</code> instance
     * assigning the STAF return code passed in and no additional information
     * regarding the exception.
     * 
     * @param  theRC  the STAF return code
     */ 
    public STAFException(int theRC) { super(); rc = theRC; }

    /**
     * Class constructor that creates a new <code>STAFException</code> instance
     * assigning the STAF return code passed in and additional information
     * regarding the exception.
     * 
     * @param  theRC  the STAF return code
     * @param  s      a string containing information regarding the exception
     */ 
    public STAFException(int theRC, String s) { super(s); rc = theRC; }

    /**
     * The actual return code issued by STAF.
     */ 
    public int rc;
}
