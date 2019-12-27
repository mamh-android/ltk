/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

import java.util.Map;

/**
 * This class is used to inspect and manipulate a message retrieved via the
 * STAF Queue service. It takes the marshalled string result from a GET request
 * submitted to the Queue service (a marshalled
 * &lt;Map:STAF/Service/Queue/Entry>), and unmarshalls it and breaks it into
 * its constituent parts including the queued message itself and its type,
 * priority, timestamp, machine, handle name, and handle number.
 *
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFQueueMsg">
 *      Section "3.4.15 Class STAFQueueMessage" in the STAF Java User's
 *      Guide</a>
 */
public class STAFQueueMessage
{
    /**
     * This constructs a wrapper to receive the marshalled queue message string
     * as input by unmarshalling it and breaking it into separate fields
     * including the queued message itself and its type, priority, timestamp,
     * machine, handle name, and handle number. 
     * 
     * @param  qMessage A String containing the marshalled string result from
     *                  a GET request submitted to the Queue service that
     *                  returns one message (e.g. a marshalled
     *                  &lt;Map:STAF/Service/Queue/Entry>),
     */
    public STAFQueueMessage(String qMessage)
    {
        // Unmarshall the result from a QUEUE GET request

        mc = STAFMarshallingContext.unmarshall(qMessage);

        Map queueMap = (Map)mc.getRootObject();

        try
        {
            priority = Integer.parseInt((String)queueMap.get("priority"));
        }
        catch (NumberFormatException e)
        {
            priority =  -1;
        }

        timestamp  = (String)queueMap.get("timestamp");
        machine    = (String)queueMap.get("machine");
        handleName = (String)queueMap.get("handleName");
            
        try
        {
            handle = Integer.parseInt((String)queueMap.get("handle"));
        }
        catch (NumberFormatException e)
        {
            handle =  -1;
        }

        type = (String)queueMap.get("type");
        message = queueMap.get("message");
    }

    /**
     * An unsigned long value (0 - 4294967296) representing the importance of
     * the message, with 0 representing the most important message.
     */ 
    public int    priority;

    /**
     * The date/time the message was received
     */ 
    public String timestamp;

    /**
     * The machine that sent the message
     */ 
    public String machine;

    /**
     * The registered name of the handle which sent the message
     */ 
    public String handleName;

    /**
     * The handle number of the handle which sent the message
     */ 
    public int    handle;

   /**
    * The type of the queued message
    */
    public String type = null;

    /**
     * The actual message itself
     */ 
    public Object message;

    /**
     * The STAF marshalling context for the marshalled queue message string
     */ 
    public STAFMarshallingContext mc;
}
