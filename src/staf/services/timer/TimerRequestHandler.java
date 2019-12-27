package com.ibm.staf.service.timer;
import com.ibm.staf.service.STAFServiceInterfaceLevel30;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: TimerRequestHandler
//
// Logical Name: TimerRequestHandler.java
//
// Description: This class performs the requested actions of STAF commands
//              after they have been parsed.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
// 04/05/2000 C Alkov     Update version to 1.01 (def:3610,3622)
// 04/27/2004 S Lucas     Updated version to 3.0.0
//
/****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.wrapper.STAFLog;
import java.util.Map;

public class TimerRequestHandler
{
    private int minFrequency;
    boolean unregisterOnNoHandle;
    boolean unregisterOnNoPath;
    int failCountLimit;

    String sName;
    STAFHandle sHandle;
    String sWatchType;
    STAFMapClassDefinition fTimerMapClass;
    STAFMapClassDefinition fTimerLongMapClass;
    STAFMapClassDefinition fWatchMapClass;
    STAFMapClassDefinition fWatchLongMapClass;
    STAFMapClassDefinition fSettingsMapClass;

    String LINESEP;
    private final static String UNEXPECTEDVARRESULT =
        "Unexpected Result from call to VAR service";
    private final static String VERSION = "3.0.3";
    
    private static String sHelpMsg;

    TimerList timerList;
    WatchList watchList;
    Timer timer;
    TimerManager tManager;
    WatchManager wManager;
    
/****************************************************************************/
//
// Method:
//   Constructor
//
// Description:
//   Constructor method for the TimerRequestHandler class.
//
// Input:
//   name - The STAF name of this service.
//   handle - The STAFHandle of this service.
//   aTimer - reference to the main Timer object
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public TimerRequestHandler(String name, STAFHandle handle, Timer aTimer)
{
    sName = name;
    sHandle = handle;
    timer = aTimer;
    sWatchType = "STAF/Service/" + sName + "/Watch";
    
    // Construct map class for a LIST TIMERS request.

    fTimerMapClass = new STAFMapClassDefinition(
        "STAF/Service/Timer/Timer");
    fTimerMapClass.addKey("type", "Type");
    fTimerMapClass.addKey("machine", "Machine");
    fTimerMapClass.addKey("notifyBy", "Notify By");
    fTimerMapClass.setKeyProperty("notifyBy", "display-short-name", "Notify");
    fTimerMapClass.addKey("notifiee", "Notifiee");
    fTimerMapClass.addKey("key", "Key");
    fTimerMapClass.addKey("lastTimestamp", "Last Fired Date-Time");
    fTimerMapClass.setKeyProperty("lastTimestamp", "display-short-name",
                                  "Last Date-Time");
    fTimerMapClass.addKey("frequency", "Frequency");
    fTimerMapClass.setKeyProperty("frequency", "display-short-name", "Freq");

    // Construct map class for a LIST TIMERS LONG request.

    fTimerLongMapClass = new STAFMapClassDefinition(
        "STAF/Service/Timer/TimerLong");
    fTimerLongMapClass.addKey("type", "Type");
    fTimerLongMapClass.addKey("machine", "Machine");
    fTimerLongMapClass.addKey("notifyBy", "Notify By");
    fTimerLongMapClass.addKey("notifiee", "Notifiee");
    fTimerLongMapClass.addKey("key", "Key");
    fTimerLongMapClass.addKey("lastTimestamp", "Last Fired Date-Time");
    fTimerLongMapClass.addKey("frequency", "Frequency");
    fTimerLongMapClass.addKey("priority", "Priority");
    fTimerLongMapClass.addKey("unRegOnNoPath", "Unregister On No Path");
    fTimerLongMapClass.addKey("unRegOnNoHandle", "Unregister On No Handle");

    // Construct map class for a LIST WATCHES request.

    fWatchMapClass = new STAFMapClassDefinition(
        "STAF/Service/Timer/Watch");
    fWatchMapClass.addKey("machine", "Machine To Watch");
    fWatchMapClass.addKey("status", "Status");
    fWatchMapClass.addKey("lastTimestamp", "Last Fired Date-Time");
    fWatchMapClass.setKeyProperty("lastTimestamp", "display-short-name",
                                  "Last Date-Time");
    fWatchMapClass.addKey("frequency", "Frequency");
    fWatchMapClass.setKeyProperty("frequency", "display-short-name", "Freq");
    fWatchMapClass.addKey("margin", "Margin");

    // Construct map-class for LIST SETTINGS information

    fSettingsMapClass = new STAFMapClassDefinition(
        "STAF/Service/Timer/Settings");
    fSettingsMapClass.addKey("directory", "Directory");
    
    // Assign the help text string for the service

    sHelpMsg = "*** " + aTimer.sServiceName + " Service Help ***" +
        aTimer.sLineSep + aTimer.sLineSep +
        "REGISTER   TYPE <Type> FREQUENCY <Milliseconds> [PRIORITY <Level>] [KEY <Key>]" +
        aTimer.sLineSep +
        "           [BYNAME] [UNREGONNOPATH <1 | 0>] [UNREGONNOHANDLE <1 | 0>]" +
        aTimer.sLineSep +
        "UNREGISTER TYPE <Type> [KEY] [BYNAME |" +
        aTimer.sLineSep +
        "           <MACHINE <Machine> HANDLE <Handle #> | NAME <Handle Name>>]" +
        aTimer.sLineSep +
        "WATCH      MACHINE <Machine> FREQUENCY <Milliseconds> [MARGIN <Milliseconds>]" +
        aTimer.sLineSep +
        "UNWATCH    MACHINE <Machine>" +
        aTimer.sLineSep +
        "LIST       <TIMERS [LONG]> | WATCHES | SETTINGS" +
        aTimer.sLineSep +
        "REFRESH" + 
        aTimer.sLineSep +
        "VERSION" +
        aTimer.sLineSep +
        "HELP";

    // Read in timer and watch list, except if we are not using persistent
    // storage

    if (!(timer.tListFileName == null))
    {
        // If tListFileName is null, so is wListFileName
        timerList = TimerUtils.readTimerList(this, timer.tListFileName);
        watchList = TimerUtils.readWatchList(this, timer.wListFileName);
    }
    
    // No timer list create new list 

    if (timerList == null)
    {
        timerList = new TimerList(this);
        watchList = new WatchList(this);
    }

    if (watchList == null)
    {
        watchList = new WatchList(this);
    }

    // Start TimerManager thread

    tManager = new TimerManager(timerList);
    tManager.start();

    // Start WatchMangager thread

    wManager = new WatchManager(watchList);
    wManager.start();
}

