/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXRethrowAction extends STAXActionDefaultImpl
{
    public STAXRethrowAction()
    { /* Do Nothing */ }

    public String getXMLInfo()
    {
        return "<rethrow/>";
    } 

    public String getInfo()
    {
        return "";
    }

    public String getDetails()
    {
        return "";
    }

    public void execute(STAXThread thread)
    {
        thread.popAction();
        thread.addCondition(new STAXRethrowExceptionCondition());
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXRethrowAction clone = new STAXRethrowAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }
}
