/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.cron;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.Iterator;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;
import org.python.util.PythonInterpreter;
import org.python.core.PyException;

public class CronService implements STAFServiceInterfaceLevel30
{
    private STAFHandle fHandle;
    private final String kVersion = "3.4.0";

    // Version of STAF (or later) required for this service
    // - STAF Version 3.1.0 or later is required so that the privacy methods
    //   in STAFUtil are available.
    // - STAF Version 3.3.3 or later is required so that the QUEUE GET ALL
    //   request can be used to get all messages off the queue at once
    private final String kRequiredSTAFVersion = "3.3.3";

    private boolean fTerminated = false;
    private STAFCommandParser fParmsParser = new STAFCommandParser();
    private STAFCommandParser fRegisterParser;
    private STAFCommandParser fListParser;
    private STAFCommandParser fTriggerParser;
    private STAFCommandParser fEnableParser;
    private STAFCommandParser fDisableParser;
    private STAFCommandParser fVersionParser;
    private STAFCommandParser fDebugParser;
    private STAFMapClassDefinition fCronIDMapClass;
    private STAFMapClassDefinition fCronIDLongMapClass;
    private STAFMapClassDefinition fCronIDShortMapClass;
    private STAFMapClassDefinition fTriggerMapClass;
    private STAFMapClassDefinition fProcessEndMapClass;
    private STAFMapClassDefinition fProcessReturnFileMapClass;
    private Hashtable<Integer, Object> fRegTable;
    private boolean fDebug = false;
    private boolean fOldVarResolution = false;
    private STAFCommandParser fUnregisterParser;
    private int fCronID = 1;
    private String fCronServiceMachine = "";
    private String fServiceName = "";
    private String fJythonVersion = "";
    private String fHashtableFileDirectory;
    private String fHashtableFileName;  // cron.ser
    private String fBackupFileName;     // cron_backup.ser
    private File fBackupFile;
    private String fLocalMachineName;

    // Map of the submitted requests that haven't completed yet
    //   Key: <reqNum>, Value:  <RequestData object>
    private Map<String, RequestData> fRequestMap =
        Collections.synchronizedMap(new HashMap<String, RequestData>());

    // Map of the submitted PROCESS START requests (without the WAIT option)
    // that haven't completed yet
    //   Key: <machineName>:<handle>, Value: <reqNum>
    private Map<String, String> fProcessHandleMap =
        Collections.synchronizedMap(new HashMap<String, String>());

    private String fServiceHandleName = "";
    private int fServiceHandleNumber = 0;
    private MonitorThread fMonitorThread = null;
    private TriggerThread fTriggerThread = null;

    private static String sHelpMsg;
    private static String fLineSep;

    private static final boolean DEBUG = true;
    private static final String kMinute = "minute";
    private static final String kHour = "hour";
    private static final String kDay = "day";
    private static final String kMonth = "month";
    private static final String kWeekday = "weekday";

    static final String sQueueTypeEnd = "STAF/Service/Cron/End";

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    // Cron Service Error Codes

    public static final int PYTHONERROR = 4001;
    private static final String PYTHONERRORInfo = "Python error";
    private static final String PYTHONERRORDesc =
        "A Python error occurred while evaluating the request to be submitted.";

    public static final int REQUESTNOTSUBMITTED = 4002;
    private static final String REQUESTNOTSUBMITTEDInfo =
        "Request not submitted";
    private static final String REQUESTNOTSUBMITTEDDesc =
        "The request was not submitted because the value of " +
        "STAFCronSubmit was not set to true.";

    public CronService()
    {
    }

    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        int rc = STAFResult.Ok;

