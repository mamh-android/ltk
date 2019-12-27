/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.TreeMap;
import java.util.Iterator;
import java.util.HashMap;
import org.python.core.Py;

public class STAXBlockAction extends STAXActionDefaultImpl
                             implements STAXTimedEventListener
{
    static final int INIT = 0;
    static final int ACTION_CALLED = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String ACTION_CALLED_STRING = "ACTION_CALLED";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    static final int BLOCK_HELD = 0;
    static final int BLOCK_RUNNING = 1;

    static final String BLOCK_HELD_STRING = new String("Held");
    static final String BLOCK_RUNNING_STRING = new String("Running");
    static final String BLOCK_UNKNOWN_STRING = new String("Unknown");

    public STAXBlockAction()
    { /* Do Nothing */ }

    public STAXBlockAction(String name, STAXAction action)
    {
        fName = name;
        fUnevalName = name;
        fAction = action;
    }

    public String getName() { return fName; }

    public void setName(String name)
    {
        fUnevalName = name;
    }

    public void setBlockAction(STAXAction action)
    {
        fAction = action;
    }

    public STAXThread getOwningThread() { return fOwningThread; }

    public STAXTimestamp getStartTimestamp() { return fStartTimestamp; }

    public int getBlockState() { return fBlockState; }

    public String getBlockStateAsString()
    {
        switch (fBlockState)
        {
            case BLOCK_HELD:
                return BLOCK_HELD_STRING;
            case BLOCK_RUNNING:
                return BLOCK_RUNNING_STRING;
            default:
                return BLOCK_UNKNOWN_STRING;
        }
    }

    public void holdBlock()
    {
        holdBlock(0);  // 0 indicates to hold indefinitely
    }

    public void holdBlock(long timeout)
    {
        fBlockState = BLOCK_HELD;
        String msg = "Holding block: "  + fName;

        // Add a timed event for the hold if a non-zero timeout was specified

        if (timeout != 0)
        {
            fHoldTimedEvent = new STAXTimedEvent(
                System.currentTimeMillis() + timeout, this);

            fOwningThread.getJob().getSTAX().getTimedEventQueue().addTimedEvent(
                fHoldTimedEvent);

            msg += ", timeout: " + timeout + " ms";
        }

        fOwningThread.getJob().log(STAXJob.JOB_LOG, "info", msg);
        
        HashMap<String, String> blockHoldMap = new HashMap<String, String>();
        blockHoldMap.put("type", "block");
        blockHoldMap.put("block", fName);
        blockHoldMap.put("status", "hold");
        blockHoldMap.put("name", fName);

        fOwningThread.getJob().generateEvent(
            STAXBlockActionFactory.STAX_BLOCK_EVENT, blockHoldMap);

        fOwningThread.visitChildren(new STAXVisitorHelper(fHoldCondition)
        {
            public void visit(Object o, Iterator iter)
            {
                STAXThread childThread = (STAXThread)o;

                childThread.visitChildren(this);
                childThread.addCondition((STAXCondition)fData);
            }
        });

        fOwningThread.addCondition(fHoldCondition);
    }

    public void releaseBlock()
    {
        fBlockState = BLOCK_RUNNING;

        String msg = "Releasing block: "  + fName;
        fOwningThread.getJob().log(STAXJob.JOB_LOG, "info", msg);

        HashMap<String, String> blockReleaseMap =
            new HashMap<String, String>();
        blockReleaseMap.put("type", "block");
        blockReleaseMap.put("block", fName);
        blockReleaseMap.put("status", "release");
        blockReleaseMap.put("name", fName);
            
        fOwningThread.getJob().generateEvent(
            STAXBlockActionFactory.STAX_BLOCK_EVENT, blockReleaseMap);

        if (fHoldTimedEvent != null)
        {
            fOwningThread.getJob().getSTAX().getTimedEventQueue().
                removeTimedEvent(fHoldTimedEvent);
            fHoldTimedEvent = null;
        }

        fOwningThread.visitChildren(new STAXVisitorHelper(fHoldCondition)
        {
            public void visit(Object o, Iterator iter)
            {
                STAXThread childThread = (STAXThread)o;

                childThread.visitChildren(this);
                childThread.removeCondition((STAXCondition)fData);
                childThread.schedule();
            }
        });

        fOwningThread.removeCondition(fHoldCondition);
        fOwningThread.schedule();
    }

    public void terminateBlock()
    {
        String msg = "Terminating block: "  + fName;
        fOwningThread.getJob().log(STAXJob.JOB_LOG, "info", msg);        
        
        HashMap<String, String> blockTermMap = new HashMap<String, String>();
        blockTermMap.put("type", "block");
        blockTermMap.put("block", fName);
        blockTermMap.put("status", "terminate");
        blockTermMap.put("name", fName);
            
        fOwningThread.getJob().generateEvent(
            STAXBlockActionFactory.STAX_BLOCK_EVENT, blockTermMap);

        fOwningThread.addCondition(fTermCondition);
        fOwningThread.schedule();
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

    public String getXMLInfo()
    {
        return "<block name=\"" + fUnevalName + "\">";
    }

    public String getInfo()
    {
        return fName;
    }

    public String getDetails()
    {
        return "Name:" + fName + 
               ";Action:" + fAction + 
               ";State:" + getStateAsString() +
               ";BlockState:" + getBlockStateAsString() +
               ";StartTimestamp:" + fStartTimestamp +
               ";OwningThread:" + fOwningThread +
               ";HoldThreadCondition:" + fHoldCondition +
               ";TerminateBlockCondition:" + fTermCondition;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            // Must assign before adding the block to the block map
            fOwningThread = thread;

            // Get the current date and time and set as the starting date/time
            fStartTimestamp = new STAXTimestamp();

            // Get the Current Block Name (aka Parent Block Name)

            String parentBlockName;
            try
            {
                // STAXCurrentBlock = "None" if no current block
                parentBlockName = thread.pyStringEval("STAXCurrentBlock");
            }
            catch (STAXPythonEvaluationException e)
            {
                parentBlockName = "None";
            }

            if (!parentBlockName.equals("None"))
            {
                try
                {
                    fName = parentBlockName + "." + 
                            thread.pyStringEval(fUnevalName);
                }
                catch (STAXPythonEvaluationException e)
                {
                    fState = COMPLETE;
                    thread.popAction();

                    thread.pySetVar("STAXBlockRC",
                                    new Integer(STAXJob.ABNORMAL_STATUS));

                    setElementInfo(new STAXElementInfo(getElement(), "name"));

                    thread.setSignalMsgVar(
                        "STAXPythonEvalMsg",
                        STAXUtil.formatErrorMessage(this), e);

                    thread.raiseSignal("STAXPythonEvaluationError");

                    return;
                }
            }

            // Enter Block

            @SuppressWarnings("unchecked")
            TreeMap<String, STAXBlockAction> blockMap =
                (TreeMap<String, STAXBlockAction>)thread.getJob().getData(
                    "blockMap");

            if (blockMap != null)
            {
                boolean valid_blockName;

                synchronized (blockMap)
                {
                    if (blockMap.containsKey(fName))
                        valid_blockName = false;
                    else
                    {
                        blockMap.put(fName, this);                    
                        valid_blockName = true;
                    }
                }

                if (!valid_blockName)
                {
                    fState = COMPLETE;
                    thread.popAction();
                    thread.pySetVar("STAXBlockRC",
                                    new Integer(STAXJob.ABNORMAL_STATUS));

                    setElementInfo(new STAXElementInfo(
                        getElement(), "name",
                        "A block with name \"" + fName + "\" already exists." +
                        "  You cannot have more than one block with the " +
                        "same name.\nNote that this situation can easily " +
                        "occur if you specify a <block> element using a " +
                        "literal block name within a <paralleliterate> " +
                        "element.\nInstead, use a variable in your block " +
                        "name to uniquely identify it.  For example: " +
                        "<block name=\"'BlockA_%s' % machName\">"));

                    thread.setSignalMsgVar(
                        "STAXInvalidBlockNameMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("STAXInvalidBlockName");

                    return;
                }
            }

            try
            {
                thread.pySetVar("STAXCurrentBlock", fName);
                thread.pyExec("STAXBlockStack.append(STAXCurrentBlock)");
            }
            catch (STAXPythonEvaluationException e)
            {
                fState = COMPLETE;
                thread.popAction();
                
                thread.pySetVar("STAXBlockRC",
                                new Integer(STAXJob.ABNORMAL_STATUS));

                thread.getJob().log(STAXJob.JOB_LOG, "error", 
                    "STAXBlockAction: Enter block " + fName +
                    " failed with " + e.toString());
                return;
            }
            
            HashMap<String, String> blockBeginMap =
                new HashMap<String, String>();
            blockBeginMap.put("type", "block");
            blockBeginMap.put("block", fName);
            blockBeginMap.put("status", "begin");
            blockBeginMap.put("name", fName);
                
            thread.getJob().generateEvent(
                STAXBlockActionFactory.STAX_BLOCK_EVENT, blockBeginMap);

            thread.pushAction(fAction.cloneAction());
            fState = ACTION_CALLED;
        }
        else if (fState == ACTION_CALLED)
        {
            HashMap<String, String> blockEndMap =
                new HashMap<String, String>();
            blockEndMap.put("type", "block");
            blockEndMap.put("block", fName);
            blockEndMap.put("status", "end");
            blockEndMap.put("name", fName);
            
            thread.getJob().generateEvent(
                STAXBlockActionFactory.STAX_BLOCK_EVENT, blockEndMap);
                                    
            fState = COMPLETE;

            thread.pySetVar("STAXBlockRC",
                            new Integer(STAXJob.NORMAL_STATUS));

            // Exit Block
            exitBlock(fName, thread);

            thread.popAction();
        }
    }

    public void exitBlock(String name, STAXThread thread)
    {
        try
        {
            if (thread.pyBoolEval(
                "STAXBlockStack[len(STAXBlockStack)-1] != " +
                "STAXCurrentBlock"))
            {
                thread.getJob().log(STAXJob.JOB_LOG, "error",
                    "STAXBlockAction: Exit Block " + name +
                    " failed.  This block is not on the STAXBlockStack");
            }
            else
            {
                thread.pyExec("STAXBlockStack.pop()");
                if (thread.pyBoolEval("len(STAXBlockStack) > 0"))
                    thread.pyExec("STAXCurrentBlock = " +
                        "STAXBlockStack[len(STAXBlockStack)-1]");
                else
                    thread.pySetVar("STAXCurrentBlock", Py.None);
            }
        }
        catch (STAXPythonEvaluationException e)
        {
            thread.getJob().log(STAXJob.JOB_LOG, "error", 
                "STAXBlockAction: Exit Block(" + name +
                " failed with " + e.toString());
        }

        @SuppressWarnings("unchecked")
        TreeMap<String, STAXBlockAction> blockMap =
            (TreeMap<String, STAXBlockAction>)thread.getJob().getData(
                "blockMap");

        if (blockMap != null)
        {
            synchronized (blockMap)
            {
                blockMap.remove(name);
            }
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        if (fState == ACTION_CALLED)
        {
            HashMap<String, String> blockEndMap =
                new HashMap<String, String>();
            blockEndMap.put("type", "block");
            blockEndMap.put("block", fName);
            blockEndMap.put("status", "end");
            blockEndMap.put("name", fName);
                
            thread.getJob().generateEvent(
                STAXBlockActionFactory.STAX_BLOCK_EVENT, blockEndMap);

            // Exit Block
            exitBlock(fName, thread);
        }
                                    
        if (cond instanceof STAXTerminateBlockCondition)
        {
            if (fHoldTimedEvent != null)
            {
                fOwningThread.getJob().getSTAX().getTimedEventQueue().
                    removeTimedEvent(fHoldTimedEvent);
                fHoldTimedEvent = null;
            }

            thread.pySetVar("STAXBlockRC",
                            new Integer(STAXJob.TERMINATED_STATUS));
        }
        else
        {
            thread.pySetVar("STAXBlockRC",
                            new Integer(STAXJob.NORMAL_STATUS));
        }

        fState = COMPLETE;
        thread.removeCondition(fHoldCondition);
        thread.removeCondition(fTermCondition);
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXBlockAction clone = new STAXBlockAction();
        
        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());
        
        clone.fUnevalName = fUnevalName;
        clone.fName = fName;
        clone.fAction = fAction;

        return clone;
    }
    
    public void timedEventOccurred(STAXTimedEvent timedEvent)
    {
        fHoldTimedEvent = null;

        if (fBlockState == BLOCK_HELD)
        {
            releaseBlock();
        }
    }

    private int fState = INIT;
    private int fBlockState = BLOCK_RUNNING;
    private String fName;
    private String fUnevalName;
    private STAXAction fAction = null;
    private STAXThread fOwningThread = null;
    private STAXHoldThreadCondition fHoldCondition =
        new STAXHoldThreadCondition("Block");
    private STAXTerminateBlockCondition fTermCondition =
        new STAXTerminateBlockCondition("Block");
    private STAXTimestamp fStartTimestamp;
    private STAXTimedEvent fHoldTimedEvent = null;
}
