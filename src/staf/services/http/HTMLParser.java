package com.ibm.staf.service.http.html;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import java.io.StringReader;
import java.io.IOException;

import java.util.HashMap;
import java.io.InputStream;

import com.ibm.staf.service.http.InvalidElementIDException;

import org.cyberneko.html.parsers.DOMParser;
import org.w3c.dom.html.HTMLDocument;
// xerces
import org.xml.sax.SAXException;
import org.xml.sax.InputSource;

import org.w3c.dom.html.HTMLCollection;
import org.w3c.dom.html.HTMLFormElement;
import org.w3c.dom.html.HTMLElement;
import org.w3c.dom.html.HTMLAnchorElement;
import org.w3c.dom.html.HTMLAreaElement;

/*****************************************************************************/
/*                                                                           */
/* Class: HTMLParser                                                         */
/* Description: This class provides the handle to manipulate a html document.*/
/*                                                                           */
/*****************************************************************************/

public class HTMLParser 
// XXX  may need to implement org.xml.sax Interface ErrorHandler
{
    
    protected HTMLDocument document;
    protected DOMParser parser;
    
    public static final String EMPTY_DOC = "<html></html>";
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: none                                                           */
/*                                                                           */
/*****************************************************************************/    
    
    public HTMLParser() 
    {
        parser  = new DOMParser();

        try
        {
            // The Xerces HTML DOM implementation does not support namespaces
            // and cannot represent XHTML documents with namespace information.
            // Therefore, in order to use the default HTML DOM implementation
            // with NekoHTML's DOMParser to parse XHTML documents, you must
            // turn off namespace processing. 

            parser.setFeature("http://xml.org/sax/features/namespaces", false);

            setContent (EMPTY_DOC);
            
        }catch (SAXException e)
        { 
            // EMPTY_DOC should not throw any errors
        }catch (IOException e)
        { 
            // EMPTY_DOC should not throw any errors
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setContent                                                        */
/* Description: sets the content to draw elements from                       */
/* Parameters: content - html content to parse and draw elements from        */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void setContent (String content) throws SAXException, 
                                                   IOException
    {
        InputSource bodySource = new InputSource(
            new StringReader(content));
        setContent(bodySource);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setContent                                                        */
/* Description: sets the content to draw elements from                       */
/* Parameters: content - html content to parse and draw elements from        */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
        
    public void setContent (InputStream content) throws SAXException, 
                                                        IOException
    {
        setContent(new InputSource(content));
        content.close();
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setContent                                                        */
/* Description: sets the content to draw elements from                       */
/* Parameters: content - html content to parse and draw elements from        */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
        
    public void setContent (InputSource source) throws SAXException, 
                                                       IOException
    {
        parser.parse(source);
        document = (HTMLDocument)parser.getDocument();
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getTitle                                                          */
/* Description: get the title of the current document                        */
/* Parameters: none                                                          */
/* Returns: the title of the current html document                           */
/*                                                                           */
/*****************************************************************************/    
    
    public String getTitle()
    {
        return document.getTitle();
    }    

/*****************************************************************************/
/*                                                                           */
/* Method: getLinkByID                                                       */
/* Description: get the WebLink identified by with id attribute id.          */
/* Parameters: id - id attribute to identify the link by                     */
/* Returns: html link                                                        */
/*                                                                           */
/*****************************************************************************/    
        
    public WebLink getLinkByID(String id) throws InvalidElementIDException
    {
        HTMLCollection links = document.getLinks();
        
        for(int i = 0; i < links.getLength(); i++)
        {
            if (id.equals( ((HTMLElement) links.item(i)).getId() ))
            {
                if ( org.apache.html.dom.HTMLAnchorElementImpl.class 
                     == links.item(i).getClass())
                return new WebLink((HTMLAnchorElement)links.item(i), i + 1);
                if ( org.apache.html.dom.HTMLAreaElementImpl.class 
                     == links.item(i).getClass())
                return new WebLink((HTMLAreaElement)links.item(i), i + 1);
            }
        }
        
        throw new InvalidElementIDException (id, "Link " + id);
    }
        
/*****************************************************************************/
/*                                                                           */
/* Method: getLinkByName                                                     */
/* Description: get the WebLink identified by with name attribute name.      */
/* Parameters: name - name attribute to identify the form by                 */
/* Returns: html link                                                        */
/*                                                                           */
/*****************************************************************************/    
        
    public WebLink getLinkByName(String name) throws InvalidElementIDException
    {
        HTMLCollection links = document.getLinks();
        
        for(int i = 0; i < links.getLength(); i++)
        {
            if (name.equals( ((HTMLElement) links.item(i)).
                              getAttribute("name") ))
            {
                if ( org.apache.html.dom.HTMLAnchorElementImpl.class 
                     == links.item(i).getClass())
                return new WebLink((HTMLAnchorElement)links.item(i), i + 1);
                if ( org.apache.html.dom.HTMLAreaElementImpl.class 
                     == links.item(i).getClass())
                return new WebLink((HTMLAreaElement)links.item(i), i + 1);
            }
        }
        
        throw new InvalidElementIDException (name, "Link " + name);
    }
    
        
/*****************************************************************************/
/*                                                                           */
/* Method: getLinkByIndex                                                    */
/* Description: get the WebLink at index.                                    */
/* Parameters: index - the index of the link                                 */
/* Returns: html link                                                        */
/*                                                                           */
/*****************************************************************************/    
    
    public WebLink getLinkByIndex(int index) throws InvalidElementIDException
    {
        HTMLCollection links = document.getLinks();
        
        index --;
        
        if ( index < links.getLength() && index > -1 )
        {
            if ( org.apache.html.dom.HTMLAnchorElementImpl.class 
                 == links.item(index).getClass())
            return new WebLink((HTMLAnchorElement)links.item(index), index + 1);
            if ( org.apache.html.dom.HTMLAreaElementImpl.class 
                 == links.item(index).getClass())
            return new WebLink((HTMLAreaElement)links.item(index), index + 1);
        }
        
        throw new InvalidElementIDException (new Integer(index + 1), 
                                              "Link index " + (index + 1));
    }    

/*****************************************************************************/
/*                                                                           */
/* Method: getFormByID                                                       */
/* Description: get the WebForm identified by with id attribute id.          */
/* Parameters: id - id attribute to identify the form by                     */
/* Returns: html form                                                        */
/*                                                                           */
/*****************************************************************************/    
    
    public WebForm getFormByID(String id) throws InvalidElementIDException
    {
        HTMLCollection forms = document.getForms();
        
        for(int i = 0; i < forms.getLength(); i++)
        {
            if (id.equals( ((HTMLElement) forms.item(i)).getId() ))
                return new WebForm((HTMLFormElement)forms.item(i), i + 1);
        }
        
        throw new InvalidElementIDException (id, "Form " + id);        
    }
        
/*****************************************************************************/
/*                                                                           */
/* Method: getFormByName                                                     */
/* Description: get the WebForm identified by with name attribute name.      */
/* Parameters: name - name attribute to identify the form by                 */
/* Returns: html form                                                        */
/*                                                                           */
/*****************************************************************************/    
    
    public WebForm getFormByName(String name) throws InvalidElementIDException
    {
        HTMLCollection forms = document.getForms();
        
        for(int i = 0; i < forms.getLength(); i++)
        {
            if (name.equals( ((HTMLFormElement) forms.item(i)).getName() ))
                return new WebForm((HTMLFormElement)forms.item(i), i + 1);
        }
        
        throw new InvalidElementIDException (name, "Form " + name);
    }    
        
/*****************************************************************************/
/*                                                                           */
/* Method: getFormByIndex                                                    */
/* Description: get the WebForm identified by with name attribute name.      */
/* Parameters: index - the index of the form                                 */
/* Returns: html form                                                        */
/*                                                                           */
/*****************************************************************************/    
    
    public WebForm getFormByIndex(int index) throws InvalidElementIDException
    {
        HTMLCollection forms = document.getForms();
        
        index --;
        
        if ( index < forms.getLength() && index > -1 )
            return new WebForm((HTMLFormElement)forms.item(index), index + 1);
        
        throw new InvalidElementIDException (new Integer(index + 1), 
                                              "Form index " + (index + 1));
    }    
        
/*****************************************************************************/
/*                                                                           */
/* Method: listForms                                                         */
/* Description: get a list of WebForms in the page.                          */
/* Parameters: none                                                          */
/* Returns: a list of form details                                           */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap[] listForms()
    {
        HTMLCollection forms = document.getForms();
        
        HashMap [] list = new HashMap[forms.getLength()];    
        
        for (int i = 0; i < list.length; i++)
            list[i] = (new WebForm((HTMLFormElement)forms.item(i), i + 1)).
                       getSummary();
            
        return list;
    }    
        
/*****************************************************************************/
/*                                                                           */
/* Method: listLinks                                                         */
/* Description: get a list of WebLinks in the page.                          */
/* Parameters: none                                                          */
/* Returns: a list of link details                                           */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap[] listLinks()
    {
        HTMLCollection links = document.getLinks();
        
        HashMap [] list = new HashMap[links.getLength()];    
        
        for (int i = 0; i < list.length; i++)
            if ( org.apache.html.dom.HTMLAnchorElementImpl.class 
                 == links.item(i).getClass())
            
                list[i] = (new WebLink((HTMLAnchorElement)links.item(i), i + 1)).
                           getSummary();
            
            else if ( org.apache.html.dom.HTMLAreaElementImpl.class 
                       == links.item(i).getClass())
            
                list[i] = (new WebLink((HTMLAreaElement)links.item(i), i + 1)).
                          getSummary();
            
        return list;
    }    
        
}

