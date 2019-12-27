package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: TimerObject
//
// Logical Name: TimerObject.java
//
// Description: This class represents the actual timers.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
//
/****************************************************************************/

import java.util.Date;
import com.ibm.staf.*;
import com.ibm.staf.wrapper.STAFLog;
import java.io.Serializable;
import java.io.ObjectInputStream;
import java.util.Map;
import java.util.HashMap;

public class TimerObject implements Runnable, Serializable 
{
    private String name;
    private String timerString;
    private long frequency;
    private long priority;
    private String machine;
    private int handle;
    private String handleName;
    private String key;
    private Date lastFireTime;
    private Date nextFireTime;
    private boolean unregisterOnNoPath;
    private boolean unregisterOnNoHandle;
    private boolean noPathVarSet;
    private boolean noHandleVarSet;
    private boolean byname;
    private int failCount;

    /* Do not serialize (transient) this instance variable since
       reference will be different from run to run */
    private transient TimerRequestHandler reqHandler;
        
/****************************************************************************/
//
// Method: 
//   Constructor
//
// Description:
//   Constructor method for TimerObject class.
//
// Input:
//   fName - The String Name (Type) of this timer.
//   fMachine - The machine name (endpoint) this timer should notify.
//   fHandle - The Handle # of the process on the machine this
//             timer should notify.
//   fHandleName - The handle name of the process this timer should notify.
//   fKey - The key of this timer (or "" if none was specified)
//   fFrequency - The frequency of this timer.
//   fPriority - The priority of the Queue message this timer should
//               send to the registered machine.
//   rHandler - The requestHandlerObject which created this Timer.
//   fUnregOnNoPath - int representing the timer specific variable for
//                    UnregisterOnNoPath
//   fUnregOnNoHandle - int representing the timer specific variable for
//                      UnregisterOnNoHandle
//   fByname - Boolean that specifies if the BYNAME parameter was specified.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public TimerObject(String fName, String fMachine, int fHandle,
                   String fHandleName, String fKey,
                   long fFrequency, long fPriority, 
                   TimerRequestHandler rHandler, int fUnregOnNoPath, 
                   int fUnregOnNoHandle, boolean fByname) 
{    
    name = fName;
    machine = fMachine;
    handle = fHandle;
    handleName = fHandleName;
    key = fKey;
    frequency = fFrequency;
    priority = fPriority;
    lastFireTime = null;
    reqHandler = rHandler;
    noHandleVarSet = false;
    noPathVarSet = false;
    byname = fByname;
    failCount = 0;

    /* Set timer specific values for unregister on no path/handle */

    if (fUnregOnNoPath == 1) 
    {
        unregisterOnNoPath = true;
        noPathVarSet = true;
    }
    else if (fUnregOnNoPath == 0) 
    {
        unregisterOnNoPath = false;
        noPathVarSet = true;
    }

    if (fUnregOnNoHandle == 1) 
    {
        unregisterOnNoHandle = true;
        noHandleVarSet = true;
    }
    else if(fUnregOnNoHandle == 0) 
    {
        unregisterOnNoHandle = false;
        noHandleVarSet = true;
    }
        
    /* create timer string for key */

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

    /* find first fire time */

    nextFireTime = this.findNextFireTime();

    /* add to timer list */

    reqHandler.timerList.put(timerString, this);
    reqHandler.tManager.wakeUpManager();   
}

public String getMachine()
{
    return machine;
}

/****************************************************************************/
//
// Method: 
//   findNextFireTime
//
// Description:
//   Calculates the next time this timer should fire a message.
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

private Date findNextFireTime() 
{
    Date currentDate = new Date();
    long fireTime = currentDate.getTime() + frequency;
    return new Date(fireTime);
    
}

/****************************************************************************/
//
// Method: 
//   fire
//
// Description:
//   This method starts a thread which will fire the message
//   to the process on the machine which was registered for
//   this timer.
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

public synchronized void fire() 
{
    /* Store this fire time and calculate next fire time */

    Date currentTime = new Date();
    lastFireTime = currentTime;
    nextFireTime = new Date(currentTime.getTime() + frequency);

    /* Start thread to send message, using STAF QUEUE service */
    
    Thread thisThread = new Thread(this, timerString);
    thisThread.start();
    
}

/****************************************************************************/
//
// Method: 
//   getNextFireTime
//
// Description:
//   Returns the nextFireTime for this timer.
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

public synchronized Date getNextFireTime() 
{    
    return this.nextFireTime;
    
}

