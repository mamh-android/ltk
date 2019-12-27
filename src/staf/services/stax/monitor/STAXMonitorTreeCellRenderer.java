/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.border.*;
import java.util.*;

public class STAXMonitorTreeCellRenderer extends JPanel 
                                         implements TreeCellRenderer
                                                       
{
    private JLabel nodeLabel;
    private JLabel elapsedTimeLabel;
    private JLabel processMonitorLabel;
    private boolean fShowNoMonitorInfo;

    private static java.net.URL greenblkURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/greenblk.gif");
        
    private static java.net.URL redblkURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/redblk.gif");
        
    private static java.net.URL yellwblkURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/yellwblk.gif");
        
    private static java.net.URL processURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/process.gif");
        
    private static java.net.URL commandURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/command.gif");
        
    private static java.net.URL subjobURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/subjob.gif");

    private ImageIcon runningBlockIcon = new ImageIcon(greenblkURL);    
    private ImageIcon heldBlockIcon = new ImageIcon(redblkURL);
    private ImageIcon parentHeldBlockIcon = new ImageIcon(yellwblkURL);
    private ImageIcon processIcon = new ImageIcon(processURL);
    private ImageIcon commandIcon = new ImageIcon(commandURL);
    private ImageIcon subjobIcon = new ImageIcon(subjobURL);
        
    private Font treeFont = new Font("Dialog", Font.PLAIN, 12);
    private Color processMonitorTextColor = new Color(6, 91, 183);
    private Color noProcessMonitorTextColor = new Color(147, 116, 74);
    private FontMetrics metrics;
    public Color selectionColor;
    
    private JPanel panel = new JPanel();
    
    private String kNoProcessMonitorText;

    public STAXMonitorTreeCellRenderer()
    {
        this(false);
    }

    public STAXMonitorTreeCellRenderer(boolean showNoMonitorInfo)
    {
        super();
        setBackground(Color.white);
        panel.setBackground(Color.white);
        nodeLabel = new JLabel();
        processMonitorLabel = new JLabel();
        elapsedTimeLabel = new JLabel();
        nodeLabel.setFont(treeFont);
        nodeLabel.setOpaque(true);
        processMonitorLabel.setFont(treeFont);
        processMonitorLabel.setOpaque(true);
        elapsedTimeLabel.setFont(treeFont);
        elapsedTimeLabel.setOpaque(true);
        elapsedTimeLabel.setBackground(Color.white);
        processMonitorLabel.setForeground(processMonitorTextColor);
        nodeLabel.setForeground(Color.black);
        nodeLabel.setBackground(Color.white);
        processMonitorLabel.setForeground(processMonitorTextColor);
        processMonitorLabel.setBackground(Color.white);
        setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        
        selectionColor = UIManager.getColor("Tree.selectionBackground");
        
        add(nodeLabel);
        add(elapsedTimeLabel);
        add(processMonitorLabel);
        add(panel);
        
        if (!showNoMonitorInfo)
        {
            kNoProcessMonitorText = "";
        }
        else
        {
            kNoProcessMonitorText = " <No STAF Monitor information available>";
        }
    }       
    
    public Component getTreeCellRendererComponent(JTree tree, Object value,
                                                  boolean selected,
                                                  boolean expanded,                                                  
                                                  boolean leaf,
                                                  int row, boolean hasFocus)
    {
        String text = (value == null) ? "" : String.valueOf(value);
        
        if (metrics == null)
        {
            metrics = tree.getFontMetrics(treeFont);
        }
        
        nodeLabel.setText(text);                
        
        if (selected)
        {            
            nodeLabel.setBackground(selectionColor);
            processMonitorLabel.setBackground(selectionColor);
            elapsedTimeLabel.setBackground(selectionColor);
            panel.setBackground(selectionColor);
            setBackground(selectionColor);
        }
        else
        {
            nodeLabel.setBackground(Color.white);
            processMonitorLabel.setBackground(Color.white);
            elapsedTimeLabel.setBackground(Color.white);
            panel.setBackground(Color.white);
            setBackground(Color.white);
        }
        
        if (value instanceof STAXMonitorTreeNode)
        {
            STAXMonitorTreeNode node = (STAXMonitorTreeNode) value;
            int type = node.fNodeType;            
        
            if (type != 0)
            {
                if (type == STAXMonitorTreeNode.blockNodeType)
                {
                    int blockStatus = node.fBlockStatus;
                                        
                    if (blockStatus == STAXMonitorTreeNode.blockRunning)
                    {                        
                        nodeLabel.setFont(treeFont);
                        nodeLabel.setIcon(runningBlockIcon);
                    }
                    else if (blockStatus == STAXMonitorTreeNode.blockHeld)
                    {                                           
                        nodeLabel.setFont(new Font("Dialog", Font.BOLD, 12));
                        nodeLabel.setIcon(heldBlockIcon);
                    }
                    else if (blockStatus == 
                             STAXMonitorTreeNode.blockParentHeld)
                    {                                              
                        nodeLabel.setFont(new Font("Dialog", Font.BOLD, 12));
                        nodeLabel.setIcon(parentHeldBlockIcon);
                    }                    
                                        
                    processMonitorLabel.setText("");
                    elapsedTimeLabel.setText("");
                    
                    processMonitorLabel.setPreferredSize(new Dimension(0, 0));
                    elapsedTimeLabel.setPreferredSize(new Dimension(0, 0));
                    
                    panel.removeAll();
                }
                else if (type == STAXMonitorTreeNode.processNodeType)
                {
                    nodeLabel.setIcon(processIcon);                    
                    nodeLabel.setFont(treeFont);
                    String monitorText = node.getProcessMonitorText();
                    nodeLabel.setText(text);
                    
                    if (monitorText.equals(""))
                    {
                        processMonitorLabel.setForeground(
                            noProcessMonitorTextColor);
                        monitorText = kNoProcessMonitorText;
                    }
                    else
                    {
                        processMonitorLabel.setForeground(
                            processMonitorTextColor);
                    }            
                    
                    processMonitorLabel.setText(monitorText);
                    
                    String elapsedTime = node.getElapsedTime();
                    if (!elapsedTime.equals(""))
                    {
                        elapsedTime = "  (" + elapsedTime + "): ";
                    }
                    elapsedTimeLabel.setText(elapsedTime);
                    
                    Dimension dim1 = nodeLabel.getPreferredSize();
                    Dimension dim2 = processMonitorLabel.getPreferredSize();
                    int width1 = metrics.stringWidth(monitorText);
                    processMonitorLabel.setPreferredSize(
                        new Dimension(width1, dim1.height));                       
                    
                    int width2 = 
                        metrics.stringWidth(elapsedTime);
                    elapsedTimeLabel.setPreferredSize(
                        new Dimension(width2, dim1.height));
                        
                    panel.removeAll();
                }        
                else if (type == STAXMonitorTreeNode.commandNodeType)                
                {
                    nodeLabel.setIcon(commandIcon);
                    nodeLabel.setFont(treeFont);
                    
                    processMonitorLabel.setText("");
                    
                    String elapsedTime = node.getElapsedTime();
                    if (!elapsedTime.equals(""))
                    {
                        elapsedTime = "  (" + elapsedTime + "): ";
                    }
                    elapsedTimeLabel.setText(elapsedTime);
                    
                    Dimension dim = nodeLabel.getPreferredSize();
                    int width = metrics.stringWidth(elapsedTime);
                    elapsedTimeLabel.setPreferredSize(
                        new Dimension(width, dim.height));
                        
                    panel.removeAll();                    
                }
                else if (type == STAXMonitorTreeNode.subjobNodeType)                
                {
                    nodeLabel.setIcon(subjobIcon);
                    nodeLabel.setFont(treeFont);
                    
                    processMonitorLabel.setText("");
                    
                    String elapsedTime = node.getElapsedTime();
                    if (!elapsedTime.equals(""))
                    {
                        elapsedTime = "  (" + elapsedTime + "): ";
                    }
                    elapsedTimeLabel.setText(elapsedTime);
                    
                    Dimension dim = nodeLabel.getPreferredSize();
                    int width = metrics.stringWidth(elapsedTime);
                    elapsedTimeLabel.setPreferredSize(
                        new Dimension(width, dim.height));
                        
                    panel.removeAll();                    
                }
            }
            else
            {
                nodeLabel.setIcon(node.getIcon());
                nodeLabel.setText(node.getPluginText());
                
                if (node.getComponent() != null)
                {
                    panel.removeAll();
                    panel.add((node.getComponent()));
                    elapsedTimeLabel.setText("");
                    elapsedTimeLabel.setPreferredSize(new Dimension(0, 0));
                    processMonitorLabel.setText("");
                    processMonitorLabel.setPreferredSize(new Dimension(0, 0));
                }
                
                if (selected)
                {
                    nodeLabel.setBackground(selectionColor);
                    processMonitorLabel.setBackground(selectionColor);
                    elapsedTimeLabel.setBackground(selectionColor);
                    panel.setBackground(selectionColor);
                    setBackground(selectionColor);

                    if (node.getComponent() != null)
                    {
                        node.getComponent().setBackground(selectionColor);
                    }
                }
                else
                {
                    nodeLabel.setBackground(Color.white);
                    processMonitorLabel.setBackground(Color.white);
                    elapsedTimeLabel.setBackground(Color.white);
                    panel.setBackground(Color.white);
                    setBackground(Color.white);
                    
                    if (node.getComponent() != null)
                    {
                        node.getComponent().setBackground(Color.white);
                    }
                }
            }
        }
        else
        {
            nodeLabel.setIcon(null);
        }
                                            
        return this;
    } 
}