package com.ibm.staf.service.http.html;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import java.util.Vector;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
// xerces
import org.w3c.dom.html.HTMLElement;
import org.w3c.dom.html.HTMLCollection;
import org.w3c.dom.html.HTMLSelectElement;
import org.w3c.dom.html.HTMLInputElement;
import org.w3c.dom.html.HTMLTextAreaElement;
import org.apache.html.dom.HTMLOptionElementImpl;

/*****************************************************************************/
/*                                                                           */
/* Class: Parameter                                                          */
/* Description: This class provides the handle to manipulate html form       */
/*              controls.                                                    */
/*                                                                           */
/*****************************************************************************/

public abstract class Parameter
{
    String name;
    String type;
    boolean use;

    public static final String NAME="NAME";
    public static final String TYPE="TYPE";
    public static final String DISABLED="IS DISABLED";
    public static final String READONLY="IS READONLY";
    public static final String VALUE="VALUE";
    public static final String POSSIBLEVALUES="OPTIONS";

    public String getType()
    {
        return type;
    }
        
    public String getKey()
    {
        return name;
    }

    public boolean apply()
    {
        return use;
    }

    public void apply(boolean use)
    {
        this.use = use;    
    }
    
    public abstract Vector[] paramString();
    public abstract String getValue();
    // ignores bad set values
    //  this may change to throw some type of error
    //  or apply(false)
    public abstract void setValue(String value) 
                      throws InvalidParameterValueException;
    public abstract void reset();
    public abstract HashMap getSummary();

} // end abstract class Parameter

/*****************************************************************************/
/*                                                                           */
/* Class: InputParameter                                                     */
/* Description: This class provides the handle to manipulate most html form  */
/*              controls of type INPUT.                                      */
/*              NOT for submit, reset, img, radio, or checkbox               */
/*                                                                           */
/*****************************************************************************/

class InputParameter extends Parameter
{
    HTMLInputElement element;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLInputElement that will be wrapped by this class  */
/*                      to manipulate the control                            */
/*                                                                           */
/*****************************************************************************/    
    
    public InputParameter(HTMLInputElement element)
    {
        name = element.getName();

        if (name == null)
        {
            name = element.getId();
        }

        type = "input type=" + element.getType();
        
        this.element = element;

        use = ! element.getDisabled();

        if (! element.getForm().hasAttribute(WebForm.VISITED))
            element.setDefaultValue(element.getValue());
                
        //System.out.println(name + " " + element.getDefaultValue() + 
        //                   " " + element.getValue());
    }

    public String toString()
    {
        String str = "<" + type + " name = " + name + " value = " + getValue();

        if (!use)
            str += " disabled";
        
        if (element != null)        
            if (element.getReadOnly())
                str += " readonly";

        str += " >";
        
        return str;
    }
    
    public void setValue(String value) throws InvalidParameterValueException
    {
        if (element == null) return;
        
        if (element.getReadOnly())
                throw new InvalidParameterValueException
                           (getKey() + ". readonly");
        if (element.getDisabled())
                throw new InvalidParameterValueException
                           (getKey() + ". disabled");
        
        element.setValue(value);
    }
    
    public String getValue()
    {
        if (element == null) return null;
    
        return element.getValue();
    }    

    public void reset()
    {
        if (element == null) return;
        
        ((HTMLInputElement)element).setValue(
                             ((HTMLInputElement)element).getDefaultValue());
    }

    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, name);
        map.put(TYPE, type);

        if (element.getDisabled())
            map.put(DISABLED, "Yes");
        else
            map.put(DISABLED, "No");

        if (element.getReadOnly())
            map.put(READONLY, "Yes");
        else
            map.put(READONLY, "No");

        map.put(VALUE, getValue());
        
        return map;    
    }
    
    public Vector[] paramString()
    {
        if (!use)
            return new Vector[0];
            
        Vector[] params = new Vector [1];
        
        params[0] = new Vector();
        params[0].addElement(name);
        params[0].addElement(getValue());
        
        return params;
    }    
}// end class InputParamater

/*****************************************************************************/
/*                                                                           */
/* Class: NullParameter                                                      */
/* Description: This class provides a way to store unused control elements.  */
/*              This class serves as a placeholder in parameter lists for    */
/*              reset buttons and an empty slot for submitless forms.        */
/*                                                                           */
/*****************************************************************************/

