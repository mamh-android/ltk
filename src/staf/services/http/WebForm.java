package com.ibm.staf.service.http.html;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.service.http.WebSession;
import com.ibm.staf.service.http.InvalidElementIDException;

import java.util.HashMap;
import java.util.Vector;
import java.util.Arrays;
// xerces
import org.w3c.dom.html.HTMLInputElement;
import org.w3c.dom.html.HTMLCollection;
import org.w3c.dom.html.HTMLFormElement;

/*****************************************************************************/
/*                                                                           */
/* Class: WebForm                                                            */
/* Description: This class provides the handle to manipulate html forms.     */
/*                                                                           */
/*****************************************************************************/

public class WebForm extends WebElement
{

    public static final String CONTROLS="CONTROLS";
    public static final String VISITED="VISITED";
    
    Parameter[] params;
    String method;
    String action;
    HTMLFormElement the_form;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLFormElement that will be wrapped by this class   */
/*                      to manipulate the form                               */
/*            index - index of this element in the list of elements of this  */
/*                    type                                                   */
/*                                                                           */
/*****************************************************************************/    

    public WebForm (HTMLFormElement element, int index)
    {
        super (element.getId(), element.getName(), index);
        
        method = element.getMethod();
        //set default if not specified
        if (method == null || method.equals(""))
            method = "GET";
            
        action = element.getAction();

        // build params
        HTMLCollection controls = element.getElements();

        // group by name
        Vector elements = groupParameters(controls);
        
        params = new Parameter [elements.size()];
        
        for (int i = 0; i < params.length; i++)
            params[i] = ParameterFactory.newParameter(elements.elementAt(i));
        
        the_form = element;
        
        if (!the_form.hasAttribute(VISITED))
            the_form.setAttribute(VISITED,"");
    }
    
    public String getMethod()
    {
        return method;
    }
    public String getAction()
    {
        return action;
    }    
/*****************************************************************************/
/*                                                                           */
/* Method: findName                                                          */
/* Description: get the index in the list that has a vector of               */
/*              HTMLInputElements with the name attribute of name.           */
/*              -1 indicates the name is not in a list                       */
/* Parameters: name - name that is being searched for                        */
/*             list - vector of vectors and HTMLElements being searched      */
/* Returns: index of the list that has a list of elements with name name or  */
/*          -1 if the name is not in a list                                  */
/*                                                                           */
/*****************************************************************************/    
    
