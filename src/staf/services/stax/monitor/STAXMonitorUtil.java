/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.lang.Math.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import javax.swing.table.*;
import java.lang.reflect.*;
import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.text.SimpleDateFormat;

public class STAXMonitorUtil
{
    static SimpleDateFormat DATE_FORMAT = new SimpleDateFormat(
        "yyyyMMdd-HH:mm:ss");

    private static STAFHandle mainMonitorHandle = null;
        
    private static Vector<Integer> staticHandleVector = new Vector<Integer>();
        
    private static Hashtable<Integer, STAFHandle> nonStaticHandleTable =
        new Hashtable<Integer, STAFHandle>();

    private static boolean debug = false;

    private static long timeOffset = 0;

    public static STAFHandle getNewSTAFHandle(String name) throws STAFException
    {
        // First ensure that we have a mainMonitorHandle
        // Next try to see if the user has ALLOWMULTIREG configured
        // Next try a static handle               
        
        STAFException allowMultiRegException = null;

        if (mainMonitorHandle == null)
        {
            try
            {
                mainMonitorHandle = new STAFHandle("STAXMonitor/MainHandle");
                
                if (debug)
                    System.out.println("Created mainMonitorHandle: " + 
                        mainMonitorHandle.getHandle());
            }
            catch (STAFException e)
            {
                // could not create the mainMonitorHandle
                throw e;
            }
        }
    
        STAFHandle newHandle = null;
            
        try
        {
            newHandle = new STAFHandle(name);
            
            if (newHandle.getHandle() == mainMonitorHandle.getHandle())
            {
                // ALLOWMULTIREG is not set;
                newHandle = null;
            }
            else
            {            
                nonStaticHandleTable.put(new Integer(newHandle.getHandle()),
                    newHandle);
            
                if (debug)
                    System.out.println("Created multireg handle " + name + 
                        ": " + newHandle.getHandle()); 
            }
        }
        catch (STAFException e)
        {
            // ALLOWMULTIREG is not set, newHandle will be null
            // Save the exception since we may  need to throw it
            allowMultiRegException = e;
        }
            
        if (newHandle == null)
        {
            STAFResult createHandleResult = mainMonitorHandle.submit2(
                "local", "HANDLE", "CREATE HANDLE NAME " + STAFUtil.wrapData(name));                
                
            if (createHandleResult.rc == 0)
            {
                try 
                {                                
                    newHandle = new STAFHandle(new 
                        Integer(createHandleResult.result).intValue());
                }
                catch (NoSuchMethodError ex)
                {
                    throw allowMultiRegException;
                }
                    
                staticHandleVector.add(new Integer(createHandleResult.result));
                
                if (debug)
                    System.out.println("Created static handle " + name + 
                        ": " + newHandle.getHandle());
            }
            else
            {
                if (debug)
                    System.out.println("RC:" + createHandleResult.rc + " " + 
                        createHandleResult.result);
                        
                throw allowMultiRegException;
            }
        }
            
        return newHandle;
    }
    
    public static void freeHandle(int handle)
    {
        if (staticHandleVector.contains(new Integer(handle)))
        {
            staticHandleVector.remove(new Integer(handle));        

            STAFResult deleteHandleResult = mainMonitorHandle.submit2(
                "local", "HANDLE", "DELETE HANDLE " + handle);
            
            if (debug)
            {
                System.out.println("Deleting handle " + handle);
                
            }

            if (deleteHandleResult.rc != 0)
            {
                System.out.println("Error deleting handle " + handle +
                    "rc=" + deleteHandleResult.rc + " " + 
                    deleteHandleResult.result);                                          
            }            
        } 
        else if (nonStaticHandleTable.containsKey(handle))
        {
            STAFHandle unRegHandle = nonStaticHandleTable.get(handle);
            
            try
            {
                if (debug)
                    System.out.println("Unregistering handle " + handle);
                    
                unRegHandle.unRegister();
            }
            catch (STAFException e)
            {
                // Ignore since this can occur if STAF is shutdown before
                // the STAX Monitor
            }

            nonStaticHandleTable.remove(handle);
        }
    }
    
