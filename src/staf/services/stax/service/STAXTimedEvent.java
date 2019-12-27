/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXTimedEvent
{
    public STAXTimedEvent(long notificationTime, STAXTimedEventListener listener)
    {
        fNotificationTime = notificationTime;
        fListener = listener;
    }

    public long getNotificationTime() { return fNotificationTime; }
    public STAXTimedEventListener getTimedEventListener() { return fListener; }

    private long fNotificationTime = 0;
    private STAXTimedEventListener fListener = null;
}