/****************************************************************************/
//
// Method:
//   help
//
// Description:
//   Handler method for help request.
//
// Input:
//   machine - name of machine making the request
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult help(String serviceName)
{
    STAFResult result = new STAFResult(STAFResult.Ok, sHelpMsg);

    return result;
}
/****************************************************************************/
//
// Method:
//   listSettings
//
// Description:
//   Handler method for listing the operational settings for the Timer service
//
// Input:
//   STAFServiceInterfaceLevel30
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult listSettings(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // LIST SETTINGS

    STAFMarshallingContext mc = new STAFMarshallingContext();
    mc.setMapClassDefinition(fSettingsMapClass);
    Map outputMap = fSettingsMapClass.createInstance();

    outputMap.put("directory", timer.fDataDir);

    mc.setRootObject(outputMap);

    return new STAFResult(STAFResult.Ok, mc.marshall());
}

/****************************************************************************/
//
// Method:
//   listTimers
//
// Description:
//   Handler method for list timers request.
//
// Input:
//   STAFServiceInterfaceLevel30
//   format - boolean specifying whether to format output
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult listTimers(STAFServiceInterfaceLevel30.RequestInfo info,
                             boolean format)
{
    return new STAFResult(STAFResult.Ok, timerList.listTimers(format));
}

/****************************************************************************/
//
// Method:
//   listWatches
//
// Description:
//   Handler method for list timers request.
//
// Input:
//   STAFServiceInterfaceLevel30
//   format - boolean specifying whether to format output
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult listWatches(STAFServiceInterfaceLevel30.RequestInfo info)
{
    return new STAFResult(STAFResult.Ok, watchList.listWatches());
}

