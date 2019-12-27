/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2009                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import javax.swing.table.*;
import javax.swing.border.*;
import javax.swing.text.DefaultHighlighter;
import javax.swing.text.BadLocationException;
import javax.swing.plaf.metal.*;
import com.ibm.staf.*;
import java.util.*;
import java.util.List;
import java.awt.*;
import java.awt.event.*;
import java.awt.AWTEvent;
import java.lang.reflect.*;
import java.io.*;

public class STAXMonitorDebugExtension implements STAXMonitorExtension,
                                                  ActionListener,
                                                  MouseListener,
                                                  TreeSelectionListener
{
    static boolean sDebug = false;
    static boolean sDebugDetail = false;
    JPanel fPanel;
    JTabbedPane fTabbedPane;
    STAFHandle fHandle;
    String fStaxMachine;
    String fStaxServiceName;
    String fJobNumber;
    STAXMonitorFrame fMonitorFrame;
    String fTitle;
    JPopupMenu fTreeTablePopupMenu = new JPopupMenu();
    JMenuItem fTreeTableExpandAllMenuItem = new JMenuItem("Expand all");
    JMenuItem fTreeTableCollapseAllMenuItem = new JMenuItem("Collapse all");
    JPopupMenu fXmlSourcePopupMenu = new JPopupMenu();
    JMenuItem fXmlSourceScrollToCurrentLineMenuItem =
        new JMenuItem("Scroll to current line");

    Vector<String> fBreakpointsTableColumnNames;

    JButton fAddBreakpointFunctionButton;
    JDialog fAddBreakpointFunctionDialog;
    JTextField fAddBreakpointFunctionTextField;
    JButton fAddBreakpointFunctionAddButton;
    JButton fAddBreakpointFunctionCancelButton;

    JTable fBreakpointsTable;
    JButton fBreakpointLineAddButton;
    JButton fBreakpointRemoveButton;
    JButton fBreakpointRemoveAllButton;
    JDialog fAddBreakpointLineDialog;
    JTextField fAddBreakpointLineNumberTextField;
    JTextField fAddBreakpointLineFileTextField;
    JTextField fAddBreakpointLineMachineTextField;
    JButton fAddBreakpointLineAddButton;
    JButton fAddBreakpointLineCancelButton;

    JPanel fXmlFilePanel;
    HashMap<String, STAXMonitorThreadTreeNode> fThreadNodeMap =
        new HashMap<String, STAXMonitorThreadTreeNode>();
    HashMap<String, JTextPane> fXMLFileTextPaneMap =
        new HashMap<String, JTextPane>();
    HashMap fThreadVariableTreeTableMap = new HashMap();
    JTree fThreadTree;
    JButton fRefreshStatus = new JButton("Refresh Status");
    JButton fResumeButton = new JButton("Resume");
    JButton fStepIntoButton = new JButton("Step Into");
    JButton fStepOverButton = new JButton("Step Over");
    JButton fStopButton = new JButton("Stop");
    JTabbedPane fXmlFileTabbedPane;
    JTabbedPane fVariablesPane;
    JTabbedPane fThreadInfoPane;
    JTextArea fPythonCode = new JTextArea(3, 50);
    JButton fExecutePythonButton = new JButton("Execute");
    STAXMonitorTreeTable fThreadVariableTreeTable;
    JTextPane fSourceFilePane;
    JScrollPane fXmlFileScrollPane;
    int fBeginXmlSelection = 0;
    HashMap<String, String> fXmlFileCache = new HashMap<String, String>();

    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName,
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;

        fPythonCode.setFont(new Font("Monospaced", Font.PLAIN, 12));
        fExecutePythonButton.addActionListener(this);

        try
        {
            fHandle = STAXMonitorUtil.getNewSTAFHandle(
                "STAX/JobMonitor/Extension/Debug");
        }
        catch (STAFException ex)
        {
        }

        fTitle = "Debug";

        STAXMonitorThreadTreeNode rootNode = new STAXMonitorThreadTreeNode("1");
        fThreadNodeMap.put("1", rootNode);
        fThreadTree = new JTree(new DefaultTreeModel(rootNode));
        fThreadTree.setCellRenderer(new STAXMonitorThreadTreeCellRenderer());
        fThreadTree.setShowsRootHandles(true);
        fThreadTree.addMouseListener(this);
        fThreadTree.addTreeSelectionListener(this);

        fPanel = new JPanel();
        fPanel.setLayout(new BorderLayout());

        JPanel threadsPanel = new JPanel();
        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        buttonPanel.add(fRefreshStatus);
        fRefreshStatus.addActionListener(this);
        buttonPanel.add(Box.createHorizontalStrut(20));
        buttonPanel.add(fResumeButton);
        fResumeButton.addActionListener(this);
        buttonPanel.add(Box.createHorizontalStrut(10));
        buttonPanel.add(fStepIntoButton);
        fStepIntoButton.addActionListener(this);
        buttonPanel.add(Box.createHorizontalStrut(10));
        buttonPanel.add(fStepOverButton);
        fStepOverButton.addActionListener(this);
        buttonPanel.add(Box.createHorizontalStrut(10));
        buttonPanel.add(fStopButton);
        fStopButton.addActionListener(this);
        fResumeButton.setEnabled(false);
        fStepIntoButton.setEnabled(false);
        fStepOverButton.setEnabled(false);
        fStopButton.setEnabled(false);
        threadsPanel.setLayout(new BorderLayout());
        threadsPanel.add(BorderLayout.NORTH, buttonPanel);
        threadsPanel.add(BorderLayout.CENTER, fThreadTree);

        JTabbedPane threadsPane = new JTabbedPane();
        threadsPane.add("Threads", new JScrollPane(threadsPanel));

        fBreakpointsTableColumnNames = new Vector<String>();
        fBreakpointsTableColumnNames.add("ID   ");
        fBreakpointsTableColumnNames.add("Function");
        fBreakpointsTableColumnNames.add("Line #");
        fBreakpointsTableColumnNames.add("XML File");
        fBreakpointsTableColumnNames.add("Machine");

        JPanel breakpointsPanel = new JPanel();
        breakpointsPanel.setLayout(new BorderLayout());

        fBreakpointsTable = new
            JTable(new STAXMonitorTableModel(
            fBreakpointsTableColumnNames, 0));

        fBreakpointsTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        fBreakpointsTable.setSelectionMode(
            ListSelectionModel.SINGLE_SELECTION);
        fBreakpointsTable.setRowSelectionAllowed(true);
        fBreakpointsTable.addMouseListener(this);

        for (int i = 0; i < fBreakpointsTableColumnNames.size(); i++)
        {
            fBreakpointsTable.getColumnModel().getColumn(i).
                setCellRenderer(new STAXMonitorBreakpointTableCellRenderer(
                new Font("Dialog", Font.PLAIN, 12), Color.black, false));

            fBreakpointsTable.getColumnModel().getColumn(i).
                setHeaderRenderer(new STAXMonitorBreakpointTableCellRenderer(
                new Font("Dialog", Font.BOLD, 12), Color.black, true));
        }

        fBreakpointsTable.updateUI();
        STAXMonitorUtil.updateRowHeights(fBreakpointsTable, 0);
        STAXMonitorUtil.sizeColumnsToFitText(fBreakpointsTable);

        JPanel breakpointLinesPanel = new JPanel();
        breakpointLinesPanel.setLayout(new
            FlowLayout(FlowLayout.LEFT, 0, 0));
        breakpointLinesPanel.add(Box.createHorizontalStrut(10));

        JPanel breakpointTriggersButtonPanel = new JPanel();
        breakpointTriggersButtonPanel.setLayout(new
            BoxLayout(breakpointTriggersButtonPanel, BoxLayout.Y_AXIS));
        fBreakpointLineAddButton = new JButton("Add Breakpoint Line...");
        fBreakpointLineAddButton.addActionListener(this);
        fAddBreakpointFunctionButton =
            new JButton("Add Breakpoint Function...");
        fAddBreakpointFunctionButton.addActionListener(this);
        fBreakpointRemoveButton = new JButton("Remove");
        fBreakpointRemoveButton.addActionListener(this);
        fBreakpointRemoveAllButton = new JButton("Remove All");
        fBreakpointRemoveAllButton.addActionListener(this);
        breakpointTriggersButtonPanel.add(fBreakpointLineAddButton);
        breakpointTriggersButtonPanel.add(Box.createVerticalStrut(5));
        breakpointTriggersButtonPanel.add(fAddBreakpointFunctionButton);
        breakpointTriggersButtonPanel.add(Box.createVerticalStrut(5));
        breakpointTriggersButtonPanel.add(fBreakpointRemoveButton);
        breakpointTriggersButtonPanel.add(Box.createVerticalStrut(5));
        breakpointTriggersButtonPanel.add(fBreakpointRemoveAllButton);

        breakpointsPanel.add(BorderLayout.CENTER,
            new JScrollPane(fBreakpointsTable));
        breakpointsPanel.add(BorderLayout.EAST, breakpointTriggersButtonPanel);

        fAddBreakpointLineDialog = new JDialog(monitorFrame,
            "Add Line Breakpoint", true);
        fAddBreakpointLineDialog.setSize(new Dimension(400, 165));

        JPanel addBreakpointLinePanel = new JPanel();
        addBreakpointLinePanel.setLayout(new BorderLayout());

        JPanel breakpointLineOptionsPanel = new JPanel();
        breakpointLineOptionsPanel.setBorder(
            new TitledBorder("Breakpoint Line options"));
        breakpointLineOptionsPanel.setLayout(
            new BoxLayout(breakpointLineOptionsPanel, BoxLayout.Y_AXIS));

        JPanel addBreakpointLineNumberPanel = new JPanel();
        addBreakpointLineNumberPanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel breakpointLineNumberName = new JLabel(" Line Number:    ");
        fAddBreakpointLineNumberTextField = new JTextField(15)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fAddBreakpointLineNumberTextField.setToolTipText(
            STAXMonitorUtil.wrapText(
            STAXMonitor.BREAKPOINT_LINE_NUMBER_TOOLTIP, 80));

        fAddBreakpointLineNumberTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fAddBreakpointLineNumberTextField.setBackground(Color.white);
        fAddBreakpointLineNumberTextField.addActionListener(this);
        addBreakpointLineNumberPanel.add(breakpointLineNumberName);
        addBreakpointLineNumberPanel.add(fAddBreakpointLineNumberTextField);

        JPanel addBreakpointLineFilePanel = new JPanel();
        addBreakpointLineFilePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel breakpointLineFileName = new JLabel(" XML File:            ");
        fAddBreakpointLineFileTextField = new JTextField(25)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fAddBreakpointLineFileTextField.setToolTipText(
            STAXMonitorUtil.wrapText(
            STAXMonitor.BREAKPOINT_LINE_FILE_TOOLTIP, 80));

        fAddBreakpointLineFileTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fAddBreakpointLineFileTextField.setBackground(Color.white);
        fAddBreakpointLineFileTextField.addActionListener(this);
        addBreakpointLineFilePanel.add(breakpointLineFileName);
        addBreakpointLineFilePanel.add(fAddBreakpointLineFileTextField);

        JPanel addBreakpointLineMachinePanel = new JPanel();
        addBreakpointLineMachinePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel breakpointLineMachineName = new JLabel(" Machine:            ");
        fAddBreakpointLineMachineTextField = new JTextField(25)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fAddBreakpointLineMachineTextField.setToolTipText(
            STAXMonitorUtil.wrapText(
            STAXMonitor.BREAKPOINT_LINE_MACHINE_TOOLTIP, 80));

        fAddBreakpointLineMachineTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fAddBreakpointLineMachineTextField.setBackground(Color.white);
        fAddBreakpointLineMachineTextField.addActionListener(this);
        addBreakpointLineMachinePanel.add(breakpointLineMachineName);
        addBreakpointLineMachinePanel.add(fAddBreakpointLineMachineTextField);

        fAddBreakpointLineAddButton = new JButton("Add");
        fAddBreakpointLineCancelButton = new JButton("Cancel");

        JPanel addBreakpointLineButtonPanel = new JPanel();
        addBreakpointLineButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addBreakpointLineButtonPanel.add(fAddBreakpointLineAddButton);
        addBreakpointLineButtonPanel.add(Box.createHorizontalStrut(20));
        addBreakpointLineButtonPanel.add(
            fAddBreakpointLineCancelButton);

        breakpointLineOptionsPanel.add(addBreakpointLineNumberPanel);
        breakpointLineOptionsPanel.add(addBreakpointLineFilePanel);
        breakpointLineOptionsPanel.add(addBreakpointLineMachinePanel);

        addBreakpointLinePanel.add(BorderLayout.CENTER,
            breakpointLineOptionsPanel);
        addBreakpointLinePanel.add(BorderLayout.SOUTH,
            addBreakpointLineButtonPanel);

        fAddBreakpointLineAddButton.addActionListener(this);
        fAddBreakpointLineCancelButton.addActionListener(this);

        fAddBreakpointLineDialog.getContentPane().add(
            addBreakpointLinePanel);

        JPanel breakpointFunctionsPanel = new JPanel();
        breakpointFunctionsPanel.setLayout(new
            FlowLayout(FlowLayout.LEFT, 0, 0));
        breakpointFunctionsPanel.add(Box.createHorizontalStrut(10));

        fAddBreakpointFunctionDialog = new JDialog(monitorFrame,
            "Add Function Breakpoint", true);
        fAddBreakpointFunctionDialog.setSize(new Dimension(400, 125));
        JPanel addBreakpointFunctionPanel = new JPanel();
        addBreakpointFunctionPanel.setLayout(new BorderLayout());
        fAddBreakpointFunctionTextField = new JTextField(15);
        fAddBreakpointFunctionTextField.setBorder(
            new TitledBorder("Enter function name here"));
        addBreakpointFunctionPanel.add(BorderLayout.CENTER,
            new JScrollPane(fAddBreakpointFunctionTextField));

        fAddBreakpointFunctionAddButton = new JButton("Add");
        fAddBreakpointFunctionCancelButton = new JButton("Cancel");

        JPanel addBreakpointFunctionButtonPanel = new JPanel();
        addBreakpointFunctionButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addBreakpointFunctionButtonPanel.add(
            fAddBreakpointFunctionAddButton);
        addBreakpointFunctionButtonPanel.add(Box.createHorizontalStrut(20));
        addBreakpointFunctionButtonPanel.add(
            fAddBreakpointFunctionCancelButton);

        addBreakpointFunctionPanel.add(BorderLayout.SOUTH,
            addBreakpointFunctionButtonPanel);

        fAddBreakpointFunctionAddButton.addActionListener(this);
        fAddBreakpointFunctionCancelButton.addActionListener(this);

        fAddBreakpointFunctionDialog.getContentPane().add(
            addBreakpointFunctionPanel);

        Vector<Vector<String>> breakpointLinesVector =
            new Vector<Vector<String>>();
        DefaultTableModel breakpointLinesModel =
            (DefaultTableModel)(fBreakpointsTable.getModel());

        STAFResult result = fHandle.submit2(
            staxMachineName, staxServiceName,
            "LIST JOB " + jobNumber + " BREAKPOINTS");

        if (result.rc == STAFResult.Ok)
        {
            STAFMarshallingContext mc = STAFMarshallingContext.unmarshall(
                result.result);
            List entryList = (List)mc.getRootObject();
            Iterator iter = entryList.iterator();

            while (iter.hasNext())
            {
                Map triggerMap = (Map)iter.next();

                String triggerID = (String)triggerMap.get("ID");
                String function = (String)triggerMap.get("function");
                String line = (String)triggerMap.get("line");
                String file = (String)triggerMap.get("file");
                String machine = (String)triggerMap.get("machine");

                Vector<String> newRow = new Vector<String>();
                newRow.add(triggerID);
                newRow.add(function);
                newRow.add(line);
                newRow.add(file);
                newRow.add(machine);

                breakpointLinesVector.add(newRow);
            }
        }

        ((DefaultTableModel)fBreakpointsTable.getModel()).setDataVector(
            breakpointLinesVector, fBreakpointsTableColumnNames);

        for (int i = 0; i < fBreakpointsTableColumnNames.size(); i++)
        {
            fBreakpointsTable.getColumnModel().getColumn(i).
                setCellRenderer(new STAXMonitorBreakpointTableCellRenderer(
                new Font("Dialog", Font.PLAIN, 12), Color.black, false));

            fBreakpointsTable.getColumnModel().getColumn(i).
                setHeaderRenderer(new STAXMonitorBreakpointTableCellRenderer(
                new Font("Dialog", Font.BOLD, 12), Color.black, true));
        }

        fBreakpointsTable.updateUI();
        STAXMonitorUtil.updateRowHeights(fBreakpointsTable, 0);
        STAXMonitorUtil.sizeColumnsToFitText(fBreakpointsTable);

        JTabbedPane breakpointsPane = new JTabbedPane();
        breakpointsPane.add("Breakpoints",
                            new JScrollPane(breakpointsPanel));

        JSplitPane upperHorizPane = new JSplitPane(
            JSplitPane.HORIZONTAL_SPLIT, threadsPane, breakpointsPane);
        upperHorizPane.setOneTouchExpandable(true);
        upperHorizPane.setResizeWeight(.50D);

        fVariablesPane = new JTabbedPane();
        fVariablesPane.add("Variables", new JScrollPane(new JPanel()));

        fThreadInfoPane = new JTabbedPane();
        fThreadInfoPane.add("Thread Information",
                            new JScrollPane(new JPanel()));

        fXmlFilePanel = new JPanel();
        fXmlFilePanel.setLayout(new BorderLayout());
        fXmlFileTabbedPane = new JTabbedPane();
        fXmlFileTabbedPane.add("XML File", new JScrollPane(fXmlFilePanel));

        JSplitPane varsThreadInfoPane =
            new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
            fVariablesPane, fThreadInfoPane);
        varsThreadInfoPane.setOneTouchExpandable(true);
        varsThreadInfoPane.setResizeWeight(.50D);

        JSplitPane lowerHorizPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
            varsThreadInfoPane, fXmlFileTabbedPane);

        lowerHorizPane.setOneTouchExpandable(true);
        lowerHorizPane.setResizeWeight(.50D);

        JSplitPane vertPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
            upperHorizPane, lowerHorizPane);
        vertPane.setOneTouchExpandable(true);
        vertPane.setResizeWeight(.50D);

        fPanel.add(BorderLayout.CENTER, vertPane);

        fTreeTablePopupMenu.add(fTreeTableExpandAllMenuItem);
        fTreeTableExpandAllMenuItem.addActionListener(this);
        fTreeTablePopupMenu.add(fTreeTableCollapseAllMenuItem);
        fTreeTableCollapseAllMenuItem.addActionListener(this);

        fXmlSourcePopupMenu.add(fXmlSourceScrollToCurrentLineMenuItem);
        fXmlSourceScrollToCurrentLineMenuItem.addActionListener(this);

        // If monitoring an existing job, update its Thread Tree Panel
        if (!newJob)
        {
            refreshThreadTree();
        }

        return fPanel;
    }

    public String getNotificationEventTypes()
    {
        return "thread breakpoint";
    }

    public void term()
    {
        STAXMonitorUtil.freeHandle(fHandle.getHandle());
    }

    public String getTitle()
    {
        return fTitle;
    }

    public int getExtensionType()
    {
        return STAXMonitorFrame.EXTENSION_ACTIVE;
    }

    public JComponent getComponent()
    {
        return fPanel;
    }

    public void handleEvent(Map map)
    {
        try
        {
            String type = (String)map.get("type");

            if (type.equals("thread"))
            {
                handleThreadEvent(map);
            }
            else if (type.equals("breakpoint"))
            {
                handleBreakpointEvent(map);
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * Refreshes the thread tree by creating a new fThreadNodeMap and
     * fThreadTree based on the current running threads for a STAX job
     * (obtained via a STAX LIST THREADS LONG request).
     */
    public void refreshThreadTree()
    {
        Map threadMap = null;  // Default for the specified thread ID

        // Submit a STAX LIST THREADS LONG request to get a list of the
        // threads currently running in the job.

        List threadList = new ArrayList();  // Default to empty thread list

        STAFResult result = fHandle.submit2(
            fStaxMachine, fStaxServiceName,
            "LIST JOB " + fJobNumber + " THREADS LONG");

        if (result.rc == STAFResult.Ok)
        {
            STAFMarshallingContext mc = STAFMarshallingContext.unmarshall(
                result.result);

            threadList = (List)mc.getRootObject();
        }

        if (sDebug)
        {
            System.out.println("refreshThreadTree: NumThreads=" +
                               threadList.size());
        }

        synchronized (fThreadNodeMap)
        {
            // Save the selected thread ID of the node in fThreadTree, if any,
            // so can select it after the tree is re-populated

            String selectedThreadID = null;
            TreePath selectedPath = fThreadTree.getSelectionPath();
            STAXMonitorThreadTreeNode node = null;

            if (selectedPath != null)
            {
                node = (STAXMonitorThreadTreeNode)
                    (selectedPath.getLastPathComponent());
                selectedThreadID = node.toString();
            }

            // Reset fThreadNodeMap and fThreadTree to be empty

            DefaultTreeModel threadTreeModel =
                (DefaultTreeModel)fThreadTree.getModel();
            if (threadTreeModel != null)
                threadTreeModel.setRoot(null);

            fThreadNodeMap = new HashMap<String, STAXMonitorThreadTreeNode>();

            // Iterate through the list of running threads and populate the
            // fThreadNodeMap and fThreadTree with these threads and assign
            // threadMap if find the specfied thread ID

            Iterator iter = threadList.iterator();

            while (iter.hasNext())
            {
                Map entryMap = (Map)iter.next();

                String id = (String)entryMap.get("threadID");
                String parentID = (String)entryMap.get("parentTID");
                String hierarchyString = (String)entryMap.get(
                    "parentHierarchy");
                String startTimestamp = (String)entryMap.get("startTimestamp");
                List callStack = (List)entryMap.get("callStack");
                List conditionStack = (List)entryMap.get("conditionStack");

                // If this thread has any parents, make sure its entire
                // parent thread hierarchy is in the JTree

                String[] parentHierarchy = new String[0];

                if (hierarchyString != null)
                    parentHierarchy = hierarchyString.split("\\.");

                STAXMonitorThreadTreeNode parentNode = null;

                if (parentHierarchy.length > 1)
                {
                    String priorParentTID = "1";

                    for (int i = 0; i < parentHierarchy.length; i++)
                    {
                        String theID = parentHierarchy[i];

                        if (!(theID.equals("1")) &&
                            !(fThreadNodeMap.containsKey(theID)))
                        {
                            node = new STAXMonitorThreadTreeNode(theID);

                            fThreadNodeMap.put(theID, node);

                            parentNode = fThreadNodeMap.get(priorParentTID);

                            ((DefaultTreeModel)fThreadTree.getModel()).
                                insertNodeInto(node, parentNode,
                                parentNode.getChildCount());
                        }

                        priorParentTID = theID;
                    }
                }

                // Create the node and add it to the fThreadNodeMap

                node = new STAXMonitorThreadTreeNode(
                    id, parentID, hierarchyString, startTimestamp,
                    callStack, conditionStack);

                // Assign the node's isRunning field based on if the thread
                // is currently at a breakpoint or not by checking if the
                // condition stack for a thread contains a Breakpoint
                // HoldThread condition

                Iterator csIter = conditionStack.iterator();

                String conditionStackString = "";

                while (csIter.hasNext())
                {
                    if (!(conditionStackString.equals("")))
                    {
                        conditionStackString = conditionStackString +
                            System.getProperty("line.separator");
                    }

                    conditionStackString = conditionStackString +
                        csIter.next().toString();
                }

                if (conditionStackString.indexOf(
                    "HoldThread: Source=Breakpoint") > -1)
                {
                    node.setRunning(false);
                }

                // Assign the node's detail text from the last entry in the
                // thread's call stack

                String lastCallStackEntry = "";

                if (callStack.size() > 0)
                {
                    lastCallStackEntry = (String)callStack.get(
                        callStack.size() - 1);
                }

                node.setDetailText(lastCallStackEntry);

                // Add the node to fThreadNodeMap

                fThreadNodeMap.put(id, node);

                // Add the node to the Thread JTree

                parentNode = fThreadNodeMap.get(parentID);

                if (id.equals("1"))
                {
                    ((DefaultTreeModel)fThreadTree.getModel()).setRoot(node);
                }
                else
                {
                    ((DefaultTreeModel)fThreadTree.getModel()).insertNodeInto(
                        node, parentNode, parentNode.getChildCount());
                }
            }

            fThreadTree.updateUI();

            for (int i = 0; i < fThreadTree.getRowCount(); i++)
            {
                fThreadTree.expandRow(i);
            }

            if (selectedThreadID != null)
            {
                // Set the selected node (if one was selected) in fThreadTree

                node = fThreadNodeMap.get(selectedThreadID);

                if (node != null)
                {
                    TreePath treePath = new TreePath(
                       ((DefaultTreeModel)fThreadTree.getModel()).
                           getPathToRoot(node));
                    fThreadTree.setSelectionPath(treePath);
                }
            }
        }
    }

    /**
     * Handles start and stop thread events generated by the STAX service
     * that indicate when a thread in the job starts and stops.
     *
     * @param A map containing information about thread start or stop event
     */
    public void handleThreadEvent(Map map)
    {
        String status = (String)map.get("status");
        String id = (String)map.get("id");

        if (status.equals("start"))
        {
            // Thread start event received

            if (sDebugDetail)
                System.out.println("TStart: id=" + id);

            String parent = (String)map.get("parent");

            synchronized (fThreadNodeMap)
            {
                STAXMonitorThreadTreeNode node = null;

                if (fThreadNodeMap.containsKey(id))
                {
                    if (sDebugDetail)
                    {
                        System.out.println(
                            "TStart: id=" + id + " - Already in nodeMap");
                    }

                    node = fThreadNodeMap.get(id);

                    if (node.isStoppedState())
                    {
                        // Thread stop event was received before the
                        // thread start event.  Remove from thread node map.

                        if (sDebugDetail)
                        {
                            System.out.println(
                                "TStart: id=" + id + " - Remove thread " +
                                "since already got stop event");
                        }

                        fThreadNodeMap.remove(id);
                    }
                    else
                    {
                        String detailText = (String)map.get("detailText");

                        if (detailText != null && detailText.length() != 0)
                        {
                            node.setDetailText(detailText);
                        }

                        fThreadTree.updateUI();
                    }

                    return;
                }

                // Add the thread to the thread node map and the JTree

                // First, need to make sure the entire parent thread
                // hierarchy is in the JTree before can add the thread node

                String hierarchyString = (String)map.get("parentHierarchy");
                String[] parentHierarchy = new String[0];

                if (hierarchyString != null)
                    parentHierarchy = hierarchyString.split("\\.");

                String priorParentTID = "1";
                STAXMonitorThreadTreeNode parentNode = null;

                for (int i = 0; i < parentHierarchy.length; i++)
                {
                    String threadID = parentHierarchy[i];

                    if (!(threadID.equals("1")) &&
                        !(fThreadNodeMap.containsKey(threadID)))
                    {
                        node = new STAXMonitorThreadTreeNode(threadID);

                        parentNode = fThreadNodeMap.get(priorParentTID);

                        if (parentNode != null)
                        {
                            if (sDebugDetail)
                            {
                                System.out.println(
                                    "TStart: id=" + id +
                                    " - Added parent TID=" + threadID + " (" +
                                    parentHierarchy + ")");
                            }

                            fThreadNodeMap.put(threadID, node);
                            ((DefaultTreeModel)fThreadTree.getModel()).
                                insertNodeInto(node, parentNode,
                                parentNode.getChildCount());
                        }
                        else if (sDebug)
                        {
                            System.out.println(
                                "TStart: id=" + id +
                                " - ERROR? Prior parent TID=" +
                                priorParentTID + " does not exist (" +
                                parentHierarchy + ")");
                        }

                    }

                    priorParentTID = threadID;
                }

                // Add the thread to the thread node map and to the JTree

                node = new STAXMonitorThreadTreeNode(id);

                fThreadNodeMap.put(id, node);

                parentNode = fThreadNodeMap.get(parent);

                if (parentNode != null)
                {
                    ((DefaultTreeModel)fThreadTree.getModel()).insertNodeInto(
                        node, parentNode, parentNode.getChildCount());
                }
                else if (sDebug)
                {
                    System.out.println(
                        "TStart: id=" + id +
                        " - ERROR? Can't add because parent " +
                        parent + " does not exist");
                }

                String detailText = (String)map.get("detailText");

                if (detailText != null)
                {
                    node.setDetailText(detailText);
                }

                fThreadTree.updateUI();
            }

            for (int i = 0; i < fThreadTree.getRowCount(); i++)
            {
                fThreadTree.expandRow(i);
            }
        }
        else if (status.equals("stop"))
        {
            // Thread stop event received

            if (sDebug)
                System.out.println("TStop: id=" + id);

            synchronized (fThreadNodeMap)
            {
                DefaultTreeModel threadTreeModel =
                    (DefaultTreeModel)fThreadTree.getModel();

                if (threadTreeModel != null)
                {
                    MutableTreeNode node =
                        (MutableTreeNode)fThreadNodeMap.get(id);

                    if (node != null)
                    {
                        // Thread is in fThreadNodeMap

                        if (!(id.equals("1")))
                        {
                            try
                            {
                                threadTreeModel.removeNodeFromParent(node);
                            }
                            catch (IllegalArgumentException e)
                            {
                                // Ignore if the node's parent doesn't exist
                                if (sDebug)
                                {
                                    System.out.println(
                                        "TStop: id=" + id +
                                        " - Error? IllegalArgumentException " +
                                        "removing node from JTree");
                                }
                            }
                        }
                        else
                        {
                            if (sDebug)
                            {
                                System.out.println(
                                    "TStop: id=" + id + " - Clear JTree");
                            }

                            threadTreeModel.setRoot(null);
                        }
                    }
                    else
                    {
                        // Thread is not in fThreadNodeMap because received
                        // the thread stop event before the thread start
                        // event.

                        // Add this thread to fThreadNodeMap with a stopped
                        // state so that when the thread start event for this
                        // thread is recevied, if its in a stopped state it
                        // knows to remove the thread instead of adding it.

                        if (sDebug)
                        {
                            System.out.println("TStop: id=" + id +
                                               " - Doesn't exist so add it");
                        }

                        STAXMonitorThreadTreeNode newNode =
                            new STAXMonitorThreadTreeNode(id);

                        newNode.setStoppedState(true);

                        fThreadNodeMap.put(id, newNode);

                        // Don't add this thread node to the JTree
                    }
                }

                fThreadNodeMap.remove(id);
                fThreadTree.updateUI();
            }
        }
    }

    /**
     * Handles start and stop breakpoint events generated by the STAX service
     * that indicate when a breakpoint in the job starts and stops.
     *
     * @param A map containing information about breakpoint start/stop event
     */
    public void handleBreakpointEvent(Map map)
    {
        String id = (String)map.get("id");
        String status = (String)map.get("status");

        if (status.equals("start"))
        {
            if (sDebug)
                System.out.println("BStart: id=" + id);

            // Refresh the Thread Tree with current status for all threads

            refreshThreadTree();

            // Check if the thread is no longer at a breakpoint.

            STAXMonitorThreadTreeNode node = fThreadNodeMap.get(id);

            if (node == null || node.isRunning())
            {
                // Thread no longer exists or is no longer at a breakpoint
                // so ignore the start notification

                if (sDebugDetail)
                {
                    System.out.println(
                        "BStart: id=" + id + " - Thread does not exist or " +
                        "is no longer at a breakpoing");
                }

                return;
            }

            // Update the XML File Panel to the indicated line in the xml file

            String lineNum = (String)map.get("lineNumber");
            String xmlFile = (String)map.get("xmlFile");
            String xmlMachine = (String)map.get("xmlMachine");
            String info = (String)map.get("info");

            if (!lineNum.equals("Unknown"))
            {
                // lineNum, xmlFile, xmlMachine can be "Unknown" if the
                // thread is initializing extensions when the job starts

                JTextPane xmlSourceFilePane =
                    getXmlSourceFilePane(lineNum, xmlFile, xmlMachine);

                fXMLFileTextPaneMap.put(id, xmlSourceFilePane);
            }

            // Update the Variables Panel for this breakpoint's thread

            String threadID = "Thread " + (String)map.get("id");

            DefaultMutableTreeNode rootNode =
                new DefaultMutableTreeNode(
                new BreakpointVariable(threadID, "", ""));

            // Set the selection path to the thread that's at this breakpoint

            TreePath selectionPath = fThreadTree.getSelectionPath();

            if (selectionPath != null)
            {
                if (selectionPath.getLastPathComponent() == node)
                {
                    fThreadTree.removeSelectionPath(selectionPath);
                    fThreadTree.setSelectionPath(selectionPath);
                    threadSelectionChanged();
                }
            }
        }
        else if (status.equals("stop"))
        {
            if (sDebug)
                System.out.println("BStop: id=" + id);

            // Refresh the Thread Tree with current status for all threads

            refreshThreadTree();

            // Check if the thread is still at a breakpoint.
            // If so, ignore the stop notification.
            // Otherwise, set the thread's isRunning flag to true.

            STAXMonitorThreadTreeNode node = fThreadNodeMap.get(id);

            if (node != null && !node.isRunning())
            {
                if (sDebugDetail)
                {
                    System.out.println(
                        "BStop: id=" + id + " - Still at breakpoint");
                }

                return;  // Still at a breakpoint so ignore stop event
            }

            // Update the XML File and Variable panels

            fXMLFileTextPaneMap.remove(id);
            fThreadVariableTreeTableMap.remove(id);

            // Update the selection path in the JTree

            TreePath selectionPath = fThreadTree.getSelectionPath();

            if (selectionPath != null)
            {
                if (selectionPath.getLastPathComponent() == node)
                {
                    fThreadTree.removeSelectionPath(selectionPath);
                    fThreadTree.setSelectionPath(selectionPath);
                }
            }
        }
        else if (status.equals("add"))
        {
            String function = (String)map.get("function");
            String line = (String)map.get("line");
            String file = (String)map.get("file");
            String machine = (String)map.get("machine");

            Vector<String> newRow = new Vector<String>();
            newRow.add(id);
            newRow.add(function);
            newRow.add(line);
            newRow.add(file);
            newRow.add(machine);

            DefaultTableModel model =
                (DefaultTableModel)fBreakpointsTable.getModel();

            synchronized (model)
            {
                model.addRow(newRow);
            }

            STAXMonitorUtil.updateRowHeights(fBreakpointsTable, 0);
            STAXMonitorUtil.sizeColumnsToFitText(fBreakpointsTable);
        }
        else if (status.equals("remove"))
        {
             DefaultTableModel model =
                (DefaultTableModel)fBreakpointsTable.getModel();

            synchronized (model)
            {
                int rowCount = model.getRowCount();

                for (int i = 0; i < rowCount; i++)
                {
                    if (model.getValueAt(i, 0).equals(id))
                    {
                        model.removeRow(i);
                        break;
                    }
                }
            }
        }
    }

    public void addNode(DefaultMutableTreeNode parentNode,
                        Object object,
                        String name,
                        String type)
    {
        if (object instanceof List)
        {
            Iterator iter = ((List)object).iterator();

            while (iter.hasNext())
            {
                Object obj = iter.next();

                if (obj instanceof Map)
                {
                    Map objMap = (Map)obj;
                    String varName = (String)objMap.get("name");
                    String varType = (String)objMap.get("type");
                    Object varValue = objMap.get("value");

                    if ((varType.equals("org.python.core.PyDictionary")) ||
                        (varType.equals("org.python.core.PyList")) ||
                        (varType.equals("org.python.core.PyTuple")))
                    {
                        DefaultMutableTreeNode newNode =
                            new DefaultMutableTreeNode(
                            new BreakpointVariable(varName,
                                                   "",
                                                   varType));
                        addNode(newNode, varValue, varName, varType);
                        parentNode.add(newNode);
                    }
                    else
                    {
                        addNode(parentNode, varValue, varName, varType);
                    }
                }
            }
        }
        else
        {
            parentNode.add(new DefaultMutableTreeNode(
                new BreakpointVariable(name,
                                       object.toString(),
                                       type)));
        }
    }

    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == fTreeTableExpandAllMenuItem)
        {
            for (int i = 0; i < fThreadVariableTreeTable.getRowCount(); i++)
            {
                fThreadVariableTreeTable.getTree().expandRow(i);
            }
        }
        else if (e.getSource() == fTreeTableCollapseAllMenuItem)
        {
            for (int i = fThreadVariableTreeTable.getRowCount() - 1; i >= 1; i--)
            {
                fThreadVariableTreeTable.getTree().collapseRow(i);
            }
        }
        else if (e.getSource() == fXmlSourceScrollToCurrentLineMenuItem)
        {
            fSourceFilePane.setCaretPosition(fBeginXmlSelection);

            Rectangle r = null;
            Rectangle scrollRect = null;

            try
            {
                r = fSourceFilePane.modelToView(
                    fSourceFilePane.getCaretPosition());
                scrollRect = new Rectangle();
                scrollRect.x = Math.min(r.x, r.x);
                scrollRect.y =  Math.min(r.y - 65, r.y - 65);
                scrollRect.width = Math.max(r.x + r.width, r.x + r.width) -
                                   scrollRect.x;
                scrollRect.height = Math.max(r.y + r.height, r.y + r.height) -
                                    scrollRect.y;
                if (fXmlFileScrollPane != null) {
                    int h =
                        fXmlFileScrollPane.getViewport().getViewRect().height;
                    scrollRect.height = h;
                }
            }
            catch (BadLocationException ex)
            {
                // XXX
            }

            fSourceFilePane.scrollRectToVisible(scrollRect);
        }
        else if (e.getSource() == fRefreshStatus)
        {
            // Refresh the Thread Tree with current status for all threads

            refreshThreadTree();
            threadSelectionChanged();
        }
        else if (e.getSource() == fResumeButton)
        {
            STAXMonitorThreadTreeNode selectedNode =
                (STAXMonitorThreadTreeNode)fThreadTree.
                getSelectionPath().getLastPathComponent();
            String threadID = selectedNode.toString();

            String request = "RESUME JOB " + fJobNumber + " THREAD " +
                         STAFUtil.wrapData(threadID);

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    fMonitorFrame,
                    "An error was encountered while attempting to RESUME " +
                    " rc=" + result.rc + " result=" + result.result);
            }

            return;
        }
        else if (e.getSource() == fStepIntoButton)
        {
            STAXMonitorThreadTreeNode selectedNode =
                (STAXMonitorThreadTreeNode)fThreadTree.
                getSelectionPath().getLastPathComponent();
            String threadID = selectedNode.toString();

            String request = "STEP JOB " + fJobNumber + " THREAD " +
                         STAFUtil.wrapData(threadID);

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    fMonitorFrame,
                    "An error was encountered while attempting to STEP " +
                    " rc=" + result.rc + " result=" + result.result);
            }

            return;
        }
        else if (e.getSource() == fStepOverButton)
        {
            STAXMonitorThreadTreeNode selectedNode =
                (STAXMonitorThreadTreeNode)fThreadTree.
                getSelectionPath().getLastPathComponent();
            String threadID = selectedNode.toString();

            String request = "STEP OVER JOB " + fJobNumber + " THREAD " +
                         STAFUtil.wrapData(threadID);

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    fMonitorFrame,
                    "An error was encountered while attempting to STEP " +
                    " rc=" + result.rc + " result=" + result.result);
            }

            return;
        }
        else if (e.getSource() == fStopButton)
        {
            STAXMonitorThreadTreeNode selectedNode =
                (STAXMonitorThreadTreeNode)fThreadTree.
                getSelectionPath().getLastPathComponent();
            String threadID = selectedNode.toString();

            String request = "STOP JOB " + fJobNumber + " THREAD " + threadID;

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    fMonitorFrame,
                    "An error was encountered while attempting to STOP " +
                    " rc=" + result.rc + " result=" + result.result);
            }

            return;
        }
        else if (e.getSource() == fAddBreakpointFunctionButton)
        {
            fAddBreakpointFunctionTextField.setText("");
            fAddBreakpointFunctionTextField.requestFocus();
            fAddBreakpointFunctionTextField.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointFunctionDialog.setLocationRelativeTo(fPanel);
            fAddBreakpointFunctionDialog.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointFunctionDialog.setVisible(true);
        }
        else if (e.getSource() == fAddBreakpointFunctionAddButton)
        {
            if (fAddBreakpointFunctionTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fPanel,
                                      "You must enter a function name",
                                      "Error adding breakpoint function",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            int confirmation = JOptionPane.showConfirmDialog(fPanel,
                "Are you certain that you want to add\n" +
                "this function breakpoint trigger?",
                "Confirm Breakpoint Trigger",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            String function = fAddBreakpointFunctionTextField.getText();

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName,
                "ADD JOB " + fJobNumber + " BREAKPOINT FUNCTION " + function);

            if (result.rc != STAFResult.Ok)
            {
                JOptionPane.showMessageDialog(fPanel,
                    "RC = " + result.rc + " Result=" + result.result,
                    "Submitted ADD BREAKPOINT",
                    JOptionPane.INFORMATION_MESSAGE);
            }
            else
            {
                fAddBreakpointFunctionDialog.setVisible(false);
            }
        }
        else if (e.getSource() == fAddBreakpointFunctionCancelButton)
        {
            fAddBreakpointFunctionDialog.setVisible(false);
        }
        else if (e.getSource() == fBreakpointLineAddButton)
        {
            fAddBreakpointLineNumberTextField.setText("");
            fAddBreakpointLineFileTextField.setText("");
            fAddBreakpointLineMachineTextField.setText("");
            fAddBreakpointLineNumberTextField.requestFocus();
            fAddBreakpointLineNumberTextField.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointLineDialog.setLocationRelativeTo(fPanel);
            fAddBreakpointLineDialog.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointLineDialog.setVisible(true);
        }
        else if (e.getSource() == fAddBreakpointLineAddButton)
        {
            if (fAddBreakpointLineNumberTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fPanel,
                                      "You must enter a line number",
                                      "Error adding breakpoint line",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            int confirmation = JOptionPane.showConfirmDialog(fPanel,
                "Are you certain that you want to add\n" +
                "this line breakpoint trigger?",
                "Confirm Breakpoint Trigger",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            String line = fAddBreakpointLineNumberTextField.getText().trim();
            String file = fAddBreakpointLineFileTextField.getText().trim();
            String machine = fAddBreakpointLineMachineTextField.getText().trim();

            String addTriggerRequest = "ADD JOB " + fJobNumber +
                                       " BREAKPOINT LINE " +
                                       line;

            if (!(file.equals("")))
            {
                addTriggerRequest = addTriggerRequest +
                                    " FILE " + file;
            }

            if (!(machine.equals("")))
            {
                addTriggerRequest = addTriggerRequest +
                                    " MACHINE " + machine;
            }

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, addTriggerRequest);

            if (result.rc != STAFResult.Ok)
            {
                JOptionPane.showMessageDialog(fPanel,
                    "RC = " + result.rc + " Result=" + result.result,
                    "Submitted ADD BREAKPOINT",
                    JOptionPane.INFORMATION_MESSAGE);
            }
            else
            {
                fAddBreakpointLineDialog.setVisible(false);
            }
        }
        else if (e.getSource() == fAddBreakpointLineCancelButton)
        {
            fAddBreakpointLineDialog.setVisible(false);
        }
        else if (e.getSource() == fBreakpointRemoveButton)
        {
            DefaultTableModel model =
                (DefaultTableModel)fBreakpointsTable.getModel();

            int removeRow = fBreakpointsTable.getSelectedRow();

            if (removeRow > -1)
            {
                String triggerID = "";

                synchronized (model)
                {
                    triggerID = (String)model.getValueAt(removeRow, 0);
                }

                int confirmation = JOptionPane.showConfirmDialog(fPanel,
                    "Are you certain that you want to\n" +
                    "remove breakpoint trigger " + triggerID + "?",
                    "Confirm Breakpoint Trigger Remove",
                    JOptionPane.YES_NO_OPTION,
                    JOptionPane.QUESTION_MESSAGE);

                if (!(confirmation == JOptionPane.YES_OPTION))
                {
                    return;
                }
                else
                {
                    String removeTriggerRequest = "REMOVE JOB " +
                        fJobNumber + " BREAKPOINT " + triggerID;

                    STAFResult result = fHandle.submit2(
                        fStaxMachine, fStaxServiceName, removeTriggerRequest);

                    if (result.rc != STAFResult.Ok)
                    {
                        JOptionPane.showMessageDialog(fPanel,
                            "RC = " + result.rc + " Result=" + result.result,
                            "Submitted REMOVE BREAKPOINT",
                            JOptionPane.INFORMATION_MESSAGE);
                    }
                }
            }
        }
        else if (e.getSource() == fBreakpointRemoveAllButton)
        {
            int confirmation = JOptionPane.showConfirmDialog(fPanel,
                "Are you certain that you want to\n" +
                "remove all breakpointss?",
                "Confirm Breakpoint Remove",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }
            else
            {
                DefaultTableModel model =
                    (DefaultTableModel)fBreakpointsTable.getModel();

                Vector<String> removeBreakpointIDs = new Vector<String>();

                synchronized (model)
                {
                    int rowCount = model.getRowCount();

                    for (int i = 0; i < rowCount; i++)
                    {
                        String breakpointID =
                            (String)model.getValueAt(i, 0);
                        removeBreakpointIDs.add(breakpointID);
                    }
                }

                Enumeration<String> idEnum = removeBreakpointIDs.elements();

                while (idEnum.hasMoreElements())
                {
                    String removeBreakpointID = idEnum.nextElement();
                    String removeBreakpointRequest = "REMOVE JOB " +
                        fJobNumber + " BREAKPOINT " + removeBreakpointID;

                    STAFResult result = fHandle.submit2(
                        fStaxMachine, fStaxServiceName,
                        removeBreakpointRequest);

                    if (result.rc != STAFResult.Ok)
                    {
                        JOptionPane.showMessageDialog(fPanel,
                            "RC = " + result.rc + " Result=" + result.result,
                            "Submitted REMOVE BREAKPOINT",
                            JOptionPane.INFORMATION_MESSAGE);
                    }
                }
            }
        }
        else if (e.getSource() == fExecutePythonButton)
        {
            STAXMonitorThreadTreeNode selectedNode =
                (STAXMonitorThreadTreeNode)fThreadTree.
                getSelectionPath().getLastPathComponent();
            String threadID = selectedNode.toString();

            String request = "PYEXEC JOB " + fJobNumber +
                " THREAD " + STAFUtil.wrapData(threadID) +
                " CODE " + STAFUtil.wrapData(fPythonCode.getText());

            STAFResult result = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (result.rc != 0)
            {
                STAXMonitorUtil.showErrorDialog(
                    fMonitorFrame,
                    "An error was encountered while attempting to PYEXEC " +
                    " rc=" + result.rc + " result=" + result.result);
            }
            else
            {
                JOptionPane.showMessageDialog(fPanel,
                    "RC = " + result.rc + " Result=" + result.result,
                    "Submitted PYEXEC request",
                    JOptionPane.INFORMATION_MESSAGE);

                JPanel variablesPanel = getThreadVariablesPanel(threadID);

                fVariablesPane.setComponentAt(0, variablesPanel);
                fVariablesPane.setTitleAt(
                    0, "Thread " + threadID + " Variables");
            }

            return;
        }
    }

    public void valueChanged(TreeSelectionEvent e)
    {
        TreePath treePath = fThreadTree.getSelectionPath();

        if (treePath == null)
        {
            // No nodes are selected in the tree
            fXmlFileTabbedPane.setComponentAt(0, new JPanel());
            fXmlFileTabbedPane.setTitleAt(0, "XML File");
            fVariablesPane.setComponentAt(0, new JPanel());
            fVariablesPane.setTitleAt(0, "Thread Variables");
            fThreadInfoPane.setTitleAt(0, "Thread Information");
            fThreadInfoPane.setComponentAt(0, new JPanel());
            fResumeButton.setEnabled(false);
            fStepIntoButton.setEnabled(false);
            fStepOverButton.setEnabled(false);
            fStopButton.setEnabled(false);
        }
    }

    public void threadSelectionChanged()
    {
        TreePath treePath = fThreadTree.getSelectionPath();

        if (treePath != null)
        {
            STAXMonitorThreadTreeNode node =
                (STAXMonitorThreadTreeNode)(treePath.getLastPathComponent());

            if (node == null)
                return;

            String threadID = node.toString();

            if (node.isRunning())
            {
                fResumeButton.setEnabled(false);
                fStepIntoButton.setEnabled(false);
                fStepOverButton.setEnabled(false);
                fStopButton.setEnabled(true);
            }
            else
            {
                fResumeButton.setEnabled(true);
                fStepIntoButton.setEnabled(true);
                fStepOverButton.setEnabled(true);
                fStopButton.setEnabled(false);
            }

            fThreadInfoPane.setTitleAt(
                0, "Thread " + threadID + " Information");

            String parentID = node.getParentID();
            String parentHierarchy = node.getParentHierarchy();
            String startTimestamp = node.getStartTimestamp();
            String detailText = node.getDetailText();
            List callStack = node.getCallStack();

            Iterator iter = callStack.iterator();

            String callStackString = "";

            while (iter.hasNext())
            {
                if (!(callStackString.equals("")))
                {
                    callStackString = callStackString +
                        System.getProperty("line.separator");
                }

                callStackString = callStackString + iter.next().toString();
            }

            String lineNumber = "";
            String file = "";
            String machine = "";

            if (callStack.size() > 0)
            {
                String lastCallStackEntry =
                    (String)callStack.get(callStack.size() - 1);

                int beginLineIndex = lastCallStackEntry.indexOf("(Line: ") + 7;
                int endLineIndex = lastCallStackEntry.indexOf(",",
                    beginLineIndex + 1);
                lineNumber = lastCallStackEntry.substring(beginLineIndex,
                    endLineIndex);

                int beginFileIndex = lastCallStackEntry.indexOf("File: ",
                    endLineIndex) + 6;
                int endFileIndex = lastCallStackEntry.indexOf(",",
                    beginFileIndex + 1);
                file = lastCallStackEntry.substring(beginFileIndex,
                    endFileIndex);

                int beginMachineIndex = lastCallStackEntry.indexOf("Machine: ",
                    endFileIndex) + 9;
                int endMachineIndex = lastCallStackEntry.indexOf(")",
                    beginMachineIndex + 1);
                machine = lastCallStackEntry.substring(beginMachineIndex,
                    endMachineIndex);
            }

            List conditionStackList = node.getConditionStack();
            iter = conditionStackList.iterator();

            String conditionStackString = "";

            while (iter.hasNext())
            {
                if (!(conditionStackString.equals("")))
                {
                    conditionStackString = conditionStackString +
                        System.getProperty("line.separator");
                }

                conditionStackString = conditionStackString +
                    iter.next().toString();
            }

            Vector<String> threadInfoColumnNames = new Vector<String>();
            threadInfoColumnNames.add("Name");
            threadInfoColumnNames.add("Value");

            JTable threadInfoTable = new
                JTable(new STAXMonitorTableModel(threadInfoColumnNames, 0));

            threadInfoTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
            threadInfoTable.setSelectionMode(
                ListSelectionModel.SINGLE_SELECTION);
            threadInfoTable.setRowSelectionAllowed(true);
            threadInfoTable.addMouseListener(this);

            Vector<Vector<String>> threadInfoVector =
                new Vector<Vector<String>>();

            Vector<String> threadIDVector = new Vector<String>();
            threadIDVector.add("Thread ID");
            threadIDVector.add(threadID);
            threadInfoVector.add(threadIDVector);

            Vector<String> parentIDVector = new Vector<String>();
            parentIDVector.add("Parent ID");
            parentIDVector.add(parentID);
            threadInfoVector.add(parentIDVector);

            Vector<String> parentHierarchyVector = new Vector<String>();
            parentHierarchyVector.add("Parent Hierarchy");
            parentHierarchyVector.add(parentHierarchy);
            threadInfoVector.add(parentHierarchyVector);

            Vector<String> startTimestampVector = new Vector<String>();
            startTimestampVector.add("Start Date-Time");
            startTimestampVector.add(startTimestamp);
            threadInfoVector.add(startTimestampVector);

            Vector<String> callStackVector = new Vector<String>();
            callStackVector.add("Call Stack");
            callStackVector.add(callStackString);
            threadInfoVector.add(callStackVector);

            Vector<String> conditionStackVector = new Vector<String>();
            conditionStackVector.add("Condition Stack");
            conditionStackVector.add(conditionStackString);
            threadInfoVector.add(conditionStackVector);

            ((STAXMonitorTableModel)threadInfoTable.getModel()).
                setDataVector(threadInfoVector, threadInfoColumnNames);

            for (int i = 0; i < threadInfoColumnNames.size(); i++)
            {
                threadInfoTable.getColumnModel().getColumn(i).
                    setCellRenderer(
                    new STAXMonitorTableCellRenderer(
                    Color.black, false,
                    new Font("Dialog", Font.PLAIN, 12)));

                threadInfoTable.getColumnModel().getColumn(i).
                    setHeaderRenderer(
                    new STAXMonitorBreakpointTableCellRenderer(
                    new Font("Dialog", Font.BOLD, 12), Color.black, true));
            }

            threadInfoTable.updateUI();
            STAXMonitorUtil.updateRowHeights(threadInfoTable, 1);
            STAXMonitorUtil.sizeColumnsToFitText(threadInfoTable);

            fThreadInfoPane.setComponentAt(0, new JScrollPane(threadInfoTable));
            // End thread info

            JPanel variablesPanel = getThreadVariablesPanel(threadID);

            fVariablesPane.setComponentAt(0,
                variablesPanel);
            fVariablesPane.setTitleAt(0, "Thread " + threadID + " Variables");

            if (lineNumber.equals("Unknown"))
            {
                // lineNumber, file, machine can be "Unknown" if the
                // thread is initializing extensions when the job starts
                return;
            }

            fSourceFilePane = fXMLFileTextPaneMap.get(threadID);

            if (fSourceFilePane == null)
            {
                fSourceFilePane =
                    getXmlSourceFilePane(lineNumber, file, machine);
            }

            fXmlFileTabbedPane.setTitleAt(0, "XML File: " + file + " on " + machine);

            fXmlFileScrollPane = new JScrollPane(fSourceFilePane);

            final JTextPane xmlSourceFilePane = fSourceFilePane;

            final JScrollPane xmlFileScrollPane = fXmlFileScrollPane;

            xmlFileScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
            xmlFileScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            fXmlFilePanel = new JPanel();
            fXmlFilePanel.setLayout(new BorderLayout());
            fXmlFilePanel.add(BorderLayout.CENTER, xmlFileScrollPane);

            fXmlFileTabbedPane.setComponentAt(0, fXmlFilePanel);

            Runnable runnable = new Runnable()
            {
                public void run()
                {
                    Rectangle r = null;
                    Rectangle scrollRect = null;

                    try
                    {
                        r = xmlSourceFilePane.modelToView(
                            xmlSourceFilePane.getCaretPosition());

                        if (r == null)
                            return;

                        scrollRect = new Rectangle();
                        scrollRect.x = Math.min(r.x, r.x);
                        scrollRect.y = Math.min(r.y - 65, r.y - 65);
                        scrollRect.width =
                            Math.max(r.x + r.width, r.x + r.width) -
                            scrollRect.x;
                        scrollRect.height =
                            Math.max(r.y + r.height, r.y + r.height) -
                            scrollRect.y;

                        if (xmlFileScrollPane != null)
                        {
                            int h = xmlFileScrollPane.getViewport().
                                getViewRect().height;
                            scrollRect.height = h;
                        }
                    }
                    catch (BadLocationException ex)
                    {
                        // XXX
                    }

                    xmlSourceFilePane.scrollRectToVisible(scrollRect);
                }
            };

            SwingUtilities.invokeLater(runnable);
        }
        else
        {
            fXmlFileTabbedPane.setComponentAt(0, new JPanel());
            fXmlFileTabbedPane.setTitleAt(0, "XML File");
            fVariablesPane.setComponentAt(0, new JPanel());
            fVariablesPane.setTitleAt(0, "Thread Variables");
            fThreadInfoPane.setTitleAt(0, "Thread Information");
            fThreadInfoPane.setComponentAt(0, new JPanel());
            fResumeButton.setEnabled(false);
            fStepIntoButton.setEnabled(false);
            fStepOverButton.setEnabled(false);
            fStopButton.setEnabled(false);
        }
    }

    public JPanel getThreadVariablesPanel(String threadID)
    {
        STAFResult listThreadVarsResult = fHandle.submit2(
            fStaxMachine, fStaxServiceName,
            "LIST JOB " + fJobNumber + " VARS THREAD " + threadID);

        if (listThreadVarsResult.rc != STAFResult.Ok)
        {
            return new JPanel();
        }

        STAFMarshallingContext mc1 = STAFMarshallingContext.unmarshall(
            listThreadVarsResult.result);

        List varList = (List)mc1.getRootObject();

        DefaultMutableTreeNode rootNode = new DefaultMutableTreeNode(
            new BreakpointVariable("Thread " + threadID, "", ""));

        Iterator iter = varList.iterator();

        while (iter.hasNext())
        {
            HashMap varMap = (HashMap)iter.next();
            String varName = (String)varMap.get("name");
            String varType = (String)varMap.get("type");
            Object varValue = varMap.get("value");

            if ((varValue instanceof Map) ||
                (varValue instanceof List))
            {
                DefaultMutableTreeNode keyNode = new DefaultMutableTreeNode(
                    new BreakpointVariable(varName, "", varType));
                addNode(keyNode, varValue, varName, varType);
                rootNode.add(keyNode);
            }
            else
            {
                rootNode.add(
                    new DefaultMutableTreeNode(new BreakpointVariable(
                        varName, varValue.toString(), varType)));
            }
        }

        TreeTableModel model = new BreakpointTreeTableModel(rootNode);
        fThreadVariableTreeTable = new STAXMonitorTreeTable(model);
        fThreadVariableTreeTable.getTree().setRootVisible(false);
        fThreadVariableTreeTable.addMouseListener(this);

        JPanel variablesPanel = new JPanel();
        variablesPanel.setLayout(new BorderLayout());
        variablesPanel.add(
            BorderLayout.CENTER, new JScrollPane(fThreadVariableTreeTable));

        JPanel pythonPanel = new JPanel();
        pythonPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        pythonPanel.setBorder(
            new TitledBorder("Execute Python Code in Current Thread"));
        pythonPanel.add(fExecutePythonButton);
        pythonPanel.add(new JScrollPane(fPythonCode));
        variablesPanel.add(BorderLayout.SOUTH, new JScrollPane(pythonPanel));

        return variablesPanel;
    }

    public JTextPane getXmlSourceFilePane(String highlightLineNumber,
                                          String file,
                                          String machine)
    {
        boolean xmlSourceAvailable = true;

        String xmlSource = "";
        StringBuffer lineNumberedXmlSource = new StringBuffer();
        String breakpointLine = "";

        if (fXmlFileCache.containsKey(file + "@@" + machine))
        {
            xmlSource = fXmlFileCache.get(file + "@@" + machine);
        }
        else
        {
            if (machine.trim().equals(""))
            {
                return new JTextPane();
            }

            STAFResult result = fHandle.submit2(
                machine, "FS", "GET FILE " + file);

            if (result.rc != STAFResult.Ok)
            {
                xmlSourceAvailable = false;
                lineNumberedXmlSource.append(
                    "XML file is not available:\n\n" +
                    "RC:" + result.rc + "\n\n" + result.result);
            }
            else
            {
                fXmlFileCache.put(file + "@@" + machine, result.result);

                xmlSource = result.result;
            }
        }

        if (xmlSourceAvailable)
        {
            BufferedReader br = new BufferedReader(
                new StringReader(xmlSource));
            String line;
            int lineNumber = 0;
            int totalLines = 0;

            try
            {
                while ((line = br.readLine()) != null)
                {
                    totalLines++;
                }

                int maxLineNumLength = String.valueOf(totalLines).length();

                br = new BufferedReader(new StringReader(xmlSource));

                while ((line = br.readLine()) != null)
                {
                    lineNumber = lineNumber + 1;

                    String currentLineString = "";

                    int currentLineNumLength =
                        String.valueOf(lineNumber).length();

                    for (int i = 1; i <=
                         (maxLineNumLength - currentLineNumLength); i++)
                    {
                        currentLineString = " " + currentLineString;
                    }

                    currentLineString = currentLineString +
                        String.valueOf(lineNumber) + " ";

                    lineNumberedXmlSource.append(
                        currentLineString + line + "\n");

                    if (highlightLineNumber.equals(String.valueOf(lineNumber)))
                    {
                        breakpointLine = currentLineString + line;
                    }
                }
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }
        }

        final JTextPane xmlSourceFilePane = new JTextPane();
        xmlSourceFilePane.setForeground(Color.black);
        xmlSourceFilePane.setEditable(false);
        xmlSourceFilePane.setText(lineNumberedXmlSource.toString());
        xmlSourceFilePane.setFont(new Font("Courier", Font.PLAIN, 12));
        xmlSourceFilePane.addMouseListener(this);

        fBeginXmlSelection = 0;

        if (xmlSourceAvailable)
        {
            fBeginXmlSelection =
                lineNumberedXmlSource.toString().indexOf(breakpointLine);
            int endXmlSelection =
                fBeginXmlSelection + breakpointLine.length();
            xmlSourceFilePane.setSelectionStart(fBeginXmlSelection);
            xmlSourceFilePane.setSelectionEnd(endXmlSelection);
            xmlSourceFilePane.setCaretPosition(fBeginXmlSelection);

            try
            {
                xmlSourceFilePane.getHighlighter().addHighlight(
                    fBeginXmlSelection, endXmlSelection,
                    DefaultHighlighter.DefaultPainter);
            }
            catch (BadLocationException ex)
            {
                // XXX
            }
        }

        return xmlSourceFilePane;
    }

    public void mouseClicked(MouseEvent e)
    {
        if (e.getClickCount() == 2)
        {
            if (e.getSource() instanceof STAXMonitorTreeTable)
            {
                JTable target = (JTable)e.getSource();
                int row = target.getSelectedRow();

                JDialog variableDialog = new JDialog(fMonitorFrame,
                    target.getValueAt(row, 0).toString() + " " +
                    target.getValueAt(row, 2).toString(), false);

                variableDialog.setSize(new Dimension(400, 200));
                JPanel variableValuePanel = new JPanel();
                variableValuePanel.setLayout(new BorderLayout());
                JEditorPane variableValueEditorPane = new JEditorPane();
                variableValueEditorPane.setContentType("text/plain");
                variableValueEditorPane.setEditable(false);
                variableValueEditorPane.setBorder(new
                    TitledBorder("Variable value"));
                variableValueEditorPane.setText(
                    target.getValueAt(row, 1).toString());

                variableValuePanel.add(BorderLayout.CENTER,
                    new JScrollPane(variableValueEditorPane));

                variableDialog.setLocationRelativeTo(fPanel);
                variableDialog.getContentPane().add(
                    variableValuePanel);

                variableDialog.setVisible(true);
            }
        }
        else if (e.getClickCount() == 1)
        {
            if (e.getSource() == fThreadTree)
            {
                // Refresh the Thread Tree with current status for all threads
                refreshThreadTree();

                TreePath selectionPath =
                    fThreadTree.getPathForLocation(e.getX(), e.getY());
                fThreadTree.setSelectionPath(selectionPath);

                threadSelectionChanged();
            }
        }
    }

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
            if (e.getSource() instanceof STAXMonitorTreeTable)
            {
                fTreeTablePopupMenu.show(e.getComponent(), e.getX(), e.getY());
            }
            else if (e.getSource() instanceof JTextPane)
            {
                fXmlSourcePopupMenu.show(e.getComponent(), e.getX(), e.getY());
            }
        }
    }


