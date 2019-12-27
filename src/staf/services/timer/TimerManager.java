package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: TimerManager
//
// Logical Name: TimerManager.java
//
// Description: This class tracks the registered timers and notifies them
//              when it is time to fire.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
//
/****************************************************************************/

import java.util.Enumeration;
import java.util.Date;

public class TimerManager extends Thread 
{
    private TimerList timerList;
    private volatile Thread tmThread = null;
    
/****************************************************************************/
//
// Method: 
//   Constructor
//
// Description:
//   Constructor method for TimerManager class.
//
// Input:
//   list - The TimerList instance this TimerManager should manage.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public TimerManager(TimerList list) 
{
    super();

    timerList = list;

}

/****************************************************************************/
//
// Method: 
//   run
//
// Description:
//   The run method the TimerManager class.
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

    while (tmThread == thisThread) 
    { 
        Enumeration list = timerList.elements();
        TimerObject to;

        /* Check to see if any of the timers need to fire messages */
        
        while (list.hasMoreElements()) 
        {
            to = (TimerObject) list.nextElement();
            Date nextFireTime = to.getNextFireTime();
            Date currentTime = new Date();
            
            if (nextFireTime.equals(currentTime) || 
                nextFireTime.before(currentTime)) 
            {
                to.fire(); // fire message
            }
        }
        
        list = timerList.elements();
        Date nextOverallFireTime = null;
        Date tempFireTime = null;

        /* This next block calculates how long to wait */
        
        do 
        {
            try 
            {
                to = (TimerObject) list.nextElement();
            }
            catch (java.util.NoSuchElementException e) 
            {
                /* No Elements in list, wait for new TimerObject to wake thread */
                waitForRegister();
                list = timerList.elements(); // get updated list of timers
                continue; // start loop over
            }
            
            tempFireTime = to.getNextFireTime();
            
            if (nextOverallFireTime == null || 
                tempFireTime.before(nextOverallFireTime)) 
            {
                nextOverallFireTime = tempFireTime;
            }
        }
        while (list.hasMoreElements());
        
        Date currentTime = new Date();
        long waitTime = nextOverallFireTime.getTime() - currentTime.getTime();
        waitForRegister(waitTime);

    }

}

/****************************************************************************/
//
// Method: 
//   start
//
// Description:
//   This method starts the TimerManager Thread and gives us a reference to
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
    if (tmThread == null) 
    {
        tmThread = new Thread(this);
        tmThread.start();
    }

}

/****************************************************************************/
//
// Method: 
//   term
//
// Description:
//   This method causes the TimerManager Thread to stop running.
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
    tmThread = null;
}

/****************************************************************************/
//
// Method: 
//   waitForRegister
//
// Description:
//   Waits indefinitely.  A successful call to register will cause the NotifyAll
//   method to be called during construction of the TimerObject.
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

private synchronized void waitForRegister() 
{
    try
    {
        wait();
    }
    catch(InterruptedException e) 
    {
        return; // Should never go here, but if we do simply return
    }
    
}

/****************************************************************************/
//
// Method: 
//   waitForRegister
//
// Description:
//   Waits for specified amount of time.  A successful call to register will 
//   cause the NotifyAll method to be called during construction of the TimerObject.
//
// Input:
//   waitTime - maximum time to wait in milliseconds
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

private synchronized void waitForRegister(long waitTime) 
{
    if (waitTime == 0) 
    {
        /* wait(0) is the same as wait(), so we return to avoid having the
           TimerManager wait forever */
        return;
    }
    try
    {
        wait(waitTime);
    }
    catch(IllegalArgumentException e) 
    {
        return; // wait time is negative, return
    }
    catch(InterruptedException e) 
    {
        return; //should never go here, but if we do simply return
    }
    
}

/****************************************************************************/
//
// Method: 
//   wakeUpManager
//
// Description:
//   Provides method for TimerObject constructor to wake this thread from call
//   to wait method.
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

public synchronized void wakeUpManager() 
{
    notifyAll();
    
}
}