/****************************************************************************/
//
// Method:
//   refresh
//
// Description:
//   Handler method for refresh request.
//
// Input:
//   STAFServiceInterfaceLevel30.RequestInfo
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult refresh()
{
    String where = "LOCAL";
    String service = "VAR";
    STAFResult res;
    STAFResult result = new STAFResult(STAFResult.Ok);

    // Resolve the MinFrequency variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Service/Timer/MinFrequency}", sHandle);

    if (res.rc == STAFResult.Ok)
    {
        try
        {
            minFrequency = Integer.parseInt(res.result);

            if (minFrequency < 0)
            {
                // Set to default
                minFrequency = 1000;
                timer.log.log(STAFLog.Warning, "Invalid value specified "+
                    "for MinFrequency, using default value (1000)");
            }
        }
        catch (NumberFormatException e)
        {
            // Set to default
            minFrequency = 1000;
            timer.log.log(STAFLog.Warning, "Invalid value specified "+
                    "for MinFrequency, using default value (1000)");
        }
    }
    else if (res.rc == STAFResult.VariableDoesNotExist)
    {
        // Set to default
        minFrequency = 1000;
    }
    else
    {
        // Unexpected Result shouldn't ever go here

        result = new STAFResult(res.rc, UNEXPECTEDVARRESULT);
        timer.log.log(STAFLog.Warning, "Unexpected error attempting to "+
                    "resolve MinFrequency, using default value (1000)");
        minFrequency = 1000;
    }

    // Resolve the UnregisterOnNoHandle variable for the local machine

    res = STAFUtil.resolveInitVar(
        "{STAF/Service/Timer/UnregisterOnNoHandle}", sHandle);

    if (res.rc == STAFResult.Ok)
    {
        try
        {
            int iunregisterOnNoHandle = Integer.parseInt(res.result);

            if (iunregisterOnNoHandle == 0)
            {
                unregisterOnNoHandle = false;
            }
            else
            {
                unregisterOnNoHandle = true;
            }
        }
        catch (NumberFormatException e)
        {
            // Set to default
            unregisterOnNoHandle = true;
            timer.log.log(STAFLog.Warning, "Invalid value specified "+
                "for UnregisterOnNoHandle, using default value (true)");
        }
    }
    else if (res.rc == STAFResult.VariableDoesNotExist)
    {
        // Set to default
        unregisterOnNoHandle = true;
    }
    else
    {
        // Unexpected Result shouldn't ever go here
        result = new STAFResult(res.rc, UNEXPECTEDVARRESULT);
        timer.log.log(STAFLog.Warning, "Unexpected error attempting to "+
            "resolve UnregisterOnNoHandle, using default value (true)");
        unregisterOnNoHandle = true;
    }

    // Resolve the UnregisterOnNoPath variable for the local machine

    res = STAFUtil.resolveInitVar(
        "{STAF/Service/Timer/UnregisterOnNoPath}", sHandle);

    if (res.rc == STAFResult.Ok)
    {
        try
        {
            int iunregisterOnNoPath = Integer.parseInt(res.result);

            if (iunregisterOnNoPath == 0)
                unregisterOnNoPath = false;
            else
                unregisterOnNoPath = true;
        }
        catch (NumberFormatException e)
        {
            // Set to default
            unregisterOnNoPath = true;
            timer.log.log(STAFLog.Warning, "Invalid value specified "+
                "for UnregisterOnNoPath, using default value (true)");
        }
    }
    else if (res.rc == STAFResult.VariableDoesNotExist)
    {
        // Set to default
        unregisterOnNoPath = true;
    }
    else
    {
        // Unexpected Result shouldn't ever go here
        result = new STAFResult(res.rc, UNEXPECTEDVARRESULT);
        timer.log.log(STAFLog.Warning, "Unexpected error attempting to "+
            "resolve UnregisterOnNoPath, using default value (true)");
        unregisterOnNoPath = true;
    }

    // Resolve the TotalFileAttempts variable for the local machine

    res = STAFUtil.resolveInitVar(
        "{STAF/Service/Timer/TotalFireAttempts}", sHandle);

    if (res.rc == STAFResult.Ok)
    {
        try
        {
            failCountLimit = Integer.parseInt(res.result);

            if (failCountLimit < 0)
            {
                timer.log.log(STAFLog.Warning, "Negative value specified for "+
                    "TotalFireAttempts, using default value (3)");
                failCountLimit = 3;
            }
        }
        catch (NumberFormatException e)
        {
            // Set to default
            failCountLimit = 3;
            timer.log.log(STAFLog.Warning, "Invalid value specified "+
                "for TotalFireAttempts, using default value (3)");
        }
    }
    else if (res.rc == STAFResult.VariableDoesNotExist)
    {
        // Set to default
        failCountLimit = 3;
    }
    else
    {
        // Unexpected Result shouldn't ever go here
        result = new STAFResult(res.rc, UNEXPECTEDVARRESULT);
        timer.log.log(STAFLog.Warning, "Unexpected error attempting to "+
            "resolve TotalFireAttempts, using default value (3)");
        failCountLimit = 3;
    }

    return result;
}