public class BreakpointTreeTableModel extends AbstractTreeTableModel
{

    public BreakpointTreeTableModel(Object root)
    {
        super(root);
    }

    public Class getColumnClass(int column)
    {
        if (column == 0)
        {
            return TreeTableModel.class;
        }
        else
        {
            return Object.class;
        }
    }

    public Object getChild(Object parent, int index)
    {
        MutableTreeNode treenode = (MutableTreeNode)parent;
        return treenode.getChildAt(index);
    }

    public int getChildCount(Object parent)
    {
        MutableTreeNode treenode = (MutableTreeNode)parent;
        return treenode.getChildCount();
    }

    public int getColumnCount()
    {
        return 3;
    }

    public String getColumnName(int column)
    {
        switch (column)
        {
            case 0:
                return " Name";
            case 1:
                return " Value";
            case 2:
                return " Type";
            default:
                return null;
        }
    }

    public Object getValueAt(Object node, int column)
    {
        DefaultMutableTreeNode treenode = (DefaultMutableTreeNode) node;
        BreakpointVariable variable =
            (BreakpointVariable)treenode.getUserObject();

        switch (column)
        {
            case 0:
                return variable.varName;
            case 1:
                return variable.varValue;
            case 2:
                return variable.varType;
            default:
                return null;
        }
    }
}

