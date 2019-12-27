package com.ibm.staf.service.sxe;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.Date;
import java.util.Calendar;
import java.util.StringTokenizer;
import java.util.Map;
import java.text.SimpleDateFormat;

/**
 * This class provides the full function of the STAFSXE
 * Service.
 */

public class STAFSXE implements STAFServiceInterfaceLevel30
{
    private STAFHandle sHandle;
    private String fServiceName;
    private STAFCommandParser sxeParser;
    private STAFCommandParser fListParser;
    private String logName = "SXELOG";
    private String fLocalMachineName = "";

    private STAFMapClassDefinition fExecutionResultsMapClass;
    private STAFMapClassDefinition fErrorResultsMapClass;
    private STAFMapClassDefinition fLogStartStopMapClass;
    private STAFMapClassDefinition fLogPassFailMapClass;
    private STAFMapClassDefinition fLogCommandMapClass;
    private STAFMapClassDefinition fSettingsMapClass;

    private static String sHelpMsg;
    private static String sLineSep;

    /* static strings for parser */

    private final static String EXECUTE = "EXECUTE";
    private final static String LIST = "LIST";
    private final static String VERSION = "VERSION";
    private final static String HELP = "HELP";
    private final static String FILE = "FILE";
    private final static String LOOP = "LOOP";
    private final static String FOREVER = "FOREVER";
    private final static String MINRUNTIME = "MINRUNTIME";
    private final static String LOGNAME = "LOGNAME";

    /* other static values */

    private final static String SXEVERSION = "3.0.3";

    private final static int START = 0;
    private final static int STOPPASS = 1;
    private final static int STOPFAIL = 2;

    private final static String LOGLEVELVAR = "{STAF/Service/SXE/LogLevel}";
    private final static String ELAPSEDTARGETVAR =
        "{STAF/Service/SXE/ElapsedTarget}";
    private final static String ELAPSEDTOLVAR =
        "{STAF/Service/SXE/ElapsedTolerance}";
    private final static String LOGNONE = "NONE";
    private final static String LOGFILEONLY = "FILE";
    private final static String LOGCOMMAND = "COMMAND";

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    /* SXE STAF return codes */

