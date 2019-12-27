/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;

public class STAXTestcaseStatusAction extends STAXActionDefaultImpl
{
    public STAXTestcaseStatusAction()
    { /* Do Nothing */ }

    public STAXTestcaseStatusAction(String status, String message)
    {
        fUnevalStatus = status;
        fStatus = status;        
        fUnevalMessage = message;
        fMessage = message;
    }

    public void setStatus(String status) { fUnevalStatus = status; }
    public void setMessage(String message) { fUnevalMessage = message; }

    public String getXMLInfo()
    {
        String msg = "<tcstatus result=\"";
        msg += fStatus;        
        msg += "\">" + fMessage + "</tcstatus>";

        return msg;
    }

    public String getInfo()
    {
        return fStatus;        
    }

    public String getDetails()
    {
        return "Status:" + fStatus + 
               ";Message:" + fMessage;
    }

    public void execute(STAXThread thread)
    {
        thread.popAction();

        String evalAttr = "result";

        try
        {
            if (fUnevalStatus == null)
                fStatus = new String();
            else
                fStatus = thread.pyStringEval(fUnevalStatus);        

            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

            if (fUnevalMessage == null)
                fMessage = new String();
            else
                fMessage = thread.pyStringEval(fUnevalMessage);
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(getElement(), evalAttr));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");

            return;
        }

        // Get Current Testcase

        String testcaseName;
        try
        {
            testcaseName = thread.pyStringEval("STAXCurrentTestcase");
            if (testcaseName.equals("None"))
                throw new STAXPythonEvaluationException("");
        }
        catch (STAXPythonEvaluationException e)
        {
            // Raise a signal if no testcase wrapper element for tcstatus.

            setElementInfo(new STAXElementInfo(getElement(), evalAttr));

            thread.setSignalMsgVar(
                "STAXTestcaseMissingMsg", STAXUtil.formatErrorMessage(this));

            thread.raiseSignal("STAXTestcaseMissingError");

            return;
        }

        // Raise a signal if status result is not valid

        if (!STAXTestcaseActionFactory.VALID_STATUS_LIST.contains(
            fStatus.toLowerCase()))
        {
            setElementInfo(new STAXElementInfo(
                getElement(), "result", "Invalid result: " + fStatus));

            thread.setSignalMsgVar(
                "STAXInvalidTcStatusResultMsg",
                STAXUtil.formatErrorMessage(this));

            thread.raiseSignal("STAXInvalidTcStatusResult");

            return;
        } 

        // Update Testcase Status

        STAXTestcase theTest = null;

        @SuppressWarnings("unchecked")
        TreeMap<String, STAXTestcase> testcaseMap =
            (TreeMap<String, STAXTestcase>)thread.getJob().getData(
                "testcaseMap");

        if (testcaseMap != null)
        {
            synchronized (testcaseMap)
            {
                theTest = testcaseMap.get(testcaseName);

                if (theTest == null)
                {
                    List<String> testcaseStack = new ArrayList<String>();

                    try
                    {
                        if (thread.pyBoolEval("len(STAXTestcaseStack) > 0"))
                        {
                            testcaseStack = new ArrayList<String>(Arrays.asList(
                                (String[])thread.getPythonInterpreter().get(
                                    "STAXTestcaseStack", String[].class)));
                        }

                        theTest = new STAXTestcase(testcaseName, testcaseStack);
                    }
                    catch (STAXPythonEvaluationException e)
                    {
                        // Should never happen

                        STAXTimestamp currentTimestamp = new STAXTimestamp();

                        System.out.println(
                            currentTimestamp.getTimestampString() +
                            " STAXTestcaseStatusAction::execute() - " +
                            "Error getting STAXTestcaseStack Python " +
                            "variable.  Testcase: " + testcaseName +
                            ", JobID: " + thread.getJob().getJobNumber() +
                            ".  " + e.toString());

                        theTest = new STAXTestcase(testcaseName);
                    }

                    testcaseMap.put(testcaseName, theTest);
                }
            }

            theTest.updateStatus(fStatus, fMessage, thread.getJob());
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXTestcaseStatusAction clone = new STAXTestcaseStatusAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalStatus = fUnevalStatus;
        clone.fStatus = fStatus;
        clone.fUnevalMessage = fUnevalMessage;
        clone.fMessage = fMessage;

        return clone;
    }

    private String fUnevalStatus = null;
    private String fStatus;
    private String fUnevalMessage = null;
    private String fMessage;
}
