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
import java.lang.reflect.*;
import java.io.*;

public class STAXMonitorTestcaseExtension implements STAXMonitorExtension,
                                                     ListSelectionListener
{
    // Column indexes in fTestcaseTable
    static final int NUM_ELEMENTS = 8;
    static final int NAME_INDEX = 0;
    static final int PASS_INDEX = 1;
    static final int FAIL_INDEX = 2;
    static final int STARTED_TIMESTAMP_INDEX = 3;
    static final int LAST_STATUS_TIMESTAMP_INDEX = 4;
    static final int DURATION_INDEX = 5;
    static final int STARTS_INDEX = 6;
    static final int INFO_INDEX = 7;

    JPanel fPanel;
    STAFHandle fHandle;
    String fStaxMachine;
    String fStaxServiceName;
    String fJobNumber;
    Vector<String> fTestcaseColumns;
    JTable fTestcaseTable;
    STAXMonitorTableModel fTestcaseTableModel;
    STAXMonitorTableSorter fTestcaseModelSorter;
    STAXMonitorFrame fMonitorFrame;
    String fTitle;
    boolean testcaseAutoResize = false;
    int numDisplayedColumns = 0;

    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName,
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;

        fTitle = "Testcase Info";

        fPanel = new JPanel();
        fPanel.setLayout(new BorderLayout());

        try
        {
            fHandle = STAXMonitorUtil.getNewSTAFHandle(
                "STAFMonitorTestcaseTableExtension");
        }
        catch (STAFException ex)
        {
        }

        fTestcaseColumns = new Vector<String>();
        fTestcaseColumns.addElement(" Name                       ");
        fTestcaseColumns.addElement(" PASS     ");
        fTestcaseColumns.addElement(" FAIL     ");
        fTestcaseColumns.addElement(" Start Date-Time");
        fTestcaseColumns.addElement(" Status Date-Time");
        fTestcaseColumns.addElement(" Duration ");
        fTestcaseColumns.addElement(" Starts");
        fTestcaseColumns.addElement(" Information ");

        fTestcaseTableModel = new STAXMonitorTableModel(fTestcaseColumns, 0);

        int sortColumn =
            monitorFrame.getParentMonitor().getDefaultTestcaseSortColumn();
        int sortOrder =
            monitorFrame.getParentMonitor().getDefaultTestcaseSortOrder();

        testcaseAutoResize =
            monitorFrame.getParentMonitor().getTestcaseAutoResize();

        boolean ascending = true;

        if (sortOrder == 0)
            ascending = true;
        else
            ascending = false;

        fTestcaseModelSorter = new STAXMonitorTableSorter(fTestcaseTableModel,
                                                          sortColumn,
                                                          ascending);

        fTestcaseTable = new JTable(fTestcaseModelSorter);
        fTestcaseModelSorter.addMouseListenerToHeaderInTable(
            fTestcaseTable, INFO_INDEX);

        fTestcaseTable.getColumnModel().getColumn(NAME_INDEX).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.blue));

        fTestcaseTable.getColumnModel().getColumn(NAME_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(PASS_INDEX).setCellRenderer(
            new STAXMonitorTableCellRenderer(new Color(0, 130, 0)));

        fTestcaseTable.getColumnModel().getColumn(PASS_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(FAIL_INDEX).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.red));

        fTestcaseTable.getColumnModel().getColumn(FAIL_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(STARTED_TIMESTAMP_INDEX).
            setCellRenderer(new STAXMonitorTableCellRenderer(Color.black,
            false, false));

        fTestcaseTable.getColumnModel().getColumn(STARTED_TIMESTAMP_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(LAST_STATUS_TIMESTAMP_INDEX).
            setCellRenderer(new STAXMonitorTableCellRenderer(Color.black,
            false, false));

        fTestcaseTable.getColumnModel().getColumn(LAST_STATUS_TIMESTAMP_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(DURATION_INDEX).
            setCellRenderer(new STAXMonitorTableCellRenderer(Color.black));

        fTestcaseTable.getColumnModel().getColumn(DURATION_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(STARTS_INDEX).
            setCellRenderer(new STAXMonitorTableCellRenderer(Color.black));

        fTestcaseTable.getColumnModel().getColumn(STARTS_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.getColumnModel().getColumn(INFO_INDEX).setCellRenderer(
            new STAXMonitorTableCellRenderer(Color.black));

        fTestcaseTable.getColumnModel().getColumn(INFO_INDEX).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(Color.black,
                true, new Font("Dialog", Font.BOLD, 12)));

        fTestcaseTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

        STAXMonitorUtil.sizeColumnsToFitText(fTestcaseTable);

        if (!newJob)
        {
            String request = "LIST JOB " + fJobNumber + " TESTCASES";
            STAFResult listResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, request);

            if (listResult.rc != 0)
            {
                throw new STAFException(
                    listResult.rc,
                    "Request=" + request +
                    "\nResult=" + listResult.result);
            }

            java.util.List tcList = (java.util.List)listResult.resultObj;
            Iterator iter = tcList.iterator();
            
            while (iter.hasNext())
            {
                Map tcMap = (Map)iter.next();
                Object rowData[] = new Object[NUM_ELEMENTS];

                String testcaseName = (String)tcMap.get("testcaseName");

                rowData[NAME_INDEX] = testcaseName;
                rowData[PASS_INDEX] = new Integer(
                    (String)tcMap.get("numPasses"));
                rowData[FAIL_INDEX] = new Integer(
                    (String)tcMap.get("numFails"));

                String elapsedTime = (String)tcMap.get("elapsedTime");

                if ((elapsedTime.equals("<Pending>")) ||
                    (elapsedTime.indexOf(":") != 1))
                {
                    rowData[DURATION_INDEX] = elapsedTime;
                }
                else
                {
                    rowData[DURATION_INDEX] = "??:??:??";
                }

                rowData[STARTS_INDEX] = new Integer(
                    (String)tcMap.get("numStarts"));

                rowData[INFO_INDEX] = (String)tcMap.get("information");

                // QUERY the testcase to retrieve startedTimestamp
                // and lastStatusTimestamp
                String queryRequest = "QUERY JOB " + fJobNumber +
                    " TESTCASE " + STAFUtil.wrapData(testcaseName);

                STAFResult queryResult = fHandle.submit2(
                    fStaxMachine, fStaxServiceName, queryRequest);

                if (queryResult.rc != 0)
                {
                    throw new STAFException(
                        queryResult.rc,
                        "Request=" + queryRequest +
                        "\nResult=" + queryResult.result);
                }

                java.util.Map queryTCMap =
                    (java.util.Map)queryResult.resultObj;

                rowData[STARTED_TIMESTAMP_INDEX] =
                    (String)queryTCMap.get("startedTimestamp");

                rowData[LAST_STATUS_TIMESTAMP_INDEX] =
                    (String)queryTCMap.get("lastStatusTimestamp");

                fTestcaseTableModel.addRow(rowData);
            }

            updateTestcaseTotals();
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseName()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(NAME_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcasePass()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(PASS_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseFail()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(FAIL_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseStartDateTime()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(STARTED_TIMESTAMP_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseStatusDateTime()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(LAST_STATUS_TIMESTAMP_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseDuration()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(DURATION_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseStarts()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(STARTS_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        if (!(monitorFrame.getParentMonitor().getDisplayTestcaseInformation()))
        {
            fTestcaseTable.removeColumn(
                fTestcaseTable.getColumnModel().getColumn(
                fTestcaseTable.convertColumnIndexToView(INFO_INDEX)));
        }
        else
        {
            numDisplayedColumns++;
        }

        int updateColumn =
            fTestcaseTable.convertColumnIndexToView(INFO_INDEX);

        if (updateColumn == -1)
        {
            updateColumn = 0;
        }

        if (numDisplayedColumns > 0)
        {
            STAXMonitorUtil.updateRowHeights(fTestcaseTable, updateColumn);
        }

        if (testcaseAutoResize)
        {
            STAXMonitorUtil.sizeColumnsToFitText(fTestcaseTable);
        }

        return fTestcaseTable;
    }

    public String getNotificationEventTypes()
    {
        return "testcasestatus testcase";
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
        return STAXMonitorFrame.EXTENSION_INFO;
    }

    public JComponent getComponent()
    {
        return fTestcaseTable;
    }

    public void valueChanged(ListSelectionEvent e)
    {

    }

    public void handleEvent(Map map)
    {
        String status = (String)map.get("status");

        String startedTimestamp = (String)map.get("startedTimestamp");

        if (startedTimestamp == null)
        {
            startedTimestamp = "";
        }

        String name = (String)map.get("name");

        String pass = (String)map.get("status-pass");

        if (pass.equals(""))
        {
            pass = "0";
        }

        String fail = (String)map.get("status-fail");

        if (fail.equals(""))
        {
            fail = "0";
        }

        String tcMessage = (String)map.get("message");

        if (tcMessage == null)
        {
            tcMessage = new String("");
        }

        String elapsedTime = (String)map.get("elapsed-time");

        String lastStatusTimestamp = (String)map.get("lastStatusTimestamp");

        if (lastStatusTimestamp == null)
        {
            lastStatusTimestamp = "";
        }

        String numStarts = (String)map.get("num-starts");

        if (numStarts == null)
        {
            numStarts = "1";
        }

        synchronized (fTestcaseTableModel)
        {
            Vector tcVector = fTestcaseTableModel.getDataVector();

            boolean found = false;
            int rowIndex = 0;
            for (int j = 0; j < tcVector.size(); j++)
            {
                if (!found)
                {
                    if (((Vector)(tcVector.elementAt(j))).
                        elementAt(NAME_INDEX).equals(name))
                    {
                        found = true;
                        rowIndex = j;
                    }
                }
            }

            if (found && tcMessage.equals(""))
            {
                // save existing message
                tcMessage = (String)((Vector)(tcVector.elementAt(rowIndex))).
                    elementAt(INFO_INDEX);
            }

            if (!found)
            {
                Object rowData[] = new Object[NUM_ELEMENTS];
                rowData[STARTED_TIMESTAMP_INDEX] = startedTimestamp;
                rowData[NAME_INDEX] = name;
                rowData[PASS_INDEX] = new Integer(pass);
                rowData[FAIL_INDEX] = new Integer(fail);
                rowData[DURATION_INDEX] = elapsedTime;
                rowData[LAST_STATUS_TIMESTAMP_INDEX] = lastStatusTimestamp;
                rowData[STARTS_INDEX] = new Integer(numStarts);
                rowData[INFO_INDEX] = tcMessage;
                fTestcaseTableModel.addRow(rowData);
            }
            else
            {
                // don't need to update the name

                // NOTE:  since the row has already been added to the table
                // we must call setValueAt on the fTestcaseModelSorter

                fTestcaseModelSorter.setValueAt(new Integer(pass),
                    fTestcaseModelSorter.map(rowIndex), PASS_INDEX);
                fTestcaseModelSorter.setValueAt(new Integer(fail),
                    fTestcaseModelSorter.map(rowIndex), FAIL_INDEX);

                if (lastStatusTimestamp != null)
                {
                    fTestcaseModelSorter.setValueAt(lastStatusTimestamp,
                        fTestcaseModelSorter.map(rowIndex),
                        LAST_STATUS_TIMESTAMP_INDEX);
                }

                if (elapsedTime != null)
                {
                    fTestcaseModelSorter.setValueAt(
                        elapsedTime, fTestcaseModelSorter.map(rowIndex),
                        DURATION_INDEX);
                }

                if (numStarts != null)
                {
                    fTestcaseModelSorter.setValueAt(new Integer(numStarts),
                        fTestcaseModelSorter.map(rowIndex), STARTS_INDEX);
                }

                if (!tcMessage.equals(""))
                {
                    fTestcaseModelSorter.setValueAt(tcMessage,
                        fTestcaseModelSorter.map(rowIndex), INFO_INDEX);
                }
            }

            fTestcaseModelSorter.sortTable();
            updateTestcaseTotals();

            int updateColumn =
                fTestcaseTable.convertColumnIndexToView(INFO_INDEX);

            if (updateColumn == -1)
            {
                updateColumn = 0;
            }

            if (numDisplayedColumns > 0)
            {
                STAXMonitorUtil.updateRowHeights(fTestcaseTable, updateColumn);
            }

            if (testcaseAutoResize)
            {
                STAXMonitorUtil.sizeColumnsToFitText(fTestcaseTable);
            }

            fTestcaseTable.getTableHeader().repaint();
        }
    }

    public void updateTestcaseTotals()
    {
        synchronized (fTestcaseTableModel)
        {
            Vector testcaseData = fTestcaseTableModel.getDataVector();

            int passTotal = 0;
            int failTotal = 0;

            for (int j = 0; j < testcaseData.size(); j++)
            {
                Vector row = (Vector)(testcaseData.elementAt(j));

                passTotal += ((Integer)row.elementAt(PASS_INDEX)).intValue();
                failTotal += ((Integer)row.elementAt(FAIL_INDEX)).intValue();
            }

            int passViewIndex =
                fTestcaseTable.convertColumnIndexToView(PASS_INDEX);
            int failViewIndex =
                fTestcaseTable.convertColumnIndexToView(FAIL_INDEX);

            if (passViewIndex != -1)
            {
                fTestcaseTable.getColumnModel().getColumn(passViewIndex).
                    setHeaderValue(" PASS: " + passTotal + " ");
            }

            if (failViewIndex != -1)
            {
                fTestcaseTable.getColumnModel().getColumn(failViewIndex).
                    setHeaderValue(" FAIL: " + failTotal + " ");
            }
        }
    }

    public void addRow(Vector<Vector<Object>> vector,
                       String name, String value)
    {
        Vector<Object> newRow = new Vector<Object>(2);
        newRow.add(name);
        newRow.add(value);
        vector.add(newRow);
    }

}