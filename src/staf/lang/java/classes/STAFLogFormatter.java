/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;
import com.ibm.staf.service.*;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.FileNotFoundException;

/**
 * This class allows you to format a STAF log (which is a binary file that has
 * been created by the STAF Log service) as html or text.
 * <p>
 * When run as an application, it submits the specified Log query request to
 * query a STAF log on any machine currently running STAF and then formats the
 * output as either html or text. Or, when run via a Java program that has
 * already submitted a LOG QUERY request, you can use the STAFLogFormatter 
 * class to format the log query result as either html or text. You can specify various options including whether you want
 * the formatted output written to a file.
 * <p>
 * Two format types are supported:
 * <p>
 * <ul>
 * <li><code>text</code> - Formats the log as text. For each log record, the
 * timestamp, level, and message fields are written in a single line with the
 * fields separated by a space. If a title is specified, the title is written
 * on the first line followed by a blank line before the log output. For
 * example, if title "STAF local LOG QUERY GLOBAL LOGNAME test1 LAST 3" is
 * specified, the log output formatted as text could look like:
 * <p>
 * STAF local LOG QUERY GLOBAL LOGNAME test1 LAST 3
 * <p>
 * 20081205-13:54:42 Info Step 1 of ScenarioB completed<br>
 * 20081205-13:54:45 Info Step 2 of ScenarioB completed<br>
 * 20081205-13:54:54 Pass Scenario completed
 * <p>
 * <li><code>html</code> - Formats the log as html. A html document is created
 * with the body containing a table with three columns (timestamp, level, and
 * message). Each row in the table contains the timestamp, level, and message
 * for a log record. The courier font is used for the html document unless you
 * set a different font. If a title is provided, a header with the title will
 * be written at the beginning of the html document.
 * </ul>
 * 
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFLogFormatter">
 *      Section "3.6.3 Class STAFLogFormatter" in the STAF Java User's Guide</a>
 * @since STAF V3.3.2
 */
public class STAFLogFormatter
{
    public static final String sLOG_MAP_CLASS_DEF = "STAF/Service/Log/LogRecord";
    public static final String sLOG_LONG_MAP_CLASS_DEF =
        "STAF/Service/Log/LogRecordLong";
    
    private static String helpText = "\nSTAFLogFormatter Help\n\n" +
        "Description:\n" +
        "\n" +
        "  The STAFLogFormatter queries a STAF log on any machine currently running\n" +
        "  STAF and formats the output as either html or text.\n" +
        "\n" +
        "Options:\n" +
        "\n" +
        "  -queryRequest <LOG QUERY Request>\n" +
        "  -fileName <File Name>\n" +
        "  -type <html | text>\n" +
        "  -fontName <Font Name>\n" +
        "  -title <Title>\n" +
        "  -machine <Log Service Machine Name>\n" +
        "  -serviceName <Log Service Name>\n" +
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
        "4) The -fileName option specifies the fully-qualified name of a file where the\n" +
        "   formatted log output will be written.  If not specified, the formatted log\n" +
        "   output is written to Stdout.\n" +
        "5) The -type option specifies the format type.  Valid values are: html or\n" +
        "   text.  If not specified, it defaults to html.\n" +
        "6) The -fontName option specifies the font to use.  If not specified, it\n" +
        "   defaults to courier (Monospaced).  It only has an effect if formatting as\n" +
        "   type html.\n" +
        "7) The -title option specifies a title for the log output.  If not\n" +
        "   specified, it defaults to the log query request if using type html or to\n" +
        "   no title if using type text.\n" +
        "8) The -machine option specifies the machine where the LOQ QUERY request\n" +
        "   will be submitted.  If not specified, it defaults to local.\n" +
        "9) The -serviceName option specifies the name of the Log service to which\n" +
        "   the query request will be submitted.  If not specified, it defaults to LOG.\n" +
        "\n" +
        "Examples: (Enter each as a single command)\n" +
        "\n" +
        "1) java com.ibm.staf.STAFLogFormatter -fileName /logs/test1.html -queryRequest\n" +
        "   \"QUERY GLOBAL LOGNAME test1 ALL\"\n" +
        "2) java com.ibm.staf.STAFLogFormatter -machine awesvc3 -queryRequest\n" +
        "   \"QUERY GLOBAL LOGNAME CIS LAST 50\"\n" +
        "3) java com.ibm.staf.STAFLogViewer  -fileName C:\\logs\\email.txt -type text\n" +
        "   -queryRequest \"QUERY MACHINE client1.company.com LOGNAME email LAST 100\"\n" +
        "4) java com.ibm.staf.STAFLogViewer -fileName /logs/job_1.html -queryRequest\n" +
        "   \"QUERY MACHINE {STAF/Config/MachineNickname} LOGNAME STAX_Job_1 ALL\"";
        
