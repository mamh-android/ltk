/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import com.installshield.wizard.swing.*;
import com.installshield.wizard.awt.*;

public class STAFOlderVersionPanel2xSwingImpl extends DefaultSwingWizardPanelImpl
{
    private JPanel outerPanel;

    public void initialize(WizardBeanEvent event)
    {
        super.initialize(event);  // do *not* remove this line
        createPanel();
        getContentPane().add(new JScrollPane(outerPanel));
    }
    
    protected void createPanel()
    {
        outerPanel = new JPanel();
        outerPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));        
        outerPanel.setBorder(new BevelBorder(BevelBorder.LOWERED));        

        JPanel olderVersionPanel = new JPanel();                
        olderVersionPanel.setLayout(new 
            BoxLayout(olderVersionPanel, BoxLayout.Y_AXIS));
        
        JPanel titlePanel = new JPanel();
        titlePanel.setLayout(new BorderLayout());
        titlePanel.add(BorderLayout.WEST, 
            new Label("An existing version of STAF is currently" +
                      " installed on this machine.", SwingConstants.LEFT));

        olderVersionPanel.add(titlePanel);
                         
        olderVersionPanel.add(Box.createVerticalStrut(15));     
        
        JPanel versionInfoPanel = new JPanel();
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc = new GridBagConstraints();
        versionInfoPanel.setLayout(gbl);
        
        JLabel versionLabel = new JLabel("Version:");
        versionLabel.setOpaque(true);
        versionLabel.setForeground(Color.black);
        versionLabel.setBackground(Color.white);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        versionInfoPanel.add(versionLabel, gbc);
        versionInfoPanel.add(Box.createHorizontalStrut(10), gbc);

        JLabel versionNumber = new 
            JLabel(resolveString("$W(stafRegistry.previousVersionNumber)"));        
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        versionInfoPanel.add(versionNumber, gbc);
        
        gbc.weightx = 0;
        versionInfoPanel.add(Box.createVerticalStrut(3), gbc);
        
        JLabel directoryLabel = new JLabel("Directory:");
        directoryLabel.setOpaque(true);
        directoryLabel.setForeground(Color.black);
        directoryLabel.setBackground(Color.white);
        
        gbc.gridwidth = 1;
        versionInfoPanel.add(directoryLabel, gbc);
        versionInfoPanel.add(Box.createHorizontalStrut(10), gbc);
        
        JLabel directory = new 
            JLabel(resolveString("$W(stafRegistry.previousVersionDirectory)"));
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        versionInfoPanel.add(directory, gbc);
        
        gbc.weightx = 0;
        versionInfoPanel.add(Box.createVerticalStrut(3), gbc);
        
        JLabel installTypeLabel = new JLabel("Install Type:");
        installTypeLabel.setOpaque(true);
        installTypeLabel.setForeground(Color.black);
        installTypeLabel.setBackground(Color.white);
        
        gbc.gridwidth = 1;
        versionInfoPanel.add(installTypeLabel, gbc);
        versionInfoPanel.add(Box.createHorizontalStrut(10), gbc);
        
        JLabel installType = new 
            JLabel(resolveString(
            "$W(stafRegistry.previousVersionInstallType)"));        
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        versionInfoPanel.add(installType, gbc);
        
        JPanel outerVersionPanel = new JPanel();
        outerVersionPanel.setLayout(new 
            FlowLayout(FlowLayout.LEFT, 0, 0));
        outerVersionPanel.add(Box.createHorizontalStrut(40));
        outerVersionPanel.add(versionInfoPanel);

        olderVersionPanel.add(outerVersionPanel);
        
        JPanel infoPanel = new JPanel();        
        infoPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        
        JPanel textPanel = new JPanel();
        textPanel.setLayout(new GridLayout(4, 0));
        textPanel.add(new Label("Before installing STAF Version 2.6.6, " +
                                "you must first uninstall the "));
        textPanel.add(new Label("current version of STAF and then restart " +
                                "this Installation program."));
        textPanel.add(new Label("    "));
        textPanel.add(new Label("Click on Cancel to exit this Installation program"));
                         
        infoPanel.add(Box.createHorizontalStrut(10));
        infoPanel.add(textPanel);

        olderVersionPanel.add(Box.createVerticalStrut(15));
        olderVersionPanel.add(infoPanel);
                         
        outerPanel.add(olderVersionPanel);        
    }   
}