/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.ArrayList;
import java.util.HashMap;

public class STAXParallelAction extends STAXActionDefaultImpl
                                implements STAXThreadCompleteListener
{
    static final int INIT = 0;
    static final int WAIT_THREADS = 1;
    static final int THREADS_COMPLETE = 2;
    static final int COMPLETE = 3;

    static final String INIT_STRING = "INIT";
    static final String WAIT_THREADS_STRING = "WAIT_THREADS";
    static final String THREADS_COMPLETE_STRING = "THREADS_COMPLETE";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXParallelAction()
    { /* Do Nothing */ }

    public STAXParallelAction(ArrayList<STAXAction> actionList)
    {
        fActionList = actionList;
    }

    public void setActionList(ArrayList<STAXAction> actionList)
    {
        fActionList = actionList;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case WAIT_THREADS:
                return WAIT_THREADS_STRING;
            case THREADS_COMPLETE:
                return THREADS_COMPLETE_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo()
    {
        return fActionList.size() + " ";
    }

    public String getDetails()
    {
        return "ActionListSize:" + fActionList.size() + 
               ";State:" + getStateAsString() +
               ";ActionList:" + fActionList +
               ";HoldThreadCondition:" + fHoldCondition +
               ";HardHoldThreadCondition:" + fHardHoldCondition +
               ";ThreadMap:" + fThreadMap;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            // Check if there is a maximum number of STAX-Threads that a
            // job can run at the same time/
            // Note: maxSTAXThreads=0 indicates no maximum

            int maxSTAXThreads = thread.getJob().getMaxSTAXThreads();

            if (maxSTAXThreads != 0)
            {
                // Check if we'll exceed the maximum number of STAX-Threads
                // allowed by adding the number of STAX-Threads to be
                // started by this parallel element to the number of threads
                // that are currently running in the job.

                int jobThreads = thread.getJob().getNumThreads();

                if ((jobThreads + fActionList.size()) > maxSTAXThreads)
                {
                    fState = COMPLETE;
                    thread.popAction();

                    String errMsg = "Cannot start " + fActionList.size() +
                        " new STAX Thread(s) in parallel.\n" +
                        "Exceeded MaxSTAXThreads=" + maxSTAXThreads +
                        " (the maximum number of STAX Threads that " +
                        "can be running simultaneously in a job).";

                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME, errMsg));

                    // Set the signal message (so that it contains this
                    // element in the call stack with its correct state)
                    // before changing the state and popping this action
                    // from the thread's action stack.

                    thread.setSignalMsgVar(
                        "STAXMaxThreadsExceededMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("STAXMaxThreadsExceeded");

                    return;
                }
            }

            // Setup all the new threads.  We need to synchronize on the
            // Thread map so that all the threads are added before the
            // threadComplete() method does its checks.

            synchronized (fThreadMap)
            {
                for (int i = 0; i < fActionList.size(); ++i)
                {
                    STAXThread childThread;

                    try
                    {
                        childThread = thread.createChildThread2();
                    }
                    catch (STAXExceedsMaxThreadsException e)
                    {
                        fState = COMPLETE;
                        thread.popAction();

                        String errMsg = "Cannot start " +
                            (fActionList.size() - i) + " out of " +
                            fActionList.size() +
                            " new STAX Threads in parallel." +
                            e.getMessage();

                        setElementInfo(new STAXElementInfo(
                            getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                            errMsg));

                        thread.setSignalMsgVar(
                            "STAXMaxThreadsExceededMsg",
                            STAXUtil.formatErrorMessage(this));

                        thread.raiseSignal("STAXMaxThreadsExceeded");

                        return;
                    }

                    STAXAction childThreadAction = fActionList.get(i);

                    fThreadMap.put(childThread.getThreadNumber(), childThread);
                    childThread.addCompletionNotifiee(this);
                    childThread.pushAction(childThreadAction.cloneAction());
                    childThread.schedule();
                }

                fState = WAIT_THREADS;
                thread.addCondition(fHoldCondition);
            }
        }
        else if (fState == THREADS_COMPLETE)
        {
            // XXX: Maybe set some variables?
            fState = COMPLETE;
            thread.popAction();
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        synchronized (fThreadMap)
        {
            if (!fThreadMap.isEmpty())
            {
                // Add a hard hold, so we can wait until our children terminate.
                // This is removed by threadComplete().

                thread.addCondition(fHardHoldCondition);

                // Now, iterate of our children and tell them to terminate

                for (STAXThread childThread : fThreadMap.values())
                {
                    childThread.terminate(
                        STAXThread.THREAD_END_STOPPED_BY_PARENT);
                }

                // Now, return so we can go to sleep

                return;
            }
        }

        fState = COMPLETE;
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXParallelAction clone = new STAXParallelAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fActionList = fActionList;

        return clone;
    }

    // STAXThreadCompleteListner method

    public void threadComplete(STAXThread thread, int endCode)
    {
        synchronized (fThreadMap)
        {
            fThreadMap.remove(thread.getThreadNumberAsInteger());

            if (fThreadMap.isEmpty())
            {
                fState = THREADS_COMPLETE;

                // thread.getParentThread() should be the thread we are
                // running on.

                thread.getParentThread().removeCondition(fHoldCondition);
                thread.getParentThread().removeCondition(fHardHoldCondition);
                thread.getParentThread().schedule();
            }
        }
    }

    private STAXHoldThreadCondition fHoldCondition = 
        new STAXHoldThreadCondition("Parallel");
    private STAXHardHoldThreadCondition fHardHoldCondition =
        new STAXHardHoldThreadCondition("Parallel");
    private int fState = INIT;
    private HashMap<Integer, STAXThread> fThreadMap =
        new HashMap<Integer, STAXThread>();
    private ArrayList<STAXAction> fActionList = new ArrayList<STAXAction>();
}
