package com.ibm.staf.service.http.html;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import java.util.HashMap;
import java.util.Vector;
// xerces
import com.ibm.staf.service.http.WebSession;
import org.w3c.dom.html.HTMLInputElement;
import org.w3c.dom.html.HTMLCollection;
import org.w3c.dom.html.HTMLFormElement;
import org.w3c.dom.html.HTMLAnchorElement;
import org.w3c.dom.html.HTMLAreaElement;

/*****************************************************************************/
/*                                                                           */
/* Class: WebElement                                                         */
/* Description: This class provides the handle to manipulate html document   */
/*              components that can be transformed into http requests.       */
/*                                                                           */
/*****************************************************************************/

public abstract class WebElement 
{    
    String id;
    String name;
    int index;
    public static final String NAME="NAME";
    public static final String ID="ID";
    public static final String INDEX="INDEX";
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: none                                                           */
/*                                                                           */
/*****************************************************************************/    
    
    WebElement()
    {
        name = null;
        id = null;
        index = -1;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: id - id of this element                                        */
/*            name - name of this element                                    */
/*            index - index of this element in the list of elements of this  */
/*                    type                                                   */
/*                                                                           */
/*****************************************************************************/    
    
    WebElement(String id, String name, int index)
    {
        this.name = name;
        this.id = id;
        this.index = index;
    }
        
    public String getName()
    {
        return name;
    }
    
    public String getId()
    {
        return id;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getRequest                                                        */
/* Description: get the hash describing the http request associated with this*/
/*              html element.                                                */
/* Parameters: none                                                          */
/* Returns: description of a http request                                    */
/*                                                                           */
/*****************************************************************************/    

    public abstract HashMap getRequest();
    
/*****************************************************************************/
/*                                                                           */
/* Method: getSummary                                                        */
/* Description: get a summary of the link.                                   */
/* Parameters: none                                                          */
/* Returns: description of a element                                         */
/*                                                                           */
/*****************************************************************************/    

    public abstract HashMap getSummary();

} // end class WebElement