    public final static int EXECUTIONERROR = 4001;
    public final static String EXECUTIONERRORInfo = "Execution Error";
    public final static String EXECUTIONERRORDesc =
        "An error occurred during execution";
    public final static int ELAPSEDTIMEFAIL = 4004;
    public final static String ELAPSEDTIMEFAILInfo =
        "Elapsed Target Time Exceeded";
    public final static String ELAPSEDTIMEFAILDesc =
        "The testcase elapsed time exceeded the elapsed target " +
        "time by more than the elapsed tolerance value.";
    public final static int ELAPSEDTIMEERROR = 4005;
    public final static String ELAPSEDTIMEERRORInfo = "Elapsed Target Error";
    public final static String ELAPSEDTIMEERRORDesc =
        "An error occurred resolving the elapsed target variables.";

/**
 * STAFSXE constructor
 */

public STAFSXE()
{
    super();
}

/**
 * Method required by implemented STAF interface. This is the
 * entry point for a STAF request.
 */

public STAFResult acceptRequest(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Try block is here to catch any unexpected errors/exceptions

    try
    {
        // Determine the command request (the first word in the request)

        String action;
        int spaceIndex = info.request.indexOf(" ");

        if (spaceIndex != -1)
            action = info.request.substring(0, spaceIndex);
        else
            action = info.request;

        String actionUC = action.toUpperCase();

        // Call the appropriate method to handle the command request

        if (actionUC.equals(EXECUTE))
            return handleExecute(info);
        else if (actionUC.equals(LIST))
            return handleList(info);
        else if (actionUC.equals(VERSION))
            return handleVersion(info);
        else if (actionUC.equals(HELP))
            return handleHelp(info);
        else
        {
            return new STAFResult(
                STAFResult.InvalidRequestString,
                "'" + action + "' is not a valid command request for the " +
                fServiceName + " service" + sLineSep + sLineSep + sHelpMsg);
        }
    }
    catch (Throwable t)
    {
        // Write the Java stack trace to the JVM log for the service

        System.out.println(
            sTimestampFormat.format(Calendar.getInstance().getTime()) +
            " ERROR: Exception on " + fServiceName + " service request: " +
            sLineSep + sLineSep + info.request + sLineSep);

        t.printStackTrace();

        // And also return the Java stack trace in the result

        StringWriter sw = new StringWriter();
        t.printStackTrace(new PrintWriter(sw));

        if (t.getMessage() != null)
        {
            return new STAFResult(
                STAFResult.JavaError,
                t.getMessage() + sLineSep + sw.toString());
        }
        else
        {
            return new STAFResult(
                STAFResult.JavaError, sw.toString());
        }
    }
}

/**
 * Checks if the elapsed time is within the specified parameters.
 *
 * Returns false if the elapsed time exceeded the target+tolerance,
 * or true otherwise.
 */

private boolean checkElapsedTime(long startTime, long stopTime,
                                 long elapsedTarget, int elapsedTolerance)
{
    /* Calculate the maxTime in millis:
       elapsedTarget + elapsedTarget * elapsedTolerance% */

    long maxTime = (long) (elapsedTarget +
        elapsedTarget*(((double) elapsedTolerance)/100));

    if (stopTime - startTime > maxTime)
        return false;
    else
        return true;
}

/**
 * Executes all commands in the specified file. One iteration
 * through this method executes the file once.
 * If a command returns a non-zero return code, execution
 * will stop at that point.
 * Returns the STAFResult from executing the file. This will
 * be the STAFResult from a failed command if one failed.
 */

private STAFResult execute(STAFHandle fHandle, String filename,
                           String logName, int loopNum)
{
    long startTime;
    long stopTime = 0;

    /* Use 1 length arrays and a StringBuffer so that other called methods
        may directly modify these primitives/objects */

    long[] elapsedTarget = new long[1];
    int[] elapsedTolerance = new int[1];
    StringBuffer elapsedTargetSBuf = new StringBuffer();

    STAFMarshallingContext mc = new STAFMarshallingContext();

    /* open file */

    BufferedReader in = null;
    STAFResult result;

    try
    {   //big try block so we can close file in finally

        try
        {
            in = new BufferedReader(new FileReader(new File(filename)));
        }
        catch(FileNotFoundException fnfe)
        {
            // Create a map containing the error results

            Map errorMap = fErrorResultsMapClass.createInstance();
            errorMap.put("loopNum", String.valueOf(loopNum));
            errorMap.put("rc", String.valueOf(STAFResult.FileOpenError));
            errorMap.put("result", fnfe.toString());

            mc.setRootObject(errorMap);
            mc.setMapClassDefinition(fErrorResultsMapClass);
            return new STAFResult(EXECUTIONERROR, mc.marshall());
        }

        /* iterate through file and execute STAF commands */

        int commandCount = 0;
        int lineCount = 0;

        /* Get startTime and log test start */

        startTime = new Date().getTime(); //get StartTime
        logFile(logName, START, fHandle, loopNum, startTime, stopTime,
            elapsedTargetSBuf.toString(), elapsedTolerance[0]);

        while(true)
        {
            String nextLine = null;

            /* Read a line from file */

            try
            {
                nextLine = in.readLine();
            }
            catch(IOException ioe)
            {
                stopTime = new Date().getTime(); //get stopTime, IOError

                // Create a map containing the error results

                Map errorMap = fErrorResultsMapClass.createInstance();
                errorMap.put("loopNum", String.valueOf(loopNum));
                errorMap.put("lineNum", String.valueOf(lineCount + 1));
                errorMap.put("commandNum", String.valueOf(commandCount + 1));
                errorMap.put("command", nextLine);
                errorMap.put("rc", String.valueOf(STAFResult.FileReadError));
                errorMap.put("result", ioe.toString());

                mc.setRootObject(errorMap);
                mc.setMapClassDefinition(fErrorResultsMapClass);
                result = new STAFResult(EXECUTIONERROR, mc.marshall());

                logFile(logName, STOPFAIL, fHandle, loopNum, startTime,
                    stopTime, elapsedTargetSBuf.toString(),
                    elapsedTolerance[0]);
                break;
            }

            if (nextLine == null)
            {   //at end of file, no failures

                stopTime = new Date().getTime(); //get stopTime, success
                boolean checkElapsed;

                try
                {
                    checkElapsed = isCheckElapsed(fHandle, elapsedTargetSBuf,
                        elapsedTarget, elapsedTolerance);
                }
                catch(Exception e)
                {
                    // Create a map containing the error results

                    Map errorMap = fErrorResultsMapClass.createInstance();
                    errorMap.put("loopNum", String.valueOf(loopNum));
                    errorMap.put("rc", String.valueOf(ELAPSEDTIMEERROR));
                    errorMap.put("result",
                                 "Error in ElapsedTime/ElapsedTolerance " +
                                 "variables.  Exception: " + e.toString());

                    mc.setRootObject(errorMap);
                    mc.setMapClassDefinition(fErrorResultsMapClass);
                    result = new STAFResult(ELAPSEDTIMEERROR, mc.marshall());

                    logCommand(logName, fHandle, STOPFAIL, null, result);

                    logFile(logName, STOPFAIL, fHandle, loopNum, startTime,
                        stopTime, elapsedTargetSBuf.toString(),
                        elapsedTolerance[0]);
                    break;
                }

                if (checkElapsed &&
                    !checkElapsedTime(startTime, stopTime, elapsedTarget[0],
                    elapsedTolerance[0]))
                {
                    //elapsed time check failed

                    // Create a map containing the error results

                    Map errorMap = fErrorResultsMapClass.createInstance();
                    errorMap.put("loopNum", String.valueOf(loopNum));
                    errorMap.put("rc", String.valueOf(ELAPSEDTIMEFAIL));
                    errorMap.put("result", "Elapsed Time: " +
                                 getElapsedTime(startTime, stopTime));

                    mc.setRootObject(errorMap);
                    mc.setMapClassDefinition(fErrorResultsMapClass);
                    result = new STAFResult(ELAPSEDTIMEFAIL, mc.marshall());

                    logFile(logName, STOPFAIL, fHandle, loopNum, startTime,
                        stopTime, elapsedTargetSBuf.toString(),
                        elapsedTolerance[0]);
                }
                else
                {
                    // Create a map containing the successful execution results

                    Map resultMap = fExecutionResultsMapClass.createInstance();
                    resultMap.put("loops", String.valueOf(loopNum));
                    resultMap.put("commands", String.valueOf(commandCount));

                    mc.setRootObject(resultMap);
                    mc.setMapClassDefinition(fExecutionResultsMapClass);
                    result = new STAFResult(STAFResult.Ok, mc.marshall());

                    logFile(logName, STOPPASS, fHandle, loopNum, startTime,
                        stopTime, elapsedTargetSBuf.toString(),
                        elapsedTolerance[0]);
                }
                break;
            }

            /* If line is blank go to next line */
            nextLine = nextLine.trim(); //trim whitespace

            if (nextLine.equals(new String()))
            {
                lineCount++;
                continue;
            }

            /* Goto next line if 1st non-whitespace character is # */
            if (nextLine.startsWith("#"))
            {
                lineCount++;
                continue;
            }

            logCommand(logName, fHandle, START, nextLine, null);

            result = sendSTAFCommand(fHandle, nextLine);

            if (result.rc != STAFResult.Ok)
            {
                stopTime = new Date().getTime(); //get stopTime, fail
                
                // Create a map containing the error results

                Map errorMap = fErrorResultsMapClass.createInstance();
                errorMap.put("loopNum", String.valueOf(loopNum));
                errorMap.put("lineNum", String.valueOf(lineCount + 1));
                errorMap.put("commandNum", String.valueOf(commandCount + 1));
                errorMap.put("command", nextLine);
                errorMap.put("rc", String.valueOf(result.rc));
                errorMap.put("result", result.result);

                mc.setRootObject(errorMap);
                mc.setMapClassDefinition(fErrorResultsMapClass);
                result = new STAFResult(EXECUTIONERROR, mc.marshall());

                logCommand(logName, fHandle, STOPFAIL, nextLine, result);

                logFile(logName, STOPFAIL, fHandle, loopNum, startTime,
                    stopTime, elapsedTargetSBuf.toString(),
                    elapsedTolerance[0]);
                break;
            }

            logCommand(logName, fHandle, STOPPASS, nextLine, result);
            commandCount++;
            lineCount++;
        }
    }
    finally
    {   //make sure we always try to close file

        try
        {
            in.close();
        }
        catch(Exception e)
        {
            // do nothing??
        }
    }

    return result;
}

/**
 * Returns a String representation of the difference in
 * two times. The inputs are two times expressed in milliseconds
 * and stored in a long (see java.util.Date).
 * The returned format is: hh:mm:ss.mmm
 */

private String getElapsedTime(long start, long stop)
{
    long time = stop - start;
    long hour = time / 3600000;
    time = time%3600000;
    long min = time / 60000;
    time = time%60000;
    long sec = time / 1000;
    long milli = time%1000;
    String milliString;

    /* Make sure that millis are represented properly */

    if(milli < 10)
    {
        milliString = "00"+String.valueOf(milli);
    }
    else if (milli < 100)
    {
        milliString = "0"+String.valueOf(milli);
    }
    else
    {
        milliString = String.valueOf(milli);
    }

    String formattedTime =
        String.valueOf(hour)+":"+String.valueOf(min)+":"+
        String.valueOf(sec)+"."+milliString;

    return formattedTime;
}

/**
 * Returns a formatted String representation of a time of day.
 * The time should be expressed in milliseconds and stored in
 * a long (see java.util.Date).
 * The returned format is: hh:mm:ss.mmm
 */

private String getTime(long time)
{
    Calendar cal = Calendar.getInstance();
    cal.setTime(new Date(time));

    int hour = cal.get(Calendar.HOUR_OF_DAY);
    int min = cal.get(Calendar.MINUTE);
    int sec = cal.get(Calendar.SECOND);
    int milli = cal.get(Calendar.MILLISECOND);
    String milliString;

    /* Make sure that millis are represented properly */

    if(milli < 10)
    {
        milliString = "00"+String.valueOf(milli);
    }
    else if (milli < 100)
    {
        milliString = "0"+String.valueOf(milli);
    }
    else
    {
        milliString = String.valueOf(milli);
    }

    String formattedTime = String.valueOf(hour)+":"+String.valueOf(min)+":"+
        String.valueOf(sec)+"."+milliString;

    return formattedTime;
}

/**
 * Given a String representation of time of day in the format
 * hh:mm:ss.mmm returns a long representing the number of milliseconds
 * since the beginning of that day (12 am).
 */

private long getTimeInMillis(String formattedTime) throws NumberFormatException
{
    StringTokenizer tok = new StringTokenizer(formattedTime, ":.");

    int hours = Integer.parseInt(tok.nextToken());
    int mins = Integer.parseInt(tok.nextToken());
    int secs = Integer.parseInt(tok.nextToken());
    String mills = tok.nextToken();

    // Make sure millis is correct (3 chars)
    // otherwise 0:0:0.5 is 5 millis instead of 500

    while(true)
    {
        if (mills.length() == 3)
        {
            break;
        }

        mills = mills.concat("0");
    }

    int millis = Integer.parseInt(mills);

    long time = (hours*3600000)+(mins*60000)+(secs*1000)+millis;

    return time;
}

/**
 * Displays help output for the SXE service.
 */

private STAFResult handleHelp(
    STAFServiceInterfaceLevel30.RequestInfo reqInfo)
{
    // Verify the requester has at least trust level 1

    STAFResult trustResult = STAFUtil.validateTrust(
        1, fServiceName, "HELP", fLocalMachineName, reqInfo);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Return help text for the service

    return new STAFResult(STAFResult.Ok, sHelpMsg);
}


/**
 * Outputs SXE Service version information.
 */


private STAFResult handleVersion(
    STAFServiceInterfaceLevel30.RequestInfo reqInfo)
{
    // Verify the requester has at least trust level 1

    STAFResult trustResult = STAFUtil.validateTrust(
        1, fServiceName, "VERSION", fLocalMachineName, reqInfo);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    return new STAFResult(STAFResult.Ok, SXEVERSION);
}

/**
 * Method required by implemented STAF interface.
 * This method is called when STAF first initializes
 * the service.
 */

public STAFResult init(STAFServiceInterfaceLevel30.InitInfo initInfo)
{
    int result = STAFResult.Ok;

    /* Generate STAFParser */

    sxeParser = new STAFCommandParser();

    sxeParser.addOption(EXECUTE, 1, STAFCommandParser.VALUENOTALLOWED);
    sxeParser.addOption(VERSION, 1, STAFCommandParser.VALUENOTALLOWED);
    sxeParser.addOption(HELP, 1, STAFCommandParser.VALUENOTALLOWED);

    sxeParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    sxeParser.addOption(LOOP, 1, STAFCommandParser.VALUEREQUIRED);
    sxeParser.addOption(MINRUNTIME, 1, STAFCommandParser.VALUEREQUIRED);

    sxeParser.addOptionGroup(FILE, 1, 1);
    sxeParser.addOptionGroup(LOOP+" "+MINRUNTIME, 0, 1);
    sxeParser.addOptionGroup(EXECUTE+" "+VERSION+" "+HELP, 1, 1);

    fListParser = new STAFCommandParser();
    fListParser.addOption("LIST", 1, STAFCommandParser.VALUENOTALLOWED);
    fListParser.addOption("SETTINGS", 1, STAFCommandParser.VALUENOTALLOWED);
    fListParser.addOptionGroup("LIST", 1, 1);
    fListParser.addOptionNeed("LIST", "SETTINGS");
    
    /* Create STAFHandle */

    try
    {
        sHandle = new STAFHandle("STAF/Service/" + initInfo.name);
    }
    catch(STAFException se)
    {
        se.printStackTrace();
        return new STAFResult(STAFResult.STAFRegistrationError,
                              se.toString());
    }

    fServiceName = initInfo.name;

    /* Parse registration parms */

    STAFCommandParser parmsParser = new STAFCommandParser();
    parmsParser.addOption(LOGNAME, 1, STAFCommandParser.VALUEREQUIRED);

    STAFCommandParseResult pResult = parmsParser.parse(initInfo.parms);

    if (pResult.rc != 0)
    {
        System.out.println(
            "Error parsing " + initInfo.name + " service parms.");
        result = pResult.rc;
        return new STAFResult(
            result, "Error parsing " + initInfo.name + " service parms.");
    }

    if (pResult.optionTimes(LOGNAME) == 1)
    {
        STAFResult resolvedResult = STAFUtil.resolveInitVar(
            pResult.optionValue(LOGNAME), sHandle);

        if (resolvedResult.rc != STAFResult.Ok)
        {
            System.out.println("Error resolving LOGNAME.  RC=" +
                resolvedResult.rc + " Result=" + resolvedResult.result);

            return resolvedResult;
        }

        logName = resolvedResult.result;
    }
    
    // Construct map class for a successful EXECUTE request.

    fExecutionResultsMapClass = new STAFMapClassDefinition(
        "STAF/Service/SXE/ExecutionResults");
    fExecutionResultsMapClass.addKey("loops", "Loops Executed");
    fExecutionResultsMapClass.addKey("commands", "Commands Per Loop");

    // Construct map class for a failed EXECUTE request.

    fErrorResultsMapClass = new STAFMapClassDefinition(
        "STAF/Service/SXE/ErrorInfo");
    fErrorResultsMapClass.addKey("loopNum", "Loop Number");
    fErrorResultsMapClass.addKey("lineNum", "Line Number");
    fErrorResultsMapClass.addKey("commandNum", "Command Number");
    fErrorResultsMapClass.addKey("command", "Command");
    fErrorResultsMapClass.addKey("rc", "RC");
    fErrorResultsMapClass.addKey("result", "Result");

    fLogStartStopMapClass = new STAFMapClassDefinition(
        "STAF/Service/SXE/LogStartStop");
    fLogStartStopMapClass.addKey("loopNum", "Loop Number");
    fLogStartStopMapClass.addKey("timestamp", "Time");
    
    fLogPassFailMapClass = new STAFMapClassDefinition(
        "STAF/Service/SXE/LogPassFail");
    fLogPassFailMapClass.addKey("loopNum", "Loop Number");
    fLogPassFailMapClass.addKey("elapsedTime", "Elapsed Time");
    fLogPassFailMapClass.addKey("elapsedTarget", "Elapsed Target");
    fLogPassFailMapClass.addKey("elapsedTolerance",
                                "Elapsed Tolerance Percent");

    fLogCommandMapClass = new STAFMapClassDefinition(
        "STAF/Service/SXE/LogCommand");
    fLogCommandMapClass.addKey("action", "Action");
    fLogCommandMapClass.addKey("command", "Command");
    fLogCommandMapClass.addKey("result", "Result");
    
    // Construct map-class for list settings information

    fSettingsMapClass = new STAFMapClassDefinition(
        "STAF/Service/SXE/Settings");
    fSettingsMapClass.addKey("logName", "Log Name");

    // Resolve the machine name variable for the local machine

    STAFResult res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    fLocalMachineName = res.result;

    // Resolve the line separator variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    sLineSep = res.result;

    // Assign the help text string for the service

    sHelpMsg = "*** " + fServiceName + " Service Help ***" +
        sLineSep + sLineSep +
        "EXECUTE FILE <File> [LOOP <Number of Loops> | MINRUNTIME <HH:MM>]" +
        sLineSep +
        "LIST    SETTINGS" +
        sLineSep +
        "VERSION" +
        sLineSep +
        "HELP";

    /* Register RCs with help service. */
    /* Do not replace result int if it already contained an error */

    int regResult = registerHelp(initInfo.name);

    if (regResult != 0)
    {
        System.out.println("Error registering " + initInfo.name +
                           " return codes with Help service.");

        if (result == 0)
        {
            result = regResult;
        }

        return new STAFResult(result, "Error registering " + initInfo.name + 
                              " return codes with Help service.");
    }
    
    return new STAFResult(result);
}

/**
 * This method determines if elapsedTime checking should be performed
 * for this execution. If both STAF variables STAF/Service/SXE/ElapsedTarget
 * and STAF/Service/SXE/ElapsedTolerance can be resolved, then elapsed time
 * checking should be performed. Note that a StringBuffer and 1 length arrays
 * are used, so that once the variables are resolved they may be used by the
 * calling method.
 * Returns true if elapsed time checking should be performed or false
 * otherwise.
 */

private boolean isCheckElapsed(STAFHandle fHandle,
                               StringBuffer elapsedTargetSBuf,
                               long[] elapsedTarget, int[] elapsedTolerance)
                               throws STAFException, NumberFormatException
{
    /* Resolve parms for ELAPSEDTARGET and ELAPSEDTOLERANCE, if both resolve
       OK, do elapsedTime checking */

    boolean checkElapsed = false;

    String targetReq = "RESOLVE HANDLE " + String.valueOf(fHandle.getHandle()) +
        " STRING " + ELAPSEDTARGETVAR;
    String tolReq = "RESOLVE HANDLE " + String.valueOf(fHandle.getHandle()) +
        " STRING " + ELAPSEDTOLVAR;
    String elapsedTargetString;

    try
    {
        elapsedTargetString = fHandle.submit("LOCAL", "VAR", targetReq);
    }
    catch(STAFException se)
    {
        //don't throw STAFResult.VariableDoesNotExist

        if (se.rc == STAFResult.VariableDoesNotExist)
        {
            return false;
        }
        else
        {
            throw new STAFException(se.rc, se.getMessage());
        }
    }

    elapsedTarget[0] = getTimeInMillis(elapsedTargetString);
    elapsedTolerance[0] = Integer.parseInt(fHandle.submit(
        "LOCAL", "VAR", tolReq));
    elapsedTargetSBuf.replace(0, elapsedTargetSBuf.length(),
                              elapsedTargetString);
    checkElapsed = true; // no exceptions, so check elapsed time

    return checkElapsed;
}

/**
 * Logs the start and success or failure of an individual command.
 * If the STAF variable STAF/Service/SXE/LogLevel is not set to "command",
 * then this logging will not occur.
 */

private void logCommand(String logName, STAFHandle handle, int logType,
                        String command, STAFResult result)
{
    STAFResult llResult = handle.submit2(
        "local", "VAR", "RESOLVE STRING " + LOGLEVELVAR);

    if ((llResult.rc == STAFResult.Ok) &&
        (llResult.result.equalsIgnoreCase(LOGCOMMAND)))
    {
        String where = "local";
        String service = "LOG";
        String request = null;
        String logMessage;
        String logLevel = "info";
        
        // Create a map containing the log message information

        Map logMap = fLogCommandMapClass.createInstance();

        switch(logType)
        {
            case START:
                logMap.put("action", "Begin");
                logMap.put("command", command);
                break;
            case STOPPASS:
                logMap.put("action", "End");
                logMap.put("command", command);
                logMap.put("result", result.result);
                break;
            case STOPFAIL:
                logMap.put("action", "End");
                logMap.put("command", command);
                logMap.put("result", result.result);
                logLevel = "error";
                break;
        }

        STAFMarshallingContext mc = new STAFMarshallingContext();
        mc.setRootObject(logMap);
        mc.setMapClassDefinition(fLogCommandMapClass);
        logMessage = mc.marshall();

        request = "LOG MACHINE LOGNAME " + logName + " LEVEL " + logLevel +
            " MESSAGE " + STAFUtil.wrapData(logMessage);

        handle.submit2(where, service, request);
    }
}

/**
 * Logs the start and success or failure of the execution of a file.
 * If the STAF variable STAF/Service/SXE/LogLevel is not set to "command" or
 * "file" then this logging will not occur.
 */

private void logFile(String logName, int logType, STAFHandle handle,
                     int loopNum, long startTime, long stopTime,
                     String elapsedTargetString, int elapsedTolerance)
{
    STAFResult llResult = handle.submit2(
        "local", "VAR", "RESOLVE STRING " + LOGLEVELVAR);

    if ((llResult.rc == STAFResult.Ok) &&
        (llResult.result.equalsIgnoreCase(LOGCOMMAND) ||
         llResult.result.equalsIgnoreCase(LOGFILEONLY)))
    {
        String where = "local";
        String service = "LOG";
        String request;
        String logMessage;
        String level = null;
        STAFMarshallingContext mc = new STAFMarshallingContext();

        // Create a map containing the start or stop information to log

        Map logMap = fLogStartStopMapClass.createInstance();
        mc.setMapClassDefinition(fLogStartStopMapClass);

        switch(logType)
        {
            case START:

                logMap.put("loopNum", String.valueOf(loopNum));
                logMap.put("timestamp", getTime(startTime));
                mc.setRootObject(logMap);
                logMessage = mc.marshall();
                
                request = "LOG MACHINE LOGNAME " + logName +
                    " LEVEL start MESSAGE " + STAFUtil.wrapData(logMessage);

                handle.submit2(where, service, request);
                break;

            case STOPPASS:

                level = "PASS";

            case STOPFAIL:

                if (level == null)
                {
                    level = "FAIL";
                }

                /* Log Stop */

                logMap.put("loopNum", String.valueOf(loopNum));
                logMap.put("timestamp", getTime(stopTime));
                mc.setRootObject(logMap);
                logMessage = mc.marshall();

                request = "LOG MACHINE LOGNAME " + logName +
                    " LEVEL stop MESSAGE " + STAFUtil.wrapData(logMessage);

                handle.submit2(where, service, request);

                /* Log Pass/Fail */
                
                // Create a map containing the pass or fail information to log

                Map logMap2 = fLogPassFailMapClass.createInstance();
                logMap2.put("loopNum", String.valueOf(loopNum));
                logMap2.put("elapsedTime",
                            getElapsedTime(startTime, stopTime));
                logMap2.put("elapsedTarget", elapsedTargetString);
                logMap2.put("elapsedTolerance",
                            String.valueOf(elapsedTolerance));
                STAFMarshallingContext mc2 = new STAFMarshallingContext();
                mc2.setRootObject(logMap2);
                mc2.setMapClassDefinition(fLogPassFailMapClass);
                logMessage = mc2.marshall();
                
                request = "LOG MACHINE LOGNAME " + logName + " LEVEL " +
                    level + " MESSAGE " + STAFUtil.wrapData(logMessage);

                handle.submit2(where, service, request);
                break;
        }
    }
}

/**
 * Resolves command parameters and prepares to execute the specified file.
 * Controls number of times file is executed.
 */

private STAFResult handleExecute(
    STAFServiceInterfaceLevel30.RequestInfo reqInfo)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "EXECUTE", fLocalMachineName, reqInfo);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    /* Parse command */

    STAFCommandParseResult pResult = sxeParser.parse(reqInfo.request);

    if (pResult.rc != STAFResult.Ok)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    /* get filename to process */

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(FILE), sHandle, reqInfo.requestNumber);

    if (res.rc != 0) return res;
    
    String filename = res.result;

    /* Get number of times to loop through file */

    int numLoops = 1; //default of 1
    boolean forever = false;

    if (pResult.optionTimes(LOOP) == 1)
    {
        try
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(LOOP), sHandle, reqInfo.requestNumber);

            if (res.rc != 0) return res;
            
            numLoops = Integer.parseInt(res.result);

            if (numLoops <= 0)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid LOOP value.  Must be > 0 or FOREVER.  LOOP=" +
                    pResult.optionValue(LOOP));
            }
        }
        catch (NumberFormatException nfe)
        {
            if (res.result.equalsIgnoreCase(FOREVER))
            {
                // see if forever specified
                forever = true;
            }
            else
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid LOOP value.  Must be > 0 or FOREVER.  LOOP=" +
                    pResult.optionValue(LOOP));
            }
        }
    }

    /* Get minRuntime */

    boolean useMinRuntime = false;
    long minRuntime = 0;

    if (pResult.optionTimes(MINRUNTIME) == 1)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(MINRUNTIME), sHandle, reqInfo.requestNumber);

        if (res.rc != 0) return res;
            
        String minRuntimeString = res.result;

        try
        {
            minRuntime = getTimeInMillis(minRuntimeString + ":0.0");
            useMinRuntime = true;
        }
        catch (NumberFormatException nfe)
        {
            return new STAFResult(
                STAFResult.InvalidValue,
                "Invalid MINRUNTIME value.  Format is HH:MM.  MINRUNTIME=" +
                pResult.optionValue(MINRUNTIME));
        }
    }

    /* Create a new STAFHandle for each file passed to SXE */

    STAFHandle fHandle;

    try
    {
        fHandle = new STAFHandle(filename);
    }
    catch(STAFException se)
    {
        return new STAFResult(STAFResult.STAFRegistrationError,
                              se.toString());
    }

    STAFResult result = null;

    int loop;

    if (forever)
    {
        loop = 1;

        while(true)
        {
            //loop forever
            setSXELoopVar(loop, fHandle);
            result = execute(fHandle, filename, logName, loop);

            if (result.rc != STAFResult.Ok)
            {
                break;
            }

            loop++;
        }
    }
    else if (useMinRuntime)
    {
        //use the minRuntime parameter rather than loop
        loop = 1;
        long tcStartTime = new Date().getTime();

        while(true)
        {
            result = execute(fHandle, filename, logName, loop);
            long tcStopTime = new Date().getTime();

            if (result.rc != STAFResult.Ok)
            {
                break;
            }

            if (tcStopTime - tcStartTime >= minRuntime)
            {
                break;
            }//do checking for minruntime elapsed

            loop++;
        }
    }
    else
    {
        for (loop = 1; loop <= numLoops; loop++)
        {
            //loop set number of times
            setSXELoopVar(loop, fHandle);
            result = execute(fHandle, filename, logName, loop);

            if (result.rc != STAFResult.Ok)
            {
                break;
            }
        }
    }

    try
    {
        fHandle.unRegister();
    }
    catch(STAFException se)
    {
        se.printStackTrace();
    }

    return result;
}

