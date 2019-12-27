/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.wrapper;
import com.ibm.staf.*;
import java.util.HashMap;

// STAFLog - This class is a wrapper around the STAF logging service.
//           It provides a set of constants for log types and levels.
//           It provides a log() method to log a message.  It also interfaces
//           with the MONITOR service for a user defined set of levels.  For
//           these level STAFLog will also send the given message to the
//           monitor service.  If there is an error logging, STAFLog will
//           also try to send an error to the MONITOR service.

public class STAFLog
{
    // Log type constants

    public static final String GLOBAL  = "GLOBAL";
    public static final String MACHINE = "MACHINE";
    public static final String HANDLE  = "HANDLE";


    // Log level constants (int format)

    public static final int Fatal     = 0x00000001;
    public static final int Error     = 0x00000002;
    public static final int Warning   = 0x00000004;
    public static final int Info      = 0x00000008;
    public static final int Trace     = 0x00000010;
    public static final int Trace2    = 0x00000020;
    public static final int Trace3    = 0x00000040;
    public static final int Debug     = 0x00000080;
    public static final int Debug2    = 0x00000100;
    public static final int Debug3    = 0x00000200;
    public static final int Start     = 0x00000400;
    public static final int Stop      = 0x00000800;
    public static final int Pass      = 0x00001000;
    public static final int Fail      = 0x00002000;
    public static final int Status    = 0x00004000;
    public static final int Reserved1 = 0x00008000;
    public static final int Reserved2 = 0x00010000;
    public static final int Reserved3 = 0x00020000;
    public static final int Reserved4 = 0x00040000;
    public static final int Reserved5 = 0x00080000;
    public static final int Reserved6 = 0x00100000;
    public static final int Reserved7 = 0x00200000;
    public static final int Reserved8 = 0x00400000;
    public static final int Reserved9 = 0x00800000;
    public static final int User1     = 0x01000000;
    public static final int User2     = 0x02000000;
    public static final int User3     = 0x04000000;
    public static final int User4     = 0x08000000;
    public static final int User5     = 0x10000000;
    public static final int User6     = 0x20000000;
    public static final int User7     = 0x40000000;
    public static final int User8     = 0x80000000;

    // Log level constants (String format)

    public static final String FatalStr     = "FATAL";
    public static final String ErrorStr     = "ERROR";
    public static final String WarningStr   = "WARNING";
    public static final String InfoStr      = "INFO";
    public static final String TraceStr     = "TRACE";
    public static final String Trace2Str    = "TRACE2";
    public static final String Trace3Str    = "TRACE3";
    public static final String DebugStr     = "DEBUG";
    public static final String Debug2Str    = "DEBUG2";
    public static final String Debug3Str    = "DEBUG3";
    public static final String StartStr     = "START";
    public static final String StopStr      = "STOP";
    public static final String PassStr      = "PASS";
    public static final String FailStr      = "FAIL";
    public static final String StatusStr    = "STATUS";
    public static final String Reserved1Str = "RESERVED1";
    public static final String Reserved2Str = "RESERVED2";
    public static final String Reserved3Str = "RESERVED3";
    public static final String Reserved4Str = "RESERVED4";
    public static final String Reserved5Str = "RESERVED5";
    public static final String Reserved6Str = "RESERVED6";
    public static final String Reserved7Str = "RESERVED7";
    public static final String Reserved8Str = "RESERVED8";
    public static final String Reserved9Str = "RESERVED9";
    public static final String User1Str     = "USER1";
    public static final String User2Str     = "USER2";
    public static final String User3Str     = "USER3";
    public static final String User4Str     = "USER4";
    public static final String User5Str     = "USER5";
    public static final String User6Str     = "USER6";
    public static final String User7Str     = "USER7";
    public static final String User8Str     = "USER8";


    // Constructors

    public STAFLog(String type, String name, STAFHandle handle)
    {
        logName = name;
        stafHandle = handle;
        logType = type;

        // This mask enables Fatal, Error, Warning, Start, Stop, Pass, Fail,
        // and Status

        monitorMask = 0x00007C07;
    }

    public STAFLog(String type, String name, STAFHandle handle, int mask)
    {
        logName = name;
        stafHandle = handle;
        logType = type;
        monitorMask = mask;
    }

    // Methods to actually log a message with a given level

    public STAFResult log(int level, String msg)
    {
        return STAFLog.log(stafHandle, logType, logName, level, msg,
                           monitorMask);
    }

    public STAFResult log(String level, String msg)
    {
        return STAFLog.log(stafHandle, logType, logName, level, msg,
                           monitorMask);
    }

    public static STAFResult log(STAFHandle theHandle, String type, String name,
                                 int level, String msg)
    {
        return STAFLog.log(theHandle, type, name, level, msg, 0x00007C07);
    }

