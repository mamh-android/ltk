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
import org.w3c.dom.html.HTMLAnchorElement;
import org.w3c.dom.html.HTMLAreaElement;

/*****************************************************************************/
/*                                                                           */
/* Class: WebLink                                                            */
/* Description: This class provides the handle to manipulate html links.     */
/*                                                                           */
/*****************************************************************************/

public class WebLink extends WebElement
{    
    String target;
    public static final String HREF="HREF";

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: id - id for the link                                           */
/*            name - name for the link                                       */
/*            index - index of this element in the list of elements of this  */
/*                    type                                                   */
/*            href - href target for the link                                */
/*                                                                           */
/*****************************************************************************/    

    public WebLink(String id, String name, int index, String href)
    {
        super (id, name, index);
        this.target = href;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLElement that provides the href for the link      */
/*            index - index of this element in the list of elements of this  */
/*                    type                                                   */
/*                                                                           */
/*****************************************************************************/    
    
    public WebLink(HTMLAnchorElement element, int index)
    {
        this(element.getId(), element.getAttribute("name"), index, 
             element.getHref());
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLElement that provides the href for the link      */
/*            index - index of this element in the list of elements of this  */
/*                    type                                                   */
/*                                                                           */
/*****************************************************************************/    
    
    public WebLink(HTMLAreaElement element, int index)
    {
        this(element.getId(), element.getAttribute("name"), index, 
             element.getHref());
    }
    
    public void setHref (String url)
    {
        target = url;
    }
    
    public String getHref ()
    {
        return target;
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
    
    public HashMap getRequest()
    {
        HashMap request = new HashMap();
        
        request.put(WebSession.REQUEST_METHOD, "GET");
        request.put(WebSession.REQUEST_URL, target);
        request.put(WebSession.REQUEST_PARAMETERS, null);
        request.put(WebSession.REQUEST_HEADERS, null);
        request.put(WebSession.REQUEST_FILES, null);
        request.put(WebSession.REQUEST_CONTENT, null);
        
        return request;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getSummary                                                        */
/* Description: get a summary of the link.                                   */
/* Parameters: none                                                          */
/* Returns: description of a link                                            */
/*                                                                           */
/*****************************************************************************/    

    public HashMap getSummary()
    {
        HashMap summary = new HashMap();
        
        summary.put(WebElement.NAME, name);
        summary.put(WebElement.ID, id);
        summary.put(WebElement.INDEX, new Integer(index));
        summary.put(HREF, target);
        
        return summary;
    }    
    
}// end class WebLink

