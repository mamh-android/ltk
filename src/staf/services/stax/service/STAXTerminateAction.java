/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.TreeMap;

public class STAXTerminateAction extends STAXActionDefaultImpl
{
    public STAXTerminateAction()
    { /* Do Nothing */ }

    public STAXTerminateAction(String blockName)
    {
        fUnevalBlockName = blockName;
        fBlockName = blockName;
    }

    public STAXTerminateAction(String blockName, String ifAttr)
    {
        fUnevalBlockName = blockName;
        fBlockName = blockName;
        fUnevalIf = ifAttr;
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

    public String getXMLInfo()
    {
        String info = "<terminate";
         
        if (fUnevalBlockName != null)
            info += " block=\"" + fBlockName + "\"";
        
        if (!fUnevalIf.equals("1"))
            info += " if=\"" + fUnevalIf + "\"";

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
                // Ignore terminate if "if" attribute evaluates to FALSE
                return;
            }

            evalAttr = "block";

            if (fUnevalBlockName == null)
                fBlockName = thread.pyStringEval("STAXCurrentBlock");
            else
                fBlockName = thread.pyStringEval(fUnevalBlockName);
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
        
        theBlock.terminateBlock();
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXTerminateAction clone = new STAXTerminateAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalBlockName = fUnevalBlockName;
        clone.fBlockName = fBlockName;
        clone.fUnevalIf = fUnevalIf;
        clone.fIf = fIf;

        return clone;
    }

    private String fUnevalBlockName = null;
    private String fBlockName = new String();
    private String fUnevalIf = "1";
    private boolean fIf = true;
}