    int findName(String name, Vector list)
    {
        for (int i = 1; i < list.size(); i++)
            if (Vector.class == list.elementAt(i).getClass())
            {
                if (name.equals(((HTMLInputElement)
                                ( (Vector)list.elementAt(i) ).elementAt(0))
                                .getName()))
                {
                    return i;
                }
            }
        return -1;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: groupParameters                                                   */
/* Description: organize the collection into objects to be turned into       */
/*              control parameters.  All input type submits are grouped, all */
/*              input type radio with the same attribute name are grouped,   */
/*              all other elements are added seperately to the list          */
/* Parameters: col - a list of HTMLElements                                  */
/* Returns: a list of objects to be turned into control parameters           */
/*                                                                           */
/*****************************************************************************/    
    
    Vector groupParameters(HTMLCollection col)
    {
        Vector list = new Vector();
        
        list.add(new Vector());
        
        for (int i = 0; i < col.getLength(); i++)
        {
            if (col.item(i).getClass() == 
                org.apache.html.dom.HTMLSelectElementImpl.class)
                
                list.addElement(col.item(i));

            else if (col.item(i).getClass() ==
                      org.apache.html.dom.HTMLInputElementImpl.class)
            {
                HTMLInputElement element = (HTMLInputElement) col.item(i);

                if (element.getType().equalsIgnoreCase("submit") || 
                    element.getType().equalsIgnoreCase("image"))

                    ((Vector)list.elementAt(0)).addElement(element);
                    
                else if(element.getType().equalsIgnoreCase("radio"))
                {
                    int indx = findName(element.getName(),list);

                    if (indx == -1)
                    {
                        indx = list.size();
                        list.addElement(new Vector());
                    }
                    ((Vector)list.elementAt(indx)).addElement(element);
                }
                else
                {
                    list.addElement(col.item(i));
                }
            }
            else if (col.item(i).getClass() == 
                      org.apache.html.dom.HTMLTextAreaElementImpl.class)
                      
                list.addElement(col.item(i));
        }    
        
        // remove submit group if it is empty
        if (((Vector)list.elementAt(0)).size() == 0)
            list.remove(0);
        
        return list;
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
        
        request.put(WebSession.REQUEST_METHOD, getMethod());
        request.put(WebSession.REQUEST_URL, getAction());
        request.put(WebSession.REQUEST_PARAMETERS, getActiveParameters());
        request.put(WebSession.REQUEST_HEADERS, getHeaders());
        request.put(WebSession.REQUEST_FILES, getActiveFiles());
        request.put(WebSession.REQUEST_CONTENT, null);
        
        return request;
    
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getActiveParameters                                               */
/* Description: get the list of non-file parameters that will be added to the*/
/*              request.                                                     */
/* Parameters: none                                                          */
/* Returns: list of non-file parameters that will be added to the request    */
/*                                                                           */
/*****************************************************************************/    
    
    public Vector getActiveParameters()
    {
        Vector list = new Vector();
        int j;

        for (int i = 0; i < params.length; i++)
        {
            if (! params[i].getType().equals("input type=file"))
            {
                Vector[] subList = params[i].paramString();
                
                for (j = 0; j < subList.length; j++)
                    list.addElement(subList[j]);
            }
        }
        if (list.size() == 0)
            return null;
            
        return list;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getActiveFiles                                                    */
/* Description: get the list of file parameters that will be added to the    */
/*              request.                                                     */
/* Parameters: none                                                          */
/* Returns: list of file parameters that will be added to the request        */
/*                                                                           */
/*****************************************************************************/    
    
    public Vector getActiveFiles()
    {
        Vector list = new Vector();
        int j;

        for (int i = 0; i < params.length; i++)
        {
            if (params[i].getType().equals("input type=file"))
            {
                Vector[] subList = params[i].paramString();
                
                for (j = 0; j < subList.length; j++)
                    list.addElement(subList[j]);
            }
        }
        if (list.size() == 0)
            return null;
        
        return list;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getAllParameters                                                  */
/* Description: get the list of parameters that are part of this form.       */
/* Parameters: none                                                          */
/* Returns: list of parameters associated with this form                     */
/*                                                                           */
/*****************************************************************************/    

    public Parameter[] getAllParameters()
    {
        return params;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: reset                                                             */
/* Description: reset this form to its initial values.  This does not change */
/*              the currently selected submit option.                        */
/* Parameters: none                                                          */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public void reset()
    {
        // this does not reset the values of all control elements
        the_form.reset();
        
        // reset all parameters
        for (int i = 0; i < params.length; i++)
            params[i].reset();
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: findParameter                                                     */
/* Description: get the index of the parameter with a key of key             */
/*              -1 indicates the name is not in a list                       */
/* Parameters: key - identifier key being searched for                       */
/* Returns: index of the parameter with a key of key -1 if the key is not in */
/*          the parameter list                                               */
/*                                                                           */
/*****************************************************************************/    

    int findParameter(String key)
    {
        for (int i = 0; i < params.length; i++)
        {
            if (params[i].getKey() == null)
            {
                if (key.equals("null"))
                    return i;
            }
            else if (params[i].getKey().equals(key))
            {
                return i;
            }
        }
                
        return -1;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getParameterSummary                                               */
/* Description: get the summary of the specified parameter                   */
/* Parameters: key - the identifier key of the parameter                     */
/* Returns: a summary of the specified parameter                             */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap getParameterSummary (String key)
                                      throws InvalidElementIDException
    {
        int indx = findParameter(key);
    
        if (indx == -1)
        {
            throw new InvalidElementIDException (key, "Control " + key);
        }
        
        return params[indx].getSummary();
    }
        
/*****************************************************************************/
/*                                                                           */
/* Method: getParameterValue                                                 */
/* Description: get the value of the specified parameter                     */
/* Parameters: key - the identifier key of the parameter                     */
/* Returns: the value of the specified parameter                             */
/*                                                                           */
/*****************************************************************************/    
    
    public String getParameterValue (String key)
                                      throws InvalidElementIDException
    {
        int indx = findParameter(key);
    
        if (indx == -1)
            throw new InvalidElementIDException (key, "Control " + key);
        
        return params[indx].getValue();
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: setParameterValue                                                 */
/* Description: set the value of the specified parameter                     */
/* Parameters: key - the identifier key of the parameter                     */
/*             value - the new value of the parameter                        */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void setParameterValue (String key, String value)
                                    throws InvalidElementIDException,
                                            InvalidParameterValueException
    {
        int indx = findParameter(key);
    
        if (indx == -1)
        {
            System.out.println("Couldn't find key " + key);
            throw new InvalidElementIDException (key, "Control " + key);
        }
        
        params[indx].setValue(value);
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getParameterKeyList                                               */
/* Description: get a list of keys for all the parameters                    */
/* Parameters: none                                                          */
/* Returns: a list of keys for the parameters                                */
/*                                                                           */
/*****************************************************************************/    

    public String[] getParameterKeyList()
    {
        String[] list = new String[params.length];
        
        for (int i = 0; i < params.length; i++)
            list[i] = params[i].getKey();
            
        return list;
    }        
    
/*****************************************************************************/
/*                                                                           */
/* Method: getHeaders                                                        */
/* Description: get the list of headers that will be added to the request.   */
/* Parameters: none                                                          */
/* Returns: list of headers that will be added to the request                */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap getHeaders()
    {
        HashMap map = new HashMap();    
    
    /*
        // XXX Are these correct ? 
        if (! the_form.hasAttribute("enctype"))
            map.put("Content-Type", the_form.getEnctype());
                
        if (! the_form.hasAttribute("accept-charset"))
            map.put("Accept-Charset", the_form.getAcceptCharset());
            
        if (! the_form.hasAttribute("accept"))
            map.put("Accept", the_form.getAttribute("accept"));
        
        if (! the_form.hasAttribute("lang"))
            map.put("Content-Language", the_form.getAttribute("lang"));
    */
        if (map.isEmpty())
            return null;
                    
        return map;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getSummary                                                        */
/* Description: get a summary of the form.                                   */
/* Parameters: none                                                          */
/* Returns: description of a form                                            */
/*                                                                           */
/*****************************************************************************/    

    public HashMap getSummary()
    {
        HashMap summary = new HashMap();
        
        summary.put(WebElement.NAME, name);
        summary.put(WebElement.ID, id);        
        summary.put(WebElement.INDEX, new Integer(index));
        summary.put(WebSession.REQUEST_METHOD, getMethod());
        summary.put(WebSession.REQUEST_URL, getAction());
        summary.put(WebSession.REQUEST_HEADERS, getHeaders());
        summary.put(CONTROLS, Arrays.asList(getAllParameters()));
                
        return summary;
    }    
} // end class WebForm