class NullParameter extends Parameter
{
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: none                                                           */
/*                                                                           */
/*****************************************************************************/    
        
    public NullParameter()
    {
        type = "null type";
        name = null;
        use = false;
    }
    
    public void reset()
    {
        return;
    }
    
    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, name);
        map.put(TYPE, type);
        map.put(VALUE, getValue());

        return map;    
    }

    public String getValue()
    {
        return "";
    }
    
    public void setValue(String value)
    {
        return;
    }
    
    public Vector[] paramString()
    {
        return new Vector[0];
    }
    
    public String toString()
    {
        String str = "<" + type + " name = " + name + " disabled >";
        
        return str;
    }
} // end class NullParameter

/*****************************************************************************/
/*                                                                           */
/* Class: TextArea                                                           */
/* Description: This class provides the handle to manipulate html form       */
/*              textarea controls.                                           */
/*                                                                           */
/*****************************************************************************/

class TextArea extends Parameter
{
    HTMLTextAreaElement element;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLTextAreaElement that will be wrapped by this     */
/*                      class to manipulate the control                      */
/*                                                                           */
/*****************************************************************************/    
            
    public TextArea(HTMLTextAreaElement element)
    {
        name = element.getName();
        if (name == null)
            name = element.getId();

        type = "textarea";
        this.element = element;

        use = ! element.getDisabled();    
        
        if (! element.getForm().hasAttribute(WebForm.VISITED))
            element.setAttribute("DefaultValue", getValue());
    }

    public String toString()
    {
        String str = "<" + type + " name = " + name;

        if (!use)
            str += " disabled";

        if (element.getReadOnly())
                str += " readonly";

        str += ">" + getValue() +"</textarea>";
        
        return str;
    }
    
    public void setValue(String value) throws InvalidParameterValueException
    {
        if (element.getReadOnly())
                throw new InvalidParameterValueException
                           (getKey() + ". readonly");
        if (element.getDisabled())
                throw new InvalidParameterValueException
                           (getKey() + ". disabled");
        
        /*  this is not correct in xerces
        element.setValue(value);    
         *  below is an alternative way to get at the info
        */    
        org.w3c.dom.NodeList list = element.getChildNodes();
        for (int i = 0; i < list.getLength(); i++)
        {
            if (list.item(i).getNodeName().equals("#text"))
                list.item(i).setNodeValue(value);
        }
    }
    
    public String getValue()
    {    
        /* this broken in xerces
        return element.getValue();
         * below is an alternative way to get at the info
        */
        org.w3c.dom.NodeList list = element.getChildNodes();
        for (int i=0;i<list.getLength();i++)
        {
            if (list.item(i).getNodeName().equals("#text"))
                return list.item(i).getNodeValue();
        }
        
        return "";            
    }    
    
    public void reset()
    {
        try
        {
            setValue(element.getAttribute("DefaultValue"));    
        }
        catch (InvalidParameterValueException e)
        {
            //should be ignored since value should be good.
        }
    }
    
    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, name);
        map.put(TYPE, type);
        map.put(VALUE, getValue());

        if (element.getDisabled())
            map.put(DISABLED, "Yes");
        else
            map.put(DISABLED, "No");

        if (element.getReadOnly())
            map.put(READONLY, "Yes");
        else
            map.put(READONLY, "No");
        
        return map;    
    }

    public Vector[] paramString()
    {
        if (!use)
            return new Vector[0];
            
        Vector[] params = new Vector [1];
        
        params[0] = new Vector();
        params[0].addElement(name);
        params[0].addElement(getValue());
        
        return params;
    }
    
    public String getType()
    {
        return type;
    }
}// end class TextArea

/*****************************************************************************/
/*                                                                           */
/* Class: Checkbox                                                           */
/* Description: This class provides the handle to manipulate html form       */
/*              input type checkbox controls.                                */
/*                                                                           */
/*****************************************************************************/

class CheckBox extends Parameter
{
    HTMLInputElement element;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLInputElement that will be wrapped by this class  */
/*                      to manipulate the control                            */
/*                                                                           */
/*****************************************************************************/    
    
