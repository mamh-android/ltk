/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import com.zerog.awt.ZGStandardDialog;
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

public class STAFAdvancedUnixOptions extends CustomCodePanel
                                     implements ActionListener
{
    private boolean inited = false;
    private JComboBox envMenuCB;                 
    private JComboBox ipvCB;
    private JComboBox perlCB;
    private JComboBox pythonCB;
    private JComboBox tclCB;
    private JCheckBox usePerlSystemPathCB;
    private JCheckBox usePythonSystemPathCB;
    private JCheckBox useTclSystemPathCB;
    private JTextField instanceNameTF;
    private CustomCodePanelProxy ccpp;

    public boolean setupUI(CustomCodePanelProxy ccpp)
    {
        this.ccpp = ccpp;

        if (inited)
        {
            return true;
        }

        inited = true;

        String osname = ccpp.substitute("$prop.os.name$");
        String osarch = ccpp.substitute("$prop.os.arch$");

        GUIAccess gui =
            (GUIAccess)ccpp.getService(com.zerog.ia.api.pub.GUIAccess.class);
        Frame f = gui.getFrame();
        Font frameFont = f.getFont();
        Font boldFrameFont = new Font(frameFont.getName(), Font.BOLD,
                                      frameFont.getSize());

        JPanel outerPanel = new JPanel();
        outerPanel.setLayout(new BorderLayout());

        JPanel titlePanel = new JPanel();
        titlePanel.setBorder(BorderFactory.createLineBorder(Color.DARK_GRAY));
        titlePanel.setLayout(new BorderLayout());
        JLabel titleLabel = new
            JLabel(" Please choose the options that you want.",
            SwingConstants.LEFT);
        titleLabel.setFont(frameFont);
        titlePanel.add(BorderLayout.NORTH, titleLabel);
        titlePanel.setBackground(Color.white);
        titlePanel.setMinimumSize(new Dimension(300, 200));
        titlePanel.setPreferredSize(new Dimension(300, 75));
        titlePanel.setMaximumSize(new Dimension(500, 200));

        outerPanel.add(titlePanel, BorderLayout.NORTH);

        JPanel optionsPanel = new JPanel();
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc = new GridBagConstraints();
        optionsPanel.setLayout(gbl);
        optionsPanel.setBackground(Color.WHITE);

        JLabel envMenuTitle = new JLabel("Update Environment/Menus for:");
        envMenuTitle.setForeground(Color.black);
        envMenuTitle.setFont(boldFrameFont);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        optionsPanel.add(envMenuTitle, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        envMenuCB = new JComboBox();
        envMenuCB.setFont(frameFont);
        envMenuCB.addItem("System");
        envMenuCB.addItem("User");
        envMenuCB.addItem("None");
        envMenuCB.setSelectedIndex(0);
        envMenuCB.addActionListener(this);

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(envMenuCB, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(5), gbc);

        JLabel ipvLabel = new JLabel("Default TCP version:");
        ipvLabel.setForeground(Color.black);
        ipvLabel.setFont(boldFrameFont);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        optionsPanel.add(ipvLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        ipvCB = new JComboBox();
        ipvCB.setFont(frameFont);
        ipvCB.addItem("IPV4");
        ipvCB.addItem("IPV4_IPV6 (requires OS IPv6 support)");
        ipvCB.setSelectedIndex(0);
        ipvCB.addActionListener(this);

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(ipvCB, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(5), gbc);

        JLabel perlLabel = new JLabel("Default Perl version:");
        perlLabel.setForeground(Color.black);
        perlLabel.setFont(boldFrameFont);

        perlCB = new JComboBox();
        perlCB.setFont(frameFont);
        perlCB.addItem("Perl 5.8");

        if (!((osname.indexOf("Linux") > -1) && (osarch.equals("ia64"))))
        {
            perlCB.addItem("Perl 5.10");
            
            // Currently for Unix, only support Perl 5.12 and 5.14 on Linux
            // x86 and amd64
            if (osname.indexOf("Linux") > -1)
            {
                perlCB.addItem("Perl 5.12");
                perlCB.addItem("Perl 5.14");
            }
        }

        if (osarch.equals("x86"))
        {
            perlCB.addItem("Perl 5.6");
        }

        perlCB.setSelectedIndex(0);
        perlCB.addActionListener(this);

        usePerlSystemPathCB = new
            JCheckBox("Use Perl version in System Path",
            false);
        usePerlSystemPathCB.setFont(frameFont);
        usePerlSystemPathCB.setBackground(Color.WHITE);

        if (((osname.indexOf("Linux") > -1) && (osarch.equals("x86"))) ||
            ((osname.indexOf("Linux") > -1) && (osarch.equals("amd64"))) ||
            ((osname.indexOf("Linux") > -1) && (osarch.equals("ia64"))) ||
            (osname.indexOf("AIX") > -1) ||
            ((osname.indexOf("SunOS") > -1) && (osarch.equals("sparc"))) ||
            (osname.indexOf("Mac OS X") > -1))
        {
            gbc.anchor = GridBagConstraints.NORTHWEST;
            optionsPanel.add(perlLabel, gbc);
            optionsPanel.add(Box.createHorizontalStrut(10), gbc);

            JPanel perlPanel = new JPanel();
            perlPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            perlPanel.setBackground(Color.WHITE);
            perlPanel.add(perlCB);
            perlPanel.add(Box.createHorizontalStrut(15));
            perlPanel.add(usePerlSystemPathCB);

            optionsPanel.add(perlPanel, gbc);

            gbc.weightx = 0;
            optionsPanel.add(Box.createVerticalStrut(5), gbc);
        }

        JLabel pythonLabel = new JLabel("Default Python version:");
        pythonLabel.setForeground(Color.black);
        pythonLabel.setFont(boldFrameFont);

        if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
            (osname.indexOf("FreeBSD") > -1) ||
            (osname.indexOf("SunOS") > -1))
        {
            gbc.anchor = GridBagConstraints.NORTHWEST;
            optionsPanel.add(pythonLabel, gbc);
            optionsPanel.add(Box.createHorizontalStrut(10), gbc);
        }

        pythonCB = new JComboBox();
        pythonCB.setFont(frameFont);

        if (osname.indexOf("Mac OS X") > -1)
        {
            // XXX When Bug #2115056 (InstallAnywhere variable
            // $EXTRACTOR_EXECUTABLE$ is blank on Mac OS X) is fixed, we can
            // check to see if the filename includes the text "universal",
            // and only display the Python versions supported for the installer
            pythonCB.addItem("Python 2.3 (supported on macosx-i386, macosx-ppc)");
            pythonCB.addItem("Python 2.6 (supported on macosx-i386, macosx-ppc)");
            pythonCB.addItem("Python 2.7 (supported on macosx-universal)");
            pythonCB.addItem("Python 3.1 (supported on macosx-universal)");
        }
        else
        {
            pythonCB.addItem("Python 2.2");
            pythonCB.addItem("Python 2.3");
            pythonCB.addItem("Python 2.4");
            pythonCB.addItem("Python 2.5");
            pythonCB.addItem("Python 2.6");
            pythonCB.addItem("Python 2.7");
            pythonCB.addItem("Python 3.0");
            pythonCB.addItem("Python 3.1");
        }

        if (osname.indexOf("FreeBSD") > -1)
        {
            pythonCB.setSelectedIndex(2);
        }
        else
        {
            pythonCB.setSelectedIndex(0);
        }

        pythonCB.addActionListener(this);

        usePythonSystemPathCB = new
            JCheckBox("Use Python version in System Path",
            false);
        usePythonSystemPathCB.setFont(frameFont);
        usePythonSystemPathCB.setBackground(Color.WHITE);

        if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
            (osname.indexOf("FreeBSD") > -1) ||
            (osname.indexOf("SunOS") > -1) ||
            (osname.indexOf("Mac OS X") > -1))
        {
            JPanel pythonPanel = new JPanel();
            pythonPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            pythonPanel.setBackground(Color.WHITE);
            pythonPanel.add(pythonCB);
            pythonPanel.add(Box.createHorizontalStrut(15));
            pythonPanel.add(usePythonSystemPathCB);

            optionsPanel.add(pythonPanel, gbc);

            gbc.weightx = 0;
            optionsPanel.add(Box.createVerticalStrut(5), gbc);
        }

        tclCB = new JComboBox();
        tclCB.setFont(frameFont);

        useTclSystemPathCB = new
            JCheckBox("Use TCL version in System Path",
            false);
        useTclSystemPathCB.setFont(frameFont);
        useTclSystemPathCB.setBackground(Color.WHITE);

        if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")))
        {
            JLabel tclLabel = new JLabel("Default TCL version:");
            tclLabel.setForeground(Color.black);
            tclLabel.setFont(boldFrameFont);

            gbc.anchor = GridBagConstraints.NORTHWEST;
            optionsPanel.add(tclLabel, gbc);
            optionsPanel.add(Box.createHorizontalStrut(10), gbc);

            tclCB.addItem("TCL 8.4");
            tclCB.addItem("TCL 8.5");
            tclCB.addItem("TCL 8.6");

            tclCB.setSelectedIndex(0);
            tclCB.addActionListener(this);

            JPanel tclPanel = new JPanel();
            tclPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            tclPanel.setBackground(Color.WHITE);
            tclPanel.add(tclCB);
            tclPanel.add(Box.createHorizontalStrut(15));
            tclPanel.add(useTclSystemPathCB);

            optionsPanel.add(tclPanel, gbc);

            gbc.weightx = 0;
            optionsPanel.add(Box.createVerticalStrut(5), gbc);
        }

        JLabel instanceNameLabel = new JLabel("Default STAF Instance Name:");
        instanceNameLabel.setForeground(Color.black);
        instanceNameLabel.setFont(boldFrameFont);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        optionsPanel.add(instanceNameLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        instanceNameTF = new JTextField("STAF", 30);

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(instanceNameTF, gbc);
        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(5), gbc);

        JScrollPane sp = new JScrollPane(optionsPanel);
        sp.setPreferredSize(new Dimension(450, 180));
        sp.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        sp.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        outerPanel.add(sp, BorderLayout.CENTER);

        add(outerPanel);

       return true;
    }

    public void panelIsDisplayed()
    {
    }

    public boolean okToContinue()
    {
        String stafInstanceName = instanceNameTF.getText();

        if (!stafInstanceName.equals("STAF"))
        {
            // Verify that the STAF Instance Name specified is valid.
            // A STAF instance name is invalid if it contains any of the
            // following special characters: ~!#$%^&*+={}[]|;':"?/<>\
            // or if it contains any whitespace at the beginning or end
            // or if it only contains whitespace.

            String errorMsg = "";
            String trimmedInstanceName = stafInstanceName.trim();
            
            if (trimmedInstanceName.length() == 0)
            {
                errorMsg = "The \"Default STAF Instance Name\" cannot " +
                    "be empty or just spaces." +
                    "\n\nResetting it to its default value, STAF.";
            }
            else if (trimmedInstanceName != stafInstanceName)
            {
                errorMsg = "The \"Default STAF Instance Name\" cannot " +
                    "contain any leading or trsiling whitespace." +
                    "\n\nResetting it to its default value, STAF.";
            }
            else
            {
                boolean found = false;
                char[] invalidChars = new char[] {
                    '~', '!', '#', '$', '%', '^', '&', '*', '+', '=', '{',
                    '}', '[', ']', '|', ';', '\'', ':', '"', '?', '/',
                    '<', '>', '\\' };
                
                for (int i = 0; i < stafInstanceName.length() && !found; i++)
                {
                    char ch = stafInstanceName.charAt(i);
                    
                    for (int j = 0; j < invalidChars.length && !found; j++)
                    {
                        if (invalidChars[j] == ch)
                        {
                            found = true;
                            errorMsg = "The \"Default STAF Instance Name\" " +
                                "cannot contain any of the following " +
                                " characters: ~!#$%^&*+={}[]|;':\"?/<>\\" +
                                "\n\nResetting it to its default value, STAF.";
                        }
                    }
                }
            }

            if (errorMsg.length() != 0)
            {
                // Display an error message

                // Get the current frame using the GUIAccess class and API
                GUIAccess gui = (GUIAccess)ccpp.getService(GUIAccess.class);
                Frame f = gui.getFrame();
                String errorLabel = "Invalid STAF Instance Name";

                // Setup the error dialog and display the error dialog
                ZGStandardDialog zgDialog = new ZGStandardDialog(
                    f, errorLabel, errorMsg);
                zgDialog.setModal(true);
                zgDialog.show();

                // Reset STAF Instance Name to default value
                instanceNameTF.setText("STAF");

                // Return false to prevent them from continuing
                return false;
            }
        }

        // Set variable STAF_INSTANCE_NAME
        
        ccpp.setVariable("$STAF_INSTANCE_NAME$", stafInstanceName);

        // Set variable UPDATE_ENVIRONMENT
        
        String envMenuSelection = (String)(envMenuCB.getSelectedItem());

        ccpp.setVariable("$UPDATE_ENVIRONMENT$", envMenuSelection);

        // Set variable USE_TCP_VERSION
        
        String ipvSelection = (String)(ipvCB.getSelectedItem());

        if (ipvSelection.equals("IPV4"))
        {
            ccpp.setVariable("$USE_TCP_VERSION$", "IPV4");
        }
        else if (ipvSelection.equals("IPV4_IPV6 (requires OS IPv6 support)"))
        {
            ccpp.setVariable("$USE_TCP_VERSION$", "IPV4_IPV6");
        }

        // Set variable USE_PERL_VERSION to the selected version in the
        // JComboBox if Perl support is provided for the operating system
        
        if (perlCB.getSelectedIndex() > -1)
        {            
            String perlSelection = (String)(perlCB.getSelectedItem());
 
            if (perlSelection.equals("Perl 5.8"))
            {
                ccpp.setVariable("$USE_PERL_VERSION$", "5.8");
            }
            else if (perlSelection.equals("Perl 5.6"))
            {
                ccpp.setVariable("$USE_PERL_VERSION$", "5.6");
            }
            else if (perlSelection.equals("Perl 5.10"))
            {
                ccpp.setVariable("$USE_PERL_VERSION$", "5.10");
            }
            else if (perlSelection.equals("Perl 5.12"))
            {
                ccpp.setVariable("$USE_PERL_VERSION$", "5.12");
            }
            else if (perlSelection.equals("Perl 5.14"))
            {
                ccpp.setVariable("$USE_PERL_VERSION$", "5.14");
            }
        }
        
        // Set variable USE_PERL_SYSTEM_PATH
        
        if (usePerlSystemPathCB.isSelected())
        {
            ccpp.setVariable("$USE_PERL_SYSTEM_PATH$", "1");
        }
        else
        {
            ccpp.setVariable("$USE_PERL_SYSTEM_PATH$", "0");
        }
        
        // Set variable USE_PYTHON_VERSION to the selected version in the
        // JComboBox if Python support is provided for the operating system
        
        if (pythonCB.getSelectedIndex() > -1)
        {
            String pythonSelection = (String)(pythonCB.getSelectedItem());

            if (pythonSelection.startsWith("Python 2.2"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "2.2");
            }
            else if (pythonSelection.startsWith("Python 2.3"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "2.3");
            }
            else if (pythonSelection.startsWith("Python 2.4"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "2.4");
            }
            else if (pythonSelection.startsWith("Python 2.5"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "2.5");
            }
            else if (pythonSelection.startsWith("Python 2.6"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "2.6");
            }
            else if (pythonSelection.startsWith("Python 2.7"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "2.7");
            }
            else if (pythonSelection.startsWith("Python 3.0"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "3.0");
            }
            else if (pythonSelection.startsWith("Python 3.1"))
            {
                ccpp.setVariable("$USE_PYTHON_VERSION$", "3.1");
            }
        }
        
        // Set variable USE_PYTHON_SYSTEM_PATH
        
        if (usePythonSystemPathCB.isSelected())
        {
            ccpp.setVariable("$USE_PYTHON_SYSTEM_PATH$", "1");
        }
        else
        {
            ccpp.setVariable("$USE_PYTHON_SYSTEM_PATH$", "0");
        }
        
        // Set variable USE_TCL_VERSION to the selected version in the
        // JComboBox if Tcl support is provided for the operating system
        
        if (tclCB.getSelectedIndex() > -1)
        {
            String tclSelection = (String)(tclCB.getSelectedItem());

            if (tclSelection.equals("TCL 8.4"))
            {
                ccpp.setVariable("$USE_TCL_VERSION$", "8.4");
            }
            else if (tclSelection.equals("TCL 8.5"))
            {
                ccpp.setVariable("$USE_TCL_VERSION$", "8.5");
            }
            else if (tclSelection.equals("TCL 8.6"))
            {
                ccpp.setVariable("$USE_TCL_VERSION$", "8.6");
            }
        }
                    
        // Set variable USE_TCL_SYSTEM_PATH
        
        if (useTclSystemPathCB.isSelected())
        {
            ccpp.setVariable("$USE_TCL_SYSTEM_PATH$", "1");
        }
        else
        {
            ccpp.setVariable("$USE_TCL_SYSTEM_PATH$", "0");
        }
        
        return true;
    }

    public boolean okToGoPrevious()
    {
        return true;
    }

    public String getTitle()
    {
        return "Advanced Options";
    }

    public void actionPerformed(ActionEvent e)
    {
        
    }
}