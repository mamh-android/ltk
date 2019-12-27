/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.eventmanager;

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
import org.python.util.PythonInterpreter;
import org.python.core.PyException;

public class EventManagerService implements STAFServiceInterfaceLevel30
{
    private STAFHandle fHandle;
    private final String kVersion = "3.4.0";
    
    // Version of STAF (or later) required for this service
    // - STAF Version 3.1.0 or later is required so that the privacy methods
    //   in STAFUtil are available.
    private final String kRequiredSTAFVersion = "3.1.0";

    private STAFCommandParser fParmsParser = new STAFCommandParser();
    private STAFCommandParser fRegisterParser;
    private STAFCommandParser fListParser;
    private STAFCommandParser fTriggerParser;
    private STAFCommandParser fEnableParser;
    private STAFCommandParser fDisableParser;
    private STAFCommandParser fVersionParser;
    private STAFCommandParser fDebugParser;
    private STAFMapClassDefinition fEventManagerIDMapClass;
    private STAFMapClassDefinition fEMIDShortMapClass;
    private STAFMapClassDefinition fEMIDMapClass;
    private STAFMapClassDefinition fSettingsMapClass;
    private STAFMapClassDefinition fTriggerMapClass;
    private STAFMapClassDefinition fProcessEndMapClass;
    private STAFMapClassDefinition fProcessReturnFileMapClass;
    private Hashtable<Integer, Object> fRegTable;
    private static boolean DEBUG = true;
    private boolean fOldVarResolution = false;
    private STAFCommandParser fUnregisterParser;
    private int fEventManagerID = 1;
    private String fEventServiceMachine = "";
    private String fEventServiceName = "";
    private String fLocalMachineName = "";
    private String fServiceName = "eventmanager";
    private int fServiceHandleNumber = 0;
    private String fJythonVersion = "";
    private String fHashtableFileDirectory;
    private String fHashtableFileName;  // eventman.ser
    private String fBackupFileName;     // eventman_backup.ser
    private File fBackupFile;

    // Map of the submitted requests that haven't completed yet
    //   Key: <reqNum>, Value:  <RequestData object>
    private Map<String, RequestData> fRequestMap =
        Collections.synchronizedMap(new HashMap<String, RequestData>());

    // Map of the submitted PROCESS START requests (without the WAIT option)
    // that haven't completed yet
    //   Key: <machineName>:<handle>, Value: <reqNum>
    private Map<String, String> fProcessHandleMap =
        Collections.synchronizedMap(new HashMap<String, String>());

    private MonitorThread fMonitorThread = null;

    private static String fLineSep;
    private static String sHelpMsg;

    static final String sQueueTypeEnd = "STAF/Service/EventManager/End";

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    // Event Manager Service Error Codes

    public static final int PYTHONERROR = 4001;
    private static final String PYTHONERRORInfo = "Python error";
    private static final String PYTHONERRORDesc =
        "A Python error occurred while evaluating the request to be submitted.";

    public static final int REQUESTNOTSUBMITTED = 4002;
    private static final String REQUESTNOTSUBMITTEDInfo =
        "Request not submitted";
    private static final String REQUESTNOTSUBMITTEDDesc =
        "The request was not submitted because the value of " +
        "STAFEventManagerSubmit was not set to true.";

    public EventManagerService()
    {
    }

    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        int rc = STAFResult.Ok;

