/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2010                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import javax.swing.JFrame;
import javax.swing.JLabel;
import java.awt.FlowLayout;
import java.awt.Font;

/**
 * This class is used to display the Wait Dialog. This Dialog is displayed
 * when the system is busy processing.
 */
public class STAXMonitorWaitDialog extends JFrame
{
    public STAXMonitorWaitDialog(JFrame parentFrame)
    {
        this(parentFrame, "Processing.  Please wait...");
    }
    
    public STAXMonitorWaitDialog(JFrame parentFrame, String waitMessage)
    {
        try
        {
            this.getContentPane().setLayout(new FlowLayout(FlowLayout.CENTER));
            this.setResizable(false);
            this.setTitle("Please wait");

            JLabel waitLabel = new JLabel();
            waitLabel.setText(waitMessage);
            waitLabel.setFont(new Font("Tahoma", 1, 13));
            
            this.getContentPane().add(waitLabel, null);
            this.setSize(300, 75);
            this.setLocationRelativeTo(parentFrame);
            
            // Display the wait dialog
            this.showDialog();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }
 
    /**
     * This method displays the wait dialog.
     */
    void showDialog()
    {
        this.setVisible(true);
        this.paint(this.getGraphics());
    }
 
    /**
     * This method hides the wait dialog.
     */
    void hideDialog()
    {
        this.setVisible(false);
    }
}