        try
        {
            STAFResult res = new STAFResult();
            fServiceHandleName = "STAF/SERVICE/" + info.name;
            fHandle = new STAFHandle(fServiceHandleName);
            fServiceHandleNumber = fHandle.getHandle();
            fServiceName = info.name;

            // Resolve the machine name variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLocalMachineName = res.result;
            
            // Resolve the line separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLineSep = res.result;

            // Verify that the required version of STAF is running on the
            // local service machine.  
            // Note:  Method compareSTAFVersion was added in STAF V3.1.0

            try
            {
                res = STAFUtil.compareSTAFVersion(
                    "local", fHandle, kRequiredSTAFVersion);

                if (res.rc != STAFResult.Ok)
                {
                    if (res.rc == STAFResult.InvalidSTAFVersion)
                    {
                        return new STAFResult(
                            STAFResult.ServiceConfigurationError,
                            "Minimum required STAF version for this service " +
                            "is not running." + fLineSep + res.result);
                    }
                    else
                    {
                        return new STAFResult(
                            STAFResult.ServiceConfigurationError,
                            "Error verifying the STAF version. RC: " + res.rc +
                            ", Additional info: " + res.result);
                    }
                }
            }
            catch (Error err)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "This service requires STAF Version " +
                    kRequiredSTAFVersion + " or later."); 
            }

            if (info.serviceJar != null)
            {
                // Create the Jython Library directory if it doesn't exist yet
                // from the jython library data in the service's jarfile.
                // Set the classpath to point to the service's jar file and to
                // the jython.jar.

                res = STAFServiceSharedJython.setupJython(
                    info.serviceJar, fHandle, info.writeLocation);

                if (res.rc != 0)
                {
                    System.out.println(res.result);
                    return res;
                }

                // Get the version of Jython distributed with the Cron service
                    
                fJythonVersion = STAFServiceSharedJython.getJythonVersion();

                // Initialize the PythonInterpreter's python.home to point to the
                // shared jython installation.

                Properties p = new Properties();
                p.setProperty("python.home",
                              STAFServiceSharedJython.getJythonDirName());
                PythonInterpreter.initialize(System.getProperties(), p, null);
            }
            
            // Parse PARMS if provided in the service configuration line

            if (info.parms != null)
            {
                fParmsParser.addOption("OLDVARRESOLUTION", 1,
                                       STAFCommandParser.VALUENOTALLOWED);

                fParmsParser.addOption("DEBUG", 1,
                                       STAFCommandParser.VALUENOTALLOWED);

                res = handleParms(info);

                if (res.rc != STAFResult.Ok)
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "Error validating parameters: RC=" + res.rc +
                        ", Result=" + res.result);
                }
            }

            // instantiate parsers as not case sensitive
            fRegisterParser = new STAFCommandParser(0, false);
            fListParser = new STAFCommandParser(0, false);
            fTriggerParser = new STAFCommandParser(0, false);
            fEnableParser = new STAFCommandParser(0, false);
            fDisableParser = new STAFCommandParser(0, false);
            fUnregisterParser = new STAFCommandParser(0, false);
            fVersionParser = new STAFCommandParser(0, false);
            fDebugParser = new STAFCommandParser(0, false);

            fRegisterParser.addOption("REGISTER", 1,
                                      STAFCommandParser.VALUENOTALLOWED);

            fRegisterParser.addOption("DESCRIPTION", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("MACHINE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("PYTHONMACHINE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOptionGroup("MACHINE PYTHONMACHINE", 1, 1);

            fRegisterParser.addOption("SERVICE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("PYTHONSERVICE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOptionGroup("SERVICE PYTHONSERVICE", 1, 1);

            fRegisterParser.addOption("REQUEST", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("PYTHONREQUEST", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOptionGroup("REQUEST PYTHONREQUEST", 1, 1);

            fRegisterParser.addOption("PREPARE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("MINUTE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("HOUR", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("DAY", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("MONTH", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("WEEKDAY", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("ONCE", 1,
                                      STAFCommandParser.VALUENOTALLOWED);

            fRegisterParser.addOption("ENABLED", 0,
                                      STAFCommandParser.VALUENOTALLOWED);

            fRegisterParser.addOption("DISABLED", 0,
                                      STAFCommandParser.VALUENOTALLOWED);

            fRegisterParser.addOptionGroup("ENABLED DISABLED", 0, 1);

            // Require at least one time interval option to be specified
            fRegisterParser.addOptionGroup(
                "MINUTE HOUR DAY MONTH WEEKDAY", 1, 5);

            fUnregisterParser.addOption("UNREGISTER", 1,
                                        STAFCommandParser.VALUENOTALLOWED);

            fUnregisterParser.addOption("ID", 1,
                                        STAFCommandParser.VALUEREQUIRED);

            fUnregisterParser.addOptionNeed("UNREGISTER", "ID");

            fListParser.addOption("LIST", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption("MACHINE", 1,
                                  STAFCommandParser.VALUEREQUIRED);

            fListParser.addOption("LONG", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption("SHORT", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOptionGroup("LONG SHORT", 0, 1);

            fTriggerParser.addOption("TRIGGER", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fTriggerParser.addOption("ID", 1,
                                     STAFCommandParser.VALUEREQUIRED);

            fTriggerParser.addOptionNeed("TRIGGER", "ID");

            fTriggerParser.addOption("SCRIPT", 0,
                                     STAFCommandParser.VALUEREQUIRED);

            fEnableParser.addOption("ENABLE", 1,
                                    STAFCommandParser.VALUENOTALLOWED);

            fEnableParser.addOption("ID", 1,
                                    STAFCommandParser.VALUEREQUIRED);

            fEnableParser.addOptionNeed("ENABLE", "ID");

            fDisableParser.addOption("DISABLE", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fDisableParser.addOption("ID", 1,
                                     STAFCommandParser.VALUEREQUIRED);

            fDisableParser.addOptionNeed("DISABLE", "ID");

            fVersionParser.addOption("VERSION", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fVersionParser.addOption("JYTHON", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fVersionParser.addOptionGroup("VERSION", 1, 1);
            fVersionParser.addOptionNeed("JYTHON", "VERSION");

            // The DEBUG request is an undocumented options that can be used
            // to debug memory leaks by listing the contents of the fRequestMap
            // and fProcessHandleMap to check if entries exist in them for
            // service requests or processes that have already completed and
            // should no longer exist in these maps

            fDebugParser.addOption("DEBUG", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fDebugParser.addOptionGroup("DEBUG", 1, 1);

            fCronIDMapClass = new STAFMapClassDefinition(
                "STAF/Service/Cron/CronID");
            fCronIDMapClass.addKey("cronID", "ID");
            fCronIDMapClass.addKey("description", "Description");
            fCronIDMapClass.setKeyProperty("description", "display-short-name",
                                           "Desc");
            fCronIDMapClass.addKey("machine", "Machine");
            fCronIDMapClass.addKey("service", "Service");
            fCronIDMapClass.addKey("request", "Request");
            fCronIDMapClass.addKey("minute", "Minute");
            fCronIDMapClass.addKey("hour", "Hour");
            fCronIDMapClass.addKey("dayOfMonth", "Day of Month");
            fCronIDMapClass.setKeyProperty("dayOfMonth", "display-short-name",
                                           "Day");
            fCronIDMapClass.addKey("month", "Month");
            fCronIDMapClass.addKey("dayOfWeek", "Day of Week");
            fCronIDMapClass.setKeyProperty("dayOfWeek", "display-short-name",
                                           "Weekday");
            fCronIDMapClass.addKey("once", "Once");

            fCronIDShortMapClass = new STAFMapClassDefinition(
                "STAF/Service/Cron/CronIDShort");
            fCronIDShortMapClass.addKey("cronID", "ID");
            fCronIDShortMapClass.addKey("description", "Description");
            fCronIDShortMapClass.setKeyProperty("description",
                                                "display-short-name",
                                                "Desc");
            fCronIDShortMapClass.addKey("machine", "Machine");
            fCronIDShortMapClass.addKey("service", "Service");
            fCronIDShortMapClass.addKey("request", "Request");

            fCronIDLongMapClass = new STAFMapClassDefinition(
                "STAF/Service/Cron/CronIDLong");
            fCronIDLongMapClass.addKey("cronID", "ID");
            fCronIDLongMapClass.addKey("description", "Description");
            fCronIDLongMapClass.addKey("machine", "Machine");
            fCronIDLongMapClass.addKey("machineType", "Machine Type");
            fCronIDLongMapClass.addKey("service", "Service");
            fCronIDLongMapClass.addKey("serviceType", "Service Type");
            fCronIDLongMapClass.addKey("request", "Request");
            fCronIDLongMapClass.addKey("requestType","Request Type");
            fCronIDLongMapClass.addKey("prepareScript", "Prepare Script");
            fCronIDLongMapClass.addKey("minute", "Minute");
            fCronIDLongMapClass.addKey("hour", "Hour");
            fCronIDLongMapClass.addKey("dayOfMonth", "Day of Month");
            fCronIDLongMapClass.addKey("month", "Month");
            fCronIDLongMapClass.addKey("dayOfWeek", "Day of Week");
            fCronIDLongMapClass.addKey("once", "Once");
            fCronIDLongMapClass.addKey("state", "State");

            // Construct map class for a TRIGGER request

            fTriggerMapClass = new STAFMapClassDefinition(
                "STAF/Service/Cron/Trigger");
            fTriggerMapClass.addKey("machine", "Machine");
            fTriggerMapClass.addKey("requestNumber", "Request Number");

            // Construct map class for process completion information for an
            // asynchronous PROCESS START request

            fProcessEndMapClass = new STAFMapClassDefinition(
                "STAF/Service/Cron/ProcessEnd");

            fProcessEndMapClass.addKey("rc", "Return Code");
            fProcessEndMapClass.addKey("key", "Key");
            fProcessEndMapClass.addKey("fileList", "Files");

            // Construct map class for a returned file from a process

            fProcessReturnFileMapClass = new STAFMapClassDefinition(
                "STAF/Service/Cron/ProcessReturnFile");

            fProcessReturnFileMapClass.addKey("rc", "Return Code");
            fProcessReturnFileMapClass.addKey("data", "Data");

            // Resolve the file separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/File}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            String fileSep = res.result;

            // Store data for the cron service in directory:
            //   <STAF writeLocation>/service/<cron service name (lower-case)>

            fHashtableFileDirectory = info.writeLocation;

            if (!fHashtableFileDirectory.endsWith(fileSep))
            {
                fHashtableFileDirectory += fileSep;
            }

            fHashtableFileDirectory = fHashtableFileDirectory +
                "service" + fileSep + fServiceName.toLowerCase();

            File dir = new File(fHashtableFileDirectory);

            if (!dir.exists())
            {
                dir.mkdirs();
            }

            fHashtableFileName = fHashtableFileDirectory + fileSep +
                "cron.ser";

            try
            {
                loadHashtable(true);
            }
            catch (Exception e)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "ERROR: Cannot load registration data from file " +
                    fHashtableFileName + ". " + fLineSep + e);
            }

            fBackupFileName = fHashtableFileDirectory + fileSep +
                "cron_backup.ser";

            fBackupFile = new File(fBackupFileName);

            // Assign the help text string for the service

            sHelpMsg = "*** " + info.name + " Service Help ***" +
                fLineSep + fLineSep +
                "REGISTER   [DESCRIPTION <Description>]" +
                fLineSep +
                "           MACHINE <Machine> | PYTHONMACHINE <Machine>" +
                fLineSep +
                "           SERVICE <Service> | PYTHONSERVICE <Machine>" +
                fLineSep +
                "           REQUEST <Request> | PYTHONREQUEST <Request>" +
                fLineSep +
                "           [PREPARE <Script>]" +
                fLineSep +
                "           [MINUTE <Minute>] [HOUR <Hour>] " +
                fLineSep +
                "           [DAY <Day>] [MONTH <Month>] " +
                fLineSep +
                "           [WEEKDAY <Weekday>]" +
                fLineSep +
                "           [ONCE]" +
                fLineSep + "           [ENABLED | DISABLED]" +
                fLineSep +
                "UNREGISTER ID <RegistrationID>" +
                fLineSep +
                "LIST       [MACHINE <Machine>] [LONG | SHORT]" +
                fLineSep +
                "TRIGGER    ID <RegistrationID> [SCRIPT <Python code>]..." +
                fLineSep +
                "ENABLE     ID <RegistrationID>" +
                fLineSep +
                "DISABLE    ID <RegistrationID>" +
                fLineSep +
                "VERSION    [JYTHON]" +
                fLineSep +
                "HELP";

            // Register RCs with the HELP service

            registerHelp();

            // Start a thread that in a continuous loop waits for a message
            // to appear in the Cron service handle's queue and handles the
            // message

            fMonitorThread = new MonitorThread(this);
            fMonitorThread.start();

            // Start a thread that in a continuous loop waits 10 seconds and
            // then checks if any registrations have been triggered

            fTriggerThread = new TriggerThread(this);
            fTriggerThread.start();
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  e.toString());
        }

        return new STAFResult(rc);
    }

    private void registerHelp()
    {
        String request = "REGISTER SERVICE " + fServiceName + " ERROR " +
            PYTHONERROR + " INFO \"" + PYTHONERRORInfo + "\" DESCRIPTION \"" +
            PYTHONERRORDesc + "\"";
        fHandle.submit2("local", "HELP", request);

        request = "REGISTER SERVICE " + fServiceName + " ERROR " +
            REQUESTNOTSUBMITTED + " INFO \"" + REQUESTNOTSUBMITTEDInfo +
            "\" DESCRIPTION \"" + REQUESTNOTSUBMITTEDDesc + "\"";
        fHandle.submit2("local", "HELP", request);
    }

    private void unregisterHelp()
    {
        fHandle.submit2("local", "HELP", "UNREGISTER SERVICE " +
                        fServiceName + " ERROR " + PYTHONERROR);
        fHandle.submit2("local", "HELP", "UNREGISTER SERVICE " +
                        fServiceName + " ERROR " + REQUESTNOTSUBMITTED);
    }

    private STAFResult handleParms(STAFServiceInterfaceLevel30.InitInfo info)
        throws STAFException
    {
        STAFCommandParseResult parseResult = fParmsParser.parse(info.parms);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("OLDVARRESOLUTION") > 0)
        {
            fOldVarResolution = true;
        }
        
        if (parseResult.optionTimes("DEBUG") > 0)
        {
            fDebug = true;
        }

        return new STAFResult(STAFResult.Ok);
    }

    public STAFResult acceptRequest(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Try block is here to catch any unexpected errors/exceptions

        try
        {
            // Determine the command request (the first word in the request)

            String action;
            int spaceIndex = info.request.indexOf(" ");

            if (spaceIndex != -1)
                action = info.request.substring(0, spaceIndex);
            else
                action = info.request;

            String actionLC = action.toLowerCase();

            // Call the appropriate method to handle the command request

            if (actionLC.equals("register"))
                return handleRegister(info);
            else if (actionLC.equals("unregister"))
                return handleUnregister(info);
            else if (actionLC.equals("list"))
                return handleList(info);
            else if (actionLC.equals("trigger"))
                return handleTrigger(info);
            else if (actionLC.equals("enable"))
                return handleEnable(info);
            else if (actionLC.equals("disable"))
                return handleDisable(info);
            else if (actionLC.equals("help"))
                return handleHelp(info);
            else if (actionLC.equals("version"))
                return handleVersion(info);
            else if (actionLC.equals("debug"))
                return handleDebug(info);
            else
            {
                return new STAFResult(
                    STAFResult.InvalidRequestString,
                    "'" + action +
                    "' is not a valid command request for the " +
                    fServiceName + " service" +
                    fLineSep + fLineSep + sHelpMsg);
            }
        }
        catch (Throwable t)
        {
            // Write the Java stack trace to the JVM log for the service

            System.out.println(
                sTimestampFormat.format(Calendar.getInstance().getTime()) +
                " ERROR: Exception on " + fServiceName + " service request:" +
                fLineSep + fLineSep + info.request + fLineSep);

            t.printStackTrace();

            // Log the error in the service log

            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + t.toString());

            // And also return the Java stack trace in the result

            return new STAFResult(STAFResult.JavaError, getStackTrace(t));
        }
    }

    private STAFResult handleHelp(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "HELP", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Return help text

        return new STAFResult(STAFResult.Ok, sHelpMsg);
    }

    private STAFResult handleList(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        STAFCommandParseResult parsedRequest = fListParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parsedRequest.errorBuffer);
        }

        STAFMarshallingContext mc = new STAFMarshallingContext();
        List<Map> cronIDList = new ArrayList<Map>();

        STAFResult sResult = new STAFResult(STAFResult.Ok, "");
        STAFResult resolvedResult = null;
        STAFResult resolvedValue = null;
        String machineParm = "";
        String typeParm = "";

        try
        {
            // Verify the requester has at least trust level 2

            STAFResult trustResult = STAFUtil.validateTrust(
                2, fServiceName, "LIST", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Resolve the MACHINE option

            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("machine"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            machineParm = resolvedValue.result;

            // Check if the LONG or SHORT options are specified

            boolean longFormat = false;
            boolean shortFormat = false;

            if (parsedRequest.optionTimes("LONG") > 0)
            {
                longFormat = true;
                mc.setMapClassDefinition(fCronIDLongMapClass);
            }
            else if (parsedRequest.optionTimes("SHORT") > 0)
            {
                shortFormat = true;
                mc.setMapClassDefinition(fCronIDShortMapClass);
            }
            else
            {
                mc.setMapClassDefinition(fCronIDMapClass);
            }

            // Process the LIST registrations request

            CronData2 theData;
            boolean match;

            synchronized (fRegTable)
            {
                if (fRegTable.size() == 0)
                {
                    mc.setRootObject(cronIDList);
                    return new STAFResult(STAFResult.Ok, mc.marshall());
                }

                int numberOfKeys = fRegTable.size();
                Integer[] ids = new Integer[numberOfKeys];
                Enumeration<Integer> idKeys = fRegTable.keys();

                for (int i = 0; idKeys.hasMoreElements(); i++)
                {
                    ids[i] = idKeys.nextElement();
                }

                Arrays.sort(ids);

                for (int i = 0; i < numberOfKeys; i++)
                {
                    Integer id = ids[i];
                    match = false;
                    theData = (CronData2) fRegTable.get(id);

                    String description = "";
                    String minutes = "";
                    String hours = "";
                    String days = "";
                    String months = "";
                    String weekdays = "";

                    if (machineParm.equals("") |
                        machineParm.equals(theData.fMachine))
                    {
                        description = theData.fDescription;
 
                        Enumeration minuteEnum = theData.fMinute.elements();

                        while (minuteEnum.hasMoreElements())
                        {
                            minutes += (minuteEnum.nextElement()).toString();

                            if (minuteEnum.hasMoreElements())
                                minutes += ",";
                        }

                        Enumeration hourEnum = theData.fHour.elements();

                        while (hourEnum.hasMoreElements())
                        {
                            hours += (hourEnum.nextElement()).toString();

                            if (hourEnum.hasMoreElements())
                                hours += ",";
                        }

                        Enumeration dayEnum = theData.fDay.elements();

                        while (dayEnum.hasMoreElements())
                        {
                            days += (dayEnum.nextElement()).toString();

                            if (dayEnum.hasMoreElements())
                                days += ",";
                        }

                        Enumeration monthEnum = theData.fMonth.elements();

                        while (monthEnum.hasMoreElements())
                        {
                            months += (monthEnum.nextElement()).toString();

                            if (monthEnum.hasMoreElements())
                                months += ",";
                        }

                        Enumeration weekdayEnum = theData.fWeekday.elements();

                        while (weekdayEnum.hasMoreElements())
                        {
                            weekdays += (weekdayEnum.nextElement()).toString();

                            if (weekdayEnum.hasMoreElements())
                                weekdays += ",";
                        }

                        // Create a map representing the matching registration
                        // and add it to the list of registrations

                        Map<String, Object> cronIDMap =
                            new TreeMap<String, Object>();

                        if (longFormat)
                        {
                            // Not calling STAFMapClass createInstance method to avoid
                            // getting an unchecked cast warning
                            //cronIDMap = fCronIDLongMapClass.createInstance();
                            cronIDMap.put("staf-map-class-name",
                                          fCronIDLongMapClass.name());
                        }
                        else if (shortFormat)
                        {
                            // Not calling STAFMapClass createInstance method to avoid
                            // getting an unchecked cast warning
                            //cronIDMap = fCronIDShortMapClass.createInstance();
                            cronIDMap.put("staf-map-class-name",
                                          fCronIDShortMapClass.name());
                        }
                        else
                        {
                            // Not calling STAFMapClass createInstance method to avoid
                            // getting an unchecked cast warning
                            //cronIDMap = fCronIDMapClass.createInstance();
                            cronIDMap.put("staf-map-class-name",
                                          fCronIDMapClass.name());
                        }

                        cronIDMap.put("cronID", id.toString());

                        if (!theData.fDescription.equals(""))
                        {
                            cronIDMap.put("description", theData.fDescription);
                        }

                        cronIDMap.put("machine", theData.fMachine);
                        cronIDMap.put("service", theData.fService);
                        cronIDMap.put("request", STAFUtil.maskPrivateData(
                            theData.fRequest));
                        
                        if ((minutes != null) && (!minutes.equals(""))
                            && !shortFormat)
                        {
                            cronIDMap.put("minute", minutes);
                        }
                        if ((hours != null) && (!hours.equals(""))
                            && !shortFormat)
                        {
                            cronIDMap.put("hour", hours);
                        }
                        if ((days != null) && (!days.equals(""))
                            && !shortFormat)
                        {
                            cronIDMap.put("dayOfMonth", days);
                        }
                        if ((months != null) && (!months.equals(""))
                            && !shortFormat)
                        {
                            cronIDMap.put("month", months);
                        }
                        if ((weekdays != null) && (!weekdays.equals(""))
                            && !shortFormat)
                        {
                            cronIDMap.put("dayOfWeek", weekdays);
                        }

                        if (!shortFormat)
                        {
                            cronIDMap.put("once", 
                                new Boolean(theData.fOnce).toString());
                        }

                        if (longFormat)
                        {
                            if (theData.fPythonMachine)
                                cronIDMap.put("machineType", "Python");
                            else
                                cronIDMap.put("machineType", "Literal");

                            if (theData.fPythonService)
                                cronIDMap.put("serviceType", "Python");
                            else
                                cronIDMap.put("serviceType", "Literal");

                            if (theData.fPythonRequest)
                                cronIDMap.put("requestType", "Python");
                            else
                                cronIDMap.put("requestType", "Literal");

                            if ((theData.fPrepare != null) &&
                                (!theData.fPrepare.equals("")))
                            {
                                cronIDMap.put("prepareScript",
                                              STAFUtil.maskPrivateData(
                                                  theData.fPrepare));
                            }

                            cronIDMap.put("state", theData.fState);
                        }

                        cronIDList.add(cronIDMap);
                    }
                }
            }
        }
        catch (Exception e)
        {
            if (DEBUG) e.printStackTrace();
            
            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + e.toString());

            return new STAFResult(STAFResult.JavaError, getStackTrace(e));
        }

        mc.setRootObject(cronIDList);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private STAFResult handleTrigger(STAFServiceInterfaceLevel30.RequestInfo
                                     info)
    {
        STAFResult resolvedValue = null;

        STAFCommandParseResult parsedRequest = fTriggerParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parsedRequest.errorBuffer);
        }

        try
        {
            // Verify the requester has at least trust level 5

            STAFResult trustResult = STAFUtil.validateTrust(
                5, fServiceName, "TRIGGER", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Resolve the ID option

            resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                "id", parsedRequest.optionValue("id"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;
            
            Integer id = new Integer(resolvedValue.result);

            // Handle any SCRIPT options

            int scriptCount = parsedRequest.optionTimes("script");
            LinkedList<String> scripts = new LinkedList<String>();

            if (scriptCount > 0)
            {
                for (int i = 1; i <= scriptCount; i++)
                {
                    resolvedValue = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("script", i),
                        fHandle, info.requestNumber);

                    scripts.add(resolvedValue.result);
                }
            }

            // Get the registration data for the specified ID, or if the ID
            // does not exist, return an error

            CronData2 regData = null;

            synchronized (fRegTable)
            {
                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, "ID " + id + " not found");
                }

                regData = (CronData2)fRegTable.get(id);
            }

            String triggerMsg = "[ID=" + id + "] [" + info.endpoint +
                ", " + info.handleName + "] Triggering a STAF command.\n" +
                info.request;

            log("info", triggerMsg);
                
            // Trigger the STAF Command in the Cron registration ID

            return trigger(id, regData, true, null, scripts);
        }
        catch (Exception e)
        {
            if (DEBUG) e.printStackTrace();
            
            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + e.toString());

            return new STAFResult(STAFResult.JavaError, getStackTrace(e));
        }
    }

    private STAFResult handleEnable(STAFServiceInterfaceLevel30.RequestInfo
                                    info)
    {
        STAFResult sResult = new STAFResult(STAFResult.Ok, "");
        STAFResult resolvedResult = null;
        STAFResult resolvedValue = null;
        Integer id;

        STAFCommandParseResult parsedRequest = fEnableParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parsedRequest.errorBuffer);
        }

        try
        {
            // Verify the requester has at least trust level 4

            STAFResult trustResult = STAFUtil.validateTrust(
                4, fServiceName, "ENABLE", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Resolve the ID option

            resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                "id", parsedRequest.optionValue("id"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            id = new Integer(resolvedValue.result);

            // Update the state for the specified ID, or if the ID does not
            // exist, return an error

            CronData2 data = null;

            synchronized (fRegTable)
            {
                // Check if registration table contains the ID

                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, "ID " + id + " not found");
                }

                data = (CronData2)fRegTable.get(id);
                data.fState = "Enabled";

                fRegTable.put(id, data);

                // Save registration data to a file

                sResult = saveHashtable();
            }
            
            if (sResult.rc != STAFResult.Ok) return sResult;

            // Log message that says the registration has been enabled

            String message = "[ID=" + id + "] [" + info.endpoint + ", " +
                    info.handleName + "] Enabled a STAF command.";

            log("info", message);
        }
        catch (Exception e)
        {
            if (DEBUG) e.printStackTrace();

            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + e.toString());

            return new STAFResult(STAFResult.JavaError, getStackTrace(e));
        }

        return sResult;
    }

    private STAFResult handleDisable(STAFServiceInterfaceLevel30.RequestInfo
                                     info)
    {
        STAFResult sResult = new STAFResult(STAFResult.Ok, "");
        STAFResult resolvedResult = null;
        STAFResult resolvedValue = null;
        Integer id;

        STAFCommandParseResult parsedRequest = fDisableParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parsedRequest.errorBuffer);
        }

        try
        {
            // Verify the requester has at least trust level 4

            STAFResult trustResult = STAFUtil.validateTrust(
                4, fServiceName, "DISABLE", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Resolve the ID option

            resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                "id", parsedRequest.optionValue("id"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            id = new Integer(resolvedValue.result);

            // Update the state for the specified ID, or if the ID does not
            // exist, return an error

            CronData2 data = null;

            synchronized (fRegTable)
            {
                // Check if registration table contains the ID

                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, "ID " + id + " not found");
                }

                data = (CronData2)fRegTable.get(id);
                data.fState = "Disabled";

                fRegTable.put(id, data);

                // Save registration data to a file

                sResult = saveHashtable();
            }

            if (sResult.rc != STAFResult.Ok) return sResult;
            
            // Log message that says the registration has been disabled

            String message = "[ID=" + id + "] [" + info.endpoint + ", " +
                    info.handleName + "] Disabled a STAF command.";

            log("info", message);
        }
        catch (Exception e)
        {
            if (DEBUG) e.printStackTrace();
            
            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + e.toString());

            return new STAFResult(STAFResult.JavaError, getStackTrace(e));
        }

        return sResult;
    }

    private STAFResult handleRegister(STAFServiceInterfaceLevel30.RequestInfo
                                      info)
    {
        STAFResult sResult = new STAFResult(STAFResult.Ok, "");
        String description = "";
        String originMachine = info.endpoint;
        String machine = "";
        boolean pythonMachine = true;
        String service = "";
        boolean pythonService = true;
        String request = "";
        boolean pythonRequest = true;
        String prepare = "";
        boolean once = false;
        String state = "Enabled";

        STAFResult resolvedResult = null;
        STAFResult resolvedValue = null;
        STAFCommandParseResult parsedRequest = fRegisterParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // For each option, if the user has specified a value, set the
        // corresponding local variable to the user-specified value.

        try
        {
            // Verify the requester has at least trust level 5

            STAFResult trustResult = STAFUtil.validateTrust(
                5, fServiceName, "REGISTER", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Process the request

            if (parsedRequest.optionTimes("description") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("description"),
                    fHandle, info.requestNumber);
                    
                description = resolvedValue.result;
            }

            if (parsedRequest.optionTimes("machine") > 0)
            {
                machine = parsedRequest.optionValue("machine");

                if (fOldVarResolution)
                {
                    // Resolve the MACHINE option

                    resolvedValue = STAFUtil.resolveRequestVar(
                        machine, fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    machine = resolvedValue.result;
                }

                pythonMachine = false;
            }
            else if (parsedRequest.optionTimes("pythonmachine") > 0)
            {
                machine = parsedRequest.optionValue("pythonmachine");
                pythonMachine = true;
            }

            if (parsedRequest.optionTimes("service") > 0)
            {
                service = parsedRequest.optionValue("service");

                if (fOldVarResolution)
                {
                    // Resolve the SERVICE option

                    resolvedValue = STAFUtil.resolveRequestVar(
                        service, fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    service = resolvedValue.result;
                }

                pythonService = false;
            }
            else if (parsedRequest.optionTimes("pythonservice") > 0)
            {
                service = parsedRequest.optionValue("pythonservice");
                pythonService = true;
            }

            if (parsedRequest.optionTimes("request") > 0)
            {
                request = parsedRequest.optionValue("request");

                if (fOldVarResolution)
                {
                    // Resolve the REQUEST option

                    resolvedValue = STAFUtil.resolveRequestVar(
                        request, fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    request = resolvedValue.result;
                }

                pythonRequest = false;
            }
            else if (parsedRequest.optionTimes("pythonrequest") > 0)
            {
                request = parsedRequest.optionValue("pythonrequest");
                pythonRequest = true;
            }

            if (parsedRequest.optionTimes("prepare") > 0)
            {
                prepare = parsedRequest.optionValue("prepare");
            }

            if (parsedRequest.optionTimes("enabled") > 0)
            {
                state = "Enabled";
            }
            else if (parsedRequest.optionTimes("disabled") > 0)
            {
                state = "Disabled";
            }

            Vector<String> minute  = new Vector<String>();
            Vector<String> hour    = new Vector<String>();
            Vector<String> day     = new Vector<String>();
            Vector<String> month   = new Vector<String>();
            Vector<String> weekday = new Vector<String>();

            if (parsedRequest.optionTimes(kMinute) > 0)
            {
                minute = cronParse(parsedRequest.optionValue(kMinute), kMinute);

                if (minute == null)
                {
                    return new STAFResult(
                        STAFResult.InvalidValue,
                        "Invalid value for " + kMinute + ":  " +
                        parsedRequest.optionValue(kMinute));
                }
            }

            if (parsedRequest.optionTimes(kHour) > 0)
            {
                hour = cronParse(parsedRequest.optionValue(kHour), kHour);

                if (hour == null)
                {
                    return new STAFResult(
                        STAFResult.InvalidValue,
                        "Invalid value for " + kHour + ":  " +
                        parsedRequest.optionValue(kHour));
                }
            }

            if (parsedRequest.optionTimes(kDay) > 0)
            {
                day = cronParse(parsedRequest.optionValue(kDay), kDay);

                if (day == null)
                {
                    return new STAFResult(
                        STAFResult.InvalidValue,
                        "Invalid value for " + kDay + ":  " +
                        parsedRequest.optionValue(kDay));
                }
            }

            if (parsedRequest.optionTimes(kMonth) > 0)
            {
                month = cronParse(parsedRequest.optionValue(kMonth), kMonth);

                if (month == null)
                {
                    return new STAFResult(
                        STAFResult.InvalidValue,
                        "Invalid value for " + kMonth + ":  " +
                        parsedRequest.optionValue(kMonth));
                }
            }

            if (parsedRequest.optionTimes(kWeekday) > 0)
            {
                weekday = cronParse(parsedRequest.optionValue(kWeekday),
                                    kWeekday);

                if (weekday == null)
                {
                    return new STAFResult(
                        STAFResult.InvalidValue,
                        "Invalid value for " + kWeekday + ":  " +
                        parsedRequest.optionValue(kWeekday));
                }
            }

            if (parsedRequest.optionTimes("once") > 0)
            {
                once = true;
            }

            if (!(prepare.equals("")) || pythonMachine || pythonService ||
                pythonRequest)
            {
                // Validate Python code by compiling it
                
                if (!(prepare.equals("")))
                {
                    try
                    {
                        CronPythonInterpreter.compileForPython(prepare);
                    }
                    catch (PyException ex)
                    {
                        String msg = "Python code compile failed for the " +
                            "PREPARE value:\n" + prepare +
                            "\n\nPyException:\n" + ex;

                        return new STAFResult(PYTHONERROR, msg);
                    }
                }

                if (pythonMachine)
                {
                    try
                    {
                        CronPythonInterpreter.compileForPython(machine);
                    }
                    catch (PyException ex)
                    {
                        String msg = "Python code compile failed for the " +
                            "MACHINE value:\n" + machine +
                            "\n\nPyException:\n" + ex;

                            return new STAFResult(PYTHONERROR, msg);
                    }
                }

                if (pythonService)
                {
                    try
                    {
                        CronPythonInterpreter.compileForPython(service);
                    }
                    catch (PyException ex)
                    {
                        String msg = "Python code compile failed for the " +
                            "SERVICE value:\n" + service +
                            "\n\nPyException:\n" + ex;

                            return new STAFResult(PYTHONERROR, msg);
                    }
                }

                if (pythonRequest)
                {
                    try
                    {
                        CronPythonInterpreter.compileForPython(request);
                    }
                    catch (PyException ex)
                    {
                        String msg = "Python code compile failed for the " +
                            "REQUEST value:\n" + request +
                            "\n\nPyException:\n" + ex;

                            return new STAFResult(PYTHONERROR, msg);
                    }
                }
            }
            
            // Add the new registration data to the registration table

            Integer index = null;

            synchronized (fRegTable)
            {
                index = new Integer(fCronID++);

                fRegTable.put(
                    index, (Object) new CronData2(
                        description, originMachine, machine, pythonMachine,
                        service, pythonService, request, pythonRequest,
                        prepare, minute, hour, day, month, weekday, once,
                        state));

                sResult = new STAFResult(STAFResult.Ok, index.toString());

                // Save registration data to a file

                STAFResult result = saveHashtable();

                if (result.rc != STAFResult.Ok) return result;
            }
                        
            // Log message that the registration was successful

            String message = "[ID=" + index + "] [" + info.endpoint + ", " +
                info.handleName +
                "] Registered a STAF command.\nRegister request: " +
                info.request;
            
            log("info", message);
        }
        catch (Exception e)
        {
            if (DEBUG) e.printStackTrace();

            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + e.toString());

            return new STAFResult(STAFResult.JavaError, getStackTrace(e));
        }

        return sResult;
    }

    private STAFResult handleUnregister(STAFServiceInterfaceLevel30.RequestInfo
                                        info)
    {
        STAFResult sResult = new STAFResult(STAFResult.Ok, "");
        String service;
        String request;
        Integer id;

        STAFResult resolvedResult = null;
        STAFResult resolvedValue = null;
        STAFCommandParseResult parsedRequest = fUnregisterParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // For each option, if the user has specified a value, set the
        // corresponding local variable to the user-specified value.

        try
        {
            // Verify the requester has at least trust level 4

            STAFResult trustResult = STAFUtil.validateTrust(
                4, fServiceName, "UNREGISTER", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Process the request

            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("id"), fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;
            
            id = new Integer(resolvedValue.result);

            // Remove the specified id from the registration table, or if the
            // id does not exist, return an error

            synchronized (fRegTable)
            {
                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(STAFResult.DoesNotExist,
                                          "ID " + id + " not found");
                }
            
                // Registration id exists,  Remove it.

                CronData2 data = (CronData2)fRegTable.get(id);

                fRegTable.remove(id);

                // If no registrations, reset the cron id to 1

                if (fRegTable.isEmpty())
                {
                    fCronID = 1;
                }

                // Save registration data to a file

                sResult = saveHashtable();
            }

            if (sResult.rc != STAFResult.Ok) return sResult;
                
            // Log message that the un-registration was successful

            String message = "[ID=" + id + "] [" + info.endpoint + ", " +
                info.handleName + "] Unregistered a STAF command.";

            log("info", message);
        }
        catch (Exception e)
        {
            if (DEBUG) e.printStackTrace();

            log("error", "Exception occurred submitting: " + info.request +
                fLineSep + e.toString());

            return new STAFResult(STAFResult.JavaError, getStackTrace(e));
        }

        return sResult;
    }

    private STAFResult handleVersion(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "VERSION", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fVersionParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        if (parsedRequest.optionTimes("JYTHON") == 0)
        {
            // Return the version of the service
            return new STAFResult(STAFResult.Ok, kVersion);
        }
        else
        {
            // Return the version of Python packaged with the service
            return new STAFResult(STAFResult.Ok, fJythonVersion);
        }
    }
    
    private STAFResult handleDebug(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 5

        STAFResult trustResult = STAFUtil.validateTrust(
            5, fServiceName, "DEBUG", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Get the contents of the fRequestMap and the fProcessHandleMap

        STAFMarshallingContext mc = new STAFMarshallingContext();

        Map<String, Map> resultMap = new HashMap<String, Map>();

        resultMap.put("RequestMap", fRequestMap);
        resultMap.put("ProcessHandleMap", fProcessHandleMap);

        mc.setRootObject(resultMap);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    /**
     * Returns the stack trace of an exception as a String.
     */
    public static String getStackTrace(Throwable aThrowable)
    {
        final Writer result = new StringWriter();
        final PrintWriter printWriter = new PrintWriter(result);
        aThrowable.printStackTrace(printWriter);
        return result.toString();
    }

    static private Vector<String> cronParse(String msg, String type)
    {
        Vector<String> result = new Vector<String>();
        
        if (msg.equalsIgnoreCase("ANY") || msg.equals("*"))
        {
            result.add("ANY");
            return result;
        }
                
        StringTokenizer options = new StringTokenizer(msg, ",");
            
        while (options.hasMoreElements())
        {
            String element = ((String)options.nextElement()).trim();
        
            // Check if a range was specified:  <start>-<end>

            int firstDashIndex = element.indexOf("-");

            if (firstDashIndex == -1)
            {
                // Single value specified.  Verify that it is valid.
                    
                if (!isValidValue(element, type))
                    return null;

                if (type.equals(kWeekday))
                {
                    int numericWeekday = getNumericWeekday(element);

                    if (numericWeekday == -1)
                    {
                        result.add(new
                            String(new Integer(element).toString()));
                    }
                    else
                    {
                        result.add(new String(
                            new Integer(numericWeekday).toString()));
                    }
                }
                else
                {
                    result.add(new String(new Integer(element).toString()));
                }
            }
            else
            {
                // Range was specified

                if (element.indexOf("-", firstDashIndex + 1) > -1)
                {
                    // Invalid.  Can only contain one dash.
                    return null;
                }

                StringTokenizer items = new StringTokenizer(element, "-");

                String start = null;
                String end = null;

                while (items.hasMoreElements())
                {
                    if (start == null)
                        start = ((String)items.nextElement()).trim();
                    else
                        end = ((String)items.nextElement()).trim();
                }

                // Verify that the values specified for the range are valid
                
                if (!isValidValue(start, type))
                    return null;

                if (!isValidValue(end, type))
                    return null;

                // Types other than weekday will always contain numeric values
                // A Weekday type can contain a numeric value or a string value

                if (!type.equals(kWeekday))
                {
                    // Type is kMonth, kDay, kHour, or kMinute

                    // Iterate through the numeric range

                    int startNum = new Integer(start).intValue();
                    int endNum = new Integer(end).intValue();

                    for (int i = startNum; i <= endNum; i++)
                    {
                        result.add(new String(new Integer(i).toString()));
                    }
                }
                else
                {
                    // Type is kWeekday

                    // Check if using string or numeric values for weekday.

                    boolean startIsNumeric = true;
                    boolean endIsNumeric = true;
                        
                    if (equalsWeekday(start)) startIsNumeric = false;
                    if (equalsWeekday(end)) endIsNumeric = false;
                        
                    int startNum = 0;
                    int endNum = 0;

                    // Get the corresponding numeric value for weekday if
                    // specified as a string, e.g. 0 for "Sunday".

                    if (startIsNumeric)
                        startNum = new Integer(start).intValue();
                    else
                        startNum = getNumericWeekday(start);

                    if (endIsNumeric)
                        endNum = new Integer(end).intValue();
                    else
                        endNum = getNumericWeekday(end);
                        
                    // Iterate throught the range, using the String or numeric
                    // representation based on the start value.

                    for (int i = startNum; i <= endNum; i++)
                    {
                        result.add(new String(new Integer(i).toString()));
                    }
                }
            }
        }

        return result;
    }
    
    static private boolean isValidValue(String value, String type)
    {
        if (value == null)
            return false;

        int valueNum = 0;

        if (type.equals(kWeekday))
        {
            if (equalsWeekday(value))
                return true;
        }
            
        try
        {
            valueNum = new Integer(value).intValue();
        }
        catch (NumberFormatException ex)
        {
            return false;
        }
        
        if (type.equals(kMinute))
        {
            if (valueNum >= 0 && valueNum <= 59)
                return true;
            else
                return false;
        }
        else if (type.equals(kHour))
        {
            if (valueNum >= 0 && valueNum <= 23)
                return true;
            else
                return false;
        }
        else if (type.equals(kDay))
        {
            if (valueNum >= 1 && valueNum <= 31)
                return true;
            else
                return false;
        }
        else if (type.equals(kMonth))
        {
            if (valueNum >= 1 && valueNum <= 12)
                return true;
            else
                return false;
        }
        else if (type.equals(kWeekday))
        {
            if (valueNum >= 0 && valueNum <= 6)
                return true;
            else
                return false;
        }
        else
        {
            return false;
        }
    }

    static private boolean equalsWeekday(String msg)
    {
         if ((msg.equalsIgnoreCase("Sunday")) ||
             (msg.equalsIgnoreCase("Monday")) ||
             (msg.equalsIgnoreCase("Tuesday")) ||
             (msg.equalsIgnoreCase("Wednesday")) ||
             (msg.equalsIgnoreCase("Thursday")) ||
             (msg.equalsIgnoreCase("Friday")) ||
             (msg.equalsIgnoreCase("Saturday")))
         {
            return true;
         }
         else
         {
            return false;
         }
    }

    static private int getNumericWeekday(String weekday)
    {
         if (weekday.equalsIgnoreCase("Sunday"))
             return 0;
         else if (weekday.equalsIgnoreCase("Monday"))
             return 1;
         else if (weekday.equalsIgnoreCase("Tuesday"))
             return 2;
         else if (weekday.equalsIgnoreCase("Wednesday"))
             return 3;
         else if (weekday.equalsIgnoreCase("Thursday"))
             return 4;
         else if (weekday.equalsIgnoreCase("Friday"))
             return 5;
         else if (weekday.equalsIgnoreCase("Saturday"))
             return 6;
         else
             return -1;
    }

    static private String getStringWeekday(int weekday)
    {
         if (weekday == 0)
             return "Sunday";
         else if (weekday == 1)
             return "Monday";
         else if (weekday == 2)
             return "Tuesday";
         else if (weekday == 3)
             return "Wednesday";
         else if (weekday == 4)
             return "Thursday";
         else if (weekday == 5)
             return "Friday";
         else if (weekday == 6)
             return "Saturday";
         else
             return "";
    }

    protected boolean getTerminated()
    {
        return fTerminated;
    }

    protected void setTerminated()
    {
        fTerminated = true;
    }

    protected void log(String level, String message)
    {
        fHandle.submit2(
            "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
            " LEVEL " + level + " MESSAGE " + STAFUtil.wrapData(message));
    }

    /**
     * Triggers the STAF service request specified by the registration id.
     * 
     * @param id The Cron registration id
     * @param theData The Cron registration data
     * @param triggerRequest A boolean flag indicating if triggered by a
     * CRON TRIGGER request
     * @param currTimestamp A cron timestamp object providing information
     * about the the time triggers, or null if triggerRequest is true
     * @param scripts A list of SCRIPT option values or null if triggerRequest
     * is false
     * 
     * @return A STAFResult is returned containing the result from triggering
     * the STAF service request specified by the registration id (which is
     * needed if triggered by a CRON TRIGGER request).
     */ 
    private STAFResult trigger(Integer id, CronData2 theData,
                               boolean triggerRequest,
                               CronTimestamp currTimestamp,
                               LinkedList<String> scripts)
    {
        String machine = theData.fMachine;
        String service = theData.fService;
        String request = theData.fRequest;
        String prepare = theData.fPrepare;
        String state   = theData.fState;

        boolean pythonMachine = theData.fPythonMachine;
        boolean pythonService = theData.fPythonService;
        boolean pythonRequest = theData.fPythonRequest;

        // Check if any Python data was provided in the registration

        if ((prepare != null && !prepare.equals("")) ||
            pythonMachine || pythonService || pythonRequest ||
            ((scripts != null) && (scripts.size() > 0)))
        {
            String regInfoMsg = "  ID: " + id.toString() +
                "\n  machine: " + machine +
                "\n  service: " + service +
                "\n  request: " + request +
                "\n  prepare: " + prepare;

            // Create a new Python Interpreter for a triggered registration

            CronPythonInterpreter myPyInt = new CronPythonInterpreter(
                this, id);
  
            // Check if a TRIGGER request is the source

            if (triggerRequest)
            {
                // Execute Python code in the SCRIPT options, if any

                for (int j = 0; j < scripts.size(); j++)
                {
                    String script = scripts.get(j);

                    try
                    {
                        myPyInt.pyExec(script);
                    }
                    catch (PyException e)
                    {
                        String pythonError = "Python error in the " +
                            "SCRIPT value.\n\nSCRIPT:\n" + script +
                            "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg;

                        String msg = "[ID=" + id + "] " + pythonError;

                        log("error", msg);

                        return new STAFResult(PYTHONERROR, pythonError);
                    }
                }
            }

            if (prepare != null && !prepare.equals(""))
            {
                try
                {
                    myPyInt.pyExec(prepare);
                }
                catch (PyException ex)
                {
                    String msg = "[ID=" + id +
                        "] Python error in the PREPARE value." +
                        "\n\nPREPARE:\n" + prepare +
                        "\n\nPyException:\n" + ex +
                        "\nProcessing registration:\n" + regInfoMsg;

                    log("error", msg);

                    return new STAFResult(PYTHONERROR, msg);
                }

                Object submit = myPyInt.pyGetVar("STAFCronSubmit");

                if ((submit == null) ||
                    (!submit.toString().equals("true")))
                {
                    // Do not submit the request for this registration
                    return new STAFResult(REQUESTNOTSUBMITTED,
                        "STAFCronSubmit=" + submit.toString());
                }
            }

            if (pythonMachine)
            {
                try
                {
                    machine = myPyInt.pyStringEval(machine);
                }
                catch (PyException ex)
                {
                    String msg = "[ID=" + id + "] Python error " +
                        "in the PYTHONMACHINE value." +
                        "\n\nPYTHONMACHINE:\n" + machine +
                        "\n\nPyException:\n" + ex +
                        "\nProcessing registration:\n" + regInfoMsg;

                    log("error", msg);

                    return new STAFResult(PYTHONERROR, msg);
                }
            }

            if (pythonService)
            {
                try
                {
                    service = myPyInt.pyStringEval(service);
                }
                catch (PyException ex)
                {
                    String msg = "[ID=" + id + "] Python error " +
                        "in the PYTHONSERVICE value." +
                        "\n\nPYTHONSERVICE:\n" + service +
                        "\n\nPyException:\n" + ex +
                        "\nProcessing registration:\n" + regInfoMsg;

                    log("error", msg);

                    return new STAFResult(PYTHONERROR, msg);
                }
            }

            if (pythonRequest)
            {
                try
                {
                    request = myPyInt.pyStringEval(request);
                }
                catch (PyException ex)
                {
                    String msg = "[ID=" + id + "] Python error " +
                        "in the PYTHONREQUEST value." +
                        "\n\nPYTHONREQUEST:\n" + request +
                        "\n\nPyException:\n" + ex +
                        "\nProcessing registration:\n" + regInfoMsg;

                    log("error", msg);

                    return new STAFResult(PYTHONERROR, msg);
                }
            }
        }

        if (!triggerRequest)
        {
            // Don't check if enabled if source is a TRIGGER request

            if (!state.equals("Enabled"))
            {
                String message = "[ID=" + id.toString() + "] " +
                    "ID is disabled. STAF command not submitted." +
                    "\nTriggers: " + currTimestamp.getTriggers();

                if (fDebug)
                {
                    message += currTimestamp.getTimeDetails();
                }

                message += "\nSTAF command: STAF " + machine + " " +
                    service + " " + request;

                log("info", message);
            
                return new STAFResult(STAFResult.Ok);
            }
        }
        
        // Check if a PROCESS START request is being submitted without the
        // WAIT option and without the NOTIFY ONEND option and, if so, add
        // "NOTIFY ONEND Handle <ServiceHandle>" to the request and return
        // the updated request.

        if (service.equalsIgnoreCase("PROCESS"))
        {
            request = updateProcessStartRequest(request);
        }
        
        // Get the machine name for the endpoint

        String machineName = machine;

        STAFResult varResult = fHandle.submit2(
            machine, "VAR", "RESOLVE STRING {STAF/Config/Machine}");

        if (varResult.rc == STAFResult.Ok)
        {
            machineName = varResult.result;
        }

        // Submit the STAF service request

        STAFResult submitResult = fHandle.submit2(
            STAFHandle.ReqQueue, machine, service, request);

        if (submitResult.rc != STAFResult.Ok)
        {
            // Error submitting the STAF service request

            String message = "[ID=" + id.toString() +
                "] " + "Error submitting a STAF command." +
                "\nTriggers: " + currTimestamp.getTriggers();

            if (fDebug)
            {
                message += currTimestamp.getTimeDetails();
                message += currTimestamp.getRegData();
            }

            message += "\nSTAF command: STAF " + machine + " " +
                service + " " + request + "\nSubmit RC: " + submitResult.rc +
                ", Result: " + submitResult.result;

            log("error", message);

            return submitResult;
        }

        String requestNumber = submitResult.result;

        fRequestMap.put(
            requestNumber, new RequestData(
                machine, service, request, id.toString(), machineName));

        String message = "[ID=" + id.toString() + "] " +
            "[" + machineName + ":" + requestNumber +
            "] Submitted a STAF command.";

        if (triggerRequest)
        {
            message += "\nTriggered manually.";
        }
        else
        {
            message += "\nTriggers: " + currTimestamp.getTriggers();

            if (fDebug)
            {
                message += currTimestamp.getTimeDetails();
                message += currTimestamp.getRegData();
            }
        }

        message += "\nSTAF command: STAF " + machine + " " + service +
            " " + request;

        log("info", message);

        if (triggerRequest)
        {
            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fTriggerMapClass);

            // Create an empty result map to contain the result

            // Not calling STAFMapClass createInstance method to avoid
            // getting an unchecked cast warning
            //Map resultMap = fTriggerMapClass.createInstance();
            Map<String, Object> resultMap = new TreeMap<String, Object>();
            resultMap.put("staf-map-class-name", fTriggerMapClass.name());

            resultMap.put("machine", machineName);
            resultMap.put("requestNumber", requestNumber);

            mc.setRootObject(resultMap);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }

        if (theData.fOnce)
        {
            synchronized (fRegTable)
            {
                // Remove the registation data

                fRegTable.remove(id);
            
                // If no registrations, reset the cron id to 1

                if (fRegTable.isEmpty())
                {
                    fCronID = 1;
                }

                // Save registration data to a file

                STAFResult result = saveHashtable();

                if (result.rc != STAFResult.Ok) return result;
            }

            // Log message that the un-registration was successful
            
            String msg = "[ID=" + id + "] [local://local, " +
                fServiceHandleName + "] Unregistered a STAF command.";

            log("info", msg);
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Checks if a PROCESS START request is being submitted without the WAIT
     * option and without the NOTIFY ONEND option.  If so, adds "NOTIFY ONEND
     * HANDLE <serviceHandle>" to the PROCESS START request so that the
     * service will be notified when the process completes and so the service
     * won't have to submit a PROCESS REGISTER ONEND request later (which may
     * be too late if the process completes very quickly).
     * @params request The PROCESS START request string
     */
    private String updateProcessStartRequest(String request)
    {
        String upperRequest = request.toUpperCase();
        String startString = "START ";
         
        if ((upperRequest.indexOf(startString) == 0) &&
            (upperRequest.indexOf(" WAIT") == -1) &&
            (upperRequest.indexOf(" NOTIFY ONEND") == -1))
        {
            // Insert "NOTIFY ONEND HANDLE fServiceHandleNumber" after
            // "START " and before the rest of the request to minimize the
            // chance of messing up the request syntax if the last option
            // value isn't "wrapped".
 
            int startLength = startString.length();

            return request.substring(
                0, startLength) + "NOTIFY ONEND HANDLE " +
                fServiceHandleNumber + " " + request.substring(startLength);
        }
        else
        {
            return request;
        }
    }

    // Note: Always call loadHashtable when synchronized on fHashtable

    private void loadHashtable() throws IOException, ClassNotFoundException
    {
        loadHashtable(false);
    }

    // Note: Always call loadHashtable when synchronized on fHashtable

    private void loadHashtable(boolean firstLoad)
        throws IOException, ClassNotFoundException
    {
        File fHashtableFile = new File(fHashtableFileName);

        if (!fHashtableFile.exists())
        {
            fRegTable = new Hashtable<Integer, Object>();
            fCronID = 1;
        }
        else
        {
            ObjectInputStream ois =  new ObjectInputStream(
                new FileInputStream(fHashtableFile));

            // Java cannot check that the object is actually a Hashtable
            // with a Integer type for the key so instead of doing the
            // following which gets an unchecked cast warning
            //   fRegTable = (Hashtable<Integer, Object>)(ois.readObject());
            // doing it this way:

            Hashtable tempRegTable = (Hashtable)(ois.readObject());
            ois.close();

            fRegTable = new Hashtable<Integer, Object>(tempRegTable.size());
            
            for (Enumeration e = tempRegTable.keys(); e.hasMoreElements();)
            {
                Integer id = (Integer)e.nextElement();
                fRegTable.put(id, tempRegTable.get(id));
            }

            tempRegTable = null;

            fCronID = findNextCronID();

            // If initializing the service, may need to convert the data to
            // the latest format

            if (firstLoad)
                convertOldCronData();
        }
    }
    
    // Note: Always call convertOldCronData when synchronized on fHashtable

    private void convertOldCronData()
    {
        // In Cron V3.2.0, a new field, fOnce, was added to the Cron
        // registation data.  A new class was created named CronData1 and will
        // be used in V3.2.0+.  When registering the Cron V3.2.0+ service for
        // the first time, if there are any instances of class CronData in the
        // registration Hashtable, they must be converted from class CronData
        // to CronData1.
        // In Cron V3.3.0, a new field, fState, was added to the Cron
        // registation data.  A new class was created named CronData2 and will
        // be used in V3.3.0+.  When registering the Cron V3.3.0+ service for
        // the first time, if there are any instances of class CronData1 in the
        // registration Hashtable, they must be converted from class CronData1
        // to CronData2.

        Hashtable<Integer, Object> newRegTable =
            new Hashtable<Integer, Object>();

        for (Enumeration<Integer> e = fRegTable.keys(); e.hasMoreElements();)
        {
            Integer id = e.nextElement();

            Object cronDataObj = fRegTable.get(id);

            try
            {
                CronData2 data = (CronData2) cronDataObj;

                newRegTable.put(id, (Object) data);
            }
            catch (ClassCastException ex)
            {
                try
                {
                    CronData1 data = (CronData1) cronDataObj;

                    CronData2 newData = new CronData2(data.fDescription,
                                                      data.fOriginMachine,
                                                      data.fMachine,
                                                      data.fPythonMachine,
                                                      data.fService,
                                                      data.fPythonService,
                                                      data.fRequest,
                                                      data.fPythonRequest,
                                                      data.fPrepare,
                                                      data.fMinute,
                                                      data.fHour,
                                                      data.fDay,
                                                      data.fMonth,
                                                      data.fWeekday,
                                                      data.fOnce,
                                                      "Enabled");

                    newRegTable.put(id, (Object) newData);
                }
                catch (ClassCastException ex2)
                {
                    CronData data = (CronData) cronDataObj;

                    CronData2 newData = new CronData2("",
                                                      data.fOriginMachine,
                                                      data.fMachine,
                                                      data.fPythonMachine,
                                                      data.fService,
                                                      data.fPythonService,
                                                      data.fRequest,
                                                      data.fPythonRequest,
                                                      data.fPrepare,
                                                      data.fMinute,
                                                      data.fHour,
                                                      data.fDay,
                                                      data.fMonth,
                                                      data.fWeekday,
                                                      false,
                                                      "Enabled");

                    newRegTable.put(id, (Object) newData);
                }
            }
        }

        fRegTable = newRegTable;
    }

    /**
     * Rename a file
     * 
     * Note: Always call renameFile when synchronized on fHashtable
     */
    private boolean renameFile(File oldFile, File newFile)
    {
        newFile.delete();

        return oldFile.renameTo(newFile);
    }
    
    /**
     * Restore the cron.ser file from the backup
     * 
     * Note: Always call restoreBackupFile when synchronized on fHashtable
     */ 
    private STAFResult restoreBackupFile()
    {
        // Copy the file

        String copyRequest = "COPY FILE " +
            STAFUtil.wrapData(fBackupFileName) +
            " TOFILE " + STAFUtil.wrapData(fHashtableFileName) +
            " TOMACHINE local";

        STAFResult res = fHandle.submit2("local", "FS", copyRequest);

        if (res.rc != STAFResult.Ok)
        {
            log("Error", "Restoring the cron.ser file from the backup " +
                "failed.  RC=" + res.rc + ", Result=" + res.result +
                ", Request=" + copyRequest);

            return res;
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Save the Cron service's registration data currently in memory to
     * a file.  If an error occurs writing to the file, restore the
     * registration data to it's previous state to keep the data in memory in
     * sync with the data in the file.
     * 
     * Note: Always call saveHashtable when synchronized on fHashtable
     */ 
    private STAFResult saveHashtable()
    {
        // Backup the Cron.service's registration file

        boolean haveBackup = renameFile(
            new File(fHashtableFileName), new File(fBackupFileName));

        // Write the cron registration data to a file

        try
        {
            ObjectOutputStream oos = new ObjectOutputStream(
                new FileOutputStream(fHashtableFileName));
            oos.writeObject(((Object)(fRegTable)));
            oos.close();
        }
        catch (IOException ex)
        {
            // Writing the registration data to a file failed.
            // Use the backup file to restore registration data in memory
            // to keep them in sync (and to back out the change just made).

            // Log to service log and print stack trace to JVM log
            
            ex.printStackTrace();

            String errorMsg = "Saving registration data to file " +
                fHashtableFileName + " failed. " + fLineSep + ex + fLineSep +
                "See the service log for more information and contact the " +
                "system administrator for the service machine.";

            log("Error", errorMsg);

            if (haveBackup)
            {
                // Restore from the backup registration file

                STAFResult res = restoreBackupFile();

                if (res.rc == STAFResult.Ok)
                {
                    try
                    {
                        // Reload registration data from backup file

                        loadHashtable();

                        log("Info", "Successfully reloaded registration " +
                            "data in memory from backup file");
                    }
                    catch (Exception e)
                    {
                        log("Error", "Registration data in memory is " +
                            "out of sync with data stored in file " +
                            fHashtableFileName + fLineSep + e);
                    }
                }
                else
                {
                    System.out.println("Restore from file " + fBackupFile +
                                       " failed");
                    log("Error", "Registration data in memory is " +
                        "out of sync with data stored in file " +
                        fHashtableFileName);
                }
            }
            else
            {
                log("Error", "Registration data in memory is " + 
                    "out of sync with data stored in file " +
                    fHashtableFileName + " with no backup file");
            }

            return new STAFResult(STAFResult.FileWriteError, errorMsg);
        }

        return new STAFResult(STAFResult.Ok);
    }

    // Note: Always call findNextCronID when synchronized on fHashtable

    private int findNextCronID()
    {
        int highestID = 0;
        for (Enumeration<Integer> e = fRegTable.keys(); e.hasMoreElements();)
        {
            Integer id = e.nextElement();
            if (id.intValue() > highestID)
            {
                highestID = id.intValue();
            }
        }

        return highestID + 1;
    }

    public STAFResult term()
    {
        fTerminated = true;

        fHandle.submit2(
            STAFHandle.ReqFireAndForget,
            "local", "QUEUE", "QUEUE TYPE " + sQueueTypeEnd +
            " MESSAGE " + STAFUtil.wrapData(""));

        unregisterHelp();

        return new STAFResult(STAFResult.Ok);
    }

    
    /**
     * This helper class thread does the following in a loop until
     * fService.getTerminated() returns true (which indicates that the Cron
     * service is terminating):
     * 1) Sleeps for 10 seconds
     * 2) If the time has changed since last time checked, iterates through
     *    the list of Cron registrations, checking if the current time matches
     *    any registered triggers and, if so, triggers the STAF service
     *    requests specified by the Cron registration.
     */ 
    class TriggerThread extends Thread
    {
        private CronService fService;

        TriggerThread(CronService service)
        {
            fService = service;
        }

        public void run()
        {
            CronTimestamp lastTimestamp = new CronTimestamp();

            while (!fService.getTerminated())
            {
                try
                {
                    // Sleep for 10 seconds so won't be constantly checking
                    // if any Cron registration triggers match

                    Thread.sleep(10000);

                    CronTimestamp currTimestamp = new CronTimestamp();

                    if (!currTimestamp.hasChanged(lastTimestamp))
                    {
                        // The timestamp up to the minute only (not seconds)
                        // has not changed since last time checked so don't
                        // need to check if any Cron registrations need to
                        // be triggered
                        
                        continue;
                    }

                    // Check if the current time matches any registered
                    // triggers and, if so, trigger the STAF service requests
                    // specified by those Cron registrations

                    CronData2 regData;

                    for (Enumeration<Integer> e = fService.fRegTable.keys();
                         e.hasMoreElements() && !fService.getTerminated();)
                    {
                        Integer id = e.nextElement();
                        regData = (CronData2)fService.fRegTable.get(id);
                        
                        if (regData == null)
                        {
                            // Skip since this registration no longer exists

                            continue;
                        }

                        if (!currTimestamp.matchesRegisteredTriggers(regData))
                        {
                            // Skip since current time doesn't match the
                            // triggers for this Cron registration

                            continue;
                        }

                        // Trigger the STAF Command in the Cron registration ID

                        STAFResult result = fService.trigger(
                            id, regData, false, currTimestamp, null);

                        if (result.rc == STAFResult.HandleDoesNotExist)
                        {
                            break;  // Service must have been terminated
                        }
                    }
                }
                catch (Exception e)
                {
                    fService.log(
                        "error", "Exception in TriggerThread:\n" + e);
                }
            } // end while !fService.getTerminated()
        }
    }

    /**
     * This helper class thread does the following in a loop until it gets
     * a message that the Cron service is terminating:
     * - Waits indefinitely for one or more messages to appear on the Cron
     *   service handle's queue and processes all messages with types:
     *   - STAF/RequestComplete
     *   - STAF/PROCESS/END
     *   - STAF/Service/Cron/End
     */ 
    class MonitorThread extends Thread
    {
        private CronService fService;

        MonitorThread(CronService service)
        {
            fService = service;
        }

        public void run()
        {
            STAFResult getResult = new STAFResult();
            boolean continueRunning = true;
            int numErrors = 0;

            // Maximum consecutive errors submitting a local QUEUE GET WAIT
            // request before we decide to exit the infinite loop
            int maxErrors = 5;
            
            while (continueRunning)
            {
                // Need a try/catch block in case an error occurs getting
                // message(s) off the queue so we can continue processing
                // additional messages

                try
                {
                    // Use the ALL option to improve performance by getting
                    // multiple messages, if available, off the queue at once

                    getResult = fService.fHandle.submit2(
                        "local", "QUEUE", "GET ALL WAIT");

                    if (getResult.rc == 0)
                    {
                        numErrors = 0;
                    }
                    else if (getResult.rc == STAFResult.HandleDoesNotExist)
                    {
                        // If the handle doesn't exist, all submit requests
                        // using this handle will fail so exit this thread

                        continueRunning = false;  // Exit MonitorThread
                        break;
                    }
                    else
                    {
                        numErrors++;

                        String msg = "STAF local QUEUE GET ALL WAIT failed " +
                            " with RC: " + getResult.rc +
                            ", Result: " + getResult.result;

                        fService.log("error", msg);

                        if (numErrors < maxErrors)
                        {
                            continue;
                        }
                        else
                        {
                            msg = "Exiting MonitorThread after the " +
                                "QUEUE GET request failed " + maxErrors +
                                " consecutive times";

                            fService.log("error", msg);

                            continueRunning = false;  // Exit MonitorThread
                            break;
                        }
                    }
                }
                catch (Throwable t)
                {
                    numErrors++;

                    String msg = "Exception getting messages off the queue." +
                        " Exception:\n" + t.toString() + "\nResult from " +
                        "STAF local QUEUE GET ALL WAIT request:\n" +
                        getResult.result;

                    t.printStackTrace();
                
                    fService.log("error", msg);

                    if (numErrors < maxErrors)
                    {
                        continue;
                    }
                    else
                    {
                        msg = "Exiting MonitorThread after the QUEUE GET " +
                            "request failed " + maxErrors +
                            " consecutive times";

                        fService.log("error", msg);

                        continueRunning = false;  // Exit MonitorThread
                        break;
                    }
                }

                try
                {
                    List queueList = (List)getResult.resultObj;
                    
                    // Iterate through the messages we got off our queue

                    Iterator queueIter = queueList.iterator();

                    while (queueIter.hasNext())
                    {
                        Map queueMessageMap = null;

                        // Need a try/catch block so can catch any errors and
                        // continue processing messages in the list

                        try
                        {
                            queueMessageMap = (Map)queueIter.next();

                            String queueType = (String)queueMessageMap.get(
                                "type");

                            if (queueType == null)
                            {
                                continue;
                            }
                            else if (queueType.equalsIgnoreCase(
                                "STAF/RequestComplete"))
                            {
                                handleRequestCompleteMsg(queueMessageMap);
                            }
                            else if (queueType.equalsIgnoreCase(
                                "STAF/PROCESS/END"))
                            {
                                handleProcessEndMsg(queueMessageMap);
                            }
                            else if (queueType.equalsIgnoreCase(
                                fService.sQueueTypeEnd))
                            {
                                // This type of queued message indicates that
                                // the service is terminating

                                continueRunning = false;  // Exit MonitorThread
                                break;
                            }
                        }
                        catch (Exception e)
                        {
                            String msg = "Exception handling queued " +
                                "message. Exception:\n" + e.toString() +
                                "\nQueued Message:\n" + queueMessageMap;

                            e.printStackTrace();

                            fService.log("error", msg);
                        }
                    } // end while more messages in list
                }
                catch (Exception e)
                {
                    String msg = "Exception handling queued messages." +
                        " Exception:\n" + e.toString();

                    e.printStackTrace();
                
                    fService.log("error", msg);
                }
            }
            
            try
            {
                fService.fHandle.unRegister();
            }
            catch (STAFException ex)
            {
                if (DEBUG)
                {
                    ex.printStackTrace();
                }
            }

            if (!fService.getTerminated())
            {
                // Set in case didn't get the sQueueTypeEnd message
                fService.setTerminated(); 
            }
        }
        
        private void handleRequestCompleteMsg(Map queueMessageMap)
        {
            Map messageMap = (Map)queueMessageMap.get("message");

            // A STAF/RequestComplete message is a map containing keys:
            //   requestNumber, rc, result

            String reqNum = (String)messageMap.get("requestNumber");
            String rc = (String)messageMap.get("rc");
            Object result = (Object)messageMap.get("result");

            // Log a message about the completed request

            String level = "";

            if (rc.equals("0"))
                level = "Pass";
            else
                level = "Fail";

            String id = null;
            String machineName = null;

            RequestData requestData = fService.fRequestMap.get(reqNum);
            
            if (requestData != null)
            {
                id = requestData.getId();
                machineName = requestData.getMachineName();
            }
            
            String resultSep = "";
            String resultString = result.toString();

            if (resultString.startsWith("[") || resultString.startsWith("{"))
                resultSep = fLineSep;

            String message = "[ID=" + id + "] [" + machineName + ":" +
                reqNum + "] Completed a STAF command. RC=" + rc +
                ", Result=" + resultSep + resultString;

            fService.log(level, message);

            if (requestData == null)
            {
                return;
            }

            // Check if a START request was successfully submitted to the
            // PROCESS service without the WAIT option

            String service = requestData.getService().toUpperCase();
            String request = requestData.getRequest().toUpperCase();
            String machine = requestData.getMachine();

            if (rc.equals("0") && service.equals("PROCESS") &&
                (request.indexOf("START") == 0)  &&
                (request.indexOf(" WAIT") == -1))
            {
                // Submitted an asynchronous PROCESS START request

                String handle = result.toString();

                fService.fProcessHandleMap.put(
                    machineName + ":" + handle, reqNum);
                
                // Check if the the PROCESS START request contains options to
                // notify the service's handle when the process ends

                if (request.indexOf(" NOTIFY ONEND HANDLE " +
                                    fService.fServiceHandleNumber) == -1)
                {
                    // Not already being notified, so need to register to be
                    // notified when the process ends

                    STAFResult res = fService.fHandle.submit2(
                        machine, "PROCESS", "NOTIFY REGISTER ONENDOFHANDLE " +
                        handle);

                    if (res.rc == STAFResult.Ok)
                    {
                        // Do nothing
                    }
                    else if (res.rc == STAFResult.ProcessAlreadyComplete)
                    {
                        // Get the process rc.  Note that the process returned
                        // file information is not available.

                        res = fService.fHandle.submit2(
                            machine, "PROCESS", "QUERY HANDLE " + handle);

                        if (res.rc == STAFResult.Ok)
                        {
                            Map processMap = (Map)res.resultObj;
                            String processRC = (String)processMap.get("rc");

                            // Log a Pass or Fail message for the process
             
                            if (processRC.equals("0"))
                                level = "Pass";
                            else
                                level = "Fail";
                            
                            message = "[ID=" + id + "] " +
                                "[" + machineName + ":" + reqNum + "] " +
                                "Process completed.  Return Code=" + processRC +
                                fLineSep + fLineSep +
                                "Warning: Process with handle " + handle +
                                " completed before the CRON service could " +
                                "ask to be notified when the process " +
                                "completed so only the process RC " +
                                "can be logged.\nTo fix this, change the " +
                                "registration entry by adding the WAIT " +
                                "option to the PROCESS START request.";

                            fService.log(level, message);
                        }
                        else
                        {
                            // Could not get process return code.

                            // Log a error message about the process
                            // completing before the CRON service could ask
                            // to be notified

                            message = "[ID=" + id + "] [" +
                                machineName + ":" + reqNum +
                                "] Process with handle " + handle +
                                " completed before the CRON service could " +
                                "ask to be notified when the process " +
                                "completed so no process completion info " +
                                "can be logged.\nTo fix this, change the " +
                                "registration entry by adding the WAIT " +
                                "option to the PROCESS START request.";

                            fService.log("error", message);
                        }

                        res = fService.fHandle.submit2(
                            machine, "PROCESS", "FREE HANDLE " + handle);

                        // Remove the corresponding entries from the process
                        // handle map and the submitted request map

                        fService.fProcessHandleMap.remove(
                            machineName + ":" + handle);
                        fService.fRequestMap.remove(reqNum);
                    }
                    else
                    {
                        message = "[ID=" + id + "] [" +
                            machineName + ":" + reqNum +
                            "] Process completion notification to " +
                            "machine " + machine +
                            " failed for handle " + handle +
                            " RC=" + res.rc + " Result=" + res.result;

                        fService.log("error", message);
                    }
                }

                return;
            }
                
            // Remove the request number from the submitted request map

            fService.fRequestMap.remove(reqNum);
        }

        /**
         * Handle an asynchronous process end message
         */
        private void handleProcessEndMsg(Map queueMessageMap)
        {
            Map messageMap = (Map)queueMessageMap.get("message");
            String handle = (String)messageMap.get("handle");
            String rc = (String)messageMap.get("rc");

            String endpoint = (String)queueMessageMap.get("machine");
            
            // Free the process handle

            STAFResult freeHandleResult = fService.fHandle.submit2(
                endpoint, "PROCESS", "FREE HANDLE " + handle);

            // Log a Pass or Fail message in the service log
            
            String level = "";

            if (rc.equals("0"))
                level = "Pass";
            else
                level = "Fail";

            // Resolve the STAF/Config/Machine variable on the endpoint to get
            // the machine name which is used in the key for the
            // fProcessHandleMap and in logged messages

            String machineName = endpoint;

            STAFResult varResult = fService.fHandle.submit2(
                endpoint, "VAR", "RESOLVE STRING {STAF/Config/Machine}");

            if (varResult.rc == STAFResult.Ok)
                machineName = varResult.result;

            String id = null;
            String reqNum = fService.fProcessHandleMap.get(
                machineName + ":" + handle);

            if (reqNum != null)
            {
                RequestData requestData = fService.fRequestMap.get(reqNum);

                if (requestData != null)
                    id = requestData.getId();
                
                fService.fRequestMap.remove(reqNum);
                fService.fProcessHandleMap.remove(machineName + ":" + handle);
            }
            
            // Create a formatted result for the process (like you get if the
            // WAIT option had been used on the PROCESS START request)

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fProcessEndMapClass);
            mc.setMapClassDefinition(fProcessReturnFileMapClass);

            // Not calling STAFMapClass createInstance method to avoid
            // getting an unchecked cast warning
            //Map processEndMap = fProcessEndMapClass.createInstance();
            Map<String, Object> processEndMap = new TreeMap<String, Object>();
            processEndMap.put("staf-map-class-name", fProcessEndMapClass.name());

            processEndMap.put("rc", rc);

            String key = (String)messageMap.get("key");
            
            if (key != null && key.length() != 0)
                processEndMap.put("key", key);
            
            List<Map<String, Object>> returnFileList =
                new ArrayList<Map<String, Object>>();
            List fileList = (List)messageMap.get("fileList");
            Iterator fileIter = fileList.iterator();

            while (fileIter.hasNext())
            {
                Map fileMap = (Map)fileIter.next();

                // Not calling STAFMapClass createInstance method to avoid
                // getting an unchecked cast warning
                //Map returnFileMap =
                //    fProcessReturnFileMapClass.createInstance();
                Map<String, Object> returnFileMap =
                    new TreeMap<String, Object>();
                returnFileMap.put("staf-map-class-name",
                                  fProcessReturnFileMapClass.name());
                returnFileMap.put("rc", (String)fileMap.get("rc"));
                returnFileMap.put("data", (String)fileMap.get("data"));
                returnFileList.add(returnFileMap);
            }

            processEndMap.put("fileList", returnFileList);
            
            mc.setRootObject(processEndMap);

            String message = "[ID=" + id + "] " +
                "[" + machineName + ":" + reqNum + "] " +
                "Process completed.\n" + mc.toString();

            fService.log(level, message);
        }
    }

    /**
     * This helper class encapsulates the time trigger information which
     * is used to determine if it's time to trigger a Cron registration
     */ 
    class CronTimestamp
    {
        private int getMinute() { return fMinute; }
        private int getHour() { return fHour; }
        private int getDay() { return fDay; }
        private int getMonth() { return fMonth; }
        private int getWeekday() { return fWeekday; }

        private boolean isMinuteChanged() { return fIsMinuteChanged; }
        private boolean isHourChanged() { return fIsHourChanged; }
        private boolean isDayChanged() { return fIsDayChanged; }
        private boolean istMonthChanged() { return fIsMonthChanged; }
        private boolean isWeekdayChanged() { return fIsWeekdayChanged; }

        private void setMinute(int minute) { fMinute = minute; }
        private void setHour(int hour) { fHour = hour; }
        private void setDay(int day) { fDay = day; }
        private void setMonth(int month) { fMonth = month; }
        private void setWeekday(int weekday) { fWeekday = weekday; }

        private int fMinute;
        private int fHour;
        private int fDay;
        private int fMonth;
        private int fWeekday;
        private boolean fIsMinuteChanged = false;
        private boolean fIsHourChanged = false;
        private boolean fIsDayChanged = false;
        private boolean fIsMonthChanged = false;
        private boolean fIsWeekdayChanged = false;
        private String fTriggers = "";
        private String fTimeDetails = "";
        private String fRegData = "";

        CronTimestamp()
        {
            Calendar timestamp = Calendar.getInstance();

            fMinute = timestamp.get(Calendar.MINUTE);
            fHour = timestamp.get(Calendar.HOUR_OF_DAY);
            fDay = timestamp.get(Calendar.DAY_OF_MONTH);

            // MONTH is 0-based, so add 1
            fMonth = timestamp.get(Calendar.MONTH) + 1;

            fWeekday = getZeroBasedWeekday(
                timestamp.get(Calendar.DAY_OF_WEEK));
        }
        
        public String toString()
        {
            String hourString = String.valueOf(fHour);

            if (fHour < 10)
                hourString = "0" + hourString; 

            String minuteString = String.valueOf(fMinute);

            if (fMinute < 10)
                minuteString = "0" + minuteString;

            return getStringWeekday(fWeekday) + ", " + fMonth + "/" + fDay +
                ", " + hourString + ":" + minuteString;
        }

        /**
         * Calender.SUNDAY -> Calendar.SATURDAY are not guaranteed to be
         * static values across all JVMs and platforms, so determine the
         * actual weekday and set it our 0-based syntax
         */
        private int getZeroBasedWeekday(int weekday)
        {
            if (weekday == java.util.Calendar.SUNDAY)
                return 0;
            else if (weekday == java.util.Calendar.MONDAY)
                return 1;
            else if (weekday == java.util.Calendar.TUESDAY)
                return 2;
            else if (weekday == java.util.Calendar.WEDNESDAY)
                return 3;
            else if (weekday == java.util.Calendar.THURSDAY)
                return 4;
            else if (weekday == java.util.Calendar.FRIDAY)
                return 5;
            else if (weekday == java.util.Calendar.SATURDAY)
                return 6;

            return weekday;
        }

        private boolean hasChanged(CronTimestamp lastTimestamp)
        {
            StringBuffer timeDetails = new StringBuffer();

            if (fDebug)
            {
                timeDetails.append("\nLast Time   : ").append(
                    lastTimestamp.toString());
                timeDetails.append("\nCurrent Time: ").append(
                    this.toString());
            }

            if (fMinute != lastTimestamp.getMinute())
            {
                fIsMinuteChanged = true;
                lastTimestamp.setMinute(fMinute);
            }

            if (fHour != lastTimestamp.getHour())
            {
                fIsHourChanged = true;
                lastTimestamp.setHour(fHour);
            }

            if (fDay != lastTimestamp.getDay())
            {
                fIsDayChanged = true;
                lastTimestamp.setDay(fDay);
            }

            if (fMonth != lastTimestamp.getMonth())
            {
                fIsMonthChanged = true;
                lastTimestamp.setMonth(fMonth);
            }

            if (fWeekday != lastTimestamp.getWeekday())
            {
                fIsWeekdayChanged = true;
                lastTimestamp.setWeekday(fWeekday);
            }
            
            if (fIsMinuteChanged || fIsHourChanged || fIsDayChanged ||
                fIsMonthChanged || fIsWeekdayChanged)
            {
                if (fDebug)
                {
                    timeDetails.append("\nChanges     : ");
                    if (fIsMinuteChanged)  timeDetails.append("minute ");
                    if (fIsHourChanged)    timeDetails.append("hour ");
                    if (fIsDayChanged)     timeDetails.append("day ");
                    if (fIsMonthChanged)   timeDetails.append("month ");
                    if (fIsWeekdayChanged) timeDetails.append("weekday");
                    fTimeDetails = timeDetails.toString();
                }

                return true;
            }
            else
            {
                return false;
            }
        }

        private boolean matchTime(Vector timeVector, int time)
        {
            if (timeVector.contains(String.valueOf(time)) ||
                timeVector.contains("ANY"))
                return true;
            else
                return false;
        }

        private boolean matchesRegisteredTriggers(CronData2 regData)
        {
            boolean minuteMatch = matchTime(regData.fMinute, fMinute);
            boolean minuteMatchOrEmpty = minuteMatch ||
                regData.fMinute.isEmpty();
            boolean minuteTrigger = fIsMinuteChanged && minuteMatch;
            
            boolean hourMatch = matchTime(regData.fHour, fHour);
            boolean hourMatchOrEmpty = hourMatch || regData.fHour.isEmpty();
            boolean hourTrigger = fIsHourChanged && hourMatch;

            boolean dayMatch = matchTime(regData.fDay, fDay);
            boolean dayMatchOrEmpty = dayMatch || regData.fDay.isEmpty();
            boolean dayTrigger = fIsDayChanged && dayMatch;

            boolean monthMatch = matchTime(regData.fMonth, fMonth);
            boolean monthMatchOrEmpty = monthMatch || regData.fMonth.isEmpty();
            boolean monthTrigger = fIsMonthChanged && monthMatch;

            boolean weekdayMatch = matchTime(regData.fWeekday, fWeekday);
            boolean weekdayMatchOrEmpty = weekdayMatch ||
                regData.fWeekday.isEmpty();
            boolean weekdayTrigger = fIsWeekdayChanged && weekdayMatch;

            // All 5 time units must match or be empty, and there must be at
            // least one trigger

            if ((minuteMatchOrEmpty && hourMatchOrEmpty && dayMatchOrEmpty &&
                 monthMatchOrEmpty && weekdayMatchOrEmpty) &&
                (minuteTrigger || hourTrigger || dayTrigger || monthTrigger ||
                 weekdayTrigger))
            {
                // Assign triggers string
               
                setTriggers(minuteTrigger,
                            hourTrigger, hourMatch,
                            dayTrigger, dayMatch,
                            monthTrigger, monthMatch,
                            weekdayTrigger, weekdayMatch);

                if (fDebug)
                {
                    setRegData(regData.fMinute, regData.fHour, regData.fDay,
                               regData.fMonth, regData.fWeekday);
                }

                return true;
            }
            else
            {
                return false;
            }
        }

        private void setTriggers(boolean minuteTrigger,
                                 boolean hourTrigger, boolean hourMatch,
                                 boolean dayTrigger, boolean dayMatch,
                                 boolean monthTrigger, boolean monthMatch,
                                 boolean weekdayTrigger, boolean weekdayMatch)
        {
            StringBuffer triggers = new StringBuffer();

            if (minuteTrigger)
            {
                triggers.append("minute[").append(fMinute).append("],");
            }

            if (hourTrigger || hourMatch)
            {
                triggers.append("hour[").append(fHour).append("],");
            }

            if (dayTrigger || dayMatch)
            {
                triggers.append("day[").append(fDay).append("],");
            }

            if (monthTrigger || monthMatch)
            {
                triggers.append("month[").append(fMonth).append("],");
            }

            if (weekdayTrigger || weekdayMatch)
            {
                triggers.append("weekday[").append(fWeekday).append("],");
            }

            fTriggers = triggers.toString();

            if (fTriggers.endsWith(","))
            {
                fTriggers = fTriggers.substring(0, fTriggers.length() - 1);
            }
        }

        private String getTriggers()
        {
            return fTriggers;
        }

        private void setRegData(Vector minute, Vector hour, Vector day,
                                Vector month, Vector weekday)
        {
            fRegData = "\nRegistration: minute" + minute.toString() +
                ",hour" + hour.toString() + ",day" + day.toString() +
                ",month" + month.toString() + ",weekday" + weekday.toString();
        }

        private String getRegData()
        {
            return fRegData;
        }

        private String getTimeDetails()
        {
            return fTimeDetails;
        }
    }


    /**
     * This helper class encapsulates the data for a STAF service request
     * that has been triggered
     */ 
    class RequestData
    {
        private String getMachine() { return fMachine; }
        private String getService() { return fService; }
        private String getRequest() { return fRequest; }
        private String getId() { return fId; }
        private String getMachineName() { return fMachineName; }

        private String fMachine;     // Endpoint for the service request
        private String fService;     // Service name
        private String fRequest;     // Service request
        private String fId;          // Registration id
        private String fMachineName; // Machine name for the endpoint

        RequestData(String machine, String service, String request,
                    String id, String machineName)
        {
            fMachine = machine;
            fService = service;
            fRequest = request;
            fId = id;
            fMachineName = machineName;
        }
        
        public String toString()
        {
            return "[ID=" + fId + "] [" + fMachineName + "] STAF " +
                fMachine + " " + fService + " " +
                STAFUtil.maskPrivateData(fRequest);
        }
    }
}