    public CheckBox (HTMLInputElement element)
    {
        name = element.getName();
        if (name == null)
            name = element.getId();

        type = "input type=checkbox";
        this.element = element;

        use = element.getChecked();
        
        if (! element.getForm().hasAttribute(WebForm.VISITED))
            element.setDefaultChecked(use);
        //System.out.println(name + " " + element.getDefaultChecked() + 
        //                   " " + element.getChecked());
    }
    
    public String toString()
    {
        String str = "<" + type + " name = " + name + " value = " + 
                     element.getAttribute("value");
        
        if (use) 
           str += " checked";
               
        if (element.getDisabled())
            str += " disabled";
                
        if (element.getReadOnly())
            str += " readonly";
        
        str += " >";
        
        return str;
    }

    public void setValue(String value) throws InvalidParameterValueException
    {
        if (element.getReadOnly())
                throw new InvalidParameterValueException
                           (getKey() + ". readonly");
        if (element.getDisabled())
                throw new InvalidParameterValueException
                           (getKey() + ". disabled");
        
        if (value.equalsIgnoreCase("CHECKED"))
            use = true;
            
        else if (value.equalsIgnoreCase("UNCHECKED"))
            use = false;
        
        else
            throw new InvalidParameterValueException(value);
            
        element.setChecked(use);
    }
        
    public String getValue()
    {
        if (use)
            return "CHECKED";
            
        return "UNCHECKED";
    
    }

    public String getKey()
    {
        return name + "=" + element.getAttribute("value");
    }
    
    public void reset()
    {
        element.setChecked(element.getDefaultChecked() );
    }

    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, getKey());
        map.put(TYPE, type);
        map.put(VALUE, getValue());

        if (element.getDisabled())
            map.put(DISABLED, "Yes");
        else
            map.put(DISABLED, "No");

        if (element.getReadOnly())
            map.put(READONLY, "Yes");
        else
            map.put(READONLY, "No");
        
        return map;    
    }
    
    public Vector[] paramString()
    {
        if (!use || element.getDisabled())
            return new Vector[0];
            
        Vector[] params = new Vector [1];
        
        params[0] = new Vector();
        params[0].addElement(name);
        params[0].addElement(element.getAttribute("value"));
        
        return params;
    }
}// end class CheckBox

/*****************************************************************************/
/*                                                                           */
/* Class: RadioGroup                                                         */
/* Description: This class provides the handle to manipulate html form       */
/*              input type radio controls.                                   */
/*                                                                           */
/*****************************************************************************/

class RadioGroup extends Parameter
{
    HTMLInputElement[] element;
    int selected;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: radioList - a list of HTMLInputElemets of type radio           */
/*                                                                           */
/*****************************************************************************/    

    public RadioGroup (Vector radioList)
    {
        name = ((HTMLInputElement) radioList.elementAt(0)).getName();
        type = "input type=radio";
        
        int size = radioList.size();
        
        element = new HTMLInputElement[size];
        selected = -1;
        
        for (int i = 0; i < size; i++)
        {
            element[i] = (HTMLInputElement) radioList.elementAt(i);
                                       
            if (element[i].getChecked() && !element[i].getDisabled())
            {
                selected = i;
                
                if (! element[0].getForm().hasAttribute(WebForm.VISITED))
                    element[i].setDefaultChecked(element[i].getChecked());
            }
        }
        
        use = (selected != -1);
    }
    
    public String toString()
    {
        if (element.length == 0)
            return "";
            
        String str = "";
        for (int i = 0; i < element.length; i++)
        {
            str += "<" + type + " name = " + name + " value = " 
                + element[i].getValue();
            
            if (i == selected)
               str += " checked";
               
            if (element[i].getDisabled())
                str += " disabled";
                
            if (element[i].getReadOnly())
                str += " readonly";
                
            str += " >\n";
        }
        
        // strip off the last \n and return
        return str.substring(0, str.length() - 1);
    }
    
