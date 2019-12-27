/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import com.ibm.staf.*;

public class STAFDemoController extends JFrame
{
    public static void main(String []argv)
    {
        STAFDemoController app = new STAFDemoController();

        // Dimension dim = app.getPreferredSize();
        // app.setSize(dim.width, dim.height);
        app.setSize(300, 200);
        app.setVisible(true);
    }

    static STAFHandle handle;
    static String stafMachineName;
    static int numResources = 2;

    // This version of STAFDemo can only communicate with machines
    // running STAF V3.x.
    static int mainVersion = 3; 

    JPanel stafStartPanel;
    String targetJavacmd;
    String targetClasspath;

    public STAFDemoController()
    {
        super("STAF Demo Controller");

        try
        {
            handle = new STAFHandle("STAFDemo_Controller");
        }
        catch(STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            System.exit(e.rc);
        }

        STAFResult stafResult = handle.submit2(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Machine}");

        if (stafResult.rc != STAFResult.Ok)
        {
            System.out.println(
                "Error getting local machine name RC: " + stafResult.rc +
                " Result: " + stafResult.result);
        }

        stafMachineName = stafResult.result;

        stafResult = handle.submit2(
            "local", "RESPOOL", "DELETE POOL STAFDemo CONFIRM FORCE");

        if ((stafResult.rc != STAFResult.Ok) &&
            (stafResult.rc != STAFResult.DoesNotExist))
        {
            System.out.println(
                "Error deleting STAFDemo resource pool RC: " + stafResult.rc +
                " Result: " + stafResult.result);
        }

        stafResult = handle.submit2(
            "local", "RESPOOL", "CREATE POOL STAFDemo DESCRIPTION " +
            STAFUtil.wrapData("STAF Demo Resource Pool"));

        if (stafResult.rc != STAFResult.Ok)
        {
            System.out.println(
                "Error creating STAFDemo resource pool RC: " + stafResult.rc +
                " Result: " + stafResult.result);
        }

        for (int i = 1; i <= numResources; ++i)
        {
            stafResult = handle.submit2(
                "local", "RESPOOL", "ADD POOL " + "STAFDemo ENTRY Resource" +
                String.valueOf(i));

            if (stafResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    "Error adding resource to  STAFDemo RC: " +
                    stafResult.rc + " Result: " + stafResult.result);
            }
        }

        ProcessEndThread procThread = new ProcessEndThread();
        procThread.start();

        stafStartPanel = new STAFStartPanel();