/****************************************************************************/
//
// Method: 
//   getProperties
//
// Description:
//   Returns the properties of this timer in a marshalled map for use in the
//   LIST TIMERS request.
//
// Input:
//   longFormat - boolean set to true if LONG is specified in the LIST request
//                so that more detailed information on each timer is provided
//
// Exceptions Thrown:
//   none
//
// Notes:
//   synchronized method
//
/****************************************************************************/

public synchronized Map getProperties(boolean longFormat) 
{
    Map properties;

    if (longFormat)
        properties = reqHandler.fTimerLongMapClass.createInstance();
    else
        properties = reqHandler.fTimerMapClass.createInstance();

    properties.put("type", name);
    properties.put("frequency", String.valueOf(frequency));
    properties.put("machine", machine);

    if (byname)
    {
        properties.put("notifyBy", "Name");
        properties.put("notifiee", handleName);
    }
    else
    {
        properties.put("notifyBy", "Handle");
        properties.put("notifiee", String.valueOf(handle));
    }

    properties.put("key", key);
    
    if (lastFireTime != null)
        properties.put("lastTimestamp", TimerUtils.formatTime(lastFireTime));

    if (longFormat)
    {
        properties.put("priority", String.valueOf(priority));

        if (noPathVarSet)
            properties.put("unRegOnNoPath",
                           String.valueOf(unregisterOnNoPath));
        else
            properties.put("unRegOnNoPath", "global");

        if (noHandleVarSet)
            properties.put("unRegOnNoHandle",
                           String.valueOf(unregisterOnNoHandle));
        else
            properties.put("unRegOnNoHandle", "global");
    }

    return properties;
}

/****************************************************************************/
//
// Method: 
//   run
//
// Description:
//   The run method of the TimerObject class.  Causes the timer
//   to send a message to the process on the machine which
//   registered the timer.
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

public synchronized void run() 
{
    String where = machine;
    String service = "QUEUE";
    Date currentTime = new Date();
    String formattedTime = TimerUtils.formatTime(currentTime);
    String queueType = "STAF/Service/Timer";

    // Create a marshalled map containing the message

    Map messageMap = new HashMap();
    messageMap.put("timerServiceName", reqHandler.sName);
    messageMap.put("type", name);
    messageMap.put("timestamp", formattedTime);
    messageMap.put("key", key);

    STAFMarshallingContext mc = new STAFMarshallingContext();
    mc.setRootObject(messageMap);
    String message = mc.marshall();

    String request;
    
    if (byname) 
    {
        request = "QUEUE NAME " + handleName + " PRIORITY " + priority +
            " TYPE " + STAFUtil.wrapData(queueType) +
            " MESSAGE " + STAFUtil.wrapData(message);
    }
    else 
    {
        request = "QUEUE HANDLE " + handle + " PRIORITY " + priority +
            " TYPE " + STAFUtil.wrapData(queueType) +
            " MESSAGE " + STAFUtil.wrapData(message);
    }
    
    try 
    {
        String result = reqHandler.sHandle.submit(where, service, request);

        // If this timer is registered byname, see if any processes were
        // available to receive messages on target machine. If not, handle
        // as if HandleDoesNotExist.
           
        if (byname) 
        {
            if (Integer.parseInt(result) == 0) 
            {
                // No processes registered to this handle name
                // See if we should unregister

                if (!this.noHandleVarSet) 
                {
                    // Use global variable

                    if (reqHandler.unregisterOnNoHandle) 
                    {
                        // unregister and log 
                        reqHandler.timer.log.log(
                            STAFLog.Info, "No processes registered to " +
                            "handle name " + handleName +
                            ", unregistering timer: " +  this.timerString);
                        reqHandler.unregister(timerString);
                        return;
                    }
                }
                else if (this.unregisterOnNoHandle) 
                {
                    // Use timer specified variable
                    // Unregister and log

                    reqHandler.timer.log.log(
                        STAFLog.Info, "No processes registered to " +
                        "handle name " + handleName +
                        ", unregistering timer: " + this.timerString);
                    reqHandler.unregister(timerString);
                    return;
                }
                
                // Did not unregister the timer, log error and continue

                reqHandler.timer.log.log(
                    STAFLog.Warning, "No processes registered to handle " +
                    "name " + handleName + " for timer: " + this.timerString);

                return;
            }
        }
    }
    catch (STAFException e) 
    {
        // STAF threw exception, see what problem was and handle appropriately
        
        if(e.rc == STAFResult.HandleDoesNotExist) 
        {
            // Handle does not exist.  See if we should unregister timer.

            if (!this.noHandleVarSet) 
            {
                // Use global variable

                if (reqHandler.unregisterOnNoHandle) 
                {
                    // Unregister and log

                    reqHandler.timer.log.log(
                        STAFLog.Info, "Registered handle does not exist, " +
                        "unregistering timer: " + this.timerString);
                    reqHandler.unregister(timerString);
                    return;
                }
            }
            else if (this.unregisterOnNoHandle) 
            {
                // Use timer specific variable
                // Unregister and log

                reqHandler.timer.log.log(
                    STAFLog.Info, "Registered handle does not exist, " +
                    "unregistering timer: " + this.timerString);
                reqHandler.unregister(timerString);
                return;
            }
            
            // Did not unregister the timer, log error and continue

            reqHandler.timer.log.log(
                STAFLog.Warning, "Registered handle does not exist for " +
                "timer: " + this.timerString);

            return;
        }
        else if (e.rc == STAFResult.NoPathToMachine || 
                 e.rc == STAFResult.CommunicationError) 
         {
            // No path to machine.  See if we should unregister timer.

            if (!this.noPathVarSet) 
            {
                // Use global variable

                if(reqHandler.unregisterOnNoPath) 
                {
                    failCount++; // increment failCount
                    
                    if (failCount >= reqHandler.failCountLimit) 
                    {
                        // Unregister and log

                        reqHandler.timer.log.log(
                            STAFLog.Info, 
                            "Not able to contact registered machine, "+
                            "unregistering timer: "+this.timerString);
                        reqHandler.unregister(timerString);
                        return;
                    }
                    else 
                    {
                        // Do not unregister, just log

                        reqHandler.timer.log.log(
                            STAFLog.Info, 
                            "Not able to contact registered machine, "+
                            "for timer: "+this.timerString+" This is attempt:"
                            +failCount);
                        return;
                    }
                }
            }
            else if (this.unregisterOnNoPath) 
            {
                // Use timer specific variable

                failCount++; // increment failCount
                
                if (failCount >= reqHandler.failCountLimit) 
                {
                    // Unregister and log

                    reqHandler.timer.log.log(
                        STAFLog.Info, 
                        "Not able to contact registered machine, "+
                        "unregistering timer: "+this.timerString);
                    reqHandler.unregister(timerString);
                    return;
                }
                else 
                {
                    // Do not unregister, just log

                    reqHandler.timer.log.log(
                        STAFLog.Info, 
                        "Not able to contact registered machine, "+
                        "for timer: "+this.timerString+" This is attempt:"+
                        failCount);
                    return;
                }
            }

            // Did not unregister the timer, log error and continue

            reqHandler.timer.log.log(
                STAFLog.Warning, 
                "Not able to contact registered machine, "+
                "for timer: "+this.timerString);
        }
        else 
        {
            /* Log error */
            reqHandler.timer.log.log(
                STAFLog.Info, 
                "Unexpected error sending message "+
                "for timer: "+this.timerString+"  RC="+e.rc);
        }
    }
}

