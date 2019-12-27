/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;

class STAXMonitorLogViewer extends JFrame
{
    static String helpText = "\nSTAXMonitorLogViewer Help\n\n" +
        "-name <STAX Log Name>\n" +
        "-machine <STAX Service Machine Name>\n" +
        "-machineNickname <STAX Service Machine Nickname>\n" +
        "-fontName <Font Name>\n" +
        "-saveAsDirectory <Directory Name>\n" +
        "-help\n" + "-version";

    static String kVersion = "3.1.0";

    public static void main(String argv[])
    {
        new STAXMonitorLogViewer(new JFrame(), null, argv);
    }

    public STAXMonitorLogViewer(Component parent, STAFHandle handle,
        String argv[])
    {
        this.parent = parent;

        STAFResult res;

        // If a handle was specified, don't do a system exit
        if (handle != null)
            fSystemExit = false;

        if (argv.length < 1)
        {
            System.out.println("Must specify at least one parameter:  " +
                               "-name, -help, or -version");
            System.out.println(helpText);
            System.exit(1);
        }
        else if (argv.length > 10)
        {
            System.out.println("Too many parameters");
            System.out.println(helpText);
            System.exit(1);
        }
        else
        {
            if (argv[0].equalsIgnoreCase("-HELP"))
            {
                System.out.println(helpText);
                System.exit(0);
            }
            else if (argv[0].equalsIgnoreCase("-VERSION"))
            {
                System.out.println(kVersion);
                System.exit(0);
            }
            else
            {
                for (int i = 0; i < argv.length; i++)
                {
                    if (argv[i] == null)
                    {
                        System.out.println(
                            "Invalid parameter name: " + argv[i]);
                        System.out.println(helpText);
                        System.exit(1);
                    }
                    else if (argv[i].equalsIgnoreCase("-name"))
                    {
                        if ((i+1) > argv.length - 1)
                        {
                            System.out.println(
                                "Parameter -name requires a value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        fLogName = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-machine"))
                    {
                        if ((i+1) >= argv.length)
                        {
                            System.out.println(
                                "Parameter -machine requires a value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        fMachine = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-machineNickname"))
                    {
                        if ((i+1) >= argv.length)
                        {
                            System.out.println(
                                "Parameter -machineNickname requires a value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        fMachineNickname = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-fontName"))
                    {
                        if ((i+1) > argv.length - 1)
                        {
                            System.out.println(
                                "Parameter -fontName requires a value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        fFontName = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-saveAsDirectory"))
                    {
                        if ((i+1) > argv.length - 1)
                        {
                            System.out.println(
                                "Parameter -saveAsDirectory requires a value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        fSaveAsDirectory = argv[i+1];
                        i++;
                    }
                    else
                    {
                        System.out.println(
                            "Invalid parameter name: " + argv[i]);
                        System.out.println(helpText);
                        System.exit(1);
                    }
                }
            }
        }

        if (fLogName == null)
        {
            System.out.println(helpText);
            System.exit(1);
        }
        
        try
        {
            if (handle != null)
            {
                fHandle = handle;
            }
            else
            {
                fHandle = new STAFHandle("STAFLogViewer");
            }
        }
        catch(STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            //e.printStackTrace();

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        String startRecord = new String("");

        if (fLogName.indexOf("_Service") > -1)
        {
            // Find the last start record in the service log file whose
            // message begins with "JobID: 1"

            res = fHandle.submit2(
                fMachine, "LOG", "QUERY MACHINE " + fMachineNickname +
                " LOGNAME " + fLogName + " LEVELMASK Start CSCONTAINS " +
                STAFUtil.wrapData("JobID: 1,") + " LAST 1");

            if (res.rc != 0)
            {
                if (res.rc == 48)
                {
                    JOptionPane.showMessageDialog(
                        parent, "Log " + fLogName + " for machine " +
                        fMachineNickname + " on machine " + fMachine +
                        " does not exist",
                        "Log Does Not Exist",
                        JOptionPane.INFORMATION_MESSAGE);
                }
                else
                {
                    JOptionPane.showMessageDialog(
                        parent, "Error querying log " + fLogName +
                        " for machine " + fMachineNickname + " on machine " +
                        fMachine + ". RC=" + res.rc, "Error Querying Log",
                        JOptionPane.INFORMATION_MESSAGE);
                }

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            // Get the log record's timestamp

            try
            {
                java.util.List outputList = (java.util.List)res.resultObj;

                if (outputList.size() == 0)
                {
                    JOptionPane.showMessageDialog(
                        parent, "Log " + fLogName + " has no entries",
                        "No Log Entries",
                        JOptionPane.INFORMATION_MESSAGE);
                
                    if (! fSystemExit)
                        return;
                    else
                        System.exit(0);
                }

                Map logRecord = (Map)outputList.get(0);
                fStartDateTime = (String)logRecord.get("timestamp");
            }
            catch (Exception e)
            {
                fStartDateTime = null;
                e.printStackTrace();
            }

            if (fStartDateTime == null)
            {
                JOptionPane.showMessageDialog(
                    parent, "Log " + fLogName + " has invalid format.  " +
                    "Log record doesn't contain key: 'timestamp'",
                    "Invalid Log Format",
                    JOptionPane.INFORMATION_MESSAGE);

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            fStartDateTime = fStartDateTime.replace('-', '@');
        }
        else
        {
            if (fLogName.endsWith("_User"))
            {
                fJobLogName = fLogName.substring(0, fLogName.length() - 5);
            }
            else
            {
                fJobLogName = fLogName;
            }

            // Find the last start record in the STAX job log file whose
            // message begins with "JobID: "

            res = fHandle.submit2(
                fMachine, "LOG",
                "QUERY MACHINE " + fMachineNickname + " LOGNAME " +
                fJobLogName + " LEVELMASK Start CSCONTAINS " +
                STAFUtil.wrapData("JobID: ") + " LAST 1");

            if (res.rc != 0)
            {
                if (res.rc == 48)
                {
                    JOptionPane.showMessageDialog(
                        parent, "Log " + fJobLogName + " for machine " +
                        fMachineNickname + " on machine " + fMachine +
                        " does not exist", "Log Does Not Exist",
                        JOptionPane.INFORMATION_MESSAGE);
                }
                else
                {
                    JOptionPane.showMessageDialog(
                        parent, "Error querying log " + fJobLogName +
                        " for machine " + fMachineNickname + " on machine " +
                        fMachine + ". RC=" + res.rc, "Error Querying Log",
                        JOptionPane.INFORMATION_MESSAGE);
                }

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            // Get the log record's timestamp

            try
            {
                java.util.List outputList = (java.util.List)res.resultObj;

                if (outputList.size() == 0)
                {
                    JOptionPane.showMessageDialog(
                        parent, "Log " + fJobLogName + " has no entries",
                        "No Job Log Entries",
                        JOptionPane.INFORMATION_MESSAGE);
                
                    if (! fSystemExit)
                        return;
                    else
                        System.exit(0);
                }

                Map logRecord = (Map)outputList.get(0);
                fStartDateTime = (String)logRecord.get("timestamp");

                String message = (String)logRecord.get("message");
                startRecord = "Started: " + fStartDateTime + ", " + message;
            }
            catch (Exception e)
            {
                fStartDateTime = null;
                e.printStackTrace();
            }
            
            if (fStartDateTime == null)
            {
                JOptionPane.showMessageDialog(
                    parent, "Log " + fJobLogName + " has invalid format.  " +
                    "LogRecord doesn't contain key: 'timestamp'",
                    "Invalid Log Format",
                    JOptionPane.INFORMATION_MESSAGE);

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            fStartDateTime = fStartDateTime.replace('-', '@');
        }

        String startDateTime = "";

        if (fStartDateTime != null)
        {
            startDateTime = " FROM " + fStartDateTime;
        }

        String queryRequest = "QUERY ALL MACHINE " + fMachineNickname +
            " LOGNAME " + fLogName + startDateTime;

        STAFLogViewer logViewer = new STAFLogViewer(
            parent, fHandle, fMachine, "LOG", queryRequest, "", fFontName,
            fSaveAsDirectory);

        // Add job log to the most recently viewed log list

        if (fLogName.indexOf("_Service") == -1)
        {
            String toolTipText = STAXMonitorUtil.wrapText(startRecord, 80);

            if (parent instanceof STAXMonitorFrame)
            {
                ((STAXMonitorFrame)parent).addRecentLog(fLogName, toolTipText);
            }
            else if (parent instanceof STAXMonitor)
            {
                ((STAXMonitor)parent).addRecentLog(fLogName, toolTipText);
            }
            else if (parent instanceof STAXMonitorSubjobExtension)
            {
                ((STAXMonitorSubjobExtension)parent).fMonitorFrame.
                    addRecentLog(fLogName, toolTipText);
            }
            else
            {
                System.out.println(
                    "STAXMonitorLogViewer: Cannot add log to most recently " +
                    "viewed log list because the parent is not a supported" +
                    " object class: " + parent.getClass().getName()); 
            }
        }
    }

    private STAFHandle fHandle;
    private String fMachine = "local";
    private String fMachineNickname = "{STAF/Config/MachineNickname}";
    private String fLogName = null;
    private String fJobLogName;
    private String fStartTimestamp;
    private String fFontName = "Dialog";
    private String fSaveAsDirectory = null;

    boolean fSystemExit = true;

    String fStartDateTime = null;

    Component parent;
}