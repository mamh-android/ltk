/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.List;
import java.util.HashMap;

public class STAXParallelIterateAction extends STAXActionDefaultImpl
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

    public STAXParallelIterateAction()
    { /* Do Nothing */ }

    public STAXParallelIterateAction(String itemvar, String in, 
                                     String indexvar, STAXAction action)
    {
        fItemvar = itemvar;
        fIn = in;
        fIndexvar = indexvar; 
        fAction = action;
    }

    public void setItemVar(String itemvar) { fItemvar = itemvar; }
    public void setIn(String in) { fIn = in; }
    public void setIndexVar(String indexvar) { fIndexvar = indexvar; }
    public void setMaxThreads(String maxthreads) { fMaxThreads = maxthreads; }
    public void setAction(STAXAction action) { fAction = action; }

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

    public String getXMLInfo()
    {
        StringBuffer result = new StringBuffer();

        result.append("<paralleliterate var=\"").append(fItemvar).append(
            "\" in=\"").append(fIn).append("\"");

        if (!fIndexvar.equals(""))
            result.append(" indexvar=\"").append(fIndexvar).append("\"");

        // Only show maxthreads value if not set to the default value of 0
        if (fMaxThreads != "0")
            result.append(" maxthreads=\"").append(fMaxThreads).append("\"");

        result.append(">");

        return result.toString();
    }

    // Used to generate the Call Stack information for this element

    public String getInfo()
    {
        // Format:
        //   <numThreadsSubmitted>/<listSize> <state> <list>
        //
        //  where:
        //    <listSize> is the size of the list (or 0 if not available yet)
        //    <numThreadsSubmitted> is the number of threads that have been
        //                          submitted so far
        //    <state>    is the string version of the current state
        //    <list>     is the unevaluated list value
        //
        //  The total length of the message is limited to 40 characters.
        //  If it exceeds 40 characters, "..." is appended.

        String info =  fListSize + "/" + fNumThreadsSubmitted + " " +
            getStateAsString() + " " + fIn;

        if (info.length() > 40)
            info += info.substring(0, 40) + "...";

        return info;
    }

    public String getDetails()
    {
        return "ListSize:" + fListSize + 
               ";State:" + getStateAsString() +
               ";NumThreadsSubmitted:" + fNumThreadsSubmitted +
               ";Itemvar:" + fItemvar +
               ";In:" + fIn +
               ";Indexvar:" + fIndexvar +
               ";Maxthreads:" + fMaxThreadsInt +
               ";Action:" + fAction +
               ";HoldThreadCondition:" + fHoldCondition +
               ";HardHoldThreadCondition:" + fHardHoldCondition + 
               ";ThreadMap:" + fThreadMap +
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
                // Create a Python list/tuple from fIn and then extract it
                // into a Java List.

                fList = thread.pyListEval(fIn);
                fListSize = fList.size();

                if (fListSize == 0)
                {
                    // Nothing in list, no threads to start; raise a signal.

                    fState = THREADS_COMPLETE;
                    thread.popAction();

                    setElementInfo(new STAXElementInfo(evalElem, evalAttr));
                    
                    thread.setSignalMsgVar(
                        "STAXEmptyListMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("STAXEmptyList");

                    return;
                }

                // Set fMaxThreadsInt to the maximum number of threads that
                // can be running at any given time

                evalAttr = "maxthreads";
                fMaxThreadsInt = thread.pyIntEval(fMaxThreads);

                if (fMaxThreadsInt > fListSize)
                {
                    // Set to the list size
                    fMaxThreadsInt = fListSize;
                }

                if (fMaxThreadsInt == 0)
                {
                    // 0 means no maximum, so set to the list size
                    fMaxThreadsInt = fListSize;
                }
                else if (fMaxThreadsInt < 0)
                {
                    // Raise a signal

                    fState = COMPLETE;
                    thread.popAction();

                    String errMsg = "The maxthreads must be an integer >= 0" +
                        ".  Invalid value: " + fMaxThreadsInt;

                    setElementInfo(new STAXElementInfo(
                        evalElem, evalAttr, errMsg));

                    thread.setSignalMsgVar(
                        "STAXInvalidMaxThreadsMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("STAXInvalidMaxThreads");

                    return;
                }
                
                // Check if there is a maximum number of STAX-Threads that a
                // job can run at the same time/
                // Note: maxSTAXThreads=0 indicates no maximum
                
                int maxSTAXThreads = thread.getJob().getMaxSTAXThreads();

                if (maxSTAXThreads != 0)
                {
                    // Check if we'll exceed the maximum number of STAX-Threads
                    // allowed by adding the number of STAX-Threads to be
                    // started by this paralleliterate element to the number
                    // of threads that are currently running in the job.

                    evalAttr = "in";

                    int jobThreads = thread.getJob().getNumThreads();
                    
                    if ((jobThreads + fMaxThreadsInt) > maxSTAXThreads)
                    {
                        fState = COMPLETE;
                        thread.popAction();

                        String errMsg = "Cannot start " + fMaxThreadsInt +
                            " new STAX Thread(s) in parallel.\n" +
                            "Exceeded MaxSTAXThreads=" + maxSTAXThreads +
                            " (the maximum number of STAX Threads that " +
                            "can be running simultaneously in a job).";

                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr, errMsg));

                        thread.setSignalMsgVar(
                            "STAXMaxThreadsExceededMsg",
                            STAXUtil.formatErrorMessage(this));

                        thread.raiseSignal("STAXMaxThreadsExceeded");

                        return;
                    }
                }

                synchronized (fThreadMap)
                {
                    fNumThreadsSubmitted = fMaxThreadsInt;

                    for (int i = 0; i < fMaxThreadsInt; i++)
                    {
                        // Setup up a new thread for each iteration
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
                                (fMaxThreadsInt - i) + " out of " +
                                fMaxThreadsInt +
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
  
                        // Assign indexvar Jython variable, if provided

                        if (fIndexvar != null && !fIndexvar.equals(""))
                        {
                            evalAttr = "indexvar";
                            childThread.pySetVar(fIndexvar, new Integer(i));
                        }

                        // Assign itemvar Jython variable

                        evalAttr = "var";
                        Object itemValue = (Object)fList.get(i);
                        childThread.pySetVar(fItemvar, itemValue);
                        fThreadMap.put(childThread.getThreadNumber(),
                                       childThread);
                        childThread.addCompletionNotifiee(this);
                        childThread.pushAction(fAction.cloneAction());
                        childThread.schedule();
                    }
                        
                    fState = WAIT_THREADS;
                    thread.addCondition(fHoldCondition);
                }
          
                return;
            }
            else if (fState == THREADS_COMPLETE)
            {
                fState = COMPLETE;
                thread.popAction();
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
        synchronized (fThreadMap)
        {
            if (!fThreadMap.isEmpty())
            {
                // Add a hard hold, so we can wait until our children terminate.
                // This is removed by threadComplete().

                thread.addCondition(fHardHoldCondition);

                // Now, iterate all our children and tell them to terminate

                for (STAXThread childThread : fThreadMap.values())
                {
                    childThread.terminate(
                        STAXThread.THREAD_END_STOPPED_BY_PARENT);
                }
            }

            fState = COMPLETE;  
        }

        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXParallelIterateAction clone = new STAXParallelIterateAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fItemvar = fItemvar;
        clone.fIn = fIn;
        clone.fIndexvar = fIndexvar;
        clone.fMaxThreads = fMaxThreads;
        clone.fAction = fAction;

        return clone;
    }

    // STAXThreadCompleteListener method

    public void threadComplete(STAXThread thread, int endCode)
    {
        synchronized (fThreadMap)
        {
            fThreadMap.remove(thread.getThreadNumberAsInteger());

            // Check if we should start another thread

            if ((fState == WAIT_THREADS) &&
                (fNumThreadsSubmitted < fListSize))
            {
                // The maxThreads attribute was set to a value > 0 and
                // and there are more threads in the list to start, and
                // the state is still WAIT_THREADS indicating that no
                // conditions have occurred that would prevent starting
                // additional threads.

                // Setup and start a new thread

                fNumThreadsSubmitted++;

                STAXThread childThread = thread.getParentThread().
                    createChildThread();

                // Assign indexvar Jython variable, if provided

                if (fIndexvar != null && !fIndexvar.equals(""))
                {
                    childThread.pySetVar(
                        fIndexvar, new Integer(fNumThreadsSubmitted - 1));
                }

                // Assign itemvar Jython variable

                Object itemValue = (Object)fList.get(fNumThreadsSubmitted - 1);
                childThread.pySetVar(fItemvar, itemValue);
                fThreadMap.put(childThread.getThreadNumber(), childThread);

                childThread.addCompletionNotifiee(this);
                childThread.pushAction(fAction.cloneAction());
                childThread.schedule();
            }
            else if (fThreadMap.isEmpty())
            {
                if (fState < THREADS_COMPLETE) fState = THREADS_COMPLETE;

                // thread.getParentThread() should be the thread we are
                // running on.

                thread.getParentThread().removeCondition(fHoldCondition);
                thread.getParentThread().removeCondition(fHardHoldCondition);
                thread.getParentThread().schedule();
            }
        }
    }

    private STAXHoldThreadCondition fHoldCondition = 
        new STAXHoldThreadCondition("ParallelIterate");
    private STAXHardHoldThreadCondition fHardHoldCondition =
        new STAXHardHoldThreadCondition("ParallelIterate");
    private int fState = INIT;
    private HashMap<Integer, STAXThread> fThreadMap =
        new HashMap<Integer, STAXThread>();
    private List fList = null;
    private int fListSize = 0;
    private int fNumThreadsSubmitted = 0;

    private String fItemvar = new String();
    private String fIn = new String();
    private String fIndexvar = new String();
    private String fMaxThreads = "0";
    private int fMaxThreadsInt = 0;
    private STAXAction fAction = null;
}