public static class BreakpointVariable
{
    String varName = "";
    String varValue = "";
    String varType = "";

    public BreakpointVariable(String varName, String varValue, String varType)
    {
        this.varName = varName;
        this.varValue = varValue;
        this.varType = varType;
    }

    public String toString()
    {
        return varName;
    }
}

public class STAXMonitorTreeTable extends JTable
{
    TreeTableCellRenderer tree;

    public STAXMonitorTreeTable(TreeTableModel treeTableModel)
    {
        super();

        tree = new TreeTableCellRenderer(treeTableModel);

        super.setModel(new TreeTableModelAdapter(treeTableModel, tree));

        ListToTreeSelectionModelWrapper selectionWrapper = new
                                ListToTreeSelectionModelWrapper();
        tree.setSelectionModel(selectionWrapper);
        tree.setShowsRootHandles(true);
        setSelectionModel(selectionWrapper.getListSelectionModel());

        setDefaultRenderer(TreeTableModel.class, tree);
        setDefaultEditor(TreeTableModel.class, new TreeTableCellEditor());

        getColumnModel().getColumn(0).setHeaderRenderer(
            new TreeTableHeaderCellRenderer());
        getColumnModel().getColumn(1).setHeaderRenderer(
            new TreeTableHeaderCellRenderer());
        getColumnModel().getColumn(2).setHeaderRenderer(
            new TreeTableHeaderCellRenderer());

        setShowGrid(false);

        setIntercellSpacing(new Dimension(0, 0));
    }

