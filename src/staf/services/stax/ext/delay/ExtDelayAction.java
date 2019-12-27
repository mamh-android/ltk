/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.extension.samples.extdelay;
import com.ibm.staf.*;
import java.util.HashMap;
import com.ibm.staf.service.stax.*;

public class ExtDelayAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
   
    static final int DELAY = 1;
    
    static final int COMPLETE = 2;

    public ExtDelayAction()
    { /* Do Nothing */ }

    public ExtDelayAction(String delayValue)
    { 
        fUnevalDelayValue = delayValue;
        fDelayValue = delayValue;
    }

    public String getDelayValue() { return fDelayValue; } 

    public void setDelayValue(String delayValue) 
    {
        fUnevalDelayValue = delayValue; 
        fDelayValue = delayValue; 
    }
    
    public String getName() { return fName; } 

    public void setName(String name) 
    {
        fName = name;
    }
    
    public ExtDelayActionFactory getActionFactory() { return fFactory; }
    
    public void setActionFactory(ExtDelayActionFactory factory) 
    {
        fFactory = factory; 
    }
    
    public String getXMLInfo()
    {
        return "<ext-delay \">" + fDelayValue + "</ext-delay>";
    }

    public String getInfo()
    {
            return fDelayValue;
    }

    public String getDetails()
    {
        return "Delay Name:" + fName + " DelayValue:" + fDelayValue;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            fThread = thread;
            
            try
            {
                fCurrentBlockName = thread.pyStringEval("STAXCurrentBlock");
            }
            catch (STAXPythonEvaluationException e)
            {
                fCurrentBlockName = "";  //Shouldn't happen
            }
            
            // Assign a name to uniquely identify the delay element displayed
            // in the "Active Job Elements" panel on the STAX Job Monitor.
            // Get the value for key "extDelayNum" stored in the ext-delay-map
            // for the job and increment it and use it in the name.
            
            HashMap extDelayMap = 
                (HashMap)fThread.getJob().getData("ext-delay-map");
            
            synchronized (extDelayMap)
            {
                Integer extDelayNum = (Integer)extDelayMap.get("extDelayNum");

                int num = extDelayNum.intValue() + 1;

                // Store the new delay number for the job
                extDelayMap.put("extDelayNum", new Integer(num));
            
                fName = "Delay" + num;
            }

            if (fUnevalDelayValue.equals(""))
            {
                // Assign default delay value if no value is specified
                // The default delay value is obtained via a parameter
                // named "delay" specified in the extension xml file.

                fUnevalDelayValue = fFactory.getParameter("delay");

                if (fUnevalDelayValue == null)
                {
                    // A default delay value was not provided.  Raise a signal.
                    fState = COMPLETE;
                    thread.popAction();
 
                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "Must specify a value for the \"delay\" element if " +
                        "no default value is specified.\n" +
                        "A default value can be specified via a parameter " +
                        "named delay in the extension xml file used to " +
                        "register the \"delay\" element."));

                    thread.setSignalMsgVar(
                        "ExtDelayInvalidValueMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("ExtDelayInvalidValue");

                    return;
                }

                fDelayValue = fUnevalDelayValue;
                fDelayInt = (new Integer(fDelayValue)).intValue();
            }
            else
            {
                try
                {
                    fDelayInt = thread.pyIntEval(fUnevalDelayValue);
                }
                catch (STAXPythonEvaluationException e)
                {
                    fState = COMPLETE;
                    thread.popAction();

                    setElementInfo(new STAXElementInfo(getElement()));

                    thread.setSignalMsgVar(
                        "STAXPythonEvalMsg",
                        STAXUtil.formatErrorMessage(this), e);

                    thread.raiseSignal("STAXPythonEvaluationError");

                    return;
                }
            
                if (fDelayInt <= 0)
                {
                    // Delay value must be an integer > 0.  Raise a signal.
                    fState = COMPLETE;
                    thread.popAction();
 
                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "Delay value " + fDelayInt +
                        " must be an integer > 0."));

                    thread.setSignalMsgVar(
                        "ExtDelayInvalidValueMsg",
                        STAXUtil.formatErrorMessage(this));

                    thread.raiseSignal("ExtDelayInvalidValue");

                    return;
                }
            }
            
            HashMap delayMap = new HashMap();
            delayMap.put("block", fCurrentBlockName);
            
            delayMap.put("delay", fDelayValue);
            delayMap.put("status", "start");
            delayMap.put("name", fName);
            
            thread.getJob().generateEvent(ExtDelayActionFactory.EXT_DELAY,
                delayMap);
            
            fState = DELAY;
        }
        else if (fState == DELAY)
        {
            if (fIteration > fDelayInt)
            {
                fState = COMPLETE;
            }
            else
            {
                HashMap delayMap = new HashMap();
                delayMap.put("block", fCurrentBlockName);
                delayMap.put("delay", fDelayValue);
                delayMap.put("status", "iterate");
                delayMap.put("name", fName);
                delayMap.put("currentiter", 
                    (new Integer(fIteration++)).toString());
                
                thread.getJob().generateEvent(ExtDelayActionFactory.EXT_DELAY,
                    delayMap);
        
                try
                {
                    Thread.sleep(1000);
                }
                catch (InterruptedException e)
                {
                }
            }
        }
        else if (fState == COMPLETE)
        {
            HashMap delayMap = new HashMap();
            delayMap.put("block", fCurrentBlockName);
            delayMap.put("delay", fDelayValue);
            delayMap.put("status", "stop");
            delayMap.put("name", fName);
            
            thread.getJob().generateEvent(ExtDelayActionFactory.EXT_DELAY,
                delayMap);
            
            fThread.popAction();
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        ExtDelayAction clone = new ExtDelayAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalDelayValue = fUnevalDelayValue;
        clone.fDelayValue = fDelayValue;
        clone.fName = fName;
        clone.fFactory = fFactory;
        
        return clone;
    }
    
    int fState = INIT;
    STAXThread fThread = null;

    private String fUnevalDelayValue = new String();
    private String fDelayValue = new String();
    private int fDelayInt = 0;
    
    private String fName = new String();
    private ExtDelayActionFactory fFactory;
    private String fCurrentBlockName = new String();
    private int fIteration = 1;
}