    public static void cleanupHandles()
    {
        if (!(staticHandleVector.isEmpty()))
        {
            for (int i = 0; i < staticHandleVector.size(); i++)
            {
                int handle = staticHandleVector.elementAt(i).intValue();

                staticHandleVector.remove(new Integer(handle));

                STAFResult deleteHandleResult = 
                    mainMonitorHandle.submit2(
                        "local", "handle", "delete handle " + handle);
                    
                if (debug)
                {
                    System.out.println("Cleaning up handle " + handle);
                }
                
                if (deleteHandleResult.rc != 0)
                {
                    System.out.println("Error deleting handle " + handle +
                        " rc=" + deleteHandleResult.rc + " " + 
                        deleteHandleResult.result);
                }
            }
        }
        
        Enumeration<STAFHandle> nonStaticHandles =
            nonStaticHandleTable.elements();
        
        while (nonStaticHandles.hasMoreElements())
        {
            STAFHandle handle = nonStaticHandles.nextElement();
            
            try
            {
                if (debug)
                {
                    System.out.println("Cleaning up handle " + 
                        handle.getHandle());
                }
                        
                handle.unRegister();
            }
            catch (STAFException e)
            {
                // Ignore since can happen if STAF is shutdown before
                // the STAX Monitor is exited
            }
        }
    }
    
    public static void unregisterMainHandle() throws STAFException
    {
        mainMonitorHandle.unRegister();
        
        if (debug)
            System.out.println("Unregistering main handle " + 
                mainMonitorHandle.getHandle());
    }
    
    public static void sizeColumnsToFitText(JTable table)
    {    
        int tableWidth = 0;
        FontMetrics metrics = table.getFontMetrics(table.getFont());
        
        for (int i = 0; i < table.getColumnCount(); i++)
        {
            int width = 0;
            int maxWidth = 0;
            Vector<Object> data = new Vector<Object>();
            data.addElement(table.getColumnModel().getColumn(i).
                getHeaderValue());
                
            for (int j = 0; j < table.getRowCount(); j++)
            {
                try
                {
                    Object obj = table.getValueAt(j,i);
                    String cellText = "";

                    if (obj != null)
                    {
                        cellText = table.getValueAt(j,i).toString();
                    }                    

                    BufferedReader reader = 
                        new BufferedReader(new StringReader(cellText));
                    String line;

                    try
                    {
                        while ((line = reader.readLine()) != null)
                        {
                            data.addElement(line);
                        }
                    }
                    catch(IOException ex)
                    {
                        ex.printStackTrace();
                    }
                    finally
                    {
                        try
                        {
                            reader.close();
                        }
                        catch (IOException ex)
                        {
                            ex.printStackTrace();
                        }
                    }
                }
                catch(Exception ex) 
                {
                    ex.printStackTrace();
                }
            }
            
            Enumeration e = data.elements();
            
            while (e.hasMoreElements())
            {
                width = metrics.stringWidth(e.nextElement().toString());
                if (width > maxWidth)
                {
                    maxWidth = width;
                }
            }
            Insets insets = 
                ((JComponent)table.getCellRenderer(0,i)).getInsets();

            // need to pad a little extra for everything to look right

            if (maxWidth > 100)
            {
                maxWidth += insets.left + insets.right + (maxWidth*.10);
            }
            else
            {
                maxWidth += insets.left + insets.right + (maxWidth*.30);
            }
            
            table.getColumnModel().getColumn(i).setPreferredWidth(maxWidth);
            
            tableWidth += maxWidth;                       
        }
                      
        Dimension d = table.getSize();
        d.width = tableWidth;        
        table.setSize(d);
    }

