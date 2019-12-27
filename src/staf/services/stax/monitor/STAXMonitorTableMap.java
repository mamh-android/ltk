/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import javax.swing.table.*; 
import javax.swing.event.TableModelListener; 
import javax.swing.event.TableModelEvent; 

public class STAXMonitorTableMap extends DefaultTableModel
                                 implements TableModelListener 
{
    protected STAXMonitorTableModel model; 

    public STAXMonitorTableModel getModel() 
    {
        return model;
    }

    public void setModel(STAXMonitorTableModel model) 
    {
        this.model = model; 
        model.addTableModelListener(this); 
    }   

    public Object getValueAt(int aRow, int aColumn) 
    {
        return model.getValueAt(aRow, aColumn); 
    }
        
    public void setValueAt(Object aValue, int aRow, int aColumn) 
    {
        model.setValueAt(aValue, aRow, aColumn); 
    }

    public int getRowCount() 
    {
        return (model == null) ? 0 : model.getRowCount(); 
    }

    public int getColumnCount() 
    {
        return (model == null) ? 0 : model.getColumnCount(); 
    }
        
    public String getColumnName(int aColumn) 
    {
        return model.getColumnName(aColumn); 
    }

    public Class getColumnClass(int aColumn) 
    {
        return model.getColumnClass(aColumn); 
    }
        
    public boolean isCellEditable(int row, int column) 
    { 
         return false;
    }
    
    public void tableChanged(TableModelEvent e) 
    {
        fireTableChanged(e);        
    }
}