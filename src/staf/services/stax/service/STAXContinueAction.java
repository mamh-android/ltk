/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXContinueAction extends STAXActionDefaultImpl
{
    public STAXContinueAction()
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

        // Add a Continue condition and provide source information for the
        // action adding the condition

        thread.addCondition(
            new STAXContinueCondition(STAXUtil.formatActionInfo(this)));
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXContinueAction clone = new STAXContinueAction();
        
        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }
}