    public static String getElapsedTime(Calendar started)
    {
        if (started == null)
        {
            return "";
        }
        
        SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");
                                
        SimpleDateFormat dayOfYearFormat = new SimpleDateFormat("D");
                            
        SimpleDateFormat yearFormat = new SimpleDateFormat("y");

        long startMillis = started.getTime().getTime();

        long currentMillis = System.currentTimeMillis();
        
        long elapsedMillis = currentMillis - startMillis + timeOffset;
        long seconds = elapsedMillis / 1000 % 60;
        long minutes = elapsedMillis / 1000 / 60 % 60;
        long hours   = elapsedMillis / 1000 / 60 / 60;
        //long hours = elapsedMillis / 1000 / 60 / 60 % 24;
        //long days = elapsedMillis / 1000 / 60 / 60 / 24;
        
        String elapsedTime = "";
                            
        /*
        if (days > 0)
        {
            elapsedTime = (new Long(days)).toString();
                                    
            if (days == 1)
                elapsedTime += " day, ";
            else
                elapsedTime += " days, ";
        }
        */
            
        elapsedTime += getTimeFormat((new Long(hours)).toString()) + ":";
        elapsedTime += getTimeFormat((new Long(minutes)).toString()) + ":";
        elapsedTime += getTimeFormat((new Long(seconds)).toString());
        
        return elapsedTime;
    }
    
    public static String getTimeFormat(String in)
    {
        if (in.length() == 1)
        {
            return "0" + in;
        }
        else
        {
            return in;
        }
    }
    
    public static void updateRowHeights(JTable table, int multiLineColumn)
    {       
        for (int i = 0 ; i < table.getRowCount() ; i++)
        {
            String value = (String)table.getValueAt(i, multiLineColumn);

            if (value == null)
                value = new String("");
            
            JTextArea textArea = new JTextArea(value);

            table.setRowHeight(i, textArea.getPreferredSize().height + 5);
        }
    }    
    
    public static void updateRowHeights(JTable table, int multiLineColumn,
                                        String fontName)
    {       
        for (int i = 0 ; i < table.getRowCount() ; i++)
        {
            String value = (String)table.getValueAt(i, multiLineColumn);

            if (value == null)
                value = new String("");
            
            JTextArea textarea = new JTextArea(value);
            textarea.setFont(new Font(fontName, Font.PLAIN, 12));

            table.setRowHeight(i, textarea.getPreferredSize().height + 5);
        }
    }    
    
    public static Calendar getCalendar(String result)
    {        
        int index = result.indexOf("Start Date");
        index = result.indexOf(":", index); 
        String startDate = result.substring(index + 2, index + 10);
        
        index = result.indexOf("Start Time");
        index = result.indexOf(":", index);         
        String startTime = result.substring(index + 2, index + 10);            
            
        int year = (new Integer(startDate.substring(0, 4))).intValue();
        int month = (new Integer(startDate.substring(4, 6))).intValue();
        int date = (new Integer(startDate.substring(6, 8))).intValue();
            
        int hours = (new Integer(startTime.substring(0, 2))).intValue();
        int minutes = (new Integer(startTime.substring(3, 5))).intValue();
        int seconds = (new Integer(startTime.substring(6, 8))).intValue();
           
        Calendar startCalendar = Calendar.getInstance();
        startCalendar.set(year, month - 1, date, hours, minutes, seconds);
        
        return startCalendar;
    }
    
    public static Calendar getCalendar2(String startDate, String startTime)
    {
        int year = (new Integer(startDate.substring(0, 4))).intValue();
        int month = (new Integer(startDate.substring(4, 6))).intValue();
        int date = (new Integer(startDate.substring(6, 8))).intValue();
            
        int hours = (new Integer(startTime.substring(0, 2))).intValue();
        int minutes = (new Integer(startTime.substring(3, 5))).intValue();
        int seconds = (new Integer(startTime.substring(6, 8))).intValue();
           
        Calendar startCalendar = Calendar.getInstance();
        startCalendar.set(year, month - 1, date, hours, minutes, seconds);
        
        return startCalendar;
    }
    
