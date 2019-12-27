/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.eventmanager;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.plaf.metal.*;
import com.ibm.staf.*;
import com.ibm.staf.service.*;

public class EventManagerUI extends JFrame implements ActionListener,
                                                      MouseListener,
                                                      KeyListener
{
    JMenuBar fMenuBar;
    JMenu fFileMenu;
    JMenuItem fFileExit;
    JMenuItem fFileNew;
    JMenu fViewMenu;
    JMenuItem fViewRefresh;
    JMenuItem fViewServiceLogLast100;
    JMenuItem fViewServiceLogForID;
    JMenuItem fViewServiceLogForSubmittedCommand;
    JMenuItem fViewEntireServiceLog;
    JMenuItem fViewDeleteServiceLog;
    JMenu fSelectedMenu;
    JMenuItem fSelectedEdit;
    JMenuItem fSelectedEnable;
    JMenuItem fSelectedDisable;
    JMenuItem fSelectedTrigger;
    JMenuItem fSelectedTriggerWithScript;
    JMenuItem fSelectedCopyToNewRegistration;
    JMenuItem fSelectedUnregister;
    JMenu fHelpMenu;
    JMenuItem fHelpAbout;
    JLabel fInstructionLabel;
    JLabel fRegistrationIDLabel;
    JTextField fServiceMachineTF = new JTextField(15);
    JTextField fServiceNameTF = new JTextField(8);
    JTextField fDescriptionTF;
    JTextField fMachineTF;
    JCheckBox fPythonMachine;
    JTextField fServiceTF;
    JCheckBox fPythonService;
    JTextField fRequestTF;
    JCheckBox fPythonRequest;
    JTextArea fPrepareTA;
    JTextField fTypeTF;
    JTextField fSubtypeTF;
    JCheckBox fEnabledCB;
    JButton fRegisterButton;
    JButton fCancelButton;
    JDialog fRegistrationDialog;
    java.util.List fEntryList;
    JTable fIDTable;
    Vector<String> fColumnNames;
    STAFHandle fHandle;
    JPopupMenu fPopupMenu = new JPopupMenu();
    JMenuItem fPopupMenuEdit = new JMenuItem("Edit");
    JMenuItem fPopupMenuEnable = new JMenuItem("Enable");
    JMenuItem fPopupMenuDisable = new JMenuItem("Disable");
    JMenuItem fPopupMenuTrigger = new JMenuItem("Trigger");
    JMenuItem fPopupMenuTriggerWithScript =
        new JMenuItem("Trigger with script...");
    JMenuItem fPopupMenuCopyToNewRegistration =
        new JMenuItem("Copy to new registration");
    JMenuItem fPopupMenuUnregister = new JMenuItem("Unregister");
    JTextArea fScriptTextArea;
    JButton fScriptTriggerButton;
    JButton fScriptCancelButton;
    JDialog fScriptDialog;
    JDialog fStafCommandsDialog;
    JComboBox fStafCommands;
    JButton fStafCommandsOkButton;
    JButton fStafCommandsCancelButton;
    
    String fServiceMachine = "";
    String fServiceName = "";
    String fLogName = "";

    public EventManagerUI()
    {
        try
        {
            fHandle = new STAFHandle("STAF/EventManager/UI");
        }
        catch (STAFException e)
        {
            System.out.println("STAF must be running to execute " +
                               "EventManagerUI");
            System.exit(0);
        }

        UIManager.put("Table.focusCellHighlightBorder", "none");

        fServiceMachineTF.setText("local");
        fServiceNameTF.setText("EventManager");

        fServiceMachine = "local";
        fServiceName = "EventManager";
        assignLogName();

        Vector idData = getRegistrationTable();

        JPanel idPanel = new JPanel();
        idPanel.setLayout(new BorderLayout());

        fMenuBar = new JMenuBar();
        fFileMenu = new JMenu("File");
        fFileNew = new JMenuItem("New Registration...");
        fFileMenu.add(fFileNew);
        fFileNew.addActionListener(this);
        fFileExit = new JMenuItem("Exit");
        fFileMenu.add(fFileExit);
        fFileExit.addActionListener(this);
        fMenuBar.add(fFileMenu);

        fViewMenu = new JMenu("View");
        fViewRefresh = new JMenuItem("Refresh");
        fViewMenu.add(fViewRefresh);
        fViewRefresh.addActionListener(this);
        fViewMenu.insertSeparator(3);
        fViewServiceLogForID = new JMenuItem("Service Log for ID...");
        fViewMenu.add(fViewServiceLogForID);
        fViewServiceLogForID.addActionListener(this);
        fViewServiceLogLast100 = new JMenuItem("Service Log last 100 records");
        fViewMenu.add(fViewServiceLogLast100);
        fViewServiceLogLast100.addActionListener(this);
        fViewServiceLogForSubmittedCommand = new
            JMenuItem("Service Log for submitted STAF command...");
        fViewMenu.add(fViewServiceLogForSubmittedCommand);
        fViewServiceLogForSubmittedCommand.addActionListener(this);
        fViewEntireServiceLog = new JMenuItem("Entire Service Log");
        fViewMenu.add(fViewEntireServiceLog);
        fViewEntireServiceLog.addActionListener(this);
        fViewDeleteServiceLog = new JMenuItem("Delete Service Log");
        fViewMenu.add(fViewDeleteServiceLog);
        fViewDeleteServiceLog.addActionListener(this);
        fMenuBar.add(fViewMenu);

        fSelectedMenu = new JMenu("Selected");
        fSelectedMenu.setEnabled(false);
        fSelectedEdit = new JMenuItem("Edit");
        fSelectedMenu.add(fSelectedEdit);
        fSelectedEdit.addActionListener(this);
        fSelectedMenu.insertSeparator(1);
        fSelectedEnable = new JMenuItem("Enable");
        fSelectedMenu.add(fSelectedEnable);
        fSelectedEnable.addActionListener(this);
        fSelectedDisable = new JMenuItem("Disable");
        fSelectedMenu.add(fSelectedDisable);
        fSelectedDisable.addActionListener(this);
        fSelectedMenu.insertSeparator(5);
        fSelectedTrigger = new JMenuItem("Trigger");
        fSelectedMenu.add(fSelectedTrigger);
        fSelectedTrigger.addActionListener(this);
        fSelectedTriggerWithScript = new JMenuItem("Trigger with script...");
        fSelectedMenu.add(fSelectedTriggerWithScript);
        fSelectedTriggerWithScript.addActionListener(this);
        fSelectedMenu.insertSeparator(9);
        fSelectedCopyToNewRegistration =
            new JMenuItem("Copy to new registration");
        fSelectedMenu.add(fSelectedCopyToNewRegistration);
        fSelectedCopyToNewRegistration.addActionListener(this);
        fSelectedMenu.insertSeparator(12);
        fSelectedUnregister = new JMenuItem("Unregister");
        fSelectedMenu.add(fSelectedUnregister);
        fSelectedUnregister.addActionListener(this);
        fMenuBar.add(fSelectedMenu);

        fHelpMenu = new JMenu("Help");
        fHelpAbout = new JMenuItem("About");
        fHelpMenu.add(fHelpAbout);
        fHelpAbout.addActionListener(this);
        fMenuBar.add(fHelpMenu);

        JPanel servicePanel = new JPanel();
        servicePanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        JLabel serviceMachineLabel = new JLabel("Service Machine:");
        serviceMachineLabel.setOpaque(true);
        serviceMachineLabel.setForeground(Color.black);
        servicePanel.add(serviceMachineLabel);

        servicePanel.add(fServiceMachineTF);
        fServiceMachineTF.addKeyListener(this);

        JLabel serviceNameLabel = new JLabel("Service Name:");
        serviceNameLabel.setOpaque(true);
        serviceNameLabel.setForeground(Color.black);
        servicePanel.add(serviceNameLabel);

        servicePanel.add(fServiceNameTF);
        fServiceNameTF.addKeyListener(this);

        idPanel.add(BorderLayout.NORTH, servicePanel);

        this.setJMenuBar(fMenuBar);

        fColumnNames = new Vector<String>();
        fColumnNames.add("ID");
        fColumnNames.add("State");
        fColumnNames.add("Description");
        fColumnNames.add("Machine");
        fColumnNames.add("Service");
        fColumnNames.add("Request");

        RegisterModel regModel = new RegisterModel(idData, fColumnNames);
        fIDTable = new JTable();
        fIDTable.setModel(regModel);
        fIDTable.setCellSelectionEnabled(false);
        fIDTable.setRowSelectionAllowed(true);
        fIDTable.addMouseListener(this);
        fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);

        fIDTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
        fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
        fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
        fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
        fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
        fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

        fPopupMenu.add(fPopupMenuEdit);
        fPopupMenuEdit.addActionListener(this);
        fPopupMenu.addSeparator();
        fPopupMenu.add(fPopupMenuEnable);
        fPopupMenuEnable.addActionListener(this);
        fPopupMenu.add(fPopupMenuDisable);
        fPopupMenuDisable.addActionListener(this);
        fPopupMenu.addSeparator();
        fPopupMenu.add(fPopupMenuTrigger);
        fPopupMenuTrigger.addActionListener(this);
        fPopupMenu.add(fPopupMenuTriggerWithScript);
        fPopupMenuTriggerWithScript.addActionListener(this);
        fPopupMenu.addSeparator();
        fPopupMenu.add(fPopupMenuCopyToNewRegistration);
        fPopupMenuCopyToNewRegistration.addActionListener(this);
        fPopupMenu.addSeparator();
        fPopupMenu.add(fPopupMenuUnregister);
        fPopupMenuUnregister.addActionListener(this);

        idPanel.add(BorderLayout.CENTER, new JScrollPane(fIDTable));

        fInstructionLabel = new JLabel();
        setInstructionLabel(idData);
        idPanel.add(BorderLayout.SOUTH, fInstructionLabel);

        getContentPane().add(idPanel);
        setTitle("EventManagerUI");
        pack();
        setSize(new Dimension(750, 450));

        setVisible(true);

        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                System.exit(0);
            }
        });
    }

    public void setInstructionLabel(Vector idData)
    {
        if (idData == null)
        {
            fInstructionLabel.setText("Enter a valid Service Machine" +
                " and Service Name");
        }
        else if (idData.size() == 0)
        {
            fInstructionLabel.setText("Click on File -> New Registration..." +
                " in the menu bar to register a new STAF command");
        }
        else
        {
            fInstructionLabel.setText("Double click on an existing " +
                "registration to edit");
        }
    }

    public static void main(String argv[])
    {
        new EventManagerUI();
    }

    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == fFileExit)
        {
            System.exit(0);
        }
        else if (e.getSource() == fFileNew)
        {
            displayfRegistrationDialog();
            fRegistrationDialog.setVisible(true);
        }
        else if (e.getSource() == fHelpAbout)
        {
            STAFResult res = fHandle.submit2(
                fServiceMachine, fServiceName, "VERSION");

            if (res.rc == 0)
            {
                JOptionPane.showMessageDialog(
                    this, fServiceName.toUpperCase() + " version " +
                    res.result, fServiceName.toUpperCase() + " VERSION " +
                    res.result, JOptionPane.INFORMATION_MESSAGE);
            }
            else
            {
                System.out.println(
                    "Error service version " +
                    "on machine " + fServiceMachine + ".  RC=" + res.rc +
                    ", Result=" + res.result);
            }
        }
        else if (e.getSource() == fViewRefresh)
        {
            Vector idData = getRegistrationTable();
            setInstructionLabel(idData);

            ((RegisterModel)(fIDTable.getModel())).setDataVector(
                idData, fColumnNames);

            fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
            fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
            fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
            fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
            fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
            fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
            fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

            fSelectedMenu.setEnabled(false);
        }
        else if (e.getSource() == fViewServiceLogForID)
        {
            String id = JOptionPane.showInputDialog(this,
                "Enter the registration ID", "View Service log for ID",
                JOptionPane.QUESTION_MESSAGE);

            if (id == null) return;

            String queryRequest =
                "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                fLogName + " CONTAINS " +
                STAFUtil.wrapData("[ID=" + id + "]") + " ALL";

            STAFLogViewer logViewer = new STAFLogViewer(
                this, fHandle, fServiceMachine, "LOG", queryRequest, "",
                "Monospaced");
        }
        else if (e.getSource() == fViewServiceLogLast100)
        {
            String queryRequest =
                "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                fLogName + " LAST 100";

            STAFLogViewer logViewer = new STAFLogViewer(
                this, fHandle, fServiceMachine, "LOG", queryRequest, "",
                "Monospaced");
        }
        else if (e.getSource() == fViewServiceLogForSubmittedCommand)
        {
            String queryRequest =
                "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                fLogName + " CONTAINS " +
                STAFUtil.wrapData("] Submitted a STAF command") + " ALL";

            STAFResult stafResult = fHandle.submit2(
                fServiceMachine, "LOG", queryRequest);

            if (stafResult.rc != 0)
            {
                JOptionPane.showMessageDialog(
                    this, "RC=" + stafResult.rc + ", Result=" +
                    stafResult.result,
                    "Error querying log " + fLogName + " on machine " +
                    fServiceMachine, JOptionPane.INFORMATION_MESSAGE);

                return;
            }

            java.util.List outputList;

            try
            {
                STAFMarshallingContext outputContext =
                    STAFMarshallingContext.unmarshall(stafResult.result);

                outputList = (java.util.List)outputContext.getRootObject();
            }
            catch (Exception ex)
            {
                ex.printStackTrace();

                JOptionPane.showMessageDialog(
                    this, "Log " + fLogName + " on machine " +
                    fServiceMachine + " has an invalid format",
                    "Invalid Log Format",
                    JOptionPane.INFORMATION_MESSAGE);

                return;
            }

            Iterator iter = outputList.iterator();
            Vector<String> stafCommands = new Vector<String>();
            int i = 0;

            try
            {
                while (iter.hasNext())
                {
                    i++;
                    Map logRecord = (Map)iter.next();

                    String timestamp = (String)logRecord.get("timestamp");
                    String logMessage = (String)logRecord.get("message");
                    int firstBracketIndex = logMessage.indexOf("]");
                    int secondBracketIndex = logMessage.indexOf("]",
                        firstBracketIndex + 1);
                    String idAndReqNum = logMessage.substring(0,
                        secondBracketIndex + 1);
                    String command = logMessage.substring(
                        logMessage.indexOf("Submitted STAF command: ") + 24);
                    stafCommands.add(0, idAndReqNum + " " + command);
                }
            }
            catch (Exception ex)
            {
                ex.printStackTrace();

                JOptionPane.showMessageDialog(
                    this, "Log " + fLogName + " on machine " +
                    fServiceName + " has an invalid format",
                    "Invalid Log Format in record #" + i,
                    JOptionPane.INFORMATION_MESSAGE);

                return;
            }

            fStafCommandsDialog = new JDialog(this,
                "Select submitted STAF command", true);
            fStafCommandsDialog.setSize(600, 150);
            JPanel stafCommandsPanel = new JPanel();
            stafCommandsPanel.setLayout(new BorderLayout());
            stafCommandsPanel.setBorder(new TitledBorder("Select STAF command"));
            fStafCommandsDialog.getContentPane().add(stafCommandsPanel);

            fStafCommands = new JComboBox(stafCommands);
            fStafCommands.setBackground(Color.white);

            stafCommandsPanel.add(BorderLayout.NORTH, fStafCommands);

            JPanel stafCommandsButtonPanel = new JPanel();
            stafCommandsButtonPanel.setLayout(new
                FlowLayout(FlowLayout.CENTER, 0, 0));

            fStafCommandsOkButton = new JButton("OK");
            fStafCommandsOkButton.addActionListener(this);
            fStafCommandsCancelButton = new JButton("Cancel");
            fStafCommandsCancelButton.addActionListener(this);
            stafCommandsButtonPanel.add(fStafCommandsOkButton);
            stafCommandsButtonPanel.add(Box.createHorizontalStrut(20));
            stafCommandsButtonPanel.add(fStafCommandsCancelButton);

            stafCommandsPanel.add(BorderLayout.SOUTH, stafCommandsButtonPanel);

            fStafCommandsDialog.setLocationRelativeTo(this);

            fStafCommandsDialog.setVisible(true);
        }
        else if (e.getSource() == fStafCommandsCancelButton)
        {
            fStafCommandsDialog.dispose();
        }
        else if (e.getSource() == fStafCommandsOkButton)
        {
            String command = (String)fStafCommands.getSelectedItem();
            int firstBracketIndex = command.indexOf("]");
            int secondBracketIndex = command.indexOf("]",
                firstBracketIndex + 1);
            String idAndReqNum = command.substring(0,
                secondBracketIndex + 1);

            fStafCommandsDialog.dispose();

            String queryRequest =
                "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                fLogName + " CONTAINS " + STAFUtil.wrapData(idAndReqNum) +
                " ALL";

            STAFLogViewer logViewer = new STAFLogViewer(
                this, fHandle, fServiceMachine, "LOG", queryRequest, "",
                "Monospaced");
        }
        else if (e.getSource() == fViewEntireServiceLog)
        {
            String queryRequest =
                "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                fLogName + " ALL";

            STAFLogViewer logViewer = new STAFLogViewer(
                this, fHandle, fServiceMachine, "LOG", queryRequest, "",
                "Monospaced");
        }
        else if (e.getSource() == fViewDeleteServiceLog)
        {
            int response = JOptionPane.showConfirmDialog(this,
                "Are you sure you want to delete the service log?");

            if (response != JOptionPane.YES_OPTION)
            {
                return;
            }

            STAFResult res = fHandle.submit2(
                fServiceMachine, "LOG",
                "DELETE MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                fLogName + " CONFIRM");

            if (res.rc != 0)
            {
                JOptionPane.showMessageDialog(
                    this, "Error deleting the " + fLogName +
                    " log on machine " + fServiceMachine + ".\n",
                    "RC=" + res.rc + ", Result=" + res.result,
                    JOptionPane.ERROR_MESSAGE);
            }
        }
        else if (e.getSource() == fCancelButton)
        {
            fRegistrationDialog.dispose();

            Vector idData = getRegistrationTable();
            setInstructionLabel(idData);

            if (idData == null)
            {
                return;
            }

            ((RegisterModel)(fIDTable.getModel())).setDataVector(
                idData, fColumnNames);

            fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
            fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
            fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
            fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
            fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
            fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
            fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

            fSelectedMenu.setEnabled(false);
        }
        else if (e.getSource() == fSelectedEdit ||
                  e.getSource() == fPopupMenuEdit)
        {
            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            String eventManagerId = "";

            Iterator idIter = fEntryList.iterator();

            while (idIter.hasNext() && !(eventManagerId.equals(selectedId)))
            {
                Map registrationMap = (Map)idIter.next();
                eventManagerId = (String)registrationMap.get("eventManagerID");

                if (eventManagerId.equals(selectedId))
                {
                    String machine = (String)registrationMap.get("machine");
                    String machineType =
                        (String)registrationMap.get("machineType");
                    String service = (String)registrationMap.get("service");
                    String serviceType =
                        (String)registrationMap.get("serviceType");
                    String request = (String)registrationMap.get("request");
                    String requestType =
                        (String)registrationMap.get("requestType");
                    String prepareScript =
                        (String)registrationMap.get("prepareScript");
                    String type = (String)registrationMap.get("type");
                    String subtype = (String)registrationMap.get("subtype");
                    String description =
                        (String)registrationMap.get("description");
                    String state =
                        (String)registrationMap.get("state");

                    displayfRegistrationDialog();

                    fRegistrationIDLabel.setText(eventManagerId);

                    fMachineTF.setText(machine);
                    fPythonMachine.setSelected(machineType.equals("Python"));
                    fServiceTF.setText(service);
                    fPythonService.setSelected(serviceType.equals("Python"));
                    fRequestTF.setText(request);
                    fPythonRequest.setSelected(requestType.equals("Python"));
                    fPrepareTA.setText(prepareScript);
                    fTypeTF.setText(type);
                    fSubtypeTF.setText(subtype);
                    fDescriptionTF.setText(description);

                    if (state.equals("Disabled"))
                    {
                        fEnabledCB.setSelected(false);
                    }
                    else
                    {
                        fEnabledCB.setSelected(true);
                    }

                    fRegistrationDialog.setVisible(true);
                }
            }
        }
        else if (e.getSource() == fSelectedTrigger ||
                 e.getSource() == fPopupMenuTrigger)
        {
            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            STAFResult result = fHandle.submit2(
                fServiceMachine, fServiceName, " TRIGGER ID " + selectedId);

            if (result.rc != 0)
            {
                showErrorDialog(fRegistrationDialog,
                                "An error was encountered while "
                                + "attempting to trigger"
                                + " rc=" + result.rc
                                + " result=" + result.result);
            }
            else
            {
                STAFMarshallingContext mc =
                    STAFMarshallingContext.unmarshall(result.result);

                Map triggerMap = (Map)mc.getRootObject();

                String triggerMachine = (String)triggerMap.get("machine");
                String triggerRequestNumber =
                    (String)triggerMap.get("requestNumber");

                String logQueryContains = "[ID=" + selectedId + "] [" +
                    triggerMachine + ":" + triggerRequestNumber + "]";
                
                try
                {
                    Thread.sleep(250);
                }
                catch(InterruptedException ex) {}

                String queryRequest =
                    "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                    fLogName + " CONTAINS " + logQueryContains + " ALL";
                
                STAFLogViewer logViewer = new STAFLogViewer(
                    this, fHandle, fServiceMachine, "LOG", queryRequest, "",
                    "Monospaced");
            }
        }
        else if (e.getSource() == fSelectedTriggerWithScript ||
                 e.getSource() == fPopupMenuTriggerWithScript)
        {
            fScriptDialog = new JDialog(this, "Trigger Script", true);
            fScriptDialog.setSize(new Dimension(400, 200));
            JPanel scriptPanel = new JPanel();
            scriptPanel.setLayout(new BorderLayout());
            fScriptTextArea = new JTextArea(5, 15);
            fScriptTextArea.setBorder(new TitledBorder("Enter script here"));
            scriptPanel.add(BorderLayout.CENTER,
                new JScrollPane(fScriptTextArea));

            fScriptTriggerButton = new JButton("Trigger");
            fScriptCancelButton = new JButton("Cancel");

            JPanel scriptButtonPanel = new JPanel();
            scriptButtonPanel.setLayout(new
                FlowLayout(FlowLayout.CENTER, 0, 0));
            scriptButtonPanel.add(fScriptTriggerButton);
            scriptButtonPanel.add(Box.createHorizontalStrut(20));
            scriptButtonPanel.add(fScriptCancelButton);

            scriptPanel.add(BorderLayout.SOUTH, scriptButtonPanel);

            fScriptTriggerButton.addActionListener(this);
            fScriptCancelButton.addActionListener(this);

            fScriptDialog.getContentPane().add(scriptPanel);

            fScriptTextArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
            fScriptDialog.setLocationRelativeTo(this);
            fScriptDialog.setFont(new Font("Monospaced", Font.PLAIN, 12));
            fScriptDialog.setVisible(true);
        }
        else if (e.getSource() == fScriptCancelButton)
        {
            fScriptDialog.dispose();
        }
        else if (e.getSource() == fScriptTriggerButton)
        {
            String triggerScript = fScriptTextArea.getText();

            fScriptDialog.dispose();

            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            STAFResult result = fHandle.submit2(
                fServiceMachine, fServiceName,
                " TRIGGER ID " + selectedId + " SCRIPT " +
                STAFUtil.wrapData(triggerScript));

            if (result.rc != 0)
            {
                showErrorDialog(fRegistrationDialog,
                                "An error was encountered while "
                                + "attempting to trigger"
                                + " rc=" + result.rc
                                + " result=" + result.result);
            }
            else
            {
                STAFMarshallingContext mc =
                    STAFMarshallingContext.unmarshall(result.result);

                Map triggerMap = (Map)mc.getRootObject();

                String triggerMachine = (String)triggerMap.get("machine");
                String triggerRequestNumber =
                    (String)triggerMap.get("requestNumber");

                String logQueryContains = "[ID=" + selectedId + "] [" +
                    triggerMachine + ":" + triggerRequestNumber + "]";

                try
                {
                    Thread.sleep(250);
                }
                catch(InterruptedException ex) {}

                String queryRequest =
                    "QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME " +
                    fLogName + " CONTAINS " + logQueryContains + " ALL";

                STAFLogViewer logViewer = new STAFLogViewer(
                    this, fHandle, fServiceMachine, "LOG", queryRequest, "",
                    "Monospaced");
            }
        }
        else if (e.getSource() == fSelectedCopyToNewRegistration ||
                 e.getSource() == fPopupMenuCopyToNewRegistration)
        {
            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            String eventManagerId = "";

            Iterator idIter = fEntryList.iterator();

            while (idIter.hasNext() && !(eventManagerId.equals(selectedId)))
            {
                Map registrationMap = (Map)idIter.next();
                eventManagerId = (String)registrationMap.get("eventManagerID");

                if (eventManagerId.equals(selectedId))
                {
                    String machine = (String)registrationMap.get("machine");
                    String machineType =
                        (String)registrationMap.get("machineType");
                    String service = (String)registrationMap.get("service");
                    String serviceType =
                        (String)registrationMap.get("serviceType");
                    String request = (String)registrationMap.get("request");
                    String requestType =
                        (String)registrationMap.get("requestType");
                    String prepareScript =
                        (String)registrationMap.get("prepareScript");
                    String type = (String)registrationMap.get("type");
                    String subtype = (String)registrationMap.get("subtype");
                    String description =
                        (String)registrationMap.get("description");
                    String state =
                        (String)registrationMap.get("state");

                    displayfRegistrationDialog();

                    fRegistrationIDLabel.setText("N/A");

                    fMachineTF.setText(machine);
                    fPythonMachine.setSelected(machineType.equals("Python"));
                    fServiceTF.setText(service);
                    fPythonService.setSelected(serviceType.equals("Python"));
                    fRequestTF.setText(request);
                    fPythonRequest.setSelected(requestType.equals("Python"));
                    fPrepareTA.setText(prepareScript);
                    fTypeTF.setText(type);
                    fSubtypeTF.setText(subtype);
                    fDescriptionTF.setText(description);

                    if (state.equals("Disabled"))
                    {
                        fEnabledCB.setSelected(false);
                    }
                    else
                    {
                        fEnabledCB.setSelected(true);
                    }

                    fRegistrationDialog.setVisible(true);
                }
            }
        }
        else if (e.getSource() == fSelectedUnregister ||
                 e.getSource() == fPopupMenuUnregister)
        {
            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            int response = JOptionPane.showConfirmDialog(this,
                "Are you sure you want to unregister ID " +
                selectedId + "?");

            if (response != JOptionPane.YES_OPTION)
            {
                return;
            }

            STAFResult result = fHandle.submit2(
                fServiceMachine, fServiceName, "UNREGISTER ID " + selectedId);

            if (result.rc != 0)
            {
                showErrorDialog(fRegistrationDialog,
                                "An error was encountered while "
                                + "attempting to unregister"
                                + " rc=" + result.rc
                                + " result=" + result.result);
            }
            else
            {
                Vector idData = getRegistrationTable();
                setInstructionLabel(idData);

                if (idData == null)
                {
                    return;
                }

                ((RegisterModel)(fIDTable.getModel())).setDataVector(
                    idData, fColumnNames);

                fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
                fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
                fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
                fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
                fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

                JOptionPane.showMessageDialog(this,
                    "Sucessfully unregistered.",
                    "Sucessfully unregistered",
                    JOptionPane.INFORMATION_MESSAGE);

                fSelectedMenu.setEnabled(false);
            }
        }
        else if (e.getSource() == fSelectedEnable ||
                 e.getSource() == fPopupMenuEnable)
        {
            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            STAFResult result = fHandle.submit2(
                fServiceMachine, fServiceName, "ENABLE ID " + selectedId);

            if (result.rc != 0)
            {
                showErrorDialog(fRegistrationDialog,
                                "An error was encountered while "
                                + "attempting to enable"
                                + " rc=" + result.rc
                                + " result=" + result.result);
            }
            else
            {
                Vector idData = getRegistrationTable();
                setInstructionLabel(idData);

                if (idData == null)
                {
                    return;
                }

                ((RegisterModel)(fIDTable.getModel())).setDataVector(
                    idData, fColumnNames);

                fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
                fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
                fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
                fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
                fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

                fSelectedMenu.setEnabled(false);
            }
        }
        else if (e.getSource() == fSelectedDisable ||
                 e.getSource() == fPopupMenuDisable)
        {
            int selectedRow = fIDTable.getSelectedRow();

            if (selectedRow == -1)
            {
                return;
            }

            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            STAFResult result = fHandle.submit2(
                fServiceMachine, fServiceName, "DISABLE ID " + selectedId);

            if (result.rc != 0)
            {
                showErrorDialog(fRegistrationDialog,
                                "An error was encountered while "
                                + "attempting to disable"
                                + " rc=" + result.rc
                                + " result=" + result.result);
            }
            else
            {
                Vector idData = getRegistrationTable();
                setInstructionLabel(idData);

                if (idData == null)
                {
                    return;
                }

                ((RegisterModel)(fIDTable.getModel())).setDataVector(
                    idData, fColumnNames);

                fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
                fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
                fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
                fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
                fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

                fSelectedMenu.setEnabled(false);
            }
        }
        else if (e.getSource() == fRegisterButton)
        {
            String request = "REGISTER";

            if (fMachineTF.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fRegistrationDialog,
                    "You must specify a value for Target Machine",
                    "Target Machine not specified", JOptionPane.ERROR_MESSAGE);
                return;
            }

            if (fServiceTF.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fRegistrationDialog,
                    "You must specify a value for Target Service",
                    "Target Service not specified", JOptionPane.ERROR_MESSAGE);
                return;
            }

            if (fRequestTF.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fRegistrationDialog,
                    "You must specify a value for Target Request",
                    "Target Request not specified", JOptionPane.ERROR_MESSAGE);
                return;
            }

            if (fPythonMachine.isSelected())
            {
                request += " PYTHONMACHINE " +
                    STAFUtil.wrapData(fMachineTF.getText());
            }
            else
            {
                request += " MACHINE " +
                    STAFUtil.wrapData(fMachineTF.getText());
            }

            if (fPythonService.isSelected())
            {
                request += " PYTHONSERVICE " +
                    STAFUtil.wrapData(fServiceTF.getText());
            }
            else
            {
                request += " SERVICE " +
                    STAFUtil.wrapData(fServiceTF.getText());
            }

            if (fPythonRequest.isSelected())
            {
                request += " PYTHONREQUEST " +
                    STAFUtil.wrapData(fRequestTF.getText());
            }
            else
            {
                request += " REQUEST " +
                    STAFUtil.wrapData(fRequestTF.getText());
            }

            if (!(fPrepareTA.getText().equals("")))
            {
                request += " PREPARE " + STAFUtil.wrapData(fPrepareTA.getText());
            }

            if (!(fTypeTF.getText().equals("")))
            {
                request += " TYPE " + STAFUtil.wrapData(fTypeTF.getText());
            }

            if (!(fSubtypeTF.getText().equals("")))
            {
                request += " SUBTYPE " + STAFUtil.wrapData(fSubtypeTF.getText());
            }

            if (!(fDescriptionTF.getText().equals("")))
            {
                request += " DESCRIPTION " +
                    STAFUtil.wrapData(fDescriptionTF.getText());
            }

            if (!(fEnabledCB.isSelected()))
            {
                request += " DISABLED";
            }

            boolean unregisterExistingID = false;
            String unregisterID = "";

            if (!(fRegistrationIDLabel.getText().equals("N/A")))
            {
                int response = JOptionPane.showConfirmDialog(fRegistrationDialog,
                    "Do you want to unregister ID " +
                    fRegistrationIDLabel.getText() + " and re-register the " +
                    "STAF request?");

                if (response != JOptionPane.YES_OPTION)
                {
                    return;
                }

                unregisterExistingID = true;
                unregisterID = fRegistrationIDLabel.getText();
            }

            // Submit the EventManager REGISTER request

            STAFResult result = fHandle.submit2(
                fServiceMachine, fServiceName, request);

            if (result.rc != 0)
            {
                showErrorDialog(fRegistrationDialog,
                                "An error was encountered while "
                                + "attempting to register"
                                + " rc=" + result.rc
                                + " result=" + result.result);
            }
            else
            {
                if (unregisterExistingID)
                {
                    STAFResult unregisterResult = fHandle.submit2(
                        fServiceMachine, fServiceName,
                        "UNREGISTER ID " + unregisterID);

                    if (unregisterResult.rc != 0)
                    {
                        showErrorDialog(fRegistrationDialog,
                                        "An error was encountered while "
                                        + "attempting to unregister ID"
                                        + unregisterID + " rc=" + result.rc
                                        + " result=" + result.result);
                    }
                }

                Vector idData = getRegistrationTable();
                setInstructionLabel(idData);

                if (idData == null)
                {
                    return;
                }

                ((RegisterModel)(fIDTable.getModel())).setDataVector(
                    idData, fColumnNames);

                fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
                fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
                fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
                fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
                fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

                fRegistrationDialog.dispose();

                JOptionPane.showMessageDialog(this,
                    "Sucessfully registered.  ID = " + result.result,
                    "Sucessfully registerd",
                    JOptionPane.INFORMATION_MESSAGE);

                fSelectedMenu.setEnabled(false);
            }
        }
    }

    public void mouseClicked(MouseEvent e)
    {
        int selectedRow = fIDTable.getSelectedRow();

        if (selectedRow != -1)
        {
            fSelectedMenu.setEnabled(true);

            String eventManagerId = "";
            String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

            Iterator idIter = fEntryList.iterator();

            while (idIter.hasNext() &&
                   !(eventManagerId.equals(selectedId)))
            {
                Map registrationMap = (Map)idIter.next();
                eventManagerId = (String)registrationMap.get("eventManagerID");

                if (eventManagerId.equals(selectedId))
                {
                    String state = (String)registrationMap.get("state");

                    if (state.equals("Enabled"))
                    {
                        fPopupMenuEnable.setEnabled(false);
                        fPopupMenuDisable.setEnabled(true);
                        fSelectedEnable.setEnabled(false);
                        fSelectedDisable.setEnabled(true);
                    }
                    else
                    {
                        fPopupMenuEnable.setEnabled(true);
                        fPopupMenuDisable.setEnabled(false);
                        fSelectedEnable.setEnabled(true);
                        fSelectedDisable.setEnabled(false);
                    }
                }
            }
        }

        if ((e.getClickCount() != 2) ||
            (selectedRow == -1))
        {
            return;
        }

        String selectedId = (String) fIDTable.getValueAt(selectedRow, 0);

        String eventManagerId = "";

        Iterator idIter = fEntryList.iterator();

        while (idIter.hasNext() && !(eventManagerId.equals(selectedId)))
        {
            Map registrationMap = (Map)idIter.next();
            eventManagerId = (String)registrationMap.get("eventManagerID");

            if (eventManagerId.equals(selectedId))
            {
                String machine = (String)registrationMap.get("machine");
                String machineType = (String)registrationMap.get("machineType");
                String service = (String)registrationMap.get("service");
                String serviceType = (String)registrationMap.get("serviceType");
                String request = (String)registrationMap.get("request");
                String requestType = (String)registrationMap.get("requestType");
                String prepareScript =
                    (String)registrationMap.get("prepareScript");
                String type = (String)registrationMap.get("type");
                String subtype = (String)registrationMap.get("subtype");
                String dayOfMonth = (String)registrationMap.get("dayOfMonth");
                String month = (String)registrationMap.get("month");
                String dayOfWeek = (String)registrationMap.get("dayOfWeek");
                String description = (String)registrationMap.get("description");
                String state =
                    (String)registrationMap.get("state");

                displayfRegistrationDialog();

                fRegistrationIDLabel.setText(eventManagerId);

                fMachineTF.setText(machine);
                fPythonMachine.setSelected(machineType.equals("Python"));
                fServiceTF.setText(service);
                fPythonService.setSelected(serviceType.equals("Python"));
                fRequestTF.setText(request);
                fPythonRequest.setSelected(requestType.equals("Python"));
                fPrepareTA.setText(prepareScript);
                fTypeTF.setText(type);
                fSubtypeTF.setText(subtype);
                fDescriptionTF.setText(description);

                if (state.equals("Disabled"))
                {
                    fEnabledCB.setSelected(false);
                }
                else
                {
                    fEnabledCB.setSelected(true);
                }

                fRegistrationDialog.setVisible(true);
            }
        }
    }

    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    public void mousePressed(MouseEvent e){}

    public void mouseReleased(MouseEvent e)
    {
        displayPopup(e);
    }

    public void displayPopup(MouseEvent e)
    {
        if (e.isPopupTrigger())
        {
            int tableIndex = fIDTable.rowAtPoint(
                new Point(e.getX(), e.getY()));

            if (tableIndex > -1)
            {
                fIDTable.setRowSelectionInterval(tableIndex, tableIndex);

                fPopupMenu.show(e.getComponent(), e.getX(), e.getY());

                fSelectedMenu.setEnabled(true);

                String eventManagerId = "";
                String selectedId = (String) fIDTable.getValueAt(tableIndex, 0);

                Iterator idIter = fEntryList.iterator();

                while (idIter.hasNext() &&
                       !(eventManagerId.equals(selectedId)))
                {
                    Map registrationMap = (Map)idIter.next();
                    eventManagerId =
                        (String)registrationMap.get("eventManagerID");

                    if (eventManagerId.equals(selectedId))
                    {
                        String state = (String)registrationMap.get("state");

                        if (state.equals("Enabled"))
                        {
                            fPopupMenuEnable.setEnabled(false);
                            fPopupMenuDisable.setEnabled(true);
                            fSelectedEnable.setEnabled(false);
                            fSelectedDisable.setEnabled(true);
                        }
                        else
                        {
                            fPopupMenuEnable.setEnabled(true);
                            fPopupMenuDisable.setEnabled(false);
                            fSelectedEnable.setEnabled(true);
                            fSelectedDisable.setEnabled(false);
                        }
                    }
                }
            }
        }
    }

    public void keyPressed(KeyEvent e)
    {
        if ((e.getSource() == fServiceMachineTF) ||
            (e.getSource() == fServiceNameTF))
        {
            if (e.getKeyCode() == KeyEvent.VK_ENTER)
            {
                // Check if service machine or service name was changed

                boolean changed = false;

                if (!fServiceMachine.equals(fServiceMachineTF.getText()))
                {
                    changed = true;
                    fServiceMachine = fServiceMachineTF.getText();
                }

                if (!fServiceName.equals(fServiceNameTF.getText()))
                {
                    changed = true;
                    fServiceName = fServiceNameTF.getText();
                }

                if (changed)
                {
                    // Assign the log name for the service log.
                    // It must have the right case in order for a log query
                    // request to work on Unix machines.

                    assignLogName();
                }

                Vector idData = getRegistrationTable();
                setInstructionLabel(idData);

                ((RegisterModel)(fIDTable.getModel())).setDataVector(
                    idData, fColumnNames);

                fIDTable.getColumnModel().getColumn(0).setMaxWidth(40);
                fIDTable.getColumnModel().getColumn(0).setPreferredWidth(40);
                fIDTable.getColumnModel().getColumn(1).setPreferredWidth(20);
                fIDTable.getColumnModel().getColumn(2).setPreferredWidth(150);
                fIDTable.getColumnModel().getColumn(3).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(4).setPreferredWidth(30);
                fIDTable.getColumnModel().getColumn(5).setPreferredWidth(265);

                fSelectedMenu.setEnabled(false);
            }
        }
    }

    public void keyReleased(KeyEvent e) {}

    public void keyTyped(KeyEvent e) {}

    public void displayfRegistrationDialog()
    {
        fRegistrationDialog = new JDialog(
            this, "EventManager Registration", true);

        JPanel optionsPanel = new JPanel();
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc = new GridBagConstraints();
        optionsPanel.setLayout(gbl);

        JLabel registrationIDTitle = new JLabel("Registration ID:");
        registrationIDTitle.setOpaque(true);
        registrationIDTitle.setForeground(Color.black);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        optionsPanel.add(registrationIDTitle, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fRegistrationIDLabel = new JLabel("N/A");
        fRegistrationIDLabel.setForeground(Color.blue);

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fRegistrationIDLabel, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel descriptionLabel = new JLabel("Description:");
        descriptionLabel.setOpaque(true);
        descriptionLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(descriptionLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fDescriptionTF = new JTextField(20)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fDescriptionTF.setToolTipText(
            wrapText("Enter a description for the registration. This field " +
            "is optional.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fDescriptionTF, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel machineLabel = new JLabel("Target Machine:");
        machineLabel.setOpaque(true);
        machineLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(machineLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fPythonMachine = new JCheckBox("Python", false)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fPythonMachine.setToolTipText(
           wrapText("Checking \"Python\" indicates that the text entered for " +
           "\"Target Machine\" contains Python code.", 80));

        gbc.gridwidth = 1;
        optionsPanel.add(fPythonMachine, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fMachineTF = new JTextField(20)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fMachineTF.setToolTipText(
            wrapText("The machine where you wish to have the STAF command " +
            "executed. This field is required. If the text contains Python " +
            "code, then select \"Python\" next to the \"Target Machine\" " +
            "label.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fMachineTF, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel serviceLabel = new JLabel("Target Service:");
        serviceLabel.setOpaque(true);
        serviceLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(serviceLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fPythonService = new JCheckBox("Python", false)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fPythonService.setToolTipText(
            wrapText("Checking \"Python\" indicates that the text entered " +
            "for \"Target Service\" contains Python code.", 80));

        gbc.gridwidth = 1;
        optionsPanel.add(fPythonService, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fServiceTF = new JTextField(20)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fServiceTF.setToolTipText(
            wrapText("The service for the STAF command you wish to have " +
            "executed. This field is required. If the text contains Python " +
            "code, then select \"Python\" next to the \"Target Service\" " +
            "label.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fServiceTF, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel requestLabel = new JLabel("Target Request:");
        requestLabel.setOpaque(true);
        requestLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(requestLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fPythonRequest = new JCheckBox("Python", false)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fPythonRequest.setToolTipText(
            wrapText("Checking \"Python\" indicates that the text entered " +
            "for \"Target Request\" contains Python code.", 80));

        gbc.gridwidth = 1;
        optionsPanel.add(fPythonRequest, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fRequestTF = new JTextField(40)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fRequestTF.setToolTipText(
            wrapText("The STAF request you wish to have executed. This field " +
            "is required. If the text contains Python code, then select " +
            "\"Python\" next to the \"Target Request\" label.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fRequestTF, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel prepareLabel = new JLabel("Prepare Script:");
        prepareLabel.setOpaque(true);
        prepareLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(prepareLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fPrepareTA = new JTextArea(5, 40)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fPrepareTA.setToolTipText(
            wrapText("Any Python code that you wish to have evaluated prior " +
            "to the evaluation of any PYTHONMACHINE, PYTHONSERVICE, or " +
            "PYTHONREQUEST options. This field is optional.", 80));

        fPrepareTA.setBorder(new BevelBorder(BevelBorder.LOWERED));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fPrepareTA, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel typeLabel = new JLabel("Type:");
        typeLabel.setOpaque(true);
        typeLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(typeLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fTypeTF = new JTextField(20)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fTypeTF.setToolTipText(
            wrapText("This specifies the Event Type for which this command " +
            "will be registered. This field is required.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fTypeTF, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel subtypeLabel = new JLabel("Subtype:");
        subtypeLabel.setOpaque(true);
        subtypeLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(subtypeLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fSubtypeTF = new JTextField(20)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fSubtypeTF.setToolTipText(
            wrapText("This specifies the Event SubType for which this " +
            "command will be registered. This field is optional.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fSubtypeTF, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JLabel enabledLabel = new JLabel("Enabled:");
        enabledLabel.setOpaque(true);
        enabledLabel.setForeground(Color.black);

        gbc.gridwidth = 1;
        optionsPanel.add(enabledLabel, gbc);
        optionsPanel.add(Box.createHorizontalStrut(10), gbc);

        fEnabledCB = new JCheckBox("", true)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fEnabledCB.setToolTipText(
            wrapText("Checking \"Enabled\" indicates that the STAF command " +
            "will be enabled when it is registered. If the STAF command is " +
            "disabled, it will not be submitted when its event type/subtype " +
            "is generated.", 80));

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        optionsPanel.add(fEnabledCB, gbc);

        gbc.weightx = 0;
        optionsPanel.add(Box.createVerticalStrut(3), gbc);

        JPanel outerPanel = new JPanel();
        outerPanel.setLayout(new BorderLayout());
        outerPanel.setBorder(new TitledBorder("EventManager Register options"));
        outerPanel.add(BorderLayout.CENTER, new JScrollPane(optionsPanel));

        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));

        fRegisterButton = new JButton("Register");
        fRegisterButton.addActionListener(this);
        buttonPanel.add(fRegisterButton);

        fCancelButton = new JButton("Cancel");
        fCancelButton.addActionListener(this);
        buttonPanel.add(fCancelButton);

        outerPanel.add(BorderLayout.SOUTH, buttonPanel);

        fRegistrationDialog.getContentPane().add(outerPanel);
        fRegistrationDialog.setSize(new Dimension(650, 380));
        fRegistrationDialog.setLocationRelativeTo(this);
    }

    public void showErrorDialog(Component comp, String message)
    {
        JTextPane messagePane = new JTextPane();
        messagePane.setFont(new Font("Dialog", Font.BOLD, 12));
        messagePane.setEditable(false);
        messagePane.setText(message);
        messagePane.select(0,0);
        JScrollPane scrollPane = new JScrollPane(messagePane);
        scrollPane.setPreferredSize(new Dimension(400, 100));
        JOptionPane.showMessageDialog(comp, scrollPane,
                                      "EventManager Error",
                                      JOptionPane.ERROR_MESSAGE);
    }
    
    private void assignLogName()
    {
        // First try using the service name for the service log name

        fLogName = fServiceName;

        // Submit a LOG QUERY request to verify that a log name with the
        // specified service name exists.  We need to do this because the
        // service log name will have the case used when the service
        // was registered.  On Unix machines, the log name must be the same
        // case as the service name that was used when registering the
        // service.  So, if the "Service name" field on the UI doesn't
        // match the name used when registering the EventManager service,
        // the log query request will fail with RC 48 (Does Not Exist).
        // If this happens, we'll then list the machine logs to see if we
        // can find another log name that matches if we ignore the case.
        // If so, then we'll assign that as the log name.

        STAFResult result = fHandle.submit2(
            fServiceMachine, "LOG",
            "QUERY MACHINE {STAF/Config/MachineNickname} " +
            "LOGNAME " + fServiceName + " LAST 1");

        if (result.rc == STAFResult.DoesNotExist)
        {
            // List the machine logs on fServiceMachine and check if any of
            // the log names match fServiceName, case-insensitive

            result = fHandle.submit2(
                fServiceMachine, "LOG",
                "LIST MACHINE {STAF/Config/MachineNickname}");

            if (result.rc == STAFResult.Ok)
            {
                // Iterate through list of logs

                java.util.List log = (java.util.List)result.resultObj;

                STAFMarshallingContext mc =
                    STAFMarshallingContext.unmarshall(
                        result.result);

                java.util.List logList = (java.util.List)mc.getRootObject();
                Iterator logIter = logList.iterator();

                while (logIter.hasNext())
                {
                    Map logMap = (HashMap)logIter.next();
                    String logName = (String)logMap.get("name");

                    if (fServiceName.equalsIgnoreCase(logName))
                    {
                        fLogName = logName;
                        break;
                    }
                }
            }
        }
    }

    public Vector getRegistrationTable()
    {
        String listRequest = "LIST LONG";

        STAFResult result = fHandle.submit2(
            fServiceMachine, fServiceName, listRequest);

        if (result.rc != STAFResult.Ok)
        {
            showErrorDialog(this, "Error listing the registrations.\nRC=" +
                result.rc + " Result=" + result.result);

            return null;
        }
        
        STAFMarshallingContext mc =
            STAFMarshallingContext.unmarshall(result.result);
        STAFMapClassDefinition mcd = mc.getMapClassDefinition(
            "STAF/Service/EventManager/EventManagerID");

        fEntryList = (java.util.List)mc.getRootObject();

        Iterator idIter = fEntryList.iterator();

        Vector<Vector<String>> idData = new Vector<Vector<String>>();
        
        while (idIter.hasNext())
        {
            Map registrationMap = (Map)idIter.next();
            String id = (String)registrationMap.get("eventManagerID");
            String state = (String)registrationMap.get("state");
            String description = (String)registrationMap.get("description");
            String machine = (String)registrationMap.get("machine");
            String service = (String)registrationMap.get("service");
            String request = (String)registrationMap.get("request");

            Vector<String> row = new Vector<String>();

            row.add(id);
            row.add(state);
            row.add(description);
            row.add(machine);
            row.add(service);
            row.add(request);

            idData.add(row);
        }

        return idData;
    }

    // This method will wrap the input string at the specified column,
    // placing a newline character at the first whitespace character to the
    // left of the column position.  If the current section of the input string
    // contains no whitespace characters, then a newline character will be
    // placed at the next whitespace character to the right of the column
    // position.

    public static String wrapText(String inputString, int wrapColumn)
    {
        int i = wrapColumn;
        StringBuffer textBuffer = new StringBuffer(inputString);
        int lastWhitespaceIndex = 0;

        while (i < textBuffer.length())
        {
            if (Character.isWhitespace(textBuffer.charAt(i)))
            {
                textBuffer.setCharAt(i, '\n');
                lastWhitespaceIndex = i;
                i = i + wrapColumn;
            }
            else
            {
                i = i - 1;

                if (i == lastWhitespaceIndex)
                {
                    // No whitespace in the current block, so find the next
                    // whitespace character to the right of the current block

                    int j = i + wrapColumn;

                    while (j < textBuffer.length())
                    {
                        if (Character.isWhitespace(textBuffer.charAt(j)))
                        {
                            textBuffer.setCharAt(j, '\n');
                            lastWhitespaceIndex = j;
                            i = j + wrapColumn;
                            break;
                        }

                        j = j + 1;
                    }

                    if (j == textBuffer.length())
                    {
                        break;
                    }
                }
            }
        }

        return textBuffer.toString();
    }

    class RegisterModel extends DefaultTableModel
    {
        public RegisterModel(Vector data, Vector columns)
        {
            super(data, columns);
        }

        public boolean isCellEditable(int row, int col)
        {
            return false;
        }
    }

    class MultiLineToolTip extends JToolTip
    {
        public MultiLineToolTip()
        {
            setUI(new MultiLineToolTipUI());
        }
    }
    
    class MultiLineToolTipUI extends MetalToolTipUI
    {
        String[] lines;
        int maxWidth = 0;

        public void paint(Graphics g, JComponent c)
        {
            FontMetrics metrics = g.getFontMetrics(g.getFont());
            Dimension size = c.getSize();
            g.setColor(c.getBackground());
            g.fillRect(0, 0, size.width, size.height);
            g.setColor(c.getForeground());

            if (lines != null)
            {
                for (int i = 0; i < lines.length; i++)
                {
                    g.drawString(lines[i], 3, (metrics.getHeight()) * ( i + 1));
                }
            }
        }

        public Dimension getPreferredSize(JComponent c)
        {
            FontMetrics metrics = c.getFontMetrics(c.getFont());
            String tipText = ((JToolTip)c).getTipText();

            if (tipText == null)
            {
                tipText = "";
            }

            BufferedReader br = new BufferedReader(new StringReader(tipText));
            String line;
            int maxWidth = 0;
            Vector<String> lineVector = new Vector<String>();

            try
            {
                while ((line = br.readLine()) != null)
                {
                    int width =
                        SwingUtilities.computeStringWidth(metrics,line);

                    if (maxWidth < width)
                    {
                        maxWidth = width;
                    }

                    lineVector.addElement(line);
                }
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }

            int numberOfLines = lineVector.size();

            if (numberOfLines < 1)
            {
                lines = null;
                numberOfLines = 1;
            }
            else
            {
                lines = new String[numberOfLines];
                int i = 0;

                for (Enumeration<String> e = lineVector.elements();
                     e.hasMoreElements(); i++)
                {
                    lines[i] = e.nextElement();
                }
            }

            int height = metrics.getHeight() * numberOfLines;
            this.maxWidth = maxWidth;

            return new Dimension(maxWidth + 8, height + 8);
        }
    }

}