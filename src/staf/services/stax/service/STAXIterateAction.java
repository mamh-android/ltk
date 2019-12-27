/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.List;
import java.util.Iterator;

public class STAXIterateAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int CALLING_ACTIONS = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String CALLING_ACTIONS_STRING = "CALLING_ACTIONS";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXIterateAction()
    { /* Do Nothing */ }

    public STAXIterateAction(String itemvar, String in, String indexvar, 
                             STAXAction action)
    {
        fItemvar = itemvar;
        fIn = in;
        fIndexvar = indexvar; 
        fAction = action;
    }

    public void setItemVar(String itemvar) { fItemvar = itemvar; }
    public void setIn(String in) { fIn = in; }
    public void setIndexVar(String indexvar) { fIndexvar = indexvar; }
    public void setIterateAction(STAXAction action) { fAction = action; }
    
    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case CALLING_ACTIONS:
                return CALLING_ACTIONS_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getXMLInfo()
    {
        if (fIndexvar.equals(""))
            return "<iterate var=\"" + fItemvar + "\" in=\"" + 
                   fIn + "\">";
        else
            return "<iterate var=\"" + fItemvar + "\" in=\"" + 
                   fIn + "\" indexvar=\"" + fIndexvar + "\">";
    }

    // Used to generate the Call Stack information for this element

    public String getInfo()
    {
        // Format:
        //   <currIndex>/<listSize> [<varValue>] <list>
        //
        //  where:
        //    <currIndex> is the index of the entry in the list currently
        //                being processed
        //    <listSize>  is the size of the list (or 0 if not available yet)
        //    <varValue>  is the evaluated entry in the list currently
        //                being processed (or blank if not available)
        //    <list>      is the unevaluated list value
        //
        //  The total length of the message is limited to 40 characters.
        //  If it exceeds 40 characters, "..." is appended.

        String info = fCurrListIndex + "/" + fListSize;


        if (fList != null)
        {
            // Make sure don't get a NullPointerException on fList
            // or a IndexOutOfBounds Exception on the index value

            int index = fCurrListIndex - 1;

            if ((index >= 0) && (index < fListSize))
                info += " " + fList.get(index);
        }

        info += " " + fIn;
        
        if (info.length() > 40)
            info = info.substring(0, 40) + "...";
        
        return info;
    }

    public String getDetails()
    {
        return "State:" + getStateAsString() +
               ";CurrListIndex:" + fCurrListIndex +
               ";ListSize:" + fListSize +
               ";Itemvar:" + fItemvar +
               ";In:" + fIn +
               ";Indexvar:" + fIndexvar +
               ";Action:" + fAction +
               ";List:" + fList; 
    }

    public void execute(STAXThread thread)
    {
        String evalElem = getElement();
        String evalAttr = "in";

        try
        {
            if (fState == INIT)
            {
                // Create a list/tuple in Jython from fIn and then extract
                // the Python tuple or array into a Java List.

                fList = thread.pyListEval(fIn);
                fListSize = fList.size();

                if (fListSize == 0)
                {
                    // Nothing in list, so done; raise a signal.

                    fState = COMPLETE;
                    thread.popAction();

                    setElementInfo(new STAXElementInfo(evalElem, evalAttr));

                    thread.setSignalMsgVar(
                        "STAXEmptyListMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("STAXEmptyList");
                }

                fState = CALLING_ACTIONS;
            } 
            else if (fState == CALLING_ACTIONS)
            {
                if (fListSize > fCurrListIndex)
                {
                    // Assign indexvar Jython variable, if provided

                    if (fIndexvar != null && !fIndexvar.equals(""))
                    {
                        evalAttr = "indexvar";
                        thread.pySetVar(
                            fIndexvar, new Integer(fCurrListIndex));
                    }

                    // Assign itemvar Jython variable

                    evalAttr = "var";
                    Object itemValue = (Object)fList.get(fCurrListIndex++);
                    thread.pySetVar(fItemvar, itemValue);
 
                    thread.pushAction(fAction.cloneAction());
                }
                else
                { 
                    fState = COMPLETE;
                    thread.popAction();
                }
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
        // that the iterate action handles.  This way, there won't be any
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
        STAXIterateAction clone = new STAXIterateAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fItemvar = fItemvar;
        clone.fIn = fIn;
        clone.fIndexvar = fIndexvar;
        clone.fAction = fAction;

        return clone;
    }

    private int fState = INIT;
    private int fCurrListIndex = 0;
    private List fList = null;
    private int fListSize = 0;

    private String fItemvar = new String();
    private String fIn = new String();
    private String fIndexvar = new String();
    private STAXAction fAction = null;
}