    // This method checks gets the current monitor message, if any, for
    // the process machine/handle.  It supports process machines that are
    // running STAF V3.x or STAF V2.x.

    public static String getMonitorMessage(String location, String handle)
    {
        String message = new String("");

        // Submit the monitor query request specifying the machine nickname

        String request = "QUERY MACHINE {STAF/Config/MachineNickname} " +
            "HANDLE " + handle;

        STAFResult result = mainMonitorHandle.submit2(
            location, "MONITOR", request);

        if (result.rc != 0)
        {
            if (result.rc == STAFResult.VariableDoesNotExist)
            {
                if (result.result.indexOf("MachineNickname") != -1)
                {
                    // Process machine must be running STAF 2.x.
                    // Try again, this time specifying the effective machine

                    request = "QUERY MACHINE {STAF/Config/EffectiveMachine} " +
                        "HANDLE " + handle;

                    result = mainMonitorHandle.submit2(
                        location, "MONITOR", request);
                }

                if (result.rc == STAFResult.VariableDoesNotExist)
                {
                    System.out.println(
                        "STAXMonitorUtil::getMonitorMessage: " +
                        "STAF " + location + " MONITOR " + request +
                        " failed with RC=" + result.rc + " Result=" +
                        result.result);
                }
            }

            if (result.rc != 0)
            {
                if (debug)
                    System.out.println(
                        "RC:" + result.rc + " " + result.result);

                return message;
            }
        }

        if (STAFMarshallingContext.isMarshalledData(result.result))
        {
            // STAF 3.x - Get message from the result object map

            Map resultsMap = (Map)result.resultObj;

            message = (String)resultsMap.get("message");
        }
        else
        {
            // STAF V2.x results are not marshalled

            message = result.result.substring(result.result.indexOf(" ") + 1);
        }

        return message;
    }

    public static void showErrorDialog(Component parent, String message)
    {
        showErrorDialog(parent, message, "STAX Error");
    }

    public static void showErrorDialog(Component parent, String message,
                                       String title)
    {
        showErrorDialog(parent, message, title,
                        new Font("Dialog", Font.BOLD, 12));
    }

    public static void showErrorDialog(Component parent, String message,
                                       Font font)
    {
        showErrorDialog(parent, message, "STAX Error", font);
    }

    public static void showErrorDialog(Component parent, String message,
                                       String title, Font font)
    {
        JTextPane messagePane = new JTextPane();
        messagePane.setFont(font);
        messagePane.setEditable(false);
        messagePane.setText(message);
        messagePane.select(0,0);
        JScrollPane scrollPane = new JScrollPane(messagePane);

        // Calculate the height for the scrollPane based on the number
        // of characters in the message

        int minHeight = 300;      // Minimum height for scrollPane
        int maxHeight = 650;      // Maximum height for scrollPane
        int avgCharsPerLine = 40; // Avg characters per line
        
        int lineHeight = 16;      // Approximate height of a line

        if (font.getName().equals("Courier"))
        {
            avgCharsPerLine = 30;
            lineHeight = 24;
        }

        int msgLength = message.length();
        int numLines = 0;
        int height = minHeight;

        if (msgLength > 0)
        {
            numLines = msgLength / avgCharsPerLine;

            if (numLines > 0) height = numLines * lineHeight;
        }

        if (height > maxHeight)
            height = maxHeight;
        else if (height < minHeight)
            height = minHeight;

        scrollPane.setPreferredSize(new Dimension(750, height));

        JOptionPane.showMessageDialog(parent, scrollPane, title,
                                      JOptionPane.ERROR_MESSAGE);
    }

