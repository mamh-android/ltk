package com.ibm.staf.service.http.html;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import java.util.Vector;
//xerces
import org.w3c.dom.html.HTMLElement;
import org.w3c.dom.html.HTMLSelectElement;
import org.w3c.dom.html.HTMLInputElement;
import org.w3c.dom.html.HTMLTextAreaElement;

/*****************************************************************************/
/*                                                                           */
/* Class: ParameterFactory                                                   */
/* Description: This class provides a way to create html form controls.      */
/*                                                                           */
/*****************************************************************************/

public class ParameterFactory 
{
    
/*****************************************************************************/
/*                                                                           */
/* Method: newParameter                                                      */
/* Description: create a new Parameter object of the appropriate sub-type.   */
/* Parameters: element - parameter to pass to the contructor                 */
/* Returns: a new Parameter object of the appropriate type                   */
/*                                                                           */
/*****************************************************************************/    

    public static Parameter newParameter(Object element)
    {
        Class elementClass = element.getClass();
        
        if (elementClass == org.apache.html.dom.HTMLSelectElementImpl.class)
            return new MultipleSelect((HTMLSelectElement) element);
            
        if (elementClass == org.apache.html.dom.HTMLTextAreaElementImpl.class)
            return new TextArea((HTMLTextAreaElement) element);
            
        if (elementClass == org.apache.html.dom.HTMLInputElementImpl.class)
        {
            if (((HTMLInputElement)element).getType().equalsIgnoreCase
                ("checkbox"))
                return new CheckBox((HTMLInputElement) element);
            
            if (((HTMLInputElement)element).getType().equalsIgnoreCase("reset"))
            {
                // don't use reset button
                return new NullParameter();
            }
                
            return new InputParameter((HTMLInputElement) element);
        }

        if (elementClass == Vector.class)
        {
            if (((Vector)element).size() > 0)
            {
                HTMLElement e = (HTMLElement)((Vector)element).elementAt(0);

                if (e.getAttribute("type").equalsIgnoreCase("RADIO"))
                    return new RadioGroup((Vector) element);

                if ((e.getAttribute("type").equalsIgnoreCase("SUBMIT")) ||
                    (e.getAttribute("type").equalsIgnoreCase("IMAGE")) )
                    return new SubmitGroup((Vector) element);
            }
            else
            {
                // empty submit list
                return new NullParameter();
            }
        }
        // invalid creation type
        return new NullParameter();
    }
}
