package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: InvalidCookieIDException                                           */
/* Description: This Exception indicates that the cookie is not present in   */
/*              the current session.                                         */
/*                                                                           */
/*****************************************************************************/
public class InvalidCookieIDException extends Exception {

    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: id - the invalid cookie id that was requested                  */
/*            s - additional message information                             */
/*                                                                           */
/*****************************************************************************/    
    
    public InvalidCookieIDException(String id, String s) {
        super("Invalid Cookie name " + id + "\n" + s);
    }

}
