/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

public class STAXTryAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int CALLED_ACTION = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String CALLED_ACTION_STRING = "CALLED_ACTION";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXTryAction()
    { /* Do Nothing */ }

    public STAXTryAction(STAXAction action, List<STAXCatchAction> catchList)
    {
        fAction = action;
        fCatchList = catchList;
    }

    public void setTryAction(STAXAction tryAction) { fAction = tryAction; }
    
    public void setCatchList(List<STAXCatchAction> catchList)
    {
        fCatchList = catchList;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case CALLED_ACTION:
                return CALLED_ACTION_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getXMLInfo()
    {
        return "<try>";
    } 

    public String getInfo()
    {
        return "";
    }

    public String getDetails()
    {
        return "State:" + getStateAsString() + 
               ";Action:" + fAction +
               ";CatchList:" + fCatchList;
    }

    STAXCatchAction getCatchHandler(STAXExceptionCondition ex)
    {
        String exceptionName = ex.getName();

        for (NamedCatch namedCatch : fNamedCatchList)
        {
            if (namedCatch.name.equals("...") ||
                namedCatch.name.equals(exceptionName) ||
                (exceptionName.startsWith(namedCatch.name) &&
                 (exceptionName.charAt(namedCatch.name.length()) == '.')))
            {
                return namedCatch.catchAction;
            }
        }

        return null;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            String evalElem = "catch";
            String evalAttr = "exception";
            int evalIndex = 0;

            fNamedCatchList = new LinkedList<NamedCatch>();

            for (STAXCatchAction catchAction : fCatchList)
            {
                try
                {
                    fNamedCatchList.add(
                        new NamedCatch(
                            thread.pyStringEval(
                                catchAction.getCatchableExceptionName()),
                            catchAction));
                }
                catch (STAXPythonEvaluationException e)
                {
                    thread.popAction();

                    setElementInfo(new STAXElementInfo(
                        evalElem, evalAttr, evalIndex));

                    thread.setSignalMsgVar(
                        "STAXPythonEvalMsg",
                        STAXUtil.formatErrorMessage(this), e);

                    thread.raiseSignal("STAXPythonEvaluationError");

                    return;
                }

                evalIndex++;
            }

            thread.pushAction(fAction.cloneAction());
            fState = CALLED_ACTION;
        } 
        else if (fState == CALLED_ACTION)
        {
            fState = COMPLETE;
            thread.popAction();
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        fState = COMPLETE;
        thread.popAction();

        if (cond instanceof STAXExceptionCondition)
        {
            STAXExceptionCondition ex = (STAXExceptionCondition)cond;
            STAXCatchAction handler = getCatchHandler(ex);

            if (handler != null)
            {
                // Push the catch block action

                handler = (STAXCatchAction)handler.cloneAction();
                handler.setException(ex);

                thread.pushAction(handler);
            }
        }
        
        // Remove any exception conditions from the condition stack that this
        // try action handles.  This way, there won't be any "dangling"
        // exception conditions hanging around if a terminate block condition
        // is added to the condition stack.
        
        thread.visitConditions(new STAXVisitor()
        {
            public void visit(Object o, Iterator iter)
            {
                if (o instanceof STAXExceptionCondition)
                {
                    STAXCatchAction handler =
                        getCatchHandler((STAXExceptionCondition)o);

                    if (handler != null) iter.remove();
                }
            }
        });
    }

    public STAXAction cloneAction()
    {
        STAXTryAction clone = new STAXTryAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fAction = fAction;
        clone.fCatchList = fCatchList;

        return clone;
    }

    class NamedCatch
    {
        public NamedCatch(String name, STAXCatchAction catchAction)
        {
            this.name = name;
            this.catchAction = catchAction;
        }

        String name;
        STAXCatchAction catchAction;
    }

    private int fState = INIT;
    private STAXAction fAction = null;
    private List<STAXCatchAction> fCatchList = null;
    private LinkedList<NamedCatch> fNamedCatchList = null;
}
