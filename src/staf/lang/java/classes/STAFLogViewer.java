/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.File;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.IOException;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Enumeration;
import java.util.Date;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.table.*;
import javax.swing.event.TableModelListener;
import javax.swing.event.TableModelEvent;
import javax.swing.filechooser.FileFilter;

/**
 * The STAFLogViewer class provides a Java GUI that can display any STAF log on
 * any machine currently running STAF. A STAF log is a binary log file that has
 * been created by the STAF Log service. This Java class submits requests to STAF,
 * so STAF has to be running. This Java class can be run as an application via
 * the command line or can be run via another Java program.
 * <p>
 * Note that the STAX Monitor Java application uses this class to display the
 * STAX job logs and the STAX service log.
 *
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFLogViewer">
 *      Section "3.6.1 Class STAFLogViewer" in the STAF Java User's Guide</a>
 * @since STAF V3.1.0
 */
public class STAFLogViewer extends JFrame
{
    static String helpText = "\nSTAFLogViewer Help\n\n" +
        "Description:\n" +
        "\n" +
        "  The STAFLogViewer queries and displays a STAF log on any machine currently\n" +
        "  running STAF.\n" +
        "\n" +
        "Options:\n" +
        "\n" +
        "  -queryRequest <LOG QUERY Request>\n" +
        "  -machine <Log Service Machine Name>\n" +
        "  -serviceName <Log Service Name>\n" +
        "  -levelMask <LEVELMASK option>\n" +
        "  -fontName <Font Name>\n" +
        "  -saveAsDirectory <Directory Name>\n" +
        "  -help\n" +
        "  -version\n" +
        "\n" +
        "Notes:\n" +
        "\n" +
        "1) You must specify one of the following options:\n" +
        "     -queryRequest or -help or -version.\n" +
        "2) If specifying the -help or -version option, it must be the first and only\n" +
        "   option specified.  All of the other options require the -queryRequest option\n" +
        "   to be specified.\n" +
        "3) The -queryRequest option specifies the QUERY request to submit to the Log\n" +
        "   service.  It must not contain the TOTAL or STATS option.  It is required\n" +
        "   unless you specified -help or -version.\n" +
        "4) The -machine option specifies the machine where the LOQ QUERY request\n" +
        "   will be submitted.  If not specified, it defaults to local.\n" +
        "5) The -serviceName option specifies the name of the Log service to which\n" +
        "   the query request will be submitted.  If not specified, it defaults to LOG.\n" +
        "6) The -levelMask option specifies a level mask to be used in the Log query\n" +
        "   request.  If not specified, it defaults to display all log levels.\n" +
        "7) The -fontName option specifies the font to use.  If not specified, it\n" +
        "   defaults to Monospaced.\n" +
        "8) The -saveAsDirectory option specifies the initial directory path that will\n" +
        "   be used if you select a \"File->Save As ...\" menu option to save the log\n" +
        "   query output in a file.  If not specified, it defaults to user's default\n" +
        "   directory which is typically the \"My Documents\" folder on Windows or the\n" +
        "   user's home directory on Unix.\n" +
        "\n" +
        "Examples: (Enter each as a single command)\n" +
        "\n" +
        "1) java com.ibm.staf.STAFLogViewer -machine awesvc3 -queryRequest\n" +
        "   \"QUERY GLOBAL LOGNAME CIS LAST 50\"\n" +
        "2) java com.ibm.staf.STAFLogViewer -levelMask \"ERROR WARNING\" -queryRequest\n" +
        "   \"QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME email LAST 100\"\n" +
        "3) java com.ibm.staf.STAFLogViewer -queryRequest \"QUERY MACHINE \n" +
        "   {STAF/Config/MachineNickname} LOGNAME STAX_Job_1 ALL\" -saveAsDirectory C:\\logs";
        
    static String kVersion = "3.1.1";

