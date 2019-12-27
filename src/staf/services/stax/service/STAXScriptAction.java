/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXScriptAction extends STAXActionDefaultImpl
{
    public STAXScriptAction()
    { /* Do Nothing */ }

    public STAXScriptAction(String value)
    {
        fValue = value;
    }

    public String getValue() { return fValue; } 
    public void setValue(String value) { fValue = value; }

    public String getXMLInfo()
    {
        return "<script>" + fValue + "</script>";
    }

    public String getInfo()
    {
        if (fValue.length() > 40)
            return fValue.substring(0, 40) + "...";
        else
            return fValue;
    }

    public String getDetails()
    {
        return "Value:" + fValue;
    }

    public void execute(STAXThread thread)
    { 
        try
        { 
            thread.pyExec(fValue);
            thread.popAction();
        }
        catch (STAXPythonEvaluationException e)
        {
            thread.popAction();

            setElementInfo(new STAXElementInfo(getElement()));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");
        }  
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXScriptAction clone = new STAXScriptAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fValue = fValue;

        return clone;
    }

    private String fValue = new String();
}
