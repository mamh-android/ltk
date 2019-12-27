/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import com.ibm.staf.STAFUtil;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.Map;

/**
 * The representation of the action to take when a STAF command,
 * &lt;stafcmd&gt;, element is encountered. 
 * <p>
 * This is produced by the STAXSTAFCommandActionFactory.  The resulting
 * STAXSTAFCommandAction object describes a &lt;stafcmd> element.  It
 * contains the location, service, and request specified, as well as a name
 * for the STAF command to be submitted.
 *
 * @see STAXSTAFCommandActionFactory
 */
public class STAXSTAFCommandAction extends STAXActionDefaultImpl
                                   implements STAXSTAFRequestCompleteListener
                              
{
    // Initial state of the action 
    static final int INIT = 0;

    // State where the STAF request command has been submitted, and
    // now it's waiting for it to complete 
    static final int WAIT_REQUEST_COMMAND = 1;            

    // State where the action has been notified that the STAF command is
    // complete
    static final int COMMAND_COMPLETE = 2;    

    // Completion state of the action
    static final int COMPLETE = 3;

    static final String INIT_STRING = "INIT";
    static final String WAIT_REQUEST_COMMAND_STRING = "WAIT_REQUEST";
    static final String COMMAND_COMPLETE_STRING = "COMMAND_COMPLETE";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    /** 
      * Creates a new STAXSTAFCommandAction instance.
      */
    public STAXSTAFCommandAction()
    {
    }

    /**
     * Gets the factory for the action.
     * @return an instance of the action's factory
     */
    public STAXSTAFCommandActionFactory getActionFactory() { return fFactory; }

    /**
     * Sets the factory for the action.
     * @param  factory  an instance of the action's factory
     */
    public void setActionFactory(STAXSTAFCommandActionFactory factory) 
    { 
        fFactory = factory; 
    }
    
    /**
     * Gets the location (destination machine name) to which the request
     * is submitted.
     * @return the location (destination machine name)
     */
    public String getLocation() { return fLocation; }

    /**
     * Sets the location (destination machine name) to which the request
     * is submitted.  This should be either LOCAL, if you wish to make a
     * request of the local machine, or the name of the machine to which
     * you want to make a request.
     * @param  location  a string containing the name of the destination 
     *                   machine for the service request submitted
     */
    public void   setLocation(String location) 
    { 
        fLocation = location; 
        fUnevalLocation = location;
    }

    /**
     * Gets the name of the STAF service to which the request is submitted.
     * @return the name of the STAF service
     */
    public String getService() { return fService; }
    
    /**
     * Sets the name of the STAF service to which the request is submitted.
     * @param  service  the name of the STAF service
     */
    public void   setService(String service) 
    { 
        fService = service; 
        fUnevalService = service;
    }
    
    /**
     * Gets the request string to submit to the STAF service.
     * @return the request string
     */
    public String getRequest() { return fRequest; }
    
    /**
     * Sets the actual request string to submit to the STAF service.
     * @param  request  the request string
     */
    public void   setRequest(String request) 
    { 
        fRequest = request; 
        fUnevalRequest = request;
    }
    
    /**
     * Gets the name used to identify the STAF command request.
     * @return the name identifying the STAF command
     */
    public String getName() { return fName; }

    /**
     * Sets a name used by the STAX Monitor to refer to the STAF command
     * request when monitoring the job. It defaults to STAFCommand&lt;number&gt;,
     * where &lt;number&gt; is a unique number for each STAF command executed in
     * a job. 
     * @param  name  the name used to identify the STAF command request
     */
    public void   setName(String name) 
    { 
        fName = name; 
        fUnevalName = name;
    }

    /**
     * Gets the STAX-Thread instance where this action is being executed.
     * @return the STAX-Thread instance where this action is being executed
     */
    public STAXThread getThread() { return fThread; }

    /**
     * Gets the STAX-Thread instance where this action is being executed.
     * @return the STAX-Thread instance where this action is being executed
     */
    public String getCurrentBlockName() { return fCurrentBlockName; }

    /**
     * Gets the timestamp for when this action was started.
     * @return the timestamp for when this action was started
     */
    public STAXTimestamp getStartTimestamp() { return fStartTimestamp; }
    
    /**
     * Gets the request number for the STAF command request that is submitted.
     * @return the request number
     */
    public int getRequestNumber() { return fRequestNumber; }

    /**
     * Gets a string identifying the state of this action.  It could be
     * "INIT", "WAIT_REQUEST", "COMMAND_COMPLETE", "COMPLETE", or "UNKNOWN".
     * @return a string identifying the state of this action.
     */
    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case WAIT_REQUEST_COMMAND:
                return WAIT_REQUEST_COMMAND_STRING;
            case COMMAND_COMPLETE:
                return COMMAND_COMPLETE_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo()
    {
        if (fName.length() > 40)
            return fName.substring(0, 40) + "...";
        else
            return fName;
    }

    public String getDetails()
    {
        return "Name:" + fName + 
               ";Location:" + fLocation +
               ";Service:" + fService + 
               ";Request:" + fRequest +
               ";State:" + getStateAsString() + 
               ";BlockName:" + fCurrentBlockName +
               ";StartTimestamp:" + fStartTimestamp +
               ";RequestNumber:" + fRequestNumber +
               ";RequestRC:" + fRequestRC +
               ";RequestResult" + fRequestResult +
               ";HoldThreadCondition:" + fHoldCondition;
    }

    public String getXMLInfo()
    {
        StringBuffer xmlInfo = new StringBuffer();

        if (fName.startsWith("STAFCommand") || fName.equals(""))
            xmlInfo.append("<stafcmd>\n");
        else
            xmlInfo.append("<stafcmd name=\"" + fName + "\">\n");

        xmlInfo.append("  <location>").append(fLocation).append(
            "</location>\n").append("  <service>").append(fService).append(
                "</service>\n").append("  <request>").append(fRequest).append(
                    "</request>\n</stafcmd>");

        return xmlInfo.toString();
    }

    // Executes the STAF command request by doing an asynchronous submit of the
    // service request to the specified machine, adds a hold thread condition,
    // and waits for the submitted STAF command request to complete.
    //
    // If in its INIT state, it does the following:
    // - Evaluates (using Python) the location, service, and request strings
    //   to resolve variables, etc.  If a Python evaluation exception occurs,
    //   it raises a STAXPythonEvaluationError signal and pops itself off the
    //   action stack.
    // - Submits the STAF Command asyncronously.  If an error occurs
    //   submitting the STAF command, it raises a STAXCommandStartError signal
    //   and pops itself off the action stack.
    // - Adds a hold thread condition while waiting for the submitted STAF
    //   command to complete so that another thread can become active.
    // - Adds the running STAF command to the stafcmdRequestMap so that it
    //   can be listed or queried.
    // - Generates an event is generated to indicate that the STAF command
    //   request has been started.
    //
    // If in its COMPLETE_REQUEST state, it does the following:
    // - Removes its entry from the stafcmdRequestMap so that it no longer
    //   will show up in the list of active STAF commands.
    // - Pops itself off the action stack since it is now complete.
    //
    // Note that this entire method is synchronized since its state can be
    // changed on another thread (e.g. via the requestComplete method).
    
    public synchronized void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            fThread = thread;  
            String evalElem = STAXElementInfo.NO_ELEMENT_NAME;
            String evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

            try
            {                
                evalElem = "location";
                fLocation = thread.pyStringEval(fUnevalLocation);

                evalElem = "service";
                fService = thread.pyStringEval(fUnevalService);

                evalElem = "request";
                fRequest = thread.pyStringEval(fUnevalRequest);
                    
                if (!fName.equals("")) 
                {
                    evalElem = "stafcmd";
                    evalAttr = "name";
                    fName = thread.pyStringEval(fUnevalName);
                }
                else
                {
                    fName = "STAFCommand" + 
                        String.valueOf(thread.getJob().getNextCmdNumber());
                    fUnevalName = fName;
                }
            }
            catch (STAXPythonEvaluationException e)
            {
                fState = COMPLETE;
                fThread.popAction();

                // Set RC and STAFResult variables when done
                fThread.pySetVar("RC", new Integer(fRequestRC));
                fThread.pySetVar("STAFResult", 
                                 "STAF command failed to start.  " +
                                 "Raised a STAXPythonEvaluationError signal.");
                
                setElementInfo(new STAXElementInfo(evalElem, evalAttr));

                fThread.setSignalMsgVar(
                    "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

                fThread.raiseSignal("STAXPythonEvaluationError");

                return;
            }
                      
            fState = WAIT_REQUEST_COMMAND;            

            // Submit STAF Command
                
            STAFResult submitResult = fThread.getJob().submitAsync(
                fLocation, fService, fRequest, this);
                  
            if (submitResult.rc == 0)
            {                    
                fRequestNumber = 
                    (new Integer(submitResult.result)).intValue();
            }
                                 
            if ((submitResult.rc != 0) || (fRequestNumber == 0))
            {
                // Request failed
                fState = COMPLETE;
                fThread.popAction();

                // Set RC and STAFResult variables when done
                fThread.pySetVar("RC", new Integer(fRequestRC));
                fThread.pySetVar("STAFResult", 
                                 "STAF command failed to start.  " +
                                 "Raised a STAXCommandStartError signal.");
                
                setElementInfo(new STAXElementInfo(
                    getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    "STAF command failed to start with RC: " +
                    submitResult.rc + " Result: " + submitResult.result));

                fThread.setSignalMsgVar(
                    "STAXCommandStartErrorMsg",
                    STAXUtil.formatErrorMessage(this));

                fThread.raiseSignal(new String("STAXCommandStartError"));
                    
                return;
            }

            fThread.addCondition(fHoldCondition);

            // Set to the current date and time.
            fStartTimestamp = new STAXTimestamp();
            
            // Add the running STAF command to the stafcmdRequestMap.

            @SuppressWarnings("unchecked")
            TreeMap<String, STAXSTAFCommandAction> stafcmds =
                (TreeMap<String, STAXSTAFCommandAction>)fThread.getJob().getData(
                    "stafcmdRequestMap");

            if (stafcmds != null)
            {
                synchronized (stafcmds)
                {
                    stafcmds.put(String.valueOf(fRequestNumber), this);
                }
            }

            // Set Current Block Name
            try
            {
                fCurrentBlockName = fThread.pyStringEval("STAXCurrentBlock");
            }
            catch (STAXPythonEvaluationException e)
            {
                fCurrentBlockName = "";  //Shouldn't happen
            }
            
            HashMap<String, String> stafCmdMap = new HashMap<String, String>();
            stafCmdMap.put("type", "command");
            stafCmdMap.put("block", fCurrentBlockName);
            stafCmdMap.put("status", "start");
            stafCmdMap.put("location", fLocation);
            stafCmdMap.put("requestNumber", String.valueOf(fRequestNumber));
            stafCmdMap.put("service", fService);
            stafCmdMap.put("request", STAFUtil.maskPrivateData(fRequest));
            stafCmdMap.put("name", fName);
                
            fThread.getJob().generateEvent(
                STAXSTAFCommandActionFactory.STAX_STAFCOMMAND_EVENT, 
                stafCmdMap);
        }        
        else if (fState == COMMAND_COMPLETE)
        {
            TreeMap stafcmds = (TreeMap)fThread.getJob().getData(
                "stafcmdRequestMap");

            if (stafcmds != null)
            {
                synchronized (stafcmds)
                {
                    stafcmds.remove(String.valueOf(fRequestNumber));
                }
            }
                
            fState = COMPLETE;
            fThread.popAction();
        }
    }
     
    // Note that this entire method is synchronized since the state of the
    // action can be changed on another thread (via the requestComplete method).
    
    public synchronized void handleCondition(STAXThread thread, 
        STAXCondition cond)
    {
        fState = COMPLETE;

        // If fRequestNumber is 0, then the execute method has not been run,
        // so the STAF command has not been submitted

        if (fRequestNumber != 0)
        {
            // Can't stop the STAF command since we don't have a way to
            // terminate a stafcmd.

            // Remove the stafcmd request # from the stafcmdRequestMap

            TreeMap stafcmds = (TreeMap)thread.getJob().getData(
                "stafcmdRequestMap");

            if (stafcmds != null)
            {
                synchronized (stafcmds)
                {
                    stafcmds.remove(String.valueOf(fRequestNumber));
                }
            }
        
            // Generate an event to indicate that the STAF command is stopped
        
            HashMap<String, String> stafCmdMap = new HashMap<String, String>();
            stafCmdMap.put("type", "command");
            stafCmdMap.put("block", fCurrentBlockName);
            stafCmdMap.put("status", "stop");
            stafCmdMap.put("location", fLocation);
            stafCmdMap.put("requestNumber", String.valueOf(fRequestNumber));
            stafCmdMap.put("service", fService);
            stafCmdMap.put("request", STAFUtil.maskPrivateData(fRequest));
            stafCmdMap.put("name", fName);
        
            thread.getJob().generateEvent(
                STAXSTAFCommandActionFactory.STAX_STAFCOMMAND_EVENT,
                stafCmdMap);
        
            // Remove the hold condition that was added when the STAF command
            // was submitted by the init() method

            thread.removeCondition(fHoldCondition);
        }

        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXSTAFCommandAction clone = new STAXSTAFCommandAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalLocation = fUnevalLocation;
        clone.fUnevalService = fUnevalService;
        clone.fUnevalRequest = fUnevalRequest;
        clone.fUnevalName = fUnevalName;

        clone.fCurrentBlockName = fCurrentBlockName;
        clone.fFactory = fFactory;
        clone.fLocation = fLocation;
        clone.fService = fService;
        clone.fRequest = fRequest;
        clone.fName = fName;

        return clone;
    }

    // requestComplete is a STAXSTAFRequestCompleteListener interface method
    //
    // Called when the submitted STAF command request completes so it can set
    // variables RC and STAFResult with the result from the submitted STAF
    // command request and remove the hold thread condition and schedule the
    // thread to run.  Generates an event to indicate that the submitted
    // STAF command request has completed.
    //
    // Note that this entire method is synchronized since the state of the
    // action can be changed on another thread (via the execute and
    // handleCondition methods).

    public synchronized void requestComplete(int requestNumber, 
        STAFResult result)
    {
        if (fState == COMPLETE) return;
        
        HashMap<String, String> stafCmdMap = new HashMap<String, String>();
        stafCmdMap.put("type", "command");
        stafCmdMap.put("block", fCurrentBlockName);
        stafCmdMap.put("status", "stop");
        stafCmdMap.put("location", fLocation);
        stafCmdMap.put("requestNumber", String.valueOf(fRequestNumber));
        stafCmdMap.put("service", fService);
        stafCmdMap.put("request", STAFUtil.maskPrivateData(fRequest));
        stafCmdMap.put("name", fName);

        fThread.getJob().generateEvent(
            STAXSTAFCommandActionFactory.STAX_STAFCOMMAND_EVENT,
            stafCmdMap);
        
        fRequestRC = result.rc;
        fRequestResult = result.result;

        // Set RC, STAFResult, and STAFResultContext variables when done

        fThread.pySetVar("RC", new Integer(fRequestRC));
        
        // Unmarshall the STAFResult using the Jython unmarshall API

        try
        { 
            fThread.pySetVar("STAFResultString", result.result);
            fThread.pyExec(
             "STAFResultContext = STAFMarshalling.unmarshall(STAFResultString)\n" +
             "STAFResult = STAFResultContext.getRootObject()");
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(
                getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                "Error while unmarshalling the STAFCommand result"));

            fThread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            fThread.raiseSignal("STAXPythonEvaluationError");
        }  

        fState = COMMAND_COMPLETE;
        fThread.removeCondition(fHoldCondition);
        fThread.schedule();
    }    
        
    STAXThread fThread = null;
    int fState = INIT;
    private STAXHoldThreadCondition fHoldCondition = 
        new STAXHoldThreadCondition("STAFCommand");
    
    private String fLocation = new String();
    private String fService = new String();
    private String fRequest = new String();
    private String fName = new String();
    
    private String fUnevalLocation = new String();
    private String fUnevalService = new String();
    private String fUnevalRequest = new String();
    private String fUnevalName = new String();

    private int fRequestNumber = 0;

    // -1 indicates an error occurred before the request was submitted
    private int fRequestRC = -1;

    private String fRequestResult = new String(); 
    private STAXTimestamp fStartTimestamp;
    private STAXSTAFCommandActionFactory fFactory;
    private String fCurrentBlockName = new String();
}
