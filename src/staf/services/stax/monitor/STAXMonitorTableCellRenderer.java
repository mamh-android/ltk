/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.awt.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.border.*;
import java.util.*;
import java.io.*;

public class STAXMonitorTableCellRenderer
             extends JTextArea implements TableCellRenderer    
{
    public java.awt.Color foregroundColor;    
    public boolean header = false;
    public Hashtable rowHeights = new Hashtable();
    public static String fontName = "Dialog";  // Default font
    public Font tableFont = new Font(fontName, Font.PLAIN, 12);    
    public JTable theTable;
    public boolean sizeUpdated = false;
    public boolean reverseTimestampOrder = false;

    private static java.net.URL ascendingURL =
        ClassLoader.getSystemClassLoader().
            getSystemResource("images/ascending.gif");

    private static ImageIcon ascendingIcon = new ImageIcon(ascendingURL);

    private static java.net.URL descendingURL =
        ClassLoader.getSystemClassLoader().
            getSystemResource("images/descending.gif");

    private static ImageIcon descendingIcon = new ImageIcon(descendingURL);

    public STAXMonitorTableCellRenderer()
    {
        super();
    }

    public STAXMonitorTableCellRenderer(java.awt.Color foreground)
    {
        this(foreground, false);
    }    
    
    public STAXMonitorTableCellRenderer(java.awt.Color foreground,                                        
                                        boolean header)
    {        
        this(foreground, header, new Font(fontName, Font.PLAIN, 12));                   
    }
    
    public STAXMonitorTableCellRenderer(java.awt.Color foreground,                                        
                                        boolean header,
                                        boolean reverseTimestampOrder)
    {        
        this(foreground, header, new Font(fontName, Font.PLAIN, 12),
             reverseTimestampOrder);                   
    }
    
    public STAXMonitorTableCellRenderer(java.awt.Color foreground,                                        
                                        boolean header,
                                        Font font)
    {
        this(foreground, header, font, false);
    }
    
    public STAXMonitorTableCellRenderer(java.awt.Color foreground,                                        
                                        boolean header,
                                        Font font,
                                        boolean reverseTimestampOrder)
    {
        setOpaque(true);   
        setFont(font);                
        
        setBorder(BorderFactory.createRaisedBevelBorder());

        foregroundColor = foreground;
        this.header = header;
        this.reverseTimestampOrder = reverseTimestampOrder;
        
        if (header)
        {
            setBackground(Color.lightGray);
        }
        else
        {
            setBackground(Color.white);
        }
    }

    public void clearRowHeights()
    {
        rowHeights.clear();
    }
    
    public Component getTableCellRendererComponent(JTable table, Object value,
                                                   boolean isSelected, 
                                                   boolean hasFocus,
                                                   int row, int col)
    {     
        theTable = table;
        
        if (value == null)
        {
            setText("");
        }
        else
        {
            String cellValue = String.valueOf(value);

            if (reverseTimestampOrder)
            {
                cellValue = cellValue.substring(9, 17) + "-" +
                    cellValue.substring(0, 8);
            }

            setText(cellValue);
        }
        
        if (!header)
        {
            if (isSelected)
            {
                setBackground(UIManager.getColor("List.selectionBackground"));
            }
            else
            {
                setBackground(Color.white);
            }
        }
        else
        {
            // is this column being sorted?
            int sortColumn =
                ((STAXMonitorTableSorter)table.getModel()).getSortColumn();
            boolean isAscending =
                ((STAXMonitorTableSorter)table.getModel()).isAscending();
            String sortColumnHeader =
                ((STAXMonitorTableSorter)table.getModel()).getSortColumnHeader();

            String columnHeader = this.getText().trim();

            if (columnHeader.startsWith("PASS:"))
                columnHeader = "PASS";
            else if (columnHeader.startsWith("FAIL:"))
                columnHeader = "FAIL";

            if (columnHeader.equals(sortColumnHeader.trim()))
            {
                JPanel sortedHeader = new JPanel();
                sortedHeader.setBorder(BorderFactory.createRaisedBevelBorder());
                sortedHeader.setLayout(new
                    BoxLayout(sortedHeader, BoxLayout.X_AXIS));
                sortedHeader.setBackground(Color.lightGray);
                this.setBorder(null);
                sortedHeader.add(this);

                if (isAscending)
                {
                    sortedHeader.add(new JLabel(ascendingIcon));
                }
                else
                {
                    sortedHeader.add(new JLabel(descendingIcon));
                }

                return sortedHeader;
            }

            setBorder(BorderFactory.createRaisedBevelBorder());
        }

        return this;
    }
    
    public Color getForeground()
    {
        return foregroundColor;
    }  
}