        getContentPane().add(stafStartPanel);

        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent event)
            {
                try
                {
                    handle.submit2(
                        "local", "QUEUE",
                        "QUEUE HANDLE " + handle.getHandle() +
                        " TYPE STAF/STAFDemo/Stop MESSAGE " +
                        STAFUtil.wrapData(""));
                    handle.unRegister();
                }
                catch(STAFException e)
                {
                    System.out.println(
                        "Error unregistering with STAF, RC: " + e.rc);
                }

                dispose();
                System.exit(0);
            }
        });
    }

    class ProcessEndThread extends Thread
    {
        public void run()
        {
            for(;;)
            {
                STAFResult stafResult = STAFDemoController.handle.submit2(
                    "local", "QUEUE", "GET WAIT");

                int index = 0;

                // Unmarshall the result from a QUEUE GET request

                STAFMarshallingContext outputContext =
                    STAFMarshallingContext.unmarshall(stafResult.result);

                Map queueMap = (Map)outputContext.getRootObject();

                String type = (String)queueMap.get("type");

                if (type != null)
                {
                    if (type.equalsIgnoreCase("STAF/Process/End"))
                    {
                        String theHandle = (String)queueMap.get("handle");
                        String theMachine = (String)queueMap.get("machine");

                        STAFDemoController.handle.submit2(
                            theMachine, "PROCESS", "FREE HANDLE " + theHandle);
                    }
                    else if (type.equalsIgnoreCase("STAF/STAFDemo/Stop"))
                    {
                        return;
                    }
                }
            }
        }
    }

    public class STAFStartPanel extends JPanel
    {
        JTextField fMachine = new JTextField(25);
        JTextField fHandle = new JTextField(25);
        JButton fStart = new JButton("Start");
        JButton fConnect = new JButton("Connect");
        JLabel fNumResourcesLabel = new JLabel();
        JButton fAddResource = new JButton("+");
        JButton fDeleteResource = new JButton("-");

        public STAFStartPanel()
        {
        // Setup display
            GridBagLayout gbl = new GridBagLayout();
            GridBagConstraints gbc = new GridBagConstraints();

            setLayout(gbl);

            gbc.anchor = GridBagConstraints.WEST;
            gbc.fill   = GridBagConstraints.HORIZONTAL;
            gbc.gridwidth = GridBagConstraints.REMAINDER;

            // Setup Process portion of panel
            JPanel processPanel = new JPanel();
            processPanel.setBorder(new TitledBorder("Process"));
            processPanel.setLayout(new GridBagLayout());
            GridBagConstraints procGBC = new GridBagConstraints();

            procGBC.anchor = GridBagConstraints.WEST;
            procGBC.fill   = GridBagConstraints.HORIZONTAL;
            procGBC.gridwidth = 1;
            processPanel.add(new JLabel("Machine:"), procGBC);

            processPanel.add(Box.createHorizontalStrut(5));
            procGBC.gridwidth = GridBagConstraints.REMAINDER;
            fMachine.setText(stafMachineName);
            processPanel.add(fMachine, procGBC);

            procGBC.gridwidth = 1;
            processPanel.add(fStart, procGBC);

            processPanel.add(Box.createHorizontalStrut(5));
            procGBC.gridwidth = GridBagConstraints.REMAINDER;
            processPanel.add(new JLabel("a new process"), procGBC);

            procGBC.gridwidth = 1;
            processPanel.add(fConnect, procGBC);

            processPanel.add(Box.createHorizontalStrut(5));
            processPanel.add(new JLabel("to handle:"));

            processPanel.add(Box.createHorizontalStrut(5));
            procGBC.gridwidth = GridBagConstraints.REMAINDER;
            processPanel.add(fHandle, procGBC);

            add(processPanel, gbc);

            // Setup ResPool panel
            JPanel respoolPanel = new JPanel();
            respoolPanel.setBorder(new TitledBorder("Resource Pool"));

            respoolPanel.add(new JLabel("Number of resource:"));
            respoolPanel.add(Box.createHorizontalStrut(5));
            fNumResourcesLabel.setText(
                String.valueOf(STAFDemoController.numResources));
            respoolPanel.add(fNumResourcesLabel);
            respoolPanel.add(Box.createHorizontalStrut(10));
            respoolPanel.add(fAddResource);
            respoolPanel.add(fDeleteResource);

            add(respoolPanel, gbc);

            // Setup button actions
            fStart.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    String machineName = fMachine.getText();

                    STAFResult stafResult = handle.submit2(
                        machineName, "MISC", "VERSION");

                    if (stafResult.rc == STAFResult.Ok)
                    {
                        String versionString = stafResult.result;

                        int version = 0;

                        try
                        {
                            version = Integer.parseInt(
                                versionString.substring(0, 1));
                        }
                        catch(NumberFormatException e)
                        { /* Do Nothing */ }

                        if (version < mainVersion)
                        {
                            String errMsg =
                                "\nMachine " + machineName +
                                " is running STAF Version " + versionString +
                                ".\nThis version of STAFDemo requires " +
                                "STAF Version " + mainVersion +
                                ".0.0 or later";

                            JOptionPane.showMessageDialog(stafStartPanel,
                                errMsg, "Error starting process",
                                JOptionPane.ERROR_MESSAGE);

                            return;
                        }
                    }
                    else
                    {
                        String errMsg =
                            "\nThis machine could not communicate with "
                            + "machine " + machineName + "\nRC: " +
                            stafResult.rc + " Result: " + stafResult.result;

                        JOptionPane.showMessageDialog(stafStartPanel,
                            errMsg, "Error starting process",
                            JOptionPane.ERROR_MESSAGE);

                        return;
                    }

                    String sepPath = ";";
                    String sepFile = "/";
                    String jstafDir = "";

                    stafResult = handle.submit2(
                        machineName, "VAR",
                        "RESOLVE" +
                        " STRING {STAF/Config/Sep/Path}" +
                        " STRING {STAF/Config/Sep/File}" +
                        " STRING {STAF/Config/OS/Name}");

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        String errMsg = "\nThis machine could not resolve " +
                            "STAF variables on machine " + machineName +
                            "\nRC: " + stafResult.rc +
                            " Result: " + stafResult.result;

                        JOptionPane.showMessageDialog(stafStartPanel,
                            errMsg, "Error starting process",
                            JOptionPane.ERROR_MESSAGE);

                        return;
                    }

                    // Unmarshall the marshalled list of var resolve results

                    STAFMarshallingContext mc =
                        STAFMarshallingContext.unmarshall(stafResult.result);

                    java.util.List varList = (java.util.List)mc.getRootObject();

                    for (int i = 0; i < varList.size(); i++)
                    {
                        Map varInfo = (Map)varList.get(i);

                        String varRC = (String)varInfo.get("rc");

                        if (!varRC.equals("0"))
                        {
                            String errMsg =
                                "\nThis machine could not resolve a STAF " +
                                "variable on machine " + machineName +
                                "\nRC: " + varInfo.get("rc") +
                                " Result: " + varInfo.get("result");

                            JOptionPane.showMessageDialog(stafStartPanel,
                                errMsg, "Error starting process",
                                JOptionPane.ERROR_MESSAGE);

                            return;
                        }
                        
                        switch (i)
                        {
                            case 0: sepPath = (String)varInfo.get("result");
                            case 1: sepFile = (String)varInfo.get("result");
                            case 2:
                            {
                                String osName = (String)varInfo.get("result");

                                if (osName.startsWith("Win"))
                                {
                                    targetJavacmd = "javaw";
                                    jstafDir = "bin";
                                }
                                else
                                {
                                    targetJavacmd = "java";
                                    jstafDir = "lib";
                                }
                            }
                            default: break;
                        }
                    }  

                    stafResult = handle.submit2(
                        machineName, "FS",
                        "QUERY ENTRY {STAF/Config/STAFRoot}/samples/demo/" +
                        "STAFDemo.jar");

                    if (stafResult.rc == STAFResult.DoesNotExist)
                    {
                        // copy the STAFDemo.jar file

                        stafResult = handle.submit2(
                            "local", "FS",
                            "COPY FILE {STAF/Config/STAFRoot}/samples/demo/" +
                            "STAFDemo.jar TOMACHINE " + machineName);

                        if (stafResult.rc != STAFResult.Ok)
                        {
                            System.out.println(
                                "Copy file RC: " + stafResult.rc +
                                " Result: " + stafResult.result);
                        }
                    }

                    String stafDemoJarFile = "{STAF/Config/STAFRoot}" +
                        sepFile + "samples" + sepFile + "demo" + sepFile +
                        "STAFDemo.jar";

                    String jStafJarFile = "{STAF/Config/STAFRoot}" + sepFile +
                        jstafDir + sepFile + "JSTAF.jar";

                    targetClasspath = "{STAF/Env/CLASSPATH}" + sepPath +
                        stafDemoJarFile + sepPath + jStafJarFile;

                    stafResult = handle.submit2(
                        machineName, "PROCESS", "START COMMAND " +
                        targetJavacmd + " " + "PARMS STAFProcess " +
                        "VAR STAFDemo/ResourcePoolMachine=" + stafMachineName +
                        " VAR STAF/Service/Log/Mask=Error" +
                        " ENV CLASSPATH=" + targetClasspath + " NOTIFY ONEND");

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        String errMsg = "Error starting process.  RC:" +
                            stafResult.rc + " Result:" + stafResult.result;

                        if (stafResult.rc == STAFResult.AccessDenied)
                        {
                            errMsg += "\nThis error code indicates that "
                                + "machine " + machineName + " has not given"
                                + " this machine a TRUST level of 5";
                        }
                        else if (stafResult.rc == STAFResult.NoPathToMachine)
                        {
                            errMsg += "\nThis error code indicates that this"
                                + " machine could not communication with "
                                + "machine " + machineName;
                        }

                        JOptionPane.showMessageDialog(stafStartPanel,
                            errMsg, "Error starting process",
                            JOptionPane.ERROR_MESSAGE);

                        return;
                    }

                    String theHandle = stafResult.result;

                    stafResult = handle.submit2(
                        machineName, "LOG",
                        "DELETE MACHINE {STAF/Config/MachineNickname} HANDLE " +
                        theHandle + " LOGNAME STAFDemo CONFIRM");

                    stafResult = handle.submit2(
                        machineName, "SEM", "POST EVENT STAFDemo/Handle/" +
                        theHandle + "/Continue");

                    STAFProcessFrame frame = new STAFProcessFrame(
                        machineName, theHandle);
                    
                    frame.setSize(425, 270);
                    frame.setVisible(true);
                }
            });

            fConnect.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFProcessFrame frame = new STAFProcessFrame(
                        fMachine.getText(), fHandle.getText());

                    frame.setSize(425, 270);
                    frame.setVisible(true);
                }
            });

            fAddResource.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = handle.submit2(
                        "local", "RESPOOL",
                        "ADD POOL STAFDemo ENTRY Resource" +
                        String.valueOf(STAFDemoController.numResources + 1));

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error adding resource RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }

                    fNumResourcesLabel.setText(
                        String.valueOf(++STAFDemoController.numResources));
                }
            });

            fDeleteResource.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = handle.submit2(
                        "local", "RESPOOL", "REMOVE POOL STAFDemo " +
                        "ENTRY Resource" +
                        String.valueOf(STAFDemoController.numResources) +
                        " CONFIRM FORCE");

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error removing resource RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }

                    fNumResourcesLabel.setText(
                        String.valueOf(--STAFDemoController.numResources));
                }
            });
        }
    }

    public class STAFProcessFrame extends JFrame
    {
        public STAFProcessFrame(String machine, String handle)
        {
            super("Machine: " + machine + " Handle: " + handle);

            getContentPane().add(new STAFProcessPanel(this, machine, handle));

            addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
                {
                    dispose();
                }
            });
        }
    }

    public class STAFProcessPanel extends JPanel
    {
        String fMachine;
        String fHandle;
        JButton fStop = new JButton("Stop");
        JButton fPause = new JButton("Pause");
        JButton fContinue = new JButton("Continue");
        JTextField fMonitorRefreshRate = new JTextField(25);
        JLabel fMonitorDate = new JLabel("Unknown");
        JLabel fMonitorStatus = new JLabel("Unknown");
        JButton fSetBackground = new JButton("Set");
        JComboBox fColorList = new JComboBox();
        MonitorThread fMonitorThread;
        STAFProcessFrame fProcessFrame;
        JCheckBox fErrorLog = new JCheckBox("Error");
        JLabel fErrorCount = new JLabel("0");
        JCheckBox fWarningLog = new JCheckBox("Warning");
        JLabel fWarningCount = new JLabel("0");
        JCheckBox fInfoLog = new JCheckBox("Info");
        JLabel fInfoCount = new JLabel("0");
        JCheckBox fDebugLog = new JCheckBox("Debug");
        JLabel fDebugCount = new JLabel("0");
        JList fLogList = new JList();
        JButton fSetLogMask = new JButton("Set Log Mask");
        JButton fRefreshLog = new JButton("Refresh Totals");
        JButton fClearLog = new JButton("Clear Log");
        JButton fDisplayLog = new JButton("Display Log");

        public STAFProcessPanel(STAFProcessFrame frame, String machine,
                                String handle)
        {
            fProcessFrame = frame;
            fMachine = machine;
            fHandle = handle;
            
            // Setup display
            GridBagLayout gbl = new GridBagLayout();
            GridBagConstraints gbc = new GridBagConstraints();

            setLayout(gbl);

            gbc.anchor = GridBagConstraints.WEST;
            // gbc.fill = GridBagConstraints.HORIZONTAL;
            gbc.fill = GridBagConstraints.BOTH;
            gbc.gridwidth = 1;
            gbc.gridx = 1;
            gbc.gridy = 1;

            // Setup Process/Semaphore panel
            JPanel processSemPanel = new JPanel();
            processSemPanel.setBorder(new TitledBorder("Queue / Semaphore"));
            processSemPanel.add(fStop);
            processSemPanel.add(fPause);
            processSemPanel.add(fContinue);
            add(processSemPanel, gbc);

            // Setup variable panel
            JPanel varPanel = new JPanel();
            varPanel.setBorder(new TitledBorder("Variable"));
            varPanel.add(fSetBackground);
            varPanel.add(new JLabel("background color to"));
            fColorList.addItem("Blue");
            fColorList.addItem("Green");
            fColorList.addItem("Red");
            fColorList.addItem("Yellow");
            varPanel.add(fColorList);

            // gbc.gridwidth = GridBagConstraints.REMAINDER;
            gbc.gridy = 2;
            add(varPanel, gbc);

            // Setup monitor panel
            JPanel monPanel = new JPanel();
            monPanel.setLayout(new GridBagLayout());
            GridBagConstraints monGBC = new GridBagConstraints();

            monGBC.anchor = GridBagConstraints.WEST;
            monGBC.fill = GridBagConstraints.HORIZONTAL;
            monGBC.gridwidth = 1;

            monPanel.setBorder(new TitledBorder("Monitor"));

            monGBC.fill = GridBagConstraints.NONE;
            monGBC.gridwidth = 3;
            monPanel.add(new JLabel("Refresh rate (sec.):"), monGBC);
            monPanel.add(Box.createHorizontalStrut(5));

            monGBC.fill = GridBagConstraints.HORIZONTAL;
            monGBC.gridwidth = 1;
            fMonitorRefreshRate.setColumns(3);
            fMonitorRefreshRate.setText("2");
            monPanel.add(fMonitorRefreshRate, monGBC);

            monPanel.add(Box.createGlue(), monGBC);
            monGBC.gridwidth = GridBagConstraints.REMAINDER;
            monPanel.add(Box.createHorizontalStrut(80), monGBC);

            monGBC.gridwidth = 1;
            monGBC.fill = GridBagConstraints.NONE;
            monPanel.add(new JLabel("Timestamp:"), monGBC);

            monGBC.fill = GridBagConstraints.HORIZONTAL;
            monPanel.add(Box.createHorizontalStrut(5));

            monGBC.gridwidth = GridBagConstraints.REMAINDER;;
            monPanel.add(fMonitorDate, monGBC);

            monGBC.gridwidth = 1;
            monGBC.fill = GridBagConstraints.NONE;
            monPanel.add(new JLabel("Status:"), monGBC);

            monGBC.fill = GridBagConstraints.HORIZONTAL;
            monPanel.add(Box.createHorizontalStrut(5));

            monGBC.gridwidth = GridBagConstraints.REMAINDER;;
            monPanel.add(fMonitorStatus, monGBC);

            gbc.gridy = 3;
            add(monPanel, gbc);

            // Setup log panel
            JPanel logPanel = new JPanel();
            logPanel.setLayout(new GridBagLayout());
            GridBagConstraints logGBC = new GridBagConstraints();

            logGBC.anchor = GridBagConstraints.WEST;
            logGBC.fill = GridBagConstraints.HORIZONTAL;
            logGBC.gridwidth = 1;

            logPanel.setBorder(new TitledBorder("Log / Variable"));

            fErrorLog.setSelected(true);
            logPanel.add(fErrorLog, logGBC);
            logGBC.gridwidth = GridBagConstraints.REMAINDER;
            logPanel.add(fErrorCount, logGBC);
            logGBC.gridwidth = 1;
            logPanel.add(fWarningLog, logGBC);
            logGBC.gridwidth = GridBagConstraints.REMAINDER;
            logPanel.add(fWarningCount, logGBC);
            logGBC.gridwidth = 1;
            logPanel.add(fInfoLog, logGBC);
            logGBC.gridwidth = GridBagConstraints.REMAINDER;
            logPanel.add(fInfoCount, logGBC);
            logGBC.gridwidth = 1;
            logPanel.add(fDebugLog, logGBC);
            logGBC.gridwidth = GridBagConstraints.REMAINDER;
            logPanel.add(fDebugCount, logGBC);
            logPanel.add(fSetLogMask, logGBC);
            logPanel.add(fRefreshLog, logGBC);
            logPanel.add(fClearLog, logGBC);
            logPanel.add(fDisplayLog, logGBC);

            // gbc.fill = GridBagConstraints.BOTH;
            gbc.gridx = 2;
            gbc.gridy = 1;
            gbc.gridheight = 3;
            add(logPanel, gbc);

            // Add button commands
            fStop.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "QUEUE", "QUEUE HANDLE " + fHandle +
                        " TYPE STAF/STAFDemo/Stop MESSAGE " +
                        STAFUtil.wrapData(""));

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error queueing message RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }
                }
            });

            fPause.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "SEM", "RESET EVENT STAFDemo/Handle/" +
                        fHandle + "/Continue");

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error pausing process RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }
                }
            });

            fContinue.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "SEM", "POST EVENT STAFDemo/Handle/" +
                        fHandle + "/Continue");

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error continuing process RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }
                }
            });

            fSetBackground.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "VAR",
                        "SET HANDLE " + fHandle +
                        " VAR STAFDemo/BackgroundColor=" +
                        (String)fColorList.getSelectedItem());

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error setting background color RC: " +
                            stafResult.rc + " Result: " + stafResult.result);
                    }
                }
            });

            fSetLogMask.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    String logMask = new String();

                    if (fErrorLog.isSelected())
                        logMask += " Error";
                    if (fWarningLog.isSelected())
                        logMask += " Warning";
                    if (fInfoLog.isSelected())
                        logMask += " Info";
                    if (fDebugLog.isSelected())
                        logMask += " Debug";

                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "VAR", "SET HANDLE " + fHandle + " VAR " +
                        STAFUtil.wrapData("STAF/Service/Log/Mask=" + logMask));

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        System.out.println(
                            "Error setting log mask RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }
                }
            });

            fRefreshLog.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "LOG",
                        "QUERY MACHINE {STAF/Config/MachineNickname} HANDLE " +
                        fHandle + " LOGNAME STAFDemo STATS");

                    if (stafResult.rc != STAFResult.Ok)
                    {
                        if (stafResult.rc == STAFResult.DoesNotExist)
                        {
                            fErrorCount.setText("0");
                            fWarningCount.setText("0");
                            fInfoCount.setText("0");
                            fDebugCount.setText("0");
                        }
                        else
                        {
                            System.out.println(
                                "Error querying log RC: " + stafResult.rc +
                                " Result: " + stafResult.result);
                        }

                        return;
                    }

                    STAFMarshallingContext outputContext =
                        STAFMarshallingContext.unmarshall(stafResult.result);

                    Map logStatsMap = (Map)outputContext.getRootObject();

                    fErrorCount.setText((String)logStatsMap.get("error"));
                    fWarningCount.setText((String)logStatsMap.get("warning"));
                    fInfoCount.setText((String)logStatsMap.get("info"));
                    fDebugCount.setText((String)logStatsMap.get("debug"));
                }
            });

            fClearLog.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    STAFResult stafResult = STAFDemoController.handle.submit2(
                        fMachine, "LOG",
                        "PURGE MACHINE {STAF/Config/MachineNickname} HANDLE " +
                        fHandle + " LOGNAME STAFDemo CONFIRMALL");

                    if ((stafResult.rc != STAFResult.Ok) &&
                        (stafResult.rc != STAFResult.DoesNotExist))
                    {
                        System.out.println(
                            "Error purging log RC: " + stafResult.rc +
                            " Result: " + stafResult.result);
                    }

                    fErrorCount.setText("0");
                    fWarningCount.setText("0");
                    fInfoCount.setText("0");
                    fDebugCount.setText("0");
                }
            });

            fDisplayLog.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent event)
                {
                    String request = "QUERY ALL " +
                        "MACHINE {STAF/Config/MachineNickname} " +
                        "HANDLE " + fHandle + " LOGNAME STAFDemo";

                    new STAFLogViewer(
                        fProcessFrame, STAFDemoController.handle,
                        fMachine, "LOG", request, "", "Dialog");
                }
            });

            // Launch monitor thread
            fMonitorThread = new MonitorThread(fMachine, fHandle,
                                               fMonitorRefreshRate,
                                               fMonitorDate, fMonitorStatus);
            fMonitorThread.start();

            // Add listener so that we can stop the monitor thread
            fProcessFrame.addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
                {
                    fMonitorThread.fContinue = false;
                }
            });
        }
    }

    public class MonitorThread extends Thread
    {
        public MonitorThread(String machine, String handle,
                             JTextField sleep, JLabel date, JLabel status)
        {
            fMachine = machine;
            fHandle = handle;
            fSleepField = sleep;
            fDateField = date;
            fStatusField = status;
    }

        String fMachine;
        String fHandle;
        JTextField fSleepField;
        JLabel fDateField;
        JLabel fStatusField;
        public boolean fContinue = true;

        public void run()
        {
            for(;;)
            {
                if (!fContinue) return;

                int sleep = 2;

                try
                {
                    sleep = Integer.parseInt(fSleepField.getText());
                    Thread.sleep(sleep * 1000);
                }
                catch(NumberFormatException e)
                { /* Do Nothing */ }
                catch(InterruptedException e)
                { /* Do Nothing */ }

                STAFResult stafResult = STAFDemoController.handle.submit2(
                    fMachine, "MONITOR",
                    "QUERY MACHINE {STAF/Config/MachineNickname} HANDLE " +
                    fHandle);

                if (stafResult.rc != STAFResult.Ok)
                {
                    fDateField.setText("Unknown");
                    fStatusField.setText("Unknown");
                }
                else
                {
                    STAFMarshallingContext outputContext =
                        STAFMarshallingContext.unmarshall(stafResult.result);

                    Map resultsMap = (Map)outputContext.getRootObject();

                    fDateField.setText((String)resultsMap.get("timestamp"));
                    fStatusField.setText((String)resultsMap.get("message"));
                }
            }
        }
    }
}