    public static void assignProcessInfo(
        Map processMap, Vector<Vector<String>> procDataVector)
    {
        // Input parameters
        //   processMap    :  Contains the process information
        // Output parameters
        //   procDataVector:  Gets assigned the process information as it
        //                    should be viewed in the STAX Monitor

        addRow(procDataVector, "Location",
               (String)processMap.get("location"));

        addRow(procDataVector, "Handle",
               (String)processMap.get("handle"));

        addRow(procDataVector, "Command",
               (String)processMap.get("command"));
        
        // Assign command mode

        String commandMode = (String)processMap.get("commandMode");
        addRow(procDataVector, "Command Mode", commandMode);
        
        // Assign command shell

        String commandShell = (String)processMap.get("commandShell");

        if (commandShell != null && !commandShell.equals(""))
            addRow(procDataVector, "Command Shell", commandShell);
        
        // Assign title

        String processTitle = (String)processMap.get("title");

        if (processTitle != null && !processTitle.equals(""))
            addRow(procDataVector, "Title", processTitle);

        // Assign parameters

        String parms = (String)processMap.get("parms");

        if (parms != null && !parms.equals(""))
            addRow(procDataVector, "Parms", parms);

        // Assign working directory

        String workdir = (String)processMap.get("workdir");

        if (workdir != null && !workdir.equals(""))
            addRow(procDataVector, "Workdir", workdir);

        // Assign workload

        String workload = (String)processMap.get("workload");

        if (workload != null && !workload.equals(""))
            addRow(procDataVector, "Workload", workload);

        // Assign variable list

        if (processMap.get("varList") instanceof java.util.List)
        {
            java.util.List varList = (java.util.List)processMap.get("varList");

            for (int i = 0; i < varList.size(); i++)
            {
                addRow(procDataVector, "Var #" + (i + 1),
                       (String)varList.get(i));
            }
        }

        // Assign environment variable list

        if (processMap.get("envList") instanceof java.util.List)
        {
            java.util.List envList = (java.util.List)processMap.get("envList");

            for (int i = 0; i < envList.size(); i++)
            {
                addRow(procDataVector, "Env #" + (i + 1),
                       (String)envList.get(i));
            }
        }

        // Assign useProcessVars

        String useProcessVars = (String)processMap.get("useProcessVars");

        if (useProcessVars != null)
            addRow(procDataVector, "UseProcessVars", "");

        // Assign stop using method

        String stopusing = (String)processMap.get("stopUsing");

        if (stopusing != null && !stopusing.equals(""))
            addRow(procDataVector, "StopUsing", stopusing);

        // Assign same/new console

        String console = (String)processMap.get("console");

        if (console != null)
        {
            if (console.equals("same"))
                addRow(procDataVector, "SameConsole", "");
            else if (console.equals("new"))
                addRow(procDataVector, "NewConsole", "");
        }

        // Assign focus

        String focus = (String)processMap.get("focus");
        if (focus != null)
        {
            addRow(procDataVector, "Focus", focus);
        }

        // Assign user name

        String username = (String)processMap.get("userName");

        if (username != null && !username.equals(""))
            addRow(procDataVector, "Username", username);

        // Assign password

        String password = (String)processMap.get("password");

        if (password != null && !password.equals(""))
            addRow(procDataVector, "Password", password);

        // Assign disabled authentication action

        String disabledauth = (String)processMap.get("disabledAuth");

        if (disabledauth != null)
        {
            if (disabledauth.equalsIgnoreCase("error"))
                addRow(procDataVector, "DisabledAuthIsError", "");
            else if (disabledauth.equalsIgnoreCase("ignore"))
                addRow(procDataVector, "IgnoreDisabledAuth", "");
        }

        // Assign stdint

        String stdin = (String)processMap.get("stdin");

        if (stdin != null && !stdin.equals(""))
            addRow(procDataVector, "Stdin", stdin);
        
        // Assign stdout file name

        String stdout = (String)processMap.get("stdoutFile");

        if (stdout != null && !stdout.equals(""))
            addRow(procDataVector, "Stdout", stdout);
        
        // Assign stdout mode

        String stdoutMode = (String)processMap.get("stdoutMode");

        if (stdoutMode != null && stdoutMode.equalsIgnoreCase("append"))
            addRow(procDataVector, "StdoutAppend", "");

        // Assign stderr file name and mode

        String stderr = (String)processMap.get("stderrFile");

        if (stderr != null && !stderr.equals(""))
            addRow(procDataVector, "Stderr", stderr);

        // Assign stderr mode

        String stderrMode = (String)processMap.get("stderrMode");

        if (stderrMode != null)
        {
            if (stderrMode.equalsIgnoreCase("stdout"))
                addRow(procDataVector, "StderrToStdout", "");
            else if (stderrMode.equalsIgnoreCase("append"))
                addRow(procDataVector, "StderrAppend", "");
        }

        // Assign return stdout

        String returnstdout = (String)processMap.get("returnStdout");

        if (returnstdout != null)
            addRow(procDataVector, "ReturnStdout", "");
        
        // Assign return stderr

        String returnstderr = (String)processMap.get("returnStderr");

        if (returnstderr != null)
            addRow(procDataVector, "ReturnStderr", "");
        
        // Assign return file list

        if (processMap.get("returnFileList") instanceof java.util.List)
        {
            java.util.List returnFileList = (java.util.List)processMap.get(
                "returnFileList");

            for (int i = 0; i < returnFileList.size(); i++)
            {
                addRow(procDataVector, "ReturnFile #" + (i + 1),
                       (String)returnFileList.get(i));
            }
        }

        // Assign static handle name

        String statichandlename = (String)processMap.get("staticHandleName");

        if (statichandlename != null)
            addRow(procDataVector, "StaticHandleName", statichandlename);

        // Assign other

        String other = (String)processMap.get("other");

        if (other != null && !other.equals(""))
            addRow(procDataVector, "Other", other);
        
        // Assign start date and time

        String startTimestamp = (String)processMap.get("startTimestamp");

        if (startTimestamp != null && !startTimestamp.equals(""))
        {
            String startDate = startTimestamp.substring(0, 8);
            String startTime = startTimestamp.substring(9);

            addRow(procDataVector, "Started", startDate + "-" + startTime);
        }
    }

