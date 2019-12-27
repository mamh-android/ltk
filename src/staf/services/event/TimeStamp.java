/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.event;

import java.text.*;
import java.util.*;

public class TimeStamp
{
    // static data

    transient private static final String TIME_PATTERN  = 
        "{0,number,00}:{1,number,00}:{2,number,00}";

    transient private static final String DATE_PATTERN  = 
        "{0,number,0000}{1,number,00}{2,number,00}";

    // static methods

    public static String currentTime()
    {
        Calendar cal = Calendar.getInstance();

        Object[] args = new Object[] 
        { 
            new Integer(cal.get(cal.HOUR_OF_DAY)), 
            new Integer(cal.get(cal.MINUTE)), 
            new Integer(cal.get(cal.SECOND)) 
        };

        return MessageFormat.format(TIME_PATTERN, args); 
    }

    public static String currentDate()
    {
        Calendar cal = Calendar.getInstance();

        Object[] args = new Object[] 
        { 
            new Integer(cal.get(cal.YEAR)), 
            new Integer(cal.get(cal.MONTH) + 1), 
            new Integer(cal.get(cal.DAY_OF_MONTH))
        };

        return MessageFormat.format(DATE_PATTERN, args);
    }

    public static long currentReal()
    {
        return new Date().getTime();
    }

    // constructors

    public TimeStamp()
    {
        fCreationTime = System.currentTimeMillis();
        fDate = currentDate();
        fTime = currentTime();
    }

    // public methods

    public String date() { return fDate; }
    public String time() { return fTime; }
    public long real() { return fCreationTime; }

    public boolean after(TimeStamp when)
    {
        return (fCreationTime > when.real());
    }

    public boolean before(TimeStamp when)
    {
        return (fCreationTime < when.real());
    }

    public boolean equals(TimeStamp when)
    {
        return (fCreationTime == when.real());
    }

    // private data members

    transient private long fCreationTime;
    transient private String fDate;
    transient private String fTime;
}
