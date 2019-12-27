/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import javax.swing.*;
import javax.swing.event.*;
import com.ibm.staf.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import java.io.*;

public class STAXMonitorSubjobExtension extends JFrame
                                        implements STAXMonitorExtension,
                                                   ActionListener,
                                                   MouseListener,
                                                   ListSelectionListener
{
    JPanel fPanel;
    STAFHandle fHandle;
    JTable fSubjobTable;
    STAXMonitorTableModel fSubjobTableModel;
    STAXMonitorTableSorter fSubjobModelSorter;
    Vector<String> fSubjobColumns;
    Hashtable<String, Calendar> fSubjobStartTimes =
        new Hashtable<String, Calendar>();
    String fStaxMachine;
    String fStaxServiceName;
    String fStaxMachineNickname;
    String fJobNumber;
    MonitorElapsedTime fElapsedTime;
    boolean fContinueElapsedTime = true;
    STAXMonitorFrame fMonitorFrame;
    String fLogViewerFontName = "Dialog";
    String fSaveAsDirectory = null;
    String fTitle;
    Hashtable<String, Vector> fSubjobHashtable =
        new Hashtable<String, Vector>();
    JPopupMenu fSubjobPopupMenu = new JPopupMenu();
    JMenuItem fSubjobStartMonitorMenuItem = new JMenuItem("Start Monitoring");
    JMenuItem fSubjobShowJobLogMenuItem = new JMenuItem("Display Job Log");
    JMenuItem fSubjobShowJobUserLogMenuItem =
        new JMenuItem("Display Job User Log");
    JMenuItem fSubjobTerminateMenuItem = new JMenuItem("Terminate Job");

    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName,
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxMachineNickname = monitorFrame.getSTAXMachineNickname();
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;
        fLogViewerFontName = monitorFrame.getParentMonitor().
            getLogViewerFontName();
        fSaveAsDirectory = monitorFrame.getParentMonitor().
            getSaveAsDirectory();

        fTitle = "Sub-jobs";

        fPanel = new JPanel();
        fPanel.setLayout(new BorderLayout());

        try
        {
            fHandle = STAXMonitorUtil.getNewSTAFHandle(
                "STAFMonitorSubjobTableExtension");
        }
        catch (STAFException ex)
        {
        }

        fSubjobColumns = new Vector<String>();
        fSubjobColumns.addElement("Job ID");
        fSubjobColumns.addElement("Job Name");
        fSubjobColumns.addElement("Function");
        fSubjobColumns.addElement("Status");
        fSubjobColumns.addElement("Started");
        fSubjobColumns.addElement("Elapsed Time");
        fSubjobColumns.addElement("Result");

        fSubjobTableModel = new STAXMonitorTableModel(fSubjobColumns, 0);

        fSubjobModelSorter =
            new STAXMonitorTableSorter(fSubjobTableModel, 0);

        fSubjobTable = new JTable(fSubjobModelSorter);
        fSubjobModelSorter.addMouseListenerToHeaderInTable(
            fSubjobTable, 6);
        fSubjobTable.setRowSelectionAllowed(true);
        fSubjobTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        fSubjobTable.addMouseListener(this);
        fSubjobTable.getSelectionModel().addListSelectionListener(this);

        fSubjobTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSubjobTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.blue));

        fSubjobTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.getColumnModel().getColumn(2).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSubjobTable.getColumnModel().getColumn(2).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.getColumnModel().getColumn(3).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSubjobTable.getColumnModel().getColumn(3).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.getColumnModel().getColumn(4).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSubjobTable.getColumnModel().getColumn(4).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.getColumnModel().getColumn(5).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSubjobTable.getColumnModel().getColumn(5).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.getColumnModel().getColumn(6).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSubjobTable.getColumnModel().getColumn(6).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSubjobTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

        STAXMonitorUtil.sizeColumnsToFitText(fSubjobTable);

        if (!newJob)
        {
            String listRequest = "LIST JOB " + fJobNumber + " SUBJOBS";
            STAFResult listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, listRequest);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Request=" + listRequest +
                    "\nResult=" + listResult.result);
            }

            java.util.List subjobList = (java.util.List)listResult.resultObj;
            Iterator iter = subjobList.iterator();
            
            while (iter.hasNext())
            {
                Map subjobMap = (Map)iter.next();
                String jobID   = (String)subjobMap.get("jobID");
                String jobName = (String)subjobMap.get("jobName");
                String function = (String)subjobMap.get("function");

                // startTimestamp format is YYYYMMDD-HH:MM:SS
                String startTimestamp =
                    (String)subjobMap.get("startTimestamp");
                String startDate = startTimestamp.substring(0, 8);
                String startTime = startTimestamp.substring(9);

                fSubjobStartTimes.put(
                    jobID, STAXMonitorUtil.getCalendar2(startDate, startTime));
                
                STAFResult queryResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, "QUERY JOB " + jobID);

                String fileName = "";
                String fileMachine = "";
                String args = "";
                String status = "Running";

                if (queryResult.rc == 48)
                {
                    status = "Complete";
                }
                else if (queryResult.rc == 0)
                {
                    Map jobInfoMap = (HashMap)queryResult.resultObj;

                    fileName = (String)jobInfoMap.get("xmlFileName");
                    fileMachine = (String)jobInfoMap.get("fileMachine");
                    args = (String)jobInfoMap.get("arguments");
                }

                String elapsedTimestamp = "00:00:00";
                
                Object rowData[] = new Object[8];
                rowData[0] = new String(jobID);
                rowData[1] = jobName;
                rowData[2] = function;
                rowData[3] = status;
                rowData[4] = startTimestamp;
                rowData[5] = elapsedTimestamp;
                rowData[6] = "";

                fSubjobTableModel.addRow(rowData);

                STAXMonitorUtil.updateRowHeights(fSubjobTable, 6);
                STAXMonitorUtil.sizeColumnsToFitText(fSubjobTable);

                STAXMonitorUtil.updateRowHeights(fSubjobTable, 6);
                STAXMonitorUtil.sizeColumnsToFitText(fSubjobTable);

                Vector<Vector<String>> subjobDataVector =
                    new Vector<Vector<String>>();

                addRow(subjobDataVector, "Job ID", jobID);
                addRow(subjobDataVector, "Job Name", jobName);
                addRow(subjobDataVector, "Job File", fileName);

                if (!fileMachine.equals(""))
                {
                    addRow(subjobDataVector, "Job File Machine",
                        fileMachine);
                }

                addRow(subjobDataVector, "Function", function);
                addRow(subjobDataVector, "Function Args", args);

                synchronized(fSubjobHashtable)
                {
                    fSubjobHashtable.put("Job " + jobID + " - " + jobName,
                        subjobDataVector);
                }
            }
        }

        fSubjobPopupMenu.add(fSubjobStartMonitorMenuItem);
        fSubjobStartMonitorMenuItem.addActionListener(this);
        fSubjobPopupMenu.addSeparator();
        fSubjobPopupMenu.add(fSubjobShowJobLogMenuItem);
        fSubjobShowJobLogMenuItem.addActionListener(this);
        fSubjobPopupMenu.add(fSubjobShowJobUserLogMenuItem);
        fSubjobShowJobUserLogMenuItem.addActionListener(this);
        fSubjobPopupMenu.addSeparator();
        fSubjobPopupMenu.add(fSubjobTerminateMenuItem);
        fSubjobTerminateMenuItem.addActionListener(this);

        fElapsedTime = new MonitorElapsedTime();
        fElapsedTime.start();

        return fSubjobTable;
    }

    public String getNotificationEventTypes()
    {
        return "subjob";
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
        return fSubjobTable;
    }

    public void valueChanged(ListSelectionEvent e)
    {
        int selectedRow = fSubjobTable.getSelectedRow();

        if (selectedRow == -1)
            return;

        String subjobID = (String)
            fSubjobTable.getValueAt(selectedRow, 0);

        String subjobName = (String)
            fSubjobTable.getValueAt(selectedRow, 1);

        synchronized(fSubjobHashtable)
        {
            fMonitorFrame.updateCurrentSelection("Job " + subjobID +
                " - " + subjobName, (Vector)fSubjobHashtable.get("Job " +
                subjobID + " - " + subjobName));
        }
    }

    public void handleEvent(Map map)
    {
        String block = (String)map.get("block");

        String jobID = (String)map.get("jobID");

        String jobName = (String)map.get("jobName");

        if (jobName == null) jobName = "<N/A>";

        if (jobName.equals("")) jobName = "<N/A>";

        String function = (String)map.get("function");

        if (function == null) function = "";

        String status = (String)map.get("status");

        String jobFile = (String)map.get("jobfile");

        if (jobFile == null) jobFile = "";

        String jobFileMachine = (String)map.get("jobfilemachine");

        if (jobFileMachine == null) jobFileMachine = "";

        String functionArgs = (String)map.get("functionargs");

        if (functionArgs == null) functionArgs = "";

        String clearLogs = (String)map.get("clearlogs");

        if (clearLogs == null) clearLogs = "";

        String logTCElapsedTime = (String)map.get("logtcelapsedtime");

        if (logTCElapsedTime == null) logTCElapsedTime = "";

        String logTCNumStarts = (String)map.get("logtcnumstarts");

        if (logTCNumStarts == null) logTCNumStarts = "";

        String logTCStartStop = (String)map.get("logtcstartstop");

        if (logTCStartStop == null) logTCStartStop = "";

        String pythonOutput = (String)map.get("pythonoutput");

        if (pythonOutput == null) pythonOutput = "";
        
        String pythonLogLevel = (String)map.get("pythonloglevel");

        if (pythonLogLevel == null) pythonLogLevel = "";

        String scriptFilesMachine = (String)map.get("scriptfilesmachine");

        if (scriptFilesMachine == null) scriptFilesMachine = "";

        if (status.equals("start"))
        {
            String startDate = (String)map.get("startdate");

            if (startDate == null) startDate = "";

            String startTime = (String)map.get("starttime");

            if (startTime == null) startTime = "";

            Object rowData[] = new Object[7];
            rowData[0] = jobID;
            rowData[1] = jobName;
            rowData[2] = function;
            rowData[3] = "Running";
            rowData[4] = startDate + "-" + startTime;
            rowData[5] = "00:00:00";
            rowData[6] = "";

            fSubjobTableModel.addRow(rowData);

            STAXMonitorUtil.updateRowHeights(fSubjobTable, 6);
            STAXMonitorUtil.sizeColumnsToFitText(fSubjobTable);

            Vector<Vector<String>> subjobDataVector =
                new Vector<Vector<String>>();

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

            boolean endOfScripts = false;
            int scriptIndex = 0;

            while (!endOfScripts)
            {
                String scriptfile = (String)map.get("scriptfile" +
                                                    scriptIndex++);

                if (scriptfile != null)
                {
                    if (scriptIndex == 1 && !scriptFilesMachine.equals(""))
                    {
                        addRow(subjobDataVector, "Script Files Machine",
                               scriptFilesMachine);
                    }

                    addRow(subjobDataVector, "Script File #" + scriptIndex,
                           scriptfile);
                }
                else
                {
                   endOfScripts = true;
                }
            }

            endOfScripts = false;
            scriptIndex = 0;

            while (!endOfScripts)
            {
                String script = (String)map.get("script" + scriptIndex++);

                if (script != null)
                {
                    addRow(subjobDataVector, "Script #" + scriptIndex, script);
                }
                else
                {
                   endOfScripts = true;
                }
            }

            addRow(subjobDataVector, "Started", startDate + "-" + startTime);
            addRow(subjobDataVector, "Block", block);

            synchronized(fSubjobHashtable)
            {
                fSubjobHashtable.put("Job " + jobID + " - " + jobName,
                    subjobDataVector);
            }

            fSubjobStartTimes.put(
                jobID, STAXMonitorUtil.getCalendar2(startDate, startTime));
        }
        else if (status.equals("stop"))
        {
            String result = STAFMarshallingContext.formatObject(
              map.get("result"));
            
            Vector jobsVector = fSubjobTableModel.getDataVector();

            boolean found = false;
            int rowIndex = -1;

            for (int j = 0; j < jobsVector.size(); j++)
            {
                if (!found)
                {
                    if (((Vector)(jobsVector.elementAt(j))).
                               elementAt(0).equals(jobID))
                    {
                        found = true;
                        rowIndex = j;
                    }
                }
            }

            if (rowIndex > -1)
            {
                fSubjobTableModel.setValueAt("Complete", rowIndex, 3);
                fSubjobTableModel.setValueAt(result, rowIndex, 6);
            }

            fSubjobStartTimes.remove(jobID);

            synchronized (fSubjobTable)
            {
                fSubjobTable.updateUI();
                STAXMonitorUtil.updateRowHeights(fSubjobTable, 6);
                STAXMonitorUtil.sizeColumnsToFitText(fSubjobTable);
            }
        }
    }

    public void addRow(Vector<Vector<String>> vector, String name, String value)
    {
        Vector<String> newRow = new Vector<String>(2);
        newRow.add(name);
        newRow.add(value);
        vector.add(newRow);
    }

    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == fSubjobStartMonitorMenuItem)
        {
            fSubjobPopupMenu.setVisible(false);

            synchronized(fSubjobModelSorter)
            {
                int rowIndex = fSubjobTable.getSelectedRow();
                String jobNumber = (String)fSubjobTable.getValueAt(rowIndex, 0);

                fMonitorFrame.getParentMonitor().
                    monitorExistingJob(jobNumber, -1);
            }
        }
        else if (e.getSource() == fSubjobShowJobLogMenuItem)
        {
            fSubjobPopupMenu.setVisible(false);

            synchronized(fSubjobModelSorter)
            {
                int rowIndex = fSubjobTable.getSelectedRow();
                String jobNumber =
                    (String)fSubjobTable.getValueAt(rowIndex, 0);

                String[] showLogParms = new String[10];
                showLogParms[0] = "-machine";
                showLogParms[1] = fStaxMachine;
                showLogParms[2] = "-machineNickname";
                showLogParms[3] = fStaxMachineNickname;
                showLogParms[4] = "-name";
                showLogParms[5] = fStaxServiceName.toUpperCase() +
                    "_Job_" + jobNumber;
                showLogParms[6] = "-fontName";
                showLogParms[7] = fLogViewerFontName;
                showLogParms[8] = "-saveAsDirectory";
                showLogParms[9] = fSaveAsDirectory;
                
                // Pass the STAX/JobMonitor/Controller handle to the log
                // viewer (since it won't be deleted if the Job Monitor
                // window is closed)

                STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                    this, fMonitorFrame.getParentMonitor().fHandle,
                    showLogParms);
            }
        }
        else if (e.getSource() == fSubjobShowJobUserLogMenuItem)
        {
            fSubjobPopupMenu.setVisible(false);

            synchronized(fSubjobModelSorter)
            {
                int rowIndex = fSubjobTable.getSelectedRow();
                String jobNumber =
                    (String)fSubjobTable.getValueAt(rowIndex, 0);

                String[] showLogParms = new String[10];
                showLogParms[0] = "-machine";
                showLogParms[1] = fStaxMachine;
                showLogParms[2] = "-machineNickname";
                showLogParms[3] = fStaxMachineNickname;
                showLogParms[4] = "-name";
                showLogParms[5] = fStaxServiceName.toUpperCase() +
                    "_Job_" + jobNumber  + "_User";
                showLogParms[6] = "-fontName";
                showLogParms[7] = fLogViewerFontName;
                showLogParms[8] = "-saveAsDirectory";
                showLogParms[9] = fSaveAsDirectory;

                // Pass the STAX/JobMonitor/Controller handle to the log
                // viewer (since it won't be deleted if the Job Monitor
                // window is closed)

                STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                    this, fMonitorFrame.getParentMonitor().fHandle,
                    showLogParms);
            }
        }
        else if (e.getSource() == fSubjobTerminateMenuItem)
        {
            fSubjobPopupMenu.setVisible(false);

            synchronized(fSubjobModelSorter)
            {
                int rowIndex = fSubjobTable.getSelectedRow();
                String jobNumber =
                    (String)fSubjobTable.getValueAt(rowIndex, 0);

                int confirmation = JOptionPane.showConfirmDialog(this,
                         "Are you certain that you want\n" +
                         "to terminate Job # " + jobNumber + " ?",
                         "Confirm Job Termination",
                         JOptionPane.YES_NO_OPTION,
                         JOptionPane.QUESTION_MESSAGE);

                if (!(confirmation == JOptionPane.YES_OPTION))
                {
                    return;
                }

                String request = "TERMINATE JOB " + jobNumber;

                STAFResult result = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, request);

                if (result.rc != 0)
                {
                    STAXMonitorUtil.showErrorDialog(
                        fMonitorFrame.getParentMonitor(),
                        "An error was encountered while " +
                        "attempting to terminate jobID " + jobNumber +
                        " rc=" + result.rc + " result=" + result.result);
                }
            }
        }
    }

    public void mouseClicked(MouseEvent e)
    {
        String subjobID = (String)
            fSubjobTable.getValueAt(fSubjobTable.getSelectedRow(), 0);

        String subjobName = (String)
            fSubjobTable.getValueAt(fSubjobTable.getSelectedRow(), 1);

        synchronized(fSubjobHashtable)
        {
            fMonitorFrame.updateCurrentSelection("Job " + subjobID +
                " - " + subjobName, (Vector)fSubjobHashtable.get("Job " +
                subjobID + " - " + subjobName));
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
            synchronized (fSubjobModelSorter)
            {
                int tableIndex =
                    fSubjobTable.rowAtPoint(new Point(e.getX(), e.getY()));

                if (tableIndex > -1)
                {
                    fSubjobTable.setRowSelectionInterval(tableIndex,
                                                         tableIndex);

                    String status =
                        (String)fSubjobTableModel.getValueAt(tableIndex, 3);

                    if (status.equals("Complete"))
                    {
                        fSubjobTerminateMenuItem.setEnabled(false);
                    }
                    else
                    {
                        fSubjobTerminateMenuItem.setEnabled(true);
                    }

                    fSubjobPopupMenu.show(e.getComponent(), e.getX(), e.getY());
                }
            }
        }
    }

    public void term()
    {
        boolean fContinueSubjobMonitor = false;

        STAXMonitorUtil.freeHandle(fHandle.getHandle());
    }

    class MonitorElapsedTime extends Thread
    {
        public void run()
        {
            final int waitTime = fMonitorFrame.getElapsedTimeInterval();

            if (waitTime == 0)
                return;

            while (fContinueElapsedTime)
            {
                synchronized(fSubjobTableModel)
                {
                    Runnable SubjobRunnable = new Runnable()
                    {
                        public void run()
                        {
                            Vector dataVector =
                                fSubjobTableModel.getDataVector();

                            for (int i = 0; i < dataVector.size(); i++)
                            {
                                Vector currentRow =
                                    (Vector)(dataVector.elementAt(i));

                                String jobID =
                                    (String)currentRow.elementAt(0);

                                final int row = i;

                                synchronized(fSubjobStartTimes)
                                {
                                    Calendar SubjobStarted =
                                        fSubjobStartTimes.get(jobID);

                                    if (SubjobStarted != null)
                                    {
                                        fSubjobTableModel.setValueAt(
                                            STAXMonitorUtil.getElapsedTime(
                                            SubjobStarted), row, 5);
                                    }
                                }
                            }

                            fSubjobTable.repaint();
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(SubjobRunnable);
                    }
                    catch (InterruptedException ex)
                    {
                         ex.printStackTrace();
                    }
                    catch (InvocationTargetException ex)
                    {
                         ex.printStackTrace();
                    }
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