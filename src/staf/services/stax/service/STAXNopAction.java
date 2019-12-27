/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXNopAction extends STAXActionDefaultImpl
{
    public STAXNopAction()
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
    {
        thread.popAction();
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXNopAction clone = new STAXNopAction();
        
        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }
}
