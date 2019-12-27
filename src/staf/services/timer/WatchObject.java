package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: WatchObject
//
// Logical Name: WatchObject.java
//
// Description: This class contains information about each machine registered
//              to be watched.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
// 02/12/2000 C Alkov     Register timers for corresponding watches using 
//                        unregonnopath 0 & unregonnohandle 0 (def:3622)
//
/****************************************************************************/

import java.io.Serializable;
import java.util.Date;
import com.ibm.staf.STAFException;
import com.ibm.staf.STAFResult;
import com.ibm.staf.wrapper.STAFLog;
import java.util.Map;

public class WatchObject implements Serializable 
{
    private String machineToWatch;
    private long frequency;
    private int margin;

    /* Do not serialize (transient) this instance variable since
       reference will be different from run to run */
    private transient TimerRequestHandler reqHandler;
    
    private Date lastTimeStamp = null;
    
/****************************************************************************/
//
// Method: 
//   Constructor
//
// Description:
//   Constructor method for TimerObject class.
//
// Input:
//   aMachineToWatch - the name of the machine to be watched
//   aFrequency - the frequency at which the machine should send notifications
//                to the WatchManager
//   aMargin - the allowable error in the response time of the machine being
//             watched
//   aReqHandler - reference to the TimerRequestHandler object
//
// Exceptions Thrown:
//   STAFException
//
// Notes:
//   none
//
/****************************************************************************/

public WatchObject(String aMachineToWatch, long aFrequency, int aMargin, 
                   TimerRequestHandler aReqHandler) throws STAFException 
{
    super();

    machineToWatch = aMachineToWatch.toUpperCase();
    frequency = aFrequency;
    margin = aMargin;
    reqHandler = aReqHandler;
    
    // Register a timer for machineToWatch.  Specify machineToWatch as the
    // key so that can match a watch with the right registration

    String request = "REGISTER TYPE " + reqHandler.sWatchType +
        " KEY " + machineToWatch + " FREQUENCY " + frequency +
        " UNREGONNOPATH 0 UNREGONNOHANDLE 0 BYNAME";

    String result = reqHandler.timer.wHandle.submit(
        machineToWatch, "TIMER", request);

    /* Set lastTimeStamp to current time */

    setLastTimeStamp(new Date());

    /* Add this timer to watchList */

    reqHandler.watchList.put(machineToWatch, this);
}

/****************************************************************************/
//
// Method: 
//   getProperties
//
// Description:
//   Returns the properties of this watch in a marshalled map for use in the
// LIST WATCHES request.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   synchronized method
//
/****************************************************************************/

public synchronized Map getProperties() 
{
    Map properties;

    properties = reqHandler.fWatchMapClass.createInstance();

    properties.put("machine", machineToWatch);

    if (lastNotificationReceived()) 
        properties.put("status", "OK");
    else 
        properties.put("status", "Missed");
    
    if (lastTimeStamp != null)
        properties.put("lastTimestamp", TimerUtils.formatTime(lastTimeStamp));

    properties.put("frequency", String.valueOf(frequency));
    properties.put("margin", String.valueOf(margin));

    return properties;
}

/****************************************************************************/
//
// Method: 
//   lastNotificationReceived
//
// Description:
//   Private method for checking if the last notification was receieved
//   from the machine being watched
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

private boolean lastNotificationReceived() 
{
    Date OKTime = new Date(lastTimeStamp.getTime() + frequency + margin);
    Date currentTime = new Date();

    if(currentTime.after(OKTime)) 
    {
        return false;
    }
    else 
    {
        return true;
    }
    
}

/****************************************************************************/
//
// Method: 
//   setLastTimeStamp
//
// Description:
//   Sets the value of lastTimeStamp (the last time a notification was
//   received from the machine being watched)
//
// Input:
//   newLastTimeStamp - Date representing the last time the machine being
//                      watched sent a notification message
//
// Exceptions Thrown:
//   none
//
// Notes:
//   synchronized method
//
/****************************************************************************/

public synchronized void setLastTimeStamp(Date newLastTimeStamp) 
{
    lastTimeStamp = newLastTimeStamp;
}

/****************************************************************************/
//
// Method: 
//   setReqHandler
//
// Description:
//   Sets the RequestHandler for the WatchObject.
//
// Input:
//   aReqHandler - Reference to the current RequestHandler
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public void setReqHandler(TimerRequestHandler aReqHandler) 
{    
    reqHandler = aReqHandler;
}

/****************************************************************************/
//
// Method: 
//   update
//
// Description:
//   Updates the frequency and margin of the Watch.
//
// Input:
//   newFrequency - The new frequency of the watch.
//   newMargin - The new margin of the watch.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   synchronized method
//
/****************************************************************************/

public synchronized STAFResult update(long newFrequency, int newMargin) 
{
    frequency = newFrequency;
    margin = newMargin;

    /* Update timer information with machineToWatch */
    
    String request = "REGISTER TYPE " + reqHandler.sWatchType +
        " KEY " + machineToWatch + " FREQUENCY " + frequency + " BYNAME";

    try 
    {
        reqHandler.sHandle.submit(machineToWatch, "TIMER", request);
    }
    catch(STAFException e) 
    {
        reqHandler.timer.log.log(STAFLog.Error, 
            "Error registering a timer with machine: " + machineToWatch +
            " RC:" + e.rc);
        return new STAFResult(e.rc);
    }
    
    return new STAFResult(STAFResult.Ok);
}
}