    // This method will wrap the input string at the specified column,
    // placing a newline character at the first whitespace character to the
    // left of the column position.  If the current section of the input string
    // contains no whitespace characters, then a newline character will be
    // placed at the next whitespace character to the right of the column
    // position.

    public static String wrapText(String inputString, int wrapColumn)
    {
        int i = wrapColumn;
        StringBuffer textBuffer = new StringBuffer(inputString);
        int lastWhitespaceIndex = 0;

        while (i < textBuffer.length())
        {
            if (Character.isWhitespace(textBuffer.charAt(i)))
            {
                textBuffer.setCharAt(i, '\n');
                lastWhitespaceIndex = i;
                i = i + wrapColumn;
            }
            else
            {
                i = i - 1;

                if (i == lastWhitespaceIndex)
                {
                    // No whitespace in the current block, so find the next
                    // whitespace character to the right of the current block

                    int j = i + wrapColumn;

                    while (j < textBuffer.length())
                    {
                        if (Character.isWhitespace(textBuffer.charAt(j)))
                        {
                            textBuffer.setCharAt(j, '\n');
                            lastWhitespaceIndex = j;
                            i = j + wrapColumn;
                            break;
                        }

                        j = j + 1;
                    }

                    if (j == textBuffer.length())
                    {
                        break;
                    }
                }
            }
        }

        return textBuffer.toString();
    }

    public static void setTimeOffset(long offset)
    {
        timeOffset = offset;
    }

    private static void addRow(Vector<Vector<String>> vector,
                               String name, String value)
    {
        Vector<String> newRow = new Vector<String>(2);
        newRow.add(name);
        newRow.add(value);
        vector.add(newRow);
    }

}
