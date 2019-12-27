/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXSignalHandlerAction extends STAXActionDefaultImpl
{
    public STAXSignalHandlerAction()
    { /* Do Nothing */ }

    public STAXSignalHandlerAction(String name, STAXAction action)
    {
        fUnevalName = name;
        fName = name;
        fAction = action;
    }

    public void setSignalName(String name) { fUnevalName = name; }
    public String getSignalName() { return fName; }

    public void setSignalAction(STAXAction action) { fAction = action; }

    public String getXMLInfo()
    {
        return "<signalhandler name=\"" + fName + "\"/>";
    }

    public String getInfo()
    {
        return fName;
    }

    public String getDetails()
    {
        return "Name:" + fName + 
               ";Action:" + fAction;
    }

    public void execute(STAXThread thread)
    {
        try
        {
            fName = thread.pyStringEval(fUnevalName);
        }
        catch (STAXPythonEvaluationException e)
        {
            thread.popAction();

            setElementInfo(new STAXElementInfo(getElement(), "signal"));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg",
                STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");

            return;
        }

        STAXSignalExecutionAction action = new STAXSignalExecutionAction(
            fName, fAction);
        action.setElement(getElement());
        action.setLineNumberMap(getLineNumberMap());
        action.setXmlFile(getXmlFile());
        action.setXmlMachine(getXmlMachine());

        thread.registerSignalHandler(fName, action);

        thread.popAction();
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXSignalHandlerAction clone = new STAXSignalHandlerAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalName = fUnevalName;
        clone.fName = fName;
        clone.fAction = fAction;

        return clone;
    }

    private String fUnevalName = new String();
    private String fName = new String();
    private STAXAction fAction =  null;
}