/*
 * Handles a LIST SETTINGS request
 */

private STAFResult handleList(STAFServiceInterfaceLevel30.RequestInfo info)
{
    STAFMarshallingContext mc = new STAFMarshallingContext();

    try
    {
        // Verify that the requesting machine/user has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "LIST", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fListParser.parse(
            info.request);
            
        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        mc.setMapClassDefinition(fSettingsMapClass);

        Map outputMap = fSettingsMapClass.createInstance();
        outputMap.put("logName", logName);

        mc.setRootObject(outputMap);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }
    catch(Exception e)
    {
        return new STAFResult(STAFResult.JavaError, "Internal Java error.");
    }
}


/**
 * Registers return code information with the help service
 */

private int registerHelp(String name)
{
    try
    {
        String request = "REGISTER SERVICE " + name +
            " ERROR " + EXECUTIONERROR +
            " INFO \"" + EXECUTIONERRORInfo +
            "\" DESCRIPTION \""+ EXECUTIONERRORDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);

        request = "REGISTER SERVICE " + name + " ERROR " + ELAPSEDTIMEFAIL +
            " INFO \"" + ELAPSEDTIMEFAILInfo + "\" DESCRIPTION \"" +
            ELAPSEDTIMEFAILDesc+"\"";
        sHandle.submit("LOCAL", "HELP", request);

        request = "REGISTER SERVICE " + name + " ERROR " + ELAPSEDTIMEERROR +
            " INFO \"" + ELAPSEDTIMEERRORInfo + "\" DESCRIPTION \"" +
            ELAPSEDTIMEERRORDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);
    }
    catch(STAFException se)
    {
        return se.rc;
    }

    return STAFResult.Ok;
}


