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

public class STAFOptionsPanel extends WizardPanel
{
    private boolean first = true;
    
    private String blankSpace = 
        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    
    // Properties
    private boolean allowSTAFToRegister = true;
    //private boolean makeAvailableToAllUsers = true;
    private boolean startSTAFOnLogin = true;
    private boolean iconsOnStartProgramsMenu = true;
    private boolean iconsOnQuickStartMenu = true;
    private String updateEnvironmentVariables = "System"; // System, User, or None
    private String defaultJavaVersion = "1.2+";
    private String defaultPerlVersion = "5.8";
    private String defaultIPvVersion = "IPv4 only";
    private String registrationName = "";
    private String registrationEmail = "";
    private String registrationOrganization = "";
    private String optionsSummary = "";    
  
    public boolean queryEnter(WizardBeanEvent event)
    {       
        if (first)
        {
            setDefaultJavaVersion("1.2+");
            setDefaultPerlVersion("5.8");
            setDefaultIPvVersion("IPv4 only");
            first = false;            
        }
        
        return true;
    }
    
    public boolean queryExit(WizardBeanEvent event)
    {
        boolean isWindows = 
            resolveString("$W(stafPlatform.windows)").equals("true");    

        if (allowSTAFToRegister)
        {
            setRegistrationName(getRegistrationName());
            setRegistrationEmail(getRegistrationEmail());
            setRegistrationOrganization(getRegistrationOrganization());
        }

        optionsSummary = "";
        
        if (allowSTAFToRegister)
        {
            optionsSummary += blankSpace + "STAF will be registered to:<br>";
            optionsSummary += blankSpace + blankSpace + "Name: ";
            optionsSummary += getRegistrationName() + "<br>";
            optionsSummary += blankSpace + blankSpace + "Email: ";
            optionsSummary += getRegistrationEmail() + "<br>";
            optionsSummary += blankSpace + blankSpace + "Organization: ";
            optionsSummary += getRegistrationOrganization() + "<br>";
        }
        else
        {
            optionsSummary += blankSpace + "Do not register STAF<br>";
        }
        
        if (resolveString("$W(stafPlatform.linux)").equals("true"))
        {
            boolean isAdminOrRoot = 
                resolveString("$W(adminOrRoot.isAdminOrRoot)").equals("true");

            if (updateEnvironmentVariables.equals("System") && isAdminOrRoot)
            {            
                optionsSummary += blankSpace + 
                    "Update System environment variables<br>";
            }
            else if (updateEnvironmentVariables.equals("User") ||
                    (updateEnvironmentVariables.equals("System") &&
                     !isAdminOrRoot))
            {
                optionsSummary += blankSpace + 
                    "Update User environment variables<br>";
            }
            else
            {
                optionsSummary += blankSpace + 
                    "Do not update environment variables<br>";
            }
        }
        
        if (isWindows)
        {
            if (updateEnvironmentVariables.equals("System"))
            {            
                optionsSummary += blankSpace + 
                    "Update System environment variables<br>";
            }
            else if (updateEnvironmentVariables.equals("User"))
            {
                optionsSummary += blankSpace + 
                    "Update User environment variables<br>";
            }
            else
            {
                optionsSummary += blankSpace + 
                    "Do not update environment variables<br>";
            }
            
            if (startSTAFOnLogin)
            {
                optionsSummary += blankSpace + "Start STAF on user login<br>";
            }
            else
            {
                optionsSummary += blankSpace + 
                    "Do not start STAF on user login<br>";
            }
               
            if (iconsOnStartProgramsMenu)
            {
                optionsSummary += blankSpace + 
                    "Place icons on the Start Programs Menu<br>";
            }
            else
            {
                optionsSummary += blankSpace + 
                    "Do not place icons on the Start Programs Menu<br>";
            }
            
            if (iconsOnQuickStartMenu)
            {
                optionsSummary += blankSpace + 
                    "Place icons on the Quick Start Menu<br>";
            }
            else
            {
                optionsSummary += blankSpace + 
                    "Do not place icons on the Quick Start Menu<br>";
            }
        }

        if (resolveString("$P(perlSupport.active)").equals("true") &&
            (resolveString("$W(stafPlatform.linux)").equals("true") ||
            isWindows))
        {
            optionsSummary += blankSpace + "Default Perl Support: " + 
                getDefaultPerlVersion() + "<br>";
        }
        
        optionsSummary += blankSpace + "Default TCP IP version: " + 
            getDefaultIPvVersion() + "<br>";
        
        return true;
    }