    public static STAFResult log(STAFHandle theHandle, String type, String name,
                                 String levelText, String msg)
    {
        return STAFLog.log(theHandle, type, name, levelText, msg, 0x00007C07);
    }

    public static STAFResult log(STAFHandle theHandle, String type, String name,
                                 int level, String msg, int mask)
    {
        String levelString = Integer.toBinaryString(level);

        levelString = nullLevel.substring(0, 32 - levelString.length()) +
                      levelString;

        String logRequest = "LOG " + type + " LOGNAME :" + name.length() +
                            ":" + name + "LEVEL " + levelString +
                            " MESSAGE :" + msg.length() + ":" + msg;

        STAFResult result = theHandle.submit2("LOCAL", "LOG", logRequest);

        if (result.rc != 0)
        {
            STAFMonitor.log(theHandle, "Logging failed, RC: " + result.rc +
                            ", on message: " + msg);
        }
        else if ((mask & level) == level)
        {
            result = STAFMonitor.log(theHandle, msg);
        }

        return result;
    }
    
    public static STAFResult log(STAFHandle theHandle, String type, String name,
                                 String levelText, String msg, int mask)
    {
        // Convert textual version of level (e.g. "info") to Integer version
        Integer levelInt = (Integer)levelMap.get(levelText.toUpperCase());

        // Check if level string not found in levelMap
        if (levelInt == null)
        {
            return new STAFResult(STAFResult.InvalidValue, levelText);
        }

        int level = levelInt.intValue();
        
        String levelString = Integer.toBinaryString(level);

        levelString = nullLevel.substring(0, 32 - levelString.length()) +
                      levelString;

        String logRequest = "LOG " + type + " LOGNAME :" + name.length() +
                            ":" + name + "LEVEL " + levelString +
                            " MESSAGE :" + msg.length() + ":" + msg;

        STAFResult result = theHandle.submit2("LOCAL", "LOG", logRequest);

        if (result.rc != 0)
        {
            STAFMonitor.log(theHandle, "Logging failed, RC: " + result.rc +
                            ", on message: " + msg);
        }
        else if ((mask & level) == level)
        {
            result = STAFMonitor.log(theHandle, msg);
        }

        return result;
    }

    // Accessor methods

    public String getName() { return logName; }
    public String getLogType() { return logType; }
    public int getMonitorMask() { return monitorMask; }

    // Instance variables

    private String logName;
    private String level;
    private STAFHandle stafHandle;
    private String logType;
    private int monitorMask;

    // Private static variables

    private static final String nullLevel = "0000000000000000000000000000000";

    // Log Level Map to convert from String format to int format 
    private static HashMap levelMap = new HashMap();
    
    static
    {
        levelMap.put(FatalStr,     new Integer(Fatal));
        levelMap.put(ErrorStr,     new Integer(Error));
        levelMap.put(WarningStr,   new Integer(Warning));
        levelMap.put(InfoStr,      new Integer(Info));
        levelMap.put(TraceStr,     new Integer(Trace));
        levelMap.put(Trace2Str,    new Integer(Trace2));
        levelMap.put(Trace3Str,    new Integer(Trace3));
        levelMap.put(DebugStr,     new Integer(Debug));
        levelMap.put(Debug2Str,    new Integer(Debug2));
        levelMap.put(Debug3Str,    new Integer(Debug3));
        levelMap.put(StartStr,     new Integer(Start));
        levelMap.put(StopStr,      new Integer(Stop));
        levelMap.put(PassStr,      new Integer(Pass));
        levelMap.put(FailStr,      new Integer(Fail));
        levelMap.put(StatusStr,    new Integer(Status));
        levelMap.put(Reserved1Str, new Integer(Reserved1));
        levelMap.put(Reserved2Str, new Integer(Reserved2));
        levelMap.put(Reserved3Str, new Integer(Reserved3));
        levelMap.put(Reserved4Str, new Integer(Reserved4));
        levelMap.put(Reserved5Str, new Integer(Reserved5));
        levelMap.put(Reserved6Str, new Integer(Reserved6));
        levelMap.put(Reserved7Str, new Integer(Reserved7));
        levelMap.put(Reserved8Str, new Integer(Reserved8));
        levelMap.put(Reserved9Str, new Integer(Reserved9));
        levelMap.put(User1Str,     new Integer(User1));
        levelMap.put(User2Str,     new Integer(User2));
        levelMap.put(User3Str,     new Integer(User3));
        levelMap.put(User4Str,     new Integer(User4));
        levelMap.put(User5Str,     new Integer(User5));
        levelMap.put(User6Str,     new Integer(User6));
        levelMap.put(User7Str,     new Integer(User7));
        levelMap.put(User8Str,     new Integer(User8));
    }
}