/****************************************************************************/
//
// Method:
//   register
//
// Description:
//   Handler method for register request.
//
// Input:
//   name - The String Name (Type) of this timer.
//   machine - The machine name this timer should notify.
//   handle - The Handle of the process on the machine this
//             timer should notify.
//   handleName - The handle name of the process this timer should notify.
//   key - The key specified for this timer (or "" if no key is specified)
//   frequency - The frequency of this timer.
//   priority - The priority of the Queue message this timer should
//               send to the registered machine.
//   unregOnNoPath - specifies a timer specific variable for unregisterOnNoPath
//                   a value of -1 means use the global variable (not timer specific)
//   unregOnNoHandle - specifies a timer specific variable for unregisterOnNoHandle
//                     a value of -1 means use the global variable (not timer specific)
//   byname - Boolean that specifies if the BYNAME parameter was specified.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult register(String name, String machine, int handle,
                           String handleName, String key,
                           long frequency, long priority, int unregOnNoPath,
                           int unregOnNoHandle, boolean byname)
{
    // Check for good frequency

    if (frequency < minFrequency)
    {
        return new STAFResult(
            STAFResult.InvalidValue,
            "FREQUENCY value " + frequency + " is less than the minimum " +
            "allowable frequency " + minFrequency);
    }

    // Check for good priority

    if (priority < 0 || priority > (Math.pow(2, 32)-1))
    {
        return new STAFResult(
            STAFResult.InvalidValue,
            "PRIORITY value " + priority + " is invalid.  " +
            "It must be between 0 and 2^32-1");
    }

    // Choose which createTimerString method to call (byname)

    String timerString;

    if (byname)
    {
        timerString = TimerUtils.createTimerString(
            name, machine, handleName, key);
    }
    else
    {
        timerString = TimerUtils.createTimerString(
            name, machine, handle, key);
    }
    
    // Check if this timer is already registered.  If so, return an error.

    if (timerList.getTimerObject(timerString) != null)
    {
        return new STAFResult(STAFResult.AlreadyExists, timerString);
    }

    // Create new timer

    TimerObject timer = new TimerObject(
        name, machine, handle, handleName, key, frequency,
        priority, this, unregOnNoPath, unregOnNoHandle, byname);
    
    // Don't allow update of a timer that already exists anymore.
    //TimerObject timer = timerList.getTimerObject(timerString);
    //timer.update(frequency, priority, unregOnNoPath, unregOnNoHandle);
    
    return new STAFResult(STAFResult.Ok);
}

/****************************************************************************/
//
// Method:
//   term
//
// Description:
//   Called when this service is stopped.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   Stops TimerManger thread, writes out TimerList, and unregisters the
//   STAFHandle.
//
/****************************************************************************/

public int term()
{
    tManager.term();
    wManager.term();

    // Don't try to write if did not create a filename, no persistent storage

    if (!(timer.tListFileName == null))
    {
         // If tListFileName is null so is wListFileName
        TimerUtils.writeTimerList(timerList, timer.tListFileName);
        TimerUtils.writeWatchList(watchList, timer.wListFileName);
    }

    try
    {
        sHandle.unRegister();
        timer.wHandle.unRegister();
    }
    catch (STAFException e)
    {
        timer.log.log(STAFLog.Warning,
                      "Error during unregister; RC = " + e.rc);
        return e.rc;
    }

    return STAFResult.Ok;

}