    public void setValue(String value) throws InvalidParameterValueException
    {
        // verify the new value is a valid radio button    
        for (int i = 0; i < element.length; i++)
        {
            if (value.equals(element[i].getValue()))
                if (!element[i].getDisabled())
                    if (!element[i].getReadOnly())
                    {
                        if (selected != -1)
                            element[selected].setChecked(false);
                            
                        selected = i;
                        element[selected].setChecked(true);
                        use = true;
    
                        return;
                    }
                    else
                        throw new InvalidParameterValueException(value 
                                                               + ". readonly");
                else
                    throw new InvalidParameterValueException(value 
                                                               + ". disabled");
        }
        throw new InvalidParameterValueException(value + ". unknown option");
    }
    
    public String getValue()
    {
        try
        {
            return element[selected].getValue();
        }
        catch (ArrayIndexOutOfBoundsException e)
        {
            // indicates no value selected for the radio control
        }

        return "";
    }

    public void reset()
    {
        selected = -1;
        
        for (int i = 0; i < element.length; i++)
        {
            element[i].setChecked(element[i].getDefaultChecked());
            
            if (element[i].getDefaultChecked() && !element[i].getDisabled())
                selected = i;
        }
        
    }

    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, name);
        map.put(TYPE, type);
        map.put(VALUE, getValue());

        if (selected == -1)
        {
            map.put(DISABLED, "");
            map.put(READONLY, "");
        }

        List possibleValuesList = new ArrayList();

        for (int i = 0; i < element.length; i++)
        {
            if (i == selected)
            {
                if (element[i].getDisabled())
                    map.put(DISABLED, "Yes");
                else
                    map.put(DISABLED, "No");

                if (element[i].getReadOnly())
                    map.put(READONLY, "Yes");
                else
                    map.put(READONLY, "No");
            }
            else
            {
                String str = element[i].getValue();

                if (element[i].getDisabled())
                {
                    str += "\n(Disabled)";
                }

                if (element[i].getReadOnly())
                {
                    str += "\n(Readonly)";
                }
                            
                possibleValuesList.add(str);
            }
        }

        map.put(POSSIBLEVALUES, possibleValuesList);
        
        return map;    
    }
    
    public Vector[] paramString()
    {    
        try
        {
            if (!use || element[selected].getDisabled())
                return new Vector[0];
        
            Vector[] params = new Vector [1];
            
            params[0] = new Vector();
            params[0].addElement(name);
            params[0].addElement(getValue());
            
            return params;
        }
        catch (ArrayIndexOutOfBoundsException e)
        {
            //  no radio buttons are selected
        }
        
        return new Vector[0];
    }    
}// end class RadioGroup

/*****************************************************************************/
/*                                                                           */
/* Class: SubmitGroup                                                        */
/* Description: This class provides the handle to manipulate html form       */
/*              input type submit controls.                                  */
/*                                                                           */
/* SubmitGroup contains a list of all the possible ways a form can be        */
/* submitted.  Only one method of submission can be used, therefore only one */
/* member of the submit group can be selected at a time.                     */
/* There are two html elements that can be used to trigger a form to submit. */
/* One is a <input type=submit>.  This is a standard form button that can    */
/* have name and value attributes.  The other is a <input type=image>.       */
/* This is a client side mapping input.  It can have name, value, and an     */
/* image file as attributes.  In addition, to send the name and value        */
/* attributes when this type of input is selected, the coordinates in the    */
/* image where the user clicked are also sent.  This coordinate information  */
/* is genereated when the form is submitted and not stored anywhere prior.   */
/*                                                                           */
/* Since all information regarding a SubmitGroup element is stored with      */
/* the corresponding html element, attributes valueX and valueY are added to */
/* an <input type=image> element when they are assigned with a SET command.  */
/* Prior to this, the valueX and valueY attributes don't exist and if the    */
/* attributes are queried, an empty string should be returned (i.e.          */
/* name=value, instead of name=value,<valueX>,<valueY>.  Unselected image    */
/* controls would benefit from placeholders, <valueX>,<valueY>, in the two   */
/* coordinate location because it indicates that the coordinates are         */
/* variable when using the SET command.  The current coords are meaningless  */
/* since they are overwritten on the next SET and it may confuse the user    */
/* indicating that only certain (0's or the previous) coords are valid.      */
/* Not only should uninitialized (no valueX or valueY) IMAGE controls be in  */
/* this format when queried, but also all the other possible values for      */
/* submitgroup image options should be in this format. The currently         */
/* selected submit option, if it is an IMAGE, will show coords in the value  */
/* line but show the variable form in the options list.                      */
/*                                                                           */
/* For example:                                                              */
/*   Name             : submit                                               */
/*   Type             : Submit group                                         */
/*   Disabled         : No                                                   */
/*   Read Only        : No                                                   */
/*   Value            : pic1=rose,2,3                                        */
/*   Possible Value 1 : pic1=rose,<valueX>,<valueY>                          */
/*   Possible Value 2 : button1=submit                                       */
/*   Possible Value 3 : pic2=tulip,<valueX>,<valueY>                         */
/*                                                                           */
/*                                                                           */
/* Also, in theory, the currently selected submit option should never be     */
/* readonly or disabled, since it is not possible to interact with controls  */
/* with either attribute.  If a readonly or disabled control becomes         */
/* selected, there is a bug.                                                 */

