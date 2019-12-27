/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

public class STAXIfAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int ACTION_CALLED = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String ACTION_CALLED_STRING = "ACTION_CALLED";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXIfAction()
    { /* Do Nothing */ }

    public STAXIfAction(String ifExpression, STAXAction ifAction, 
                        List<STAXElseIf> elseifs, STAXAction elseAction)
    {
        fIfExpression = ifExpression;
        fIfAction = ifAction;
        fElseifs = elseifs;
        fElseAction = elseAction;
    }

    public void setIfExpression(String ifExpression)
    {
        fIfExpression = ifExpression;
    }
    public String getIfExpression()
    {
        return fIfExpression;
    }

    public void setIfAction(STAXAction ifAction)
    {
        fIfAction = ifAction;
    }

    public void setElseifs(List<STAXElseIf> elseifs)
    {
        fElseifs = elseifs;
    }

    public void setElseAction(STAXAction elseAction)
    {
        fElseAction = elseAction;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case ACTION_CALLED:
                return ACTION_CALLED_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo()
    {
        int msgLength = fIfExpression.length();
        if (msgLength > 40)
            return fIfExpression.substring(0, 40) + "...";
        else
            return fIfExpression;
    }

    public String getDetails()
    {
        return "State:" + getStateAsString() +
               ";IfExpression:" + fIfExpression +
               ";IfAction:" + fIfAction +
               ";Elseifs:" + fElseifs +
               ";ElseAction:" + fElseAction;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            String evalElem = getElement();
            String evalAttr = "expr";
            int evalIndex = 0;

            try
            {
                if (thread.pyBoolEval(fIfExpression))
                {
                    thread.pushAction(fIfAction.cloneAction());
                }
                else
                {   
                    Iterator<STAXElseIf> it = fElseifs.iterator();
                    STAXElseIf elseif;
                    boolean elseif_performed = false;

                    evalElem = "elseif";
                    evalAttr = "expr";

                    while (it.hasNext() && !elseif_performed)
                    {
                        elseif = it.next();

                        if (thread.pyBoolEval(elseif.getExpression()))
                        {
                            thread.pushAction(elseif.getAction().cloneAction());
                            elseif_performed = true;
                        }

                        evalIndex++;
                    }

                    if (! elseif_performed && fElseAction != null)
                        thread.pushAction(fElseAction.cloneAction());
                }

                fState = ACTION_CALLED;
            }
            catch (STAXPythonEvaluationException e)
            {
                fState = COMPLETE;
                thread.popAction();

                setElementInfo(new STAXElementInfo(
                    evalElem, evalAttr, evalIndex));

                thread.setSignalMsgVar(
                    "STAXPythonEvalMsg",
                    STAXUtil.formatErrorMessage(this), e);

                thread.raiseSignal("STAXPythonEvaluationError");
            }
        }
        else if (fState == ACTION_CALLED)
        {
            fState = COMPLETE;
            thread.popAction();
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        fState = COMPLETE;
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXIfAction clone = new STAXIfAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fIfExpression = fIfExpression;
        clone.fIfAction = fIfAction;
        clone.fElseifs = fElseifs;
        clone.fElseAction = fElseAction;

        return clone;
    }

    private int fState = INIT;
    private String fIfExpression = new String("");
    private STAXAction fIfAction = null;
    private List<STAXElseIf> fElseifs = null;
    private STAXAction fElseAction = null;
}
