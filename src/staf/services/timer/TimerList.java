package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: TimerList
//
// Logical Name: TimerList.java
//
// Description: This class is a customized HashTable for storing the 
//              registered timers.
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
import java.util.Enumeration;
import java.util.Hashtable;
import java.io.Serializable;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

public class TimerList extends Hashtable implements Serializable 
{
    // Do not serialize (transient) this instance variable since
    // reference will be different from run to run

    transient TimerRequestHandler reqHandler;
    
/****************************************************************************/
//
// Method: 
//   Constructor
//
// Description:
//   Constructor method for TimerList class.
//
// Input:
//   freqHandler - The instance of the TimerRequestHandler creating 
//   this TimerList.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public TimerList(TimerRequestHandler freqHandler) 
{
    super();

    reqHandler = freqHandler;
}

/****************************************************************************/
//
// Method: 
//   containsTimer
//
// Description:
//   Determines if the specified timer exists in the list.
//
// Input:
//   tString - The unique String representation of the timer to 
//   look for.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public boolean containsTimer(String tString) 
{
    return this.containsKey(tString);
    
}

/****************************************************************************/
//
// Method: 
//   getTimerObject
//
// Description:
//   Returns the TimerObject corresponding to the String timerString.
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
    return (TimerObject) this.get(timerString);
    
}


/****************************************************************************/
//
// Method: 
//   timerExists
//
// Description:
//   Returns true if a timer with the specified timer type already exists.
//
// Input:
//   timer type - The timer type (name).
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public boolean timerExists(String type)
{
    for (Enumeration e = this.elements(); e.hasMoreElements();) 
    {
        TimerObject to = (TimerObject)e.nextElement();
        Map timerMap = to.getProperties(false);

        if (((String)timerMap.get("type")).equalsIgnoreCase(type))
            return true;
    }

    return false;
}


/****************************************************************************/
//
// Method: 
//   listTimers
//
// Description:
//   Returns a marshalled list of all timers in this TimerList.
//
// Input:
//   longFormat - boolean set to true if LONG is specified in the LIST request
//                so that more detailed information on each timer is provided
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public String listTimers(boolean longFormat) 
{
    STAFMarshallingContext mc = new STAFMarshallingContext();

    if (longFormat)
        mc.setMapClassDefinition(reqHandler.fTimerLongMapClass);
    else
        mc.setMapClassDefinition(reqHandler.fTimerMapClass);

    List timerList = new ArrayList();
    
    for (Enumeration e = this.elements(); e.hasMoreElements();) 
    {
        TimerObject to = (TimerObject)e.nextElement();
        Map timerMap = to.getProperties(longFormat);
        timerList.add(timerMap);
    }

    mc.setRootObject(timerList);

    return mc.marshall();
}

/****************************************************************************/
//
// Method: 
//   setReqHandler
//
// Description:
//   Sets the RequestHandler for the TimerList object, since that instance
//   variable cannot be serialized. (Reference changes from run to run.)  Then
//   goes through list and sets the RequestHandler for each TimerObject in list.
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
    // Set reqHandler for TimerList Object
    
    reqHandler = aReqHandler;

    // Set reqHandler for all TimerObjects referenced by this TimerList
    
    Enumeration elements = this.elements();
    TimerObject to;

    if (!elements.hasMoreElements()) 
    {
        // List is empty do not update TimerObjects (there are none)
        return;
    }

    // Go through list and set reqHandler on each Timer in list
    
    do 
    {
        to = (TimerObject) elements.nextElement();
        to.setReqHandler(reqHandler);
    }
    while (elements.hasMoreElements());
}

}
