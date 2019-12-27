/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.plaf.metal.*;
import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.text.SimpleDateFormat;
import java.text.ParsePosition;
import java.lang.reflect.*;
import java.util.jar.*;
import java.util.List;
import java.util.ArrayList;
import javax.swing.filechooser.FileFilter;

public class STAXMonitor extends JFrame implements ActionListener,
                                                   ItemListener,
                                                   KeyListener,
                                                   MouseListener,
                                                   ListSelectionListener,
                                                   ChangeListener,
                                                   Runnable
{
    // Set to the version of the STAX Monitor
    static final String fVersion = "3.5.0 Beta 1";

    // Set to the version of STAF required by the STAX Monitor
    static final String fSTAFRequiredVersion = "3.3.3";

    // Set to the version of the STAX service required by the STAX Monitor
    static final String fServiceRequiredVersion = "3.5.0 Beta 1";

    // Set to the version of the Event service required by the STAX Monitor
    static final String fEventRequiredVersion = "3.1.0";

    static final String sInterfaceSeparator = "://";
    static final String sPortSeparator = "@";

    // This is the maximum queue size that the STAX Monitor will automatically
    // increase the maxQueueSize to if less than this number
    private static int MAXQUEUESIZE = 10000;

    public static final String DEFAULT_STRING = new String("Default");

    // These are the "pretty" versions of the Python Output combo box entries

    private static final String JOBUSERLOG_PRETTY = new String("Job User Log");
    private static final String MESSAGE_PRETTY = new String("Message");
    private static final String JOBUSERLOGANDMSG_PRETTY = new String(
        "Job User Log & Message");
    private static final String JVMLOG_PRETTY = new String("JVM Log");
    
    private static final String[] PYTHON_OUTPUTS_PRETTY =
    {
        JOBUSERLOG_PRETTY, MESSAGE_PRETTY, JOBUSERLOGANDMSG_PRETTY,
        JVMLOG_PRETTY
    };
    
    // These are the valid values for the "PYTHONOUTPUT" option 

    public static final String JOBUSERLOG_STRING = new String("JobUserLog");
    public static final String MESSAGE_STRING = new String("Message");
    public static final String JOBUSERLOGANDMSG_STRING = new String(
        "JobUserLogAndMsg");
    public static final String JVMLOG_STRING = new String("JVMLog");
    
    /**
     * Note: Extensions, etc should not use LOGLEVELS as we will
     * probably move this static variable into JSTAF (e.g. STAFUtil.java)
     * in the future.
     */
    public static final String[] LOGLEVELS =
    {
        "Info", "Fatal", "Error", "Warning", "Trace", "Trace2", "Trace3",
        "Debug", "Debug2", "Debug3", "Start", "Stop", "Pass", "Fail", "Status",
        "User1", "User2", "User3", "User4", "User5", "User6", "User7", "User8" 
    };

    // ToolTip text

    public static final String BREAKPOINT_LINE_NUMBER_TOOLTIP =
        "Enter the line number for the STAX element where the breakpoint " +
        "will be added.  The line number must " +
        "match the first line of the STAX element.  This field " +
        "is required.";

    public static final String BREAKPOINT_LINE_FILE_TOOLTIP =
        "Enter the full path for the XML file where the breakpoint will " +
        "be added.  This field is optional.  If not specified, the " +
        "STAXJobXMLFile (i.e. the value of the FILE parameter on the " +
        "STAX EXECUTE request) will be used.";

    public static final String BREAKPOINT_LINE_MACHINE_TOOLTIP =
        "Enter the machine where the XML file is located.  This field " +
        "is optional.  If not specified, the breakpoint will be added " +
        "for the specified XML file located on any machine.";

    Vector fBreakpointTableColumnNames;

    STAFHandle fHandle;
    String fFileSep;
    String fLocalMachineName;
    String fSTAXMonitorEndpoint;

    String fStaxMachineInterface = null;
    String fStaxMachineIdentifier = null;
    String fStaxMachinePort = null;

    String fStaxConfigMachine = "";
    String fStaxInstanceUUID = "STAXInstanceUUID";
    String fLocalInstanceUUID = "LocalInstanceUUID";
    boolean fIsSTAXServiceLocal = false;
    
    String fMonitorFileDirectory;
    String fExtensionsDirectory = null;
    JDialog fPropertiesDialog;
    boolean fPropertiesAtStartup = true;
    String kMonitorPropertiesFileName = "monprp.ser";
    String kMonitorLastJobDataFileName = "monljp.ser";
    STAFCommandParser fArgsParser = new STAFCommandParser(0, false);
    String fPropertiesFileName;
    String fLastJobParmsFileName;
    String fStaxMachineName = "";
    String fStaxMachineNickname;
    JTextField fStaxMachineNameField;
    String fStaxServiceName = "";
    JTextField fStaxServiceNameField;
    String fEventMachineName = "";
    JTextField fEventMachineNameField;
    String fEventServiceName = "";
    JTextField fEventServiceNameField;
    JCheckBox fShowNoSTAXMonitorInformation = new
        JCheckBox("Show Process <No STAX Monitor Information> message", false);
    boolean fShowNoSTAXMonitorInformationBool = false;
    JCheckBox fLimitMessages = new
        JCheckBox("Limit number of Messages displayed to: ", true);
    boolean fLimitMessagesBool = true;
    JTextField fLimitMessagesField;
    
    static Boolean fDefaultShowNoSTAXMonitorInformation = new Boolean(false);
    static Integer fDefaultAutoMonitorSubjobs = new Integer(
        STAXMonitorFrame.AUTOMONITOR_RECOMMENDED);
    static Boolean fDefaultLimitMessages = new Boolean(true);
    static String fDefaultLimitMessagesText = "200";
    static String fDefaultProcessMonitorSeconds = "60";
    static String fDefaultElapsedTimeSeconds = "1";

    String fLimitMessagesFieldText = fDefaultLimitMessagesText;
    JTextField fProcessMonitorSecondsField;
    String fProcessMonitorSecondsFieldText = fDefaultProcessMonitorSeconds;
    JTextField fElapsedTimeSecondsField;
    String fElapsedTimeSecondsFieldText = fDefaultElapsedTimeSeconds;

    static String fDefaultMessageFontName = "Dialog";
    static String fDefaultLogViewerFontName = "Dialog";
    String fMessageFontName = fDefaultMessageFontName;
    String fLogViewerFontName = fDefaultLogViewerFontName;
    JComboBox fMessageFontNameCB = new JComboBox();
    JComboBox fLogViewerFontNameCB = new JComboBox();

    String fSaveAsDirectory = null;
    JTextField fSaveAsDirectoryField;
    JButton fSaveAsDirectoryBrowseButton;

    JComboBox fTestcaseSortColumnCB = new JComboBox();
    JComboBox fTestcaseSortOrderCB = new JComboBox();
    JCheckBox fTestcaseNameCB;
    JCheckBox fTestcasePassCB;
    JCheckBox fTestcaseFailCB;
    JCheckBox fTestcaseStartDateTimeCB;
    JCheckBox fTestcaseStatusDateTimeCB;
    JCheckBox fTestcaseDurationCB;
    JCheckBox fTestcaseStartsCB;
    JCheckBox fTestcaseInformationCB;
    JCheckBox fTestcaseAutoResizeCB;
    boolean fDisplayTestcaseName = true;
    boolean fDisplayTestcasePass = true;
    boolean fDisplayTestcaseFail = true;
    boolean fDisplayTestcaseStartDateTime = true;
    boolean fDisplayTestcaseStatusDateTime = true;
    boolean fDisplayTestcaseDuration = true;
    boolean fDisplayTestcaseStarts = true;
    boolean fDisplayTestcaseInformation = true;
    boolean fTestcaseAutoResize = true;
    int fTestcaseSortColumn;
    int fTestcaseSortOrder;

    JButton fPropertiesOKButton = new JButton("Save");
    JButton fPropertiesCancelButton = new JButton("Cancel");
    JButton fStartNewJobButton = new JButton("Submit New Job...");
    JButton fSubmitLastJobButton = new JButton("Resubmit Previous Job");
    JTable fActiveJobsTable;
    STAXMonitorTableModel fActiveJobsTableModel;
    STAXMonitorTableSorter fActiveJobsModelSorter;
    Vector fActiveJobColumns;
    JMenu fFileMenu;
    JMenu fDisplayMenu;
    JMenu fHelpMenu;
    JMenuItem fHelpAbout;
    JMenuItem fJobParametersMenuItem;
    JMenuItem fStartNewJobMenuItem;
    JMenuItem fFileProperties;
    JMenuItem fFileExit;
    JMenuItem fDisplayServiceLog;
    JMenuItem fDisplaySTAXJVMLog;
    JMenuItem fDisplayOtherJVMLog;
    JMenuItem fDisplaySelectedJobLog;
    JMenuItem fDisplaySelectedJobUserLog;
    JMenu fDisplayRecentLogs;
    JMenuItem fRecentLogsMenuItems[] = new JMenuItem[20];
    Vector fRecentLogs = new Vector();
    Vector fRecentLogsTooltips = new Vector();
    JMenuItem fDisplayJobLog;
    JMenuItem fDisplayJobUserLog;
    JMenu fStartNewJobFileMenu;
    JMenuItem fStartNewJobFileExit;
    JMenuItem fStartNewJobFileOpen;
    JMenuItem fStartNewJobFileSave;
    JMenuItem fStartNewJobFileSaveAs;
    Hashtable fPropertiesData;
    JDialog fStartNewJobDialog;
    String fLocalXmlFileName = "";
    JTextField fLocalXmlFileNameField;
    String fOtherXmlFileName = "";
    JTextField fOtherXmlFileNameField;
    String fOtherXmlFileMachineName = "";
    JTextField fOtherXmlFileMachineField;
    String fFunction = "";
    JTextField fFunctionField;
    JRadioButton fDefaultFunctionRB;
    JRadioButton fOtherFunctionRB;
    JRadioButton fMachineLocalRB;
    JRadioButton fMachineOtherRB;
    JRadioButton fMonitorYesRB;
    JRadioButton fMonitorNoRB;
    JRadioButton fClearLogsYesRB;
    JRadioButton fClearLogsNoRB;
    JRadioButton fClearLogsDefaultRB;
    JTextArea fArguments;
    JButton fClearArguments = new JButton("Clear");
    boolean fArgumentsEnabled;
    String fArgs = "";
    String fJobName = "";
    JTextField fJobNameField;
    JButton fStartNewJobSubmitButton;
    JButton fStartNewJobTestButton;
    JButton fStartNewJobCancelButton;
    JButton fStartNewJobClearButton;
    JButton fBrowseButton;
    JList fScriptList;
    JButton fScriptAddButton;
    JButton fScriptDeleteButton;
    JButton fScriptDeleteAllButton;
    JDialog fAddScriptDialog;
    JTextArea fAddScriptTextArea;
    JButton fAddScriptAddButton;
    JButton fAddScriptCancelButton;
    JDialog fEditScriptDialog;
    JTextArea fEditScriptTextArea;
    JButton fEditScriptSaveButton;
    JButton fEditScriptCancelButton;
    File fLastFileDirectory;
    File fLastJobParmsFileDirectory;
    Vector fScriptVector = new Vector();
    Hashtable fMonitorTable = new Hashtable();
    JPopupMenu fJobPopupMenu = new JPopupMenu();
    JMenuItem fJobShowMonitorMenuItem = new JMenuItem("Show Monitor window");
    JMenuItem fJobStartMonitorMenuItem = new JMenuItem("Start Monitoring");
    JMenuItem fJobShowJobLogMenuItem = new JMenuItem("Display Job Log");
    JMenuItem fJobShowJobUserLogMenuItem =
        new JMenuItem("Display Job User Log");
    JMenuItem fJobShowSTAXServiceLogMenuItem =
        new JMenuItem("Display STAX Service Log");
    JMenuItem fJobShowSTAXJVMLogMenuItem =
        new JMenuItem("Display STAX JVM Log");
    JMenuItem fJobShowOtherJVMLogMenuItem =
        new JMenuItem("Display Other JVM Log");
    JMenuItem fJobTerminateJobMenuItem = new JMenuItem("Terminate Job");
    String fJobParmsID = "STAX/Job/Monitor/Parameters";
    String fStartNewJobTitle = "STAX Job Parameters ";
    String fCurrentJobParmsFile = "";
    boolean fCurrentJobParmsNotSaved = false;
    boolean fContinueElapsedTime = true;
    MonitorElapsedTime fElapsedTime;
    Hashtable fJobStartTimes = new Hashtable();
    Hashtable fJobStartDateTimes = new Hashtable();
    Vector fRecentFiles = new Vector();
    String fRecentFilesName;
    String kRecentFilesFileName = "monrcfl.ser";
    JMenuItem fRecentFileMenuItems[] = new JMenuItem[10];
    String fCloseOnEndJobID = "";
    String fStartNewJobParmFileName = "";
    boolean fStartNewJobCloseOnEnd = false;
    Vector fLocalExtJarFiles;
    Vector fUpdatedLocalExtJarFiles;
    Vector fOldLocalExtJarFiles;
    Map fMonitorExtensionMap = new TreeMap();

    Vector fBreakpointFunctionsVector = new Vector();
    JButton fBreakpointFunctionAddButton;
    JButton fBreakpointFunctionDeleteButton;
    JButton fBreakpointFunctionDeleteAllButton;
    JDialog fAddBreakpointFunctionDialog;
    JTextField fAddBreakpointFunctionTextField;
    JButton fAddBreakpointFunctionAddButton;
    JButton fAddBreakpointFunctionCancelButton;
    JDialog fEditBreakpointFunctionDialog;
    JTextField fEditBreakpointFunctionTextField;
    JButton fEditBreakpointFunctionSaveButton;
    JButton fEditBreakpointFunctionCancelButton;

    JTable fBreakpointTable;
    JButton fBreakpointLineAddButton;
    JButton fBreakpointDeleteButton;
    JButton fBreakpointDeleteAllButton;
    JDialog fAddBreakpointLineDialog;
    JTextField fAddBreakpointLineNumberTextField;
    JTextField fAddBreakpointLineFileTextField;
    JTextField fAddBreakpointLineMachineTextField;
    JButton fAddBreakpointLineAddButton;
    JButton fAddBreakpointLineCancelButton;
    JDialog fEditBreakpointLineDialog;
    JTextField fEditBreakpointLineNumberTextField;
    JTextField fEditBreakpointLineFileTextField;
    JTextField fEditBreakpointLineMachineTextField;
    JButton fEditBreakpointLineSaveButton;
    JButton fEditBreakpointLineCancelButton;

    JCheckBox fBreakpointFirstFunctionCB;
    JCheckBox fBreakpointSubjobFirstFunctionCB;

    STAXMonitorTableModel fBreakpointsTableModel;
    STAXMonitorTableSorter fBreakpointsModelSorter;

    // List of temporary extension jar files
    Vector fTempLocalExtFiles = new Vector();

    JList fPluginJarsList;

    // List of monitor extension plugin classes to be loaded by each job
    Vector fPluginClasses = new Vector();

    JButton fPluginJarsAddButton;
    JButton fPluginJarsDeleteButton;
    JButton fPluginJarsDeleteAllButton;
    JDialog fAddPluginJarsDialog;
    JTextField fAddPluginJarsTextField;
    JButton fAddPluginJarsBrowseButton;
    JButton fAddPluginJarsAddButton;
    JButton fAddPluginJarsCancelButton;
    JDialog fEditPluginJarsDialog;
    JTextField fEditPluginJarsTextField;
    JButton fEditPluginJarsSaveButton;
    JButton fEditPluginJarsCancelButton;

    JTable fExtensionsTable;
    Vector fExtensionsColumns;
    STAXMonitorTableModel fExtensionsTableModel;
    STAXMonitorTableSorter fExtensionsModelSorter;
    static String EXT_COLUMN_NAME = "Name";
    static String EXT_COLUMN_SOURCE = "Source";
    static String EXT_COLUMN_VERSION = "Version";
    static String EXT_COLUMN_JARFILE = "Jar File Name";
    static String EXT_COLUMN_OVERRIDES = "Overrides";
    static String EXT_COLUMN_PREREQ = "Prereq";
    static String EXT_COLUMN_DESCRIPTION = "Description";

    String fOldStaxMachineName;
    String fOldStaxServiceName;
    String fOldEventMachineName;
    String fOldEventServiceName;
    Vector fScriptFilesVector;
    JList fScriptFilesList;
    JButton fScriptFilesAddButton;
    JButton fScriptFilesDeleteButton;
    JButton fScriptFilesDeleteAllButton;
    JDialog fAddScriptFilesDialog;
    JTextField fAddScriptFilesTextField;
    JButton fAddScriptFilesBrowseButton;
    JButton fAddScriptFilesAddButton;
    JButton fAddScriptFilesCancelButton;
    JDialog fEditScriptFilesDialog;
    JTextField fEditScriptFilesTextField;
    JButton fEditScriptFilesSaveButton;
    JButton fEditScriptFilesCancelButton;
    JTextField fScriptFilesMachineTextField;
    String fScriptFilesMachineName;
    JRadioButton fLocalScriptMachineRB;
    JRadioButton fXMLJobFileScriptMachineRB;
    JRadioButton fOtherScriptMachineRB;
    JRadioButton fAutoMonitorSubjobsRB;
    boolean fAutoMonitorSubjobs = false;
    JRadioButton fAutoMonitorRecommendedSubjobsRB;
    boolean fAutoMonitorRecommendedSubjobs = false;
    JRadioButton fNeverAutoMonitorSubjobsRB;
    boolean fNeverAutoMonitorSubjobs = false;
    JButton fStartNewJobWizardButton;
    JDialog fWizardDialog;
    JButton fWizardSaveButton = new JButton("Save");
    JButton fWizardPreviewXMLButton = new JButton("Preview XML...");
    JButton fWizardCancelButton = new JButton("Cancel");
    JPanel prologPanel;
    JPanel fWizardFunctionArgsPanel;
    HashMap fWizardFunctionMap;
    HashMap fWizardFunctionTypeMap;
    HashMap fWizardFunctionPrologMap;
    HashMap fWizardFunctionEpilogMap;
    HashMap fWizardFunctionArgTableMap;
    HashMap fWizardFunctionArgDefaultsMap;
    JList fWizardFunctionsList;
    TitledBorder functionDescriptionBorder = new TitledBorder("");
    TitledBorder functionArgumentBorder = new TitledBorder("");
    JLabel fWizardNoArgsAllowedLabel;
    JLabel fWizardNoArgsDefinedLabel;
    JLabel fWizardSingleArgLabel;
    JLabel fWizardListArgsLabel;
    JLabel fWizardMapArgsLabel;
    Object[] fWizardFunctionTableColumnNames =
        {
            "Name", "Description", "Required", "Value"
        };

    JRadioButton fLogTCElapsedTimeYesRB;
    JRadioButton fLogTCElapsedTimeNoRB;
    JRadioButton fLogTCElapsedTimeDefaultRB;
    JRadioButton fLogTCNumStartsYesRB;
    JRadioButton fLogTCNumStartsNoRB;
    JRadioButton fLogTCNumStartsDefaultRB;
    JRadioButton fLogTCStartStopYesRB;
    JRadioButton fLogTCStartStopNoRB;
    JRadioButton fLogTCStartStopDefaultRB;

    JComboBox fPythonOutputCB = new JComboBox();
    JComboBox fPythonLogLevelCB = new JComboBox();
    
    JTable fLogOptionsTable;
    JEditorPane fWizardFunctionProlog;
    JButton fWizardDetailsButton;

    // Used to compare if should assign saved function arg list for a function
    // when using the Job Wizard
    String fWizardSavedFunctionName = "";
    String fWizardSavedFileName = "";
    String fWizardSavedFileMachineName = "";
    Vector fWizardSavedFunctionArgList = new Vector();
    String fWizardFileName = "";
    String fWizardFileMachineName = "";

    Color lightRed = new Color(255, 204, 204);
    Color lightGreen = new Color(204, 255, 204);
    Color lightYellow = new Color(255, 255, 153);

    String helpText = "\nSTAXMonitor Help\n\n" + 
        "-job <jobNumber> [-closeonend]\n" + 
        "-jobparms <jobParmsFile> [-closeonend]\n" +
        "-extensions\n" +
        "-properties [-staxMachine <machineName>] " +
                    "[-staxServiceName <serviceName>]\n" +
        //"            [-jobParmsDirectory <directoryName>]\n" +
        "            [-noStart]\n" +
        "-help\n" +
        "-version\n";

    private static java.net.URL splashURL =
        ClassLoader.getSystemClassLoader().
        getSystemResource("images/splash.gif");

    final String kSplashText = "Version " + fVersion +
        ", Copyright IBM Corp. 2003-2005";

    static final String STAX_MONITOR_EXTENSION =
        new String("staf/staxmonitor/extension/");
    static final String STAX_EXTENSION_INFO =
        new String("staf/staxinfo/extension");
    static final String NOT_APPLICABLE = new String("<N/A>");
    static final String sNONE = new String("<None>");
    static final int STAX_SERVICE_EXTENSION = 1;
    static final int LOCAL_EXTENSION = 2;

    private STAFVersion fMonitorVersion = null;
    STAFVersion fSTAXServiceVersion;

    public STAXMonitor(String argv[])
    {
        String jobIDToMonitor = "";

        try
        {
            fHandle =
                STAXMonitorUtil.getNewSTAFHandle("STAX/JobMonitor/Controller");
        }
        catch(STAFException ex)
        {
            if (ex.rc == STAFResult.HandleAlreadyExists)
            {
                System.out.println("Ensure that SET ALLOWMULTIREG is " +
                                   "specified in the STAF.cfg file");
            }
            else
            {
                System.out.println("STAX Job Monitor V" + fVersion +
                                   " requires STAF V" + fSTAFRequiredVersion +
                                   " or later to be running");
            }

            System.exit(0);
        }

        STAFResult result = new STAFResult();

        // Resolve the line separator variable for the local machine

        result = fHandle.submit2(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Sep/Line}");

        if (result.rc != STAFResult.Ok)
        {
            JOptionPane.showMessageDialog(
                this, "Error resolving {STAF/Config/Sep/Line} " +
                "variable\n\nRC: " + result.rc + "\nResult: " + result.result,
                "Error resolving variable", JOptionPane.ERROR_MESSAGE);

            System.exit(0);
        }

        String lineSep = result.result;

        // Verify the required version of STAF is running on the STAX Monitor
        // Note:  Method compareSTAFVersion was added in STAF V3.1.0

        try
        {
            result = STAFUtil.compareSTAFVersion(
                "local", fHandle, fSTAFRequiredVersion);

            if (result.rc != STAFResult.Ok)
            {
                if (result.rc == STAFResult.InvalidSTAFVersion)
                {
                    JOptionPane.showMessageDialog(
                        this, "ERROR: Minimum required STAF version for " +
                        "the STAX Monitor is not running." + lineSep +
                        result.result, "Required STAF Version Not Running",
                        JOptionPane.ERROR_MESSAGE);
                }
                else
                {
                    JOptionPane.showMessageDialog(
                        this, "Error verifying STAF version. RC: " +
                        result.rc + ", Additional info: " + result.result,
                        "Error verifying STAF version",
                        JOptionPane.ERROR_MESSAGE);
                }

                System.exit(0);
            }
        }
        catch (Error err)
        {
            JOptionPane.showMessageDialog(
                this, "ERROR: The STAX Monitor requires STAF Version " +
                fSTAFRequiredVersion + " or later.",
                "Required STAF Version Not Running",
                JOptionPane.ERROR_MESSAGE);

            System.exit(0);
        }

        // Verify that the version specified for the STAX Monitor is valid

        try
        {
            fMonitorVersion = new STAFVersion(fVersion);
        }
        catch (NumberFormatException e)
        {
            System.out.println("ERROR: " + fVersion +
                               " is an invalid STAX monitor version.\n" +
                               e.toString());
            System.exit(0);
        }
        
        // Resolve the {STAF/Config/Machine} variable

        result = fHandle.submit2(
            "local", "VAR", "RESOLVE STRING {STAF/Config/Machine}");
        
        if (result.rc != 0)
        {
            JOptionPane.showMessageDialog(
                this, "Error resolving {STAF/Config/Machine} " +
                "variable\n\nRC: " + result.rc + "\nResult: " + result.result,
                "Error resolving variable", JOptionPane.ERROR_MESSAGE);   
        }
        else
        {
            fLocalMachineName = result.result;
        }

        fFileSep = File.separator;
        String userHome = System.getProperty("user.home");

        if (!(userHome.endsWith(fFileSep)))
        {
            userHome += fFileSep;
        }

        fMonitorFileDirectory = userHome + "staxmonitordata3";

        File dir = new File(fMonitorFileDirectory);

        if (!dir.exists())
        {
            if (!(dir.mkdirs()))
            {
                JOptionPane.showMessageDialog(this,
                                      "Error creating directory: " +
                                      fMonitorFileDirectory,
                                      "Error creating directory",
                                      JOptionPane.ERROR_MESSAGE);
            }
        }

        fLastJobParmsFileName = fMonitorFileDirectory +
                                fFileSep + kMonitorLastJobDataFileName;
        fPropertiesFileName = fMonitorFileDirectory +
                              fFileSep + kMonitorPropertiesFileName;

        fRecentFilesName =  fMonitorFileDirectory + fFileSep +
                            kRecentFilesFileName;

        fCurrentJobParmsFile = fLastJobParmsFileName;

        // Check if any arguments were specified

        if (argv.length > 0)
        {
            if (argv[0].equalsIgnoreCase("-HELP"))
            {
                System.out.println(helpText);
                System.exit(0);
            }
            else if (argv[0].equalsIgnoreCase("-VERSION"))
            {
                System.out.println(kSplashText);
                System.exit(0);
            }
            else if (argv[0].equalsIgnoreCase("-JOB"))
            {
                jobIDToMonitor = resolveVar(argv[1], fHandle.getHandle());

                if (jobIDToMonitor == null)
                {
                    System.out.println("Invalid parameters");
                    System.out.println(helpText);
                    System.exit(0);
                }

                if (argv.length > 2)
                {
                    if (argv[2].equalsIgnoreCase("-CLOSEONEND"))
                    {
                        fCloseOnEndJobID = jobIDToMonitor;
                    }
                    else
                    {
                        System.out.println("Invalid parameters");
                        System.out.println(helpText);
                        System.exit(0);
                    }
                }
            }
            else if (argv[0].equalsIgnoreCase("-JOBPARMS"))
            {
                fStartNewJobParmFileName = resolveVar(
                    argv[1], fHandle.getHandle());

                if (argv.length > 2)
                {
                    if (argv[2].equalsIgnoreCase("-CLOSEONEND"))
                    {
                        fStartNewJobCloseOnEnd = true;
                    }
                    else
                    {
                        System.out.println("Invalid parameters");
                        System.out.println(helpText);
                        System.exit(0);
                    }
                }
            }
            else if (argv[0].equalsIgnoreCase("-EXTENSIONS"))
            {
                // Handle later in the constructor
            }
            else if (argv[0].equalsIgnoreCase("-PROPERTIES"))
            {
                boolean noStart = false;
                int numPropsSet = 0;
                String newStaxMachineName = "";
                String newStaxServiceName = "";
                String newEventMachineName = "";
                String newEventServiceName = "";
                String newJobParmsDirectory = "";

                for (int i = 1; i < argv.length; i++)
                {
                    if (argv[i].equalsIgnoreCase("-STAXMACHINE"))
                    {
                        if (!newStaxMachineName.equals(""))
                        {
                            System.out.println(
                                "Invalid parameters - " + 
                                "Can specify -staxMachine property only once");
                            System.out.println(helpText);
                            System.exit(0);
                        }

                        i++;

                        if (i < argv.length)
                        {
                            newStaxMachineName = resolveVar(
                                argv[i], fHandle.getHandle());
                            numPropsSet++;
                        }
                        else
                        {
                            System.out.println(
                                "Invalid parameters - " +
                                "Must specify a value for -staxMachine");
                            System.out.println(helpText);
                            System.exit(0);
                        }
                    }
                    else if (argv[i].equalsIgnoreCase("-STAXSERVICENAME"))
                    {
                        if (!newStaxServiceName.equals(""))
                        {
                            System.out.println(
                                "Invalid parameters - " + 
                                "Can specify -staxServiceName property only once");
                            System.out.println(helpText);
                            System.exit(0);
                        }

                        i++;

                        if (i < argv.length)
                        {
                            newStaxServiceName = resolveVar(
                                argv[i], fHandle.getHandle());
                            numPropsSet++;
                        }
                        else
                        {
                            System.out.println(
                                "Invalid parameters - " +
                                "Must specify a value for -staxServiceName");
                            System.out.println(helpText);
                            System.exit(0);
                        }
                    }
                    /*
                    else if (argv[i].equalsIgnoreCase("-JOBPARMSDIRECTORY"))
                    {
                        if (!newJobParmsDirectory.equals(""))
                        {
                            System.out.println(
                                "Invalid parameters - " + 
                                "Can specify -jobParmsDirectory property " +
                                "only once");
                            System.out.println(helpText);
                            System.exit(0);
                        }

                        i++;

                        if (i < argv.length)
                        {
                            newJobParmsDirectory = resolveVar(
                                argv[i], fHandle.getHandle());
                            numPropsSet++;
                        }
                        else
                        {
                            System.out.println(
                                "Invalid parameters - " +
                                "Must specify a value for -jobParmsDirectory");
                            System.out.println(helpText);
                            System.exit(0);
                        }
                    }
                    */
                    else if (argv[i].equalsIgnoreCase("-NOSTART"))
                    {
                        noStart = true;
                    }
                    else
                    {
                        System.out.println("Invalid parameters");
                        System.out.println(helpText);
                        System.exit(0);
                    }
                }

                if (numPropsSet > 0)
                {
                    updateProperties(newStaxMachineName, newStaxServiceName,
                                     newEventMachineName, newEventServiceName,
                                     newJobParmsDirectory, !noStart);
                }
                else
                {
                    System.out.println(
                        "No properties specified to be updated");
                }

                if (noStart)
                {
                    System.exit(0);
                }
            }
            else
            {
                System.out.println("Invalid parameters");
                System.out.println(helpText);
                System.exit(0);
            }
        }

        ImageIcon image = new ImageIcon(splashURL);
        SplashScreen splash = new SplashScreen(image, "Initializing...   " +
                                               kSplashText);
        splash.run();
        try
        {
            Thread.sleep(1000);
        }
        catch (InterruptedException ex)
        {
        }

        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                exit();
                System.exit(0);
            }
        });

        File propertiesFile = new File(fPropertiesFileName);

        if (!propertiesFile.exists())
        {
            fStaxMachineName = "local";
            fStaxServiceName = "STAX";
            fIsSTAXServiceLocal = true;

            // Get Event machine/service based on STAX machine/service
            
            EventServiceInfo eventServiceInfo = new EventServiceInfo(
                fStaxMachineName, fStaxServiceName, this, true);

            fEventMachineName = eventServiceInfo.getMachine();
            fEventServiceName = eventServiceInfo.getService();

            fPropertiesCancelButton.setEnabled(false);

            createPropertiesDialog();
            fPropertiesDialog.setVisible(true);

            // At this point the user has clicked on "Save" in the properties
            // dialog, so resolveMachineNames() and saveProperties() have
            // already been called in the actionPerformed code
        }
        else
        {
            createPropertiesDialog();
            loadProperties();
            fPropertiesAtStartup = false;
            
            // This can be true if the properties file is corrupt and could
            // not be read
            
            if (fStaxMachineName.equals(""))
            {
                fStaxMachineName = "local";
                fStaxServiceName = "STAX";
                fIsSTAXServiceLocal = true;

                // Get Event machine/service based on STAX machine/service
            
                EventServiceInfo eventServiceInfo = new EventServiceInfo(
                    fStaxMachineName, fStaxServiceName, this, true);

                fEventMachineName = eventServiceInfo.getMachine();
                fEventServiceName = eventServiceInfo.getService();
                
                resolveMachineNames();
                
                saveProperties();
            }
        }

        getContentPane().setLayout(new BorderLayout());
        getContentPane().setBackground(Color.white);

        JMenuBar mainMenuBar = new JMenuBar();
        setJMenuBar(mainMenuBar);
        fFileMenu = new JMenu("File");
        mainMenuBar.add(fFileMenu);
        fDisplayMenu = new JMenu("Display");
        mainMenuBar.add(fDisplayMenu);
        fHelpMenu = new JMenu("Help");
        mainMenuBar.add(fHelpMenu);
        fHelpAbout = new JMenuItem("About");
        fHelpAbout.addActionListener(this);
        fJobParametersMenuItem = new JMenuItem("Submit New Job...");
        fJobParametersMenuItem.addActionListener(this);
        fStartNewJobMenuItem = new JMenuItem("Resubmit Previous Job");
        fStartNewJobMenuItem.addActionListener(this);
        fFileProperties = new JMenuItem("Properties...");
        fFileProperties.addActionListener(this);
        fFileExit = new JMenuItem("Exit");
        fFileExit.addActionListener(this);
        fDisplayServiceLog = new JMenuItem("Display STAX Service Log");
        fDisplayServiceLog.addActionListener(this);
        fDisplaySelectedJobLog = new JMenuItem("Display Selected Job's Log");
        fDisplaySelectedJobLog.addActionListener(this);
        fDisplaySelectedJobLog.setEnabled(false);
        fDisplaySelectedJobUserLog = new
            JMenuItem("Display Selected Job's User Log");
        fDisplaySelectedJobUserLog.addActionListener(this);
        fDisplaySelectedJobUserLog.setEnabled(false);
        fDisplayRecentLogs = new JMenu("Display Recent Logs");
        
        fDisplayJobLog = new JMenuItem("Display Job Log...");
        fDisplayJobLog.addActionListener(this);
        fDisplayJobUserLog = new JMenuItem("Display Job User Log...");
        fDisplayJobUserLog.addActionListener(this);

        // Verify that STAF V3.2.1 or later is running on the STAX Monitor in
        // order to allow the Display JVM Log options to be selectable (as
        // the STAFJVMLogViewer was added in STAF V3.2.1.

        boolean jvmLogSupport = true;

        try
        {
            result = STAFUtil.compareSTAFVersion(
                "local", fHandle, "3.2.1");

            if (result.rc != STAFResult.Ok)
            {
                if (result.rc == STAFResult.InvalidSTAFVersion)
                {
                    // Minimum required STAF version for the STAX Monitor's
                    // "Display JVM Log" options is not running
                    jvmLogSupport = false;
                }
                else
                {
                    JOptionPane.showMessageDialog(
                        this, "Error verifying STAF version. RC: " +
                        result.rc + ", Additional info: " + result.result,
                        "Error verifying STAF version",
                        JOptionPane.ERROR_MESSAGE);
                }
            }
        }
        catch (Error err)
        {
            // Ignore as this error would have already been caught
        }
        
        fDisplaySTAXJVMLog = new JMenuItem("Display STAX JVM Log");
        fDisplaySTAXJVMLog.addActionListener(this);
        fDisplayOtherJVMLog = new JMenuItem("Display Other JVM Log...");
        fDisplayOtherJVMLog.addActionListener(this);

        if (!jvmLogSupport)
        {
            // Grey out these menu items
            fDisplaySTAXJVMLog.setEnabled(false);
            fDisplayOtherJVMLog.setEnabled(false);
        }
        
        fHelpMenu.add(fHelpAbout);
        fFileMenu.add(fFileProperties);
        fFileMenu.insertSeparator(1);
        fFileMenu.add(fJobParametersMenuItem);
        fFileMenu.add(fStartNewJobMenuItem);
        fFileMenu.insertSeparator(4);
        fFileMenu.add(fFileExit);
        fDisplayMenu.add(fDisplayServiceLog);
        fDisplayMenu.insertSeparator(1);
        fDisplayMenu.add(fDisplaySelectedJobLog);
        fDisplayMenu.add(fDisplaySelectedJobUserLog);
        fDisplayMenu.insertSeparator(5);
        fDisplayMenu.add(fDisplayRecentLogs);
        fDisplayMenu.insertSeparator(6);
        fDisplayMenu.add(fDisplayJobLog);
        fDisplayMenu.add(fDisplayJobUserLog);
        fDisplayMenu.insertSeparator(9);
        fDisplayMenu.add(fDisplaySTAXJVMLog);
        fDisplayMenu.add(fDisplayOtherJVMLog);

        JPanel mainPanel = new JPanel();
        mainPanel.setLayout(new BorderLayout());

        JPanel jobsPanel = new JPanel();
        jobsPanel.setLayout(new BorderLayout());
        jobsPanel.setBorder(new TitledBorder("Active Jobs"));

        fActiveJobColumns = new Vector();
        fActiveJobColumns.addElement(" Job ID ");
        fActiveJobColumns.addElement(" Job Name ");
        fActiveJobColumns.addElement(" Monitored ");
        fActiveJobColumns.addElement(" Function ");
        fActiveJobColumns.addElement(" Status ");
        fActiveJobColumns.addElement(" Started ");
        fActiveJobColumns.addElement(" Elapsed Time ");
        fActiveJobColumns.addElement(" Result ");

        fActiveJobsTableModel =
            new STAXMonitorTableModel(fActiveJobColumns, 0);

        fActiveJobsModelSorter =
            new STAXMonitorTableSorter(fActiveJobsTableModel, 0);
        fActiveJobsTable = new JTable(fActiveJobsModelSorter);
        fActiveJobsModelSorter.
            addMouseListenerToHeaderInTable(fActiveJobsTable, 7);
        fActiveJobsTable.addMouseListener(this);
        fActiveJobsTable.getSelectionModel().addListSelectionListener(this);

        updateJobTableRenderers();

        fActiveJobsTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        fActiveJobsTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        fActiveJobsTable.setRowSelectionAllowed(true);

        STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);

        JScrollPane jobsScrollPane = new JScrollPane(fActiveJobsTable);
        jobsPanel.add(jobsScrollPane);

        JPanel startNewJobPanel = new JPanel();
        startNewJobPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));

        fStartNewJobButton.addActionListener(this);
        startNewJobPanel.add(fStartNewJobButton);
        startNewJobPanel.add(Box.createHorizontalStrut(20));
        fSubmitLastJobButton.addActionListener(this);
        startNewJobPanel.add(fSubmitLastJobButton);

        mainPanel.add(BorderLayout.CENTER, jobsPanel);
        mainPanel.add(BorderLayout.SOUTH, startNewJobPanel);

        fJobPopupMenu.add(fJobShowMonitorMenuItem);
        fJobShowMonitorMenuItem.addActionListener(this);
        fJobPopupMenu.add(fJobStartMonitorMenuItem);
        fJobStartMonitorMenuItem.addActionListener(this);
        fJobPopupMenu.addSeparator();
        fJobPopupMenu.add(fJobShowSTAXServiceLogMenuItem);
        fJobShowSTAXServiceLogMenuItem.addActionListener(this);
        fJobPopupMenu.addSeparator();
        fJobPopupMenu.add(fJobShowJobLogMenuItem);
        fJobShowJobLogMenuItem.addActionListener(this);
        fJobPopupMenu.add(fJobShowJobUserLogMenuItem);
        fJobShowJobUserLogMenuItem.addActionListener(this);
        fJobPopupMenu.addSeparator();
        fJobPopupMenu.add(fJobTerminateJobMenuItem);
        fJobTerminateJobMenuItem.addActionListener(this);

        // Get STAX service version

        String versionRequest = "VERSION";

        STAFResult versionResult = fHandle.submit2(
            fStaxMachineName, fStaxServiceName, versionRequest);

        if (versionResult.rc == 0)
        {
            String staxServiceVersion = versionResult.result;
            fSTAXServiceVersion = new STAFVersion(staxServiceVersion);
            
            // Check if STAX service is at the required version

            STAFVersion requiredVersion1 = new STAFVersion(
                fServiceRequiredVersion);

            if (fSTAXServiceVersion.compareTo(requiredVersion1) < 0)
            {
                JOptionPane.showMessageDialog(this,
                    "WARNING: STAX service machine is not at STAX version " +
                    requiredVersion1 + " or later.\n" + 
                    "machine=" + fStaxMachineName + ", service=" +
                    fStaxServiceName + ", version=" + fSTAXServiceVersion,
                    "Incorrect STAX service version",
                    JOptionPane.ERROR_MESSAGE);
            }
            else
            {
                // Load the extensions before displaying the main STAX Monitor panel
                loadExtensions();
            }
        }
        
        // Handle -extensions parameter

        if (argv.length > 0 && argv[0].equalsIgnoreCase("-EXTENSIONS"))
        {
            // Display the registered STAX Monitor Extensions

            displayMonitorExtensions();
        }

        // Get the endpoint for the STAX Monitor machine that the STAX service
        // machine will use when communicating with the STAX Monitor machine
        
        STAFResult whoamiResult = fHandle.submit2(
            fStaxMachineName, "MISC", "WHOAMI");

        if (whoamiResult.rc != 0)
        {
            System.out.println(
                "Error submitting MISC WHOAMI request to the STAX " +
                "service machine " + fStaxMachineName + ", RC: " +
                whoamiResult.rc + ", Result: " + whoamiResult.result);

            // Default to using the local machine's name
            fSTAXMonitorEndpoint = fLocalMachineName;
        }
        else
        {
            Map whoamiMap = (HashMap)whoamiResult.resultObj;
            fSTAXMonitorEndpoint = (String)whoamiMap.get("endpoint");

            // Check if the STAX service machine is a remote machine, and if so,
            // get the current timestamp on the STAX service machine

            String isLocalRequest = (String)whoamiMap.get("isLocalRequest");

            if (isLocalRequest.equalsIgnoreCase("No"))
            {
                STAFResult whoareyouResult = fHandle.submit2(
                    fStaxMachineName, "MISC", "WHOAREYOU");

                long localCurrentMillis = System.currentTimeMillis();

                if (whoareyouResult.rc == 0)
                {
                    Map whoareyouMap = (HashMap)whoareyouResult.resultObj;

                    if (whoareyouMap.containsKey("currentTimestamp"))
                    {
                        String staxMachineCurrentTimestamp =
                            (String)whoareyouMap.get("currentTimestamp");

                        Date staxMachineDate =
                            STAXMonitorUtil.DATE_FORMAT.parse(
                                staxMachineCurrentTimestamp,
                                new ParsePosition(0));

                        Calendar staxMachineCalendar = Calendar.getInstance();
                        staxMachineCalendar.setTime(staxMachineDate);

                        long staxMachineCurrentMillis =
                            staxMachineCalendar.getTimeInMillis();

                        if (staxMachineCurrentMillis < localCurrentMillis)
                        {
                            STAXMonitorUtil.setTimeOffset(
                                0 - (localCurrentMillis -
                                staxMachineCurrentMillis));
                        }
                        else
                        {
                            STAXMonitorUtil.setTimeOffset(
                                staxMachineCurrentMillis -
                                localCurrentMillis);
                        }
                    }
                }
                else
                {
                    System.out.println(
                        "Error submitting MISC WHOAREYOU request to the STAX " +
                        "service machine " + fStaxMachineName + ", RC: " +
                        whoareyouResult.rc + ", Result: " +
                        whoareyouResult.result);
                }
            }
        }

        // Check what the maximum queue size is set to on the STAX Monitor
        // machine and if < MAXQUEUESIZE, increase the maximum queue size
        // so that the STAX Monitor handles' queues won't get full and lose
        // messages if lots of messages are received when running STAX jobs

        result = fHandle.submit2("local", "MISC", "LIST SETTINGS");
        
        if (result.rc != 0)
        {
            JOptionPane.showMessageDialog(
                this, "Error listing STAF settings\n\nRC: " + result.rc +
                "\nResult: " + result.result,
                "Error listing STAF settings", JOptionPane.ERROR_MESSAGE);   
        }
        else
        {
            try
            {
                Map resultMap = (Map)result.resultObj;
                String maxQueueSizeString = (String)resultMap.get(
                    "maxQueueSize");

                int maxQueueSize =
                    (new Integer(maxQueueSizeString)).intValue();

                if (maxQueueSize < MAXQUEUESIZE)
                {
                    result = fHandle.submit2(
                        "local", "MISC", "SET MAXQUEUESIZE " + MAXQUEUESIZE);

                    if (result.rc != 0)
                    {
                        JOptionPane.showMessageDialog(
                            this, "Error Setting Maximum Queue Size",
                            "An error occurred increasing the maximum " +
                            "queue size to " + MAXQUEUESIZE +
                            "\n\nRC: " + result.rc +
                            "\nResult: " + result.result,
                            JOptionPane.ERROR_MESSAGE);
                    }
                }
            }
            catch (Exception e)
            {
                JOptionPane.showMessageDialog(
                    this, "Error Getting Maximum Queue Size",
                    "An error occurred getting the maximum queue size. " +
                    e.toString(),
                    JOptionPane.ERROR_MESSAGE);
            }
        }

        splash.close();

        fBreakpointTableColumnNames = new Vector();
        fBreakpointTableColumnNames.add("Function");
        fBreakpointTableColumnNames.add("Line #");
        fBreakpointTableColumnNames.add("XML File");
        fBreakpointTableColumnNames.add("Machine");

        getContentPane().add(mainPanel);

        ToolTipManager.sharedInstance().setDismissDelay(10000);

        setTitle("STAX 3 Job Monitor");
        pack();
        setSize(new Dimension(640, 300));
        setVisible(true);

        createStartNewJobDialog();
        loadJobParms(fLastJobParmsFileName);

        registerForJobEvents();

        Runnable runnable = new Runnable()
        {
            public void run()
            {
                seedExistingJobs();
            }
        };

        try
        {
            SwingUtilities.invokeAndWait(runnable);
        }
        catch (InterruptedException e)
        {
        }
        catch (InvocationTargetException e)
        {
        }

        if (!jobIDToMonitor.equals(""))
        {
            repaint();

            synchronized(fActiveJobsModelSorter)
            {
                int rows = fActiveJobsTable.getRowCount();
                int rowIndex = -1;

                for (int r = 0; r < rows; r++)
                {
                    String jobNumber =
                        ((Integer)fActiveJobsTable.getValueAt
                        (r, 0)).toString();

                    if (jobNumber.equals(jobIDToMonitor))
                    {
                        rowIndex = r;
                        break;
                    }
                }

                if (rowIndex == -1)
                {
                    STAXMonitorUtil.showErrorDialog(
                        this, "JobID " + jobIDToMonitor + " was not found.");
                }
                else
                {
                    monitorExistingJob(jobIDToMonitor, rowIndex);
                }
            }
            repaint();
        }

        fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                    fCurrentJobParmsFile);

        fStartNewJobFileSave.setEnabled(false);

        fWizardSaveButton.addActionListener(this);
        fWizardPreviewXMLButton.addActionListener(this);
        fWizardCancelButton.addActionListener(this);

        (new Thread(this)).start();

        fElapsedTime = new MonitorElapsedTime();
        fElapsedTime.start();

        if (!(fStartNewJobParmFileName.equals("")))
        {
            File jobParmsFile = new File(fStartNewJobParmFileName);

            if (!jobParmsFile.exists())
            {
                STAXMonitorUtil.showErrorDialog(
                    this, "Job Parameters File does not exist\n" +
                    "File: " + fStartNewJobParmFileName);

                exit();
                System.exit(0);
            }

            if (!(loadJobParms(fStartNewJobParmFileName)))
            {
                exit();
                System.exit(0);
            }

            submitNewJob();

            fCurrentJobParmsNotSaved = false;
            fCurrentJobParmsFile = fStartNewJobParmFileName;

            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fStartNewJobParmFileName);
        }
    }

    public void loadExtensions()
    {
        // Get, Validate, and Load Monitor Extensions before show main panel

        String errorText = "";
        STAFResult result;

        result = loadLocalExtensions();

        if (result.rc != 0)
        {
            if (!errorText.equals(""))
                errorText += "\n\n";

            errorText += "Errors Validating Local Monitor Extensions:" +
                         result.result;
        }

        result = loadSTAXServiceExtensions();

        if (result.rc != 0)
        {
            if (!errorText.equals(""))
                errorText += "\n\n";

            errorText += "Errors Validating STAX Service Monitor " +
                         "Extensions:" + result.result;
        }

        if (!errorText.equals(""))
        {
            STAXMonitorUtil.showErrorDialog(
                this, errorText, "STAX Monitor Extension Errors");
        }

        // Load STAX Registered Monitor Extensions table information

        Iterator iter = fMonitorExtensionMap.entrySet().iterator();

        while (iter.hasNext())
        {
            Map.Entry entry = (Map.Entry)iter.next();
            String extName = (String)entry.getKey();
            ExtensionInfo extInfo = (ExtensionInfo)entry.getValue();

            Object rowData[] = new Object[fExtensionsColumns.size()];
            rowData[fExtensionsColumns.indexOf(EXT_COLUMN_NAME)] = extName;
            rowData[fExtensionsColumns.indexOf(EXT_COLUMN_VERSION)] =
                extInfo.getVersion();

            if (extInfo.getSource() == STAX_SERVICE_EXTENSION)
            {
                rowData[fExtensionsColumns.indexOf(EXT_COLUMN_SOURCE)] =
                    fStaxMachineName;
            }
            else
            {
                rowData[fExtensionsColumns.indexOf(EXT_COLUMN_SOURCE)] =
                    extInfo.getSourceStr();
            }

            rowData[fExtensionsColumns.indexOf(EXT_COLUMN_JARFILE)] =
                extInfo.getJarFileName();
            rowData[fExtensionsColumns.indexOf(EXT_COLUMN_OVERRIDES)] =
                extInfo.getOverriddenJarFileName();
            rowData[fExtensionsColumns.indexOf(EXT_COLUMN_PREREQ)] =
                extInfo.getRequiredMonitorVersion();
            rowData[fExtensionsColumns.indexOf(EXT_COLUMN_DESCRIPTION)] =
                extInfo.getDescription();

            fExtensionsTableModel.addRow(rowData);
        }

        fExtensionsTable.updateUI();
        STAXMonitorUtil.updateRowHeights(fExtensionsTable, 1);
        STAXMonitorUtil.sizeColumnsToFitText(fExtensionsTable);
    }

    public void displayMonitorExtensions()
    {

        // Display the registered STAX Monitor Extensions

        StringBuffer output = new StringBuffer(
            "\nRegistered STAX Monitor Extensions:\n\n");

        if (!fMonitorExtensionMap.isEmpty())
        {
            Iterator iter = fMonitorExtensionMap.entrySet().iterator();

            while (iter.hasNext())
            {
                Map.Entry entry = (Map.Entry)iter.next();
                String extName = (String)entry.getKey();
                ExtensionInfo extInfo = (ExtensionInfo)entry.getValue();

                String sourceMachine;
                if (extInfo.getSource() == STAX_SERVICE_EXTENSION)
                    sourceMachine = fStaxMachineName;
                else
                    sourceMachine = extInfo.getSourceStr();

                output.append("Monitor Extension     : " + extName +
                    "\nVersion               : " + extInfo.getVersion() +
                    "\nSource Machine        : " + sourceMachine +
                    "\nJar File Name         : " + extInfo.getJarFileName() +
                    "\nOverridden Jar File   : " +
                    extInfo.getOverriddenJarFileName() +
                    "\nMonitor Version Prereq: " +
                    extInfo.getRequiredMonitorVersion() +
                    "\nDescription           : " + extInfo.getDescription() +
                    "\n\n");
            }
        }
        else
        {
            output.append("None\n\n");
        }

        System.out.println(output.toString());
    }

    public void createPropertiesDialog()
    {
        fPropertiesDialog = new JDialog(this, "STAX Monitor Properties", true);
        JPanel propertiesPanel = new JPanel();
        propertiesPanel.setLayout(new
            BoxLayout(propertiesPanel, BoxLayout.Y_AXIS));

        JTabbedPane propertiesTabbedPane = new JTabbedPane();

        JPanel staxMachinePanel = new JPanel();
        staxMachinePanel.setBorder(new TitledBorder("STAX Machine"));
        staxMachinePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        fStaxMachineNameField = new JTextField(19);
        fStaxMachineNameField.setText(fStaxMachineName);
        fStaxServiceNameField = new JTextField(7);
        fStaxServiceNameField.setText(fStaxServiceName);

        staxMachinePanel.add(Box.createHorizontalStrut(5));
        staxMachinePanel.add(new JLabel("Machine: "));
        staxMachinePanel.add(fStaxMachineNameField);
        staxMachinePanel.add(Box.createHorizontalStrut(5));
        staxMachinePanel.add(new JLabel("STAX Service Name: "));
        staxMachinePanel.add(fStaxServiceNameField);
        staxMachinePanel.add(Box.createHorizontalStrut(5));

        JPanel eventMachinePanel = new JPanel();
        eventMachinePanel.setBorder(new TitledBorder("Event Machine"));
        eventMachinePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        fEventMachineNameField = new JTextField(19);
        fEventMachineNameField.setText(fEventMachineName);
        fEventMachineNameField.setEditable(false);
        fEventServiceNameField = new JTextField(7);
        fEventServiceNameField.setText(fEventServiceName);
        fEventServiceNameField.setEditable(false);

        eventMachinePanel.add(Box.createHorizontalStrut(5));
        eventMachinePanel.add(new JLabel("Machine: "));
        eventMachinePanel.add(fEventMachineNameField);
        eventMachinePanel.add(Box.createHorizontalStrut(5));
        eventMachinePanel.add(new JLabel("Event Service Name: "));
        eventMachinePanel.add(fEventServiceNameField);

        JPanel propertiesButtonPanel = new JPanel();
        propertiesButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        propertiesButtonPanel.add(fPropertiesOKButton);
        propertiesButtonPanel.add(Box.createHorizontalStrut(20));
        propertiesButtonPanel.add(fPropertiesCancelButton);

        fPropertiesOKButton.addActionListener(this);
        fPropertiesCancelButton.addActionListener(this);

        // Create Extensions panel

        JPanel extensionsPanel = new JPanel(new GridLayout(1,0));
        extensionsPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        extensionsPanel.setBorder(new TitledBorder("STAX Monitor Extensions"));
        extensionsPanel.add(Box.createHorizontalStrut(10));

        fExtensionsColumns = new Vector();
        fExtensionsColumns.addElement(EXT_COLUMN_NAME);
        fExtensionsColumns.addElement(EXT_COLUMN_VERSION);
        fExtensionsColumns.addElement(EXT_COLUMN_SOURCE);
        fExtensionsColumns.addElement(EXT_COLUMN_JARFILE);
        fExtensionsColumns.addElement(EXT_COLUMN_OVERRIDES);
        fExtensionsColumns.addElement(EXT_COLUMN_PREREQ);
        fExtensionsColumns.addElement(EXT_COLUMN_DESCRIPTION);

        fExtensionsTableModel = new STAXMonitorTableModel(
            fExtensionsColumns, 0);
        fExtensionsModelSorter = new STAXMonitorTableSorter(
            fExtensionsTableModel, 0);
        fExtensionsTable = new JTable(fExtensionsModelSorter);
        fExtensionsModelSorter.addMouseListenerToHeaderInTable(
            fExtensionsTable, 1);
        fExtensionsTable.addMouseListener(this);
        fExtensionsTable.getSelectionModel().addListSelectionListener(this);
        updateExtensionsTableRenderers();
        fExtensionsTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        fExtensionsTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        fExtensionsTable.setRowSelectionAllowed(true);
        STAXMonitorUtil.sizeColumnsToFitText(fExtensionsTable);

        JScrollPane extensionsScrollPane = new JScrollPane(fExtensionsTable);
        extensionsScrollPane.setPreferredSize(new Dimension(620,220));
        extensionsPanel.add(extensionsScrollPane);

        // Create Extension Jar Files Panel

        fLocalExtJarFiles = new Vector();
        JPanel pluginJarsPanel = new JPanel();
        pluginJarsPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        pluginJarsPanel.setBorder(new TitledBorder(
            "Local Extension Jar Files"));
        pluginJarsPanel.add(Box.createHorizontalStrut(10));

        fPluginJarsList = new JList(fLocalExtJarFiles);
        fPluginJarsList.setModel(new DefaultListModel());
        fPluginJarsList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        fPluginJarsList.setCellRenderer(new STAXJobListCellRenderer());
        fPluginJarsList.addMouseListener(this);

        JPanel pluginJarsButtonPanel = new JPanel();
        pluginJarsButtonPanel.setLayout(new
            BoxLayout(pluginJarsButtonPanel, BoxLayout.Y_AXIS));
        fPluginJarsAddButton = new JButton("Add...");
        fPluginJarsAddButton.addActionListener(this);
        fPluginJarsDeleteButton = new JButton("Delete");
        fPluginJarsDeleteButton.addActionListener(this);
        fPluginJarsDeleteAllButton = new JButton("Delete All");
        fPluginJarsDeleteAllButton.addActionListener(this);
        pluginJarsButtonPanel.add(fPluginJarsAddButton);
        pluginJarsButtonPanel.add(Box.createVerticalStrut(5));
        pluginJarsButtonPanel.add(fPluginJarsDeleteButton);
        pluginJarsButtonPanel.add(Box.createVerticalStrut(5));
        pluginJarsButtonPanel.add(fPluginJarsDeleteAllButton);

        JScrollPane fPluginJarsScrollPane = new JScrollPane(fPluginJarsList);
        fPluginJarsScrollPane.setPreferredSize(new Dimension(500,220));

        pluginJarsPanel.add(fPluginJarsScrollPane);
        pluginJarsPanel.add(Box.createHorizontalStrut(10));
        pluginJarsPanel.add(pluginJarsButtonPanel);

        fAddPluginJarsDialog = new JDialog(this,
            "Add Extension Jar File", true);
        fAddPluginJarsDialog.setSize(new Dimension(400, 115));
        JPanel addPluginJarsPanel = new JPanel();
        addPluginJarsPanel.setLayout(new BorderLayout());
        fAddPluginJarsTextField = new JTextField(15);
        fAddPluginJarsTextField.setBorder(new TitledBorder(
            "Enter extension jar file name here"));
        addPluginJarsPanel.add(BorderLayout.CENTER,
            new JScrollPane(fAddPluginJarsTextField));

        fAddPluginJarsBrowseButton = new JButton("Browse...");
        fAddPluginJarsAddButton = new JButton("Add");
        fAddPluginJarsCancelButton = new JButton("Cancel");

        JPanel addPluginJarsButtonPanel = new JPanel();
        addPluginJarsButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addPluginJarsButtonPanel.add(fAddPluginJarsBrowseButton);
        addPluginJarsButtonPanel.add(Box.createHorizontalStrut(20));
        addPluginJarsButtonPanel.add(fAddPluginJarsAddButton);
        addPluginJarsButtonPanel.add(Box.createHorizontalStrut(20));
        addPluginJarsButtonPanel.add(fAddPluginJarsCancelButton);

        addPluginJarsPanel.add(BorderLayout.SOUTH, addPluginJarsButtonPanel);

        fAddPluginJarsAddButton.addActionListener(this);
        fAddPluginJarsCancelButton.addActionListener(this);
        fAddPluginJarsBrowseButton.addActionListener(this);

        fAddPluginJarsDialog.getContentPane().add(addPluginJarsPanel);

        fEditPluginJarsDialog = new JDialog(this,
           "Edit Plugin Jar", true);
        fEditPluginJarsDialog.setSize(new Dimension(400, 115));
        JPanel editPluginJarsPanel = new JPanel();
        editPluginJarsPanel.setLayout(new BorderLayout());
        fEditPluginJarsTextField = new JTextField(15);
        fEditPluginJarsTextField.setBorder(new TitledBorder(
            "Update extension jar file name here"));
        editPluginJarsPanel.add(BorderLayout.CENTER,
                           new JScrollPane(fEditPluginJarsTextField));

        fEditPluginJarsSaveButton = new JButton("Save");
        fEditPluginJarsCancelButton = new JButton("Cancel");

        JPanel editPluginJarsButtonPanel = new JPanel();
        editPluginJarsButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        editPluginJarsButtonPanel.add(fEditPluginJarsSaveButton);
        editPluginJarsButtonPanel.add(Box.createHorizontalStrut(20));
        editPluginJarsButtonPanel.add(fEditPluginJarsCancelButton);

        editPluginJarsPanel.add(BorderLayout.SOUTH, editPluginJarsButtonPanel);

        fEditPluginJarsSaveButton.addActionListener(this);
        fEditPluginJarsCancelButton.addActionListener(this);

        fEditPluginJarsDialog.getContentPane().add(editPluginJarsPanel);

        // Create Options panel

        JPanel optionsPanel = new JPanel();
        optionsPanel.setBorder(new TitledBorder("Options"));
        optionsPanel.setLayout(new BoxLayout(optionsPanel,
            BoxLayout.Y_AXIS));

        JPanel displayProcessMonitorPanel = new JPanel();
        displayProcessMonitorPanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel processMonitor = new JLabel("Update Process Monitor "
            + "information every  ");
        processMonitor.setForeground(Color.black);
        displayProcessMonitorPanel.add(processMonitor);

        fProcessMonitorSecondsField = new JTextField(5);
        fProcessMonitorSecondsField.setText(fDefaultProcessMonitorSeconds);
        displayProcessMonitorPanel.add(fProcessMonitorSecondsField);
        JLabel secondsLabel = new JLabel("  seconds");
        secondsLabel.setForeground(Color.black);
        displayProcessMonitorPanel.add(secondsLabel);

        JPanel displayElapsedTimePanel = new JPanel();
        displayElapsedTimePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel elapsedTime = new JLabel("Update Elapsed Time " + "every  ");
        elapsedTime.setForeground(Color.black);
        displayElapsedTimePanel.add(elapsedTime);

        fElapsedTimeSecondsField = new JTextField(5);
        fElapsedTimeSecondsField.setText(fDefaultElapsedTimeSeconds);
        secondsLabel = new JLabel("  seconds");
        secondsLabel.setForeground(Color.black);
        displayElapsedTimePanel.add(fElapsedTimeSecondsField);
        displayElapsedTimePanel.add(secondsLabel);

        JPanel monitorInfoPanel = new JPanel();
        monitorInfoPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        monitorInfoPanel.add(fShowNoSTAXMonitorInformation);

        JPanel limitMessagesPanel = new JPanel();
        limitMessagesPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));

        fLimitMessagesField = new JTextField(5);
        fLimitMessagesField.setText(fDefaultLimitMessagesText);

        limitMessagesPanel.add(fLimitMessages);
        limitMessagesPanel.add(fLimitMessagesField);

        fMessageFontNameCB = new JComboBox();
        fLogViewerFontNameCB = new JComboBox();

        // Add the Dialog font name as the first item in the font combo box
        // as the first item is the default selection if no selection is made

        fMessageFontNameCB.addItem("Dialog");
        fLogViewerFontNameCB.addItem("Dialog");

        // Get the names of all available fonts and add to the font comboboxes

        GraphicsEnvironment env = GraphicsEnvironment.
            getLocalGraphicsEnvironment();
        String[] fontNames = env.getAvailableFontFamilyNames();
        
        for (int i = 0; i < fontNames.length; i++)
        {            if (!fontNames[i].equals("Dialog"))
            {
                fMessageFontNameCB.addItem(fontNames[i]);
                fLogViewerFontNameCB.addItem(fontNames[i]);
            }
        }

        JPanel messageFontNamePanel = new JPanel();
        messageFontNamePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel messageFontName = new JLabel("Messages Font Name:    ");
        fMessageFontNameCB.setFont(new Font("Dialog", Font.PLAIN, 12));
        fMessageFontNameCB.setBackground(Color.white);
        fMessageFontNameCB.addActionListener(this);
        messageFontNamePanel.add(messageFontName);
        messageFontNamePanel.add(fMessageFontNameCB);

        JPanel logViewerFontNamePanel = new JPanel();
        logViewerFontNamePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel logViewerFontName = new JLabel("Log Viewer Font Name:  ");
        fLogViewerFontNameCB.setFont(new Font("Dialog", Font.PLAIN, 12));
        fLogViewerFontNameCB.setBackground(Color.white);
        logViewerFontNamePanel.add(logViewerFontName);
        logViewerFontNamePanel.add(fLogViewerFontNameCB);

        JPanel saveAsDirectoryPanel = new JPanel();
        saveAsDirectoryPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel saveAsDirectoryLabel = new JLabel(
            "Log Viewer Save As Directory:  ");
        saveAsDirectoryLabel.setForeground(Color.black);
        saveAsDirectoryPanel.add(saveAsDirectoryLabel);

        fSaveAsDirectoryField = new JTextField(20);

        if (fSaveAsDirectory != null)
            fSaveAsDirectoryField.setText(fSaveAsDirectory);

        saveAsDirectoryPanel.add(fSaveAsDirectoryField);

        fSaveAsDirectoryBrowseButton = new JButton("Browse...");
        fSaveAsDirectoryBrowseButton.addActionListener(this);
        saveAsDirectoryPanel.add(Box.createHorizontalStrut(5));
        saveAsDirectoryPanel.add(fSaveAsDirectoryBrowseButton);
        
        JPanel testcasesPanel = new JPanel();
        testcasesPanel.setBorder(new TitledBorder("Testcase Options"));
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc = new GridBagConstraints();
        testcasesPanel.setLayout(gbl);

        JPanel testcaseDisplayColumnsPanel = new JPanel();
        testcaseDisplayColumnsPanel.setLayout(new
            FlowLayout(FlowLayout.LEFT));
        JLabel testcaseDisplayColumns =
            new JLabel("Select the columns that will appear on the " +
            "\"Testcase Info\" panel:  ",
            SwingConstants.LEFT);
        testcaseDisplayColumnsPanel.add(testcaseDisplayColumns);

        gbc.anchor = GridBagConstraints.NORTHWEST;
        gbc.gridwidth = 3;
        testcasesPanel.add(testcaseDisplayColumnsPanel, gbc);
        testcasesPanel.add(Box.createHorizontalStrut(3), gbc);

        JLabel tempLabel = new JLabel("");

        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx = 1.0;
        testcasesPanel.add(tempLabel, gbc);

        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(1), gbc);

        JPanel testcaseColumnsPanel1 = new JPanel();
        testcaseColumnsPanel1.setLayout(new
            BoxLayout(testcaseColumnsPanel1, BoxLayout.X_AXIS));
        fTestcaseNameCB = new JCheckBox("Name", true);
        testcaseColumnsPanel1.add(Box.createHorizontalStrut(30), gbc);
        testcaseColumnsPanel1.add(fTestcaseNameCB);

        fTestcaseStatusDateTimeCB = new JCheckBox("Status Date-Time", false);
        testcaseColumnsPanel1.add(Box.createHorizontalStrut(99), gbc);
        testcaseColumnsPanel1.add(fTestcaseStatusDateTimeCB);

        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(testcaseColumnsPanel1, gbc);
        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(2), gbc);

        JPanel testcaseColumnsPanel2 = new JPanel();
        testcaseColumnsPanel2.setLayout(new
            BoxLayout(testcaseColumnsPanel2, BoxLayout.X_AXIS));
        fTestcasePassCB = new JCheckBox("PASS", true);
        testcaseColumnsPanel2.add(Box.createHorizontalStrut(30), gbc);
        testcaseColumnsPanel2.add(fTestcasePassCB);

        fTestcaseDurationCB = new JCheckBox("Duration", true);
        testcaseColumnsPanel2.add(Box.createHorizontalStrut(100), gbc);
        testcaseColumnsPanel2.add(fTestcaseDurationCB);

        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(testcaseColumnsPanel2, gbc);
        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(2), gbc);

        JPanel testcaseColumnsPanel3 = new JPanel();
        testcaseColumnsPanel3.setLayout(new
            BoxLayout(testcaseColumnsPanel3, BoxLayout.X_AXIS));

        fTestcaseFailCB = new JCheckBox("FAIL", true);
        testcaseColumnsPanel3.add(Box.createHorizontalStrut(30), gbc);
        testcaseColumnsPanel3.add(fTestcaseFailCB);

        fTestcaseStartsCB = new JCheckBox("Starts", true);
        testcaseColumnsPanel3.add(Box.createHorizontalStrut(108), gbc);
        testcaseColumnsPanel3.add(fTestcaseStartsCB);

        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(testcaseColumnsPanel3, gbc);
        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(2), gbc);

        JPanel testcaseColumnsPanel4 = new JPanel();
        testcaseColumnsPanel4.setLayout(new
            BoxLayout(testcaseColumnsPanel4, BoxLayout.X_AXIS));

        fTestcaseStartDateTimeCB = new JCheckBox("Start Date-Time", true);
        testcaseColumnsPanel4.add(Box.createHorizontalStrut(30), gbc);
        testcaseColumnsPanel4.add(fTestcaseStartDateTimeCB);

        fTestcaseInformationCB = new JCheckBox("Information", true);
        testcaseColumnsPanel4.add(Box.createHorizontalStrut(43), gbc);
        testcaseColumnsPanel4.add(fTestcaseInformationCB);

        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(testcaseColumnsPanel4, gbc);
        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(3), gbc);

        fTestcaseNameCB.addActionListener(this);
        fTestcasePassCB.addActionListener(this);
        fTestcaseFailCB.addActionListener(this);
        fTestcaseStartDateTimeCB.addActionListener(this);
        fTestcaseStatusDateTimeCB.addActionListener(this);
        fTestcaseDurationCB.addActionListener(this);
        fTestcaseStartsCB.addActionListener(this);
        fTestcaseInformationCB.addActionListener(this);

        JPanel testcaseSortPanel = new JPanel();
        testcaseSortPanel.setLayout(new FlowLayout(FlowLayout.LEFT));

        JLabel testcaseSortColumn = new JLabel("Default Sort Column:  ");
        fTestcaseSortColumnCB = new JComboBox();
        fTestcaseSortColumnCB.setFont(new Font("Dialog", Font.PLAIN, 12));
        fTestcaseSortColumnCB.setBackground(Color.white);

        JLabel testcaseSortOrder = new JLabel("  Sort Order:  ");
        fTestcaseSortOrderCB = new JComboBox();
        fTestcaseSortOrderCB.setFont(new Font("Dialog", Font.PLAIN, 12));
        fTestcaseSortOrderCB.setBackground(Color.white);

        fTestcaseSortColumnCB.addItem("Name");
        fTestcaseSortColumnCB.addItem("PASS");
        fTestcaseSortColumnCB.addItem("FAIL");
        fTestcaseSortColumnCB.addItem("Start Date-Time");
        fTestcaseSortColumnCB.addItem("Status Date-Time");
        fTestcaseSortColumnCB.addItem("Duration");
        fTestcaseSortColumnCB.addItem("Starts");
        fTestcaseSortColumnCB.addItem("Information");
        fTestcaseSortColumnCB.setSelectedIndex(3);

        fTestcaseSortOrderCB.addItem("Ascending");
        fTestcaseSortOrderCB.addItem("Descending");

        testcaseSortPanel.add(testcaseSortColumn);
        testcaseSortPanel.add(fTestcaseSortColumnCB);
        testcaseSortPanel.add(testcaseSortOrder);
        testcaseSortPanel.add(fTestcaseSortOrderCB);

        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(testcaseSortPanel, gbc);
        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(3), gbc);

        fTestcaseAutoResizeCB = new
            JCheckBox("Automatically resize table columns", true);
        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(fTestcaseAutoResizeCB, gbc);
        gbc.weightx = 0;
        testcasesPanel.add(Box.createVerticalStrut(3), gbc);

        JPanel restoreTestcaseDefaultSettingsPanel = new JPanel();
        restoreTestcaseDefaultSettingsPanel.setLayout(new
            BoxLayout(restoreTestcaseDefaultSettingsPanel, BoxLayout.Y_AXIS));
        JButton restoreTestcaseDefaultSettingsButton =
            new JButton("Restore Default Settings");
        restoreTestcaseDefaultSettingsPanel.add(Box.createHorizontalStrut(475));
        restoreTestcaseDefaultSettingsPanel.add(
            restoreTestcaseDefaultSettingsButton);
        gbc.gridwidth = 1;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        testcasesPanel.add(restoreTestcaseDefaultSettingsPanel, gbc);
        gbc.weightx = 0;

        restoreTestcaseDefaultSettingsButton.addActionListener(
            new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                {
                    fTestcaseNameCB.setSelected(true);
                    fTestcasePassCB.setSelected(true);
                    fTestcaseFailCB.setSelected(true);
                    fTestcaseStartDateTimeCB.setSelected(true);
                    fTestcaseStatusDateTimeCB.setSelected(false);
                    fTestcaseDurationCB.setSelected(true);
                    fTestcaseStartsCB.setSelected(true);
                    fTestcaseInformationCB.setSelected(true);
                    fTestcaseSortColumnCB.setSelectedIndex(3);
                    fTestcaseSortOrderCB.setSelectedIndex(0);
                    fTestcaseAutoResizeCB.setSelected(true);
                }
            }
        );

        optionsPanel.add(displayProcessMonitorPanel);
        optionsPanel.add(displayElapsedTimePanel);
        optionsPanel.add(monitorInfoPanel);
        optionsPanel.add(limitMessagesPanel);
        optionsPanel.add(messageFontNamePanel);
        optionsPanel.add(logViewerFontNamePanel);
        optionsPanel.add(saveAsDirectoryPanel);
        
        fShowNoSTAXMonitorInformation.addItemListener(this);
        fLimitMessages.addItemListener(this);

        JPanel machinePanel = new JPanel();
        machinePanel.setLayout(new BoxLayout(machinePanel, BoxLayout.Y_AXIS));
        machinePanel.add(staxMachinePanel);
        machinePanel.add(eventMachinePanel);

        JPanel subjobsPanel = new JPanel();
        subjobsPanel.setBorder(new TitledBorder("Sub-jobs"));
        subjobsPanel.setLayout(new BoxLayout(subjobsPanel,
            BoxLayout.Y_AXIS));

        fAutoMonitorSubjobsRB = new JRadioButton(
            "Automatically monitor sub-jobs", false);
        fAutoMonitorRecommendedSubjobsRB = new JRadioButton(
            "Automatically monitor recommended sub-jobs", true);
        fNeverAutoMonitorSubjobsRB = new JRadioButton(
            "Never automatically monitor sub-jobs", false);
        fAutoMonitorSubjobsRB.addItemListener(this);
        fAutoMonitorRecommendedSubjobsRB.addItemListener(this);
        fNeverAutoMonitorSubjobsRB.addItemListener(this);
        ButtonGroup monitorSubjobsGroup = new ButtonGroup();
        monitorSubjobsGroup.add(fAutoMonitorSubjobsRB);
        monitorSubjobsGroup.add(fAutoMonitorRecommendedSubjobsRB);
        monitorSubjobsGroup.add(fNeverAutoMonitorSubjobsRB);

        subjobsPanel.add(fAutoMonitorSubjobsRB);
        subjobsPanel.add(fAutoMonitorRecommendedSubjobsRB);
        subjobsPanel.add(fNeverAutoMonitorSubjobsRB);

        JPanel infoPanel = new JPanel();
        infoPanel.setBorder(new TitledBorder("STAX Monitor Info"));
        infoPanel.setLayout(new BoxLayout(infoPanel,
            BoxLayout.Y_AXIS));

        propertiesTabbedPane.addTab("Machine Info", machinePanel);
        propertiesTabbedPane.addTab("Options", optionsPanel);
        propertiesTabbedPane.addTab("Testcases", testcasesPanel);
        propertiesTabbedPane.addTab("Sub-jobs", subjobsPanel);
        propertiesTabbedPane.addTab("Extensions", extensionsPanel);
        propertiesTabbedPane.addTab("Extension Jars", pluginJarsPanel);

        // Set tab colors
        for (int i = 0; i < propertiesTabbedPane.getTabCount(); i++)
        {
            if (i == 0)
            {
                propertiesTabbedPane.setBackgroundAt(i, Color.lightGray);
            }
            else
            {
                propertiesTabbedPane.setBackgroundAt(i, Color.white);
                propertiesTabbedPane.setForegroundAt(i, Color.darkGray);
            }
        }

        propertiesPanel.add(new JScrollPane(propertiesTabbedPane));
        propertiesPanel.add(propertiesButtonPanel);

        propertiesTabbedPane.addChangeListener(this);

        fPropertiesDialog.setSize(new Dimension(675, 370));
        fPropertiesDialog.getContentPane().add(
            new JScrollPane(propertiesPanel));
    }

    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == fPropertiesCancelButton)
        {
            fPropertiesDialog.setVisible(false);
        }
        else if (e.getSource() == fPropertiesOKButton)
        {
            if (validateProperties())
            {
                fStaxMachineName = fStaxMachineNameField.getText();
                fStaxServiceName = fStaxServiceNameField.getText();
                fEventMachineName = fEventMachineNameField.getText();
                fEventServiceName = fEventServiceNameField.getText();

                fLimitMessagesFieldText = fLimitMessagesField.getText();
                fProcessMonitorSecondsFieldText =
                    fProcessMonitorSecondsField.getText();
                fElapsedTimeSecondsFieldText =
                    fElapsedTimeSecondsField.getText();

                fMessageFontName = (String)fMessageFontNameCB.getSelectedItem();

                fLogViewerFontName = (String)fLogViewerFontNameCB.
                    getSelectedItem();

                fSaveAsDirectory = fSaveAsDirectoryField.getText();
                    
                fPropertiesDialog.setVisible(false);

                Endpoint staxEndpoint = new Endpoint(fStaxMachineName);
                fStaxMachineInterface = staxEndpoint.getInterface();
                fStaxMachineIdentifier = staxEndpoint.getMachineIdentifier();
                fStaxMachinePort = staxEndpoint.getPort();

                resolveMachineNames();

                fDisplayTestcaseName = fTestcaseNameCB.isSelected();
                fDisplayTestcasePass = fTestcasePassCB.isSelected();
                fDisplayTestcaseFail = fTestcaseFailCB.isSelected();
                fDisplayTestcaseStartDateTime =
                    fTestcaseStartDateTimeCB.isSelected();
                fDisplayTestcaseStatusDateTime =
                    fTestcaseStatusDateTimeCB.isSelected();
                fDisplayTestcaseDuration = fTestcaseDurationCB.isSelected();
                fDisplayTestcaseStarts = fTestcaseStartsCB.isSelected();
                fDisplayTestcaseInformation =
                    fTestcaseInformationCB.isSelected();
                fTestcaseAutoResize = fTestcaseAutoResizeCB.isSelected();
                fTestcaseSortColumn = fTestcaseSortColumnCB.getSelectedIndex();
                fTestcaseSortOrder = fTestcaseSortOrderCB.getSelectedIndex();

                fAutoMonitorSubjobs = fAutoMonitorSubjobsRB.isSelected();
                fAutoMonitorRecommendedSubjobs =
                    fAutoMonitorRecommendedSubjobsRB.isSelected();
                fNeverAutoMonitorSubjobs =
                    fNeverAutoMonitorSubjobsRB.isSelected();

                fLocalExtJarFiles.removeAllElements();

                if (fUpdatedLocalExtJarFiles != null)
                {
                    for (int i = 0; i < fUpdatedLocalExtJarFiles.size(); i++)
                    {
                        fLocalExtJarFiles.addElement(
                            fUpdatedLocalExtJarFiles.elementAt(i));
                    }
                }

                saveProperties();

                // Check if need to restart Monitor due to a property change

                boolean restart = false;

                if (!(fStaxMachineName.equals(fOldStaxMachineName)))
                {
                    restart = true;
                }

                if (!(fStaxServiceName.equals(fOldStaxServiceName)))
                {
                    restart = true;
                }

                if (!(fEventMachineName.equals(fOldEventMachineName)))
                {
                    restart = true;
                }

                if (!(fEventServiceName.equals(fOldEventServiceName)))
                {
                    restart = true;
                }

                if (!(fLocalExtJarFiles.equals(fOldLocalExtJarFiles)))
                {
                    restart = true;
                }

                if (!fPropertiesAtStartup && restart)
                {
                    JOptionPane.showMessageDialog(this,
                         "You must now shutdown and restart the STAX\n" +
                         "Job Monitor to use the updated properties",
                         "STAX Monitor Properties Updated",
                         JOptionPane.WARNING_MESSAGE);
                }

                fOldStaxMachineName = fStaxMachineName;
                fOldStaxServiceName = fStaxServiceName;
                fOldEventMachineName = fEventMachineName;
                fOldEventServiceName = fEventServiceName;

                fOldLocalExtJarFiles = new Vector(fLocalExtJarFiles);
            }
        }
        else if (e.getSource() == fFileExit)
        {
            exit();
            System.exit(0);
        }
        else if (e.getSource() == fStartNewJobFileExit)
        {
            repaint();
            fStartNewJobDialog.dispose();
        }
        else if (e.getSource() == fFileProperties)
        {
            fStaxMachineNameField.setText(fStaxMachineName);
            fStaxServiceNameField.setText(fStaxServiceName);
            fEventMachineNameField.setText(fEventMachineName);
            fEventServiceNameField.setText(fEventServiceName);
            fProcessMonitorSecondsField.setText(
                fProcessMonitorSecondsFieldText);
            fElapsedTimeSecondsField.setText(
                fElapsedTimeSecondsFieldText);
            fShowNoSTAXMonitorInformation.setSelected(
                fShowNoSTAXMonitorInformationBool);
            fLimitMessages.setSelected(fLimitMessagesBool);

            if (fLimitMessagesBool)
                fLimitMessagesField.setText(fLimitMessagesFieldText);

            fMessageFontNameCB.setSelectedItem(fMessageFontName);
            fLogViewerFontNameCB.setSelectedItem(fLogViewerFontName);

            fTestcaseNameCB.setSelected(fDisplayTestcaseName);
            fTestcasePassCB.setSelected(fDisplayTestcasePass);
            fTestcaseFailCB.setSelected(fDisplayTestcaseFail);
            fTestcaseStartDateTimeCB.setSelected(fDisplayTestcaseStartDateTime);
            fTestcaseStatusDateTimeCB.setSelected(fDisplayTestcaseStatusDateTime);
            fTestcaseDurationCB.setSelected(fDisplayTestcaseDuration);
            fTestcaseStartsCB.setSelected(fDisplayTestcaseStarts);
            fTestcaseInformationCB.setSelected(fDisplayTestcaseInformation);
            fTestcaseAutoResizeCB.setSelected(fTestcaseAutoResize);
            fTestcaseSortColumnCB.setSelectedIndex(fTestcaseSortColumn);
            fTestcaseSortOrderCB.setSelectedIndex(fTestcaseSortOrder);

            fAutoMonitorSubjobsRB.setSelected(fAutoMonitorSubjobs);
            fAutoMonitorRecommendedSubjobsRB.setSelected(
                fAutoMonitorRecommendedSubjobs);
            fNeverAutoMonitorSubjobsRB.setSelected(fNeverAutoMonitorSubjobs);

            DefaultListModel pluginJarsListModel = new DefaultListModel();

            for (int i = 0; i < fLocalExtJarFiles.size(); i++)
            {
                pluginJarsListModel.addElement(fLocalExtJarFiles.elementAt(i));
            }

            fPluginJarsList.setModel(pluginJarsListModel);

            fSaveAsDirectoryField.setText(fSaveAsDirectory);
            fPropertiesCancelButton.setEnabled(true);
            fPropertiesDialog.setLocationRelativeTo(this);
            fPropertiesDialog.setVisible(true);
        }
        else if (e.getSource() == fHelpAbout)
        {
            ImageIcon image = new ImageIcon(splashURL);
            SplashScreen splash = new SplashScreen(image, 5000, kSplashText);
            splash.run();
        }
        else if (e.getSource() == fStartNewJobButton)
        {
            showStartNewJobDialog();
        }
        else if (e.getSource() == fJobParametersMenuItem)
        {
            // repaint is needed so that the JMenu will be removed
            repaint();
            showStartNewJobDialog();
        }
        else if (e.getSource() == fStartNewJobCancelButton)
        {
            repaint();
            fStartNewJobDialog.dispose();
        }
        else if ((e.getSource() == fStartNewJobSubmitButton) ||
                 (e.getSource() == fSubmitLastJobButton) ||
                 (e.getSource() == fStartNewJobMenuItem))
        {
            if (validateParms("submit"))
            {
                fLocalXmlFileName = fLocalXmlFileNameField.getText();
                fOtherXmlFileName = fOtherXmlFileNameField.getText();
                fOtherXmlFileMachineName = fOtherXmlFileMachineField.getText();
                fFunction = fFunctionField.getText();
                fArgs = fArguments.getText();
                fJobName = fJobNameField.getText();
                fScriptFilesMachineName =
                    fScriptFilesMachineTextField.getText();

                fStartNewJobDialog.setVisible(false);

                if (!submitNewJob())
                {
                    fStartNewJobDialog.setVisible(true);
                }
            }
        }
        else if (e.getSource() == fStartNewJobTestButton)
        {
            if (validateParms("test"))
            {
                fLocalXmlFileName = fLocalXmlFileNameField.getText();
                fOtherXmlFileName = fOtherXmlFileNameField.getText();
                fOtherXmlFileMachineName = fOtherXmlFileMachineField.getText();
                fFunction = fFunctionField.getText();
                fArgs = fArguments.getText();
                fJobName = fJobNameField.getText();
                fScriptFilesMachineName =
                    fScriptFilesMachineTextField.getText();

                testJob();
            }
        }
        else if (e.getSource() == fStartNewJobWizardButton)
        {
            if (validateParms("read"))
            {
                displayJobWizard();
            }
        }
        else if (e.getSource() == fStartNewJobClearButton)
        {
            fJobName = "";
            fJobNameField.setText(fJobName);

            fFunction = "";
            fFunctionField.setText(fFunction);

            fArgs = "";
            fArguments.setText(fArgs);
            
            fWizardSavedFunctionArgList = new Vector();
            fWizardSavedFileName = "";
            fWizardSavedFileMachineName = "";
            
            fLocalXmlFileName = "";
            fLocalXmlFileNameField.setText(fLocalXmlFileName);

            fOtherXmlFileMachineName = "";
            fOtherXmlFileMachineField.setText(fOtherXmlFileMachineName);

            fOtherXmlFileName = "";
            fOtherXmlFileNameField.setText(fOtherXmlFileName);

            fScriptFilesMachineName = "";
            fScriptFilesMachineTextField.setText(fScriptFilesMachineName);

            ((DefaultListModel)(fScriptList.getModel())).removeAllElements();
            ((DefaultListModel)(fScriptFilesList.getModel())).removeAllElements();

            fMonitorYesRB.setSelected(true);
            fMonitorNoRB.setSelected(false);

            fDefaultFunctionRB.setSelected(true);
            fOtherFunctionRB.setSelected(false);

            fMachineLocalRB.setSelected(true);
            fMachineOtherRB.setSelected(false);

            fLocalScriptMachineRB.setSelected(false);
            fXMLJobFileScriptMachineRB.setSelected(true);
            fOtherScriptMachineRB.setSelected(false);

            fClearLogsYesRB.setSelected(false);
            fClearLogsNoRB.setSelected(false);
            fClearLogsDefaultRB.setSelected(true);

            fLogTCElapsedTimeYesRB.setSelected(false);
            fLogTCElapsedTimeNoRB.setSelected(false);
            fLogTCElapsedTimeDefaultRB.setSelected(true);

            fLogTCNumStartsYesRB.setSelected(false);
            fLogTCNumStartsNoRB.setSelected(false);
            fLogTCNumStartsDefaultRB.setSelected(true);

            fLogTCStartStopYesRB.setSelected(false);
            fLogTCStartStopNoRB.setSelected(false);
            fLogTCStartStopDefaultRB.setSelected(true);

            fPythonOutputCB.setSelectedItem(DEFAULT_STRING);
            fPythonLogLevelCB.setSelectedItem(DEFAULT_STRING);

            int rowCount = fBreakpointsTableModel.getRowCount();

            for(int i = 0; i < rowCount; i++)
            {
                fBreakpointsTableModel.removeRow(0);
            }

            fBreakpointFirstFunctionCB.setSelected(false);
            fBreakpointSubjobFirstFunctionCB.setSelected(false);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fBrowseButton)
        {
            showBrowseFileDialog();
        }
        else if (e.getSource() == fStartNewJobFileSave)
        {
            if (fCurrentJobParmsFile.equals(""))
            {
                showSaveJobParmsFileDialog();
            }
            else
            {
                saveJobParms(fCurrentJobParmsFile);
                fStartNewJobFileSave.setEnabled(false);
                updateRecentFiles(fCurrentJobParmsFile);
                fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                            fCurrentJobParmsFile);
            }
        }
        else if (e.getSource() == fStartNewJobFileSaveAs)
        {
            showSaveJobParmsFileDialog();
        }
        else if (e.getSource() == fStartNewJobFileOpen)
        {
            showOpenJobParmsFileDialog();
        }
        else if (e.getSource() == fScriptAddButton)
        {
            fAddScriptTextArea.setText("");
            fAddScriptTextArea.requestFocus();
            fAddScriptTextArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
            fAddScriptDialog.setLocationRelativeTo(fStartNewJobDialog);
            fAddScriptDialog.setFont(new Font("Monospaced", Font.PLAIN, 12));
            fAddScriptDialog.setVisible(true);
        }
        else if (e.getSource() == fAddScriptAddButton)
        {
            if (fAddScriptTextArea.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                                      "You must enter a value for the Script",
                                      "Error adding script",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            ((DefaultListModel)(fScriptList.getModel())).
                addElement(fAddScriptTextArea.getText());
            fAddScriptDialog.setVisible(false);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fAddScriptCancelButton)
        {
            fAddScriptDialog.setVisible(false);
        }
        else if (e.getSource() == fEditScriptSaveButton)
        {
            if (fEditScriptTextArea.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                                      "You must enter a value for the Script",
                                      "Error saving script",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            int editIndex = fScriptList.getSelectedIndex();
            ((DefaultListModel)(fScriptList.getModel())).
                setElementAt(fEditScriptTextArea.getText(), editIndex);
            fEditScriptDialog.setVisible(false);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fEditScriptCancelButton)
        {
            fEditScriptDialog.setVisible(false);
        }
        else if (e.getSource() == fScriptDeleteAllButton)
        {
            int confirmation = JOptionPane.showConfirmDialog(this,
                 "Are you certain that you\n" +
                 "want to delete all scripts?",
                 "Delete All Scripts",
                 JOptionPane.YES_NO_OPTION,
                 JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            ((DefaultListModel)(fScriptList.getModel())).removeAllElements();
            fScriptVector.removeAllElements();

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fScriptDeleteButton)
        {
            int deleteIndex = fScriptList.getSelectedIndex();
            if (deleteIndex > -1)
            {
                ((DefaultListModel)(fScriptList.getModel())).
                    removeElementAt(deleteIndex);
                fScriptVector.removeElementAt(deleteIndex);

                fCurrentJobParmsNotSaved = true;
                fStartNewJobFileSave.setEnabled(true);
                fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                            fCurrentJobParmsFile + " *");
            }
        }
        else if (e.getSource() == fBreakpointFunctionAddButton)
        {
            fAddBreakpointFunctionTextField.setText("");
            fAddBreakpointFunctionTextField.requestFocus();
            fAddBreakpointFunctionTextField.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointFunctionDialog.setLocationRelativeTo(
                fStartNewJobDialog);
            fAddBreakpointFunctionDialog.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointFunctionDialog.setVisible(true);
        }
        else if (e.getSource() == fAddBreakpointFunctionAddButton)
        {
            if (fAddBreakpointFunctionTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                                      "You must enter a function name",
                                      "Error adding breakpoint function",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            Vector newRow = new Vector();
            newRow.add(fAddBreakpointFunctionTextField.getText());
            newRow.add("");
            newRow.add("");
            newRow.add("");

            fBreakpointsTableModel.addRow(newRow);
            fAddBreakpointFunctionDialog.setVisible(false);

            STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
            STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fAddBreakpointFunctionCancelButton)
        {
            fAddBreakpointFunctionDialog.setVisible(false);
        }
        else if (e.getSource() == fEditBreakpointFunctionSaveButton)
        {
            if (fEditBreakpointFunctionTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                                      "You must enter a function name",
                                      "Error saving breakpoint function",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            int editRow = fBreakpointTable.getSelectedRow();
            fBreakpointsTableModel.
                setValueAt(fEditBreakpointFunctionTextField.getText().trim(),
                           editRow, 0);

            STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
            STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);

            fEditBreakpointFunctionDialog.setVisible(false);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fEditBreakpointFunctionCancelButton)
        {
            fEditBreakpointFunctionDialog.setVisible(false);
        }
        else if (e.getSource() == fBreakpointLineAddButton)
        {
            fAddBreakpointLineNumberTextField.setText("");
            fAddBreakpointLineFileTextField.setText("");
            fAddBreakpointLineMachineTextField.setText("");
            fAddBreakpointLineNumberTextField.requestFocus();
            fAddBreakpointLineNumberTextField.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointLineDialog.setLocationRelativeTo(
                fStartNewJobDialog);
            fAddBreakpointLineDialog.setFont(
                new Font("Dialog", Font.PLAIN, 12));
            fAddBreakpointLineDialog.setVisible(true);
        }
        else if (e.getSource() == fAddBreakpointLineAddButton)
        {
            if (fAddBreakpointLineNumberTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                                      "You must enter a line number",
                                      "Error adding breakpoint line",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            Vector newRow = new Vector();
            newRow.add("");
            newRow.add(fAddBreakpointLineNumberTextField.getText().trim());
            newRow.add(fAddBreakpointLineFileTextField.getText().trim());
            newRow.add(fAddBreakpointLineMachineTextField.getText().trim());

            fBreakpointsTableModel.addRow(newRow);
            fAddBreakpointLineDialog.setVisible(false);

            STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
            STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fAddBreakpointLineCancelButton)
        {
            fAddBreakpointLineDialog.setVisible(false);
        }
        else if (e.getSource() == fEditBreakpointLineSaveButton)
        {
            if (fEditBreakpointLineNumberTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                                      "You must enter a line number",
                                      "Error saving breakpoint line",
                                      JOptionPane.ERROR_MESSAGE);
                return;
            }

            int editRow = fBreakpointTable.getSelectedRow();
            fBreakpointsTableModel.
                setValueAt(fEditBreakpointLineNumberTextField.getText().trim(),
                           editRow, 1);
            fBreakpointsTableModel.
                setValueAt(fEditBreakpointLineFileTextField.getText().trim(),
                           editRow, 2);
            fBreakpointsTableModel.
                setValueAt(fEditBreakpointLineMachineTextField.getText().trim(),
                           editRow, 3);

            STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
            STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);

            fEditBreakpointLineDialog.setVisible(false);

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fEditBreakpointLineCancelButton)
        {
            fEditBreakpointLineDialog.setVisible(false);
        }
        else if (e.getSource() == fBreakpointDeleteAllButton)
        {
            int confirmation = JOptionPane.showConfirmDialog(this,
                 "Are you certain that you\n" +
                 "want to delete all breakpoints?",
                 "Delete All Functions",
                 JOptionPane.YES_NO_OPTION,
                 JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            STAXMonitorTableSorter model = 
                (STAXMonitorTableSorter)fBreakpointTable.getModel();

            int rowCount = fBreakpointsTableModel.getRowCount();

            for(int i = 0; i < rowCount; i++)
            {
                fBreakpointsTableModel.removeRow(0);
            }

            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }
        else if (e.getSource() == fBreakpointDeleteButton)
        {
            int deleteRow = fBreakpointTable.getSelectedRow();

            if (deleteRow > -1)
            {
                fBreakpointsTableModel.removeRow(deleteRow);

                fCurrentJobParmsNotSaved = true;
                fStartNewJobFileSave.setEnabled(true);
                fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                            fCurrentJobParmsFile + " *");

                STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
                STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);
            }
        }
        else if (e.getSource() == fJobShowMonitorMenuItem)
        {
            synchronized(fMonitorTable)
            {
                int rowIndex = fActiveJobsTable.getSelectedRow();

                if (rowIndex != -1)
                {
                    String jobNumber = ((Integer)fActiveJobsTable.getValueAt(
                        rowIndex, 0)).toString();
                    
                    if (fMonitorTable.containsKey(jobNumber))
                    {
                        STAXMonitorFrame monitorFrame =
                            (STAXMonitorFrame)fMonitorTable.get(jobNumber);
                        monitorFrame.setState(Frame.NORMAL);
                        monitorFrame.toFront();
                        monitorFrame.requestFocus();
                    }
                }
                else
                {
                    STAXMonitorUtil.showErrorDialog(
                        this, "System busy.  Try again shortly.",
                        "System Busy");
                }

                fJobPopupMenu.setVisible(false);
            }
        }
        else if (e.getSource() == fJobStartMonitorMenuItem)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                int rowIndex = fActiveJobsTable.getSelectedRow();
                
                if (rowIndex != -1)
                {
                    String jobNumber = ((Integer)fActiveJobsTable.getValueAt(
                        rowIndex, 0)).toString();
                    monitorExistingJob(jobNumber, rowIndex);
                }
                else
                {
                    STAXMonitorUtil.showErrorDialog(
                        this, "System busy.  Try again shortly.",
                        "System Busy");
                }
            }
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fJobShowJobLogMenuItem ||
                 e.getSource() == fDisplaySelectedJobLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                int rowIndex = fActiveJobsTable.getSelectedRow();
                
                if (rowIndex != -1)
                {
                    String jobNumber = ((Integer)fActiveJobsTable.getValueAt(
                        rowIndex, 0)).toString();
                    String[] showLogParms = new String[10];
                    showLogParms[0] = "-machine";
                    showLogParms[1] = fStaxMachineName;
                    showLogParms[2] = "-machineNickname";
                    showLogParms[3] = fStaxMachineNickname;
                    showLogParms[4] = "-name";
                    showLogParms[5] = fStaxServiceName.toUpperCase() +
                        "_Job_" + jobNumber;
                    showLogParms[6] = "-fontName";
                    showLogParms[7] = fLogViewerFontName;
                    showLogParms[8] = "-saveAsDirectory";
                    showLogParms[9] = fSaveAsDirectory;

                    STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                        this, fHandle, showLogParms);
                }
                else
                {
                    STAXMonitorUtil.showErrorDialog(
                        this, "System busy.  Try again shortly.",
                        "System Busy");
                }
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            getContentPane().invalidate();
            getContentPane().repaint();
            fJobPopupMenu.setVisible(false);
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fDisplayJobLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                String jobNumber = JOptionPane.showInputDialog(this,
                    "Enter the Job ID", "Display Job Log",
                    JOptionPane.QUESTION_MESSAGE);

                if (jobNumber == null) return;

                String[] showLogParms = new String[10];
                showLogParms[0] = "-machine";
                showLogParms[1] = fStaxMachineName;
                showLogParms[2] = "-machineNickname";
                showLogParms[3] = fStaxMachineNickname;
                showLogParms[4] = "-name";
                showLogParms[5] = fStaxServiceName.toUpperCase() +
                    "_Job_" + jobNumber;
                showLogParms[6] = "-fontName";
                showLogParms[7] = fLogViewerFontName;
                showLogParms[8] = "-saveAsDirectory";
                showLogParms[9] = fSaveAsDirectory;

                STAXMonitorLogViewer logViewer =
                    new STAXMonitorLogViewer(this, fHandle, showLogParms);
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            getContentPane().invalidate();
            getContentPane().repaint();
            fJobPopupMenu.setVisible(false);
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fDisplayJobUserLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                String jobNumber = JOptionPane.showInputDialog(this,
                    "Enter the Job ID", "Display Job User Log",
                    JOptionPane.QUESTION_MESSAGE);

                if (jobNumber == null) return;

                String[] showLogParms = new String[10];
                showLogParms[0] = "-machine";
                showLogParms[1] = fStaxMachineName;
                showLogParms[2] = "-machineNickname";
                showLogParms[3] = fStaxMachineNickname;
                showLogParms[4] = "-name";
                showLogParms[5] = fStaxServiceName.toUpperCase() +
                    "_Job_" + jobNumber + "_User";
                showLogParms[6] = "-fontName";
                showLogParms[7] = fLogViewerFontName;
                showLogParms[8] = "-saveAsDirectory";
                showLogParms[9] = fSaveAsDirectory;

                STAXMonitorLogViewer logViewer =
                    new STAXMonitorLogViewer(this, fHandle, showLogParms);
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            getContentPane().invalidate();
            getContentPane().repaint();
            fJobPopupMenu.setVisible(false);
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fJobShowJobUserLogMenuItem ||
                 e.getSource() == fDisplaySelectedJobUserLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                int rowIndex = fActiveJobsTable.getSelectedRow();

                if (rowIndex != -1)
                {
                    String jobNumber = ((Integer)fActiveJobsTable.getValueAt(
                        rowIndex, 0)).toString();
                    String[] showLogParms = new String[10];
                    showLogParms[0] = "-machine";
                    showLogParms[1] = fStaxMachineName;
                    showLogParms[2] = "-machineNickname";
                    showLogParms[3] = fStaxMachineNickname;
                    showLogParms[4] = "-name";
                    showLogParms[5] = fStaxServiceName.toUpperCase() +
                        "_Job_" + jobNumber + "_User";
                    showLogParms[6] = "-fontName";
                    showLogParms[7] = fLogViewerFontName;
                    showLogParms[8] = "-saveAsDirectory";
                    showLogParms[9] = fSaveAsDirectory;
                    
                    STAXMonitorLogViewer logViewer = new STAXMonitorLogViewer(
                        this, fHandle, showLogParms);
                }
                else
                {
                    STAXMonitorUtil.showErrorDialog(
                        this, "System busy.  Try again shortly.",
                        "System Busy");
                }
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fJobShowSTAXServiceLogMenuItem ||
                 e.getSource() == fDisplayServiceLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                String[] showLogParms = new String[10];
                showLogParms[0] = "-machine";
                showLogParms[1] = fStaxMachineName;
                showLogParms[2] = "-machineNickname";
                showLogParms[3] = fStaxMachineNickname;
                showLogParms[4] = "-name";
                showLogParms[5] = fStaxServiceName.toUpperCase() + "_Service";
                showLogParms[6] = "-fontName";
                showLogParms[7] = fLogViewerFontName;
                showLogParms[8] = "-saveAsDirectory";
                showLogParms[9] = fSaveAsDirectory;

                STAXMonitorLogViewer logViewer =
                    new STAXMonitorLogViewer(this, fHandle, showLogParms);
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fJobShowSTAXJVMLogMenuItem ||
                 e.getSource() == fDisplaySTAXJVMLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                // Display just the JVM log for the current JVM
                boolean displayAll = false;

                // Use Monospaced for the default font name instead of
                // fLogViewerFontName.

                STAFJVMLogViewer logViewer = new STAFJVMLogViewer(
                    this, fHandle, fStaxMachineName, fStaxServiceName,
                    displayAll, "Monospaced");
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fJobShowOtherJVMLogMenuItem ||
                 e.getSource() == fDisplayOtherJVMLog)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                // Ask for the Machine where the service whose JVMLog you
                // want to browse is located (defaults to local).
                // If just whitespace for the machine, you're prompted again
                // until you enter a machine name or press Cancel.

                Object machineValue = new String("");

                while (machineValue.toString().trim().length() == 0)
                {
                    machineValue = JOptionPane.showInputDialog(
                        this,
                        "Enter the machine where the service whose " +
                        "JVM Log\n you want to display is registered",
                        "Enter the Java Service machine",
                        JOptionPane.INFORMATION_MESSAGE, null, null,
                        // Set initialValue parameter to fStaxMachineName
                        fStaxMachineName);

                    if (machineValue == null)
                        return;  // Cancel button was selected
                }

                String machine = (String)machineValue;

                // Display a list of the Java services registered on the
                // specified machine
                String service = "";

                // Display just the JVM log for the current JVM
                boolean displayAll = false;

                STAFJVMLogViewer logViewer = new STAFJVMLogViewer(
                    this, fHandle, machine, service,
                    displayAll, fLogViewerFontName);
            }

            fJobPopupMenu.setVisible(false);
            fJobPopupMenu.updateUI();
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fSaveAsDirectoryBrowseButton)
        {
            DirectoryFilter filter = new DirectoryFilter();
            JFileChooser fileChooser = new JFileChooser(fSaveAsDirectory);
            fileChooser.setFileFilter(filter);
            fileChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            fileChooser.setAcceptAllFileFilterUsed(false);

            fileChooser.setDialogTitle("Select a Directory");
            int rc = fileChooser.showDialog(this, "Select this Directory");

            if (rc == JFileChooser.APPROVE_OPTION)
            {
                try
                {
                    fSaveAsDirectoryField.setText(
                        fileChooser.getSelectedFile().getCanonicalPath());
                }
                catch (IOException ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        else if (e.getSource() == fJobTerminateJobMenuItem)
        {
            fJobPopupMenu.setVisible(false);

            synchronized(fActiveJobsModelSorter)
            {
                int rowIndex = fActiveJobsTable.getSelectedRow();

                if (rowIndex != -1)
                {
                    String jobNumber = ((Integer)fActiveJobsTable.getValueAt(
                        rowIndex, 0)).toString();
                    
                    int confirmation = JOptionPane.showConfirmDialog(
                        this, "Are you certain that you want\n" +
                        "to terminate Job ID " + jobNumber + " ?",
                        "Confirm Job Termination",
                        JOptionPane.YES_NO_OPTION,
                        JOptionPane.QUESTION_MESSAGE);

                    if (!(confirmation == JOptionPane.YES_OPTION))
                    {
                        return;
                    }

                    String request = "TERMINATE JOB " + jobNumber;

                    STAFResult result = fHandle.submit2(
                        fStaxMachineName, fStaxServiceName, request);

                    if (result.rc != 0)
                    {
                        STAXMonitorUtil.showErrorDialog(
                            this, "An error was encountered while " +
                            "attempting to terminate Job ID " + jobNumber +
                            ".\n\nRC: " + result.rc +
                            "\nResult: " + result.result);
                    }
                }
                else
                {
                    STAXMonitorUtil.showErrorDialog(
                        this, "System busy.  Try again shortly.",
                        "System Busy");
                }
            }
            Dimension size = getSize();
            size.width = size.width + 1;
            setSize(size);
        }
        else if (e.getSource() == fPluginJarsAddButton)
        {
            fAddPluginJarsTextField.setText("");
            fAddPluginJarsTextField.requestFocus();
            fAddPluginJarsDialog.setLocationRelativeTo(this);
            fAddPluginJarsDialog.setVisible(true);
        }
        else if (e.getSource() == fAddPluginJarsAddButton)
        {
            String pluginJarName = fAddPluginJarsTextField.getText();
            File pluginJarFile = new File(pluginJarName);

            // Make sure plugin jar file exists

            if (!(pluginJarFile.exists()))
            {
                STAXMonitorUtil.showErrorDialog(
                    fAddPluginJarsDialog,
                    "Extension Jar File does not exist\n" +
                    "File: " + pluginJarName);
            }

            // Make sure plugin jar file not already present
            else if (fLocalExtJarFiles.contains(pluginJarName))
            {
                STAXMonitorUtil.showErrorDialog(
                    fAddPluginJarsDialog,
                    "Extension Jar File cannot be added more than once\n" +
                    "File: " + pluginJarName);
            }
            else
            {
                ((DefaultListModel)(fPluginJarsList.getModel())).
                    addElement(fAddPluginJarsTextField.getText());

                fUpdatedLocalExtJarFiles.addElement(pluginJarName);
                fAddPluginJarsDialog.setVisible(false);
            }
        }
        else if (e.getSource() == fAddPluginJarsCancelButton)
        {
            fAddPluginJarsDialog.setVisible(false);
        }
        else if (e.getSource() == fEditPluginJarsSaveButton)
        {
            int editIndex = fPluginJarsList.getSelectedIndex();

            String pluginJarName = fEditPluginJarsTextField.getText();
            File pluginJarFile = new File(pluginJarName);

            if (!(pluginJarFile.exists()))
            {
                STAXMonitorUtil.showErrorDialog(
                    fEditPluginJarsDialog,
                    "Extension Jar File does not exist\n" +
                    "File: " + pluginJarName);
            }
            else
            {
                ((DefaultListModel)(fPluginJarsList.getModel())).
                    setElementAt(fEditPluginJarsTextField.getText(),
                    editIndex);

                fUpdatedLocalExtJarFiles.setElementAt(pluginJarName, editIndex);
                fEditPluginJarsDialog.setVisible(false);
            }
        }
        else if (e.getSource() == fEditPluginJarsCancelButton)
        {
            fEditPluginJarsDialog.setVisible(false);
        }
        else if (e.getSource() == fPluginJarsDeleteAllButton)
        {
            int confirmation = JOptionPane.showConfirmDialog(this,
                 "Are you certain that you\n" +
                 "want to delete all plugin jar files?",
                 "Delete All Plugin Jar Files",
                 JOptionPane.YES_NO_OPTION,
                 JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            ((DefaultListModel)(fPluginJarsList.getModel())).
                removeAllElements();
            fUpdatedLocalExtJarFiles.removeAllElements();
        }
        else if (e.getSource() == fPluginJarsDeleteButton)
        {
            int deleteIndex = fPluginJarsList.getSelectedIndex();
            if (deleteIndex > -1)
            {
                ((DefaultListModel)(fPluginJarsList.getModel())).
                    removeElementAt(deleteIndex);
                fUpdatedLocalExtJarFiles.removeElementAt(deleteIndex);
            }
        }
        else if (e.getSource() == fAddPluginJarsBrowseButton)
        {
            STAXMonitorFileFilter filter = new STAXMonitorFileFilter();
            filter.addExtension("jar");
            JFileChooser jarFileChooser = new JFileChooser(fLastFileDirectory);
            jarFileChooser.setFileFilter(filter);
            jarFileChooser.setDialogTitle("Select a Plugin Jar File");
            int rc = jarFileChooser.showDialog(this,
                "Select this Plugin Jar File");

            if (rc == JFileChooser.APPROVE_OPTION)
            {
                try
                {
                    fAddPluginJarsTextField.setText(
                        jarFileChooser.getSelectedFile().getCanonicalPath());
                    fLastFileDirectory = jarFileChooser.getCurrentDirectory();

                }
                catch (IOException ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        else if (e.getSource() == fScriptFilesAddButton)
        {
            fAddScriptFilesTextField.setText("");
            fAddScriptFilesTextField.requestFocus();
            fAddScriptFilesDialog.setLocationRelativeTo(this);
            fAddScriptFilesDialog.setVisible(true);
        }
        else if (e.getSource() == fAddScriptFilesAddButton)
        {
            if (fAddScriptFilesTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                    "You must enter a value for the Script File name",
                    "Error adding script file", JOptionPane.ERROR_MESSAGE);
                return;
            }

            ((DefaultListModel)(fScriptFilesList.getModel())).
               addElement(fAddScriptFilesTextField.getText());
            fScriptFilesVector.addElement(fAddScriptFilesTextField.getText());
            fAddScriptFilesDialog.setVisible(false);
        }
        else if (e.getSource() == fAddScriptFilesCancelButton)
        {
            fAddScriptFilesDialog.setVisible(false);
        }
        else if (e.getSource() == fEditScriptFilesSaveButton)
        {
            if (fEditScriptFilesTextField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(fStartNewJobDialog,
                    "You must enter a value for the Script File name",
                    "Error saving script file", JOptionPane.ERROR_MESSAGE);
                return;
            }

            int editIndex = fScriptFilesList.getSelectedIndex();
            ((DefaultListModel)(fScriptFilesList.getModel())).
                setElementAt(fEditScriptFilesTextField.getText(), editIndex);
            fEditScriptFilesDialog.setVisible(false);

        }
        else if (e.getSource() == fEditScriptFilesCancelButton)
        {
            fEditScriptFilesDialog.setVisible(false);
        }
        else if (e.getSource() == fScriptFilesDeleteAllButton)
        {
            int confirmation = JOptionPane.showConfirmDialog(this,
                 "Are you certain that you\n" +
                 "want to delete all script files?",
                 "Delete All Script Files",
                 JOptionPane.YES_NO_OPTION,
                 JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            ((DefaultListModel)(fScriptFilesList.getModel())).
                removeAllElements();
            fScriptFilesVector.removeAllElements();
        }
        else if (e.getSource() == fScriptFilesDeleteButton)
        {
            int deleteIndex = fScriptFilesList.getSelectedIndex();
            if (deleteIndex > -1)
            {
                ((DefaultListModel)(fScriptFilesList.getModel())).
                    removeElementAt(deleteIndex);
                fScriptFilesVector.removeElementAt(deleteIndex);
            }
        }
        else if (e.getSource() == fAddScriptFilesBrowseButton)
        {
            JFileChooser scriptFilesChooser = new
                JFileChooser(fLastFileDirectory);
            scriptFilesChooser.setDialogTitle("Select a Script File");
            int rc = scriptFilesChooser.showDialog(this,
                "Select this Script File");

            if (rc == JFileChooser.APPROVE_OPTION)
            {
                try
                {
                    fAddScriptFilesTextField.setText(
                        scriptFilesChooser.getSelectedFile().
                        getCanonicalPath());
                    fLastFileDirectory =
                        scriptFilesChooser.getCurrentDirectory();
                }
                catch (IOException ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        else if (e.getSource() == fWizardSaveButton)
        {
            String function = (String)
                fWizardFunctionsList.getSelectedValue();

            if (function.indexOf("(default)") > -1)
            {
                function = function.substring(0, function.indexOf(" "));
            }

            fWizardSavedFunctionName = function;
            fWizardSavedFileName = fWizardFileName;
            fWizardSavedFileMachineName = fWizardFileMachineName;

            String functionType = (String)
                fWizardFunctionTypeMap.get(function);

            JTable table = (JTable)
                fWizardFunctionArgTableMap.get(function);

            if (!verifyRequiredArguments(table))
            {
                STAXMonitorUtil.showErrorDialog(
                    fWizardDialog, "A value must be specified " +
                    "for all required arguments");
                return;
            }

            int confirmation = JOptionPane.showConfirmDialog(fWizardDialog,
                 "Are you certain that you want\n" +
                 "to save the job wizard options?\n" +
                 "This will overwrite existing\n" +
                 "options in the STAX Monitor.",
                 "Save Job Wizard Options",
                 JOptionPane.YES_NO_OPTION,
                 JOptionPane.QUESTION_MESSAGE);

            if (!(confirmation == JOptionPane.YES_OPTION))
            {
                return;
            }

            int rows = table.getRowCount();

            fWizardDialog.dispose();

            fFunction = function;
            fFunctionField.setText(function);
            fOtherFunctionRB.setSelected(true);

            fWizardSavedFunctionArgList.removeAllElements();

            if (functionType.equalsIgnoreCase("None"))
            {
                fArgs = "";
                fArguments.setText("");
            }
            else if (functionType.equalsIgnoreCase("Undefined"))
            {
                fWizardSavedFunctionArgList.removeAllElements();

                String arguments = "";

                String staxArgValue =
                    ((ArgumentValue)table.getValueAt(0, 3)).getString();
                
                if (!staxArgValue.equals(""))
                {
                    fWizardSavedFunctionArgList.add(staxArgValue);
                    arguments += staxArgValue;
                }

                fArgs = arguments;
                fArguments.setText(arguments);
            }
            else if (functionType.equalsIgnoreCase("Single"))
            {
                fWizardSavedFunctionArgList.removeAllElements();

                String arguments = (String)table.getValueAt(0, 3);
                fWizardSavedFunctionArgList.add(arguments);
                
                fArgs = arguments;
                fArguments.setText(arguments);
            }
            else if (functionType.equalsIgnoreCase("List"))
            {
                fWizardSavedFunctionArgList.removeAllElements();

                StringBuffer arguments = new StringBuffer("[\n  ");

                Vector argDefaults = (Vector)
                fWizardFunctionArgDefaultsMap.get(function);

                // Determine the last item that will be specified in this list.
                // All arguments prior to the last item must be included.

                int lastindex = -1;

                for (int i = 0; i < rows; i++)
                {
                    String required =
                        ((ArgumentRequired)table.getValueAt(i, 2)).getString();

                    String value =
                        ((ArgumentValue)table.getValueAt(i, 3)).getString();

                    fWizardSavedFunctionArgList.add(value);

                    if ( (required.equals("Yes")) ||
                         (required.equals("No") &&
                          !(((String)argDefaults.get(i)).equals(value))) ||
                         (required.equals("Other") && !value.equals("")) )
                    {
                        lastindex = i;
                    }
                }

                for (int i = 0; i <= lastindex; i++)
                {
                    String value =
                        ((ArgumentValue)table.getValueAt(i, 3)).getString();

                    if (i > 0)
                        arguments.append(",\n  ");

                    arguments.append(value);
                }

                arguments.append("\n]");

                fArgs = arguments.toString();
                fArguments.setText(fArgs);
            }
            else if (functionType.equalsIgnoreCase("Map"))
            {
                fWizardSavedFunctionArgList.removeAllElements();

                StringBuffer arguments = new StringBuffer("{\n  ");
                int numArgs = 0;

                Vector argDefaults = (Vector)
                    fWizardFunctionArgDefaultsMap.get(function);

                for (int i = 0; i < rows; i++)
                {
                    String required =
                        ((ArgumentRequired)table.getValueAt(i, 2)).getString();

                    String value =
                        ((ArgumentValue)table.getValueAt(i, 3)).getString();

                    fWizardSavedFunctionArgList.add(value);

                    if ( ( required.equals("Yes")) ||
                         ( required.equals("No") &&
                           !((String)argDefaults.get(i)).equals(value) ) ||
                         ( required.equals("Other") && !value.equals("") ) )
                    {
                        if (numArgs > 0) arguments.append(",\n  ");

                        if (!required.equals("Other"))
                        {
                            arguments.append("'");
                            arguments.append(table.getValueAt(i, 0));
                            arguments.append("': ");
                        }

                        arguments.append(value);

                        numArgs++;
                    }
                }

                arguments.append("\n}");

                fArgs = arguments.toString();
                fArguments.setText(fArgs);
            }

            if (!fArguments.getText().equals(""))
            {
                fArguments.setEnabled(false);
                fArgumentsEnabled = false;
            }
            else
            {
                fArguments.setEnabled(true);
                fArgumentsEnabled = true;
            }

            return;
        }
        else if (e.getSource() == fWizardPreviewXMLButton)
        {
            previewWizardXML();
        }
        else if (e.getSource() == fWizardCancelButton)
        {
            fWizardDialog.dispose();
        }
        else if (e.getSource() == fWizardDetailsButton)
        {
            displayJobWizardDetails();
        }
        else if (e.getSource() == fClearArguments)
        {
            fArguments.setEnabled(true);
            fArguments.setText("");
            fWizardSavedFunctionArgList = new Vector();
            fArgumentsEnabled = true;
        }
        else
        {
            for (int i=0; i< fRecentFiles.size(); i++)
            {
                if (e.getSource() == fRecentFileMenuItems[i])
                {
                    String fileName = fRecentFileMenuItems[i].getText();
                    loadJobParms(fileName);
                    fCurrentJobParmsNotSaved = false;
                    fCurrentJobParmsFile = fileName;
                    fStartNewJobFileSave.setEnabled(false);
                    fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                        fileName);
                    updateRecentFiles(fileName);
                }
            }

            for (int i=0; i< fRecentLogs.size(); i++)
            {
                if (e.getSource() == fRecentLogsMenuItems[i])
                {
                    String logName = fRecentLogsMenuItems[i].getText();

                    String[] showLogParms = new String[10];
                    showLogParms[0] = "-machine";
                    showLogParms[1] = fStaxMachineName;
                    showLogParms[2] = "-machineNickname";
                    showLogParms[3] = fStaxMachineNickname;
                    showLogParms[4] = "-name";
                    showLogParms[5] = logName;
                    showLogParms[6] = "-fontName";
                    showLogParms[7] = fLogViewerFontName;
                    showLogParms[8] = "-saveAsDirectory";
                    showLogParms[9] = fSaveAsDirectory;

                    STAXMonitorLogViewer logViewer =
                        new STAXMonitorLogViewer(this, fHandle, showLogParms);
                }
            }
       }
    }

    public void displayJobWizard()
    {
        StringBuffer request = new StringBuffer();

        if (fMachineLocalRB.isSelected())
        {
            fWizardFileMachineName = "";
            fWizardFileName = fLocalXmlFileNameField.getText();
            request.append("EXECUTE FILE ").append(
                STAFUtil.wrapData(fWizardFileName));
        }
        else
        {
            fWizardFileMachineName = fOtherXmlFileMachineField.getText();
            fWizardFileName = fOtherXmlFileNameField.getText();
            request.append("EXECUTE FILE ").append(
                STAFUtil.wrapData(fWizardFileName));
            request.append(" MACHINE ").append(
                STAFUtil.wrapData(fWizardFileMachineName));
        }

        request.append(" TEST RETURNDETAILS");

        // Submit the STAX EXECUTE request and display a "Please Wait" dialog
        // while the request is running

        STAXMonitorExecuteResult executeResult = submitExecuteRequest(
            request.toString());

        if (executeResult.getRC() != STAFResult.Ok)
        {
            STAXMonitorUtil.showErrorDialog(
                fStartNewJobDialog,
                executeResult.getResult().toString(),
                new Font("Courier", Font.PLAIN, 12));

            return;
        }

        // A successful STAX EXECUTE TEST RETURNDETAILS request returns
        // a STAFMarshallingContext in the marshalled result that contains
        // detailed information about the functions in the xml file

        STAFMarshallingContext mc =
            (STAFMarshallingContext)executeResult.getResult();
        
        Map jobDetailsMap = (Map)mc.getRootObject();
        
        TreeSet functions = new TreeSet();
        fWizardFunctionMap = new HashMap();
        fWizardFunctionTypeMap = new HashMap();
        fWizardFunctionPrologMap = new HashMap();
        fWizardFunctionEpilogMap = new HashMap();
        fWizardFunctionArgTableMap = new HashMap();
        fWizardFunctionArgDefaultsMap = new HashMap();
        String kNoArgsDefined = "FUNCTION_DEFINES_NO_ARGS";
        String kNoArgsDefinedText = "Undefined";
        String kNoArgsAllowed = "FUNCTION_ALLOWS_NO_ARGS";
        String kNoArgsAllowedText = "None";
        String kSingleArg = "FUNCTION_DEFINES_ONE_ARG";
        String kSingleArgText = "Single";
        String kListArgs = "FUNCTION_DEFINES_LIST_ARGS";
        String kListArgsText = "List";
        String kMapArgs = "FUNCTION_DEFINES_MAP_ARGS";
        String kMapArgsText = "Map";
        
        String defaultCall = (String)jobDetailsMap.get("defaultCall");

        // Iterate through the list of functions in the job

        java.util.List functionList =
            (java.util.List)jobDetailsMap.get("functionList");
        Iterator functionIter = functionList.iterator();
            
        while (functionIter.hasNext())
        {
            Map functionMap = (Map)functionIter.next();
            String functionName = (String)functionMap.get("functionName");

            if (functionName.equals(defaultCall))
            {
                functions.add(functionName + " (default)");
            }
            else
            {
                functions.add(functionName);
            }

            String functionProlog = (String)functionMap.get("prolog");

            if (functionProlog != null && !functionProlog.equals(""))
            {
                /* No changes */
            }
            else
            {
                functionProlog = "N/A";
            }

            String functionEpilog = (String)functionMap.get("epilog");

            String argDefinition = (String)functionMap.get("argDefinition");
            if (argDefinition.equals(kNoArgsDefined))
                argDefinition = kNoArgsDefinedText;
            else if (argDefinition.equals(kNoArgsAllowed))
                argDefinition = kNoArgsAllowedText;
            else if (argDefinition.equals(kSingleArg))
                argDefinition = kSingleArgText;
            else if (argDefinition.equals(kListArgs))
                argDefinition = kListArgsText;
            else if (argDefinition.equals(kMapArgs))
                argDefinition = kMapArgsText;

            fWizardFunctionPrologMap.put(functionName, functionProlog);
            fWizardFunctionEpilogMap.put(functionName, functionEpilog);
            fWizardFunctionTypeMap.put(functionName, argDefinition);

            // Iterate through the function argument list

            Vector functionArgs = new Vector();
            Vector functionArgDefaults = new Vector();
            java.util.List argList = (java.util.List)functionMap.get("argList");
            Iterator argIter = argList.iterator();

            while (argIter.hasNext())
            {
                Map argMap = (Map)argIter.next();
                Vector argInfo = new Vector();

                String argName = (String)argMap.get("argName");
                argInfo.add(argName);

                String argDescription = (String)argMap.get("description");

                if (argDescription != null && !argDescription.equals(""))
                {
                    /* No changes */
                }
                else
                {
                    argDescription = "N/A";
                }

                argInfo.add(argDescription);
                
                String argType = (String)argMap.get("type");

                if (argType.equals("ARG_REQUIRED"))
                    argType = "Yes";
                else if (argType.equals("ARG_OPTIONAL"))
                    argType = "No";
                else
                    argType = "Other";
                
                argInfo.add(argType);

                String argDefaultValue = (String)argMap.get("defaultValue");

                if (argDefaultValue != null && !argDefaultValue.equals("N/A"))
                {
                    /* No changes */
                }
                else
                {
                    argDefaultValue = "";
                }
                
                argInfo.add(argDefaultValue);

                functionArgDefaults.add(argDefaultValue);

                functionArgs.add(argInfo);
            }

            if (argDefinition.equals(kNoArgsDefinedText))
            {
                Vector argInfo = new Vector();
                argInfo.add("STAXArg");
                argInfo.add("Function Argument(s)");
                argInfo.add("No");
                argInfo.add("");
                functionArgs.add(argInfo);
                functionArgDefaults.add("");
            }

            fWizardFunctionMap.put(functionName, functionArgs);
            fWizardFunctionArgDefaultsMap.put(functionName,
                functionArgDefaults);
        }

        fWizardDialog = new JDialog(this, "STAX Job Wizard - " +
            fWizardFileName, true);
        JPanel wizardPanel = new JPanel();
        wizardPanel.setLayout(new
            BoxLayout(wizardPanel, BoxLayout.Y_AXIS));

        JTabbedPane wizardTabbedPane = new JTabbedPane();

        JPanel outerFunctionsPanel = new JPanel();
        outerFunctionsPanel.setLayout(new BorderLayout());

        JPanel functionsPanel = new JPanel();
        functionsPanel.setBorder(new TitledBorder("Functions"));
        functionsPanel.setLayout(new BorderLayout());

        JPanel functionListandPrologPanel = new JPanel();
        functionListandPrologPanel.setLayout(new BorderLayout());

        fWizardFunctionProlog = new JEditorPane();

        fWizardFunctionProlog.setEditable(false);

        functionsPanel.add(BorderLayout.WEST, Box.createHorizontalStrut(10));
        fWizardFunctionsList = new JList(functions.toArray());
        fWizardFunctionsList.setBackground(Color.white);
        fWizardFunctionsList.setFont(new Font("Dialog", Font.BOLD, 12));
        JScrollPane functionsScrollPane = new
            JScrollPane(fWizardFunctionsList);
        functionsScrollPane.setPreferredSize(new Dimension(225, 100));
        functionsPanel.add(BorderLayout.CENTER, functionsScrollPane);
        functionsPanel.add(BorderLayout.EAST, Box.createHorizontalStrut(10));
        functionListandPrologPanel.add(BorderLayout.WEST, functionsPanel);

        prologPanel = new JPanel();
        functionDescriptionBorder.setTitle("Function Description");
        prologPanel.setBorder(functionDescriptionBorder);
        prologPanel.setLayout(new BorderLayout());
        prologPanel.add(BorderLayout.CENTER, new
            JScrollPane(fWizardFunctionProlog));
        fWizardFunctionProlog.setContentType("text/html");

        fWizardDetailsButton = new JButton("Details...");
        fWizardDetailsButton.addActionListener(this);
        JPanel detailsButtonPanel = new JPanel();
        detailsButtonPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));
        detailsButtonPanel.add(fWizardDetailsButton);
        prologPanel.add(BorderLayout.SOUTH, detailsButtonPanel);

        fWizardFunctionArgsPanel = new JPanel();
        functionArgumentBorder.setTitle("Function Arguments");
        fWizardFunctionArgsPanel.setBorder(functionArgumentBorder);
        fWizardFunctionArgsPanel.setLayout(new BorderLayout());

        functionListandPrologPanel.add(BorderLayout.CENTER, prologPanel);

        JPanel wizardFunctionArgsTypeLabelPanel = new JPanel();
        wizardFunctionArgsTypeLabelPanel.setLayout(new
            BoxLayout(wizardFunctionArgsTypeLabelPanel, BoxLayout.Y_AXIS));

        fWizardNoArgsAllowedLabel = new JLabel("None");
        fWizardNoArgsAllowedLabel.setForeground(Color.black);
        wizardFunctionArgsTypeLabelPanel.add(fWizardNoArgsAllowedLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createVerticalStrut(10));

        fWizardSingleArgLabel = new JLabel("Single");
        fWizardSingleArgLabel.setForeground(Color.black);
        wizardFunctionArgsTypeLabelPanel.add(fWizardSingleArgLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createVerticalStrut(10));

        fWizardListArgsLabel = new JLabel("List");
        fWizardListArgsLabel.setForeground(Color.black);
        wizardFunctionArgsTypeLabelPanel.add(fWizardListArgsLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createVerticalStrut(10));

        fWizardMapArgsLabel = new JLabel("Map");
        fWizardMapArgsLabel.setForeground(Color.black);
        wizardFunctionArgsTypeLabelPanel.add(fWizardMapArgsLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createVerticalStrut(10));

        fWizardNoArgsDefinedLabel = new JLabel("Undefined");
        fWizardNoArgsDefinedLabel.setForeground(Color.black);
        wizardFunctionArgsTypeLabelPanel.add(fWizardNoArgsDefinedLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createVerticalStrut(10));

        fWizardFunctionArgsPanel.add(BorderLayout.NORTH,
            wizardFunctionArgsTypeLabelPanel);

        // Create all of the function argument tables

        Iterator iter = functions.iterator();

        while (iter.hasNext())
        {
            String function = (String)iter.next();

            if (function.indexOf("(default)") > -1)
            {
                function = function.substring(0, function.indexOf(" "));
            }

            Vector functionVector =
                (Vector)fWizardFunctionMap.get(function);

            int numRows = functionVector.size();
            Object[][] fObjectData = new Object[numRows][5];

            for (int i = 0; i < numRows; i++)
            {
                Vector row = (Vector)functionVector.elementAt(i);

                fObjectData[i][0] = new
                    ArgumentName((String)row.elementAt(0));

                fObjectData[i][1] = new
                    ArgumentDescription((String)row.elementAt(1));

                fObjectData[i][2] = new
                    ArgumentRequired((String)row.elementAt(2));

                fObjectData[i][3] = new
                    ArgumentValue((String)row.elementAt(3));
            }

            WizardTableModel fWizardFunctionTableModel =
                new WizardTableModel(fObjectData,
                fWizardFunctionTableColumnNames);

            JTable fWizardFunctionTable = new
                JTable(fWizardFunctionTableModel)
            {
                public TableCellRenderer getCellRenderer(int row, int column)
                {
                    Object cellObj = getModel().getValueAt(row, column);
                    String cellContent = (cellObj == null) ? "" :
                        String.valueOf(cellObj);

                    if ((cellObj instanceof ArgumentName) ||
                        (cellObj instanceof ArgumentDescription) ||
                        (cellObj instanceof ArgumentRequired) ||
                        (cellObj instanceof ArgumentValue))
                    {
                        return new STAXWizardFunctionTableCellRenderer();
                    }
                    else
                    {
                        return new DefaultTableCellRenderer();
                    }
                }

                public TableCellEditor getCellEditor(int row, int column)
                {
                    Object cellObj = getModel().getValueAt(row, column);
                    String cellContent = (cellObj == null) ? "" :
                        String.valueOf(cellObj);

                    if ((cellObj instanceof ArgumentName) ||
                        (cellObj instanceof ArgumentDescription) ||
                        (cellObj instanceof ArgumentRequired) ||
                        (cellObj instanceof ArgumentValue))
                    {
                        return new STAXWizardFunctionTableCellEditor(new
                            JCheckBox());
                    }
                    else
                    {
                        return new DefaultCellEditor(new JTextField(cellContent));
                    }
                }
            };

            updateWizardFunctionTableRenderers(fWizardFunctionTable);

            fWizardFunctionTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
            fWizardFunctionTable.setSelectionMode(
                ListSelectionModel.SINGLE_SELECTION);
            fWizardFunctionTable.setRowSelectionAllowed(true);
            fWizardFunctionTable.setBackground(Color.white);

            for (int i = 0 ; i < fWizardFunctionTable.getRowCount() ; i++)
            {
                fWizardFunctionTable.setRowHeight(i, 25);
            }

            fWizardFunctionTable.getColumnModel().getColumn(0).
                setPreferredWidth(130);

            fWizardFunctionTable.getColumnModel().getColumn(1).
                setPreferredWidth(480);

            fWizardFunctionTable.getColumnModel().getColumn(2).
                setPreferredWidth(60);

            fWizardFunctionTable.getColumnModel().getColumn(3).
                setPreferredWidth(300);

            fWizardFunctionArgTableMap.put(function, fWizardFunctionTable);
        }

        JPanel wizardButtonPanel = new JPanel();
        wizardButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        wizardButtonPanel.add(fWizardSaveButton);
        wizardButtonPanel.add(Box.createHorizontalStrut(20));
        wizardButtonPanel.add(fWizardPreviewXMLButton);
        wizardButtonPanel.add(Box.createHorizontalStrut(20));
        wizardButtonPanel.add(fWizardCancelButton);

        JPanel functionPanel = new JPanel();

        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints gbc1 = new GridBagConstraints();
        GridBagConstraints gbc2 = new GridBagConstraints();
        functionPanel.setLayout(gbl);

        gbc1.gridwidth = GridBagConstraints.RELATIVE;
        gbc1.gridheight = 1;
        gbc1.fill = GridBagConstraints.BOTH;
        gbc1.anchor = GridBagConstraints.NORTHWEST;
        gbc1.gridx = 0;
        gbc1.gridy = 0;
        gbc1.weightx = 1.0;
        gbc1.weighty = 1.0;
        functionPanel.add(functionListandPrologPanel, gbc1);

        gbc2.gridwidth = GridBagConstraints.RELATIVE;
        gbc2.gridheight = 1;
        gbc2.fill = GridBagConstraints.BOTH;
        gbc2.anchor = GridBagConstraints.NORTHWEST;
        gbc2.gridx = 0;
        gbc2.gridy = 1;
        gbc2.weightx = 1.0;
        gbc2.weighty = 1.0;
        functionPanel.add(fWizardFunctionArgsPanel, gbc2);

        JPanel instructionsPanel = new JPanel();
        instructionsPanel.setLayout(new
            BoxLayout(instructionsPanel, BoxLayout.Y_AXIS));
        JTextPane instructions = new JTextPane();
        instructions.setForeground(Color.black);
        instructions.setEditable(false);
        instructions.setBackground(UIManager.getColor("Panel.background"));
        instructions.setFont(new Font("Dialog", Font.BOLD, 13));
        instructions.setText("Select a function from the Functions list, " +
            "fill in the values for the Function arguments as needed, and " +
            "then click on Save to return to the STAX Job Parameters " +
            "dialog.  Note that after you save the Function arguments " +
            "you will not be able to edit them directly in the STAX Job " +
            "Parameters dialog; you will need to reopen the Job Wizard " +
            "in order to edit the parameters, or click on the Clear button " +
            "in the STAX Job Parameters dialog.");
        instructionsPanel.add(Box.createVerticalStrut(7));
        instructionsPanel.add(instructions);
        instructionsPanel.add(Box.createVerticalStrut(7));

        outerFunctionsPanel.add(BorderLayout.NORTH, instructionsPanel);
        outerFunctionsPanel.add(BorderLayout.CENTER, functionPanel);

        wizardTabbedPane.addTab("Functions", outerFunctionsPanel);

        wizardPanel.add(wizardTabbedPane);
        wizardPanel.add(wizardButtonPanel);

        wizardTabbedPane.setBackgroundAt(0, Color.lightGray);

        wizardTabbedPane.addChangeListener(this);

        fWizardDialog.setSize(new Dimension(1025, 700));
        fWizardDialog.getContentPane().
            add(wizardPanel);

        if (fDefaultFunctionRB.isSelected() && !defaultCall.equals(""))
        {
            fWizardFunctionsList.setSelectedValue(defaultCall +
                " (default)", true);
        }
        else if (fOtherFunctionRB.isSelected())
        {
            fWizardFunctionsList.setSelectedValue(fFunctionField.getText(),
                true);

            if (fWizardFunctionsList.getSelectedIndex() == -1)
            {
                fWizardFunctionsList.setSelectedValue(fFunctionField.getText()
                    + " (default)", true);

                if (fWizardFunctionsList.getSelectedIndex() == -1)
                {
                    fWizardFunctionsList.setSelectedIndex(0);
                }
            }
        }
        else
        {
            fWizardFunctionsList.setSelectedIndex(0);
        }

        String function = (String)fWizardFunctionsList.getSelectedValue();

        if (function.indexOf("(default)") > -1)
        {
            function = function.substring(0, function.indexOf(" "));
        }

        // Assign Wizard's Saved Function Argument List if using same 
        // xml file, machine, and function as was last saved via the Wizard.

        if (function.equals(fWizardSavedFunctionName) &&
            fWizardFileName.equals(fWizardSavedFileName) &&
            fWizardFileMachineName.equals(fWizardSavedFileMachineName))
        {
            String functionType = (String)
                fWizardFunctionTypeMap.get(function);

            JTable table = (JTable)
                fWizardFunctionArgTableMap.get(function);

            if (table.getRowCount() >= fWizardSavedFunctionArgList.size())
            {
                if (functionType.equals("Single") ||
                    functionType.equals("Undefined") ||
                    functionType.equals("List") ||
                    functionType.equals("Map"))
                {
                    for (int i = 0; i < fWizardSavedFunctionArgList.size(); i++)
                    {
                        table.setValueAt(new ArgumentValue(
                            (String)fWizardSavedFunctionArgList.elementAt(i)),
                            i, 3);
                    }
                }
            }
        }

        updateWizardFunctionTable();

        fWizardFunctionsList.addListSelectionListener(this);

        fWizardDialog.setVisible(true);
    }

    public void updateWizardFunctionTable()
    {
        Object[][] fObjectData;

        String function = (String)fWizardFunctionsList.getSelectedValue();

        if (function.indexOf("(default)") > -1)
        {
            function = function.substring(0, function.indexOf(" "));
        }

        functionDescriptionBorder.setTitle("Description for function " +
            function);

        functionArgumentBorder.setTitle("Arguments for function " +
            function);

        Vector functionVector =
            (Vector)fWizardFunctionMap.get(function);

        fWizardFunctionArgsPanel.removeAll();

        updateWizardFunctionType((String)
            fWizardFunctionTypeMap.get(function));

        JPanel wizardFunctionArgsDetailsPanel = new JPanel();
        wizardFunctionArgsDetailsPanel.setLayout(new
            BoxLayout(wizardFunctionArgsDetailsPanel, BoxLayout.Y_AXIS));

        JPanel wizardFunctionArgsTypeLabelPanel = new JPanel();
        wizardFunctionArgsTypeLabelPanel.setLayout(new
            BoxLayout(wizardFunctionArgsTypeLabelPanel, BoxLayout.X_AXIS));

        wizardFunctionArgsTypeLabelPanel.add(fWizardNoArgsAllowedLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createHorizontalStrut(10));

        wizardFunctionArgsTypeLabelPanel.add(fWizardSingleArgLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createHorizontalStrut(10));

        wizardFunctionArgsTypeLabelPanel.add(fWizardListArgsLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createHorizontalStrut(10));

        wizardFunctionArgsTypeLabelPanel.add(fWizardMapArgsLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createHorizontalStrut(10));

        wizardFunctionArgsTypeLabelPanel.add(fWizardNoArgsDefinedLabel);
        wizardFunctionArgsTypeLabelPanel.add(Box.createHorizontalStrut(10));

        wizardFunctionArgsTypeLabelPanel.add(Box.createVerticalStrut(10));

        wizardFunctionArgsDetailsPanel.add(wizardFunctionArgsTypeLabelPanel);
        wizardFunctionArgsDetailsPanel.add(Box.createVerticalStrut(10));

        fWizardFunctionArgsPanel.add(BorderLayout.NORTH,
            wizardFunctionArgsDetailsPanel);
        fWizardFunctionArgsPanel.add(BorderLayout.CENTER,
            new JScrollPane((JTable)fWizardFunctionArgTableMap.get(function)));

        fWizardFunctionProlog.setText((String)
            fWizardFunctionPrologMap.get(function));
        fWizardFunctionProlog.setCaretPosition(0);

        fWizardFunctionArgsPanel.invalidate();
        fWizardFunctionArgsPanel.validate();
        fWizardFunctionArgsPanel.repaint();
        prologPanel.repaint();
    }

    public void displayJobWizardDetails()
    {
        String function = (String)fWizardFunctionsList.getSelectedValue();

        if (function.indexOf("(default)") > -1)
        {
            function = function.substring(0, function.indexOf(" "));
        }

        final JDialog fWizardDetailsDialog = new JDialog(fWizardDialog,
            function + " Details", false);
        JPanel wizardDetailsPanel = new JPanel();
        wizardDetailsPanel.setLayout(new BorderLayout());

        JEditorPane fWizardFunctionDetails = new JEditorPane();
        fWizardFunctionDetails.setEditable(false);
        fWizardFunctionDetails.setContentType("text/html");

        fWizardFunctionDetails.setText((String)
            fWizardFunctionPrologMap.get(function) +
            (String)fWizardFunctionEpilogMap.get(function));

        fWizardFunctionDetails.setCaretPosition(0);

        wizardDetailsPanel.add(BorderLayout.CENTER,
            new JScrollPane(fWizardFunctionDetails));

        JButton wizardDetailsCloseButton = new JButton("Close");

        wizardDetailsCloseButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                fWizardDetailsDialog.dispose();
            }
        });

        JPanel wizardDetailsButtonPanel = new JPanel();
        wizardDetailsButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        wizardDetailsButtonPanel.add(wizardDetailsCloseButton);

        wizardDetailsPanel.add(BorderLayout.SOUTH, wizardDetailsButtonPanel);

        fWizardDetailsDialog.setSize(925, 600);
        fWizardDetailsDialog.setLocationRelativeTo(fWizardDialog);
        fWizardDetailsDialog.getContentPane().add(wizardDetailsPanel);
        fWizardDetailsDialog.setVisible(true);
    }

    public void updateWizardFunctionType(String type)
    {
        if (type.equals(fWizardNoArgsAllowedLabel.getText()))
        {
            fWizardNoArgsAllowedLabel.setForeground(Color.blue);
        }
        else
        {
            fWizardNoArgsAllowedLabel.setForeground(Color.darkGray);
        }

        if (type.equals(fWizardNoArgsDefinedLabel.getText()))
        {
            fWizardNoArgsDefinedLabel.setForeground(Color.blue);
        }
        else
        {
            fWizardNoArgsDefinedLabel.setForeground(Color.darkGray);
        }

        if (type.equals(fWizardSingleArgLabel.getText()))
        {
            fWizardSingleArgLabel.setForeground(Color.blue);
        }
        else
        {
            fWizardSingleArgLabel.setForeground(Color.darkGray);
        }

        if (type.equals(fWizardListArgsLabel.getText()))
        {
            fWizardListArgsLabel.setForeground(Color.blue);
        }
        else
        {
            fWizardListArgsLabel.setForeground(Color.darkGray);
        }

        if (type.equals(fWizardMapArgsLabel.getText()))
        {
            fWizardMapArgsLabel.setForeground(Color.blue);
        }
        else
        {
            fWizardMapArgsLabel.setForeground(Color.darkGray);
        }
    }

    public void previewWizardXML()
    {
        final JDialog previewXMLDialog = new JDialog(fWizardDialog,
            " Preview XML", false);
        JPanel previewXMLPanel = new JPanel();
        previewXMLPanel.setLayout(new BorderLayout());

        JTextArea previewXMLTextArea = new JTextArea();
        previewXMLTextArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
        previewXMLTextArea.setEditable(false);

        String function = (String)fWizardFunctionsList.getSelectedValue();

        if (function.indexOf("(default)") > -1)
        {
            function = function.substring(0, function.indexOf(" "));
        }

        String functionType = (String)
                fWizardFunctionTypeMap.get(function);

        JTable table = (JTable)
            fWizardFunctionArgTableMap.get(function);

        int rows = table.getRowCount();

        if (!verifyRequiredArguments(table))
        {
            STAXMonitorUtil.showErrorDialog(
                fWizardDialog, "A value must be specified " +
                "for all required arguments");
            return;
        }

        if (function.indexOf("(default)") > -1)
        {
            function = function.substring(0, function.indexOf(" "));
        }

        StringBuffer args = new StringBuffer();

        if (functionType.equalsIgnoreCase("None"))
        {
            // Do nothing - args is already ""
        }
        else if (functionType.equalsIgnoreCase("Undefined"))
        {
            String staxArgValue =
                ((ArgumentValue)table.getValueAt(0, 3)).getString();

            if (!staxArgValue.equals(""))
            {
                args.append(staxArgValue);
            }
        }
        else if (functionType.equalsIgnoreCase("Single"))
        {
            args.append(table.getValueAt(0, 3));
        }
        else if (functionType.equalsIgnoreCase("List"))
        {
            args.append("[\n   ");

            Vector argDefaults = (Vector)
                fWizardFunctionArgDefaultsMap.get(function);

            // Determine the last item that will be specified in this list.
            // All arguments prior to the last item must be included.

            int lastindex = -1;

            for (int i = 0; i < rows; i++)
            {
                String required =
                    ((ArgumentRequired)table.getValueAt(i, 2)).getString();

                String value =
                    ((ArgumentValue)table.getValueAt(i, 3)).getString();
                
                if ( (required.equals("Yes")) ||
                     (required.equals("No") &&
                      !(((String)argDefaults.get(i)).equals(value))) ||
                     (required.equals("Other") && !value.equals("")) )
                {
                    lastindex = i;
                }
            }

            for (int i = 0; i <= lastindex; i++)
            {
                String value =
                    ((ArgumentValue)table.getValueAt(i, 3)).getString();

                if (i > 0)
                    args.append(",\n   ");

                args.append(value);
            }

            args.append("\n  ]");
        }
        else if (functionType.equalsIgnoreCase("Map"))
        {
            args.append("{\n   ");
            int numArgs = 0;

            Vector argDefaults = (Vector)
                fWizardFunctionArgDefaultsMap.get(function);

            for (int i = 0; i < rows; i++)
            {
                String required =
                    ((ArgumentRequired)table.getValueAt(i, 2)).getString();

                String value =
                    ((ArgumentValue)table.getValueAt(i, 3)).getString();

                if ( ( required.equals("Yes")) ||
                     ( required.equals("No") &&
                       !((String)argDefaults.get(i)).equals(value) ) ||
                     ( required.equals("Other") && !value.equals("") ) )
                {
                    if (numArgs > 0) args.append(",\n   ");

                    if (!required.equals("Other"))
                    {
                        args.append("'").append(table.getValueAt(i, 0)).
                             append("': ");
                    }

                    args.append(value);

                    numArgs++;
                }
            }

            args.append("\n  }");
        }

        String callText = "";

        if (args.equals(""))
        {
            callText = "<call function=\"'" + function + "'\">\n</call>";
        }
        else
        {
            callText = "<call function=\"'" +
                function + "'\">\n  " + args.toString() + "\n</call>";
        }

        previewXMLTextArea.setText(callText);

        previewXMLTextArea.setCaretPosition(0);

        previewXMLPanel.add(BorderLayout.CENTER,
            new JScrollPane(previewXMLTextArea));

        JButton previewXMLCloseButton = new JButton("Close");

        previewXMLCloseButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                previewXMLDialog.dispose();
            }
        });

        JPanel previewXMLButtonPanel = new JPanel();
        previewXMLButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        previewXMLButtonPanel.add(previewXMLCloseButton);

        previewXMLPanel.add(BorderLayout.SOUTH, previewXMLButtonPanel);

        previewXMLDialog.setSize(300, 200);
        previewXMLDialog.setLocationRelativeTo(fWizardDialog);
        previewXMLDialog.getContentPane().add(previewXMLPanel);
        previewXMLDialog.setVisible(true);
    }

    public boolean verifyRequiredArguments(JTable table)
    {
        int rows = table.getRowCount();

        for (int i = 0; i < rows; i++)
        {
            ArgumentRequired required =
                (ArgumentRequired)table.getValueAt(i, 2);

            if (required.getString().equals("Yes"))
            {
                ArgumentValue value =
                    (ArgumentValue)table.getValueAt(i, 3);

                if (value.getString().equals(""))
                {
                    return false;
                }
            }
        }

        return true;
    }

    public void updateWizardFunctionTableRenderers(JTable wizardFunctionTable)
    {
        wizardFunctionTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXWizardFunctionTableCellRenderer());

        wizardFunctionTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXWizardFunctionTableCellRenderer(
                new Font("Dialog", Font.BOLD, 12), Color.lightGray));

        wizardFunctionTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXWizardFunctionTableCellRenderer());

        wizardFunctionTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXWizardFunctionTableCellRenderer(
                new Font("Dialog", Font.BOLD, 12), Color.lightGray));

        wizardFunctionTable.getColumnModel().getColumn(2).setCellRenderer(
            new STAXWizardFunctionTableCellRenderer());

        wizardFunctionTable.getColumnModel().getColumn(2).
            setHeaderRenderer(new STAXWizardFunctionTableCellRenderer(
                new Font("Dialog", Font.BOLD, 12), Color.lightGray));

        wizardFunctionTable.getColumnModel().getColumn(3).setCellRenderer(
            new STAXWizardFunctionTableCellRenderer(
                new Font("Monospaced", Font.PLAIN, 12), Color.white));

        wizardFunctionTable.getColumnModel().getColumn(3).
            setHeaderRenderer(new STAXWizardFunctionTableCellRenderer(
                new Font("Dialog", Font.BOLD, 12), Color.lightGray));
    }

    public class WizardTableModel extends javax.swing.table.DefaultTableModel
    {
        public WizardTableModel()
        {
            super();
        }

        public WizardTableModel(java.lang.Object[][] data,
                                java.lang.Object[] columnNames)
        {
            super(data, columnNames);
        }

        public WizardTableModel(java.lang.Object[] columnNames, int numRows)
        {
            super(columnNames, numRows);
        }

        public WizardTableModel(int numRows, int numColumns)
        {
            super(numRows, numColumns);
        }

        public WizardTableModel(java.util.Vector columnNames, int numRows)
        {
            super(columnNames, numRows);
        }

        public WizardTableModel(java.util.Vector data,
                              java.util.Vector columnNames)
        {
            super(data, columnNames);
        }

        public boolean isCellEditable(int row, int column)
        {
            switch(column)
            {
                case 0  : return false;
                case 1  : return true;
                case 2  : return false;
                case 3  : return true;
                default : return false;
            }
        }
    }

    public class STAXWizardFunctionTableCellRenderer
             extends DefaultTableCellRenderer
    {
        public Hashtable rowHeights = new Hashtable();
        private boolean isHeader = true;
        private JTextField argumentNameField = new JTextField();
        private JTextField argumentDescriptionField = new JTextField();
        private JTextField argumentRequiredField = new JTextField();
        private JTextField argumentValueField = new JTextField();
        private JButton descriptionButton;
        private JButton editButton;
        private JButton resetButton;
        private JLabel headerLabel = new JLabel();

        public STAXWizardFunctionTableCellRenderer()
        {
            this(new Font("Dialog", Font.PLAIN, 12), Color.white);
            isHeader = false;
        }

        public STAXWizardFunctionTableCellRenderer(Font font, Color background)
        {
            headerLabel.setFont(font);
            headerLabel.setOpaque(true);
            headerLabel.setBackground(background);
            headerLabel.setForeground(Color.black);

            argumentNameField.setEditable(false);
            argumentNameField.setBackground(Color.white);

            argumentDescriptionField.setEditable(false);
            argumentDescriptionField.setBackground(Color.white);

            argumentRequiredField.setEditable(false);
            argumentRequiredField.setBackground(Color.white);

            argumentValueField.setEditable(false);
            argumentValueField.setBackground(Color.white);
        }

        public void clearRowHeights()
        {
            rowHeights.clear();
        }

        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row, int col)
        {
            Color rowBackground = lightGreen;

            if (value instanceof WizardArgumentInterface)
            {
                ArgumentRequired argReq =
                    (ArgumentRequired)table.getModel().getValueAt(row, 2);

                if (argReq.getString().equals("Yes"))
                {
                    ArgumentValue argValue =
                        (ArgumentValue)table.getModel().getValueAt(row, 3);

                    if (argValue.getString().equals(""))
                    {
                        rowBackground = lightRed;
                    }
                }
                else if (argReq.getString().equals("Other"))
                {
                    ArgumentValue argValue =
                        (ArgumentValue)table.getModel().getValueAt(row, 3);

                    if (argValue.getString().equals(""))
                    {
                        rowBackground = lightYellow;
                    }
                }

                ArgumentName argName =
                    (ArgumentName)table.getModel().getValueAt(row, 0);

                if (argName.getString().equals("STAXArg"))
                {
                    ArgumentValue argValue =
                        (ArgumentValue)table.getModel().getValueAt(row, 3);

                    if (argValue.getString().equals(""))
                    {
                        rowBackground = lightYellow;
                    }
                }
            }

            String cellContent = (value == null) ? "" :
                String.valueOf(value);

            if (value instanceof ArgumentName)
            {
                JPanel namePanel = new JPanel();
                argumentNameField.setText(((ArgumentName)value).getString());
                argumentNameField.setBorder(null);
                argumentNameField.setBackground(rowBackground);
                namePanel.setLayout(new BorderLayout());
                namePanel.setBackground(rowBackground);
                namePanel.add(BorderLayout.CENTER, argumentNameField);

                Dimension size = argumentNameField.getPreferredSize();
                size.height = 25;
                argumentNameField.setPreferredSize(size);

                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(rowBackground);
                panel.add(BorderLayout.NORTH, namePanel);

                return panel;
            }
            else if (value instanceof ArgumentDescription)
            {
                descriptionButton = new JButton("More...");
                descriptionButton.setPreferredSize(new Dimension(75, 27));

                JPanel descPanel = new JPanel();
                argumentDescriptionField.setText(
                    ((ArgumentDescription)value).getFirstLine());
                argumentDescriptionField.setBorder(null);
                argumentDescriptionField.setBackground(rowBackground);
                descPanel.setLayout(new BorderLayout());
                descPanel.setBackground(rowBackground);
                descPanel.add(BorderLayout.CENTER, argumentDescriptionField);

                JPanel buttonPanel = new JPanel();
                buttonPanel.setLayout(new BorderLayout());
                buttonPanel.setBackground(rowBackground);
                buttonPanel.add(BorderLayout.NORTH, descriptionButton);
                descPanel.add(BorderLayout.EAST, buttonPanel);

                Dimension size = argumentDescriptionField.getPreferredSize();
                size.height = 25;
                argumentDescriptionField.setPreferredSize(size);

                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(rowBackground);
                panel.add(BorderLayout.NORTH, descPanel);

                return panel;
            }
            else if (value instanceof ArgumentRequired)
            {
                JPanel requiredPanel = new JPanel();
                argumentRequiredField.setText(
                    ((ArgumentRequired)value).getString());
                argumentRequiredField.setBorder(null);
                argumentRequiredField.setBackground(rowBackground);
                requiredPanel.setLayout(new BorderLayout());
                requiredPanel.setBackground(rowBackground);
                requiredPanel.add(BorderLayout.CENTER, argumentRequiredField);

                Dimension size = argumentRequiredField.getPreferredSize();
                size.height = 25;
                argumentRequiredField.setPreferredSize(size);

                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(rowBackground);
                panel.add(BorderLayout.NORTH, requiredPanel);

                return panel;
            }
            else if (value instanceof ArgumentValue)
            {
                editButton = new JButton("Edit...");
                editButton.setPreferredSize(new Dimension(75, 27));
                resetButton = new JButton("Default");
                resetButton.setPreferredSize(new Dimension(75, 27));

                JPanel valuePanel = new JPanel();
                argumentValueField.setText(((ArgumentValue)value).getString());
                argumentValueField.setBorder(null);
                argumentValueField.setFont(
                    new Font("Monospaced", Font.PLAIN, 12));
                argumentValueField.setBackground(rowBackground);
                valuePanel.setLayout(new BorderLayout());
                valuePanel.setBackground(rowBackground);
                valuePanel.add(BorderLayout.CENTER, argumentValueField);

                JPanel buttonPanel = new JPanel();
                buttonPanel.setLayout(new BorderLayout());
                buttonPanel.setBackground(rowBackground);
                buttonPanel.add(BorderLayout.WEST, editButton); // was NORTH

                ArgumentRequired argReq =
                    (ArgumentRequired)table.getModel().getValueAt(row, 2);

                if (argReq.getString().equals("No"))
                {
                    buttonPanel.add(BorderLayout.EAST, resetButton);
                }

                valuePanel.add(BorderLayout.EAST, buttonPanel);

                Dimension size = argumentValueField.getPreferredSize();
                size.height = 25;
                argumentValueField.setPreferredSize(size);

                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(rowBackground);
                panel.add(BorderLayout.NORTH, valuePanel);

                return panel;
            }
            else
            {
                headerLabel.setBorder(BorderFactory.createRaisedBevelBorder());
                headerLabel.setText((String)value);
                return headerLabel;
            }
        }
    }

    public class STAXWizardFunctionTableCellEditor
        extends DefaultCellEditor
    {
        JButton button;
        boolean isPushed;
        boolean resetPushed;
        String textEditValue = "";
        JDialog fEditValueDialog;
        JTextArea fEditValueTextArea;
        JButton fEditValueSaveButton;
        JButton fEditValueCancelButton;
        Object cellObj;
        int inputRow;
        JTextField textfield;
        JDialog fMoreDescriptionDialog;
        JEditorPane fMoreDescriptionEditorPane;
        JButton fMoreDescriptionCloseButton;
        JButton resetButton;

        public STAXWizardFunctionTableCellEditor(JCheckBox checkBox)
        {
            super(checkBox);

            button = new JButton();
            button.setOpaque(true);
            button.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                {
                    if (e.getSource() == button)
                    {
                        isPushed = true;
                        fireEditingStopped();
                    }
                }
            });

            resetButton = new JButton();
            resetButton.setOpaque(true);
            resetButton.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                {
                    if (e.getSource() == resetButton)
                    {
                        resetPushed = true;
                        fireEditingStopped();
                    }
                }
            });

            fEditValueDialog = new JDialog(fWizardDialog,
                "Edit Argument Value", true);
            fEditValueDialog.setDefaultCloseOperation(
                WindowConstants.DO_NOTHING_ON_CLOSE);
            fEditValueDialog.setSize(new Dimension(400, 200));
            JPanel editValuePanel = new JPanel();
            editValuePanel.setLayout(new BorderLayout());
            fEditValueTextArea = new JTextArea(5, 15);
            fEditValueTextArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
            fEditValueTextArea.setBorder(new
                TitledBorder("Edit argument value here"));
            editValuePanel.add(BorderLayout.CENTER,
                new JScrollPane(fEditValueTextArea));

            fEditValueSaveButton = new JButton("Save");
            fEditValueCancelButton = new JButton("Cancel");

            JPanel editValueButtonPanel = new JPanel();
            editValueButtonPanel.setLayout(new
                FlowLayout(FlowLayout.CENTER, 0, 0));
            editValueButtonPanel.add(fEditValueSaveButton);
            editValueButtonPanel.add(Box.createHorizontalStrut(20));
            editValueButtonPanel.add(fEditValueCancelButton);

            editValuePanel.add(BorderLayout.SOUTH, editValueButtonPanel);

            fEditValueDialog.getContentPane().add(editValuePanel);

            fMoreDescriptionDialog = new JDialog(fWizardDialog,
                "Argument Description", true);
            fMoreDescriptionDialog.setDefaultCloseOperation(
                WindowConstants.DO_NOTHING_ON_CLOSE);
            fMoreDescriptionDialog.setSize(new Dimension(400, 200));
            JPanel moreDescriptionPanel = new JPanel();
            moreDescriptionPanel.setLayout(new BorderLayout());
            fMoreDescriptionEditorPane = new JEditorPane();
            fMoreDescriptionEditorPane.setContentType("text/html");
            fMoreDescriptionEditorPane.setEditable(false);
            fMoreDescriptionEditorPane.setBorder(new
                TitledBorder("Argument Description"));

            moreDescriptionPanel.add(BorderLayout.CENTER,
                new JScrollPane(fMoreDescriptionEditorPane));

            fMoreDescriptionCloseButton = new JButton("Close");

            JPanel moreDescriptionButtonPanel = new JPanel();
            moreDescriptionButtonPanel.setLayout(new
                FlowLayout(FlowLayout.CENTER, 0, 0));
            moreDescriptionButtonPanel.add(fMoreDescriptionCloseButton);

            moreDescriptionPanel.add(BorderLayout.SOUTH,
                moreDescriptionButtonPanel);

            fMoreDescriptionDialog.getContentPane().add(
                moreDescriptionPanel);
        }

        public Component getTableCellEditorComponent(JTable table,
            Object value, boolean isSelected, int row, int column)
        {
            String stringValue = (value == null) ? "" : String.valueOf(value);

            // need this so getCellEditorValue can access it
            textEditValue = stringValue;
            inputRow = row + 1;

            textfield = new JTextField(((WizardArgumentInterface)value).
                                       getString());

            cellObj = value;

            if (value instanceof ArgumentValue)
            {
                button.setText("Edit...");
                button.setPreferredSize(new Dimension(75, 27));

                JPanel textpanel = new JPanel();
                textpanel.setLayout(new BorderLayout());
                textfield.setBorder(null);
                textfield.setOpaque(true);
                textfield.setBackground(Color.white);
                textfield.setForeground(Color.black);
                textfield.setEditable(true);
                textfield.setFont(new Font("Monospaced", Font.PLAIN, 12));
                textpanel.add(BorderLayout.CENTER, textfield);

                JPanel buttonPanel = new JPanel();
                buttonPanel.setLayout(new BorderLayout());
                buttonPanel.add(BorderLayout.WEST, button);

                resetButton.setText("Default");

                ArgumentRequired argReq =
                    (ArgumentRequired)table.getModel().getValueAt(row, 2);

                if (argReq.getString().equals("No"))
                {
                    buttonPanel.add(BorderLayout.EAST, resetButton);
                }

                textpanel.add(BorderLayout.EAST, buttonPanel);

                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(Color.white);
                panel.add(BorderLayout.NORTH, textpanel);

                return panel;
            }
            else if (value instanceof ArgumentDescription)
            {
                Color rowBackground = lightGreen;

                if (value instanceof WizardArgumentInterface)
                {
                    ArgumentRequired argReq =
                        (ArgumentRequired)table.getModel().getValueAt(row, 2);

                    if (argReq.getString().equals("Yes"))
                    {
                        ArgumentValue argValue =
                            (ArgumentValue)table.getModel().getValueAt(row, 3);

                        if (argValue.getString().equals(""))
                        {
                            rowBackground = lightRed;
                        }
                    }
                    else if (argReq.getString().equals("Other"))
                    {
                        ArgumentValue argValue =
                            (ArgumentValue)table.getModel().getValueAt(row, 3);

                        if (argValue.getString().equals(""))
                        {
                            rowBackground = lightYellow;
                        }
                    }

                    ArgumentName argName =
                        (ArgumentName)table.getModel().getValueAt(row, 0);

                    if (argName.getString().equals("STAXArg"))
                    {
                        ArgumentValue argValue =
                            (ArgumentValue)table.getModel().getValueAt(row, 3);

                        if (argValue.getString().equals(""))
                        {
                            rowBackground = lightYellow;
                        }
                    }
                }

                button.setText("More...");
                button.setPreferredSize(new Dimension(75, 27));

                JPanel textpanel = new JPanel();
                textpanel.setLayout(new BorderLayout());
                textpanel.setBackground(rowBackground);
                textfield.setBorder(null);
                textfield.setOpaque(true);
                textfield.setBackground(rowBackground);
                textfield.setForeground(Color.black);
                textfield.setEditable(false);
                textpanel.add(BorderLayout.CENTER, textfield);
                textpanel.add(BorderLayout.EAST, button);

                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(rowBackground);
                panel.add(BorderLayout.NORTH, textpanel);

                return panel;
            }
            else
            {
                JPanel panel = new JPanel();
                panel.setBackground(Color.lightGray);
                return panel;
            }
        }

        public Object getCellEditorValue()
        {
            String value = "";

            if (isPushed)
            {
                if (cellObj instanceof ArgumentValue)
                {
                    ((ArgumentValue)(cellObj)).setString(textEditValue);
                    fEditValueTextArea.setText(
                        ((WizardArgumentInterface)(cellObj)).getString());
                    fEditValueTextArea.requestFocus();
                    fEditValueDialog.setLocationRelativeTo(
                        button.getParent());
                    fEditValueDialog.setTitle("Edit Argument Value");

                    final String originalText = textEditValue;

                    fEditValueSaveButton.addActionListener(new
                        ActionListener()
                    {
                        public void actionPerformed(ActionEvent e)
                        {
                            ((WizardArgumentInterface)(cellObj)).setString(
                                fEditValueTextArea.getText());
                            fEditValueDialog.setVisible(false);
                        }
                    });

                    fEditValueCancelButton.addActionListener(new
                        ActionListener()
                    {
                        public void actionPerformed(ActionEvent e)
                        {
                            fEditValueDialog.setVisible(false);
                        }
                    });

                    fEditValueDialog.setVisible(true);
                    isPushed = false;
                }
                else if (cellObj instanceof ArgumentDescription)
                {
                    fMoreDescriptionEditorPane.setText(
                        ((WizardArgumentInterface)(cellObj)).getString());
                    fMoreDescriptionEditorPane.requestFocus();
                    fMoreDescriptionDialog.setLocationRelativeTo(
                        button.getParent());
                    fMoreDescriptionDialog.setTitle("Argument Description");

                    final String originalText = textEditValue;

                    fMoreDescriptionCloseButton.addActionListener(new
                        ActionListener()
                    {
                        public void actionPerformed(ActionEvent e)
                        {
                            fMoreDescriptionDialog.setVisible(false);
                        }
                    });

                    fMoreDescriptionDialog.setVisible(true);
                    isPushed = false;
                }
            }

            if (resetPushed)
            {
                String defaultValue = "";

                String function = (String)
                    fWizardFunctionsList.getSelectedValue();

                if (function.indexOf("(default)") > -1)
                {
                    function = function.substring(0, function.indexOf(" "));
                }

                Vector argDefaults = (Vector)
                    fWizardFunctionArgDefaultsMap.get(function);

                ((WizardArgumentInterface)(cellObj)).setString(
                    (String)argDefaults.get(inputRow - 1));
                resetPushed = false;
            }

            return cellObj;
        }

        public boolean shouldSelectCell(EventObject e)
        {
            return true;
        }

        public boolean stopCellEditing()
        {
            if (cellObj instanceof ArgumentValue)
            {
                ((WizardArgumentInterface)cellObj).
                    setString(textfield.getText());
            }

            isPushed = false;
            resetPushed = false;

            return super.stopCellEditing();
        }

        protected void fireEditingStopped()
        {
            super.fireEditingStopped();
        }
    }

    public void valueChanged(ListSelectionEvent e)
    {
        if (e.getSource() == fWizardFunctionsList)
        {
            updateWizardFunctionTable();
        }
        else
        {
            if (fActiveJobsTable.getSelectedRow() > -1)
            {
                fDisplaySelectedJobLog.setEnabled(true);
                fDisplaySelectedJobUserLog.setEnabled(true);
            }
            else
            {
                fDisplaySelectedJobLog.setEnabled(false);
                fDisplaySelectedJobUserLog.setEnabled(false);
            }
        }
    }

    public void itemStateChanged(ItemEvent e)
    {
        Object source = e.getSource();

        if ((source == fMonitorYesRB) ||
            (source == fMonitorNoRB) ||
            (source == fDefaultFunctionRB) ||
            (source == fOtherFunctionRB) ||
            (source == fMachineLocalRB) ||
            (source == fMachineOtherRB))
        {
            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }

        if (source == fMachineLocalRB)
        {
            if (fMachineLocalRB.isSelected())
            {
                fOtherXmlFileMachineField.setEnabled(false);
                fOtherXmlFileNameField.setEnabled(false);
                fOtherXmlFileMachineField.setEnabled(false);
                fBrowseButton.setEnabled(true);
                fLocalXmlFileNameField.setEnabled(true);
            }
        }
        else if (source == fMachineOtherRB)
        {
            if (fMachineOtherRB.isSelected())
            {
                fOtherXmlFileMachineField.setEnabled(true);
                fOtherXmlFileNameField.setEnabled(true);
                fBrowseButton.setEnabled(false);
                fLocalXmlFileNameField.setEnabled(false);
            }
        }
        else if (source == fDefaultFunctionRB)
        {
            if (fDefaultFunctionRB.isSelected())
            {
                fFunctionField.setEnabled(false);
            }
        }
        else if (source == fOtherFunctionRB)
        {
            if (fOtherFunctionRB.isSelected())
            {
                fFunctionField.setEnabled(true);
            }
        }
        if (source == fLocalScriptMachineRB)
        {
            if (fLocalScriptMachineRB.isSelected())
            {
                fScriptFilesMachineTextField.setEnabled(false);
                fAddScriptFilesBrowseButton.setEnabled(true);
            }
        }
        else if (source == fXMLJobFileScriptMachineRB)
        {
            if (fXMLJobFileScriptMachineRB.isSelected())
            {
                fScriptFilesMachineTextField.setEnabled(false);
                fAddScriptFilesBrowseButton.setEnabled(true);
            }
        }
        else if (source == fOtherScriptMachineRB)
        {
            if (fOtherScriptMachineRB.isSelected())
            {
                fScriptFilesMachineTextField.setEnabled(true);
                fAddScriptFilesBrowseButton.setEnabled(false);
            }
        }
        else if (source == fLimitMessages)
        {
            if (fLimitMessages.isSelected())
            {
                fLimitMessagesField.setEnabled(true);

                if ((fLimitMessagesFieldText == null) ||
                    (fLimitMessagesFieldText.equals("")))
                {
                    fLimitMessagesField.setText(fDefaultLimitMessagesText);
                }
                else
                {
                    fLimitMessagesField.setText(fLimitMessagesFieldText);
                }
            }
            else
            {
                fLimitMessagesField.setEnabled(false);
                fLimitMessagesField.setText("");
            }
        }
    }

    public void keyPressed(KeyEvent e) {}

    public void keyReleased(KeyEvent e) {}

    public void keyTyped(KeyEvent e)
    {
        Object source = e.getSource();

        if ((source == fJobNameField) ||
            (source == fFunctionField) ||
            (source == fArgs) ||
            (source == fLocalXmlFileNameField) ||
            (source == fOtherXmlFileMachineField) ||
            (source == fArguments) ||
            (source == fOtherXmlFileNameField))
        {
            fCurrentJobParmsNotSaved = true;
            fStartNewJobFileSave.setEnabled(true);
            fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                        fCurrentJobParmsFile + " *");
        }

    }

    public void mouseClicked(MouseEvent e)
    {
        if (e.getSource() == fScriptList)
        {
            if (e.getClickCount() == 2)
            {
                int editIndex = fScriptList.getSelectedIndex();

                if (editIndex > -1)
                {
                    String currentValue = (String)
                        ((DefaultListModel)(fScriptList.getModel())).
                        getElementAt(editIndex);
                    fEditScriptTextArea.setText(currentValue);
                    fEditScriptTextArea.setFont(
                        new Font("Monospaced", Font.PLAIN, 12));
                    fEditScriptTextArea.requestFocus();
                    fEditScriptDialog.setLocationRelativeTo(fStartNewJobDialog);
                    fEditScriptDialog.setVisible(true);
                }
            }
        }
        else if (e.getSource() == fScriptFilesList)
        {
            if (e.getClickCount() == 2)
            {
                int editIndex = fScriptFilesList.getSelectedIndex();

                if (editIndex > -1)
                {
                    String currentValue = (String)
                        ((DefaultListModel)(fScriptFilesList.getModel())).
                        getElementAt(editIndex);
                    fEditScriptFilesTextField.setText(currentValue);
                    fEditScriptFilesTextField.requestFocus();
                    fEditScriptFilesDialog.setLocationRelativeTo(
                        fStartNewJobDialog);
                    fEditScriptFilesDialog.setVisible(true);
                }
            }
        }
        else if (e.getSource() == fPluginJarsList)
        {
            if (e.getClickCount() == 2)
            {
                int editIndex = fPluginJarsList.getSelectedIndex();

                if (editIndex > -1)
                {
                    String currentValue = (String)
                        ((DefaultListModel)(fPluginJarsList.getModel())).
                        getElementAt(editIndex);
                    fEditPluginJarsTextField.setText(currentValue);
                    fEditPluginJarsTextField.requestFocus();
                    fEditPluginJarsDialog.setLocationRelativeTo(
                        fPropertiesDialog);
                    fEditPluginJarsDialog.setVisible(true);
                }
            }
        }
        else if (e.getSource() == fBreakpointTable)
        {
            if (e.getClickCount() == 2)
            {
                int editRow = fBreakpointTable.getSelectedRow();

                if (editRow > -1)
                {
                    String currentFunction = (String)
                        fBreakpointsTableModel.getValueAt(editRow, 0);
                    String currentLineNumber = (String)
                        fBreakpointsTableModel.getValueAt(editRow, 1);
                    String currentLineFile = (String)
                        fBreakpointsTableModel.getValueAt(editRow, 2);
                    String currentLineMachine = (String)
                        fBreakpointsTableModel.getValueAt(editRow, 3);

                    if (currentFunction.equals(""))
                    {
                        fEditBreakpointLineNumberTextField.setText(
                            currentLineNumber);
                        fEditBreakpointLineNumberTextField.setFont(
                            new Font("Dialog", Font.PLAIN, 12));

                        fEditBreakpointLineFileTextField.setText(
                            currentLineFile);
                        fEditBreakpointLineFileTextField.setFont(
                            new Font("Dialog", Font.PLAIN, 12));

                        fEditBreakpointLineMachineTextField.setText(
                            currentLineMachine);
                        fEditBreakpointLineMachineTextField.setFont(
                            new Font("Dialog", Font.PLAIN, 12));

                        fEditBreakpointLineNumberTextField.requestFocus();
                        fEditBreakpointLineDialog.setLocationRelativeTo(
                            fStartNewJobDialog);
                        fEditBreakpointLineDialog.setVisible(true);
                    }
                    else
                    {
                        fEditBreakpointFunctionTextField.setText(
                            currentFunction);
                        fEditBreakpointFunctionTextField.setFont(
                            new Font("Dialog", Font.PLAIN, 12));
                        fEditBreakpointFunctionTextField.requestFocus();
                        fEditBreakpointFunctionDialog.setLocationRelativeTo(
                            fStartNewJobDialog);
                        fEditBreakpointFunctionDialog.setVisible(true);
                    }
                }
            }
        }
    }

    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}

    public void mousePressed(MouseEvent e)
    {
        displayPopup(e);
    }

    public void mouseReleased(MouseEvent e)
    {
        displayPopup(e);
    }

    public void displayPopup(MouseEvent e)
    {
        if (e.isPopupTrigger())
        {
            synchronized (fActiveJobsModelSorter)
            {
                int tableIndex =
                    fActiveJobsTable.rowAtPoint(new Point(e.getX(), e.getY()));

                if (tableIndex > -1)
                {
                    fActiveJobsTable.setRowSelectionInterval(tableIndex,
                                                             tableIndex);

                    // Check what the status is for this job

                    String status = "";

                    try
                    {
                        status = (String)fActiveJobsTable.getValueAt(
                            tableIndex, 4);
                    }
                    catch (ClassCastException ex)
                    {
                        // XXX: This is for debugging how this could happen
                        System.out.println(
                            "Invalid status column in Active Jobs Table: " +
                            fActiveJobsTable.getValueAt(tableIndex, 4));
                        throw ex;
                    }

                    // Check if the job is already being monitored

                    Boolean isMonitored = new Boolean(false);

                    try
                    {
                        isMonitored = (Boolean)fActiveJobsTable.getValueAt(
                            tableIndex, 2);
                    }
                    catch (ClassCastException ex)
                    {
                        // XXX: This is for debugging how this could happen
                        System.out.println(
                            "Invalid isMonitored column in Active Jobs Table: " +
                            fActiveJobsTable.getValueAt(tableIndex, 2));
                        throw ex;
                    }

                    // Determine if the "Show Monitor Window" menu item for
                    // this job should be greyed out or not based on if the
                    // job is already being monitored or not.
                    // And determine if the "Start Monitoring" menu item for
                    // this job should be greyed out or not based on if the
                    // job is already be being monitored or not and if the
                    // job is in a Pending state.

                    if (isMonitored.booleanValue())
                    {
                        fJobShowMonitorMenuItem.setEnabled(true);
                        fJobStartMonitorMenuItem.setEnabled(false);
                    }
                    else
                    {
                        fJobShowMonitorMenuItem.setEnabled(false);

                        // Don't allow job to be monitored if it's in a
                        // "Pending" state

                        if (status.equals("Pending"))
                            fJobStartMonitorMenuItem.setEnabled(false);
                        else
                            fJobStartMonitorMenuItem.setEnabled(true);
                    }
                    
                    // Determine if the "Terminate Job" menu item for this
                    // job should be greyed out or not based on the job's
                    // status.  Only allow a job to be terminated if it is
                    // in a "Running" state.

                    if (status.equals("Complete"))
                        fJobTerminateJobMenuItem.setEnabled(false);
                    else if (status.equals("Pending"))
                        fJobTerminateJobMenuItem.setEnabled(false);
                    else
                        fJobTerminateJobMenuItem.setEnabled(true);
                    
                    fJobPopupMenu.show(e.getComponent(), e.getX(), e.getY());
                }
            }
        }
    }

    /**
     * This method is called when a job is no longer being monitored.
     * @param jobNumber A string containing the ID for the job that is no
     * longer being monitored
     */
    public void monitorExiting(String jobNumber)
    {
        synchronized(fActiveJobsModelSorter)
        {
            synchronized (fMonitorTable)
            {
                if (fMonitorTable.containsKey(jobNumber))
                    fMonitorTable.remove(jobNumber);
            }

            // Check if the job is found in the Active Jobs Table and if so,
            // set isComplete to true if the job is complete or false if it's
            // still running.

            Vector jobsVector = fActiveJobsTableModel.getDataVector();

            boolean isComplete = false;
            int rowIndex = -1;

            for (int j = 0; j < jobsVector.size(); j++)
            {
                if (((Vector)(jobsVector.elementAt(j))).elementAt(0).equals(
                    new Integer(jobNumber)))
                {
                    isComplete = ((String)((Vector)(jobsVector.elementAt(j))).
                                  elementAt(4)).equals("Complete");
                    rowIndex = j;
                    break;
                }
            }

            if (rowIndex == -1)
                return;  // Job ID not found in Active Jobs Table

            if (isComplete)
            {
                // Remove the completed job from the Active Jobs Table since
                // it is no longer being monitored. 

                fJobPopupMenu.setVisible(false);

                fActiveJobsTableModel.removeRow(rowIndex);
                ((STAXMonitorTableCellRenderer)fActiveJobsTable.
                 getColumnModel().getColumn(0).getCellRenderer()).
                    clearRowHeights();
                    
                repaint();
                invalidate();
                validate();
            }
            else
            {
                // Indicate that the job is no longer being monitored

                fActiveJobsTableModel.setValueAt(
                    new Boolean(false), rowIndex, 2);
            }

            synchronized (fActiveJobsTable)
            {
                fActiveJobsTable.updateUI();
                STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
                STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
            }
        }
    }

    public static void main(String argv[])
    {
        new STAXMonitor(argv);
    }

    /**
     * Converts the "pretty" value for "Python Output", as displayed by the
     * STAX Monitor in the combo box, to a valid value for the PYTHONOUTPUT
     * option
     * @param pythonOutput A string containing the "pretty" value for the
     * PYTHONOUTPUT option
     * @returns A string containing a valid value for the PYTHONOUTPUT option
     */
    private static String getPythonOutputFromPrettyString(String pythonOutput)
    {
        if (pythonOutput.equals(JOBUSERLOG_PRETTY))
            return JOBUSERLOG_STRING;
        else if (pythonOutput.equals(MESSAGE_PRETTY))
            return MESSAGE_STRING;
        else if (pythonOutput.equals(JOBUSERLOGANDMSG_PRETTY))
            return JOBUSERLOGANDMSG_STRING;
        else if (pythonOutput.equals(JVMLOG_PRETTY))
            return JVMLOG_STRING;
        else
            return DEFAULT_STRING;
    }

    /**
     * Converts a valid value for the PYTHONOUTPUT option to the "pretty"
     * value for "Python Output", as displayed by the
     * STAX Monitor in the combo box
     * @param pythonOutput A string containing a valid value for the
     * PYTHONOUTPUT option
     * @returns A string containing the "pretty" value for "Python Output",
     * that can be displayed by the STAX Monitor in the "Python Output"
     * combo box
     */
    private static String getPrettyPythonOutput(String pythonOutput)
    {
        if (pythonOutput.equals(JOBUSERLOG_STRING))
            return JOBUSERLOG_PRETTY;
        else if (pythonOutput.equals(MESSAGE_STRING))
            return MESSAGE_PRETTY;
        else if (pythonOutput.equals(JOBUSERLOGANDMSG_STRING))
            return JOBUSERLOGANDMSG_PRETTY;
        else if (pythonOutput.equals(JVMLOG_STRING))
            return JVMLOG_PRETTY;
        else
            return DEFAULT_STRING;
    }

    private String resolveVar(String optionValue, int handle)
    {
        String value = optionValue;
        STAFResult resolvedResult = null;

        if (optionValue.indexOf("{") != -1)
        {
            resolvedResult = fHandle.submit2(
                "local", "VAR", "RESOLVE ASHANDLE " + handle + " STRING " +
                optionValue);

            if (resolvedResult.rc != 0)
            {
                return optionValue;
            }

            value = resolvedResult.result;
        }

        return value;
    }

    public boolean validateProperties()
    {
        if (fStaxMachineNameField.getText().equals(""))
        {
            JOptionPane.showMessageDialog(fPropertiesDialog,
                "You must enter the STAX Machine Name",
                "Properties Error", JOptionPane.ERROR_MESSAGE);
            return false;
        }

        if (fStaxServiceNameField.getText().equals(""))
        {
            JOptionPane.showMessageDialog(fPropertiesDialog,
                "You must enter the STAX Service Name",
                "Properties Error", JOptionPane.ERROR_MESSAGE);
            return false;
        }

        // Get Event machine/service based on STAX machine/service

        EventServiceInfo eventServiceInfo = new EventServiceInfo(
            fStaxMachineNameField.getText(), fStaxServiceNameField.getText(),
            this, true);

        fEventMachineNameField.setText(eventServiceInfo.getMachine());
        fEventServiceNameField.setText(eventServiceInfo.getService());

        String processMonitorSeconds = fProcessMonitorSecondsField.getText();
        Integer seconds;

        try
        {
            seconds = new Integer(processMonitorSeconds);
        }
        catch (NumberFormatException e)
        {
            JOptionPane.showMessageDialog(fPropertiesDialog,
                "Invalid number specified for Process Monitor Seconds",
                "Properties Error", JOptionPane.ERROR_MESSAGE);

            return false;
        }

        if (seconds.intValue() < 0)
        {
            JOptionPane.showMessageDialog(fPropertiesDialog,
                "Process Monitor Seconds must be 0 or greater",
                "Properties Error", JOptionPane.ERROR_MESSAGE);

            return false;
        }

        String elapsedTimeSeconds = fElapsedTimeSecondsField.getText();

        try
        {
            seconds = new Integer(elapsedTimeSeconds);
        }
        catch (NumberFormatException e)
        {
            JOptionPane.showMessageDialog(fPropertiesDialog,
                "Invalid number specified for Elapsed Time Seconds",
                "Properties Error", JOptionPane.ERROR_MESSAGE);

            return false;
        }

        if (seconds.intValue() < 0)
        {
            JOptionPane.showMessageDialog(fPropertiesDialog,
                "Elapsed Time Seconds must be 0 or greater",
                "Properties Error", JOptionPane.ERROR_MESSAGE);

            return false;
        }

        return true;
    }

    public boolean validateParms(String type)
    {
        if (fMachineLocalRB.isSelected())
        {
            if (fLocalXmlFileNameField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(
                    fStartNewJobDialog,
                    "You must enter the local XML File name",
                    "Error " + type + "ing job",
                    JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }
        else
        {
            if (fOtherXmlFileNameField.getText().equals(""))
            {
                JOptionPane.showMessageDialog(
                    fStartNewJobDialog,
                    "You must enter the other XML File name",
                    "Error " + type + "ing job",
                    JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }

        if (fMachineOtherRB.isSelected() &&
            fOtherXmlFileMachineField.getText().equals(""))
        {
            JOptionPane.showMessageDialog(
                fStartNewJobDialog,
                "You must enter the Machine name",
                "Error " + type + "ing job",
                JOptionPane.ERROR_MESSAGE);
            return false;
        }

        if (fOtherFunctionRB.isSelected() &&
            fFunctionField.getText().equals(""))
        {
            JOptionPane.showMessageDialog(
                fStartNewJobDialog,
                "You must enter the Function name",
                "Error " + type + "ing job",
                JOptionPane.ERROR_MESSAGE);
            return false;
        }

        if (fOtherScriptMachineRB.isSelected() &&
            fScriptFilesMachineTextField.getText().equals(""))
        {
            JOptionPane.showMessageDialog(
                fStartNewJobDialog,
                "You must enter the Other Script File machine name",
                "Error " + type + "ing job",
                JOptionPane.ERROR_MESSAGE);
            return false;
        }

        return true;
    }

    public boolean submitNewJob()
    {
        if (fMonitorNoRB.isSelected())
        {
            StringBuffer request = new StringBuffer();

            if (fMachineLocalRB.isSelected())
            {
                request.append("EXECUTE FILE ").append(
                    STAFUtil.wrapData(fLocalXmlFileName));
            }
            else
            {
                request.append("EXECUTE FILE ").append(
                    STAFUtil.wrapData(fOtherXmlFileName));
                request.append(" MACHINE ").append(
                           STAFUtil.wrapData(fOtherXmlFileMachineName));
            }

            if (fOtherFunctionRB.isSelected())
            {
                request.append(" FUNCTION ").append(
                    STAFUtil.wrapData(fFunction));
            }

            if (fArgs != null && !(fArgs.equals("")))
            {
                request.append(" ARGS ").append(STAFUtil.wrapData(fArgs));
            }

            if (fArgs != null && !fJobName.equals(""))
            {
                request.append(" JOBNAME ").append(
                    STAFUtil.wrapData(fJobName));
            }

            fScriptVector = new Vector();
            Enumeration e = ((DefaultListModel)(fScriptList.getModel())).
                elements();

            while (e.hasMoreElements())
            {
                String nextScript = (String)e.nextElement();
                fScriptVector.addElement(nextScript);
                request.append(" SCRIPT ").append(
                    STAFUtil.wrapData(nextScript));
            }

            if (((DefaultListModel)(fScriptFilesList.getModel())).getSize() > 0)
            {
                if (fXMLJobFileScriptMachineRB.isSelected())
                {
                    // Don't add a SCRIPTFILEMACHINE option since it will
                    // default to the XML Job File machine if not specified
                }
                else if (fLocalScriptMachineRB.isSelected())
                {
                    request.append(" SCRIPTFILEMACHINE ").append(
                        STAFUtil.wrapData(fSTAXMonitorEndpoint));
                }
                else if (fOtherScriptMachineRB.isSelected())
                {
                    request.append(" SCRIPTFILEMACHINE ").append(
                        STAFUtil.wrapData(fScriptFilesMachineName));
                }
            }

            fScriptFilesVector = new Vector();
            e = ((DefaultListModel)(fScriptFilesList.getModel())).elements();

            while (e.hasMoreElements())
            {
                String nextScriptFile = (String)e.nextElement();
                fScriptFilesVector.addElement(nextScriptFile);
                request.append(" SCRIPTFILE ").append(
                    STAFUtil.wrapData(nextScriptFile));
            }

            if (fClearLogsYesRB.isSelected())
            {
                request.append(" CLEARLOGS Enabled");
            }
            else if (fClearLogsNoRB.isSelected())
            {
                request.append(" CLEARLOGS Disabled");
            }

            if (fLogTCElapsedTimeYesRB.isSelected())
            {
                request.append(" LOGTCELAPSEDTIME Enabled");
            }
            else if (fLogTCElapsedTimeNoRB.isSelected())
            {
                request.append(" LOGTCELAPSEDTIME Disabled");
            }

            if (fLogTCNumStartsYesRB.isSelected())
            {
                request.append(" LOGTCNUMSTARTS Enabled");
            }
            else if (fLogTCNumStartsNoRB.isSelected())
            {
                request.append(" LOGTCNUMSTARTS Disabled");
            }

            if (fLogTCStartStopYesRB.isSelected())
            {
                request.append(" LOGTCSTARTSTOP Enabled");
            }
            else if (fLogTCStartStopNoRB.isSelected())
            {
                request.append(" LOGTCSTARTSTOP Disabled");
            }

            String pythonOutput = getPythonOutputFromPrettyString(
                (String)fPythonOutputCB.getSelectedItem());

            if (!pythonOutput.equals(DEFAULT_STRING))
            {
                request.append(" PYTHONOUTPUT " + pythonOutput);
            }

            String pythonLogLevel = (String)fPythonLogLevelCB.getSelectedItem();

            if (!pythonLogLevel.equals(DEFAULT_STRING))
            {
                request.append(" PYTHONLOGLEVEL " + pythonLogLevel);
            }

            for (int i = 0; i < fBreakpointTable.getRowCount(); i++)
            {
                String function =
                    ((String)fBreakpointTable.getValueAt(i, 0)).trim();
                String line =
                    ((String)fBreakpointTable.getValueAt(i, 1)).trim();
                String xmlFile =
                    ((String)fBreakpointTable.getValueAt(i, 2)).trim();
                String xmlMachine =
                    ((String)fBreakpointTable.getValueAt(i, 3)).trim();

                if (function.equals(""))
                {
                    if (xmlFile.equals(""))
                    {
                        if (fMachineLocalRB.isSelected())
                        {
                            xmlFile = fLocalXmlFileName;
                        }
                        else
                        {
                            xmlFile = fOtherXmlFileName;
                        }
                    }

                    if (!(xmlMachine.equals("")))
                    {
                        xmlMachine = " MACHINE " + STAFUtil.wrapData(xmlMachine);
                    }

                    if (!(xmlFile.equals("")))
                    {
                        request.append(" BREAKPOINT " +
                            STAFUtil.wrapData(line + "@@" +
                                              xmlFile + "@@" +
                                              xmlMachine));
                    }
                }
                else
                {
                    request.append(" BREAKPOINT " +
                        STAFUtil.wrapData(function));
                }
            }
            
            if (fBreakpointFirstFunctionCB.isSelected())
            {
                request.append(" BREAKPOINTFIRSTFUNCTION");
            }

            if (fBreakpointSubjobFirstFunctionCB.isSelected())
            {
                request.append(" BREAKPOINTSUBJOBFIRSTFUNCTION");
            }
            
            // Submit the STAX EXECUTE request and display a "Please Wait"
            // dialog while the request is preparing to run the job

            STAXMonitorExecuteResult executeResult = submitExecuteRequest(
                request.toString());

            if (executeResult.getRC() != STAFResult.Ok)
            {
                // Display a "STAX Error" popup with an error message

                STAXMonitorUtil.showErrorDialog(
                    this,
                    executeResult.getResult().toString(),
                    new Font("Courier", Font.PLAIN, 12));

                return false;
            }
        }
        else
        {
            try
            {
                fScriptVector = new Vector();
                Enumeration e = ((DefaultListModel)(fScriptList.getModel())).
                    elements();

                while (e.hasMoreElements())
                {
                    fScriptVector.addElement(e.nextElement());
                }

                fScriptFilesVector = new Vector();
                e = ((DefaultListModel)
                    (fScriptFilesList.getModel())).elements();

                while (e.hasMoreElements())
                {
                    fScriptFilesVector.addElement(e.nextElement());
                }

                String file = "";
                String xmlMachine = "";
                String function = "";

                if (fMachineLocalRB.isSelected())
                {
                    file = fLocalXmlFileName;
                }
                else
                {
                    file = fOtherXmlFileName;
                    xmlMachine = fOtherXmlFileMachineName;
                }

                if (fOtherFunctionRB.isSelected())
                {
                    function = fFunction;
                }

                String limitMsgs = "";

                if (fLimitMessagesField.getText().equals(""))
                {
                    limitMsgs = "-1";
                }
                else
                {
                    limitMsgs = fLimitMessagesField.getText();
                }

                String scriptFilesMachineName = fScriptFilesMachineName;

                if (fLocalScriptMachineRB.isSelected())
                    scriptFilesMachineName = fSTAXMonitorEndpoint;
                else if (fOtherScriptMachineRB.isSelected())
                    scriptFilesMachineName = fScriptFilesMachineName;
                else
                    scriptFilesMachineName = "";

                int autoMonitor = 0;

                if (fAutoMonitorSubjobs)
                {
                    autoMonitor = STAXMonitorFrame.AUTOMONITOR_ALWAYS;
                }
                else if (fAutoMonitorRecommendedSubjobs)
                {
                    autoMonitor = STAXMonitorFrame.AUTOMONITOR_RECOMMENDED;
                }
                else if (fNeverAutoMonitorSubjobs)
                {
                    autoMonitor = STAXMonitorFrame.AUTOMONITOR_NEVER;
                }

                int clearLogs = STAXMonitorFrame.DEFAULT;

                if (fClearLogsYesRB.isSelected())
                    clearLogs = STAXMonitorFrame.ENABLED;
                else if (fClearLogsNoRB.isSelected())
                    clearLogs = STAXMonitorFrame.DISABLED;

                int logTCElapsedTime = STAXMonitorFrame.DEFAULT;

                if (fLogTCElapsedTimeYesRB.isSelected())
                    logTCElapsedTime = STAXMonitorFrame.ENABLED;
                else if (fLogTCElapsedTimeNoRB.isSelected())
                    logTCElapsedTime = STAXMonitorFrame.DISABLED;

                int logTCNumStarts = STAXMonitorFrame.DEFAULT;

                if (fLogTCNumStartsYesRB.isSelected())
                    logTCNumStarts = STAXMonitorFrame.ENABLED;
                else if (fLogTCNumStartsNoRB.isSelected())
                    logTCNumStarts = STAXMonitorFrame.DISABLED;

                int logTCStartStop = STAXMonitorFrame.DEFAULT;

                if (fLogTCStartStopYesRB.isSelected())
                    logTCStartStop = STAXMonitorFrame.ENABLED;
                else if (fLogTCStartStopNoRB.isSelected())
                    logTCStartStop = STAXMonitorFrame.DISABLED;


                String pythonOutput = getPythonOutputFromPrettyString(
                    (String)fPythonOutputCB.getSelectedItem());

                String pythonLogLevel =
                    (String)fPythonLogLevelCB.getSelectedItem();

                Vector breakpointTriggerVector = new Vector();

                for (int i = 0; i < fBreakpointTable.getRowCount(); i++)
                {
                    String bpFunction =
                        ((String)fBreakpointTable.getValueAt(i, 0)).trim();
                    String bpLine =
                        ((String)fBreakpointTable.getValueAt(i, 1)).trim();
                    String bpXmlFile =
                        ((String)fBreakpointTable.getValueAt(i, 2)).trim();
                    String bpXmlMachine =
                        ((String)fBreakpointTable.getValueAt(i, 3)).trim();

                    if (bpFunction.equals(""))
                    {
                        if (bpXmlFile.equals(""))
                        {
                            if (fMachineLocalRB.isSelected())
                            {
                                bpXmlFile = fLocalXmlFileName;
                            }
                            else
                            {
                                bpXmlFile = fOtherXmlFileName;
                            }
                        }

                        if (!(bpXmlMachine.equals("")))
                        {
                            bpXmlMachine = " MACHINE " +
                                STAFUtil.wrapData(bpXmlMachine);
                        }

                        if (!(bpXmlFile.equals("")))
                        {
                            breakpointTriggerVector.add(" BREAKPOINT " +
                                STAFUtil.wrapData(bpLine + "@@" +
                                                  bpXmlFile + "@@" +
                                                  bpXmlMachine));
                        }
                    }
                    else
                    {
                        breakpointTriggerVector.add(" BREAKPOINT " +
                            STAFUtil.wrapData(bpFunction));
                    }
                }

                boolean breakpointFirstFunction =
                    fBreakpointFirstFunctionCB.isSelected();

                boolean breakpointSubjobFirstFunction =
                    fBreakpointSubjobFirstFunctionCB.isSelected();

                STAXMonitorFrame monitor = new STAXMonitorFrame(
                    this, fStaxMachineName, fStaxServiceName,
                    fStaxMachineNickname, "",
                    fEventMachineName, fEventServiceName,
                    file, xmlMachine, function, fArgs, fJobName,
                    fScriptVector, fShowNoSTAXMonitorInformation.isSelected(),
                    limitMsgs, fLocalExtJarFiles, fScriptFilesVector,
                    scriptFilesMachineName, clearLogs, logTCElapsedTime,
                    logTCNumStarts, logTCStartStop, autoMonitor,
                    pythonOutput, pythonLogLevel, breakpointTriggerVector,
                    breakpointFirstFunction, breakpointSubjobFirstFunction);

                synchronized (fMonitorTable)
                {
                    fMonitorTable.put(monitor.getJobNumber(), monitor);

                    if (fStartNewJobCloseOnEnd)
                    {
                        fCloseOnEndJobID = monitor.getJobNumber();
                    }
                }

                // Update the "Monitored" column for the job in the Active
                // Jobs Table to show the job is being monitored

                synchronized (fActiveJobsTable)
                {
                    Vector jobsVector = fActiveJobsTableModel.getDataVector();
                    
                    for (int j = 0; j < jobsVector.size(); j++)
                    {
                        if (((Vector)(jobsVector.elementAt(j))).elementAt(0).
                            equals(new Integer(monitor.getJobNumber())))
                        {
                            fActiveJobsModelSorter.setValueAt(
                                new Boolean(true), j, 2);
                            fActiveJobsTable.updateUI();
                            STAXMonitorUtil.sizeColumnsToFitText(
                                fActiveJobsTable);
                            STAXMonitorUtil.updateRowHeights(
                                fActiveJobsTable, 7);
                            break;
                        }
                    }
                }
            }
            catch (STAFException ex)
            {
                // Display a "STAX Error" popup with an error message

                STAXMonitorUtil.showErrorDialog(
                    this, ex.getMessage(),
                    new Font("Courier", Font.PLAIN, 12));

                return false;
            }
        }

        return true;
    }

    public boolean testJob()
    {
        StringBuffer request = new StringBuffer();

        if (fMachineLocalRB.isSelected())
        {
            request.append("EXECUTE FILE ").append(
                       STAFUtil.wrapData(fLocalXmlFileName));
        }
        else
        {
            request.append("EXECUTE FILE ").append(
                      STAFUtil.wrapData(fOtherXmlFileName));
            request.append(" MACHINE ").append(
                       STAFUtil.wrapData(fOtherXmlFileMachineName));
        }

        if (fOtherFunctionRB.isSelected())
        {
            request.append(" FUNCTION ").append(STAFUtil.wrapData(fFunction));
        }

        if (!(fArgs.equals("")))
        {
            request.append(" ARGS ").append(STAFUtil.wrapData(fArgs));
        }

        if (!fJobName.equals(""))
        {
            request.append(" JOBNAME ").append(STAFUtil.wrapData(fJobName));
        }

        fScriptVector = new Vector();
        Enumeration e = ((DefaultListModel)(fScriptList.getModel())).
            elements();

        while (e.hasMoreElements())
        {
            String nextScript = (String)e.nextElement();
            fScriptVector.addElement(nextScript);
            request.append(" SCRIPT ").append(STAFUtil.wrapData(nextScript));
        }

        if (((DefaultListModel)(fScriptFilesList.getModel())).getSize() > 0)
        {
            if (fXMLJobFileScriptMachineRB.isSelected())
            {
                // Don't add a SCRIPTFILEMACHINE option since it will
                // default to the XML Job File machine if not specified
            }
            else if (fLocalScriptMachineRB.isSelected())
            {
                request.append(" SCRIPTFILEMACHINE ").append(
                    STAFUtil.wrapData(fSTAXMonitorEndpoint));
            }
            else if (fOtherScriptMachineRB.isSelected())
            {
                request.append(" SCRIPTFILEMACHINE ").append(
                    STAFUtil.wrapData(fScriptFilesMachineName));
            }
        }

        fScriptFilesVector = new Vector();
        e = ((DefaultListModel)(fScriptFilesList.getModel())).elements();

        while (e.hasMoreElements())
        {
            String nextScriptFile = (String)e.nextElement();
            fScriptFilesVector.addElement(nextScriptFile);
            request.append(" SCRIPTFILE ").append(
                STAFUtil.wrapData(nextScriptFile));
        }

        request.append(" TEST");
        
        // Submit the STAX EXECUTE request and display a "Please Wait" dialog
        // while the request is running to validate the xml file

        STAXMonitorExecuteResult executeResult = submitExecuteRequest(
            request.toString());

        if (executeResult.getRC() != STAFResult.Ok)
        {
            // Display a "STAX Error" popup with an error message

            STAXMonitorUtil.showErrorDialog(
                fStartNewJobDialog,
                executeResult.getResult().toString(),
                new Font("Courier", Font.PLAIN, 12));

            return false;
        }

        // Display a "Validation Successful" popup with the Job ID in the
        // message

        JOptionPane.showMessageDialog(
            fStartNewJobDialog,
            "Validation Successful\n\n" +
            "The job ID generated was: " + executeResult.getResult(),
            "Validation Successful",
            JOptionPane.INFORMATION_MESSAGE);

        return true;
    }

    /**
     * Submits the specified EXECUTE request to the STAX service and displays
     * a * "Please wait" message while the request is preparing to execute
     * the STAX job.
     * @param request The EXECUTE request to be submitted to the STAX service
     * @return STAXMonitorExecuteResult
     * If no error occurred submitting the request, its fRC field will be 0
     * and its fResult field will contain the result from the request.
     * Or, if an error occurred submitting the request, its fRC field will be
     * non-zero and its fResult field will contain an error message string.
     */ 
    public STAXMonitorExecuteResult submitExecuteRequest(String request)
    {
        // Create a new handle to submit the STAX EXECUTE request
        // asynchronously so that it's queue will only contain the
        // STAF/RequestComplete message for this STAF service request

        STAFHandle fSubmitHandle = null;

        try
        {
            fSubmitHandle = STAXMonitorUtil.getNewSTAFHandle(
                fStaxServiceName + "/JobMonitor/" + fStaxMachineName +
                "/SubmitExecute");
        }
        catch (STAFException ex)
        {
            return new STAXMonitorExecuteResult(
                ex.rc,
                "Creating a new STAF handle to submit a STAX EXECUTE " +
                "request failed.\n" + ex.toString());
        }

        // Submit the STAX EXECUTE request asynchronously because it
        // may take more than a few seconds to validate the xml file
        // (especially if it contains <function-import> elements)

        STAFResult result = fSubmitHandle.submit2(
            STAFHandle.ReqQueue,
            fStaxMachineName, fStaxServiceName, request.toString());

        int requestNumber = 0;

        if (result.rc == 0)
            requestNumber = Integer.parseInt(result.result);

        if ((result.rc != 0) || (requestNumber == 0))
        {
            // Submitting the STAX EXECUTE request failed
            
            return new STAXMonitorExecuteResult(
                result.rc,
                "RC: " + result.rc + "\n" + result.result +
                "\n\nSTAF " + fStaxMachineName + " " + fStaxServiceName +
                " " + request.toString() +
                "\n\nSubmitting this request asynchronously failed.");
        }

        // Display a "Please Wait" pop-up while waiting for the
        // asynchronous submit of the STAX EXECUTE request to complete

        STAXMonitorWaitDialog waitDialog = new STAXMonitorWaitDialog(this);

        // Wait for the STAX EXECUTE request to complete

        String getRequest = "GET WAIT TYPE STAF/RequestComplete";
        result = fSubmitHandle.submit2("local", "QUEUE", getRequest);

        // Delete the fSubmitHandle since no longer needed

        STAXMonitorUtil.freeHandle(fSubmitHandle.getHandle());

        // No longer display the wait dialog
        waitDialog.hideDialog();

        // Display an error popup if the QUEUE GET WAIT request failed

        if (result.rc != STAFResult.Ok)
        {
            return new STAXMonitorExecuteResult(
                result.rc,
                "RC: " + result.rc + "\n" + result.result +
                "\n\nSTAF local QUEUE " + getRequest);
        }

        // Get the STAX EXECUTE request's RC and result from the message
        // sent to the fSubmitHandle's queue

        Map queueMap = (Map)result.resultObj;
        Map messageMap = (Map)queueMap.get("message");
        int executeRC = Integer.parseInt((String)messageMap.get("rc"));

        if (executeRC != STAFResult.Ok)
        {
            // Note that the "result" field in the STAF/RequestComplete
            // message may contain a String or a STAFMarshallingContext object

            String command = "STAF " + fStaxMachineName + " " +
                fStaxServiceName + " " + request.toString();
            
            return new STAXMonitorExecuteResult(
                executeRC,
                generateErrorMsg(
                    executeRC,
                    messageMap.get("result").toString() + "\n\n" + command));
        }

        return new STAXMonitorExecuteResult(
            STAFResult.Ok, messageMap.get("result"));
    }

    public void monitorExistingJob(String jobNumber, int rowIndex)
    {
        if (fMonitorTable.containsKey(jobNumber))
        {
            STAXMonitorFrame monitorFrame =
                (STAXMonitorFrame)fMonitorTable.get(jobNumber);
            monitorFrame.setState(Frame.NORMAL);
            monitorFrame.toFront();
            monitorFrame.requestFocus();
            return;
        }

        try
        {
            String limitMsgs = "";

            if (fLimitMessagesField.getText().equals(""))
            {
                limitMsgs = "-1";
            }
            else
            {
                limitMsgs = fLimitMessagesField.getText();
            }

            int autoMonitor = 0;

            if (fAutoMonitorSubjobs)
            {
                autoMonitor = STAXMonitorFrame.AUTOMONITOR_ALWAYS;
            }
            else if (fAutoMonitorRecommendedSubjobs)
            {
                autoMonitor = STAXMonitorFrame.AUTOMONITOR_RECOMMENDED;
            }
            else if (fNeverAutoMonitorSubjobs)
            {
                autoMonitor = STAXMonitorFrame.AUTOMONITOR_NEVER;
            }

            // 817339
            // Query the job to get the information necessary to
            // restart the job.

            STAFResult queryResult = fHandle.submit2(
                fStaxMachineName, fStaxServiceName, "QUERY JOB " + jobNumber);

            if (queryResult.rc != 0)
            {
                return;
            }

            Map jobInfoMap = (HashMap)queryResult.resultObj;
            
            String jobName       = (String)jobInfoMap.get("jobName");
            String xmlfile       = (String)jobInfoMap.get("xmlFileName");
            String fileMachine   = (String)jobInfoMap.get("fileMachine");
            String function      = (String)jobInfoMap.get("function");
            String args          = (String)jobInfoMap.get("arguments");
            String scriptMachine = (String)jobInfoMap.get("scriptMachine");
            String state         = (String)jobInfoMap.get("state");

            // XXX: Is this right?
            if (state.equals("Pending"))
            {
                STAXMonitorUtil.showErrorDialog(
                    this, "Cannot monitor a job in a Pending state.  " +
                    "Please wait until the job is in a Running state.");
                return;
            }

            // Convert the list of scripts to a vector of scripts

            java.util.List scriptList =
                (java.util.List)jobInfoMap.get("scriptList");
            Vector scripts = new Vector();

            Iterator iter = scriptList.iterator();

            while (iter.hasNext())
            {
                scripts.add((String)iter.next());
            }

            // Convert the list of scriptfiles to a vector of scriptfiles

            java.util.List scriptFileList =
                (java.util.List)jobInfoMap.get("scriptFileList");
            Vector scriptFiles = new Vector();

            iter = scriptFileList.iterator();

            while (iter.hasNext())
            {
                scriptFiles.add((String)iter.next());
            }

            // Convert the enabled/disabled strings for clearLogs,
            // logTCElapsedTime, logTCNumStarts, and logTCStartStop to ints

            String clearLogsString = (String)jobInfoMap.get("clearLogs");
            int clearLogs = STAXMonitorFrame.DEFAULT;

            if (clearLogsString.equals("Enabled"))
                clearLogs = STAXMonitorFrame.ENABLED;
            else if (clearLogsString.equals("Disabled"))
                clearLogs = STAXMonitorFrame.DISABLED;
            
            String logTCElapsedTimeString =
                (String)jobInfoMap.get("logTCElapsedTime");
            int logTCElapsedTime = STAXMonitorFrame.DEFAULT;

            if (logTCElapsedTimeString.equals("Enabled"))
                logTCElapsedTime = STAXMonitorFrame.ENABLED;
            else if (logTCElapsedTimeString.equals("Disabled"))
                logTCElapsedTime = STAXMonitorFrame.DISABLED;

            String logTCNumStartsString =
                (String)jobInfoMap.get("logTCNumStarts");
            int logTCNumStarts = STAXMonitorFrame.DEFAULT;

            if (logTCNumStartsString.equals("Enabled"))
                logTCNumStarts = STAXMonitorFrame.ENABLED;
            else if (logTCNumStartsString.equals("Disabled"))
                logTCNumStarts = STAXMonitorFrame.DISABLED;

            String logTCStartStopString =
                (String)jobInfoMap.get("logTCStartStop");
            int logTCStartStop = STAXMonitorFrame.DEFAULT;

            if (logTCStartStopString.equals("Enabled"))
                logTCStartStop = STAXMonitorFrame.ENABLED;
            else if (logTCStartStopString.equals("Disabled"))
                logTCStartStop = STAXMonitorFrame.DISABLED;

            STAXMonitorFrame monitor = new STAXMonitorFrame(
                false, this, fStaxMachineName, fStaxServiceName,
                fStaxMachineNickname, jobNumber,
                fEventMachineName, fEventServiceName,
                xmlfile, fileMachine, function, args, jobName, scripts,
                fShowNoSTAXMonitorInformation.isSelected(), limitMsgs,
                fLocalExtJarFiles, scriptFiles, scriptMachine, clearLogs,
                logTCElapsedTime, logTCNumStarts, logTCStartStop,
                autoMonitor);

            synchronized (fMonitorTable)
            {
                fMonitorTable.put(jobNumber, monitor);
            }

            synchronized (fActiveJobsModelSorter)
            {
                // If STAXMonitorFrame calls this method, it passes -1 for the
                // rowIndex since it doesn't know the row number for the job.
                // So, determine the row index for the job if rowIndex == -1.

                if (rowIndex == -1)
                {
                    Vector jobsVector = fActiveJobsTableModel.getDataVector();

                    for (int j = 0; j < jobsVector.size(); j++)
                    {
                        if (((Vector)(jobsVector.elementAt(j))).elementAt(0).equals(
                            new Integer(jobNumber)))
                        {
                            rowIndex = j;
                            break;
                        }
                    }
                }

                if (rowIndex == -1)
                    return;  // Job not found in Active Job Table.
                
                // Job found in Active Job Table.  Set the "Monitored" column
                // for the job to show it is being monitored

                fActiveJobsModelSorter.setValueAt(
                    new Boolean(true), rowIndex, 2);

                synchronized (fActiveJobsTable)
                {
                    fActiveJobsTable.updateUI();
                    STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
                    STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
                }
            }
        }
        catch (STAFException ex)
        {
            // Display a "STAX Error" popup with an error message

            STAXMonitorUtil.showErrorDialog(
                this, ex.getMessage(), new Font("Courier", Font.PLAIN, 12));
        }
    }

    public void addMonitoredJob(STAXMonitorFrame monitor)
    {
        synchronized (fMonitorTable)
        {
            fMonitorTable.put(monitor.getJobNumber(), monitor);
        }
    }

    public void showBrowseFileDialog()
    {
        STAXMonitorFileFilter filter = new STAXMonitorFileFilter();
        filter.addExtension("xml");
        JFileChooser xmlFileChooser = new JFileChooser(fLastFileDirectory);
        xmlFileChooser.setFileFilter(filter);
        xmlFileChooser.setDialogTitle("Select an XML Job Definition File");
        int rc = xmlFileChooser.showDialog(this, "Select this XML File");

        if (rc == JFileChooser.APPROVE_OPTION)
        {
            try
            {
                fLocalXmlFileNameField.setText(
                    xmlFileChooser.getSelectedFile().getCanonicalPath());
                fLastFileDirectory = xmlFileChooser.getCurrentDirectory();

                fCurrentJobParmsNotSaved = true;
                fStartNewJobFileSave.setEnabled(true);
                fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                            fCurrentJobParmsFile + " *");
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    public void showOpenJobParmsFileDialog()
    {
        JFileChooser jobParmsFileChooser =
            new JFileChooser(fLastJobParmsFileDirectory);
        jobParmsFileChooser.setDialogTitle("Open Job Parameters file");
        int rc = jobParmsFileChooser.showOpenDialog(this);

        if (rc == JFileChooser.APPROVE_OPTION)
        {
            try
            {
                String fileName =
                    jobParmsFileChooser.getSelectedFile().getCanonicalPath();
                loadJobParms(fileName);

                File jobParmsFileDirectory =
                    jobParmsFileChooser.getCurrentDirectory();

                if (!(jobParmsFileDirectory.equals(fLastJobParmsFileDirectory)))
                {
                    fLastJobParmsFileDirectory = jobParmsFileDirectory;
                    saveProperties();
                }

                fCurrentJobParmsNotSaved = false;
                fCurrentJobParmsFile = fileName;
                fStartNewJobFileSave.setEnabled(false);
                fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                            fileName);

                updateRecentFiles(fileName);
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    public void showSaveJobParmsFileDialog()
    {
        JFileChooser jobParmsFileChooser =
            new JFileChooser(fLastJobParmsFileDirectory);
        jobParmsFileChooser.setDialogTitle("Save current Job Parameters as");
        int rc = jobParmsFileChooser.showSaveDialog(this);

        if (rc == JFileChooser.APPROVE_OPTION)
        {
            try
            {
                String fileName =
                    jobParmsFileChooser.getSelectedFile().getCanonicalPath();
                saveJobParms(fileName);
                fLastJobParmsFileDirectory =
                    jobParmsFileChooser.getCurrentDirectory();

                fCurrentJobParmsNotSaved = false;
                fCurrentJobParmsFile = fileName;
                fStartNewJobFileSave.setEnabled(false);
                fStartNewJobDialog.setTitle(fStartNewJobTitle + "  " +
                                            fileName);

                updateRecentFiles(fileName);
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    public void showStartNewJobDialog()
    {
        if (fStartNewJobDialog != null)
        {
            fStartNewJobDialog.setVisible(true);
        }
    }

    public void createStartNewJobDialog()
    {
        fStartNewJobDialog = new JDialog(this, fStartNewJobTitle, true);

        // this is required for the menu bar to correctly repaint
        fStartNewJobDialog.addWindowListener(new WindowAdapter()
            {
                public void windowActivated(WindowEvent ev)
                {
                    ev.getWindow().repaint();
                }
            }
        );

        JTabbedPane startNewJobTabbedPane = new JTabbedPane();

        JMenuBar mainMenuBar = new JMenuBar();
        fStartNewJobDialog.setJMenuBar(mainMenuBar);
        fStartNewJobFileMenu = new JMenu("File");
        mainMenuBar.add(fStartNewJobFileMenu);

        fStartNewJobFileOpen = new JMenuItem("Open");
        fStartNewJobFileOpen.addActionListener(this);
        fStartNewJobFileMenu.add(fStartNewJobFileOpen);

        fStartNewJobFileSave = new JMenuItem("Save");
        fStartNewJobFileSave.addActionListener(this);
        fStartNewJobFileSave.setEnabled(false);
        fStartNewJobFileMenu.add(fStartNewJobFileSave);

        fStartNewJobFileSaveAs = new JMenuItem("Save As...");
        fStartNewJobFileSaveAs.addActionListener(this);
        fStartNewJobFileMenu.add(fStartNewJobFileSaveAs);

        fStartNewJobFileMenu.insertSeparator(3);

        fStartNewJobFileExit = new JMenuItem("Exit");
        fStartNewJobFileExit.addActionListener(this);
        fStartNewJobFileMenu.add(fStartNewJobFileExit);

        JPanel newJobPanel = new JPanel();
        newJobPanel.setLayout(new BorderLayout());

        JPanel jobInfoPanel = new JPanel();
        jobInfoPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        jobInfoPanel.setBorder(new TitledBorder("Job Information"));
        fJobNameField = new JTextField(15);
        fJobNameField.addKeyListener(this);
        fJobNameField.setText(fJobName);

        JPanel jobNameMonitorPanel = new JPanel();
        jobNameMonitorPanel.setLayout(new
            BoxLayout(jobNameMonitorPanel, BoxLayout.X_AXIS));
        jobNameMonitorPanel.setBorder(new TitledBorder("Job Options"));

        JPanel jobNamePanel = new JPanel();
        jobNamePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        jobNamePanel.add(new JLabel("Job Name: "));
        jobNamePanel.add(fJobNameField);

        JPanel monitorPanel = new JPanel();
        monitorPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        monitorPanel.add(new JLabel("Monitor:"));
        monitorPanel.add(Box.createHorizontalStrut(20));
        fMonitorYesRB = new JRadioButton("Yes", true);
        fMonitorNoRB = new JRadioButton("No", false);
        fMonitorYesRB.addItemListener(this);
        fMonitorNoRB.addItemListener(this);
        ButtonGroup monitorGroup = new ButtonGroup();
        monitorGroup.add(fMonitorYesRB);
        monitorGroup.add(fMonitorNoRB);

        monitorPanel.add(fMonitorYesRB);
        monitorPanel.add(Box.createHorizontalStrut(10));
        monitorPanel.add(fMonitorNoRB);

        monitorPanel.add(Box.createHorizontalStrut(20));

        jobNameMonitorPanel.add(jobNamePanel);
        jobNameMonitorPanel.add(Box.createHorizontalStrut(15));
        jobNameMonitorPanel.add(monitorPanel);

        jobInfoPanel.add(Box.createHorizontalStrut(10));
        jobInfoPanel.add(jobNameMonitorPanel);
        jobInfoPanel.add(Box.createHorizontalStrut(20));

        JPanel functionPanel = new JPanel();
        functionPanel.setLayout(new
            BoxLayout(functionPanel, BoxLayout.Y_AXIS));
        functionPanel.setBorder(new TitledBorder("Start Function"));
        fDefaultFunctionRB = new JRadioButton("default", true);
        fDefaultFunctionRB.addItemListener(this);
        fOtherFunctionRB = new JRadioButton("other", false);
        fOtherFunctionRB.addItemListener(this);
        ButtonGroup functionGroup = new ButtonGroup();
        functionGroup.add(fDefaultFunctionRB);
        functionGroup.add(fOtherFunctionRB);
        fFunctionField = new JTextField(15);
        fFunctionField.addKeyListener(this);
        fFunctionField.setText(fFunction);
        fFunctionField.setEnabled(false);

        JPanel defaultPanel = new JPanel();
        defaultPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        defaultPanel.add(Box.createHorizontalStrut(10));
        defaultPanel.add(fDefaultFunctionRB);

        JPanel otherFunctionPanel = new JPanel();
        otherFunctionPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        otherFunctionPanel.add(Box.createHorizontalStrut(10));
        otherFunctionPanel.add(fOtherFunctionRB);
        otherFunctionPanel.add(Box.createHorizontalStrut(10));
        otherFunctionPanel.add(fFunctionField);

        JPanel argsPanel = new JPanel();
        argsPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        argsPanel.add(Box.createHorizontalStrut(10));
        JPanel argsLeftPanel = new JPanel();
        argsLeftPanel.setLayout(new BorderLayout());
        argsLeftPanel.add(BorderLayout.NORTH, new JLabel("Arguments:"));
        argsLeftPanel.add(BorderLayout.CENTER, fClearArguments);
        fClearArguments.addActionListener(this);
        argsPanel.add(argsLeftPanel);
        fArguments = new JTextArea(5, 70);
        fArguments.setLineWrap(true);
        fArguments.setFont(new Font("Monospaced", Font.PLAIN, 12));
        fArguments.addKeyListener(this);
        argsPanel.add(Box.createHorizontalStrut(10));
        argsPanel.add(fArguments);

        functionPanel.add(defaultPanel);
        functionPanel.add(otherFunctionPanel);
        functionPanel.add(Box.createVerticalStrut(10));
        functionPanel.add(argsPanel);

        jobInfoPanel.add(functionPanel);

        JPanel xmlJobPanel = new JPanel();
        xmlJobPanel.setLayout(new BoxLayout(xmlJobPanel, BoxLayout.Y_AXIS));
        xmlJobPanel.setBorder(new TitledBorder("XML Job File"));
        fMachineLocalRB = new JRadioButton("local machine", true);
        fMachineLocalRB.addItemListener(this);
        fMachineOtherRB = new JRadioButton("other machine", false);
        fMachineOtherRB.addItemListener(this);
        ButtonGroup machineGroup = new ButtonGroup();
        machineGroup.add(fMachineLocalRB);
        machineGroup.add(fMachineOtherRB);

        JPanel localPanel = new JPanel();
        localPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        localPanel.add(Box.createHorizontalStrut(10));
        localPanel.add(fMachineLocalRB);
        localPanel.add(Box.createHorizontalStrut(13));
        localPanel.add(new JLabel("Filename: "));
        fLocalXmlFileNameField = new JTextField(15);
        fLocalXmlFileNameField.addKeyListener(this);
        fLocalXmlFileNameField.setText(fLocalXmlFileName);
        localPanel.add(fLocalXmlFileNameField);
        localPanel.add(Box.createHorizontalStrut(10));
        fBrowseButton = new JButton("Browse...");
        fBrowseButton.addActionListener(this);
        localPanel.add(fBrowseButton);

        fOtherXmlFileMachineField = new JTextField(15);
        fOtherXmlFileMachineField.addKeyListener(this);
        fOtherXmlFileMachineField.setEnabled(false);

        JPanel otherPanel = new JPanel();
        otherPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        otherPanel.add(Box.createHorizontalStrut(10));
        otherPanel.add(fMachineOtherRB);
        otherPanel.add(Box.createHorizontalStrut(10));
        otherPanel.add(new JLabel("Name: "));
        otherPanel.add(fOtherXmlFileMachineField);
        otherPanel.add(Box.createHorizontalStrut(10));
        otherPanel.add(new JLabel("Filename: "));
        fOtherXmlFileNameField = new JTextField(15);
        fOtherXmlFileNameField.addKeyListener(this);
        fOtherXmlFileNameField.setText(fOtherXmlFileName);
        fOtherXmlFileNameField.setEnabled(false);
        otherPanel.add(fOtherXmlFileNameField);
        otherPanel.add(Box.createHorizontalStrut(30));

        xmlJobPanel.add(localPanel);
        xmlJobPanel.add(otherPanel);

        JPanel scriptPanel = new JPanel();
        scriptPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        scriptPanel.setBorder(new TitledBorder("Scripts"));
        scriptPanel.add(Box.createHorizontalStrut(10));

        fScriptList = new JList(fScriptVector);
        fScriptList.setModel(new DefaultListModel());
        fScriptList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        fScriptList.setCellRenderer(new STAXJobListCellRenderer(
            new Font("Monospaced", Font.PLAIN, 12)));
        fScriptList.addMouseListener(this);

        JPanel scriptButtonPanel = new JPanel();
        scriptButtonPanel.setLayout(new
            BoxLayout(scriptButtonPanel, BoxLayout.Y_AXIS));
        fScriptAddButton = new JButton("Add...");
        fScriptAddButton.addActionListener(this);
        fScriptDeleteButton = new JButton("Delete");
        fScriptDeleteButton.addActionListener(this);
        fScriptDeleteAllButton = new JButton("Delete All");
        fScriptDeleteAllButton.addActionListener(this);
        scriptButtonPanel.add(fScriptAddButton);
        scriptButtonPanel.add(Box.createVerticalStrut(5));
        scriptButtonPanel.add(fScriptDeleteButton);
        scriptButtonPanel.add(Box.createVerticalStrut(5));
        scriptButtonPanel.add(fScriptDeleteAllButton);

        JScrollPane fScriptScrollPane = new JScrollPane(fScriptList);
        fScriptScrollPane.setPreferredSize(new Dimension(400, 150));

        scriptPanel.add(fScriptScrollPane);
        scriptPanel.add(Box.createHorizontalStrut(10));
        scriptPanel.add(scriptButtonPanel);

        fAddScriptDialog = new JDialog(fStartNewJobDialog,
                                       "Add Script", true);
        fAddScriptDialog.setSize(new Dimension(400, 200));
        JPanel addScriptPanel = new JPanel();
        addScriptPanel.setLayout(new BorderLayout());
        fAddScriptTextArea = new JTextArea(5, 15);
        fAddScriptTextArea.setBorder(new TitledBorder("Enter script here"));
        addScriptPanel.add(BorderLayout.CENTER,
                           new JScrollPane(fAddScriptTextArea));

        fAddScriptAddButton = new JButton("Add");
        fAddScriptCancelButton = new JButton("Cancel");

        JPanel addScriptButtonPanel = new JPanel();
        addScriptButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addScriptButtonPanel.add(fAddScriptAddButton);
        addScriptButtonPanel.add(Box.createHorizontalStrut(20));
        addScriptButtonPanel.add(fAddScriptCancelButton);

        addScriptPanel.add(BorderLayout.SOUTH, addScriptButtonPanel);

        fAddScriptAddButton.addActionListener(this);
        fAddScriptCancelButton.addActionListener(this);

        fAddScriptDialog.getContentPane().add(addScriptPanel);

        fEditScriptDialog = new JDialog(fStartNewJobDialog,
                                       "Edit Script", true);
        fEditScriptDialog.setSize(new Dimension(400, 200));
        JPanel editScriptPanel = new JPanel();
        editScriptPanel.setLayout(new BorderLayout());
        fEditScriptTextArea = new JTextArea(5, 15);
        fEditScriptTextArea.setBorder(new TitledBorder("Update script here"));
        editScriptPanel.add(BorderLayout.CENTER,
                           new JScrollPane(fEditScriptTextArea));

        fEditScriptSaveButton = new JButton("Save");
        fEditScriptCancelButton = new JButton("Cancel");

        JPanel editScriptButtonPanel = new JPanel();
        editScriptButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        editScriptButtonPanel.add(fEditScriptSaveButton);
        editScriptButtonPanel.add(Box.createHorizontalStrut(20));
        editScriptButtonPanel.add(fEditScriptCancelButton);

        editScriptPanel.add(BorderLayout.SOUTH, editScriptButtonPanel);

        fEditScriptSaveButton.addActionListener(this);
        fEditScriptCancelButton.addActionListener(this);

        fEditScriptDialog.getContentPane().add(editScriptPanel);

        fScriptFilesVector = new Vector();
        JPanel scriptFilesPanel = new JPanel();
        scriptFilesPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        scriptFilesPanel.add(Box.createHorizontalStrut(10));

        fScriptFilesList = new JList(fScriptFilesVector);
        fScriptFilesList.setModel(new DefaultListModel());
        fScriptFilesList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        fScriptFilesList.setCellRenderer(new STAXJobListCellRenderer());
        fScriptFilesList.addMouseListener(this);

        JPanel scriptFilesButtonPanel = new JPanel();
        scriptFilesButtonPanel.setLayout(new
            BoxLayout(scriptFilesButtonPanel, BoxLayout.Y_AXIS));
        fScriptFilesAddButton = new JButton("Add...");
        fScriptFilesAddButton.addActionListener(this);
        fScriptFilesDeleteButton = new JButton("Delete");
        fScriptFilesDeleteButton.addActionListener(this);
        fScriptFilesDeleteAllButton = new JButton("Delete All");
        fScriptFilesDeleteAllButton.addActionListener(this);
        scriptFilesButtonPanel.add(fScriptFilesAddButton);
        scriptFilesButtonPanel.add(Box.createVerticalStrut(5));
        scriptFilesButtonPanel.add(fScriptFilesDeleteButton);
        scriptFilesButtonPanel.add(Box.createVerticalStrut(5));
        scriptFilesButtonPanel.add(fScriptFilesDeleteAllButton);

        JScrollPane fScriptFilesScrollPane = new JScrollPane(fScriptFilesList);
        fScriptFilesScrollPane.setPreferredSize(new Dimension(350,120));

        scriptFilesPanel.add(fScriptFilesScrollPane);
        scriptFilesPanel.add(Box.createHorizontalStrut(10));
        scriptFilesPanel.add(scriptFilesButtonPanel);

        JPanel scriptFilesMachinePanel = new JPanel();
        scriptFilesMachinePanel.setBorder(new TitledBorder("Script Files"));
        scriptFilesMachinePanel.setLayout(new
            BoxLayout(scriptFilesMachinePanel, BoxLayout.Y_AXIS));

        JPanel scriptMachinePanel = new JPanel();
        scriptMachinePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));

        fLocalScriptMachineRB = new JRadioButton("Local machine", false);
        fXMLJobFileScriptMachineRB = new JRadioButton("XML Job File machine",
                                                      true);
        fOtherScriptMachineRB = new JRadioButton("Other machine", false);
        fLocalScriptMachineRB.addItemListener(this);
        fXMLJobFileScriptMachineRB.addItemListener(this);
        fOtherScriptMachineRB.addItemListener(this);

        ButtonGroup scriptMachineGroup = new ButtonGroup();
        scriptMachineGroup.add(fLocalScriptMachineRB);
        scriptMachineGroup.add(fXMLJobFileScriptMachineRB);
        scriptMachineGroup.add(fOtherScriptMachineRB);

        fScriptFilesMachineTextField = new JTextField(20);
        fScriptFilesMachineTextField.setEnabled(false);
        scriptMachinePanel.add(fLocalScriptMachineRB);
        scriptMachinePanel.add(fXMLJobFileScriptMachineRB);
        scriptMachinePanel.add(fOtherScriptMachineRB);
        scriptMachinePanel.add(fScriptFilesMachineTextField);

        scriptFilesMachinePanel.add(Box.createVerticalStrut(10));
        scriptFilesMachinePanel.add(scriptMachinePanel);
        scriptFilesMachinePanel.add(scriptFilesPanel);

        fAddScriptFilesDialog = new JDialog(this,
            "Add Script File", true);
        fAddScriptFilesDialog.setSize(new Dimension(400, 115));
        JPanel addScriptFilesPanel = new JPanel();
        addScriptFilesPanel.setLayout(new BorderLayout());
        fAddScriptFilesTextField = new JTextField(15);
        fAddScriptFilesTextField.setBorder(new TitledBorder(
            "Enter script file name here"));
        addScriptFilesPanel.add(BorderLayout.CENTER,
            new JScrollPane(fAddScriptFilesTextField));

        fAddScriptFilesBrowseButton = new JButton("Browse...");
        fAddScriptFilesAddButton = new JButton("Add");
        fAddScriptFilesCancelButton = new JButton("Cancel");

        JPanel addScriptFilesButtonPanel = new JPanel();
        addScriptFilesButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addScriptFilesButtonPanel.add(fAddScriptFilesBrowseButton);
        addScriptFilesButtonPanel.add(Box.createHorizontalStrut(20));
        addScriptFilesButtonPanel.add(fAddScriptFilesAddButton);
        addScriptFilesButtonPanel.add(Box.createHorizontalStrut(20));
        addScriptFilesButtonPanel.add(fAddScriptFilesCancelButton);

        addScriptFilesPanel.add(BorderLayout.SOUTH, addScriptFilesButtonPanel);

        fAddScriptFilesAddButton.addActionListener(this);
        fAddScriptFilesCancelButton.addActionListener(this);
        fAddScriptFilesBrowseButton.addActionListener(this);

        fAddScriptFilesDialog.getContentPane().add(addScriptFilesPanel);

        fEditScriptFilesDialog = new JDialog(this,
           "Edit Script File", true);
        fEditScriptFilesDialog.setSize(new Dimension(400, 115));
        JPanel editScriptFilesPanel = new JPanel();
        editScriptFilesPanel.setLayout(new BorderLayout());
        fEditScriptFilesTextField = new JTextField(15);
        fEditScriptFilesTextField.setBorder(new TitledBorder(
            "Update script file name here"));
        editScriptFilesPanel.add(BorderLayout.CENTER,
            new JScrollPane(fEditScriptFilesTextField));

        fEditScriptFilesSaveButton = new JButton("Save");
        fEditScriptFilesCancelButton = new JButton("Cancel");

        JPanel editScriptFilesButtonPanel = new JPanel();
        editScriptFilesButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        editScriptFilesButtonPanel.add(fEditScriptFilesSaveButton);
        editScriptFilesButtonPanel.add(Box.createHorizontalStrut(20));
        editScriptFilesButtonPanel.add(fEditScriptFilesCancelButton);

        editScriptFilesPanel.add(BorderLayout.SOUTH,
            editScriptFilesButtonPanel);

        fEditScriptFilesSaveButton.addActionListener(this);
        fEditScriptFilesCancelButton.addActionListener(this);

        fEditScriptFilesDialog.getContentPane().add(editScriptFilesPanel);

        // Beginning of "Log Options" Tab Panel

        JPanel logOptionsPanel = new JPanel();
        logOptionsPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        logOptionsPanel.setBorder(new TitledBorder("Log Options"));

        JPanel logOptionsSubPanel = new JPanel();
        logOptionsSubPanel.setLayout(new
            BoxLayout(logOptionsSubPanel, BoxLayout.Y_AXIS));

        JPanel clearLogsPanel = new JPanel();
        clearLogsPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));

        clearLogsPanel.add(new JLabel("Clear Logs                   :"));
        clearLogsPanel.add(Box.createHorizontalStrut(20));
        fClearLogsYesRB = new JRadioButton("Enabled", false);
        fClearLogsYesRB.setBackground(Color.white);
        fClearLogsNoRB = new JRadioButton("Disabled", false);
        fClearLogsNoRB.setBackground(Color.white);
        fClearLogsDefaultRB = new JRadioButton("Default", true);
        fClearLogsDefaultRB.setBackground(Color.white);
        fClearLogsYesRB.addItemListener(this);
        fClearLogsNoRB.addItemListener(this);
        fClearLogsDefaultRB.addItemListener(this);
        ButtonGroup clearLogsGroup = new ButtonGroup();
        clearLogsGroup.add(fClearLogsYesRB);
        clearLogsGroup.add(fClearLogsNoRB);
        clearLogsGroup.add(fClearLogsDefaultRB);
        clearLogsPanel.add(fClearLogsYesRB);
        clearLogsPanel.add(Box.createHorizontalStrut(10));
        clearLogsPanel.add(fClearLogsNoRB);
        clearLogsPanel.add(Box.createHorizontalStrut(10));
        clearLogsPanel.add(fClearLogsDefaultRB);

        JPanel logTCElapsedTimePanel = new JPanel();
        logTCElapsedTimePanel.setLayout(new BoxLayout(logTCElapsedTimePanel,
                                                      BoxLayout.X_AXIS));
        logTCElapsedTimePanel.add(new JLabel("Log TC Elapsed Time:"));
        logTCElapsedTimePanel.add(Box.createHorizontalStrut(20));
        fLogTCElapsedTimeYesRB = new JRadioButton("Enabled", false);
        fLogTCElapsedTimeYesRB.setBackground(Color.white);
        fLogTCElapsedTimeNoRB = new JRadioButton("Disabled", false);
        fLogTCElapsedTimeNoRB.setBackground(Color.white);
        fLogTCElapsedTimeDefaultRB = new JRadioButton("Default", true);
        fLogTCElapsedTimeDefaultRB.setBackground(Color.white);
        fLogTCElapsedTimeYesRB.addItemListener(this);
        fLogTCElapsedTimeNoRB.addItemListener(this);
        fLogTCElapsedTimeDefaultRB.addItemListener(this);
        ButtonGroup logTCElapsedTimeGroup = new ButtonGroup();
        logTCElapsedTimeGroup.add(fLogTCElapsedTimeYesRB);
        logTCElapsedTimeGroup.add(fLogTCElapsedTimeNoRB);
        logTCElapsedTimeGroup.add(fLogTCElapsedTimeDefaultRB);
        logTCElapsedTimePanel.add(fLogTCElapsedTimeYesRB);
        logTCElapsedTimePanel.add(Box.createHorizontalStrut(10));
        logTCElapsedTimePanel.add(fLogTCElapsedTimeNoRB);
        logTCElapsedTimePanel.add(Box.createHorizontalStrut(10));
        logTCElapsedTimePanel.add(fLogTCElapsedTimeDefaultRB);

        JPanel logTCNumStartsPanel = new JPanel();
        logTCNumStartsPanel.setLayout(new BoxLayout(logTCNumStartsPanel,
                                                    BoxLayout.X_AXIS));
        logTCNumStartsPanel.add(new JLabel("Log TC Num Starts    :"));
        logTCNumStartsPanel.add(Box.createHorizontalStrut(20));
        fLogTCNumStartsYesRB = new JRadioButton("Enabled", false);
        fLogTCNumStartsYesRB.setBackground(Color.white);
        fLogTCNumStartsNoRB = new JRadioButton("Disabled", false);
        fLogTCNumStartsNoRB.setBackground(Color.white);
        fLogTCNumStartsDefaultRB = new JRadioButton("Default", true);
        fLogTCNumStartsDefaultRB.setBackground(Color.white);
        fLogTCNumStartsYesRB.addItemListener(this);
        fLogTCNumStartsNoRB.addItemListener(this);
        fLogTCNumStartsDefaultRB.addItemListener(this);
        ButtonGroup logTCNumStartsGroup = new ButtonGroup();
        logTCNumStartsGroup.add(fLogTCNumStartsYesRB);
        logTCNumStartsGroup.add(fLogTCNumStartsNoRB);
        logTCNumStartsGroup.add(fLogTCNumStartsDefaultRB);
        logTCNumStartsPanel.add(fLogTCNumStartsYesRB);
        logTCNumStartsPanel.add(Box.createHorizontalStrut(10));
        logTCNumStartsPanel.add(fLogTCNumStartsNoRB);
        logTCNumStartsPanel.add(Box.createHorizontalStrut(10));
        logTCNumStartsPanel.add(fLogTCNumStartsDefaultRB);

        JPanel logTCStartStopPanel = new JPanel();
        logTCStartStopPanel.setLayout(new BoxLayout(logTCStartStopPanel,
                                                    BoxLayout.X_AXIS));
        logTCStartStopPanel.add(new JLabel("Log TC Start/Stop      :"));
        logTCStartStopPanel.add(Box.createHorizontalStrut(20));
        fLogTCStartStopYesRB = new JRadioButton("Enabled", false);
        fLogTCStartStopYesRB.setBackground(Color.white);
        fLogTCStartStopNoRB = new JRadioButton("Disabled", false);
        fLogTCStartStopNoRB.setBackground(Color.white);
        fLogTCStartStopDefaultRB = new JRadioButton("Default", true);
        fLogTCStartStopDefaultRB.setBackground(Color.white);
        fLogTCStartStopYesRB.addItemListener(this);
        fLogTCStartStopNoRB.addItemListener(this);
        fLogTCStartStopDefaultRB.addItemListener(this);
        ButtonGroup logTCStartStopGroup = new ButtonGroup();
        logTCStartStopGroup.add(fLogTCStartStopYesRB);
        logTCStartStopGroup.add(fLogTCStartStopNoRB);
        logTCStartStopGroup.add(fLogTCStartStopDefaultRB);
        logTCStartStopPanel.add(fLogTCStartStopYesRB);
        logTCStartStopPanel.add(Box.createHorizontalStrut(10));
        logTCStartStopPanel.add(fLogTCStartStopNoRB);
        logTCStartStopPanel.add(Box.createHorizontalStrut(10));
        logTCStartStopPanel.add(fLogTCStartStopDefaultRB);

        fPythonOutputCB = new JComboBox();

        // Add "Default" as the first item in the Python Output combo box,
        // as the first item is the default selection if no selection is made
        fPythonOutputCB.addItem(DEFAULT_STRING);

        // Add all valid python outputs to the Python Output combobox
        for (int i = 0; i < PYTHON_OUTPUTS_PRETTY.length; i++)
        {
            fPythonOutputCB.addItem(PYTHON_OUTPUTS_PRETTY[i]);
        }

        JPanel pythonOutputPanel = new JPanel();
        pythonOutputPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        pythonOutputPanel.add(new JLabel("Python Output            :"));
        pythonOutputPanel.add(Box.createHorizontalStrut(20));
        fPythonOutputCB.setBackground(Color.white);
        fPythonOutputCB.addActionListener(this);
        pythonOutputPanel.add(fPythonOutputCB);

        fPythonLogLevelCB = new JComboBox();

        // Add "Default" as the first item in the Python Log Level combo box,
        // as the first item is the default selection if no selection is made
        fPythonLogLevelCB.addItem(DEFAULT_STRING);

        // Add all valid log levels to the Python Log Level combo box
        for (int i = 0; i < LOGLEVELS.length; i++)
        {
            fPythonLogLevelCB.addItem(LOGLEVELS[i]);
        }
        
        JPanel pythonLogLevelPanel = new JPanel();
        pythonLogLevelPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        pythonLogLevelPanel.add(new JLabel("Python Log Level       :"));
        pythonLogLevelPanel.add(Box.createHorizontalStrut(20));
        fPythonLogLevelCB.setBackground(Color.white);
        fPythonLogLevelCB.addActionListener(this);
        pythonLogLevelPanel.add(fPythonLogLevelCB);
        
        // Add the various panels to the logOptionsSubPanel

        logOptionsSubPanel.add(Box.createHorizontalStrut(15));
        logOptionsSubPanel.add(clearLogsPanel);
        logOptionsSubPanel.add(logTCElapsedTimePanel);
        logOptionsSubPanel.add(logTCNumStartsPanel);
        logOptionsSubPanel.add(logTCStartStopPanel);
        logOptionsSubPanel.add(Box.createVerticalStrut(20));
        logOptionsSubPanel.add(pythonOutputPanel);
        logOptionsSubPanel.add(pythonLogLevelPanel);
        
        logOptionsPanel.add(Box.createHorizontalStrut(10));
        logOptionsPanel.add(logOptionsSubPanel);
        logOptionsPanel.add(Box.createHorizontalStrut(20));

        // End of "Log Options" Tab Panel

        // Beginning of "Breakpoints" Tab Panel

        JPanel breakpointsPanel = new JPanel();
        breakpointsPanel.setLayout(new BorderLayout());
        breakpointsPanel.setBorder(new TitledBorder("Breakpoints"));

        fBreakpointFirstFunctionCB = new JCheckBox(
            "Set breakpoint on first function", false);

        fBreakpointSubjobFirstFunctionCB = new JCheckBox(
            "Set breakpoint on subjob first function", false);

        fBreakpointsTableModel =
            new STAXMonitorTableModel(fBreakpointTableColumnNames, 0);
        fBreakpointsModelSorter =
            new STAXMonitorTableSorter(fBreakpointsTableModel);

        fBreakpointTable = new JTable(fBreakpointsModelSorter);
        fBreakpointTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        fBreakpointTable.setSelectionMode(
            ListSelectionModel.SINGLE_SELECTION);
        fBreakpointTable.setRowSelectionAllowed(true);
        fBreakpointTable.addMouseListener(this);

        for (int i = 0; i < fBreakpointTableColumnNames.size(); i++)
        {
            fBreakpointTable.getColumnModel().getColumn(i).
                setCellRenderer(new STAXMonitorTableCellRenderer(
                Color.black, false, new Font("Dialog", Font.PLAIN, 12)));

            fBreakpointTable.getColumnModel().getColumn(i).
                setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true, new Font("Dialog", Font.BOLD, 12)));
        }

        TableColumn column = null;

        for (int i = 0; i < 4; i++) {
            column = fBreakpointTable.getColumnModel().getColumn(i);
            if (i == 1) {
                column.setPreferredWidth(15);
            } else {
                column.setPreferredWidth(125);
            }
        }

        fBreakpointTable.updateUI();
        STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
        STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);

        JPanel breakpointLinesPanel = new JPanel();
        breakpointLinesPanel.setLayout(new
            FlowLayout(FlowLayout.LEFT, 0, 0));
        breakpointLinesPanel.add(Box.createHorizontalStrut(10));

        JPanel breakpointLinesButtonPanel = new JPanel();
        breakpointLinesButtonPanel.setLayout(new
            BoxLayout(breakpointLinesButtonPanel, BoxLayout.Y_AXIS));
        fBreakpointLineAddButton = new JButton("Add Breakpoint Line...");
        fBreakpointLineAddButton.addActionListener(this);
        fBreakpointFunctionAddButton = new JButton("Add Breakpoint Function...");
        fBreakpointFunctionAddButton.addActionListener(this);
        fBreakpointDeleteButton = new JButton("Delete");
        fBreakpointDeleteButton.addActionListener(this);
        fBreakpointDeleteAllButton = new JButton("Delete All");
        fBreakpointDeleteAllButton.addActionListener(this);
        breakpointLinesButtonPanel.add(fBreakpointLineAddButton);
        breakpointLinesButtonPanel.add(Box.createVerticalStrut(5));
        breakpointLinesButtonPanel.add(fBreakpointFunctionAddButton);
        breakpointLinesButtonPanel.add(Box.createVerticalStrut(5));
        breakpointLinesButtonPanel.add(fBreakpointDeleteButton);
        breakpointLinesButtonPanel.add(Box.createVerticalStrut(5));
        breakpointLinesButtonPanel.add(fBreakpointDeleteAllButton);

        JScrollPane fBreakpointLinesScrollPane =
            new JScrollPane(fBreakpointTable);
        fBreakpointLinesScrollPane.setPreferredSize(
            new Dimension(400, 130));

        breakpointLinesPanel.add(fBreakpointLinesScrollPane);
        breakpointLinesPanel.add(Box.createHorizontalStrut(10));
        breakpointLinesPanel.add(breakpointLinesButtonPanel);

        fAddBreakpointLineDialog = new JDialog(fStartNewJobDialog,
            "Add Line Breakpoint", true);
        fAddBreakpointLineDialog.setSize(new Dimension(400, 165));

        JPanel addBreakpointLinePanel = new JPanel();
        addBreakpointLinePanel.setLayout(new BorderLayout());

        JPanel breakpointLineOptionsPanel = new JPanel();
        breakpointLineOptionsPanel.setBorder(
            new TitledBorder("Breakpoint Line options"));
        breakpointLineOptionsPanel.setLayout(
            new BoxLayout(breakpointLineOptionsPanel, BoxLayout.Y_AXIS));

        JPanel addBreakpointLineNumberPanel = new JPanel();
        addBreakpointLineNumberPanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel breakpointLineNumberName = new JLabel(" Line Number:    ");
        fAddBreakpointLineNumberTextField = new JTextField(15)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fAddBreakpointLineNumberTextField.setToolTipText(
            STAXMonitorUtil.wrapText(BREAKPOINT_LINE_NUMBER_TOOLTIP, 80));

        fAddBreakpointLineNumberTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fAddBreakpointLineNumberTextField.setBackground(Color.white);
        fAddBreakpointLineNumberTextField.addActionListener(this);
        addBreakpointLineNumberPanel.add(breakpointLineNumberName);
        addBreakpointLineNumberPanel.add(fAddBreakpointLineNumberTextField);

        JPanel addBreakpointLineFilePanel = new JPanel();
        addBreakpointLineFilePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel breakpointLineFileName = new JLabel(" XML File:            ");
        fAddBreakpointLineFileTextField = new JTextField(25)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fAddBreakpointLineFileTextField.setToolTipText(
            STAXMonitorUtil.wrapText(BREAKPOINT_LINE_FILE_TOOLTIP, 80));

        fAddBreakpointLineFileTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fAddBreakpointLineFileTextField.setBackground(Color.white);
        fAddBreakpointLineFileTextField.addActionListener(this);
        addBreakpointLineFilePanel.add(breakpointLineFileName);
        addBreakpointLineFilePanel.add(fAddBreakpointLineFileTextField);

        JPanel addBreakpointLineMachinePanel = new JPanel();
        addBreakpointLineMachinePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel breakpointLineMachineName = new JLabel(" Machine:            ");
        fAddBreakpointLineMachineTextField = new JTextField(25)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fAddBreakpointLineMachineTextField.setToolTipText(
            STAXMonitorUtil.wrapText(BREAKPOINT_LINE_MACHINE_TOOLTIP, 80));

        fAddBreakpointLineMachineTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fAddBreakpointLineMachineTextField.setBackground(Color.white);
        fAddBreakpointLineMachineTextField.addActionListener(this);
        addBreakpointLineMachinePanel.add(breakpointLineMachineName);
        addBreakpointLineMachinePanel.add(fAddBreakpointLineMachineTextField);

        fAddBreakpointLineAddButton = new JButton("Add");
        fAddBreakpointLineCancelButton = new JButton("Cancel");

        JPanel addBreakpointLineButtonPanel = new JPanel();
        addBreakpointLineButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addBreakpointLineButtonPanel.add(fAddBreakpointLineAddButton);
        addBreakpointLineButtonPanel.add(Box.createHorizontalStrut(20));
        addBreakpointLineButtonPanel.add(
            fAddBreakpointLineCancelButton);

        breakpointLineOptionsPanel.add(addBreakpointLineNumberPanel);
        breakpointLineOptionsPanel.add(addBreakpointLineFilePanel);
        breakpointLineOptionsPanel.add(addBreakpointLineMachinePanel);

        addBreakpointLinePanel.add(BorderLayout.CENTER,
            breakpointLineOptionsPanel);
        addBreakpointLinePanel.add(BorderLayout.SOUTH,
            addBreakpointLineButtonPanel);

        fAddBreakpointLineAddButton.addActionListener(this);
        fAddBreakpointLineCancelButton.addActionListener(this);

        fAddBreakpointLineDialog.getContentPane().add(
            addBreakpointLinePanel);

        fEditBreakpointLineDialog = new JDialog(fStartNewJobDialog,
            "Edit Line Breakpoint", true);
        fEditBreakpointLineDialog.setSize(new Dimension(400, 165));

        JPanel editBreakpointLinePanel = new JPanel();
        editBreakpointLinePanel.setLayout(new BorderLayout());

        JPanel editBreakpointLineOptionsPanel = new JPanel();
        editBreakpointLineOptionsPanel.setBorder(
            new TitledBorder("Breakpoint Line options"));
        editBreakpointLineOptionsPanel.setLayout(
            new BoxLayout(editBreakpointLineOptionsPanel, BoxLayout.Y_AXIS));

        JPanel editBreakpointLineNumberPanel = new JPanel();
        editBreakpointLineNumberPanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel editBreakpointLineNumberName = new JLabel(" Line Number:    ");
        fEditBreakpointLineNumberTextField = new JTextField(15)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fEditBreakpointLineNumberTextField.setToolTipText(
            STAXMonitorUtil.wrapText(BREAKPOINT_LINE_NUMBER_TOOLTIP, 80));

        fEditBreakpointLineNumberTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fEditBreakpointLineNumberTextField.setBackground(Color.white);
        fEditBreakpointLineNumberTextField.addActionListener(this);
        editBreakpointLineNumberPanel.add(editBreakpointLineNumberName);
        editBreakpointLineNumberPanel.add(fEditBreakpointLineNumberTextField);

        JPanel editBreakpointLineFilePanel = new JPanel();
        editBreakpointLineFilePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel editBreakpointLineFileName =
            new JLabel(" XML File:            ");
        fEditBreakpointLineFileTextField = new JTextField(25)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fEditBreakpointLineFileTextField.setToolTipText(
            STAXMonitorUtil.wrapText(BREAKPOINT_LINE_FILE_TOOLTIP, 80));

        fEditBreakpointLineFileTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fEditBreakpointLineFileTextField.setBackground(Color.white);
        fEditBreakpointLineFileTextField.addActionListener(this);
        editBreakpointLineFilePanel.add(editBreakpointLineFileName);
        editBreakpointLineFilePanel.add(fEditBreakpointLineFileTextField);

        JPanel editBreakpointLineMachinePanel = new JPanel();
        editBreakpointLineMachinePanel.setLayout(
            new FlowLayout(FlowLayout.LEFT, 0, 0));
        JLabel editBreakpointLineMachineName =
            new JLabel(" Machine:            ");
        fEditBreakpointLineMachineTextField = new JTextField(25)
            {
                public JToolTip createToolTip()
                {
                    MultiLineToolTip tip = new MultiLineToolTip();
                    tip.setComponent(this);
                    return tip;
                }
            };

        fEditBreakpointLineMachineTextField.setToolTipText(
            STAXMonitorUtil.wrapText(BREAKPOINT_LINE_MACHINE_TOOLTIP, 80));

        fEditBreakpointLineMachineTextField.setFont(
            new Font("Dialog", Font.PLAIN, 12));
        fEditBreakpointLineMachineTextField.setBackground(Color.white);
        fEditBreakpointLineMachineTextField.addActionListener(this);
        editBreakpointLineMachinePanel.add(editBreakpointLineMachineName);
        editBreakpointLineMachinePanel.add(fEditBreakpointLineMachineTextField);

        fEditBreakpointLineSaveButton = new JButton("Save");
        fEditBreakpointLineCancelButton = new JButton("Cancel");

        JPanel editBreakpointLineButtonPanel = new JPanel();
        editBreakpointLineButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        editBreakpointLineButtonPanel.add(fEditBreakpointLineSaveButton);
        editBreakpointLineButtonPanel.add(Box.createHorizontalStrut(20));
        editBreakpointLineButtonPanel.add(
            fEditBreakpointLineCancelButton);

        editBreakpointLineOptionsPanel.add(editBreakpointLineNumberPanel);
        editBreakpointLineOptionsPanel.add(editBreakpointLineFilePanel);
        editBreakpointLineOptionsPanel.add(editBreakpointLineMachinePanel);

        editBreakpointLinePanel.add(BorderLayout.CENTER,
            editBreakpointLineOptionsPanel);
        editBreakpointLinePanel.add(BorderLayout.SOUTH,
            editBreakpointLineButtonPanel);

        fEditBreakpointLineSaveButton.addActionListener(this);
        fEditBreakpointLineCancelButton.addActionListener(this);

        fEditBreakpointLineDialog.getContentPane().add(
            editBreakpointLinePanel);

        JPanel breakpointFunctionsPanel = new JPanel();
        breakpointFunctionsPanel.setLayout(new
            FlowLayout(FlowLayout.LEFT, 0, 0));
        breakpointFunctionsPanel.add(Box.createHorizontalStrut(10));

        JPanel breakpointFunctionsButtonPanel = new JPanel();
        breakpointFunctionsButtonPanel.setLayout(new
            BoxLayout(breakpointFunctionsButtonPanel, BoxLayout.Y_AXIS));
        fBreakpointFunctionDeleteButton = new JButton("Delete");
        fBreakpointFunctionDeleteButton.addActionListener(this);
        fBreakpointFunctionDeleteAllButton = new JButton("Delete All");
        fBreakpointFunctionDeleteAllButton.addActionListener(this);
        breakpointFunctionsButtonPanel.add(fBreakpointFunctionDeleteButton);
        breakpointFunctionsButtonPanel.add(Box.createVerticalStrut(5));
        breakpointFunctionsButtonPanel.add(fBreakpointFunctionDeleteAllButton);

        fAddBreakpointFunctionDialog = new JDialog(fStartNewJobDialog,
            "Add Function Breakpoint", true);
        fAddBreakpointFunctionDialog.setSize(new Dimension(400, 125));
        JPanel addBreakpointFunctionPanel = new JPanel();
        addBreakpointFunctionPanel.setLayout(new BorderLayout());
        fAddBreakpointFunctionTextField = new JTextField(15);
        fAddBreakpointFunctionTextField.setBorder(
            new TitledBorder("Enter function name here"));
        addBreakpointFunctionPanel.add(BorderLayout.CENTER,
                           new JScrollPane(fAddBreakpointFunctionTextField));

        fAddBreakpointFunctionAddButton = new JButton("Add");
        fAddBreakpointFunctionCancelButton = new JButton("Cancel");

        JPanel addBreakpointFunctionButtonPanel = new JPanel();
        addBreakpointFunctionButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        addBreakpointFunctionButtonPanel.add(fAddBreakpointFunctionAddButton);
        addBreakpointFunctionButtonPanel.add(Box.createHorizontalStrut(20));
        addBreakpointFunctionButtonPanel.add(
            fAddBreakpointFunctionCancelButton);

        addBreakpointFunctionPanel.add(BorderLayout.SOUTH,
            addBreakpointFunctionButtonPanel);

        fAddBreakpointFunctionAddButton.addActionListener(this);
        fAddBreakpointFunctionCancelButton.addActionListener(this);

        fAddBreakpointFunctionDialog.getContentPane().add(
            addBreakpointFunctionPanel);

        fEditBreakpointFunctionDialog = new JDialog(fStartNewJobDialog,
            "Edit Function Name", true);
        fEditBreakpointFunctionDialog.setSize(new Dimension(400, 125));
        JPanel editBreakpointFunctionPanel = new JPanel();
        editBreakpointFunctionPanel.setLayout(new BorderLayout());
        fEditBreakpointFunctionTextField = new JTextField(15);
        fEditBreakpointFunctionTextField.setBorder(new
            TitledBorder("Update function name here"));
        editBreakpointFunctionPanel.add(BorderLayout.CENTER,
                           new JScrollPane(fEditBreakpointFunctionTextField));

        fEditBreakpointFunctionSaveButton = new JButton("Save");
        fEditBreakpointFunctionCancelButton = new JButton("Cancel");

        JPanel editBreakpointFunctionButtonPanel = new JPanel();
        editBreakpointFunctionButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        editBreakpointFunctionButtonPanel.add(
            fEditBreakpointFunctionSaveButton);
        editBreakpointFunctionButtonPanel.add(Box.createHorizontalStrut(20));
        editBreakpointFunctionButtonPanel.add(
            fEditBreakpointFunctionCancelButton);

        editBreakpointFunctionPanel.add(BorderLayout.SOUTH,
            editBreakpointFunctionButtonPanel);

        fEditBreakpointFunctionSaveButton.addActionListener(this);
        fEditBreakpointFunctionCancelButton.addActionListener(this);

        fEditBreakpointFunctionDialog.getContentPane().add(
            editBreakpointFunctionPanel);

        //breakpointTabbedPane.addTab("Line Number",
        //                            breakpointLinesPanel);
        //breakpointTabbedPane.addTab("Function",
        //                            breakpointFunctionsPanel);

        JPanel breakpointFirstFunctionPanel = new JPanel();
        breakpointFirstFunctionPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        breakpointFirstFunctionPanel.add(fBreakpointFirstFunctionCB);
        breakpointFirstFunctionPanel.add(fBreakpointSubjobFirstFunctionCB);

        breakpointsPanel.add(breakpointFirstFunctionPanel, BorderLayout.NORTH);
        breakpointsPanel.add(new JScrollPane(breakpointLinesPanel),
                             BorderLayout.CENTER);

        // End of "Breakpoints" Tab Panel

        fStartNewJobSubmitButton = new JButton("Submit New Job");
        fStartNewJobTestButton = new JButton("Test");
        fStartNewJobWizardButton = new JButton("Job Wizard...");
        fStartNewJobCancelButton = new JButton("Close");
        fStartNewJobClearButton = new JButton("Clear all Parameters");

        JPanel startNewJobButtonPanel = new JPanel();
        startNewJobButtonPanel.setLayout(new
            FlowLayout(FlowLayout.CENTER, 0, 0));
        startNewJobButtonPanel.add(fStartNewJobSubmitButton);
        startNewJobButtonPanel.add(Box.createHorizontalStrut(20));
        startNewJobButtonPanel.add(fStartNewJobTestButton);
        startNewJobButtonPanel.add(Box.createHorizontalStrut(20));
        startNewJobButtonPanel.add(fStartNewJobWizardButton);
        startNewJobButtonPanel.add(Box.createHorizontalStrut(20));
        startNewJobButtonPanel.add(fStartNewJobCancelButton);
        startNewJobButtonPanel.add(Box.createHorizontalStrut(40));
        startNewJobButtonPanel.add(fStartNewJobClearButton);

        fStartNewJobSubmitButton.addActionListener(this);
        fStartNewJobTestButton.addActionListener(this);
        fStartNewJobWizardButton.addActionListener(this);
        fStartNewJobCancelButton.addActionListener(this);
        fStartNewJobClearButton.addActionListener(this);

        loadRecentFiles();

        JPanel jobInfoTab = new JPanel();
        jobInfoTab.setLayout(new BoxLayout(jobInfoTab, BoxLayout.Y_AXIS));
        jobInfoTab.add(xmlJobPanel);
        jobInfoTab.add(jobNameMonitorPanel);

        startNewJobTabbedPane.addTab("Job Info", new JScrollPane(jobInfoTab));
        startNewJobTabbedPane.addTab("Function", new JScrollPane(functionPanel));
        startNewJobTabbedPane.addTab("Scripts", new JScrollPane(scriptPanel));
        startNewJobTabbedPane.addTab("Script Files", new JScrollPane(scriptFilesMachinePanel));
        startNewJobTabbedPane.addTab("Log Options", new JScrollPane(logOptionsPanel));
        startNewJobTabbedPane.addTab("Breakpoints", new JScrollPane(breakpointsPanel));

        // Set tab colors
        for (int i = 0; i < startNewJobTabbedPane.getTabCount(); i++)
        {
            if (i == 0)
            {
                startNewJobTabbedPane.setBackgroundAt(i, Color.lightGray);
            }
            else
            {
                startNewJobTabbedPane.setBackgroundAt(i, Color.white);
                startNewJobTabbedPane.setForegroundAt(i, Color.darkGray);
            }
        }

        newJobPanel.add(BorderLayout.CENTER,
            startNewJobTabbedPane);
        newJobPanel.add(BorderLayout.SOUTH, startNewJobButtonPanel);

        fStartNewJobDialog.setSize(new Dimension(650, 325));
        fStartNewJobDialog.getContentPane().
            add(newJobPanel);
        fStartNewJobDialog.setLocationRelativeTo(this);

        startNewJobTabbedPane.addChangeListener(this);
    }

    public void stateChanged(ChangeEvent e)
    {
        JTabbedPane pane = (JTabbedPane)e.getSource();

        for (int i = 0; i < pane.getTabCount(); i++)
        {
            pane.setBackgroundAt(i, Color.white);
            pane.setForegroundAt(i, Color.darkGray);
        }

        int selected = ((JTabbedPane)e.getSource()).getSelectedIndex();

        if (selected != -1)
        {
            pane.setForegroundAt(selected, Color.black);
        }
    }
    
    public void updateProperties(String newStaxMachineName,
                                 String newStaxServiceName,
                                 String newEventMachineName,
                                 String newEventServiceName,
                                 String newJobParmsDirectory,
                                 boolean displayMessage)
    {
        // Default values for properties if properties file doesn't exist

        String staxMachineName = "local";
        String staxServiceName = "STAX";
        String eventMachineName = "local";
        String eventServiceName = "Event";
        Boolean showNoSTAXMonitorInformation =
            fDefaultShowNoSTAXMonitorInformation;
        Boolean limitMessages = fDefaultLimitMessages;
        String limitMessagesField = fDefaultLimitMessagesText;
        Integer autoMonitorSubjobs = fDefaultAutoMonitorSubjobs;
        Vector localExtJarFiles = new Vector();
        String processMonitorSeconds = fDefaultProcessMonitorSeconds; 
        String elapsedTimeSeconds = fDefaultElapsedTimeSeconds;
        String messageFontName = fDefaultMessageFontName;
        String logViewerFontName = fDefaultLogViewerFontName;
        String saveAsDirectory = null;

        // Get the properties from the properties file (if it exists)

        try
        {
            File propertiesFile = new File(fPropertiesFileName);

            if (propertiesFile.exists())
            {
                ObjectInputStream ois = 
                    new ObjectInputStream(new FileInputStream(propertiesFile));

                Hashtable propertiesData = (Hashtable)(ois.readObject());
                ois.close();                        
            
                staxMachineName = (String)propertiesData.get(
                    "staxMachineName");
                staxServiceName = (String)propertiesData.get(
                    "staxServiceName");
                eventMachineName = (String)propertiesData.get(
                    "eventMachineName");
                eventServiceName = (String)propertiesData.get(
                    "eventServiceName");
                showNoSTAXMonitorInformation = (Boolean)propertiesData.get(
                    "showNoSTAXMonitorInformation");
                limitMessages = (Boolean)propertiesData.get("limitMessages");
                limitMessagesField = (String)propertiesData.get(
                    "limitMessagesField");

                localExtJarFiles = (Vector)propertiesData.get("pluginJars");

                // This key was changed from processMonitorSeconds to
                // processMonitorSeconds-1 in STAX V3.1.2, in order to pick
                // up the new default value of 60 seconds
                processMonitorSeconds = (String)propertiesData.get(
                    "processMonitorSeconds-1");
                elapsedTimeSeconds = (String)propertiesData.get(
                    "elapsedTimeSeconds");
                
                try 
                {
                    autoMonitorSubjobs = 
                        (Integer)propertiesData.get("autoMonitorSubjobs");
                }
                catch (ClassCastException ex)
                {
                    // Do nothing since default is already set
                }

                if (propertiesData.get("messageFontName") != null)
                {
                    messageFontName = (String)propertiesData.get(
                        "messageFontName");
                }
                
                if (propertiesData.get("logViewerFontName") != null)
                {
                    logViewerFontName = (String)propertiesData.get(
                        "logViewerFontName");
                }

                // Log Viewer Save As Directory (Added in V3.3.5)

                if (propertiesData.get("saveAsDirectory") != null)
                {
                    saveAsDirectory = (String)propertiesData.get(
                        "saveAsDirectory");
                }
            }
        }
        catch (IOException e) 
        {
            e.printStackTrace();
            return;
        }
        catch (ClassNotFoundException e) 
        {
            e.printStackTrace();
            return;
        }

        // If the STAX Machine/Service names change, update them

        if (!newStaxMachineName.equals(""))
            staxMachineName = newStaxMachineName;
        
        if (!newStaxServiceName.equals(""))
            staxServiceName = newStaxServiceName;
        
        // Get event machine and service names based by submitting a
        // LIST SETTINGS request to the STAX service machine

        EventServiceInfo eventServiceInfo = new EventServiceInfo(
            staxMachineName, staxServiceName, this, displayMessage);

        eventMachineName = eventServiceInfo.getMachine();
        eventServiceName = eventServiceInfo.getService();

        // Write the new properties to the properties file

        Hashtable propertiesData = new Hashtable();
        propertiesData.put("staxMachineName", staxMachineName);
        propertiesData.put("staxServiceName", staxServiceName);
        propertiesData.put("eventMachineName", eventMachineName);
        propertiesData.put("eventServiceName", eventServiceName);
        propertiesData.put("showNoSTAXMonitorInformation", 
            showNoSTAXMonitorInformation);
        propertiesData.put("limitMessages", limitMessages); 
        propertiesData.put("limitMessagesField", limitMessagesField);
        propertiesData.put("pluginJars", localExtJarFiles);
        // This key was changed from processMonitorSeconds to
        // processMonitorSeconds-1 in STAX V3.1.2, in order to pick
        // up the new default value of 60 seconds
        propertiesData.put("processMonitorSeconds-1", processMonitorSeconds);
        propertiesData.put("elapsedTimeSeconds", elapsedTimeSeconds);
        propertiesData.put("autoMonitorSubjobs", autoMonitorSubjobs);

        propertiesData.put("messageFontName", messageFontName);
        propertiesData.put("logViewerFontName", logViewerFontName);
        propertiesData.put("saveAsDirectory", saveAsDirectory);
            
        try
        {
            ObjectOutputStream oos = new ObjectOutputStream(new
                FileOutputStream(fPropertiesFileName));
            oos.writeObject(((Object)(propertiesData)));                                
            oos.close();
            
            // Print the updated properties

            System.out.println("\nUpdated Properties:");

            if (!newStaxMachineName.equals(""))
            {
                System.out.println("  STAX Machine = " + newStaxMachineName);
            }

            if (!newStaxServiceName.equals(""))
            {
                System.out.println("  STAX Service Name = " +
                                   newStaxServiceName);
            }

            if (!newEventMachineName.equals(""))
            {
                System.out.println("  Event Machine = " + newEventMachineName);
            }

            if (!newEventServiceName.equals(""))
            {
                System.out.println("  Event Service Name = " +
                                   newEventServiceName);
            }

            if (!newJobParmsDirectory.equals(""))
            {
                System.out.println("  jobParmsDirectory = " +
                                   newJobParmsDirectory);
            }
        } 
        catch (IOException ex) 
        {
            ex.printStackTrace();
        }
    }

    public void saveProperties()
    {
        // Get STAX service version

        String versionRequest = "VERSION";

        STAFResult versionResult = fHandle.submit2(
            fStaxMachineName, fStaxServiceName, versionRequest);

        if (versionResult.rc == 0)
        {
            String staxServiceVersion = versionResult.result;
            fSTAXServiceVersion = new STAFVersion(staxServiceVersion);
        
            // Check if STAX service is at the required version (or later)

            STAFVersion requiredVersion1 = new STAFVersion(
                fServiceRequiredVersion);

            if (fSTAXServiceVersion.compareTo(requiredVersion1) < 0)
            {
                JOptionPane.showMessageDialog(this,
                    "WARNING: STAX service machine is not at STAX version " +
                    requiredVersion1 + " or later.\n" + 
                    "machine=" + fStaxMachineName + ", service=" +
                    fStaxServiceName + ", version=" + fSTAXServiceVersion,
                    "Incorrect STAX service version",
                    JOptionPane.ERROR_MESSAGE);
            }
        }
        
        try
        {
            Hashtable propertiesData = new Hashtable();
            propertiesData.put("staxMachineName", fStaxMachineName);
            propertiesData.put("staxServiceName", fStaxServiceName);
            propertiesData.put("eventMachineName", fEventMachineName);
            propertiesData.put("eventServiceName", fEventServiceName);

            fShowNoSTAXMonitorInformationBool =
                fShowNoSTAXMonitorInformation.isSelected();
            propertiesData.put("showNoSTAXMonitorInformation",
                               new Boolean(fShowNoSTAXMonitorInformationBool));

            fLimitMessagesBool = fLimitMessages.isSelected();

            propertiesData.put("limitMessages", 
                               new Boolean(fLimitMessagesBool));

            fLimitMessagesFieldText = fLimitMessagesField.getText();

            propertiesData.put("limitMessagesField", fLimitMessagesFieldText);

            propertiesData.put("pluginJars", fLocalExtJarFiles);
            // This key was changed from processMonitorSeconds to
            // processMonitorSeconds-1 in STAX V3.1.2, in order to pick
            // up the new default value of 60 seconds
            propertiesData.put("processMonitorSeconds-1",
                fProcessMonitorSecondsFieldText);
            propertiesData.put("elapsedTimeSeconds",
                fElapsedTimeSecondsFieldText);

            int autoMonitor = 0;

            if (fAutoMonitorSubjobs)
            {
                autoMonitor = STAXMonitorFrame.AUTOMONITOR_ALWAYS;
            }
            else if (fAutoMonitorRecommendedSubjobs)
            {
                autoMonitor = STAXMonitorFrame.AUTOMONITOR_RECOMMENDED;
            }
            else if (fNeverAutoMonitorSubjobs)
            {
                autoMonitor = STAXMonitorFrame.AUTOMONITOR_NEVER;
            }

            propertiesData.put("autoMonitorSubjobs", new Integer(autoMonitor));

            propertiesData.put("messageFontName",
                               fMessageFontNameCB.getSelectedItem());

            propertiesData.put("logViewerFontName",
                               fLogViewerFontNameCB.getSelectedItem());

            propertiesData.put("saveAsDirectory", fSaveAsDirectory);

            if (fLastJobParmsFileDirectory != null)
            {
                propertiesData.put("lastJobParmsFileDirectory",
                    fLastJobParmsFileDirectory.getAbsolutePath());
            }

            propertiesData.put("displayTestcaseNameColumn",
                new Boolean(fTestcaseNameCB.isSelected()));
            propertiesData.put("displayTestcasePassColumn",
                new Boolean(fTestcasePassCB.isSelected()));
            propertiesData.put("displayTestcaseFailColumn",
                new Boolean(fTestcaseFailCB.isSelected()));
            propertiesData.put("displayTestcaseStartDateTimeColumn",
                new Boolean(fTestcaseStartDateTimeCB.isSelected()));
            propertiesData.put("displayTestcaseStatusDateTimeColumn",
                new Boolean(fTestcaseStatusDateTimeCB.isSelected()));
            propertiesData.put("displayTestcaseDurationColumn",
                new Boolean(fTestcaseDurationCB.isSelected()));
            propertiesData.put("displayTestcaseStartsColumn",
                new Boolean(fTestcaseStartsCB.isSelected()));
            propertiesData.put("displayTestcaseInformationColumn",
                new Boolean(fTestcaseInformationCB.isSelected()));
            propertiesData.put("testcaseAutoResize",
                new Boolean(fTestcaseAutoResizeCB.isSelected()));

            propertiesData.put("testcaseSortColumnName",
                               fTestcaseSortColumnCB.getSelectedItem());
            propertiesData.put("testcaseSortOrder",
                               fTestcaseSortOrderCB.getSelectedItem());

            ObjectOutputStream oos = new ObjectOutputStream(new
                FileOutputStream(fPropertiesFileName));
            oos.writeObject(((Object)(propertiesData)));
            oos.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }

    public void loadProperties()
    {
        try
        {
            File propertiesFile = new File(fPropertiesFileName);

            ObjectInputStream ois =
                new ObjectInputStream(new FileInputStream(propertiesFile));
            Hashtable propertiesData = (Hashtable)(ois.readObject());

            ois.close();

            fStaxMachineName = (String)propertiesData.get("staxMachineName");
            fStaxMachineNameField.setText(fStaxMachineName);
            fStaxServiceName = (String)propertiesData.get("staxServiceName");
            fStaxServiceNameField.setText(fStaxServiceName);

            resolveMachineNames();

            Boolean showNoSTAXMonitorInformation =
                (Boolean)propertiesData.get("showNoSTAXMonitorInformation");
            Boolean limitMessages =
                (Boolean)propertiesData.get("limitMessages");

            Integer autoMonitorSubjobs;

            try
            {
                autoMonitorSubjobs =
                    (Integer)propertiesData.get("autoMonitorSubjobs");
            }
            catch (ClassCastException ex)
            {
                autoMonitorSubjobs = fDefaultAutoMonitorSubjobs;
            }

            fOldStaxMachineName = fStaxMachineName;
            fOldStaxServiceName = fStaxServiceName;
            fOldEventMachineName = fEventMachineName;
            fOldEventServiceName = fEventServiceName;

            if (autoMonitorSubjobs != null)
            {
                if (autoMonitorSubjobs.intValue() ==
                    STAXMonitorFrame.AUTOMONITOR_ALWAYS)
                {
                    fAutoMonitorSubjobsRB.setSelected(true);
                    fAutoMonitorSubjobs = true;
                    fAutoMonitorRecommendedSubjobsRB.setSelected(false);
                    fAutoMonitorRecommendedSubjobs = false;
                    fNeverAutoMonitorSubjobsRB.setSelected(false);
                    fNeverAutoMonitorSubjobs = false;
                }
                else if (autoMonitorSubjobs.intValue() ==
                    STAXMonitorFrame.AUTOMONITOR_RECOMMENDED)
                {
                    fAutoMonitorSubjobsRB.setSelected(false);
                    fAutoMonitorSubjobs = false;
                    fAutoMonitorRecommendedSubjobsRB.setSelected(true);
                    fAutoMonitorRecommendedSubjobs = true;
                    fNeverAutoMonitorSubjobsRB.setSelected(false);
                    fNeverAutoMonitorSubjobs = false;
                }
                else
                {
                    fAutoMonitorSubjobsRB.setSelected(false);
                    fAutoMonitorSubjobs = false;
                    fAutoMonitorRecommendedSubjobsRB.setSelected(false);
                    fAutoMonitorRecommendedSubjobs = false;
                    fNeverAutoMonitorSubjobsRB.setSelected(true);
                    fNeverAutoMonitorSubjobs = true;
                }
            }

            fLocalExtJarFiles = (Vector)propertiesData.get("pluginJars");

            fUpdatedLocalExtJarFiles = new Vector();

            for (int i = 0; i < fLocalExtJarFiles.size(); i++)
            {
                fUpdatedLocalExtJarFiles.addElement(
                    fLocalExtJarFiles.elementAt(i));
            }

            if (fLocalExtJarFiles == null)
            {
                fLocalExtJarFiles = new Vector();
                fUpdatedLocalExtJarFiles = new Vector();
            }

            fOldLocalExtJarFiles = new Vector(fLocalExtJarFiles);

            DefaultListModel pluginJarsModel =
                (DefaultListModel)fPluginJarsList.getModel();
            pluginJarsModel.removeAllElements();

            if (fLocalExtJarFiles != null)
            {
                for (int i = 0; i < fLocalExtJarFiles.size(); i++)
                {
                    pluginJarsModel.addElement(
                        (String)fLocalExtJarFiles.elementAt(i));
                }
            }

            if (showNoSTAXMonitorInformation != null)
            {
                fShowNoSTAXMonitorInformation.setSelected(
                    showNoSTAXMonitorInformation.booleanValue());
            }

            fShowNoSTAXMonitorInformationBool =
                fShowNoSTAXMonitorInformation.isSelected();

            if (limitMessages != null)
            {
                fLimitMessages.setSelected(
                    limitMessages.booleanValue());

                fLimitMessagesFieldText = (String)
                    propertiesData.get("limitMessagesField");

                fLimitMessagesField.setText(fLimitMessagesFieldText);
            }

            fLimitMessagesBool = fLimitMessages.isSelected();

            // This key was changed from processMonitorSeconds to
            // processMonitorSeconds-1 in STAX V3.1.2, in order to pick
            // up the new default value of 60 seconds
            String processMonitorSeconds =
                (String)propertiesData.get("processMonitorSeconds-1");

            if (processMonitorSeconds != null)
            {
                fProcessMonitorSecondsFieldText = processMonitorSeconds;
                fProcessMonitorSecondsField.setText(processMonitorSeconds);
            }

            String elapsedTimeSeconds =
                (String)propertiesData.get("elapsedTimeSeconds");

            if (elapsedTimeSeconds != null)
            {
                fElapsedTimeSecondsFieldText = elapsedTimeSeconds;

                fElapsedTimeSecondsField.setText(elapsedTimeSeconds);
            }

            String messageFontName = (String)propertiesData.get(
                "messageFontName");

            if (messageFontName != null)
            {
                fMessageFontName = messageFontName;
                fMessageFontNameCB.setSelectedItem(messageFontName);
            }

            String logViewerFontName = (String)propertiesData.get(
                "logViewerFontName");

            if (logViewerFontName != null)
            {
                fLogViewerFontName = logViewerFontName;
                fLogViewerFontNameCB.setSelectedItem(logViewerFontName);
            }
            
            String saveAsDirectory = (String)propertiesData.get(
                "saveAsDirectory");

            if (saveAsDirectory != null)
            {
                fSaveAsDirectory = saveAsDirectory;
                fSaveAsDirectoryField.setText(saveAsDirectory);
            }

            String lastJobParmsFileDirectory = (String)propertiesData.get(
                "lastJobParmsFileDirectory");

            if (lastJobParmsFileDirectory != null)
            {
                fLastJobParmsFileDirectory =
                    new File(lastJobParmsFileDirectory);
            }

            Boolean displayTestcaseNameColumn =
                (Boolean)propertiesData.get("displayTestcaseNameColumn");
            Boolean displayTestcasePassColumn =
                (Boolean)propertiesData.get("displayTestcasePassColumn");
            Boolean displayTestcaseFailColumn =
                (Boolean)propertiesData.get("displayTestcaseFailColumn");
            Boolean displayTestcaseStartDateTimeColumn =
                (Boolean)propertiesData.get(
                "displayTestcaseStartDateTimeColumn");
            Boolean displayTestcaseStatusDateTimeColumn =
                (Boolean)propertiesData.get(
                "displayTestcaseStatusDateTimeColumn");
            Boolean displayTestcaseDurationColumn =
                (Boolean)propertiesData.get("displayTestcaseDurationColumn");
            Boolean displayTestcaseStartsColumn =
                (Boolean)propertiesData.get("displayTestcaseStartsColumn");
            Boolean displayTestcaseInformationColumn =
                (Boolean)propertiesData.get("displayTestcaseInformationColumn");
            Boolean testcaseAutoResize =
                (Boolean)propertiesData.get("testcaseAutoResize");

            if (displayTestcaseNameColumn != null)
            {
                fTestcaseNameCB.setSelected(
                    displayTestcaseNameColumn.booleanValue());
                fDisplayTestcaseName = displayTestcaseNameColumn.booleanValue();
            }

            if (displayTestcasePassColumn != null)
            {
                fTestcasePassCB.setSelected(
                    displayTestcasePassColumn.booleanValue());
                fDisplayTestcasePass = displayTestcasePassColumn.booleanValue();
            }

            if (displayTestcaseFailColumn != null)
            {
                fTestcaseFailCB.setSelected(
                    displayTestcaseFailColumn.booleanValue());
                fDisplayTestcaseFail = displayTestcaseFailColumn.booleanValue();
            }

            if (displayTestcaseStartDateTimeColumn != null)
            {
                fTestcaseStartDateTimeCB.setSelected(
                    displayTestcaseStartDateTimeColumn.booleanValue());
                fDisplayTestcaseStartDateTime =
                    displayTestcaseStartDateTimeColumn.booleanValue();
            }

            if (displayTestcaseStatusDateTimeColumn != null)
            {
                fTestcaseStatusDateTimeCB.setSelected(
                    displayTestcaseStatusDateTimeColumn.booleanValue());
                fDisplayTestcaseStatusDateTime =
                    displayTestcaseStatusDateTimeColumn.booleanValue();
            }

            if (displayTestcaseDurationColumn != null)
            {
                fTestcaseDurationCB.setSelected(
                    displayTestcaseDurationColumn.booleanValue());
                fDisplayTestcaseDuration =
                    displayTestcaseDurationColumn.booleanValue();
            }

            if (displayTestcaseStartsColumn != null)
            {
                fTestcaseStartsCB.setSelected(
                    displayTestcaseStartsColumn.booleanValue());
                fDisplayTestcaseStarts =
                    displayTestcaseStartsColumn.booleanValue();
            }

            if (displayTestcaseInformationColumn != null)
            {
                fTestcaseInformationCB.setSelected(
                    displayTestcaseInformationColumn.booleanValue());
                fDisplayTestcaseInformation =
                    displayTestcaseInformationColumn.booleanValue();
            }

            if (testcaseAutoResize != null)
            {
                fTestcaseAutoResizeCB.setSelected(
                    testcaseAutoResize.booleanValue());
                fTestcaseAutoResize = testcaseAutoResize.booleanValue();
            }

            String testcaseSortColumnName = (String)propertiesData.get(
                "testcaseSortColumnName");

            if (testcaseSortColumnName != null)
            {
                fTestcaseSortColumnCB.setSelectedItem(testcaseSortColumnName);
                fTestcaseSortColumn = fTestcaseSortColumnCB.getSelectedIndex();
            }

            String testcaseSortOrder = (String)propertiesData.get(
                "testcaseSortOrder");

            if (testcaseSortOrder != null)
            {
                fTestcaseSortOrderCB.setSelectedItem(testcaseSortOrder);
                fTestcaseSortOrder = fTestcaseSortOrderCB.getSelectedIndex();
            }
        }
        catch (IOException e)
        {e.printStackTrace();
            return;
        }
        catch (ClassNotFoundException e)
        {e.printStackTrace();
            return;
        }
    }
    
    public void resolveMachineNames()
    {
        // Get STAX Machine full name
 
        STAFResult res = fHandle.submit2(
            fStaxMachineName,
            "VAR", "RESOLVE STRING {STAF/Config/Machine}");

        if (res.rc == 0)
        {
            fStaxConfigMachine = res.result;
        }
        else
        {
            System.out.println(
                "Error resolving string {STAF/Config/Machine} " +
                "on machine " + fStaxMachineName + ", RC: " + res.rc +
                ", Result: " + res.result);
        }

        // Get STAX Machine nickname
            
        res = fHandle.submit2(
            fStaxMachineName,
           "VAR", "RESOLVE STRING {STAF/Config/MachineNickname}");

        if (res.rc == 0)
        {
            fStaxMachineNickname = res.result;
        }
        else
        {
            System.out.println(
                "Error resolving string {STAF/Config/MachineNickname} " +
                "on machine " + fStaxMachineName + ", RC: " + res.rc +
                ", Result: " + res.result);
        }
            
        // Check if the STAX Monitor is running on the same machine as
        // the STAX service using the same STAF instance and get the
        // UUID for the STAF instance used by the STAX machine.

        String isLocalRequest = "No";

        STAFResult result = fHandle.submit2(
            fStaxMachineName, "MISC", "WhoAreYou");

        if (result.rc == 0)
        {
            Map resultsMap = (Map)result.resultObj;
        
            isLocalRequest = (String)resultsMap.get("isLocalRequest");
   
            if (isLocalRequest == null)
                isLocalRequest = "No";

            fStaxInstanceUUID = (String)resultsMap.get("instanceUUID");
        }
        else
        {
            System.out.println(
                "STAF " + fStaxMachineName + " MISC WHOAREYOU failed, " +
                "RC: " + result.rc + ", Result: " + result.result);
        }

        if (isLocalRequest.equalsIgnoreCase("Yes"))
            fIsSTAXServiceLocal = true;
        else
            fIsSTAXServiceLocal = false;

        // Get Event machine/service based on STAX machine/service
        // (Note:  Used to get from properties file)
            
        EventServiceInfo eventServiceInfo = new EventServiceInfo(
            fStaxMachineName, fStaxServiceName, this, true);

        fEventMachineName = eventServiceInfo.getMachine();
        fEventServiceName = eventServiceInfo.getService();
            
        fEventMachineNameField.setText(fEventMachineName);
        fEventServiceNameField.setText(fEventServiceName);
    }

    public void saveJobParms(String fileName)
    {
        try
        {
            Hashtable jobParms = new Hashtable();
            jobParms.put("id", fJobParmsID);
            fJobName = fJobNameField.getText();
            jobParms.put("jobName", fJobName);
            fOtherXmlFileMachineName = fOtherXmlFileMachineField.getText();
            jobParms.put("otherXmlFileMachine", fOtherXmlFileMachineName);
            fLocalXmlFileName = fLocalXmlFileNameField.getText();
            jobParms.put("localXmlFileName", fLocalXmlFileName);
            fOtherXmlFileName = fOtherXmlFileNameField.getText();
            jobParms.put("otherXmlFileName", fOtherXmlFileName);
            fFunction = fFunctionField.getText();
            jobParms.put("argumentsEnabled", new Boolean(fArgumentsEnabled));
            jobParms.put("function", fFunction);
            fArgs = fArguments.getText();
            jobParms.put("args", fArgs);

            ListModel scriptModel = fScriptList.getModel();
            Vector scriptVector = new Vector(scriptModel.getSize());

            for (int i = 0; i < scriptModel.getSize(); i++)
            {
                scriptVector.addElement(scriptModel.getElementAt(i));
            }

            jobParms.put("scriptVector", scriptVector);

            ListModel scriptFilesModel = fScriptFilesList.getModel();
            Vector scriptFilesVector = new Vector(scriptFilesModel.getSize());

            for (int i = 0; i < scriptFilesModel.getSize(); i++)
            {
                scriptFilesVector.addElement(scriptFilesModel.getElementAt(i));
            }

            jobParms.put("scriptFilesVector", scriptFilesVector);

            jobParms.put("localMachine",
                         new Boolean(fMachineLocalRB.isSelected()));
            jobParms.put("otherFunction",
                         new Boolean(fOtherFunctionRB.isSelected()));
            jobParms.put("monitor",
                         new Boolean(fMonitorYesRB.isSelected()));

            // To maintain backwards compatibility with the meaning of the
            // "localScriptMachine" key with already existing job parm files:
            // - if fXMLJobFileScriptMachineRB is selected:
            //      Assign "localScriptMachine" = true
            // - else if fOtherScriptMachineRB is selected:
            //      Assign "localScriptMachine" = false
            // - else if fLocalScriptMachineRB is selected:
            //      Don't put a "localScriptMachine" key in the jobParms map

            if (fXMLJobFileScriptMachineRB.isSelected())
            {
                jobParms.put("localScriptMachine", new Boolean(true));
            }
            else if (fOtherScriptMachineRB.isSelected())
            {
                jobParms.put("localScriptMachine", new Boolean(false));
            }
            else if (fLocalScriptMachineRB.isSelected())
            {
                // Don't put a "localScriptMachine" key in the map
            }

            fScriptFilesMachineName = fScriptFilesMachineTextField.getText();
            jobParms.put("otherScriptMachine", fScriptFilesMachineName);

            int clearLogs = STAXMonitorFrame.DEFAULT;

            if (fClearLogsYesRB.isSelected())
                clearLogs = STAXMonitorFrame.ENABLED;
            else if (fClearLogsNoRB.isSelected())
                clearLogs = STAXMonitorFrame.DISABLED;

            jobParms.put("clearLogs", new Integer(clearLogs));

            int logTCElapsedTime = STAXMonitorFrame.DEFAULT;

            if (fLogTCElapsedTimeYesRB.isSelected())
                logTCElapsedTime = STAXMonitorFrame.ENABLED;
            else if (fLogTCElapsedTimeNoRB.isSelected())
                logTCElapsedTime = STAXMonitorFrame.DISABLED;

            jobParms.put("logTCElapsedTime", new Integer(logTCElapsedTime));

            int logTCNumStarts = STAXMonitorFrame.DEFAULT;

            if (fLogTCNumStartsYesRB.isSelected())
                logTCNumStarts = STAXMonitorFrame.ENABLED;
            else if (fLogTCNumStartsNoRB.isSelected())
                logTCNumStarts = STAXMonitorFrame.DISABLED;

            jobParms.put("logTCNumStarts", new Integer(logTCNumStarts));

            int logTCStartStop = STAXMonitorFrame.DEFAULT;

            if (fLogTCStartStopYesRB.isSelected())
                logTCStartStop = STAXMonitorFrame.ENABLED;
            else if (fLogTCStartStopNoRB.isSelected())
                logTCStartStop = STAXMonitorFrame.DISABLED;

            jobParms.put("logTCStartStop", new Integer(logTCStartStop));

            jobParms.put("pythonOutput", getPythonOutputFromPrettyString(
                (String)fPythonOutputCB.getSelectedItem()));

            jobParms.put("pythonLogLevel",
                         fPythonLogLevelCB.getSelectedItem());

            jobParms.put("wizardSavedFileMachineName",
                         fWizardSavedFileMachineName);
            jobParms.put("wizardSavedFileName", fWizardSavedFileName);
            jobParms.put("wizardSavedFunctionName", fWizardSavedFunctionName);
            jobParms.put("wizardSavedFunctionArgList",
                fWizardSavedFunctionArgList);

            if (fLastFileDirectory != null)
            {
                jobParms.put("lastFileDirectory",
                             fLastFileDirectory.getAbsolutePath());
            }

            jobParms.put("breakpointLinesVector",
                fBreakpointsTableModel.getDataVector());

            jobParms.put("breakpointFirstFunction",
                new Boolean(fBreakpointFirstFunctionCB.isSelected()));

            jobParms.put("breakpointSubjobFirstFunction",
                new Boolean(fBreakpointSubjobFirstFunctionCB.isSelected()));

            ObjectOutputStream oos = new ObjectOutputStream(new
                FileOutputStream(fileName));
            oos.writeObject(((Object)(jobParms)));
            oos.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }

    public boolean loadJobParms(String fileName)
    {
        try
        {
            File jobParmsFile = new File(fileName);

            if (!jobParmsFile.exists())
            {
                return false;
            }

            ObjectInputStream ois = null;

            try
            {
                ois = new ObjectInputStream(new FileInputStream(jobParmsFile));
            }
            catch (StreamCorruptedException ex)
            {
                if (ois != null)
                {
                    ois.close();
                }

                STAXMonitorUtil.showErrorDialog(
                    this, "File " + jobParmsFile +
                    "\nis not a valid STAX Monitor Job Parameters file." +
                    "\nNot a serialized file.");
                return false;
            }

            Object obj = ois.readObject();
            ois.close();

            if (!(obj instanceof Hashtable))
            {
                STAXMonitorUtil.showErrorDialog(
                    this, "File " + jobParmsFile +
                    "\nis not a valid STAX Monitor Job Parameters file." +
                    "\nIncorrect File Type.");
                return false;
            }

            Hashtable jobParms = (Hashtable)(obj);

            if (jobParms.get("id") == null)
            {
                STAXMonitorUtil.showErrorDialog(
                    this, "File " + jobParmsFile +
                    "\nis not a valid STAX Monitor Job Parameters file." +
                    "\nID not found.");
                return false;
            }
            else if (!((String)jobParms.get("id")).equals(fJobParmsID))
            {
                STAXMonitorUtil.showErrorDialog(
                    this, "File " + jobParmsFile +
                    "\nis not a valid STAX Monitor Job Parameters file." +
                    "\nIncorrect ID.");
                return false;
            }

            fJobName = (String)jobParms.get("jobName");
            fJobNameField.setText(fJobName);
            fOtherXmlFileMachineName =
                (String)jobParms.get("otherXmlFileMachine");
            fOtherXmlFileMachineField.setText(fOtherXmlFileMachineName);
            fLocalXmlFileName = (String)jobParms.get("localXmlFileName");
            fLocalXmlFileNameField.setText(fLocalXmlFileName);
            fOtherXmlFileName = (String)jobParms.get("otherXmlFileName");
            fOtherXmlFileNameField.setText(fOtherXmlFileName);
            fFunction = (String)jobParms.get("function");
            fFunctionField.setText(fFunction);
            fArgs = (String)jobParms.get("args");

            Boolean argsEnabled =
                ((Boolean)jobParms.get("argumentsEnabled"));

            if (argsEnabled == null)
            {
                fArguments.setEnabled(true);
                fArgumentsEnabled = true;
            }
            else
            {
                fArguments.setEnabled(argsEnabled.booleanValue());
                fArgumentsEnabled = argsEnabled.booleanValue();
            }

            fScriptFilesMachineName =
                (String)jobParms.get("otherScriptMachine");

            if (fScriptFilesMachineName == null)
            {
                fScriptFilesMachineName = "";
            }

            fScriptFilesMachineTextField.setText(fScriptFilesMachineName);

            if (fArgs == null)
            {
                fArgs = "";
            }

            fArguments.setText(fArgs);

            Vector scriptVector = (Vector)jobParms.get("scriptVector");

            DefaultListModel scriptModel =
                (DefaultListModel)fScriptList.getModel();
            scriptModel.removeAllElements();

            if (scriptVector != null)
            {
                fScriptVector = scriptVector;

                for (int i = 0; i < scriptVector.size(); i++)
                {
                    scriptModel.addElement((String)scriptVector.elementAt(i));
                }
            }
            else
            {
                fScriptVector = new Vector();
            }

            Vector scriptFilesVector =
                (Vector)jobParms.get("scriptFilesVector");

            DefaultListModel scriptFilesModel =
                (DefaultListModel)fScriptFilesList.getModel();
            scriptFilesModel.removeAllElements();

            if (scriptFilesVector != null)
            {
                fScriptFilesVector = scriptFilesVector;

                for (int i = 0; i < scriptFilesVector.size(); i++)
                {
                    scriptFilesModel.addElement(
                        (String)scriptFilesVector.elementAt(i));
                }
            }
            else
            {
                fScriptFilesVector = new Vector();
            }

            if (((Boolean)jobParms.get("localMachine")).booleanValue())
            {
                fMachineLocalRB.setSelected(true);
            }
            else
            {
                fMachineOtherRB.setSelected(true);
            }

            if (((Boolean)jobParms.get("otherFunction")).booleanValue())
            {
                fOtherFunctionRB.setSelected(true);
            }
            else
            {
                fDefaultFunctionRB.setSelected(true);
            }

            // To maintain backwards compatibility with the meaning of the
            // "localScriptMachine" key with already existing job parm files:
            // - if localScriptMachine=true
            //    indicates fXMLJobFileScriptMachineRB is selected
            // - else if localScriptMachine=false
            //    indicates fOtherScriptMachineRB is selected
            // - else if no localScriptMachine key
            //    indicates fLocalScriptMachineRB is selected

            Boolean localScriptMachine =
                (Boolean)jobParms.get("localScriptMachine");

            if (localScriptMachine != null)
            {
                if (localScriptMachine.booleanValue())
                {
                    fXMLJobFileScriptMachineRB.setSelected(true);
                }
                else
                {
                    fOtherScriptMachineRB.setSelected(true);
                }
            }
            else
            {
                fLocalScriptMachineRB.setSelected(true);
            }

            if (((Boolean)jobParms.get("monitor")).booleanValue())
            {
                fMonitorYesRB.setSelected(true);
            }
            else
            {
                fMonitorNoRB.setSelected(true);
            }

            // Handle boolean (true/false) values for clearLogs to support
            // saved JobParms for jobs run with STAX V1.4.1 and earlier.
            // After STAX V1.4.1, clearLogs is an Integer.

            Object clearLogsObj = jobParms.get("clearLogs");

            if (clearLogsObj instanceof Integer)
            {
                Integer clearLogsInt = (Integer)jobParms.get("clearLogs");

                if (clearLogsInt != null)
                {
                    int clearLogs = clearLogsInt.intValue();

                    if (clearLogs == STAXMonitorFrame.ENABLED)
                        fClearLogsYesRB.setSelected(true);
                    else if (clearLogs == STAXMonitorFrame.DISABLED)
                        fClearLogsNoRB.setSelected(true);
                    else
                        fClearLogsDefaultRB.setSelected(true);
                }
            }
            else if (clearLogsObj instanceof Boolean)
            {
                Boolean clearLogsBool = (Boolean)clearLogsObj;

                if (clearLogsBool != null)
                {
                    if (clearLogsBool.booleanValue())
                        fClearLogsYesRB.setSelected(true);
                    else
                        fClearLogsNoRB.setSelected(true);
                }
            }

            Integer logTCElapsedTimeInt =
                (Integer)jobParms.get("logTCElapsedTime");

            if (logTCElapsedTimeInt != null)
            {
                int logTCElapsedTime = logTCElapsedTimeInt.intValue();

                if (logTCElapsedTime == STAXMonitorFrame.ENABLED)
                    fLogTCElapsedTimeYesRB.setSelected(true);
                else if (logTCElapsedTime == STAXMonitorFrame.DISABLED)
                    fLogTCElapsedTimeNoRB.setSelected(true);
                else
                    fLogTCElapsedTimeDefaultRB.setSelected(true);
            }

            Integer logTCNumStartsInt = (Integer)jobParms.get("logTCNumStarts");

            if (logTCNumStartsInt != null)
            {
                int logTCNumStarts = logTCNumStartsInt.intValue();

                if (logTCNumStarts == STAXMonitorFrame.ENABLED)
                    fLogTCNumStartsYesRB.setSelected(true);
                else if (logTCNumStarts == STAXMonitorFrame.DISABLED)
                    fLogTCNumStartsNoRB.setSelected(true);
                else
                    fLogTCNumStartsDefaultRB.setSelected(true);
            }

            Integer logTCStartStopInt = (Integer)jobParms.get("logTCStartStop");

            if (logTCStartStopInt != null)
            {
                int logTCStartStop = logTCStartStopInt.intValue();

                if (logTCStartStop == STAXMonitorFrame.ENABLED)
                    fLogTCStartStopYesRB.setSelected(true);
                else if (logTCStartStop == STAXMonitorFrame.DISABLED)
                    fLogTCStartStopNoRB.setSelected(true);
                else
                    fLogTCStartStopDefaultRB.setSelected(true);
            }

            String pythonOutput = (String)jobParms.get("pythonOutput");

            if (pythonOutput != null)
                fPythonOutputCB.setSelectedItem(
                    getPrettyPythonOutput(pythonOutput));

            String pythonLogLevel = (String)jobParms.get("pythonLogLevel");

            if (pythonLogLevel != null)
                fPythonLogLevelCB.setSelectedItem(pythonLogLevel);

            fWizardSavedFileMachineName =
                (String)jobParms.get("wizardSavedFileMachineName");

            if (fWizardSavedFileMachineName == null)
                fWizardSavedFileMachineName = "";

            fWizardSavedFileName = (String)jobParms.get("wizardSavedFileName");

            if (fWizardSavedFileName == null)
                fWizardSavedFileName = "";

            fWizardSavedFunctionName =
                (String)jobParms.get("wizardSavedFunctionName");

            if (fWizardSavedFunctionName == null)
                fWizardSavedFunctionName = "";
            
            fWizardSavedFunctionArgList =
                (Vector)jobParms.get("wizardSavedFunctionArgList");

            if (fWizardSavedFunctionArgList == null)
            {
                fWizardSavedFunctionArgList = new Vector();
            }

            String lastFileDirectory = (String)jobParms.get(
                "lastFileDirectory");

            if (lastFileDirectory != null)
            {
                fLastFileDirectory = new File(lastFileDirectory);
            }
            else
            {
                fLastFileDirectory = new File(System.getProperty("user.dir"));
            }

            Vector breakpointLinesVector =
                (Vector)jobParms.get("breakpointLinesVector");

            if (breakpointLinesVector == null)
            {
                breakpointLinesVector = new Vector();
            }

            for (int j = 0; j < breakpointLinesVector.size(); j++)
            {
                fBreakpointsTableModel.addRow(
                    (Vector)(breakpointLinesVector.elementAt(j)));
            }

            for (int i = 0; i < fBreakpointTableColumnNames.size(); i++)
            {
                fBreakpointTable.getColumnModel().getColumn(i).
                    setCellRenderer(new STAXExtensionsTableCellRenderer(
                        SwingConstants.LEFT));

                fBreakpointTable.getColumnModel().getColumn(i).
                    setHeaderRenderer(new STAXExtensionsTableCellRenderer(
                        new Font("Dialog", Font.BOLD, 12), Color.lightGray));
            }

            fBreakpointTable.updateUI();
            STAXMonitorUtil.updateRowHeights(fBreakpointTable, 0);
            STAXMonitorUtil.sizeColumnsToFitText(fBreakpointTable);

            Boolean breakpointFirstFunction =
                (Boolean)jobParms.get("breakpointFirstFunction");

            if (breakpointFirstFunction != null)
            {
                if (breakpointFirstFunction.booleanValue())
                {
                    fBreakpointFirstFunctionCB.setSelected(true);
                }
            }

            Boolean breakpointSubjobFirstFunction =
                (Boolean)jobParms.get("breakpointSubjobFirstFunction");

            if (breakpointSubjobFirstFunction != null)
            {
                if (breakpointSubjobFirstFunction.booleanValue())
                {
                    fBreakpointSubjobFirstFunctionCB.setSelected(true);
                }
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
            return false;
        }
        catch (ClassNotFoundException e)
        {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    public void loadRecentFiles()
    {
        try
        {
            File recentFile = new File(fRecentFilesName);
            if (!recentFile.exists())
            {
                fRecentFiles = new Vector();
                return;
            }
            else
            {
                ObjectInputStream ois =
                    new ObjectInputStream(new FileInputStream(recentFile));
                fRecentFiles = (Vector)(ois.readObject());
                ois.close();
            }
        }
        catch (IOException e)
        {
            fRecentFiles = new Vector();
            return;
        }
        catch (ClassNotFoundException e)
        {
            fRecentFiles = new Vector();
            return;
        }

        if (!fRecentFiles.isEmpty())
        {
            fStartNewJobFileMenu.insertSeparator(
                fStartNewJobFileMenu.getItemCount()-1);

            for (int i=0; i < fRecentFiles.size(); i++)
            {
                if (fRecentFiles.elementAt(i) == null)
                {
                    return;
                }

                fRecentFileMenuItems[i] =
                    new JMenuItem((String)fRecentFiles.elementAt(i));
                fStartNewJobFileMenu.insert(fRecentFileMenuItems[i],
                    fStartNewJobFileMenu.getItemCount()-2);
                fRecentFileMenuItems[i].addActionListener(this);
            }
        }
    }

    public void updateRecentFiles(String newFile)
    {
        if (fRecentFiles.contains(newFile))
        {
            int priorIndex = fRecentFiles.indexOf(newFile);

            if (priorIndex == 0)
            {
                return;
            }
            else
            {
                for (int i=priorIndex; i > 0; i--)
                {
                    fRecentFiles.setElementAt(fRecentFiles.elementAt(i-1), i);
                    fRecentFileMenuItems[i].setText(
                        fRecentFileMenuItems[i-1].getText());
                }

                fRecentFiles.setElementAt(newFile, 0);

                fRecentFileMenuItems[0].setText(newFile);
            }
        }
        else
        {
            if (fRecentFiles.isEmpty())
            {
                fStartNewJobFileMenu.insertSeparator(
                    fStartNewJobFileMenu.getItemCount()-1);
            }

            fRecentFiles.insertElementAt(newFile, 0);

            fRecentFiles.setSize(10);

            for (int i=0; i < fRecentFileMenuItems.length; i++)
            {
                if (fRecentFileMenuItems[i] != null)
                {
                    fStartNewJobFileMenu.remove(fRecentFileMenuItems[i]);
                }
            }

            for (int i=0; i <10; i++)
            {
                if (fRecentFiles.elementAt(i) == null)
                {
                    break;
                }

                fRecentFileMenuItems[i] =
                    new JMenuItem((String)fRecentFiles.elementAt(i));
                fStartNewJobFileMenu.insert(fRecentFileMenuItems[i],
                    fStartNewJobFileMenu.getItemCount()-2);
                fRecentFileMenuItems[i].addActionListener(this);
            }
        }
    }

    public void updateRecentLogs(String logQuery, String tooltip)
    {
        if (fRecentLogs.contains(logQuery))
        {
            int priorIndex = fRecentLogs.indexOf(logQuery);

            if (priorIndex == 0)
            {
                return;
            }
            else
            {
                for (int i=priorIndex; i > 0; i--)
                {
                    fRecentLogs.setElementAt(fRecentLogs.elementAt(i-1), i);
                    fRecentLogsTooltips.setElementAt(
                        fRecentLogsTooltips.elementAt(i-1), i);

                    fRecentLogsMenuItems[i].setText(
                        fRecentLogsMenuItems[i-1].getText());
                    fRecentLogsMenuItems[i].setToolTipText(
                        fRecentLogsMenuItems[i-1].getToolTipText());
                }

                fRecentLogs.setElementAt(logQuery, 0);
                fRecentLogsTooltips.setElementAt(tooltip, 0);

                fRecentLogsMenuItems[0].setText(logQuery);
                fRecentLogsMenuItems[0].setToolTipText(tooltip);
            }
        }
        else
        {
            fRecentLogs.insertElementAt(logQuery, 0);
            fRecentLogsTooltips.insertElementAt(tooltip, 0);

            fRecentLogs.setSize(20);
            fRecentLogsTooltips.setSize(20);

            for (int i=0; i < fRecentLogsMenuItems.length; i++)
            {
                if (fRecentLogsMenuItems[i] != null)
                {
                    fDisplayRecentLogs.remove(fRecentLogsMenuItems[i]);
                }
            }

            for (int i=0; i < 20; i++)
            {
                if (fRecentLogs.elementAt(i) == null)
                {
                    break;
                }

                fRecentLogsMenuItems[i] =
                    new JMenuItem((String)fRecentLogs.elementAt(i))
                    {
                        public JToolTip createToolTip()
                        {
                            MultiLineToolTip tip = new MultiLineToolTip();
                            tip.setComponent(this);
                            return tip;
                        }
                    };

                fRecentLogsMenuItems[i].setToolTipText(
                    (String)fRecentLogsTooltips.elementAt(i));

                fDisplayRecentLogs.insert(fRecentLogsMenuItems[i], i);

                fRecentLogsMenuItems[i].addActionListener(this);
            }
        }
    }

    public void updateJobTableRenderers()
    {

        fActiveJobsTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(0).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(1).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(2).setCellRenderer(
            new STAXJobTableCellRenderer());

        fActiveJobsTable.getColumnModel().getColumn(2).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(3).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(3).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(4).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(4).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(5).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(5).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(6).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(6).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));

        fActiveJobsTable.getColumnModel().getColumn(7).setCellRenderer(
            new STAXMonitorTableCellRenderer(
                Color.black, false,
                new Font("Dialog", Font.PLAIN, 12)));

        fActiveJobsTable.getColumnModel().getColumn(7).
            setHeaderRenderer(new STAXMonitorTableCellRenderer(
                Color.black, true,
                new Font("Dialog", Font.BOLD, 12)));
    }

    public void updateExtensionsTableRenderers()
    {
        for (int i = 0; i < fExtensionsColumns.size(); i++)
        {
            fExtensionsTable.getColumnModel().getColumn(i).
                setCellRenderer(new STAXExtensionsTableCellRenderer(
                    SwingConstants.LEFT));

            fExtensionsTable.getColumnModel().getColumn(i).
                setHeaderRenderer(new STAXExtensionsTableCellRenderer(
                    new Font("Dialog", Font.BOLD, 12), Color.lightGray));
        }
    }

    public void registerForJobEvents()
    {
        STAFResult registerResult = fHandle.submit2(
            fEventMachineName,
            fEventServiceName, "REGISTER TYPE " +
            fStaxServiceName.toUpperCase() + "/" + fStaxConfigMachine +
            " SUBTYPE Job MAXATTEMPTS 1 ACKNOWLEDGETIMEOUT 1000 BYHANDLE");

        if (registerResult.rc != 0)
        {
            JOptionPane.showMessageDialog(
                this,
                "Could not register for Job events\n" +
                "RC = " + registerResult.rc,
                "Error registering Job Monitor",
                JOptionPane.ERROR_MESSAGE);
        }
    }

    public void unregisterForJobEvents()
    {
        STAFResult unRegisterResult = fHandle.submit2(
            fEventMachineName,
            fEventServiceName, "UNREGISTER TYPE " +
            fStaxServiceName.toUpperCase() + "/" + fStaxConfigMachine +
            " SUBTYPE Job SUBTYPE staxjobmessage " +
            "SUBTYPE staxjobtestcasestatus");

        if (unRegisterResult.rc != 0)
        {
            // do not display message
        }
    }

    public void seedExistingJobs()
    {
        synchronized(fActiveJobsModelSorter)
        {
            String request = "LIST JOBS";

            STAFResult listJobsResult = fHandle.submit2(
                fStaxMachineName, fStaxServiceName, request);

            if (listJobsResult.rc != 0)
            {
                System.out.println("LIST JOBS request failed with RC: " +
                                   listJobsResult.rc + ", Result: " +
                                   listJobsResult.result);
                return;
            }

            Vector jobsVector = fActiveJobsTableModel.getDataVector();
            Vector jobIDs = new Vector();

            for (int k = 0; k < jobsVector.size(); k++)
            {
                jobIDs.addElement(
                    (((Vector)(jobsVector.elementAt(k))).elementAt(0)));
            }
            
            // Iterate thru the list of STAX jobs currently running

            java.util.List outputList =
                (java.util.List)listJobsResult.resultObj;

            Iterator iter = outputList.iterator();

            while (iter.hasNext())
            {
                Map jobInfoMap = (Map)iter.next();

                String jobID = (String)jobInfoMap.get("jobID");

                String jobName = (String)jobInfoMap.get("jobName");
                if (jobName == null) jobName = sNONE;

                String function = (String)jobInfoMap.get("function");

                // startTimestamp format is YYYYMMDD-HH:MM:SS
                String startTimestamp = (String)jobInfoMap.get(
                    "startTimestamp");
                String startDate = startTimestamp.substring(0, 8);
                String startTime = startTimestamp.substring(9);

                String state = (String)jobInfoMap.get("state");

                // Update jobs in the active jobs table

                boolean found = false;
                int rowIndex = 0;

                for (int j = 0; j < jobsVector.size(); j++)
                {
                    if (!found)
                    {
                        if (((Vector)(jobsVector.elementAt(j))).
                                        elementAt(0).equals(jobID))
                        {
                            found = true;
                            rowIndex = j;
                        }
                    }
                }

                if (!found)
                {
                    Object rowData[] = new Object[8];
                    rowData[0] = new Integer(jobID);
                    rowData[1] = jobName;

                    if (fMonitorTable.containsKey(jobID))
                        rowData[2] = new Boolean(true);
                    else
                        rowData[2] = new Boolean(false);
                    
                    final int waitTime = getElapsedTimeInterval();
                    String initialTime = "";

                    if (waitTime > 0) initialTime = "00:00:00";

                    rowData[3] = function;
                    rowData[4] = state;
                    rowData[5] = startTimestamp;
                    rowData[6] = initialTime;
                    rowData[7] = "";

                    fActiveJobsTableModel.addRow(rowData);
                }

                jobIDs.remove(jobID);

                STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
                STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);

                fJobStartTimes.put(jobID, getCalendar(startDate, startTime));

                fJobStartDateTimes.put(jobID,
                    startDate + "@" + startTime);
            }
        }
    }

    Calendar getCalendar(String startDate, String startTime)
    {
        int year = (new Integer(startDate.substring(0, 4))).intValue();
        int month = (new Integer(startDate.substring(4, 6))).intValue();
        int date = (new Integer(startDate.substring(6, 8))).intValue();

        int hours = (new Integer(startTime.substring(0, 2))).intValue();
        int minutes = (new Integer(startTime.substring(3, 5))).intValue();
        int seconds = (new Integer(startTime.substring(6, 8))).intValue();

        Calendar startCalendar = Calendar.getInstance();
        startCalendar.set(year, month - 1, date, hours, minutes, seconds);

        return startCalendar;
    }

    public void addRecentLog(String queryRequest, String tooltip)
    {
        updateRecentLogs(queryRequest, tooltip);
    }

    public void run()
    {
        STAFResult getResult;
        boolean continueRunning = true;
        int numErrors = 0;

        // Maximum consecutive errors submitting a local QUEUE GET WAIT
        // request before we decide to exit the infinite loop
        int maxErrors = 5;

        while (continueRunning)
        {
            // Use the ALL option to improve performance by getting multiple
            // messages, if available, off the queue at once

            getResult = fHandle.submit2("local", "QUEUE", "GET ALL WAIT");

            if (getResult.rc == STAFResult.Ok)
            {
                numErrors = 0;
            }
            else if (getResult.rc == STAFResult.HandleDoesNotExist ||
                     getResult.rc == STAFResult.RequestCancelled)
            {
                // If the handle doesn't exist, or the request was cancelled
                // (because the handle no longer exists), exit the thread
                // since this handle cannot submit any more requests
                // successfully,

                continueRunning = false;
                break;
            }
            else
            {
                numErrors++;

                System.out.println(
                    "STAF local QUEUE GET ALL WAIT request failed with RC: " +
                    getResult.rc + ", Result: " + getResult.result);

                if (numErrors < maxErrors)
                {
                    continue;
                }
                else
                {
                    System.out.println(
                        "Exiting the queue monitor thread after the " +
                        "QUEUE GET request failed " + maxErrors +
                        " consecutive times");
                    
                    continueRunning = false;
                    break; // Don't process any more messages on the queue
                }
            }
            
            java.util.List queueList = (java.util.List)getResult.resultObj;

            // Iterate through the list of messages we got off our queue

            Iterator queueIter = queueList.iterator();

            while (queueIter.hasNext())
            {
                Map queueMap = (Map)queueIter.next();
                String queueType = (String)queueMap.get("type");

                if (queueType == null)
                    continue; // Ignore message

                if (queueType.equalsIgnoreCase("STAF/STAXMonitor/End"))
                {
                    continueRunning = false;
                    break; // Don't process any more messages on the queue
                }
                else if (!queueType.equalsIgnoreCase("STAF/Service/Event"))
                {
                    continue; // Ignore messages that don't have this type
                }
                
                // Process only STAF/Service/Event messages whose event type
                // matches the STAX service name and machine for which the
                // STAX Monitor is configured

                Map messageMap = (Map)queueMap.get("message"); 
                String type = (String)messageMap.get("type");
                
                if (!type.equalsIgnoreCase(fStaxServiceName + "/" +
                                           fStaxConfigMachine))
                {
                    continue;  // Ignore messages that don't have this type
                }
                
                // Process STAF/Service/Event messages with matching event type

                String eventID = (String)messageMap.get("eventID");

                STAFResult ackResult = fHandle.submit2(
                    STAFHandle.ReqFireAndForget, fEventMachineName,
                    fEventServiceName, "ACKNOWLEDGE EVENTID " + eventID);
                    
                String subtype = (String)messageMap.get("subtype");

                if (!subtype.equals("Job"))
                {
                    continue;  // Ignore messages that don't have subtype Job
                }

                // Process STAF/ServiceEvent messages with matching event type
                // and have event subtype "Job"

                Map propertyMap = (HashMap)messageMap.get("propertyMap");
                String status = (String)propertyMap.get("status");
                    
                if (status == null)
                    continue; // Ignore message
                    
                if (status.equals("begin"))
                {
                    final String jobID = (String)propertyMap.get("jobID");
                    final String jobName = (String)propertyMap.get("jobName");
                    final String startFunction = (String)propertyMap.get(
                        "startFunction");
                    String startTimestamp = (String)propertyMap.get(
                        "startTimestamp");
                        
                    String startDate = "";
                    String startTime = "";

                    if ((startTimestamp != null) &&
                        (startTimestamp.length() > 9))
                    {
                        // startTimestamp format is YYYYMMDD-HH:MM:SS
                        startDate = startTimestamp.substring(0, 8);
                        startTime = startTimestamp.substring(9);
                    }

                    final String runnableStartDate = startDate;
                    final String runnableStartTime = startTime;

                    Runnable runnable = new Runnable()
                    {
                        public void run()
                        {
                            addJob(jobID, jobName, startFunction,
                                   runnableStartDate, runnableStartTime);
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(runnable);
                    }
                    catch (InterruptedException e) {}
                    catch (InvocationTargetException e) {}
                }
                else if (status.equals("run"))
                {
                    // The job is now in a Running state so update its "State"
                    // and "Start Function" (and possibly the "Job Name" and
                    // "Start Timestamp" if the job run event is received
                    // before the job start event

                    final String jobID = (String)propertyMap.get("jobID");
                    final String startFunction =
                        (String)propertyMap.get("startFunction");
                    final String jobName = (String)propertyMap.get("jobName");
                    final String startTimestamp =
                        (String)propertyMap.get("startTimestamp");

                    Runnable runnable = new Runnable()
                    {
                        public void run()
                        {
                            updateJob(jobID, jobName, startFunction,
                                      startTimestamp);
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(runnable);
                    }
                    catch (InterruptedException e) {}
                    catch (InvocationTargetException e) {}
                }
                else if (status.equals("end"))
                {
                    String jobID = (String)propertyMap.get("jobID");
                    String result = STAFMarshallingContext.formatObject(
                        propertyMap.get("result"));
                        
                    final String runnableJobID = jobID;
                    final String runnableResult = result;
                        
                    Runnable runnable = new Runnable()
                    {
                        public void run()
                        {
                            removeJob(runnableJobID, runnableResult);
                        }
                    };

                    try
                    {
                        SwingUtilities.invokeAndWait(runnable);
                    }
                    catch (InterruptedException e) {}
                    catch (InvocationTargetException e) {}

                    if (fCloseOnEndJobID.equals(jobID))
                    {
                        exit();
                        System.exit(0);
                    }
                }
            } // end while iterating through the queue list
        } // end while continueRunning 

        try
        {
            STAXMonitorUtil.unregisterMainHandle();
        }
        catch (STAFException e)
        {
            // Ignore any errors
        }
    }

    /**
     * This method is called when the STAX Monitor receives a job begin event
     * indicating that the job is being submitted for execution (has a
     * "Pending" state).  This method adds the job to the Active Jobs Table
     * if is doesn't already exist and if a STAX QUERY JOB request verifies
     * that the job is still running.
     */ 
    public void addJob(String jobID, String jobName, String startFunction,
                       String startDate, String startTime)
    {
        synchronized(fActiveJobsModelSorter)
        {
            // Determine if job already is in Active Jobs Table.  If so,
            // received job run event before job begin event so ignore.

            Vector jobsVector = fActiveJobsTableModel.getDataVector();

            for (int j = 0; j < jobsVector.size(); j++)
            {
                if (((Vector)(jobsVector.elementAt(j))).elementAt(0).equals(
                    new Integer(jobID)))
                {
                    return;  // Job is already in the Active Jobs Table
                }
            }

            // Job is not in Active Jobs Table.  Submit a STAX QUERY JOB
            // request to make sure didn't receive a job end event before the
            // job begin event

            String request = "QUERY JOB " + jobID;
            
            STAFResult result = fHandle.submit2(
                fStaxMachineName, fStaxServiceName, request);
            
            if (result.rc != STAFResult.Ok)
                return;  // The job has already completed so don't add the job

            // Add the job to the Active Jobs Table using the query job info

            Object rowData[] = new Object[8];
            rowData[0] = new Integer(jobID);

            Map jobInfo = (Map)result.resultObj;

            if (jobName.equals(""))
                jobName = NOT_APPLICABLE;

            rowData[1] = jobName;

            synchronized (fMonitorTable)
            {
                if (fMonitorTable.containsKey(jobID))
                    rowData[2] = new Boolean(true);
                else
                    rowData[2] = new Boolean(false);
            }

            final int waitTime = getElapsedTimeInterval();
            String initialTime = "";

            if (waitTime > 0)
                initialTime = "00:00:00";

            rowData[3] = (String)jobInfo.get("startFunction");
            rowData[4] = (String)jobInfo.get("state");
            rowData[5] = startDate + "-" + startTime;
            rowData[6] = initialTime;
            rowData[7] = "";

            fActiveJobsTableModel.addRow(rowData);
            STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
            STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
        }

        fJobStartTimes.put(jobID, getCalendar(startDate, startTime));
        fJobStartDateTimes.put(jobID, startDate + "@" + startTime);
    }

    /**
     * This method is called when the STAX Monitor receives a job run event
     * indicating that the job is being executed (has a "Running" state).
     */ 
    public void updateJob(String jobID, String jobName, String startFunction,
                          String startTimestamp)
    {
        synchronized (fActiveJobsModelSorter)
        {
            Vector jobsVector = fActiveJobsTableModel.getDataVector();

            // Check if the job already is in the Active Jobs Table.  If so,
            // update isMonitored and start function.  If the job's status in
            // the "Active Jobs Table" is not "Complete", set the job's status
            // to "Running".

            for (int j = 0; j < jobsVector.size(); j++)
            {
                Vector jobVector = (Vector)jobsVector.elementAt(j);

                if (jobVector.elementAt(0).equals(new Integer(jobID)))
                {
                    synchronized (fMonitorTable)
                    {
                        if (fMonitorTable.containsKey(jobID))
                        {
                            fActiveJobsTableModel.setValueAt(
                                new Boolean(true), j, 2);
                        }
                        else
                        {
                            fActiveJobsTableModel.setValueAt(
                                new Boolean(false), j, 2);
                        }
                    }

                    fActiveJobsTableModel.setValueAt(startFunction, j, 3);

                    if (!jobVector.elementAt(4).equals("Complete"))
                    {
                        fActiveJobsTableModel.setValueAt("Running", j, 4);
                    }

                    synchronized (fActiveJobsTable)
                    {
                        fActiveJobsTable.updateUI();
                        STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
                        STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
                    }

                    return;
                }
            }

            // The job is not in Active Jobs Table.
            // Submit a STAX QUERY JOB request to make sure the job is still
            // running (e.g. because could have received a job end event
            // before the job run event),

            String request = "QUERY JOB " + jobID;
            
            STAFResult result = fHandle.submit2(
                fStaxMachineName, fStaxServiceName, request);
            
            if (result.rc != STAFResult.Ok)
                return;  // The job has already completed so don't add the job

            // The job is still running, so add it to the Active Jobs Table
            // (this means that the job run event was received before the
            // job begin event),

            Object rowData[] = new Object[8];
            rowData[0] = new Integer(jobID);

            Map jobInfo = (Map)result.resultObj;

            if (jobName.equals(""))
                rowData[1] = NOT_APPLICABLE;
            else
                rowData[1] = jobName;

            synchronized (fMonitorTable)
            {
                if (fMonitorTable.containsKey(jobID))
                    rowData[2] = new Boolean(true);
                else
                    rowData[2] = new Boolean(false);
            }

            rowData[3] = (String)jobInfo.get("startFunction");
            rowData[4] = (String)jobInfo.get("state");

            String startDate = "";
            String startTime = "";

            if ((startTimestamp != null) && (startTimestamp.length() > 9))
            {
                startDate = startTimestamp.substring(0, 8);
                startTime = startTimestamp.substring(9);
                rowData[5] = startDate + "-" + startTime;
                fJobStartTimes.put(jobID, getCalendar(startDate, startTime));
                fJobStartDateTimes.put(jobID, startDate + "@" + startTime);
            }
            else
            {
                rowData[5] = "";
            }

            int waitTime = getElapsedTimeInterval();
            String initialTime = "";

            if (waitTime > 0)
                initialTime = "00:00:00";

            rowData[6] = initialTime;
            rowData[7] = "";

            fActiveJobsTableModel.addRow(rowData);
            STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
            STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
        }
    }

    /**
     * This method is called when the STAX Monitor receives a job stop event
     * indicating that the job has completed
     */ 
    public void removeJob(String jobID, String result)
    {
        synchronized(fActiveJobsModelSorter)
        {
            // Check if the job is in the Active Jobs Table

            Vector jobsVector = fActiveJobsTableModel.getDataVector();
            int rowIndex = -1;

            for (int j = 0; j < jobsVector.size(); j++)
            {
                if (((Vector)(jobsVector.elementAt(j))).elementAt(0).equals(
                    new Integer(jobID)))
                {
                    rowIndex = j;
                    break;
                }
            }

            if (rowIndex == -1)
                return;  // Job not found in Active Jobs Table

            if (!fMonitorTable.containsKey(jobID))
            {
                // The job is not being monitored so remove the job from
                // the Active Jobs Table

                fJobPopupMenu.setVisible(false);
                fActiveJobsTableModel.removeRow(rowIndex);

                ((STAXMonitorTableCellRenderer)fActiveJobsTable.
                 getColumnModel().getColumn(0).getCellRenderer()).
                    clearRowHeights();
            }
            else
            {
                // The job is being monitored so update its "Status" as
                // complete and its "Result"

                fActiveJobsTableModel.setValueAt("Complete", rowIndex, 4);
                fActiveJobsTableModel.setValueAt(result, rowIndex, 7);
            }

            fJobStartTimes.remove(jobID);
            fJobStartDateTimes.remove(jobID);

            synchronized (fActiveJobsTable)
            {
                fActiveJobsTable.updateUI();
                STAXMonitorUtil.updateRowHeights(fActiveJobsTable, 7);
                STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
            }
        }
    }

    public void exit()
    {
        // Delete temporary extension jar files on local system copied from
        // STAX Service machine (if not the local system)

        Iterator iter = fTempLocalExtFiles.iterator();

        while (iter.hasNext())
        {
            // Delete the extension jar file in fExtensionDirectory

            String jarFileName = (String)(iter.next());

            String deleteRequest = "DELETE ENTRY " +
                STAFUtil.wrapData(jarFileName) + " CONFIRM";

            STAFResult deleteResult = fHandle.submit2(
                "local", "FS", deleteRequest);
        }

        if (fExtensionsDirectory != null)
        {
            // Delete the local fExtensionDirectory

            String deleteRequest = "DELETE ENTRY " +
                STAFUtil.wrapData(fExtensionsDirectory) + " RECURSE CONFIRM";

            STAFResult deleteResult = fHandle.submit2(
                "local", "FS", deleteRequest);
        }

        // Unregister for Job Events

        unregisterForJobEvents();

        try
        {
            ObjectOutputStream oos =
                new ObjectOutputStream(new FileOutputStream(fRecentFilesName));
            oos.writeObject(((Object)(fRecentFiles)));
            oos.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        fHandle.submit2(
            "local", "QUEUE", "QUEUE TYPE STAF/STAXMonitor/End MESSAGE " +
            STAFUtil.wrapData(""));

        saveJobParms(fLastJobParmsFileName);

        fContinueElapsedTime = false;

        STAXMonitorUtil.cleanupHandles();
    }

    public String getStaxConfigMachine()
    {
        return fStaxConfigMachine;
    }

    public String getStaxInstanceUUID()
    {
        return fStaxInstanceUUID;
    }

    public String getVersion()
    {
        return fVersion;
    }

    public Vector getPluginClasses()
    {
        return fPluginClasses;
    }

    public int getElapsedTimeInterval()
    {
        return new Integer(fElapsedTimeSecondsFieldText).intValue() * 1000;
    }

    public int getProcessMonitorInterval()
    {
        return new Integer(fProcessMonitorSecondsFieldText).intValue() * 1000;
    }

    public String getMessageFontName()
    {
        return fMessageFontName;
    }

    public String getLogViewerFontName()
    {
        return fLogViewerFontName;
    }

    public String getSaveAsDirectory()
    {
        return fSaveAsDirectory;
    }
    
    public int getDefaultTestcaseSortColumn()
    {
        return fTestcaseSortColumn;
    }

    public int getDefaultTestcaseSortOrder()
    {
        // 0 is Ascending, 1 is Descending
        return fTestcaseSortOrder;
    }

    public boolean getDisplayTestcaseName()
    {
        return fDisplayTestcaseName;
    }

    public boolean getDisplayTestcasePass()
    {
        return fDisplayTestcasePass;
    }

    public boolean getDisplayTestcaseFail()
    {
        return fDisplayTestcaseFail;
    }

    public boolean getDisplayTestcaseStartDateTime()
    {
        return fDisplayTestcaseStartDateTime;
    }

    public boolean getDisplayTestcaseStatusDateTime()
    {
        return fDisplayTestcaseStatusDateTime;
    }

    public boolean getDisplayTestcaseDuration()
    {
        return fDisplayTestcaseDuration;
    }

    public boolean getDisplayTestcaseStarts()
    {
        return fDisplayTestcaseStarts;
    }

    public boolean getDisplayTestcaseInformation()
    {
        return fDisplayTestcaseInformation;
    }

    public boolean getTestcaseAutoResize()
    {
        return fTestcaseAutoResize;
    }
    
    public String generateErrorMsg(int rc, String result)
    {
        String errorMsg = "RC: " + rc;
        String rcDescription = "";

        if ((rcDescription = getErrorDescription(rc)).length() > 0)
            errorMsg += " (" + rcDescription + ")";

        if (result.length() > 0)
            errorMsg += "\nAdditional info\n---------------\n" + result;

        return errorMsg;
    }

    public String getErrorDescription(int rc)
    {
        String errorDescription = "";
        String helpServiceMachine = "local";

        if (!fStaxMachineName.equals(""))
            helpServiceMachine = fStaxMachineName;

        if (rc < 4000)
        {
            // Submit a request to the HELP service to get the error
            // description for this error

            STAFResult result = fHandle.submit2(
                helpServiceMachine, "HELP", "ERROR " + rc);

            if (result.rc == STAFResult.Ok)
            {
                Map errorHelpMap = (Map)result.resultObj;
                errorDescription = (String)errorHelpMap.get("description");
            }
        }
        else
        {
            // Submit a request to the HELP servicve to get the error
            // description for this STAX service error

            String staxServiceName = "STAX";

            if (!fStaxServiceName.equals(""))
                staxServiceName = fStaxServiceName;

            STAFResult result = fHandle.submit2(
                helpServiceMachine, "HELP", "ERROR " + rc + " SERVICE " +
                staxServiceName);

            if (result.rc == STAFResult.Ok)
            {
                Map errorHelpMap = (Map)result.resultObj;
                errorDescription = (String)errorHelpMap.get("description");
            }
        }

        return errorDescription;
    }

    // Convert a list of messages to a single string with numbered messages
    // where each message is prepended with "\n\n" + messageNumber + ")  "

    private String convertMsgs(java.util.List msgList)
    {
        StringBuffer msg = new StringBuffer();

        Iterator iter = msgList.iterator();
        int i = 0;

        while (iter.hasNext())
        {
            msg.append("\n\n" + ++i + ")  " + (String)iter.next());
        }

        return msg.toString();
    }

    // Load monitor extensions from local extension jar files

    private STAFResult loadLocalExtensions()
    {
        // Return if no local extension jar files specified

        if (fLocalExtJarFiles.isEmpty())
        {
            return new STAFResult(STAFResult.Ok);
        }

        java.util.List msgList = new ArrayList();

        // Iterate through the local extension jar file list, loading valid
        // monitor extensions from each extension jar file.

        Iterator iter = fLocalExtJarFiles.iterator();

        while (iter.hasNext())
        {
            String jarFileName = (String)(iter.next());
            String localJarFileName = jarFileName;

            // Validate extensions in the jar file and load valid extensions

            validateExtension(localJarFileName, LOCAL_EXTENSION,
                              jarFileName, msgList);
        }

        if (msgList.size() == 0)
            return new STAFResult(STAFResult.Ok);
        else
            return new STAFResult(STAFResult.InvalidValue,
                                  convertMsgs(msgList));
    }

    // Load monitor extensions from extension jar files registered on the
    // STAX service machine.

    private STAFResult loadSTAXServiceExtensions()
    {
        java.util.List msgList = new ArrayList();
        java.util.List monitorExtList = new ArrayList();

        // Create a list of extension jar files registered on the STAX service
        // machine that contain one or more monitor extensions

        createExtensionJarFileList(monitorExtList, msgList);

        // Return if no monitor extensions are registered for the STAX service

        if (monitorExtList.isEmpty())
        {
            if (msgList.isEmpty())
                return new STAFResult(STAFResult.Ok);
            else
                return new STAFResult(STAFResult.InvalidValue,
                                      convertMsgs(msgList));
        }

        // For each monitor extension in the extension jar file, check if
        // it's valid and load an instance of the extension's class.

        Iterator iter = monitorExtList.iterator();
        boolean firstExtension = true;

        while (iter.hasNext())
        {
            String jarFileName = (String)(iter.next());
            String localJarFileName = jarFileName;
            File extJarFile = null;

            if (!fIsSTAXServiceLocal)
            {
                // If 1st extension, create fExtensionsDirectory

                if (firstExtension)
                {
                    firstExtension = false;

                    // Assign name of directory to temporarily store extension
                    // jar files copied from the STAX Service machine
                    fExtensionsDirectory = fMonitorFileDirectory + fFileSep +
                        "extensions" + fFileSep + fStaxConfigMachine +
                        fFileSep + fStaxServiceName;

                    File extDir = new File(fExtensionsDirectory);

                    if (extDir.exists())
                    {
                        // Delete the local fExtensionDirectory

                        String deleteRequest = "DELETE ENTRY " +
                             STAFUtil.wrapData(fExtensionsDirectory) +
                             " RECURSE CONFIRM";

                        STAFResult deleteResult = fHandle.submit2(
                            "local", "FS", deleteRequest);

                        if (deleteResult.rc != 0 && deleteResult.rc != 50)
                        {
                            // Ignore RC 50 errors since it could indicate
                            // another instance of the STAX Monitor talking
                            // to the same STAX service machine is running.

                            msgList.add(
                                "WARNING: Error deleting local temporary " +
                                "directory used to store extension jar " +
                                "files copied from STAX service machine.\n" +
                                "Service: FS, Request: " + deleteRequest +
                                ", RC: " + deleteResult.rc + ", Result: " +
                                deleteResult.result);
                        }
                    }

                    if (!extDir.exists())
                    {
                        if (!(extDir.mkdirs()))
                        {
                            msgList.add(
                                "Error creating directory " +
                                fExtensionsDirectory + " to use for " +
                                "temporarily storing extension jar files " +
                                "copied from STAX service machine " +
                                fStaxMachineName);

                            return new STAFResult(STAFResult.InvalidValue,
                                                  convertMsgs(msgList));
                        }
                    }
                }

                // Copy the extension jar files from the STAX service machine
                // to the local STAX Monitor machine's fExtensionsDirectory.

                extJarFile = new File(jarFileName);
                String toFile = fExtensionsDirectory + fFileSep +
                                extJarFile.getName();

                String copyRequest = "COPY FILE " +
                    STAFUtil.wrapData(jarFileName) + " TOFILE " +
                    STAFUtil.wrapData(toFile);

                STAFResult copyResult = fHandle.submit2(
                    fStaxMachineName, "FS", copyRequest);

                if (copyResult.rc != 0)
                {
                    msgList.add(
                        "Error copying extension jar file " + jarFileName +
                        " from STAX service machine.  machine=" +
                        fStaxMachineName + ", Service: FS, Request: " +
                        copyRequest + ", rc=" + copyResult.rc +
                        ", Result: " + copyResult.result);

                    continue;  // Skip this jar file
                }
                else
                {
                    localJarFileName = toFile;
                    fTempLocalExtFiles.add(toFile);
                }
            }

            // Validate extensions in the jar file and load valid extensions

            validateExtension(localJarFileName, STAX_SERVICE_EXTENSION,
                              jarFileName, msgList);
        }

        if (msgList.size() == 0)
            return new STAFResult(STAFResult.Ok);
        else
            return new STAFResult(STAFResult.InvalidValue,
                                  convertMsgs(msgList));
    }

    // Create a list of extension jar files registered on the STAX service
    // machine that contain at least one monitor extension.

    private void createExtensionJarFileList(java.util.List monitorExtList,
                                            java.util.List msgList)
    {
        // Get STAX service version

        String versionRequest = "VERSION";

        STAFResult versionResult = fHandle.submit2(
            fStaxMachineName, fStaxServiceName, versionRequest);

        if (versionResult.rc == 0)
        {
            String staxServiceVersion = versionResult.result;
            fSTAXServiceVersion = new STAFVersion(staxServiceVersion);
        }

        // Get extension jar file information from the STAX service machine

        String queryRequest = "QUERY EXTENSIONJARFILES";

        STAFResult queryResult = fHandle.submit2(
            fStaxMachineName, fStaxServiceName, queryRequest);

        if (queryResult.rc != 0)
        {
            if (queryResult.rc == STAFResult.InvalidRequestString)
            {
                // Check if STAX service is at the required version or later

                STAFVersion requiredVersion = new STAFVersion(
                    fServiceRequiredVersion);

                if (fSTAXServiceVersion.compareTo(requiredVersion) < 0)
                {
                    msgList.add(
                        "Cannot get monitor extensions from STAX " +
                        "service machine because it's not at STAX " +
                        "version " + requiredVersion + " or later.  " +
                        "machine=" + fStaxMachineName +
                        ", service=" + fStaxServiceName +
                        ", version=" + fSTAXServiceVersion );

                    return;
                }
            }

            msgList.add(
                "Error querying extension jar files on STAX service machine " +
                fStaxMachineName + ", Service: " + fStaxServiceName +
                ", Request: " + queryRequest + ", RC: " + queryResult.rc +
                ", Result: " + queryResult.result);

            return;
        }

        // Read through the query output and add the extension jar file names
        // that contain at least one monitor extension to monitoExtList
        
        java.util.List extensionList = (java.util.List)queryResult.resultObj;
        Iterator iter = extensionList.iterator();

        while (iter.hasNext())
        {
            Map extensionMap = (Map)iter.next();

            String jarFileName = (String)extensionMap.get("extensionJarFile");
            Map monitorExtensionMap =
                (Map)extensionMap.get("monitorExtensions");

            if (monitorExtensionMap != null)
                monitorExtList.add(jarFileName);
        }
    }

    // Validate each monitor extension provided in the specified extension
    // jar file.  Load an instance of each valid monitor extension class.
    // Add valid monitor extension information to the fMonitorExtensionMap
    // and add valid extension instances to the fPluginMap.
    // Returns STAFResult (rc, result):
    // - If no validation errors, rc = 0, result = ""
    // - If any validation errors occur, a rc = non-zero value and
    //   result = validation error(s)/warning(s)

    private void validateExtension(String localJarFileName, int source,
                                   String jarFileName, java.util.List msgList)
    {
        // Assign a string value for the source of the extension jar files

        String sourceStr = "STAX service";

        if (source == STAX_SERVICE_EXTENSION)
            sourceStr = "STAX service";
        else if (source == LOCAL_EXTENSION)
            sourceStr = "local";

        // Access the extension jar file

        int numMonExtensions = 0;  // Count of monitor extensions in jar file
        JarFile jarFile = null;

        try
        {
            jarFile = new JarFile(localJarFileName);
        }
        catch (IOException e)
        {
            msgList.add(
                "Unable to access " + sourceStr + " extension jar file " +
                jarFileName + "\n" + e.getMessage());

            return;
        }

        // Access the extension jar file's manifest

        Manifest manifest = null;

        try
        {
            manifest = jarFile.getManifest();
        }
        catch (IOException e)
        {
            msgList.add(
                "Unable to access manifest in " + sourceStr +
                " extension jar file " + jarFileName + "\n" + e.getMessage());

            return;
        }

        Map manifestEntryMap = manifest.getEntries();

        // Get information (e.g. version, description, prereq monitor
        // version), if provided, from the manifest

        String extVersion = NOT_APPLICABLE;
        String description = NOT_APPLICABLE;
        String monitorVersion = NOT_APPLICABLE;

        if (manifestEntryMap.containsKey(STAX_EXTENSION_INFO))
        {
            Attributes attrs = manifest.getAttributes(STAX_EXTENSION_INFO);

            if (attrs.getValue("Extension-Version") != null)
            {
                extVersion = attrs.getValue("Extension-Version");
            }

            if (attrs.getValue("Extension-Description") != null)
            {
                description = attrs.getValue("Extension-Description");
            }

            if (attrs.getValue("Required-Monitor-Version") != null)
            {
                monitorVersion = attrs.getValue("Required-Monitor-Version");

                // Make sure STAX Monitor is at this version or later.

                STAFVersion requiredVersion;

                try
                {
                    requiredVersion = new STAFVersion(monitorVersion);

                    if (fMonitorVersion.compareTo(requiredVersion) < 0)
                    {
                        msgList.add(
                            "Monitor extensions in " + sourceStr +
                            " extension jar file " + jarFileName +
                            " require version " + requiredVersion +
                            " or later of the STAX Monitor." +
                            "  You are running version " + fMonitorVersion +
                            " so not loading these monitor extensions.");

                        // Not loading monitor extensions in this jar file
                        return;
                    }
                }
                catch (NumberFormatException e)
                {
                    msgList.add(
                        "Monitor extensions in " + sourceStr +
                        " extension jar file " + jarFileName +
                        " require an invalid version (" + monitorVersion +
                        ") of the STAX Monitor.\n" + e.toString() +
                        "\nLoading these monitor extensions anyway.");
                }
            }
        }

        // Create a class loader used to load STAX so it can be passed to
        // the STAXMonitorExtensionClassLoader to load STAX classes

        Class c = this.getClass();
        ClassLoader parentClassLoader = c.getClassLoader();

        // Make sure can load the extension class

        ClassLoader loader = new STAXMonitorExtensionClassLoader(
            jarFile, parentClassLoader);

        // Iterate through the staf/staxmonitor/extension entries in the
        // manifest, adding valid monitor extensions to fMonitorExtensionMap

        Iterator mfIter = manifestEntryMap.keySet().iterator();

        while (mfIter.hasNext())
        {
            String entry = (String)mfIter.next();

            if (!entry.startsWith(STAX_MONITOR_EXTENSION))
            {
                continue; // Ignore these entries
            }

            String extName = entry.substring(STAX_MONITOR_EXTENSION.length());

            numMonExtensions++;

            Attributes attrs = manifest.getAttributes(STAX_MONITOR_EXTENSION +
                                                      extName);

            if (!attrs.containsKey(new Attributes.Name("Extension-Class")))
            {
                msgList.add(
                    "Monitor extension '" + extName + "' is missing " +
                    "attribute Extension-Class in manifest for " +
                    sourceStr + " extension jar file " + jarFileName +
                    ".");

                continue;  // Skip this extension
            }

            String pluginClassName = attrs.getValue("Extension-Class");

            if (fMonitorExtensionMap.containsKey(extName))
            {
                ExtensionInfo extInfo =
                    (ExtensionInfo)fMonitorExtensionMap.get(extName);

                if ((source == STAX_SERVICE_EXTENSION) &&
                    (extInfo.getSource() == LOCAL_EXTENSION))
                {
                    // Local extension is overriding STAX Service extension

                    extInfo.putOverriddenJarFileName(jarFileName);

                    // Display warning message if the STAX Service
                    // extension's version is equal or later than the local
                    // extension's version.

                    STAFVersion serviceExtVersion;
                    STAFVersion localExtVersion;

                    try
                    {
                        serviceExtVersion = new STAFVersion(
                            extInfo.getVersion());

                        localExtVersion = new STAFVersion(extVersion);

                        int compareValue = serviceExtVersion.compareTo(
                            localExtVersion);

                        if (compareValue == 0)
                        {
                            msgList.add(
                                "WARNING: Monitor extension '" + extName +
                                "' version " + localExtVersion + " in " +
                                sourceStr + " extension jar file " +
                                jarFileName +
                                " is overridden by the same version (" +
                                serviceExtVersion + ") of the extension in " +
                                extInfo.getSourceStr() +
                                " extension jar file " +
                                extInfo.getJarFileName() + ".");
                        }
                        else if (compareValue < 0)
                        {
                            msgList.add(
                                "WARNING: Monitor extension '" + extName +
                                "' version " + localExtVersion +
                                " in " + sourceStr + " extension jar file " +
                                jarFileName +
                                " is overridden by an earlier version (" +
                                serviceExtVersion + ") of the extension in " +
                                extInfo.getSourceStr() +
                                " extension jar file " +
                                extInfo.getJarFileName() + ".");
                        }
                    }
                    catch (NumberFormatException e)
                    {
                        msgList.add(
                            "WARNING: Monitor extension '" + extName +
                            "' version " + extVersion + " in " + sourceStr +
                            " extension jar file " + jarFileName +
                            " is overridden by version (" +
                            extInfo.getVersion() + ") of the extension in " +
                            extInfo.getSourceStr() + " extension jar file " +
                            extInfo.getJarFileName() + ".\n" + e.toString());
                    }
                }
                else
                {
                    msgList.add(
                        "Cannot register monitor extension '" + extName +
                        "' in " + sourceStr + " extension jar file " +
                        jarFileName + " because it was already " +
                        "registered from " + extInfo.getSourceStr() +
                        " extension jar file " + extInfo.getJarFileName() +
                        ".");
                }

                continue;  // Skip this extension
            }

            Class pluginClass = null;
            Object pluginObj = null;

            try
            {
                pluginClass = loader.loadClass(pluginClassName);

                // Try using a constructor that accepts a STAX object

                Class [] parameterTypes = new Class[1];
                parameterTypes[0] = this.getClass();

                try
                {
                    Constructor construct =
                        pluginClass.getConstructor(parameterTypes);
                    Object [] initArgs = new Object[1];
                    initArgs[0] = this;
                    pluginObj = construct.newInstance(initArgs);

                    // Add to extension info to fMonitorExtensionMap
                    fMonitorExtensionMap.put(
                        extName,
                        new ExtensionInfo(jarFileName, extVersion,
                                          description, monitorVersion,
                                          source));

                    // Add extension class to fPluginClasses
                    fPluginClasses.add(pluginClass);
                }
                catch (NoSuchMethodException e)
                {
                    // Extension does not have a constructor that accepts
                    // a STAX object, so use constructor without parameters
                    pluginObj = pluginClass.newInstance();

                    // Add to extension info to fMonitorExtensionMap
                    fMonitorExtensionMap.put(
                        extName,
                        new ExtensionInfo(jarFileName, extVersion,
                                          description, monitorVersion,
                                          source));

                    // Add extension class to fPluginClasses
                    fPluginClasses.add(pluginClass);
                }
                catch (InvocationTargetException e)
                {
                    msgList.add(
                        "Cannot load class for monitor extension '" +
                        extName + "' in " + sourceStr +
                        " extension jar file " + jarFileName +
                        ".  Class name: " + pluginClassName + "\n" +
                        e.getMessage());

                    e.printStackTrace();
                }
                catch (IllegalArgumentException e)
                {
                    msgList.add(
                        "Cannot load class for monitor extension '" +
                        extName + "' in " + sourceStr +
                        " extension jar file " + jarFileName +
                        ".  Class name: " + pluginClassName + "\n" +
                        e.getMessage());

                    e.printStackTrace();
                }
            }
            catch (ClassNotFoundException e)
            {
                msgList.add(
                    "Cannot load class for monitor extension '" +
                    extName + "' in " + sourceStr +
                    " extension jar file " + jarFileName +
                    ".  Class name: " + pluginClassName + "\n" +
                    e.getMessage());

                e.printStackTrace();
            }
            catch (InstantiationException e)
            {
                msgList.add(
                    "Cannot load class for monitor extension '" +
                    extName + "' in " + sourceStr +
                    " extension jar file " + jarFileName +
                    ".  Class name: " + pluginClassName + "\n" +
                    e.getMessage());

                e.printStackTrace();
            }
            catch (IllegalAccessException e)
            {
                msgList.add(
                    "Cannot load class for monitor extension '" +
                    extName + "' in " + sourceStr +
                    " extension jar file " + jarFileName +
                    ".  Class name: " + pluginClassName +
                    "\n" + e.getMessage());

                e.printStackTrace();
            }
        }

        if (numMonExtensions == 0)
        {
            msgList.add(
                "No monitor extensions exist in " + sourceStr +
                " extension jar file " + jarFileName +
                ".  It's manifest file does not specify any " +
                "entries beginning with " + STAX_MONITOR_EXTENSION + ".");
        }

        return;
    }

    public class SplashScreen extends JWindow implements Runnable,
                                                         MouseListener,
                                                         ActionListener
    {
        public SplashScreen(ImageIcon image, int timeout, String msg)
        {
            int w = image.getIconWidth() + 5;
            int h = image.getIconHeight() + 5;
            Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
            int x = (screen.width - w) / 2;
            int y = (screen.height - h) / 2;
            setBounds(x, y, w, h);

            JPanel splashPanel = new JPanel();
            splashPanel.setLayout(new BorderLayout());
            splashPanel.setBorder(new BevelBorder(BevelBorder.RAISED));
            JLabel picture = new JLabel(image);
            splashPanel.add("Center", picture);
            picture.addMouseListener(this);

            JLabel text = new JLabel(msg, SwingConstants.CENTER);
            text.setPreferredSize(new Dimension(w,25));
            text.setOpaque(true);
            text.setBackground(Color.black);
            text.setForeground(Color.white);
            text.addMouseListener(this);

            splashPanel.add("South", text);
            getContentPane().add(splashPanel);

            javax.swing.Timer timer = new javax.swing.Timer(0, this);
            timer.setRepeats(false);
            timer.setInitialDelay(timeout);
            timer.start();
        }

        public SplashScreen(ImageIcon image, String msg)
        {
            // Displays the initializing splash screen without a timer and
            // does not close it you  mouse click on the splash screen

            int w = image.getIconWidth() + 5;
            int h = image.getIconHeight() + 5;
            Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
            int x = (screen.width - w) / 2;
            int y = (screen.height - h) / 2;
            setBounds(x, y, w, h);

            JPanel splashPanel = new JPanel();
            splashPanel.setLayout(new BorderLayout());
            splashPanel.setBorder(new BevelBorder(BevelBorder.RAISED));
            JLabel picture = new JLabel(image);
            splashPanel.add("Center", picture);

            JLabel text = new JLabel(msg, SwingConstants.CENTER);
            text.setPreferredSize(new Dimension(w,25));
            text.setOpaque(true);
            text.setBackground(Color.black);
            text.setForeground(Color.white);

            splashPanel.add("South", text);
            getContentPane().add(splashPanel);
        }

        public void run()
        {
            setVisible(true);
        }

        public void mouseClicked(MouseEvent event)
        {
            setVisible(false);
            dispose();
        }

        public void mouseEntered(MouseEvent event) {}
        public void mouseExited(MouseEvent event) {}
        public void mousePressed(MouseEvent event) {}
        public void mouseReleased(MouseEvent event) {}

        public void actionPerformed(ActionEvent event)
        {
            setVisible(false);
            dispose();
        }

        public void close()
        {
            setVisible(false);
            dispose();
        }

    }

    public class DirectoryFilter extends FileFilter
    {
        public boolean accept(File file)
        {
            if (file == null) return false;

            if (file.isDirectory())
                return true;
            else
                return false;
        }
        
        public String getDescription()
        {
            return "All Directories";
        }
    }

    public class STAXJobTableCellRenderer
             extends JPanel implements TableCellRenderer
    {
        private JLabel label = new JLabel();
        public Hashtable rowHeights = new Hashtable();
        private boolean isHeader = true;

        private java.net.URL greenballURL =
            ClassLoader.getSystemClassLoader().
                getSystemResource("images/grnball.gif");

        private ImageIcon monitoredIcon = new ImageIcon(greenballURL);

        public STAXJobTableCellRenderer()
        {
            this(new Font("Dialog", Font.PLAIN, 12), Color.white);
            isHeader = false;
        }

        public STAXJobTableCellRenderer(Font font, Color background)
        {
            label.setFont(font);
            label.setOpaque(true);
            label.setBackground(background);
            label.setForeground(Color.black);
            label.setHorizontalAlignment(SwingConstants.CENTER);

            setBorder(BorderFactory.createRaisedBevelBorder());
            
            JPanel innerPanel = new JPanel();
            innerPanel.setLayout(new BorderLayout());
            innerPanel.add(label, BorderLayout.NORTH);
            
            setLayout(new BorderLayout());
            add(innerPanel, BorderLayout.NORTH);
        }

        public void clearRowHeights()
        {
            rowHeights.clear();
        }

        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row, int col)
        {
            if (isHeader)
            {
                setBackground(Color.lightGray);
                label.setBackground(Color.lightGray);
            }
            else if (isSelected)
            {
                setBackground(UIManager.getColor("Table.selectionBackground"));
                label.setBackground(
                    UIManager.getColor("Table.selectionBackground"));
            }
            else
            {
                setBackground(Color.white);
                label.setBackground(Color.white);
            }

            if (fActiveJobsTableModel.getColumnClass(col).equals(
                Boolean.class) && !isHeader)
            {
                label.setText("");
                if (((Boolean)value).booleanValue())
                {
                    label.setIcon(monitoredIcon);
                }
                else
                {
                    label.setIcon(null);
                }
                return this;
            }
            else
            {
                label.setText((value == null) ? "" : String.valueOf(value));
            }

            return this;
        }
    }

    public class STAXJobListCellRenderer
             extends JTextArea implements ListCellRenderer
    {
        public Hashtable rowHeights = new Hashtable();

        public STAXJobListCellRenderer()
        {
            setBorder(BorderFactory.createRaisedBevelBorder());
        }

        public STAXJobListCellRenderer(Font font)
        {
            setFont(font);
            setBorder(BorderFactory.createRaisedBevelBorder());
        }

        public Component getListCellRendererComponent(JList list,
                                                      Object value,
                                                      int index,
                                                      boolean isSelected,
                                                      boolean cellHasFocus)
        {
            setText((value == null) ? "" : String.valueOf(value));

            if (isSelected)
            {
                setBackground(fScriptList.getSelectionBackground());
            }
            else
            {
                setBackground(Color.white);
            }

            return this;
        }
    }

    public class STAXExtensionsTableCellRenderer
             extends JLabel implements TableCellRenderer
    {
        public Hashtable rowHeights = new Hashtable();
        private boolean isHeader = true;

        private java.net.URL greenballURL =
            ClassLoader.getSystemClassLoader().
                getSystemResource("images/grnball.gif");

        private ImageIcon monitoredIcon = new ImageIcon(greenballURL);

        public STAXExtensionsTableCellRenderer()
        {
            this(new Font("Dialog", Font.PLAIN, 12), Color.white);
            isHeader = false;
        }

        public STAXExtensionsTableCellRenderer(int alignment)
        {
            this(new Font("Dialog", Font.PLAIN, 12), Color.white);
            isHeader = false;
            setHorizontalAlignment(alignment);
        }

        public STAXExtensionsTableCellRenderer(Font font, Color background)
        {
            setFont(font);
            setOpaque(true);
            setBackground(background);
            setForeground(Color.black);
            setHorizontalAlignment(SwingConstants.CENTER);

            setBorder(BorderFactory.createRaisedBevelBorder());
        }

        public void clearRowHeights()
        {
            rowHeights.clear();
        }

        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row, int col)
        {
            if (isHeader)
            {
                setBackground(Color.lightGray);
            }
            else if (isSelected)
            {
                setBackground(UIManager.getColor("Table.selectionBackground"));
            }
            else
            {
                setBackground(Color.white);
            }

            setText((value == null) ? "" : String.valueOf(value));

            return this;
        }
    }

    class MonitorElapsedTime extends Thread
    {
        public void run()
        {
            final int waitTime = getElapsedTimeInterval();

            if (waitTime == 0)
                return;

            while (fContinueElapsedTime)
            {
                try
                {
                    Thread.sleep(waitTime);
                }
                catch (InterruptedException ex)
                {
                }

                final Enumeration jobElapsedTimeKeys = fJobStartTimes.keys();

                Runnable runnable = new Runnable()
                {
                    public void run()
                    {
                        while (fContinueElapsedTime &&
                               jobElapsedTimeKeys.hasMoreElements())
                        {
                            String jobID =
                                (String)jobElapsedTimeKeys.nextElement();

                            Calendar jobStarted = (Calendar)
                                fJobStartTimes.get(jobID);

                            updateElapsedTime(jobID,
                                STAXMonitorUtil.getElapsedTime(jobStarted));
                        }

                        synchronized (fActiveJobsTable)
                        {
                            fActiveJobsTable.repaint();
                        }
                    }
                };

                try
                {
                    SwingUtilities.invokeAndWait(runnable);
                }
                catch (InterruptedException ex)
                {
                     ex.printStackTrace();
                }
                catch (InvocationTargetException ex)
                {
                     ex.printStackTrace();
                }
            }
        }

        /**
         * Find the row in the Active Jobs Table for the specified job ID
         * and update its elapsed time field
         */ 
        public void updateElapsedTime(String jobID, String elapsedTime)
        {
            Vector jobsVector = fActiveJobsTableModel.getDataVector();
            
            for (int j = 0; j < jobsVector.size(); j++)
            {
                if (((Vector)(jobsVector.elementAt(j))).elementAt(0).equals(
                    new Integer(jobID)))
                {
                    fActiveJobsTableModel.setValueAt(elapsedTime, j, 6);

                    /* XXX: Need?
                    synchronized (fActiveJobsTable)
                    {
                        fActiveJobsTable.updateUI();
                        STAXMonitorUtil.sizeColumnsToFitText(fActiveJobsTable);
                    }
                    */
                }
            }
        }
    }

    // Helper class to contain information about each monitor extension

    public class ExtensionInfo
    {
        private String fJarFileName;
        private String fVersion;
        private String fDescription;
        private String fRequiredMonitorVersion;
        private int    fSource;  // STAX_SERVICE_EXTENSION or LOCAL_EXTENSION
        private String fOverriddenJarFileName = new String("");

        public ExtensionInfo(String jarFileName, String version,
                      String description, String requiredMonitorVersion,
                      int source)
        {
            fJarFileName = jarFileName;
            fVersion = version;
            fDescription = description;
            fRequiredMonitorVersion = requiredMonitorVersion;
            fSource = source;
        }

        public String getJarFileName() { return fJarFileName; }
        public String getVersion()     { return fVersion; }
        public String getDescription() { return fDescription; }
        public String getRequiredMonitorVersion()
        { return fRequiredMonitorVersion; }
        public int    getSource()      { return fSource; }

        public String getSourceStr()
        {
            if (fSource == STAX_SERVICE_EXTENSION)
                return "STAX Service";
            else if (fSource == LOCAL_EXTENSION)
                return "local";
            else
                return "unknown";
        }

        public String getOverriddenJarFileName()
        { return fOverriddenJarFileName; }

        public void putOverriddenJarFileName(String jarFileName)
        { fOverriddenJarFileName = jarFileName; }
    }

    private class Endpoint
    {
        private String fInterface = null;
        private String fMachineIdentifier = null;
        private String fPort = null;

        private Endpoint(String endpoint)
        {
            int beginIndex = 0;
            int endIndex = endpoint.length();
            
            // Remove the interface, if present, from the beginning of the 
            // endpoint, indicated by <Interface>://

            int interfaceIndex = endpoint.indexOf(sInterfaceSeparator);
            
            if (interfaceIndex > -1)
            {
                fInterface = endpoint.substring(0, interfaceIndex);
                beginIndex = interfaceIndex + sInterfaceSeparator.length();
            }

            // Remove the port, if present, from the end of the endpoint, 
            // indicated by @<Port>

            int portIndex = endpoint.lastIndexOf(sPortSeparator);

            if (portIndex > -1)
            {
                endIndex = portIndex;
                fPort = endpoint.substring(portIndex + sPortSeparator.length());
            }

            fMachineIdentifier = endpoint.substring(beginIndex, endIndex);
        }

        String getInterface()
        {
            return fInterface;
        }

        String getMachineIdentifier()
        {
            return fMachineIdentifier;
        }

        String getPort()
        {
            return fPort;
        }
    }


    // Helper class to contain information about the Event Service associated
    // with the STAX Service used by the STAX Monitor

    private class EventServiceInfo
    {
        private String eventMachineName;
        private String eventServiceName;
        
        private EventServiceInfo(String staxMachineName,
                                 String staxServiceName,
                                 STAXMonitor staxMonitor,
                                 boolean displayMessage)
        {
            String errorTitle = "Error Listing STAX Service Settings";
            String errorText = "STAX Service Machine:  " +
                staxMachineName + "\nSTAX Service Name:  " +
                staxServiceName + "\n\n";
            
            // Set to default values
            this.eventMachineName = staxMachineName;
            this.eventServiceName = "Event";
            
            Endpoint staxEndpoint = new Endpoint(staxMachineName);
            fStaxMachineInterface = staxEndpoint.getInterface();
            fStaxMachineIdentifier = staxEndpoint.getMachineIdentifier();
            fStaxMachinePort = staxEndpoint.getPort();

            // Get event machine and service names based by submitting a
            // LIST SETTINGS request to the STAX service machine
            
            String listRequest = "LIST SETTINGS";            

            STAFResult listResult = fHandle.submit2(
                staxMachineName, staxServiceName, listRequest);

            if (listResult.rc != 0)
            {
                // An error occurred trying to list the STAX service settings

                if (listResult.rc == STAFResult.UnknownService)
                {
                    errorTitle = "STAX Service Not Registered";
                    errorText += "WARNING: The STAX service is not " +
                        "registered on the specified machine." +
                        "\n\nVerify the STAX Monitor's properties.\n";
                    
                }
                else if (listResult.rc == STAFResult.NoPathToMachine)
                {
                    errorTitle = "No Path to STAX Service Machine";
                    errorText += "WARNING: No path to the STAX service " +
                        "machine\nThis indicates that the STAX Monitor is " +
                        "not able to submit a request to the specified " +
                        "STAX service machine.  This error usually " +
                        "indicates that STAF is not running or that " +
                        "this is not the correct STAX service machine.\n\n" +
                        "Verify the STAX Monitor's properties.\n";
                }
                else
                {
                    errorText += "Error listing settings on STAX service " +
                        "machine\n\nRequest: " + listRequest +
                        "\nRC: " + listResult.rc +
                        "\nResult: " + listResult.result +
                        "\n\nVerify the STAX Monitor's properties.\n";
                }
                
                if (displayMessage)
                {
                    STAXMonitorUtil.showErrorDialog(
                        staxMonitor, errorText, errorTitle);
                }

                return;
            }
            else if (!STAFMarshallingContext.isMarshalledData(
                     listResult.result))
            {
                errorText += "Error listing settings on STAX service " +
                    "machine\n\nRequest: " + listRequest +
                    "\nRC: " + listResult.rc +
                    "\nResult: " + listResult.result +
                    "\n\nThe LIST SETTINGS result is not marshalled data.\n";

                if (displayMessage)
                {
                    STAXMonitorUtil.showErrorDialog(
                        staxMonitor, errorText, errorTitle);
                }

                return;
            }

            // Get the event machine name and the event service name

            Map settingsMap = (Map)listResult.resultObj;

            this.eventMachineName = (String)settingsMap.get("eventMachine");
            this.eventServiceName = (String)settingsMap.get("eventService");

            if (this.eventMachineName.equalsIgnoreCase("local") ||
                this.eventMachineName.equalsIgnoreCase("local://local"))
            {
                this.eventMachineName = staxMachineName;
            }
            else
            {
                Endpoint eventEndpoint = new Endpoint(this.eventMachineName);
                String eventMachineInterface = eventEndpoint.getInterface();
                String eventMachineIdentifier = eventEndpoint.getMachineIdentifier();
                String eventMachinePort = eventEndpoint.getPort();
                
                // Determine port to use for Event machine if not specified

                if (eventMachinePort == null)
                {
                    String port = "";

                    if (eventMachineInterface == null)
                    {
                        STAFResult res = fHandle.submit2(
                            staxMachineName, "VAR",
                            "RESOLVE STRING {STAF/Config/DefaultInterface}");

                        if (res.rc != 0)
                        {
                            System.out.println(
                                "Error resolving variable " +
                                "STAF/Config/DefaultInterface on machine: " +
                                staxMachineName + "\nRC: " + res.rc +
                                ", Result: " + res.result);
                            return;
                        }

                        eventMachineInterface = res.result;
                    }

                    STAFResult res = fHandle.submit2(
                        staxMachineName, "MISC", "QUERY INTERFACE " +
                        eventMachineInterface);

                    if (res.rc != 0)
                    {
                        System.out.println(
                            "Error submitting MISC QUERY INTERFACE request " +
                            "to machine: " + staxMachineName + "\nRC: " +
                            res.rc + ", Result: " + res.result);
                        return;
                    }

                    Map interfaceMap = (Map)res.resultObj;
                    Map optionMap = (Map)interfaceMap.get("optionMap");
                    
                    if (optionMap.containsKey("Port"))
                        port = (String)optionMap.get("Port");

                    eventMachinePort = port;
                }

                String eventMachine = "";

                if (fStaxMachineInterface != null)
                    eventMachine += fStaxMachineInterface + sInterfaceSeparator;
                
                eventMachine += eventMachineIdentifier + sPortSeparator +
                    eventMachinePort;

                this.eventMachineName = eventMachine;
            }
            
            // Get the version of the event service

            STAFResult versionResult = fHandle.submit2(
                this.eventMachineName, this.eventServiceName, "VERSION");

            if (versionResult.rc != 0)
            {
                // An error occurred getting the version of the Event service

                if (versionResult.rc == STAFResult.UnknownService)
                {
                    errorTitle = "Event Service Not Registered";
                    errorText += "WARNING: Event service " +
                        this.eventServiceName +
                        " is not registered on machine " +
                        this.eventMachineName + ".\n";
                }
                else if (versionResult.rc == STAFResult.NoPathToMachine)
                {
                    errorTitle = "No Path to Event Service Machine";
                    errorText += "WARNING: No path to the Event service " +
                        "machine " + this.eventMachineName +
                        "\nThis indicates that the STAX Monitor is " +
                        "not able to submit a request to the specified " +
                        "Event service machine.  This error usually " +
                        "indicates that STAF is not running on the machine " +
                        "or this is not the correct Event service machine.\n";
                }
                else
                {
                    errorTitle = "Error Getting Event Service Version";
                    errorText += "Error getting the version of the Event " +
                        "service\nRC: " + versionResult.rc +
                        ", Result: " + versionResult.result + "\n";
                }
                
                if (displayMessage)
                {
                    STAXMonitorUtil.showErrorDialog(
                        staxMonitor, errorText, errorTitle);
                }

                return;
            }

            // Verify that the version of the Event service is at the
            // required version or later

            STAFVersion eventServiceVersion = new STAFVersion(
                versionResult.result);
            STAFVersion eventRequiredVersion = new STAFVersion(
                fEventRequiredVersion);

            if (eventServiceVersion.compareTo(eventRequiredVersion) < 0)
            {
                if (displayMessage)
                {
                    STAXMonitorUtil.showErrorDialog(
                        staxMonitor, "The STAX Monitor requires version " +
                        eventRequiredVersion +
                        " or later of the event service.  The version of the " +
                        this.eventServiceName + " service on machine " +
                        this.eventMachineName + " is " + eventServiceVersion +
                        ".",  "Invalid Event Service Version");

                    return;
                }
            }
        }
        
        private String getMachine()
        {
            return eventMachineName;
        }
        
        private String getService()
        {
            return eventServiceName;
        }
        
        private void setMachine(String eventMachine)
        {
            this.eventMachineName = eventMachine;
        }
        
        private void setService(String eventService)
        {
            this.eventServiceName = eventService;
        }
    }

    public class ArgumentTableCellRenderer
             extends DefaultTableCellRenderer
    {
        public Hashtable rowHeights = new Hashtable();
        private boolean isHeader = true;
        private Color background = Color.white;

        public ArgumentTableCellRenderer()
        {
            this(new Font("Courier", Font.PLAIN, 12), Color.white);
            isHeader = false;
        }

        public ArgumentTableCellRenderer(Font font, Color background)
        {
            //textfield.setFont(font);
            //textfield.setOpaque(true);
            //textfield.setBackground(background);
            //textfield.setForeground(Color.black);
            //textfield.setEditable(true);
        }

        public void clearRowHeights()
        {
            rowHeights.clear();
        }

        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row, int col)
        {
            String cellContent = (value == null) ? "" :
                String.valueOf(value);

            if (value instanceof ArgumentName)
            {
                JPanel panel = new JPanel();
                panel.setLayout(new BorderLayout());
                panel.setBackground(Color.white);
                //panel.add(BorderLayout.NORTH, combo);

                return panel;
            }
            else
            {
                JPanel panel = new JPanel();
                panel.setBackground(Color.white);
                return panel;
            }
        }
    }

    public interface WizardArgumentInterface
    {
        public void setString(String string);
        public String getString();
        public String getFirstLine();
    }

    public class ArgumentName implements WizardArgumentInterface
    {
        private String string;

        public ArgumentName(String string)
        {
            this.string = string;
        }

        public String getString()
        {
            return string;
        }

        public void setString(String string)
        {
            this.string = string;
        }

        public String getFirstLine()
        {
            return string;
        }

        public String toString()
        {
            return string;
        }
    }

    public class ArgumentDescription implements WizardArgumentInterface
    {
        private String string;
        private String firstLine;

        public ArgumentDescription(String string)
        {
            this.string = string;

            StringTokenizer tok = new StringTokenizer(string, "\n");
            int count = tok.countTokens();

            if (count < 2)
            {
                firstLine = string;
            }
            else
            {
                firstLine = tok.nextToken() + "...";
            }
        }

        public String getString()
        {
            return string;
        }

        public void setString(String string)
        {
            this.string = string;
        }

        public String getFirstLine()
        {
            return firstLine;
        }

        public String toString()
        {
            return string;
        }
    }

    public class ArgumentRequired implements WizardArgumentInterface
    {
        private String string;

        public ArgumentRequired(String string)
        {
            this.string = string;
        }

        public String getString()
        {
            return string;
        }

        public void setString(String string)
        {
            this.string = string;
        }

        public String getFirstLine()
        {
            return string;
        }

        public String toString()
        {
            return string;
        }
    }

    public class ArgumentValue implements WizardArgumentInterface
    {
        private String string;
        private String firstLine;

        public ArgumentValue(String string)
        {
            this.string = string;
        }

        public String getString()
        {
            return string;
        }

        public void setString(String string)
        {
            this.string = string;
        }

        public String getFirstLine()
        {
            return string;
        }

        public String toString()
        {
            return string;
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
            FontMetrics metrics = c.getFontMetrics(g.getFont());
            Dimension size = c.getSize();
            g.setColor(c.getBackground());
            g.fillRect(0, 0, size.width, size.height);
            g.setColor(c.getForeground());

            if (lines != null)
            {
                for (int i = 0; i < lines.length; i++)
                {
                    g.drawString(lines[i], 3, (metrics.getHeight()) * (i + 1));
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
            Vector lineVector = new Vector();

            try
            {
                while ((line = br.readLine()) != null)
                {
                    int width =
                        SwingUtilities.computeStringWidth(metrics, line);

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

                for (Enumeration e = lineVector.elements();
                     e.hasMoreElements();
                     i++)
                {
                    lines[i] = (String)e.nextElement();
                }
            }

            int height = metrics.getHeight() * numberOfLines;
            this.maxWidth = maxWidth;

            return new Dimension(maxWidth + 8, height + 8);
        }
    }
}

