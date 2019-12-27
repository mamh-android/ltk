package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: WatchList
//
// Logical Name: WatchList.java
//
// Description: This class is a customized HashTable for storing the 
//              registered watches.
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
import java.io.Serializable;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

public class WatchList extends Hashtable implements Serializable 
{
    /* Do not serialize (transient) this instance variable since
       reference will be different from run to run */
    transient TimerRequestHandler reqHandler;
    
/****************************************************************************/
//
// Method: 
//   Constructor
//
// Description:
//   Constructor method for WatchList class.
//
// Input:
//   aReqHandler - The instance of the TimerRequestHandler creating 
//   this TimerList.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public WatchList(TimerRequestHandler aReqHandler) 
{    
    super();
    
    reqHandler = aReqHandler;
}

/****************************************************************************/
//
// Method: 
//   containsWatch
//
// Description:
//   Determines if the specified watch exists in the list.
//
// Input:
//   watch - The unique String representation of the watch to 
//   look for.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public boolean containsWatch(String watch) 
{
    return this.containsKey(watch);
}

/****************************************************************************/
//
// Method: 
//   getWatchObject
//
// Description:
//   Returns the WatchObject corresponding to the String watch.
//
// Input:
//   watch - The unique String representation of the watch.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public WatchObject getWatchObject(String watch) 
{
    return (WatchObject) this.get(watch);
}


/****************************************************************************/
//
// Method: 
//   listWatches
//
// Description:
//   Returns a marshalled list of all watches in this WatchList.
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

public String listWatches() 
{
    STAFMarshallingContext mc = new STAFMarshallingContext();

    mc.setMapClassDefinition(reqHandler.fWatchMapClass);

    List watchList = new ArrayList();
    
    for (Enumeration e = this.elements(); e.hasMoreElements();) 
    {
        WatchObject wo = (WatchObject) e.nextElement();
        Map watchMap = wo.getProperties();
        watchList.add(watchMap);
    }

    mc.setRootObject(watchList);

    return mc.marshall();
}

/****************************************************************************/
//
// Method: 
//   setReqHandler
//
// Description:
//   Sets the RequestHandler for the WatchList object, since that instance
//   variable cannot be serialized. (Reference changes from run to run.)  Then
//   goes through list and sets the RequestHandler for each WatchObject in list.
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
    /* Set reqHandler for WatchList Object */
    
    reqHandler = aReqHandler;

    /* Set reqHandler for all WatchObjects referenced by this WatchList */
    
    Enumeration elements = this.elements();
    WatchObject wo;

    if (!elements.hasMoreElements()) 
    {
        /* list is empty do not update WatchObjects (there are none) */
        return;
    }

    /* Go through list and set reqHandler on each Watch in list */
    
    do 
    {
        wo = (WatchObject) elements.nextElement();
        wo.setReqHandler(reqHandler);
    }
    while (elements.hasMoreElements());
}

}
