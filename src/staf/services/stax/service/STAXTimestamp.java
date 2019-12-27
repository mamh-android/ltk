/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.util.Calendar;
import java.text.SimpleDateFormat;

public class STAXTimestamp
{
    private static SimpleDateFormat dateFormat = 
        new SimpleDateFormat("yyyyMMdd");
    private static SimpleDateFormat timeFormat = 
        new SimpleDateFormat("HH:mm:ss");
    private static SimpleDateFormat timestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    public STAXTimestamp()
    {
        // Gets the current date and time
        fCalendar = Calendar.getInstance();
    }

    public Calendar getCalendar()
    {
        return fCalendar;
    } 

    public String getTimestampString() 
    { 
        return timestampFormat.format(fCalendar.getTime()).toString();
    }
    
    public String getDateString()
    {
        return dateFormat.format(fCalendar.getTime()).toString();
    }
    
    public String getTimeString()
    {
        return timeFormat.format(fCalendar.getTime()).toString();
    }

    Calendar fCalendar;
}