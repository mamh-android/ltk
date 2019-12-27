/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFUtil;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import org.python.core.Py;

public class STAXTestcaseAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int TESTCASE_ENTERED = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String TESTCASE_ENTERED_STRING = "TESTCASE_ENTERED";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";
    
    static final String kDefault = "default";
    static final String kStrict = "strict";

    public STAXTestcaseAction()
    { /* Do Nothing */ }

    public STAXTestcaseAction(String name, STAXAction action, String mode)
    {
        fUnevalName = name;
        fName = name;
        fUnevalMode = mode;
        fMode = mode;
        fAction = action;
    }

    public String getName() { return fName; }
    public void setName(String name) { fUnevalName = name; }
    
    public String getMode() { return fMode; }
    public void setMode(String mode) { fUnevalMode = mode; }

    public void setTestcaseAction(STAXAction action) { fAction = action; }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case TESTCASE_ENTERED:
                return TESTCASE_ENTERED_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getXMLInfo()
    {
        return  "<testcase name=\"" + fName + "\" mode=\"" + fMode + "\">";
    }

    public String getInfo()
    {
        return fName;
    }

    public String getDetails()
    {
        return "Name:" + fName + ";Mode:" + fMode + ";Action:" + fAction +
               ";State:" + getStateAsString();
    }

    public void execute(STAXThread thread)
    {
        fThread = thread;

        if (fState == INIT)
        {
            String evalAttr = "name";

            try
            {
                fName = thread.pyStringEval(fUnevalName);
                
                evalAttr = "mode";
                fMode = thread.pyStringEval(fUnevalMode);
                
                if (!(fMode.equalsIgnoreCase(kStrict)) &&
                    !(fMode.equalsIgnoreCase(kDefault)))
                {
                    // Raise a Invalid Testcase Mode signal
                    fState = COMPLETE;
                    fThread.popAction();
 
                    setElementInfo(new STAXElementInfo(
                        getElement(), evalAttr, "Invalid mode: " + fMode));

                    fThread.setSignalMsgVar(
                        "STAXInvalidTestcaseModeMsg",
                        STAXUtil.formatErrorMessage(this));

                    fThread.raiseSignal("STAXInvalidTestcaseMode");        
                    
                    fMode = "default";
                }
            }
            catch (STAXPythonEvaluationException e)
            {
                fState = COMPLETE;
                thread.popAction();

                setElementInfo(new STAXElementInfo(getElement(), evalAttr));

                thread.setSignalMsgVar(
                    "STAXPythonEvalMsg",
                    STAXUtil.formatErrorMessage(this), e);

                thread.raiseSignal("STAXPythonEvaluationError");

                return;
            }

            // Get Current Testcase
            try
            {
                String currentTestcase = thread.pyStringEval(
                                         "STAXCurrentTestcase");
                if (!currentTestcase.equals("None"))
                    fName = currentTestcase + "." + fName;
            }
            catch (STAXPythonEvaluationException e)
            {
                // Ignore
            }

            // Enter Testcase
            try
            {
                thread.pySetVar("STAXCurrentTestcase", fName);
                thread.pyExec("STAXTestcaseStack.append(STAXCurrentTestcase)");
            }
            catch (STAXPythonEvaluationException e)
            {
                fState = COMPLETE;
                thread.popAction();
                thread.getJob().log(STAXJob.JOB_LOG, "error", 
                    "STAXTestcaseAction: Enter testcase " + fName +
                    " failed with " + e.toString());
                return;
            }
            
            // Get current date and time and set as the testcase start timestamp
            fStartTimestamp = new STAXTimestamp();

            // Determine the testcase mode
            int mode;

            if (fMode.equalsIgnoreCase(kStrict))
                mode = STAXTestcase.STRICT_MODE;
            else
                mode = STAXTestcase.DEFAULT_MODE;

            // Check if the testcase name is already in the testcaseMap.
            // If not, add it to the testcaseMap.  If the testcase already
            // exists, if this instance of the testcase has mode strict, then
            // set its mode to strict if its not already strict.

            STAXTestcase theTest = null;

            @SuppressWarnings("unchecked")
            TreeMap<String, STAXTestcase> testcaseMap = 
                (TreeMap<String, STAXTestcase>)thread.getJob().getData(
                    "testcaseMap");

            if (testcaseMap == null)
            {
                fState = COMPLETE;
                thread.popAction();
                thread.getJob().log(
                    STAXJob.JOB_LOG, "warning", 
                    "STAXTestcaseAction: Enter testcase " + fName +
                    " failed because the job's testcaseMap is null, " +
                    "possibly because the job is terminating");
                return;
            }

            synchronized (testcaseMap)
            {
                theTest = testcaseMap.get(fName);
                
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

                        theTest = new STAXTestcase(fName, mode, testcaseStack);
                    }
                    catch (STAXPythonEvaluationException e)
                    {
                        // Should never happen
                        
                        STAXTimestamp currentTimestamp = new STAXTimestamp();

                        System.out.println(
                            currentTimestamp.getTimestampString() +
                            " STAXTestcaseAction::execute() - " +
                            "Error getting STAXTestcaseStack Python " +
                            "variable.  Testcase: " + fName +
                            ", Job ID: " + thread.getJob().getJobNumber() +
                            ".  " + e.toString());

                        theTest = new STAXTestcase(fName, mode);
                    }

                    testcaseMap.put(fName, theTest);
                }
                else if ((mode == STAXTestcase.STRICT_MODE) &&
                         (theTest.getMode() != STAXTestcase.STRICT_MODE))
                {
                    theTest.setMode(mode);
                }
            }

            // Start the testcase
            theTest.start(thread.getJob());
            
            thread.pushAction(fAction.cloneAction());
            fState = TESTCASE_ENTERED;
        }
        else if (fState == TESTCASE_ENTERED)
        {
            fState = COMPLETE;
            exitTestcase(fName);
            thread.popAction();
        }
    }

    public void exitTestcase(String name)
    {
        try
        {
            // Stop the testcase
            
            STAXTestcase theTest = null;

            TreeMap testcaseMap =
                (TreeMap)fThread.getJob().getData("testcaseMap");

            if (testcaseMap != null)
            {
                synchronized (testcaseMap)
                {
                    theTest = (STAXTestcase)testcaseMap.get(fName);
                }
            }
            else
            {
                fThread.getJob().log(
                    STAXJob.JOB_LOG, "warning", 
                    "STAXTestcaseAction: exitTestcase(" + name +
                    ") failed because the job's testcaseMap is null, " +
                    "possibly because the job is terminating");
            }

            if (theTest != null)
                theTest.stop(fThread.getJob(), fStartTimestamp);
            
            if (fThread.pyBoolEval(
                "STAXTestcaseStack[len(STAXTestcaseStack)-1] != " +
                "STAXCurrentTestcase"))
            {
                fThread.getJob().log(STAXJob.JOB_LOG, "error", 
                    "STAXTestcaseAction: exitTestcase(" + name +
                    ") failed. This testcase is not on the STAXTestcaseStack");
            }
            else
            {
                fThread.pyExec("STAXTestcaseStack.pop()");
                if (fThread.pyBoolEval("len(STAXTestcaseStack) > 0"))
                    fThread.pyExec("STAXCurrentTestcase = " +
                        "STAXTestcaseStack[len(STAXTestcaseStack)-1]");
                else
                    fThread.pySetVar("STAXCurrentTestcase", Py.None);
            }
        }
        catch (STAXPythonEvaluationException e)
        {
            fThread.getJob().log(STAXJob.JOB_LOG, "error", 
                "STAXTestcaseAction: exitTestcase(" + name +
                ") failed with " + e.toString());
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        if (fState == TESTCASE_ENTERED)
            exitTestcase(fName);

        fState = COMPLETE;
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXTestcaseAction clone = new STAXTestcaseAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalName = fUnevalName;
        clone.fName = fName;
        clone.fUnevalMode = fUnevalMode;
        clone.fMode = fMode;
        clone.fAction = fAction;

        return clone;
    }

    private int fState = INIT;
    private String fUnevalName = null;
    private String fName = null;
    private String fUnevalMode = null;
    private String fMode = null;
    private STAXAction fAction = null;
    private STAXThread fThread = null;
    private STAXTimestamp fStartTimestamp;
}
