/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXBreakAction extends STAXActionDefaultImpl
{
    public STAXBreakAction()
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

        // Add a Break condition and provide source information for the
        // action adding the condition

        thread.addCondition(
            new STAXBreakCondition(STAXUtil.formatActionInfo(this)));
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXBreakAction clone = new STAXBreakAction();
        
        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }
}
