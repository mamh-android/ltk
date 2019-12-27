/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFUtil;
import java.util.TreeMap;

public class STAXHoldAction extends STAXActionDefaultImpl
{
    public STAXHoldAction()
    { /* Do Nothing */ }

    public STAXHoldAction(String blockName)
    {
        fUnevalBlockName = blockName;
        fBlockName = blockName;
    }

    public STAXHoldAction(String blockName, String ifAttr)
    {
        fUnevalBlockName = blockName;
        fBlockName = blockName;
        fUnevalIf = ifAttr;
    }

    public STAXHoldAction(String blockName, String ifAttr, String timeout)
    {
        fUnevalBlockName = blockName;
        fBlockName = blockName;
        fUnevalIf = ifAttr;
        fUnevalTimeoutString = timeout;
    }

    public String getBlockName() { return fBlockName; }

    public void setBlockName(String blockName)
    {
        fUnevalBlockName = blockName;
    }

    public boolean getIf() { return fIf; }

    public void setIf(String ifAttr)
    {
        fUnevalIf = ifAttr;
    }

    public long getTimeout() { return fTimeout; }
    
    public void setTimeout(String timeoutString)
    {
        fUnevalTimeoutString = timeoutString;
    }

    public String getXMLInfo()
    {
        String info = "<hold";
         
        if (fUnevalBlockName != null)
            info += " block=\"" + fBlockName + "\"";
        
        if (!fUnevalIf.equals("1"))
            info += " if=\"" + fUnevalIf + "\"";

        if (fUnevalTimeoutString != null)
            info += " timeout=\"" + fTimeoutString + "\"";

        info += "/>";

        return info;
    }

    public String getInfo()
    {
        return fBlockName;
    }

    public String getDetails()
    {
        return "BlockName:" + fBlockName;
    }

    public void execute(STAXThread thread)
    {
        thread.popAction();

        String evalElem = getElement();
        String evalAttr = "if";

        try
        {
            fIf = thread.pyBoolEval(fUnevalIf);

            if (!fIf)
            {
                // Ignore hold if "if" attribute evaluates to FALSE
                return;
            }

            evalAttr = "block";

            if (fUnevalBlockName == null)
                fBlockName = thread.pyStringEval("STAXCurrentBlock");
            else
                fBlockName = thread.pyStringEval(fUnevalBlockName);

            evalAttr = "timeout";

            fTimeout = 0;  // Default value of 0 holds block indefinitely

            if (fUnevalTimeoutString != null)
            {
                // A timeout was specified for the hold

                fTimeoutString = thread.pyStringEval(fUnevalTimeoutString);
                
                STAFResult result = STAFUtil.convertDurationString(
                    fTimeoutString);

                if (result.rc == STAFResult.Ok)
                {
                    try
                    {
                        fTimeout = Long.parseLong(result.result);
                    }
                    catch (NumberFormatException e)
                    {
                        // Should not happen since convertDurationString
                        // should have already found this

                        result.rc = STAFResult.InvalidValue;
                        result.result = "";
                    }
                }

                if (result.rc != STAFResult.Ok)
                {
                    setElementInfo(new STAXElementInfo(
                        evalElem, evalAttr,
                        "Invalid timeout value: " + fTimeoutString +
                        "\n\n" + result.result));

                    thread.setSignalMsgVar(
                        "STAXInvalidTimerValueMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("STAXInvalidTimerValue");
                    return;
                }
            }
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(evalElem, evalAttr));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");
            return;
        }

        STAXBlockAction theBlock = null;
        TreeMap blockMap = (TreeMap)thread.getJob().getData("blockMap");

        if (blockMap != null)
        {
            theBlock = (STAXBlockAction)blockMap.get(fBlockName);
        }

        if (theBlock == null)
        {
            setElementInfo(new STAXElementInfo(
                evalElem, evalAttr,
                "Block '" + fBlockName + "' does not exist."));

            thread.setSignalMsgVar(
                "STAXBlockDoesNotExistMsg",
                STAXUtil.formatErrorMessage(this));

            thread.raiseSignal("STAXBlockDoesNotExist");
            return;

        }

        if (theBlock.getBlockState() == STAXBlockAction.BLOCK_RUNNING)
            theBlock.holdBlock(fTimeout);
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXHoldAction clone = new STAXHoldAction(
            fUnevalBlockName, fUnevalIf, fUnevalTimeoutString);

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }

    private String fUnevalBlockName = null;
    private String fBlockName = new String();
    private String fUnevalIf = "1";
    private boolean fIf = true;
    private String fUnevalTimeoutString = null;
    private String fTimeoutString = new String();
    private long fTimeout = 0;  // 0 indicates to hold indefinitely
}
