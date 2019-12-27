/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import com.ibm.staf.STAFUtil;
import org.python.core.Py;
import org.python.core.PyList;
import org.python.core.PyObject;
import java.util.*;

public class STAXProcessAction extends STAXActionDefaultImpl
                               implements STAXSTAFRequestCompleteListener,
                                          STAXProcessCompleteListener,
                                          STAXThreadCompleteListener,
                                          STAXTimedEventListener
{
    static boolean sDebug = false;  // Debug flag for process

    static final int INIT = 0;
    static final int WAIT_REQUEST = 1;
    static final int REQUEST_ERROR = 2;
    static final int REQUEST_TIMEOUT = 3;
    static final int RUNNING = 4;
    static final int COMPLETE = 5;

    static final String INIT_STRING = "INIT";
    static final String WAIT_REQUEST_STRING = "WAIT_REQUEST";
    static final String REQUEST_ERROR_STRING = "REQUEST_ERROR";
    static final String REQUEST_TIMEOUT_STRING = "REQUEST_TIMEOUT";
    static final String RUNNING_STRING = "RUNNING";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXProcessAction()
    { /* Do Nothing */ }

    public STAXProcessActionFactory getActionFactory() { return fFactory; }
    public void setActionFactory(STAXProcessActionFactory factory) 
    { 
        fFactory = factory; 
    }

    public String getLocation() { return fLocation; }
    public void   setLocation(String location) 
    { 
        fLocation = location; 
        fUnevalLocation = location;
   }

    public String getCommand() { return fCommand; }
    public void   setCommand(String command) 
    { 
        fCommand = command; 
        fUnevalCommand = command;
    }
    
    public String getName() { return fName; }
    public void   setName(String name)
    { 
        fName = name; 
        fUnevalName = name;
    }
    
    public String getWorkload() { return fWorkload; }
    public void   setWorkload(String workload, String workloadIf) 
    { 
        fWorkload = workload; 
        fUnevalWorkload = workload;
        fWorkloadIf = workloadIf;
    }
    
    public String getTitle() { return fTitle; }
    public void   setTitle(String title, String titleIf) 
    { 
        fTitle = title; 
        fUnevalTitle = title;
        fTitleIf = titleIf;
    }
    
    public String getParms() { return fParms; }
    public void   setParms(String parms, String parmsIf) 
    { 
        fParms = parms;
        fUnevalParms = parms;
        fParmsIf = parmsIf;
    }
    
    public String getWorkdir() { return fWorkdir; }
    public void   setWorkdir(String workdir, String workdirIf) 
    { 
        fWorkdir = workdir;
        fUnevalWorkdir = workdir;
        fWorkdirIf = workdirIf;
    }
    
    public Vector<String> getVars() { return fVars; }
    public void   setVars(String vars, String varsIf) 
    { 
        fUnevalVars.add(vars);
        fVarsIf.add(varsIf);
    }
    
    public Vector<String> getEnvs() { return fEnvs; }
    public void   setEnvs(String envs, String envsIf) 
    { 
        fUnevalEnvs.add(envs);
        fEnvsIf.add(envsIf);
    }
    
    public String getUseprocessvars() { return fUseprocessvars; }
    public void   setUseprocessvars(String useprocessvars,
                                    String useprocessvarsIf) 
    { 
        fUseprocessvars = useprocessvars;
        fUnevalUseprocessvars = useprocessvars; 
        fUseprocessvarsIf = useprocessvarsIf;
    }
        
    public String getStopusing() { return fStopusing; }
    public void   setStopusing(String stopusing, String stopusingIf) 
    { 
        fStopusing = stopusing;
        fUnevalStopusing = stopusing; 
        fStopusingIf = stopusingIf;
    }
    
    public String getConsole() { return fConsole; }
    public void   setConsole(String console, String consoleIf) 
    { 
        fConsole = console; 
        fUnevalConsole = console;
        fConsoleIf = consoleIf;
    }

    public String getFocus() { return fFocus; }
    public void   setFocus(String focus, String focusIf) 
    { 
        fFocus = focus; 
        fUnevalFocus = focus;
        fFocusIf = focusIf;
    }
        
    public String getUsername() { return fUsername; }
    public void   setUsername(String username, String usernameIf) 
    { 
        fUsername = username;
        fUnevalUsername = username; 
        fUsernameIf = usernameIf;
    }
    
    public String getPassword() { return fPassword; }
    public void   setPassword(String password, String passwordIf) 
    { 
        fPassword = password; 
        fUnevalPassword = password;
        fPasswordIf = passwordIf;
    }
    
    public String getDisabledauth() { return fDisabledauth; }
    public void   setDisabledauth(String disabledauth, String disabledauthIf) 
    { 
        fDisabledauth = disabledauth; 
        fUnevalDisabledauth = disabledauth;
        fDisabledauthIf = disabledauthIf;
    }
    
    public String getStatichandlename() { return fStatichandlename; }
    public void   setStatichandlename(String statichandlename, 
                                      String statichandlenameIf) 
    { 
        fStatichandlename = statichandlename; 
        fUnevalStatichandlename = statichandlename;
        fStatichandlenameIf = statichandlenameIf;
    }
        
    public String getStdin() { return fStdin; }
    public void   setStdin(String stdin, String stdinIf) 
    { 
        fStdin = stdin; 
        fUnevalStdin = stdin;
        fStdinIf = stdinIf;
    }
    
    public String getStdout() { return fStdout; }
    public String getStdoutMode() { return fStdoutMode; }
    public void   setStdout(String stdout, String stdoutMode, String stdoutIf) 
    { 
        fStdout = stdout; 
        fUnevalStdout = stdout;
        fStdoutIf = stdoutIf;
        fStdoutMode = stdoutMode;
        fUnevalStdoutMode = stdoutMode;
    }
        
    public String getStderr() { return fStderr; }
    public String getStderrMode() { return fStderrMode; }

    public void   setStderr(String stderr, String stderrMode, String stderrIf) 
    { 
        fStderr = stderr;
        fUnevalStderr = stderr; 
        fStderrIf = stderrIf;
        fStderrMode = stderrMode;
        fUnevalStderrMode = stderrMode;
    }

    public String getReturnStdout() { return fReturnStdout; }
    public void   setReturnStdout(String returnStdout, String returnStdoutIf) 
    {
        fReturnStdout = returnStdout;
        fUnevalReturnStdout = returnStdout;
        fReturnStdoutIf = returnStdoutIf;
    }
    
    public String getReturnStderr() { return fReturnStderr; }
    public void   setReturnStderr(String returnStderr, String returnStderrIf) 
    {
        fReturnStderr = returnStderr;
        fUnevalReturnStderr = returnStderr;
        fReturnStderrIf = returnStderrIf;
    }

    public Vector<String> getReturnFiles() { return fReturnFiles; }
    public void   setReturnFiles(String returnFile, String returnFileIf) 
    { 
        fUnevalReturnFiles.add(returnFile);
        fReturnFilesIf.add(returnFileIf);
    }
    
    public String getOther() { return fOther; }
    public void   setOther(String other, String otherIf) 
    { 
        fOther = other; 
        fUnevalOther = other;
        fOtherIf = otherIf;
    }
    
    public STAXAction getProcessAction() { return fProcessAction; }
    public void   setProcessAction(STAXAction processAction, 
        String processActionIf) 
    { 
        fProcessAction = processAction;
        fProcessActionIf = processActionIf;
        fUnevalProcessActionIf = processActionIf;
    }
    
    public String getCommandMode() { return fCommandMode; }
    public void   setCommandMode(String mode) 
    {
        fCommandMode = mode;
        fUnevalCommandMode = mode;
    }

    public String getCommandShell() { return fCommandShell; }
    public void   setCommandShell(String shell) 
    {
        fCommandShell = shell;
        fUnevalCommandShell = shell;
    }

    public STAXThread getThread() { return fThread; }

    public String getCurrentBlockName() { return fCurrentBlockName; }

    public STAXTimestamp getStartTimestamp() { return fStartTimestamp; }

    public int getProcessHandle() { return fProcessHandle; }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case WAIT_REQUEST:
                return WAIT_REQUEST_STRING;
            case REQUEST_ERROR:
                return REQUEST_ERROR_STRING;
            case REQUEST_TIMEOUT:
                return REQUEST_TIMEOUT_STRING;
            case RUNNING:
                String stateString = RUNNING_STRING;
                
                if (fIsProcessRunning && !fIsProcessActionRunning)
                    stateString += " (process)";
                else if (fIsProcessRunning && fIsProcessActionRunning)
                    stateString += " (process, process-action)";
                else if (!fIsProcessRunning && fIsProcessActionRunning)
                    stateString += " (process-action)";

                return stateString;
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
        StringBuffer result = new StringBuffer();

        result.append("State:").append(getStateAsString()).append(";Name:").
            append(fName).append(";Location:").append(fLocation).
            append(";Command:").append(fCommand);
        if (!fCommandMode.equals(""))
            result.append(";CommandMode:").append(fCommandMode);
        if (!fCommandShell.equals(""))
            result.append(";CommandShell:").append(fCommandShell);
        if (!fWorkload.equals(""))
            result.append(";Workload:").append(fWorkload);
        if (!fTitle.equals(""))
            result.append(";Title:").append(fTitle);
        if (!fParms.equals(""))
            result.append(";Parms:").append(fParms);
        if (!fWorkdir.equals(""))
            result.append(";Workdir:").append(fWorkdir);

        for (int i = 0; i < fVars.size(); i++)
        {
            result.append(";Var").append(i + 1).append(":").append(
                fVars.elementAt(i));
        }

        for (int i = 0; i < fEnvs.size(); i++)
        {
            result.append(";Env").append(i + 1).append(":").append(
                fEnvs.elementAt(i));
        }

        if (!fUseprocessvars.equals(""))
            result.append(";Useprocessvars:").append(fUseprocessvars);
        if (!fStopusing.equals(""))
            result.append(";Stopusing:").append(fStopusing);
        if (!fConsole.equals(""))
            result.append(";Console:").append(fConsole);
        if (!fFocus.equals(""))
            result.append(";Focus:").append(fFocus);
        if (!fUsername.equals(""))
            result.append(";Username:").append(fUsername);
        if (!fPassword.equals(""))
            result.append(";Password:*******");
        if (!fDisabledauth.equals(""))
            result.append(";Disabledauth:").append(fDisabledauth);
        if (!fStatichandlename.equals(""))
            result.append(";Statichandlename:").append(fStatichandlename);
        if (!fStdin.equals(""))
            result.append(";Stdin:").append(fStdin);
        if (!fStdout.equals(""))
            result.append(";Stdout:").append(fStdout);
        if (!fStdoutMode.equals(""))
            result.append(";StdoutMode:").append(fStdoutMode);
        if (!fStderr.equals(""))
            result.append(";Stderr:").append(fStderr);
        if (!fStderrMode.equals(""))
            result.append(";StderrMode:").append(fStderrMode);
        if (!fReturnStdout.equals(""))
            result.append(";ReturnStdout:").append(fReturnStdout);
        if (!fReturnStderr.equals(""))
            result.append(";ReturnStderr:").append(fReturnStderr);
        
        for (int i = 0; i < fReturnFiles.size(); i++)
        {
            result.append(";ReturnFile").append(i + 1).append(":").
                append(fReturnFiles.elementAt(i));
        }

        if (!fOther.equals(""))
            result.append(";Other:").append(fOther);

        result.append(";RequestNumber:").append(fRequestNumber).
            append(";RequestRC:").append(fRequestRC).
            append(";Request:").append(fRequest).
            append(";RequestResult:").append(fRequestResult).
            append(";ProcessHandle:").append(fProcessHandle).
            append(";ProcessRC:").append(fProcessRC).
            append(";ProcessTimestamp:").append(fProcessTimestamp).
            append(";StartTimestamp:").append(fStartTimestamp).
            append(";BlockName:").append(fCurrentBlockName);

        return result.toString();
    }

    public void execute(STAXThread thread)
    {
        synchronized (this)
        {
            if (fState == INIT)
            {
                fThread = thread;

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " STAXProcessAction::execute(): fState=INIT");
                }
 
                // Evaluate the process element and attribute values and
                // generate a process start request and assign to fRequest.

                if (generateProcessStartRequest() != 0)
                {
                    if (sDebug)
                    {
                        STAXTimestamp currentTimestamp = new STAXTimestamp();

                        System.out.println(
                            currentTimestamp.getTimestampString() + " Debug " +
                            "JobID=" + fThread.getJob().getJobNumber() +
                            " ThreadID=" + fThread.getThreadNumber() +
                            " key=" + fLocation +
                            " STAXProcessAction::execute(): " +
                            "generateProcessStartRequest() failed - return");
                    }

                    return;
                }

                // Set current block name
                try
                {
                    fCurrentBlockName = fThread.pyStringEval(
                        "STAXCurrentBlock");
                }
                catch (STAXPythonEvaluationException e)
                {
                    fCurrentBlockName = "";  // Shouldn't happen
                }

                // Set to the current date and time.
                fStartTimestamp = new STAXTimestamp();
         
                fState = WAIT_REQUEST;
                
                fProcessKey = new Integer(
                    fThread.getJob().getNextProcessKey()).toString();
                
                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation +
                        " STAXProcessAction::execute(): " +
                        "Submitting PROCESS START NOTIFY ONEND KEY " +
                        fProcessKey + " " + fRequest);
                }

                // Submit PROCESS START STAF Command

                STAFResult submitResult = fThread.getJob().submitAsync(
                    fLocation, "process", "START NOTIFY ONEND KEY " +
                    fProcessKey + " " + fRequest, this);
            
                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation +
                        " STAXProcessAction::execute(): " +
                        "PROCESS START request submit RC=" + submitResult.rc +
                        ", Result=" + submitResult.result);
                }
                    
                if (submitResult.rc == 0 )
                {
                    fRequestNumber = (new Integer(submitResult.result)).
                                                  intValue();
                }
                
                if (fRequestNumber == 0)
                {
                    // Request failed - Raise a STAXProcessStartError signal
                    fState = COMPLETE;
                    fThread.popAction();
                    
                    setPythonVariablesWhenFail(
                        "Process failed to start.  Raised a " +
                        "STAXProcessStartError signal.");
                    
                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "The process failed to start, Request#: 0, RC: " +
                        submitResult.rc + ", STAFResult: " +
                        submitResult.result));

                    fThread.setSignalMsgVar(
                        "STAXProcessStartErrorMsg",
                        STAXUtil.formatErrorMessage(this));

                    fThread.raiseSignal("STAXProcessStartError");
                    
                    return;        
                }
                
                if (false && sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation +
                        " STAXProcessAction::execute(): " +
                        "Add timed event - timeout=" +
                        fThread.getJob().getSTAX().getProcessTimeout());
                }
            
                // Add a TimedEvent to the queue and return 

                fTimedEvent = new STAXTimedEvent(System.currentTimeMillis() + 
                    fThread.getJob().getSTAX().getProcessTimeout(), this);

                fThread.getJob().getSTAX().getTimedEventQueue().addTimedEvent(
                    fTimedEvent);

                fThread.addCondition(fHoldCondition);

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation +
                        " STAXProcessAction::execute(): " +
                        "Added hold condition: State=" + getStateAsString());
                }
            }
            else if (fState == REQUEST_ERROR)
            {
                fState = COMPLETE;
                fThread.popAction();

                fThread.pySetVar("RC", new Integer(fRequestRC));
                fThread.pySetVar("STAFResult", fRequestResult);
                fThread.pySetVar("STAXResult", Py.None);

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::execute(): " +
                        "fState=REQUEST_ERROR - " +
                        "Raise STAXProcessStartError signal");
                }

                setElementInfo(new STAXElementInfo(
                    getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    "The process failed to start, RC: " +
                    fRequestRC + ", STAFResult: " + fRequestResult));

                fThread.setSignalMsgVar(
                    "STAXProcessStartErrorMsg",
                    STAXUtil.formatErrorMessage(this));

                fThread.raiseSignal("STAXProcessStartError");
            }
            else if (fState == REQUEST_TIMEOUT)
            {
                // The process request did not start within the timeout,
                // PROCESSTIMEOUT, (requestComplete was not called) so
                // generate a STAXProcessStartTimeout signal

                fState = COMPLETE;
                fThread.popAction();
         
                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::execute(): " +
                        "fState=REQUEST_TIMEOUT - Raise " +
                        "STAXProcessStartTimeout signal");
                }

                String timeoutMsg = "Process did not start within timeout " +
                    "value " + fThread.getJob().getSTAX().getProcessTimeout();

                // Set RC, STAFResult, STAXResult to indicate a timeout signal

                fRequestRC = STAFResult.Timeout;
                fThread.pySetVar("RC", new Integer(STAFResult.Timeout));
                fThread.pySetVar("STAFResult", timeoutMsg +
                                 ".  Raised a STAXProcessStartTimeout signal");
                fThread.pySetVar("STAXResult", Py.None);

                setElementInfo(new STAXElementInfo(
                    getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    timeoutMsg));

                fThread.setSignalMsgVar(
                    "STAXProcessStartTimeoutMsg",
                    STAXUtil.formatErrorMessage(this));

                fThread.raiseSignal("STAXProcessStartTimeout");
            }
            else if ((fState == RUNNING) &&
                     !fIsProcessRunning && !fIsProcessActionRunning)
            {
                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::execute(): fState=" +
                        getStateAsString());
                }

                fState = COMPLETE;
                fThread.popAction();

                // Set variables when process and process-action have completed

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::execute(): fState=COMPLETE, " +
                        "Set RC=" + fProcessRC +
                        ", STAFResult=" + fProcessSTAFResult);
                        // ", STAXResult=" + fProcessSTAXResult.length());
                }

                fThread.pySetVar("RC", new Long(fProcessRC));
                fThread.pySetVar("STAFResult", fProcessSTAFResult);

                try
                {
                    fThread.pySetVar("STAXResult", fProcessSTAXResult);
                }
                catch (Exception ex)
                {
                    // This should never happen
                    ex.printStackTrace();

                    fThread.pySetVar("STAXResult", Py.None);
                    fProcessSTAXResult = Py.None;

                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "Error setting STAXResult to resultList.\n" +
                        "fileList: " + fProcessSTAXResult.toString() +
                        "\n\n" + ex.toString()));

                    fThread.setSignalMsgVar(
                        "STAXPythonEvalMsg",
                        STAXUtil.formatErrorMessage(this));

                    fThread.raiseSignal("STAXPythonEvaluationError");
                }
            }
            else if (fState == COMPLETE)
            {
                // Note: We shouldn't be called in this state.

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::execute(): fState=COMPLETE " +
                        " when shouldn't be called in this state");
                }

                fThread.popAction();
            }
            else if (sDebug)
            {
                // Should not get here
                
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::execute(): fState=" +
                    getStateAsString() + " is an unexpected state");
            }
        }  // End synchronized (this)
    }


    // Note that this entire method is synchronized
    public synchronized void handleCondition(STAXThread thread,
                                             STAXCondition cond)
    {
        synchronized (this)
        {
            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::handleCondition():" +
                    " fState=" + getStateAsString());
            }
                
            // Check if the process is running

            if ((fState == RUNNING) && fIsProcessRunning)
            {
                fIsProcessRunning = false;

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::handleCondition():" +
                        " removeHoldCondition and stop handle");
                }
            
                // Stop the process and free its termination info
                // 1) Want to do this asynchronously in a single request for
                //    performance reasons since we're in synchronized code
                // 2) Had to add a short delay (1 second) to allow time for
                //    the process to be completely stopped before submitting
                //    the FREE request 

                String stopFreeProcessCommand =
                    "STAF local PROCESS STOP HANDLE " + fProcessHandle +
                    " {STAF/Config/Sep/Command} " +
                    "STAF local DELAY DELAY 1000" +
                    " {STAF/Config/Sep/Command} " +
                    "STAF local PROCESS FREE HANDLE " + fProcessHandle;

                thread.getJob().submitAsyncForget(
                    fLocation, "PROCESS", "START SHELL COMMAND " +
                    STAFUtil.wrapData(stopFreeProcessCommand) + " WAIT");

                // Generate a stop process event

                generateProcessStopEvent();

                // Remove the process from the processRequestMap 

                String key = (fLocation + ":" + fProcessHandle).toLowerCase();
                TreeMap processes = (TreeMap)thread.getJob().getData(
                    "processRequestMap");
                
                if (processes != null)
                {
                    synchronized (processes)
                    {
                        processes.remove(key);
                    }
                }
            }

            // Terminate the process-action task if running

            synchronized (fThreadMap)
            {
                if (!fThreadMap.isEmpty())
                {
                    if (sDebug)
                    {
                        STAXTimestamp currentTimestamp = new STAXTimestamp();

                        System.out.println(
                            currentTimestamp.getTimestampString() + " Debug " +
                            "JobID=" + fThread.getJob().getJobNumber() +
                            " ThreadID=" + fThread.getThreadNumber() +
                            " key=" + fLocation + ":" + fProcessHandle +
                            " STAXProcessAction::handleCondition():" +
                            " Add HardHoldThread condition and terminate " +
                            "process-action thread");
                    }

                    // Add a hard hold so we can wait until our children
                    // terminate.  This is removed by threadComplete().

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
            
            // Remove the HoldThread condition from the thread's condition
            // stack (if it exists)

            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::handleCondition(): Set fState=" +
                    "COMPLETE and remove HoldThread condition: " +
                    thread.getConditionStack());
            }

            thread.removeCondition(fHoldCondition);

            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::handleCondition(): " +
                    "Removed HoldThread condition");
            }

            thread.popAction();
        }
    }

    public STAXAction cloneAction()
    {
        STAXProcessAction clone = new STAXProcessAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalLocation = fUnevalLocation;
        clone.fUnevalCommand = fUnevalCommand;
        clone.fUnevalName = fUnevalName;
        clone.fUnevalWorkload = fUnevalWorkload;
        clone.fUnevalWorkdir = fUnevalWorkdir;
        clone.fUnevalTitle = fUnevalTitle;
        clone.fUnevalParms = fUnevalParms;
        clone.fUnevalWorkdir = fUnevalWorkdir;
        clone.fUnevalVars = fUnevalVars;
        clone.fUnevalEnvs = fUnevalEnvs;
        clone.fUnevalUseprocessvars = fUnevalUseprocessvars;
        clone.fUnevalStopusing = fUnevalStopusing;
        clone.fUnevalConsole = fUnevalConsole;
        clone.fUnevalFocus = fUnevalFocus;
        clone.fUnevalUsername = fUnevalUsername;
        clone.fUnevalPassword = fUnevalPassword;
        clone.fUnevalDisabledauth = fUnevalDisabledauth;
        clone.fUnevalStatichandlename = fUnevalStatichandlename;
        clone.fUnevalStdin = fUnevalStdin;
        clone.fUnevalStdout = fUnevalStdout;
        clone.fUnevalStdoutMode = fUnevalStdoutMode;
        clone.fUnevalStderr = fUnevalStderr;
        clone.fUnevalStderrMode = fUnevalStderrMode;
        clone.fUnevalReturnStdout = fUnevalReturnStdout;
        clone.fUnevalReturnStderr = fUnevalReturnStderr;
        clone.fUnevalReturnFiles = fUnevalReturnFiles;
        clone.fUnevalOther = fUnevalOther;
        clone.fUnevalCommandMode = fUnevalCommandMode;
        clone.fUnevalCommandShell = fUnevalCommandShell;

        clone.fCurrentBlockName = fCurrentBlockName;
        clone.fFactory = fFactory;
        clone.fLocation = fLocation;
        clone.fCommand = fCommand;
        clone.fName = fName;
        clone.fWorkload = fWorkload;
        clone.fWorkloadIf = fWorkloadIf;
        clone.fWorkdir = fWorkdir;
        clone.fWorkdirIf = fWorkdirIf;
        clone.fTitle = fTitle;
        clone.fTitleIf = fTitleIf;
        clone.fParms = fParms;
        clone.fParmsIf = fParmsIf;
        clone.fVars = new Vector<String>();
        clone.fVarsIf = fVarsIf;
        clone.fEnvs = new Vector<String>();
        clone.fEnvsIf = fEnvsIf;
        clone.fUseprocessvars = fUseprocessvars;
        clone.fUseprocessvarsIf = fUseprocessvarsIf;
        clone.fStopusing = fStopusing;
        clone.fStopusingIf = fStopusingIf;
        clone.fConsole = fConsole;
        clone.fConsoleIf = fConsoleIf;
        clone.fFocus = fFocus;
        clone.fFocusIf = fFocusIf;
        clone.fUsername = fUsername;
        clone.fUsernameIf = fUsernameIf;
        clone.fPassword = fPassword;
        clone.fPasswordIf = fPasswordIf;
        clone.fDisabledauth = fDisabledauth;
        clone.fDisabledauthIf = fDisabledauthIf;
        clone.fStatichandlename = fStatichandlename;
        clone.fStatichandlenameIf = fStatichandlenameIf;
        clone.fStdin = fStdin;
        clone.fStdinIf = fStdinIf;
        clone.fStdout = fStdout;
        clone.fStdoutIf = fStdoutIf;
        clone.fStdoutMode = fStdoutMode;
        clone.fStderr = fStderr;
        clone.fStderrIf = fStderrIf;
        clone.fStderrMode = fStderrMode;
        clone.fReturnStdout = fReturnStdout;
        clone.fReturnStdoutIf = fReturnStdoutIf;
        clone.fReturnStderr = fReturnStderr;
        clone.fReturnStderrIf = fReturnStderrIf;
        clone.fReturnFiles = new Vector<String>();
        clone.fReturnFilesIf = fReturnFilesIf;
        clone.fOther = fOther;
        clone.fOtherIf = fOtherIf;
        clone.fProcessAction = fProcessAction;
        clone.fProcessActionIf = fProcessActionIf;
        clone.fCommandMode = fCommandMode;
        clone.fCommandShell = fCommandShell;
       
        return clone;
    }

    // STAXTimedEventListener method

    // Note that this entire method is synchronized
    public synchronized void timedEventOccurred(STAXTimedEvent timedEvent)
    {
        if (sDebug)
        {
            STAXTimestamp currentTimestamp = new STAXTimestamp();

            System.out.println(
                currentTimestamp.getTimestampString() + " Debug " +
                "JobID=" + fThread.getJob().getJobNumber() +
                " ThreadID=" + fThread.getThreadNumber() +
                " key=" + fLocation + ":" + fProcessHandle +
                " STAXProcessAction::timedEventOccurred(): fState=" +
                getStateAsString());
        }
            
        if (fState == WAIT_REQUEST)
        {
            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::timedEventOccurred(): Set " +
                    "fState=REQUEST_TIMEOUT, remove holdCondition, " +
                    "and schedule thread");
            }

            fState = REQUEST_TIMEOUT;
            fThread.removeCondition(fHoldCondition);
            fThread.schedule();
        }
    }

    // STAXSTAFRequestCompleteListener method

    // Note that this entire method is synchronized
    public synchronized void requestComplete(int requestNumber,
        STAFResult result)
    {
        if (sDebug)
        {
            STAXTimestamp currentTimestamp = new STAXTimestamp();

            System.out.println(
                currentTimestamp.getTimestampString() + " Debug " +
                "JobID=" + fThread.getJob().getJobNumber() +
                " ThreadID=" + fThread.getThreadNumber() +
                " key=" + fLocation +
                " STAXProcessAction::requestComplete(): fState=" +
                getStateAsString() + ", RC=" + result.rc +
                ", Result=" + result.result);
        }
        // Already COMPLETE, so something like a STAXProcessRequestTimeout
        // signal could have already been raised, so just return

        if (fState == COMPLETE) return;

        // Remove request timeout event 

        fThread.getJob().getSTAX().getTimedEventQueue().removeTimedEvent(
            fTimedEvent);
        
        if (result.rc == 0)
        {
            fState = RUNNING;

            try
            {
                // XXX: Why do we store the handle in "int" form instead of
                //      just storing the handle in "string" form

                fProcessHandle = Integer.parseInt(result.result);

                // Register for Process Completion

                String key = new String();
                String machine = fLocation;
                
                if (fUsingProcessKeyOption)
                {
                    key = fProcessKey;
                }
                else
                {
                    key = (machine + "/" + fProcessHandle).toLowerCase();
                }

                @SuppressWarnings("unchecked")
                HashMap<String, Object> processMap =
                    (HashMap<String, Object>)fThread.getJob().getData(
                        "processMap");

                if (processMap != null)
                {
                    synchronized (processMap)
                    {
                        // If we have a race condition, we may have already
                        // received the "process end" message
                            
                        if (processMap.containsKey(key))
                        {
                            if (sDebug)
                            {
                                STAXTimestamp currentTimestamp =
                                    new STAXTimestamp();

                                System.out.println(
                                    currentTimestamp.getTimestampString() +
                                    " Debug JobID=" +
                                    fThread.getJob().getJobNumber() +
                                    " ThreadID=" + fThread.getThreadNumber() +
                                    " key=" + fLocation + ":" + fProcessHandle +
                                    " STAXProcessAction::requestComplete(): " + 
                                    "RACE CONDITION - remove from processMap" +
                                    ", key=" + key);
                            }

                            STAXSTAFMessage completionMessage = 
                                (STAXSTAFMessage)processMap.get(key);
                                
                            processMap.remove(key);
                            
                            // If a process-action is specified, start its action before
                            // calling processComplete()

                            if ((fProcessAction != null) && fProcessActionFlag)
                            {
                                // Start action specified by <process-action> element

                                synchronized (fThreadMap)
                                {
                                    fIsProcessActionRunning = true;

                                    fThread.pySetVar("STAXProcessHandle",
                                                     String.valueOf(fProcessHandle));

                                    STAXThread childThread = fThread.createChildThread();

                                    fThreadMap.put(childThread.getThreadNumber(), childThread);

                                    STAXAction childThreadAction = fProcessAction;

                                    childThread.addCompletionNotifiee(this);
                                    childThread.pushAction(
                                        childThreadAction.cloneAction());
                                    childThread.schedule();
                                }
                            }

                            processComplete(machine, fProcessHandle,
                                completionMessage.getProcessRC(), 
                                completionMessage.getProcessResultAsList(),
                                completionMessage.getProcessTimestamp());

                            return;
                        }
                        else
                        {
                            fIsProcessRunning = true;

                            if (false && sDebug)
                            {
                                STAXTimestamp currentTimestamp =
                                    new STAXTimestamp();

                                System.out.println(
                                    currentTimestamp.getTimestampString() +
                                    " Debug JobID=" +
                                    fThread.getJob().getJobNumber() +
                                    " ThreadID=" + fThread.getThreadNumber() +
                                    " key=" + fLocation + ":" + fProcessHandle +
                                    " STAXProcessAction::requestComplete(): " +
                                    "Not race condition: add to processMap, " +
                                    "key=" + key);
                            }
                        
                            processMap.put(key, this);
                        }
                    }
                }
            }
            catch (NumberFormatException e)
            {
                // XXX: Do something.  Maybe set fState = ERROR?
            }

            if (fIsProcessRunning)
            {
                // Add the process to the processRequestMap

                String key = (fLocation + ":" + fProcessHandle).toLowerCase();

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::requestComplete(): " +
                        "Add to processRequestMap");
                }

                @SuppressWarnings("unchecked")
                TreeMap<String, STAXProcessAction> processes =
                    (TreeMap<String, STAXProcessAction>)fThread.getJob().
                    getData("processRequestMap");

                if (processes != null)
                {
                    synchronized (processes)
                    {
                        processes.put(key, this);
                    }
                }

                // Generate a process start event
                generateProcessStartEvent();
            }
        
            if ((fProcessAction != null) && fProcessActionFlag)
            {
                // Start action specified by <process-action> element

                synchronized (fThreadMap)
                {
                    fIsProcessActionRunning = true;

                    fThread.pySetVar("STAXProcessHandle",
                                     String.valueOf(fProcessHandle));
                      
                    STAXThread childThread = fThread.createChildThread();
                     
                    fThreadMap.put(childThread.getThreadNumber(), childThread);
                    
                    STAXAction childThreadAction = fProcessAction;
                   
                    childThread.addCompletionNotifiee(this);
                    childThread.pushAction(
                        childThreadAction.cloneAction());
                    childThread.schedule();
                }
            }
        }
        else
        {
            if ((result.rc == STAFResult.InvalidRequestString) &&
                fUsingProcessKeyOption)
            {
                // The target machine may not support the KEY option,
                // so re-submit the request without the KEY option.
                // If the second submit fails, then the InvalidRequestString
                // is not due to the KEY option not being supported.

                fUsingProcessKeyOption = false;
                
                // Submit PROCESS START STAF Command

                STAFResult submitResult = fThread.getJob().submitAsync(
                    fLocation, "PROCESS", "START NOTIFY ONEND " +
                    fRequest, this);
                    
                if (submitResult.rc == 0 )
                {
                    fRequestNumber = (new Integer(submitResult.result)).
                                                  intValue();
                }
                
                if (fRequestNumber == 0)
                {
                    // Request failed - Raise a STAXProcessStartError signal
                    fState = COMPLETE;
                    fThread.popAction();

                    fThread.pySetVar("RC", new Integer(submitResult.rc));
                    fThread.pySetVar("STAFResult", submitResult.result);
                    fThread.pySetVar("STAXResult", Py.None);

                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "The process failed to start, Request#: 0, RC: " +
                        submitResult.rc + ", STAFResult: " +
                        submitResult.result));

                    fThread.setSignalMsgVar(
                        "STAXProcessStartErrorMsg",
                        STAXUtil.formatErrorMessage(this));
                    
                    fThread.raiseSignal("STAXProcessStartError");
                    
                    fThread.schedule();

                    return;        
                }
            }
            else
            {
                fRequestRC = result.rc;
                fRequestResult = result.result;

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::requestComplete(): " +
                        "set fState=REQUEST_ERROR and remove holdCondition " +
                        "and schedule thread");
                }

                fState = REQUEST_ERROR;
                fThread.removeCondition(fHoldCondition);
                fThread.schedule();
            }
        }
    }

    public synchronized void processComplete(String machine, int handle, 
        long rc, List fileList, String timestamp)
    {
        if (sDebug)
        {
            STAXTimestamp currentTimestamp = new STAXTimestamp();

            System.out.println(
                currentTimestamp.getTimestampString() + " Debug " +
                "JobID=" + fThread.getJob().getJobNumber() +
                " ThreadID=" + fThread.getThreadNumber() +
                " key=" + fLocation + ":" + fProcessHandle +
                " STAXProcessAction::processComplete(): " +
                "RC=" + rc + " fState=" + getStateAsString());
        }

        fThread.getJob().submitAsyncForget(
            fLocation, "PROCESS", "FREE HANDLE " + fProcessHandle);

        if (fState == COMPLETE) return;

        if (fIsProcessRunning)
        {
            fIsProcessRunning = false;

            // We only generate the stop event if we aren't COMPLETE.  If we
            // were terminated before the process finished, then we generated
            // the stop event at that time.
            generateProcessStopEvent();

            // Remove the process from the processRequestMap 

            String key = (fLocation + ":" + fProcessHandle).toLowerCase();

            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::processComplete():" +
                    " Remove from processRequestMap");
            }

            TreeMap processes = (TreeMap)fThread.getJob().getData(
                "processRequestMap");

            if (processes != null)
            {
                synchronized (processes)
                {
                    processes.remove(key);
                }
            }
        }

        // Save process RC, STAFResult, and STAXResult values to set (in
        // the execute method) when both the process and process-action
        // have completed.

        fProcessRC = rc;
        fProcessSTAFResult = Py.None;

        PyList resultList = getResultList(fileList);

        if (resultList != null)
        {          
            fProcessSTAXResult = resultList;
        }
           
        fProcessTimestamp = timestamp;

        // If the process-action (if any) has completed, remove the HoldThread
        // condition and schedule the thread to run

        if (fThreadMap.isEmpty())
        {
            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::processComplete():" +
                    " Remove HoldThread condition: " +
                    fThread.getConditionStack());
            }

            fThread.removeCondition(fHoldCondition);
            fThread.schedule();
        }
    }

    public synchronized void threadComplete(STAXThread thread, int endCode)
    {
        synchronized(fThreadMap)
        {
            if (sDebug)
            {
                STAXTimestamp currentTimestamp = new STAXTimestamp();

                System.out.println(
                    currentTimestamp.getTimestampString() + " Debug " +
                    "JobID=" + fThread.getJob().getJobNumber() +
                    " ThreadID=" + fThread.getThreadNumber() +
                    " key=" + fLocation + ":" + fProcessHandle +
                    " STAXProcessAction::threadComplete():" + 
                    " fState=" + getStateAsString());
            }

            fThreadMap.remove(thread.getThreadNumberAsInteger());

            if (fThreadMap.isEmpty())
            {
                fIsProcessActionRunning = false;

                // Note that thread.getParentThread() is the thread we are
                // running on

                if (sDebug)
                {
                    STAXTimestamp currentTimestamp = new STAXTimestamp();

                    System.out.println(
                        currentTimestamp.getTimestampString() + " Debug " +
                        "JobID=" + fThread.getJob().getJobNumber() +
                        " ThreadID=" + fThread.getThreadNumber() +
                        " key=" + fLocation + ":" + fProcessHandle +
                        " STAXProcessAction::threadComplete():" +
                        " Remove HardHoldThread condition: " +
                        thread.getParentThread().getConditionStack());
                }

                thread.getParentThread().removeCondition(fHardHoldCondition);
                
                if ((fState == RUNNING) && !fIsProcessRunning)
                {
                    if (sDebug)
                    {
                        STAXTimestamp currentTimestamp = new STAXTimestamp();

                        System.out.println(
                            currentTimestamp.getTimestampString() + " Debug " +
                            "JobID=" + fThread.getJob().getJobNumber() +
                            " ThreadID=" + fThread.getThreadNumber() +
                            " key=" + fLocation + ":" + fProcessHandle +
                            " STAXProcessAction::threadComplete():" +
                            " Remove HoldThread condition: " +
                            thread.getParentThread().getConditionStack());
                    }

                    thread.getParentThread().removeCondition(fHoldCondition);
                }

                thread.getParentThread().schedule();
            }
        }
    }
    
    public PyList getResultList(List fileList)
    {
        if (fileList == null || fileList.size() == 0) return null;

        // Create a Python List to contain a list of returned file info:
        //   [ [ File1 RC, File1 Data ], [ File2 RC, File2 Data ], ... ] 
        PyList resultList = new PyList();

        try
        {
            Iterator iter = fileList.iterator();

            while (iter.hasNext())
            {
                Map fileInfoMap = (Map)iter.next();

                // Create a Python List to contain file information (RC & data)
                PyList fileInfoList = new PyList();

                // Assign file RC
                fileInfoList.append(Py.java2py(
                    new Integer((String)fileInfoMap.get("rc"))));
                fileInfoList.append(Py.java2py(
                    (String)fileInfoMap.get("data")));

                // Add file information List to result List
                resultList.append(fileInfoList);
            }
        }
        catch (Exception ex)
        {
            // This should never happen
            ex.printStackTrace();

            fThread.pySetVar("STAXResult", Py.None);
            
            setElementInfo(new STAXElementInfo(
                getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                "Error assigning the content of the files returned by the " +
                "\"process\" to STAXResult.  File List: " +
                fileList.toString() + "\n\n" + ex.toString()));

            fThread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this));

            fThread.raiseSignal("STAXPythonEvaluationError");
        }
        
        return resultList;
    }

    private void setPythonVariablesWhenFail(String stafResult)
    {
        fThread.pySetVar("RC", new Integer(-1));
        fThread.pySetVar("STAFResult", stafResult);
        fThread.pySetVar("STAXResult", Py.None);
    }

    // This method evaluates the process element/sub-elements and attribute
    // values and generates a process start request and assigns it to fRequest.
    // If an error occurs, it raises the appropriate signal and returns a 
    //non-zero value.

    private int generateProcessStartRequest()
    {
        StringBuffer request = new StringBuffer("");
        String evalElem = STAXElementInfo.NO_ELEMENT_NAME;
        String evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
        int evalIndex = 0;

        try
        {
            evalElem = "location";
            fLocation = fThread.pyStringEval(fUnevalLocation);

            if (!fName.equals("")) 
            {
                evalElem = "process";
                evalAttr = "name";
                fName = fThread.pyStringEval(fUnevalName);
            }
            else
            {
                fName = "Process" + 
                        String.valueOf(fThread.getJob().getNextProcNumber());
                fUnevalName = fName;
            }

            evalElem = "command";
            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
            fCommand = fThread.pyStringEval(fUnevalCommand);                

            if (!fCommandMode.equals(""))
            {
                evalAttr = "mode";
                fCommandMode= fThread.pyStringEval(fCommandMode);
                   
                if (!(fCommandMode.equalsIgnoreCase("shell")) &&
                    !(fCommandMode.equalsIgnoreCase("default")))
                {
                    // Raise a StartProcessError signal
                    fState = COMPLETE;
                    fThread.popAction();

                    setPythonVariablesWhenFail(
                        "Process failed to start.  Raised a " +
                        "STAXProcessStartError signal.");

                    setElementInfo(new STAXElementInfo(
                        evalElem, evalAttr, evalIndex,
                        "Invalid value: " + fCommandMode +
                        "\n\nValid values for attribute \"mode\" are: " +
                        "'default' or 'shell'."));

                    fThread.setSignalMsgVar(
                        "STAXProcessStartErrorMsg",
                        STAXUtil.formatErrorMessage(this));

                    fThread.raiseSignal("STAXProcessStartError");

                    return 1;
                }

                if (fCommandMode.equalsIgnoreCase("shell"))
                {
                    request.append("SHELL ");

                    if (!fCommandShell.equals(""))
                    {
                        evalAttr = "shell";
                        fCommandShell= fThread.pyStringEval(fCommandShell);
                        request.append(STAFUtil.wrapData(fCommandShell));
                        request.append(" ");
                    }
                }
                else
                    fCommandShell = "";
            }

            request.append("COMMAND ");
            request.append(STAFUtil.wrapData(fCommand));

            if (!fWorkload.equals("")) 
            {
                evalElem = "workload";
                evalAttr = "if";

                if (fThread.pyBoolEval(fWorkloadIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fWorkload = fThread.pyStringEval(fUnevalWorkload);
                    request.append(" WORKLOAD ");
                    request.append(STAFUtil.wrapData(fWorkload));
                }
                else
                    fWorkload = "";
            }

            if (!fTitle.equals(""))
            {
                evalElem = "title";
                evalAttr = "if";

                if (fThread.pyBoolEval(fTitleIf)) 
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fTitle = fThread.pyStringEval(fUnevalTitle);
                    request.append(" TITLE ");
                    request.append(STAFUtil.wrapData(fTitle));
                }
                else 
                    fTitle = "";
            }
              
            if (!fParms.equals(""))
            {
                evalElem = "parms";
                evalAttr = "if";

                if (fThread.pyBoolEval(fParmsIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fParms = fThread.pyStringEval(fUnevalParms);
                    request.append(" PARMS ");
                    request.append(STAFUtil.wrapData(fParms));
                }
                else
                    fParms = "";
            } 
             
            if (!fWorkdir.equals(""))
            {
                evalElem = "workdir";
                evalAttr = "if";

                if (fThread.pyBoolEval(fWorkdirIf)) 
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fWorkdir = fThread.pyStringEval(fUnevalWorkdir);
                    request.append(" WORKDIR ");
                    request.append(STAFUtil.wrapData(fWorkdir));
                }
                else
                    fWorkdir = "";
            }
            
            evalElem = "var";

            for (int i = 0; i < fUnevalVars.size(); i++)
            {
                evalIndex = i;
                evalAttr = "if";
                String varsIf = fThread.pyStringEval(fVarsIf.elementAt(i));

                if (fThread.pyBoolEval(varsIf))
                {
                    // Evaluate as a list; each item in list is a single var
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    List varList = fThread.pyListEval(
                        fUnevalVars.elementAt(i));
                    for (int j = 0; j < varList.size(); j++)
                    {
                        String var = ((Object)varList.get(j)).toString();
                        fVars.add(var);
                        request.append(" VAR ");
                        request.append(STAFUtil.wrapData(var));
                    }
                }
            }
            
            evalElem = "env";

            for (int i = 0; i < fUnevalEnvs.size(); i++)
            {
                evalIndex = i;
                evalAttr = "if";
                String envsIf = fThread.pyStringEval(fEnvsIf.elementAt(i));

                if (fThread.pyBoolEval(envsIf))
                {
                    // Evaluate as a list; each item in list is a single var
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    List envList = fThread.pyListEval(
                        fUnevalEnvs.elementAt(i));
                    for (int j = 0; j < envList.size(); j++)
                    {
                        String env = ((Object)envList.get(j)).toString();
                        fEnvs.add(env);
                        request.append(" ENV ");
                        request.append(STAFUtil.wrapData(env));
                    }        
                }
            }
                
            evalIndex = 0;

            if (!fUseprocessvars.equals(""))
            {
                evalElem = "useprocessvars";
                evalAttr = "if";

                if (fThread.pyBoolEval(fUseprocessvarsIf)) 
                    request.append(" USEPROCESSVARS");
                else
                    fUseprocessvars = "";
            }
               
            if (!fStopusing.equals(""))
            {
                evalElem = "stopusing";
                evalAttr = "if";

                if (fThread.pyBoolEval(fStopusingIf)) 
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fStopusing = fThread.pyStringEval(fUnevalStopusing);
                    request.append(" STOPUSING ");
                    request.append(STAFUtil.wrapData(fStopusing));
                }
                else
                    fStopusing = ""; 
            }
                
            if (!fConsole.equals(""))
            {
                evalElem = "console";
                evalAttr = "if";

                if (fThread.pyBoolEval(fConsoleIf))
                {
                    evalAttr = "use";
                    fConsole = fThread.pyStringEval(fUnevalConsole);

                    if (fConsole.equalsIgnoreCase("new")) 
                        request.append(" NEWCONSOLE");
                    else if (fConsole.equalsIgnoreCase("same")) 
                        request.append(" SAMECONSOLE");
                    else
                    {
                        // Raise a StartProcessError signal
                        fState = COMPLETE;
                        fThread.popAction();

                        setPythonVariablesWhenFail(
                            "Process failed to start.  Raised a " +
                            "STAXProcessStartError signal.");

                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr, evalIndex,
                            "Invalid value: " + fConsole +
                            "\n\nValid values for attribute \"use\" are: " +
                            "'new' or 'same'."));

                        fThread.setSignalMsgVar(
                            "STAXProcessStartErrorMsg",
                            STAXUtil.formatErrorMessage(this));

                        fThread.raiseSignal("STAXProcessStartError");

                        return 1;
                    }
                }
                else
                    fConsole = "";
            }

            if (!fFocus.equals(""))
            {
                evalElem = "focus";
                evalAttr = "if";

                if (fThread.pyBoolEval(fFocusIf))
                {
                    evalAttr = "mode";
                    fFocus = fThread.pyStringEval(fUnevalFocus);

                    if (fFocus.equalsIgnoreCase("minimized"))
                    {
                        fFocus = "Minimized";
                        request.append(" FOCUS Minimized");
                    }
                    else if (fFocus.equalsIgnoreCase("foreground"))
                    {
                        fFocus = "Foreground";
                        request.append(" FOCUS Foreground");
                    }
                    else if (fFocus.equalsIgnoreCase("background")) 
                    {
                        fFocus = "Background";
                        request.append(" FOCUS Background");
                    }
                    else
                    {
                        // Raise a StartProcessError signal
                        fState = COMPLETE;
                        fThread.popAction();

                        setPythonVariablesWhenFail(
                            "Process failed to start.  Raised a " +
                            "STAXProcessStartError signal.");

                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr, evalIndex,
                            "Invalid valid: " + fFocus +
                            "\n\nValid values for attribute \"mode\" are: " +
                            "'minimized', 'foreground', or 'background'."));

                        fThread.setSignalMsgVar(
                            "STAXProcessStartErrorMsg",
                            STAXUtil.formatErrorMessage(this));

                        fThread.raiseSignal("STAXProcessStartError");

                        return 1;
                    }
                }
                else
                    fFocus = "";
            }
                
            if (!fUsername.equals(""))
            {
                evalElem = "username";
                evalAttr = "if";

                if (fThread.pyBoolEval(fUsernameIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fUsername = fThread.pyStringEval(fUnevalUsername);
                    request.append(" USERNAME ");
                    request.append(STAFUtil.wrapData(fUsername));
                }
                else
                    fUsername = "";
            }

            if (!fPassword.equals(""))
            {
                evalElem = "password";
                evalAttr = "if";

                if (fThread.pyBoolEval(fPasswordIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fPassword = fThread.pyStringEval(fUnevalPassword);
                    request.append(" PASSWORD ");
                    request.append(STAFUtil.wrapData(fPassword));
                }
                else
                    fPassword = "";
            }
            
            if (!fDisabledauth.equals(""))
            {
                evalElem = "disabledauth";
                evalAttr = "if";

                if (fThread.pyBoolEval(fDisabledauthIf))
                {
                    evalAttr = "action";

                    fDisabledauth = fThread.pyStringEval(fUnevalDisabledauth);

                    if (fDisabledauth.equalsIgnoreCase("error")) 
                        request.append(" DISABLEDAUTHISERROR");
                    else if (fDisabledauth.equalsIgnoreCase("ignore")) 
                        request.append(" IGNOREDISABLEDAUTH");
                    else
                    {
                        // Raise a StartProcessError signal
                        fState = COMPLETE;
                        fThread.popAction();

                        setPythonVariablesWhenFail(
                            "Process failed to start.  Raised a " +
                            "STAXProcessStartError signal.");
 
                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr, evalIndex,
                            "Invalid value: " + fDisabledauth +
                            "\n\nValid values for attribute \"action\" are: " +
                            "'error' or 'ignore'."));

                        fThread.setSignalMsgVar(
                            "STAXProcessStartErrorMsg",
                            STAXUtil.formatErrorMessage(this));

                        fThread.raiseSignal("STAXProcessStartError");

                        return 1;
                    }
                }
                else
                    fDisabledauth = "";
            }
                
            if (!fStatichandlename.equals(""))
            {
                evalElem = "statichandlename";
                evalAttr = "if";

                if (fThread.pyBoolEval(fStatichandlenameIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fStatichandlename = fThread.pyStringEval(
                                        fUnevalStatichandlename);
                    request.append(" STATICHANDLENAME ");
                    request.append(STAFUtil.wrapData(fStatichandlename));
                }
                else
                    fStatichandlename = "";
            }
                
            if (!fStdin.equals(""))
            {
                evalElem = "stdin";
                evalAttr = "if";

                if (fThread.pyBoolEval(fStdinIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fStdin = fThread.pyStringEval(fUnevalStdin);
                    request.append(" STDIN ");
                    request.append(STAFUtil.wrapData(fStdin));
                }
                else
                    fStdin = "";
            }
                
            if (!fStdout.equals(""))
            {
                evalElem = "stdout";
                evalAttr = "if";

                if (fThread.pyBoolEval(fStdoutIf))
                {
                    evalAttr = "mode";
                    fStdoutMode = fThread.pyStringEval(fUnevalStdoutMode);
 
                    if (fStdoutMode.equalsIgnoreCase("replace"))
                    {
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                        fStdout = fThread.pyStringEval(fUnevalStdout);
                        request.append(" STDOUT ");
                        request.append(STAFUtil.wrapData(fStdout));
                    }
                    else if (fStdoutMode.equalsIgnoreCase("append"))
                    {
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                        fStdout = fThread.pyStringEval(fUnevalStdout);
                        request.append(" STDOUTAPPEND ");
                        request.append(STAFUtil.wrapData(fStdout));
                    } 
                    else
                    {
                        // Raise a StartProcessError signal
                        fState = COMPLETE;
                        fThread.popAction();

                        setPythonVariablesWhenFail(
                            "Process failed to start.  Raised a " +
                            "STAXProcessStartError signal.");
 
                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr, evalIndex,
                            "Invalid value: " + fStdoutMode +
                            "\n\nValid values for attribute \"mode\" are: " +
                            "'replace' or 'append'."));

                        fThread.setSignalMsgVar(
                            "STAXProcessStartErrorMsg",
                            STAXUtil.formatErrorMessage(this));

                        fThread.raiseSignal("STAXProcessStartError");
                        return 1;        

                    }
                }
                else
                {
                    fStdout = "";
                    fStdoutMode = "";
                }
            }
                
            if (!fStderr.equals("") || !fUnevalStderrMode.equals(""))
            {
                evalElem = "stderr";
                evalAttr = "if";

                if (fThread.pyBoolEval(fStderrIf)) 
                {
                    evalAttr = "mode";
                    fStderrMode = fThread.pyStringEval(fUnevalStderrMode);

                    if (! fStderrMode.equalsIgnoreCase("stdout"))
                    {
                        // Check if EXACT same file name was specified
                        // for stdout and stderr and if so, set mode=stdout
                        evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                        fStderr = fThread.pyStringEval(fUnevalStderr);
                        if (fStderr.equals(fStdout))
                            fStderrMode = "stdout";
                    }
                        
                    evalAttr = "mode";

                    if (fStderrMode.equalsIgnoreCase("replace"))
                    {
                        request.append(" STDERR ");
                        request.append(STAFUtil.wrapData(fStderr));
                    }                
                    else if (fStderrMode.equalsIgnoreCase("append"))
                    {
                        request.append(" STDERRAPPEND ");
                        request.append(STAFUtil.wrapData(fStderr));
                    }
                    else if (fStderrMode.equalsIgnoreCase("stdout"))
                    {
                        fStderr = "";
                        request.append(" STDERRTOSTDOUT");
                    }
                    else
                    {
                        // Raise a StartProcessError signal
                        fState = COMPLETE;
                        fThread.popAction();

                        setPythonVariablesWhenFail(
                            "Process failed to start.  Raised a " +
                            "STAXProcessStartError signal.");
 
                        setElementInfo(new STAXElementInfo(
                            evalElem, evalAttr, evalIndex,
                            "Invalid value: " + fStderrMode +
                            "\n\nValid values for attribute \"mode\" are: " +
                            "'replace', 'append', or 'stdout'."));

                        fThread.setSignalMsgVar(
                            "STAXProcessStartErrorMsg",
                            STAXUtil.formatErrorMessage(this));

                        fThread.raiseSignal("STAXProcessStartError");
                        
                        return 1;
                    }
                }
                else
                {
                    fStderr = "";
                    fStderrMode = "";
                }
            }
            
            if (!fReturnStdout.equals(""))
            {
                evalElem = "returnstdout";
                evalAttr = "if";

                if (fThread.pyBoolEval(fReturnStdoutIf)) 
                    request.append(" RETURNSTDOUT");
                else
                    fReturnStdout = "";
            }
             
            if (!fReturnStderr.equals(""))
            {
                evalElem = "returnstderr";
                evalAttr = "if";

                if (fThread.pyBoolEval(fReturnStderrIf)) 
                    request.append(" RETURNSTDERR");
                else
                    fReturnStderr = "";
            }
                
            evalElem = "returnfile";

            for (int i = 0; i < fUnevalReturnFiles.size(); i++)
            {
                evalIndex = i;
                evalAttr = "if";
                String returnFilesIf = fThread.pyStringEval(
                    fReturnFilesIf.elementAt(i));

                if (fThread.pyBoolEval(returnFilesIf))
                {
                    // Evaluate as a list; each list item is a single file
                        
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    List fileList = fThread.pyListEval(
                        fUnevalReturnFiles.elementAt(i));

                    for (int j = 0; j < fileList.size(); j++)
                    {
                        String file = ((Object)fileList.get(j)).toString();
                        fReturnFiles.add(file);
                        request.append(" RETURNFILE ");
                        request.append(STAFUtil.wrapData(file));
                    }
                }
            }

            evalIndex = 0;

            if (fProcessAction != null)
            {
                evalElem = "process-action";
                evalAttr = "if";
                fProcessActionFlag = fThread.pyBoolEval(fProcessActionIf);
            }

            if (!fOther.equals(""))
            {
                evalElem = "other";
                evalAttr = "if";

                if (fThread.pyBoolEval(fOtherIf))
                {
                    evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
                    fOther = fThread.pyStringEval(fUnevalOther);
                    request.append(" ");
                    request.append(fOther);
                }
                else
                    fOther = "";      
            }
        }
        catch (STAXPythonEvaluationException e)
        {
            fState = COMPLETE;
            fThread.popAction();

            setPythonVariablesWhenFail(
                "Process failed to start.  Raised a " +
                "STAXPythonEvaluationError signal.");

            setElementInfo(new STAXElementInfo(
                evalElem, evalAttr, evalIndex));

            fThread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            fThread.raiseSignal("STAXPythonEvaluationError");

            return 1;
        }
                  
        fRequest = request.toString();

        return 0;
    }


    private void generateProcessStartEvent()
    {
        // Generate a start process event

        HashMap<String, String> processMap = new HashMap<String, String>();
        processMap.put("type", "process");
        processMap.put("block", fCurrentBlockName);
        processMap.put("status", "start");
        processMap.put("location", fLocation);
        processMap.put("command", STAFUtil.maskPrivateData(fCommand));
        processMap.put("handle", String.valueOf(fProcessHandle));
        processMap.put("parms", STAFUtil.maskPrivateData(fParms));
        processMap.put("name", fName);
        processMap.put("mode", fCommandMode);
        processMap.put("workload", fWorkload);
        processMap.put("title", fTitle);
        processMap.put("workdir", fWorkdir);

        if (!fCommandShell.equals(""))
        {    
            processMap.put("shell", fCommandShell);
        }

        // Convert the fVars vector to a list

        List<String> varList = new ArrayList<String>();

        for (int i = 0; i < fVars.size(); i++)
        {
            varList.add(fVars.elementAt(i));
        }

        processMap.put("varList", STAFMarshallingContext.marshall(
            varList, new STAFMarshallingContext()));

        // Convert the fEnvs vector to a list

        List<String> envList = new ArrayList<String>();

        for (int i = 0; i < fEnvs.size(); i++)
        {
            envList.add(fEnvs.elementAt(i));
        }

        processMap.put("envList", STAFMarshallingContext.marshall(
            envList, new STAFMarshallingContext()));
        
        if (!fUseprocessvars.equals(""))
        {
            processMap.put("useprocessvars", "");
        }

        processMap.put("stopusing", fStopusing);

        if (fConsole.equalsIgnoreCase("new"))
        {
            processMap.put("newconsole", "");
        }
        else if (fConsole.equalsIgnoreCase("same"))
        {
            processMap.put("sameconsole", "");
        }

        if (!fFocus.equals(""))
        {
            processMap.put("focus", fFocus);
        }

        processMap.put("username", fUsername);

        if (!fPassword.equals(""))
        {
            processMap.put("password", "*******");
        }

        if (fDisabledauth.equalsIgnoreCase("error"))
        {
            processMap.put("disabledauthiserror", "");
        }
        else if (fDisabledauth.equalsIgnoreCase("ignore"))
        {
            processMap.put("ignoredisabledauth", "");
        }

        processMap.put("stdin", fStdin);       

        if (fStdoutMode.equalsIgnoreCase("replace"))
        {
            processMap.put("stdout", fStdout);
        }
        else if (fStdoutMode.equalsIgnoreCase("append"))
        {
            processMap.put("stdoutappend", fStdout);
        }

        if (fStderrMode.equalsIgnoreCase("replace"))
        {
            processMap.put("stderr", fStderr);
        }
        else if (fStderrMode.equalsIgnoreCase("append"))
        {
           processMap.put("stderrappend", fStderr);
        }
        else if (fStderrMode.equalsIgnoreCase("stdout"))
        {
            processMap.put("stderrtostdout", "");
        }

        if (!fReturnStdout.equals(""))
        {
            processMap.put("returnstdout", "");
        }

        if (!fReturnStderr.equals(""))
        {
            processMap.put("returnstderr", "");
        }

        // Convert the fReturnFiles vector to a string containing a marshalled
        // list of strings

        List<String> returnFileList = new ArrayList<String>();

        for (int i = 0; i < fReturnFiles.size(); i++)
        {
            returnFileList.add(fReturnFiles.elementAt(i));
        }

        processMap.put("returnFileList", STAFMarshallingContext.marshall(
            returnFileList, new STAFMarshallingContext()));
        
        processMap.put("statichandlename", fStatichandlename);

        processMap.put("other", fOther);

        fThread.getJob().generateEvent(
            STAXProcessActionFactory.STAX_PROCESS_EVENT, processMap);
    }
    

    private void generateProcessStopEvent()
    {
        // Generate a stop process event

        HashMap<String, String> processMap = new HashMap<String, String>();
        processMap.put("type", "process");
        processMap.put("block", fCurrentBlockName);
        processMap.put("status", "stop");
        processMap.put("location", fLocation);
        processMap.put("command", STAFUtil.maskPrivateData(fCommand));
        processMap.put("handle", String.valueOf(fProcessHandle));
        processMap.put("parms", STAFUtil.maskPrivateData(fParms));
        processMap.put("name", fName);

        fThread.getJob().generateEvent(
            STAXProcessActionFactory.STAX_PROCESS_EVENT, processMap);
    }


    private String replace(String source, String find, String replace) 
    {
        StringBuffer result = new StringBuffer(source);
        int position = source.indexOf(find);
        
        while (position > -1) 
        {
            result.replace(position, position + find.length(), replace);
            position = result.toString().indexOf(find, position +
            replace.length());
        }
        
        return result.toString();
    }

    STAXThread fThread = null;
    int fState = INIT;
    boolean fIsProcessRunning = false;
    boolean fIsProcessActionRunning = false;
    
    private String fLocation = new String();
    private String fCommand = new String();
    private String fName = new String();
    private String fWorkload = new String("");
    private String fWorkloadIf = new String("");
    private String fTitle = new String("");
    private String fTitleIf = new String("");
    private String fParms = new String("");
    private String fParmsIf = new String("");
    private String fWorkdir = new String("");
    private String fWorkdirIf = new String("");
    private Vector<String> fVars = new Vector<String>();
    private Vector<String> fVarsIf = new Vector<String>();
    private Vector<String> fEnvs = new Vector<String>();
    private Vector<String> fEnvsIf = new Vector<String>();
    private String fUseprocessvars = new String("");
    private String fUseprocessvarsIf = new String("");
    private String fStopusing = new String("");
    private String fStopusingIf = new String("");
    private String fConsole = new String("");
    private String fConsoleIf = new String("");
    private String fFocus = new String("");
    private String fFocusIf = new String("");
    private String fUsername = new String("");
    private String fUsernameIf = new String("");
    private String fPassword = new String("");
    private String fPasswordIf = new String("");
    private String fDisabledauth = new String("");
    private String fDisabledauthIf = new String("");
    private String fStatichandlename = new String("");
    private String fStatichandlenameIf = new String("");
    private String fStdin = new String("");
    private String fStdinIf = new String("");
    private String fStdout = new String("");
    private String fStdoutIf = new String("");
    private String fStdoutMode = new String("");
    private String fStderr = new String("");
    private String fStderrIf = new String("");
    private String fStderrMode = new String("");
    private String fReturnStdout = new String("");
    private String fReturnStdoutIf = new String("");
    private String fReturnStderr = new String("");
    private String fReturnStderrIf = new String("");
    private Vector<String> fReturnFiles = new Vector<String>();
    private Vector<String> fReturnFilesIf = new Vector<String>();
    private String fOther = new String("");
    private String fOtherIf = new String("");
    private String fCommandMode = new String("");
    private String fCommandShell = new String("");
    private STAXAction fProcessAction = null;
    private String fProcessActionIf = new String("");
    
    private String fUnevalLocation = new String();
    private String fUnevalCommand = new String();
    private String fUnevalName = new String();
    private String fUnevalWorkload = new String("");
    private String fUnevalTitle = new String("");
    private String fUnevalParms = new String("");
    private String fUnevalWorkdir = new String("");
    private Vector<String> fUnevalVars = new Vector<String>();
    private Vector<String> fUnevalEnvs = new Vector<String>();
    private String fUnevalUseprocessvars = new String("");
    private String fUnevalStopusing = new String("");
    private String fUnevalConsole = new String("");
    private String fUnevalFocus = new String("");
    private String fUnevalUsername = new String("");
    private String fUnevalPassword = new String("");
    private String fUnevalDisabledauth = new String("");
    private String fUnevalStatichandlename = new String("");
    private String fUnevalStdin = new String("");
    private String fUnevalStdout = new String("");
    private String fUnevalStdoutMode = new String("");
    private String fUnevalStderr = new String("");
    private String fUnevalStderrMode = new String("");
    private String fUnevalReturnStdout = new String("");
    private String fUnevalReturnStderr = new String("");
    private Vector<String> fUnevalReturnFiles = new Vector<String>();
    private String fUnevalOther = new String("");
    private String fUnevalProcessActionIf = new String("");
    private String fUnevalCommandMode = new String("");
    private String fUnevalCommandShell = new String("");

    private STAXHoldThreadCondition fHoldCondition = 
        new STAXHoldThreadCondition("Process");
    private STAXHardHoldThreadCondition fHardHoldCondition =
        new STAXHardHoldThreadCondition("Process");
    private int fRequestNumber = 0;
    private int fRequestRC = 0;
    private String fRequest = new String();
    private String fRequestResult = new String();
    private int fProcessHandle = 0;
    private long fProcessRC = -1;
    private PyObject fProcessSTAFResult = Py.None;
    private PyObject fProcessSTAXResult = Py.None;
    private String fProcessTimestamp = new String();
    private STAXTimestamp fStartTimestamp;
    private STAXProcessActionFactory fFactory;
    private String fCurrentBlockName = new String();
    private HashMap<Integer, STAXThread> fThreadMap =
        new HashMap<Integer, STAXThread>();
    private boolean fProcessActionFlag = true;
    private STAXTimedEvent fTimedEvent = null;
    private String fProcessKey = "";
    private boolean fUsingProcessKeyOption = true;
}