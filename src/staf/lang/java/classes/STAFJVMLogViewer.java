/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.table.*;
import javax.swing.event.TableModelListener;
import javax.swing.event.TableModelEvent;

/**
 * This class provides a Java GUI that can display any STAF log on any machine
 * currently running STAF. A STAF log is a binary log file that has been created
 * by the STAF Log service. This Java class submits requests to STAF, so STAF has
 * to be running. This Java class can be run as an application via the command
 * line or can be run via another Java program.
 * <p>
 * Note that the STAX Monitor Java application uses this class to display the
 * STAX job logs and the STAX service log.
 * 
 * 
 * This class provides a Java GUI that can display a JVM Log for any STAF Java
 * service that is currently registered. Each Java service that is registered
 * with STAF runs in a JVM (Java Virtual Machine). A JVM Log is a text log file
 * that is asociated with each JVM created by STAF. Note that more than one Java
 * service may use the same JVM (and thus share the same JVM Log file) depending
 * on the options used when registering the service. Section 4.4 Service
 * Registration in the STAF User's Guide provides more information on
 * registering STAF Java services using the JSTAF library.
 * <p>
 * A JVM Log file contains JVM start information such as the date/time when the
 * JVM was created, the JVM executable, and the J2 options used to start the JVM
 * It also any other information logged by the JVM. This includes any errors that
 * may have occurred while the JVM was running and any debug information output
 * by a Java service. Also, the JVM Log for the STAX service contains the output
 * from any print statements that are used within a &lt;script> element in a
 * STAX xml job file which is useful when debugging Python code contained in a
 * &lt;script> element. When a problem occurs with a STAF Java service, you
 * should always check it's JVM Log as it may contain information to help debug
 * the problem.
 * <p>
 * STAF stores JVM Log files in the {STAF/DataDir}/lang/java/jvm/&lt;JVMName>
 * directory. STAF retains a configurable number of JVM Logs (five by default)
 * for each JVM. The current JVM log file is named JVMLog.1 and older saved JVM
 * log files, if any, are named JVMLog.2 to JVMLog.&lt;MAXLOGS>. When a JVM is
 * started, if the size of the JVMLog.1 file exceeds the maximum configurable
 * size (1M by default), the JVMLog.1 file is copied to JVMLog.2 and so on for
 * any older JVM Logs, and a new JVMLog.1 file will be created.
 * <p>
 * When using the STAFJVMLogViewer, you can specify the machine where the STAF
 * JVM log resides (e.g. where the Java service is registered) and you can
 * specify/select the name of the STAF service whose JVM Log you want to display
 * This Java class submits requests to STAF, so STAF has to be running. This
 * Java class can be run as an application via the command line or can be run
 * via another Java program.
 * <p>
 * Note that the STAX Monitor Java application uses the STAFJVMLogViewer class
 * to display the JVM log for the STAX service and for other services. 
 *
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFJVMLogViewer">
 *      Section "3.6.2 Class STAFJVMLogViewer" in the STAF Java User's Guide</a>
 * @since STAF V3.2.1
 */ 
public class STAFJVMLogViewer extends JFrame
{
    static String helpText = "\nSTAFJVMLogViewer Help\n\n" +
        "Description:\n\n" +
        "The STAFJVMLogViewer displays a JVM log for a STAF Java service " +
        "that is\ncurrently registered.  You can specify the machine where " +
        "the STAF Java service\nis registered and you can specify/select " +
        "the name of the STAF service and it\nwill display it's JVM log.\n\n" +
        "Parameters:\n\n" +
        "-machine <Machine where the STAF Java service is registered>\n" +
        "-serviceName <Java Service Name>\n" +
        "-displayAll\n" +
        "-fontName <Font Name>\n" +
        "-help\n" +
        "-version\n\n" +
        "Notes:\n\n" +
        "1) If the -machine option is not specified, it defaults to local.\n" +
        "2) If the -serviceName option is not specified, you'll be prompted " +
        "to select a\nservice registered on the specified machine whose " +
        "JVM Log you want to display.\n" +
        "3) If the -displayAll option is not specified, only the entries in " +
        "the JVM Log\nfrom the last time the JVM was created are shown.\n" +
        "4) If the -fontName option is not specified, it defaults to the " +
        "Monospaced\nfont.\n" +
        "5) If specifying the -help or -version option, it must be the " +
        "first (and only)\noption specified.\n\n" +
        "Examples:\n\n" +
        "  java com.ibm.staf.STAFJVMLogViewer\n" +
        "  java com.ibm.staf.STAFJVMLogViewer -serviceName STAX\n" +
        "  java com.ibm.staf.STAFJVMLogViewer -machine server1\n" +
        "  java com.ibm.staf.STAFJVMLogViewer -machine server1 " +
        "-serviceName CRON";