    private static String kVersion = "3.0.0";

    private static int TYPE_TEXT = 1;
    private static int TYPE_HTML = 2;
    private static String FIELD_SEPARATOR = " ";

    private STAFMarshallingContext fContext = null;
    private String fFileName = null;
    private String fFontName = "courier";
    private String fTitle = null;
    private String fLineSep = "";

    private StringBuffer fOutputString = new StringBuffer();
    private BufferedWriter fFileWriter = null;
    
    /**
     * This constructs a formatter for a STAF Log.
     * 
     * @param context The STAF marshalling context containing the output from
     *                a STAF Log QUERY request.
     */ 
    public STAFLogFormatter(STAFMarshallingContext context) throws STAFException
    {
        // Verify the marshalling context is valid (contains the output
        // from a LOG QUERY request (without the TOTAL or STATS option)

        if (context == null)
        {
            throw new STAFException(
                STAFResult.InvalidValue, "Marshalling context cannot be null");
        }

        if (!context.hasMapClassDefinition(sLOG_MAP_CLASS_DEF) &&
            !context.hasMapClassDefinition(sLOG_LONG_MAP_CLASS_DEF))
        {
            throw new STAFException(
                STAFResult.InvalidValue,
                "Marshalling context does not contain " +
                "the " + sLOG_MAP_CLASS_DEF + " or " +
                sLOG_LONG_MAP_CLASS_DEF + " map class definition. " +
                "Make sure the log query request does not contain the " +
                "TOTAL or STATS option.");
        }

        if (context.getRootObject() == null)
        {
            throw new STAFException(
                STAFResult.InvalidValue,
                "Marshalling context's root object cannot be null");
        }

        if (!(context.getRootObject() instanceof List))
        {
            throw new STAFException(
                STAFResult.InvalidValue,
                "Marshalling context's root object is not a list. " +
                "Make sure the log query request does not contain the " +
                "TOTAL or STATS option.");
        }

        fContext = context;
        fLineSep = System.getProperty("line.separator");
    }

    public void setFileName(String fileName)
    {
        fFileName = fileName;
    }
    
    // setFontName does not have any effect if formatting as text

    public void setFontName(String fontName)
    {
        fFontName = fontName;
    }
    
    public void setTitle(String title)
    {
        fTitle = title;
    }

    public String format(String typeString) throws IOException, STAFException 
    {
        // Verify the type specified is valid

        int type;

        if (typeString.equalsIgnoreCase("text"))
        {
            type = TYPE_TEXT;
        }
        else if (typeString.equalsIgnoreCase("html"))
        {
            type = TYPE_HTML;
        }
        else
        {
            throw new STAFException(
                STAFResult.InvalidValue,
                "Invalid value for the type option: " + typeString +
                "\nValid values are text or html, case-insensitive");
        }

        // If a file name was set, create a file writer

        fFileWriter = null;

        if (fFileName != null)
            fFileWriter = new BufferedWriter(new FileWriter(fFileName));

        try
        {
            writeHeader(type);
            writeBody(type);
            writeFooter(type);
        }
        finally
        {
            if (fFileWriter != null)
            {
                try
                {
                    fFileWriter.close();
                }
                catch (Exception ioe)
                { /* Do nothing */ }
            }
        }
        
        String result = new String("");

        if (fFileName == null)
        {
             // No file name was specified so return the formatted output in
             // the result
             
             result = fOutputString.toString();
        }
        
        return result;
    }

