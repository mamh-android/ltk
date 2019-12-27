/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

public class STAXFinallyAction extends STAXActionDefaultImpl
                               implements STAXThreadCompleteListener
{
    /**
     * Flag to enable debugging information to be written to the STAX JVM log
     * to help debug a problem with the finally element
     */
    static final boolean DEBUG = false;

    private static final int INIT = 0;
    private static final int TRY_ACTION = 1;
    private static final int WAIT_THREAD = 2;
    private static final int THREAD_COMPLETE = 3;
    private static final int COMPLETE = 4;

    private static final String INIT_STRING = "INIT";
    private static final String TRY_ACTION_STRING = "TRY_ACTION";
    private static final String WAIT_THREAD_STRING = "WAIT_THREAD";
    private static final String THREAD_COMPLETE_STRING = "THREAD_COMPLETE";
    private static final String COMPLETE_STRING = "COMPLETE";
    private static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    private static final boolean USE_SAME_PYINTERPRETER = true;

    public STAXFinallyAction()
    { /* Do Nothing */ }

    public STAXFinallyAction(STAXAction tryAction, STAXAction finallyAction)
    {
        fTryAction = tryAction;
        fFinallyAction = finallyAction;
    }
    
    public void setTryAction(STAXAction tryAction)
    {
        fTryAction = tryAction;
    }

    public void setFinallyAction(STAXAction finallyAction)
    {
        fFinallyAction = finallyAction;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case TRY_ACTION:
                return TRY_ACTION_STRING;
            case WAIT_THREAD:
                return WAIT_THREAD_STRING;
            case THREAD_COMPLETE:
                return THREAD_COMPLETE_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo() { return ""; }

    public String getDetails()
    {
        return "State:" + getStateAsString() +
               ";TryAction:" + fTryAction +
               ";FinallyAction:" + fFinallyAction;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            if (DEBUG)
                System.out.println(
                    "STAXFinallyAction::execute(): fState=INIT");

            // Execute the try/catch actions

            thread.pushAction(fTryAction.cloneAction());
            fState = TRY_ACTION;
        }
        else if (fState == TRY_ACTION)
        {
            // Try and catch tasks are done.
            // Now execute the finally action on a new thread

            if (DEBUG)
                System.out.println(
                    "STAXFinallyAction::execute(): fState=TRY_ACTION" +
                    " - Now run finally action");

            // Add a hard hold condition, so we can wait until the finally
            // child thread completes.  This is removed by threadComplete().

            thread.addCondition(fHardHoldCondition);

            // Need to set the state to WAIT_THREAD before the child thread is
            // scheduled to run the finally action

            fState = WAIT_THREAD;

            // Create a new thread and use same Python interpreter
                
            fFinallyThread = thread.createChildThread(USE_SAME_PYINTERPRETER);
            fFinallyThread.addCompletionNotifiee(this);

            // Put the finally action on this child thread and schedule
            // the child thread to run

            fFinallyThread.pushAction(fFinallyAction.cloneAction());
            fFinallyThread.schedule();
        }
        else if (fState == THREAD_COMPLETE)
        {
            if (DEBUG)
                System.out.println(
                    "STAXFinallyAction::execute(): fState=THREAD_COMPLETE");

            fState = COMPLETE;
            thread.popAction();
        }
        else
        {
            // Should not get here (e.g. state should not be WAIT_THREAD)

            System.out.println(
                "ERROR: STAXFinallyAction::execute(): fState=" +
                getStateAsString());
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        if (DEBUG)
            System.out.println(
                "STAXFinallyAction::handleCondition(): fState=" +
                getStateAsString());

        if (fState == TRY_ACTION)
        {
            // Remove all inheritable conditions off the condition stack
            // that were added while running the try/catch actions and
            // save them

            thread.visitConditions(new STAXVisitor()
            {
                public void visit(Object o, Iterator iter)
                {
                    STAXCondition condition = (STAXCondition)o;

                    if (condition.isInheritable())
                    {
                        fSaveConditionList.add(condition);
                        iter.remove();
                    }
                }
            });

            if (DEBUG)
                System.out.println(
                    "STAXFinallyAction::handleCondition(): " +
                    "After remove inheritable conditions: " +
                    thread.getConditionStack());

            // Add a hard hold condition, so we can wait until the finally
            // child thread completes.  This is removed by threadComplete().

            thread.addCondition(fHardHoldCondition);

            // Need to set the state to WAIT_THREAD before the child thread is
            // scheduled to run the finally action

            fState = WAIT_THREAD;

            // Create a new thread and use same Python interpreter
                
            fFinallyThread = thread.createChildThread(USE_SAME_PYINTERPRETER);
            fFinallyThread.addCompletionNotifiee(this);

            // Put the finally action on this child thread and schedule
            // the child thread to run

            fFinallyThread.pushAction(fFinallyAction.cloneAction());
            fFinallyThread.schedule();

            return;
        }

        fState = COMPLETE;
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXFinallyAction clone = new STAXFinallyAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fTryAction = fTryAction;
        clone.fFinallyAction = fFinallyAction;
        
        return clone;
    }
    
    // STAXThreadCompleteListener method

    public void threadComplete(STAXThread thread, int endCode)
    {
        // Note:  thread.getParentThread() should be the thread we are
        // running on.
            
        if (DEBUG)
            System.out.println("STAXFinallyAction::threadComplete(): " +
                               "fState=" + getStateAsString());
        
        // If no inheritable conditions were added to the condition stack
        // when running the finally action, put the inheritable conditions
        // saved before the finally action ran back on the condition stack
        
        fHasInheritableConditions = false;
        
        thread.getParentThread().visitConditions(new STAXVisitor()
        {
            public void visit(Object o, Iterator iter)
            {
                STAXCondition condition = (STAXCondition)o;
                
                if (condition.isInheritable())
                {
                    fHasInheritableConditions = true;
                }
            }
        });

        if (!fHasInheritableConditions)
        {
            if (DEBUG)
                System.out.println(
                    "STAXFinallyAction::threadComplete(): " +
                    "Restore saved conditions");

            for (STAXCondition cond : fSaveConditionList)
            {
                thread.getParentThread().addCondition(cond);
            }
        }
            
        fState = THREAD_COMPLETE;

        if (DEBUG)
            System.out.println(
                "STAXFinallyAction::threadComplete(): Remove " +
                "HardHoldThread condition: " +
                thread.getParentThread().getConditionStack());

        thread.getParentThread().removeCondition(fHardHoldCondition);
        thread.getParentThread().schedule();
    }

    private STAXHardHoldThreadCondition fHardHoldCondition =
        new STAXHardHoldThreadCondition("Finally");
    private int fState = INIT;
    private STAXAction fTryAction;
    private STAXAction fFinallyAction;
    private STAXThread fFinallyThread = null;
    private List<STAXCondition> fSaveConditionList =
        new ArrayList<STAXCondition>();
    private boolean fSavedConditions = false;
    private boolean fHasInheritableConditions = false;
}
