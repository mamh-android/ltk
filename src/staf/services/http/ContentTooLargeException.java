package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: ContentTooLargeException                                           */
/* Description: This Exception indicates that cannot set the session content */
/*              because it is too large and caused an OutofMemoryError.      */
/*                                                                           */
/*****************************************************************************/

public class ContentTooLargeException extends Exception 
{
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: s - additional message information                             */
/*                                                                           */
/*****************************************************************************/    

    public ContentTooLargeException(String s)
    {
        super(s);
    }
}
