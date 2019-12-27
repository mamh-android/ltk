package com.ibm.staf.service.http.html;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: InvalidParameterValueException                                     */
/* Description: This Exception indicates that the value specified was not    */
/*              valid for the parameter.                                     */
/*                                                                           */
/*****************************************************************************/

public class InvalidParameterValueException extends Exception 
{
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: value - the invalid parameter value id that was specified      */
/*                                                                           */
/*****************************************************************************/    

    public InvalidParameterValueException(String value)
    {
        super("Invalid parameter value " + value);
    }

}
