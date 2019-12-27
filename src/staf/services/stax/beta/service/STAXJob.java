/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import com.ibm.staf.wrapper.STAFLog;
import java.util.Map;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;

import java.util.Vector;
import java.util.StringTokenizer;
import java.util.Date;
import java.util.Set;

import org.python.core.Py;

import org.python.core.PyObject;
import org.python.core.PyCode;

/* Jython 2.1:
import org.python.core.__builtin__;
*/
// Jython 2.5:
import org.python.core.CompileMode;

/**
 * The representation of a STAX job.  A STAX job is created by the STAXParser
 * when the STAX service receives an EXECUTE request specifying a file or string
 * containing XML that defines a STAX job.
 */
public class STAXJob implements STAXThreadCompleteListener,
                                STAXSTAFQueueListener
{
    // For debugging - Counts and prints the number of cache gets/adds
    static final boolean COUNT_PYCODE_CACHES = false;

    static final String STAX_JOB_EVENT = new String("Job");

    // Logfile types:

    /**
     * Indicates to log to the STAX Service Log file
     */
    static final int SERVICE_LOG = 1;

    /**
     * Indicates to log to the STAX Job Log file
     */
    public static final int JOB_LOG = 2;

    /**
     * Indicates to log to the STAX Job User Log file
     */
    public static final int USER_JOB_LOG = 3;

    /**
     * Indicates to log to the JVM Log file
     */
    public static final int JVM_LOG = 4;

    public static final int NO_NOTIFY_ONEND = 0;
    public static final int NOTIFY_ONEND_BY_HANDLE = 1;
    public static final int NOTIFY_ONEND_BY_NAME = 2;

    /**
     * Job completion status codes and their string values
     */
    public static final int ABNORMAL_STATUS = -1;
    public static final int NORMAL_STATUS = 0;
    public static final int TERMINATED_STATUS = 1;
    public static final int UNKNOWN_STATUS = 2;

    public static final String ABNORMAL_STATUS_STRING = "Abnormal";
    public static final String NORMAL_STATUS_STRING = "Normal";
    public static final String TERMINATED_STATUS_STRING = "Terminated";
    public static final String UNKNOWN_STATUS_STRING = "Unknown";

    /**
     * Job state codes and their string values
     */
    public static final int PENDING_STATE = 0;
    public static final int RUNNING_STATE = 1;
    public static final String PENDING_STATE_STRING = "Pending";
    public static final String RUNNING_STATE_STRING = "Running";

    static final int BREAKPOINT_FUNCTION = 0;
    static final int BREAKPOINT_LINE = 1;

    /** 
      * Creates a new STAXJob instance passing in a STAX object which
      * represents the STAX service that is executing this job.
      */
    public STAXJob(STAX staxService) {
        this(staxService, new STAXDocument());
    }
    
    /**
     * Creates a new STAXJob instance using an existing STAX 
     * document.
     * 
     * @param staxService the STAX service.
     * @param document the STAX document to be executed by 
     * this job.
     */
    public STAXJob(STAX staxService, STAXDocument document) {
        fSTAX = staxService;
        fDocument = document;
        STAXThread thread = new STAXThread(this);
        thread.addCompletionNotifiee(this);
        fThreadMap.put(thread.getThreadNumberAsInteger(), thread);
        fClearlogs = fSTAX.getClearlogs();
        fLogTCElapsedTime = fSTAX.getLogTCElapsedTime();
        fLogTCNumStarts   = fSTAX.getLogTCNumStarts();
        fLogTCStartStop   = fSTAX.getLogTCStartStop();
        fPythonOutput     = fSTAX.getPythonOutput();
        fPythonLogLevel   = fSTAX.getPythonLogLevel();
        fMaxSTAXThreads   = fSTAX.getMaxSTAXThreads();
    }

    /**
     * Gets the STAX object which represents the job's STAX service
     * @return an instance of the job's STAX service
     */
    public STAX getSTAX() { return fSTAX; }

    /**
     * Gets the STAX document that is executed by this job.
     * 
     * @return the job's STAX document.
     */
    public STAXDocument getSTAXDocument() { return fDocument; }
    
    /**
     * Sets the STAX document to be executed by this job
     * @param document the STAX document to be executed by this job
     */
    public void setSTAXDocument(STAXDocument document)
    {
        fDocument = document;
    }    

    /**
     * Gets the next number for a thread in a job
     * @return a number for the next thread in a job
     */
    public int getNextThreadNumber()
    {
        synchronized (fNextThreadNumberSynch)
        {
            return fNextThreadNumber++;
        }
    }

    /**
     * Gets the next number for a breakpoint in a job
     * @return a number for the next breakpoint in a job
     */
    public int getNextBreakpointNumber()
    {
        synchronized (fNextBreakpointNumberSynch)
        {
            return fNextBreakpointNumber++;
        }
    }

    public void setDefaultCallAction(STAXCallAction action)
    {
        fDefaultCallAction = action;
    }

    /**
     * Gets the name of the function that should be called to start the
     * execution of a job
     * @return the name of the starting function for a job
     */
    public String getStartFunction() { return fDocument.getStartFunction(); }

    /**
     * Sets the name of the function that should be called to start the
     * execution of a job
     * @param  startFunction   the name of the starting function for a job
     */
    public void setStartFunction(String startFunction)
    { fDocument.setStartFunction(startFunction); }

    public String getStartFuncArgs() { return fDocument.getStartFunctionArgs(); }

    public void setStartFuncArgs(String startFuncArgs)
    { fDocument.setStartFuncArgs(startFuncArgs); }

    public void setExecuteAndHold()
    { fExecuteAndHold = true; }

    // Sets the value of the FUNCTION option on a STAX EXECUTE request
    public void setStartFunctionOverride(String startFunction)
    { fStartFunction = startFunction; }

    public String getStartFunctionOverride() { return fStartFunction; }
    
    // Sets the value of the ARGS option on a STAX EXECUTE request
    public void setStartFuncArgsOverride(String startFuncArgs)
    { fStartFuncArgs = startFuncArgs; }

    public String getStartFuncArgsOverride() { return fStartFuncArgs; }

    public String getJobName() { return fJobName; }

    public void setJobName(String jobName) { fJobName = jobName; }

    public int getJobNumber() { return fJobNumber; }

    public void setJobNumber(int jobNumber) { fJobNumber = jobNumber; }
    
    public int getNextProcNumber()
    {
        synchronized (fNextProcNumberSynch)
        { 
            return fProcNumber++;
        }
    }
    
    public int getNextCmdNumber()
    {
        synchronized (fNextCmdNumberSynch)
        {
            return fCmdNumber++;
        }
    }

    public int getNextProcessKey()
    {
        synchronized (fNextProcessKeySynch)
        {
            return fProcessKey++;
        }
    }
    public String getJobDataDir() { return fJobDataDir; }

    public void setJobDataDir(String jobDataDir) { fJobDataDir = jobDataDir; }
    
    public String getXmlMachine() { return fXmlMachine; }

    public void setXmlMachine(String machName) 
    { fXmlMachine = machName; }

    public String getXmlFile() { return fXmlFile; }

    public void setXmlFile(String fileName) { fXmlFile = fileName; }

    public List getScripts() { return fScripts; }

    public void setScript(String script) { fScripts.add(script); }

    public List getScriptFiles() { return fScriptFiles; }

    public void setScriptFile(String fileName) { fScriptFiles.add(fileName); }
    
    public String getScriptFileMachine() { return fScriptFileMachine; }

    public void setScriptFileMachine(String machName) 
    { fScriptFileMachine = machName; }

    public String getSourceMachine() { return fSourceMachine; }

    public void setSourceMachine(String machName) 
    { fSourceMachine = machName; }

    public String getSourceHandleName() { return fSourceHandleName; }

    public void setSourceHandleName(String handleName) 
    { fSourceHandleName = handleName; }

    public int getSourceHandle() { return fSourceHandle; }

    public void setSourceHandle(int handle) 
    { fSourceHandle = handle; }

    public boolean getClearlogs() { return fClearlogs; }

    public String getClearLogsAsString()
    {
        if (fClearlogs)
            return "Enabled";
        else
            return "Disabled";
    }

    public void setClearlogs(boolean clearlogs) 
    { fClearlogs = clearlogs; }

    public int getMaxSTAXThreads() { return fMaxSTAXThreads; }

    public String getWaitTimeout() { return fWaitTimeout; }
    
    public void setWaitTimeout(String timeout)
    { fWaitTimeout = timeout; }

    public int getNotifyOnEnd() { return fNotifyOnEnd; }

    public String getNotifyOnEndAsString()
    {
        if (fNotifyOnEnd == STAXJob.NOTIFY_ONEND_BY_NAME)
            return "By Name";
        else if (fNotifyOnEnd == STAXJob.NOTIFY_ONEND_BY_HANDLE)
            return "By Handle";
        else 
            return "No";
    }

    public void setNotifyOnEnd(int notifyFlag) { fNotifyOnEnd = notifyFlag; }
    
    public int getState() { return fState; }

    public String getStateAsString()
    {
        if (fState == PENDING_STATE)
            return PENDING_STATE_STRING;
        else
            return RUNNING_STATE_STRING;
    }

    public void setState(int state) { fState = state; }
    
    public PyObject getResult() { return fResult; }

    public void setResult(PyObject result) 
    { fResult = result; }

    public int getCompletionStatus() { return fCompletionStatus; }

    public String getCompletionStatusAsString()
    {
        if (fCompletionStatus == STAXJob.NORMAL_STATUS)
            return STAXJob.NORMAL_STATUS_STRING;
        else if (fCompletionStatus == STAXJob.TERMINATED_STATUS)
            return STAXJob.TERMINATED_STATUS_STRING;
        else if (fCompletionStatus == STAXJob.ABNORMAL_STATUS)
            return STAXJob.ABNORMAL_STATUS_STRING;
        else
            return STAXJob.UNKNOWN_STATUS_STRING;
    }

    public void setCompletionStatus(int status)
    {
        fCompletionStatus = status;
    }
    
    public STAFMarshallingContext getJobLogErrorsMC()
    {
        return fJobLogErrorsMC;
    }

    public List getTestcaseList() { return fTestcaseList; }

    public void setTestcaseList(List testcaseList)
    { fTestcaseList = testcaseList; }

    public Map getTestcaseTotalsMap() { return fTestcaseTotalsMap; }

    public void setTestcaseTotalsMap(Map testcaseTotalsMap)
    { fTestcaseTotalsMap = testcaseTotalsMap; }

    public STAXTimestamp getStartTimestamp() { return fStartTimestamp; }
    
    public void setStartTimestamp()
    {
        // Get the current date and time and set as the starting date/time
        fStartTimestamp = new STAXTimestamp();
    }
    
    public STAXTimestamp getEndTimestamp() { return fEndTimestamp; }
    
    public Integer getJobNumberAsInteger() { return new Integer(fJobNumber); }

    public STAFHandle getSTAFHandle() { return fHandle; }

    public void setSTAFHandle() throws STAFException
    {
        fHandle = new STAFHandle("STAX/Job/" + fJobNumber);
    }

    public boolean getLogTCElapsedTime() { return fLogTCElapsedTime; }

    public String getLogTCElapsedTimeAsString()
    {
        if (fLogTCElapsedTime)
            return "Enabled";
        else
            return "Disabled";
    }

    public void setLogTCElapsedTime(boolean logTCElapsedTime)
    { fLogTCElapsedTime = logTCElapsedTime; }

    public boolean getLogTCNumStarts() { return fLogTCNumStarts; }

    public String getLogTCNumStartsAsString()
    {
        if (fLogTCNumStarts)
            return "Enabled";
        else
            return "Disabled";
    }

    public void setLogTCNumStarts(boolean logTCNumStarts)
    { fLogTCNumStarts = logTCNumStarts; }

    public boolean getLogTCStartStop() { return fLogTCStartStop; }
    
    public String getLogTCStartStopAsString()
    {
        if (fLogTCStartStop)
            return "Enabled";
        else
            return "Disabled";
    }
    
    public void setLogTCStartStop(boolean logTCStartStop)
    { fLogTCStartStop = logTCStartStop; }

    public int getPythonOutput() { return fPythonOutput; }

    public void setPythonOutput(int pythonOutput)
    {
        fPythonOutput = pythonOutput;   
    }

    public String getPythonLogLevel() { return fPythonLogLevel; }
    
    public void setPythonLogLevel(String logLevel)
    {
        fPythonLogLevel = logLevel;
    }

    public int getNumThreads()
    {
        return fThreadMap.size();
    }

    public Map getThreadMapCopy()
    {
        return fThreadMap;
    }

    public STAXThread getThread(Integer threadNumber)
    {
        synchronized (fThreadMap)
        {
            return (STAXThread)fThreadMap.get(threadNumber);
        }
    }

    public void addThread(STAXThread thread)
    {
        synchronized (fThreadMap)
        {
            fThreadMap.put(thread.getThreadNumberAsInteger(), thread);
        }
    }

    public void addThreadIfDoesNotExceedMax(STAXThread thread)
        throws STAXExceedsMaxThreadsException
    {
        synchronized (fThreadMap)
        {
            if ((fMaxSTAXThreads != 0) &&
                (fThreadMap.size() >= fMaxSTAXThreads))
            {
                throw new STAXExceedsMaxThreadsException(
                    "Exceeded MaxSTAXThreads=" + fMaxSTAXThreads +
                    " (the maximum number of STAX Threads that " +
                    "can be running simultaneously in a job).");
            }

            fThreadMap.put(thread.getThreadNumberAsInteger(), thread);
        }
    }

    public void removeThread(Integer threadNumber)
    {
        synchronized (fThreadMap)
        {
            fThreadMap.remove(threadNumber);
        }
    }

    public void addFunction(STAXFunctionAction function)
    {
        fDocument.addFunction(function);
    }

    public STAXAction getFunction(String name)
    {
        return fDocument.getFunction(name);
    }

    public boolean getBreakpointFirstFunction()
    {
        return fBreakpointFirstFunction;
    }

    public void setBreakpointFirstFunction(boolean breakpointFirstFunction)
    {
        fBreakpointFirstFunction = breakpointFirstFunction;
    }

    public boolean getBreakpointSubjobFirstFunction()
    {
        return fBreakpointSubjobFirstFunction;
    }

    public void setBreakpointSubjobFirstFunction(
                    boolean breakpointSubjobFirstFunction)
    {
        fBreakpointSubjobFirstFunction = breakpointSubjobFirstFunction;
    }

    public boolean functionExists(String name)
    {
        return fDocument.functionExists(name);
    }

    public void addDefaultAction(STAXAction action)
    {
        fDocument.addDefaultAction(action);
    }

    public void addCompletionNotifiee(STAXJobCompleteListener listener)
    {
        synchronized (fCompletionNotifiees)
        {
            fCompletionNotifiees.addLast(listener);
        }
    }

    public STAFResult addCompletionNotifiee2(STAXJobCompleteNotifiee notifiee)
    {
        synchronized (fCompletionNotifiees)
        {
            // Check if the notifiee is already in the list

            Iterator iter = fCompletionNotifiees.iterator();

            while (iter.hasNext())
            {
                Object notifieeObj = iter.next();

                if (notifieeObj instanceof
                    com.ibm.staf.service.stax.STAXJobCompleteNotifiee)
                {
                    STAXJobCompleteNotifiee aNotifiee =
                        (STAXJobCompleteNotifiee)notifieeObj;

                    if (aNotifiee.getMachine().equals(notifiee.getMachine()) &&
                        aNotifiee.getHandle() == notifiee.getHandle() &&
                        aNotifiee.getHandleName().equals(notifiee.getHandleName()))
                    {
                        return new STAFResult(
                            STAFResult.AlreadyExists,
                            "A notifiee is already registered for machine=" +
                            notifiee.getMachine() +
                            ", handle=" + notifiee.getHandle() +
                            ", handleName=" + notifiee.getHandleName());
                    }
                }

            }

            // Add to the end of the list

            fCompletionNotifiees.addLast(notifiee);
        }

        return new STAFResult(STAFResult.Ok, "");
    }

    public STAFResult removeCompletionNotifiee(STAXJobCompleteNotifiee notifiee)
    {
        boolean found = false;

        synchronized (fCompletionNotifiees)
        {
            // Check if notifiee exists.  If so, remove the notifiee

            Iterator iter = fCompletionNotifiees.iterator();

            while (iter.hasNext())
            {
                Object notifieeObj = iter.next();

                if (notifieeObj instanceof
                    com.ibm.staf.service.stax.STAXJobCompleteNotifiee)
                {
                    STAXJobCompleteNotifiee aNotifiee =
                        (STAXJobCompleteNotifiee)notifieeObj;
                
                    if (aNotifiee.getMachine().equals(notifiee.getMachine()) &&
                        aNotifiee.getHandle() == notifiee.getHandle() &&
                        aNotifiee.getHandleName().equals(notifiee.getHandleName()))
                    {
                        // Remove notifiee from list

                        fCompletionNotifiees.remove(aNotifiee);
                        found = true;
                        return new STAFResult(STAFResult.Ok, "");
                    }
                }
            }
        }
                
        return new STAFResult(
            STAFResult.DoesNotExist,
            "No notifiee registered for machine=" + notifiee.getMachine() +
            ", handle=" + notifiee.getHandle() +
            ", handleName=" + notifiee.getHandleName());
    }

    public LinkedList getCompletionNotifiees()
    {
        synchronized(fCompletionNotifiees)
        {
            return new LinkedList(fCompletionNotifiees);
        }
    }

    // Gets the compiled Jython code for the specified code string.
    // Note that we cache compiled Jython code so that if it is used more
    // than once, it eliminates the overhead of recompiling the code string
    // each time it is executed.
    //
    // Input Arguments:
    //  - codeString:  must be valid Python code; however, like ordinary
    //                 Python code, the existence of variables will not be
    //                 checked until the code is executed.
    //  - kind:        is either 'exec' if the string is made up of
    //                 statements, 'eval' if it is an expression.

    public PyCode getCompiledPyCode(String codeString, String kind)
    {
        synchronized(fCompiledPyCodeCache)
        {
            PyCode codeObject = (PyCode)fCompiledPyCodeCache.get(codeString);

            if (codeObject == null)
            {
                if (COUNT_PYCODE_CACHES) fCompiledPyCodeCacheAdds++;

                if (kind.equals("eval"))
                {
                    // Set to avoid error of setting code object to nothing
                    if (codeString.equals("")) codeString = "None";
                
                    /* Jython 2.1:
                    codeObject = __builtin__.compile(
                        "STAXPyEvalResult = " + codeString,
                        "<pyEval string>", "exec");
                    */
                    // Jython 2.5:
                    codeObject = Py.compile_flags(
                        codeString, "<pyEval string>",
                        CompileMode.eval, Py.getCompilerFlags());
                }
                else
                {
                    /* Jython 2.1:
                    codeObject = __builtin__.compile(
                        codeString, "<pyExec string>", "exec");
                    */
                    // Jython 2.5:
                    codeObject = Py.compile_flags(
                        codeString, "<pyExec string>",
                        CompileMode.exec, Py.getCompilerFlags());
                }

                fCompiledPyCodeCache.put(codeString, codeObject);
            }
            else
            {
                if (COUNT_PYCODE_CACHES) fCompiledPyCodeCacheGets++;
            }
        
            return codeObject;
        }
    }
    
    // Queue listener methods

    public void registerSTAFQueueListener(String msgType, 
                                          STAXSTAFQueueListener listener)
    {
        synchronized (fQueueListenerMap)
        {
            TreeSet listenerSet = (TreeSet)fQueueListenerMap.get(msgType);

            if (listenerSet == null)
            {
                listenerSet = new TreeSet(new STAXObjectComparator());
                fQueueListenerMap.put(msgType, listenerSet);
            }

            listenerSet.add(listener);
        }
    }

    public void unregisterSTAFQueueListener(String msgType, 
                                            STAXSTAFQueueListener listener)
    {
        synchronized (fQueueListenerMap)
        {
            TreeSet listenerSet = (TreeSet)fQueueListenerMap.get(msgType);

            if (listenerSet != null)
                listenerSet.remove(listener);
        }
    }

    //
    // Data management functions
    //

    public boolean setData(String dataName, Object data)
    {
        // Return true if added successfully, else return false.

        synchronized (fDataMap)
        {
            if (!fDataMap.containsKey(dataName))
            {
                fDataMap.put(dataName, data);
                return true;
            }
        }

        return false;
    }
 
    public Object getData(String dataName)
    {
        synchronized (fDataMap)
        { return fDataMap.get(dataName); }
    }

    public boolean removeData(String dataName)
    {
        // Return true if removed successfully, else return false.

        synchronized (fDataMap)
        {
            if (fDataMap.containsKey(dataName))
            {
                fDataMap.remove(dataName);
                return true;
            }
        }

        return false;
    }

    //
    // Execution methods
    //

    public void startExecution() 
        throws STAFException, STAXException
    {
        // Check if the MAXRETURNFILESIZE setting is not 0 (no maximum size
        // limit) and if so, set variable STAF/MaxReturnFileSize for the STAX
        // job handle

        if (fSTAX.getMaxReturnFileSize() != 0)
        {
            String request = "SET VAR " + STAFUtil.wrapData(
                "STAF/MaxReturnFileSize=" + fSTAX.getMaxReturnFileSize());

            STAFResult result = fHandle.submit2("local", "VAR", request);

            if (result.rc != STAFResult.Ok)
            {
                String msg = "The STAX service could not set the maximum " +
                    "file size variable for Job ID " + fJobNumber +
                    "\nSTAF local VAR " + request + " failed with RC=" +
                    result.rc + ", Result=" + result.result;

                log(STAXJob.JOB_LOG, "error", msg);
            }
        }

        fSTAFQueueMonitor = new STAFQueueMonitor(this);
        fSTAFQueueMonitor.start();

        // Initialize data for the Job

        fSTAX.visitJobManagementHandlers(new STAXVisitorHelper(this)
        {
            public void visit(Object o, Iterator iter)
            {
                STAXJobManagementHandler handler = (STAXJobManagementHandler)o;

                handler.initJob((STAXJob)fData);
            }
        });

        // Register to listen for messages that STAF requests have completed

        registerSTAFQueueListener("STAF/RequestComplete", this);

        // Get the main thread, thread 1

        STAXThread thread = (STAXThread)fThreadMap.get(new Integer(1));
        
        // Use a custom OutputStream to redirect the Python Interpreter's
        // stdout and stderr to somewhere other than the JVM Log and/or
        // to reformat the output (e.g. add a timestamp, job ID) when
        // logging to the JVM Log.  Use the log level specified for the
        // job's python output.

        thread.setPythonInterpreterStdout(new STAXPythonOutput(this));

        // Override the log level to use to always be "Error" for stderr
        thread.setPythonInterpreterStderr(new STAXPythonOutput(this, "Error"));

        // Get the starting function for the job

        STAXAction action = (STAXAction)fDocument.getFunction(
            fDocument.getStartFunction());

        if (action == null)
        {
            throw new STAXInvalidStartFunctionException(
                "'" + fDocument.getStartFunction() +
                "' is not a valid function name.  No function with " +
                "this name is defined.");
        }
        else
        {
            // Call the main function passing any arguments

            String startFunctionUneval = "'" + fDocument.getStartFunction() +
                "'";

            if (fDefaultCallAction != null)
            {
                STAXCallAction callAction = fDefaultCallAction;
                callAction.setFunction(startFunctionUneval);
                callAction.setArgs(fDocument.getStartFunctionArgs());
                action = callAction;
            }
            else
            {
                STAXCallAction callAction = new STAXCallAction(
                    startFunctionUneval, fDocument.getStartFunctionArgs());
                callAction.setLineNumber(
                    "<External>", "<Error in ARGS option>");
                callAction.setXmlFile(getXmlFile());
                callAction.setXmlMachine(getXmlMachine());
                action = callAction;
            }
        }

        ArrayList actionList = new ArrayList();
        
        // If HOLD was specified on EXECUTE request, add hold action as
        // the first action in the actionList.
        if (fExecuteAndHold)
           actionList.add(new STAXHoldAction("'main'"));

        // Add additional Python variables about the Job
        thread.pySetVar("STAXJobID", new Integer(fJobNumber));
        thread.pySetVar("STAXJobName", fJobName);
        thread.pySetVar("STAXJobXMLFile", fXmlFile);
        thread.pySetVar("STAXJobXMLMachine", fXmlMachine);
        thread.pySetVar("STAXJobStartDate", fStartTimestamp.getDateString());
        thread.pySetVar("STAXJobStartTime", fStartTimestamp.getTimeString());
        thread.pySetVar("STAXJobSourceMachine", fSourceMachine);
        thread.pySetVar("STAXJobSourceHandleName", fSourceHandleName);
        thread.pySetVar("STAXJobSourceHandle", new Integer(fSourceHandle));
        thread.pySetVar("STAXJobStartFunctionName",
                        fDocument.getStartFunction());
        thread.pySetVar("STAXJobStartFunctionArgs",
                        fDocument.getStartFunctionArgs());
        thread.pySetVar("STAXCurrentFunction", Py.None);
        thread.pySetVar("STAXCurrentXMLFile", fXmlFile);
        thread.pySetVar("STAXCurrentXMLMachine", fXmlMachine);

        if (!fScriptFileMachine.equals(""))
            thread.pySetVar("STAXJobScriptFileMachine", fScriptFileMachine);
        else
            thread.pySetVar("STAXJobScriptFileMachine", fSourceMachine);

        thread.pySetVar("STAXJobScriptFiles", fScriptFiles.toArray());
        thread.pySetVar("STAXJobHandle", fHandle);

        thread.pySetVar("STAXServiceName", fSTAX.getServiceName());
        thread.pySetVar("STAXServiceMachine", fSTAX.getLocalMachineName());
        thread.pySetVar("STAXServiceMachineNickname",
                        fSTAX.getLocalMachineNickname());
        thread.pySetVar("STAXEventServiceName", fSTAX.getEventServiceName());
        thread.pySetVar("STAXEventServiceMachine",
                        fSTAX.getEventServiceMachine());

        thread.pySetVar("STAXJobUserLog", new STAFLog(
            STAFLog.MACHINE,
            fSTAX.getServiceName().toUpperCase() + "_Job_" +
            fJobNumber + "_User",
            fHandle, 0));
        thread.pySetVar("STAXJobLogName",
                        fSTAX.getServiceName().toUpperCase() +
                        "_Job_" + fJobNumber);
        thread.pySetVar("STAXJobUserLogName",
                        fSTAX.getServiceName().toUpperCase() +
                        "_Job_" + fJobNumber + "_User");
        
        thread.pySetVar("STAXJobWriteLocation", fJobDataDir);
        thread.pySetVar("STAXMessageLog", new Integer(0));
        thread.pySetVar("STAXLogMessage", new Integer(0));

        if (fLogTCElapsedTime)
            thread.pySetVar("STAXLogTCElapsedTime", new Integer(1));
        else
            thread.pySetVar("STAXLogTCElapsedTime", new Integer(0));

        if (fLogTCNumStarts)
            thread.pySetVar("STAXLogTCNumStarts", new Integer(1));
        else
            thread.pySetVar("STAXLogTCNumStarts", new Integer(0));

        if (fLogTCStartStop)
            thread.pySetVar("STAXLogTCStartStop", new Integer(1));
        else
            thread.pySetVar("STAXLogTCStartStop", new Integer(0));

        thread.pySetVar("STAXPythonOutput",
                        STAXPythonOutput.getPythonOutputAsString(
                            getPythonOutput()));

        thread.pySetVar("STAXPythonLogLevel", getPythonLogLevel());
 
        // Add default signal handlers to the actionList for the main block.
        addDefaultSignalHandlers(actionList);
     
        // Put the "main" function/block at the bottom of the stack,
        // with its action being a sequence group that contains the
        // default actions (scripts and signalhandlers) in the <stax> 
        // element and then the call of the main function. 
      
        LinkedList defaultActions = fDocument.getDefaultActions();
        
        while (!defaultActions.isEmpty())
        {
            actionList.add((STAXAction)defaultActions.removeLast());
        }

        actionList.add(action);  // Add call of main function

        // Create a sequence action for the "main" function/block
        STAXActionDefaultImpl mainSequence = new STAXSequenceAction(actionList);
        mainSequence.setLineNumber("sequence", "<Internal>");
        mainSequence.setXmlFile(getXmlFile());
        mainSequence.setXmlMachine(getXmlMachine());

        STAXActionDefaultImpl mainBlock = new STAXBlockAction(
            "main", mainSequence);
        mainBlock.setLineNumber("block", "<Internal>");
        mainBlock.setXmlFile(getXmlFile());
        mainBlock.setXmlMachine(getXmlMachine());

        thread.pushAction(mainBlock);

        // Change the job's state from pending to running
        fState = RUNNING_STATE;
        
        // Generate the job is running event

        HashMap jobRunningMap = new HashMap();
        jobRunningMap.put("type", "job");
        jobRunningMap.put("status", "run");
        jobRunningMap.put("jobID", String.valueOf(fJobNumber));
        jobRunningMap.put("startFunction", fDocument.getStartFunction());
        jobRunningMap.put("jobName", fJobName);
        jobRunningMap.put("startTimestamp",
                          fStartTimestamp.getTimestampString());

        generateEvent(STAXJob.STAX_JOB_EVENT, jobRunningMap, true);

        thread.pySetVar("STAXJob", this);

        thread.setBreakpointFirstFunction(fBreakpointFirstFunction);

        // Schedule the thread to run 
        thread.schedule();
    }
    
    /**
     * This is a recursive function that checks if a function needs to
     * import other xml files that contain functions it requires.
     * 
     * Checks if the specified function has any function-import sub-elements
     * that specify files to be imported.  If so, it parses the files (if not
     * already in the cache) and adds their required functions to the job's
     * fFunctionMap. For any new functions added to the job's fFunctionMap,
     * it recursively calls the addImportedFunctions method.
     */ 
    public void addImportedFunctions(STAXFunctionAction functionAction) 
        throws STAFException, STAXException
    {
        List importList = functionAction.getImportList();

        if (importList.size() == 0)
            return;

        // Process function-import list (since not empty)

        String evalElem = "function-import";
        int evalIndex = 0;
        Iterator iter = importList.iterator();

        while (iter.hasNext())
        {
            STAXFunctionImport functionImport =
                (STAXFunctionImport)iter.next();

            String machine = functionImport.getMachine();
            String file = functionImport.getFile();
            String directory = functionImport.getDirectory();

            boolean directorySpecified = false;

            if (directory != null)
                directorySpecified = true;

            boolean machineSpecified = false;

            if (machine != null)
            {
                machineSpecified = true;
                
                // Check if "machine" contains any STAF variables
                
                if (machine.indexOf("{") != -1)
                {
                    // Resolve variables on the local STAX service machine

                    STAFResult result = this.submitSync(
                        "local", "VAR", "RESOLVE STRING " +
                        STAFUtil.wrapData(machine));

                    if (result.rc != STAFResult.Ok)
                    {
                        String errorMsg = "Cause:  Error resolving STAF " +
                            "variables in the \"machine\" attribute " +
                            "for element type \" function-import\"," + 
                            "\nRC: " + result.rc +
                            ", Result: " + result.result;

                        functionAction.setElementInfo(
                            new STAXElementInfo(
                                evalElem, "machine", evalIndex, errorMsg));

                        throw new STAXFunctionImportException(
                            STAXUtil.formatErrorMessage(functionAction));
                    }

                    machine = result.result;
                }
            }
            else
            {
                machine = fXmlMachine;
            }

            // Check if file or directory contains any STAF variables.

            String evalAttr = "file";
            String unresValue = file;

            if (directorySpecified)
            {
                evalAttr = "directory";
                unresValue = directory;
            }
            
            if (unresValue.indexOf("{") != -1)
            {
                // Resolve variables on the local STAX service machine

                STAFResult result = this.submitSync(
                    "local", "VAR", "RESOLVE STRING " +
                    STAFUtil.wrapData(unresValue));

                if (result.rc != STAFResult.Ok)
                {
                    String errorMsg = "Cause:  Error resolving STAF " +
                        "variables in the \"" + evalAttr + "\" attribute " +
                        "for element type \"function-import\"." + 
                        "\nRC: " + result.rc +
                        ", Result: " + result.result;

                    functionAction.setElementInfo(
                        new STAXElementInfo(
                            evalElem, evalAttr, evalIndex, errorMsg));

                    throw new STAXFunctionImportException(
                        STAXUtil.formatErrorMessage(functionAction));
                }

                if (!directorySpecified)
                    file = result.result;
                else
                    directory = result.result;
            }

            // Handle any functions specified to be imported

            String functions = functionImport.getFunctions();
            
            Vector importedFunctionList = new Vector();

            if (functions != null)
            {
                // Convert string containing a whitespace-separated list of
                // functions to a vector.
                //
                // Note: The tokenizer uses the default delimiter set, which
                // is " \t\n\r\f" and consists of the space character,
                // the tab character, the newline character, the
                // carriage-return character, and the form-feed character.
                // Delimiter characters themselves will not be treated as
                // tokens. 

                StringTokenizer st = new StringTokenizer(functions);

                while (st.hasMoreElements())
                {
                    importedFunctionList.add(st.nextElement());
                }
            }

            // Get the file separator for the machine where the import file/
            // directory resides as this is needed to normalize the path name
            // and may also be needed to determine if its a relative file
            // name, and to get the parent directory.
            
            STAFResult result = STAXFileCache.getFileSep(
                machine, this.getSTAX().getSTAFHandle());

            if (result.rc != STAFResult.Ok)
            {
                String errorMsg = "Cause:  No response from machine \"" +
                    machine + "\" when trying to get the contents of a " +
                    "file specified by element type \"function-import\"." +
                    "\nRC: " + result.rc + ", Result: " + result.result;

                functionAction.setElementInfo(
                    new STAXElementInfo(
                        evalElem, "machine", evalIndex, errorMsg));

                throw new STAXFunctionImportException(
                    STAXUtil.formatErrorMessage(functionAction));
            }

            String fileSep = result.result;

            // Set a flag to indicate if the file/directory name is
            // case-sensitive (e.g. true if it resides on a Unix machine,
            // false if Windows)

            boolean caseSensitiveFileName = true;

            if (fileSep.equals("\\"))
            {
                // Windows machine so not case-sensitive

                caseSensitiveFileName = false;
            }

            // Import the file specified in the function-import element.
            // If the file specified in the function-import element isn't
            // already in the file cache, get the file and parse it.
            // Then, add the functions defined in this file to the main job's
            // function map.

            // If no machine attribute is specified, then the file specified
            // could be a relative file name so need to assign its absolute
            // file name

            if (!machineSpecified)
            {
                // Check if a relative path was specified

                String entry = file;

                if (directorySpecified)
                    entry = directory;

                if (STAXUtil.isRelativePath(entry, fileSep))
                {
                    // Assign the absolute name assuming it is relative
                    // to the parent xml file's path

                    String currentFile = functionAction.getXmlFile();
                    
                    if (currentFile.equals(STAX.INLINE_DATA))
                    {
                        // Cannot specify a relative file path if the parent
                        // xml file is STAX.INLINE_DATA

                        String errorMsg = "Cause:  Cannot specify a " +
                            "relative path in attribute \"" + evalAttr +
                            "\" for element type \"function-import\" when " +
                            "the parent xml file is " + STAX.INLINE_DATA;

                        functionAction.setElementInfo(
                            new STAXElementInfo(
                                evalElem, STAXElementInfo.NO_ATTRIBUTE_NAME,
                                evalIndex, errorMsg));

                        throw new STAXFunctionImportException(
                            STAXUtil.formatErrorMessage(functionAction));
                    }

                    entry = STAXUtil.getParentPath(currentFile, fileSep) +
                        entry;

                    if (!directorySpecified)
                        file = entry;
                    else
                        directory = entry;
                }
            }

            // Normalize the import file/directory name so that we have a
            // better chance at matching file names that are already cached

            if (!directorySpecified)
                file = STAXUtil.normalizeFilePath(file, fileSep);
            else
                directory = STAXUtil.normalizeFilePath(directory, fileSep);

            // Create a STAX XML Parser

            STAXParser parser = null;

            try
            {
                parser = new STAXParser(getSTAX());
            }
            catch (Exception ex)
            {
                String errorMsg = ex.getClass().getName() + "\n" +
                    ex.getMessage();
                
                functionAction.setElementInfo(
                    new STAXElementInfo(
                        evalElem, STAXElementInfo.NO_ATTRIBUTE_NAME,
                        evalIndex, errorMsg));

                throw new STAXFunctionImportException(
                    STAXUtil.formatErrorMessage(functionAction));
            }

            // Create a list of files to process

            List theFileList = new ArrayList();

            if (!directorySpecified)
            {
                // There will be just one file to process

                theFileList.add(file);
            }
            else
            {
                // Submit a FS LIST DIRECTORY request for all *.xml files
                // in the directory

                result = submitSync(
                    machine, "FS", "LIST DIRECTORY " +
                    STAFUtil.wrapData(directory) +
                    " TYPE F EXT xml CASEINSENSITIVE");

                if (result.rc != 0)           
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    String errorMsg = "Cause:  ";

                    if (result.rc == STAFResult.NoPathToMachine)
                    {
                        errorMsg = errorMsg + "No response from machine ";
                        evalAttr = "machine";
                    }
                    else
                    {
                        errorMsg = errorMsg + "Error ";
                    }

                    errorMsg = errorMsg + "when submitting a FS LIST " +
                        "DIRECTORY request to list all *.xml files in the " +
                        "directory specified by element type " +
                        "\"function-import\":" +
                        "\n  Directory: " + directory +
                        "\n  Machine: " + machine +
                        "\n\nRC: " + result.rc + ", Result: " + result.result;

                    functionAction.setElementInfo(
                        new STAXElementInfo(
                            evalElem, evalAttr, evalIndex, errorMsg));

                    throw new STAXFunctionImportException(
                        STAXUtil.formatErrorMessage(functionAction));
                }

                Iterator dirListIter = ((List)result.resultObj).iterator();

                while (dirListIter.hasNext())
                {
                    theFileList.add(directory + fileSep +
                                    (String)dirListIter.next());
                }
            }

            Iterator fileListIter = theFileList.iterator();

            while (fileListIter.hasNext())
            {
                file = (String)fileListIter.next();

                Date dLastModified = null;
                STAXJob job = null;

                // If file caching is enabled, find the modification date of
                // the file being imported

                if (getSTAX().getFileCaching())
                {
                    if (STAXFileCache.get().isLocalMachine(machine))
                    {
                        File fileObj = new File(file);

                        // Make sure the file exists

                        if (fileObj.exists())
                        {
                            long lastModified = fileObj.lastModified();

                            if (lastModified > 0)
                            {
                                // Chop off the milliseconds because some
                                // systems don't report modTime to milliseconds

                                lastModified = ((long)(lastModified/1000))*1000;

                                dLastModified = new Date(lastModified);
                            }
                        }
                    }

                    if (dLastModified == null)
                    {
                        // Find the remote file mod time using STAF

                        STAFResult entryResult = submitSync(
                            machine, "FS", "GET ENTRY " +
                            STAFUtil.wrapData(file) + " MODTIME");

                        if (entryResult.rc == 0)
                        {
                            String modDate = entryResult.result;
                            dLastModified = STAXFileCache.convertSTAXDate(
                                modDate);
                        }
                    }

                    // Check for an up-to-date file in the cache

                    if ((dLastModified != null) &&
                        STAXFileCache.get().checkCache(
                            machine, file, dLastModified,
                            caseSensitiveFileName))
                    {
                        // Get the doc from cache
                        
                        STAXDocument doc = STAXFileCache.get().getDocument(
                            machine, file, caseSensitiveFileName);

                        if (doc != null)
                        {
                            job = new STAXJob(getSTAX(), doc);
                        }
                    }
                }

                // If the file was not in cache, then retrieve it using STAF

                if (job == null)
                {
                    result = submitSync(
                        machine, "FS", "GET FILE " + STAFUtil.wrapData(file));

                    if (result.rc != 0)           
                    {
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                        String errorMsg = "Cause:  ";

                        if (result.rc == STAFResult.NoPathToMachine)
                        {
                            errorMsg = errorMsg + "No response from machine ";
                            evalAttr = "machine";
                        }
                        else
                        {
                            errorMsg = errorMsg + "Error ";
                        }

                        errorMsg = errorMsg + "when submitting a FS GET " +
                            "request to get a file specified by element type " +
                            "\"function-import\":" +
                            "\n  File: " + file +
                            "\n  Machine: " + machine +
                            "\n\nRC: " + result.rc + ", Result: " + result.result;

                        functionAction.setElementInfo(
                            new STAXElementInfo(
                                evalElem, evalAttr, evalIndex, errorMsg));

                        throw new STAXFunctionImportException(
                            STAXUtil.formatErrorMessage(functionAction));
                    }

                    // Parse the XML document

                    try
                    {
                        job = parser.parse(result.result, file, machine);
                    }
                    catch (Exception ex)
                    {
                        String errorMsg = "Cause: " +
                            ex.getClass().getName() + "\n";

                        // Make sure that the error message contains the File:
                        // and Machine: where the error occurred

                        if (ex.getMessage().indexOf("File: ") == -1 ||
                            ex.getMessage().indexOf("Machine: ") == -1)
                        {
                            errorMsg = errorMsg + "\nFile: " + file +
                                ", Machine: " + machine;
                        }

                        errorMsg = errorMsg + ex.getMessage();
                        
                        functionAction.setElementInfo(
                            new STAXElementInfo(
                                evalElem, STAXElementInfo.NO_ATTRIBUTE_NAME,
                                evalIndex, errorMsg));

                        throw new STAXFunctionImportException(
                            STAXUtil.formatErrorMessage(functionAction));
                    }

                    // Add the XML document to the cache

                    if (getSTAX().getFileCaching() && (dLastModified != null))
                    {
                        STAXFileCache.get().addDocument(
                            machine, file, job.getSTAXDocument(),
                            dLastModified, caseSensitiveFileName);
                    }
                }

                // Get a map of the functions in this xml file being imported

                HashMap functionMap = job.getSTAXDocument().getFunctionMap();

                // Verify that if function names were specified in the
                // function-import element, verify that all the function names
                // specified exist in the xml file

                if (!importedFunctionList.isEmpty())
                {
                    Iterator functionIterator = importedFunctionList.iterator();

                    while (functionIterator.hasNext())
                    {
                        String functionName = (String)functionIterator.next();

                        if (!functionMap.containsKey(functionName))
                        {
                            // Function name specified in the function-import
                            // element's "functions" attribute does not exist
                        
                            String errorMsg = "Cause:  Function \"" +
                                functionName + "\" does not exist in file \"" +
                                file + "\" on machine \"" + machine + "\".";

                            functionAction.setElementInfo(
                                new STAXElementInfo(
                                    evalElem, STAXElementInfo.NO_ATTRIBUTE_NAME,
                                    evalIndex, errorMsg));

                            throw new STAXFunctionImportException(
                                STAXUtil.formatErrorMessage(functionAction));
                        }
                    }
                }
            
                // Create a set containing the functions requested to be
                // imported.  If no functions were specifically requested to
                // be imported, add all the functions in the xml file to the
                // set.
            
                Set functionSet;

                if (importedFunctionList.isEmpty())
                    functionSet = functionMap.keySet();
                else
                    functionSet = new LinkedHashSet(importedFunctionList);

                // Add the functions requested to be imported via a
                // function-import element to the main job's fFunctionMap and
                // add any other functions that these functions also require
                // to the main job's fFunctionMap

                Iterator functionIterator = functionSet.iterator();
                Vector requiredFunctionList = new Vector();

                while (functionIterator.hasNext())
                {
                    String functionName = (String)functionIterator.next();

                    STAXFunctionAction function =
                        (STAXFunctionAction)functionMap.get(functionName);

                    // If this function does not exist, add the function to
                    // the job's fFunctionMap

                    if (!(functionExists(functionName)))
                    {
                        // Add the function to the job's function map

                        this.addFunction(function);
                    
                        // Check if this function requires any other functions
                        // and if so, add them to the list of required
                        // functions

                        StringTokenizer requiredFunctions =
                            new StringTokenizer(function.getRequires(), " ");

                        while (requiredFunctions.hasMoreElements())
                        {
                            requiredFunctionList.add(
                                requiredFunctions.nextElement());
                        }

                        // Recursive call to add functions this functions
                        // requires if specified by function-import elements

                        addImportedFunctions(function);
                    }
                }

                // Process required functions (functions that the imported
                // functions also require)

                for (int i = 0; i < requiredFunctionList.size(); i++)
                {
                    String functionName =
                        (String)requiredFunctionList.elementAt(i);

                    if (!functionExists(functionName))
                    {
                        addRequiredFunctions(functionName, functionMap);
                    }
                }
            } // End while (fileListIter.hasNext())

            evalIndex++;
        } // End while
    }

    /**
     * This recursive method adds the specified required function to the
     * main job's fFunctionMap.  And, if this function has any function-import
     * elements, it recursively adds these functions that are required by the
     * imported function.  And, if this function has any required functions
     * from the same xml file, it recursively adds these required functions.
     */ 
    private void addRequiredFunctions(String functionName, HashMap functionMap)
        throws STAFException, STAXException
    {
        STAXFunctionAction function = 
            (STAXFunctionAction)functionMap.get(functionName);        

        this.addFunction(function);

        // If this function has any function-import elements, recursively
        // add these functions from the imported xml file that are
        // required by the imported function to the main job's
        // fFunctionMap if not already present

        addImportedFunctions(function);

        // If this function has a "requires" attribute specifying
        // additional functions in the same xml file that it requires,
        // recursively add these required functions to the main job's
        // fFunctionMap if not already present

        StringTokenizer requiredFunctions = new StringTokenizer(
            function.getRequires(), " ");
            
        while (requiredFunctions.hasMoreElements())
        {
            String reqFunctionName = (String)requiredFunctions.nextElement();

            if (!functionExists(reqFunctionName))
            {
                addRequiredFunctions(reqFunctionName, functionMap);
            }
        }
    }

    public void addDefaultSignalHandlers(ArrayList actionList)
    {
        // Array of default SignalHandlers.  Each signal is described by
        // a signal name, an action, and a signal message variable name.
        // If the signal message variable name is not null, a message will
        // be sent to the STAX Monitor and logged with level 'error'.
        
        String[][] defaultSHArray = 
        {
            {
                "STAXPythonEvaluationError", "terminate", "STAXPythonEvalMsg"
            },
            {
                "STAXProcessStartError", "continue", "STAXProcessStartErrorMsg"
            },
            {
                "STAXProcessStartTimeout", "continue",
                "STAXProcessStartTimeoutMsg"
            },
            {
                "STAXCommandStartError", "terminate",
                "STAXCommandStartErrorMsg"
            },
            {
                "STAXFunctionDoesNotExist", "terminate",
                "STAXFunctionDoesNotExistMsg"
            },
            {
                "STAXInvalidBlockName", "terminate", "STAXInvalidBlockNameMsg"
            },
            {
                "STAXBlockDoesNotExist", "continue", "STAXBlockDoesNotExistMsg"
            },
            {
                "STAXLogError", "continue", "STAXLogMsg"
            },
            {
                "STAXTestcaseMissingError", "continue",
                "STAXTestcaseMissingMsg"
            },
            {
                "STAXInvalidTcStatusResult", "continue",
                "STAXInvalidTcStatusResultMsg"
            },
            {
                "STAXInvalidTimerValue", "terminate",
                "STAXInvalidTimerValueMsg"
            },
            {
                "STAXNoSuchSignalHandler", "continue",
                "STAXNoSuchSignalHandlerMsg"
            },
            {
                "STAXEmptyList", "continue", null
            },
            {
                "STAXMaxThreadsExceeded", "terminate",
                "STAXMaxThreadsExceededMsg"
            },
            {
                "STAXInvalidMaxThreads", "terminate",
                "STAXInvalidMaxThreadsMsg"
            },
            {
                "STAXFunctionArgValidate", "terminate",
                "STAXFunctionArgValidateMsg"
            },
            {
                "STAXImportError", "terminate", "STAXImportErrorMsg"
            },
            {
                "STAXFunctionImportError", "terminate",
                "STAXFunctionImportErrorMsg"
            },
            {
                "STAXInvalidTestcaseMode", "continue",
                "STAXInvalidTestcaseModeMsg"
            }
        };

        // Add default SignalHandlers to actionList

        for (int i = 0; i < defaultSHArray.length; i++)
        {
            ArrayList signalHandlerActionList = new ArrayList();

            String signalName = defaultSHArray[i][0];
            String signalAction = defaultSHArray[i][1];
            String signalMsgVarName = defaultSHArray[i][2];
            String signalMsgText = "";

            if (signalAction.equals("terminate"))
            {
                signalMsgText = "'" + signalName + " signal raised. " +
                    "Terminating job. '" + " + " + signalMsgVarName;
            }
            else if (signalAction.equals("continue"))
            {
                signalMsgText = "'" + signalName + " signal raised. " +
                    "Continuing job. '" + " + " + signalMsgVarName;
            }

            if (signalMsgVarName == null)
            {
                // Add a No Operation (nop) Action to the action list

                signalHandlerActionList.add(new STAXNopAction());
            }
            else
            {
                // Add a Log Action to the action list

                int logfile = STAXJob.JOB_LOG;

                if (signalName == "STAXLogError")
                {
                    // Log the error message in the STAX JVM log (otherwise
                    // may get a duplicate STAXLogError signal)

                    logfile = STAXJob.JVM_LOG;
                }

                // Log the message with level 'Error' and send the message
                // to the STAX Monitor

                signalHandlerActionList.add(new STAXLogAction(
                    signalMsgText, "'error'", "1", "1", logfile));
            }
            
            if (signalAction.equals("terminate"))
            {
                // Add a Terminate Job Action to the action list

                signalHandlerActionList.add(
                    new STAXTerminateAction("'main'"));
            }

            // Add the signalhandler action to the action list

            if (signalHandlerActionList.size() == 1)
            {
                // Don't need a STAXSequenceAction since only 1 action in list

                actionList.add(new STAXSignalHandlerAction(
                    "'" + signalName + "'",
                    (STAXAction)signalHandlerActionList.get(0)));
            }
            else
            {
                // Wrap the action list is a STAXSequenceAction

                actionList.add(new STAXSignalHandlerAction(
                    "'" + signalName + "'",
                    new STAXSequenceAction(signalHandlerActionList)));
            }
        }
    }

    //
    // Event methods
    //

    public void generateEvent(String eventSubType, Map details)
    {
        generateEvent(eventSubType, details, false);
    }

    public void generateEvent(String eventSubType, Map details, 
                              boolean notifyAll)
    {
        StringBuffer detailsString = new StringBuffer();
        String key = "";
        String value = "";

        Iterator keyIter = details.keySet().iterator();
        
        while (keyIter.hasNext())
        {
            key = (String)keyIter.next();
            
            value = (String)details.get(key);
            
            if (value == null)
            {
                // do nothing
            }
            else
            {
                detailsString.append("PROPERTY ").append(
                    STAFUtil.wrapData(key + "=" + value)).append(" ");
            }
        }
        
        // The type machine should always be the local machine
        // The details parm must already be in the :length: format        
        STAFResult result = submitSync(
                        fSTAX.getEventServiceMachine(),
                        fSTAX.getEventServiceName(), "GENERATE TYPE " +
                        fSTAX.getServiceName().toUpperCase() + "/" + 
                        fSTAX.getLocalMachineName() + "/" + fJobNumber +
                        " SUBTYPE " + STAFUtil.wrapData(eventSubType) +
                        " ASYNC " + detailsString.toString());
                        
        // Debug
        if (false)
        {
            System.out.println("Event:\n" +  "GENERATE TYPE " +
                        fSTAX.getServiceName().toUpperCase() + "/" + 
                        fSTAX.getLocalMachineName() + "/" + fJobNumber +
                        " SUBTYPE " + STAFUtil.wrapData(eventSubType) +
                        " ASYNC " + details);
        }
        
        if (notifyAll)
        {
            submitSync(fSTAX.getEventServiceMachine(),
                       fSTAX.getEventServiceName(), "GENERATE TYPE " +
                       fSTAX.getServiceName().toUpperCase() + "/" + 
                       fSTAX.getLocalMachineName() + " SUBTYPE " + 
                       STAFUtil.wrapData(eventSubType) +
                       " ASYNC " + detailsString.toString());
        
            // Debug
            if (false)
            {
                System.out.println("Event:\n" + "GENERATE TYPE " +
                            fSTAX.getServiceName().toUpperCase() + "/" + 
                            fSTAX.getLocalMachineName() + " SUBTYPE " + 
                            STAFUtil.wrapData(eventSubType) +
                            " ASYNC " + details);
            }
        }
    }

    public STAFResult log(int logfile, String level, String message)
    {
        if (logfile == STAXJob.JVM_LOG)
        {
            // Log to the JVM log instead of a STAX Job log

            STAXTimestamp currentTimestamp = new STAXTimestamp();

            System.out.println(
                currentTimestamp.getTimestampString() + " " + message);

            return new STAFResult();
        }

        String serviceName = fSTAX.getServiceName().toUpperCase();
        String logName;

        // Don't resolve messages logged to the STAX_Service log and to
        // STAX Job logs to avoid possible RC 13 errors which would prevent
        // the message from being logged.

        boolean noResolveMessage = false;

        if (logfile == STAXJob.SERVICE_LOG)
        {
            logName = serviceName + "_Service";
            noResolveMessage = true;
        }
        else if (logfile == STAXJob.JOB_LOG)
        {
            logName = serviceName + "_Job_" + fJobNumber;
            noResolveMessage = true;
        }
        else if (logfile == STAXJob.USER_JOB_LOG)
        {
            logName = serviceName + "_Job_" + fJobNumber + "_User";
        }
        else
        {
            // Log to STAX_Service log if invalid logfile specified 

            STAXTimestamp currentTimestamp = new STAXTimestamp();
            System.out.println(currentTimestamp.getTimestampString() +
                               " STAX Service Error: Invalid Logfile " +
                               logfile);

            logName = serviceName + "_Service";
            noResolveMessage = true;
        }

        String logRequest = "LOG MACHINE LOGNAME " +
            STAFUtil.wrapData(logName) + " LEVEL " + level;

        if (noResolveMessage) logRequest += " NORESOLVEMESSAGE";

        logRequest += " MESSAGE " + STAFUtil.wrapData(message);
        
        STAFResult result = submitSync("LOCAL", "LOG", logRequest);

        // Check if the result was unsuccessful except ignore the following
        // errors:
        // - UnknownService (2) in case the LOG service is not registered
        // - HandleDoesNotExist (5) in case the STAX job's handle has been
        //   unregistered indicating the job has completed

        if ((result.rc != STAFResult.Ok) &&
            (result.rc != STAFResult.UnknownService) &&
            (result.rc != STAFResult.HandleDoesNotExist))
        {
            if (logfile != STAXJob.USER_JOB_LOG)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() +
                    " Log request failed with RC " + result.rc +
                    " and Result " + result.result +
                    "  level: " + level +
                    "  logRequest: " + logRequest);
            }
            else if ((result.rc == STAFResult.VariableDoesNotExist) &&
                     (!noResolveMessage))
            {
                // Retry logging the error message without resolving
                // variables in the message

                logRequest += " NORESOLVEMESSAGE";

                result = submitSync("LOCAL", "LOG", logRequest);
            }
        }

        return result;
    }
    
    public void clearLogs()
    {
        String serviceName = fSTAX.getServiceName().toUpperCase();
        String jobLogName;
        String userJobLogName;
        
        jobLogName = serviceName + "_Job_" + fJobNumber;
        userJobLogName = serviceName + "_Job_" + fJobNumber + "_User";
        
        String logRequest = "DELETE MACHINE " +
            fSTAX.getLocalMachineNickname() + " LOGNAME " + 
            STAFUtil.wrapData(jobLogName) + " CONFIRM";
                                
        STAFResult result = submitSync("LOCAL", "LOG", logRequest);
        
        logRequest = "DELETE MACHINE " + fSTAX.getLocalMachineNickname() +
            " LOGNAME " +  STAFUtil.wrapData(userJobLogName) + " CONFIRM";
                                
        result = submitSync("LOCAL", "LOG", logRequest);
    }
    
    public void clearJobDataDir()
    {
        // Delete the job data directory and recreate it
        
        File dir = new File(fJobDataDir);

        if (dir.exists())
        {
            String deleteDirRequest = "DELETE ENTRY " +
                STAFUtil.wrapData(fJobDataDir) + " RECURSE CONFIRM";
                                
            submitSync("local", "FS", deleteDirRequest);
        }

        if (!dir.exists())
        {
            dir.mkdirs();
        }
    }

    public List getBreakpointFunctionList()
    {
        return fBreakpointFunctionList;
    }

    public void setBreakpointFunctionList(List functionList)
    {
        synchronized(fBreakpointFunctionList)
        {
            fBreakpointFunctionList = functionList;
        }
    }

    public int addBreakpointFunction(String functionName)
    {
        int breakpointID = getNextBreakpointNumber();

        synchronized(fBreakpointFunctionList)
        {
            fBreakpointFunctionList.add(functionName);
        }

        synchronized (fBreakpointsMap)
        {
            fBreakpointsMap.put(String.valueOf(breakpointID),
                new STAXBreakpoint(BREAKPOINT_FUNCTION,
                                  functionName, "", "", ""));
        }

        return breakpointID;
    }

    public boolean isBreakpointFunction(String functionName)
    {
        synchronized (fBreakpointsMap)
        {
            Iterator iter = fBreakpointsMap.keySet().iterator();

            while (iter.hasNext())
            {
                String id = (String)iter.next();

                STAXBreakpoint breakpoint =
                    (STAXBreakpoint)fBreakpointsMap.get(id);

                if (functionName.equals(breakpoint.getFunction()))
                {
                    return true;
                }
            }
        }
        return false;
    }

    public List getBreakpointLineList()
    {
        return fBreakpointLineList;
    }

    public TreeMap getBreakpointsMap()
    {
        return fBreakpointsMap;
    }

    public void setBreakpointLineList(List lineList)
    {
        synchronized(fBreakpointLineList)
        {
            fBreakpointLineList = lineList;
        }
    }

    public STAFResult removeBreakpoint(String id)
    {
        synchronized(fBreakpointsMap)
        {
            if (!(fBreakpointsMap.containsKey(id)))
            {
                return new STAFResult(STAFResult.DoesNotExist, id);
            }
            else
            {
                fBreakpointsMap.remove(id);

                return new STAFResult(STAFResult.Ok);
            }
        }
    }

    public int addBreakpointLine(String line, String file, String machine)
    {
        int breakpointID = getNextBreakpointNumber();

        synchronized(fBreakpointLineList)
        {
            fBreakpointLineList.add(line + " " + machine + " " + file);
        }

        synchronized (fBreakpointsMap)
        {
            fBreakpointsMap.put(String.valueOf(breakpointID),
                new STAXBreakpoint(BREAKPOINT_LINE,
                                   "", line, file, machine));
        }

        return breakpointID;
    }

    public boolean isBreakpointLine(String line, String file, String machine)
    {
        synchronized (fBreakpointsMap)
        {
            Iterator iter = fBreakpointsMap.keySet().iterator();

            while (iter.hasNext())
            {
                String id = (String)iter.next();

                STAXBreakpoint breakpoint =
                    (STAXBreakpoint)fBreakpointsMap.get(id);

                if (line.equals(breakpoint.getLine()) &&
                    file.equals(breakpoint.getFile()) &&
                    (breakpoint.getMachine().equals("") ||
                    machine.equalsIgnoreCase(breakpoint.getMachine())))
                {
                    return true;
                }
            }
        }
        return false;
    }

    public boolean breakpointsEmpty()
    {
        return (fBreakpointsMap.isEmpty());
    }

    // Submit methods
    
    public STAFResult submitAsync(String location, String service, 
        String request, STAXSTAFRequestCompleteListener listener)
    {
        STAFResult result;
        
        synchronized(fRequestMap)
        {
            result = fHandle.submit2(
                STAFHandle.ReqQueue, location, service, request);
            
            if (result.rc == STAFResult.Ok)
            {
                try
                {
                    // Convert request number to a negative integer if greater
                    // then Integer.MAX_VALUE

                    Integer requestNumber = STAXUtil.convertRequestNumber(
                        result.result);

                    fRequestMap.put(requestNumber, listener);

                    result.result = requestNumber.toString();
                }
                catch (NumberFormatException e)
                {
                    System.out.println(
                        (new STAXTimestamp()).getTimestampString() +
                        " STAXJob::submitAsync - " + e.toString());

                    result.result = "0";
                }
            }
        }
        
        return result;
    }
    
    public STAFResult submitSync(String location, String service, 
                                 String request)
    {
        STAFHandle theHandle = fHandle;
        
        // If the STAX job's handle has not yet been assigned, use the
        // STAX service's handle to submit the STAF service request

        if (fHandle == null)
            theHandle = fSTAX.getSTAFHandle();

        STAFResult result = theHandle.submit2(
            STAFHandle.ReqSync, location, service, request);
        
        return result;
    }

    public STAFResult submitAsyncForget(String location, String service, 
                                        String request)
    {
        STAFResult result = fHandle.submit2(STAFHandle.ReqFireAndForget, 
            location, service, request);
        
        return result;
    }
    
    // STAXSTAFQueueListener method

    public void handleQueueMessage(STAXSTAFMessage message, STAXJob job)
    {
        int requestNumber = message.getRequestNumber();

        STAXSTAFRequestCompleteListener listener = null;

        synchronized (fRequestMap)
        {
            Integer key = new Integer(requestNumber);

            listener = (STAXSTAFRequestCompleteListener)fRequestMap.get(key);

            if (listener != null)
            {
                fRequestMap.remove(key);
            }
        }

        if (listener != null)
        {
            listener.requestComplete(requestNumber, new STAFResult(
                message.getRequestRC(), message.getRequestResult()));
        }
        else
        {   // Log a message in the job log
            String msg = "STAXJob.handleQueueMessage: " +
                         " No listener found for message:\n" +
                         STAFMarshallingContext.unmarshall(
                             message.getResult()).toString();
            job.log(STAXJob.JOB_LOG, "warning", msg);
        }
    }

    // STAXThreadCompleteListener method

    public void threadComplete(STAXThread thread, int endCode)
    {
        // Debug:
        if (false)
        {
            System.out.println("Thread #" + thread.getThreadNumber() +
                               " complete");
        }

        boolean jobComplete = false;

        synchronized (fThreadMap)
        {
            fThreadMap.remove(thread.getThreadNumberAsInteger());

            if (fThreadMap.isEmpty())
                jobComplete = true;
        }

        if (jobComplete == true)
        {
            // Perform terminateJob on interested parties

            fSTAX.visitJobManagementHandlers(new STAXVisitorHelper(this)
            {
                public void visit(Object o, Iterator iter)
                {
                    STAXJobManagementHandler handler =
                                             (STAXJobManagementHandler)o;

                    handler.terminateJob((STAXJob)fData);
                }
            });
            
            // Set the result so that it is available upon job completion.

            String resultToEval = "STAXResult";

            try 
            {
                if (thread.pyBoolEval("isinstance(STAXResult, STAXGlobal)"))
                {
                    // Use the STAXGlobal class's get() method so that the job
                    // result when using toString() will be it's contents and
                    // not org.python.core.PyFinalizableInstance

                    resultToEval = "STAXResult.get()";
                }
            }
            catch (STAXPythonEvaluationException e)
            {   /* Ignore error and assume not a STAXGlobal object */ }
                
            try
            {
                fResult = thread.pyObjectEval(resultToEval);
            }
            catch (STAXPythonEvaluationException e)
            {
                fResult = Py.None;
            }
            
            // Log the result from the job in a Status message in the Job log

            STAFMarshallingContext mc = STAFMarshallingContext.
                unmarshall(fResult.toString());
            log(STAXJob.JOB_LOG, "status", "Job Result: " + mc);
            
            // Log a Stop message for the job in the STAX Service and Job logs

            String msg = "JobID: " + fJobNumber; 
            log(STAXJob.SERVICE_LOG, "stop", msg);
            log(STAXJob.JOB_LOG, "stop", msg);
            
            // Get the current date and time and set as job ending date/time

            fEndTimestamp = new STAXTimestamp();

            // Generate job completion event
            
            HashMap jobEndMap = new HashMap();
            jobEndMap.put("type", "job");
            jobEndMap.put("block", "main");
            jobEndMap.put("status", "end");
            jobEndMap.put("jobID", String.valueOf(fJobNumber));
            jobEndMap.put("result", fResult.toString()); 
            jobEndMap.put("jobCompletionStatus", getCompletionStatusAsString());

            generateEvent(STAXJob.STAX_JOB_EVENT, jobEndMap, true);

            // Query its job log for any messages with level "Error".
            // Save the result's marshalling context so it's available when
            // writing the job result to a file and when the RETURNRESULT
            // option is specified on a STAX EXECUTE request

            fJobLogErrorsMC = getJobLogErrors();

            // Write the job result information to files in the STAX job
            // directory

            writeJobResultsToFile();

            // Send a STAF/Service/STAX/End message to indicate the job has
            // completed

            submitSync("local", "QUEUE", "QUEUE TYPE STAF/Service/STAX/End " +
                       "MESSAGE " + STAFUtil.wrapData(""));

            // Debug
            if (COUNT_PYCODE_CACHES && STAX.CACHE_PYTHON_CODE)
            {   
                System.out.println(
                    "Job " + fJobNumber + ": " +
                    " cacheGets=" + fCompiledPyCodeCacheGets +
                    " cacheAdds=" + fCompiledPyCodeCacheAdds);
            }
            
            while (!fCompletionNotifiees.isEmpty())
            {
                STAXJobCompleteListener listener =
                    (STAXJobCompleteListener)fCompletionNotifiees.removeFirst();

                if (listener != null) listener.jobComplete(this);
            }

            try
            {
                fHandle.unRegister();
            }
            catch (STAFException e)
            {
                /* Do Nothing */
            }

            // Clear out job's private variables
            
            fScripts = new ArrayList();
            fScriptFiles = new ArrayList();
            fThreadMap = new LinkedHashMap();
            fCompletionNotifiees = new LinkedList();
            fCompiledPyCodeCache = new HashMap();

            // Commented out setting these variables to new (empty) objects,
            // as this could cause the "Testcases" and "Testcase Totals" values
            // in the job result to be empty due to timing issues
            //fTestcaseList = new ArrayList();
            //fTestcaseTotalsMap = new HashMap();

            // Commented out setting the following variables to null because
            // STAFQueueMonitor.notifyListeners() is running in another
            // thread and could access these variables and get a NPE.
            // fQueueListenerMap = null;
            // fHandle = null;
            // fSTAFQueueMonitor = null;
            // fRequestMap = new HashMap();
            //
            // synchronized (fDataMap)
            // {
            //     fDataMap = new HashMap();
            // }
        }
    }

    /**
     * This method is called to clean up a job that is in a pending state.
     * This is a job that has had a job number assigned to it but did not
     * start execution because either an error occurred (e.g. xml parsing,
     * etc) before the job actually started execution or because the TEST
     * option was specified, which indicates to test that the job doesn't
     * contain xml parsing or Python compile errors and to not actually
     * run the job.
     */
    public void cleanupPendingJob(STAFResult result)
    {
        STAXJob job = fSTAX.removeFromJobMap(getJobNumberAsInteger());

        if (job == null)
        {
            // The job is not in the job map
            return;
        }

        if (result.rc != STAFResult.Ok)
        {
            setCompletionStatus(TERMINATED_STATUS);
            
            // Log the error message in the job log

            if (STAFMarshallingContext.isMarshalledData(result.result))
            {
                try
                {
                    STAFMarshallingContext mc =
                        STAFMarshallingContext.unmarshall(result.result);
                    Map resultMap = (Map)mc.getRootObject();
                    result.result = (String)resultMap.get("errorMsg");
                }
                catch (Exception e)
                {
                    // Ignore any errors
                    System.out.println(e.toString());
                }
            }

            log(STAXJob.JOB_LOG, "error", result.result);
        }
        
        // Log the testcase totals in the Job log by first calling the
        // STAXTestcaseActionFactory's initJob() method (to initialize the
        // testcase totals to 0) and then by calling its terminateJob() method
        // which logs the testcase totals

        STAXJobManagementHandler testcaseHandler =
            (STAXJobManagementHandler)fSTAX.getActionFactory("testcase");
        
        testcaseHandler.initJob(this);
        testcaseHandler.terminateJob(this);

        // Log the result from the job in a Status message in the Job log

        STAFMarshallingContext mc = STAFMarshallingContext.
            unmarshall(fResult.toString());
        log(STAXJob.JOB_LOG, "status", "Job Result: " + mc);

        // Log a Stop message for the job in the STAX Service and Job logs

        String msg = "JobID: " + fJobNumber; 
        log(STAXJob.SERVICE_LOG, "stop", msg);
        log(STAXJob.JOB_LOG, "stop", msg);

        if (fEndTimestamp == null)
            fEndTimestamp = new STAXTimestamp();

        // Generate job completion event

        HashMap jobEndMap = new HashMap();
        jobEndMap.put("type", "job");
        jobEndMap.put("block", "main");
        jobEndMap.put("status", "end");
        jobEndMap.put("jobID", String.valueOf(fJobNumber));
        jobEndMap.put("result", fResult.toString()); 
        jobEndMap.put("jobCompletionStatus", getCompletionStatusAsString());

        generateEvent(STAXJob.STAX_JOB_EVENT, jobEndMap, true);

        // Query its job log for any messages with level "Error".
        // Save the result's marshalling context so it's available when
        // writing the job result to a file and when the RETURNRESULT
        // option is specified on a STAX EXECUTE request

        fJobLogErrorsMC = getJobLogErrors();

        // Write the job result information to files in the STAX job
        // directory

        writeJobResultsToFile();

        if (fHandle != null)
        {
            // Unregister the job's handle
            try
            {
                fHandle.unRegister();
            }
            catch (STAFException e)
            {
                /* Do Nothing */
            }
        }
    }

    /**
     * Query the STAX Job Log to get any messages with level "Error" that
     * were logged for this job.
     * 
     * @return STAFMarshallingContext Return a marshalling context for the
     * result from the LOG QUERY request whose root object is a list of
     * error messages.
     */
    private STAFMarshallingContext getJobLogErrors()
    {
        String jobLogName = fSTAX.getServiceName().toUpperCase() + "_Job_" +
            getJobNumber();

        String request = "QUERY MACHINE " + fSTAX.getLocalMachineNickname() +
            " LOGNAME " + STAFUtil.wrapData(jobLogName) +
            " LEVELMASK Error" +
            " FROM " + getStartTimestamp().getTimestampString();

        STAFResult result = submitSync("local", "LOG", request);

        // RC 4010 from the LOG QUERY request means exceeded the default
        // maximum query records

        if ((result.rc == STAFResult.Ok) || (result.rc == 4010))
            return result.resultContext;

        // Ignore the following errors from the LOG QUERY request:
        // - UnknownService (2) in case the LOG service is not registered
        // - HandleDoesNotExist (5) in case the STAX job's handle has been
        //   unregistered indicating the job has completed

        if ((result.rc != STAFResult.UnknownService) &&
            (result.rc != STAFResult.HandleDoesNotExist))
        {
            // Log an error in the JVM log

            STAXTimestamp currentTimestamp = new STAXTimestamp();

            System.out.println(
                currentTimestamp.getTimestampString() + " " + 
                "STAXJob::getJobLogErrors() failed for Job ID: " +
                getJobNumber() + "\nSTAF local LOG " + request +
                "  RC=" + result.rc + ", Result=" + result.result);
        }

        return null;
    }

    /*
     * Write the marshalled job result information to files in the STAX job
     * directory
     */ 
    private void writeJobResultsToFile()
    {
        // Create the marshalling context for the results without the testcase
        // list

        STAFMarshallingContext resultMC = new STAFMarshallingContext();
        resultMC.setMapClassDefinition(fSTAX.fResultMapClass);
        resultMC.setMapClassDefinition(
            STAXTestcaseActionFactory.fTestcaseTotalsMapClass);
        
        Map resultMap = fSTAX.fResultMapClass.createInstance();

        resultMap = addCommonFieldsToResultMap(resultMap);

        resultMC.setRootObject(resultMap);

        String marshalledResultString = resultMC.marshall();

        // Write the marshalled string for the results without the testcase
        // list to file marshalledResults.txt in directory fJobDataDir

        writeStringToFile(fSTAX.getResultFileName(fJobNumber),
                          marshalledResultString);

        // Create the marshalling context for the results with the testcase
        // list

        resultMC = new STAFMarshallingContext();
        resultMC.setMapClassDefinition(fSTAX.fDetailedResultMapClass);
        resultMC.setMapClassDefinition(
            STAXTestcaseActionFactory.fTestcaseTotalsMapClass);
        resultMC.setMapClassDefinition(
            STAXTestcaseActionFactory.fQueryTestcaseMapClass);
        
        resultMap = fSTAX.fDetailedResultMapClass.createInstance();

        resultMap = addCommonFieldsToResultMap(resultMap);

        resultMap.put("testcaseList", fTestcaseList);

        resultMC.setRootObject(resultMap);

        marshalledResultString = resultMC.marshall();

        // Write the marshalled string for the results with the testcase
        // list to file marshalledResultsLong.txt in directory fJobDataDir

        writeStringToFile(fSTAX.getDetailedResultFileName(fJobNumber),
                          marshalledResultString);
    }

    private Map addCommonFieldsToResultMap(Map inResultMap)
    {
        Map resultMap = new HashMap(inResultMap);

        if (!fJobName.equals(""))
            resultMap.put("jobName", fJobName);

        resultMap.put("startTimestamp",
                      fStartTimestamp.getTimestampString());
        resultMap.put("endTimestamp",
                      fEndTimestamp.getTimestampString());
        resultMap.put("status", getCompletionStatusAsString());
        resultMap.put("result", fResult.toString());
        resultMap.put("testcaseTotals", fTestcaseTotalsMap);
        resultMap.put("jobLogErrors", fJobLogErrorsMC);
        resultMap.put("xmlFileName", fXmlFile);
        resultMap.put("fileMachine", fXmlMachine);
        resultMap.put("function", getStartFunction());
        resultMap.put("arguments",
                      STAFUtil.maskPrivateData(getStartFuncArgs()));
        resultMap.put("scriptList", fScripts);
        resultMap.put("scriptFileList", fScriptFiles);

        if (!fScriptFileMachine.equals(""))
            resultMap.put("scriptMachine", fScriptFileMachine);

        return resultMap;
    }
        
    /**
     *  Write the contents of a string to the specified file name
     */ 
    private void writeStringToFile(String fileName, String data)
    {
        FileWriter out = null;

        try
        {
            File outFile = new File(fileName);
            out = new FileWriter(outFile);
            out.write(data);
        }
        catch (IOException e)
        {
            log(STAXJob.JVM_LOG, "Error",
                "Error writing to job results file: " + fileName + ".\n" +
                e.toString());
        }
        finally
        {
            try
            {
                if (out != null) out.close();
            }
            catch (IOException e)
            {
                // Do nothing
            }
        }
    }

    private STAX fSTAX;
    private STAXDocument fDocument;
    private STAXCallAction fDefaultCallAction = null;
    private Object fNextThreadNumberSynch = new Object();
    private int fNextThreadNumber = 1;
    private int fJobNumber = 0;
    private Object fNextProcNumberSynch = new Object();
    private int fProcNumber = 1;
    private Object fNextCmdNumberSynch = new Object();
    private int fCmdNumber = 1;
    private Object fNextProcessKeySynch = new Object();
    private int fProcessKey = 1;
    private String fJobDataDir = new String();
    private String fJobName = new String();
    private String fXmlMachine = new String();
    private String fXmlFile = new String();
    private boolean fClearlogs;
    private int fMaxSTAXThreads;
    private String fWaitTimeout = null;
    private String fStartFunction = new String();
    private String fStartFuncArgs = new String();
    private List fScripts = new ArrayList();
    private List fScriptFiles = new ArrayList();
    private String fScriptFileMachine = new String();
    private Map fThreadMap = new LinkedHashMap();
    private STAFHandle fHandle = null;
    private STAFQueueMonitor fSTAFQueueMonitor = null;
    private LinkedList fCompletionNotifiees = new LinkedList();    
    private boolean fExecuteAndHold = false;
    private String fSourceMachine = new String();  // Source == Originating
    private String fSourceHandleName = new String();
    private int fSourceHandle;
    private int fNotifyOnEnd = STAXJob.NO_NOTIFY_ONEND;
    private int fState = PENDING_STATE;
    private PyObject fResult = Py.None;
    private int fCompletionStatus = STAXJob.NORMAL_STATUS;
    private STAXTimestamp fStartTimestamp;
    private STAXTimestamp fEndTimestamp;
    private HashMap fQueueListenerMap = new HashMap();
    private HashMap fDataMap = new HashMap();
    private HashMap fRequestMap = new HashMap(); // Map of active STAF requests
    private boolean fLogTCElapsedTime;
    private boolean fLogTCNumStarts;
    private boolean fLogTCStartStop;
    private int fPythonOutput;
    private String fPythonLogLevel;
    private HashMap fCompiledPyCodeCache = new HashMap();
    private long fCompiledPyCodeCacheAdds = 0;
    private long fCompiledPyCodeCacheGets = 0;
    private List fTestcaseList = new ArrayList();
    private List fBreakpointFunctionList = new ArrayList();
    private List fBreakpointLineList = new ArrayList();
    private Map fTestcaseTotalsMap = new HashMap();
    private TreeMap fBreakpointsMap = new TreeMap();
    private Object fNextBreakpointNumberSynch = new Object();
    private int fNextBreakpointNumber = 1;
    private boolean fBreakpointFirstFunction = false;
    private boolean fBreakpointSubjobFirstFunction = false;
    private STAFMarshallingContext fJobLogErrorsMC =
        new STAFMarshallingContext();

    class STAFQueueMonitor extends Thread
    {
        STAFQueueMonitor(STAXJob job)
        {
            fJob = job;
        }
        
        /**
         *  Log an error message in the JVM log and the STAX Job log
         */
        public void logMessage(String message, Throwable t)
        {
            STAXTimestamp currentTimestamp = new STAXTimestamp();
            
            message = "STAXJob$STAFQueueMonitor.run(): " + message;

            if (t != null)
            {
                // Add the Java stack trace to the message

                StringWriter sw = new StringWriter();
                t.printStackTrace(new PrintWriter(sw));

                if (t.getMessage() != null)
                    message += "\n" + t.getMessage() + "\n" + sw.toString();
                else
                    message += "\n" + sw.toString();
            }
            
            // Truncate the message to a maximum size of 3000 (in case
            // the result from the QUEUE GET WAIT request is very large)

            if (message.length() > 3000)
                message = message.substring(0, 3000) + "...";

            // Log an error message in the STAX JVM log

            System.out.println(
                currentTimestamp.getTimestampString() +
                " Error: STAX Job ID " + fJob.getJobNumber() + ". " +
                message);

            // Log an error message in the STAX Job log

            fJob.log(STAXJob.JOB_LOG, "Error", message);
        }

        public void run()
        {
            STAFMarshallingContext mc;
            List messageList;
            int maxMessages = fJob.getSTAX().getMaxGetQueueMessages();
            String request = "GET WAIT FIRST " + maxMessages;
            STAFResult result;
            int numErrors = 0;

            // Maximum consecutive errors submitting a local QUEUE GET WAIT
            // request before we decide to exit the infinite loop
            int maxErrors = 5;

            // Process messages on the STAX job handle's queue until we get
            // a "STAF/Service/STAX/End" message or until an error occurs 5
            // consecutive times submitting a STAF local QUEUE GET request
            // (so that we don't get stuck in an infinite loop eating CPU).

            for (;;)
            {
                result = null;

                try
                {
                    // For better performance when more than 1 message is on
                    // the queue waiting to be processed, get multiple
                    // messages off the queue at a time (up to maxMessages)
                    // so that the total size of the messages hopefully won't
                    // cause an OutOfMemory problem.
                    // Note:  If an error occurs unmarshalling the list of
                    // messages, all these messages will be lost.
                    
                    result = submitSync("local", "QUEUE", request);

                    if (result == null)
                    {
                        numErrors++;
                        
                        logMessage(
                            "STAF local QUEUE " + request +
                            " returned null. This may have been caused by " +
                            "running out of memory creating the result.",
                            null);

                        if (numErrors < maxErrors)
                        {
                            continue;
                        }
                        else
                        {
                            logMessage(
                                "Exiting this thread after the QUEUE GET " +
                                "request failed " + maxErrors +
                                " consecutive times.", null);

                            return;  // Exit STAFQueueMonitor thread
                        }
                    }
                    
                    if (result.rc == STAFResult.Ok)
                    {
                        numErrors = 0;
                    }
                    else if (result.rc == STAFResult.HandleDoesNotExist)
                    {
                        // This means that the STAX job's handle has been
                        // unregistered which means that the STAX job is no
                        // longer running so we should exit this thread.
                        // We've seen this happen before this thread gets
                        // the message with type "STAF/Service/STAX/End" off
                        // the queue.

                        return;  // Exit STAFQueueMonitor thread
                    }
                    else
                    {
                        numErrors++;
                        
                        logMessage(
                            "STAF local QUEUE " + request +
                            " failed with RC=" + result.rc + ", Result=" +
                            result.result, null);

                        if (numErrors < maxErrors)
                        {
                            continue;
                        }
                        else
                        {
                            logMessage(
                                "Exiting this thread after the QUEUE GET " +
                                "request failed " + maxErrors +
                                " consecutive times", null);

                            return;  // Exit STAFQueueMonitor thread
                        }
                    }
                }
                catch (Throwable t)
                {
                    // Note: One possible reason for an exception occurring
                    // submitting a QUEUE GET WAIT request can be because
                    // an error occurred when the result was auto-unmarshalled
                    // due to invalid marshalled data in the result (though
                    // in STAF V3.3.3 a fix was made to the Java unmarshall
                    // method so that this problem should no longer occur).

                    numErrors++;

                    logMessage(
                        "Exception getting message(s) off the queue.", t);

                    if (numErrors < maxErrors)
                    {
                        continue;
                    }
                    else
                    {
                        logMessage(
                            "Exiting this thread after the QUEUE GET " +
                            "request failed " + maxErrors +
                            " consecutive times", null);

                        return;  // Exit STAFQueueMonitor thread
                    }
                }

                try
                {
                    // Unmarshall the result from a QUEUE GET request, but
                    // don't unmarshall indirect objects

                    mc = STAFMarshallingContext.unmarshall(
                        result.result,
                        STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS);
                    
                    messageList = (List)mc.getRootObject();
                }
                catch (Throwable t)
                {
                    // Log an error message and continue

                    logMessage(
                        "Exception unmarshalling queued messages. " +
                        "\nMarshalled string: " + result.result, t);
                    continue;
                }

                // Iterate through the list of messages removed from our
                // handle's queue and process each message

                Iterator iter = messageList.iterator();
                STAXSTAFMessage msg;
                    
                while (iter.hasNext())
                {
                    // Process the message

                    try
                    {
                        msg = new STAXSTAFMessage((Map)iter.next());
                    }
                    catch (Throwable t)
                    {
                        // Log an error message and continue

                        logMessage("Exception handling a queued message.", t);
                        continue;
                    }
                    
                    String type = msg.getType();
                        
                    if (type != null &&
                        type.equalsIgnoreCase("STAF/Service/STAX/End"))
                    {
                        return;  // Exit STAFQueueMonitorThread
                    }
                        
                    try
                    {
                        notifyListeners(msg);
                    }
                    catch (Throwable t)
                    {
                        // Log an error message and continue

                        logMessage(
                            "Exception notifying listeners for a message " +
                            "with type '" + type +
                            "' from handle " + msg.getHandle() +
                            " on machine " + msg.getMachine() +
                            " \nMessage: " + msg.getMessage(), t);
                        continue;
                    }
                }
            }
        }

        public void notifyListeners(STAXSTAFMessage msg)
        {
            // Perform a lookup of registered message handlers for
            // this message type and pass the STAXSTAFMessage to
            // the registered message handlers.
            
            String theType = msg.getType();
            int listenersFound = 0;

            synchronized (fQueueListenerMap) 
            {
                Iterator mapIter = fQueueListenerMap.keySet().iterator();

                while (mapIter.hasNext())
                {
                    String msgType = (String)mapIter.next();

                    if ((msgType == null) ||
                        // Messages from 3.x clients that STAX is interested
                        // in will have a type
                        (theType != null &&
                         theType.equalsIgnoreCase(msgType)) ||
                        // The following is for messages from 2.x clients
                        // which will have a null type and the message will
                        // begin with the type (which for processes will be
                        // STAF/PROCESS/END instead of STAF/Process/End
                        (theType == null && msg.getMessage().toUpperCase().
                         startsWith(msgType.toUpperCase())))
                    {
                        TreeSet listenerSet = (TreeSet)fQueueListenerMap.get(
                            msgType);

                        if (listenerSet == null) continue;

                        Iterator iter = listenerSet.iterator();

                        while (iter.hasNext())
                        {
                            STAXSTAFQueueListener listener = 
                                (STAXSTAFQueueListener)iter.next();

                            listener.handleQueueMessage(msg, fJob);
                            listenersFound++;
                        }
                    }
                }
                
                if ((listenersFound == 0) &&
                    (theType.equals("STAF/RequestComplete") ||
                     theType.equals("STAF/Process/End")))
                {
                    // Log a warning message in the job log as this could
                    // indicate a problem in the how the STAX service is
                    // handling these messages

                    try
                    {
                        fJob.log(STAXJob.JOB_LOG, "warning",
                                 "STAXJob.notifyListeners: " +
                                 "No listener found for message:\n" +
                                 STAFMarshallingContext.unmarshall(
                                     msg.getResult()).toString());
                    }
                    catch (Throwable t)
                    {
                        // Ignore
                    }
                }
            }
        }

        STAXJob fJob;

    } // end STAFQueueMonitor

}
