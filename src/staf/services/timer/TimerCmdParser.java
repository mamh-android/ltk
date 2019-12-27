package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: TimerCmdParser
//
// Logical Name: TimerCmdParser.java
//
// Description: This class parsers commands received from STAF.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
//
/****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import com.ibm.staf.wrapper.STAFLog;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Enumeration;
import java.io.PrintWriter;
import java.io.StringWriter;

public class TimerCmdParser
{
    private Timer fTimer;

    private STAFCommandParser registerParser;
    private STAFCommandParser unregisterParser;
    private STAFCommandParser listParser;
    private STAFCommandParser watchParser;
    private STAFCommandParser unwatchParser;

    private final static String REGISTER = "REGISTER";
    private final static String UNREGISTER = "UNREGISTER";
    private final static String LIST = "LIST";
    private final static String SP = " ";
    private final static String TYPE = "TYPE";
    private final static String FREQUENCY = "FREQUENCY";
    private final static String PRIORITY = "PRIORITY";
    private final static String MACHINE = "MACHINE";
    private final static String HANDLE = "HANDLE";
    private final static String KEY = "KEY";
    private final static String TIMERS = "TIMERS";
    private final static String HELP = "HELP";
    private final static String VERSION = "VERSION";
    private final static String REFRESH = "REFRESH";
    private final static String UNREGONNOPATH = "UNREGONNOPATH";
    private final static String UNREGONNOHANDLE = "UNREGONNOHANDLE";
    private final static String NAME = "NAME";
    private final static String BYNAME = "BYNAME";
    private final static String LONG = "LONG";
    private final static String WATCHES = "WATCHES";
    private final static String WATCH = "WATCH";
    private final static String UNWATCH = "UNWATCH";
    private final static String MARGIN = "MARGIN";

    private final static String INVALIDOPTION =
      "You must have at least 1, but no more than 1 of the options,"+
      " REGISTER UNREGISTER WATCH UNWATCH LIST REFRESH VERSION HELP";

    private final static String BADFREQUENCY =
      "The value for FREQUENCY must be an integer.";

    private final static String BADMARGIN =
      "The value for MARGIN must be an integer.";

    private final static String BADPRIORITY =
      "The value for PRIORITY must be an integer.";

    private final static String BADHANDLE =
      "The value for HANDLE must be an integer.";

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");


/****************************************************************************/
//
// Method:
//   Constructor
//
// Description:
//   Constructor method for TimerCmdParser class.
//
// Input:
//   timer - The instance of the Timer service creating this CmdParser.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public TimerCmdParser(Timer timer)
{
    fTimer = timer;

    /* Create registerParser */

    registerParser = new STAFCommandParser();
    registerParser.addOption(REGISTER, 1, STAFCommandParser.VALUENOTALLOWED);
    registerParser.addOption(TYPE, 1, STAFCommandParser.VALUEREQUIRED);
    registerParser.addOption(FREQUENCY, 1, STAFCommandParser.VALUEREQUIRED);
    registerParser.addOption(KEY, 1, STAFCommandParser.VALUEREQUIRED);
    registerParser.addOption(PRIORITY, 1, STAFCommandParser.VALUEREQUIRED);
    registerParser.addOption(UNREGONNOPATH, 1,
        STAFCommandParser.VALUEREQUIRED);
    registerParser.addOption(UNREGONNOHANDLE, 1,
        STAFCommandParser.VALUEREQUIRED);
    registerParser.addOption(BYNAME, 1, STAFCommandParser.VALUENOTALLOWED);
    registerParser.addOptionGroup(REGISTER, 1, 1);
    registerParser.addOptionGroup(TYPE, 1, 1);
    registerParser.addOptionGroup(FREQUENCY, 1, 1);

    /* Create unregisterParser */

    unregisterParser = new STAFCommandParser();
    unregisterParser.addOption(UNREGISTER, 1,
        STAFCommandParser.VALUENOTALLOWED);
    unregisterParser.addOption(TYPE, 1, STAFCommandParser.VALUEREQUIRED);
    unregisterParser.addOption(MACHINE, 1, STAFCommandParser.VALUEREQUIRED);
    unregisterParser.addOption(HANDLE, 1, STAFCommandParser.VALUEREQUIRED);
    unregisterParser.addOption(NAME, 1, STAFCommandParser.VALUEREQUIRED);
    unregisterParser.addOption(KEY, 1, STAFCommandParser.VALUEREQUIRED);
    unregisterParser.addOption(BYNAME, 1, STAFCommandParser.VALUENOTALLOWED);
    unregisterParser.addOptionGroup(UNREGISTER, 1, 1);
    unregisterParser.addOptionGroup(TYPE, 1, 1);
    unregisterParser.addOptionGroup(NAME+SP+HANDLE, 0, 1);
    unregisterParser.addOptionGroup(MACHINE+SP+BYNAME, 0, 1);
    unregisterParser.addOptionNeed(MACHINE, HANDLE+SP+NAME);
    unregisterParser.addOptionNeed(HANDLE, MACHINE);
    unregisterParser.addOptionNeed(NAME, MACHINE);

    /* Create listParser */

    listParser = new STAFCommandParser();
    listParser.addOption(LIST,      1, STAFCommandParser.VALUENOTALLOWED);
    listParser.addOption(TIMERS,    1, STAFCommandParser.VALUENOTALLOWED);
    listParser.addOption(WATCHES,   1, STAFCommandParser.VALUENOTALLOWED);
    listParser.addOption(LONG, 1, STAFCommandParser.VALUENOTALLOWED);
    listParser.addOption("SETTINGS", 1, STAFCommandParser.VALUENOTALLOWED);
    listParser.addOptionGroup(LIST, 1, 1);
    listParser.addOptionGroup(TIMERS+SP+WATCHES+SP+"SETTINGS", 1, 1);
    listParser.addOptionNeed(LONG, TIMERS);

    /* Create watchParser */

    watchParser = new STAFCommandParser();
    watchParser.addOption(WATCH, 1, STAFCommandParser.VALUENOTALLOWED);
    watchParser.addOption(MACHINE, 1, STAFCommandParser.VALUEREQUIRED);
    watchParser.addOption(FREQUENCY, 1, STAFCommandParser.VALUEREQUIRED);
    watchParser.addOption(MARGIN, 1, STAFCommandParser.VALUEREQUIRED);
    watchParser.addOptionGroup(WATCH, 1, 1);
    watchParser.addOptionGroup(MACHINE, 1, 1);
    watchParser.addOptionGroup(FREQUENCY, 1, 1);

    /* Create unwatchParser */

    unwatchParser = new STAFCommandParser();
    unwatchParser.addOption(UNWATCH, 1, STAFCommandParser.VALUENOTALLOWED);
    unwatchParser.addOption(MACHINE, 1, STAFCommandParser.VALUEREQUIRED);
    unwatchParser.addOptionGroup(UNWATCH, 1, 1);
    unwatchParser.addOptionGroup(MACHINE, 1, 1);

}

