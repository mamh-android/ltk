/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.extension.samples.extmessage;

import javax.swing.*;
import javax.swing.event.*;
import com.ibm.staf.*;
import java.util.*;
import java.awt.*;
import com.ibm.staf.service.stax.*;
    
public class ExtMessage implements STAXMonitorExtension
{
    STAFHandle fHandle;
    String fLocalMachine;
    String fStaxMachine;
    String fStaxServiceName;
    String fJobNumber;
    JTextArea fMessageTextArea;
    STAXMonitorFrame fMonitorFrame;
    String fTitle;
    
    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName, 
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;

        fTitle = "Message Text";
        
        fMessageTextArea = new JTextArea();
        fMessageTextArea.setEditable(false);
        fMessageTextArea.setFont((new Font("Courier", Font.PLAIN, 12)));
       
        return fMessageTextArea;
    }        

    public String getNotificationEventTypes()
    {
        return "message";
    }
    
    public void term() {}
    
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
        return fMessageTextArea;
    }
    
    public void handleEvent(Map map)
    {
        String msg = (String)map.get("messagetext");
        fMessageTextArea.append(msg.substring(msg.indexOf(" ") + 1) + "\n");
    } 
}