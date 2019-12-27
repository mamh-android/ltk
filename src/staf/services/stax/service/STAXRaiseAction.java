/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.TreeMap;

public class STAXRaiseAction extends STAXActionDefaultImpl
{
    public STAXRaiseAction()
    { /* Do Nothing */ }

    public STAXRaiseAction(String signal)
    {
        fUnevalSignal = signal;
        fSignal = signal;
    }

    public String getSignal() { return fSignal; }

    public void setSignal(String signal)
    {
        fUnevalSignal = signal;
    }

    public String getXMLInfo()
    {
        return "<raise signal=\"" + fSignal + "\"/>";
    }

    public String getInfo()
    {
        return fSignal;
    }

    public String getDetails()
    {
        return "Signal:" + fSignal;
    }

    public void execute(STAXThread thread)
    {
        thread.popAction();

        try
        {
            fSignal = thread.pyStringEval(fUnevalSignal);
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(getElement(), "signal"));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");

            return;
        }

        thread.raiseSignal(fSignal, this);
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXRaiseAction clone = new STAXRaiseAction();
        
        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalSignal = fUnevalSignal;
        clone.fSignal = fSignal;

        return clone;
    }

    private String fUnevalSignal;
    private String fSignal;
}
