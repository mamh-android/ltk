/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXException extends Exception
{
    public STAXException(String message)
    {
        super("\n" + message);
    }

    public STAXException(String message, STAXActionDefaultImpl action)
    {
        super("\n" + message);
        
        fElement = action.getElement();
        fLineNumber = action.getLineNumber();
        fXmlFile = action.getXmlFile();
        fXmlMachine = action.getXmlMachine();
    }

    public void setLineNumber(String lineNumber) { fLineNumber = lineNumber; }
    public String getLineNumber() { return fLineNumber; }
    public void setXmlFile(String xmlFile) { fXmlFile = xmlFile; }
    public String getXmlFile()    { return fXmlFile; }
    public void setXmlMachine(String xmlMachine) { fXmlMachine = xmlMachine; }
    public String getXmlMachine() { return fXmlMachine; }

    private String fElement = new String("Unknown");
    private String fLineNumber = new String("Unknown");
    private String fXmlFile = new String("Unknown");
    private String fXmlMachine = new String("Unknown");
}