    private void appendLine(String line) throws IOException
    {
        if (fFileName == null)
        {
            fOutputString.append(line).append(fLineSep);
        }
        else
        {
            fFileWriter.write(line);
            fFileWriter.newLine();
        }
    }

    private void writeHeader(int type) throws IOException
    {
        if (type == TYPE_TEXT)
        {
            // Do nothing
        }
        else if (type == TYPE_HTML)
        {
            // Monospaced is not recognized as a valid fixed font by style sheets
            // so use courier instead for the font name

            if (fFontName.equalsIgnoreCase("Monospaced"))
                fFontName = "courier";

            // Write HTML header

            appendLine("<html>");
            appendLine("");
            appendLine("<head>");
            appendLine("<style type=\"text/css\">");
            appendLine("  h3");
            appendLine("  {");
            appendLine("    font-family: \"" + fFontName + "\"");
            appendLine("  }");
            appendLine("  th");
            appendLine("  {");
            appendLine("    background-color: #4477BB;");
            appendLine("    font-family: \"" +  fFontName + "\"");
            appendLine("  }");
            appendLine("  tr");
            appendLine("  {");
            appendLine("    background-color: #DDEEFF;");
            appendLine("    font-family: \"" + fFontName + "\"");
            appendLine("  }");
            appendLine("  tr.altcolor");
            appendLine("  {");
            appendLine("    background-color: #99CCFF;");
            appendLine("    font-family: \"" + fFontName + "\"");
            appendLine("  }");
            appendLine("</style>");
            appendLine("</head>");
            appendLine("");
        }
    }
    
    private void writeBody(int type) throws IOException, STAFException
    {
        if (type == TYPE_TEXT)
        {
            if (fTitle != null)
            {
                appendLine(fTitle);
                appendLine("");
            }
        }
        else if (type == TYPE_HTML)
        {
            // Write HTML body

            appendLine("<body>");
            appendLine("");

            if (fTitle != null)
            {
                // Write html header containing the title

                appendLine("<h3>" + fTitle + "</h3>");
                appendLine("");
            }

            appendLine("<table border=\"3\" width=\"100%%\">");
            appendLine("<tr>");
            appendLine("  <th width=\"15%%\" align=\"left\">Date-Time</th>");
            appendLine("  <th width=\"5%%\" align=\"left\">Level</th>");
            appendLine("  <th width=\"80%%\" align=\"left\">Message</th>");
            appendLine("</tr>");
        }

        boolean altRowColor = false;

        List list = (List)fContext.getRootObject();
        Iterator iter = list.iterator();

        while (iter.hasNext())
        {
            Object entry = iter.next();

            // Verify that the entry is an instance of the
            // STAF/Service/Log/LogRecord or STAF/Service/Log/LogRecordLong
            // map class

            if (!(entry instanceof Map))
            {
                throw new STAFException(
                    STAFResult.InvalidValue,
                    "Input marshalling context's root object is not a list " +
                    "of all " + sLOG_MAP_CLASS_DEF + " or " +
                    sLOG_LONG_MAP_CLASS_DEF + " map class objects (1)");
            }

            Map logMap = (Map)entry;

            if (!logMap.containsKey("staf-map-class-name"))
            {
                throw new STAFException(
                    STAFResult.InvalidValue,
                    "Input marshalling context's root object is not a list " +
                    "of all " + sLOG_MAP_CLASS_DEF + " or " +
                    sLOG_LONG_MAP_CLASS_DEF + " map class objects (2)");
            }

            String mapClassDef = (String)logMap.get("staf-map-class-name");

            if (!mapClassDef.equals(sLOG_MAP_CLASS_DEF) &&
                !mapClassDef.equals(sLOG_LONG_MAP_CLASS_DEF))
            {
                throw new STAFException(
                    STAFResult.InvalidValue,
                    "Input marshalling context's root object is not a list " +
                    "of all " + sLOG_MAP_CLASS_DEF + " or " +
                    sLOG_LONG_MAP_CLASS_DEF + " map class objects (3)");
            }

            if (type == TYPE_TEXT)
            {
                // Write a line containing the timestamp, level, and message
                // separated by a space

                String line =
                    (String)logMap.get("timestamp") + FIELD_SEPARATOR +
                    (String)logMap.get("level") + FIELD_SEPARATOR +
                    (String)logMap.get("message");

                appendLine(line);
            }
            else if (type == TYPE_HTML)
            {
                // Alternate colors for each row in the table

                if (altRowColor)
                {
                    appendLine("<tr valign=\"top\" class=\"altcolor\">");
                    altRowColor = false;
                }
                else
                {
                    appendLine("<tr valign=\"top\">");
                    altRowColor = true;
                }

                // Write a row containing the timestamp, level, and message

                appendLine("  <td>" + logMap.get("timestamp") + "</td>");

                appendLine("  <td>" + logMap.get("level") + "</td>");

                // Need to preserve whitespace in the message to retain correct
                // text formatting (preserves spaces and line breaks)

                appendLine("  <td style=\"white-space: pre\">" +
                           logMap.get("message") + "</td>");

                appendLine("</tr>");
            }
        }
        
        if (type == TYPE_HTML)
        {
            appendLine("</table>");
            appendLine("");
            appendLine("</body>");
        }
    }