/*****************************************************************************/

class SubmitGroup extends Parameter
{
    int selected;
    HTMLInputElement[] element;
    static final String ACTIVE = "active";
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: radioList - a list of HTMLInputElements of type submit and img */
/*                                                                           */
/*****************************************************************************/    

    public SubmitGroup (Vector submitList)
    {
        name = "submit";
        type = "input type = ";
        
        selected = -1;
        
        int size = submitList.size();
        
        element = new HTMLInputElement[size];
        
        for (int i = 0; i < size; i++)
        {
            element[i] = (HTMLInputElement) submitList.elementAt(i);
            
            if (element[i].hasAttribute(ACTIVE))
                selected = i;
                
            else if (selected == -1)
            {
                String value = element[i].getAttribute("value");
                
                // if it is an IMAGE element set a default location
                if (element[i].getType().equalsIgnoreCase("IMAGE"))
                {
                    element[i].setAttribute("valueX","<valueX>");
                    element[i].setAttribute("valueY","<valueY>");
                    value = value + ",<valueX>,<valueY>";
                }
                
                try
                {    
                    setValue(element[i].getAttribute("name") + "=" 
                            + value);
                }
                catch (InvalidParameterValueException e)
                {
                    //System.out.println("Invalid Submit value");
                }
            }
        }
        
        use = (selected != -1);
    }

    public String getType()
    {
        return "Submit group";
    }

    public String toString()
    {
        String str = " ";
        for (int i = 0; i < element.length; i++)
        {
            str += "<" + type + element[i].getAttribute("type") + " name = " 
                + element[i].getAttribute("name") + " value = " 
                + element[i].getAttribute("value");
            
            if (element[i].getType().equalsIgnoreCase("image"))
                str += " X=" + element[i].getAttribute("valueX") 
                      +" Y=" + element[i].getAttribute("valueY");
            
            if (element[i].hasAttribute("disabled"))
                str += " disabled";
            
            str += ">";    
            
            if (i == selected)
               str += " active";

            str += "\n";
        }
        
        // strip off the last \n and return
        return str.substring(0, str.length() - 1);
    }
    
    public void setValue(String value)throws InvalidParameterValueException
    {
        // need to verify value is a valid submit button
        int indx = value.indexOf('=');
        
        if (indx == -1 ) 
            throw new InvalidParameterValueException(value);
        
        String n = value.substring(0,indx);
        String v = value.substring(indx+1);
        
        for (int i = 0; i < element.length; i++)
        {
            if (element[i].getType().equalsIgnoreCase("SUBMIT"))
            {
                // select the submit button
                if ( v.equals(element[i].getAttribute("value")) 
                     && n.equals(element[i].getAttribute("name")) 
                     && !element[i].hasAttribute("disabled") )
                {
                    if (selected != -1)
                        element[selected].removeAttribute(ACTIVE);
    
                    selected = i;
                    element[selected].setAttribute(ACTIVE,"");
                    
                    return;
                }
            }
            else
            {
                // IMAGE - set the coord for the image and select it
                if ( n.equals(element[i].getAttribute("name")) 
                     && !element[i].hasAttribute("disabled") )
                {
                    if (v.indexOf(',') == -1)
                        throw new InvalidParameterValueException(v);
                    if (v.substring(0,v.indexOf(',')).
                         equals(element[i].getAttribute("value")))
                    {
                        String coord = v.substring(v.indexOf(',') + 1);
                        
                        if (coord.indexOf(',') == -1)
                            throw new InvalidParameterValueException(v);
                            
                        String x = coord.substring(0,coord.indexOf(','));
                        String y = coord.substring(coord.indexOf(',') + 1);
    
                        if (selected != -1)
                            element[selected].removeAttribute(ACTIVE);
    
                        selected = i;
                        element[selected].setAttribute(ACTIVE,"");
                        element[i].setAttribute("valueX", x);
                        element[i].setAttribute("valueY", y);
                
                        return;
                    }
                }
            }
        }
        throw new InvalidParameterValueException(value + ". unknown option");
    }