/****************************************************************************/
//
// Method:
//   unregister
//
// Description:
//   Handler method for unregister request.
//
// Input:
//   timerString - TimerString representing timer to unregister
//
// Exceptions Thrown:
//   none
//
// Notes:
//   The trust level checking is done in TimerCmdParser.parseUnregister()
//   because we must know the machine the timer is running on and the
//   machine submitting the request.
//
/****************************************************************************/

public STAFResult unregister(String timerString)
{
    /* Unregister timer */

    if (timerList.containsTimer(timerString))
    {
        timerList.remove(timerString);
        return new STAFResult(STAFResult.Ok);
    }
    else
    {
        return new STAFResult(
            STAFResult.DoesNotExist,
            "The TYPE specified does not exist for the specified " +
            "machine and handle number/name (and key if specified)");
    }
}

/****************************************************************************/
//
// Method:
//   getTimerObject
//
// Description:
//   Handler method for getting a timer object from the timer list.
//
// Input:
//   timerString - The unique String representation of the timer.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public TimerObject getTimerObject(String timerString)
{
    return timerList.getTimerObject(timerString);
}

/****************************************************************************/
//
// Method:
//   unwatch
//
// Description:
//   Handler method for unregister request.
//
// Input:
//   machineToWatch - name of the machine to be unwatched
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult unwatch(String machineToWatch)
{
    // Unregister watch
    
    String machineUpper = machineToWatch.toUpperCase();

    if (!watchList.containsWatch(machineToWatch.toUpperCase()))
    {
        return new STAFResult(
            STAFResult.InvalidValue,
            "Machine " + machineToWatch + " is not currently being watched");
    }
    
    try
    {
        // Unregister the timer with machineToWatch

        String request = "UNREGISTER TYPE " + sWatchType +
            " BYNAME KEY " + machineUpper;

        timer.wHandle.submit(machineToWatch, "TIMER", request);
    }
    catch (STAFException e)
    {
        timer.log.log(STAFLog.Warning,
            "Error unregistering watch. Could not unregister " +
            "corresponding timer with machine: " + machineToWatch +
            ". RC:"+e.rc);
    }

    watchList.remove(machineUpper);

    return new STAFResult(STAFResult.Ok);
}

/****************************************************************************/
//
// Method:
//   version
//
// Description:
//   Handler method for version request.
//
// Input:
//   STAFServiceInterfaceLevel30.RequestInfo
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult version(STAFServiceInterfaceLevel30.RequestInfo info)
{
    return new STAFResult(STAFResult.Ok, VERSION);
}

/****************************************************************************/
//
// Method:
//   watch
//
// Description:
//   Handler method for the watch request.
//
// Input:
//   machineToWatch - the name of the machine to be watched
//   frequency - The frequency of this watch
//   margin - the allowable error in the response time of the machine being
//            watched
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult watch(String machineToWatch, long frequency, int margin)
{
    String machineToWatchOrg = machineToWatch;

    // Use uppercase for hashtable key

    machineToWatch = machineToWatch.toUpperCase();

    // See if watch already exists

    if ((watchList != null) && (watchList.containsWatch(machineToWatch)))
    {
        // Watch exists, update watch

        return new STAFResult(STAFResult.AlreadyExists, machineToWatchOrg);
        // Don't allow update of existing watch anymore
        //WatchObject watch = watchList.getWatchObject(machineToWatch);
        //watch.update(frequency, margin);
    }
    else
    {
        try
        {
            // Create new watch
            new WatchObject(machineToWatch, frequency, margin, this);
        }
        catch (STAFException e)
        {
            timer.log.log(STAFLog.Error,
                "Error registering a timer with machine: " + machineToWatchOrg +
                " RC:" + e.rc);
            return new STAFResult(
                e.rc, "Error registering a timer with machine: " +
                machineToWatchOrg);
        }
    }

    return new STAFResult(STAFResult.Ok);
}

}