    private void writeFooter(int type) throws IOException
    {
        if (type == TYPE_TEXT)
        {
            // Do nothing
        }
        else if (type == TYPE_HTML)
        {
            appendLine("</html>");
        }
    }

    private static void checkIfArgHasValue(String argv[], int i, Set options)
    {
        if (((i+1) > argv.length - 1) ||
            (options.contains(argv[i+1])))
        {
            System.err.println(
                "\nERROR: Option " + argv[i] + " requires a value");
            System.err.println(helpText);
            System.exit(1);
        }
    }

    private static void verifyLogQueryResult(STAFResult queryResult,
                                             STAFHandle handle,
                                             String machine,
                                             String serviceName,
                                             String logRequest)
    {
        if (queryResult.rc == 4010)
        {
            String errMsg = "\nThe LOG QUERY request failed with RC 4010 " +
                "because the query criteria selected more records than " +
                "allowed by the DefaultMaxQueryRecords setting. " +
                "Use the FIRST <Num> or LAST <Num> option to specify " +
                "the number of records or the ALL option if you " +
                "really want all of the records.";

            // Get the defaultMaxQueryRecords setting for the LOG service

            STAFResult logSettingsResult = handle.submit2(
                machine, serviceName, "LIST SETTINGS");

            if (logSettingsResult.rc == STAFResult.Ok)
            {
                try
                {
                    Map settingsMap = (Map)logSettingsResult.resultObj;

                    String defaultMaxQueryRecords =
                        (String)settingsMap.get("defaultMaxQueryRecords");
            
                    errMsg += "  The last " + defaultMaxQueryRecords +
                        " log entries will be formatted.";
                }
                catch (Exception e)
                {
                    // Ignore
                }
            }

            System.err.println(errMsg);
        }
        else if (queryResult.rc != 0)
        {
            String errMsg;

            if (queryResult.rc == STAFResult.DoesNotExist)
            {
                errMsg = "\nThe log does not exist.  " +
                    "The log query request failed with RC=" + queryResult.rc +
                    ", Result=" + queryResult.result + "\n\n" + logRequest;

            }
            else
            {
                errMsg = "\nThe log query request failed with RC=" +
                    queryResult.rc + ", Result=" + queryResult.result + "\n\n" +
                    logRequest;
            }
                    
            System.err.println(errMsg); 
            System.exit(1);
        }

        // Verify the marshalling context is valid (contains the output from a
        // LOG QUERY request (without the TOTAL or STATS option)

        if ((queryResult.resultContext == null) ||
            (queryResult.resultObj == null))
        {
            String errMsg = "\nThe log query request returned a null " +
                "marshalling context or root. \n\nInvalid query request: " +
                logRequest;
            
            System.err.println(errMsg); 
            System.exit(1);
        }
            
        if ((!queryResult.resultContext.hasMapClassDefinition(
                 sLOG_MAP_CLASS_DEF) &&
             !queryResult.resultContext.hasMapClassDefinition(
                 sLOG_LONG_MAP_CLASS_DEF)) ||
            !(queryResult.resultObj instanceof java.util.List))
        {
            String errMsg = "\nThe log query request cannot contain the " +
                "TOTAL or STATS option.\n\nInvalid query request: " +
                logRequest;

            System.err.println(errMsg); 
            System.exit(1);
        }
    }