    public String getValue()
    {
        if (element.length == 0)
            return "No submit inputs";
    
        if (selected == -1)
            return "";
                
        String coord = "";
        
        if (element[selected].getType().equalsIgnoreCase("image"))
            coord = "," + element[selected].getAttribute("valueX")
                   + "," + element[selected].getAttribute("valueY");
            
        return element[selected].getAttribute("name") + "=" 
                + element[selected].getAttribute("value") + coord;
    }    

    public void reset()
    {
        if (selected == -1)
            return;
        
        selected = -1;
        
        for (int i = 0; (selected == -1) && (i < element.length); i++)
        {
            String value = element[i].getAttribute("value");
                
            // if it is an IMAGE element set a default location
            if (element[i].getType().equalsIgnoreCase("IMAGE"))
            {
                element[i].setAttribute("valueX","<valueX>");
                element[i].setAttribute("valueY","<valueY>");
                value = value +",<valueX>,<valueY>";
            }
                
            try
            {    
                setValue(element[i].getAttribute("name") + "=" 
                        + value);
            }
            catch (InvalidParameterValueException e)
            {/* Ignore this and try again on the next loop */}
        }
    }

    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, name);
        map.put(TYPE, getType());
        map.put(VALUE, getValue());
        
        if (selected == -1)
        {
            map.put(DISABLED, "");
            map.put(READONLY, "");
        }
        else
        {
            map.put(DISABLED, "No");
            map.put(READONLY, "No");
        }

        List possibleValuesList = new ArrayList();

        for (int i = 0; i < element.length; i++)
        {
            String coord = "";

            if (element[i].getType().equalsIgnoreCase("image"))
            {
                coord = "," + "<valueX>" + "," + "<valueY>";
            }

            String str = element[i].getAttribute("name") + "=" +
                         element[i].getAttribute("value") + coord;

            possibleValuesList.add(str);
        }

        map.put(POSSIBLEVALUES, possibleValuesList);

        return map;    
    }
    
    public Vector[] paramString()
    {
        if (element.length == 0 || !use ||
            // the selected element has no name or value
            ( element[selected].getAttribute("name").equals("") && 
              element[selected].getAttribute("value").equals("")))
            // there is no valid parameter to submit
            return new Vector[0];
        
        Vector[] params;
            
        String value = element[selected].getAttribute("value");
        
        if (element[selected].getType().equalsIgnoreCase("image"))
        {    
            int i = 3;
            if (value.equals("")) 
                i--;
            // deal with image        
            params = new Vector [i];
                            
            params[0] = new Vector();
            params[0].addElement(element[selected].getAttribute("name") + ".x");
            params[0].addElement(element[selected].getAttribute("valueX"));
            
            params[1] = new Vector();
            params[1].addElement(element[selected].getAttribute("name") + ".y");
            params[1].addElement(element[selected].getAttribute("valueY"));
            
            if (i == 3)
            {
                params[2] = new Vector();
                params[2].addElement(element[selected].getAttribute("name"));
                params[2].addElement(value);
            }
        } else
        {
            // deal with button
            params = new Vector [1];
            
            params[0] = new Vector();
            params[0].addElement(element[selected].getAttribute("name"));
            params[0].addElement(value);
        }
        return params;
    }    
} // end class SubmitGroup

/*****************************************************************************/
/*                                                                           */
/* Class: MultipleSelect                                                     */
/* Description: This class provides the handle to manipulate html form       */
/*              select controls.                                             */
/*                                                                           */
/*****************************************************************************/

