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

public class STAXMonitorSTAFCmdExtension implements STAXMonitorExtension,
                                                      MouseListener
{
    JPanel fPanel;
    STAFHandle fHandle;
    JTable fSTAFCmdTable;
    STAXMonitorTableModel fSTAFCmdTableModel;
    STAXMonitorTableSorter fSTAFCmdModelSorter;
    Vector<String> fSTAFCmdColumns;
    Hashtable<String, Calendar> fSTAFCmdStartTimes =
        new Hashtable<String, Calendar>();
    String fStaxMachine;
    String fStaxServiceName;
    String fJobNumber;
    MonitorElapsedTime fElapsedTime;
    boolean fContinueElapsedTime = true;
    STAXMonitorFrame fMonitorFrame;
    String fTitle;
    Hashtable<String, Vector> fCmdHashtable = new Hashtable<String, Vector>();

    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName,
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;

        fTitle = "STAFCmds";

        fPanel = new JPanel();
        fPanel.setLayout(new BorderLayout());

        try
        {
            fHandle = STAXMonitorUtil.getNewSTAFHandle(
                "STAX/JobMonitor/Extension/STAFCmdTable");
        }
        catch (STAFException ex)
        {
        }

        fSTAFCmdColumns = new Vector<String>();
        fSTAFCmdColumns.add("Command Name");
        fSTAFCmdColumns.add("Elapsed Time");
        fSTAFCmdColumns.add("Block");
        fSTAFCmdColumns.add("Machine:Request#");
        fSTAFCmdColumns.add("Service");
        fSTAFCmdColumns.add("Request");

        fSTAFCmdTableModel = new STAXMonitorTableModel(fSTAFCmdColumns, 0);

        fSTAFCmdModelSorter =
            new STAXMonitorTableSorter(fSTAFCmdTableModel, 0);

        fSTAFCmdTable = new JTable(fSTAFCmdModelSorter);
        fSTAFCmdModelSorter.addMouseListenerToHeaderInTable(
            fSTAFCmdTable, 3);
        fSTAFCmdTable.setRowSelectionAllowed(true);
        fSTAFCmdTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        fSTAFCmdTable.addMouseListener(this);

        fSTAFCmdTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSTAFCmdTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSTAFCmdTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.blue));

        fSTAFCmdTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSTAFCmdTable.getColumnModel().getColumn(2).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSTAFCmdTable.getColumnModel().getColumn(2).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSTAFCmdTable.getColumnModel().getColumn(3).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSTAFCmdTable.getColumnModel().getColumn(3).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSTAFCmdTable.getColumnModel().getColumn(4).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSTAFCmdTable.getColumnModel().getColumn(4).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSTAFCmdTable.getColumnModel().getColumn(5).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fSTAFCmdTable.getColumnModel().getColumn(5).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fSTAFCmdTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

        STAXMonitorUtil.sizeColumnsToFitText(fSTAFCmdTable);

        if (!newJob)
        {
            String listRequest = "LIST JOB " + fJobNumber + " STAFCMDS";
            STAFResult listResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName, listRequest);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Request=" + listRequest +
                    "\nResult=" + listResult.result);
            }

            java.util.List stafcmdList = (java.util.List)listResult.resultObj;
            Iterator iter = stafcmdList.iterator();
            
            while (iter.hasNext())
            {
                Map stafcmdMap = (Map)iter.next();
                String commandName   = (String)stafcmdMap.get("stafcmdName");
                String location      = (String)stafcmdMap.get("location");
                String requestNumber = (String)stafcmdMap.get("requestNum");
                String service       = (String)stafcmdMap.get("service");
                String request       = (String)stafcmdMap.get("request");

                STAFResult queryResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName,
                    "QUERY JOB " + fJobNumber + " STAFCMD " + requestNumber);

                if (queryResult.rc != 0)
                {
                    // Ignore as Stafcmd could have already completed
                    continue;
                }

                stafcmdMap = (HashMap)queryResult.resultObj;

                String block = (String)stafcmdMap.get("blockName");
                String stafcmdName = (String)stafcmdMap.get("stafcmdName");

                String startTimestamp = 
                    (String)stafcmdMap.get("startTimestamp");
                String startDate = startTimestamp.substring(0, 8);
                String startTime = startTimestamp.substring(9);

                String STAFCmdID = location + ":" + requestNumber;

                synchronized(fSTAFCmdStartTimes)
                {
                    fSTAFCmdStartTimes.put(
                        STAFCmdID,
                        STAXMonitorUtil.getCalendar2(startDate, startTime));
                }

                Object rowData[] = new Object[6];
                rowData[0] = commandName;
                rowData[1] = "";
                rowData[2] = block;
                rowData[3] = STAFCmdID;
                rowData[4] = service;
                rowData[5] = request;

                fSTAFCmdTableModel.addRow(rowData);

                STAXMonitorUtil.updateRowHeights(fSTAFCmdTable, 1);
                STAXMonitorUtil.sizeColumnsToFitText(fSTAFCmdTable);

                Vector<Vector<String>> cmdDataVector =
                    new Vector<Vector<String>>();

                addRow(cmdDataVector, "Location", location);
                addRow(cmdDataVector, "Request Number", requestNumber);
                addRow(cmdDataVector, "Service", service);
                addRow(cmdDataVector, "Request", request);
                addRow(cmdDataVector, "Started", startDate + "-" + startTime);

                synchronized(fCmdHashtable)
                {
                    fCmdHashtable.put(STAFCmdID, cmdDataVector);
                }
            }
        }

        fElapsedTime = new MonitorElapsedTime();
        fElapsedTime.start();

        //return fPanel;
        return fSTAFCmdTable;
    }

    public String getNotificationEventTypes()
    {
        return "stafcommand";
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
        return fSTAFCmdTable;
    }
    
    public void mouseClicked(MouseEvent e)
    {
        int selectedRow = fSTAFCmdTable.getSelectedRow();

        if (selectedRow == -1)
            return;

        String cmdID = (String)
            fSTAFCmdTable.getValueAt(selectedRow, 3);

        synchronized(fCmdHashtable)
        {
            fMonitorFrame.updateCurrentSelection((String)fSTAFCmdTable.
                getValueAt(selectedRow, 0),
                (Vector)fCmdHashtable.get(cmdID));
        }
    }

    public void mousePressed(MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}

    public void handleEvent(Map map)
    {
        String block = (String)map.get("block");

        String commandName = (String)map.get("name");

        String location = (String)map.get("location");

        String requestNumber = (String)map.get("requestNumber");

        final String STAFCmdID = location + ":" + requestNumber;

        String service = (String)map.get("service");
        String request = (String)map.get("request");
        String status = (String)map.get("status");

        if (status == null)
        {
            return;  // Ignore event
        }

        if (status.equals("start"))
        {
            Object rowData[] = new Object[6];
            rowData[0] = commandName;
            rowData[1] = "";
            rowData[2] = block;
            rowData[3] = STAFCmdID;
            rowData[4] = service;
            rowData[5] = request;

            fSTAFCmdTableModel.addRow(rowData);

            STAXMonitorUtil.updateRowHeights(fSTAFCmdTable, 1);
            STAXMonitorUtil.sizeColumnsToFitText(fSTAFCmdTable);

            Vector<Vector<String>> cmdDataVector =
                new Vector<Vector<String>>();

            addRow(cmdDataVector, "Location", location);
            addRow(cmdDataVector, "Request Number", requestNumber);
            addRow(cmdDataVector, "Service", service);
            addRow(cmdDataVector, "Request", request);

            STAFResult queryResult = fHandle.submit2(
                fStaxMachine, fStaxServiceName,
                "QUERY JOB " + fJobNumber + " STAFCMD " + requestNumber);

            if (queryResult.rc != 0)
            {
                // Stafcmd has already completed, so asynchronously remove it

                Runnable runnable = new Runnable()
                {
                    public void run()
                    {
                        Vector dataVector = fSTAFCmdTableModel.getDataVector();

                        for (int i = 0; i < dataVector.size(); i++)
                        {
                            Vector currentRow = (Vector)(dataVector.elementAt(i));

                            synchronized(fCmdHashtable)
                            {
                                fCmdHashtable.remove(STAFCmdID);
                            }

                            synchronized(fSTAFCmdStartTimes)
                            {
                                fSTAFCmdStartTimes.remove(STAFCmdID);
                            }

                            if (((String)currentRow.elementAt(3)).
                                equalsIgnoreCase(STAFCmdID))
                            {
                                fSTAFCmdTableModel.removeRow(i);
                                STAXMonitorUtil.updateRowHeights(fSTAFCmdTable, 1);
                                STAXMonitorUtil.sizeColumnsToFitText(fSTAFCmdTable);
                                break;
                            }
                        }
                    }
                };
                SwingUtilities.invokeLater(runnable);

                return;
            }
            
            Map processMap = (HashMap)queryResult.resultObj;

            // startTimestamp format is YYYYMMDD-HH:MM:SS
            String startTimestamp = (String)processMap.get("startTimestamp");
            String startDate = startTimestamp.substring(0, 8);
            String startTime = startTimestamp.substring(9);

            synchronized(fSTAFCmdStartTimes)
            {
                fSTAFCmdStartTimes.put(
                    STAFCmdID,
                    STAXMonitorUtil.getCalendar2(startDate, startTime));
            }

            addRow(cmdDataVector, "Started", startDate + "-" + startTime);

            synchronized(fCmdHashtable)
            {
                fCmdHashtable.put(STAFCmdID, cmdDataVector);
            }
        }
        else if (status.equals("stop"))
        {
            Vector dataVector = fSTAFCmdTableModel.getDataVector();

            for (int i = 0; i < dataVector.size(); i++)
            {
                Vector currentRow = (Vector)(dataVector.elementAt(i));

                synchronized(fCmdHashtable)
                {
                    fCmdHashtable.remove(STAFCmdID);
                }

                synchronized(fSTAFCmdStartTimes)
                {
                    fSTAFCmdStartTimes.remove(STAFCmdID);
                }

                if (((String)currentRow.elementAt(3)).
                    equalsIgnoreCase(STAFCmdID))
                {
                    fSTAFCmdTableModel.removeRow(i);
                    STAXMonitorUtil.updateRowHeights(fSTAFCmdTable, 1);
                    STAXMonitorUtil.sizeColumnsToFitText(fSTAFCmdTable);
                    break;
                }
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

    public void term()
    {
        boolean fContinueSTAFCmdMonitor = false;

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
                synchronized(fSTAFCmdTableModel)
                {
                    Runnable STAFCmdRunnable = new Runnable()
                    {
                        public void run()
                        {
                            Vector dataVector =
                                fSTAFCmdTableModel.getDataVector();

                            for (int i = 0; i < dataVector.size(); i++)
                            {
                                Vector currentRow =
                                    (Vector)(dataVector.elementAt(i));

                                String locationAndHandle =
                                    (String)currentRow.elementAt(3);

                                final int row = i;

                                synchronized(fSTAFCmdStartTimes)
                                {
                                    Calendar STAFCmdStarted =
                                        fSTAFCmdStartTimes.get(
                                            locationAndHandle);

                                    if (STAFCmdStarted != null)
                                    {
                                        fSTAFCmdTableModel.setValueAt(
                                            STAXMonitorUtil.getElapsedTime(
                                            STAFCmdStarted), row, 1);
                                    }
                                }
                            }

                            fSTAFCmdTable.repaint();
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(STAFCmdRunnable);
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