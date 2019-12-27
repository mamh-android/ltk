/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.ArrayList;
import java.util.Iterator;

public class STAXLoopAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int TOP_OF_LOOP = 1;
    static final int BOTTOM_OF_LOOP = 2;
    static final int COMPLETE = 3;

    static final String INIT_STRING = "INIT";
    static final String TOP_OF_LOOP_STRING = "TOP_OF_LOOP";
    static final String BOTTOM_OF_LOOP_STRING = "BOTTOM_OF_LOOP";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXLoopAction()
    { /* Do Nothing */ }

    public STAXLoopAction(String indexvar, String from, String to, String by,
                          String in_while, String until, STAXAction action)
    {
        fIndexvar = indexvar;
        fFrom = from;
        fTo = to;
        fBy = by;
        fWhile = in_while; 
        fUntil = until;
        fAction = action;
    }
    
    public void setIndexVar(String indexvar) { fIndexvar = indexvar; }
    public void setFrom(String from) { fFrom = from; }
    public void setTo(String to) { fTo = to; }
    public void setBy(String by) { fBy = by; }
    public void setWhile(String in_while) { fWhile = in_while; }
    public void setUntil(String until) { fUntil = until; }
    public void setAction(STAXAction action) { fAction = action; }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case TOP_OF_LOOP:
                return TOP_OF_LOOP_STRING;
            case BOTTOM_OF_LOOP:
                return BOTTOM_OF_LOOP_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getXMLInfo()
    {
        String optional_value1 = "";
        String optional_value2 = "";

        if (fIndexvar != null)
            optional_value1 = "var=\"" + fIndexvar + "\" ";
        if (fTo != null)
            optional_value2 = " to=\"" + fTo + "\"";
        if (fWhile != null)
            optional_value2 += " while=\"" + fWhile + "\"";
        if (fUntil != null)
            optional_value2 += " until=\"" + fUntil + "\""; 

        return "<loop " + optional_value1 + "from=\"" + 
                fFrom + "\" by=\"" + fBy + "\"" + optional_value2 + ">";
    }

    public String getInfo()
    {
        StringBuffer result = new StringBuffer();

        result.append("#").append(fCurrLoopIndex);

        if (fFrom != null)
            result.append(" from ").append(fFrom);

        if (fTo != null)
            result.append(" to ").append(fToInt);
        else if (fWhile == null && fUntil == null)
            result.append(" to forever");

        if (fByInt != 1)
          result.append(" by ").append(fByInt);

        if (fWhile != null)
            result.append(" while ").append(fWhile);

        if (fUntil != null)
            result.append(" until ").append(fUntil); 

        String resultStr = result.toString();
        
        if (resultStr.length() > 40)
            return resultStr.substring(0, 40) + "...";
        else
            return resultStr;
    }

    public String getDetails()
    {
        return "State:" + getStateAsString() +
               ";CurrLoopIndex:" + fCurrLoopIndex +
               ";Indexvar:" + fIndexvar +
               ";From:" + fFrom +
               ";FromInt:" + fFromInt +
               ";To:" + fTo +
               ";ToInt:" + fToInt +
               ";By:" + fBy +
               ";ByInt:" + fByInt +
               ";While:" + fWhile +
               ";Until:" + fUntil +
               ";To_Descending:" + fTo_Descending +
               ";Action:" + fAction;
    }

    public void execute(STAXThread thread)
    {
        String evalElem = getElement();
        String evalAttr = "from";

        try
        {
            if (fState == INIT)
            {
                // Evaluate integers fFrom, fTo, and fBy.

                fFromInt = thread.pyIntEval(fFrom);
                fCurrLoopIndex = fFromInt;

                if (fTo != null)
                {
                    evalAttr = "to";
                    fToInt = thread.pyIntEval(fTo);
                }

                evalAttr = "by";
                fByInt = thread.pyIntEval(fBy);

                if (fByInt < 0) fTo_Descending = true;
                else fTo_Descending = false;

                fState = TOP_OF_LOOP;
            }
            else if (fState == TOP_OF_LOOP)
            {
                // Assign indexvar Jython variable, if provided
                
                if (fIndexvar != null)
                {
                    evalAttr = "indexvar";
                    thread.pySetVar(fIndexvar, new Integer(fCurrLoopIndex));
                }
                
                // Evaluate "to" and "while" expressions, if provided, at the
                // top of the loop to see if should continue

                boolean continueLoop = true;

                if (fTo != null && ! fTo_Descending)
                    continueLoop = fCurrLoopIndex <= fToInt;
                else if (fTo != null && fTo_Descending)
                    continueLoop = fCurrLoopIndex >= fToInt;

                evalAttr = "while";

                if (continueLoop && fWhile != null)
                    continueLoop = thread.pyBoolEval(fWhile);
                
                if (!continueLoop)
                {
                    fState = COMPLETE;
                    thread.popAction();
                    return;
                }

                thread.pushAction(fAction.cloneAction());
                
                fState = BOTTOM_OF_LOOP;
            }
            else if (fState == BOTTOM_OF_LOOP)
            {
                // Check the "until" expression, if provided, at the bottom
                // of the loop to see if should continue

                evalAttr = "until";

                if (fUntil != null && thread.pyBoolEval(fUntil))
                {
                    fState = COMPLETE;
                    thread.popAction();
                    return;
                }
                
                // Increment the loop index

                fCurrLoopIndex += fByInt;

                fState = TOP_OF_LOOP;
            }
        }
        catch (STAXPythonEvaluationException e)
        {
            fState = COMPLETE;
            thread.popAction();

            setElementInfo(new STAXElementInfo(evalElem, evalAttr));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg",
                STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        if (!(cond instanceof STAXContinueCondition))
        {
            fState = COMPLETE;  
            thread.popAction();
        }

        // Remove any break or continue conditions from the condition stack
        // that the loop action handles.  This way, there won't be any
        // "dangling" break or continue conditions hanging around if a
        // terminate block condition is added to the condition stack.

        thread.visitConditions(new STAXVisitor()
        {
            public void visit(Object o, Iterator iter)
            {
                if ((o instanceof STAXBreakCondition) ||
                    (o instanceof STAXContinueCondition))
                {
                    iter.remove();
                }
            }
        });
    }

    public STAXAction cloneAction()
    {
        STAXLoopAction clone = new STAXLoopAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fIndexvar = fIndexvar;
        clone.fFrom = fFrom;
        clone.fTo = fTo;
        clone.fBy = fBy;
        clone.fWhile = fWhile;
        clone.fUntil = fUntil;
        clone.fAction = fAction;

        return clone;
    }

    private int fState = INIT;
    private int fCurrLoopIndex = 0;
    private boolean fTo_Descending = false;
    private int fFromInt;
    private int fToInt;
    private int fByInt;

    private String fIndexvar;
    private String fFrom;
    private String fTo;
    private String fBy;
    private String fWhile;
    private String fUntil;
    private STAXAction fAction = null;
}