/**
 * Un-registers return code information with the help service
 */

private int unregisterHelp(String name)
{
    try
    {
        String request = "UNREGISTER SERVICE " + name + " ERROR " +
            EXECUTIONERROR;
        sHandle.submit("LOCAL", "HELP", request);

        request = "UNREGISTER SERVICE " + name + " ERROR " + ELAPSEDTIMEFAIL;
        sHandle.submit("LOCAL", "HELP", request);

        request = "UNREGISTER SERVICE " + name + " ERROR " + ELAPSEDTIMEERROR;
        sHandle.submit("LOCAL", "HELP", request);
    }
    catch(STAFException se)
    {
        return se.rc;
    }

    return STAFResult.Ok;
}

/**
 * Executes a staf command using the given handle.
 */

private STAFResult sendSTAFCommand(STAFHandle handle, String command)
{
    STAFResult result;

    String where = command.substring(0, command.indexOf(" "));
    String service = command.substring(
        command.indexOf(" ")+1, command.indexOf(" ", command.indexOf(" ")+1));
    String request = command.substring(
        command.indexOf(" ", command.indexOf(" ")+1)+1);

    return handle.submit2(where, service, request);
}

/**
 * Sets a handle variable "sxeloop" to the specified integer.
 */

private void setSXELoopVar(int loopNum, STAFHandle fHandle)
{
    String request = "SET VAR sxeloop=" + String.valueOf(loopNum);
    fHandle.submit2("LOCAL", "VAR", request);

}

/**
 * Required by implemented STAF interface.
 * This method is called when shutting the service down.
 */

public STAFResult term()
{
    // Un-register Help data

    int rc = unregisterHelp(fServiceName);

    if (rc != STAFResult.Ok)
        return new STAFResult(rc, "Error unregistering HELP data.");

    // Un-register the service handle

    try
    {
        sHandle.unRegister();
    }
    catch (STAFException ex)
    {
        if (rc == STAFResult.Ok)
        {
            rc = STAFResult.STAFRegistrationError;
        }
        return new STAFResult(rc, ex.toString());
    }

    return new STAFResult(rc);
}
}
