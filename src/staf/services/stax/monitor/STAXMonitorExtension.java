/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import javax.swing.*;
import com.ibm.staf.*;
import java.util.*;

public interface STAXMonitorExtension
{
    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName, String staxServiceName,
                           String jobNumber) throws STAFException;
        // The extension should initialize the JComponent and 
        // perform any other necessary initialization activites, 
        // and return the JComponent that will be displayed 
        // in the STAX Monitor frame

    public String getNotificationEventTypes();
        // Returns a space-delimited string of event types (such as
        // process, stafcmd, block, etc.) that the extension wants to
        // be notified about
        
    public String getTitle();
        // Returns the title of the extension, which is displayed in the
        // View menu bar and in the JTabbedPane header
        
    public JComponent getComponent();
        // Returns the JComponent that will be displayed in the Monitor
        // frame

    public int getExtensionType();
        // Returns one of the following:
        //    STAXMonitorFrame.EXTENSION_ACTIVE (upper left panel)
        //    STAXMonitorFrame.EXTENSION_STATUS (upper right panel)
        //    STAXMonitorFrame.EXTENSION_INFO (lower panel)
           
    public void handleEvent(Map eventMap);
        // The extension should use the Map information to update its display
        
    public void term();
        // The extension should perform any necessary termination activities
}