        try
        {
            fHandle = new STAFHandle("STAF/SERVICE/" + info.name);
            fServiceHandleNumber = fHandle.getHandle();
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  e.toString());
        }

        try
        {
            STAFResult res = new STAFResult();

            // Resolve the machine name variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLocalMachineName = res.result;
            
            fServiceName = info.name;
            fEventServiceMachine = "local";
            fEventServiceName = "event";
            
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

                // Get the version of Jython distributed with the service
                    
                fJythonVersion = STAFServiceSharedJython.getJythonVersion();

                // Initialize the PythonInterpreter's python.home to point to
                // the shared jython installation.

                Properties p = new Properties();
                p.setProperty("python.home",
                              STAFServiceSharedJython.getJythonDirName());
                PythonInterpreter.initialize(System.getProperties(), p, null);

                // IMPORTANT: First creation of a PythonInterpreter needs to
                // occur after it's been initialized in order for this
                // property to be assigned to it.
            }

            // Parse PARMS if provided in the service configuration line

            if (info.parms != null)
            {
                fParmsParser.addOption("EVENTSERVICEMACHINE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("EVENTSERVICENAME", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("OLDVARRESOLUTION", 1,
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

            fRegisterParser.addOption("TYPE", 1,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("SUBTYPE", 0,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("PREPARE", 0,
                                      STAFCommandParser.VALUEREQUIRED);

            fRegisterParser.addOption("ENABLED", 0,
                                      STAFCommandParser.VALUENOTALLOWED);

            fRegisterParser.addOption("DISABLED", 0,
                                      STAFCommandParser.VALUENOTALLOWED);

            fRegisterParser.addOptionGroup("ENABLED DISABLED", 0, 1);

            fRegisterParser.addOptionNeed("REGISTER", "MACHINE PYTHONMACHINE");
            fRegisterParser.addOptionNeed("MACHINE PYTHONMACHINE", "REGISTER");

            fUnregisterParser.addOption("UNREGISTER", 1,
                                        STAFCommandParser.VALUENOTALLOWED);

            fUnregisterParser.addOption("ID", 1,
                                        STAFCommandParser.VALUEREQUIRED);

            fUnregisterParser.addOptionNeed("UNREGISTER", "ID");

            fListParser.addOption("LIST", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption("MACHINE", 1,
                                  STAFCommandParser.VALUEREQUIRED);

            fListParser.addOption("TYPE", 1,
                                  STAFCommandParser.VALUEREQUIRED);

            fListParser.addOption("LONG", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption("SETTINGS", 1, 
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption("SHORT", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOptionGroup("MACHINE SETTINGS", 0, 1);

            fListParser.addOptionGroup("TYPE SETTINGS", 0, 1);

            fListParser.addOptionGroup("LONG SETTINGS", 0, 1);

            fListParser.addOptionGroup("LONG SHORT", 0, 1);

            fTriggerParser.addOption("TRIGGER", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fTriggerParser.addOption("ID", 1,
                                     STAFCommandParser.VALUEREQUIRED);

            fTriggerParser.addOption("SCRIPT", 0,
                                     STAFCommandParser.VALUEREQUIRED);

            fTriggerParser.addOptionNeed("TRIGGER", "ID");

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

            // Construct map class for a LIST request.

            fEMIDMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/EventManagerID");
            fEMIDMapClass.addKey("eventManagerID", "ID");
            fEMIDMapClass.addKey("description", "Description");
            fEMIDMapClass.setKeyProperty("description", "display-short-name",
                                         "Desc");
            fEMIDMapClass.addKey("machine", "Machine");
            fEMIDMapClass.addKey("service", "Service");
            fEMIDMapClass.addKey("request", "Request");
            fEMIDMapClass.addKey("type", "Event Type");
            fEMIDMapClass.addKey("subtype", "Event Subtype");

            // Construct map class for a LIST SHORT request.

            fEMIDShortMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/EventManagerIDShort");
            fEMIDShortMapClass.addKey("eventManagerID", "ID");
            fEMIDShortMapClass.addKey("description", "Description");
            fEMIDShortMapClass.setKeyProperty("description",
                                              "display-short-name",
                                              "Desc");
            fEMIDShortMapClass.addKey("machine", "Machine");
            fEMIDShortMapClass.addKey("service", "Service");
            fEMIDShortMapClass.addKey("request", "Request");

            // Construct map class for a LIST LONG request.

            fEventManagerIDMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/EventManagerID");
            fEventManagerIDMapClass.addKey("eventManagerID", "ID");
            fEventManagerIDMapClass.addKey("description", "Description");
            fEventManagerIDMapClass.addKey("machine", "Machine");
            fEventManagerIDMapClass.addKey("machineType", "Machine Type");
            fEventManagerIDMapClass.addKey("service", "Service");
            fEventManagerIDMapClass.addKey("serviceType", "Service Type");
            fEventManagerIDMapClass.addKey("request", "Request");
            fEventManagerIDMapClass.addKey("requestType", "Request Type");
            fEventManagerIDMapClass.addKey("type", "Event Type");
            fEventManagerIDMapClass.addKey("subtype", "Event Subtype");
            fEventManagerIDMapClass.addKey("prepareScript", "Prepare Script");
            fEventManagerIDMapClass.addKey("state", "State");

            // Construct map-class for list settings information

            fSettingsMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/Settings");
            fSettingsMapClass.addKey(
                "eventServiceMachine", "Event Service Machine");
            fSettingsMapClass.addKey(
                "eventServiceName", "Event Service Name");

            // Construct map class for a TRIGGER request

            fTriggerMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/Trigger");
            fTriggerMapClass.addKey("machine", "Machine");
            fTriggerMapClass.addKey("requestNumber", "Request Number");

            // Construct map class for process completion information for an
            // asynchronous PROCESS START request

            fProcessEndMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/ProcessEnd");

            fProcessEndMapClass.addKey("rc", "Return Code");
            fProcessEndMapClass.addKey("key", "Key");
            fProcessEndMapClass.addKey("fileList", "Files");

            // Construct map class for a returned file from a process

            fProcessReturnFileMapClass = new STAFMapClassDefinition(
                "STAF/Service/EventManager/ProcessReturnFile");

            fProcessReturnFileMapClass.addKey("rc", "Return Code");
            fProcessReturnFileMapClass.addKey("data", "Data");

            // Resolve the file separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/File}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            String fileSep = res.result;

            // Store data for the eventmanager service in directory:
            //   <STAF writeLocation>/service/<em service name (lower-case)>

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
                "eventman.ser";

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
                "eventman_backup.ser";

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
                "           TYPE <EventType> [SUBTYPE <EventSubType>]" +
                fLineSep +
                "           [PREPARE <Script>]" +
                fLineSep +
                "           [ENABLED | DISABLED]" +
                fLineSep +
                "UNREGISTER ID <RegistrationID>" +
                fLineSep +
                "LIST       <[MACHINE <Machine>] [TYPE <EventType>] [LONG | SHORT]> | SETTINGS" +
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
            // to appear in the service's handle's queue and handles the
            // message

            fMonitorThread = new MonitorThread(this);
            fMonitorThread.start();
        }
        catch (STAFException e)
        {
            rc = STAFResult.STAFRegistrationError;
            return new STAFResult(rc, e.toString());
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
        STAFCommandParseResult parseResult= fParmsParser.parse(info.parms);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("EVENTSERVICEMACHINE") > 0)
        {
            STAFResult res = STAFUtil.resolveInitVar(
                parseResult.optionValue("EVENTSERVICEMACHINE"), fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fEventServiceMachine = res.result;
        }

        if (parseResult.optionTimes("EVENTSERVICENAME") > 0)
        {
            STAFResult res = STAFUtil.resolveInitVar(
                parseResult.optionValue("EVENTSERVICENAME"), fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fEventServiceName = res.result;
        }

        if (parseResult.optionTimes("OLDVARRESOLUTION") > 0)
        {
            fOldVarResolution = true;
        }

        return new STAFResult(STAFResult.Ok);
    }

    public STAFResult acceptRequest(STAFServiceInterfaceLevel30.RequestInfo
                                    info)
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

        // Return the help text for the service

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
        List<Map> emIDList = new ArrayList<Map>();

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

            if (parsedRequest.optionTimes("SETTINGS") > 0)
            {
                // LIST SETTINGS

                mc.setMapClassDefinition(fSettingsMapClass);

                // Not calling STAFMapClass createInstance method to avoid
                // getting an unchecked cast warning
                //Map outputMap = fSettingsMapClass.createInstance();
                Map<String, Object> outputMap = new TreeMap<String, Object>();
                outputMap.put("staf-map-class-name", fSettingsMapClass.name());

                outputMap.put(
                    "eventServiceMachine", fEventServiceMachine);
                outputMap.put(
                    "eventServiceName", fEventServiceName);
                
                mc.setRootObject(outputMap);

                return new STAFResult(STAFResult.Ok, mc.marshall());
            }

            // Resolve the MACHINE option

            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("machine"),
                fHandle, info.requestNumber);
                
            if (resolvedValue.rc != 0) return resolvedValue;

            machineParm = resolvedValue.result;

            // Resolve the TYPE option

            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("type"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            typeParm = resolvedValue.result;

            // Check if the LONG or SHORT options is specified

            boolean longFormat = false;
            boolean shortFormat = false;

            if (parsedRequest.optionTimes("LONG") > 0)
            {
                longFormat = true;
                mc.setMapClassDefinition(fEventManagerIDMapClass);
            }
            else if (parsedRequest.optionTimes("SHORT") > 0)
            {
                shortFormat = true;
                mc.setMapClassDefinition(fEMIDShortMapClass);
            }
            else
            {
                mc.setMapClassDefinition(fEMIDMapClass);
            }
            
            // Process the LIST registrationss request

            EventManagerData2 theData;
            boolean match;

            synchronized (fRegTable)
            {
                if (fRegTable.size() == 0)
                {
                    mc.setRootObject(emIDList);
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
                    theData = (EventManagerData2) fRegTable.get(id);

                    if (machineParm.equals("") ||
                        machineParm.equalsIgnoreCase(theData.fMachine))
                    {
                        if (typeParm.equals("") ||
                            typeParm.equalsIgnoreCase(theData.fType))
                        {
                            match = true;
                        }
                    }

                    if (match)
                    {
                        // Create a map representing the matching registration
                        // and add it to the list of registrations

                        Map<String, Object> emIDMap =
                            new TreeMap<String, Object>();

                        if (longFormat)
                        {
                            // Not calling STAFMapClass createInstance method
                            // to avoid getting an unchecked cast warning
                            //emIDMap = fEventManagerIDMapClass.createInstance();
                            emIDMap.put("staf-map-class-name",
                                        fEventManagerIDMapClass.name());
                        }
                        else if (shortFormat)
                        {
                            // Not calling STAFMapClass createInstance method
                            // to avoid getting an unchecked cast warning
                            //emIDMap = fEMIDShortMapClass.createInstance();
                            emIDMap.put("staf-map-class-name",
                                        fEMIDShortMapClass.name());
                        }
                        else
                        {
                            // Not calling STAFMapClass createInstance method
                            // to avoid getting an unchecked cast warning
                            //emIDMap = fEMIDMapClass.createInstance();
                            emIDMap.put("staf-map-class-name",
                                        fEMIDMapClass.name());
                        }

                        emIDMap.put("eventManagerID", id.toString());

                        if (!theData.fDescription.equals(""))
                        {
                            emIDMap.put("description", theData.fDescription);
                        }

                        emIDMap.put("machine", theData.fMachine);
                        emIDMap.put("service", theData.fService);
                        emIDMap.put("request", STAFUtil.maskPrivateData(
                            theData.fRequest));
                            
                        if (!shortFormat)
                        {
                            emIDMap.put("type", theData.fType);
                        }
                        
                        if ((theData.fSubtype != null) &&
                            (!theData.fSubtype.equals(""))
                            & !shortFormat)
                        {
                            emIDMap.put("subtype", theData.fSubtype);
                        }
                        
                        if (longFormat)
                        {
                            if (theData.fPythonMachine)
                                emIDMap.put("machineType", "Python");
                            else
                                emIDMap.put("machineType", "Literal");

                            if (theData.fPythonService)
                                emIDMap.put("serviceType", "Python");
                            else
                                emIDMap.put("serviceType", "Literal");

                            if (theData.fPythonRequest)
                                emIDMap.put("requestType", "Python");
                            else
                                emIDMap.put("requestType", "Literal");

                            if ((theData.fPrepare != null) &&
                                (!theData.fPrepare.equals("")))
                            {
                                emIDMap.put("prepareScript",
                                            STAFUtil.maskPrivateData(
                                                theData.fPrepare));
                            }

                            emIDMap.put("state", theData.fState);
                        }

                        emIDList.add(emIDMap);
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

        mc.setRootObject(emIDList);

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

            EventManagerData2 theData = null;

            synchronized (fRegTable)
            {
                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, "ID " + id + " not found");
                }
                
                theData = (EventManagerData2)fRegTable.get(id);
            }
            
            // Create a marshalling context and set any map classes (if any)

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fTriggerMapClass);

            // Create an empty result map to contain the result

            // Not calling STAFMapClass createInstance method to avoid
            // getting an unchecked cast warning
            //Map resultMap = fTriggerMapClass.createInstance();
            Map<String, Object> resultMap = new TreeMap<String, Object>();
            resultMap.put("staf-map-class-name", fTriggerMapClass.name());

            String triggerMsg = "[ID=" + id + "] [" + info.endpoint +
                ", " + info.handleName + "] Triggering a STAF command.\n" +
                info.request;

            log("info", triggerMsg);

            String machine = theData.fMachine;
            String service = theData.fService;
            String request = theData.fRequest;
            String prepare = theData.fPrepare;

            boolean pythonMachine = theData.fPythonMachine;
            boolean pythonService = theData.fPythonService;
            boolean pythonRequest = theData.fPythonRequest;

            // Check if any Python data was provided in the registration

            if ((prepare != null && !prepare.equals("")) ||
                pythonMachine || pythonService || pythonRequest ||
                scripts.size() > 0)
            {
                String regInfoMsg = "  ID: " + id.toString() +
                    "\n  machine: " + machine + "\n  service: " + service +
                    "\n  request: " + request + "\n  prepare: " + prepare;

                // Create a new Python Interpreter for a triggered registration

                EventManagerPythonInterpreter myPyInt =
                    new EventManagerPythonInterpreter(this, id);
                
                // Execute any SCRIPT options

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

                // Evaluate any Python data provided in the registration

                if (prepare != null && !prepare.equals(""))
                {
                    try
                    {
                        myPyInt.pyExec(prepare);
                    }
                    catch (PyException e)
                    {
                        String pythonError = "Python error in the " +
                            "PREPARE value.\n\nPREPARE:\n" + prepare +
                            "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg;

                        String msg = "[ID=" + id + "] " + pythonError;

                        log("error", msg);

                        return new STAFResult(PYTHONERROR, pythonError);
                    }

                    Object submit = myPyInt.pyGetVar(
                        "STAFEventManagerSubmit");
 
                    if ((submit == null) ||
                        (!submit.toString().equals("true")))
                    {
                        // Do not submit the request for this registration
                        return new STAFResult(REQUESTNOTSUBMITTED,
                            "STAFEventManagerSubmit=" + submit.toString());
                    }
                }

                if (pythonMachine)
                {
                    try
                    {
                        machine = myPyInt.pyStringEval(theData.fMachine);
                    }
                    catch (PyException e)
                    {
                        String pythonError = "Python error in the " +
                            "PYTHONMACHINE value.\n\nPYTHONMACHINE: " +
                            theData.fMachine + "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg;

                        String msg = "[ID=" + id + "] " + pythonError;

                        log("error", msg);

                        return new STAFResult(PYTHONERROR, pythonError);
                    }
                }

                if (pythonService)
                {
                    try
                    {
                        service = myPyInt.pyStringEval(theData.fService);
                    }
                    catch (PyException e)
                    {
                        String pythonError = "Python error in the " +
                            "PYTHONSERVICE value.\n\nPYTHONSERVICE: " +
                            theData.fService + "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg;

                        String msg = "[ID=" + id + "] " + pythonError;

                        log("error", msg);

                        return new STAFResult(PYTHONERROR, pythonError);
                    }
                }

                if (pythonRequest)
                {
                    try
                    {
                        request = myPyInt.pyStringEval(theData.fRequest);
                    }
                    catch (PyException e)
                    {
                        String pythonError = "Python error in the " +
                            "PYTHONREQUEST value.\n\nPYTHONREQUEST: " +
                            theData.fRequest + "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg;

                        String msg = "[ID=" + id + "] " + pythonError;

                        log("error", msg);

                        return new STAFResult(PYTHONERROR, pythonError);
                    }
                }
            }

            // Check if a PROCESS START request is being submitted without
            // the WAIT option and without the NOTIFY ONEND option and,
            // if so, add "NOTIFY ONEND Handle <ServiceHandle>" to the
            // request and return the updated request.

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

                String message = "[ID=" + id.toString() + "] " +
                    "Error submitting a STAF command." +
                    "\nTriggered manually." +
                    "\nSTAF command: STAF " + machine + " " +
                    service + " " + request +
                    "\nSubmit RC: " + submitResult.rc +
                    ", Result: " + submitResult.result;

                log("error", message);

                return submitResult;
            }

            String requestNumber = submitResult.result;

            fRequestMap.put(
                requestNumber, new RequestData(
                    machine, service, request, id.toString(), machineName));

            String message = "[ID=" + id.toString() + "] " +
                "[" + machineName + ":" + requestNumber + "] " +
                "Submitted a STAF command.\nTriggered manually." +
                "\nSTAF command: STAF " + machine + " " +
                service + " " + request;

            log("info", message);

            resultMap.put("machine", machineName);
            resultMap.put("requestNumber", requestNumber);

            // Set the result map as the root object for the marshalling
            // context and return the marshalled result

            mc.setRootObject(resultMap);

            return new STAFResult(STAFResult.Ok, mc.marshall());
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

            EventManagerData2 data = null;

            synchronized (fRegTable)
            {
                // Check if registration table contains the ID

                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, "ID " + id + " not found");
                }

                data = (EventManagerData2)fRegTable.get(id);
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

            EventManagerData2 data = null;

            synchronized (fRegTable)
            {
                // Check if registration table contains the ID

                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, "ID " + id + " not found");
                }

                data = (EventManagerData2)fRegTable.get(id);
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
        String type = "";
        String subtype = "";
        String prepare = "";
        String state = "Enabled";

        STAFResult resolvedResult = null;
        STAFResult resolvedValue = null;
        STAFCommandParseResult parsedRequest = fRegisterParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parsedRequest.errorBuffer);
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

            // Resolve the TYPE option

            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("type"), fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            type = resolvedValue.result;

            if (parsedRequest.optionTimes("subtype") > 0)
            {
                // Resolve the SUBTYPE option

                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("subtype"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;
                
                subtype = resolvedValue.result;
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

            String registerRequest = "REGISTER TYPE " +
                STAFUtil.wrapData(type);

            if (!(subtype.equals("")))
            {
                registerRequest += " SUBTYPE " + STAFUtil.wrapData(subtype);
            }

            if (!(prepare.equals("")) || pythonMachine || pythonService ||
                pythonRequest)
            {
                // validate Python code

                if (!(prepare.equals("")))
                {
                    try
                    {
                        EventManagerPythonInterpreter.compileForPython(
                            prepare);
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
                        EventManagerPythonInterpreter.compileForPython(
                            machine);
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
                        EventManagerPythonInterpreter.compileForPython(
                            service);
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
                        EventManagerPythonInterpreter.compileForPython(
                            request);
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

            // Register with the Event service

            STAFResult registerResult = fHandle.submit2(
                fEventServiceMachine, fEventServiceName, registerRequest);

            if (registerResult.rc != 0)
            {
                return registerResult;
            }

            // Add the new registration data to the registration table

            Integer index = null;

            synchronized (fRegTable)
            {
                index = new Integer(fEventManagerID++);

                fRegTable.put(
                    index, (Object) new EventManagerData2(
                        description, originMachine, machine, pythonMachine,
                        service, pythonService, request, pythonRequest,
                        type, subtype, prepare, state));

                sResult = new STAFResult(STAFResult.Ok, index.toString());
                
                // Save registration data to a file

                STAFResult result = saveHashtable();

                if (result.rc != STAFResult.Ok) return result;
            }

            // Log message that the registration was successful

            String message = "[ID=" + index + "] [" + info.endpoint +
                ", " + info.handleName +
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
            return new STAFResult(
                STAFResult.InvalidRequestString, parsedRequest.errorBuffer);
        }

        // For each option, if the user has specified a value, set the
        // corresponding local variable to the user-specified value.

        try
        {
            // Verify the requester has at least trust level 4

            STAFResult trustResult = STAFUtil.validateTrust(
                4, fServiceName, "UNREGISTER", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Resolve the ID option

            resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                "id", parsedRequest.optionValue("id"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;
            
            id = new Integer(resolvedValue.result);

            // Remove the specified id from the registration table, or if the
            // id does not exist, return an error

            EventManagerData2 data = null;

            synchronized (fRegTable)
            {
                if (!fRegTable.containsKey(id))
                {
                    return new STAFResult(STAFResult.DoesNotExist,
                                          "ID " + id + " not found");
                }
                
                // Registration id exists.  Remove it.

                data = (EventManagerData2)fRegTable.get(id);

                fRegTable.remove(id);

                // If no registrations, reset the Event Manager id to 1

                if (fRegTable.isEmpty())
                {
                    fEventManagerID = 1;
                }
                
                // Save registration data to a file

                sResult = saveHashtable();
            }

            if (sResult.rc != STAFResult.Ok) return sResult;
                
            // Log message that the un-registration was successful

            String message = "[ID=" + id + "] [" + info.endpoint + ", " +
                info.handleName + "] Unregistered a STAF command.";

            log("info", message);

            String unregType = data.fType;
            String unregSubtype = data.fSubtype;
            boolean unregWithEventService = true;

            // Only unregister with the Event Service if there are no
            // other fRegTable entries with this type and subtype

            for (Enumeration e = fRegTable.elements(); e.hasMoreElements();)
            {
                EventManagerData2 theData =
                    (EventManagerData2)e.nextElement();

                if (unregType.equalsIgnoreCase(theData.fType) &&
                    theData.fSubtype.equalsIgnoreCase(unregSubtype))
                {
                    unregWithEventService = false;
                }
            }

            if (unregWithEventService)
            {
                String unRegisterRequest = "UNREGISTER TYPE " +
                    STAFUtil.wrapData(unregType);

                if (!(data.fSubtype.equals("")))
                {
                    unRegisterRequest += " SUBTYPE " +
                        STAFUtil.wrapData(unregSubtype);
                }

                // Unregister with the Event service

                STAFResult unRegisterResult = fHandle.submit2(
                    fEventServiceMachine, fEventServiceName,
                    unRegisterRequest);

                if (unRegisterResult.rc != 0)
                {
                    return unRegisterResult;
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

    protected void log(String level, String message)
    {
        fHandle.submit2(
            "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
            " LEVEL " + level + " MESSAGE " + STAFUtil.wrapData(message));
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
            fEventManagerID = 1;
            return;
        }
        else
        {
            ObjectInputStream ois = new ObjectInputStream(
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

            fEventManagerID = findNextEventManagerID();

            // If initializing the service, may need to convert the data to
            // the latest format

            if (firstLoad)
                convertOldEventManagerData();
        }
    }

    // Note: Always call convertOldEventManagerData when synchronized on
    // fHashtable

    private void convertOldEventManagerData()
    {
        // In EventManager V3.1.3, a new field, fDescription, was added to the
        // EventManager registation data.  A new class was created named
        // EventManagerData1 and will be used in V3.1.3+.  When registering the
        // EventManager V3.1.3+ service for the first time, if there are any
        // instances of class EventManagerData in the registration Hashtable,
        // they must be converted from class EventManagerData to
        // EventManagerData1.
        // In EventManager V3.3.0, a new field, fState, was added to the
        // EventManager registation data.  A new class was created named
        // EventManagerData2 and will be used in V3.3.0+.  When registering the
        // EventManager V3.3.0+ service for the first time, if there are any
        // instances of class EventManagerData1 in the registration Hashtable,
        // they must be converted from class EventManagerData1 to
        // EventManagerData2.

        Hashtable<Integer, Object> newRegTable =
            new Hashtable<Integer, Object>();

        for (Enumeration<Integer> e = fRegTable.keys(); e.hasMoreElements();)
        {
            Integer id = e.nextElement();

            Object eventManagerDataObj = fRegTable.get(id);

            try
            {
                EventManagerData2 data =
                    (EventManagerData2) eventManagerDataObj;

                newRegTable.put(id, (Object) data);
            }
            catch (ClassCastException ex)
            {
                try
                {
                    EventManagerData1 data =
                        (EventManagerData1) eventManagerDataObj;

                    EventManagerData2 newData = new 
                        EventManagerData2(data.fDescription,
                                          data.fOriginMachine,
                                          data.fMachine,
                                          data.fPythonMachine,
                                          data.fService,
                                          data.fPythonService,
                                          data.fRequest,
                                          data.fPythonRequest,
                                          data.fType,
                                          data.fSubtype,
                                          data.fPrepare,
                                          "Enabled");
                    newRegTable.put(id, (Object) newData);
                }
                catch (ClassCastException ex2)
                {
                    EventManagerData data =
                        (EventManagerData) eventManagerDataObj;

                    EventManagerData2 newData = new 
                        EventManagerData2("",
                                          data.fOriginMachine,
                                          data.fMachine,
                                          data.fPythonMachine,
                                          data.fService,
                                          data.fPythonService,
                                          data.fRequest,
                                          data.fPythonRequest,
                                          data.fType,
                                          data.fSubtype,
                                          data.fPrepare,
                                          "Enabled");

                    newRegTable.put(id, (Object) newData);
                }
            }
        }

        fRegTable = newRegTable;

        // Register with the Event service

        for (Enumeration e = fRegTable.elements(); e.hasMoreElements();)
        {
            EventManagerData2 data = (EventManagerData2) e.nextElement();

            String request = "REGISTER TYPE " + STAFUtil.wrapData(data.fType);

            if ((data.fSubtype != null) && !(data.fSubtype.equals("")))
            {
                request += " SUBTYPE " + STAFUtil.wrapData(data.fSubtype);
            }

            STAFResult registerResult = fHandle.submit2(
                fEventServiceMachine, fEventServiceName, request);

            if (registerResult.rc != 0)
            {
                String message = "STAF " + fEventServiceMachine +
                    " " + fEventServiceName + " " + request +
                    " failed.  RC=" + registerResult.rc +
                    ", Result=" + registerResult.result;

                log("error", message);
            }
        }
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
     * Restore the eventmanager.ser file from the backup
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
            log("Error", "Restoring the eventman.ser file from the backup " +
                "failed.  RC=" + res.rc + ", Result=" + res.result +
                ", Request=" + copyRequest);

            return res;
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Save the EventManager service's registration data currently in memory
     * to a file.  If an error occurs writing to the file, restore the
     * registration data to it's previous state to keep the data in memory in
     * sync with the data in the file.
     * 
     * Note: Always call saveHashtable when synchronized on fHashtable
     */ 
    private STAFResult saveHashtable()
    {
        // Backup the EventManager.service's registration file

        boolean haveBackup = renameFile(
            new File(fHashtableFileName), new File(fBackupFileName));

        // Write the EventManager registration data to a file

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

    // Note: Always call findNextEventManagerID when synchronized on
    // fHashtable

    private int findNextEventManagerID()
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
        fHandle.submit2(
            STAFHandle.ReqFireAndForget,
            "local", "QUEUE", "QUEUE TYPE " + sQueueTypeEnd +
            " MESSAGE " + STAFUtil.wrapData(""));

        unregisterHelp();

        return new STAFResult(STAFResult.Ok);
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

        RequestData requestData = fRequestMap.get(reqNum);

        if (requestData != null)
        {
            id = requestData.getId();
            machineName = requestData.getMachineName();
        }
        
        String resultSep = "";
        String resultString = result.toString();

        if (resultString.startsWith("[") || resultString.startsWith("{"))
            resultSep = fLineSep;

        String message = "[ID=" + id + "] [" + machineName + ":" + reqNum +
            "] Completed a STAF command. RC=" + rc +
            ", Result=" + resultSep + resultString;

        log(level, message);

        if (requestData == null)
        {
            return;
        }

        // Check if a START request was successfully submitted to the PROCESS
        // service without the WAIT option

        String service = requestData.getService().toUpperCase();
        String request = requestData.getRequest().toUpperCase();
        String machine = requestData.getMachine();

        if (rc.equals("0") && service.equals("PROCESS") &&
            (request.indexOf("START") == 0)  &&
            (request.indexOf(" WAIT") == -1))
        {
            // Submitted an asynchronous PROCESS START request

            String handle = result.toString();

            fProcessHandleMap.put(
                machineName + ":" + handle, reqNum);

            // Check if the the PROCESS START request contains options to
            // notify the service's handle when the process ends

            if (request.indexOf(" NOTIFY ONEND HANDLE " +
                                fServiceHandleNumber) == -1)
            {
                // Not already being notified, so need to register to be
                // notified when the process ends
                
                STAFResult res = fHandle.submit2(
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

                    res = fHandle.submit2(
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
                            " completed before the EventManager service " +
                            "could ask to be notified when the process " +
                            "completed so only the process RC " +
                            "can be logged.\nTo fix this, change the " +
                            "registration entry by adding the WAIT " +
                            "option to the PROCESS START request.";

                        log(level, message);
                    }
                    else
                    {
                        // Could not get process return code.

                        // Log a error message about the process completing
                        // before the EventManager service could ask to be
                        // notified

                        message = "[ID=" + id + "] [" +
                            machineName + ":" + reqNum +
                            "] Process with handle " + handle +
                            " completed before the EventManager service " +
                            "could ask to be notified when the process " +
                            "completed so no process completion info " +
                            "can be logged.\nTo fix this, change the " +
                            "registration entry by adding the WAIT " +
                            "option to the PROCESS START request.";

                        log("error", message);
                    }

                    res = fHandle.submit2(
                        machine, "PROCESS", "FREE HANDLE " + handle);

                    // Remove the corresponding entries from the process
                    // handle map and the submitted request map

                    fProcessHandleMap.remove(machineName + ":" + handle);
                    fRequestMap.remove(reqNum);
                }
                else
                {
                    message = "[ID=" + id + "] [" + machineName + ":" +
                        reqNum + "] Process completion notification to " +
                        "machine " + machine + " failed for handle " +
                        handle + " RC=" + res.rc +
                        " Result=" + res.result;
                        
                    log("error", message);
                }
            }

            return;
        }
        
        // Remove this for requests other than ansynchronous PROCESS START
        // requests

        fRequestMap.remove(reqNum);
    }

    private void handleProcessEndMsg(Map queueMessageMap)
    {
        Map messageMap = (Map)queueMessageMap.get("message");
        String handle = (String)messageMap.get("handle");
        String rc = (String)messageMap.get("rc");

        String endpoint = (String)queueMessageMap.get("machine");

        // Free the process handle

        STAFResult freeHandleResult = fHandle.submit2(
            endpoint, "PROCESS", "FREE HANDLE " + handle);

        // Log a Pass or Fail message in the service log
        
        String level = "";

        if (rc.equals("0"))
            level = "Pass";
        else
            level = "Fail";

        // Resolve the STAF/Config/Machine variable on the endpoint to get
        // the machine name which is used in the key for the fProcessHandleMap
        // and in logged messages

        String machineName = endpoint;

        STAFResult varResult = fHandle.submit2(
            endpoint, "VAR", "RESOLVE STRING {STAF/Config/Machine}");

        if (varResult.rc == STAFResult.Ok)
            machineName = varResult.result;

        String id = null;
        String reqNum = fProcessHandleMap.get(machineName + ":" + handle);

        if (reqNum != null)
        {
            RequestData requestData = fRequestMap.get(reqNum);

            if (requestData != null)
                id = requestData.getId();

            fRequestMap.remove(reqNum);
            fProcessHandleMap.remove(machineName + ":" + handle);
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
            //Map returnFileMap = fProcessReturnFileMapClass.createInstance();
            Map<String, Object> returnFileMap = new TreeMap<String, Object>();
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

        log(level, message);
    }

    private void handleEventServiceMsg(Map queueMessageMap,
                                       String marshalledQueuedMsg)
    {
        // A STAF/Service/Event message is a map containing keys: 
        // eventServiceName, eventID, machine, handleName, handle, timestamp,
        // type, subtype, and propertyMap (whose values could contain
        // marshalled data).

        // Assign the data in the message to Java variables

        Map messageMap = (Map)queueMessageMap.get("message");
        String eventService = (String)messageMap.get("eventServiceName");
        String eventID = (String)messageMap.get("eventID");
        String generatingMachine = (String)messageMap.get("machine");
        String generatingProcess = (String)messageMap.get("handleName");
        String generatingHandle = (String)messageMap.get("handle");
        String eventTimestamp = (String)messageMap.get("timestamp");
        String eventType = (String)messageMap.get("type");
        String eventSubtype = (String)messageMap.get("subtype");
        Map eventProperties = (Map)messageMap.get("propertyMap");

        String eventInfoMsg = "Event service message information:" +
            "\n  eventservice: " + eventService +
            "\n  eventid: " + eventID +
            "\n  type: " + eventType +
            "\n  subtype: " + eventSubtype +
            "\n  generatingmachine: " + generatingMachine +
            "\n  generatingprocess: " + generatingProcess +
            "\n  generatinghandle: " + generatingHandle +
            "\n  eventtimestamp: " + eventTimestamp +
            "\n  properties: " + eventProperties;
       
        // Process all EventManager registrations whose type/subtype
        // match that in the event message

        EventManagerData2 theData;

        for (Enumeration<Integer> enumData = fRegTable.keys();
              enumData.hasMoreElements();)
        {
            Integer id = enumData.nextElement();
            
            theData = (EventManagerData2)fRegTable.get(id);
            
            if (theData == null)
            {
                continue;
            }
            
            if (!eventType.equalsIgnoreCase(theData.fType))
            {
                // Event type does not match EventManager registration
                continue;
            }
            else if (!theData.fSubtype.equals("") &&
                     !eventSubtype.equalsIgnoreCase(theData.fSubtype))
            {
                // Event subtype does not match EventManager registration
                continue;
            }

            // Matching EventManager registration found

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
                pythonMachine || pythonService || pythonRequest)
            {
                String regInfoMsg = "  ID: " + id.toString() +
                    "\n  machine: " + machine + "\n  service: " + service +
                    "\n  request: " + request + "\n  prepare: " + prepare;

                // Unmarshall the message, but ignore indirect objects so
                // that the actual message will still be a String
                // containing marshalled data.

                STAFMarshallingContext mc = STAFMarshallingContext.unmarshall(
                    marshalledQueuedMsg,
                    STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS);

                Map queueMap = (Map)mc.getRootObject();
                String queuedMessage = (String)queueMap.get("message");
                
                // Create a new Python Interpreter for a triggered registration

                EventManagerPythonInterpreter myPyInt =
                    new EventManagerPythonInterpreter(this, id);

                // Unmarshall the message using the Jython unmarshalling
                // method, and assign the Python variables from the data in
                // the message sent by the Event service.

                STAFResult res = myPyInt.unmarshallMessage(queuedMessage);
                    
                if (res.rc != STAFResult.Ok)
                {
                    String errMsg =
                        "[ID=" + id + "] Problem during Python setup.\n\n" +
                        res.result + "\n\nProcessing registration:\n" +
                        regInfoMsg + "\n" + eventInfoMsg;
                        
                    log("error", errMsg);
                    
                    continue;
                }
                    
                // Evaluate any Python data provided in the registration

                if (prepare != null && !prepare.equals(""))
                {
                    try
                    {
                        myPyInt.pyExec(prepare);
                    }
                    catch (PyException e)
                    {
                        String msg = "[ID=" + id + "] Python error in the " +
                            "PREPARE value.\n\nPREPARE:\n" + prepare +
                            "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg +
                            "\n\n" + eventInfoMsg;

                        log("error", msg);

                        continue;
                    }

                    Object submit = myPyInt.pyGetVar(
                        "STAFEventManagerSubmit");

                    if ((submit == null) ||
                        (!submit.toString().equals("true")))
                    {
                        // Do not submit the request for this registration
                        continue;
                    }
                }

                if (pythonMachine)
                {
                    try
                    {
                        machine = myPyInt.pyStringEval(machine);
                    }
                    catch (PyException e)
                    {
                        String msg = "[ID=" + id + "] Python error in the " +
                            "PYTHONMACHINE value." +
                            "\n\nPYTHONMACHINE: " + machine +
                            "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg +
                            "\n\n" + eventInfoMsg;

                        log("error", msg);

                        continue;
                    }
                }

                if (pythonService)
                {
                    try
                    {
                        service = myPyInt.pyStringEval(service);
                    }
                    catch (PyException e)
                    {
                        String msg = "[ID=" + id + "] Python error in the " +
                            "PYTHONSERVICE value." +
                            "\n\nPYTHONSERVICE: " + service +
                            "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg +
                            "\n\n" + eventInfoMsg;

                        log("error", msg);

                        continue;
                    }
                }

                if (pythonRequest)
                {
                    try
                    {
                        request = myPyInt.pyStringEval(request);
                    }
                    catch (PyException e)
                    {
                        String msg = "[ID=" + id + "] Python error in the " +
                            "PYTHONREQUEST value." +
                            "\n\nPYTHONREQUEST: " + request +
                            "\n\nPyException:\n" + e +
                            "\nProcessing registration:\n" + regInfoMsg +
                            "\n\n" + eventInfoMsg;

                        log("error", msg);

                        continue;
                    }
                }
            }

            if (!state.equals("Enabled"))
            {
                String message = "[ID=" + id.toString() + "] " +
                    "ID is disabled. STAF command not submitted.\n" +
                    "Event information: type=" + eventType +
                    " subtype=" + eventSubtype +
                    " prepare=" + theData.fPrepare +
                    " eventservice=" + eventService +
                    " eventid=" + eventID +
                    " generatingmachine=" + generatingMachine +
                    " generatingprocess=" + generatingProcess +
                    " generatinghandle=" + generatingHandle +
                    " eventtimestamp=" + eventTimestamp +
                    " properties=" + eventProperties +
                    "\nSTAF command: STAF " + machine + " " + service +
                    " " + request;
                
                log("info", message);

                continue;
            }

            // Check if a PROCESS START request is being submitted without
            // the WAIT option and without the NOTIFY ONEND option and, if
            // so, add "NOTIFY ONEND Handle <ServiceHandle>" to the
            // request and return the updated request.
                
            if (service.equalsIgnoreCase("PROCESS"))
            {
                request = updateProcessStartRequest(request);
            }

            // Get the machine name for the endpoint

            String machineName = machine;
            
            STAFResult varResult = fHandle.submit2(
                machine, "VAR", "RESOLVE STRING {STAF/Config/Machine}");

            if (varResult.rc == STAFResult.Ok)
                machineName = varResult.result;

            // Submit the STAF service request

            STAFResult submitResult = fHandle.submit2(
                STAFHandle.ReqQueue, machine, service, request);
            
            if (submitResult.rc != STAFResult.Ok)
            {
                // Error submitting the STAF service request

                String message = "[ID=" + id.toString() + "] " +
                    "Error submitting a STAF command." +
                    "\nEvent information:" +
                    " type=" + eventType +
                    " subtype=" + eventSubtype +
                    " prepare=" + theData.fPrepare +
                    " eventservice=" + eventService +
                    " eventid=" + eventID +
                    " generatingmachine=" + generatingMachine +
                    " generatingprocess=" + generatingProcess +
                    " generatinghandle=" + generatingHandle +
                    " eventtimestamp=" + eventTimestamp +
                    " properties=" + eventProperties +
                    "\nSTAF command: STAF " + machine + " " + service +
                    " " + request + "\nSubmit RC: " + submitResult.rc +
                    ", Result: " + submitResult.result;

                log("error", message);

                continue;
            }

            String requestNumber = submitResult.result;
                    
            fRequestMap.put(
                requestNumber, new RequestData(
                    machine, service, request, id.toString(), machineName));
            
            String message = "[ID=" + id.toString() + "] " +
                "[" + machineName + ":" + requestNumber + "] " +
                "Submitted a STAF command." +
                "\nEvent information:" +
                " type=" + eventType +
                " subtype=" + eventSubtype +
                " prepare=" + theData.fPrepare +
                " eventservice=" + eventService +
                " eventid=" + eventID +
                " generatingmachine=" + generatingMachine +
                " generatingprocess=" + generatingProcess +
                " generatinghandle=" + generatingHandle +
                " eventtimestamp=" + eventTimestamp +
                " properties=" + eventProperties +
                "\nSTAF command: STAF " + machine + " " + service +
                " " + request;

            log("info", message);
        }
    }

    /**
     * This helper class thread does the following in a loop until it gets
     * a message that the EventManager service is terminating:
     * - Waits until a message is on the EM service handle's queue and
     *   processes messages with types:
     *   - STAF/RequestComplete
     *   - STAF/PROCESS/END
     *   - STAF/Service/Event
     *   - STAF/Service/EventManager/End
     */ 
    class MonitorThread extends Thread
    {
        private EventManagerService fService;

        MonitorThread(EventManagerService service)
        {
            fService = service;
        }

        public void run()
        {
            STAFResult getResult = new STAFResult();
            Map queueMessageMap = null;
            boolean continueRunning = true;
            int numErrors = 0;
            
            // Maximum consecutive errors submitting a local QUEUE GET WAIT
            // request before we decide to exit the infinite loop
            int maxErrors = 5;

            while (continueRunning)
            {
                // Need a try/catch block in case an error occurs getting
                // a message off the queue so we can continue processing
                // additional messages
                
                try
                {
                    getResult = fService.fHandle.submit2(
                        "local", "QUEUE", "GET WAIT");

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

                        String msg = "STAF local QUEUE GET WAIT failed " +
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

                    String msg = "Exception getting a message off the queue." +
                        " Exception:\n" + t.toString() + "\nResult from " +
                        "STAF local QUEUE GET WAIT request:\n" +
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

                // Need a try/catch block so can catch any errors that
                // occur when processing the queued message

                try
                {
                    queueMessageMap = (Map)getResult.resultObj;

                    String queueType = (String)queueMessageMap.get("type");
                            
                    if (queueType == null)
                    {
                        continue;
                    }
                    else if (queueType.equalsIgnoreCase(
                        "STAF/RequestComplete"))
                    {
                        fService.handleRequestCompleteMsg(queueMessageMap);
                    }
                    else if (queueType.equalsIgnoreCase("STAF/PROCESS/END"))
                    {
                        fService.handleProcessEndMsg(queueMessageMap);
                    }
                    else if (queueType.equalsIgnoreCase("STAF/Service/Event"))
                    {
                        if (!(queueMessageMap.get("message") instanceof Map))
                        {
                            System.out.println(
                                "EventManagerService: " +
                                "Unsupported message format.  " +
                                "Ignoring this message.\n" +
                                "priority=" + queueMessageMap.get("priority") +
                                ", timestamp=" +
                                queueMessageMap.get("timestamp") +
                                ", machine=" + queueMessageMap.get("machine") +
                                ", handleName=" +
                                queueMessageMap.get("handleName") +
                                "\nqueueType=" + queueType + "\nmessage=" +
                                queueMessageMap.get("message").toString());

                            // Ignore this message
                            continue;
                        }

                        // Provide both the unmarshalled message and the
                        // marshalled message string because it needs to
                        // unmarshall it using Python

                        fService.handleEventServiceMsg(queueMessageMap,
                                                       getResult.result);
                    }
                    else if (queueType.equalsIgnoreCase(sQueueTypeEnd))
                    {
                        // This type of queued message indicates that
                        // the service is terminating

                        continueRunning = false;  // Exit MonitorThread     
                        break;
                    }
                }
                catch (Exception e)
                {
                    String msg = "Exception handling queued message. " +
                        "Exception:\n" + e.toString() +
                        "\nQueued Message:\n" + queueMessageMap;
                
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