    /*protected void createUI(WizardBeanEvent event) 
    {        
        createPanel();
        getContentPane().add(new JScrollPane(outerPanel));
    }
    
    protected void createPanel()
    {
        boolean isWindows = 
            resolveString("$W(stafPlatform.windows)").equals("true");

        outerPanel = new JPanel();
        outerPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));        
        outerPanel.setBorder(new BevelBorder(BevelBorder.LOWERED));        

        JPanel optionsPanel = new JPanel();                
        optionsPanel.setLayout(new BoxLayout(optionsPanel, BoxLayout.Y_AXIS));

        JPanel titlePanel = new JPanel();
        titlePanel.setLayout(new BorderLayout());
        titlePanel.add(BorderLayout.WEST, 
            new Label("Please Choose the options that you want.", 
            SwingConstants.LEFT));

        optionsPanel.add(titlePanel);
                         
        optionsPanel.add(Box.createVerticalStrut(15));        
        
        String envText = "Make STAF available to all users";
        
        if (resolveString("$W(stafPlatform.linux)").equals("true"))
        {
            if (resolveString("$W(adminOrRoot.isAdminOrRoot)").equals("true"))
            {
                envText = "Update System Environment";
            }
            else
            {
                envText = "Update User Environment";
            }
        }
        
        if (resolveString("$W(stafPlatform.aix)").equals("true"))
        {            
            envText = "Update System Environment";
        }

        JPanel allUsersPanel = new JPanel();
        allUsersPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));                        
        availableToAllUsersCB = new 
            JCheckBox(envText, getMakeAvailableToAllUsers());
        availableToAllUsersCB.addActionListener(this);
        allUsersPanel.add(Box.createHorizontalStrut(10));
        allUsersPanel.add(availableToAllUsersCB);
        optionsPanel.add(allUsersPanel);

        if (isWindows)
        {                        
            JPanel startSTAFPanel = new JPanel();
            startSTAFPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            startSTAFCB = new JCheckBox("Start STAF on user login", 
                getStartSTAFOnLogin());
            startSTAFCB.addActionListener(this);
            startSTAFPanel.add(Box.createHorizontalStrut(10));
            startSTAFPanel.add(startSTAFCB);
            optionsPanel.add(startSTAFPanel);
            
            JPanel iconsOnStartProgramsMenuPanel = new JPanel();
            iconsOnStartProgramsMenuPanel.setLayout(new 
                FlowLayout(FlowLayout.LEFT, 0, 0));
            iconsOnStartProgramsMenuCB = new 
                JCheckBox("Place Icons on the Start Programs Menu", 
                getIconsOnStartProgramsMenu());
            iconsOnStartProgramsMenuCB.addActionListener(this);
            iconsOnStartProgramsMenuPanel.add(Box.createHorizontalStrut(10));
            iconsOnStartProgramsMenuPanel.add(iconsOnStartProgramsMenuCB);
            optionsPanel.add(iconsOnStartProgramsMenuPanel);
            
            JPanel iconsOnQuickStartMenuPanel = new JPanel();
            iconsOnQuickStartMenuPanel.setLayout(new 
                FlowLayout(FlowLayout.LEFT, 0, 0));
            iconsOnQuickStartMenuCB = new 
                JCheckBox("Place Icons on the Quick Start Menu", 
                getIconsOnQuickStartMenu());
            iconsOnQuickStartMenuCB.addActionListener(this);
            iconsOnQuickStartMenuPanel.add(Box.createHorizontalStrut(10));
            iconsOnQuickStartMenuPanel.add(iconsOnQuickStartMenuCB);
            optionsPanel.add(iconsOnQuickStartMenuPanel);
        }
        
        JPanel allowRegistrationPanel = new JPanel();
        allowRegistrationPanel .setLayout(new 
            FlowLayout(FlowLayout.LEFT, 0, 0));
        allowRegistrationCB = new 
            JCheckBox("Allow STAF to Register", getAllowSTAFToRegister());        
        allowRegistrationCB.addActionListener(this);
        allowRegistrationPanel.add(Box.createHorizontalStrut(10));
        allowRegistrationPanel.add(allowRegistrationCB);        
        optionsPanel.add(allowRegistrationPanel);
        
        JPanel registrationPanel = new JPanel();
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc = new GridBagConstraints();
        registrationPanel.setLayout(gbl);
        
        JLabel nameLabel = new JLabel("Name:");
        nameLabel.setOpaque(true);
        nameLabel.setForeground(Color.black);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        registrationPanel.add(nameLabel, gbc);
        registrationPanel.add(Box.createHorizontalStrut(10), gbc);

        nameTF = new JTextField(20);
        nameTF.setText(getRegistrationName());
        nameTF.setEnabled(allowSTAFToRegister);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        registrationPanel.add(nameTF, gbc);
        
        gbc.weightx = 0;
        registrationPanel.add(Box.createVerticalStrut(3), gbc);
        
        JLabel emailLabel = new JLabel("Email:");
        emailLabel.setOpaque(true);
        emailLabel.setForeground(Color.black);
        
        gbc.gridwidth = 1;
        registrationPanel.add(emailLabel, gbc);
        registrationPanel.add(Box.createHorizontalStrut(10), gbc);
        
        emailTF = new JTextField(20);
        emailTF.setText(getRegistrationEmail());
        emailTF.setEnabled(allowSTAFToRegister);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        registrationPanel.add(emailTF, gbc);
        
        gbc.weightx = 0;
        registrationPanel.add(Box.createVerticalStrut(3), gbc);
        
        JLabel organizationLabel = new JLabel("Organization:");
        organizationLabel.setOpaque(true);
        organizationLabel.setForeground(Color.black);
        
        gbc.gridwidth = 1;
        registrationPanel.add(organizationLabel, gbc);
        registrationPanel.add(Box.createHorizontalStrut(10), gbc);
        
        organizationTF = new JTextField(20);
        organizationTF.setText(getRegistrationOrganization());
        organizationTF.setEnabled(allowSTAFToRegister);
        
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
            resolveString("$W(stafPlatform.linux)").equals("true"))
        {            
            perl56RB = new JRadioButton("Perl 5.6", 
                getDefaultPerlVersion().equals("5.6"));
            perl50RB = new JRadioButton("Perl 5.005.3",
                getDefaultPerlVersion().equals("5.005.3"));
            perl56RB.addActionListener(this);
            perl50RB.addActionListener(this);
            ButtonGroup perlBG = new ButtonGroup();
            perlBG.add(perl56RB);
            perlBG.add(perl50RB);
            
            JPanel perlVersionPanel = new JPanel();
            perlVersionPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            perlVersionPanel.add(Box.createHorizontalStrut(10));
            JLabel perlLabel = new JLabel("Default Perl Support:");
            perlLabel.setOpaque(true);
            perlLabel.setForeground(Color.black);
            perlVersionPanel.add(perlLabel);
            perlVersionPanel.add(Box.createHorizontalStrut(10));
            perlVersionPanel.add(perl56RB);
            perlVersionPanel.add(Box.createHorizontalStrut(5));
            perlVersionPanel.add(perl50RB);
            optionsPanel.add(Box.createVerticalStrut(10));
            optionsPanel.add(perlVersionPanel);
        } 
                         
        outerPanel.add(optionsPanel);        
    }*/       
    
