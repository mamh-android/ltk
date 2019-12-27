/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.extension.samples.extdelay;

import javax.swing.*;
import javax.swing.event.*;
import com.ibm.staf.*;
import java.util.*;
import java.awt.*;
import java.lang.reflect.*;
import com.ibm.staf.service.stax.*;
    
public class ExtDelay implements STAXMonitorExtension
{
    STAFHandle fHandle;
    String fLocalMachine;
    String fStaxMachine;
    String fStaxServiceName;
    String fJobNumber;
    STAXMonitorFrame fMonitorFrame;
    String fTitle;
    HashMap fProgressBarMap;
    boolean fContinueElapsedTime = true;
    Hashtable fMonitorDelayStartTimes = new Hashtable();
    Hashtable fMonitorDelayTimeLabels = new Hashtable();
    MonitorElapsedTime fElapsedTime;
    
    private ImageIcon delayIcon;
    
    public JComponent init(STAXMonitorFrame monitorFrame, boolean newJob,
                           String staxMachineName, 
                           String staxServiceName, String jobNumber)
                           throws STAFException
    {
        fMonitorFrame = monitorFrame;
        fStaxMachine = staxMachineName;
        fStaxServiceName = staxServiceName;
        fJobNumber = jobNumber;

        fTitle = "Delay text";
        
        Class c = this.getClass();
        ClassLoader classLoader = c.getClassLoader();
        
        delayIcon = ((STAXMonitorExtensionClassLoader)classLoader).
            getImage("delay.gif");
        
        fProgressBarMap = new HashMap();
        
        fElapsedTime = new MonitorElapsedTime();
        fElapsedTime.start();
       
        return new JPanel();
    }        

    public String getNotificationEventTypes()
    {
        return "ext-delay";
    }
    
    public void term() {}
    
    public String getTitle() 
    { 
        return fTitle; 
    }
    
    public int getExtensionType()
    {
        return STAXMonitorFrame.EXTENSION_ACTIVE_JOB_ELEMENTS;
    }
    
    public JComponent getComponent()
    {
        return new JPanel();
    }
    
    public void handleEvent(Map map)
    {
        String status = (String)map.get("status");
        
        String block = (String)map.get("block");
        
        String id = (String)map.get("name");
        
        if (status.equals("start"))
        {
            String delay = (String)map.get("delay");

            Vector delayDataVector = new Vector();
                
            addRow(delayDataVector, "Delay Value", delay);
            
            JProgressBar progressBar = new JProgressBar();

            progressBar.setMaximum((new Integer(delay)).intValue());
            progressBar.setStringPainted(true);
            Dimension dim = progressBar.getPreferredSize();
            dim.height = 20;
            progressBar.setPreferredSize(dim);
            
            synchronized(fProgressBarMap)
            {
                fProgressBarMap.put(id, progressBar);
            }
            
            JLabel elapsedTimeLabel = new JLabel();
            elapsedTimeLabel.setFont(new Font("Dialog", Font.PLAIN, 12));
            
            JPanel extPanel = new JPanel();
            extPanel.setLayout(new BorderLayout());
            extPanel.setBackground(Color.white);
            extPanel.add(BorderLayout.CENTER, progressBar);
            extPanel.add(BorderLayout.EAST, elapsedTimeLabel);

            fMonitorFrame.addActiveJobElementsNode("delay", id, 
                block, id, delayIcon, extPanel, delayDataVector);
          
            synchronized(fMonitorDelayStartTimes)
            {
                fMonitorDelayStartTimes.put(id, Calendar.getInstance());
            }
                
            synchronized(fMonitorDelayTimeLabels)
            {
                fMonitorDelayTimeLabels.put(id, elapsedTimeLabel);
            }
        }
        else if (status.equals("stop"))
        {
            synchronized(fProgressBarMap)
            {
                fProgressBarMap.remove(id);
            }
            
            synchronized(fMonitorDelayStartTimes)
            {
                fMonitorDelayStartTimes.remove(id);
            }
            
            synchronized(fMonitorDelayTimeLabels)
            {
                fMonitorDelayTimeLabels.remove(id);
            }
                   
            fMonitorFrame.removeActiveJobElementsNode(id, block);
        }
        else if (status.equals("iterate"))
        {
            String delay = (String)map.get("delay");
            
            String currentIter = (String)map.get("currentiter");
            
            Integer delayInt = new Integer(delay);
            
            final Integer currentIterInt = new Integer(currentIter);
            
            float delayFloat = (float)(delayInt.intValue());
            float currentIterFloat = (float)(currentIterInt.intValue());
            
            int percent = (int)((currentIterFloat / delayFloat * 100));
            
            final JProgressBar progressBar = 
                (JProgressBar)fProgressBarMap.get(id);
            
            progressBar.setValue(currentIterInt.intValue());
            
            fMonitorFrame.setActiveJobElementsNodeText(id, id + 
                "  " + (new Integer(percent).toString()) + "% complete");
        }
    }

    public void addRow(Vector vector, String name, String value)
    {
        Vector newRow = new Vector(2);
        newRow.add(name);
        newRow.add(value);
        vector.add(newRow);
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
                final Enumeration delayElapsedTimeKeys = 
                    fMonitorDelayStartTimes.keys();                
                    
                Runnable delayRunnable = new Runnable()
                {
                    public void run()
                    {
                        while (fContinueElapsedTime && 
                               delayElapsedTimeKeys.hasMoreElements())
                        {
                            String delayKey = 
                                (String)delayElapsedTimeKeys.nextElement();
                                
                            Calendar delayStarted = (Calendar)
                                fMonitorDelayStartTimes.get(delayKey);
                
                            synchronized(fMonitorDelayTimeLabels)
                            {
                                JLabel elapsedTimeLabel = (JLabel)
                                    fMonitorDelayTimeLabels.get(delayKey);
                            
                                elapsedTimeLabel.setText("  (" + STAXMonitorUtil.
                                    getElapsedTime(delayStarted) + ")");
                            }
                        }
                    }
                };
                    
                try
                {
                    SwingUtilities.invokeAndWait(delayRunnable);
                }
                catch (InterruptedException ex)
                {
                     ex.printStackTrace();
                }
                catch (InvocationTargetException ex)
                {
                     ex.printStackTrace();
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