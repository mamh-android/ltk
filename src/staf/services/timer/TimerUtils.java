package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: TimerUtils
//
// Logical Name: TimerUtils.java
//
// Description: This class provides some static methods for perfoming
//              some common functions.
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
import java.text.SimpleDateFormat;
import com.ibm.staf.*;
import com.ibm.staf.wrapper.STAFLog;
import java.io.*;
import java.text.ParsePosition;

public class TimerUtils
{
    private final static String STAFDateFormat = "yyyyMMdd-HH:mm:ss";

/****************************************************************************/
//
// Method:
//   createTimerString
//
// Description:
//   Static method to create a unique String to represent the timer.
//
// Input:
//   name - The String Name (Type) of this timer.
//   machine - The machine name this timer should notify.
//   handle - The Handle of the process on the machine this
//             timer should notify.
//   key - The key for this timer (or "" if no keyY)
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static String createTimerString(String name, String machine,
                                       int handle, String key)
{
    String result = name + " " + machine + " " + handle + " " + key;
    result = result.toUpperCase();
    return result;

}

/****************************************************************************/
//
// Method:
//   createTimerString
//
// Description:
//   Static method to create a unique String to represent the timer.
//
// Input:
//   name - The String Name (Type) of this timer.
//   machine - The machine name this timer should notify.
//   handleName - The name of the handle on the machine this
//             timer should notify.
//   key - The key for this timer (or "" if no keyY)
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static String createTimerString(String name, String machine,
                                       String handleName, String key)
{
    String result = name + " " + machine + " " + handleName + " " + key;
    result = result.toUpperCase();
    return result;

}

/****************************************************************************/
//
// Method:
//   formatDate
//
// Description:
//   Static method to format the Date Object to a standard STAF
//   date format.
//
// Input:
//   date - The Date to be formatted.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static String formatTime(Date date)
{
    String result;

    if (date == null)
    {
        result = "null";
    }
    else
    {
        SimpleDateFormat formatter = new SimpleDateFormat(STAFDateFormat);
        result = formatter.format(date);
    }

    return result;

}

/****************************************************************************/
//
// Method:
//   parseDate
//
// Description:
//   Static method to parse a Date Object from a standard STAF
//   date formatted String.
//
// Input:
//   formattedDate - A STAF formatted date String.
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static Date parseDate(String formattedDate)
{
    SimpleDateFormat formatter = new SimpleDateFormat(STAFDateFormat);
    Date date = formatter.parse(formattedDate, new ParsePosition(0));

    return date;

}

/****************************************************************************/
//
// Method:
//   readTimerList
//
// Description:
//   Static method to read in serialized timer list and call method to set the
//   RequestHandler reference in the list and all TimerObjects in the list.
//
// Input:
//   reqHandler - Reference to the current RequestHandler
//   fileName - the file name of the serialized TimerList Object
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static TimerList readTimerList(TimerRequestHandler reqHandler,
                                      String fileName)
{
    TimerList list = null;
    FileInputStream in = null;;
    ObjectInputStream s = null;

    try
    {
        in = new FileInputStream(fileName);
        s = new ObjectInputStream(in);
        list = (TimerList) s.readObject();
    }
    catch(FileNotFoundException e)
    {
        // File doesn't exist
        reqHandler.timer.log.log(
            STAFLog.Warning, "File " + fileName + " does not exist. " +
            "Creating new timer list.");
        return null;
    }
    catch(IOException e)
    { 
        // Other IO error
        reqHandler.timer.log.log(
            STAFLog.Error, "Error reading file " + fileName +
            ". Creating new timer list.");
        return null;
    }
    catch(ClassNotFoundException e)
    {
        // Shouldn't ever go here
        reqHandler.timer.log.log(
            STAFLog.Error,
            "ClassNotFoundException thrown attempting to read in persistent" +
            "storage. Creating new timer list.");
        return null;
    }
    finally
    {
        try
        {
            s.close();
            in.close();
        }
        catch(NullPointerException e)
        {
            // Will get thrown when file doesn't exist becuase s will be null
            // Do nothing
        }
        catch(IOException e)
        {
            // IOException closing streams, log
            reqHandler.timer.log.log(
                STAFLog.Warning, "Error closing file " + fileName);
        }
    }

    list.setReqHandler(reqHandler);

    return list;

}

/****************************************************************************/
//
// Method:
//   readWatchList
//
// Description:
//   Static method to read in serialized watch list and call method to set the
//   RequestHandler reference in the list and all WatchObjects in the list.
//
// Input:
//   reqHandler - Reference to the current RequestHandler
//   fileName - the file name of the serialized WatchList Object
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static WatchList readWatchList(TimerRequestHandler reqHandler,
                                      String fileName)
{
    WatchList list = null;
    FileInputStream in = null;
    ObjectInputStream s = null;

    try
    {
        in = new FileInputStream(fileName);
        s = new ObjectInputStream(in);
        list = (WatchList) s.readObject();
    }
    catch(FileNotFoundException e)
    {
        // File doesn't exist
        reqHandler.timer.log.log(
            STAFLog.Warning, "File " + fileName + " does not exist. " +
            "Creating new timer list.");
        return null;
    }
    catch(IOException e)
    {
        // Other IO error
        reqHandler.timer.log.log(
            STAFLog.Error, "Error reading file " + fileName +
            ". Creating new timer list.");
        return null;
    }
    catch(ClassNotFoundException e)
    {
        // Shouldn't ever go here
        reqHandler.timer.log.log(
            STAFLog.Error,
            "ClassNotFoundException thrown attempting to read in persistent" +
            "storage. Creating new watch list.");
        return null;
    }
    finally
    {
        try
        {
            s.close();
            in.close();
        }
        catch(NullPointerException e)
        {
            // Will get thrown when file doesn't exist becuase s will be null
            // Do nothing
        }
        catch(IOException e)
        {
            // IOException closing streams, log
            reqHandler.timer.log.log(STAFLog.Warning,
                "Error closing file "+fileName);
        }
    }

    list.setReqHandler(reqHandler);

    return list;

}

/****************************************************************************/
//
// Method:
//   writeTimerList
//
// Description:
//   Static method to write out serialized timer list to disk for persistent
//   storage.
//
// Input:
//   reqHandler - Reference to the current RequestHandler
//   fileName - the file name of the serialized TimerList Object
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static void writeTimerList(TimerList listToWrite, String fileName)
{
    FileOutputStream out = null;
    ObjectOutputStream s = null;

    try
    {
        out = new FileOutputStream(fileName);
        s = new ObjectOutputStream(out);
        s.writeObject(listToWrite);
        s.flush();
    }
    catch(FileNotFoundException e)
    {
        // Could not open file for writing
        listToWrite.reqHandler.timer.log.log(
            STAFLog.Error, "Could not open file " + fileName +
            " does not exist. Persistent storage of timers failed.");
        return;
    }
    catch(IOException e)
    {
        // Other IO error
        listToWrite.reqHandler.timer.log.log(
            STAFLog.Error, "Error writing to file " + fileName +
            ". Persistent storage of timers failed.");
    }
    finally
    {
        try
        {
            s.close();
            out.close();
        }
        catch(NullPointerException e)
        {
            // Will get thrown if problem creating one of the OutputStreams
            // Do nothing
        }
        catch(IOException e)
        {
            // IOException closing streams, log
            listToWrite.reqHandler.timer.log.log(
                STAFLog.Warning, "Error closing file " + fileName);
        }
    }

}

/****************************************************************************/
//
// Method:
//   writeWatchList
//
// Description:
//   Static method to write out serialized watch list to disk for persistent
//   storage.
//
// Input:
//   reqHandler - Reference to the current RequestHandler
//   fileName - the file name of the serialized WatchList Object
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public static void writeWatchList(WatchList listToWrite, String fileName)
{
    FileOutputStream out = null;
    ObjectOutputStream s = null;

    try
    {
        out = new FileOutputStream(fileName);
        s = new ObjectOutputStream(out);
        s.writeObject(listToWrite);
        s.flush();
    }
    catch(FileNotFoundException e)
    {
        // Could not open file for writing
        listToWrite.reqHandler.timer.log.log(
            STAFLog.Error, "Could not open file " + fileName +
            " does not exist. Persistent storage of timers failed.");
        return;
    }
    catch(IOException e)
    {
        // Other IO error
        listToWrite.reqHandler.timer.log.log(
            STAFLog.Error, "Error writing to file " + fileName +
            ". Persistent storage of timers failed.");
    }
    finally
    {
        try
        {
            s.close();
            out.close();
        }
        catch(NullPointerException e)
        {
            // Will get thrown if problem creating one of the OutputStreams
            // Do nothing
        }
        catch(IOException e)
        {
            // IOException closing streams, log
            listToWrite.reqHandler.timer.log.log(
                STAFLog.Warning, "Error closing file " + fileName);
        }
    }
}
}