    public boolean getAllowSTAFToRegister()
    {
        return allowSTAFToRegister;     
    }
    
    public void setAllowSTAFToRegister(boolean bool)
    {       
        allowSTAFToRegister = bool;
    }   
    
    public String getUpdateEnvironmentVariables()
    {
        return updateEnvironmentVariables;
    }
    
    public void setUpdateEnvironmentVariables(String str)
    {       
        updateEnvironmentVariables = str;
    }
    
    public boolean getStartSTAFOnLogin()
    {
        return startSTAFOnLogin;     
    }
    
    public void setStartSTAFOnLogin(boolean bool)
    {       
        startSTAFOnLogin = bool;
    }
    
    public boolean getIconsOnStartProgramsMenu()
    {
        return iconsOnStartProgramsMenu;
    }
    
    public void setIconsOnStartProgramsMenu(boolean bool)
    {       
        iconsOnStartProgramsMenu = bool;
    }
    
    public boolean getIconsOnQuickStartMenu()
    {
        return iconsOnQuickStartMenu;
    }
    
    public void setIconsOnQuickStartMenu(boolean bool)
    {       
        iconsOnQuickStartMenu = bool;
    }
  
    public String getDefaultJavaVersion()
    {
        return defaultJavaVersion;     
    }
    
    public void setDefaultJavaVersion(String str)
    {       
        defaultJavaVersion = str;
    }      
    
    public String getDefaultPerlVersion()
    {
        return defaultPerlVersion;     
    }
    
    public void setDefaultPerlVersion(String str)
    {       
        defaultPerlVersion = str;
    }
    
    public String getDefaultIPvVersion()
    {
        return defaultIPvVersion;     
    }
    
    public void setDefaultIPvVersion(String str)
    {       
        defaultIPvVersion = str;
    } 
    
    public String getRegistrationName()
    {
        return registrationName;     
    }
    
    public void setRegistrationName(String str)
    {       
        registrationName = str;
    }
    
    public String getRegistrationEmail()
    {
        return registrationEmail;     
    }
    
    public void setRegistrationEmail(String str)
    {       
        registrationEmail = str;
    }
    
    public String getRegistrationOrganization()
    {
        return registrationOrganization;     
    }
    
    public void setRegistrationOrganization(String str)
    {       
        registrationOrganization = str;
    }
    
    public String getOptionsSummary()
    {
        return optionsSummary;     
    }
    
    public void setOptionsSummary(String str)
    {       
        optionsSummary = str;
    }
    
    public String getJavaPath()
    {
        if (defaultJavaVersion.equals("1.2+"))
        {
            return "java12";
        }
        else
        {
            return "java11";
        }
    }
    
    public void setJavaPath(String str)
    {       
        
    }
}