/****************************************************************************/
//
// Method: 
//   setReqHandler
//
// Description:
//   Sets the RequestHandler for the TimerObject.
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
//   Updates the frequency and priority of the Timer.
//
// Input:
//   newFrequency - The new frequency of the timer.
//   newPriority - The new priority of the timer.
//   fUnregOnNoPath - specifies a timer specific variable for unregisterOnNoPath
//                    a value of -1 means use the global variable (not timer specific)
//   fUnregOnNoHandle - specifies a timer specific variable for unregisterOnNoHandle
//                      a value of -1 means use the global variable (not timer specific)
//
// Exceptions Thrown:
//   none
//
// Notes:
//   synchronized method
//
/****************************************************************************/

public synchronized void update(long newFrequency, long newPriority, 
                                int fUnregOnNoPath, int fUnregOnNoHandle) 
{
    frequency = newFrequency;
    priority = newPriority;
    noHandleVarSet = false;
    noPathVarSet = false;

    // Set timer specific values for unregister on no path/handle.
    // A value of -1 means to use global value for variable.

    if (fUnregOnNoPath == 1) 
    {
        unregisterOnNoPath = true;
        noPathVarSet = true;
    }
    else if (fUnregOnNoPath == 0) 
    {
        unregisterOnNoPath = false;
        noPathVarSet = true;
    }

    if (fUnregOnNoHandle == 1) 
    {
        unregisterOnNoHandle = true;
        noHandleVarSet = true;
    }
    else if(fUnregOnNoHandle == 0) 
    {
        unregisterOnNoHandle = false;
        noHandleVarSet = true;
    }

    // Update nextFireTime

    if (lastFireTime == null) 
    {
        nextFireTime = findNextFireTime();
    }
    else 
    {
        nextFireTime = new Date(lastFireTime.getTime() + frequency);
    }
}

}