    public void updateUI()
    {
        super.updateUI();

        if (tree != null)
        {
            tree.updateUI();
        }
    }

    public int getEditingRow()
    {
        return editingRow;
    }

    public void setRowHeight(int rowHeight)
    {
        super.setRowHeight(rowHeight);

        if ((tree != null) && (tree.getRowHeight() != rowHeight))
        {
            tree.setRowHeight(getRowHeight());
        }
    }

    public JTree getTree()
    {
        return tree;
    }

    public class TreeTableCellRenderer extends JTree implements
                 TableCellRenderer
    {
        int visibleRow;

        public TreeTableCellRenderer(TreeModel model)
        {
            super(model);
        }

        public void updateUI()
        {
            super.updateUI();

            TreeCellRenderer renderer = getCellRenderer();

            if (renderer instanceof DefaultTreeCellRenderer)
            {
                DefaultTreeCellRenderer defaultRenderer =
                    ((DefaultTreeCellRenderer)renderer);
                defaultRenderer.setTextSelectionColor(UIManager.getColor
                    ("Table.selectionForeground"));
                defaultRenderer.setBackgroundSelectionColor(UIManager.getColor
                    ("Table.selectionBackground"));
                defaultRenderer.setLeafIcon(null);
                defaultRenderer.setOpenIcon(null);
                defaultRenderer.setClosedIcon(null);
            }
        }

        public void setRowHeight(int rowHeight)
        {
            if (rowHeight > 0)
            {
                super.setRowHeight(rowHeight);

                if ((STAXMonitorTreeTable.this != null) &&
                    (STAXMonitorTreeTable.this.getRowHeight() != rowHeight))
                {
                    STAXMonitorTreeTable.this.setRowHeight(getRowHeight());
                }
            }
        }

        public void setBounds(int x, int y, int w, int h)
        {
            super.setBounds(x, 0, w, STAXMonitorTreeTable.this.getHeight());
        }

        public void paint(Graphics g)
        {
            g.translate(0, -visibleRow * getRowHeight());
            super.paint(g);
        }

        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row, int column)
        {
            if (isSelected)
            {
                setBackground(table.getSelectionBackground());
            }
            else
            {
                setBackground(table.getBackground());
            }

            visibleRow = row;

            return this;
        }
    }

    public class TreeTableHeaderCellRenderer extends JTextField implements
                 TableCellRenderer
    {
        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row, int column)
        {
            setText(value.toString());
            setBackground(Color.lightGray);
            setEditable(false);
            setFont(new Font("Dialog", Font.BOLD, 12));
            setBorder(BorderFactory.createRaisedBevelBorder());

            return this;
        }
    }

    public class TreeTableCellEditor extends AbstractCellEditor implements
                 TableCellEditor
    {
        public Component getTableCellEditorComponent(JTable table,
                                                     Object value,
                                                     boolean isSelected,
                                                     int r, int c)
        {
            return tree;
        }

        public boolean isCellEditable(EventObject e)
        {
            if (e instanceof MouseEvent)
            {
                for (int i = getColumnCount() - 1; i >= 0; i--)
                {
                    if (getColumnClass(i) == TreeTableModel.class)
                    {
                        MouseEvent me = (MouseEvent)e;
                        MouseEvent newME = new MouseEvent(
                            tree, me.getID(), me.getWhen(), me.getModifiers(),
                            me.getX() - getCellRect(0, i, true).x,
                            me.getY(), me.getClickCount(),
                            me.isPopupTrigger());
                        tree.dispatchEvent(newME);
                        break;
                    }
                }
            }

            return false;
        }
    }

    class ListToTreeSelectionModelWrapper extends DefaultTreeSelectionModel
    {
        boolean updatingListSelectionModel;

        public ListToTreeSelectionModelWrapper()
        {
            super();
            getListSelectionModel().addListSelectionListener
                (createListSelectionListener());
        }

        ListSelectionModel getListSelectionModel()
        {
            return listSelectionModel;
        }

        public void resetRowSelection()
        {
            if (!updatingListSelectionModel)
            {
                updatingListSelectionModel = true;

                try
                {
                    super.resetRowSelection();
                }
                finally
                {
                    updatingListSelectionModel = false;
                }
            }
        }

        protected ListSelectionListener createListSelectionListener()
        {
            return new ListSelectionHandler();
        }

        protected void updateSelectedPathsFromSelectedRows()
        {
            if (!updatingListSelectionModel)
            {
                updatingListSelectionModel = true;

                try
                {
                    int min = listSelectionModel.getMinSelectionIndex();
                    int max = listSelectionModel.getMaxSelectionIndex();

                    clearSelection();

                    if ((min != -1) && (max != -1))
                    {
                        for (int i = min; i <= max; i++)
                        {
                            if (listSelectionModel.isSelectedIndex(i))
                            {
                                TreePath selPath = tree.getPathForRow(i);

                                if(selPath != null)
                                {
                                    addSelectionPath(selPath);
                                }
                            }
                        }
                    }
                }
                finally
                {
                    updatingListSelectionModel = false;
                }
            }
        }

        class ListSelectionHandler implements ListSelectionListener
        {
            public void valueChanged(ListSelectionEvent e)
            {
                updateSelectedPathsFromSelectedRows();
            }
        }
    }
}

