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

public class STAXMonitorTreeNode extends DefaultMutableTreeNode
{
    public static int blockNodeType = 1;
    public static int processNodeType = 2;
    public static int commandNodeType = 3;
    public static int subjobNodeType = 4;
        
    public static int blockRunning = 1;
    public static int blockHeld = 2;
    public static int blockParentHeld = 3;

    public int fNodeType = 0;
    public String fPluginNodeType;
    public int fBlockStatus = blockRunning;
    private String processMonitorText = "";
    private String elapsedTime = "";
    private String pluginText = "";
    
    private ImageIcon icon;
    private JComponent component = null;
    
    public STAXMonitorTreeNode()
    {
        super();
    }    
    
    public STAXMonitorTreeNode(Object userObject)
    {
        super(userObject);
    }

    public STAXMonitorTreeNode(Object userObject, int nodeType)
    {
        super(userObject);
        fNodeType = nodeType;
    }
    
    public STAXMonitorTreeNode(Object userObject, String pluginNodeType,
        ImageIcon image, JComponent component)
    {
        super(userObject);
        fPluginNodeType = pluginNodeType;
        
        pluginText = (String)userObject;
        icon = image;
        
        this.component = component;
    }
    
    public void setBlockStatus(int status)
    {
        fBlockStatus = status;
        
        int childCount = getChildCount();
        
        for (int i = 0; i < childCount; i++)
        {
            STAXMonitorTreeNode child = (STAXMonitorTreeNode)getChildAt(i);
            
            if (child.fNodeType == blockNodeType)
            {
                if (child.fBlockStatus != blockHeld)
                {
                    if (fBlockStatus == blockHeld)
                    {
                        child.setBlockStatus(blockParentHeld);
                    }
                    else if (fBlockStatus == blockParentHeld)
                    {
                        child.setBlockStatus(blockParentHeld);
                    }                    
                    else
                    {
                        child.setBlockStatus(blockRunning);
                    }
                }
            }
        }

    }
          
    public int getBlockStatus()
    {
        return fBlockStatus;
    }
    
    public void setProcessMonitorText(String text)
    {
        processMonitorText = text;
    }
    
    public void setPluginText(String text)
    {
        pluginText = text;
    }
    
    public String getPluginText()
    {
        return pluginText;
    }
    
    public String getProcessMonitorText()
    {
        return processMonitorText;
    }
    
    public void setElapsedTime(String time)
    {
        elapsedTime = time;
    }
    
    public String getElapsedTime()
    {
        return elapsedTime;
    }    
    
    public ImageIcon getIcon()
    {
        return icon;
    }
    
    public JComponent getComponent()
    {
        return component;
    }
} 