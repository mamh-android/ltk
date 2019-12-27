/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.python.core.*;

public class STAXReturnAction extends STAXActionDefaultImpl
{
    public STAXReturnAction()
    { /* Do Nothing */ }

    public STAXReturnAction(String value)
    {
        fValue = value;
    }

    public String getValue() { return fValue; } 
    public void setValue(String value) { fValue = value; }

    public String getXMLInfo()
    {
        return "<return>" + fValue + "</return>";
    }

    public String getInfo()
    {
        int valueLength = fValue.length();
        if (valueLength > 40)
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
        if (fValue.equals(null))
        {
            fValue = "None";
        }

        thread.popAction();

        PyObject result = null;
        
        try
        {
            result = thread.pyObjectEval(fValue);
            thread.addCondition(new STAXReturnCondition(result));
        }
        catch (STAXPythonEvaluationException e)
        {
            thread.pySetVar("STAXResult", Py.None);

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
        STAXReturnAction clone = new STAXReturnAction(fValue);
        
        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }

    private String fValue = "None";
}