public interface TreeTableModel extends TreeModel
{
    public Class getColumnClass(int column);

    public int getColumnCount();

    public String getColumnName(int column);

    public Object getValueAt(Object node, int column);

    public boolean isCellEditable(Object node, int column);

    public void setValueAt(Object aValue, Object node, int column);
}

public abstract class AbstractTreeTableModel implements TreeTableModel
{
    Object root;
    EventListenerList listenerList = new EventListenerList();

    public AbstractTreeTableModel(Object root)
    {
        this.root = root;
    }

    public Object getRoot()
    {
        return root;
    }

    public boolean isLeaf(Object node)
    {
        return getChildCount(node) == 0;
    }

    public void valueForPathChanged(TreePath path, Object newValue) {}

    public int getIndexOfChild(Object parent, Object child)
    {
        for (int i = 0; i < getChildCount(parent); i++)
        {
                if (getChild(parent, i).equals(child))
            {
                    return i;
                }
        }

            return -1;
    }

    public void addTreeModelListener(TreeModelListener l)
    {
        listenerList.add(TreeModelListener.class, l);
    }

    public void removeTreeModelListener(TreeModelListener l)
    {
        listenerList.remove(TreeModelListener.class, l);
    }

    protected void fireTreeNodesChanged(Object source, Object[] path,
                                        int[] childIndices,
                                        Object[] children)
    {
        Object[] listeners = listenerList.getListenerList();
        TreeModelEvent e = null;

        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==TreeModelListener.class)
            {
                if (e == null)
                {
                    e = new TreeModelEvent(source, path,
                                           childIndices, children);
                }

                ((TreeModelListener)listeners[i+1]).treeNodesChanged(e);
            }
        }
    }

    protected void fireTreeNodesInserted(Object source, Object[] path,
                                        int[] childIndices,
                                        Object[] children)
    {
        Object[] listeners = listenerList.getListenerList();
        TreeModelEvent e = null;

        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==TreeModelListener.class)
            {
                if (e == null)
                {
                    e = new TreeModelEvent(source, path,
                                           childIndices, children);
                }

                ((TreeModelListener)listeners[i+1]).treeNodesInserted(e);
            }
        }
    }

    protected void fireTreeNodesRemoved(Object source, Object[] path,
                                        int[] childIndices,
                                        Object[] children)
    {
        Object[] listeners = listenerList.getListenerList();
        TreeModelEvent e = null;

        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==TreeModelListener.class)
            {
                if (e == null)
                {
                    e = new TreeModelEvent(source, path,
                                           childIndices, children);
                }

                ((TreeModelListener)listeners[i+1]).treeNodesRemoved(e);
            }
        }
    }

    protected void fireTreeStructureChanged(Object source, Object[] path,
                                        int[] childIndices,
                                        Object[] children)
    {
        Object[] listeners = listenerList.getListenerList();
        TreeModelEvent e = null;

        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==TreeModelListener.class)
            {
                if (e == null)
                {
                    e = new TreeModelEvent(source, path,
                                           childIndices, children);
                }

                ((TreeModelListener)listeners[i+1]).treeStructureChanged(e);
            }
        }
    }

    public Class getColumnClass(int column)
    {
        return Object.class;
    }

    public boolean isCellEditable(Object node, int column)
    {
         return getColumnClass(column) == TreeTableModel.class;
    }

    public void setValueAt(Object aValue, Object node, int column) {}
}

