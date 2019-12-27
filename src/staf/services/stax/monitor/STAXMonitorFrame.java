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
import java.util.List;
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
import java.util.jar.*;

public class STAXMonitorFrame extends JFrame implements ActionListener,
                                                        Runnable,
                                                        KeyListener,
                                                        MouseListener,
                                                        TreeSelectionListener,
                                                        ChangeListener
{
    public static final int EXTENSION_ACTIVE = 1;
    public static final int EXTENSION_STATUS = 2;
    public static final int EXTENSION_INFO = 3;
    public static final int EXTENSION_ACTIVE_JOB_ELEMENTS = 4;
    public static final int EXTENSION_ACTIVE_JOB_ELEMENTS_AND_ACTIVE = 5;
    public static final int EXTENSION_ACTIVE_JOB_ELEMENTS_AND_STATUS = 6;
    public static final int EXTENSION_ACTIVE_JOB_ELEMENTS_AND_INFO = 7;

    public static final int AUTOMONITOR_ALWAYS = 1;
    public static final int AUTOMONITOR_RECOMMENDED = 2;
    public static final int AUTOMONITOR_NEVER = 3;

    public static final int DISABLED = 0;
    public static final int ENABLED = 1;
    public static final int DEFAULT = 2;

    // STAX Service Error Codes
    static final int BlockNotHeld = 4002;
    static final int BlockAlreadyHeld = 4003;

    String fStaxMachine;
    String fStaxServiceName;
    String fStaxMachineNickname;
    String fStaxConfigMachine;
    String fStaxInstanceUUID;
    String fEventMachine;
    String fEventServiceName;
    String fJobNumber;
    String fXmlFileName;
    String fXmlFileMachine;
    String fFunction;
    String fArgs;
    String fJobName;
    Vector<String> fScripts;
    Vector<String> fScriptFilesVector;
    String fScriptFilesMachineName;
    Vector<String> fBreakpointTriggerVector;
    static String kMonitorFrameTitle = "STAX Job Monitor";
    JLabel fCurrentSelection;
    STAXMonitorTreeNode fCurrentSelectedNode;
    JTable fDetailsTable;
    STAXMonitorTableModel fDetailsTableModel;
    STAXMonitorTableSorter fDetailsModelSorter;
    JTree fMonitorTree;
    DefaultTreeModel fMonitorTreeModel;
    STAFHandle fHandle;
    private Thread fQueueThread;
    JTable fMessageTable;
    JScrollPane fMessageScrollPane;
    STAXMonitorTableModel fMessageTableModel;
    STAXMonitorTableSorter fMessageModelSorter;
    Hashtable<String, STAXMonitorTreeNode> fMonitorTreeBlocks =
        new Hashtable<String, STAXMonitorTreeNode>(); // key is block name
    Hashtable<STAXMonitorTreeNode, String> fMonitorTreeBlockNodes =
        new Hashtable<STAXMonitorTreeNode, String>(); // value is block node
    Hashtable<String, STAXMonitorTreeNode> fCmdIDHashtable =
        new Hashtable<String, STAXMonitorTreeNode>();
    Hashtable<String, STAXMonitorTreeNode> fSubjobIDHashtable =
        new Hashtable<String, STAXMonitorTreeNode>();
    Hashtable<String, STAXMonitorTreeNode> fProcIDHashtable =
        new Hashtable<String, STAXMonitorTreeNode>();
    Hashtable<String, STAXMonitorTreeNode> fPluginNodeHashtable =
        new Hashtable<String, STAXMonitorTreeNode>();
    Hashtable<STAXMonitorTreeNode, String> fPluginNodeToIDHashtable =
        new Hashtable<STAXMonitorTreeNode, String>();
    Hashtable<STAXMonitorTreeNode, String> fPluginNodeToNodeTextHashtable =
        new Hashtable<STAXMonitorTreeNode, String>();
    Hashtable<STAXMonitorTreeNode, String> fMonitorTreeProcessNodes =
        new Hashtable<STAXMonitorTreeNode, String>();
    Hashtable<STAXMonitorTreeNode, String> fMonitorTreeCommandNodes =
        new Hashtable<STAXMonitorTreeNode, String>();
    Hashtable<STAXMonitorTreeNode, String> fMonitorTreeSubjobNodes =
        new Hashtable<STAXMonitorTreeNode, String>();
    Hashtable<STAXMonitorTreeNode, Calendar> fMonitorTreeProcessStartTimes =
        new Hashtable<STAXMonitorTreeNode, Calendar>();
    Hashtable<STAXMonitorTreeNode, Calendar> fMonitorTreeCommandStartTimes =
        new Hashtable<STAXMonitorTreeNode, Calendar>();
    Hashtable<STAXMonitorTreeNode, Calendar> fMonitorTreeSubjobStartTimes =
        new Hashtable<STAXMonitorTreeNode, Calendar>();
    boolean fMonitorStopped = false;
    JPanel fSelectionDetailsPanel;
    Hashtable<String, Vector<Vector<String>>> fDataHashtable =
        new Hashtable<String, Vector<Vector<String>>>();
    Vector<String> fDataColumns;
    Vector<String> fMessageColumns;
    STAFProcessMonitor fProcessMonitor;
    MonitorElapsedTime fElapsedTime;
    boolean fContinueProcessMonitor = true;
    boolean fContinueElapsedTime = true;
    JPopupMenu fBlockPopupMenu = new JPopupMenu();
    JMenuItem fBlockHoldMenuItem = new JMenuItem("Hold");
    JMenuItem fBlockReleaseMenuItem = new JMenuItem("Release");
    JMenuItem fBlockTerminateMenuItem = new JMenuItem("Terminate");

    JPopupMenu fProcessPopupMenu = new JPopupMenu();
    JMenuItem fProcessStopMenuItem = new JMenuItem("Stop");

    JPopupMenu fSubjobPopupMenu = new JPopupMenu();
    JMenuItem fSubjobStartMonitoringMenuItem = new
        JMenuItem("Start Monitoring");
    JMenuItem fSubjobDisplayJobLogMenuItem = new
        JMenuItem("Display Job Log");
    JMenuItem fSubjobDisplayJobUserLogMenuItem = new
        JMenuItem("Display Job User Log");
    JMenuItem fSubjobTerminateMenuItem = new JMenuItem("Terminate Job");
    STAXMonitor fParentMonitor;
    TreePath previousTreePath;
    JMenu fFileMenu;
    JMenu fDisplayMenu;
    JMenu fViewMenu;
    JMenuItem fFileResubmit;
    JMenuItem fFileExitAndResubmit;
    JMenuItem fFileExit;
    JMenuItem fDisplayJobLog = new JMenuItem("Display Job Log");
    JMenuItem fDisplayJobUserLog = new JMenuItem("Display Job User Log");
    boolean fShowNoSTAXMonitorInformation = false;
    int fClearLogs = STAXMonitorFrame.DEFAULT;
    int fLogTCElapsedTime = STAXMonitorFrame.DEFAULT;
    int fLogTCNumStarts = STAXMonitorFrame.DEFAULT;
    int fLogTCStartStop = STAXMonitorFrame.DEFAULT;
    int fAutoMonitorSubjobs = STAXMonitorFrame.AUTOMONITOR_NEVER;
    String fPythonOutput = STAXMonitor.DEFAULT_STRING;
    String fPythonLogLevel = STAXMonitor.DEFAULT_STRING;
    String fLimitMessages = "";
    Hashtable<STAXMonitorExtension, Vector<String>> fRegisteredPlugins =
        new Hashtable<STAXMonitorExtension, Vector<String>>();
    JCheckBoxMenuItem fProcessMenu;
    JCheckBoxMenuItem fSTAFCmdMenu;
    JCheckBoxMenuItem fSubjobMenu;
    JCheckBoxMenuItem fDebugMenu;
    JCheckBoxMenuItem fTestcaseInfoMenu;
    JCheckBoxMenuItem fTestcaseStatusMenu;
    JCheckBoxMenuItem fActiveJobElementsMenu;
    JCheckBoxMenuItem fCurrentSelectionMenu;
    JCheckBoxMenuItem fMessagesMenu;
    Hashtable<Object, STAXMonitorExtension> fViewablePlugins =
        new Hashtable<Object, STAXMonitorExtension>();
    Hashtable<String, JComponent> fViewableComponents =
        new Hashtable<String, JComponent>();
    JTabbedPane fActiveElementsPane = new JTabbedPane();
    JTabbedPane fStatusPane = new JTabbedPane();
    JTabbedPane fInfoPane = new JTabbedPane();
    Vector fPluginJarsVector;
    Vector<String> fExternalPluginNotificationTypes = new Vector<String>();
    Hashtable<STAXMonitorTreeNode, Vector> fPluginNodeDetailsHashtable =
        new Hashtable<STAXMonitorTreeNode, Vector>();
    STAXMonitorProcessExtension processTablePlugin;
    STAXMonitorExtension stafcmdTablePlugin;
    STAXMonitorExtension subjobTablePlugin;
    STAXMonitorExtension testcaseTablePlugin;
    STAXMonitorExtension testcaseInfoTablePlugin;
    STAXMonitorExtension breakpointPlugin;
    String fMessageFontName = "Dialog";
    String fLogViewerFontName = "Dialog";
    String fSaveAsDirectory = null;
    boolean fBreakpointFirstFunction = false;
    boolean fBreakpointSubjobFirstFunction = false;

    public STAXMonitorFrame()
    {
    }

    public STAXMonitorFrame(STAXMonitor parentMonitor, String staxMach,
                            String staxService, String staxMachNickname,
                            String jobNum,
                            String eventMach, String eventService,
                            boolean showNoMonitorInfo, String limitMessages,
                            Vector pluginJarsVector,
                            int autoMonitorSubjobs) throws STAFException
    {
        this(false, parentMonitor, staxMach, staxService, staxMachNickname,
             jobNum, eventMach, eventService, "", "", "", "", "", null,
             showNoMonitorInfo, limitMessages, pluginJarsVector, null, "",
             STAXMonitorFrame.DEFAULT, STAXMonitorFrame.DEFAULT,
             STAXMonitorFrame.DEFAULT, STAXMonitorFrame.DEFAULT,
             autoMonitorSubjobs,
             STAXMonitor.DEFAULT_STRING, STAXMonitor.DEFAULT_STRING,
             new Vector<String>(), false, false);
    }

    public STAXMonitorFrame(STAXMonitor parentMonitor, String staxMachine,
                            String staxServiceName, 
                            String staxMachineNickname, String jobNumber,
                            String eventMachine, String eventServiceName,
                            String xmlFileName, String xmlFileMachine,
                            String function, String args, String jobName,
                            Vector<String> scripts, boolean showNoMonitorInfo,
                            String limitMessages, Vector pluginJarsVector,
                            Vector<String> scriptFilesVector,
                            String scriptFilesMachineName,
                            int clearLogs, int logTCElapsedTime,
                            int logTCNumStarts, int logTCStartStop,
                            int autoMonitorSubjobs) throws STAFException
    {
        this(true, parentMonitor, staxMachine, staxServiceName,
             staxMachineNickname, jobNumber,
             eventMachine, eventServiceName, xmlFileName, xmlFileMachine,
             function, args, jobName, scripts, showNoMonitorInfo,
             limitMessages, pluginJarsVector, scriptFilesVector,
             scriptFilesMachineName, clearLogs, logTCElapsedTime,
             logTCNumStarts, logTCStartStop, autoMonitorSubjobs,
             STAXMonitor.DEFAULT_STRING, STAXMonitor.DEFAULT_STRING,
             new Vector<String>(), false, false);
    }
    
    public STAXMonitorFrame(STAXMonitor parentMonitor, String staxMachine,
                            String staxServiceName, 
                            String staxMachineNickname, String jobNumber,
                            String eventMachine, String eventServiceName,
                            String xmlFileName, String xmlFileMachine,
                            String function, String args, String jobName,
                            Vector<String> scripts, boolean showNoMonitorInfo,
                            String limitMessages,
                            Vector pluginJarsVector,
                            Vector<String> scriptFilesVector,
                            String scriptFilesMachineName,
                            int clearLogs, int logTCElapsedTime,
                            int logTCNumStarts, int logTCStartStop,
                            int autoMonitorSubjobs, String pythonOutput,
                            String pythonLogLevel,
                            Vector<String> breakpointTriggerVector,
                            boolean breakpointFirstFunction,
                            boolean breakpointSubjobFirstFunction)
                            throws STAFException
    {
        this(true, parentMonitor, staxMachine, staxServiceName,
             staxMachineNickname, jobNumber,
             eventMachine, eventServiceName, xmlFileName, xmlFileMachine,
             function, args, jobName, scripts, showNoMonitorInfo,
             limitMessages, pluginJarsVector, scriptFilesVector,
             scriptFilesMachineName, clearLogs, logTCElapsedTime,
             logTCNumStarts, logTCStartStop, autoMonitorSubjobs,
             pythonOutput, pythonLogLevel,
             breakpointTriggerVector, breakpointFirstFunction,
             breakpointSubjobFirstFunction);
    }

    public STAXMonitorFrame(boolean newJob,
                            STAXMonitor parentMonitor, String staxMachine,
                            String staxServiceName,
                            String staxMachineNickname, String jobNumber,
                            String eventMachine, String eventServiceName,
                            String xmlFileName, String xmlFileMachine,
                            String function, String args, String jobName,
                            Vector<String> scripts,
                            boolean showNoMonitorInfo,
                            String limitMessages,
                            Vector pluginJarsVector,
                            Vector<String> scriptFilesVector,
                            String scriptFilesMachineName,
                            int clearLogs,
                            int autoMonitorSubjobs) throws STAFException
    {
        this(newJob, parentMonitor, staxMachine, staxServiceName,
             staxMachineNickname, jobNumber,
             eventMachine, eventServiceName, xmlFileName, xmlFileMachine,
             function, args, jobName, scripts, showNoMonitorInfo,
             limitMessages, pluginJarsVector, scriptFilesVector,
             scriptFilesMachineName, clearLogs, STAXMonitorFrame.DEFAULT,
             STAXMonitorFrame.DEFAULT, STAXMonitorFrame.DEFAULT,
             autoMonitorSubjobs,
             STAXMonitor.DEFAULT_STRING, STAXMonitor.DEFAULT_STRING,
             new Vector<String>(), false, false);
    }
    
    public STAXMonitorFrame(boolean newJob,
                            STAXMonitor parentMonitor, String staxMachine,
                            String staxServiceName,
                            String staxMachineNickname,
                            String jobNumber,
                            String eventMachine, String eventServiceName,
                            String xmlFileName, String xmlFileMachine,
                            String function, String args, String jobName,
                            Vector<String> scripts,
                            boolean showNoMonitorInfo,
                            String limitMessages,
                            Vector pluginJarsVector,
                            Vector<String> scriptFilesVector,
                            String scriptFilesMachineName,
                            int clearLogs, int logTCElapsedTime,
                            int logTCNumStarts, int logTCStartStop,
                            int autoMonitorSubjobs) throws STAFException
    {
        this(newJob, parentMonitor, staxMachine, staxServiceName,
             staxMachineNickname, jobNumber,
             eventMachine, eventServiceName, xmlFileName, xmlFileMachine,
             function, args, jobName, scripts, showNoMonitorInfo,
             limitMessages, pluginJarsVector, scriptFilesVector,
             scriptFilesMachineName, clearLogs, STAXMonitorFrame.DEFAULT,
             STAXMonitorFrame.DEFAULT, STAXMonitorFrame.DEFAULT,
             autoMonitorSubjobs,
             STAXMonitor.DEFAULT_STRING, STAXMonitor.DEFAULT_STRING,
             new Vector<String>(), false, false);
    }

    public STAXMonitorFrame(boolean newJob,
                            STAXMonitor parentMonitor, String staxMachine,
                            String staxServiceName,
                            String staxMachineNickname,
                            String jobNumber,
                            String eventMachine, String eventServiceName,
                            String xmlFileName, String xmlFileMachine,
                            String function, String args, String jobName,
                            Vector<String> scripts,
                            boolean fShowNoSTAXMonitorInformation,
                            String limitMessages,
                            Vector pluginJarsVector,
                            Vector<String> scriptFilesVector,
                            String scriptFilesMachineName,
                            int clearLogs, int logTCElapsedTime,
                            int logTCNumStarts, int logTCStartStop,
                            int autoMonitorSubjobs,
                            String pythonOutput, String pythonLogLevel,
                            Vector<String> breakpointTriggerVector,
                            boolean breakpointFirstFunction,
                            boolean breakpointSubjobFirstFunction)
        throws STAFException
    {
        fPluginJarsVector = pluginJarsVector;
        fParentMonitor = parentMonitor;
        fStaxMachine = staxMachine;
        fStaxServiceName = staxServiceName;
        fStaxMachineNickname = staxMachineNickname;
        fStaxConfigMachine = parentMonitor.getStaxConfigMachine();
        fStaxInstanceUUID = parentMonitor.getStaxInstanceUUID();
        fJobNumber = jobNumber;
        fEventMachine = eventMachine;
        fLimitMessages = limitMessages;
        fBreakpointTriggerVector = breakpointTriggerVector;
        fBreakpointFirstFunction = breakpointFirstFunction;
        fBreakpointSubjobFirstFunction = breakpointSubjobFirstFunction;

        if (fEventMachine.equals(""))
        {
            fEventMachine = "local";
        }

        fEventServiceName = eventServiceName;
        fXmlFileName = xmlFileName;
        fXmlFileMachine = xmlFileMachine;
        fFunction = function;
        fArgs = args;
        fJobName = jobName;
        fScripts = scripts;
        fScriptFilesVector = scriptFilesVector;
        fScriptFilesMachineName = scriptFilesMachineName;
        fClearLogs = clearLogs;
        fLogTCElapsedTime = logTCElapsedTime;
        fLogTCNumStarts = logTCNumStarts;
        fLogTCStartStop = logTCStartStop;
        fAutoMonitorSubjobs = autoMonitorSubjobs;
        fPythonOutput = pythonOutput;
        fPythonLogLevel = pythonLogLevel;

        fMessageFontName = parentMonitor.getMessageFontName();
        fLogViewerFontName = parentMonitor.getLogViewerFontName();
        fSaveAsDirectory = parentMonitor.getSaveAsDirectory();

        if (newJob)
        {
            StringBuffer request = new StringBuffer("EXECUTE HOLD FILE ");
            request.append(STAFUtil.wrapData(fXmlFileName));

            if (!fXmlFileMachine.equals(""))
            {
                request.append(" MACHINE ").append(
                    STAFUtil.wrapData(fXmlFileMachine));
            }

            if (!fFunction.equals(""))
            {
                request.append(" FUNCTION ").append(
                    STAFUtil.wrapData(fFunction));
            }

            if (fArgs != null && !fArgs.equals(""))
            {
                request.append(" ARGS ").append(STAFUtil.wrapData(fArgs));
            }

            if (fJobName != null && !fJobName.equals(""))
            {
                request.append(" JOBNAME ").append(
                    STAFUtil.wrapData(fJobName));
            }

            if (fScripts != null)
            {
                if (!fScripts.isEmpty())
                {
                    for (int i=0; i < fScripts.size(); i++)
                    {
                        request.append(" SCRIPT ").append(
                            STAFUtil.wrapData(fScripts.elementAt(i)));
                    }
                }
            }

            if (fScriptFilesVector != null)
            {
                if (!fScriptFilesVector.isEmpty())
                {
                    if (fScriptFilesMachineName != null &&
                        !fScriptFilesMachineName.equals(""))
                    {
                        request.append(" SCRIPTFILEMACHINE ").append(
                            STAFUtil.wrapData(fScriptFilesMachineName));
                    }

                    for (int i=0; i < fScriptFilesVector.size(); i++)
                    {
                        request.append(" SCRIPTFILE ").append(
                            STAFUtil.wrapData(
                                fScriptFilesVector.elementAt(i)));
                    }
                }
            }

            if (fClearLogs == STAXMonitorFrame.ENABLED)
                request.append(" CLEARLOGS Enabled");
            else if (fClearLogs == STAXMonitorFrame.DISABLED)
                request.append(" CLEARLOGS Disabled");

            if (fLogTCElapsedTime == STAXMonitorFrame.ENABLED)
                request.append(" LOGTCELAPSEDTIME Enabled");
            else if (fLogTCElapsedTime == STAXMonitorFrame.DISABLED)
                request.append(" LOGTCELAPSEDTIME Disabled");

            if (fLogTCNumStarts == STAXMonitorFrame.ENABLED)
                request.append(" LOGTCNUMSTARTS Enabled");
            else if (fLogTCNumStarts == STAXMonitorFrame.DISABLED)
                request.append(" LOGTCNUMSTARTS Disabled");

            if (fLogTCStartStop == STAXMonitorFrame.ENABLED)
                request.append(" LOGTCSTARTSTOP Enabled");
            else if (fLogTCStartStop == STAXMonitorFrame.DISABLED)
                request.append(" LOGTCSTARTSTOP Disabled");

            if (!fPythonOutput.equals(STAXMonitor.DEFAULT_STRING))
                request.append(" PYTHONOUTPUT " + fPythonOutput);

            if (!fPythonLogLevel.equals(STAXMonitor.DEFAULT_STRING))
                request.append(" PYTHONLOGLEVEL " + fPythonLogLevel);

            if (fBreakpointTriggerVector != null)
            {
                if (!fBreakpointTriggerVector.isEmpty())
                {
                    for (int i=0; i < fBreakpointTriggerVector.size(); i++)
                    {
                        request.append(
                            fBreakpointTriggerVector.elementAt(i) + " ");
                    }
                }
            }

            if (fBreakpointFirstFunction)
            {
                request.append(" BREAKPOINTFIRSTFUNCTION");
            }

            if (fBreakpointSubjobFirstFunction)
            {
                request.append(" BREAKPOINTSUBJOBFIRSTFUNCTION");
            }

            // Submit the STAX EXECUTE request and display a "Please Wait"
            // dialog while the request is preparing to run the job

            STAXMonitorExecuteResult executeResult =
                parentMonitor.submitExecuteRequest(request.toString());

            if (executeResult.getRC() != STAFResult.Ok)
            {
                throw new STAFException(
                    executeResult.getRC(),
                    executeResult.getResult().toString());
            }

            // Assign the job number that was generated for this job

            fJobNumber = executeResult.getResult().toString();
        }

        try
        {
            fHandle = STAXMonitorUtil.getNewSTAFHandle(
                fStaxServiceName + "/JobMonitor/" + fStaxMachine + "/" +
                fJobNumber);
        }
        catch (STAFException e)
        {
            throw e;
        }

        boolean jobAlreadyHeld = false;

        if (!newJob)
        {
            String request = "HOLD JOB " + fJobNumber;
            STAFResult holdResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (holdResult.rc != 0)
            {
                if (holdResult.rc == BlockAlreadyHeld)
                {
                    jobAlreadyHeld = true;
                }
                else
                {
                    throw new STAFException(
                        holdResult.rc,
                        "Error holding Job ID " + fJobNumber + "\n" +
                        holdResult.result);
                }
            }
        }
        
        setTitle(kMonitorFrameTitle);
        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                if (!fMonitorStopped)
                {
                    stopMonitor();
                }
                closeMonitor();
            }
        });

        fActiveElementsPane.addChangeListener(this);
        fStatusPane.addChangeListener(this);
        fInfoPane.addChangeListener(this);

        JMenuBar mainMenuBar = new JMenuBar();
        setJMenuBar(mainMenuBar);
        fFileMenu = new JMenu("File");
        mainMenuBar.add(fFileMenu);
        fDisplayMenu = new JMenu("Display");
        mainMenuBar.add(fDisplayMenu);
        fViewMenu = new JMenu("View");
        mainMenuBar.add(fViewMenu);
        fFileResubmit = new JMenuItem("Resubmit Job");
        fFileResubmit.addActionListener(this);
        fFileExitAndResubmit = new JMenuItem("Exit and Resubmit Job");
        fFileExitAndResubmit.addActionListener(this);
        fFileExit = new JMenuItem("Exit");
        fFileExit.addActionListener(this);
        fFileMenu.add(fFileResubmit);
        fFileMenu.add(fFileExitAndResubmit);
        fFileMenu.insertSeparator(2);
        fFileMenu.add(fFileExit);
        fDisplayMenu.add(fDisplayJobLog);
        fDisplayMenu.add(fDisplayJobUserLog);
        fDisplayJobLog.addActionListener(this);
        fDisplayJobUserLog.addActionListener(this);

        fDataColumns = new Vector<String>();
        fDataColumns.addElement(" Name ");
        fDataColumns.addElement(" Value ");

        fMessageColumns = new Vector<String>();
        fMessageColumns.addElement(" Timestamp ");
        fMessageColumns.addElement(" Message ");

        getContentPane().setLayout(new BorderLayout());
        getContentPane().setBackground(Color.white);

        fSelectionDetailsPanel = new JPanel();
        fSelectionDetailsPanel.setLayout(new BorderLayout());

        JPanel selectionButtonPanel = new JPanel();
        selectionButtonPanel.setLayout(new
                                       FlowLayout(FlowLayout.LEFT, 0, 0));

        JPanel detailsTablePanel = new JPanel();
        detailsTablePanel.setLayout(new BorderLayout());

        fDetailsTableModel = new STAXMonitorTableModel(fDataColumns, 0);
        fDetailsModelSorter = new STAXMonitorTableSorter(fDetailsTableModel);
        fDetailsTable = new JTable(fDetailsModelSorter);
        fDetailsModelSorter.addMouseListenerToHeaderInTable(fDetailsTable, 1);
        fDetailsTable.setVisible(false);
        fDetailsTable.getTableHeader().setVisible(false);

        updateDetailsTableRenderers();

        JPanel messagesPanel = new JPanel();
        messagesPanel.setLayout(new BorderLayout());

        fMessageTableModel = new STAXMonitorTableModel(fMessageColumns, 0);
        fMessageModelSorter =
            new STAXMonitorTableSorter(fMessageTableModel, 0, fMessageFontName);
        fMessageTable = new JTable(fMessageModelSorter);
        fMessageModelSorter.addMouseListenerToHeaderInTable(fMessageTable, 1);

        fMessageTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font(fMessageFontName, Font.PLAIN, 12)));

        fMessageTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font(fMessageFontName, Font.BOLD, 12)));

        fMessageTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font(fMessageFontName, Font.PLAIN, 12)));

        fMessageTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font(fMessageFontName, Font.BOLD, 12)));

        fMessageTable.setFont(new Font(fMessageFontName, Font.PLAIN, 12));

        fMessageScrollPane = new JScrollPane(fMessageTable);

        fMessageScrollPane.setVerticalScrollBarPolicy(
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
        fMessageScrollPane.setHorizontalScrollBarPolicy(
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        fMessageTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        STAXMonitorUtil.sizeColumnsToFitText(fMessageTable);

        fInfoPane.add("Messages", fMessageScrollPane);
        fViewableComponents.put("Messages", fMessageScrollPane);

        JScrollPane detailsScrollPane = new JScrollPane(fDetailsTable);

        detailsScrollPane.setVerticalScrollBarPolicy(
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
        detailsScrollPane.setHorizontalScrollBarPolicy(
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        fDetailsTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

        STAXMonitorUtil.sizeColumnsToFitText(fDetailsTable);

        fCurrentSelection = new JLabel("");
        fCurrentSelection.setFont(new Font("Dialog", Font.BOLD, 14));
        fCurrentSelection.setForeground(new Color(23, 0, 122));

        fSelectionDetailsPanel.add(BorderLayout.NORTH, fCurrentSelection);

        JPanel selectionDetailsPanel1 = new JPanel();
        selectionDetailsPanel1.setLayout(new BorderLayout());
        fSelectionDetailsPanel.add(BorderLayout.CENTER, detailsScrollPane);

        int index = fInfoPane.getTabCount();

        fInfoPane.setBackgroundAt(index - 1, Color.white);
        fInfoPane.setForegroundAt(index - 1, Color.darkGray);

        fMonitorTree = new JTree();
        fMonitorTree.setCellRenderer(new
            STAXMonitorTreeCellRenderer(fShowNoSTAXMonitorInformation));
        fMonitorTree.putClientProperty("JTree.lineStyle", "Angled");
        fMonitorTree.setShowsRootHandles(true);
        fMonitorTree.addTreeSelectionListener(this);
        fMonitorTreeModel = (DefaultTreeModel) fMonitorTree.getModel();
        fMonitorTreeModel.setRoot(new STAXMonitorTreeNode());
        fMonitorTree.addMouseListener(this);

        JPanel treeAndSelectionPanel = new JPanel();
        treeAndSelectionPanel.setLayout(new BorderLayout());

        fActiveJobElementsMenu = new JCheckBoxMenuItem(
            "Active Job Elements", true);
        fActiveJobElementsMenu.addActionListener(this);
        fViewMenu.add(fActiveJobElementsMenu);

        JScrollPane scrollPane = new JScrollPane(fMonitorTree);
        fActiveElementsPane.add("Active Job Elements", scrollPane);

        fActiveElementsPane.setForegroundAt(0, Color.black);

        fViewableComponents.put("Active Job Elements", scrollPane);

        JSplitPane horizPane = new JSplitPane(
            JSplitPane.HORIZONTAL_SPLIT, fActiveElementsPane, fStatusPane);
        horizPane.setOneTouchExpandable(true);

        JSplitPane vertPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
            horizPane, fInfoPane);
        vertPane.setOneTouchExpandable(true);

        JPanel monitorPanel = new JPanel();
        monitorPanel.setLayout(new BorderLayout());
        monitorPanel.add(vertPane);

        fSelectionDetailsPanel.setVisible(true);

        // Handle extensions
        try
        {
            processTablePlugin = new STAXMonitorProcessExtension();
            JComponent processTable =
                processTablePlugin.init(this, newJob, fStaxMachine,
                fStaxServiceName, fJobNumber);
            processTable.setVisible(true);

            Vector<String> notificationTypes = new Vector<String>();

            StringTokenizer types = new StringTokenizer(
                processTablePlugin.getNotificationEventTypes(), " ");

            while (types.hasMoreElements())
            {
                String type = types.nextToken().toLowerCase();
                notificationTypes.add(type);
            }

            fRegisteredPlugins.put(processTablePlugin, notificationTypes);

            fProcessMenu = new JCheckBoxMenuItem(
                processTablePlugin.getTitle(), true);
            fProcessMenu.addActionListener(this);
            fViewMenu.add(fProcessMenu);

            fViewablePlugins.put(fProcessMenu, processTablePlugin);

            JScrollPane procScrollPane =
                new JScrollPane(processTablePlugin.getComponent());
            procScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
            procScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            fActiveElementsPane.addTab(processTablePlugin.getTitle(),
                                       procScrollPane);

            fViewableComponents.put(processTablePlugin.getTitle(),
                                    procScrollPane);

            int i = fActiveElementsPane.getTabCount();
            fActiveElementsPane.setBackgroundAt(i - 1, Color.white);
            fActiveElementsPane.setForegroundAt(i - 1, Color.darkGray);
        }
        catch (STAFException ex)
        {
            System.out.println("STAFException thrown during " +
                "STAXMonitorProcessExtension initialization.  RC: " +
                ex.rc + "\n" + ex.getMessage());
        }

        try
        {
            stafcmdTablePlugin = new STAXMonitorSTAFCmdExtension();
            JComponent stafcmdTable =
                stafcmdTablePlugin.init(this, newJob, fStaxMachine,
                fStaxServiceName, fJobNumber);
            stafcmdTable.setVisible(true);

            Vector<String> notificationTypes = new Vector<String>();

            StringTokenizer types = new StringTokenizer(
                stafcmdTablePlugin.getNotificationEventTypes(), " ");

            while (types.hasMoreElements())
            {
                String type = types.nextToken().toLowerCase();
                notificationTypes.add(type);
            }

            fRegisteredPlugins.put(stafcmdTablePlugin, notificationTypes);

            fSTAFCmdMenu = new JCheckBoxMenuItem(
                stafcmdTablePlugin.getTitle(), true);
            fSTAFCmdMenu.addActionListener(this);
            fViewMenu.add(fSTAFCmdMenu);

            JScrollPane stafcmdScrollPane =
                new JScrollPane(stafcmdTablePlugin.getComponent());
            stafcmdScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
            stafcmdScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            fActiveElementsPane.addTab(stafcmdTablePlugin.getTitle(),
                stafcmdScrollPane);

            fViewableComponents.put(stafcmdTablePlugin.getTitle(),
                                    stafcmdScrollPane);

            fViewablePlugins.put(fSTAFCmdMenu, stafcmdTablePlugin);

            int i = fActiveElementsPane.getTabCount();
            fActiveElementsPane.setBackgroundAt(i - 1, Color.white);
            fActiveElementsPane.setForegroundAt(i - 1, Color.darkGray);
        }
        catch (STAFException ex)
        {
            System.out.println("STAFException thrown during " +
                "STAXMonitorSTAFCmdExtension initialization.  RC: " +
                ex.rc + "\n" + ex.getMessage());
        }

        try
        {
            subjobTablePlugin = new STAXMonitorSubjobExtension();
            JComponent subjobTable =
                subjobTablePlugin.init(this, newJob, fStaxMachine,
                    fStaxServiceName, fJobNumber);
            subjobTable.setVisible(true);

            Vector<String> notificationTypes = new Vector<String>();

            StringTokenizer types = new StringTokenizer(
                subjobTablePlugin.getNotificationEventTypes(), " ");

            while (types.hasMoreElements())
            {
                String type = types.nextToken().toLowerCase();
                notificationTypes.add(type);
            }

            fRegisteredPlugins.put(subjobTablePlugin, notificationTypes);

            fSubjobMenu = new JCheckBoxMenuItem(
                subjobTablePlugin.getTitle(), true);
            fSubjobMenu.addActionListener(this);
            fViewMenu.add(fSubjobMenu);

            JScrollPane subjobScrollPane =
                new JScrollPane(subjobTablePlugin.getComponent());
            subjobScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
            subjobScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            fActiveElementsPane.addTab(subjobTablePlugin.getTitle(),
                subjobScrollPane);

            fViewableComponents.put(subjobTablePlugin.getTitle(),
                                    subjobScrollPane);

            fViewablePlugins.put(fSubjobMenu, subjobTablePlugin);

            int i = fActiveElementsPane.getTabCount();
            fActiveElementsPane.setBackgroundAt(i - 1, Color.white);
            fActiveElementsPane.setForegroundAt(i - 1, Color.darkGray);
        }
        catch (STAFException ex)
        {
            System.out.println("STAFException thrown during " +
                "STAXMonitorSubjobExtension initialization.  RC: " +
                ex.rc + "\n" + ex.getMessage());
        }

        // Debug
        try
        {
            breakpointPlugin = new STAXMonitorDebugExtension();
            JComponent breakpointPanel =
                breakpointPlugin.init(this, newJob, fStaxMachine,
                fStaxServiceName, fJobNumber);
            breakpointPanel.setVisible(true);

            Vector<String> notificationTypes = new Vector<String>();

            StringTokenizer types = new StringTokenizer(
                breakpointPlugin.getNotificationEventTypes(), " ");

            while (types.hasMoreElements())
            {
                String type = types.nextToken().toLowerCase();
                notificationTypes.add(type);
            }

            fRegisteredPlugins.put(breakpointPlugin, notificationTypes);

            fActiveElementsPane.addTab(breakpointPlugin.getTitle(),
                breakpointPlugin.getComponent());

            int i = fActiveElementsPane.getTabCount();
            fActiveElementsPane.setBackgroundAt(i - 1, Color.white);
            fActiveElementsPane.setForegroundAt(i - 1, Color.darkGray);

            fDebugMenu = new JCheckBoxMenuItem(
                breakpointPlugin.getTitle(), true);
            fDebugMenu.addActionListener(this);
            fViewMenu.add(fDebugMenu);

            fViewableComponents.put(breakpointPlugin.getTitle(),
                                    breakpointPlugin.getComponent());

            fViewablePlugins.put(fDebugMenu, breakpointPlugin);
        }
        catch (STAFException ex)
        {
            System.out.println("STAFException thrown during " +
                "STAXMonitorDebugExtension initialization.  RC: " +
                ex.rc + "\n" + ex.getMessage());
        }

        fViewMenu.addSeparator();

        // Testcase Status
        try
        {
            testcaseTablePlugin = new STAXMonitorTestcaseExtension();
            JComponent testcaseTable =
                testcaseTablePlugin.init(this, newJob, fStaxMachine,
                fStaxServiceName, fJobNumber);
            testcaseTable.setVisible(true);

            Vector<String> notificationTypes = new Vector<String>();

            StringTokenizer types = new StringTokenizer(
                testcaseTablePlugin.getNotificationEventTypes(), " ");

            while (types.hasMoreElements())
            {
                String type = types.nextToken().toLowerCase();
                notificationTypes.add(type);
            }

            fRegisteredPlugins.put(testcaseTablePlugin, notificationTypes);

            fTestcaseStatusMenu = new JCheckBoxMenuItem(
                testcaseTablePlugin.getTitle(), true);
            fTestcaseStatusMenu.addActionListener(this);

            JScrollPane tcScrollPane =
                new JScrollPane(testcaseTablePlugin.getComponent());
            tcScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
            tcScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            fViewMenu.add(fTestcaseStatusMenu);

            fViewablePlugins.put(fTestcaseStatusMenu, testcaseTablePlugin);

            fStatusPane.addTab(testcaseTablePlugin.getTitle(),
                tcScrollPane);

            fViewableComponents.put(testcaseTablePlugin.getTitle(),
                                    tcScrollPane);

            int i = fStatusPane.getTabCount();
            fStatusPane.setBackgroundAt(i - 1, Color.white);
            fInfoPane.setForegroundAt(i - 1, Color.darkGray);
        }
        catch (STAFException ex)
        {
            System.out.println("STAFException thrown during " +
                "STAXMonitorTestcaseExtension initialization.  RC: " +
                ex.rc + "\n" + ex.getMessage());
        }

        // Testcase Info
        try
        {
            testcaseInfoTablePlugin = new STAXMonitorTestcaseExtension();
            JComponent testcaseTable =
                testcaseInfoTablePlugin.init(this, newJob, fStaxMachine,
                fStaxServiceName, fJobNumber);
            testcaseTable.setVisible(true);

            Vector<String> notificationTypes = new Vector<String>();

            StringTokenizer types = new StringTokenizer(
                testcaseInfoTablePlugin. getNotificationEventTypes(), " ");

            while (types.hasMoreElements())
            {
                String type = types.nextToken().toLowerCase();
                notificationTypes.add(type);
            }

            fRegisteredPlugins.put(testcaseInfoTablePlugin, notificationTypes);

            fTestcaseInfoMenu = new JCheckBoxMenuItem(
                testcaseInfoTablePlugin.getTitle() + " ", false);
            fTestcaseInfoMenu.addActionListener(this);

            JScrollPane tcScrollPane =
                new JScrollPane(testcaseInfoTablePlugin.getComponent());
            tcScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
            tcScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            fViewablePlugins.put(fTestcaseInfoMenu, testcaseInfoTablePlugin);
        }
        catch (STAFException ex)
        {
            System.out.println("STAFException thrown during " +
                "STAXMonitorTestcaseExtension initialization.  RC: " +
                ex.rc + "\n" + ex.getMessage());
        }

        fViewMenu.addSeparator();

        fMessagesMenu =
            new JCheckBoxMenuItem("Messages", true);
        fMessagesMenu.addActionListener(this);

        fViewMenu.add(fMessagesMenu);

        fCurrentSelectionMenu =
            new JCheckBoxMenuItem("Current Selection", true);
        fCurrentSelectionMenu.addActionListener(this);

        fViewMenu.add(fCurrentSelectionMenu);

        // XXX fix this later....need to use the type to determine where
        // the plugin should be placed in the menu

        if (fTestcaseInfoMenu != null)
        {
            fViewMenu.add(fTestcaseInfoMenu);
        }

        fBlockPopupMenu.add(fBlockHoldMenuItem);
        fBlockHoldMenuItem.addActionListener(this);
        fBlockPopupMenu.add(fBlockReleaseMenuItem);
        fBlockReleaseMenuItem.addActionListener(this);
        fBlockPopupMenu.add(fBlockTerminateMenuItem);
        fBlockTerminateMenuItem.addActionListener(this);

        fProcessPopupMenu.add(fProcessStopMenuItem);
        fProcessStopMenuItem.addActionListener(this);

        fSubjobPopupMenu.add(fSubjobStartMonitoringMenuItem);
        fSubjobStartMonitoringMenuItem.addActionListener(this);
        fSubjobPopupMenu.addSeparator();
        fSubjobPopupMenu.add(fSubjobDisplayJobLogMenuItem);
        fSubjobDisplayJobLogMenuItem.addActionListener(this);
        fSubjobPopupMenu.add(fSubjobDisplayJobUserLogMenuItem);
        fSubjobDisplayJobUserLogMenuItem.addActionListener(this);
        fSubjobPopupMenu.addSeparator();
        fSubjobPopupMenu.add(fSubjobTerminateMenuItem);
        fSubjobTerminateMenuItem.addActionListener(this);

        fProcessMonitor = new STAFProcessMonitor();
        fProcessMonitor.start();

        fElapsedTime = new MonitorElapsedTime();
        fElapsedTime.start();

        if (!newJob)
        {
            // Determine which blocks in the job are currently held

            String request = "LIST JOB " + fJobNumber + " BLOCKS";
            STAFResult listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Error listing Job Blocks\n"+ listResult.result);
            }

            java.util.List blockList = (java.util.List)listResult.resultObj;
            Iterator iter = blockList.iterator();

            Vector<STAXMonitorTreeNode> heldBlocks =
                new Vector<STAXMonitorTreeNode>();

            while (iter.hasNext())
            {
                Map blockInfoMap = (Map)iter.next();
                String block = (String)blockInfoMap.get("blockName");
                String status = (String)blockInfoMap.get("state");
                
                heldBlocks.addAll(handleBlock(block, status));
            }

            for (int i = 0; i < heldBlocks.size(); i++)
            {
                heldBlocks.elementAt(i).setBlockStatus(
                    STAXMonitorTreeNode.blockHeld);
            }

            // Determine the processes that are currently running in the job

            request = "LIST JOB " + fJobNumber + " PROCESSES";
            listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Error listing Job PROCESSES\n" + listResult.result);
            }

            java.util.List processList = (java.util.List)listResult.resultObj;
            iter = processList.iterator();
            
            while (iter.hasNext())
            {
                Map processMap = (Map)iter.next();
                String location = (String)processMap.get("location");
                String handleNumber = (String)processMap.get("handle");

                request = "QUERY JOB " + fJobNumber + " PROCESS " +
                          location + ":" + handleNumber;

                STAFResult queryResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, request);

                if (queryResult.rc != 0)
                {
                    // Ignore since the process may have already completed
                    continue;
                }

                processMap = (HashMap)queryResult.resultObj;

                String block = (String)processMap.get("blockName");
                String processName = (String)processMap.get("processName");

                handleProcess(processMap, block, processName, location,
                              handleNumber, false);
            }
            
            // Determine the stafcmds that are currently running in the job

            request = "LIST JOB " + fJobNumber + " STAFCMDS";
            listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Error listing Job STAFCMDS\n" + listResult.result);
            }

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Error listing Job STAFCMDS\n" + listResult.result);
            }

            java.util.List stafcmdList = (java.util.List)listResult.resultObj;
            iter = stafcmdList.iterator();
            
            while (iter.hasNext())
            {
                Map stafcmdMap = (Map)iter.next();
                String commandName   = (String)stafcmdMap.get("stafcmdName");
                String location      = (String)stafcmdMap.get("location");
                String requestNumber = (String)stafcmdMap.get("requestNum");
                String service       = (String)stafcmdMap.get("service");
                String cmdRequest    = (String)stafcmdMap.get("request");

                request = "QUERY JOB " + fJobNumber + " STAFCMD " +
                          requestNumber;

                STAFResult queryResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, request);

                if (queryResult.rc != 0)
                {
                    // Ignore since the stafcmd may have already completed
                    continue;
                }

                stafcmdMap = (HashMap)queryResult.resultObj;

                String block = (String)stafcmdMap.get("blockName");

                handleCommand(commandName, block, location, requestNumber,
                              service, cmdRequest);
            }

            request = "LIST JOB " + fJobNumber + " SUBJOBS";

            listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Error listing Job SUBJOBS\n" + listResult.result);
            }

            java.util.List subjobList = (java.util.List)listResult.resultObj;
            iter = subjobList.iterator();
            
            while (iter.hasNext())
            {
                Map subjobMap = (Map)iter.next();
                String jobID = (String)subjobMap.get("jobID");
                jobName = (String)subjobMap.get("jobName");
                String jobBlock = (String)subjobMap.get("blockName");
                
                request = "QUERY JOB " + jobID;
                STAFResult queryResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, request);

                if (queryResult.rc != 0)
                    continue;
                
                Map jobMap = (HashMap)queryResult.resultObj;

                String jobFile        = (String)jobMap.get("xmlFileName");
                String jobFileMachine = (String)jobMap.get("fileMachine");
                String subjobFunction = (String)jobMap.get("function");
                String functionArgs   = (String)jobMap.get("arguments");
                String scriptFileMachine = (String)jobMap.get("scriptMachine");

                // Convert the list of scripts to a vector of scripts

                java.util.List scriptList =
                    (java.util.List)jobMap.get("scriptList");
                scripts = new Vector<String>();

                Iterator scriptsIter = scriptList.iterator();

                while (scriptsIter.hasNext())
                {
                    scripts.add((String)scriptsIter.next());
                }

                // Convert the list of scriptfiles to a vector of scriptfiles

                java.util.List scriptFileList =
                    (java.util.List)jobMap.get("scriptFileList");
                scriptFilesVector = new Vector<String>();

                Iterator scriptFilesIter = scriptFileList.iterator();

                while (scriptFilesIter.hasNext())
                {
                    scriptFilesVector.add((String)scriptFilesIter.next());
                }

                // startTimestamp format is YYYYMMDD-HH:MM:SS
                String startTimestamp = (String)jobMap.get("startTimestamp");
                String startDate = startTimestamp.substring(0, 8);
                String startTime = startTimestamp.substring(9);

                String clearLogsString = (String)jobMap.get("clearLogs");
                String logTCElapsedTimeString =
                    (String)jobMap.get("logTCElapsedTime");
                String logTCNumStartsString =
                    (String)jobMap.get("logTCNumStarts");
                String logTCStartStopString =
                    (String)jobMap.get("logTCStartStop");

                String pythonOutputString = (String)jobMap.get("pythonOutput");
                String pythonLogLevelString = (String)jobMap.get(
                    "pythonLogLevel");

                String subjobText = "Job " + jobID + " - " + jobName;

                STAXMonitorTreeNode newNode = new STAXMonitorTreeNode(
                    subjobText, STAXMonitorTreeNode.subjobNodeType);

                STAXMonitorTreeNode parentNode = fMonitorTreeBlocks.get(
                    jobBlock);

                // parentNode can be null if the block has ended
                if (parentNode != null)
                {
                    fMonitorTreeModel.insertNodeInto(newNode, parentNode,
                        parentNode.getChildCount());
                    TreeNode[] parentNodes =
                        fMonitorTreeModel.getPathToRoot(parentNode);

                    fMonitorTree.expandPath(new TreePath(parentNodes));

                    fSubjobIDHashtable.put(jobID, newNode);
                    fMonitorTreeSubjobNodes.put(newNode, jobID);

                    fMonitorTreeSubjobStartTimes.put(
                        newNode,
                        STAXMonitorUtil.getCalendar2(startDate, startTime));
                }

                Vector<Vector<String>> subjobDataVector =
                    new Vector<Vector<String>>();

                addRow(subjobDataVector, "Job ID", jobID);
                addRow(subjobDataVector, "Job Name", jobName);
                addRow(subjobDataVector, "Clear Logs", clearLogsString);
                addRow(subjobDataVector, "Log TC Elapsed Time",
                       logTCElapsedTimeString);
                addRow(subjobDataVector, "Log TC Num Starts",
                       logTCNumStartsString);
                addRow(subjobDataVector, "Log TC Start/Stop",
                       logTCStartStopString);
                addRow(subjobDataVector, "Python Output",
                       pythonOutputString);
                addRow(subjobDataVector, "Python Log Level",
                       pythonLogLevelString);
                addRow(subjobDataVector, "Job File", jobFile);

                if (!jobFileMachine.equals(""))
                {
                    addRow(subjobDataVector,
                           "Job File Machine", jobFileMachine);
                }

                addRow(subjobDataVector, "Function", subjobFunction);
                addRow(subjobDataVector, "Function Args", functionArgs);

                for (int i = 0; i < scriptFilesVector.size(); ++i)
                {
                    if (i == 0)
                    {
                        addRow(subjobDataVector, "Script Files Machine",
                               scriptFileMachine);
                    }

                    addRow(subjobDataVector, "Script File #" + (i + 1),
                           scriptFilesVector.elementAt(i));
                }

                for (int i = 0; i < scripts.size(); ++i)
                {
                    addRow(subjobDataVector, "Script #" + (i + 1),
                           scripts.elementAt(i));
                }

                addRow(subjobDataVector, "Start Date", startDate);
                addRow(subjobDataVector, "Start Time", startTime);
                addRow(subjobDataVector, "Block", jobBlock);

                synchronized(fDataHashtable)
                {
                    fDataHashtable.put(subjobText, subjobDataVector);
                }
            }
        }

        try
        {
            Thread.sleep(500);
        }
        catch (InterruptedException ex)
        {
        }

        fInfoPane.add("Current Selection", fSelectionDetailsPanel);
        fViewableComponents.put("Current Selection", fSelectionDetailsPanel);

        index = fInfoPane.getTabCount();

        fInfoPane.setBackgroundAt(index - 1, Color.white);
        fInfoPane.setForegroundAt(index - 1, Color.darkGray);

        // Get monitor extension (plugin) classes
        Vector<Class> pluginClasses = parentMonitor.getPluginClasses();

        // Handle external extensions (plugins)
        for (int p = 0; p < pluginClasses.size(); p++)
        {
            try
            {
                Class pluginClass = pluginClasses.elementAt(p);
                Object pluginObj = null;

                // Try using a constructor that accepts a STAX object

                Class [] parameterTypes = new Class[1];
                parameterTypes[0] = this.getClass();

                try
                {
                    Constructor construct =
                        pluginClass.getConstructor(parameterTypes);
                    Object [] initArgs = new Object[1];
                    initArgs[0] = this;
                    pluginObj = construct.newInstance(initArgs);
                }
                catch (NoSuchMethodException e)
                {
                    // Extension does not have a constructor that accepts
                    // a STAX object, so use constructor without parameters
                    pluginObj = pluginClass.newInstance();
                }

                STAXMonitorExtension plugin = (STAXMonitorExtension)pluginObj;

                JComponent pluginComponent =
                    plugin.init(this, newJob, fStaxMachine,
                    fStaxServiceName, fJobNumber);

                JScrollPane pluginScrollPane = null;

                pluginComponent.setVisible(true);

                Vector<String> notificationTypes = new Vector<String>();

                StringTokenizer types = new StringTokenizer(
                    plugin.getNotificationEventTypes(), " ");

                while (types.hasMoreElements())
                {
                    String type = types.nextToken().toLowerCase();
                    notificationTypes.add(type);
                    fExternalPluginNotificationTypes.add(type);
                }

                fRegisteredPlugins.put(plugin, notificationTypes);

                pluginScrollPane =
                    new JScrollPane(plugin.getComponent());
                pluginScrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                pluginScrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                fViewableComponents.put(plugin.getTitle(), pluginScrollPane);

                pluginComponent.setVisible(true);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.addTab(plugin.getTitle(),
                                               pluginScrollPane);

                    int i = fActiveElementsPane.getTabCount();
                    fActiveElementsPane.setBackgroundAt(i - 1, Color.white);
                    fActiveElementsPane.setForegroundAt(i - 1, Color.darkGray);
                }
                else if (pluginType == STAXMonitorFrame.EXTENSION_STATUS)
                {
                    fStatusPane.addTab(plugin.getTitle(), pluginScrollPane);

                    int i = fStatusPane.getTabCount();
                    fStatusPane.setBackgroundAt(i - 1, Color.white);
                    fStatusPane.setForegroundAt(i - 1, Color.darkGray);
                }
                else if (pluginType == STAXMonitorFrame.EXTENSION_INFO)
                {
                    fInfoPane.addTab(plugin.getTitle(), pluginScrollPane);

                    int i = fInfoPane.getTabCount();
                    fInfoPane.setBackgroundAt(i - 1, Color.white);
                    fInfoPane.setForegroundAt(i - 1, Color.darkGray);
                }
            }
            catch (InvocationTargetException e)
            {
                System.out.println("InvocationTargetException thrown during " +
                                   "STAXMonitor plugin initialization\n" +
                                   e.getMessage());

                e.printStackTrace();
            }
            catch (InstantiationException e)
            {
                System.out.println("InstantiationException thrown during " +
                                   "STAXMonitor plugin initialization\n" +
                                   e.getMessage());

                e.printStackTrace();
            }
            catch (IllegalAccessException e)
            {
                System.out.println("IllegalAccessException thrown during " +
                                   "STAXMonitor plugin initialization\n" +
                                   e.getMessage());

                e.printStackTrace();
            }
            catch (STAFException ex)
            {
                System.out.println("STAFException thrown during " +
                                   "STAXMonitor plugin initialization.  RC: " +
                                   ex.rc);
            }
        }

        getContentPane().add(BorderLayout.CENTER, monitorPanel);

        pack();
        setSize(new Dimension(950, 600));
        setLocation(50, 30);
        setVisible(true);

            // the calls to invalidate and validate are needed so that
            // the dividers will be located correctly on Linux systems
        horizPane.setDividerLocation(.55);
        invalidate();
        validate();
        vertPane.setDividerLocation(.68);
        invalidate();
        validate();

        startMonitor();

        if (!jobAlreadyHeld)
        {
            STAFResult releaseResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, "RELEASE JOB " + fJobNumber);

            if ((releaseResult.rc == STAFResult.DoesNotExist) ||
                (releaseResult.rc == BlockNotHeld))
            {
                // need to wait for slower machines
                for (int i = 0; i < 15; i++)
                {
                    try
                    {
                        Thread.sleep(2000);

                        releaseResult = fHandle.submit2(
                            fStaxMachine, fStaxServiceName,
                            "RELEASE JOB " + fJobNumber);

                        if ((releaseResult.rc != STAFResult.DoesNotExist) &&
                            (releaseResult.rc != BlockNotHeld))
                        {
                            break;
                        }
                    }
                    catch (InterruptedException ex)
                    {
                    }
                }
            }

            if (releaseResult.rc != 0)
            {
                JOptionPane.showMessageDialog(
                    this,
                    "An error was encountered while attempting to release " +
                    "the job, RC: " + releaseResult.rc +
                    ", Result: " + releaseResult.result,
                    "Error releasing job", JOptionPane.ERROR_MESSAGE);
            }
        }

        String theJobName = fJobName;

        if (fJobName == null) theJobName = "";

        setTitle(kMonitorFrameTitle + " Machine:" +
                 fStaxMachine + " JobID:" + fJobNumber + " " +
                 theJobName + " <Running>");

        fQueueThread = new Thread(this);
        fQueueThread.start(); // this calls the run() method
    }

    public STAXMonitor getParentMonitor()
    {
        return fParentMonitor;
    }

    public void actionPerformed(ActionEvent e)
    {
        String action = "";
        if (e.getSource() == fBlockHoldMenuItem)
        {
            action = "hold";
        }
        else if (e.getSource() == fBlockReleaseMenuItem)
        {
            action = "release";
        }
        else if (e.getSource() == fBlockTerminateMenuItem)
        {
            action = "terminate";
        }
        else if (e.getSource() == fProcessStopMenuItem)
        {
            // Get the process ID for the current selected node

            String processID = null;

            if (fMonitorTreeProcessNodes.containsKey(fCurrentSelectedNode))
                processID = fMonitorTreeProcessNodes.get(fCurrentSelectedNode);

            if (processID == null)
            {
                // The process is no longer running
                return;
            }
            
            // The processID has format processName;location;handle, but
            // note that processName could could a ";",
            // Need to object the location and handle from processID.

            String[] temp;
            temp = processID.split(";");

            if (temp.length < 2)
            {
                // Should never happen
                STAXMonitorUtil.showErrorDialog(
                    this,
                    "An error was encountered while attempting to stop the " +
                    "process because could not extract the process location " +
                    "and handle from: " + processID);
                return;
            }

            String handle = temp[temp.length - 1];
            String location = temp[temp.length - 2];

            String request = "STOP JOB " + fJobNumber + " PROCESS " +
                STAFUtil.wrapData(location + ":" + handle);

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    this,
                    "An error was encountered while attempting to stop the " +
                    "process,\n\nSTAF " + fStaxMachine + " " +
                    fStaxServiceName + " " + request +
                    "\n\nRC: " + result.rc + ", Result: " + result.result);
            }

            return;
        }
        else if (e.getSource() == fSubjobStartMonitoringMenuItem)
        {
            String subjobID = fMonitorTreeSubjobNodes.get(
                fCurrentSelectedNode);
            fParentMonitor.monitorExistingJob(subjobID, -1);

            return;
        }
        else if (e.getSource() == fSubjobDisplayJobLogMenuItem)
        {
            String subjobID = fMonitorTreeSubjobNodes.get(
                fCurrentSelectedNode);

            String[] showLogParms = new String[10];
            showLogParms[0] = "-machine";
            showLogParms[1] = fStaxMachine;
            showLogParms[2] = "-machineNickname";
            showLogParms[3] = fStaxMachineNickname;
            showLogParms[4] = "-name";
            showLogParms[5] = fStaxServiceName.toUpperCase() +
                "_Job_" + subjobID;
            showLogParms[6] = "-fontName";
            showLogParms[7] = fLogViewerFontName;
            showLogParms[8] = "-saveAsDirectory";
            showLogParms[9] = fSaveAsDirectory;

            // Pass the STAX/JobMonitor/Controller handle to the log viewer
            // (since it won't be deleted if the Job Monitor window is closed)

            STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                this, fParentMonitor.fHandle, showLogParms);

            return;
        }
        else if (e.getSource() == fSubjobDisplayJobUserLogMenuItem)
        {
            String subjobID = fMonitorTreeSubjobNodes.get(
                fCurrentSelectedNode);

            String[] showLogParms = new String[10];
            showLogParms[0] = "-machine";
            showLogParms[1] = fStaxMachine;
            showLogParms[2] = "-machineNickname";
            showLogParms[3] = fStaxMachineNickname;
            showLogParms[4] = "-name";
            showLogParms[5] = fStaxServiceName.toUpperCase() +
                "_Job_" + subjobID + "_User";
            showLogParms[6] = "-fontName";
            showLogParms[7] = fLogViewerFontName;
            showLogParms[8] = "-saveAsDirectory";
            showLogParms[9] = fSaveAsDirectory;

            // Pass the STAX/JobMonitor/Controller handle to the log viewer
            // (since it won't be deleted if the Job Monitor window is closed)

            STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                this, fParentMonitor.fHandle, showLogParms);

            return;
        }
        else if (e.getSource() == fSubjobTerminateMenuItem)
        {
            String subjobID = fMonitorTreeSubjobNodes.get(
                fCurrentSelectedNode);

            int confirmation = JOptionPane.showConfirmDialog(this,
                "Are you certain that you want\n" +
                "to terminate Job # " + subjobID + " ?",
                "Confirm Job Termination",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            String request = "TERMINATE JOB " + subjobID;

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    fParentMonitor,
                    "An error was encountered while attempting to " +
                    "terminate jobID " + subjobID + ", RC: " + result.rc +
                    ", Result: " + result.result);
            }

            return;
        }
        else if (e.getSource() == fDisplayJobLog)
        {
            fBlockPopupMenu.setVisible(false);

            String[] showLogParms = new String[10];
            showLogParms[0] = "-machine";
            showLogParms[1] = fStaxMachine;
            showLogParms[2] = "-machineNickname";
            showLogParms[3] = fStaxMachineNickname;
            showLogParms[4] = "-name";
            showLogParms[5] = fStaxServiceName.toUpperCase() +
                "_Job_" + fJobNumber;
            showLogParms[6] = "-fontName";
            showLogParms[7] = fLogViewerFontName;
            showLogParms[8] = "-saveAsDirectory";
            showLogParms[9] = fSaveAsDirectory;

            // Pass the STAX/JobMonitor/Controller handle to the log viewer
            // (since it won't be deleted if the Job Monitor window is closed)

            STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                this, fParentMonitor.fHandle, showLogParms);

            fBlockPopupMenu.setVisible(false);

            return;
        }
        else if (e.getSource() == fDisplayJobUserLog)
        {
            fBlockPopupMenu.setVisible(false);

            String[] showLogParms = new String[10];
            showLogParms[0] = "-machine";
            showLogParms[1] = fStaxMachine;
            showLogParms[2] = "-machineNickname";
            showLogParms[3] = fStaxMachineNickname;
            showLogParms[4] = "-name";
            showLogParms[5] = fStaxServiceName.toUpperCase() +
                "_Job_" + fJobNumber + "_User";
            showLogParms[6] = "-fontName";
            showLogParms[7] = fLogViewerFontName;
            showLogParms[8] = "-saveAsDirectory";
            showLogParms[9] = fSaveAsDirectory;

            // Pass the STAX/JobMonitor/Controller handle to the log viewer
            // (since it won't be deleted if the Job Monitor window is closed)

            STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                this, fParentMonitor.fHandle, showLogParms);

            fBlockPopupMenu.setVisible(false);

            return;
        }
        else if (e.getSource() == fFileExit)
        {
            if (!fMonitorStopped)
            {
                stopMonitor();
            }

            closeMonitor();
            return;
        }
        else if ((e.getSource() == fFileResubmit) ||
                 (e.getSource() == fFileExitAndResubmit))
        {
            try
            {
                fParentMonitor.addMonitoredJob(new STAXMonitorFrame(
                    true, fParentMonitor, fStaxMachine, fStaxServiceName,
                    fStaxMachineNickname, fJobNumber, fEventMachine,
                    fEventServiceName, fXmlFileName, fXmlFileMachine,
                    fFunction, fArgs, fJobName, fScripts,
                    fShowNoSTAXMonitorInformation, fLimitMessages,
                    fPluginJarsVector, fScriptFilesVector,
                    fScriptFilesMachineName, fClearLogs,
                    fLogTCElapsedTime, fLogTCNumStarts, fLogTCStartStop,
                    fAutoMonitorSubjobs, fPythonOutput, fPythonLogLevel,
                    fBreakpointTriggerVector, fBreakpointFirstFunction,
                    fBreakpointSubjobFirstFunction));
            }
            catch (STAFException ex)
            {
                STAXMonitorUtil.showErrorDialog(
                    this, ex.getMessage(),
                    new Font("Courier", Font.PLAIN, 12));

                return;
            }

            if (e.getSource() == fFileExitAndResubmit)
            {
                if (!fMonitorStopped)
                {
                    stopMonitor();
                }

                closeMonitor();
            }

            return;
        }
        else if (e.getSource() == fProcessMenu)
        {
            STAXMonitorExtension plugin = fViewablePlugins.get(e.getSource());

            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(plugin.getComponent());
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.addTab(plugin.getTitle(), scrollPane);

                    int index = fActiveElementsPane.getTabCount();
                    fActiveElementsPane.setBackgroundAt(
                        index - 1, Color.white);
                    fActiveElementsPane.setForegroundAt(
                        index - 1, Color.darkGray);
                }

                fViewableComponents.put(plugin.getTitle(), scrollPane);
            }
            else
            {
                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.remove(
                        fViewableComponents.get(plugin.getTitle()));
                }

                fViewableComponents.remove(plugin.getTitle());
            }

            return;
        }
        else if (e.getSource() == fSTAFCmdMenu)
        {
            STAXMonitorExtension plugin = fViewablePlugins.get(e.getSource());

            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(plugin.getComponent());
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.addTab(plugin.getTitle(), scrollPane);

                    int index = fActiveElementsPane.getTabCount();
                    fActiveElementsPane.setBackgroundAt(
                        index - 1, Color.white);
                    fActiveElementsPane.setForegroundAt(
                        index - 1, Color.darkGray);
                }

                fViewableComponents.put(plugin.getTitle(), scrollPane);
            }
            else
            {
                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.remove(
                        fViewableComponents.get(plugin.getTitle()));
                }

                fViewableComponents.remove(plugin.getTitle());
            }

            return;
        }
        else if (e.getSource() == fSubjobMenu)
        {
            STAXMonitorExtension plugin = fViewablePlugins.get(e.getSource());

            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(plugin.getComponent());
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.addTab(plugin.getTitle(), scrollPane);

                    int index = fActiveElementsPane.getTabCount();
                    fActiveElementsPane.setBackgroundAt(
                        index - 1, Color.white);
                    fActiveElementsPane.setForegroundAt(
                        index - 1, Color.darkGray);
                }

                fViewableComponents.put(plugin.getTitle(), scrollPane);
            }
            else
            {
                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.remove(
                        fViewableComponents.get(plugin.getTitle()));
                }

                fViewableComponents.remove(plugin.getTitle());
            }

            return;
        }
        else if (e.getSource() == fDebugMenu)
        {
            STAXMonitorExtension plugin = fViewablePlugins.get(e.getSource());

            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(plugin.getComponent());
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.addTab(
                        plugin.getTitle(), plugin.getComponent());

                    int index = fActiveElementsPane.getTabCount();
                    fActiveElementsPane.setBackgroundAt(
                        index - 1, Color.white);
                    fActiveElementsPane.setForegroundAt(
                        index - 1, Color.darkGray);
                }

                fViewableComponents.put(plugin.getTitle(),
                                        plugin.getComponent());
            }
            else
            {
                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_ACTIVE)
                {
                    fActiveElementsPane.remove(
                        fViewableComponents.get(plugin.getTitle()));
                }

                fViewableComponents.remove(plugin.getTitle());
            }

            return;
        }
        else if (e.getSource() == fActiveJobElementsMenu)
        {
            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(fMonitorTree);
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                fActiveElementsPane.add("Active Job Elements", scrollPane);
                fViewableComponents.put("Active Job Elements", scrollPane);
            }
            else
            {
                fActiveElementsPane.remove(
                    fViewableComponents.get("Active Job Elements"));
                fViewableComponents.remove("Active Job Elements");
            }

            return;
        }
        else if (e.getSource() == fTestcaseInfoMenu)
        {
            STAXMonitorExtension plugin = fViewablePlugins.get(e.getSource());

            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(plugin.getComponent());
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_INFO)
                {
                    fInfoPane.addTab(plugin.getTitle() + " ", scrollPane);

                    int index = fInfoPane.getTabCount();
                    fInfoPane.setBackgroundAt(index - 1, Color.white);
                    fInfoPane.setForegroundAt(index - 1, Color.darkGray);
                }

                fViewableComponents.put(plugin.getTitle() + " ", scrollPane);
            }
            else
            {
                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_INFO)
                {
                    fInfoPane.remove(
                        fViewableComponents.get(plugin.getTitle() + " "));
                }

                fViewableComponents.remove(plugin.getTitle() + " ");
            }

            return;
        }
        else if (e.getSource() == fTestcaseStatusMenu)
        {
            STAXMonitorExtension plugin = fViewablePlugins.get(e.getSource());

            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane = new JScrollPane(plugin.getComponent());
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_INFO)
                {
                    fStatusPane.addTab(plugin.getTitle(), scrollPane);

                    int index = fStatusPane.getTabCount();
                    fStatusPane.setBackgroundAt(index - 1, Color.white);
                    fStatusPane.setForegroundAt(index - 1, Color.darkGray);
                }

                fViewableComponents.put(plugin.getTitle(), scrollPane);

            }
            else
            {
                int pluginType = plugin.getExtensionType();

                if (pluginType == STAXMonitorFrame.EXTENSION_INFO)
                {
                    fStatusPane.remove(
                        fViewableComponents.get(plugin.getTitle()));
                }

                fViewableComponents.remove(plugin.getTitle());
            }

            return;
        }
        else if (e.getSource() == fCurrentSelectionMenu)
        {
            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane =
                    new JScrollPane(fSelectionDetailsPanel);
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
                fInfoPane.add("Current Selection", scrollPane);
                fViewableComponents.put("Current Selection", scrollPane);

                int index = fInfoPane.getTabCount();
                fInfoPane.setBackgroundAt(index - 1,
                    Color.white);
                fInfoPane.setForegroundAt(index - 1,
                    Color.darkGray);
            }
            else
            {
                fInfoPane.remove(fViewableComponents.get("Current Selection"));
                fViewableComponents.remove("Current Selection");
            }

            return;
        }
        else if (e.getSource() == fMessagesMenu)
        {
            if (((JCheckBoxMenuItem)e.getSource()).isSelected())
            {
                JScrollPane scrollPane =
                    new JScrollPane(fMessageTable);
                scrollPane.setVerticalScrollBarPolicy(
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
                scrollPane.setHorizontalScrollBarPolicy(
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

                fInfoPane.add("Messages", scrollPane);
                fViewableComponents.put("Messages", scrollPane);
            }
            else
            {
                fInfoPane.remove(fViewableComponents.get("Messages"));
                fViewableComponents.remove("Messages");
            }

            return;
        }
        else
        {
            return;
        }

        String blockNode = fMonitorTreeBlockNodes.get(fCurrentSelectedNode);

        String request = action + " JOB " + fJobNumber + " BLOCK " +
                         STAFUtil.wrapData(blockNode);

        STAFResult result = fHandle.submit2(
            fStaxMachine, fStaxServiceName, request);

        if (result.rc != 0)
        {
            STAXMonitorUtil.showErrorDialog(
                this,
                "An error was encountered while attempting to " + action +
                ", RC: " + result.rc + ", Result: " + result.result);
        }
    }

    public void stateChanged(ChangeEvent e)
    {
        JTabbedPane pane = (JTabbedPane)e.getSource();

        for (int i = 0; i < pane.getTabCount(); i++)
        {
            pane.setBackgroundAt(i, Color.white);
            pane.setForegroundAt(i, Color.darkGray);
        }

        int selected = ((JTabbedPane)e.getSource()).getSelectedIndex();

        if (selected != -1)
        {
            pane.setForegroundAt(selected, Color.black);
        }
    }

    void handleJobEvent(Map propertyMap)
    {
        String block = (String)propertyMap.get("block");

        String status = (String)propertyMap.get("status");

        if ((!status.equals("end")) && (!status.equals("stop")))
        {
            handleBlock(block, "Running");
        }

        String type = (String)propertyMap.get("type");

        // At this point, all of the blocks have been added

        if (type.equals("block"))
        {
            if (status.equals("end"))
            {
                // Remove a block when get a "end" status message as this
                // indicates the block has ended (normally or terminated).
                // Note that a block type message with a "terminate" status
                // indicates that a request was made to terminate a block,
                // but the block may not have ended yet so it should
                // continue to be displayed by the STAX Monitor until an
                // "end" status message for the block is received.

                synchronized(fMonitorTreeModel)
                {
                    synchronized(fMonitorTreeBlocks)
                    {
                        fMonitorTreeModel.removeNodeFromParent(
                            fMonitorTreeBlocks.get(block));
                        fMonitorTreeBlockNodes.remove(
                            fMonitorTreeBlocks.get(block));
                        fMonitorTreeBlocks.remove(block);
                    }

                    fCurrentSelection.setText("");
                }
            }
            else if (status.equals("hold"))
            {
                fMonitorTreeBlocks.get(block).setBlockStatus(
                    STAXMonitorTreeNode.blockHeld);

                fMonitorTree.updateUI();
            }
            else if (status.equals("release"))
            {
                fMonitorTreeBlocks.get(block).setBlockStatus(
                    STAXMonitorTreeNode.blockRunning);

                fMonitorTree.updateUI();
            }
        }
        else if (type.equals("command"))
        {
            String commandName = (String)propertyMap.get("name");
            String location = (String)propertyMap.get("location");
            String requestNumber = (String)propertyMap.get("requestNumber");

            if (status.equals("start"))
            {
                String service = (String)propertyMap.get("service");
                String request = (String)propertyMap.get("request");

                handleCommand(commandName, block, location, requestNumber,
                              service, request);

            }
            else if (status.equals("stop"))
            {
                String commandID = commandName + ";" + location + ";"
                                   + requestNumber;

                if (fCmdIDHashtable.containsKey(commandID))
                {
                    fMonitorTreeModel.removeNodeFromParent(
                        fCmdIDHashtable.get(commandID));
                    fMonitorTreeCommandNodes.remove(
                        fCmdIDHashtable.get(commandID));
                    fMonitorTreeCommandStartTimes.remove(
                        fCmdIDHashtable.get(commandID));
                    fCmdIDHashtable.remove(commandID);
                    fDataHashtable.remove(commandID);
                }
            }
        }
        else if (type.equals("process"))
        {
            String processName = (String)propertyMap.get("name");
            String location = (String)propertyMap.get("location");
            String handleNumber = (String)propertyMap.get("handle");

            if (status.equals("start"))
            {
                handleProcess(propertyMap, block, processName, location,
                              handleNumber, true);
            }
            else if (status.equals("stop"))
            {
                String processID = processName + ";" + location + ";" +
                    handleNumber;

                if (fProcIDHashtable.containsKey(processID))
                {
                    synchronized(fMonitorTreeModel)
                    {
                        STAXMonitorTreeNode processNode =
                            fProcIDHashtable.get(processID);

                        if (processNode == null)
                        {
                            System.out.println("processNode " +
                                               processID + " is null");
                            System.out.println(fMonitorTreeModel);
                            System.exit(99);
                        }

                        try
                        {
                            fMonitorTreeModel.removeNodeFromParent(processNode);
                        }
                        catch(NullPointerException e)
                        {
                            e.printStackTrace();
                        }

                        fMonitorTreeProcessNodes.remove(
                            fProcIDHashtable.get(processID));
                        fMonitorTreeProcessStartTimes.remove(
                            fProcIDHashtable.get(processID));
                        fProcIDHashtable.remove(processID);
                        fDataHashtable.remove(processID);
                    }
                }
                else
                {
                    // XXX got a stop before the start?
                }
            }
            else
            {
                // unknown status property
            }
        }
        else if (type.equals("subjob"))
        {
            String jobID = (String)propertyMap.get("jobID");

            if (status.equals("start"))
            {
                handleSubjob(jobID, block, propertyMap);

            } else if (status.equals("stop"))
            {
                if (fSubjobIDHashtable.containsKey(jobID))
                {
                    fMonitorTreeModel.removeNodeFromParent(
                        fSubjobIDHashtable.get(jobID));
                    fMonitorTreeSubjobNodes.remove(
                        fSubjobIDHashtable.get(jobID));
                    fMonitorTreeSubjobStartTimes.remove(
                        fSubjobIDHashtable.get(jobID));
                    fCmdIDHashtable.remove(jobID);
                    fDataHashtable.remove(jobID);
                }
            }
        }
        else if (type.equals("job"))
        {
            if (status.equals("end"))
            {
                fMonitorTreeModel.setRoot(new STAXMonitorTreeNode(null));

                String jobName = fJobName;

                if (fJobName == null) jobName = "";
                
                setTitle(kMonitorFrameTitle + " Machine:" +
                         fStaxMachine + " JobID:" + fJobNumber + " " +
                         jobName + " <Completed>");

                fMonitorTree.setVisible(false);
                stopMonitor();
            }
        }
    }

    void handleCommand(String commandName, String block, String location,
                       String requestNumber, String service, String request)
    {
        final String commandID =
            commandName + ";" + location + ";" + requestNumber;

        STAXMonitorTreeNode newNode =
            new STAXMonitorTreeNode(commandName,
            STAXMonitorTreeNode.commandNodeType);

        STAXMonitorTreeNode parentNode = fMonitorTreeBlocks.get(block);
        
        if (parentNode == null)
        {
            // The parentNode can be null if the block has ended

            // Asynchronously remove the command from the Active Job Elements
            // tree

            Runnable runnable = new Runnable()
            {
                public void run()
                {
                    if (fCmdIDHashtable.containsKey(commandID))
                    {
                        fMonitorTreeModel.removeNodeFromParent(
                            fCmdIDHashtable.get(commandID));
                        fMonitorTreeCommandNodes.remove(
                            fCmdIDHashtable.get(commandID));
                        fMonitorTreeCommandStartTimes.remove(
                            fCmdIDHashtable.get(commandID));
                        fCmdIDHashtable.remove(commandID);
                        fDataHashtable.remove(commandID);
                    }
                }
            };

            SwingUtilities.invokeLater(runnable);

            return;
        }

        fMonitorTreeModel.insertNodeInto(newNode, parentNode,
                                         parentNode.getChildCount());
        TreeNode[] parentNodes =
            fMonitorTreeModel.getPathToRoot(parentNode);
        fMonitorTree.expandPath(new TreePath(parentNodes));
        fCmdIDHashtable.put(commandID, newNode);
        fMonitorTreeCommandNodes.put(newNode, commandID);

        Vector<Vector<String>> cmdDataVector = new Vector<Vector<String>>();

        addRow(cmdDataVector, "Location", location);
        addRow(cmdDataVector, "Request Number", requestNumber);
        addRow(cmdDataVector, "Service", service);
        addRow(cmdDataVector, "Request", request);

        STAFResult queryResult = fHandle.submit2(
            fStaxMachine, fStaxServiceName,
            "QUERY JOB " + fJobNumber + " STAFCMD " + requestNumber);

        if (queryResult.rc != 0)
        {
            // Stafcmd has already completed, so asynchronously remove
            // it from the Active Job Elements tree

            Runnable runnable = new Runnable()
            {
                public void run()
                {
                    if (fCmdIDHashtable.containsKey(commandID))
                    {
                        fMonitorTreeModel.removeNodeFromParent(
                            fCmdIDHashtable.get(commandID));
                        fMonitorTreeCommandNodes.remove(
                            fCmdIDHashtable.get(commandID));
                        fMonitorTreeCommandStartTimes.remove(
                            fCmdIDHashtable.get(commandID));
                        fCmdIDHashtable.remove(commandID);
                        fDataHashtable.remove(commandID);
                    }
                }
            };
            SwingUtilities.invokeLater(runnable);

            return;
        }

        Map stafcmdMap = (HashMap)queryResult.resultObj;

        String startTimestamp = (String)stafcmdMap.get("startTimestamp");
        String startDate = startTimestamp.substring(0, 8);
        String startTime = startTimestamp.substring(9);

        fMonitorTreeCommandStartTimes.put(
            newNode, STAXMonitorUtil.getCalendar2(startDate, startTime));

        addRow(cmdDataVector, "Started", startDate + "-" + startTime);

        fDataHashtable.put(commandID, cmdDataVector);
    }

    void handleSubjob(String jobID, String block, Map propertyMap)
    {
        String jobName = (String)propertyMap.get("jobName");
        String function = (String)propertyMap.get("function");
        String jobFile = (String)propertyMap.get("jobfile");
        String jobFileMachine = (String)propertyMap.get("jobfilemachine");
        String functionArgs = (String)propertyMap.get("functionargs");
        String clearLogs = (String)propertyMap.get("clearlogs");
        String autoMonitor = (String)propertyMap.get("monitor");
        String logTCElapsedTime = (String)propertyMap.get("logtcelapsedtime");
        String logTCNumStarts = (String)propertyMap.get("logtcnumstarts");
        String logTCStartStop = (String)propertyMap.get("logtcstartstop");
        String pythonOutput = (String)propertyMap.get("pythonoutput");
        String pythonLogLevel = (String)propertyMap.get("pythonloglevel");
        String scriptFilesMachine = (String)propertyMap.get(
            "scriptfilesmachine");

        // holdJob will be null if the sub-job was not specified to be held
        // Otherwise, holdJob contains the hold timeout in milliseconds, with
        // a hold timeout of 0 indicating to job the job indefinitely
        String holdJob = (String)propertyMap.get("hold");

        String startDate = (String)propertyMap.get("startdate");
        String startTime = (String)propertyMap.get("starttime");

        String subjobText = "Job " + jobID;

        if (!(jobName.equals("")))
        {
            subjobText += " - " + jobName;
        }

        STAXMonitorTreeNode newNode =
            new STAXMonitorTreeNode(subjobText,
            STAXMonitorTreeNode.subjobNodeType);

        STAXMonitorTreeNode parentNode = fMonitorTreeBlocks.get(block);
        
        fMonitorTreeModel.insertNodeInto(newNode, parentNode,
                                         parentNode.getChildCount());
        TreeNode[] parentNodes =
            fMonitorTreeModel.getPathToRoot(parentNode);

        fMonitorTree.expandPath(new TreePath(parentNodes));

        fSubjobIDHashtable.put(jobID, newNode);
        fMonitorTreeSubjobNodes.put(newNode, jobID);

        fMonitorTreeSubjobStartTimes.put(
            newNode, STAXMonitorUtil.getCalendar2(startDate, startTime));

        Vector<Vector<String>> subjobDataVector = new Vector<Vector<String>>();

        addRow(subjobDataVector, "Job ID", jobID);
        addRow(subjobDataVector, "Job Name", jobName);
        addRow(subjobDataVector, "Clear Logs", clearLogs);
        addRow(subjobDataVector, "Log TC Elapsed Time", logTCElapsedTime);
        addRow(subjobDataVector, "Log TC Num Starts", logTCNumStarts);
        addRow(subjobDataVector, "Log TC Start/Stop", logTCStartStop);
        addRow(subjobDataVector, "Python Output", pythonOutput);
        addRow(subjobDataVector, "Python Log Level", pythonLogLevel);
        addRow(subjobDataVector, "Job File", jobFile);

        if (!jobFileMachine.equals(""))
        {
            addRow(subjobDataVector, "Job File Machine", jobFileMachine);
        }

        addRow(subjobDataVector, "Function", function);
        addRow(subjobDataVector, "Function Args", functionArgs);

        if (propertyMap.get("scriptFileList") instanceof java.util.List)
        {
            java.util.List scriptFileList = (java.util.List)propertyMap.get(
                "scriptFileList");

            for (int i = 0; i < scriptFileList.size(); i++)
            {
                if (i == 0 && !scriptFilesMachine.equals(""))
                {
                    addRow(subjobDataVector, "Script Files Machine",
                           scriptFilesMachine);
                }

                addRow(subjobDataVector, "Script File #" + (i + 1),
                       (String)scriptFileList.get(i));
            }
        }

        if (propertyMap.get("scriptList") instanceof java.util.List)
        {
            java.util.List scriptList = (java.util.List)propertyMap.get(
                "scriptList");

            for (int i = 0; i < scriptList.size(); i++)
            {
                addRow(subjobDataVector, "Script #" + (i + 1),
                       (String)scriptList.get(i));
            }
        }

        addRow(subjobDataVector, "Started", startDate + "-" + startTime);
        addRow(subjobDataVector, "Block", block);

        fDataHashtable.put(subjobText, subjobDataVector);

        if ((fAutoMonitorSubjobs == STAXMonitorFrame.AUTOMONITOR_ALWAYS) ||
            (fAutoMonitorSubjobs == STAXMonitorFrame.AUTOMONITOR_RECOMMENDED
            && autoMonitor.equals("true")))
        {
            fParentMonitor.monitorExistingJob(jobID, -1);

            // After starting to monitor the sub-job, if a <job-hold> element
            // was specified with an "if" element that evaluated to a true
            // value (as indicated by holdJob != null), release the sub-job.
            // This ensures that the sub-job is monitored from the beginning
            // (e.g. will show all messages sent to the STAX Monitor).
            
            if (holdJob != null)
            {
                STAFResult result = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, "RELEASE JOB " + jobID);

                if (result.rc != STAFResult.Ok && result.rc != BlockNotHeld &&
                    result.rc != STAFResult.DoesNotExist)
                {
                    STAXMonitorUtil.showErrorDialog(
                        this,
                        "An error was encountered while attempting to " +
                        "release a subjob with JobID=" + jobID +
                        ", RC: " + result.rc + ", Result: " + result.result);
                }
            }    
        }

        requestFocus();
    }

    void handleProcess(Map processMap, String block, String processName,
                       String location, String handleNumber, boolean event)
    {
        final String processID =
            processName + ";" + location + ";" + handleNumber;

        STAXMonitorTreeNode newNode = new STAXMonitorTreeNode(processName,
            STAXMonitorTreeNode.processNodeType);

        synchronized(fMonitorTreeBlocks)
        {
            STAXMonitorTreeNode parentNode = fMonitorTreeBlocks.get(block);

            synchronized(fMonitorTreeModel)
            {
                fMonitorTreeModel.insertNodeInto(newNode, parentNode,
                                                 parentNode.getChildCount());
                TreeNode[] parentNodes =
                    fMonitorTreeModel.getPathToRoot(parentNode);

                fMonitorTree.expandPath(new TreePath(parentNodes));
            }
        }

        fProcIDHashtable.put(processID, newNode);
        fMonitorTreeProcessNodes.put(newNode, processID);

        if (event)
        {
            // Submit a STAX QUERY JOB <JobID> PROCESS <location:handle>
            // request to get the start timestamp for the process.
            // The start timestamp was not available when the process
            // start event was generated.
        
            STAFResult queryResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName,
                "QUERY JOB " + fJobNumber + " PROCESS " +
                location + ":" + handleNumber);

            if (queryResult.rc != 0)
            {
                // Process has already completed, so asynchronously remove
                // it from the Active Job Elements tree

                Runnable runnable = new Runnable()
                {
                    public void run()
                    {
                        if (fProcIDHashtable.containsKey(processID))
                        {
                            fMonitorTreeModel.removeNodeFromParent(
                                fProcIDHashtable.get(processID));
                            fMonitorTreeCommandNodes.remove(
                                fProcIDHashtable.get(processID));
                            fMonitorTreeCommandStartTimes.remove(
                                fProcIDHashtable.get(processID));
                            fProcIDHashtable.remove(processID);
                            fDataHashtable.remove(processID);
                        }
                    }
                };
                SwingUtilities.invokeLater(runnable);

                return;
            }

            // Replace the process map with the query process map since
            // it contains all of the information about the process
            processMap = (HashMap)queryResult.resultObj;
        }

        String startTimestamp = (String)processMap.get("startTimestamp");

        // startTimestamp format is YYYYMMDD-HH:MM:SS
        String startDate = startTimestamp.substring(0, 8);
        String startTime = startTimestamp.substring(9);

        fMonitorTreeProcessStartTimes.put(
            newNode, STAXMonitorUtil.getCalendar2(startDate, startTime));

        Vector<Vector<String>> procDataVector = new Vector<Vector<String>>();
        STAXMonitorUtil.assignProcessInfo(processMap, procDataVector);
        
        fDataHashtable.put(processID, procDataVector);
    }

    Vector<STAXMonitorTreeNode> handleBlock(String block, String status)
    {
        Vector<STAXMonitorTreeNode> heldBlocks =
            new Vector<STAXMonitorTreeNode>();
        
        if (block.indexOf(".") == -1)
        {
            // will occur only for main block
            if (!fMonitorTreeBlocks.containsKey(block))
            {
                // starting main
                STAXMonitorTreeNode newNode = new STAXMonitorTreeNode(
                    block, STAXMonitorTreeNode.blockNodeType);

                if (status.startsWith("Held"))
                {
                    heldBlocks.add(newNode);
                }

                fMonitorTreeBlocks.put(block, newNode);
                fMonitorTreeBlockNodes.put(newNode, block);

                fMonitorTreeModel.setRoot(newNode);
            }
        }
        else
        {
            String firstBlock = block.substring(0, block.indexOf("."));

            if (!fMonitorTreeBlocks.containsKey(firstBlock))
            {
                 // this can occur if we are starting to monitor
                 // after the jobs has already started.  need to
                 // seed the hashmap with the first block (main)

                 STAXMonitorTreeNode newNode = new
                     STAXMonitorTreeNode(firstBlock,
                     STAXMonitorTreeNode.blockNodeType);

                 fMonitorTreeBlocks.put(firstBlock, newNode);
                 fMonitorTreeBlockNodes.put(newNode, firstBlock);

                 fMonitorTreeModel.insertNodeInto(newNode,
                     (STAXMonitorTreeNode)fMonitorTreeModel.getRoot(), 0);

                 fMonitorTree.expandPath(new TreePath(
                     (STAXMonitorTreeNode)fMonitorTreeModel.getRoot()));
            }

            String blockSection;
            int lastIndex = 0;
            lastIndex = block.indexOf(".");
            boolean lastBlock = false;

            // we've already processed the first segment, main
            do
            {
                int nextIndex = block.indexOf(".", lastIndex + 1);

                if (nextIndex == -1)
                {
                    // this means we've reach the final block segment
                    blockSection = block;
                    lastBlock = true;
                }
                else
                {
                    blockSection = block.substring(0, nextIndex);
                }

                if (!fMonitorTreeBlocks.containsKey(blockSection))
                {
                    String lastBlockSection = blockSection.substring(
                        blockSection.lastIndexOf(".")+1);

                    STAXMonitorTreeNode newNode = new STAXMonitorTreeNode(
                        lastBlockSection, STAXMonitorTreeNode.blockNodeType);

                    if (lastBlock && status.startsWith("Held"))
                    {
                        heldBlocks.add(newNode);
                    }

                    STAXMonitorTreeNode parentNode = fMonitorTreeBlocks.get(
                        blockSection.substring(
                            0, blockSection.lastIndexOf(".")));

                    fMonitorTreeModel.insertNodeInto(
                        newNode, parentNode, parentNode.getChildCount());
                    fMonitorTreeBlocks.put(blockSection, newNode);
                    fMonitorTreeBlockNodes.put(newNode, blockSection);

                    TreeNode[] parentNodes = fMonitorTreeModel.getPathToRoot(
                        parentNode);

                    fMonitorTree.expandPath(new TreePath(parentNodes));
                }
                lastIndex = nextIndex;
            }
            while (!blockSection.equals(block));
        }

        return heldBlocks;
    }


    void handleJobMessage(Map propertyMap)
    {
        String message = (String)propertyMap.get("messagetext");
        
        int timestampIndex = message.indexOf(" ");

        Object rowData[] = new Object[2];
        rowData[0] = message.substring(0, timestampIndex);

        if (fMessageTableModel.getRowCount() ==
            (new Integer(fLimitMessages)).intValue())
        {
            fMessageTableModel.removeRow(0);
        }

        if (fMessageTableModel.getRowCount() == 0)
        {
            int firstMsgLen = (message.substring(timestampIndex + 1)).length();

            StringBuffer padding = new StringBuffer();

            for (int i = 0; i < 200 - firstMsgLen; i++)
            {
                padding.append(" ");
            }

            rowData[1] = message.substring(timestampIndex + 1) +
                padding.toString();
        }
        else
        {
            rowData[1] = message.substring(timestampIndex + 1);
        }

        fMessageTableModel.addRow(rowData);

        STAXMonitorUtil.updateRowHeights(fMessageTable, 1, fMessageFontName);
        STAXMonitorUtil.sizeColumnsToFitText(fMessageTable);

        Rectangle rect = fMessageTable.getVisibleRect();
        Point point = rect.getLocation();
        point.setLocation(rect.getX() + rect.getWidth(),
                          rect.getY() + rect.getHeight());
        int lastRow = fMessageTable.rowAtPoint(point);

        if (lastRow > (fMessageTableModel.getRowCount() - 5))
        {
            fMessageTable.scrollRectToVisible(new
                Rectangle(0,fMessageTable.getPreferredSize().height,0,0));

            fMessageTable.scrollRectToVisible(new
                Rectangle(0,fMessageTable.getPreferredSize().height,0,0));
        }
    }

    public void addActiveJobElementsNode(String type, String id, String block,
        ImageIcon image, JComponent component, Vector detailsData)
    {
        // pass the ID as the NodeText
        addActiveJobElementsNode(type, id, block, id, image, component,
                                 detailsData);
    }

    public void addActiveJobElementsNode(String type, String id, String block,
        String nodeText, ImageIcon image, JComponent component,
        Vector detailsData)
    {
        STAXMonitorTreeNode newNode =
            new STAXMonitorTreeNode(nodeText, type, image, component);

        STAXMonitorTreeNode parentNode = fMonitorTreeBlocks.get(block);
        fMonitorTreeModel.insertNodeInto(
            newNode, parentNode, parentNode.getChildCount());
        TreeNode[] parentNodes = fMonitorTreeModel.getPathToRoot(parentNode);

        fMonitorTree.expandPath(new TreePath(parentNodes));

        fPluginNodeHashtable.put(id, newNode);
        fPluginNodeToIDHashtable.put(newNode, id);
        fPluginNodeToNodeTextHashtable.put(newNode, nodeText);
        fPluginNodeDetailsHashtable.put(newNode, detailsData);
    }

    public void removeActiveJobElementsNode(String id, String block)
    {
        STAXMonitorTreeNode nodeToRemove = fPluginNodeHashtable.get(id);

        fMonitorTreeModel.removeNodeFromParent(nodeToRemove);
        fPluginNodeHashtable.remove(id);
        fPluginNodeToIDHashtable.remove(nodeToRemove);
        fPluginNodeToNodeTextHashtable.remove(nodeToRemove);
        fPluginNodeDetailsHashtable.remove(nodeToRemove);
    }

    public void setActiveJobElementsNodeText(String id, String text)
    {
        STAXMonitorTreeNode node = fPluginNodeHashtable.get(id);
        node.setPluginText(text);

        fMonitorTree.updateUI();

        if (fCurrentSelectedNode != null)
        {
            synchronized(fMonitorTree)
            {
                fMonitorTree.setSelectionPath(
                    new TreePath(fCurrentSelectedNode.getPath()));
            }
        }
    }

    public void keyPressed(KeyEvent e)
    {
            }

    public void keyReleased(KeyEvent e) {}
    public void keyTyped(KeyEvent e) {}

    public void mouseClicked(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}

    public void mousePressed(MouseEvent e)
    {
        displayPopup(e);
    }

    public void mouseReleased(MouseEvent e)
    {
        displayPopup(e);
    }

    public void displayPopup(MouseEvent e)
    {
        if (e.isPopupTrigger())
        {
            TreePath treePath =
                fMonitorTree.getPathForLocation(e.getX(), e.getY());

            if(treePath != null)
            {
                STAXMonitorTreeNode treeNode =
                    (STAXMonitorTreeNode)treePath.getLastPathComponent();

                if (treeNode.fNodeType == STAXMonitorTreeNode.blockNodeType)
                {
                    if (treeNode.fBlockStatus ==
                        STAXMonitorTreeNode.blockRunning)
                    {
                        fBlockHoldMenuItem.setEnabled(true);
                        fBlockReleaseMenuItem.setEnabled(false);
                        fBlockTerminateMenuItem.setEnabled(true);
                    }
                    else if (treeNode.fBlockStatus ==
                             STAXMonitorTreeNode.blockHeld)
                    {
                        fBlockHoldMenuItem.setEnabled(false);
                        fBlockReleaseMenuItem.setEnabled(true);
                        fBlockTerminateMenuItem.setEnabled(true);
                    }
                    else if (treeNode.fBlockStatus ==
                             STAXMonitorTreeNode.blockParentHeld)
                    {
                        fBlockHoldMenuItem.setEnabled(false);
                        fBlockReleaseMenuItem.setEnabled(false);
                        fBlockTerminateMenuItem.setEnabled(true);
                    }

                    fMonitorTree.setSelectionPath(treePath);
                    fBlockPopupMenu.show(
                        e.getComponent(), e.getX(), e.getY());
                }
                else if (treeNode.fNodeType ==
                         STAXMonitorTreeNode.subjobNodeType)
                {
                    fMonitorTree.setSelectionPath(treePath);
                    fSubjobPopupMenu.show(
                        e.getComponent(), e.getX(), e.getY());
                }
                else if (treeNode.fNodeType ==
                         STAXMonitorTreeNode.processNodeType)
                {
                    fProcessStopMenuItem.setEnabled(true);

                    fMonitorTree.setSelectionPath(treePath);
                    fProcessPopupMenu.show(
                        e.getComponent(), e.getX(), e.getY());
                }
            }
        }
    }

    public void valueChanged(TreeSelectionEvent e)
    {
        TreePath selectedPath = e.getPath();

        if ((e.getNewLeadSelectionPath() != null) &&
            (e.getNewLeadSelectionPath().getPath() != null) &&
            (e.getNewLeadSelectionPath().getPath()[0] != null))
        {
            String pathRoot =
                e.getNewLeadSelectionPath().getPath()[0].toString();

            if ((e.getNewLeadSelectionPath().getPath().length == 1) &&
                (pathRoot != null))
            {

                if (!(pathRoot.equals("main")))

                {
                        // selection has been removed
                    fCurrentSelection.setText("");
                        // next line is required due to Sun BugIDs 4398286, 4402032
                    fDetailsTable.setRowHeight(30);
                    fDetailsTableModel =
                        new STAXMonitorTableModel(fDataColumns, 0);
                    fDetailsModelSorter.setModel(fDetailsTableModel);

                    updateDetailsTableRenderers();
                    STAXMonitorUtil.sizeColumnsToFitText(fDetailsTable);
                    fDetailsTable.setVisible(false);
                    fDetailsTable.getTableHeader().setVisible(false);

                    fBlockPopupMenu.setVisible(false);
                }
            }
        }

        if (fCurrentSelectedNode ==
            (STAXMonitorTreeNode)selectedPath.getLastPathComponent())
        {
            return;
        }

        fCurrentSelectedNode =
            (STAXMonitorTreeNode)selectedPath.getLastPathComponent();
        String id = fCurrentSelectedNode.toString();

        if (fCurrentSelectedNode == null)
        {
            return;
        }

        if (e.isAddedPath())
        {
            fCurrentSelection.setText(id);
            fDetailsTable.setVisible(true);
            fDetailsTable.getTableHeader().setVisible(true);

            if (fMonitorTreeProcessNodes.containsKey(fCurrentSelectedNode))
            {
                id = fMonitorTreeProcessNodes.get(fCurrentSelectedNode);
            }

            if (fMonitorTreeCommandNodes.containsKey(fCurrentSelectedNode))
            {
                id = fMonitorTreeCommandNodes.get(fCurrentSelectedNode);
            }

            if (id == null)
            {
                return;
            }

            if (fDataHashtable.containsKey(id))
            {
                int idIndex = id.indexOf(";");

                if (idIndex != -1)
                {
                    updateCurrentSelection(id.substring(0, idIndex),
                        (Vector)fDataHashtable.get(id));
                }
                else
                {
                    updateCurrentSelection(id, fDataHashtable.get(id));
                }
            }
            else if (fPluginNodeDetailsHashtable.containsKey(
                fCurrentSelectedNode))
            {
                String pluginNodeText = fPluginNodeToNodeTextHashtable.get(
                    fCurrentSelectedNode);
                updateCurrentSelection(
                    pluginNodeText, 
                    fPluginNodeDetailsHashtable.get(fCurrentSelectedNode));
            }
            else
            {
                // next line is required due to Sun BugIDs 4398286, 4402032
                fDetailsTable.setRowHeight(30);
                fDetailsTableModel = new STAXMonitorTableModel(
                    fDataColumns, 0);
                fDetailsModelSorter.setModel(fDetailsTableModel);

                updateDetailsTableRenderers();
                STAXMonitorUtil.sizeColumnsToFitText(fDetailsTable);
                fDetailsTable.setVisible(false);
                fDetailsTable.getTableHeader().setVisible(false);
            }

            previousTreePath = selectedPath;
        }
    }

    public void updateCurrentSelection(String title, Vector tableData)
    {
        if (fViewableComponents.get("Current Selection") == null)
        {
            return;
        }

        if (fInfoPane.getTabCount() > 0)
        {
            fInfoPane.setSelectedIndex(fInfoPane.
                indexOfTab("Current Selection"));
        }

        fCurrentSelection.setText(title);

        // next line is required due to Sun BugIDs 4398286, 4402032
        fDetailsTable.setRowHeight(30);
        fDetailsTableModel = new STAXMonitorTableModel(
            tableData, fDataColumns);
        fDetailsModelSorter.setModel(fDetailsTableModel);
        updateDetailsTableRenderers();
        STAXMonitorUtil.updateRowHeights(fDetailsTable, 1);
        STAXMonitorUtil.sizeColumnsToFitText(fDetailsTable);
        fDetailsTable.setVisible(true);
        fDetailsTable.getTableHeader().setVisible(true);
    }

    public void updateDetailsTableRenderers()
    {
        fDetailsTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.black));

        fDetailsTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fDetailsTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.black));

        fDetailsTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));
    }

    public String getJobNumber()
    {
        return fJobNumber;
    }

    public String getSTAXMachineNickname()
    {
        return fStaxMachineNickname;
    }

    public void addRow(Vector<Vector<String>> vector,
                       String name, String value)
    {
        Vector<String> newRow = new Vector<String>(2);
        newRow.add(name);
        newRow.add(value);
        vector.add(newRow);
    }

    public void addRecentLog(String queryRequest, String tooltip)
    {
        fParentMonitor.addRecentLog(queryRequest, tooltip);
    }

    public void run()
    {
        STAFResult getResult;
        boolean continueRunning = true;

        while (continueRunning)
        {
            // Use the ALL option to improve performance by getting multiple
            // messages, if available, off the queue at once

            getResult = fHandle.submit2("local", "QUEUE", "GET ALL WAIT");

            if (getResult.rc != STAFResult.Ok)
            {
                // It is possible to receive a HandleDoesNotExist if
                // the MonitorFrame is being closed

                continueRunning = false;
                break;  // Can't process any more messages 
            }

            java.util.List queueList = (java.util.List)getResult.resultObj;
            
            // Iterate through the messages got off our handle's queue

            Iterator queueIter = queueList.iterator();

            while (queueIter.hasNext())
            {
                Map queueMap = (Map)queueIter.next();
                String queueType = (String)queueMap.get("type");
            
                if (queueType == null)
                    continue;  // Ignore message

                if (queueType.equalsIgnoreCase("STAF/STAXMonitor/End"))
                {
                    continueRunning = false;
                    break; // Don't process any more messages on the queue
                }
                else if (!queueType.equalsIgnoreCase("STAF/Service/Event"))
                {
                    continue; // Ignore messages that don't have this type
                }

                // Process only STAF/Service/Event messages whose event type
                // matches the STAX service name and machine for which the
                // STAX Monitor is configured

                Map messageMap = (Map)queueMap.get("message");
                String type = (String)messageMap.get("type");

                if (!type.equalsIgnoreCase(fStaxServiceName + "/" +
                    fStaxConfigMachine + "/" + fJobNumber))
                {
                    continue; // Ignore messages that don't have this type
                }

                // Process STAF/Service/Event messages with matching event type

                String eventID = (String)messageMap.get("eventID");

                STAFResult ackResult = fHandle.submit2(
                    STAFHandle.ReqFireAndForget, fEventMachine,
                    fEventServiceName, "ACKNOWLEDGE EVENTID " + eventID);

                String subtype = (String)messageMap.get("subtype");

                final Map propertyMap = (Map)messageMap.get("propertyMap");

                if (subtype.equals("Message"))
                {
                    Runnable runnable = new Runnable()
                    {
                        public void run()
                        {
                            handleJobMessage(propertyMap);
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(runnable);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    catch (InvocationTargetException e)
                    {
                    }
                }
                else if (subtype.equals("Job") ||
                         subtype.equals("Block") ||
                         subtype.equals("Process") ||
                         subtype.equals("STAFCommand") ||
                         subtype.equals("SubJob") ||
                         subtype.equals("breakpoint"))
                {
                    Runnable runnable = new Runnable()
                    {
                        public void run()
                        {
                            handleJobEvent(propertyMap);
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(runnable);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    catch (InvocationTargetException e)
                    {
                    }
                }

                // Process the extensions

                Enumeration<STAXMonitorExtension> plugins =
                    fRegisteredPlugins.keys();

                while (plugins.hasMoreElements())
                {
                    final STAXMonitorExtension plugin = plugins.nextElement();

                    Vector<String> pluginRegisteredTypes =
                        fRegisteredPlugins.get(plugin);

                    if (pluginRegisteredTypes.contains(subtype.toLowerCase()))
                    {
                        Runnable runnable = new Runnable()
                        {
                            public void run()
                            {
                                plugin.handleEvent(propertyMap);
                            }
                        };

                        try
                        {
                            SwingUtilities.invokeAndWait(runnable);
                        }
                        catch (InterruptedException e)
                        {
                        }
                        catch (InvocationTargetException e)
                        {
                            e.printStackTrace();
                        }
                    }
                } // end while iterating through plugins
            } // end while iterating through the queue list
        } // end while continueRunning 
    }

    void startMonitor()
    {
        if (fStaxMachine.equals("") || fJobNumber.equals(""))
        {
            JOptionPane.showMessageDialog(
                this, "You must specify both a STAX Machine" +
                " and a Job Number",
                "Error starting Job Monitor", JOptionPane.ERROR_MESSAGE);
        }
        else
        {
            StringBuffer externalSubTypes = new StringBuffer();

            for (int i = 0; i < fExternalPluginNotificationTypes.size(); i++)
            {
                externalSubTypes.append("SUBTYPE ").append(
                    fExternalPluginNotificationTypes.elementAt(i)).append(" ");
            }

            String request = "REGISTER TYPE " + STAFUtil.wrapData(
                fStaxServiceName.toUpperCase() + "/" + fStaxConfigMachine +
                "/" + fJobNumber) +
                " SUBTYPE Job SUBTYPE Block SUBTYPE Process" +
                " SUBTYPE STAFCommand SUBTYPE Message" +
                " SUBTYPE Testcase SUBTYPE TestCaseStatus" +
                " SUBTYPE subjob SUBTYPE breakpoint" +
                " SUBTYPE breakpointtrigger SUBTYPE thread " +
                externalSubTypes.toString() +
                " MAXATTEMPTS 1 ACKNOWLEDGETIMEOUT 1000 BYHANDLE";

            STAFResult registerResult = fHandle.submit2(
                fEventMachine, fEventServiceName, request);

            if (registerResult.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    this, "Could not register for Job events\n\n" +
                    "STAF " + fEventMachine + " " + fEventServiceName +
                    " " + request + "\n\nRC: " + registerResult.rc +
                    ", Result: " + registerResult.result,
                    "Error Registering for Job Events");
            }
            else
            {
                String jobName = fJobName;

                if (fJobName == null) jobName = "";
                
                setTitle(kMonitorFrameTitle + " Machine:" +
                         fStaxMachine + " JobID:" + fJobNumber + " " +
                         jobName);
            }
        }
    }

    void stopMonitor()
    {
        fMonitorStopped = true;

        StringBuffer externalSubTypes = new StringBuffer();

        for (int i = 0; i < fExternalPluginNotificationTypes.size(); i++)
        {
            externalSubTypes.append("SUBTYPE ").append(
                fExternalPluginNotificationTypes.elementAt(i)).append(" ");
        }

        String request = "UNREGISTER TYPE " + STAFUtil.wrapData(
            fStaxServiceName.toUpperCase() + "/" + fStaxConfigMachine +
            "/" + fJobNumber) +
            " SUBTYPE Job SUBTYPE Block SUBTYPE Process" +
            " SUBTYPE STAFCommand SUBTYPE Testcase" +
            " SUBTYPE TestcaseStatus SUBTYPE Message SUBTYPE subjob" +
            " SUBTYPE breakpoint SUBTYPE breakpointtrigger" +
            " SUBTYPE thread " + externalSubTypes.toString();

        STAFResult unRegisterResult = fHandle.submit2(
            fEventMachine, fEventServiceName, request);

        if (unRegisterResult.rc != 0 &&
            unRegisterResult.rc != STAFResult.HandleDoesNotExist)
        {
            STAXMonitorUtil.showErrorDialog(
                this, "Could not unregister for Job events\n\n" +
                "STAF " + fEventMachine + " " + fEventServiceName +
                " " + request + "\n\nRC: " + unRegisterResult.rc +
                ", Result: " + unRegisterResult.result,
                "Error Unregistering for Job Events");
        }

        fHandle.submit2(
            "local", "QUEUE", "QUEUE TYPE STAF/STAXMonitor/End MESSAGE " +
            STAFUtil.wrapData(""));
    }

    void closeMonitor()
    {
        if (fParentMonitor != null)
        {
            fParentMonitor.monitorExiting(fJobNumber);
        }
        fContinueProcessMonitor = false;
        fContinueElapsedTime = false;

        STAXMonitorUtil.freeHandle(fHandle.getHandle());

        Enumeration<STAXMonitorExtension> plugins = fRegisteredPlugins.keys();

        while (plugins.hasMoreElements())
        {
            plugins.nextElement().term();
        }

        this.dispose();
    }

    public int getProcessMonitorInterval()
    {
        return fParentMonitor.getProcessMonitorInterval();
    }

    public int getElapsedTimeInterval()
    {
        return fParentMonitor.getElapsedTimeInterval();
    }

    class STAFProcessMonitor extends Thread
    {
        public void run()
        {
            final int frequency = getProcessMonitorInterval();

            if (frequency == 0)
                return;

            while (fContinueProcessMonitor)
            {
                int waitTime = frequency;

                final Enumeration<STAXMonitorTreeNode> processNodeKeys =
                    fMonitorTreeProcessNodes.keys();

                int numProcesses = fMonitorTreeProcessNodes.size();

                while (fContinueProcessMonitor &&
                       processNodeKeys.hasMoreElements())
                {
                    STAXMonitorTreeNode processNode = null;
                    String processID = null;

                    if (processNodeKeys.hasMoreElements())
                    {
                        processNode = processNodeKeys.nextElement();
                    }

                    if (processNode != null)
                    {
                        processID = fMonitorTreeProcessNodes.get(processNode);
                    }

                    if (processID != null)
                    {
                        int handleIndex = processID.lastIndexOf(";");
                        String handle = processID.substring(handleIndex + 1);
                        processID = processID.substring(0, handleIndex);

                        int locationIndex = processID.lastIndexOf(";");
                        String location = processID.substring(
                            locationIndex + 1);

                        String processName = processID.substring(
                            0, locationIndex);

                        String message = STAXMonitorUtil.
                            getMonitorMessage(location, handle);

                        final String processMessage = message;
                        final STAXMonitorTreeNode processNode1 = processNode;

                        Runnable runnable = new Runnable()
                        {
                            public void run()
                            {
                                processNode1.setProcessMonitorText(processMessage);
                                fMonitorTreeModel.nodeChanged(processNode1);

                                if (fCurrentSelectedNode != null)
                                {
                                    synchronized(fMonitorTree)
                                    {
                                        fMonitorTree.setSelectionPath(
                                            new TreePath(fCurrentSelectedNode.
                                            getPath()));
                                    }
                                }
                            }
                        };
                            SwingUtilities.invokeLater(runnable);
                    }
                }

                waitTime = frequency;
                try
                {
                    Thread.sleep(waitTime);
                }
                catch (InterruptedException ex)
                {
                }
            }
        }
    }

    class MonitorElapsedTime extends Thread
    {
        public void run()
        {
            final int waitTime = getElapsedTimeInterval();

            if (waitTime == 0)
                return;

            while (fContinueElapsedTime)
            {
                // handle Processes
                final Enumeration<STAXMonitorTreeNode> processElapsedTimeKeys =
                    fMonitorTreeProcessStartTimes.keys();

                Runnable processRunnable = new Runnable()
                {
                    public void run()
                    {
                        while (fContinueElapsedTime &&
                               processElapsedTimeKeys.hasMoreElements())
                        {
                            STAXMonitorTreeNode processNode =
                                processElapsedTimeKeys.nextElement();

                            Calendar processStarted = 
                                fMonitorTreeProcessStartTimes.get(processNode);

                            processNode.setElapsedTime(
                                STAXMonitorUtil.getElapsedTime(processStarted));

                            fMonitorTreeModel.nodeChanged(processNode);
                        }

                        if (fCurrentSelectedNode != null)
                        {
                            synchronized(fMonitorTree)
                            {
                                fMonitorTree.setSelectionPath(new
                                    TreePath(fCurrentSelectedNode.
                                        getPath()));
                            }
                        }
                    }
                };

                try
                {
                    SwingUtilities.invokeAndWait(processRunnable);
                }
                catch (InterruptedException ex)
                {
                     ex.printStackTrace();
                }
                catch (InvocationTargetException ex)
                {
                     ex.printStackTrace();
                }

                // handle Commands
                final Enumeration<STAXMonitorTreeNode> commandElapsedTimeKeys =
                    fMonitorTreeCommandStartTimes.keys();

                Runnable commandRunnable = new Runnable()
                {
                    public void run()
                    {
                        while (fContinueElapsedTime &&
                               commandElapsedTimeKeys.hasMoreElements())
                        {
                            STAXMonitorTreeNode commandNode =
                                commandElapsedTimeKeys.nextElement();
                            Calendar commandStarted =
                                fMonitorTreeCommandStartTimes.get(commandNode);

                            commandNode.setElapsedTime(
                               STAXMonitorUtil.getElapsedTime(commandStarted));

                            fMonitorTreeModel.nodeChanged(commandNode);
                        }

                        if (fCurrentSelectedNode != null)
                        {
                            synchronized(fMonitorTree)
                            {
                                fMonitorTree.setSelectionPath(
                                    new TreePath(
                                        fCurrentSelectedNode.getPath()));
                            }
                        }
                    }
                };

                try
                {
                    SwingUtilities.invokeAndWait(commandRunnable);
                }
                catch (InterruptedException ex)
                {
                     ex.printStackTrace();
                }
                catch (InvocationTargetException ex)
                {
                     ex.printStackTrace();
                }

                // handle Subjobs
                final Enumeration<STAXMonitorTreeNode> subjobElapsedTimeKeys =
                    fMonitorTreeSubjobStartTimes.keys();

                Runnable subjobRunnable = new Runnable()
                {
                    public void run()
                    {
                        while (fContinueElapsedTime &&
                               subjobElapsedTimeKeys.hasMoreElements())
                        {
                            STAXMonitorTreeNode subjobNode =
                                subjobElapsedTimeKeys.nextElement();
                            Calendar subjobStarted = 
                                fMonitorTreeSubjobStartTimes.get(subjobNode);

                            subjobNode.setElapsedTime(
                                STAXMonitorUtil.getElapsedTime(subjobStarted));

                            fMonitorTreeModel.nodeChanged(subjobNode);
                        }

                        if (fCurrentSelectedNode != null)
                        {
                            synchronized(fMonitorTree)
                            {
                                fMonitorTree.setSelectionPath(
                                    new TreePath(
                                        fCurrentSelectedNode.getPath()));
                            }
                        }
                    }
                };

                try
                {
                    SwingUtilities.invokeAndWait(subjobRunnable);
                }
                catch (InterruptedException ex)
                {
                     ex.printStackTrace();
                }
                catch (InvocationTargetException ex)
                {
                     ex.printStackTrace();
                }

                try
                {
                    Thread.sleep(waitTime);
                }
                catch (InterruptedException ex)
                {
                }
            }
        }
    }
}