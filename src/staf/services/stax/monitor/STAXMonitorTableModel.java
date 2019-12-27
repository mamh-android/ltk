/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import javax.swing.*;
import javax.swing.table.*;
import java.util.*;

public class STAXMonitorTableModel
      extends javax.swing.table.DefaultTableModel
{    
    public STAXMonitorTableModel()
    {
        super();
    }

    public STAXMonitorTableModel(java.lang.Object[][] data,
                                         java.lang.Object[] columnNames)
    {
        super(data, columnNames);
    }

    public STAXMonitorTableModel(java.lang.Object[] columnNames,
                                         int numRows)
    {
        super(columnNames, numRows);
    }

    public STAXMonitorTableModel(int numRows, int numColumns)
    {
        super(numRows, numColumns);
    }

    public STAXMonitorTableModel(java.util.Vector columnNames, 
                                        int numRows)
    {
        super(columnNames, numRows);
    }

    public STAXMonitorTableModel(java.util.Vector data,
                                         java.util.Vector columnNames)
    {
        super(data, columnNames);
    }
    
    public Class getColumnClass(int col)
    {
        if (dataVector.isEmpty())
        {
            return (new Object()).getClass();
        }
        else
        {
            Vector v = (Vector)dataVector.elementAt(0);
            return v.elementAt(col).getClass();
        }
    }

    public boolean isCellEditable(int row, int column)
    {        
        return false;
    }
}