public class TreeTableModelAdapter extends AbstractTableModel
{
    JTree tree;
    TreeTableModel treeTableModel;

    public TreeTableModelAdapter(TreeTableModel treeTableModel, JTree tree)
    {
        this.tree = tree;
        this.treeTableModel = treeTableModel;

        tree.addTreeExpansionListener(new TreeExpansionListener()
        {
            public void treeExpanded(TreeExpansionEvent event)
            {
                fireTableDataChanged();
            }

            public void treeCollapsed(TreeExpansionEvent event)
            {
                fireTableDataChanged();
            }
        });

        treeTableModel.addTreeModelListener(new TreeModelListener()
        {
            public void treeNodesChanged(TreeModelEvent e)
            {
                delayedFireTableDataChanged();
            }

            public void treeNodesInserted(TreeModelEvent e)
            {
                delayedFireTableDataChanged();
            }

            public void treeNodesRemoved(TreeModelEvent e)
            {
                delayedFireTableDataChanged();
            }

            public void treeStructureChanged(TreeModelEvent e)
            {
                delayedFireTableDataChanged();
            }
        });
    }

    protected void delayedFireTableDataChanged()
    {
        SwingUtilities.invokeLater(new Runnable()
        {
            public void run()
            {
                fireTableDataChanged();
            }
        });
    }