class MultipleSelect extends Parameter
{
    HTMLOptionElementImpl [] options;
    HTMLSelectElement element;
    boolean multiple;
    int numSelected;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: element - HTMLSelectElement that will be wrapped by this class */
/*                      to manipulate the control                            */
/*                                                                           */
/*****************************************************************************/    
    
    public MultipleSelect (HTMLSelectElement element)
    {
        name = element.getName();

        type = "select";

        this.element = element;
    
        use = ! element.getDisabled();
        
        HTMLCollection theOptions = element.getOptions();

        numSelected = 0;

        int size = theOptions.getLength();
        
        options = new HTMLOptionElementImpl[size];

        for (int i = 0; i < size; i++) 
        {    
            options[i] = (HTMLOptionElementImpl) theOptions.item(i);
                        
            if (options[i].getSelected() && !options[i].getDisabled())
            {
                numSelected ++;
                
                if (! element.getForm().hasAttribute(WebForm.VISITED))
                    options[i].setDefaultSelected(true);
            }
        }
        
        multiple = element.getMultiple();
    }

    public String toString()
    {
        String str = "<select name=" + name;
        
        if (multiple)
            str += " multiple";
        
        str +=">\n";
        
        for (int i = 0; i < options.length; i++)
        {
            str += "<option value = " + options[i].getValue();
            
            if (options[i].getSelected())
               str += " selected";
               
            if (options[i].getDisabled())
                str += " disabled";
            
            str += " />\n";
        }
        
        str += "</select>";
        
        return str;
    }    
    
    public void setValue(String value) throws InvalidParameterValueException
    {
        // need to verify value is a valid select option    
        // need to verify multiple
        for (int i = 0; i < options.length; i++)
        {
            if (value.equals(options[i].getValue()))
    
                if(! options[i].getReadOnly())
                    if(! options[i].getDisabled())
                    {
                        if (options[i].getSelected())
                        {
                            options[i].setSelected(false);
                            numSelected --;
                        }
                        else if (multiple || numSelected == 0)
                        {
                            options[i].setSelected(true);
                            numSelected ++;
                        }else
                        {
                            throw new InvalidParameterValueException
                            (value +". Deselect " + getValue() + 
                             " before setting a new value");
                        }
                        return;
                    }
                    else
                        throw new InvalidParameterValueException
                                   (value + ". disabled");
                else
                    throw new InvalidParameterValueException
                               (value + ". readonly");
            
        }
        throw new InvalidParameterValueException(value + ". unknown option");
    }

// multiple values are \n delimited
    public String getValue()
    {

        String str = "";
        int i=0;
        int j=0;
        
        if (numSelected == 0)
            return str;
        
        for ( ; (i < numSelected) && (j < options.length); j++)
        {
            if (options[j].getSelected())
            {
                str += options[j].getValue() + "\n";
            }
        }
        return str.substring(0,str.length() - 1);
    }    

    public void reset()
    {
        for (int i = 0; i < options.length; i++) 
        {    
            options[i].setSelected(options[i].getDefaultSelected());
        }
    }

    public HashMap getSummary()
    {
        HashMap map = new HashMap();
        
        map.put(NAME, name);

        if (multiple)
            type += " multiple";

        map.put(TYPE, type);

        map.put(VALUE, getValue());

        if (element.getDisabled())
            map.put(DISABLED, "Yes");
        else
            map.put(DISABLED, "No");

        if (element.hasAttribute("readonly"))
            map.put(READONLY, "Yes");
        else
            map.put(READONLY, "No");
        
        List possibleValuesList = new ArrayList();

        for (int i = 0; i < options.length; i++)
        {
            if (!options[i].getSelected())
            {
                String str = options[i].getValue();
                            
                possibleValuesList.add(str);
            }
        }

        map.put(POSSIBLEVALUES, possibleValuesList);
        
        return map;    
    }
    
    public Vector[] paramString()
    {
        if (!use)
            return new Vector[0];
            
        Vector[] params = new Vector [numSelected];
        
        int i=0;
        int j=0;
        for ( ; (i < numSelected) && (j < options.length); j++)
        {
            if (options[j].getSelected() &&
                !options[j].getDisabled() )
            {
                params[i] = new Vector();
                params[i].addElement(name);
                params[i].addElement(options[j].getValue());
                i++;
            }
        }
        return params;
    }
}// end class MultipleSelect