    /**
     * This method is used to execute the STAFLogViewer class from the command
     * line.
     * <p>
     * To get help on the arguments to specify for the STAFLogViewer class from
     * the command line, specify the following:
     * <p>
     * <code>java com.ibm.staf.STAFLogViewer -help</code>
     * <p>
     * The arguments that can be specified from the command line when executing
     * the STAFLogViewer class are:
     * <ul>
     * <li>-queryRequest &lt;LOG QUERY Request>
     * <li>-machine &lt;Log Service Machine Name>
     * <li>-serviceName &lt;Log Service Name>
     * <li>-levelMask &lt;LEVELMASK option>
     * <li>-fontName &lt;Font Name>
     * <li>-saveAsDirectory &lt;Directory Name>
     * <li>-help
     * <li>-version
     * </ul>
     * <p>
     * You must specify -queryRequest or -help or -version.
     * <p>
     * Here's a description of each of the arguments:
     * <p>
     * <ul>
     * <li><code>-queryRequest</code> specifies the log query options. This
     *     option is required (unless you are specifying the -help or -version
     *     options).
     *     It should begin with the QUERY option and include any additional
     *     query options, including the options to specify whether the log is
     *     a GLOBAL, MACHINE, or HANDLE log. You may not specify the LEVELMASK
     *     option within the queryRequest. To specify the LEVELMASK option, use
     *     the -levelMask option.
     * <p>
     * <li><code>-machine</code> specifies the name of the machine where the
     * STAF log is located. This optiontis optional. The default is local.
     * <li><code>-serviceName</code> specifies the name of the log service.
     * This option is optional. The default is LOG.
     * <li><code>-levelMask</code> specifies the log levels to be initially
     * displayed. This option is optional. The default is to display all log
     * levels. Only level names are supported (for example "ERROR DEBUG".);
     * use of the 32 byte bit string to represent the log mask is not supported.
     * These are the log levels that will be displayed when the initial
     * STAFLogViewer is displayed. The user can then change the log levels to
     * be displayed via the "Levels" menu.
     * <li><code>-fontName</code> specifies the name of the font to use when
     * displaying the STAF log. This option is optional. The default is
     * Monospaced. Examples of other fonts are Dialog and TimesRoman.
     * <li><code>-saveAsDirectory</code> specifies the initial directory path
     * that will be used if you select the "File->Save As Text..." or
     * "File->Save As Html..." menu item to save the log query output in a file.
     * If not specified, it defaults to user's default directory which is
     * typically the "My Documents" folder on Windows or the user's home
     * directory on Unix.
     * <li><code>-help</code> displays help information for the STAFLogViewer.
     * This option is optional.
     * <li><code>-version</code> displays the version of the STAFLogViewer.
     * This option is optional.
     * </ul>
     * 
     * @param argv The command-line arguments to be specified by the user.
     */
    public static void main(String argv[])
    {
        String machine = "local";
        String serviceName = "LOG";
        String queryRequest = "";
        String fontName = "Monospaced";
        String saveAsDirectory = null;
        String levelMask = "";

        int minArgsAllowed = 1;  // -queryRequest or -help or -version

        // Maximum of 12 arguments: A maximum of 6 options can be specifed at
        // once, but need to multiple 6 by 2 (to get 12) since each of these
        // options requires a value (and each option value also counts as an
        // argument).
        // e.g. -queryRequest <request> -machine <machine> -serviceName <name>
        //      -levelMask <mask> -fontName <fontName> -saveAsDirectory <dir>
        int maxArgsAllowed = 12;

        boolean queryRequestArg = false;
        
        Set options = new HashSet();
        options.add("-queryRequest");
        options.add("-machine");
        options.add("-serviceName");
        options.add("-levelMask");
        options.add("-fontName");
        options.add("-saveAsDirectory");
        options.add("-help");
        options.add("-version");

        if (argv.length < minArgsAllowed)
        {
            System.out.println(
                "\nERROR: Must specify " + minArgsAllowed + " of the " +
                "following options:  -queryRequest, -help, or -version");
            System.out.println(helpText);
            System.exit(1);
        }
        else if (argv.length > maxArgsAllowed)
        {
            System.out.println(
                "\nERROR: Too many arguments.  You specified " + argv.length +
                " options, but only up to " + maxArgsAllowed +
                " arguments are allowed.");

            System.out.println(helpText);
            System.exit(1);
        }
        else if (argv[0].equalsIgnoreCase("-help"))
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
                if (argv[i].equalsIgnoreCase("-queryRequest"))
                {
                    checkIfArgHasValue(argv, i, options);
                    queryRequestArg = true;
                    queryRequest = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-machine"))
                {
                    checkIfArgHasValue(argv, i, options);
                    machine = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-serviceName"))
                {
                    checkIfArgHasValue(argv, i, options);
                    serviceName = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-levelMask"))
                {
                    checkIfArgHasValue(argv, i, options);
                    levelMask = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-fontName"))
                {
                    checkIfArgHasValue(argv, i, options);
                    fontName = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-saveAsDirectory"))
                {
                    saveAsDirectory = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-version"))
                {
                    System.out.println(
                        "\nERROR: Option -version must be specified " +
                        "as the first and only option");
                    System.out.println(helpText);
                    System.exit(1);
                }
                else if (argv[i].equalsIgnoreCase("-help"))
                {
                    System.out.println(
                        "\nERROR: Option -help must be specified as " +
                        "the first and only option");
                    System.out.println(helpText);
                    System.exit(1);
                }
                else
                {
                    System.out.println("Invalid option name: " + argv[i]);
                    System.out.println(helpText);
                    System.exit(1);
                }
            }

            if (!queryRequestArg)
            {
                System.out.println(
                    "\nERROR: Must specify " + minArgsAllowed + " of the " +
                    "following options:  -queryRequest, -help, or -version");
                System.out.println(helpText);
                System.exit(1);
            }
        }

        new STAFLogViewer(null,
                          null,
                          machine,
                          serviceName,
                          queryRequest,
                          levelMask,
                          fontName,
                          saveAsDirectory);
    }
    
    private static void checkIfArgHasValue(String argv[], int i, Set options)
    {
        if (((i+1) > argv.length - 1) ||
            (options.contains(argv[i+1])))
        {
            System.out.println(
                "\nERROR: Option " + argv[i] + " requires a value");
            System.out.println(helpText);
            System.exit(1);
        }
    }

    /**
     * This constructs a viewer for a STAF Log by submitting the specified
     * QUERY request to the STAF LOG service on the local machine.  The
     * viewer will display the messages in the log using the Monospaced font.
     * 
     * @param parent
     * @param handle
     * @param queryRequest
     */ 
    public STAFLogViewer(Component parent,
                         STAFHandle handle,
                         String queryRequest)
    {
        this(parent, handle, "local", "LOG", queryRequest, "",
             "Monospaced", null);
    }

    /**
     * This constructs a viewer for a STAF Log by submitting the specified
     * QUERY request to the STAF LOG service on the specified machine.  The
     * viewer will display the messages in the log using the Monospaced font.
     * 
     * @param parent
     * @param handle
     * @param machine
     * @param queryRequest
     */ 
    public STAFLogViewer(Component parent,
                         STAFHandle handle,
                         String machine,
                         String queryRequest)
    {
        this(parent, handle, machine, "LOG", queryRequest, "",
             "Monospaced", null);
    }

    /**
     * This constructs a viewer for a STAF Log by submitting the specified
     * QUERY request to the specified STAF service on the specified machine.
     * The viewer sill display the messages in the log using the Monospaced
     * font.
     * 
     * @param parent
     * @param handle
     * @param machine
     * @param serviceName
     * @param queryRequest
     */ 
    public STAFLogViewer(Component parent,
                         STAFHandle handle,
                         String machine,
                         String serviceName,
                         String queryRequest)
    {
        this(parent, handle, machine, serviceName, queryRequest, "",
             "Monospaced", null);
    }

    /**
     * This constructs a viewer for a STAF Log by submitting the specified
     * QUERY request to the specified STAF service on the specified machine
     * using the specified level mask.  The viewer sill display the messages
     * in the log using the Monospaced font.
     * 
     * @param parent
     * @param handle
     * @param machine
     * @param serviceName
     * @param queryRequest
     * @param levelMask
     */ 
    public STAFLogViewer(Component parent,
                         STAFHandle handle,
                         String machine,
                         String serviceName,
                         String queryRequest,
                         String levelMask)
    {
        this(parent, handle, machine, serviceName, queryRequest, levelMask,
             "Monospaced", null);
    }

    /**
     * This constructs a viewer for a STAF Log by submitting the specified
     * QUERY request to the specified STAF service on the specified machine
     * using the specified level mask.  The viewer sill display the messages
     * in the log using the specified font.
     * 
     * @param parent
     * @param handle
     * @param machine
     * @param serviceName
     * @param queryRequest
     * @param levelMask
     * @param fontName
     */ 
    public STAFLogViewer(Component parent,
                         STAFHandle handle,
                         String machine,
                         String serviceName,
                         String queryRequest,
                         String levelMask,
                         String fontName)
    {
        this(parent, handle, machine, serviceName, queryRequest, levelMask,
             "Monospaced", null);
    }

    /**
     * This constructs a viewer for a STAF Log by submitting the specified
     * QUERY request to the specified STAF service on the specified machine
     * using the specified level mask.  The viewer sill display the messages
     * in the log using the specified font and will use the specified "Save As
     * Directory" name.
     * 
     * @param parent
     * @param handle
     * @param machine
     * @param serviceName
     * @param queryRequest
     * @param levelMask
     * @param fontName
     * @param saveAsDirectory
     */ 
    public STAFLogViewer(Component parent,
                         STAFHandle handle,
                         String machine,
                         String serviceName,
                         String queryRequest,
                         String levelMask,
                         String fontName,
                         String saveAsDirectory)
    {
        this.parent = parent;

        fMachine = machine;
        fServiceName = serviceName;
        fQueryRequest = queryRequest;
        fLogRequest = "STAF " + fMachine + " " + fServiceName + " " +
            fQueryRequest;
        fLevelMask = levelMask;
        fFontName = fontName;

        if (saveAsDirectory != null)
            fLastSaveAsDirectory = saveAsDirectory;

        STAFResult res;

        // If a handle was specified, don't do a system exit
        if (handle != null)
            fSystemExit = false;

        try
        {
            if (handle == null)
            {
                fHandle = new STAFHandle("STAFLogViewer");
            }
            else
            {
                fHandle = handle;
            }
        }
        catch(STAFException e)
        {
            String errMsg = "Error registering with STAF, RC: " + e.rc;

            displayMessage(errMsg, "Error Registering with STAF", parent);
            //e.printStackTrace();

            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        String frameTitle = "STAF " + fMachine + " " + fServiceName + " " +
            fQueryRequest;

        fLogFrame = new STAFLogFrame(frameTitle);

        Vector logLines = refreshTable(false);

        if (logLines == null)
        {
            if (! fSystemExit)
                return;
            else
                System.exit(0);
        }

        if (logLines.size() == 0)
        {
            String errMsg = "\nThe log has no entries\n\n" +
                fLogFrame.getTitle();

            displayMessage(errMsg, "No Log Entries", parent);

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

    private void displayMessage(String message, String title, Component parent)
    {
        if (parent == null)
        {
            System.out.println(message);
        }
        else
        {
            JOptionPane.showMessageDialog(
                parent, message, title, JOptionPane.INFORMATION_MESSAGE);
        }
    }

    private int verifyLogQueryResult(STAFResult queryResult,
                                      Component parent)
    {
        if (queryResult.rc == 4010)
        {
            String errMsg = "\nThe LOG QUERY request failed with RC 4010 " +
                "because the\nquery criteria selected more records than " +
                "allowed by the\nDefaultMaxQueryRecords setting. " +
                "Use the FIRST <Num> or\nLAST <Num> option to specify " +
                "the number of records or\nthe ALL option if you " +
                "really want all of the records.";

            // Get the defaultMaxQueryRecords setting for the LOG service

            STAFResult logSettingsResult = fHandle.submit2(
                fMachine, fServiceName, "LIST SETTINGS");

            if (logSettingsResult.rc == STAFResult.Ok)
            {
                try
                {
                    Map settingsMap = (Map)logSettingsResult.resultObj;
                    
                    String defaultMaxQueryRecords =
                        (String)settingsMap.get("defaultMaxQueryRecords");
                    
                    errMsg += "\nThe last " + defaultMaxQueryRecords +
                        " log entries will be displayed.";
                }
                catch (Exception e)
                {
                    // Ignore
                }
            }

            // Always want this message displayed as a popup

            if (parent == null)
                displayMessage(errMsg, "RC 4010", new JFrame());
            else
                displayMessage(errMsg, "RC 4010", parent);

            return queryResult.rc;
        }
        else if ((queryResult.rc != 0) &&
                 (queryResult.rc != STAFResult.FileOpenError))
        {
            if (queryResult.rc == STAFResult.DoesNotExist)
            {
                String errMsg = "\nThe log does not exist.  " +
                    "The log query request failed with RC=" +
                    queryResult.rc + ", Result=" + queryResult.result +
                    "\n\n" + fLogRequest;

                displayMessage(errMsg, "Log Does Not Exist", parent); 
            }
            else
            {
                String errMsg = "\nThe log query request failed " +
                    "with RC=" + queryResult.rc +
                    ", Result=" + queryResult.result + "\n\n" +
                    fLogRequest;

                displayMessage(errMsg, "Log Query Request Failed",
                               parent);
            }

            return queryResult.rc;
        }

        // Verify the marshalling context is valid (contains the output
        // from a LOG QUERY request (without the TOTAL or STATS option)

        if ((queryResult.resultContext == null) ||
            (queryResult.resultObj == null))
        {
            String errMsg = "\nThe log query request returned a null " +
                "marshalling context or root. \n\nInvalid query request: " +
                fLogRequest;

            displayMessage(errMsg, "Invalid Log Query Request", parent);

            // Just picked a non-zero return code
            return STAFResult.InvalidValue;
        }

        if ((!queryResult.resultContext.hasMapClassDefinition(
                 STAFLogFormatter.sLOG_MAP_CLASS_DEF) &&
             !queryResult.resultContext.hasMapClassDefinition(
                 STAFLogFormatter.sLOG_LONG_MAP_CLASS_DEF)) ||
             !(queryResult.resultObj instanceof java.util.List))
        {
            String errMsg = "\nThe log query request cannot contain the " +
                "TOTAL or STATS option.\n\nInvalid query request: " +
                fLogRequest;

            displayMessage(errMsg, "Invalid Log Query Request", parent);

            // Just picked a non-zero return code
            return STAFResult.InvalidValue;
        }
        
        return STAFResult.Ok;
    }

    /**
     * This method returns the lines in the output from the LOG QUERY request.
     * 
     * @param  returnEmptyVector  A flag used if the LOG QUERY request contains
     *                            no lines.  If true, it indicates to return an
     *                            return an empty vector if the LOG QUERY request
     *                            returns no data.  If false, it indicates to
     *                            display an error message that says that the
     *                            log has no entries and returns null.
     * @return Returns a Vector containing the lines in the output from a
     *         LOG QUERY request..
     */ 
    public Vector getLogLines(boolean returnEmptyVector)
    {
        java.util.List outputList;

        if (fLevelMask != "")
        {
            // If the LEVELMASK was specified, only do this processing the
            // initial time the log is displayed, since the user may change the
            // selected levels by using the menu bar

            processLevelMask();

            fLevelMask = "";
        }

        fLogFrame.setTitle(fLogRequest);

        try
        {
            STAFResult queryResult = fHandle.submit2(
                fMachine, fServiceName, fQueryRequest + getLogMask());

            int rc = verifyLogQueryResult(queryResult, parent);
            
            if ((rc != STAFResult.Ok) && (rc != 4010))
                return null;

            fLogQueryResultContext = queryResult.resultContext;
            
            // Assign outputList to the log query result object

            outputList = (java.util.List)queryResult.resultObj;
        }
        catch (Exception e)
        {
            String errMsg = "The log has an invalid format";
            displayMessage(errMsg, "Invalid Log Format", parent);
            e.printStackTrace();

            return null;
        }

        if (outputList.size() == 0)
        {
            if (returnEmptyVector)
            {
                return new Vector();
            }
            else
            {
                String errMsg = "\nThe log has no entries\n\n" +
                    fLogRequest;
                displayMessage(errMsg, "No Log Entries", parent);
                return null;
            }
        }

        // Create a vector (logLines) from the outputList

        Iterator iter = outputList.iterator();
        Vector logLines = new Vector();
        int i = 0;

        try
        {
            while (iter.hasNext())
            {
                i++;
                Map logRecord = (Map)iter.next();
                Vector thisLogData = new Vector();

                thisLogData.add((String)logRecord.get("timestamp"));
                thisLogData.add((String)logRecord.get("level"));
                thisLogData.add((String)logRecord.get("message"));
                logLines.add(thisLogData);
            }
        }
        catch (Exception e)
        {
            String errMsg = "Invalid log format in record #" + i;

            displayMessage(errMsg, "Invalid Log Format", parent);

            e.printStackTrace();

            return null;
        }

        return logLines;
    }

    /**
     * Sets the selected log levels based on the level mask.
     */ 
    public void processLevelMask()
    {
        fLevelAll.setSelected(false);
        fLevelFatal.setSelected(
            fLevelMask.toUpperCase().indexOf("FATAL") != -1);
        fLevelError.setSelected(
            fLevelMask.toUpperCase().indexOf("ERROR") != -1);
        fLevelWarning.setSelected(
            fLevelMask.toUpperCase().indexOf("WARNING") != -1);
        fLevelInfo.setSelected(
            fLevelMask.toUpperCase().indexOf("INFO") != -1);
        fLevelTrace.setSelected(
            fLevelMask.toUpperCase().indexOf("TRACE") != -1);
        fLevelTrace2.setSelected(
            fLevelMask.toUpperCase().indexOf("TRACE2") != -1);
        fLevelTrace3.setSelected(
            fLevelMask.toUpperCase().indexOf("TRACE3") != -1);
        fLevelDebug.setSelected(
            fLevelMask.toUpperCase().indexOf("DEBUG") != -1);
        fLevelDebug2.setSelected(
            fLevelMask.toUpperCase().indexOf("DEBUG2") != -1);
        fLevelDebug3.setSelected(
            fLevelMask.toUpperCase().indexOf("DEBUG3") != -1);
        fLevelStart.setSelected(
            fLevelMask.toUpperCase().indexOf("START") != -1);
        fLevelStop.setSelected(
            fLevelMask.toUpperCase().indexOf("STOP") != -1);
        fLevelPass.setSelected(
            fLevelMask.toUpperCase().indexOf("PASS") != -1);
        fLevelFail.setSelected(
            fLevelMask.toUpperCase().indexOf("FAIL") != -1);
        fLevelStatus.setSelected(
            fLevelMask.toUpperCase().indexOf("STATUS") != -1);
        fLevelUser1.setSelected(
            fLevelMask.toUpperCase().indexOf("USER1") != -1);
        fLevelUser2.setSelected(
            fLevelMask.toUpperCase().indexOf("USER2") != -1);
        fLevelUser3.setSelected(
            fLevelMask.toUpperCase().indexOf("USER3") != -1);
        fLevelUser4.setSelected(
            fLevelMask.toUpperCase().indexOf("USER4") != -1);
        fLevelUser5.setSelected(
            fLevelMask.toUpperCase().indexOf("USER5") != -1);
        fLevelUser6.setSelected(
            fLevelMask.toUpperCase().indexOf("USER6") != -1);
        fLevelUser7.setSelected(
            fLevelMask.toUpperCase().indexOf("USER8") != -1);
        fLevelUser8.setSelected(
            fLevelMask.toUpperCase().indexOf("USER8") != -1);
    }

    /**
     * Gets the log mask based on the selected log levels.
     * 
     * @return A string containing the log mask.
     */ 
    public String getLogMask()
    {
        String logMask = "";

        if (fLevelFatal.isSelected())
        {
            logMask += " FATAL";
        }

        if (fLevelError.isSelected())
        {
            logMask += " ERROR";
        }

        if (fLevelWarning.isSelected())
        {
            logMask += " WARNING";
        }

        if (fLevelInfo.isSelected())
        {
            logMask += " INFO";
        }

        if (fLevelTrace.isSelected())
        {
            logMask += " TRACE";
        }

        if (fLevelTrace2.isSelected())
        {
            logMask += " TRACE2";
        }

        if (fLevelTrace3.isSelected())
        {
            logMask += " TRACE3";
        }

        if (fLevelDebug.isSelected())
        {
            logMask += " DEBUG";
        }

        if (fLevelDebug2.isSelected())
        {
            logMask += " DEBUG2";
        }

        if (fLevelDebug3.isSelected())
        {
            logMask += " DEBUG3";
        }

        if (fLevelStart.isSelected())
        {
            logMask += " START";
        }

        if (fLevelStop.isSelected())
        {
            logMask += " STOP";
        }

        if (fLevelPass.isSelected())
        {
            logMask += " PASS";
        }

        if (fLevelFail.isSelected())
        {
            logMask += " FAIL";
        }

        if (fLevelStatus.isSelected())
        {
            logMask += " STATUS";
        }

        if (fLevelUser1.isSelected())
        {
            logMask += " USER1";
        }

        if (fLevelUser2.isSelected())
        {
            logMask += " USER2";
        }

        if (fLevelUser3.isSelected())
        {
            logMask += " USER3";
        }

        if (fLevelUser4.isSelected())
        {
            logMask += " USER4";
        }

        if (fLevelUser5.isSelected())
        {
            logMask += " USER5";
        }

        if (fLevelUser6.isSelected())
        {
            logMask += " USER6";
        }

        if (fLevelUser7.isSelected())
        {
            logMask += " USER7";
        }

        if (fLevelUser8.isSelected())
        {
            logMask += " USER8";
        }

        if (!(logMask.equals("")))
        {
            logMask = " LEVELMASK" + logMask;
        }
        else
        {
            logMask = " LEVELMASK" + " 00000000000000000000000000000000";
        }

        return logMask;
    }

    /**
     * This method determines the number of times a string exists in another
     * string.
     * 
     * @param str    A string that will be searched for the number of times the
     *               string specified by the <code>match</code> parameter is
     *               found in it.
     * @param match  A string containing what to look for in the <code>str</code>
     *               parameter.
     * @return A count of the number of occurrences of the specified match
     *         string that exist in the specified str.
     */ 
    public int occurrences(String str, String match)
    {
        int count = 0;
        int index =  0;

        while (index != -1)
        {
            index = str.indexOf(match, index + 1);
            if (index > -1)
                count++;
        }

        return count;
    }

    /**
     * This method determines if all the log levels are selected.
     * 
     * @return Returns true if all the log levels are selected or false if all
     *         the log levels are not selected.
     */ 
    public boolean allLevelsSelected()
    {
        if (fLevelFatal.isSelected() &&
            fLevelError.isSelected() &&
            fLevelWarning.isSelected() &&
            fLevelInfo.isSelected() &&
            fLevelTrace.isSelected() &&
            fLevelTrace2.isSelected() &&
            fLevelTrace3.isSelected() &&
            fLevelDebug.isSelected() &&
            fLevelDebug2.isSelected() &&
            fLevelDebug3.isSelected() &&
            fLevelStart.isSelected() &&
            fLevelStop.isSelected() &&
            fLevelPass.isSelected() &&
            fLevelFail.isSelected() &&
            fLevelStatus.isSelected() &&
            fLevelUser1.isSelected() &&
            fLevelUser2.isSelected() &&
            fLevelUser3.isSelected() &&
            fLevelUser4.isSelected() &&
            fLevelUser5.isSelected() &&
            fLevelUser6.isSelected() &&
            fLevelUser7.isSelected() &&
            fLevelUser8.isSelected())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * This method refreshes the table containing the output from the LOG QUERY
     * request and returns the lines in the output from the LOG QUERY request.
     * 
     * @param  returnEmptyVector  A flag used if the LOG QUERY request contains
     *                            no lines.  If true, it indicates to return an
     *                            return an empty vector if the LOG QUERY request
     *                            returns no data.  If false, it indicates to
     *                            display an error message that says that the
     *                            log has no entries and returns null.
     * @return Returns a Vector containing the lines in the output from a
     *         LOG QUERY request. 
     */ 
    public Vector refreshTable(boolean returnEmptyVector)
    {
        Vector logLines = getLogLines(returnEmptyVector);
        if (logLines == null) return null;

        fLogTable.setRowHeight(30);
        fLogTable.setModel(new STAFTableModel(logLines, columnNames));

        updateLogTableRenderers();
        updateRowHeights(fLogTable, 2, fFontName);
        sizeColumnsToFitText(fLogTable);

        return logLines;
    }

    /**
     * This method updates the log table cell renderers.
     */ 
    public void updateLogTableRenderers()
    {
        fLogTable.getColumnModel().getColumn(0).setHeaderRenderer(
            new STAFLogTableCellRenderer(true));

        fLogTable.getColumnModel().getColumn(1).setHeaderRenderer(
            new STAFLogTableCellRenderer(true));

        fLogTable.getColumnModel().getColumn(2).setHeaderRenderer(
            new STAFLogTableCellRenderer(true));

        fLogTable.getColumnModel().getColumn(0).setCellRenderer(
            new STAFLogTableCellRenderer(false));

        fLogTable.getColumnModel().getColumn(1).setCellRenderer(
            new STAFLogTableCellRenderer(false));

        fLogTable.getColumnModel().getColumn(2).setCellRenderer(
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
                {
                    maxWidth = width;
                }
            }

            Insets insets =
                ((JComponent)table.getCellRenderer(0,i)).getInsets();
                // need to pad a little extra for everything to look right
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

            fFileSaveAsTextMenuItem = new JMenuItem("Save As Text...");
            fFileSaveAsTextMenuItem.addActionListener(this);
            fFileMenu.add(fFileSaveAsTextMenuItem);
            
            fFileSaveAsHtmlMenuItem = new JMenuItem("Save As Html...");
            fFileSaveAsHtmlMenuItem.addActionListener(this);
            fFileMenu.add(fFileSaveAsHtmlMenuItem);
            
            fFileMenu.insertSeparator(2);

            fFileExit = new JMenuItem("Exit");
            fFileExit.addActionListener(this);
            fFileMenu.add(fFileExit);

            fViewRefresh = new JMenuItem("Refresh");
            fViewRefresh.addActionListener(this);
            fViewMenu.add(fViewRefresh);
            fViewChangeFont = new JMenuItem("Change Font...");
            fViewChangeFont.addActionListener(this);
            fViewMenu.add(fViewChangeFont);

            fChangeFontDialog = new JDialog(this, "Change Font", true);
            fChangeFontDialog.setSize(220, 120);
            JPanel changeFontPanel = new JPanel();
            changeFontPanel.setLayout(new BorderLayout());
            changeFontPanel.setBorder(new TitledBorder("Select Font"));
            fChangeFontDialog.getContentPane().add(changeFontPanel);

            fFontMenuItems = new Vector();
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

            fLevels = new JMenu("Levels");
            mainMenuBar.add(fLevels);
            fLevelAll = new JCheckBoxMenuItem("All", true);
            fLevelAll.addActionListener(this);
            fLevels.add(fLevelAll);
            fLevels.addSeparator();

            fLevelFatal = new JCheckBoxMenuItem("Fatal", true);
            fLevelFatal.addActionListener(this);
            fLevels.add(fLevelFatal);
            fLevelError = new JCheckBoxMenuItem("Error", true);
            fLevelError.addActionListener(this);
            fLevels.add(fLevelError);
            fLevelWarning = new JCheckBoxMenuItem("Warning", true);
            fLevelWarning.addActionListener(this);
            fLevels.add(fLevelWarning);
            fLevelInfo = new JCheckBoxMenuItem("Info", true);
            fLevelInfo.addActionListener(this);
            fLevels.add(fLevelInfo);
            fLevelTrace = new JCheckBoxMenuItem("Trace", true);
            fLevelTrace.addActionListener(this);
            fLevels.add(fLevelTrace);
            fLevelTrace2 = new JCheckBoxMenuItem("Trace2", true);
            fLevelTrace2.addActionListener(this);
            fLevels.add(fLevelTrace2);
            fLevelTrace3 = new JCheckBoxMenuItem("Trace3", true);
            fLevelTrace3.addActionListener(this);
            fLevels.add(fLevelTrace3);
            fLevelDebug = new JCheckBoxMenuItem("Debug", true);
            fLevelDebug.addActionListener(this);
            fLevels.add(fLevelDebug);
            fLevelDebug2 = new JCheckBoxMenuItem("Debug2", true);
            fLevelDebug2.addActionListener(this);
            fLevels.add(fLevelDebug2);
            fLevelDebug3 = new JCheckBoxMenuItem("Debug3", true);
            fLevelDebug3.addActionListener(this);
            fLevels.add(fLevelDebug3);
            fLevelStart = new JCheckBoxMenuItem("Start", true);
            fLevelStart.addActionListener(this);
            fLevels.add(fLevelStart);
            fLevelStop = new JCheckBoxMenuItem("Stop", true);
            fLevelStop.addActionListener(this);
            fLevels.add(fLevelStop);
            fLevelPass = new JCheckBoxMenuItem("Pass", true);
            fLevelPass.addActionListener(this);
            fLevels.add(fLevelPass);
            fLevelFail = new JCheckBoxMenuItem("Fail", true);
            fLevelFail.addActionListener(this);
            fLevels.add(fLevelFail);
            fLevelStatus = new JCheckBoxMenuItem("Status", true);
            fLevelStatus.addActionListener(this);
            fLevels.add(fLevelStatus);
            fLevelUser1 = new JCheckBoxMenuItem("User1", true);
            fLevelUser1.addActionListener(this);
            fLevels.add(fLevelUser1);
            fLevelUser2 = new JCheckBoxMenuItem("User2", true);
            fLevelUser2.addActionListener(this);
            fLevels.add(fLevelUser2);
            fLevelUser3 = new JCheckBoxMenuItem("User3", true);
            fLevelUser3.addActionListener(this);
            fLevels.add(fLevelUser3);
            fLevelUser4 = new JCheckBoxMenuItem("User4", true);
            fLevelUser4.addActionListener(this);
            fLevels.add(fLevelUser4);
            fLevelUser5 = new JCheckBoxMenuItem("User5", true);
            fLevelUser5.addActionListener(this);
            fLevels.add(fLevelUser5);
            fLevelUser6 = new JCheckBoxMenuItem("User6", true);
            fLevelUser6.addActionListener(this);
            fLevels.add(fLevelUser6);
            fLevelUser7 = new JCheckBoxMenuItem("User7", true);
            fLevelUser7.addActionListener(this);
            fLevels.add(fLevelUser7);
            fLevelUser8 = new JCheckBoxMenuItem("User8", true);
            fLevelUser8.addActionListener(this);
            fLevels.add(fLevelUser8);

            columnNames = new Vector();
            columnNames.add(new String("Date-Time"));
            columnNames.add(new String("Level"));
            columnNames.add(new String("Message"));

            fLogTable = new JTable();

            fLogTable.setFont(new Font(fFontName, Font.PLAIN, 12));
            fAvailableFonts.setSelectedItem(fFontName);
            fLogTable.setRowSelectionAllowed(true);
            fLogTable.setColumnSelectionAllowed(false);
            fLogTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

            updateRowHeights(fLogTable, 2, fFontName);
            sizeColumnsToFitText(fLogTable);

            JScrollPane logScroll = new JScrollPane(fLogTable);
            getContentPane().add(logScroll);

            addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
            {
                    if (! fSystemExit)
                        dispose();
                    else
                    {
                        System.exit(0);
                    }
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
            else if ((e.getSource() == fFileSaveAsTextMenuItem) ||
                     (e.getSource() == fFileSaveAsHtmlMenuItem))
            {
                // Display dialog to select file to save log in

                JFileChooser fileChooser = new JFileChooser(
                    fLastSaveAsDirectory);

                if (e.getSource() == fFileSaveAsTextMenuItem)
                {
                    // Set filter to TEXT (*.txt) files

                    FileFilter fileFilter = new TextFileFilter();
                    fileChooser.setFileFilter(fileFilter);
                }
                else if (e.getSource() == fFileSaveAsHtmlMenuItem)
                {
                    // Set filter to HTML (*.html and *.htm) files

                    FileFilter fileFilter = new HtmlFileFilter();
                    fileChooser.setFileFilter(fileFilter);
                }

                fileChooser.setDialogTitle("Save Log As");
                int rc = fileChooser.showSaveDialog(this);

                if (rc == JFileChooser.APPROVE_OPTION)
                {
                    try
                    {
                        fLastSaveAsDirectory = fileChooser.
                            getCurrentDirectory().toString();

                        String fileName = fileChooser.getSelectedFile().
                            getCanonicalPath();

                        if (e.getSource() == fFileSaveAsTextMenuItem)
                            saveAsFile("text", fileName);
                        else
                            saveAsFile("html", fileName);
                    }
                    catch (IOException ex)
                    {
                        ex.printStackTrace();
                    }
                }
            }
            else if (e.getSource() == fViewRefresh)
            {
                refreshTable(true);
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
                updateRowHeights(fLogTable, 2, fFontName);
                sizeColumnsToFitText(fLogTable);
                fChangeFontDialog.setVisible(false);
            }
            else if (e.getSource() == fChangeFontCancelButton)
            {
                fChangeFontDialog.setVisible(false);
            }
            else if ((e.getSource() == fLevelAll) &&
                fLevelAll.isSelected())
            {
                fLevelFatal.setSelected(true);
                fLevelError.setSelected(true);
                fLevelWarning.setSelected(true);
                fLevelInfo.setSelected(true);
                fLevelTrace.setSelected(true);
                fLevelTrace2.setSelected(true);
                fLevelTrace3.setSelected(true);
                fLevelDebug.setSelected(true);
                fLevelDebug2.setSelected(true);
                fLevelDebug3.setSelected(true);
                fLevelStart.setSelected(true);
                fLevelStop.setSelected(true);
                fLevelPass.setSelected(true);
                fLevelFail.setSelected(true);
                fLevelStatus.setSelected(true);
                fLevelUser1.setSelected(true);
                fLevelUser2.setSelected(true);
                fLevelUser3.setSelected(true);
                fLevelUser4.setSelected(true);
                fLevelUser5.setSelected(true);
                fLevelUser6.setSelected(true);
                fLevelUser7.setSelected(true);
                fLevelUser8.setSelected(true);

                refreshTable(true);
            }
            else if ((e.getSource() == fLevelAll) &&
                !(fLevelAll.isSelected()))
            {
                fLevelFatal.setSelected(false);
                fLevelError.setSelected(false);
                fLevelWarning.setSelected(false);
                fLevelInfo.setSelected(false);
                fLevelTrace.setSelected(false);
                fLevelTrace2.setSelected(false);
                fLevelTrace3.setSelected(false);
                fLevelDebug.setSelected(false);
                fLevelDebug2.setSelected(false);
                fLevelDebug3.setSelected(false);
                fLevelStart.setSelected(false);
                fLevelStop.setSelected(false);
                fLevelPass.setSelected(false);
                fLevelFail.setSelected(false);
                fLevelStatus.setSelected(false);
                fLevelUser1.setSelected(false);
                fLevelUser2.setSelected(false);
                fLevelUser3.setSelected(false);
                fLevelUser4.setSelected(false);
                fLevelUser5.setSelected(false);
                fLevelUser6.setSelected(false);
                fLevelUser7.setSelected(false);
                fLevelUser8.setSelected(false);

                refreshTable(true);
            }
            else if ((e.getSource() == fLevelFatal))
            {
                refreshTable(true);

                if (!(fLevelFatal.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelError))
            {
                refreshTable(true);

                if (!(fLevelError.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelWarning))
            {
                refreshTable(true);

                if (!(fLevelWarning.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelInfo))
            {
                refreshTable(true);

                if (!(fLevelInfo.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelTrace))
            {
                refreshTable(true);

                if (!(fLevelTrace.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelTrace2))
            {
                refreshTable(true);

                if (!(fLevelTrace2.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelTrace3))
            {
                refreshTable(true);

                if (!(fLevelTrace3.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelDebug))
            {
                refreshTable(true);

                if (!(fLevelDebug.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelDebug2))
            {
                refreshTable(true);

                if (!(fLevelDebug2.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelDebug3))
            {
                refreshTable(true);

                if (!(fLevelDebug3.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelStart))
            {
                refreshTable(true);

                if (!(fLevelStart.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelStop))
            {
                refreshTable(true);

                if (!(fLevelStop.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelPass))
            {
                refreshTable(true);

                if (!(fLevelPass.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelFail))
            {
                refreshTable(true);

                if (!(fLevelFail.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelStatus))
            {
                refreshTable(true);

                if (!(fLevelStatus.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser1))
            {
                refreshTable(true);

                if (!(fLevelUser1.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser2))
            {
                refreshTable(true);

                if (!(fLevelUser2.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser3))
            {
                refreshTable(true);

                if (!(fLevelUser3.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser4))
            {
                refreshTable(true);

                if (!(fLevelUser4.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser5))
            {
                refreshTable(true);

                if (!(fLevelUser5.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser6))
            {
                refreshTable(true);

                if (!(fLevelUser6.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser7))
            {
                refreshTable(true);

                if (!(fLevelUser7.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
            else if ((e.getSource() == fLevelUser8))
            {
                refreshTable(true);

                if (!(fLevelUser8.isSelected()))
                {
                    fLevelAll.setSelected(false);
                }
                else if (allLevelsSelected())
                {
                    fLevelAll.setSelected(true);
                }
            }
        }

        private void saveAsFile(String type, String fileName)
        {
            try
            {
                STAFLogFormatter logFormatter = new STAFLogFormatter(
                    fLogQueryResultContext);

                logFormatter.setFileName(fileName);
                logFormatter.setFontName(fFontName);

                if (!type.equals("text"))
                    logFormatter.setTitle(fLogRequest);
                
                logFormatter.format(type);
            }
            catch (Exception e)
            {
                String errMsg = "Saving the log as a " + type +
                    " file failed.\n" + e.toString();

                displayMessage(errMsg, "Cannot Save Log", parent);

                return;
            }
        }
    }

    public class TextFileFilter extends FileFilter
    {
        public boolean accept(File file)
        {
            if (file == null) return false;

            if (file.isDirectory()) return true;

            String fileName = file.getName();
            int i = fileName.lastIndexOf('.');

            if (i > 0 && i < fileName.length() - 1)
            {
                String extension = fileName.substring(i + 1).toLowerCase();

                if (extension.equals("txt"))
                    return true;
            }

            return false;
        }

        public String getDescription()
        {
            return "TEXT (*.txt)";
        }
    }

    public class HtmlFileFilter extends FileFilter
    {
        public boolean accept(File file)
        {
            if (file == null) return false;

            if (file.isDirectory()) return true;
                
            String fileName = file.getName();
            int i = fileName.lastIndexOf('.');
            
            if (i > 0 && i < fileName.length() - 1)
            {
                String extension = fileName.substring(i + 1).toLowerCase();
                    
                if (extension.equals("html") || extension.equals("htm"))
                    return true;
            }
            
            return false;
        }
        
        public String getDescription()
        {
            return "HTML (*.htm, *.html)";
        }
    }

    public class STAFLogTableCellRenderer
             extends JTextArea implements TableCellRenderer
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

public class STAFTableSorter extends STAFTableMap
{
    int indexes[];
    Vector sortingColumns = new Vector();
    boolean ascending = true;
    int compares;
    int sortColumn = -1; // by default, don't sort on any column
    String fontName = "Dialog";

    public STAFTableSorter()
    {
        indexes = new int[0]; // for consistency
    }

    public STAFTableSorter(STAFTableModel model)
    {
        setModel(model);
    }

    public STAFTableSorter(STAFTableModel model, int column)
    {
        setModel(model);
        sortColumn = column;
    }

    public STAFTableSorter(STAFTableModel model, int column,
                            String theFontName)
    {
        setModel(model);
        sortColumn = column;
        fontName = theFontName;
    }

    public void setModel(STAFTableModel model)
    {
        super.setModel(model);
        reallocateIndexes();
    }

    public int compareRowsByColumn(int row1, int row2, int column)
    {
        Class type = model.getColumnClass(column);
        TableModel data = model;

        // Check for nulls.

        Object o1 = data.getValueAt(row1, column);
        Object o2 = data.getValueAt(row2, column);

        // If both values are null, return 0.
        if (o1 == null && o2 == null)
        {
            return 0;
        }
        else if (o1 == null)
        { // Define null less than everything.
            return -1;
        }
        else if (o2 == null)
        {
            return 1;
        }

        if (type.getSuperclass() == java.lang.Number.class)
        {
            Number n1 = (Number)data.getValueAt(row1, column);
            double d1 = n1.doubleValue();
            Number n2 = (Number)data.getValueAt(row2, column);
            double d2 = n2.doubleValue();

            if (d1 < d2)
            {
                return -1;
            }
            else if (d1 > d2)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (type == java.util.Date.class)
        {
            Date d1 = (Date)data.getValueAt(row1, column);
            long n1 = d1.getTime();
            Date d2 = (Date)data.getValueAt(row2, column);
            long n2 = d2.getTime();

            if (n1 < n2)
            {
                return -1;
            }
            else if (n1 > n2)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (type == String.class)
        {
            String s1 = (String)data.getValueAt(row1, column);
            String s2    = (String)data.getValueAt(row2, column);
            int result = s1.compareTo(s2);

            if (result < 0)
            {
                return -1;
            }
            else if (result > 0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (type == Boolean.class)
        {
            Boolean bool1 = (Boolean)data.getValueAt(row1, column);
            boolean b1 = bool1.booleanValue();
            Boolean bool2 = (Boolean)data.getValueAt(row2, column);
            boolean b2 = bool2.booleanValue();

            if (b1 == b2)
            {
                return 0;
            }
            else if (b1)
            { // Define false < true
                return 1;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            Object v1 = data.getValueAt(row1, column);
            String s1 = v1.toString();
            Object v2 = data.getValueAt(row2, column);
            String s2 = v2.toString();
            int result = s1.compareTo(s2);

            if (result < 0)
            {
                return -1;
            }
            else if (result > 0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    public int compare(int row1, int row2)
    {
        compares++;
        for (int level = 0; level < sortingColumns.size(); level++)
        {
            Integer column = (Integer)sortingColumns.elementAt(level);
            int result = compareRowsByColumn(row1, row2, column.intValue());
            if (result != 0)
            {
                return ascending ? result : -result;
            }
        }
        return 0;
    }

    public void reallocateIndexes()
    {
        int rowCount = model.getRowCount();


        indexes = new int[rowCount];

        for (int row = 0; row < rowCount; row++)
        {
            indexes[row] = row;
        }
    }

    public void tableChanged(TableModelEvent e)
    {
        if (e.getType() != TableModelEvent.UPDATE)
        {
            reallocateIndexes();

            if (sortColumn >= 0)
            {
                sortByColumn(sortColumn);
            }

            super.tableChanged(e);
        }
    }

    public void checkModel()
    {
        if (indexes.length != model.getRowCount())
        {
            System.err.println("Sorter not informed of a change in model.");
        }
    }

    public void sort(Object sender)
    {
        checkModel();

        compares = 0;
        shuttlesort((int[])indexes.clone(), indexes, 0, indexes.length);
    }

    public void n2sort()
    {
        for (int i = 0; i < getRowCount(); i++)
        {
            for (int j = i+1; j < getRowCount(); j++)
            {
                if (compare(indexes[i], indexes[j]) == -1)
                {
                    swap(i, j);
                }
            }
        }
    }

    public void shuttlesort(int from[], int to[], int low, int high)
    {
        if (high - low < 2)
        {
            return;
        }

        int middle = (low + high)/2;
        shuttlesort(to, from, low, middle);
        shuttlesort(to, from, middle, high);

        int p = low;
        int q = middle;

        if (high - low >= 4 && compare(from[middle-1], from[middle]) <= 0)
        {
            for (int i = low; i < high; i++)
            {
                to[i] = from[i];
            }
            return;
        }

        // A normal merge.

        for (int i = low; i < high; i++)
        {
            if (q >= high || (p < middle && compare(from[p], from[q]) <= 0))
            {
                to[i] = from[p++];
            }
            else
            {
                to[i] = from[q++];
            }
        }
    }

    public void swap(int i, int j)
    {
        int tmp = indexes[i];
        indexes[i] = indexes[j];
        indexes[j] = tmp;
    }

    public Object getValueAt(int aRow, int aColumn)
    {
        checkModel();
        return model.getValueAt(indexes[aRow], aColumn);
    }

    public void setValueAt(Object aValue, int aRow, int aColumn)
    {
        checkModel();
        model.setValueAt(aValue, indexes[aRow], aColumn);

        fireTableChanged(new TableModelEvent(this, aRow, aRow, aColumn));
    }

    public void sortByColumn(int column)
    {
        sortByColumn(column, true);
    }

    public void sortByColumn(int column, boolean ascending)
    {
        this.ascending = ascending;
        sortingColumns.removeAllElements();
        sortingColumns.addElement(new Integer(column));
        sortColumn = column;
        sort(this);
        super.tableChanged(new TableModelEvent(this));
    }

    public int map(int row)
    {
        // XXX: is there a better way to find the sorted index?
        Vector rowVector = new Vector();
        for (int i = 0; i < indexes.length; i++)
        {
            rowVector.addElement(new Integer(indexes[i]));
        }
        return rowVector.indexOf(new Integer(row));
    }

    public void addMouseListenerToHeaderInTable(JTable table, int column)
    {
        final STAFTableSorter sorter = this;
        final JTable tableView = table;
        final int multiLineColumn = column;
        tableView.setColumnSelectionAllowed(false);
        MouseAdapter listMouseListener = new MouseAdapter()
        {
            public void mouseClicked(MouseEvent e)
            {
                synchronized (tableView)
                {
                    TableColumnModel columnModel = tableView.getColumnModel();
                    int viewColumn = columnModel.getColumnIndexAtX(e.getX());
                    int column =
                        tableView.convertColumnIndexToModel(viewColumn);
                    if (e.getClickCount() == 1 && column != -1)
                    {
                        int shiftPressed =
                            e.getModifiers()&InputEvent.SHIFT_MASK;
                        boolean ascending = (shiftPressed == 0);
                        sorter.sortByColumn(column, ascending);

                        tableView.setModel(sorter);

                        updateRowHeights(tableView, multiLineColumn, fontName);
                    }
                }
            }
        };
        JTableHeader th = tableView.getTableHeader();
        th.addMouseListener(listMouseListener);
    }
}

    private STAFHandle fHandle;
    private String fMachine = "local";
    private String fServiceName = "LOG";
    private JTable fLogTable;
    private STAFMarshallingContext fLogQueryResultContext;
    private String fQueryRequest = "";
    private String fLogRequest = "";
    private String fLevelMask = "";
    private String fFontName = "Monospaced";
    private String fLastSaveAsDirectory = null;

    boolean fSystemExit = true;

    STAFTableModel fLogTableModel;
    STAFTableSorter fLogModelSorter;

    String fStartDateTime = null;
    Vector columnNames;

    STAFLogFrame fLogFrame;
    Component parent;
    JMenu fFileMenu;
    JMenu fViewMenu;
    JMenuItem fFileExit;
    JMenuItem fFileSaveAsTextMenuItem;
    JMenuItem fFileSaveAsHtmlMenuItem;
    JMenuItem fViewRefresh;
    JMenuItem fViewChangeFont;
    JDialog fChangeFontDialog;
    JComboBox fAvailableFonts;
    JButton fChangeFontOkButton;
    JButton fChangeFontCancelButton;
    Vector fFontMenuItems;
    JMenu fLevels;
    JCheckBoxMenuItem fLevelAll;
    JCheckBoxMenuItem fLevelFatal;
    JCheckBoxMenuItem fLevelError;
    JCheckBoxMenuItem fLevelWarning;
    JCheckBoxMenuItem fLevelInfo;
    JCheckBoxMenuItem fLevelTrace;
    JCheckBoxMenuItem fLevelTrace2;
    JCheckBoxMenuItem fLevelTrace3;
    JCheckBoxMenuItem fLevelDebug;
    JCheckBoxMenuItem fLevelDebug2;
    JCheckBoxMenuItem fLevelDebug3;
    JCheckBoxMenuItem fLevelStart;
    JCheckBoxMenuItem fLevelStop;
    JCheckBoxMenuItem fLevelPass;
    JCheckBoxMenuItem fLevelFail;
    JCheckBoxMenuItem fLevelStatus;
    JCheckBoxMenuItem fLevelUser1;
    JCheckBoxMenuItem fLevelUser2;
    JCheckBoxMenuItem fLevelUser3;
    JCheckBoxMenuItem fLevelUser4;
    JCheckBoxMenuItem fLevelUser5;
    JCheckBoxMenuItem fLevelUser6;
    JCheckBoxMenuItem fLevelUser7;
    JCheckBoxMenuItem fLevelUser8;
}