    public Class getColumnClass(int column)
    {
        return treeTableModel.getColumnClass(column);
    }

    public int getColumnCount()
    {
        return treeTableModel.getColumnCount();
    }

    public String getColumnName(int column){
        return treeTableModel.getColumnName(column);
    }

    public int getRowCount()
    {
        return tree.getRowCount();
    }

    public Object getValueAt(int row, int column)
    {
        return treeTableModel.getValueAt(nodeForRow(row), column);
    }

    public boolean isCellEditable(int row, int column)
    {
         return treeTableModel.isCellEditable(nodeForRow(row), column);
    }

    protected Object nodeForRow(int row)
    {
        TreePath treePath = tree.getPathForRow(row);
        return treePath.getLastPathComponent();
    }

    public void setValueAt(Object value, int row, int column)
    {
        treeTableModel.setValueAt(value, nodeForRow(row), column);
    }
}

public class AbstractCellEditor implements CellEditor
{

    protected EventListenerList listenerList = new EventListenerList();

    public Object getCellEditorValue()
    {
        return null;
    }

    public boolean isCellEditable(EventObject e)
    {
        return true;
    }

    public boolean shouldSelectCell(EventObject anEvent)
    {
        return false;
    }

    public boolean stopCellEditing()
    {
        return true;
    }