/****************************************************************************/
//
// Method:
//   parse
//
// Description:
//   This method parses the first word of the request and handles
//   accordingly.
//
// Input:
//   STAFServiceInterfaceLevel30 request information
//   (including the submitting machine name, the submitting handle name,
//    the submitting handle#, a string containing any command
//    options & option values).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult parse(STAFServiceInterfaceLevel30.RequestInfo info)
{
    try
    {
        STAFResult result = new STAFResult(STAFResult.Ok);

        // Determine the command request (the first word in the request)

        String action;
        int spaceIndex = info.request.indexOf(" ");

        if (spaceIndex != -1)
            action = info.request.substring(0, spaceIndex);
        else
            action = info.request;

        String actionUC = action.toUpperCase();

        // Call the appropriate method to handle the command request

        if (actionUC.equals(REGISTER))
        {
            result = parseRegister(info);
        }
        else if (actionUC.equals(UNREGISTER))
        {
            result = parseUnregister(info);
        }
        else if (actionUC.equals(WATCH))
        {
            result = parseWatch(info);
        }
        else if (actionUC.equals(UNWATCH))
        {
            result = parseUnwatch(info);
        }
        else if (actionUC.equals(LIST))
        {
            result = parseList(info);
        }
        else if (actionUC.equals(HELP))
        {
            // Verify the requester has at least trust level 1

            STAFResult trustResult = STAFUtil.validateTrust(
                1, fTimer.sServiceName, "HELP", fTimer.localMachine, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            result = fTimer.reqHandler.help(fTimer.sServiceName);
        }
        else if (actionUC.equals(VERSION))
        {
            // Verify the requester has at least trust level 1

            STAFResult trustResult = STAFUtil.validateTrust(
                1, fTimer.sServiceName, "VERSION", fTimer.localMachine, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            result = fTimer.reqHandler.version(info);
        }
        else if (actionUC.equals(REFRESH))
        {
            // Verify the requester has at least trust level 3

            STAFResult trustResult = STAFUtil.validateTrust(
                3, fTimer.sServiceName, "REFRESH", fTimer.localMachine, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            result = fTimer.reqHandler.refresh();
        }
        else
        {
            result = new STAFResult(
                STAFResult.InvalidRequestString, INVALIDOPTION);
        }

        return result;
    }
    catch (Throwable t)
    {
        // Write the Java stack trace to the JVM log for the service

        System.out.println(
            sTimestampFormat.format(Calendar.getInstance().getTime()) +
            " ERROR: Exception on " + fTimer.sServiceName +
            " service request:" + fTimer.sLineSep + fTimer.sLineSep +
            info.request + fTimer.sLineSep);

        t.printStackTrace();

        // And also return the Java stack trace in the result

        StringWriter sr = new StringWriter();
        t.printStackTrace(new PrintWriter(sr));

        if (t.getMessage() != null)
        {
            return new STAFResult(
                STAFResult.JavaError,
                t.getMessage() + fTimer.sLineSep + sr.toString());
        }
        else
        {
            return new STAFResult(
                STAFResult.JavaError, sr.toString());
        }
    }
}

/****************************************************************************/
//
// Method:
//   parseList
//
// Description:
//   This method parses parameters for the List command.
//
// Input:
//   STAFServiceInterfaceLevel30 request information
//   (including the submitting machine name, the submitting handle name,
//    the submitting handle#, a string containing any command
//    options & option values).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   private method
//
/****************************************************************************/

private STAFResult parseList(STAFServiceInterfaceLevel30.RequestInfo info)
{
    STAFCommandParseResult pResult = listParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(pResult.rc, pResult.errorBuffer);
    }

    // Verify the requester has at least trust level 2

    STAFResult trustResult = STAFUtil.validateTrust(
        2, fTimer.sServiceName, "LIST", fTimer.localMachine, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    boolean longFormat = false;

    if (pResult.optionTimes(LONG) == 1)
    {
       longFormat = true;
    }

    if (pResult.optionTimes(TIMERS) == 1)
    {
        // Call List Timers request handler
        return fTimer.reqHandler.listTimers(info, longFormat);
    }
    else if (pResult.optionTimes(WATCHES) == 1)
    {
        // Call List Watches request handler
        return fTimer.reqHandler.listWatches(info);
    }
    else if (pResult.optionTimes("SETTINGS") == 1)
    {
        return fTimer.reqHandler.listSettings(info);
    }
    else
    {
        // Should not go here, but return InvalidRequestString
        return new STAFResult(STAFResult.InvalidRequestString);
    }
}

/****************************************************************************/
//
// Method:
//   parseRegister
//
// Description:
//   This method parses parameters for the Register command.
//
// Input:
//   STAFServiceInterfaceLevel30 request information
//   (including the submitting machine name, the submitting handle name.
//    the submitting handle#, a string containing any command
//    options & option values).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   private method
//
/****************************************************************************/

private STAFResult parseRegister(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fTimer.sServiceName, "REGISTER", fTimer.localMachine, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = registerParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(pResult.rc, pResult.errorBuffer);
    }

    int unregOnNoPath = -1;
    int unregOnNoHandle = -1;
    boolean byname = false;
    String key = "";
    int priority = 5; // set default priority
    STAFResult res = new STAFResult();

    // Resolve the TYPE option
    
    res = STAFUtil.resolveRequestVar(
        pResult.optionValue(TYPE), fTimer.sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String name = res.result;

    // Resolve the FREQUENCY option

    res = STAFUtil.resolveRequestVarAndCheckInt(
        FREQUENCY, pResult.optionValue(FREQUENCY),
        fTimer.sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    int frequency = Integer.parseInt(res.result);
    
    // Get optional parameter if specified

    if (pResult.numInstances() > 3)
    {
        // If KEY option is specified, get its value

        if (pResult.optionTimes(KEY) == 1)
        {
            // Resolve the KEY option

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(KEY), fTimer.sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            key = res.result;
        }

        // If priority option exists get value

        if (pResult.optionTimes(PRIORITY) == 1)
        {
            res = STAFUtil.resolveRequestVarAndCheckInt(
                PRIORITY, pResult.optionValue(PRIORITY),
                fTimer.sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            priority = Integer.parseInt(res.result);
        }

        // If unregOnNoPath option exists get value

        if (pResult.optionTimes(UNREGONNOPATH) == 1)
        {
            res = STAFUtil.resolveRequestVarAndCheckInt(
                UNREGONNOPATH, pResult.optionValue(UNREGONNOPATH),
                fTimer.sHandle, info.requestNumber);

            if (res.rc != 0) return res;
            
            unregOnNoPath = Integer.parseInt(res.result);
        }

        // If unregOnNoHandle option exists get value

        if (pResult.optionTimes(UNREGONNOHANDLE) == 1)
        {
            res = STAFUtil.resolveRequestVarAndCheckInt(
                UNREGONNOHANDLE, pResult.optionValue(UNREGONNOHANDLE),
                fTimer.sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            unregOnNoHandle = Integer.parseInt(res.result);
        }

        // If byname option specified register byname

        if (pResult.optionTimes(BYNAME) == 1)
        {
            byname = true;
        }
    }

    // Call Register request handler

    return fTimer.reqHandler.register(
        name, info.endpoint, info.handle, info.handleName, key,
        frequency, priority, unregOnNoPath, unregOnNoHandle, byname);
}

/****************************************************************************/
//
// Method:
//   parseUnregister
//
// Description:
//   This method parses parameters for the Unregister command.
//
// Input:
//   STAFServiceInterfaceLevel30 request information
//   (including the submitting machine name, the submitting handle name,
//    the submitting handle#, a string containing any command
//    options & option values).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   private method
//   Trust level checking is done here because we must know the machine the
//   timer is running on and the machine submitting the request.
//
/****************************************************************************/

private STAFResult parseUnregister(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fTimer.sServiceName, "UNREGISTER", fTimer.localMachine, info);
    
    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = unregisterParser.parse(info.request);
    
    if (pResult.rc != 0)
    {
        return new STAFResult(pResult.rc, pResult.errorBuffer);
    }

    // Set default values

    int reqdTrustLevel = 3;
    boolean byname = false;
    String fMachine = info.endpoint;
    int fHandle = info.handle;
    String fHandleName = info.handleName;
    String key = "";
    STAFResult res = new STAFResult();
    
    // Resolve the TYPE option
    
    res = STAFUtil.resolveRequestVar(
        pResult.optionValue(TYPE), fTimer.sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String name = res.result;

    // Get optional parameters if specified

    if (pResult.numInstances() > 2)
    {
        if (pResult.optionTimes(MACHINE) == 1)
        {
            // Resolve the MACHINE option

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(MACHINE), fTimer.sHandle,
                info.requestNumber);

            if (res.rc != 0) return res;

            fMachine = res.result;

            if (pResult.optionTimes(HANDLE) == 1)
            {
                // Resolve the HANDLE option

                res = STAFUtil.resolveRequestVarAndCheckInt(
                    HANDLE, pResult.optionValue(HANDLE),
                    fTimer.sHandle, info.requestNumber);

                if (res.rc != 0) return res;

                fHandle = Integer.parseInt(res.result);
            }
            else
            {
                // Resolve the NAME option

                res = STAFUtil.resolveRequestVar(
                    pResult.optionValue(NAME), fTimer.sHandle,
                    info.requestNumber);

                if (res.rc != 0) return res;

                fHandleName = res.result;
                byname = true;
            }
        }

        if (pResult.optionTimes(BYNAME) == 1)
        {
            // User specified BYNAME
            byname = true;
        }


        // If KEY option is specified, get its value

        if (pResult.optionTimes(KEY) == 1)
        {
            // Resolve the KEY option

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(KEY), fTimer.sHandle,
                info.requestNumber);

            if (res.rc != 0) return res;

            key = res.result;
        }
    }

    // Create proper timer string for timer to unregister

    String tString;

    if (byname)
        tString = TimerUtils.createTimerString(name, fMachine, fHandleName, key);
    else
        tString = TimerUtils.createTimerString(name, fMachine, fHandle, key);

    // Get the timer object, if it exists

    TimerObject to = fTimer.reqHandler.getTimerObject(tString);
    
    if (to != null)
    {
        // Verify the requester has at least trust level 4 if different
        // machine than the one that previously registered the timer

        if (!info.endpoint.equalsIgnoreCase(to.getMachine()))
        {
            trustResult = STAFUtil.validateTrust(
                4, fTimer.sServiceName, "UNREGISTER", fTimer.localMachine, info);
    
            if (trustResult.rc != STAFResult.Ok) return trustResult;
        }
    }
    
    /* Call UnRegister request handler */

    return fTimer.reqHandler.unregister(tString);
}

/****************************************************************************/
//
// Method:
//   parseUnwatch
//
// Description:
//   This method parses parameters for the Unwatch command.
//
// Input:
//   STAFServiceInterfaceLevel30 request information
//   (including the submitting machine name, the submitting handle name,
//    the submitting handle#, a string containing any command
//    options & option values).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   private method
//   Trust level checking is done here because we must know the machine the
//   timer is running on and the machine submitting the request.
//
/****************************************************************************/

private STAFResult parseUnwatch(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 4

    STAFResult trustResult = STAFUtil.validateTrust(
        4, fTimer.sServiceName, "UNWATCH", fTimer.localMachine, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = unwatchParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(pResult.rc, pResult.errorBuffer);
    }
    
    // Resolve the MACHINE option

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(MACHINE), fTimer.sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String machineToWatch = res.result;

    return fTimer.reqHandler.unwatch(machineToWatch);
}

/****************************************************************************/
//
// Method:
//   parseWatch
//
// Description:
//   This method parses parameters for the Watch command.
//
// Input:
//   STAFServiceInterfaceLevel30 request information
//   (including the submitting machine name, the submitting handle name,
//    the submitting handle#, a string containing any command
//    options & option values).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   private method
//   Trust level checking is done here because we must know the machine the
//   timer is running on and the machine submitting the request.
//
/****************************************************************************/

private STAFResult parseWatch(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fTimer.sServiceName, "WATCH", fTimer.localMachine, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = watchParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(pResult.rc, pResult.errorBuffer);
    }

    // Initialize the values;

    int margin = 0;
    STAFResult res;

    // Resolve the MACHINE option

    res = STAFUtil.resolveRequestVar(
        pResult.optionValue(MACHINE), fTimer.sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String machineToWatch = res.result;

    // Resolve the FREQUENCY option

    res = STAFUtil.resolveRequestVarAndCheckInt(
        FREQUENCY, pResult.optionValue(FREQUENCY),
        fTimer.sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    int frequency = Integer.parseInt(res.result);

    if (pResult.optionTimes(MARGIN) == 1)
    {
        // Resolve the MARGIN option

        res = STAFUtil.resolveRequestVarAndCheckInt(
            MARGIN, pResult.optionValue(MARGIN),
            fTimer.sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        margin = Integer.parseInt(res.result);
    }

    return fTimer.reqHandler.watch(machineToWatch, frequency, margin);
}

}
