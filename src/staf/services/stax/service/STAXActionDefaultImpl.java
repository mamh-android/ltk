/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import java.util.HashMap;
import java.util.Vector;

public class STAXActionDefaultImpl implements STAXAction
{
    public STAXActionDefaultImpl()
    { /* Do Nothing */ }

    public String getInfo()
    {
        return "";
    }

    public String getDetails()
    {
        return "";
    }

    public void execute(STAXThread thread)
    { /* Do Nothing */ }

    public void handleCondition(STAXThread thread, STAXCondition condition)
    { /* Do Nothing */ }

    public STAXAction cloneAction()
    {
        STAXActionDefaultImpl clone = new STAXActionDefaultImpl();

        clone.fElement = fElement;
        clone.fLineNumberMap =
            new HashMap<String, Vector<String>>(fLineNumberMap);
        clone.fXmlFile = fXmlFile;
        clone.fXmlMachine = fXmlMachine;

        return clone;
    }

    public void setElement(String element)
    {
        fElement = element;
    }

    public String getElement()
    {
        // Return the root element for this action

        return fElement;
    }

    public void setLineNumber(Node root)
    {
        NamedNodeMap attrs = root.getAttributes();

        setLineNumber(root.getNodeName(),
                      STAXUtil.getLineNumberFromAttrs(attrs));
    }

    /**
     * Use if need to override the element name in the Node.
     */
    public void setLineNumber(Node root, String element)
    {
        NamedNodeMap attrs = root.getAttributes();

        setLineNumber(element,
                      STAXUtil.getLineNumberFromAttrs(attrs));
    }


    public void setLineNumber(String element, String lineNumber)
    {
        if ((fLineNumberMap.size() == 0) && fElement.equals(""))
        {
            // Assumes first line number set is for the root element for
            // this action (if the root element is not already set.
            // Other entries in the map are child elements.
            fElement = element;
        }

        synchronized (fLineNumberMap)
        {
            Vector<String> lineNumbers;

            if (!fLineNumberMap.containsKey(element))
            {
                lineNumbers = new Vector<String>();
                lineNumbers.add(lineNumber);
            }
            else
            {
                lineNumbers = fLineNumberMap.get(element);
                lineNumbers.add(lineNumber);
            }
            
            fLineNumberMap.put(element, lineNumbers);
        }
    }

    public String getLineNumber()
    {
        // Specifying no element indicates to get the line number for the
        // root element for this action.

        return getLineNumber(fElement, 0);
    }

    public String getLineNumber(String element)
    {
        return getLineNumber(element, 0);
    }

    public String getLineNumber(String element, int index)
    {
        String lineNumber = "Unknown";

        synchronized (fLineNumberMap)
        {
            if (fLineNumberMap.containsKey(element))
            {
                Vector lineNumbers = (Vector)fLineNumberMap.get(element);

                if (index == -1)
                    index = lineNumbers.size() - 1;

                if (lineNumbers.size() > index)
                {
                    lineNumber = (String)lineNumbers.get(index);
                }
            }
        }

        return lineNumber;
    }
    
    // Used to clone line numbers

    public HashMap<String, Vector<String>> getLineNumberMap()
    {
        return fLineNumberMap;
    }

    public void setLineNumberMap(HashMap<String, Vector<String>> lineNumberMap)
    {
        fLineNumberMap = new HashMap<String, Vector<String>>(lineNumberMap);
    }

    public String getXmlFile()
    {
        return fXmlFile;
    }

    public void setXmlFile(String xmlFile)
    {
        fXmlFile = xmlFile;
    }

    public String getXmlMachine()
    {
        return fXmlMachine;
    }

    public void setXmlMachine(String xmlMachine)
    {
        fXmlMachine = xmlMachine;
    }

    public void setElementInfo(STAXElementInfo info)
    {
        fElementInfo = info;
    }

    public STAXElementInfo getElementInfo()
    {
        return fElementInfo;
    }

    private String fElement = new String("");  // Root element name
    private String fXmlFile = new String("Unknown");
    private String fXmlMachine = new String("Unknown");
    private STAXElementInfo fElementInfo = new STAXElementInfo();

    // Map containing the line numbers for the root element for this action
    // and, optionally, the line numbers for child elements for this action.
    private HashMap<String, Vector<String>> fLineNumberMap =
        new HashMap<String, Vector<String>>();
}
