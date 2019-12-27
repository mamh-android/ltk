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

public class STAXMonitorProcessExtension implements STAXMonitorExtension,
                                                      MouseListener
{
    JPanel fPanel;
    STAFHandle fHandle;
    JTable fProcessTable;
    STAXMonitorTableModel fProcessTableModel;
    STAXMonitorTableSorter fProcessModelSorter;
    Vector<String> fProcessColumns;
    boolean fContinueProcessMonitor = true;
    STAFProcessMonitor fProcessMonitor;
    Hashtable<String, Calendar> fProcessStartTimes =
        new Hashtable<String, Calendar>();
    String fStaxMachine;
    String fStaxServiceName;
    String fJobNumber;
    MonitorElapsedTime fElapsedTime;
    boolean fContinueElapsedTime = true;
    STAXMonitorFrame fMonitorFrame;
    String fTitle;
    Hashtable<String, Vector> fProcessHashtable =
        new Hashtable<String, Vector>();

    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName,
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;

        fTitle = "Processes";

        fPanel = new JPanel();
        fPanel.setLayout(new BorderLayout());

        try
        {
            fHandle = STAXMonitorUtil.getNewSTAFHandle(
                "STAX/JobMonitor/Extension/ProcessTable");
        }
        catch (STAFException ex)
        {
        }

        fProcessColumns = new Vector<String>();
        fProcessColumns.addElement("Process Name");
        fProcessColumns.addElement("Elapsed Time");
        fProcessColumns.addElement("Status");
        fProcessColumns.addElement("Block");
        fProcessColumns.addElement("Machine:Handle");
        fProcessColumns.addElement("Command");

        fProcessTableModel = new STAXMonitorTableModel(fProcessColumns, 0);

        fProcessModelSorter =
            new STAXMonitorTableSorter(fProcessTableModel, 0);

        fProcessTable = new JTable(fProcessModelSorter);
        fProcessModelSorter.addMouseListenerToHeaderInTable(
            fProcessTable, 3);
        fProcessTable.setRowSelectionAllowed(true);
        fProcessTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        fProcessTable.addMouseListener(this);

        fProcessTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fProcessTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fProcessTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.blue));

        fProcessTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fProcessTable.getColumnModel().getColumn(2).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.blue));

        fProcessTable.getColumnModel().getColumn(2).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fProcessTable.getColumnModel().getColumn(3).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fProcessTable.getColumnModel().getColumn(3).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fProcessTable.getColumnModel().getColumn(4).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));


        fProcessTable.getColumnModel().getColumn(4).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fProcessTable.getColumnModel().getColumn(5).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fProcessTable.getColumnModel().getColumn(5).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fPanel.add(BorderLayout.CENTER, new JScrollPane(fProcessTable));

        fProcessTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

        STAXMonitorUtil.sizeColumnsToFitText(fProcessTable);

        if (!newJob)
        {
            // Determine the processes that are currently running in the job

            String request = "LIST JOB " + fJobNumber + " PROCESSES";
            STAFResult listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, request);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Request=" + request +
                    "\nResult=" + listResult.result);
            }

            java.util.List processList = (java.util.List)listResult.resultObj;
            Iterator iter = processList.iterator();
            
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
                    continue;
                }

                processMap = (HashMap)queryResult.resultObj;

                String block = (String)processMap.get("blockName");
                String processName = (String)processMap.get("processName");
                String command = (String)processMap.get("command");

                String processID = location + ":" + handleNumber;
                String machineHandleText = location + ":";
                machineHandleText += handleNumber;

                String startTimestamp = 
                    (String)processMap.get("startTimestamp");
                String startDate = startTimestamp.substring(0, 8);
                String startTime = startTimestamp.substring(9);

                synchronized(fProcessStartTimes)
                {
                    fProcessStartTimes.put(
                        processID,
                        STAXMonitorUtil.getCalendar2(startDate, startTime));
                }
                
                Object rowData[] = new Object[6];
                rowData[0] = processName;
                rowData[1] = "";
                rowData[2] = "";
                rowData[3] = block;
                rowData[4] = machineHandleText;
                rowData[5] = command;

                fProcessTableModel.addRow(rowData);

                Vector<Vector<String>> procDataVector =
                    new Vector<Vector<String>>();
                STAXMonitorUtil.assignProcessInfo(processMap, procDataVector);

                synchronized(fProcessHashtable)
                {
                    fProcessHashtable.put(processID, procDataVector);
                }

                STAXMonitorUtil.updateRowHeights(fProcessTable, 1);
                STAXMonitorUtil.sizeColumnsToFitText(fProcessTable);
            }
        }

        fProcessMonitor = new STAFProcessMonitor();
        fProcessMonitor.start();

        fElapsedTime = new MonitorElapsedTime();
        fElapsedTime.start();

        return fProcessTable;
    }

    public String getNotificationEventTypes()
    {
        return "process";
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
        return fProcessTable;
    }

    public void mouseClicked(MouseEvent e)
    {
        int selectedRow = fProcessTable.getSelectedRow();

        if (selectedRow == -1)
            return;

        String processID = (String)
            fProcessTable.getValueAt(selectedRow, 4);

        synchronized(fProcessHashtable)
        {
            fMonitorFrame.updateCurrentSelection(
                (String)fProcessTable.getValueAt(selectedRow, 0),
                fProcessHashtable.get(processID));
        }
    }

    public void mousePressed(MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}

    public void handleEvent(Map map)
    {
        String block = (String)map.get("block");
        String processName = (String)map.get("name");
        String location = (String)map.get("location");
        String machineHandleText = location + ":";
        String handleNumber = (String)map.get("handle");

        machineHandleText += handleNumber;

        final String processID = location + ":" + handleNumber;
        String command = (String)map.get("command");
        String status = (String)map.get("status");

        if (status.equals("start"))
        {
            String parms = (String)map.get("parms");

            Object rowData[] = new Object[6];
            rowData[0] = processName;
            rowData[1] = " ";
            rowData[2] = " ";
            rowData[3] = block;
            rowData[4] = machineHandleText;
            rowData[5] = command;

            fProcessTableModel.addRow(rowData);

            STAXMonitorUtil.updateRowHeights(fProcessTable, 1);
            STAXMonitorUtil.sizeColumnsToFitText(fProcessTable);

            // Get process start timestamp because it's not provided in
            // the event property map.

            STAFResult queryResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName,
                "QUERY JOB " + fJobNumber + " PROCESS " +
                location + ":" + handleNumber);

            if (queryResult.rc != 0)
            {
                // Process has already completed, so asynchronously remove it

                Runnable runnable = new Runnable()
                {
                    public void run()
                    {
                        Vector dataVector = fProcessTableModel.getDataVector();

                        for (int i = 0; i < dataVector.size(); i++)
                        {
                            Vector currentRow = (Vector)(dataVector.elementAt(i));

                            synchronized(fProcessHashtable)
                            {
                                fProcessHashtable.remove(processID);
                            }

                            synchronized(fProcessStartTimes)
                            {
                                fProcessStartTimes.remove(processID);
                            }

                            if (((String)currentRow.elementAt(3)).
                                equalsIgnoreCase(processID))
                            {
                                fProcessTableModel.removeRow(i);
                                STAXMonitorUtil.updateRowHeights(fProcessTable, 1);
                                STAXMonitorUtil.sizeColumnsToFitText(fProcessTable);
                                break;
                            }
                        }
                    }
                };
                SwingUtilities.invokeLater(runnable);

                return;
            }
            
            Map processMap = (HashMap)queryResult.resultObj;

            String startTimestamp = (String)processMap.get("startTimestamp");

            // startTimestamp format is YYYYMMDD-HH:MM:SS
            String startDate = startTimestamp.substring(0, 8);
            String startTime = startTimestamp.substring(9);
                
            synchronized(fProcessStartTimes)
            {
                fProcessStartTimes.put(
                    processID,
                    STAXMonitorUtil.getCalendar2(startDate, startTime));
            }

            Vector<Vector<String>> procDataVector =
                new Vector<Vector<String>>();
            STAXMonitorUtil.assignProcessInfo(processMap, procDataVector);

            synchronized(fProcessHashtable)
            {
                fProcessHashtable.put(processID, procDataVector);
            }
        }
        else if (status.equals("stop"))
        {
            Vector dataVector = fProcessTableModel.getDataVector();

            for (int i = 0; i < dataVector.size(); i++)
            {
                Vector currentRow = (Vector)(dataVector.elementAt(i));

                synchronized(fProcessHashtable)
                {
                    fProcessHashtable.remove(machineHandleText);
                }

                synchronized(fProcessStartTimes)
                {
                    fProcessStartTimes.remove(machineHandleText);
                }

                if (((String)currentRow.elementAt(4)).
                    equalsIgnoreCase(machineHandleText))
                {
                    fProcessTableModel.removeRow(i);
                    STAXMonitorUtil.updateRowHeights(fProcessTable, 1);
                    STAXMonitorUtil.sizeColumnsToFitText(fProcessTable);
                    break;
                }
            }
        }
    }

    public void term()
    {
        boolean fContinueProcessMonitor = false;

        STAXMonitorUtil.freeHandle(fHandle.getHandle());
    }

    class STAFProcessMonitor extends Thread
    {
        public void run()
        {
            final int frequency =  fMonitorFrame.getProcessMonitorInterval();

            if (frequency <= 0)
                return;

            while (fContinueProcessMonitor)
            {
                int waitTime = frequency;
                Runnable runnable;


                final Vector dataVector = fProcessTableModel.getDataVector();


                for (int i = 0; i < dataVector.size(); i++)
                {
                    Vector currentRow = (Vector)(dataVector.elementAt(i));

                    String locationAndHandle = (String)currentRow.elementAt(4);
                    final String location = locationAndHandle.substring(
                        0, locationAndHandle.lastIndexOf(":"));
                    final String handle = locationAndHandle.substring(
                        locationAndHandle.lastIndexOf(":") + 1);

                    final int row = i;

                    String message = STAXMonitorUtil.
                        getMonitorMessage(location, handle);

                    final String processMessage = message;

                    runnable = new Runnable()
                    {
                        public void run()
                        {
                            try
                            {
                                fProcessTableModel.setValueAt(processMessage,
                                    row, 2);
                                STAXMonitorUtil.sizeColumnsToFitText(
                                    fProcessTable);
                                fProcessTable.repaint();
                            }
                            catch(java.lang.ArrayIndexOutOfBoundsException e)
                            {
                                // do nothing
                            }
                        }
                    };

                    SwingUtilities.invokeLater(runnable);
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

    class MonitorElapsedTime extends Thread
    {
        public void run()
        {
            final int waitTime = fMonitorFrame.getElapsedTimeInterval();

            if (waitTime == 0)
                return;

            while (fContinueElapsedTime)
            {
                synchronized(fProcessTableModel)
                {
                    Runnable processRunnable = new Runnable()
                    {
                        public void run()
                        {
                            Vector dataVector =
                                fProcessTableModel.getDataVector();

                            for (int i = 0; i < dataVector.size(); i++)
                            {
                                Vector currentRow =
                                    (Vector)(dataVector.elementAt(i));

                                String locationAndHandle =
                                    (String)currentRow.elementAt(4);

                                final int row = i;

                                synchronized(fProcessStartTimes)
                                {
                                    Calendar processStarted = 
                                        fProcessStartTimes.get(
                                            locationAndHandle);

                                    {
                                        fProcessTableModel.setValueAt(
                                            STAXMonitorUtil.getElapsedTime(
                                            processStarted), row, 1);
                                    }
                                }
                            }

                            fProcessTable.repaint();
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