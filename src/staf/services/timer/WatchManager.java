package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: WatchManager
//
// Logical Name: WatchManager.java
//
// Description: This class waits to receive notifications from the registered
//              watches and then records the notification.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
//
/****************************************************************************/

import com.ibm.staf.STAFException;
import com.ibm.staf.wrapper.STAFLog;
import com.ibm.staf.STAFQueueMessage;
import java.util.Map;
import java.util.Date;

public class WatchManager extends Thread
{
    private WatchList watchList;
    private volatile Thread wmThread;

    private static String sTimerEndQueueType = "STAF/Service/Timer/End";

/****************************************************************************/
//
// Method:
//   Constructor
//
// Description:
//   Constructor method for WatchManager class.
//
// Input:
//   list - The WatchList instance this WatchManager should manage.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public WatchManager(WatchList list)
{    super();

    watchList = list;

}

/****************************************************************************/
//
// Method:
//   run
//
// Description:
//   The run method the WatchManager class.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   This object runs in its own thread.
//
/****************************************************************************/

public void run()
{
    Thread thisThread = Thread.currentThread();

    while (wmThread == thisThread)
    {
        String result;
        String where = "LOCAL";
        String service = "QUEUE";
        String request = "GET WAIT";

        try
        {
            result = watchList.reqHandler.timer.wHandle.submit(
                where, service, request);
        }
        catch (STAFException e)
        {
            watchList.reqHandler.timer.log.log(
                STAFLog.Error,
                "Error retrieving watch message from queue. RC=" + e.rc);
            continue;
        }

        STAFQueueMessage queueMessage = new STAFQueueMessage(result);
        
        String queueType = queueMessage.type;

        if (queueType.equalsIgnoreCase(sTimerEndQueueType))
        {
            // This type of queue message indicates that the service
            // is terminating

            return;
        }
        else if (queueType.equalsIgnoreCase("STAF/Service/Timer"))
        {
            // Extract the key and timestamp from the queued message

            /* We are going to use the timestamp from the queue service, rather 
               than from the timer service on the requesting machine to avoid
               differences in the system clocks between machines */

            String timeStamp = queueMessage.timestamp;
            Date thisFireTime = TimerUtils.parseDate(timeStamp);
            
            // Get the key if specified (from the message map)

            Map messageMap = (Map)queueMessage.message;
            String key = (String)messageMap.get("key");

            // Get the watch object for the key and update its timestamp

            WatchObject wo = watchList.getWatchObject(key);
       
            try
            {
                wo.setLastTimeStamp(thisFireTime);
            }
            catch (NullPointerException e)
            {
                // No corresponding watch object for this queued message.
                // Ignore message
                continue;
            }
        }
    }
}

/****************************************************************************/
//
// Method:
//   start
//
// Description:
//   This method starts the WatchManager Thread and gives us a reference to
//   the Thread object.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   This object runs in its own thread.
//
/****************************************************************************/

public void start()
{
    if (wmThread == null)
    {
        wmThread = new Thread(this);
        wmThread.start();
    }
}

/****************************************************************************/
//
// Method:
//   term
//
// Description:
//   This method causes the WatchManager Thread to stop running.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   This object runs in its own thread.
//
/****************************************************************************/

public void term()
{
    // Send bogus queue message to watch manager to wake it from the QUEUE GET WAIT
    // command. This will cause the thread to die quickly and stafproc will then be
    // able to quickly shutdown.

    String request = "QUEUE NAME " + watchList.reqHandler.sWatchType +
        " TYPE " + sTimerEndQueueType + " MESSAGE QUIT";

    watchList.reqHandler.sHandle.submit2("local", "QUEUE", request);

    wmThread = null;
}
}
