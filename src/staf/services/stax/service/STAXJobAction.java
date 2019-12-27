/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import com.ibm.staf.service.*;
import com.ibm.staf.STAFUtil;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.Vector;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import org.python.core.Py;

/**
 * The representation of the action to take when a sub-job, &lt;job&gt;,
 * element is encountered.
 * <p>
 * This is produced by the STAXJobActionFactory.  The resulting STAXJobAction
 * object describes a &lt;job> element.  It contains the job-file or job-data
 * to be executed and other optional data, as well as a name for the sub-job
 * to be executed.
 *
 * @see STAXJobActionFactory
 */
public class STAXJobAction extends STAXActionDefaultImpl
                           implements STAXJobCompleteListener,
                                      STAXThreadCompleteListener
{
    /**
     * Flag to enable debugging information to be written to the STAX JVM log
     * to help debug a problem with the job element
     */
    static final boolean sDebug = false;

    /**
     *  Initial state of the action
     */ 
    static final int INIT = 0;

    /**
     *  Running state of the action
     */
    static final int RUNNING = 1;

    /**
     *  Completion state of the action
     */
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String RUNNING_STRING = "RUNNING";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    /**
      * Creates a new STAXJobAction instance.
      */
    public STAXJobAction()
    {
    }

    /**
     * Gets the factory for the action.
     * @return an instance of the action's factory
     */
    public STAXJobActionFactory getActionFactory() { return fFactory; }

    /**
     * Sets the factory for the action.
     * @param  factory  an instance of the action's factory
     */
    public void setActionFactory(STAXJobActionFactory factory)
    {
        fFactory = factory;
    }

    /**
     * Gets the name specified for the sub-job.
     * @return the name of the sub-job
     */
    public String getName() { return fName; }

    /**
     * Sets a name used to identify the sub-job.  It defaults to
     * the value of the function name called to start the job.
     * @param  name  the name used to identify the sub-job
     */
    public void setName(String name)
    {
        fName = name;
        fUnevalName = name;
    }

    /**
     * Gets the clear logs option specified for the sub-job.
     * @return the clear log option specified for the sub-job
     */
    public String getClearlogs() { return fClearlogs; }

    /**
     * Sets the clearlogs option which is used to determine whether
     * to delete the STAX Job and Job User logs before the job is
     * executed to ensure that only one job's contents are in the logs.
     * It defaults to the same value specified for its parent job.
     * @param  clearlogs  the clearlog option for the sub-job
     */
    public void setClearlogs(String clearlogs)
    {
        fClearlogs = clearlogs;
        fUnevalClearlogs = clearlogs;
    }

    /**
     * Gets the monitor option specified for the sub-job.
     * @return the monitor option specified for the sub-job
     */
    public String getMonitor() { return fMonitor; }

    /**
     * Sets the monitor option which is used to determine whether
     * to automatically monitor the sub-job
     * @param  monitor  the monitor option for the sub-job
     */
    public void setMonitor(String monitor)
    {
        fMonitor = monitor;
        fUnevalMonitor = monitor;
    }

    /**
     * Gets the Log TC Elapsed Time option specified for the sub-job.
     * @return the Log TC Elapsed Time option specified for the sub-job
     */
    public String getLogTCElapsedTime() { return fLogTCElapsedTime; }

    /**
     * Sets the Log TC Elapsed Time option which is used to determine whether
     * to log the elapsed time for testcases in summary records in the STAX
     * Job log and for a LIST TESTCASES request.
     * It defaults to the same value specified for its parent job.
     * @param  logTCElapsedTime  the logTCElapsedTime option for the sub-job
     */
    public void setLogTCElapsedTime(String logTCElapsedTime)
    {
        fLogTCElapsedTime = logTCElapsedTime;
        fUnevalLogTCElapsedTime = logTCElapsedTime;
    }

    /**
     * Gets the Log TC Num Starts option specified for the sub-job.
     * @return the Log TC Num Starts option specified for the sub-job
     */
    public String getLogTCNumStarts() { return fLogTCNumStarts; }

    /**
     * Sets the Log TC Num Starts option which is used to determine whether
     * to log the number of starts for testcases in summary records in the
     * STAX Job log and for a LIST TESTCASES request.
     * It defaults to the same value specified for its parent job.
     * @param  logTCNumStarts  the logTCNumStarts option for the sub-job
     */
    public void setLogTCNumStarts(String logTCNumStarts)
    {
        fLogTCNumStarts = logTCNumStarts;
        fUnevalLogTCNumStarts = logTCNumStarts;
    }
    
    /**
     * Gets the Log TC Start/Stop option specified for the sub-job.
     * @return the Log TC Start/Stop option specified for the sub-job
     */
    public String getLogTCStartStop() { return fLogTCStartStop; }

    /**
     * Sets the Log TC Start/Stop option which is used to determine whether
     * to log start and stop status records for testcases in the STAX Job log.
     * It defaults to the same value specified for its parent job.
     * @param  logTCStartStop  the logTCStartStop option for the sub-job
     */
    public void setLogTCStartStop(String logTCStartStop)
    {
        fLogTCStartStop = logTCStartStop;
        fUnevalLogTCStartStop = logTCStartStop;
    }
    
    /**
     * Gets the Python Output option specified for the sub-job.
     * @return the Python Output option specified for the sub-job
     */
    public String getPythonOutput() { return fPythonOutput; }

    /**
     * Sets the Python Output option.
     * It defaults to the same value specified for its parent job.
     * @param  pythonOutput  the pythonOutput option for the sub-job
     */
    public void setPythonOutput(String pythonOutput)
    {
        fPythonOutput = pythonOutput;
        fUnevalPythonOutput = pythonOutput;
    }
    
    /**
     * Gets the Python Log Level option specified for the sub-job.
     * @return the Python Log Level option specified for the sub-job
     */
    public String getPythonLogLevel() { return fPythonLogLevel; }

    /**
     * Sets the Python Log Level option.
     * It defaults to the same value specified for its parent job.
     * @param  pythonLogLevel  the pythonLogLevel option for the sub-job
     */
    public void setPythonLogLevel(String pythonLogLevel)
    {
        fPythonLogLevel = pythonLogLevel;
        fUnevalPythonLogLevel = pythonLogLevel;
    }

    /**
     * Gets the fully-qualified name of a file containing the XML document
     * for the STAX job to be executed.
     * @return the job file name
     */
    public String getJobFile() { return fJobFile; }

    /**
     * Gets the name of the machine where the XML job file is located
     * @return the job file machine name
     */
    public String getJobFileMachine() { return fJobFileMachine; }

    /**
     * Sets the fully-qualified name of a file containing the XML document
     * for the STAX job to be executed.
     * @param  jobXMLFile  a string containing the fully-qualified name of the
     *                     file containing the XML document for the STAX job
     *                     to be executed
     * @param  machine     a string containing the name of the machine where
     *                     the XML file is located
     */
    public void setJobFile(String jobFile, String machine)
    {
        fJobFile = jobFile;
        fUnevalJobFile = jobFile;
        fJobFileMachine = machine;
        fUnevalJobFileMachine = machine;
    }

    /**
     * Gets the string containing the XML document for the STAX job to be
     * executed.
     * @return the job data
     */
    public String getJobData() { return fJobData; }

    /**
     * Gets the indicator of whether the XML data is to be evaluated by Python
     * in the parent job.
     * @return the job data eval indicator
     */
    public String getJobDataEval() { return fJobDataEval; }

    /**
     * Sets a string containing the XML document for the STAX job to be
     * executed.
     * @param  jobData  a string containing the XML document for the
     *                  STAX job to be executed
     * @param  eval     indicates the XML data is to be evaluated by Python
     *                  in the parent job if it evaluates to true
     *      */
    public void setJobData(String jobData, String eval)
    {
        fJobData = jobData;
        fUnevalJobData = jobData;
        fJobDataEval = eval;
    }

    /**
     * Gets the name of function to call to start the STAX job.
     * @return the starting function name for the STAX job
     */
    public String getFunction() { return fFunction; }

    /**
     * Sets the name of the function to call to start the STAX job.
     * @param  function    the name of the starting function
     * @param  functionIf  indicates to ignore the function specified if it
     *                     evaluates to false
     */
    public void setFunction(String function, String functionIf)
    {
        fFunction = function;
        fUnevalFunction = function;
        fFunctionIf = functionIf;
    }

    /**
     * Gets the arguments to pass to the function called to start the STAX job.
     * @return the starting function arguments for the STAX job
     */
    public String getFunctionArgs() { return fFunctionArgs; }

    /**
     * Sets the arguments to pass to the function called to start the STAX job.
     * @param  args    the arguments passed to the starting function
     * @param  eval    indicates the arguments are to be evaluated by Python
     *                 in the parent job if it evaluates to true
     * @param  argsIf  indicates to ignore the function args specified if it
     *                 evaluates to false
     */
    public void setFunctionArgs(String args, String eval, String argsIf)
    {
        fFunctionArgs = args;
        fUnevalFunctionArgs = args;
        fFunctionArgsEval = eval;
        fFunctionArgsIf = argsIf;
    }

    /**
     * Gets a vector containing the scripts (Python code) specified for the job.
     * @return a vector containing the scripts specified for the STAX job
     */
    public Vector<String> getScripts() { return fScripts; }

    /**
     * Adds a script (Python code) to the scripts vector.
     * @param  script    a script consist of Python code
     * @param  eval      indicates the script is to be evaluated by Python
     *                   in the parent job if it evaluates to true
     * @param  scriptIf  indicates to ignore the script if it evaluates to
     *                   false
     */
    public void setScripts(String script, String eval, String scriptIf)
    {
        fUnevalScripts.add(script);
        fScriptsEval.add(eval);
        fScriptsIf.add(scriptIf);
    }

    public String getScriptFileElementName()
    {
        return fScriptFileElementName;
    }

    public void setScriptFileElementName(String elementName)
    {
        fScriptFileElementName = elementName;
    }

    /**
     * Gets a vector containing the script file names specified for the job.
     * @return a vector containing the names of the script files
     */
    public Vector<String> getScriptFiles() { return fScriptFiles; }

    /**
     * Sets the name(s) of the script file(s) containing Python code to be
     * executed in the STAX job.
     * @param  scriptFiles    the name(s) of the script file(s)
     * @param  machine        the name of the machine where the script file(s)
     *                        are located
     * @param  scriptFilesIf  indicates to ignore the scriptfiles if it
     *                        evaluates to false
     */
    public void setScriptFiles(String scriptFiles, String machine,
                               String scriptFilesIf)
    {
        fUnevalScriptFiles = scriptFiles;
        fScriptFilesMachine = machine;
        fUnevalScriptFilesMachine = machine;
        fScriptFilesIf = scriptFilesIf;
    }

    /**
     * Gets the hold timeout
     * @return the hold timeout
     */ 
    public long getHoldTimeout()
    {
        return fHoldTimeout;
    }

    /**
     * Sets the hold timeout (so that the job is started and then immediately
     * held).
     * @param holdTimeout    the maximum time to hold the job
     * @param holdIf         indicates to ignore the hold if it evaluates to
     *                       false
     */
    public void setHold(String holdTimeout, String holdIf)
    {
        fUnevalHoldTimeout = holdTimeout;
        fHoldIf = holdIf;
    }

    /**
     * Gets the STAXAction object to be executed after the sub-job has started.
     * @return the STAXAction object for the STAX job
     */
    public STAXAction getJobAction() { return fJobAction; }

    /**
     * Sets the name of the function to call to start the STAX job.
     * @param  function    the name of the STAF service
     * @param  functionIf  indicates to ignore the function specified if it
     *                     evaluates to false
     */
    public void setJobAction(STAXAction jobAction, String jobActionIf)
    {
        fJobAction = jobAction;
        fJobActionIf = jobActionIf;
    }

    /**
     * Gets the request string to submit to the STAF service.
     * @return the request string
     */
    public String getRequest() { return fRequest; }

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
     * Gets the job ID for the sub-job that is submitted.
     * @return the job ID for the sub-job
     */
    public int getJobID() { return fJobID; }

    /**
     * Gets the name of the function used to start the STAX job
     * @return the starting function name for the STAX job
     */
    public String getStartFunction() { return fStartFunction; }

    /**
     * Sets the name of the function to call to start the STAX job.
     * @parm function    the name of starting function
     */
    public void setStartFunction(String function)
    {
        fStartFunction = function;
    }

    /**
     * Gets the arguments to pass to the function called to start the STAX job.
     * @return the starting function arguments for the STAX job
     */
    public String getStartFuncArgs() { return fStartFunctionArgs; }

    /**
     * Sets the arguments to pass to the function called to start the STAX job.
     * @param  args    the arguments passed to the starting function
     */
    public void setStartFuncArgs(String args)
    {
        fStartFunctionArgs = args;
    }

    /**
     * Gets a string identifying the state of this action.  It could be
     * "INIT", "RUNNING [([job], [job-action])]", "COMPLETE", or "UNKNOWN".
     * @return a string identifying the state of this action.
     */
    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case RUNNING:
                String stateString = RUNNING_STRING;

                if (fIsJobRunning && !fIsJobActionRunning)
                    stateString += " (job)";
                else if (fIsJobRunning && fIsJobActionRunning)
                    stateString += " (job, job-action)";
                else if (!fIsJobRunning && fIsJobActionRunning)
                    stateString += " (job-action)";

                return stateString;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo()
    {
        return fName;
    }

    public String getDetails()
    {
        return "JobName:" + fName +
               ";JobID:" + fJobID +
               ";Request:" + fRequest +
               ";State:" + getStateAsString() +
               ";BlockName:" + fCurrentBlockName +
               ";StartTimestamp:" + fStartTimestamp;
    }

    /**
     *  Submits an EXECUTE request to the STAX service by doing an asynchronous
     *  submit of the request to the local STAX machine, adds a hold thread
     *  condition, and waits for the submitted job to complete.
     *  <p>
     *  If in its INIT state, it does the following:
     *  - Evaluates (using Python) values as needed. If a Python evaluation
     *    exception occurs, it raises a STAXPythonEvaluationError signal and
     *    pops itself off the action stack.
     *  - Submits the STAX EXECUTE request asynchronously.  If an error occurs
     *    submitting the request, it sets RC and STAFResult and pops itself
     *    off the action stack.
     *  - Adds a hold thread condition while waiting for the job to complete
     *    so that another thread can become active.
     *  - Adds the running job to the subJobMap so that it can be listed.
     *  - Generates an event is generated to indicate that the sub-job has
     *    been started.
     *  <p>
     *  If in its RUNNING state, but neither the job or the job-action is
     *  still running, it does the following:
     *  - Removes its entry from the subJobMap so that it no longer will show
     *    up in the list of sub-jobs.
     *  - Pops itself off the action stack since it is now complete.
     *  <p>
     *  Note that this entire method is synchronized since its state can be
     *  changed on another thread (e.g. via the jobComplete, threadComplete,
     *  or handleCondition method).
     * 
     * @parm thread    the STAXThread the sub-job is running on
     */

    public synchronized void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            if (sDebug)
                System.out.println("STAXJobAction::execute(): fState=INIT");

            fThread = thread;
            StringBuffer request = new StringBuffer("execute");

            String evalElem = STAXElementInfo.NO_ELEMENT_NAME;
            String evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
            int evalIndex = 0;

            try
            {
                if (!fUnevalJobFile.equals(""))
                {
                    evalElem = "job-file";
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

                    fJobFile = thread.pyStringEval(fJobFile);

                    request.append(" file ").append(STAFUtil.wrapData(fJobFile));

                    if (!fJobFileMachine.equals(""))
                    {
                        evalAttr = "machine";

                        fJobFileMachine =
                            thread.pyStringEval(fUnevalJobFileMachine);

                        request.append(" machine ").append(
                            STAFUtil.wrapData(fJobFileMachine));
                    }
                }
                else if (!fUnevalJobData.equals(""))
                {
                    evalElem = "job-data";
                    evalAttr = "eval";

                    if (thread.pyBoolEval(fJobDataEval))
                    {
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

                        fJobData = thread.pyStringEval(fUnevalJobData);
                    }

                    request.append(" data ").append(
                        STAFUtil.wrapData(fJobData));
                }

                if (!fUnevalName.equals(""))
                {
                    evalElem = "job";
                    evalAttr = "name";

                    fName = thread.pyStringEval(fUnevalName);

                    request.append(" jobname ").append(
                        STAFUtil.wrapData(fName));
                }

                // Evaluate the clearlogs attribute

                evalElem = "job";
                evalAttr = "clearlogs";

                if (fUnevalClearlogs.equals(""))
                {
                    // Default to parent job's option if not specified
                    fClearlogs = thread.getJob().getClearLogsAsString();
                }
                else
                {
                    fClearlogs = thread.pyStringEval(fUnevalClearlogs);

                    if (fClearlogs.equalsIgnoreCase("parent"))
                    {
                        fClearlogs = thread.getJob().getClearLogsAsString();
                    }
                    else if (fClearlogs.equalsIgnoreCase("default"))
                    {
                        fClearlogs = thread.getJob().getSTAX().
                                            getClearLogsAsString();
                    }
                    else if (fClearlogs.equalsIgnoreCase("Enabled") ||
                             fClearlogs.equalsIgnoreCase("Disabled"))
                    {
                        // Do nothing - already set to Enabled or Disabled
                    }
                    else if (thread.pyBoolEval(fUnevalClearlogs))
                    {
                        fClearlogs = "Enabled";
                    }
                    else
                    {
                        fClearlogs = "Disabled";
                    }
                }

                request.append(" clearlogs ").append(fClearlogs);

                // Evaluate the monitor attribute

                evalElem = "job";
                evalAttr = "monitor";

                if (fUnevalMonitor.equals(""))
                {
                    fMonitor = "false";
                }
                else if (thread.pyBoolEval(fUnevalMonitor))
                {
                    fMonitor = "true";
                }
                else
                {
                    fMonitor = "false";
                }

                // Evaluate the logtcelapsedtime attribute

                evalElem = "job";
                evalAttr = "logtcelapsedtime";

                if (fUnevalLogTCElapsedTime.equals(""))
                {
                    // Default to parent job's option if not specified
                    fLogTCElapsedTime =
                        thread.getJob().getLogTCElapsedTimeAsString();
                }
                else
                {
                    fLogTCElapsedTime =
                        thread.pyStringEval(fUnevalLogTCElapsedTime);

                    if (fLogTCElapsedTime.equalsIgnoreCase("parent"))
                    {
                        fLogTCElapsedTime =
                            thread.getJob().getLogTCElapsedTimeAsString();
                    }
                    else if (fLogTCElapsedTime.equalsIgnoreCase("default"))
                    {
                        fLogTCElapsedTime = thread.getJob().getSTAX().
                                            getLogTCElapsedTimeAsString();
                    }
                    else if (fLogTCElapsedTime.equalsIgnoreCase("Enabled") ||
                             fLogTCElapsedTime.equalsIgnoreCase("Disabled"))
                    {
                        // Do nothing - already set to Enabled or Disabled
                    }
                    else if (thread.pyBoolEval(fUnevalLogTCElapsedTime))
                    {
                        fLogTCElapsedTime = "Enabled";
                    }
                    else
                    {
                        fLogTCElapsedTime = "Disabled";
                    }
                }

                request.append(" logtcelapsedtime ").append(fLogTCElapsedTime);

                // Evaluate the logtcnumstarts attribute

                evalElem = "job";
                evalAttr = "logtcnumstarts";

                if (fUnevalLogTCNumStarts.equals(""))
                {
                    // Default to parent job's option if not specified
                    fLogTCNumStarts =
                        thread.getJob().getLogTCNumStartsAsString();
                }
                else
                {
                    fLogTCNumStarts =
                        thread.pyStringEval(fUnevalLogTCNumStarts);

                    if (fLogTCNumStarts.equalsIgnoreCase("parent"))
                    {
                        fLogTCNumStarts =
                            thread.getJob().getLogTCNumStartsAsString();
                    }
                    else if (fLogTCNumStarts.equalsIgnoreCase("default"))
                    {
                        fLogTCNumStarts = thread.getJob().getSTAX().
                                          getLogTCNumStartsAsString();
                    }
                    else if (fLogTCNumStarts.equalsIgnoreCase("Enabled") ||
                             fLogTCNumStarts.equalsIgnoreCase("Disabled"))
                    {
                        // Do nothing - already set to Enabled or Disabled
                    }
                    else if (thread.pyBoolEval(fUnevalLogTCNumStarts))
                    {
                        fLogTCNumStarts = "Enabled";
                    }
                    else
                    {
                        fLogTCNumStarts = "Disabled";
                    }
                }

                request.append(" logtcnumstarts ").append(fLogTCNumStarts);

                // Evaluate the logtcstartstop attribute

                evalElem = "job";
                evalAttr = "logtcstartstop";

                if (fUnevalLogTCStartStop.equals(""))
                {
                    // Default to parent job's option if not specified
                    fLogTCStartStop =
                        thread.getJob().getLogTCStartStopAsString();
                }
                else
                {
                    fLogTCStartStop =
                        thread.pyStringEval(fUnevalLogTCStartStop);

                    if (fLogTCStartStop.equalsIgnoreCase("parent"))
                    {
                        fLogTCStartStop =
                            thread.getJob().getLogTCStartStopAsString();
                    }
                    else if (fLogTCStartStop.equalsIgnoreCase("default"))
                    {
                        fLogTCStartStop = thread.getJob().getSTAX().
                                          getLogTCStartStopAsString();
                    }
                    else if (fLogTCStartStop.equalsIgnoreCase("Enabled") ||
                             fLogTCStartStop.equalsIgnoreCase("Disabled"))
                    {
                        // Do nothing - already set to Enabled or Disabled
                    }
                    else if (thread.pyBoolEval(fUnevalLogTCStartStop))
                    {
                        fLogTCStartStop = "Enabled";
                    }
                    else
                    {
                        fLogTCStartStop = "Disabled";
                    }
                }

                request.append(" logtcstartstop ").append(fLogTCStartStop);

                // Evaluate the pythonoutput attribute

                evalElem = "job";
                evalAttr = "pythonoutput";

                if (fUnevalPythonOutput.equals(""))
                {
                    // Default to parent job's option if not specified
                    fPythonOutput = STAXPythonOutput.getPythonOutputAsString(
                        thread.getJob().getPythonOutput());
                }
                else
                {
                    fPythonOutput = thread.pyStringEval(fUnevalPythonOutput);

                    if (fPythonOutput.equalsIgnoreCase("parent"))
                    {
                        fPythonOutput =
                            STAXPythonOutput.getPythonOutputAsString(
                                thread.getJob().getPythonOutput());
                    }
                    else if (fPythonOutput.equalsIgnoreCase("default"))
                    {
                        fPythonOutput =
                            STAXPythonOutput.getPythonOutputAsString(
                                thread.getJob().getSTAX().getPythonOutput());
                    }
                }

                request.append(" PYTHONOUTPUT ").append(
                    STAFUtil.wrapData(fPythonOutput));

                // Evaluate the pythonloglevel attribute

                evalElem = "job";
                evalAttr = "pythonloglevel";

                if (fUnevalPythonLogLevel.equals(""))
                {
                    // Default to parent job's option if not specified
                    fPythonLogLevel = thread.getJob().getPythonLogLevel();
                }
                else
                {
                    fPythonLogLevel = thread.pyStringEval(fUnevalPythonLogLevel);

                    if (fPythonLogLevel.equalsIgnoreCase("parent"))
                    {
                        fPythonLogLevel = thread.getJob().getPythonLogLevel();
                    }
                    else if (fPythonLogLevel.equalsIgnoreCase("default"))
                    {
                        fPythonLogLevel = thread.getJob().getSTAX().
                            getPythonLogLevel();
                    }
                }

                request.append(" PYTHONLOGLEVEL ").append(
                    STAFUtil.wrapData(fPythonLogLevel));

                // Evaluate the job-function element

                if (!fUnevalFunction.equals(""))
                {
                    evalElem = "job-function";
                    evalAttr = "if";

                    if (thread.pyBoolEval(fFunctionIf))
                    {
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

                        fFunction = thread.pyStringEval(fUnevalFunction);

                        request.append(" FUNCTION ").append(
                            STAFUtil.wrapData(fFunction));
                    }
                }

                // Evaluate the job-function-args element

                if (!fUnevalFunctionArgs.equals(""))
                {
                    evalElem = "job-function-args";
                    evalAttr = "if";

                    if (thread.pyBoolEval(fFunctionArgsIf))
                    {
                        if (thread.pyBoolEval(fFunctionArgsEval))
                        {
                            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

                            fFunctionArgs =
                                thread.pyStringEval(fUnevalFunctionArgs);
                        }

                        request.append(" ARGS ").append(
                            STAFUtil.wrapData(fFunctionArgs));
                    }
                }

                // Evaluate the job-script elements

                evalElem = "job-script";
                
                for (int i = 0; i < fUnevalScripts.size(); i++)
                {
                    evalIndex = i;
                    evalAttr = "if";
                    String scriptsIf = thread.pyStringEval(
                        fScriptsIf.elementAt(i));

                    if (thread.pyBoolEval(scriptsIf))
                    {
                        evalAttr = "eval";
                        String script = new String();

                        if (thread.pyBoolEval(fScriptsEval.elementAt(i)))
                        {
                            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

                            script = thread.pyStringEval(
                                fUnevalScripts.elementAt(i));

                            fScripts.add(script);
                        }
                        else
                        {
                            script = fUnevalScripts.elementAt(i);

                            fScripts.add(script);
                        }

                        request.append(" SCRIPT ").append(
                            STAFUtil.wrapData(script));
                    }
                }

                // Evaluate the job-scriptfile / job-scriptfiles element

                evalIndex = 0;

                if (!fUnevalScriptFiles.equals(""))
                {
                    // Use to get the element name used as could be either
                    // "job-scriptfile" or "job-scriptfiles"
                    evalElem = getScriptFileElementName();

                    evalAttr = "if";

                    if (thread.pyBoolEval(fScriptFilesIf))
                    {
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

                        // Evaluate as a list; each item in the list is a
                        // single scriptfile.

                        List scriptFileList =
                            thread.pyListEval(fUnevalScriptFiles);

                        for (int j = 0; j < scriptFileList.size(); j++)
                        {
                            String scriptFile =
                                ((Object)scriptFileList.get(j)).toString();
                            fScriptFiles.add(scriptFile);
                            request.append(" SCRIPTFILE ").append(
                                       STAFUtil.wrapData(scriptFile));
                        }

                        if (!fUnevalScriptFilesMachine.equals(""))
                        {
                            evalAttr = "machine";

                            fScriptFilesMachine =
                                thread.pyStringEval(fUnevalScriptFilesMachine);

                            request.append(" SCRIPTFILEMACHINE ").append(
                                       STAFUtil.wrapData(fScriptFilesMachine));
                        }
                    }
                }

                // Evaluate the job-hold element

                evalElem = "job-hold";
                evalAttr = "if";

                if (fUnevalHoldTimeout != null)
                {
                    // Indicates a job-hold element was specified
                    
                    fHoldFlag = thread.pyBoolEval(fHoldIf);
                }

                if (fHoldFlag)
                {
                    evalAttr = "timeout";
                    
                    String holdTimeoutString = thread.pyStringEval(
                        fUnevalHoldTimeout);

                    STAFResult result = STAFUtil.convertDurationString(
                        holdTimeoutString);

                    if (result.rc == STAFResult.Ok)
                    {
                        try
                        {
                            fHoldTimeout = Long.parseLong(result.result);
                        }
                        catch (NumberFormatException e)
                        {
                            // Should not happen since convertDurationString
                            // should have already found this

                            result.rc = STAFResult.InvalidValue;
                            result.result = "";
                        }
                    }

                    if (result.rc != STAFResult.Ok)
                    {
                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr,
                            "Invalid timeout value: " + holdTimeoutString +
                            "\n\n" + result.result));

                        thread.setSignalMsgVar(
                            "STAXInvalidTimerValueMsg",
                            STAXUtil.formatErrorMessage(this));

                        thread.raiseSignal("STAXInvalidTimerValue");
                        return;
                    }
                        
                    request.append(" HOLD");
                    
                    if (fHoldTimeout != 0)
                        request.append(" " + fHoldTimeout);
                }

                if (fThread.getBreakpointCondition() ||
                    fThread.getJob().getBreakpointSubjobFirstFunction())
                {
                    request.append(" breakpointfirstfunction ");
                    request.append(" breakpointsubjobfirstfunction ");
                }

                // Evaluate the job-action element

                if (fJobAction != null)
                {
                    evalElem = "job-action";
                    evalAttr = "if";
                    fJobActionFlag = thread.pyBoolEval(fJobActionIf);
                }
            }
            catch (STAXPythonEvaluationException e)
            {
                fState = COMPLETE;
                fThread.popAction();
                
                fThread.pySetVar("RC", new Integer(-1));
                fThread.pySetVar("STAFResult", Py.None);
                fThread.pySetVar("STAXResult", Py.None);
                fThread.pySetVar("STAXSubJobID", new Integer(0));
                fThread.pySetVar("STAXSubJobStatus",
                                 STAXJob.UNKNOWN_STATUS_STRING);

                setElementInfo(new STAXElementInfo(
                    evalElem, evalAttr, evalIndex));

                fThread.setSignalMsgVar(
                    "STAXPythonEvalMsg",
                    STAXUtil.formatErrorMessage(this), e);

                fThread.raiseSignal("STAXPythonEvaluationError");

                return;
            }

            fRequest = request.toString();
            
            // Create request info structure for STAX EXECUTE request

            String staxMachineName =
                fThread.getJob().getSTAX().getLocalMachineName();

            String staxServiceName =
                fThread.getJob().getSTAX().getServiceName();

            fState = RUNNING;

            // Submit STAX EXECUTE Command

            STAXSubmitResult res = fThread.getJob().getSTAX().execute(
                new STAFServiceInterfaceLevel30.RequestInfo(
                    "",  // XXX: Default UUID to what?
                    staxMachineName, staxMachineName,
                    "STAF/Service/" + staxServiceName, 1, 5, true,
                    0,  // XXX: diagEnabled flag is not available
                    fRequest,
                    0,    // XXX: requestNumber field is not available
                    "none://anonymous",// Default user to "none://anonymous"
                    "local://local",  // Default endpoint to "local://local"
                    "local"           // Default physicalInterfaceID to "local"
                    ), this);

            fSubmitRC = res.getResult().rc;
            fSubmitResult = res.getResult().result;
            fSubJob = res.getJob();

            if (fSubmitRC == 0)
            {
                fIsJobRunning = true;
                fJobID = (new Integer(fSubmitResult)).intValue();
                fThread.pySetVar("STAXSubJobID", fSubmitResult);

                // Set actual starting function and function args

                setStartFunction(fSubJob.getStartFunction());
                setStartFuncArgs(fSubJob.getStartFuncArgs());

                if ((fJobAction != null) && fJobActionFlag)
                {
                    // Start action specified by <job-action> element

                    synchronized (fThreadMap)
                    {
                        fIsJobActionRunning = true;
                        STAXThread childThread = fThread.createChildThread();

                        fThreadMap.put(childThread.getThreadNumber(),
                                       childThread);

                        STAXAction childThreadAction = fJobAction;

                        childThread.addCompletionNotifiee(this);
                        childThread.pushAction(
                            childThreadAction.cloneAction());
                        childThread.schedule();
                    }
                }
            }
            else
            {
                // STAX EXECUTE Request failed

                fState = COMPLETE;
                fThread.popAction();

                // If the job is in a pending state, need to do clean-up activities
                // like removing the job from the job map, and logging a stop record
                // in the job log and service log, and generating a job stop event

                if ((fSubJob != null) &&
                    (fSubJob.getState() == STAXJob.PENDING_STATE))
                {
                    fSubJob.cleanupPendingJob(res.getResult());
                }

                fThread.pySetVar("RC", new Integer(fSubmitRC));
                fThread.pySetVar("STAFResult", fSubmitResult);

                if (STAFMarshallingContext.isMarshalledData(fSubmitResult))
                {
                    // STAFResult is a marshalled map containing the Job ID 
                    // and and Error Message.  If so, assign these values to
                    // STAXSubJobID and STAFResult.  This way, STAFResult
                    // will always be a string containing the error messsage
                    // instead of marshalled data.
                    
                    try
                    { 
                        fThread.pyExec(
                            "STAXTempMap = STAFMarshalling.unmarshall(STAFResult).getRootObject()\n" +
                            "STAFResult = STAXTempMap['errorMsg']\n" +
                            "STAXSubJobID = int(STAXTempMap['jobID'])\n" +
                            "del STAXTempMap");
                    }
                    catch (STAXPythonEvaluationException e)
                    {
                        // Should never happen

                        System.out.println(e.toString());
                        fThread.pySetVar("STAFResult", fSubmitResult);
                        fThread.pySetVar("STAXSubJobID", new Integer(0));
                    }
                }
                else
                {
                    fThread.pySetVar("STAXSubJobID", new Integer(0));
                }
                
                fThread.pySetVar("STAXResult", Py.None);

                if (fSubJob != null)
                {
                    fThread.pySetVar("STAXSubJobStatus",
                                     fSubJob.getCompletionStatusAsString());
                }
                else
                {
                    fThread.pySetVar("STAXSubJobStatus",
                                     STAXJob.TERMINATED_STATUS_STRING);
                }

                return;
            }

            // Set to the current date and time.
            fStartTimestamp = new STAXTimestamp();

            // Add the running STAX EXECUTE command to the subJobMap

            @SuppressWarnings("unchecked")
            TreeMap<String, STAXJobAction> subJobs =
                (TreeMap<String, STAXJobAction>)fThread.getJob().getData(
                    "subJobMap");

            if (subJobs != null)
            {
                synchronized (subJobs)
                {
                    subJobs.put(String.valueOf(fJobID), this);
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

            // Generate a start event for a sub-job

            HashMap<String, String> subJobMap = new HashMap<String, String>();
            subJobMap.put("type", "subjob");
            subJobMap.put("block", fCurrentBlockName);
            subJobMap.put("status", "start");
            subJobMap.put("jobID", String.valueOf(fJobID));
            subJobMap.put("jobName", fName);

            if (!fJobFile.equals(""))
            {
                subJobMap.put("jobfile", fJobFile);
            }
            else
            {
                subJobMap.put("jobfile", STAX.INLINE_DATA);
            }

            subJobMap.put("jobfilemachine", fJobFileMachine);
            subJobMap.put("function", fStartFunction);
            subJobMap.put("functionargs", fStartFunctionArgs);
            subJobMap.put("clearlogs", fClearlogs);
            subJobMap.put("monitor", fMonitor);
            subJobMap.put("logtcelapsedtime", fLogTCElapsedTime);
            subJobMap.put("logtcnumstarts", fLogTCNumStarts);
            subJobMap.put("logtcstartstop", fLogTCStartStop);
            subJobMap.put("scriptfilesmachine", fScriptFilesMachine);
            subJobMap.put("pythonoutput", fPythonOutput);
            subJobMap.put("pythonloglevel", fPythonLogLevel);

            // Convert the script file vector to a list

            List<String> scriptFileList = new ArrayList<String>();

            for (int i = 0; i < fScriptFiles.size(); i++)
            {
                scriptFileList.add(fScriptFiles.elementAt(i));
            }

            subJobMap.put("scriptFileList", STAFMarshallingContext.marshall(
                scriptFileList, new STAFMarshallingContext()));

            // Convert the script vector to a list

            List<String> scriptList = new ArrayList<String>();

            for (int i = 0; i < fScripts.size(); i++)
            {
                scriptList.add(fScripts.elementAt(i));
            }

            subJobMap.put("scriptList", STAFMarshallingContext.marshall(
                scriptList, new STAFMarshallingContext()));

            if (fHoldFlag)
            {
                // A job-hold element was specified and its "if" attribute
                // evaluated via Python to a true value
                subJobMap.put("hold", "" + fHoldTimeout);
            }

            subJobMap.put("startdate",
                          fSubJob.getStartTimestamp().getDateString());
            subJobMap.put("starttime",
                          fSubJob.getStartTimestamp().getTimeString());

            fThread.getJob().generateEvent(
                STAXJobActionFactory.STAX_SUBJOB_EVENT,
                subJobMap);

            fThread.addCondition(fHoldCondition);
        }
        else if ((fState == RUNNING) &&
                 !fIsJobRunning && !fIsJobActionRunning)
        {
            if (sDebug)
                System.out.println(
                    "STAXJobAction::execute(): Set fState=COMPLETE");

            synchronized(this)
            {
                fState = COMPLETE;
                fThread.popAction();

                // Assign variables when job and job-action have completed

                fThread.pySetVar("RC", new Integer(fSubmitRC));
                fThread.pySetVar("STAFResult", fSubmitResult);
                fThread.pySetVar("STAXResult", fSubJob.getResult());
                fThread.pySetVar("STAXSubJobStatus",
                                 fSubJob.getCompletionStatusAsString());

                // Remove from map of currently executing sub-jobs

                TreeMap subJobs = (TreeMap)fThread.getJob().getData(
                    "subJobMap");
                 
                if (subJobs != null)
                {
                    synchronized (subJobs)
                    {
                        subJobs.remove(String.valueOf(fJobID));
                    }
                }
            }
        }
        else if (fState == COMPLETE)
        {
            // Note: We shouldn't be called in this state.

            if (sDebug)
                System.out.println(
                    "ERROR: STAXJobAction::execute(): fState=COMPLETE");

            fThread.popAction();
        }
        else if (sDebug)
        {
            // Should not get here
            System.out.println(
                "ERROR: STAXJobAction::execute(): fState=" +
                getStateAsString());
        }
    }

    /**
     *  Note that this entire method is synchronized since the state of the
     *  action can be changed on another thread (via the jobComplete method).
     * 
     * @parm thread    the STAXThread the sub-job is running on
     * @parm cond      the STAXCondition to be handled
     */ 

    public synchronized void handleCondition(STAXThread thread,
        STAXCondition cond)
    {
        if (sDebug)
            System.out.println(
                "STAXJobAction::handleCondition(): fState=" +
                getStateAsString());

        // Terminate the job if running

        if ((fState == RUNNING) && fIsJobRunning)
        {
            fIsJobRunning = false;

            thread.getJob().submitAsyncForget(
                "local", thread.getJob().getSTAX().getServiceName(),
                "TERMINATE JOB " + fJobID);

            // Generate an event to indicate that the sub-job is stopped

            HashMap<String, String> subJobMap = new HashMap<String, String>();
            subJobMap.put("type", "subjob");
            subJobMap.put("block", fCurrentBlockName);
            subJobMap.put("status", "stop");
            subJobMap.put("jobID", String.valueOf(fJobID));
            subJobMap.put("result", thread.getJob().getResult().toString());

            thread.getJob().generateEvent(
                STAXJobActionFactory.STAX_SUBJOB_EVENT,
                subJobMap);

            // Remove from map of currently executing sub-jobs

            TreeMap subJobs = (TreeMap)thread.getJob().getData("subJobMap");

            if (subJobs != null)
            {
                synchronized (subJobs)
                {
                    subJobs.remove(String.valueOf(fJobID));
                }
            }
        }

        // Terminate the job-action task if running
        
        synchronized (fThreadMap)
        {
            if (!fThreadMap.isEmpty())
            {
                if (sDebug)
                    System.out.println(
                        "STAXJobAction::handleCondition(): Add " +
                        "HardHoldThread condition and terminate job-action " +
                        "thread");

                // Add a hard hold so we can wait until our children terminate
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

        thread.pySetVar("STAXSubJobStatus",
                        thread.getJob().getCompletionStatusAsString());

        // Remove the HoldThread condition from the thread's condition
        // stack (if it exists)

        if (sDebug)
            System.out.println(
                "STAXJobAction::handleCondition(): Set fState=COMPLETE " +
                "and remove HoldThread condition: " +
                thread.getConditionStack());

        thread.removeCondition(fHoldCondition);

        thread.popAction();
    }

    /**
     * cloneAction is a STAXAction interface method
     * <p>
     * Called when cloning the action (e.g. when creating a new STAX-Thread)
     * 
     * @return    the cloned STAXAction object
     */

    public STAXAction cloneAction()
    {
        STAXJobAction clone = new STAXJobAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalName = fUnevalName;
        clone.fUnevalClearlogs = fUnevalClearlogs;
        clone.fUnevalMonitor = fUnevalMonitor;
        clone.fUnevalLogTCElapsedTime = fUnevalLogTCElapsedTime;
        clone.fUnevalLogTCNumStarts = fUnevalLogTCNumStarts;
        clone.fUnevalLogTCStartStop = fUnevalLogTCStartStop;
        clone.fUnevalPythonOutput = fUnevalPythonOutput;
        clone.fUnevalPythonLogLevel = fUnevalPythonLogLevel;
        clone.fUnevalJobFile = fUnevalJobFile;
        clone.fUnevalJobFileMachine = fUnevalJobFileMachine;
        clone.fUnevalJobData = fUnevalJobData;
        clone.fUnevalFunction = fUnevalFunction;
        clone.fUnevalFunctionArgs = fUnevalFunctionArgs;
        clone.fUnevalScripts = fUnevalScripts;
        clone.fUnevalScriptFiles = fUnevalScriptFiles;
        clone.fUnevalScriptFilesMachine = fUnevalScriptFilesMachine;
        clone.fUnevalHoldTimeout = fUnevalHoldTimeout;

        clone.fCurrentBlockName = fCurrentBlockName;
        clone.fFactory = fFactory;
        clone.fName = fName;
        clone.fClearlogs = fClearlogs;
        clone.fMonitor = fMonitor;
        clone.fJobFile = fJobFile;
        clone.fJobFileMachine = fJobFileMachine;
        clone.fJobData = fJobData;
        clone.fJobDataEval = fJobDataEval;
        clone.fFunction = fFunction;
        clone.fFunctionIf = fFunctionIf;
        clone.fFunctionArgs = fFunctionArgs;
        clone.fFunctionArgsIf = fFunctionArgsIf;
        clone.fFunctionArgsEval = fFunctionArgsEval;
        clone.fScripts = fScripts;
        clone.fScriptsIf = fScriptsIf;
        clone.fScriptsEval = fScriptsEval;
        clone.fScriptFiles = fScriptFiles;
        clone.fScriptFilesIf = fScriptFilesIf;
        clone.fScriptFilesMachine = fScriptFilesMachine;
        clone.fScriptFileElementName = fScriptFileElementName;
        clone.fHoldTimeout = fHoldTimeout;
        clone.fHoldIf = fHoldIf;
        clone.fHoldFlag = fHoldFlag;
        clone.fJobAction = fJobAction;
        clone.fJobActionIf = fJobActionIf;
        
        return clone;
    }

    /**
     * jobComplete is a STAXJobCompleteListener interface method
     * <p>
     * Called when the submitted sub-job completes so it can set variables
     * RC and STAXResult with the result from the STAX EXECUTE request.
     * Generates an event to indicate that the submitted sub-job has completed.
     * If the job-action (if any) has completed, removes the hold thread
     * condition and schedules the thread to run.
     * <p>
     * Note that this entire method is synchronized since its state can be
     * changed on another thread (e.g. via the execute, handleCondition, and
     * threadComplete methods).
     * 
     * @parm job    the STAXJob object that submitted the sub-job 
     */ 

    public synchronized void jobComplete(STAXJob job)
    {
        if (sDebug)
            System.out.println(
                "STAXJobAction::jobComplete(): fState=" + getStateAsString());

        if (fState == COMPLETE) return;

        fIsJobRunning = false;

        // Save completed job element so can get the job result and status
        fSubJob = job;

        // Generate a stop event for the sub-job

        HashMap<String, String> subJobMap = new HashMap<String, String>();
        subJobMap.put("type", "subjob");
        subJobMap.put("block", fCurrentBlockName);
        subJobMap.put("status", "stop");
        subJobMap.put("jobID", String.valueOf(fJobID));
        subJobMap.put("result", job.getResult().toString());

        fThread.getJob().generateEvent(
            STAXJobActionFactory.STAX_SUBJOB_EVENT,
            subJobMap);

        // If the job-action (if any) has completed, remove the HoldThread
        // condition and schedule the thread to run

        if (fThreadMap.isEmpty())
        {
            if (sDebug)
                System.out.println(
                    "STAXJobAction::jobComplete(): Remove HoldThread " +
                    "condition: " + fThread.getConditionStack());
            
            fThread.removeCondition(fHoldCondition);
            fThread.schedule();
        }
    }

    /**
     *  Called when the job-action thread for the sub-job completes so it can
     *  remove the thread from the thread map, set fIsJobActionRunning to
     *  false, and remove the HardHoldThread condition from the parent
     *  thread's condition stack (if it exists), and if the job no longer is
     *  running, it removes the HoldThread condition from the parent thread's
     *  condition stack.
     *  <p>
     *  Note that this entire method is synchronized since its state can be
     *  changed on another thread (e.g. via the execute, handleCondition, and
     *  jobComplete methods).
     * 
     * @parm thread    the STAXThread the sub-job is running on
     * @parm endCode   the code indicating how the thread ended
     */

    public synchronized void threadComplete(STAXThread thread, int endCode)
    {
        synchronized(fThreadMap)
        {
            if (sDebug)
                System.out.println(
                    "STAXJobAction::threadComplete(): fState=" +
                    getStateAsString());

            fThreadMap.remove(thread.getThreadNumberAsInteger());

            if (fThreadMap.isEmpty())
            {
                fIsJobActionRunning = false;

                // Note that thread.getParentThread() is the thread we are
                // running on

                if (sDebug)
                    System.out.println(
                        "STAXJobAction::threadComplete(): Remove " +
                        "HardHoldThread condition: " +
                        thread.getParentThread().getConditionStack());

                thread.getParentThread().removeCondition(fHardHoldCondition);
                
                if ((fState == RUNNING) && !fIsJobRunning)
                {
                    if (sDebug)
                        System.out.println(
                            "STAXJobAction::threadComplete(): Remove " +
                            "HoldThread condition: " +
                            thread.getParentThread().getConditionStack());

                    thread.getParentThread().removeCondition(fHoldCondition);
                }

                thread.getParentThread().schedule();
            }
        }
    }


    STAXThread fThread = null;
    int fState = INIT;
    boolean fIsJobRunning = false;
    boolean fIsJobActionRunning = false;
    private STAXHoldThreadCondition fHoldCondition = 
        new STAXHoldThreadCondition("Job");
    private STAXHardHoldThreadCondition fHardHoldCondition =
        new STAXHardHoldThreadCondition("Job");

    private String fName = new String();
    private String fClearlogs = new String();
    private String fMonitor = new String();
    private String fLogTCElapsedTime = new String();
    private String fLogTCNumStarts = new String();
    private String fLogTCStartStop = new String();
    private String fPythonOutput = new String();
    private String fPythonLogLevel = new String();
    private String fJobFile = new String();
    private String fJobFileMachine = new String();
    private String fJobData = new String();
    private String fJobDataEval = new String();
    private String fFunction = new String();
    private String fFunctionIf = new String();
    private String fFunctionArgs = new String();
    private String fFunctionArgsIf = new String();
    private String fFunctionArgsEval = new String();
    private Vector<String> fScripts = new Vector<String>();
    private Vector<String> fScriptsIf = new Vector<String>();
    private Vector<String> fScriptsEval = new Vector<String>();
    private Vector<String> fScriptFiles = new Vector<String>();
    private String fScriptFilesIf = new String();
    private String fScriptFilesMachine = new String();
    private String fScriptFileElementName = new String();
    private long fHoldTimeout = 0; // Default value of 0 holds job indefinitely
    private String fHoldIf = new String();
    private boolean fHoldFlag = false;
    private STAXAction fJobAction = null;
    private String fJobActionIf = new String();
    private boolean fJobActionFlag = true;

    private String fUnevalName = new String();
    private String fUnevalClearlogs = new String();
    private String fUnevalMonitor = new String();
    private String fUnevalLogTCElapsedTime = new String();
    private String fUnevalLogTCNumStarts = new String();
    private String fUnevalLogTCStartStop = new String();
    private String fUnevalPythonOutput = new String();
    private String fUnevalPythonLogLevel = new String();
    private String fUnevalJobFile = new String();
    private String fUnevalJobFileMachine = new String();
    private String fUnevalJobData = new String();
    private String fUnevalFunction = new String();
    private String fUnevalFunctionArgs = new String();
    private Vector<String> fUnevalScripts = new Vector<String>();
    private String fUnevalScriptFiles = new String();
    private String fUnevalScriptFilesMachine = new String();
    private String fUnevalHoldTimeout = null;
    private String fUnevalJobActionIf = new String();

    private String fStartFunction = new String();
    private String fStartFunctionArgs = new String();
    private String fRequest = new String();
    private int fRequestNumber = 0;
    private int fSubmitRC = 0;
    private String fSubmitResult = new String();
    private STAXJob fSubJob = null;
    private int fJobID = 0;
    private STAXTimestamp fStartTimestamp;
    private STAXJobActionFactory fFactory;
    private String fCurrentBlockName = new String();

    private HashMap<Integer, STAXThread> fThreadMap =
        new HashMap<Integer, STAXThread>();
}
