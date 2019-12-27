package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: InvalidElementIDException                                          */
/* Description: This Exception indicates that the id does not correspond to  */
/*              an element.                                                  */
/*                                                                           */
/*****************************************************************************/

public class InvalidElementIDException extends Exception 
{
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: id - the invalid elment id that was requested                  */
/*            s - additional message information                             */
/*                                                                           */
/*****************************************************************************/    

    public InvalidElementIDException(Object id, String s) 
    {
        super("Invalid ID " + id.toString() + "\n" + s);
    }

}