    /**
     * This method is used to execute the STAFLogFormatter class from the
     * command line.
     * <p>
     * To get help on the arguments to specify for the STAFLogFormatter class
     * from the command line, specify the following:
     * <p>
     * <code>java com.ibm.staf.STAFLogViewer -help</code>
     * <p>
     * The arguments that can be specified from the command line when executing
     * the STAFLogFormatter class are:
     * <ul>
     * <li>-queryRequest &lt;LOG QUERY Request>
     * <li>-fileName <File Name>
     * <li>-type <html | text>
     * <li>-fontName <Font Name>
     * <li>-title <Title>
     * <li>-machine <Log Service Machine Name>
     * <li>-serviceName <Log Service Name>
     * <li>-help
     * <li>-version
     * </ul>
     * <p>
     * You must specify one of the following options: -queryRequest or -help
     * or -version. If specifying the -help or -version option, it must be
     * the first and only option specified. All of the other options require
     * the -queryRequest option to be specified.
     * <p>
     * Here's a description of all of the options:
     * <p>
     * <ul>
     * <li><code>-queryRequest</code> specifies the QUERY request to submit to
     * the Log service. This option is required unless you specified -help or
     * -version. It should begin with the QUERY option and include any
     * additional query options, including the options to specify whether the
     * log is a GLOBAL, MACHINE, or HANDLE log. You may not specify the TOTAL
     * or STATS option within the queryRequest.
     * <p>
     * <li><code>-fileName</code> specifies the fully-qualified name of a file
     * where the formatted log output will be written. This option is optional.
     * If not specified, the formatted log output is written to Stdout.
     * <p>
     * <li><code>-type</code> specifies the format type. Valid values are:
     * html or text. This option is optional. If not specified, it defaults
     * to html.
     * <p>
     * <li><code>-fontName</code> specifies the name of the font to use when
     * formatting the STAF log. It does not have an effect if formatting the
     * log as text. This option is optional. If not specified, it defaults to
     * courier (which is a Monospaced font). Examples of other valid font
     * names are dialog and timesRoman.
     * <p>
     * <li><code>-title</code> specifies a title for the log output. This
     * option is optional. If not specified, it defaults to the actual log
     * query request submitted if using type html or to no title if using
     * type text.
     * <p>
     * <li><code>-machine</code> specifies the name of the machine where the
     * STAF log is located. This option is optional. If not specified, it
     * defaults to local.
     * <p>
     * <li><code>-serviceName</code> specifies the name of the Log service
     * to which the query request will be submitted. This option is optional.
     * If not specified, it defaults to LOG.
     * <p>
     * <li><code>-help</code> displays help information for the STAFLogFormatter.
     * This option is optional, but if specified, it must be the first and only
     * option specified.
     * <p>
     * <li><code>-version</code> displays the version of the STAFLogFormatter.
     * This option is optional, but if specified, it must be the first and
     * only option specified.
     * </ul>
     * 
     * @param argv The command-line arguments to be specified by the user.
     */ 
    public static void main(String argv[])
    {
        String queryRequest = "";
        String fileName = null;
        String type = "html";
        String title = null;
        String fontName = "courier";
        String machine = "local";
        String serviceName = "LOG";

        int minArgsAllowed = 1;  // -queryRequest or -help or -version

        // Maximum of 14 arguments: A maximum of 7 options can be specifed at
        // once, but need to multiple 7 by 2 (to get 14) since each of these
        // options requires a value (and each option value also counts as an
        // argument).
        // e.g. -queryRequest <request> -fileName <name> -type <type>
        //      -title <title> -fontName <fontName> -machine <machine>
        //      -serviceName <name>
        int maxArgsAllowed = 14;

        boolean queryRequestArg = false;

        Set options = new HashSet();
        options.add("-queryRequest");
        options.add("-fileName");
        options.add("-type");
        options.add("-title");
        options.add("-fontName");
        options.add("-machine");
        options.add("-serviceName");
        options.add("-help");
        options.add("-version");
        
        if (argv.length < minArgsAllowed)
        {
            System.err.println(
                "\nERROR: Must specify " + minArgsAllowed + " of the " +
                "following options:  -queryRequest, -help, or -version");
            System.err.println(helpText);
            System.exit(1);
        }
        else if (argv.length > maxArgsAllowed)
        {
            System.err.println(
                "\nERROR: Too many arguments.  You specified " + argv.length +
                " arguments, but only up to " + maxArgsAllowed +
                " arguments are allowed.");

            System.err.println(helpText);
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
                else if (argv[i].equalsIgnoreCase("-fileName"))
                {
                    checkIfArgHasValue(argv, i, options);
                    fileName = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-type"))
                {
                    checkIfArgHasValue(argv, i, options);
                    type = argv[i+1];
                    i++;

                    if (!type.equalsIgnoreCase("html") &&
                        !type.equalsIgnoreCase("text"))
                    {
                        System.err.println(
                            "\nERROR: Invalid value for -type option: " + type +
                            "\nValid values are html or text.");
                        System.exit(1);
                    }
                }
                else if (argv[i].equalsIgnoreCase("-fontName"))
                {
                    checkIfArgHasValue(argv, i, options);
                    fontName = argv[i+1];
                    i++;
                }
                else if (argv[i].equalsIgnoreCase("-title"))
                {
                    checkIfArgHasValue(argv, i, options);
                    title = argv[i+1];
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
                else if (argv[i].equalsIgnoreCase("-version"))
                {
                    System.err.println(
                        "\nERROR: Option -version must be specified " +
                        "as the first and only option");
                    System.err.println(helpText);
                    System.exit(1);
                }
                else if (argv[i].equalsIgnoreCase("-help"))
                {
                    System.err.println(
                        "\nERROR: Option -help must be specified as " +
                        "the first and only option");
                    System.err.println(helpText);
                    System.exit(1);
                }
                else
                {
                    System.err.println("\nInvalid option name: " + argv[i]);
                    System.err.println(helpText);
                    System.exit(1);
                }
            }
       
            if (!queryRequestArg)
            {
                System.err.println(
                    "\nERROR: Must specify " + minArgsAllowed + " of the " +
                    "following options:  -queryRequest, -help, or -version");
                System.err.println(helpText);
                System.exit(1);
            }
        }

        // Create a STAF Handle
        
        STAFHandle handle = null;

        try
        {
            handle = new STAFHandle("STAFLogFormatter");
        }
        catch(STAFException e)
        {
            String errMsg = "Error registering with STAF, RC: " + e.rc;

            System.err.println(errMsg);
            System.exit(1);
        }

        // Submit the Log query request

        String logRequest = "STAF " + machine + " " + serviceName + " " +
            queryRequest;

        STAFResult queryResult = handle.submit2(
            machine, serviceName, queryRequest);

        verifyLogQueryResult(
            queryResult, handle, machine, serviceName, logRequest);

        // Format the log query output

        String output = new String("");

        try
        {
            STAFLogFormatter logFormatter = new STAFLogFormatter(
                queryResult.resultContext);
        
            if (fileName != null)
                logFormatter.setFileName(fileName);
            
            if (fontName != null)
                logFormatter.setFontName(fontName);

            // If the -title option is not specified, default to the log query
            // request unless using type text

            if (title != null)
                logFormatter.setTitle(title);
            else if (!type.equalsIgnoreCase("text"))
                logFormatter.setTitle(logRequest);

            output = logFormatter.format(type);
        }
        catch (Exception e)
        {
            String errMsg = "\nFormatting the log as " + type;

            if (fileName != null)
            {
                errMsg += " and/or saving the formatted output to file " +
                    fileName;
            }
                
            errMsg += " failed.\n" + e.toString();

            System.err.println(errMsg);
            System.exit(1);
        }

        // If no fileName was specified, write the formatted output to stdout

        if (fileName == null)
            System.out.println(output);

        System.exit(0);
    }
}
