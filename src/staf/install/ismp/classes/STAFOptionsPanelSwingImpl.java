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

public class STAFOptionsPanelSwingImpl extends DefaultSwingWizardPanelImpl
                                       implements ActionListener, 
                                                  KeyListener
{
    private JRadioButton systemEnvVarsRB;
    private JRadioButton userEnvVarsRB;
    private JRadioButton noneEnvVarsRB;
    private JCheckBox startSTAFCB;
    private JCheckBox iconsOnStartProgramsMenuCB;    
    private JCheckBox iconsOnQuickStartMenuCB;
    private JCheckBox allowRegistrationCB;
    private JRadioButton java11RB;
    private JRadioButton java12RB;
    private JRadioButton perl58RB;
    private JRadioButton perl56RB;
    private JRadioButton perl50RB;
    private JRadioButton ipv4RB;
    private JRadioButton ipv6RB;
    private JPanel outerPanel;
    private JTextField nameTF;   
    private JTextField emailTF;
    private JTextField organizationTF;
    private boolean first = true;
    
    private String blankSpace = 
        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    
    // Properties
    private boolean allowSTAFToRegister = true;
    private String updateEnvironmentVariables = "System";
    private boolean startSTAFOnLogin = true;
    private boolean iconsOnStartProgramsMenu = true;
    private boolean iconsOnQuickStartMenu = true;
    private String defaultJavaVersion = "";
    private String defaultPerlVersion = "";
    private String defaultIPvVersion = "";
    private String registrationName = "";
    private String registrationEmail = "";
    private String registrationOrganization = "";
    private String optionsSummary = "";    
 
    public void initialize(WizardBeanEvent event)
    {
        super.initialize(event);  // do *not* remove this line
        createPanel();
        getContentPane().add(new JScrollPane(outerPanel));
    }
    
    protected void createPanel()
    {
        boolean isWindows = 
            resolveString("$W(stafPlatform.windows)").equals("true");

        boolean isLinux =
            resolveString("$W(stafPlatform.linux)").equals("true");

        outerPanel = new JPanel();
        outerPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));        
        //outerPanel.setBorder(new BevelBorder(BevelBorder.LOWERED));        

        JPanel optionsPanel = new JPanel();                
        optionsPanel.setLayout(new BoxLayout(optionsPanel, BoxLayout.Y_AXIS));

        JPanel titlePanel = new JPanel();
        titlePanel.setLayout(new BorderLayout());
        titlePanel.add(BorderLayout.WEST, 
            new Label("Please Choose the options that you want.", 
            SwingConstants.LEFT));

        optionsPanel.add(titlePanel);
                         
        optionsPanel.add(Box.createVerticalStrut(5));        
        
        String envText = "Update Environment:  ";

        JPanel allUsersPanel = new JPanel();
        allUsersPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));                        
        systemEnvVarsRB = new JRadioButton("System");
        systemEnvVarsRB.addActionListener(this);
        userEnvVarsRB = new JRadioButton("User");
        userEnvVarsRB.addActionListener(this);
        noneEnvVarsRB = new JRadioButton("None");
        noneEnvVarsRB.addActionListener(this);
        ButtonGroup envVarsBG = new ButtonGroup();
        envVarsBG.add(systemEnvVarsRB);
        envVarsBG.add(userEnvVarsRB);
        envVarsBG.add(noneEnvVarsRB);
        allUsersPanel.add(Box.createHorizontalStrut(10));
        allUsersPanel.add(new JLabel(envText));
        allUsersPanel.add(systemEnvVarsRB);
        allUsersPanel.add(userEnvVarsRB);
        allUsersPanel.add(noneEnvVarsRB);
        optionsPanel.add(allUsersPanel);

        String updateVars =
            getSTAFOptionsPanel().getUpdateEnvironmentVariables();
            
        if (updateVars.equals("System"))
        {
            systemEnvVarsRB.setSelected(true);
        }
        else if (updateVars.equals("User"))
        {
            userEnvVarsRB.setSelected(true);
        }
        else if (updateVars.equals("None"))
        {
            noneEnvVarsRB.setSelected(true);
        }

        if (isWindows)
        {                        
            JPanel startSTAFPanel = new JPanel();
            startSTAFPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            startSTAFCB = new JCheckBox("Start STAF on user login", 
                getSTAFOptionsPanel().getStartSTAFOnLogin());
            startSTAFCB.addActionListener(this);
            startSTAFPanel.add(Box.createHorizontalStrut(10));
            startSTAFPanel.add(startSTAFCB);
            optionsPanel.add(startSTAFPanel);
            
            JPanel iconsOnStartProgramsMenuPanel = new JPanel();
            iconsOnStartProgramsMenuPanel.setLayout(new 
                FlowLayout(FlowLayout.LEFT, 0, 0));
            iconsOnStartProgramsMenuCB = new 
                JCheckBox("Place Icons on the Start Programs Menu", 
                getSTAFOptionsPanel().getIconsOnStartProgramsMenu());
            iconsOnStartProgramsMenuCB.addActionListener(this);
            iconsOnStartProgramsMenuPanel.add(Box.createHorizontalStrut(10));
            iconsOnStartProgramsMenuPanel.add(iconsOnStartProgramsMenuCB);
            optionsPanel.add(iconsOnStartProgramsMenuPanel);
            
            JPanel iconsOnQuickStartMenuPanel = new JPanel();
            iconsOnQuickStartMenuPanel.setLayout(new 
                FlowLayout(FlowLayout.LEFT, 0, 0));
            iconsOnQuickStartMenuCB = new 
                JCheckBox("Place Icons on the Quick Start Menu", 
                getSTAFOptionsPanel().getIconsOnQuickStartMenu());
            iconsOnQuickStartMenuCB.addActionListener(this);
            iconsOnQuickStartMenuPanel.add(Box.createHorizontalStrut(10));
            iconsOnQuickStartMenuPanel.add(iconsOnQuickStartMenuCB);
            optionsPanel.add(iconsOnQuickStartMenuPanel);
        }
        
        JPanel allowRegistrationPanel = new JPanel();
        allowRegistrationPanel .setLayout(new 
            FlowLayout(FlowLayout.LEFT, 0, 0));
        allowRegistrationCB = new 
            JCheckBox("Allow STAF to Register", 
            getSTAFOptionsPanel().getAllowSTAFToRegister());        
        allowRegistrationCB.addActionListener(this);
        allowRegistrationPanel.add(Box.createHorizontalStrut(10));
        allowRegistrationPanel.add(allowRegistrationCB);        
        optionsPanel.add(allowRegistrationPanel);
        
        JPanel registrationPanel = new JPanel();
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc = new GridBagConstraints();
        registrationPanel.setLayout(gbl);
        
        JLabel nameLabel = new JLabel("Name:");
        nameLabel.setOpaque(false);
        nameLabel.setForeground(Color.black);
        
        gbc.anchor = GridBagConstraints.NORTHWEST;
        registrationPanel.add(nameLabel, gbc);
        registrationPanel.add(Box.createHorizontalStrut(10), gbc);

        nameTF = new JTextField(20);
        nameTF.setText(getSTAFOptionsPanel().getRegistrationName());
        nameTF.setEnabled(allowSTAFToRegister);
        nameTF.addKeyListener(this);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        registrationPanel.add(nameTF, gbc);
        
        gbc.weightx = 0;
        registrationPanel.add(Box.createVerticalStrut(3), gbc);
        
        JLabel emailLabel = new JLabel("Email:");
        emailLabel.setOpaque(false);
        emailLabel.setForeground(Color.black);
        
        gbc.gridwidth = 1;
        registrationPanel.add(emailLabel, gbc);
        registrationPanel.add(Box.createHorizontalStrut(10), gbc);
        
        emailTF = new JTextField(20);
        emailTF.setText(getSTAFOptionsPanel().getRegistrationEmail());
        emailTF.setEnabled(allowSTAFToRegister);
        emailTF.addKeyListener(this);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        registrationPanel.add(emailTF, gbc);
        
        gbc.weightx = 0;
        registrationPanel.add(Box.createVerticalStrut(3), gbc);
        
        JLabel organizationLabel = new JLabel("Organization:");
        organizationLabel.setOpaque(false);
        organizationLabel.setForeground(Color.black);
        
        gbc.gridwidth = 1;
        registrationPanel.add(organizationLabel, gbc);
        registrationPanel.add(Box.createHorizontalStrut(10), gbc);
        
        organizationTF = new JTextField(20);
        organizationTF.setText(getSTAFOptionsPanel().
            getRegistrationOrganization());
        organizationTF.setEnabled(allowSTAFToRegister);
        organizationTF.addKeyListener(this);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        registrationPanel.add(organizationTF, gbc);
        
        JPanel outerRegistrationPanel = new JPanel();
        outerRegistrationPanel.setLayout(new 
            FlowLayout(FlowLayout.LEFT, 0, 0));
        outerRegistrationPanel.add(Box.createHorizontalStrut(40));
        outerRegistrationPanel.add(registrationPanel);

        optionsPanel.add(outerRegistrationPanel);
      
        if (resolveString("$P(perlSupport.active)").equals("true") &&
            (resolveString("$W(stafPlatform.linux)").equals("true") ||
            isWindows))
        {   
            perl58RB = new JRadioButton("Perl 5.8", 
                getSTAFOptionsPanel().getDefaultPerlVersion().equals("5.8"));         
            perl56RB = new JRadioButton("Perl 5.6", 
                getSTAFOptionsPanel().getDefaultPerlVersion().equals("5.6"));
            perl50RB = new JRadioButton("Perl 5.0",
                getSTAFOptionsPanel().getDefaultPerlVersion().equals(
                "5.0"));
            perl58RB.addActionListener(this);
            perl56RB.addActionListener(this);
            perl50RB.addActionListener(this);
            ButtonGroup perlBG = new ButtonGroup();
            perlBG.add(perl58RB);
            perlBG.add(perl56RB);
            perlBG.add(perl50RB);

            if (resolveString("$W(stafPlatform.windows)").equals("true"))
            {
                perl50RB.setEnabled(false);
            }
            
            JPanel perlVersionPanel = new JPanel();
            perlVersionPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            perlVersionPanel.add(Box.createHorizontalStrut(10));
            JLabel perlLabel = new JLabel("Default Perl Support:");
            perlLabel.setOpaque(false);
            perlLabel.setForeground(Color.black);
            perlVersionPanel.add(perlLabel);
            perlVersionPanel.add(Box.createHorizontalStrut(10));
            perlVersionPanel.add(perl58RB);
            perlVersionPanel.add(Box.createHorizontalStrut(10));
            perlVersionPanel.add(perl56RB);
            perlVersionPanel.add(Box.createHorizontalStrut(5));
            perlVersionPanel.add(perl50RB);
            optionsPanel.add(Box.createVerticalStrut(5));
            optionsPanel.add(perlVersionPanel);
        }
        
        ipv4RB = new JRadioButton("IPv4 only", 
            getSTAFOptionsPanel().getDefaultIPvVersion().equals("IPv4 only"));
        ipv6RB = new JRadioButton("IPv4/IPv6 (requires OS IPv6 support)", 
            getSTAFOptionsPanel().getDefaultPerlVersion().equals("IPv4 and IPv6"));
        ipv4RB.addActionListener(this);
        ipv6RB.addActionListener(this);
        ButtonGroup ipvBG = new ButtonGroup();
        ipvBG.add(ipv4RB);
        ipvBG.add(ipv6RB);
        JPanel ipvVersionPanel = new JPanel();
        ipvVersionPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        ipvVersionPanel.add(Box.createHorizontalStrut(10));
        JLabel ipvLabel = new JLabel("Default TCP version:");
        ipvLabel.setOpaque(false);
        ipvLabel.setForeground(Color.black);
        ipvVersionPanel.add(ipvLabel);
        ipvVersionPanel.add(Box.createHorizontalStrut(10));
        ipvVersionPanel.add(ipv4RB);
        ipvVersionPanel.add(Box.createHorizontalStrut(10));
        ipvVersionPanel.add(ipv6RB);
        optionsPanel.add(ipvVersionPanel);
                         
        outerPanel.add(new JScrollPane(optionsPanel));
    }
    
    public void actionPerformed(ActionEvent e)
    {
        boolean isWindows = 
            resolveString("$W(stafPlatform.windows)").equals("true");

        if (e.getSource() == allowRegistrationCB)
        {
            getSTAFOptionsPanel().setAllowSTAFToRegister(
                allowRegistrationCB.isSelected());
            
            if (!allowRegistrationCB.isSelected())
            {
                nameTF.setEnabled(false);
                emailTF.setEnabled(false);
                organizationTF.setEnabled(false);
                
                JOptionPane.showMessageDialog(outerPanel,
                    "Enabling STAF to register will allow us to track\n" +
                    "the STAF user base.  This will help ensure continued\n" +
                    "support and funding of STAF in the future",
                    "Information", JOptionPane.INFORMATION_MESSAGE);
            }
            else
            {
                nameTF.setEnabled(true);
                emailTF.setEnabled(true);
                organizationTF.setEnabled(true);
            }
                        
        }

        if (e.getSource() == systemEnvVarsRB)
        {
            getSTAFOptionsPanel().setUpdateEnvironmentVariables("System");

            if (isWindows)
            {
                if (! iconsOnStartProgramsMenuCB.isEnabled())
                {
                    iconsOnStartProgramsMenuCB.setSelected(true);
                    iconsOnStartProgramsMenuCB.setEnabled(true);
                    getSTAFOptionsPanel().setIconsOnStartProgramsMenu(
                        iconsOnStartProgramsMenuCB.isSelected());
                }

                if (! iconsOnQuickStartMenuCB.isEnabled())
                {
                    iconsOnQuickStartMenuCB.setSelected(true);
                    iconsOnQuickStartMenuCB.setEnabled(true);
                    getSTAFOptionsPanel().setIconsOnQuickStartMenu(
                        iconsOnQuickStartMenuCB.isSelected());
                }

                if (! startSTAFCB.isEnabled())
                {
                    startSTAFCB.setSelected(true);
                    startSTAFCB.setEnabled(true);
                    getSTAFOptionsPanel().setStartSTAFOnLogin(
                        startSTAFCB.isSelected());
                }
            }
        }

        if (e.getSource() == userEnvVarsRB)
        {
            getSTAFOptionsPanel().setUpdateEnvironmentVariables("User");

            if (isWindows)
            {
                if (! iconsOnStartProgramsMenuCB.isEnabled())
                {
                    iconsOnStartProgramsMenuCB.setSelected(true);
                    iconsOnStartProgramsMenuCB.setEnabled(true);
                    getSTAFOptionsPanel().setIconsOnStartProgramsMenu(
                        iconsOnStartProgramsMenuCB.isSelected());
                }

                if (! iconsOnQuickStartMenuCB.isEnabled())
                {
                    iconsOnQuickStartMenuCB.setSelected(true);
                    iconsOnQuickStartMenuCB.setEnabled(true);
                    getSTAFOptionsPanel().setIconsOnQuickStartMenu(
                        iconsOnQuickStartMenuCB.isSelected());
                }

                if (! startSTAFCB.isEnabled())
                {
                    startSTAFCB.setSelected(true);
                    startSTAFCB.setEnabled(true);
                    getSTAFOptionsPanel().setStartSTAFOnLogin(
                        startSTAFCB.isSelected());
                }
            }
        }

        if (e.getSource() == noneEnvVarsRB)
        {
            getSTAFOptionsPanel().setUpdateEnvironmentVariables("None");

            if (isWindows)
            {
                iconsOnStartProgramsMenuCB.setSelected(false);
                iconsOnStartProgramsMenuCB.setEnabled(false);
                getSTAFOptionsPanel().setIconsOnStartProgramsMenu(
                    iconsOnStartProgramsMenuCB.isSelected());

                iconsOnQuickStartMenuCB.setSelected(false);
                iconsOnQuickStartMenuCB.setEnabled(false);
                getSTAFOptionsPanel().setIconsOnQuickStartMenu(
                    iconsOnQuickStartMenuCB.isSelected());

                startSTAFCB.setSelected(false);
                startSTAFCB.setEnabled(false);
                getSTAFOptionsPanel().setStartSTAFOnLogin(
                    startSTAFCB.isSelected());
            }
        }
        
        if (e.getSource() == startSTAFCB)
        {
            getSTAFOptionsPanel().setStartSTAFOnLogin(
                startSTAFCB.isSelected());
        }
        
        if (e.getSource() == iconsOnStartProgramsMenuCB)
        {
            getSTAFOptionsPanel().setIconsOnStartProgramsMenu(
                iconsOnStartProgramsMenuCB.isSelected());
        }
        
        if (e.getSource() == iconsOnQuickStartMenuCB)
        {
            getSTAFOptionsPanel().setIconsOnQuickStartMenu(
                iconsOnQuickStartMenuCB.isSelected());
        }
       
        if (e.getSource() == java12RB)
        {
            getSTAFOptionsPanel().setDefaultJavaVersion("1.2+");
        }
        
        if (e.getSource() == java11RB)
        {
            getSTAFOptionsPanel().setDefaultJavaVersion("1.1.x");
        }

        if (e.getSource() == perl58RB)
        {
            getSTAFOptionsPanel().setDefaultPerlVersion("5.8");
        }
        
        if (e.getSource() == perl56RB)
        {
            getSTAFOptionsPanel().setDefaultPerlVersion("5.6");
        }
        
        if (e.getSource() == perl50RB)
        {
            getSTAFOptionsPanel().setDefaultPerlVersion("5.0");
        }
        
        if (e.getSource() == ipv4RB)
        {
            getSTAFOptionsPanel().setDefaultIPvVersion("IPv4 only");
        }
        
        if (e.getSource() == ipv6RB)
        {
            getSTAFOptionsPanel().setDefaultIPvVersion("IPv4 and IPv6");
        }
    }

    public void keyTyped(KeyEvent e)
    {
        if (e.getSource() == nameTF)
        {
            getSTAFOptionsPanel().setRegistrationName(nameTF.getText());
            return;
        }

        if (e.getSource() == emailTF)
        {
            getSTAFOptionsPanel().setRegistrationEmail(emailTF.getText());
            return;
        }

        if (e.getSource() == organizationTF)
        {
            getSTAFOptionsPanel().setRegistrationOrganization(
                organizationTF.getText());
            return;
        }
    }

    public void keyPressed(KeyEvent e)
    {
        if (e.getSource() == nameTF)
        {
            getSTAFOptionsPanel().setRegistrationName(nameTF.getText());
            return;
        }

        if (e.getSource() == emailTF)
        {
            getSTAFOptionsPanel().setRegistrationEmail(emailTF.getText());
            return;
        }

        if (e.getSource() == organizationTF)
        {
            getSTAFOptionsPanel().setRegistrationOrganization(
                organizationTF.getText());
            return;
        }
    }

    public void keyReleased(KeyEvent e)
    {
        if (e.getSource() == nameTF)
        {
            getSTAFOptionsPanel().setRegistrationName(nameTF.getText());
            return;
        }

        if (e.getSource() == emailTF)
        {
            getSTAFOptionsPanel().setRegistrationEmail(emailTF.getText());
            return;
        }

        if (e.getSource() == organizationTF)
        {
            getSTAFOptionsPanel().setRegistrationOrganization(
                organizationTF.getText());
            return;
        }
    }

    private STAFOptionsPanel getSTAFOptionsPanel()
    {
         return (STAFOptionsPanel)getPanel();
    }
}