    public void cancelCellEditing() {}

    public void addCellEditorListener(CellEditorListener l)
    {
        listenerList.add(CellEditorListener.class, l);
    }

    public void removeCellEditorListener(CellEditorListener l)
    {
        listenerList.remove(CellEditorListener.class, l);
    }

    protected void fireEditingStopped()
    {
        Object[] listeners = listenerList.getListenerList();

        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==CellEditorListener.class)
            {
                ((CellEditorListener)listeners[i+1]).
                    editingStopped(new ChangeEvent(this));
            }
        }
    }

    protected void fireEditingCanceled()
    {
        Object[] listeners = listenerList.getListenerList();

        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==CellEditorListener.class)
            {
                ((CellEditorListener)listeners[i+1]).
                    editingCanceled(new ChangeEvent(this));
            }
        }
    }
}

public class STAXMonitorThreadTreeCellRenderer extends JLabel
                                               implements TreeCellRenderer
{
    private java.net.URL greenballURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/greenball.gif");
    private java.net.URL redballURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/redball.gif");

    private ImageIcon runningThreadIcon = new ImageIcon(greenballURL);
    private ImageIcon stoppedThreadIcon = new ImageIcon(redballURL);

    private Font treeFont = new Font("Dialog", Font.PLAIN, 12);
    public Color selectionColor =
        UIManager.getColor("Tree.selectionBackground");

    public STAXMonitorThreadTreeCellRenderer()
    {
        super();
        setOpaque(true);
        setBackground(Color.white);
        setFont(treeFont);
    }

    public Component getTreeCellRendererComponent(JTree tree, Object value,
                                                  boolean selected,
                                                  boolean expanded,
                                                  boolean leaf,
                                                  int row, boolean hasFocus)
    {
        String text = (value == null) ? "" : String.valueOf(value);

        if (value instanceof STAXMonitorThreadTreeNode)
        {
            STAXMonitorThreadTreeNode node = (STAXMonitorThreadTreeNode) value;

            if (node.isRunning())
            {
                setIcon(runningThreadIcon);
            }
            else
            {
                setIcon(stoppedThreadIcon);
            }

            setText(text + " " + node.getDetailText());
        }
        else
        {
            setText(text);
        }

        if (selected)
        {
            setBackground(selectionColor);
        }
        else
        {
            setBackground(Color.white);
        }

        return this;
    }
}

public class STAXMonitorBreakpointTableCellRenderer
             extends JLabel implements TableCellRenderer
{
    public Hashtable rowHeights = new Hashtable();
    private boolean isHeader = true;

    public STAXMonitorBreakpointTableCellRenderer()
    {
        this(new Font("Dialog", Font.PLAIN, 12), Color.white, false);
        isHeader = false;
    }

    public STAXMonitorBreakpointTableCellRenderer(int alignment)
    {
        this(new Font("Dialog", Font.PLAIN, 12), Color.white, false);
        isHeader = false;
        setHorizontalAlignment(alignment);
    }

    public STAXMonitorBreakpointTableCellRenderer(Font font,
                                                  Color background,
                                                  boolean isHeader)
    {
        this.isHeader = isHeader;
        setFont(font);
        setOpaque(true);
        setBackground(background);
        setForeground(Color.black);
        setHorizontalAlignment(SwingConstants.LEFT);

        setBorder(BorderFactory.createRaisedBevelBorder());
    }

    public void clearRowHeights()
    {
        rowHeights.clear();
    }

    public Component getTableCellRendererComponent(JTable table,
                                                   Object value,
                                                   boolean isSelected,
                                                   boolean hasFocus,
                                                   int row, int col)
    {
        if (isHeader)
        {
            setBackground(Color.lightGray);
        }
        else if (isSelected)
        {
            setBackground(UIManager.getColor("Table.selectionBackground"));
        }
        else
        {
            setBackground(Color.white);
        }

        setText((value == null) ? "" : String.valueOf(value));

        return this;
    }
}

public class STAXMonitorThreadTreeNode extends DefaultMutableTreeNode
{
    private boolean fRunning = true;
    private String fDetailText = "";
    private String fParentID = null;
    private String fParentHierarchy = null;
    private String fStartTimestamp;
    private List fCallStack = new ArrayList();
    private List fConditionStack = new ArrayList();

    // Flag to indicate if the thread is no longer running.
    // This is used if the thread stop event arrives before the thread
    // start event.
    private boolean fStoppedState = false;

    private ImageIcon icon;
    private JComponent component = null;

    public STAXMonitorThreadTreeNode()
    {
        super();
    }

    public STAXMonitorThreadTreeNode(Object userObject)
    {
        super(userObject);
    }

    public STAXMonitorThreadTreeNode(
        Object userObject, String parentID, String parentHierarchy,
        String startTimestamp, List callStack, List conditionStack)
    {
        super(userObject);
        fParentID = parentID;
        fParentHierarchy = parentHierarchy;
        fStartTimestamp = startTimestamp;
        fCallStack = callStack;
        fConditionStack = conditionStack;
    }

    public String getParentID()
    {
        return fParentID;
    }

    public String getParentHierarchy()
    {
        return fParentHierarchy;
    }

    public String getStartTimestamp()
    {
        return fStartTimestamp;
    }

    public List getCallStack()
    {
        return fCallStack;
    }

    public List getConditionStack()
    {
        return fConditionStack;
    }

    public boolean isRunning()
    {
        return fRunning;
    }

    public void setRunning(boolean running)
    {
        fRunning = running;
    }

    public String getDetailText()
    {
        return fDetailText;
    }

    public void setDetailText(String detailText)
    {
        fDetailText = detailText;
    }

    public boolean isStoppedState()
    {
        return fStoppedState;
    }

    public void setStoppedState(boolean stoppedState)
    {
        fStoppedState = stoppedState;
    }
}

class MultiLineToolTip extends JToolTip
{
    public MultiLineToolTip()
    {
        setUI(new MultiLineToolTipUI());
    }
}

class MultiLineToolTipUI extends MetalToolTipUI
    {
    String[] lines;
    int maxWidth = 0;

    public void paint(Graphics g, JComponent c)
    {
        FontMetrics metrics = c.getFontMetrics(g.getFont());
        Dimension size = c.getSize();
        g.setColor(c.getBackground());
        g.fillRect(0, 0, size.width, size.height);
        g.setColor(c.getForeground());

        if (lines != null)
        {
            for (int i = 0; i < lines.length; i++)
            {
                g.drawString(lines[i], 3, (metrics.getHeight()) * (i + 1));
            }
        }
    }

    public Dimension getPreferredSize(JComponent c)
    {
        FontMetrics metrics = c.getFontMetrics(c.getFont());
        String tipText = ((JToolTip)c).getTipText();

        if (tipText == null)
        {
            tipText = "";
        }

        BufferedReader br = new BufferedReader(new StringReader(tipText));
        String line;
        int maxWidth = 0;
        Vector<String> lineVector = new Vector<String>();

        try
        {
            while ((line = br.readLine()) != null)
            {
                int width =
                    SwingUtilities.computeStringWidth(metrics, line);

                if (maxWidth < width)
                {
                    maxWidth = width;
                }

                lineVector.add(line);
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        int numberOfLines = lineVector.size();

        if (numberOfLines < 1)
        {
            lines = null;
            numberOfLines = 1;
        }
        else
        {
            lines = new String[numberOfLines];
            int i = 0;

            for (Enumeration<String> e = lineVector.elements();
                 e.hasMoreElements();
                 i++)
            {
                lines[i] = e.nextElement();
            }
        }

        int height = metrics.getHeight() * numberOfLines;
        this.maxWidth = maxWidth;

        return new Dimension(maxWidth + 8, height + 8);
    }
}

}
