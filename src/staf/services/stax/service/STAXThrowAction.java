/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.*;
import java.text.*;

public class STAXThrowAction extends STAXActionDefaultImpl
{
    public STAXThrowAction()
    { /* Do Nothing */ }

    public STAXThrowAction(String exceptionName, String data)
    { 
        fExceptionName = exceptionName;
        fUnevalExceptionName = exceptionName;
        fExceptionData = data;
        fUnevalExceptionData = data;
    }

    public String getExName() { return fExceptionName; }

    public void setExName(String exception)
    {
        fUnevalExceptionName = exception;
    }

    public String getData() { return fExceptionData; }

    public void setData(String data)
    {
        fUnevalExceptionData = data;
    }

    public String getXMLInfo()
    {
        return "<throw exception=\"" + fExceptionName + "\">" + fExceptionData +
               "</throw>";
    } 

    public String getInfo()
    {
        return fExceptionName;
    }
 
    public String getDetails()
    {
        return "ExceptionName:" + fExceptionName + 
               ";ExceptionData:" + fExceptionData;
    }

    public void execute(STAXThread thread)
    {         
        thread.popAction();

        String evalElem = getElement();
        String evalAttr = "exception";

        try
        {
            fExceptionName = thread.pyStringEval(fUnevalExceptionName);

            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
            fExceptionData = thread.pyStringEval(fUnevalExceptionData);

            fExceptionCallStack = thread.getCallStack();
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(evalElem, evalAttr));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");

            return;
        }

        // Add an Exception condition and provide source information for the
        // action adding the condition

        thread.addCondition(
            new STAXExceptionCondition(fExceptionName, fExceptionData,
                                       STAXUtil.formatActionInfo(this),
                                       fExceptionCallStack));
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXThrowAction clone = new STAXThrowAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalExceptionName = fUnevalExceptionName;
        clone.fUnevalExceptionData = fUnevalExceptionData;
        clone.fExceptionName = fExceptionName;
        clone.fExceptionData = fExceptionData;
        clone.fExceptionCallStack = fExceptionCallStack;

        return clone;
    }   

    private String fUnevalExceptionName = null;
    private String fExceptionName = null;
    private String fUnevalExceptionData = new String("None");
    private String fExceptionData = new String("None");
    private List<String> fExceptionCallStack = new ArrayList<String>();
}