    static String kVersion = "3.0.1";

    /**
     * This method is used to execute the STAFJVMLogViewer class from the command
     * line.
     * <p>
     * To get help on the arguments to specify for the STAFJVMLogViewer class from
     * the command line, specify the following:
     * <p>
     * <code>java com.ibm.staf.STAFJVMLogViewer -help</code>
     * <p>
     * The arguments that can be specified from the command line when executing
     * the STAFJVMLogViewer class are:
     * <ul>
     * <li>-machine &lt;Machine where the STAF Java service is registered>
     * <li>-serviceName &lt;Java Service Name>
     * <li>-displayAll
     * <li>-fontName &lt;Font Name>
     * <li>-help
     * <li>-version
     * </ul>
     * <p>
     * All of the options are optional. If specifying the -help or -version
     * option, this option must be the first (and only) option specified.
     * <p>
     * Here's a description of each of the arguments.
     * <p>
     * <ul>
     * <li><code>-machine</code> specifies the endpoint for the machine where
     * the STAF JVM log is located. This option is optional. The default is
     * local.
     * <p>
     * <li><code>-serviceName</code> specifies the name of a Java service that
     * is currently registered whose JVM Log you want to display. This option
     * is optional. If it is not specified, you'll be prompted to select a
     * service registered on the specified machine whose JVM Log you want to
     * display.
     * <p>
     * <li><code>-displayAll</code> specifies to display all of the entries in the
     * JVM Log. This option is optional. The default is to display only the
     * entries in the JVM Log from the last time the JVM was created.
     * <p>
     * <li><code>-fontName</code> specifies the name of the font to use when
     * displaying the STAF JVM Log. This option is optional. The default is
     * Monospaced. Examples of other fonts are Dialog and TimesRoman.
     * <p>
     * <li><code>-help</code> displays help information for the
     * STAFJVMLogViewer. This option is optional.
     * <p>
     * <li><code>-version</code> displays the version of the STAFJVMLogViewer.
     * This option is optional.
     * </ul>Y
     * 
     * @param argv The command-line arguments to be specified by the user.
     */ 
    public static void main(String argv[])
    {
        String machine = "local";
        String serviceName = "";
        boolean displayAll = false;
        String fontName = "Monospaced";
        int maxArgsAllowed = 7;

        if (argv.length == 0)
        {
            // Do no argument processing
        }
        else if (argv.length > maxArgsAllowed)
        {
            System.out.println(
                "\nERROR: Too many arguments.  You specified " + argv.length +
                " arguments, but only up to " + maxArgsAllowed +
                " arguments are allowed.");
            System.out.println(helpText);
            System.exit(1);
        }
        else
        {
            if (argv[0].equalsIgnoreCase("-help"))
            {
                System.out.println(helpText);
                System.exit(0);
            }
            else if (argv[0].equalsIgnoreCase("-version"))
            {
                System.out.println(kVersion);
                System.exit(0);
            }
            else
            {
                for (int i = 0; i < argv.length; i++)
                {
                    if (argv[i].equalsIgnoreCase("-machine"))
                    {
                        if ((i+1) >= argv.length)
                        {
                            System.out.println(
                                "\nERROR: Parameter -machine requires a " +
                                "value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        machine = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-serviceName"))
                    {
                        if ((i+1) > argv.length - 1)
                        {
                            System.out.println(
                                "\nERROR: Parameter -serviceName requires " +
                                "a value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        serviceName = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-displayAll"))
                    {
                        displayAll = true;
                    }
                    else if (argv[i].equalsIgnoreCase("-fontName"))
                    {
                        if ((i+1) > argv.length - 1)
                        {
                            System.out.println(
                                "\nERROR: Parameter -fontName requires a " +
                                "value");
                            System.out.println(helpText);
                            System.exit(1);
                        }

                        fontName = argv[i+1];
                        i++;
                    }
                    else if (argv[i].equalsIgnoreCase("-version"))
                    {
                        System.out.println(
                            "\nERROR: Parameter -version must be specified " +
                            "as the first and only parameter");
                        System.out.println(helpText);
                        System.exit(1);
                    }
                    else if (argv[i].equalsIgnoreCase("-help"))
                    {
                        System.out.println(
                            "\nERROR: Parameter -help must be specified as " +
                            "the first and only parameter");
                        System.out.println(helpText);
                        System.exit(1);
                    }
                    else
                    {
                        System.out.println(
                            "\nERROR: Invalid parameter name: " + argv[i]);
                        System.out.println(helpText);
                        System.exit(1);
                    }
                }
            }
        }

        new STAFJVMLogViewer(new JFrame(), null, machine, serviceName,
                             displayAll, fontName);
    }

    /**
     * This constructs a viewer for the specified service's JVM Log on the
     * local machine with the specified service.
     */ 
    public STAFJVMLogViewer(Component parent, STAFHandle handle,
                            String serviceName)
    {
        this(parent, handle, "local", serviceName, false, "Monospaced");
    }

    /**
     * This constructs a viewer for the specified service's JVM Log on the
     * specified machine.
     */ 
    public STAFJVMLogViewer(Component parent, STAFHandle handle,
                            String machine, String serviceName)
    {
        this(parent, handle, machine, serviceName, false, "Monospaced");
    }

    /**
     * This constructs a viewer for the specified service's JVM Log on the
     * specified machine and with the ability to specify the displayAll flag
     * and the font name to use when displaying the entries in the
     * service's JVM Log.
     * 
     * @param parent
     * @param handle
     * @param machine
     * @param serviceName
     * @param displayAll
     * @param fontName
     */ 
    public STAFJVMLogViewer(Component parent, STAFHandle handle,
                            String machine, String serviceName,
                            boolean displayAll, String fontName)
    {
        this.parent = parent;

        fMachine = machine;
        fServiceName = serviceName;
        fDisplayAll = displayAll;
        fFontName = fontName;
        fJVMLogName = "JVMLog.1";

        STAFResult res;

        // If a handle was specified, don't do a system exit
        if (handle != null)
            fSystemExit = false;

        try
        {
            if (handle == null)
            {
                fHandle = new STAFHandle("STAFJVMLogViewer");
            }
            else
            {
                fHandle = handle;
            }
        }
        catch(STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            //e.printStackTrace();

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        if (serviceName.equals(""))
        {
            // Determine the service name whose JVM Log is to be viewed

            // Get a list of all Java services registered on the specified
            // machine

            String request = "LIST SERVICES";
            STAFResult result = fHandle.submit2(machine, "SERVICE", request);

            if (result.rc != 0)
            {
                showErrorDialog(
                    parent,
                    "Error submitting request:  STAF " + machine +
                    " SERVICE " + request + "\n\n" + "RC:" + result.rc +
                    "\nResult: " + result.result);

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            java.util.List serviceList = (java.util.List)result.resultObj;
            Iterator serviceIter = serviceList.iterator();
            java.util.List javaServiceList = new ArrayList();

            while (serviceIter.hasNext())
            {
                Map serviceMap = (Map)serviceIter.next();
                String library = (String)serviceMap.get("library");

                if (library.equals("JSTAF"))
                {
                    String theServiceName = (String)serviceMap.get("name");
                    javaServiceList.add(theServiceName);
                }
            }

            if (javaServiceList.size() == 0)
            {
                showErrorDialog(
                    parent, "No Java services are registered on machine " +
                    machine);

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            // Show a dialog asking the user to select a service.

            // Convert the list of Java services to an array 
            Object[] possibleValues = javaServiceList.toArray(new Object[0]);

            Object selectedValue = JOptionPane.showInputDialog(
                parent, "Choose the service whose JVM Log you want to display",
                "Select Java Service", JOptionPane.INFORMATION_MESSAGE,
                null, possibleValues, null);

            if (selectedValue == null)
            {
                // Cancel button was selected
                if (! fSystemExit)
                    return;
                else
                    System.exit(1);
            }

            serviceName = (String)selectedValue;
        }
        
        // Determine the name of the JVMLog to view as follows:
        // Submit:  STAF <serviceMachine> SERVICE QUERY SERVICE <serviceName>
        // If successful, the result is a marshalled map.  Get the "Options"
        // value from the map which is a list of the options specified when
        // registering the service.  Iterate through the options list and see
        // if a value in the list begins with "JVMName=".  If so, get it's
        // JVMName value.  Otherwise, assign STAFJVM1 as the JVMName.
        // The most current JVM Log location is:
        //  {STAF/DataDir}/lang/java/jvm/<JVMName>/JVMLog.1

        String JVMName = "STAFJVM1";  // Default JVM Name
        
        res = fHandle.submit2(machine, "SERVICE",
                             "QUERY SERVICE " + serviceName);

        if (res.rc != 0)
        {
            String errorMsg = "Error submitting request: STAF " + machine +
                " SERVICE QUERY SERVICE " + serviceName +
                "\nRC: " + res.rc + "\nResult: " + res.result;
            
            if (res.rc == STAFResult.DoesNotExist)
            {
                errorMsg += "\n\nThe " + serviceName + " service is not " +
                    "currently registered on machine " + machine;
            }

            JOptionPane.showMessageDialog(
                    parent, errorMsg, "Error Querying Service",
                    JOptionPane.INFORMATION_MESSAGE);

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        try
        {
            Map serviceMap = (HashMap)res.resultObj;
            java.util.List optionList = (java.util.List)serviceMap.get(
                "options");
            Iterator optionIter = optionList.iterator();
            String jvmNameOption = "JVMName=";

            while (optionIter.hasNext())
            {
                String option = (String)optionIter.next();
                
                if (option.startsWith(jvmNameOption))
                {
                    JVMName = (option.substring(jvmNameOption.length())).
                        trim();
                    break;
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();

            JOptionPane.showMessageDialog(
                parent, "Error determining the JVMName for the " +
                serviceName + " service on machine " + machine,
                "Error Determining the JVMName",
                JOptionPane.INFORMATION_MESSAGE);

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        fJVMName = JVMName;

        fJVMLogDirName = "{STAF/DataDir}{STAF/Config/Sep/File}lang" +
            "{STAF/Config/Sep/File}java{STAF/Config/Sep/File}jvm" +
            "{STAF/Config/Sep/File}" + fJVMName + "{STAF/Config/Sep/File}";

        // Resolve fJVMLogDirName value so that it looks better when displayed
        // (e.g.  in the frame title, etc)

        res = fHandle.submit2(
            machine, "VAR", "RESOLVE STRING " + fJVMLogDirName);

        if (res.rc == 0)
            fJVMLogDirName = res.result;
        
        fJVMLogFileName = fJVMLogDirName + fJVMLogName;

        String frameTitle = "JVM Log for Service " + serviceName +
            " - STAF " + machine + " FS GET FILE " + fJVMLogFileName;
        
        try
        {
            // Determine the number of JVM Logs that exist for the JVMName

            String request = "LIST DIRECTORY " +
                STAFUtil.wrapData(fJVMLogDirName) + " NAME JVMLog SORTBYNAME";

            res = fHandle.submit2(machine, "FS", request);

            if (res.rc != 0)
            {
                String errorMsg = "Error submitting request: STAF " +
                    fMachine + " " + request + "\nRC: " + res.rc +
                    "\nResult: " + res.result;

                JOptionPane.showMessageDialog(
                        parent, errorMsg,
                        "Error Determining the Number of JVM Logs",
                        JOptionPane.INFORMATION_MESSAGE);

                if (! fSystemExit)
                    return;
                else
                    System.exit(0);
            }

            // Assign list of the JVM Logs and determine the size of that
            // list (which is the number of JVM Logs for this JVM)
        
            java.util.List jvmLogList = (java.util.List)res.resultObj;
            fMaxJVMLogs = jvmLogList.size();
        }
        catch (Exception e)
        {
            e.printStackTrace();

            JOptionPane.showMessageDialog(
                parent, "Error determining the number of JVM Logs that " +
                "exist\nfor service " + serviceName + " on machine " + machine,
                "Error Determining the number of JVM Logs Available",
                JOptionPane.INFORMATION_MESSAGE);

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        // Create the Log Frame and populate it with data from the JVM Log

        fLogFrame = new STAFLogFrame(frameTitle);

        Vector logLines = refreshTable();

        if (logLines == null)
        {
            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        if (logLines.size() == 0)
        {
            JOptionPane.showMessageDialog(
                parent, "Log +  has no entries\n\n" + fLogFrame.getTitle(),
                "No Log Entries",
                JOptionPane.INFORMATION_MESSAGE);

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        fLogFrame.setSize(800, 400);
        fLogFrame.setVisible(true);

        String osName = System.getProperties().getProperty("os.name");

        if (osName.equals("Windows 2000"))
        {
            fLogFrame.setState(JFrame.ICONIFIED);
            fLogFrame.setState(JFrame.NORMAL);
        }
        else
        {
            fLogFrame.toFront();
        }
    }

    /**
     *  This method returns the lines in the JVM Log file..
     *
     *  @return Returns a Vector containing the lines in the JVM Log file,
     */
    public Vector getLogLines()
    {
        STAFResult stafResult = fHandle.submit2(
            fMachine, "FS", "GET FILE " + STAFUtil.wrapData(fJVMLogFileName));

        if (stafResult.rc != 0)
        {
            String errorMsg = "Error submitting request: STAF " + fMachine +
                " FS GET FILE " + fJVMLogFileName + "\nRC: " + stafResult.rc +
                "\nResult: " + stafResult.result;
            
            JOptionPane.showMessageDialog(
                    parent, errorMsg,
                    "Error getting JVMLog",
                    JOptionPane.INFORMATION_MESSAGE);

            return null;
        }

        String logContents = stafResult.result;

        // If displayAll == false, search for the start of the last log within
        // the JVMLog by searching for the last occurrence of:
        String beginNewLogString =
            "***************************************************************" +
            "****************\n" +
            "*** nnnnnnnn-nn:nn:nn"; // - Start of Log for JVMName: xxxx
        String startOfLogString = " - Start of Log for JVMName: ";
        
        if (!fDisplayAll)
        {
            // Only show the last log contained in the JVMLog file

            int index = logContents.lastIndexOf(startOfLogString);
            
            if (index - beginNewLogString.length() >= 0)
            {
                index = index - beginNewLogString.length();
                logContents = logContents.substring(index);
            }
        }

        Vector logLines = new Vector();
        Vector thisLogData = new Vector();
        thisLogData.add(logContents);
        logLines.add(thisLogData);
        
        return logLines;
    }

    /**
     * Displays a popup containing the specified error message.
     * 
     * @param parent  The parent componenet
     * @param message The error message to display.
     */ 
    public void showErrorDialog(Component parent, String message)
    {
        showErrorDialog(parent, message, "STAFJVMLogViwer Error");
    }

    /**
     * Displays a popup containing the specified error message and specified
     * title.
     * 
     * @param parent  The parent componenet
     * @param message The error message to display.
     * @param title   The title of the error message to display
     */ 
    public void showErrorDialog(Component parent, String message,
                                String title)
    {
        JTextPane messagePane = new JTextPane();
        messagePane.setFont(new Font("Dialog", Font.BOLD, 12));
        messagePane.setEditable(false);
        messagePane.setText(message);
        messagePane.select(0,0);
        JScrollPane scrollPane = new JScrollPane(messagePane);

        // Calculate the height for the scrollPane based on the number
        // of characters in the message

        int minHeight = 180;      // Minimum height for scrollPane
        int maxHeight = 650;      // Maximum height for scrollPane
        int avgCharsPerLine = 40; // Avg characters per line
        int lineHeight = 16;      // Approximate height of a line
        int msgLength = message.length();
        int numLines = 0;
        int height = minHeight;

        if (msgLength > 0)
        {
            numLines = msgLength / avgCharsPerLine;

            if (numLines > 0) height = numLines * lineHeight;
        }

        if (height > maxHeight)
            height = maxHeight;
        else if (height < minHeight)
            height = minHeight;

        scrollPane.setPreferredSize(new Dimension(510, height));

        JOptionPane.showMessageDialog(parent, scrollPane, title,
                                      JOptionPane.ERROR_MESSAGE);
    }

    /**
     * This method refreshes the table containing the data in the service's
     * JVM Log file and returns the lines in service's JVM Log file.
     * 
     * @return Returns a Vector containing the lines in tservice's JVM Log file.
     */ 
    public Vector refreshTable()
    {
        Vector logLines = getLogLines();
        if (logLines == null) return null;

        fLogTable.setRowHeight(30);
        fLogTable.setModel(new STAFTableModel(logLines, columnNames));

        updateLogTableRenderers();
        updateRowHeights(fLogTable, 0, fFontName);
        sizeColumnsToFitText(fLogTable);

        // Scroll to bottom
        fLogVerticalScrollBar.setValue(fLogVerticalScrollBar.getMaximum());

        return logLines;
    }

    /**
     * This method updates the log table cell renderers.
     */ 
    public void updateLogTableRenderers()
    {
        fLogTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAFLogTableCellRenderer(false));
    }

    /**
     * This method updates the row heights for the specified table.
     * 
     * @param table           The JTable to be updated.
     * @param multiLineColumn
     * @param fontName        The name of the font to use for the text area
     *                        in the table.
     */ 
    public static void updateRowHeights(JTable table, int multiLineColumn,
                                        String fontName)
    {
        int numLines = 1;

        for (int i = 0 ; i < table.getRowCount() ; i++)
        {
            JTextArea textarea = new JTextArea(
                (String)table.getValueAt(i, multiLineColumn));

            textarea.setFont(new Font(fontName, Font.PLAIN, 12));

            int height = textarea.getPreferredSize().height + 5;

            table.setRowHeight(i, height);
        }
    }

    /**
     * This method updates the size of the columns to fit the text.
     * 
     * @param table           The JTable to be updated.
     */
    public static void sizeColumnsToFitText(JTable table)
    {
        int tableWidth = 0;
        FontMetrics metrics = table.getFontMetrics(table.getFont());

        for (int i = 0; i < table.getColumnCount(); i++)
        {
            int width = 0;
            int maxWidth = 0;
            Vector data = new Vector();
            data.addElement(table.getColumnModel().getColumn(i).
                getHeaderValue());

            for (int j = 0; j < table.getRowCount(); j++)
            {
                try
                {
                    Object obj = table.getValueAt(j,i);
                    String cellText = "";

                    if (obj != null)
                    {
                        cellText = table.getValueAt(j,i).toString();
                    }

                    BufferedReader reader =
                        new BufferedReader(new StringReader(cellText));
                    String line;

                    try
                    {
                        while ((line = reader.readLine()) != null)
                        {
                            data.addElement(line);
                        }
                    }
                    catch(IOException ex)
                    {
                        ex.printStackTrace();
                    }
                    finally
                    {
                        try
                        {
                            reader.close();
                        }
                        catch (IOException ex)
                        {
                            ex.printStackTrace();
                        }
                    }
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                }
            }

            Enumeration e = data.elements();

            while (e.hasMoreElements())
            {
                width = metrics.stringWidth((String)e.nextElement());

                if (width > maxWidth)
                    maxWidth = width;
            }

            Insets insets = ((JComponent)table.getCellRenderer(0,i)).
                getInsets();

            // Need to pad a little extra for everything to look right
            maxWidth += insets.left + insets.right + (maxWidth*.15);

            table.getColumnModel().getColumn(i).setPreferredWidth(maxWidth);

            tableWidth += maxWidth;
        }

        Dimension d = table.getSize();
        d.width = tableWidth;
        table.setSize(d);
    }

    public class STAFLogFrame extends JFrame implements ActionListener
    {
        public STAFLogFrame(String title)
        {
            super(title);

            JMenuBar mainMenuBar = new JMenuBar();
            setJMenuBar(mainMenuBar);
            fFileMenu = new JMenu("File");
            mainMenuBar.add(fFileMenu);
            fViewMenu = new JMenu("View");
            mainMenuBar.add(fViewMenu);
            fFileExit = new JMenuItem("Exit");
            fFileExit.addActionListener(this);
            fFileMenu.add(fFileExit);

            fViewRefresh = new JMenuItem("Refresh");
            fViewRefresh.addActionListener(this);
            fViewMenu.add(fViewRefresh);
            fViewMenu.insertSeparator(1);

            fViewShowCurrent = new JMenuItem("Show Current");
            fViewShowCurrent.addActionListener(this);
            fViewMenu.add(fViewShowCurrent);

            fViewShowAll = new JMenuItem("Show All");
            fViewShowAll.addActionListener(this);
            fViewMenu.add(fViewShowAll);
            fViewMenu.insertSeparator(4);

            fViewSelectJVMLogName = new JMenuItem(
                "Select JVM Log Name...");
            fViewSelectJVMLogName.addActionListener(this);
            fViewMenu.add(fViewSelectJVMLogName);
            fViewMenu.insertSeparator(6);

            fViewChangeFont = new JMenuItem("Change Font...");
            fViewChangeFont.addActionListener(this);
            fViewMenu.add(fViewChangeFont);

            // Set up Change JVM Log Name panels
            
            fSelectJVMLogNameDialog = new JDialog(
                this, "Select JVM Log Name", true);
            JPanel selectJVMLogNamePanel = new JPanel();
            selectJVMLogNamePanel.setLayout(new BorderLayout());
            String selectLogNameTitle = "Select JVM Log Name in Directory " +
                fJVMLogDirName + " ";
            selectJVMLogNamePanel.setBorder(new TitledBorder(
                selectLogNameTitle));

            // Calculate the width for the "Select Log Name" panel based on 
            // the title length which varies based on the length of the
            // JVM Log Directory name

            FontMetrics metrics = selectJVMLogNamePanel.getFontMetrics(
                selectJVMLogNamePanel.getFont());
            int panelWidth = metrics.stringWidth(selectLogNameTitle);
            panelWidth += (int)(panelWidth * .15); // Need to pad a little
            
            fSelectJVMLogNameDialog.setSize(panelWidth, 120);
            fSelectJVMLogNameDialog.getContentPane().add(
                selectJVMLogNamePanel);

            // Determine the number of JVM Logs that exist for the JVMName

            java.util.List jvmLogNameList = new ArrayList();

            for (int i = 1; i <= fMaxJVMLogs; i++)
            {
                jvmLogNameList.add("JVMLog." + i);
            }

            // Convert the list of existing JVM Log names to an array 
            Object[] possibleValues = jvmLogNameList.toArray(new Object[0]);

            fAvailableJVMLogNames = new JComboBox(possibleValues);
            fAvailableJVMLogNames.setBackground(Color.white);
            JPanel jvmLogNamesComboBoxPanel = new JPanel();
            jvmLogNamesComboBoxPanel.add(fAvailableJVMLogNames);
            selectJVMLogNamePanel.add(BorderLayout.NORTH,
                                      jvmLogNamesComboBoxPanel);
            
            JPanel selectJVMLogNameButtonPanel = new JPanel();
            selectJVMLogNameButtonPanel.setLayout(
                new FlowLayout(FlowLayout.CENTER, 0, 0));

            fSelectJVMLogNameOkButton = new JButton("OK");
            fSelectJVMLogNameOkButton.addActionListener(this);
            fSelectJVMLogNameCancelButton = new JButton("Cancel");
            fSelectJVMLogNameCancelButton.addActionListener(this);
            selectJVMLogNameButtonPanel.add(fSelectJVMLogNameOkButton);
            selectJVMLogNameButtonPanel.add(Box.createHorizontalStrut(20));
            selectJVMLogNameButtonPanel.add(fSelectJVMLogNameCancelButton);

            selectJVMLogNamePanel.add(
                BorderLayout.SOUTH, selectJVMLogNameButtonPanel);

            // Set up Change Font Dialog panels

            fChangeFontDialog = new JDialog(this, "Change Font", true);
            fChangeFontDialog.setSize(220, 120);
            JPanel changeFontPanel = new JPanel();
            changeFontPanel.setLayout(new BorderLayout());
            changeFontPanel.setBorder(new TitledBorder("Select Font"));
            fChangeFontDialog.getContentPane().add(changeFontPanel);

            GraphicsEnvironment env = GraphicsEnvironment.
                getLocalGraphicsEnvironment();
            String[] fontNames = env.getAvailableFontFamilyNames();

            fAvailableFonts = new JComboBox(fontNames);
            fAvailableFonts.setBackground(Color.white);
            
            changeFontPanel.add(BorderLayout.NORTH, fAvailableFonts);

            JPanel changeFontButtonPanel = new JPanel();
            changeFontButtonPanel.setLayout(new
                FlowLayout(FlowLayout.CENTER, 0, 0));

            fChangeFontOkButton = new JButton("OK");
            fChangeFontOkButton.addActionListener(this);
            fChangeFontCancelButton = new JButton("Cancel");
            fChangeFontCancelButton.addActionListener(this);
            changeFontButtonPanel.add(fChangeFontOkButton);
            changeFontButtonPanel.add(Box.createHorizontalStrut(20));
            changeFontButtonPanel.add(fChangeFontCancelButton);

            changeFontPanel.add(BorderLayout.SOUTH, changeFontButtonPanel);
 
            columnNames = new Vector();
            columnNames.add("");

            fLogTable = new JTable();

            fLogTable.setFont(new Font(fFontName, Font.PLAIN, 12));
            fAvailableFonts.setSelectedItem(fFontName);
            fLogTable.setRowSelectionAllowed(true);
            fLogTable.setColumnSelectionAllowed(false);
            fLogTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

            updateRowHeights(fLogTable, 0, fFontName);
            sizeColumnsToFitText(fLogTable);

            JScrollPane logScroll = new JScrollPane(fLogTable);

            // Scroll to bottom
            fLogVerticalScrollBar = logScroll.getVerticalScrollBar();
            fLogVerticalScrollBar.setValue(fLogVerticalScrollBar.getMaximum());

            getContentPane().add(logScroll);

            addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
                {
                    if (! fSystemExit)
                        dispose();
                    else
                        System.exit(0);
                }
            });
        }

        public void actionPerformed(ActionEvent e)
        {
            if (e.getSource() == fFileExit)
            {
                if (! fSystemExit)
                    dispose();
                else
                {
                    System.exit(0);
                }
            }
            else if (e.getSource() == fViewRefresh)
            {
                refreshTable();
            }
            else if (e.getSource() == fViewShowCurrent)
            {
                fDisplayAll = false;
                refreshTable();
            }
            else if (e.getSource() == fViewShowAll)
            {
                fDisplayAll = true;
                refreshTable();
            }
            else if (e.getSource() == fViewSelectJVMLogName)
            {
                fSelectJVMLogNameDialog.setLocationRelativeTo(fLogFrame);
                fSelectJVMLogNameDialog.setVisible(true);
            }
            else if (e.getSource() == fSelectJVMLogNameOkButton)
            {
                fJVMLogName = (String)fAvailableJVMLogNames.getSelectedItem();
                fJVMLogFileName = fJVMLogDirName + fJVMLogName;
        
                String frameTitle = "JVM Log for Service " + fServiceName +
                    " - STAF " + fMachine + " FS GET FILE " + fJVMLogFileName;

                fLogFrame.setTitle(frameTitle);

                refreshTable();
                fSelectJVMLogNameDialog.setVisible(false);
            }
            else if (e.getSource() == fSelectJVMLogNameCancelButton)
            {
                fSelectJVMLogNameDialog.setVisible(false);
            }
            else if (e.getSource() == fViewChangeFont)
            {
                fChangeFontDialog.setLocationRelativeTo(fLogFrame);
                fChangeFontDialog.setVisible(true);
            }
            else if (e.getSource() == fChangeFontOkButton)
            {
                fFontName = (String)fAvailableFonts.getSelectedItem();
                fLogTable.setFont(new Font(fFontName, Font.PLAIN, 12));
                updateLogTableRenderers();
                updateRowHeights(fLogTable, 0, fFontName);
                sizeColumnsToFitText(fLogTable);
                fChangeFontDialog.setVisible(false);
            }
            else if (e.getSource() == fChangeFontCancelButton)
            {
                fChangeFontDialog.setVisible(false);
            }
        }

    }
    
    public class STAFLogTableCellRenderer extends JTextArea
                                          implements TableCellRenderer
    {
        public Hashtable rowHeights = new Hashtable();
        private boolean isHeader = true;

        public STAFLogTableCellRenderer()
        {
            this(false);
        }

        public STAFLogTableCellRenderer(boolean isHeader)
        {
            if (isHeader)
            {
                setFont(new Font(fFontName, Font.BOLD, 12));
                setBackground(Color.lightGray);
            }
            else
            {
                setFont(new Font(fFontName, Font.PLAIN, 12));
                setBackground(Color.white);
            }

            this.isHeader = isHeader;
            setOpaque(true);
            setForeground(Color.black);
            //setHorizontalAlignment(SwingConstants.LEFT);

            setBorder(BorderFactory.createRaisedBevelBorder());
        }

        public void clearRowHeights()
        {
            rowHeights.clear();
        }

        public Component getTableCellRendererComponent(
            JTable table, Object value, boolean isSelected, boolean hasFocus,
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

public class STAFTableModel extends javax.swing.table.DefaultTableModel
{
    public STAFTableModel()
    {
        super();
    }
    
    public STAFTableModel(java.lang.Object[][] data,
                          java.lang.Object[] columnNames)
    {
        super(data, columnNames);
    }

    public STAFTableModel(java.lang.Object[] columnNames,
                          int numRows)
    {
        super(columnNames, numRows);
    }

    public STAFTableModel(int numRows, int numColumns)
    {
        super(numRows, numColumns);
    }

    public STAFTableModel(java.util.Vector columnNames,
                          int numRows)
    {
        super(columnNames, numRows);
    }

    public STAFTableModel(java.util.Vector data,
                          java.util.Vector columnNames)
    {
        super(data, columnNames);
    }

    public Class getColumnClass(int col)
    {
        if (dataVector.isEmpty())
        {
            return (new Object()).getClass();
        }
        else
        {
            Vector v = (Vector)dataVector.elementAt(0);
            return v.elementAt(col).getClass();
        }
    }

    public boolean isCellEditable(int row, int column)
    {
        return false;
    }
}

public class STAFTableMap extends DefaultTableModel 
                          implements TableModelListener
{
    protected STAFTableModel model;

    public STAFTableModel getModel()
    {
        return model;
    }

    public void setModel(STAFTableModel model)
    {
        this.model = model;
        model.addTableModelListener(this);
    }

    public Object getValueAt(int aRow, int aColumn)
    {
        return model.getValueAt(aRow, aColumn);
    }

    public void setValueAt(Object aValue, int aRow, int aColumn)
    {
        model.setValueAt(aValue, aRow, aColumn);
    }

    public int getRowCount()
    {
        return (model == null) ? 0 : model.getRowCount();
    }

    public int getColumnCount()
    {
        return (model == null) ? 0 : model.getColumnCount();
    }

    public String getColumnName(int aColumn)
    {
        return model.getColumnName(aColumn);
    }

    public Class getColumnClass(int aColumn)
    {
        return model.getColumnClass(aColumn);
    }

    public boolean isCellEditable(int row, int column)
    {
         return false;
    }

    public void tableChanged(TableModelEvent e)
    {
        fireTableChanged(e);
    }
}

    private STAFHandle fHandle;
    private String fMachine = "local";
    private String fServiceName = "";
    private String fJVMName = "";
    private int fMaxJVMLogs = 1;
    private String fJVMLogDirName = "";
    private String fJVMLogName = "JVMLog.1";
    private String fJVMLogFileName = null;
    private String fLogName = null;
    private JTable fLogTable;
    private boolean fDisplayAll = false;
    private String fFontName = "Monospaced";

    boolean fSystemExit = true;

    STAFTableModel fLogTableModel;

    String fStartDateTime = null;
    Vector columnNames;

    STAFLogFrame fLogFrame;
    JScrollBar fLogVerticalScrollBar;
    Component parent;
    JMenu fFileMenu;
    JMenu fViewMenu;
    JMenuItem fFileExit;
    JMenuItem fViewRefresh;
    JMenuItem fViewShowCurrent;
    JMenuItem fViewShowAll;
    JMenuItem fViewSelectJVMLogName;
    JMenuItem fViewChangeFont;

    JDialog fSelectJVMLogNameDialog;
    JComboBox fAvailableJVMLogNames;
    JButton fSelectJVMLogNameOkButton;
    JButton fSelectJVMLogNameCancelButton;

    JDialog fChangeFontDialog;
    JComboBox fAvailableFonts;
    JButton fChangeFontOkButton;
    JButton fChangeFontCancelButton;
}