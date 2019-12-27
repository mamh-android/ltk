/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import com.ibm.staf.service.*;

import org.w3c.dom.Document;
import org.xml.sax.SAXNotRecognizedException;
import org.xml.sax.SAXNotSupportedException;
import org.xml.sax.InputSource;

import org.python.core.*;

import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.Map;
import java.util.HashMap;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.LinkedHashMap;
import java.util.Set;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.PrintWriter;
import org.python.util.PythonInterpreter;
import java.util.Properties;
import java.util.jar.*;
import java.io.IOException;
import java.io.File;
import java.io.BufferedReader;
import java.util.StringTokenizer;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

public class STAX implements STAFServiceInterfaceLevel30, STAXJobCompleteListener
{
    static final String fVersion = "3.5.0 Beta 1";

    // Version of STAF (or later) required for this service
    private final String kRequiredSTAFVersion = "3.3.4";

    // Version of Event service (or later) required for this service
    private final String kRequiredEventServiceVersion = "3.1.2";

    // This is the maximum queue size that the STAX service will automatically
    // increase the maxQueueSize to if less than this number
    private static int MAXQUEUESIZE = 1000;

    // This is the maximum record size that the STAX service will automatically
    // increase the LOG service's maxRecordSize to if less than this number
    private static int MAXLOGRECORDSIZE = 1048576;  // 1M

    static boolean CACHE_PYTHON_CODE = true;

    static final String INLINE_DATA = "<inline data>";
    static final String sNOT_PROVIDED = "<Not Provided>";
    static final String sNONE = "<None>";
    static final String sNA = "<N/A>";

    static final String sQueueTypeJobWaitComplete = new String(
        "STAF/Service/STAX/JobWaitComplete/");
    static final String sQueueTypeJobEnd = new String(
        "STAF/Service/STAX/Job/End");
    static final String sJobResultMapClassName = new String(
        "STAF/Service/STAX/JobResult");
    static final String sJobDetailedResultMapClassName = new String(
    "STAF/Service/STAX/JobDetailedResult");
    static final String sJobDetailsMapClassName = new String(
        "STAF/Service/STAX/JobDetails");
    static final String sFunctionInfoMapClassName = new String(
        "STAF/Service/STAX/FunctionInfo");
    static final String sArgInfoMapClassName = new String(
        "STAF/Service/STAX/ArgInfo");
    static final String sArgPropertyInfoMapClassName = new String(
        "STAF/Service/STAX/ArgPropertyInfo");
    static final String sArgPropertyDataInfoMapClassName = new String(
        "STAF/Service/STAX/ArgPropertyDataInfo");
    static final String sFileCacheMapClassName = new String(
        "STAF/Service/STAX/FileCache");
    static final String sFileCacheSummaryMapClassName = new String(
        "STAF/Service/STAX/FileCacheSummary");
    static final String sMachineCacheMapClassName = new String(
    "STAF/Service/STAX/MachineCache");
    static final String sPurgeStatsMapClassName = new String(
        "STAF/Service/STAX/PurgeStats");
    static final String sJobInfoMapClassName = new String(
        "STAF/Service/STAX/JobInfo");
    static final String sThreadInfoMapClassName = new String(
        "STAF/Service/STAX/ThreadInfo");
    static final String sThreadLongInfoMapClassName = new String(
        "STAF/Service/STAX/ThreadLongInfo");
    static final String sThreadVariableMapClassName = new String(
        "STAF/Service/STAX/ThreadVariable");
    static final String sSettingsMapClassName = new String(
        "STAF/Service/STAX/Settings");
    static final String sExtensionElementMapClassName = new String(
        "STAF/Service/STAX/ExtensionElement");
    static final String sExtensionJarFileMapClassName = new String(
        "STAF/Service/STAX/ExtensionJarFile");
    static final String sExtensionInfoMapClassName = new String(
        "STAF/Service/STAX/ExtensionInfo");
    static final String sServiceExtensionMapClassName = new String(
        "STAF/Service/STAX/ServiceExtension");
    static final String sMonitorExtensionMapClassName = new String(
        "STAF/Service/STAX/MonitorExtension");
    static final String sQueryJobMapClassName = new String(
        "STAF/Service/STAX/QueryJob");
    static final String sQueryThreadMapClassName = new String(
        "STAF/Service/STAX/QueryThread");
    static final String sNotifieeMapClassName = new String(
        "STAF/Service/STAX/JobNotifiee");
    static final String sAllNotifieeMapClassName = new String(
        "STAF/Service/STAX/Notifiee");
    static final String sGetResultMapClassName = new String(
        "STAF/Service/STAX/GetResult");
    static final String sGetDetailedResultMapClassName = new String(
        "STAF/Service/STAX/GetDetailedResult");
    static final String sExecuteErrorResultMapClassName = new String(
        "STAF/Service/STAX/ExecuteErrorResult");

    static final String PACKAGE_NAME = "com.ibm.staf.service.stax.STAX";
    
    private static String sHelpMsg;
    public static String lineSep;
    public static String fileSep;

    // STAF Service Interface Levels Supported for Generic Requests
    static final String INTERFACE_LEVEL_30 =
        "com.ibm.staf.service.STAFServiceInterfaceLevel30";


    // STAX Service Error Code
    static final int ErrorSubmittingExecuteRequest = 4001;
    static final int BlockNotHeld = 4002;
    static final int BlockAlreadyHeld = 4003;
    static final int JobNotComplete = 4004;
    static final int BreakpointBlockHeld = 4005;

    static final String STAX_EXTENSION = new String("staf/stax/extension/");
    static final String STAX_MONITOR_EXTENSION =
        new String("staf/staxmonitor/extension/");
    static final String STAX_EXTENSION_INFO =
        new String("staf/staxinfo/extension");

    // Simple STAX XML document to verify that parsing works
    static final String XML_DATA = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" +
        "<!DOCTYPE stax SYSTEM \"stax.dtd\">" +
        "<stax>" +
        "<defaultcall function=\"test\"/>" +
        "<function name=\"test\"><nop/></function>" +
        "</stax>";

    static SimpleDateFormat DATE_FORMAT = new SimpleDateFormat(
        "yyyyMMdd-HH:mm:ss");

    public STAX() { /* Do Nothing */ }

    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        int rc = STAFResult.Ok;
        STAFResult res = new STAFResult();

        try
        {
            fServiceName = info.name;

            // Get the services STAF handle

            fHandle = new STAFHandle("STAF/Service/" + info.name);

            // Resolve the line separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            lineSep = res.result;

            // Resolve the machine name variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLocalMachineName = res.result;

            // Resolve the machine nickname variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/MachineNickname}",
                                          fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLocalMachineNickname = res.result;
            
            // Add the local machine name to the cache alias list
            STAXFileCache.get().addLocalMachineName(fLocalMachineName);

            // Resolve the file separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/File}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fileSep = res.result;

            // Get the STAF Instance UUID

            res = fHandle.submit2("local", "MISC", "WhoAreYou");

            if (res.rc != 0)
            {
                System.out.println("Local MISC WhoAreYou request failed.  " +
                                   "RC=" + res.rc + ", Result=" + res.result);
                return res;
            }
                
            STAFMarshallingContext mc = STAFMarshallingContext.unmarshall(
                res.result);

            Map resultsMap = (Map)mc.getRootObject();
            fInstanceUUID = (String)resultsMap.get("instanceUUID");

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
                            "is not running." + lineSep + res.result);
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

            // Assign the location that the service can write data to,
            // <STAF Write Location>/service/<service name (lower-case)>
            // and create the directory if it doesn't exist.

            fDataDir = info.writeLocation + fileSep + "service" + fileSep +
                fServiceName.toLowerCase();

            File dir = new File(fDataDir);

            if (!dir.exists())
            {
                dir.mkdirs();
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

                // Get the version of Jython distributed with the STAX service

                fJythonVersion = STAFServiceSharedJython.getJythonVersion();

                // Initialize the PythonInterpreter's python.home to point to
                // the shared jython installation.

                Properties p = new Properties();
                p.setProperty("python.home",
                              STAFServiceSharedJython.getJythonDirName());
                PythonInterpreter.initialize(System.getProperties(), p, null);
            }

            // Create a Xerces DOM parser to verify that Xerces is installed

            try
            {
                org.apache.xerces.parsers.DOMParser parser =
                  new org.apache.xerces.parsers.DOMParser();
            }
            catch (NoClassDefFoundError e)
            {
                rc = STAFResult.ServiceConfigurationError;
                System.out.println(
                    "ERROR: xercesImpl.jar is not in the classpath.");
                return new STAFResult(
                    rc, "ERROR: xercesImpl.jar is not in the classpath.");
            }

            // Create a Python Interpreter to verify that Jython is installed

            try
            {
                PythonInterpreter pyi = new PythonInterpreter();
                pyi = null;
            }
            catch (NoClassDefFoundError e)
            {
                rc = STAFResult.ServiceConfigurationError;
                System.out.println(
                    "ERROR: jython.jar is not in the classpath.");
                return new STAFResult(
                    rc, "ERROR: jython.jar is not in the classpath.");
            }

            // Verify that the version specified for the STAX service is valid

            try
            {
                STAFVersion serviceVersion = new STAFVersion(fVersion);
            }
            catch (NumberFormatException e)
            {
                rc = STAFResult.ServiceConfigurationError;

                System.out.println(
                    "ERROR: " + fVersion +
                    " is an invalid STAX service version." + lineSep +
                    e.toString());

                return new STAFResult(
                    rc, "ERROR: " + fVersion +
                    " is an invalid STAX service version." + lineSep +
                    e.toString());
            }

            // Assign the STAX service machine as the default EVENT service
            // machine and assign "Event" as the default EVENT service name.
            // These will be overridden if an EVENTSERVICEMACHINE or
            // EVENTSERVICENAME parameter is specified.

            fEventServiceMachine = "local";
            fEventServiceName = "Event";

            // Assign 5 as the default number of physical threads used by STAX
            fNumThreads = 5;

            // Check what the maximum queue size is set to on the STAX Service
            // machine and if < MAXQUEUESIZE, increase the maximum queue size
            // so that the STAX Service handles' queues won't get full and
            // lose messages if lots of messages are received when running
            // STAX jobs.

            res = fHandle.submit2("local", "MISC", "LIST SETTINGS");

            if (res.rc != 0)
            {
                System.out.println(
                    "ERROR:  Cannot list STAF settings.  RC=" + res.rc +
                    ", STAFResult=" + res.result);   
            }
            else
            {
                try
                {
                    Map resultMap = (Map)res.resultObj;
                    String maxQueueSizeString = (String)resultMap.get(
                        "maxQueueSize");

                    int maxQueueSize =
                        (new Integer(maxQueueSizeString)).intValue();

                    if (maxQueueSize < MAXQUEUESIZE)
                    {
                        res = fHandle.submit2(
                            "local", "MISC", "SET MAXQUEUESIZE " +
                            MAXQUEUESIZE);

                        if (res.rc != 0)
                        {
                            System.out.println(
                                "ERROR:  Cannot increase maximum queue " +
                                "size to " + MAXQUEUESIZE + ".  RC: " +
                                res.rc + ", Result: " + res.result);
                        }
                    }
                }
                catch (Exception e)
                {
                    System.out.println(
                        "ERROR: An exception occured getting the maximum " +
                        "queue size for STAF handles: " + e.toString());
                }
            }

            // Check what the maximum record size is set to for the LOG
            // service on the STAX Service machine and if < MAXLOGRECORDSIZE,
            // increase the maximum record size so that messages logged to
            // the STAX job logs won't get truncated (at least not until
            // 1M characters have been written).  The default MaxRecordSize
            // for the LOG service is 100,000 characters.

            res = fHandle.submit2("local", "LOG", "LIST SETTINGS");

            if (res.rc != 0)
            {
                System.out.println(
                    "Warning:  Cannot list the local LOG service's " +
                    "settings.  RC=" + res.rc + ", STAFResult=" + res.result);
            }
            else
            {
                try
                {
                    Map resultMap = (Map)res.resultObj;
                    String maxRecordSizeString = (String)resultMap.get(
                        "maxRecordSize");

                    int maxRecordSize =
                        (new Integer(maxRecordSizeString)).intValue();

                    if (maxRecordSize < MAXLOGRECORDSIZE)
                    {
                        res = fHandle.submit2(
                            "local", "LOG", "SET MAXRECORDSIZE " +
                            MAXLOGRECORDSIZE);

                        if (res.rc != 0)
                        {
                            System.out.println(
                                "Warning:  Cannot increase maximum log " +
                                "record size to " + MAXLOGRECORDSIZE +
                                ".  RC: " + res.rc +
                                ", Result: " + res.result);
                        }
                    }
                }
                catch (Exception e)
                {
                    System.out.println(
                        "Warning: An exception occured getting the maximum " +
                        "record size for the local LOG service: " +
                        e.toString());
                }
            }

            // Parse PARMS if provided in the STAX configuration line

            if (info.parms != null)
            {
                fParmsParser.addOption("EVENTSERVICEMACHINE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("EVENTSERVICENAME", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("NUMTHREADS", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("EXTENSIONXMLFILE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("EXTENSIONFILE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("EXTENSION", 0,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("PROCESSTIMEOUT", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("CLEARLOGS", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("LOGTCELAPSEDTIME", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("LOGTCNUMSTARTS", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("LOGTCSTARTSTOP", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("PYTHONOUTPUT", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("PYTHONLOGLEVEL", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("FILECACHING", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("MAXFILECACHESIZE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("FILECACHEALGORITHM", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("MAXFILECACHEAGE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("MAXMACHINECACHESIZE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("MAXRETURNFILESIZE", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("MAXGETQUEUEMESSAGES", 1,
                                       STAFCommandParser.VALUEREQUIRED);
                fParmsParser.addOption("MAXSTAXTHREADS", 1,
                                       STAFCommandParser.VALUEREQUIRED);

                // CACHEPYTHONCODE is an undocumented parameter
                // Mainly there in case any problems arise with the caching
                // of Python code added in STAX V1.5.0.
                fParmsParser.addOption("CACHEPYTHONCODE", 1,
                                       STAFCommandParser.VALUEREQUIRED);

                fParmsParser.addOptionNeed("MAXFILECACHEAGE", "FILECACHEALGORITHM");
                fParmsParser.addOptionGroup(
                    "EXTENSIONFILE EXTENSIONXMLFILE", 0, 1);

                res = handleParms(info);

                if (res.rc != STAFResult.Ok)
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "Error validating parameters: RC=" + res.rc +
                        ", Result=" + res.result);
                }
            }
            
            // Verify that the required version of the Event service
            // is running on the event service machine

            try
            {
                STAFResult eventVersionRes = STAFUtil.compareSTAFVersion(
                    fEventServiceMachine, fHandle,
                    kRequiredEventServiceVersion, fEventServiceName);

                if (eventVersionRes.rc != STAFResult.Ok)
                {
                    if (eventVersionRes.rc == STAFResult.InvalidSTAFVersion)
                    {
                        return new STAFResult(
                            STAFResult.ServiceConfigurationError,
                            "Minimum required version for the " +
                            fEventServiceName + " service on machine " +
                            fEventServiceMachine + " (as requeired by the " +
                            fServiceName + " service) is not running." +
                            lineSep + eventVersionRes.result);
                    }
                    else
                    {
                        // Do nothing; Event service not configured or had an
                        // invalid version
                    }
                }
            }
            catch (Error err)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "The " + fServiceName + " service requires the " +
                    fEventServiceName + " service on machine " +
                    fEventServiceMachine + " to be running Version " +
                    kRequiredEventServiceVersion + " or later."); 
            }

            // Create the STAX Thread Queue

            fThreadQueue = new STAXThreadQueue(fNumThreads);

            // Prime the element pump

            // Note that elements that implement a STAX request handler pass
            // parameters "this" to the action factory constructor.

            fActionFactoryMap.put("process",
                                  new STAXProcessActionFactory(this));
            fActionFactoryMap.put("stafcmd",
                                  new STAXSTAFCommandActionFactory(this));
            fActionFactoryMap.put("job",
                                  new STAXJobActionFactory(this));
            fActionFactoryMap.put("block", new STAXBlockActionFactory(this));
            fActionFactoryMap.put("testcase",
                                  new STAXTestcaseActionFactory(this));
            fActionFactoryMap.put("log", new STAXLogActionFactory(this));
            fActionFactoryMap.put("message", new STAXMessageActionFactory(this));
            fActionFactoryMap.put("breakpoint",
                                  new STAXBreakpointActionFactory(this));
            fActionFactoryMap.put("function",
                                  new STAXFunctionActionFactory(this));
            fActionFactoryMap.put("signalhandler",
                                  new STAXSignalHandlerActionFactory());
            fActionFactoryMap.put("sequence", new STAXSequenceActionFactory());
            fActionFactoryMap.put("parallel", new STAXParallelActionFactory());
            fActionFactoryMap.put("call", new STAXCallActionFactory());
            fActionFactoryMap.put("call-with-list",
                                  new STAXCallWithListActionFactory());
            fActionFactoryMap.put("call-with-map",
                                  new STAXCallWithMapActionFactory());
            fActionFactoryMap.put("script", new STAXScriptActionFactory());
            fActionFactoryMap.put("if", new STAXIfActionFactory());
            fActionFactoryMap.put("nop", new STAXNopActionFactory());
            fActionFactoryMap.put("loop", new STAXLoopActionFactory());
            fActionFactoryMap.put("iterate", new STAXIterateActionFactory());
            fActionFactoryMap.put("paralleliterate",
                                  new STAXParallelIterateActionFactory());
            fActionFactoryMap.put("terminate",
                                  new STAXTerminateActionFactory());
            fActionFactoryMap.put("hold", new STAXHoldActionFactory());
            fActionFactoryMap.put("release", new STAXReleaseActionFactory());
            fActionFactoryMap.put("tcstatus",
                                  new STAXTestcaseStatusActionFactory());
            fActionFactoryMap.put("break", new STAXBreakActionFactory());
            fActionFactoryMap.put("continue", new STAXContinueActionFactory());
            fActionFactoryMap.put("raise", new STAXRaiseActionFactory());
            fActionFactoryMap.put("try", new STAXTryActionFactory());
            fActionFactoryMap.put("throw", new STAXThrowActionFactory());
            fActionFactoryMap.put("rethrow", new STAXRethrowActionFactory());
            fActionFactoryMap.put("timer", new STAXTimerActionFactory());
            fActionFactoryMap.put("import", new STAXImportActionFactory());
            fActionFactoryMap.put("return", new STAXReturnActionFactory());

            // Prime the element pump with the Extension elements

            res = loadExtensions();

            if (res.rc != 0)
            {
                System.out.println(res.result);
                logToServiceLog("error", res.result);
                return res;
            }

            // Execute parser

            fExecuteParser.addOption("EXECUTE", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("FILE", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("MACHINE", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("DATA", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("JOBNAME", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("FUNCTION", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("ARGS", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("SCRIPT", 0,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("SCRIPTFILE", 0,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("SCRIPTFILEMACHINE", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("HOLD", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("TEST", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            // RETURNDETAILS option is un-documented
            fExecuteParser.addOption("RETURNDETAILS", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fExecuteParser.addOption("WAIT", 1,
                                     STAFCommandParser.VALUEALLOWED);
            fExecuteParser.addOption("RETURNRESULT", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("DETAILS", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("NOTIFY", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("ONEND", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("BYNAME", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("PRIORITY", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("KEY", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("CLEARLOGS", 1,
                                     STAFCommandParser.VALUEALLOWED);
            fExecuteParser.addOption("LOGTCELAPSEDTIME", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("LOGTCNUMSTARTS", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("LOGTCSTARTSTOP", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("PYTHONOUTPUT", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("PYTHONLOGLEVEL", 1,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("BREAKPOINT", 0,
                                     STAFCommandParser.VALUEREQUIRED);
            fExecuteParser.addOption("BREAKPOINTFIRSTFUNCTION", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fExecuteParser.addOption("BREAKPOINTSUBJOBFIRSTFUNCTION", 1,
                                     STAFCommandParser.VALUENOTALLOWED);

            fExecuteParser.addOptionGroup("FILE DATA", 0, 1);
            fExecuteParser.addOptionGroup("HOLD TEST WAIT", 0, 1);

            fExecuteParser.addOptionNeed("EXECUTE", "FILE DATA");
            fExecuteParser.addOptionNeed("MACHINE", "FILE");
            fExecuteParser.addOptionNeed("SCRIPTFILEMACHINE", "SCRIPTFILE");
            fExecuteParser.addOptionNeed("RETURNRESULT", "WAIT");
            fExecuteParser.addOptionNeed("DETAILS", "RETURNRESULT");
            fExecuteParser.addOptionNeed("RETURNDETAILS", "TEST");
            fExecuteParser.addOptionNeed("NOTIFY", "ONEND");
            fExecuteParser.addOptionNeed("ONEND", "NOTIFY");
            fExecuteParser.addOptionNeed("BYNAME", "NOTIFY");
            fExecuteParser.addOptionNeed("PRIORITY", "NOTIFY");
            fExecuteParser.addOptionNeed("KEY", "NOTIFY");

            // List parser

            fListParser.addOption("LIST", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("JOBS", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("JOB", 1,
                                  STAFCommandParser.VALUEREQUIRED);
            fListParser.addOption("SETTINGS", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("FILECACHE", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("SORTBYLRU", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("SORTBYLFU", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("SUMMARY", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("MACHINECACHE", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("EXTENSIONS", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("EXTENSIONJARFILES", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("THREADS", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("LONG", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("FORMAT", 1,
                                  STAFCommandParser.VALUEREQUIRED);
            fListParser.addOption("VARS", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fListParser.addOption("THREAD", 1,
                                  STAFCommandParser.VALUEREQUIRED);
            fListParser.addOption("SHORT", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            StringBuffer jobOptions = new StringBuffer("THREADS VARS");

            // Add additional options from registered list handlers
            synchronized (fListRequestMap)
            {
                Iterator iter = fListRequestMap.keySet().iterator();
                while (iter.hasNext())
                {
                    String typeName = (String)iter.next();
                    fListParser.addOption(typeName.toUpperCase(), 1,
                                          STAFCommandParser.VALUENOTALLOWED);
                    jobOptions.append(" " + typeName.toUpperCase());
                }
            }

            String listJobOptions = new String(jobOptions);
            String listOptions = new String(
                "JOB JOBS SETTINGS FILECACHE MACHINECACHE " +
                "EXTENSIONS EXTENSIONJARFILES");

            fListParser.addOptionGroup("LIST", 1, 1);
            fListParser.addOptionGroup(listOptions, 0, 1);
            fListParser.addOptionGroup(
                "SORTBYLRU SORTBYLFU SUMMARY", 0, 1);
            fListParser.addOptionNeed("LIST", listOptions);
            fListParser.addOptionNeed(listOptions, "LIST");
            fListParser.addOptionNeed(listJobOptions, "JOB");
            fListParser.addOptionNeed("JOB", listJobOptions);
            fListParser.addOptionNeed(
                "SORTBYLRU SORTBYLFU SUMMARY", "FILECACHE");
            fListParser.addOptionNeed("LONG", "THREADS");
            fListParser.addOptionNeed("VARS", "THREAD");
            fListParser.addOptionNeed("THREAD", "VARS");
            fListParser.addOptionNeed("SHORT", "VARS");

            // Query parser

            fQueryParser.addOption("QUERY", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fQueryParser.addOption("JOB", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fQueryParser.addOption("THREAD", 0,
                                   STAFCommandParser.VALUEREQUIRED);
            fQueryParser.addOption("EXTENSIONJARFILE", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fQueryParser.addOption("EXTENSIONJARFILES", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fQueryParser.addOption("VAR", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fQueryParser.addOption("SHORT", 1,
                                   STAFCommandParser.VALUENOTALLOWED);

            jobOptions = new StringBuffer("THREAD");

            // Add additional options from registered query handlers
            synchronized (fQueryRequestMap)
            {
                Iterator iter = fQueryRequestMap.keySet().iterator();
                while (iter.hasNext())
                {
                    String typeName = ((String)iter.next()).toUpperCase();
                    fQueryParser.addOption(typeName, 0,
                                           STAFCommandParser.VALUEREQUIRED);
                    jobOptions.append(" " + typeName);
                }
            }
            String queryJobOptions = jobOptions.toString();
            String queryOptions = "JOB EXTENSIONJARFILE EXTENSIONJARFILES";

            fQueryParser.addOptionGroup("QUERY", 1, 1);

            fQueryParser.addOptionGroup(queryOptions, 0, 1);
            fQueryParser.addOptionNeed(queryOptions, "QUERY");
            fQueryParser.addOptionNeed("QUERY", queryOptions);
            fQueryParser.addOptionNeed(queryJobOptions, "JOB");
            fQueryParser.addOptionNeed("VAR", "THREAD");
            fQueryParser.addOptionNeed("SHORT", "VAR");

            // Get parser

            fGetParser.addOption("GET", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fGetParser.addOption("DTD", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fGetParser.addOption("RESULT", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fGetParser.addOption("JOB", 1,
                                  STAFCommandParser.VALUEREQUIRED);
            fGetParser.addOption("DETAILS", 1,
                                  STAFCommandParser.VALUENOTALLOWED);

            // Undocumented option STAX-EXTENSIONS-FILE - gets the DTD for
            // the STAX Extensions XML file
            fGetParser.addOption("STAX-EXTENSIONS-FILE", 1,
                                 STAFCommandParser.VALUENOTALLOWED);

            fGetParser.addOptionGroup("GET", 1, 1);
            fGetParser.addOptionGroup("DTD RESULT", 0, 1);
            fGetParser.addOptionNeed("GET", "DTD RESULT");
            fGetParser.addOptionNeed("DTD RESULT", "GET");
            fGetParser.addOptionNeed("RESULT", "JOB");
            fGetParser.addOptionNeed("JOB", "RESULT");
            fGetParser.addOptionNeed("DETAILS", "RESULT");
            fGetParser.addOptionNeed("STAX-EXTENSIONS-FILE", "DTD");

            // Set parser

            fSetParser.addOption("SET", 1,
                                 STAFCommandParser.VALUENOTALLOWED);
            fSetParser.addOption("CLEARLOGS", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("LOGTCELAPSEDTIME", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("LOGTCNUMSTARTS", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("LOGTCSTARTSTOP", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("PYTHONOUTPUT", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("PYTHONLOGLEVEL", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("FILECACHING", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("MAXFILECACHESIZE", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("FILECACHEALGORITHM", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("MAXFILECACHEAGE", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("MAXMACHINECACHESIZE", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("MAXRETURNFILESIZE", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("MAXGETQUEUEMESSAGES", 1,
                                 STAFCommandParser.VALUEREQUIRED);
            fSetParser.addOption("MAXSTAXTHREADS", 1,
                                 STAFCommandParser.VALUEREQUIRED);

            String setOptions = "CLEARLOGS LOGTCELAPSEDTIME " +
                "LOGTCNUMSTARTS LOGTCSTARTSTOP PYTHONOUTPUT PYTHONLOGLEVEL " +
                "FILECACHING MAXFILECACHESIZE FILECACHEALGORITHM " +
                "MAXFILECACHEAGE MAXMACHINECACHESIZE MAXRETURNFILESIZE " +
                "MAXGETQUEUEMESSAGES MAXSTAXTHREADS";
            fSetParser.addOptionNeed("SET", setOptions);
            fSetParser.addOptionNeed(setOptions, "SET");

            // Notify parser

            fNotifyParser.addOption("NOTIFY", 1,
                                    STAFCommandParser.VALUENOTALLOWED);
            fNotifyParser.addOption("REGISTER", 1,
                                    STAFCommandParser.VALUENOTALLOWED);
            fNotifyParser.addOption("UNREGISTER", 1,
                                    STAFCommandParser.VALUENOTALLOWED);
            fNotifyParser.addOption("LIST", 1,
                                    STAFCommandParser.VALUENOTALLOWED);
            fNotifyParser.addOption("ONENDOFJOB", 1,
                                    STAFCommandParser.VALUEREQUIRED);
            fNotifyParser.addOption("JOB", 1,
                                    STAFCommandParser.VALUEREQUIRED);
            fNotifyParser.addOption("BYNAME", 1,
                                    STAFCommandParser.VALUENOTALLOWED);
            fNotifyParser.addOption("PRIORITY", 1,
                                    STAFCommandParser.VALUEREQUIRED);

            fNotifyParser.addOptionGroup("NOTIFY", 1, 1);
            fNotifyParser.addOptionGroup("REGISTER UNREGISTER LIST", 1, 1);
            fNotifyParser.addOptionNeed("ONENDOFJOB", "REGISTER UNREGISTER");
            fNotifyParser.addOptionNeed("REGISTER UNREGISTER", "ONENDOFJOB");
            fNotifyParser.addOptionNeed("JOB", "LIST");
            fNotifyParser.addOptionNeed("BYNAME", "REGISTER");
            fNotifyParser.addOptionNeed("PRIORITY", "REGISTER");
            
            // Purge Parser
            
            fPurgeParser.addOption("PURGE", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fPurgeParser.addOption("FILECACHE", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fPurgeParser.addOption("MACHINECACHE", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fPurgeParser.addOption("CONFIRM", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            
            fPurgeParser.addOptionGroup("FILECACHE MACHINECACHE", 1, 1);
            fPurgeParser.addOptionGroup("CONFIRM", 1, 1);
            fPurgeParser.addOptionNeed("FILECACHE MACHINECACHE", "PURGE");

            // Stop Parser

            fStopParser.addOption("STOP", 1,
                                   STAFCommandParser.VALUENOTALLOWED);
            fStopParser.addOption("JOB", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fStopParser.addOption("THREAD", 1,
                                   STAFCommandParser.VALUEREQUIRED);

            fStopParser.addOptionGroup("STOP", 1, 1);

            fStopParser.addOptionNeed("JOB THREAD", "STOP");
            fStopParser.addOptionNeed("STOP", "JOB THREAD");
            fStopParser.addOptionNeed("JOB", "THREAD");
            fStopParser.addOptionNeed("THREAD", "JOB");

            // PyExec Parser

            fPyExecParser.addOption("PYEXEC", 1,
                                    STAFCommandParser.VALUENOTALLOWED);
            fPyExecParser.addOption("JOB", 1,
                                    STAFCommandParser.VALUEREQUIRED);
            fPyExecParser.addOption("THREAD", 1,
                                    STAFCommandParser.VALUEREQUIRED);
            fPyExecParser.addOption("CODE", 1,
                                    STAFCommandParser.VALUEREQUIRED);

            fPyExecParser.addOptionGroup("PYEXEC", 1, 1);

            fPyExecParser.addOptionNeed("JOB THREAD", "PYEXEC");
            fPyExecParser.addOptionNeed("PYEXEC", "JOB THREAD");
            fPyExecParser.addOptionNeed("CODE", "PYEXEC");
            fPyExecParser.addOptionNeed("PYEXEC", "CODE");
            fPyExecParser.addOptionNeed("JOB", "THREAD");
            fPyExecParser.addOptionNeed("THREAD", "JOB");

            // Version parser

            fVersionParser.addOption("VERSION", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fVersionParser.addOption("JYTHON", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
            fVersionParser.addOptionGroup("VERSION", 1, 1);
            fVersionParser.addOptionNeed("JYTHON", "VERSION");

            // Help parser

            fHelpParser.addOption("HELP", 1,
                                  STAFCommandParser.VALUENOTALLOWED);
            fHelpParser.addOptionGroup("HELP", 1, 1);

            // Construct map-class for job result information

            fJobResultMapClass = new STAFMapClassDefinition(
                sJobResultMapClassName);

            fJobResultMapClass.addKey("jobID",  "Job ID");
            fJobResultMapClass.addKey("startTimestamp", "Start Date-Time");
            fJobResultMapClass.addKey("endTimestamp", "End Date-Time");
            fJobResultMapClass.addKey("status", "Status");
            fJobResultMapClass.addKey("result", "Result");
            fJobResultMapClass.addKey("jobLogErrors", "Job Log Errors");
            fJobResultMapClass.addKey("testcaseTotals", "Testcase Totals");
            
            // Construct map-class for job result information with
            // testcase list

            fJobDetailedResultMapClass = new STAFMapClassDefinition(
                sJobDetailedResultMapClassName);

            fJobDetailedResultMapClass.addKey("jobID",  "Job ID");
            fJobDetailedResultMapClass.addKey("startTimestamp",
                                              "Start Date-Time");
            fJobDetailedResultMapClass.addKey("endTimestamp",
                                              "End Date-Time");
            fJobDetailedResultMapClass.addKey("status", "Status");
            fJobDetailedResultMapClass.addKey("result", "Result");
            fJobDetailedResultMapClass.addKey("jobLogErrors",
                                              "Job Log Errors");
            fJobDetailedResultMapClass.addKey("testcaseTotals",
                                              "Testcase Totals");
            fJobDetailedResultMapClass.addKey("testcaseList", "Testcases");

            // Construct map-class for job details information

            fJobDetailsMapClass = new STAFMapClassDefinition(
                sJobDetailsMapClassName);

            fJobDetailsMapClass.addKey("jobID",        "Job ID");
            fJobDetailsMapClass.addKey("defaultCall",  "Default Call");
            fJobDetailsMapClass.addKey("functionList", "Functions");

            // Construct map-class for function information

            fFunctionInfoMapClass = new STAFMapClassDefinition(
                sFunctionInfoMapClassName);

            fFunctionInfoMapClass.addKey("functionName",  "Function Name");
            fFunctionInfoMapClass.addKey("prolog",        "Prolog");
            fFunctionInfoMapClass.addKey("epilog",        "Epilog");
            fFunctionInfoMapClass.addKey("argDefinition", "Argument Definition");
            fFunctionInfoMapClass.addKey("argList",       "Arguments");

            // Construct map-class for function information

            fArgInfoMapClass = new STAFMapClassDefinition(sArgInfoMapClassName);

            fArgInfoMapClass.addKey("argName",      "Argument Name");
            fArgInfoMapClass.addKey("description",  "Description");
            fArgInfoMapClass.addKey("type",         "Type");
            fArgInfoMapClass.addKey("defaultValue", "Default Value");
            fArgInfoMapClass.addKey("private",      "Private");
            fArgInfoMapClass.addKey("properties",   "Properties");

            // Construct map-class for function arg property information

            fArgPropertyInfoMapClass = new 
                STAFMapClassDefinition(sArgPropertyInfoMapClassName);

            fArgPropertyInfoMapClass.addKey("propertyName",
                                            "Name");
            fArgPropertyInfoMapClass.addKey("propertyDescription",
                                            "Description");

            fArgPropertyInfoMapClass.addKey("propertyValue",
                                            "Value");

            fArgPropertyInfoMapClass.addKey("propertyData",
                                            "Data");

            // Construct map-class for function arg property information

            fArgPropertyDataInfoMapClass = new 
                STAFMapClassDefinition(sArgPropertyDataInfoMapClassName);

            fArgPropertyDataInfoMapClass.addKey("dataType",
                                                "Type");

            fArgPropertyDataInfoMapClass.addKey("dataValue",
                                                "Value");

            fArgPropertyDataInfoMapClass.addKey("dataData",
                                                "Data");

            // Construct map-class for file cache list
            
            fFileCacheMapClass = new STAFMapClassDefinition(
                sFileCacheMapClassName);

            fFileCacheMapClass.addKey("machine", "Machine");
            fFileCacheMapClass.addKey("file", "File");
            fFileCacheMapClass.addKey("hits", "Hits");
            fFileCacheMapClass.addKey("lastHit", "Last Hit Date-Time");
            fFileCacheMapClass.setKeyProperty(
                "lastHit", "display-short-name", "Last Hit");
            fFileCacheMapClass.addKey("addDate", "Added Date-Time");
            fFileCacheMapClass.setKeyProperty(
                "addDate", "display-short-name", "Added");
            
            // Construct map-class for file cache summary

            fFileCacheSummaryMapClass = new STAFMapClassDefinition(
                sFileCacheSummaryMapClassName);

            fFileCacheSummaryMapClass.addKey("hitRatio", "Hit Ratio");
            fFileCacheSummaryMapClass.addKey("hitCount", "Hit Count");
            fFileCacheSummaryMapClass.addKey("missCount", "Miss Count");
            fFileCacheSummaryMapClass.addKey("requestCount", "Request Count");
            fFileCacheSummaryMapClass.addKey("lastPurgeDate",
                                             "Last Purge Date-Time");

            // Construct map-class for machine cache list
            
            fMachineCacheMapClass = new STAFMapClassDefinition(
                sMachineCacheMapClassName);

            fMachineCacheMapClass.addKey("machine", "Machine");
            fMachineCacheMapClass.addKey("fileSep", "File Separator");
            fMachineCacheMapClass.setKeyProperty(
                "fileSep", "display-short-name", "File Sep");
            fMachineCacheMapClass.addKey("hits", "Hits");
            fMachineCacheMapClass.addKey("lastHit", "Last Hit Date-Time");
            fMachineCacheMapClass.setKeyProperty(
                "lastHit", "display-short-name", "Last Hit");
            fMachineCacheMapClass.addKey("addDate", "Added Date-Time");
            fMachineCacheMapClass.setKeyProperty(
                "addDate", "display-short-name", "Added");

            // Construct map-class for purge stats
            
            fPurgeStatsMapClass = new STAFMapClassDefinition(
                sPurgeStatsMapClassName);
            
            fPurgeStatsMapClass.addKey("numPurged", "Number Purged");
            fPurgeStatsMapClass.addKey("numRemaining", "Number Remaining");
            
            // Construct map-class for list jobs information

            fJobInfoMapClass = new STAFMapClassDefinition(
                sJobInfoMapClassName);

            fJobInfoMapClass.addKey("jobID",          "Job ID");
            fJobInfoMapClass.addKey("jobName",        "Job Name");
            fJobInfoMapClass.addKey("startTimestamp", "Start Date-Time");
            fJobInfoMapClass.addKey("function",       "Function");
            fJobInfoMapClass.addKey("state",          "State");

            // Construct map-class for list job <Job ID> threads information

            fThreadInfoMapClass = new STAFMapClassDefinition(
                sThreadInfoMapClassName);

            fThreadInfoMapClass.addKey("threadID",  "Thread ID");
            fThreadInfoMapClass.addKey("parentTID", "Parent TID");
            fThreadInfoMapClass.addKey("state",     "State");

            // Construct map-class for list job <Job ID> threads long info

            fThreadLongInfoMapClass = new STAFMapClassDefinition(
                sThreadLongInfoMapClassName);

            fThreadLongInfoMapClass.addKey("threadID",  "Thread ID");
            fThreadLongInfoMapClass.addKey("parentTID", "Parent TID");
            fThreadLongInfoMapClass.addKey("parentHierarchy",
                                           "Parent Hierarchy");
            fThreadLongInfoMapClass.addKey("state",     "State");
            fThreadLongInfoMapClass.addKey("startTimestamp",
                                           "Start Date-Time");
            fThreadLongInfoMapClass.addKey("callStack", "Call Stack");
            fThreadLongInfoMapClass.addKey("conditionStack",
                                           "Condition Stack");

            // Construct map-class for list settings information

            fSettingsMapClass = new STAFMapClassDefinition(
                sSettingsMapClassName);

            fSettingsMapClass.addKey("eventMachine",   "Event Machine");
            fSettingsMapClass.addKey("eventService",   "Event Service Name");
            fSettingsMapClass.addKey("numThreads",     "Number of Threads");
            fSettingsMapClass.addKey("processTimeout", "Process Timeout");
            fSettingsMapClass.addKey("fileCaching", "File Caching");
            fSettingsMapClass.addKey("maxFileCacheSize",
                                     "Max File Cache Size");
            fSettingsMapClass.addKey("fileCacheAlgorithm",
                                     "File Cache Algorithm");
            fSettingsMapClass.addKey("maxFileCacheAge", "Max File Cache Age");
            fSettingsMapClass.addKey("maxMachineCacheSize",
                                     "Max Machine Cache Size");
            fSettingsMapClass.addKey("maxReturnFileSize",
                                     "Max Return File Size");
            fSettingsMapClass.addKey("maxGetQueueMessages",
                                     "Max Get Queue Messages");
            fSettingsMapClass.addKey("maxSTAXThreads", "Max STAX-Threads");
            fSettingsMapClass.addKey("clearLogs",      "Clear Logs");
            fSettingsMapClass.addKey("logTCElapsedTime",
                                      "Log TC Elapsed Time");
            fSettingsMapClass.addKey("logTCNumStarts", "Log TC Num Starts");
            fSettingsMapClass.addKey("logTCStartStop", "Log TC Start/Stop");
            fSettingsMapClass.addKey("pythonOutput", "Python Output");
            fSettingsMapClass.addKey("pythonLogLevel", "Python Log Level");
            fSettingsMapClass.addKey("extensions", "Extensions");
            fSettingsMapClass.addKey("extensionFile", "Extension File");

            // Construct map-class for extension element information

            fExtensionElementMapClass = new STAFMapClassDefinition(
                sExtensionElementMapClassName);

            fExtensionElementMapClass.addKey("extensionElement",
                                             "Extension Element");
            fExtensionElementMapClass.addKey("extensionJarFile",
                                             "Extension Jar File");

            // Construct map-class for extension jar file information

            fExtensionJarFileMapClass = new STAFMapClassDefinition(
                sExtensionJarFileMapClassName);

            fExtensionJarFileMapClass.addKey("extensionJarFile",
                                             "Extension Jar File");
            fExtensionJarFileMapClass.addKey("version", "Version");
            fExtensionJarFileMapClass.addKey("description", "Description");

            // Construct map-class for extension information

            fExtensionInfoMapClass = new STAFMapClassDefinition(
                sExtensionInfoMapClassName);

            fExtensionInfoMapClass.addKey("extensionJarFile",
                                          "Extension Jar File");
            fExtensionInfoMapClass.addKey("version", "Version");
            fExtensionInfoMapClass.addKey("description", "Description");
            fExtensionInfoMapClass.addKey("parameterList", "Parameters");
            fExtensionInfoMapClass.addKey("serviceExtensions",
                                          "Service Extensions");
            fExtensionInfoMapClass.addKey("monitorExtensions",
                                          "Monitor Extensions" );

            // Construct map-class for service extension information

            fServiceExtensionMapClass = new STAFMapClassDefinition(
                sServiceExtensionMapClassName);

            fServiceExtensionMapClass.addKey("requiredServiceVersion",
                                             "Service Version Prereq");
            fServiceExtensionMapClass.addKey("includedElementList",
                                             "Included Elements");
            fServiceExtensionMapClass.addKey("excludedElementList",
                                             "Excluded Elements");

            // Construct map-class for monitor extension information

            fMonitorExtensionMapClass = new STAFMapClassDefinition(
                sMonitorExtensionMapClassName);

            fMonitorExtensionMapClass.addKey("requiredMonitorVersion",
                                             "Monitor Version Prereq");
            fMonitorExtensionMapClass.addKey("extensionNameList",
                                             "Extension Names");

            // Construct map-class for query job information

            fQueryJobMapClass = new STAFMapClassDefinition(
                sQueryJobMapClassName);

            fQueryJobMapClass.addKey("jobID", "Job ID");
            fQueryJobMapClass.addKey("jobName", "Job Name");
            fQueryJobMapClass.addKey("startTimestamp", "Start Date-Time");
            fQueryJobMapClass.addKey("xmlFileName", "XML File Name");
            fQueryJobMapClass.addKey("fileMachine", "File Machine");
            fQueryJobMapClass.addKey("function", "Function");
            fQueryJobMapClass.addKey("arguments", "Arguments");
            fQueryJobMapClass.addKey("scriptList", "Scripts");
            fQueryJobMapClass.addKey("scriptFileList", "Script Files");
            fQueryJobMapClass.addKey("scriptMachine", "Script Machine");
            fQueryJobMapClass.addKey("sourceMachine", "Source Machine");
            fQueryJobMapClass.addKey("notifyOnEnd", "Notify OnEnd");
            fQueryJobMapClass.addKey("clearLogs", "Clear Logs");
            fQueryJobMapClass.addKey("logTCElapsedTime",
                                     "Log TC Elapsed Time");
            fQueryJobMapClass.addKey("logTCNumStarts", "Log TC Num Starts");
            fQueryJobMapClass.addKey("logTCStartStop", "Log TC Start/Stop");
            fQueryJobMapClass.addKey("pythonOutput", "Python Output");
            fQueryJobMapClass.addKey("pythonLogLevel", "Python Log Level");
            fQueryJobMapClass.addKey("numThreadsRunning", "Threads Running");
            fQueryJobMapClass.addKey("numBlocksRunning", "Blocks Running");
            fQueryJobMapClass.addKey("numBlocksHeld", "Blocks Held");
            fQueryJobMapClass.addKey("numBlocksUnknown", "Blocks Unknown");
            fQueryJobMapClass.addKey("state", "State");

            // Construct map-class for query thread information for a job

            fQueryThreadMapClass = new STAFMapClassDefinition(
                sQueryThreadMapClassName);

            fQueryThreadMapClass.addKey("threadID", "Thread ID");
            fQueryThreadMapClass.addKey("parentTID", "Parent TID");
            fQueryThreadMapClass.addKey("parentHierarchy", "Parent Hierarchy");
            fQueryThreadMapClass.addKey("startTimestamp", "Start Date-Time");
            fQueryThreadMapClass.addKey("callStack", "Call Stack");
            fQueryThreadMapClass.addKey("conditionStack", "Condition Stack");

            fThreadVariableMapClass = new STAFMapClassDefinition(
                sThreadVariableMapClassName);

            fThreadVariableMapClass.addKey("name", "Name");
            fThreadVariableMapClass.addKey("value", "Value");
            fThreadVariableMapClass.addKey("type", "Type");

            // Construct map-class for listing registered notifiees for
            // job completion for a specified job

            fNotifieeMapClass = new STAFMapClassDefinition(
                sNotifieeMapClassName);

            fNotifieeMapClass.addKey("machine", "Machine");
            fNotifieeMapClass.addKey("handle", "Handle");
            fNotifieeMapClass.setKeyProperty(
                "handle", "display-short-name", "H");
            fNotifieeMapClass.addKey("handleName", "Handle Name");
            fNotifieeMapClass.addKey("notifyBy", "Notify By");
            fNotifieeMapClass.setKeyProperty(
                "notifyBy", "display-short-name", "Notify");
            fNotifieeMapClass.addKey("priority", "Priority");
            fNotifieeMapClass.setKeyProperty(
                "priority", "display-short-name", "P");
            
            // Construct map-class for listing registered notifiees for
            // job completion for all jobs

            fAllNotifieeMapClass = new STAFMapClassDefinition(
                sAllNotifieeMapClassName);

            fAllNotifieeMapClass.addKey("jobID", "Job ID");
            fAllNotifieeMapClass.setKeyProperty(
                "jobID", "display-short-name", "ID");
            fAllNotifieeMapClass.addKey("machine", "Machine");
            fAllNotifieeMapClass.addKey("handle", "Handle");
            fAllNotifieeMapClass.setKeyProperty(
                "handle", "display-short-name", "H");
            fAllNotifieeMapClass.addKey("handleName", "Handle Name");
            fAllNotifieeMapClass.addKey("notifyBy", "Notify By");
            fAllNotifieeMapClass.setKeyProperty(
                "notifyBy", "display-short-name", "Notify");
            fAllNotifieeMapClass.addKey("priority", "Priority");
            fAllNotifieeMapClass.setKeyProperty(
                "priority", "display-short-name", "P");

            // Construct map-class for job results without the testcase list

            fResultMapClass = new STAFMapClassDefinition(
                sGetResultMapClassName);
            fResultMapClass.addKey("jobName", "Job Name");
            fResultMapClass.addKey("startTimestamp", "Start Date-Time");
            fResultMapClass.addKey("endTimestamp", "End Date-Time");
            fResultMapClass.addKey("status", "Status");
            fResultMapClass.addKey("result", "Result");
            fResultMapClass.addKey("jobLogErrors", "Job Log Errors");
            fResultMapClass.addKey("testcaseTotals", "Testcase Totals");
            fResultMapClass.addKey("xmlFileName", "XML File Name");
            fResultMapClass.addKey("fileMachine", "File Machine");
            fResultMapClass.addKey("function", "Function");
            fResultMapClass.addKey("arguments",  "Arguments");
            fResultMapClass.addKey("scriptList", "Scripts");
            fResultMapClass.addKey("scriptFileList", "Script Files");
            fResultMapClass.addKey("scriptMachine", "Script Machine");

            // Construct map-class for job results with the testcase list

            fDetailedResultMapClass = new STAFMapClassDefinition(
                sGetDetailedResultMapClassName);
            fDetailedResultMapClass.addKey("jobName", "Job Name");
            fDetailedResultMapClass.addKey("startTimestamp",
                                           "Start Date-Time");
            fDetailedResultMapClass.addKey("endTimestamp", "End Date-Time");
            fDetailedResultMapClass.addKey("status", "Status");
            fDetailedResultMapClass.addKey("result", "Result");
            fDetailedResultMapClass.addKey("jobLogErrors", "Job Log Errors");
            fDetailedResultMapClass.addKey("testcaseTotals",
                                           "Testcase Totals");
            fDetailedResultMapClass.addKey("testcaseList", "Testcases");
            fDetailedResultMapClass.addKey("xmlFileName", "XML File Name");
            fDetailedResultMapClass.addKey("fileMachine", "File Machine");
            fDetailedResultMapClass.addKey("function", "Function");
            fDetailedResultMapClass.addKey("arguments",  "Arguments");
            fDetailedResultMapClass.addKey("scriptList", "Scripts");
            fDetailedResultMapClass.addKey("scriptFileList", "Script Files");
            fDetailedResultMapClass.addKey("scriptMachine",
                                           "Script Machine");

            // Construct map-class for a job error result

            fExecuteErrorResultMapClass = new STAFMapClassDefinition(
                sExecuteErrorResultMapClassName);
            fExecuteErrorResultMapClass.addKey("jobID", "Job ID");
            fExecuteErrorResultMapClass.addKey("errorMsg", "Error Message");

            // Log the registered extensions in the STAX Service Log

            synchronized (fExtensionsJarList)
            {
                Iterator iter = fExtensionsJarList.iterator();

                StringBuffer result = new StringBuffer(
                    lineSep + "Registered Extensions for " + fServiceName +
                    " Version " + fVersion + ":" + lineSep);

                while (iter.hasNext())
                {
                    STAXExtension ext = (STAXExtension)iter.next();

                    result.append(lineSep + getFormattedExtensionInfo(ext));
                }

                logToServiceLog("info", result.toString());
                System.out.println(result.toString());
            }

            // Create the DTD

            // Get the current date and time

            STAXTimestamp currTimestamp = new STAXTimestamp();

            // Get a list of the service extension element names
            
            List extensionElements = new LinkedList();

            synchronized (fExtensionElementMap)
            {
                Iterator iter = fExtensionElementMap.keySet().iterator();

                while (iter.hasNext())
                {
                    extensionElements.add((String)iter.next());
                }
            }

// Begin DTD chunk............................................................
            fDTD =
"<!--\n" +
"   STAf eXecution (STAX) Document Type Definition (DTD)\n" +
"\n" +
"   STAX Version: " + fVersion + "\n" +
"\n" +
"   STAX Extension Elements: " + extensionElements + "\n" +
"\n" +
"   Generated Date: " + currTimestamp.getTimestampString() + "\n" +
"\n" +
"   This DTD module is identified by the SYSTEM identifier:\n" +
"\n" +
"     SYSTEM 'stax.dtd'\n" +
"\n" +
"-->\n" +
"\n" +
"<!-- Parameter entities referenced in Element declarations -->\n" +
"\n" +
"<!ENTITY % stax-elems 'function | script | signalhandler'>\n" +
"\n";
// End DTD chunk..............................................................

// Begin DTD chunk............................................................
            String staxDTDsection =
"\n" +
"<!--================= STAX Job Definition ========================== -->\n" +
"<!--\n" +
"     The root element STAX contains all other elements.  It consists\n" +
"     of an optional defaultcall element and any number of function,\n" +
"     script, and/or signalhandler elements.\n" +
"-->\n" +
"<!ELEMENT stax         ((%stax-elems;)*, defaultcall?, (%stax-elems;)*)>\n" +
"\n" +
"<!--================= The Default Call Function Element ============ -->\n" +
"<!--\n" +
"     The defaultcall element defines the function to call by default\n" +
"     to start the job.  This can be overridden by the 'FUNCTION'\n" +
"     parameter when submitting the job to be executed.\n" +
"     The function attribute's value is a literal.\n" +
"-->\n" +
"<!ELEMENT defaultcall  (#PCDATA)>\n" +
"<!ATTLIST defaultcall\n" +
"          function     IDREF    #REQUIRED\n" +
">\n";
// End DTD chunk..............................................................

            StringBuffer taskEntitySection =
                new StringBuffer("<!ENTITY % task       '");
            StringBuffer elemDTDsection = new StringBuffer();
            int numTasks = 0;

            synchronized (fActionFactoryMap)
            {
                Iterator factory_iter = fActionFactoryMap.keySet().iterator();
                while (factory_iter.hasNext())
                {
                    String actionFactoryName = (String)factory_iter.next();
                    STAXActionFactory factory = (STAXActionFactory)
                        fActionFactoryMap.get(actionFactoryName);

                    elemDTDsection.append(factory.getDTDInfo());

                    String taskName = factory.getDTDTaskName();

                    if (taskName != null)
                    {
                        // Put 3 actions in each line for the task entity.
                        if (numTasks == 0)
                            taskEntitySection.append(factory.getDTDTaskName());
                        else if (numTasks % 3 == 0)
                            taskEntitySection.append(" | \n" +
                                                 "                       " +
                                                 factory.getDTDTaskName());
                        else
                            taskEntitySection.append(" | " +
                                                 factory.getDTDTaskName());
                        numTasks++;
                    }
                }
            }

            taskEntitySection.append("'>\n");

            StringBuffer dtdBuff = new StringBuffer(fDTD);
            dtdBuff.append(taskEntitySection);
            dtdBuff.append(staxDTDsection);
            dtdBuff.append(elemDTDsection);
            fDTD = dtdBuff.toString();

            // Assign the help text string for the service

            sHelpMsg = "*** " + info.name + " Service Help ***" +
                lineSep + lineSep +
                "EXECUTE   < <FILE <XML File Name> [MACHINE <Machine Name>]> | DATA <XML Data> >" +
                lineSep +
                "          [JOBNAME <Job Name>] [FUNCTION <Function ID>] [ARGS <Arguments>]" +
                lineSep +
                "          [SCRIPTFILE <File Name>... [SCRIPTFILEMACHINE <Machine Name>]] " +
                lineSep +
                "          [SCRIPT <Python Code>]... [CLEARLOGS [<Enabled | Disabled>]] " +
                lineSep +
                "          [ HOLD | TEST [RETURNDETAILS] | " +
                lineSep +
                "            WAIT [<Number>[s|m|h|d|w]] [RETURNRESULT [DETAILS]] ]" +
                lineSep +
                "          [ NOTIFY ONEND [BYNAME] [PRIORITY <Priority>] [KEY <Key>] ]" +
                lineSep +
                "          [LOGTCELAPSEDTIME <Enabled | Disabled>]" +
                lineSep +
                "          [LOGTCNUMSTARTS <Enabled | Disabled>]" +
                lineSep +
                "          [LOGTCSTARTSTOP <Enabled | Disabled>]" +
                lineSep +
                "          [PYTHONOUTPUT <Python Output>] [PYTHONLOGLEVEL <Log Level>]" +
                lineSep +
                "          [ BREAKPOINT <Function name> | <Line>[@@<File>[@@<Machine>]] ]..." +
                lineSep +
                "          [BREAKPOINTFIRSTFUNCTION] [BREAKPOINTSUBJOBFIRSTFUNCTION]" +
                lineSep + lineSep +
                "GET       DTD" +
                lineSep +
                "GET       RESULT JOB <Job ID> [DETAILS]" +
                lineSep + lineSep +
                "LIST      JOBS | SETTINGS | MACHINECACHE |" +
                lineSep +
                "          FILECACHE [SORTBYLRU | SORTBYLFU | SUMMARY] |" +
                lineSep +
                "          EXTENSIONS | EXTENSIONJARFILES |" +
                lineSep +
                "          JOB <Job ID> <" + fHelpListJobOptions + ">" +
                lineSep + lineSep +
                "QUERY     EXTENSIONJARFILE <Jar File Name> | EXTENSIONJARFILES |" +
                lineSep +
                "          JOB <Job ID> [" + fHelpQueryJobOptions + "]" +
                lineSep + lineSep +
                fHelpGenericRequests +
                "STOP      JOB <Job ID> THREAD <Thread ID>" +
                lineSep + lineSep +
                "PYEXEC    JOB <Job ID> THREAD <Thread ID> CODE <Python Code>" +
                lineSep + lineSep +
                "SET       [CLEARLOGS <Enabled | Disabled>]" +
                lineSep +
                "          [LOGTCELAPSEDTIME <Enabled | Disabled>]" +
                lineSep +
                "          [LOGTCNUMSTARTS <Enabled | Disabled>]" +
                lineSep +
                "          [LOGTCSTARTSTOP <Enabled | Disabled>]" +
                lineSep +
                "          [PYTHONOUTPUT <Python Output>]" +
                lineSep +
                "          [PYTHONLOGLEVEL <Log Level>]" +
                lineSep +
                "          [FILECACHING <Enabled | Disabled>]" +
                lineSep +
                "          [MAXFILECACHESIZE <Max Files>]" +
                lineSep +
                "          [FILECACHEALGORITHM <LRU | LFU>]" +
                lineSep +
                "          [MAXFILECACHEAGE <Number>[s|m|h|d|w]]" +
                lineSep +
                "          [MAXMACHINECACHESIZE <Max Machines>]" +
                lineSep +
                "          [MAXRETURNFILESIZE <Number>[k|m]]" +
                lineSep +
                "          [MAXGETQUEUEMESSAGES <Number>]" +
                lineSep +
                "          [MAXSTAXTHREADS <Number>]" +
                lineSep + lineSep +
                "NOTIFY    REGISTER   ONENDOFJOB <Job ID> [BYNAME] [PRIORITY <Priority>]" +
                lineSep +
                "NOTIFY    UNREGISTER ONENDOFJOB <Job ID>" +
                lineSep +
                "NOTIFY    LIST       [JOB <Job ID>]" +
                lineSep + lineSep +
                "PURGE     <FILECACHE | MACHINECACHE> CONFIRM" +
                lineSep + lineSep +
                "VERSION   [JYTHON]" +
                lineSep + lineSep +
                "HELP";

            // Register Help Data

            registerHelpData(
                ErrorSubmittingExecuteRequest,
                "Error submitting execute request",
                "Additional information about the error is put into the " +
                "STAF Result.");

            registerHelpData(
                BlockNotHeld,
                "Block not held",
                "Requested to release a block that is not held.");

            registerHelpData(
                BlockAlreadyHeld,
                "Block already held",
                "Requested to hold a block that is already held.");

            registerHelpData(
                JobNotComplete,
                "Job not complete",
                "Requested to get results for a job that is still running.");

            registerHelpData(
                BreakpointBlockHeld,
                "Unable to step/resume breakpoint - block held",
                "Requested to step/resume a breakpoint whose block " +
                "is currently held.");

            // Parse a simple STAX job to verify that the JVM used by the STAX
            // service can be used to parse a STAX job (verifies it works with
            // the Xerces parser and Xalan)

            (new STAXParser(this)).parse(XML_DATA, INLINE_DATA, "local");
        }
        catch (STAXException ex)
        {
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  ex.toString());
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  e.toString());
        }
        catch (Throwable t)
        {
            rc = STAFResult.ServiceConfigurationError;
            t.printStackTrace();
            return new STAFResult(rc, t.toString());
        }

        return new STAFResult(rc);
    }

    public String getDTD() { return fDTD; }

    public String getServiceName() { return fServiceName; }

    public String getEventServiceMachine() { return fEventServiceMachine; }

    public String getEventServiceName() { return fEventServiceName; }

    public String getLocalMachineName() { return fLocalMachineName; }

    public String getLocalMachineNickname() { return fLocalMachineNickname; }

    public String getDataDir() { return fDataDir; }

    public String getInstanceUUID() { return fInstanceUUID; }

    public int getProcessTimeout() { return fProcessTimeout; }

    public long getMaxReturnFileSize() { return fMaxReturnFileSize; }

    public int getMaxGetQueueMessages() { return fMaxGetQueueMessages; }

    public int getMaxSTAXThreads() { return fMaxSTAXThreads; }

    public boolean getClearlogs() { return fClearLogs; }

    public String getClearLogsAsString()
    {
        if (fClearLogs)
            return "Enabled";
        else
            return "Disabled";
    }

    public boolean getLogTCElapsedTime() { return fLogTCElapsedTime; }

    public String getLogTCElapsedTimeAsString()
    {
        if (fLogTCElapsedTime)
            return "Enabled";
        else
            return "Disabled";
    }

    public boolean getLogTCNumStarts() { return fLogTCNumStarts; }

    public String getLogTCNumStartsAsString()
    {
        if (fLogTCNumStarts)
            return "Enabled";
        else
            return "Disabled";
    }

    public boolean getLogTCStartStop() { return fLogTCStartStop; }
 
    public String getLogTCStartStopAsString()
    {
        if (fLogTCStartStop)
            return "Enabled";
        else
            return "Disabled";
    }
    
    public int getPythonOutput() { return fPythonOutput; }

    public String getPythonLogLevel() { return fPythonLogLevel; }

    public boolean getFileCaching()
    {
        return fFileCaching;
    }
    
    public String getFileCachingAsString()
    {
        if (fFileCaching)
            return "Enabled";
        else
            return "Disabled";
    }

    public int getNextJobNumber()
    {
        synchronized (fNextJobNumberSynch)
        {
            // If the job number has reached its maximum value, reset it so
            // that the next job number will be 1 (instead of -2147483648)

            if (fNextJobNumber == Integer.MAX_VALUE)
                fNextJobNumber = 1;

            return fNextJobNumber++;
        }
    }

    public STAFHandle getSTAFHandle() { return fHandle; }

    public STAXActionFactory getActionFactory(String elementName)
    {
        return (STAXActionFactory)fActionFactoryMap.get(elementName);
    }

    public TreeMap getJobMap()
    {
        synchronized(this)
        {
            return fJobMap;
        }
    }

    public STAXJob removeFromJobMap(Integer jobID)
    {
        synchronized (fJobMap)
        {
            return (STAXJob)fJobMap.remove(jobID);
        }
    }

    public void setExtension(STAXExtension ext)
    {
        synchronized(fExtensionsJarList)
        {
            fExtensionsJarList.add(ext);
        }
    }

    public STAXThreadQueue getThreadQueue() { return fThreadQueue; }

    public STAXTimedEventQueue getTimedEventQueue() { return fTimedEventQueue; }

    public String getResultFileName(int jobID)
    {
        return this.getDataDir() + this.fileSep + "job" + this.fileSep +
            "Job" + jobID + this.fileSep + "marshalledResult.txt";
    }

    public String getDetailedResultFileName(int jobID)
    {
        return this.getDataDir() + this.fileSep + "job" + this.fileSep +
            "Job" + jobID + this.fileSep + "marshalledDetailedResult.txt";
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

            if (actionLC.equals("execute"))
                return handleExecute(info, null);
            else if (actionLC.equals("list"))
                return handleList(info);
            else if (actionLC.equals("query"))
                return handleQuery(info);
            else if (actionLC.equals("get"))
                return handleGet(info);
            else if (actionLC.equals("set"))
                return handleSet(info);
            else if (actionLC.equals("notify"))
                return handleNotify(info);
            else if (actionLC.equals("purge"))
                return handlePurge(info);
            else if (actionLC.startsWith("stop"))
                return handleStop(info);
            else if (actionLC.startsWith("pyexec"))
                return handlePyExec(info);
            else if (actionLC.equals("version"))
                return handleVersion(info);
            else if (actionLC.equals("help"))
                return handleHelp(info);
            else
                return handleGenericRequest(info);
        }
        catch (Throwable t)
        {
            // Write the Java stack trace to the JVM log for the service
            
            STAXTimestamp currTimestamp = new STAXTimestamp();

            System.out.println(
                currTimestamp.getTimestampString() + " ERROR: Exception on " +
                fServiceName + " service request: " +
                lineSep + lineSep + info.request + lineSep);

            t.printStackTrace();
            
            // And also return the Java stack trace in the result

            StringWriter sr = new StringWriter();
            t.printStackTrace(new PrintWriter(sr));

            if (t.getMessage() != null)
            {
                return new STAFResult(
                    STAFResult.JavaError,
                    t.getMessage() + lineSep + sr.toString());
            }
            else
            {
                return new STAFResult(
                    STAFResult.JavaError, sr.toString());
            }
        }
    }

    public STAFResult term()
    {
        // Issue a STAX TERMINATE JOB request for each job still running

        synchronized (fJobMap)
        {
            Iterator iter = fJobMap.values().iterator();

            while (iter.hasNext())
            {
                STAXJob thisJob = (STAXJob)iter.next();

                STAFResult result = acceptRequest(
                    new STAFServiceInterfaceLevel30.RequestInfo(
                    "",  // XXX: Default UUID to what?
                    fLocalMachineName, fLocalMachineName,
                    "STAF/Service/" + fServiceName, 1, 5, true,
                    0,  // XXX: diagEnable flag is not available
                    "TERMINATE JOB " + thisJob.getJobNumber(),
                    0, // XXX: requestNumber is not available
                    "none://anonymous",// Default user
                    "local://local",   // Default endpoint to local://local
                    "local"            // Default physicalInterfaceID to local
                    ));

                if (result.rc != 0)
                {
                    System.out.println("Error terminating STAX Job " +
                        thisJob.getJobNumber() + ".  RC=" + result.rc +
                        ", result=" + result.result);
                }
            }
        }

        // Wait until all jobs are terminated or maximum wait time exceeded

        int maxLoops = 60;  // 60 seconds is the maximum wait time

        for (int i = 0; fJobMap.size() != 0 && i < maxLoops; i++)
        {
            try
            {
                Thread.sleep(1000);   // Sleep 1 second
            }
            catch (InterruptedException ex)
            { }
        }

        // Un-register Help Data

        unregisterHelpData(ErrorSubmittingExecuteRequest);
        unregisterHelpData(BlockNotHeld);
        unregisterHelpData(BlockAlreadyHeld);
        unregisterHelpData(JobNotComplete);
        unregisterHelpData(BreakpointBlockHeld);

        // Un-register the service handle

        try
        {
            fHandle.unRegister();
        }
        catch (STAFException ex)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  ex.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult handleExecute(STAFServiceInterfaceLevel30.RequestInfo info,
                                     STAXJobCompleteListener listener)
    {
        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, fServiceName, "EXECUTE", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        STAXSubmitResult res = execute(info, listener);

        // If the job is in a pending state, need to do clean-up activities
        // like removing the job from the job map, and logging a stop record
        // in the job log and service log, and generating a job stop event

        if ((res.getJob() != null) &&
            (res.getJob().getState() == STAXJob.PENDING_STATE))
        {
            STAFResult result = new STAFResult(
                res.getResult().rc, res.getResult().result);

            res.getJob().cleanupPendingJob(res.getResult());
            return result;
        }
        else
        {
            return res.getResult();
        }
    }

    public STAXSubmitResult execute(STAFServiceInterfaceLevel30.RequestInfo info,
                                    STAXJobCompleteListener listener)
    {
        STAXParser parser = null;
        STAXJob job = null;
        STAFResult resolvedValue;
        String fileName = "";
        String scriptMachName = "";
        STAXJobCompleteNotifiee notifiee = null;

        // Parse the request and return an error if the request is not valid

        STAFCommandParseResult parseResult= fExecuteParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAXSubmitResult(new STAFResult(
                STAFResult.InvalidRequestString, parseResult.errorBuffer));
        }

        try
        {
            // Before assigning a new job number and adding to the list of
            // running STAX jobs, perform validation of the specified options
            // on the STAX EXECUTE request

            job = new STAXJob(this);
            job.setSourceMachine(info.endpoint);
            job.setSourceHandleName(info.handleName);
            job.setSourceHandle(info.handle);

            // Set the file name

            if (parseResult.optionTimes("FILE") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("FILE"),
                    fHandle, info.requestNumber);
                
                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);
                
                fileName = resolvedValue.result;
            }
            else if (parseResult.optionTimes("DATA") > 0)
            {
                // Parse an XML document provided in a String

                fileName = INLINE_DATA;
            }

            job.setXmlFile(fileName);

            // Set the machine name for where the xml file/data resides

            String machName;

            if (parseResult.optionTimes("MACHINE") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("MACHINE"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                machName = resolvedValue.result;
            }
            else
                machName = info.endpoint;

            job.setXmlMachine(machName);
            
            // Set the job name if one is specified

            if (parseResult.optionTimes("JOBNAME") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("JOBNAME"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                job.setJobName(resolvedValue.result);
            }
            
            // Assign the starting function name if specified in the request

            if (parseResult.optionTimes("FUNCTION") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("FUNCTION"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);
                
                job.setStartFunctionOverride(resolvedValue.result);
            }

            // Assign the arguments for the starting function if specified

            if (parseResult.optionTimes("ARGS") > 0)
            {
                // Not resolving STAF variables for ARGS to support Python
                // dictionaries which use { } symbols as well.

                // Parse and compile starting function's arguments
                
                job.setStartFuncArgsOverride(
                    STAXUtil.parseAndCompileForPython(
                        parseResult.optionValue("ARGS"),
                        "\nInvalid value in the \"ARGS\" option."));
            }

            // Check if the HOLD option is specified

            if (parseResult.optionTimes("HOLD") > 0)
            {
                job.setExecuteAndHold();
            }

            // Set the script file machine (if SCRIPTFILEMACHINE or SCRIPTILFE
            // options are specified in the request

            if (parseResult.optionTimes("SCRIPTFILEMACHINE") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("SCRIPTFILEMACHINE"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                scriptMachName = resolvedValue.result;
                job.setScriptFileMachine(resolvedValue.result);
            }
            else if (parseResult.optionTimes("SCRIPTFILE") > 0)
            {
                scriptMachName = machName;
                job.setScriptFileMachine(machName);
            }

            // Set the script files (if any SCRIPTFILE option(s) are specified
            // in the request

            for (int i = 1; i <= parseResult.optionTimes("SCRIPTFILE"); i++)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("SCRIPTFILE", i),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                job.setScriptFile(resolvedValue.result);
            }

            // Set the Clear Logs flag

            if (parseResult.optionTimes("CLEARLOGS") > 0)
            {
                // Resolve the value specified for CLEARLOGS
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("CLEARLOGS"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                if (resolvedValue.result.equalsIgnoreCase("ENABLED") ||
                    resolvedValue.result.equals(""))
                {
                    job.setClearlogs(true);
                }
                else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                {
                    job.setClearlogs(false);
                }
                else
                {
                    return new STAXSubmitResult(new STAFResult(
                        STAFResult.InvalidValue,
                        "CLEARLOGS value must be ENABLED or DISABLED.  " +
                        "Invalid value: " + resolvedValue.result));
                }
            }

            // Handle LOGTCELAPSEDTIME setting

            if (parseResult.optionTimes("LOGTCELAPSEDTIME") > 0)
            {
                // Resolve the value specified for LOGTCELAPSEDTIME
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("LOGTCELAPSEDTIME"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                {
                    job.setLogTCElapsedTime(true);
                }
                else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                {
                    job.setLogTCElapsedTime(false);
                }
                else
                {
                    return new STAXSubmitResult(new STAFResult(
                        STAFResult.InvalidValue,
                        "LOGTCELAPSEDTIME value must be ENABLED or DISABLED.  " +
                        "Invalid value: " + resolvedValue.result));
                }
            }

            // Handle LOGTCNUMSTARTS setting

            if (parseResult.optionTimes("LOGTCNUMSTARTS") > 0)
            {
                // Resolve the value specified for LOGTCNUMSTARTS
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("LOGTCNUMSTARTS"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                {
                    job.setLogTCNumStarts(true);
                }
                else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                {
                    job.setLogTCNumStarts(false);
                }
                else
                {
                    return new STAXSubmitResult(new STAFResult(
                        STAFResult.InvalidValue,
                        "LOGTCNUMSTARTS value must be ENABLED or DISABLED.  " +
                        "Invalid value: " + resolvedValue.result));
                }
            }

            // Handle LOGTCSTARTSTOP setting

            if (parseResult.optionTimes("LOGTCSTARTSTOP") > 0)
            {
                // Resolve the value specified for LOGTCSTARTSTOP
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("LOGTCSTARTSTOP"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                {
                    job.setLogTCStartStop(true);
                }
                else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                {
                    job.setLogTCStartStop(false);
                }
                else
                {
                    return new STAXSubmitResult(new STAFResult(
                        STAFResult.InvalidValue,
                        "LOGTCSTARTSTOP value must be ENABLED or DISABLED.  " +
                        "Invalid value: " + resolvedValue.result));
                }
            }

            // Handle PYTHONOUTPUT setting

            if (parseResult.optionTimes("PYTHONOUTPUT") > 0)
            {
                // Resolve the value specified for PYTHONOUTPUT
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("PYTHONOUTPUT"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                STAFResult result = STAXPythonOutput.isValidPythonOutput(
                    resolvedValue.result);

                if (result.rc != STAFResult.Ok)
                    return new STAXSubmitResult(result);

                job.setPythonOutput(Integer.parseInt(result.result));
            }

            // Handle PYTHONLOGLEVEL setting

            if (parseResult.optionTimes("PYTHONLOGLEVEL") > 0)
            {
                // Resolve the value specified for PYTHONLOGLEVEL
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("PYTHONLOGLEVEL"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue);

                STAFResult result = STAXPythonOutput.isValidLogLevel(
                    resolvedValue.result);

                if (result.rc != STAFResult.Ok)
                    return new STAXSubmitResult(result);

                job.setPythonLogLevel(result.result);
            }

            // Set the wait timeout for the job if specified in the request

            if (parseResult.optionTimes("WAIT") > 0)
            {
                String waitString = parseResult.optionValue("WAIT");

                if (waitString.length() == 0)
                {
                    job.setWaitTimeout("");
                }
                else
                {
                    // Resolve the WAIT value, verify that it is a valid
                    // duration timeout value, and convert it to milliseconds
                    // if needed

                    resolvedValue =
                        STAFUtil.resolveRequestVarAndConvertDuration(
                            "WAIT", waitString, fHandle, info.requestNumber);

                    if (resolvedValue.rc != STAFResult.Ok)
                        return new STAXSubmitResult(resolvedValue);
                    
                    job.setWaitTimeout(resolvedValue.result);
                }
            }

            // Check if NOTIFY ONEND was specified
            
            if (parseResult.optionTimes("NOTIFY") > 0)
            {
                if (parseResult.optionTimes("BYNAME") > 0)
                    job.setNotifyOnEnd(STAXJob.NOTIFY_ONEND_BY_NAME);
                else
                    job.setNotifyOnEnd(STAXJob.NOTIFY_ONEND_BY_HANDLE);

                int priority = 5;  // Default value for priority

                if (parseResult.optionTimes("PRIORITY") > 0)
                {
                    // Resolve the value specified for PRIORITY and make sure
                    // the value specified is an integer

                    resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                        "PRIORITY", parseResult.optionValue("PRIORITY"),
                        fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0)
                    {
                        return new STAXSubmitResult(new STAFResult(
                            resolvedValue.rc, resolvedValue.result));
                    }

                    priority = Integer.parseInt(resolvedValue.result);
                }

                String key = null;

                if (parseResult.optionTimes("KEY") > 0)
                {
                    // Resolve the value specified for KEY

                    resolvedValue = STAFUtil.resolveRequestVar(
                        parseResult.optionValue("KEY"),
                        fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0)
                    {
                        return new STAXSubmitResult(new STAFResult(
                            resolvedValue.rc, resolvedValue.result));
                    }

                    key = resolvedValue.result;
                }

                notifiee = new STAXJobCompleteNotifiee(
                    job.getNotifyOnEnd(), info.endpoint,
                    info.handle, info.handleName, priority, key);
            }

            // Assign a job number and add the job to the job map so that
            // it shows up in a LIST JOBS request and can be queryied

            job.setJobNumber(this.getNextJobNumber());

            synchronized (fJobMap)
            {
                fJobMap.put(job.getJobNumberAsInteger(), job);
            }
        }
        catch (Exception e)
        {
            return new STAXSubmitResult(
                new STAFResult(STAX.ErrorSubmittingExecuteRequest,
                               "Caught " + e.getClass().getName() + ": " +
                               e.getMessage()));
        }

        try
        {
            // Now that the job has been assigned a job number and is in the
            // job map, the job is considered to be started.  So, from now on
            // when a new STAXSubmitResult is returned, we need to pass it the
            // job so that we'll be able to clean-up pending jobs.

            // Clear the STAX Job Logs if the ClearLogs flag is enabled (this
            // needs to be done before the job start message is logged) and
            // assign the name of the Job Data Directory and clear it

            if (job.getClearlogs()) job.clearLogs();

            job.setJobDataDir(getDataDir() + STAX.fileSep + "job" +
                              STAX.fileSep + "Job" + job.getJobNumber());
            job.clearJobDataDir();

            // Set the job's timestamp

            job.setStartTimestamp();
            
            // Log a "start" job message in the Service and Job logs

            String msg = "JobID: " + job.getJobNumber() + 
                         ", File: " + job.getXmlFile() +
                         ", Machine: " + job.getXmlMachine() + 
                         ", Function: " + job.getStartFunctionOverride() +
                         ", Args: " + job.getStartFuncArgsOverride() +
                         ", JobName: ";

            if (job.getJobName().equals(""))
                msg += "<N/A>";
            else
                msg += job.getJobName();

            Iterator it = job.getScriptFiles().iterator();

            while (it.hasNext())
                msg += ", ScriptFile: " + (String)it.next();

            if (! job.getScriptFileMachine().equals(""))
                msg += ", ScriptFileMachine: " + job.getScriptFileMachine();

            it = job.getScripts().iterator();
            while (it.hasNext())
                msg += ", Script: " + (String)it.next();

            job.log(STAXJob.SERVICE_LOG, "start", msg);
            job.log(STAXJob.JOB_LOG, "start", msg);
            
            // Generate the job begin event

            HashMap jobStartMap = new HashMap();
            jobStartMap.put("type", "job");
            jobStartMap.put("block", "main");
            jobStartMap.put("status", "begin");
            jobStartMap.put("jobID", String.valueOf(job.getJobNumber()));
            jobStartMap.put("startFunction", job.getStartFunctionOverride());
            jobStartMap.put("jobName", job.getJobName());
            jobStartMap.put("startTimestamp",
                            job.getStartTimestamp().getTimestampString());

            job.generateEvent(STAXJob.STAX_JOB_EVENT, jobStartMap, true);

            // Check if the file is cached.  If not, get the file contents and
            // perform xml parsing of the data in the file or inline data.

            parser = new STAXParser(this);

            if (parseResult.optionTimes("FILE") > 0)
            {
                // Parse an XML document from a file.

                // Get the file separator for the machine where the xml file
                // resides as this is needed to normalize the path name

                STAFResult result = STAXFileCache.getFileSep(
                    job.getXmlMachine(), fHandle);

                if (result.rc != STAFResult.Ok)
                {
                    // VAR RESOLVE request failed on the remote machine

                    return new STAXSubmitResult(new STAFResult(
                        result.rc, "Error submitting a STAF request to " +
                        " machine " + job.getXmlMachine() + " to get file " +
                        fileName + ", RC: " + result.rc + ", Result: " +
                        result.result), job);
                }

                String fileSep = result.result;
                
                // Set a flag to indicate if the file name is case-sensitive
                // (e.g. true if the file resides on a Unix machine, false if
                // Windows)

                boolean caseSensitiveFileName = true;

                if (fileSep.equals("\\"))
                {
                    // File resides on a Windows machine so file name is
                    // not case-sensitive

                    caseSensitiveFileName = false;
                }
                
                // Normalize the xml file name so that we have a better chance
                // at matching file names that are already cached)

                fileName = STAXUtil.normalizeFilePath(fileName, fileSep);
                
                job.setXmlFile(fileName);

                // If caching is enabled, check if the same file is already
                // in the file cache

                boolean cacheHit = false;
                Date dLastModified = null;
                
                if (fFileCaching)
                {
                    // Get the last modification date of the file

                    if (STAXFileCache.get().isLocalMachine(job.getXmlMachine()))
                    {
                        // The machine is local, so can use Java to find the
                        // modification date

                        File file = new File(fileName);
                        
                        if (file.exists())
                        {
                            long lastModified = file.lastModified();

                            if (lastModified > 0)
                            {
                                // Chop off the milliseconds because some
                                // systems don't report modification time
                                // with milliseconds precision

                                lastModified = ((long)(lastModified/1000))*1000;
                                
                                dLastModified = new Date(lastModified);
                            }
                        }
                    }
                    
                    if (dLastModified == null)
                    {
                        // Find the remote file mod time using STAF

                        STAFResult entryResult = fHandle.submit2(
                            job.getXmlMachine(), "FS",
                            "GET ENTRY " + fileName + " MODTIME");

                        if (entryResult.rc == 0)
                        {
                            String modDate = entryResult.result;
                            dLastModified = STAXFileCache.convertSTAXDate(
                                modDate);
                        }
                    }
                
                    // Check for an up-to-date file in the cache

                    if ((dLastModified != null) &&
                        STAXFileCache.get().checkCache(
                            job.getXmlMachine(), fileName, dLastModified,
                            caseSensitiveFileName))
                    {
                        // Get the document from the cache

                        STAXDocument doc = STAXFileCache.get().getDocument(
                            job.getXmlMachine(), fileName,
                            caseSensitiveFileName);

                        if (doc != null)
                        {
                            job.setSTAXDocument(doc);
                            cacheHit = true;
                        }
                    }
                }

                // If the doc was not loaded from cache, load from the machine

                if (!cacheHit)
                {
                    STAFResult copyResult = fHandle.submit2(
                        job.getXmlMachine(), "FS", "GET FILE " +
                        STAFUtil.wrapData(fileName));
    
                    if (copyResult.rc != 0)
                    {
                        // There was an error in the FS service submission
                        return new STAXSubmitResult(new STAFResult(
                            copyResult.rc, "Error getting XML file " +
                            fileName + " from machine " + job.getXmlMachine() +
                            "\n\n" + copyResult.result), job);
                    }
    
                    // Parse the XML document

                    job = parser.parse(
                        copyResult.result, fileName, job.getXmlMachine(), job);
                    
                    // Add the XML document to the cache

                    if (fFileCaching && (dLastModified != null))
                    {
                        STAXFileCache.get().addDocument(
                            job.getXmlMachine(), fileName,
                            job.getSTAXDocument(), dLastModified,
                            caseSensitiveFileName);
                    }
                }
            }
            else if (parseResult.optionTimes("DATA") > 0)
            {
                // Parse an XML document provided in a String

                job = parser.parse(parseResult.optionValue("DATA"),
                                   fileName, job.getXmlMachine(), job);
            }

            // Recursively process any function-import elements for all the
            // functions added to the job's function map

            Iterator functionIter = job.getSTAXDocument().getFunctionMap().
                values().iterator();

            while (functionIter.hasNext())
            {
                job.addImportedFunctions(
                    (STAXFunctionAction)functionIter.next());
            }

            if (parseResult.optionTimes("FUNCTION") > 0)
            {
                // Need to set start function for the job's document AFTER
                // xml parsing is done in order to override the default
                // function if specified in a defaultcall element

                job.setStartFunction(job.getStartFunctionOverride());

                // Set to null since overriding the starting function
                job.setStartFuncArgs(null);

                // Make sure that the start function specified is valid

                if (!job.functionExists(job.getStartFunction()))
                {
                    throw new STAXInvalidStartFunctionException(
                        "'" + job.getStartFunction() +
                        "' is not a valid function name.  No function with " +
                        "this name is defined.");
                }
            }

            if (parseResult.optionTimes("ARGS") > 0)
            {
                // Need to set function arguments for the job's document AFTER
                // xml parsing is done in order to override the default
                // function arguments if specified in a defaultcall element
                
                job.setStartFuncArgs(job.getStartFuncArgsOverride());
            }

            // Add Python code specified by any SCRIPTFILE options to the job's
            // default action list

            if (job.getScriptFiles().size() > 0)
            {
                // For any SCRIPTFILE options specified, get the contents of
                // the script file and perform compile the Python code in the
                // script file to make sure there are no Python compiler
                // errors and create a STAXScriptAction and add it as a
                // default action for the job

                Iterator iter = job.getScriptFiles().iterator();

                while (iter.hasNext())
                {
                    String scriptFile = (String)iter.next();

                    STAFResult getResult = fHandle.submit2(
                        scriptMachName, "FS", "GET FILE " +
                        STAFUtil.wrapData(scriptFile));

                    if (getResult.rc != 0)
                    {
                        // There was an error in the FS service submission
                        return new STAXSubmitResult(new STAFResult(
                            getResult.rc, "Error getting Script file " +
                            scriptFile + " from machine " +
                            scriptMachName + "\n\n" + getResult.result), job);
                    }

                    // Parse and compile SCRIPTFILE contents containing Python code

                    STAXScriptAction scriptAction = new STAXScriptAction();
                    scriptAction.setLineNumber(
                        "script", "<Input via SCRIPTFILE option>");
                    scriptAction.setXmlFile(scriptFile);
                    scriptAction.setXmlMachine(scriptMachName);

                    scriptAction.setElementInfo(new STAXElementInfo(
                        "script", STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "The file containing this Python code was specified " +
                        "via a SCRIPTFILE option."));

                    scriptAction.setValue(STAXUtil.parseAndCompileForPython(
                        getResult.result, scriptAction));
                    job.addDefaultAction(scriptAction);
                }
            }
            
            // For any SCRIPT options specified, compile its Python code to
            // make sure there are no Python compiler errors and create a
            // STAXScriptAction and add it as a default action for the job

            for (int i = 1; i <= parseResult.optionTimes("SCRIPT"); i++)
            {
                // Parse and compile SCRIPT value containing Python code

                STAXScriptAction scriptAction = new STAXScriptAction();
                scriptAction.setLineNumber(
                    "script", "<Input via SCRIPT option>");
                scriptAction.setXmlFile("<None>");
                scriptAction.setXmlMachine("<None>");

                scriptAction.setElementInfo(new STAXElementInfo(
                    "script", STAXElementInfo.NO_ATTRIBUTE_NAME,
                    "This Python code was specified via SCRIPT option #" +
                    i + "."));

                String code = STAXUtil.parseAndCompileForPython(
                    parseResult.optionValue("SCRIPT", i), scriptAction);

                scriptAction.setValue(code);
                job.addDefaultAction(scriptAction);

                // Store the scripts with private data masked since only used
                // for display by a QUERY JOB <JobID> or GET RESULT <JobID>
                // request
                job.setScript(STAFUtil.maskPrivateData(code));
            }

            // For any BREAKPOINT options specified, add breakpoints for the
            // job

            for (int i = 1;
                 i <= parseResult.optionTimes("BREAKPOINT"); i++)
            {
                boolean isLineBreakpoint = false;

                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("BREAKPOINT", i),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0)
                    return new STAXSubmitResult(resolvedValue, job);

                String breakpointValue = resolvedValue.result;

                if (breakpointValue.indexOf("@@") > -1)
                {
                    isLineBreakpoint = true;
                }
                else
                {
                    try
                    {
                        Integer.parseInt(breakpointValue);

                        isLineBreakpoint = true;
                    }
                    catch (NumberFormatException e)
                    {
                        isLineBreakpoint = false;
                    }
                }

                if (!isLineBreakpoint)
                {
                    job.addBreakpointFunction(breakpointValue);
                }
                else
                {
                    String line = "";
                    String file = fileName;  // Default is EXECUTE FILE
                    String machine = "";

                    if (breakpointValue.indexOf("@@") == -1)
                    {
                        line = breakpointValue;
                    }
                    else
                    {
                        int firstSepIndex = breakpointValue.indexOf("@@");

                        line = breakpointValue.substring(0, firstSepIndex);

                        int secondSepIndex =
                            breakpointValue.indexOf("@@", firstSepIndex + 2);

                        if (secondSepIndex == -1)
                        {
                            file = breakpointValue.substring(
                                firstSepIndex + 2);

                        }
                        else
                        {
                            file = breakpointValue.substring(
                                firstSepIndex + 2, secondSepIndex);

                            machine = breakpointValue.substring(
                                secondSepIndex + 2);
                        }
                    }

                    String breakpointMachineName = machine;

                    if (machine.equals(""))
                    {
                        breakpointMachineName = info.endpoint;
                    }

                    // Get the file separator for the breakpoint machine

                    String fileSep = "/";

                    STAFResult result = STAXFileCache.getFileSep(
                        breakpointMachineName, fHandle);

                    if (result.rc == STAFResult.Ok)
                        fileSep = result.result;

                    file = STAXUtil.normalizeFilePath(file, fileSep);

                    job.addBreakpointLine(line, file, machine);
                }
            }

            // Handle the TEST option

            if (parseResult.optionTimes("TEST") > 0)
            {
                if (parseResult.optionTimes("RETURNDETAILS") > 0)
                {
                    // Get a list of functions to return

                    HashMap functionMap = job.getSTAXDocument().getFunctionMap();

                    java.util.Collection functionColl = functionMap.keySet();
                    TreeSet sortedFunctions = new TreeSet(functionColl);
                    Iterator iter = sortedFunctions.iterator();

                    // Create the marshalling context and set the map class
                    // definitions

                    STAFMarshallingContext mc = new STAFMarshallingContext();
                    mc.setMapClassDefinition(fJobDetailsMapClass);
                    mc.setMapClassDefinition(fFunctionInfoMapClass);
                    mc.setMapClassDefinition(fArgInfoMapClass);
                    mc.setMapClassDefinition(fArgPropertyInfoMapClass);
                    mc.setMapClassDefinition(fArgPropertyDataInfoMapClass);
                    List functionList = new ArrayList();

                    Map jobDetailsMap = fJobDetailsMapClass.createInstance();

                    jobDetailsMap.put("jobID",
                                      Integer.toString(job.getJobNumber()));

                    jobDetailsMap.put("defaultCall", job.getStartFunction());

                    while (iter.hasNext())
                    {
                        String functionName = (String)iter.next();

                        STAXFunctionAction function =
                            (STAXFunctionAction)functionMap.get(functionName);

                        functionList.add(function.getArgumentInfo(
                            fFunctionInfoMapClass, fArgInfoMapClass,
                            fArgPropertyInfoMapClass,
                            fArgPropertyDataInfoMapClass));
                    }

                    jobDetailsMap.put("functionList", functionList);

                    mc.setRootObject(jobDetailsMap);

                    return new STAXSubmitResult(
                            new STAFResult(STAFResult.Ok, mc.marshall()), job);
                }

                return new STAXSubmitResult(
                    new STAFResult(
                        STAFResult.Ok, String.valueOf(job.getJobNumber())),
                    job);
            }
            
            // Make sure that the start function is valid

            if (!job.functionExists(job.getStartFunction()))
            {
                throw new STAXInvalidStartFunctionException(
                    "'" + job.getStartFunction() +
                    "' is not a valid function name.  No function with " +
                    "this name is defined.");
            }

            // Create a new handle for the job

            job.setSTAFHandle();

            // Adds the STAX service as a notifiee for when the job completes

            job.addCompletionNotifiee(this);

            // If this is a sub-job (indicated by a non-null listener), make
            // sure the sub-job is also notified when the job completes.
            if (listener != null)
            {
                job.addCompletionNotifiee(listener);
            }

            // If NOTIFY ONEND was specified, make sure the originator of the
            // STAX EXECUTE request is also notified when the job completes.

            if (notifiee != null)
            {
                job.addCompletionNotifiee2(notifiee);
            }

            // If WAIT was specified, add the job ID to the execute wait list

            if (parseResult.optionTimes("WAIT") > 0)
            {
                synchronized (fExecuteWaitList)
                {
                    fExecuteWaitList.add(Integer.toString(job.getJobNumber()));
                }
            }

            if (parseResult.optionTimes("BREAKPOINTFIRSTFUNCTION") > 0)
            {
                job.setBreakpointFirstFunction(true);
            }

            if (parseResult.optionTimes("BREAKPOINTSUBJOBFIRSTFUNCTION") > 0)
            {
                job.setBreakpointSubjobFirstFunction(true);
            }

            job.startExecution();
        }
        catch (STAFException e)
        {
            removeJobFromExecuteWaitList(job);

            return new STAXSubmitResult(
                new STAFResult(STAX.ErrorSubmittingExecuteRequest,
                               "Caught " + e.getClass().getName() + ": " +
                               e.getMessage()), job);
        }
        catch (STAXException e)
        {
            removeJobFromExecuteWaitList(job);

            return new STAXSubmitResult(
                new STAFResult(STAX.ErrorSubmittingExecuteRequest,
                               "Caught " + e.getClass().getName() + ": " +
                               e.getMessage()), job);
        }
        catch (SAXNotRecognizedException e)
        {
            removeJobFromExecuteWaitList(job);

            return new STAXSubmitResult(
                new STAFResult(STAX.ErrorSubmittingExecuteRequest,
                               "Caught SAXNRE: " + e.getMessage()), job);
        }
        catch (SAXNotSupportedException e)
        {
            removeJobFromExecuteWaitList(job);

            return new STAXSubmitResult(
                new STAFResult(STAX.ErrorSubmittingExecuteRequest,
                               "Caught SAXNSE: " + e.getMessage()), job);
        }
        catch (Exception e)
        {
            removeJobFromExecuteWaitList(job);

            e.printStackTrace();

            return new STAXSubmitResult(
                new STAFResult(STAX.ErrorSubmittingExecuteRequest,
                               "Caught Exception: " +
                               e.getClass().getName() + ": " +
                               e.getMessage()), job);
        }

        String jobNumber = Integer.toString(job.getJobNumber());

        if (parseResult.optionTimes("WAIT") > 0)
        {
            STAFResult getResult = fHandle.submit2(
                "local", "QUEUE", "GET WAIT " + job.getWaitTimeout() +
                " TYPE " + sQueueTypeJobWaitComplete + jobNumber);

            if (getResult.rc != 0)
            {
                // There was an error in the QUEUE service submission

                if (getResult.rc == STAFResult.Timeout)
                {
                    return new STAXSubmitResult(
                        new STAFResult(getResult.rc, jobNumber), job);
                }
                else
                {
                    return new STAXSubmitResult(
                        new STAFResult(getResult.rc, getResult.result), job);
                }
            }

            if (parseResult.optionTimes("RETURNRESULT") > 0)
            {
                // Return information about the completed job

                // Create the marshalling context and set the map class defs

                STAFMarshallingContext mc = new STAFMarshallingContext();

                boolean details = false;

                if (parseResult.optionTimes("DETAILS") > 0)
                    details = true;

                Map jobResultMap;
                 
                if (!details)
                {
                    mc.setMapClassDefinition(fJobResultMapClass);
                    mc.setMapClassDefinition(
                        STAXTestcaseActionFactory.fTestcaseTotalsMapClass);

                    jobResultMap = fJobResultMapClass.createInstance();
                }
                else
                {
                    mc.setMapClassDefinition(fJobDetailedResultMapClass);
                    mc.setMapClassDefinition(
                        STAXTestcaseActionFactory.fTestcaseTotalsMapClass);
                    mc.setMapClassDefinition(
                        STAXTestcaseActionFactory.fQueryTestcaseMapClass);

                    jobResultMap = fJobDetailedResultMapClass.createInstance();
                }

                jobResultMap.put("jobID", String.valueOf(job.getJobNumber()));
                jobResultMap.put("startTimestamp", job.getStartTimestamp().
                                 getTimestampString());
                jobResultMap.put("endTimestamp", job.getEndTimestamp().
                                 getTimestampString());

                // XXX: Do we want to be able to return non-string objects?
                jobResultMap.put("result", job.getResult().toString());

                jobResultMap.put("status", job.getCompletionStatusAsString());

                jobResultMap.put("testcaseTotals", job.getTestcaseTotalsMap());

                if (details)
                {
                    jobResultMap.put("testcaseList", job.getTestcaseList());
                }

                jobResultMap.put("jobLogErrors", job.getJobLogErrorsMC());

                mc.setRootObject(jobResultMap);

                return new STAXSubmitResult(
                    new STAFResult(STAFResult.Ok, mc.marshall()), job);
            }
        }

        return new STAXSubmitResult(new STAFResult(STAFResult.Ok, jobNumber),
                                    job);
    }

    private void removeJobFromExecuteWaitList(STAXJob job)
    {
        if ((job != null) && (job.getWaitTimeout() != null) &&
            (job.getJobNumber() != 0))
        {
            String jobNumber = Integer.toString(job.getJobNumber());
            
            // Remove this job number from the execute wait list

            synchronized (fExecuteWaitList)
            {
                fExecuteWaitList.remove(jobNumber);

                if (fExecuteWaitList.contains(jobNumber))
                {
                    fExecuteWaitList.remove(
                        fExecuteWaitList.indexOf(jobNumber));
                }
            }
        }
    }

    private STAFResult handleList(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "LIST", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fListParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Create the marshalling context

        STAFMarshallingContext mc = new STAFMarshallingContext();

        // Provide output based on the option(s) specified

        if (parseResult.optionTimes("JOBS") != 0)
        {
            // Create output for LIST JOBS request

            mc.setMapClassDefinition(fJobInfoMapClass);
            List jobList = new ArrayList();

            synchronized (fJobMap)
            {
                Iterator iter = fJobMap.values().iterator();

                while (iter.hasNext())
                {
                    STAXJob job = (STAXJob)iter.next();

                    Map jobInfoMap = fJobInfoMapClass.createInstance();

                    jobInfoMap.put("jobID",
                                   String.valueOf(job.getJobNumber()));

                    if (!job.getJobName().equals(""))
                        jobInfoMap.put("jobName", job.getJobName());

                    jobInfoMap.put("startTimestamp", job.getStartTimestamp().
                                   getTimestampString());

                    jobInfoMap.put("function", job.getStartFunction());
                    
                    jobInfoMap.put("state", job.getStateAsString());

                    jobList.add(jobInfoMap);
                }
            } // end synchronized (fJobMap)

            mc.setRootObject(jobList);
        }
        else if (parseResult.optionTimes("SETTINGS") != 0)
        {
            // LIST SETTINGS

            mc.setMapClassDefinition(fSettingsMapClass);

            Map settingsMap = fSettingsMapClass.createInstance();
            settingsMap.put("eventMachine", fEventServiceMachine);
            settingsMap.put("eventService", fEventServiceName);
            settingsMap.put("numThreads", String.valueOf(fNumThreads));
            settingsMap.put("processTimeout", String.valueOf(fProcessTimeout));
            settingsMap.put("fileCaching", getFileCachingAsString());
            settingsMap.put("maxFileCacheSize", String.valueOf(
                            STAXFileCache.get().getMaxCacheSize()));
            settingsMap.put("fileCacheAlgorithm",
                            STAXFileCache.get().getAlgorithmString());
            settingsMap.put("maxFileCacheAge", String.valueOf(
                            STAXFileCache.get().getMaxAge()));
            settingsMap.put("maxMachineCacheSize", String.valueOf(
                            STAXMachineCache.get().getMaxCacheSize()));
            settingsMap.put("maxReturnFileSize",
                            String.valueOf(fMaxReturnFileSize));
            settingsMap.put("maxGetQueueMessages",
                            String.valueOf(fMaxGetQueueMessages));
            settingsMap.put("maxSTAXThreads",
                            String.valueOf(fMaxSTAXThreads));
            settingsMap.put("clearLogs", getClearLogsAsString());
            settingsMap.put("logTCElapsedTime", getLogTCElapsedTimeAsString());
            settingsMap.put("logTCNumStarts", getLogTCNumStartsAsString());
            settingsMap.put("logTCStartStop", getLogTCStartStopAsString());
            settingsMap.put("pythonOutput",
                            STAXPythonOutput.getPythonOutputAsString(
                                getPythonOutput()));
            settingsMap.put("pythonLogLevel", getPythonLogLevel());
            settingsMap.put("extensions", fExtensionList);
            settingsMap.put("extensionFile", fExtensionFile);

            mc.setRootObject(settingsMap);
        }
        else if (parseResult.optionTimes("FILECACHE") != 0)
        {
            if (parseResult.optionTimes("SUMMARY") == 0)
            {
                // LIST FILECACHE

                // By default, sort based on the cache alorithm
                int sortBy = STAXFileCache.get().getAlgorithm();

                if (parseResult.optionTimes("SORTBYLRU") != 0)
                    sortBy = STAXFileCache.LRU;
                else if (parseResult.optionTimes("SORTBYLFU") != 0)
                    sortBy = STAXFileCache.LFU;

                mc.setMapClassDefinition(fFileCacheMapClass);
                List fileCacheList = new ArrayList();
                List cacheContents = STAXFileCache.get().getCacheContents(
                    sortBy);
            
                Iterator iter = cacheContents.iterator();
            
                while (iter.hasNext())
                {
                    STAXFileCache.FileCacheEntry item =
                        (STAXFileCache.FileCacheEntry)iter.next();

                    Map fileCacheMap = fFileCacheMapClass.createInstance();

                    fileCacheMap.put("machine", item.getMachine());
                    fileCacheMap.put("file", item.getFilename());
                    fileCacheMap.put("hits", String.valueOf(item.getHits()));
                    fileCacheMap.put("lastHit", DATE_FORMAT.format(
                        item.getLastHitDate()));
                    fileCacheMap.put("addDate", DATE_FORMAT.format(
                        item.getAddDate()));

                    fileCacheList.add(fileCacheMap);
                }
            
                mc.setRootObject(fileCacheList);
            }
            else
            {
                // LIST FILECACHE SUMMARY

                // Provide summary information for the File Cache

                STAXFileCache fileCache = STAXFileCache.get();
                long hitCount = fileCache.getCacheHitCount();
                long missCount = fileCache.getCacheMissCount();

                Map summaryMap = fFileCacheSummaryMapClass.createInstance();

                float hitRatio = 0;
                long requestCount = hitCount + missCount;

                if (requestCount > 0)
                    hitRatio = (float)hitCount / requestCount;

                java.text.NumberFormat nf =
                    java.text.NumberFormat.getPercentInstance();

                summaryMap.put("hitRatio", nf.format(hitRatio));
                summaryMap.put("hitCount", String.valueOf(hitCount));
                summaryMap.put("missCount", String.valueOf(missCount));
                summaryMap.put("requestCount", String.valueOf(requestCount));
                summaryMap.put("lastPurgeDate", DATE_FORMAT.format(
                    fileCache.getLastPurgeDate()));

                mc.setMapClassDefinition(fFileCacheSummaryMapClass);
                mc.setRootObject(summaryMap);
            }
        }
        else if (parseResult.optionTimes("MACHINECACHE") != 0)
        {
            // LIST MACHINECACHE
            
            mc.setMapClassDefinition(fMachineCacheMapClass);

            List machineCacheList = new ArrayList();
            List cacheContents = STAXMachineCache.get().getCacheContents();
            
            // Sort the contents by the last hit date

            Collections.sort(cacheContents, new Comparator() {
                public int compare(Object o1, Object o2)
                {
                    // Sort by newest entry first
                    return ((STAXMachineCache.MachineCacheEntry)o1).
                        getLastHitDate().compareTo(
                            ((STAXMachineCache.MachineCacheEntry)o2).
                            getLastHitDate()) * -1;
                }
            });
            
            Iterator iter = cacheContents.iterator();
            
            while (iter.hasNext())
            {
                STAXMachineCache.MachineCacheEntry item =
                    (STAXMachineCache.MachineCacheEntry)iter.next();

                Map machineCacheMap = fMachineCacheMapClass.createInstance();

                machineCacheMap.put("machine", item.getMachine());
                machineCacheMap.put("fileSep", item.getFileSep());
                machineCacheMap.put("hits", String.valueOf(item.getHits()));
                machineCacheMap.put("lastHit", DATE_FORMAT.format(
                    item.getLastHitDate()));
                machineCacheMap.put("addDate", DATE_FORMAT.format(
                    item.getAddDate()));

                machineCacheList.add(machineCacheMap);
            }
            
            mc.setRootObject(machineCacheList);
        }
        else if (parseResult.optionTimes("EXTENSIONS") != 0)
        {
            // LIST EXTENSIONS

            mc.setMapClassDefinition(fExtensionElementMapClass);

            List extList = new LinkedList();

            Iterator iter = fExtensionElementMap.keySet().iterator();

            while (iter.hasNext())
            {
                String elementName = (String)iter.next();

                Map extMap = fExtensionElementMapClass.createInstance();
                extMap.put("extensionElement", "<" + elementName + ">");
                extMap.put("extensionJarFile",
                           fExtensionElementMap.get(elementName));

                extList.add(extMap);
            }

            mc.setRootObject(extList);
        }
        else if (parseResult.optionTimes("EXTENSIONJARFILES") != 0)
        {
            // LIST EXTENSIONJARFILES

            mc.setMapClassDefinition(fExtensionJarFileMapClass);

            List extJarFileList = new LinkedList();

            synchronized (fExtensionsJarList)
            {
                Iterator iter = fExtensionsJarList.iterator();

                while (iter.hasNext())
                {
                    STAXExtension ext = (STAXExtension)iter.next();

                    Map extMap = fExtensionJarFileMapClass.createInstance();
                    extMap.put("extensionJarFile", ext.getJarFileName());

                    if (!ext.getExtVersion().equals(""))
                        extMap.put("version", ext.getExtVersion());

                    if (!ext.getExtDescription().equals(""))
                        extMap.put("description", ext.getExtDescription());

                    extJarFileList.add(extMap);
                }
            }

            mc.setRootObject(extJarFileList);
        }
        else
        {
            // LIST JOB <Job ID> <THREADS | PROCESSES | STAFCMDS |
            //                    SUBJOBS | BLOCKS | TESTCASES ...>

            STAXJob job = null;

            try
            {
                job = (STAXJob)fJobMap.get(
                       new Integer(parseResult.optionValue("JOB")));

                if (job == null)
                {
                    return new STAFResult(STAFResult.DoesNotExist,
                                          parseResult.optionValue("JOB"));
                }
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(STAFResult.InvalidValue,
                                      parseResult.optionValue("JOB"));
            }

            if (parseResult.optionTimes("THREADS") != 0)
            {
                List threadList = new ArrayList();
                Map jobThreadMap = job.getThreadMapCopy();

                if (jobThreadMap.size() == 0)
                {
                    // No threads are running

                    mc.setRootObject(threadList);
                    return new STAFResult(STAFResult.Ok, mc.marshall());
                }

                boolean longFormat = false;

                if (parseResult.optionTimes("LONG") == 0)
                {
                    mc.setMapClassDefinition(fThreadInfoMapClass);
                }
                else
                {
                    longFormat = true;
                    mc.setMapClassDefinition(fThreadLongInfoMapClass);
                }

                Iterator iter = jobThreadMap.values().iterator();

                while (iter.hasNext())
                {
                    STAXThread thread = (STAXThread)iter.next();

                    String threadID = String.valueOf(thread.getThreadNumber());

                    STAXThread parentThread = thread.getParentThread();
                    String parentThreadID = null;

                    if (parentThread != null)
                    {
                        parentThreadID = String.valueOf(
                            parentThread.getThreadNumber());
                    }

                    String state = thread.getStateAsString();

                    if (!longFormat)
                    {
                        Map threadMap = fThreadInfoMapClass.createInstance();
                        threadMap.put("threadID", threadID);
                        threadMap.put("parentTID", parentThreadID);
                        threadMap.put("state", state);

                        threadList.add(threadMap);
                    }
                    else
                    {
                        Map threadMap =
                            fThreadLongInfoMapClass.createInstance();
                        threadMap.put("threadID", threadID);
                        threadMap.put("parentTID", parentThreadID);

                        if (parentThread != null)
                        {
                            threadMap.put(
                                "parentHierarchy",
                                thread.getParentHierarchy());
                        }

                        threadMap.put("state", state);
                        threadMap.put(
                            "startTimestamp",
                            thread.getStartTimestamp().getTimestampString());
                        threadMap.put("callStack", thread.getCallStack());
                        threadMap.put(
                            "conditionStack", thread.getConditionStack());

                        threadList.add(threadMap);
                    }
                }

                mc.setRootObject(threadList);
            }
            else if (parseResult.optionTimes("VARS") != 0)
            {
                Integer threadID;

                try
                {
                    threadID = new Integer(parseResult.optionValue("THREAD"));
                }
                catch (NumberFormatException e)
                {
                    return new STAFResult(STAFResult.InvalidValue,
                                          parseResult.optionValue("THREAD"));
                }

                String variableName = "";
                String queryType = "LONG";

                if (parseResult.optionTimes("SHORT") != 0)
                {
                    queryType = "SHORT";
                }
                
                STAXThread thread = job.getThread(threadID);

                if (thread == null)
                {
                    return new STAFResult(STAFResult.DoesNotExist,
                                          parseResult.optionValue("THREAD"));
                }

                Map locals = thread.getLocals();
                List variableList;

                if (queryType.equals("SHORT"))
                {
                    variableList = createShortMarshalledList(locals);
                }
                else // LONG
                {
                    variableList = createLongMarshalledList(locals);
                }

                mc.setRootObject(variableList);
                mc.setMapClassDefinition(fThreadVariableMapClass);

                return new STAFResult(STAFResult.Ok, mc.marshall());
            }
            else
            {
                int typeInstanceNum = 0;   // 0 indicates type option not found

                for (int i = 1; i <= 3; i++)
                {
                    if (!parseResult.instanceName(i).equalsIgnoreCase("LIST") &&
                        !parseResult.instanceName(i).equalsIgnoreCase("JOB"))
                    {   // Found the type option
                        typeInstanceNum = i;
                        break;
                    }
                }

                String typeName = parseResult.instanceName(typeInstanceNum);

                STAXListRequestHandler handler;

                synchronized (fListRequestMap)
                {
                    if (!fListRequestMap.containsKey(typeName.toUpperCase()))
                        return new STAFResult(STAFResult.DoesNotExist,typeName);
                    else
                        handler = (STAXListRequestHandler)fListRequestMap.
                                   get(typeName.toUpperCase());
                }

                STAXRequestSettings settings =
                   new STAXRequestSettings(lineSep);

                return handler.handleListRequest(typeName, job, settings);
            }
        }

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private STAFResult handleQuery(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "QUERY", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fQueryParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Create the marshalling context

        STAFMarshallingContext mc = new STAFMarshallingContext();

        // Provide output based on the option(s) specified

        if (parseResult.optionTimes("EXTENSIONJARFILE") != 0)
        {
            // QUERY EXTENSIONJARFILE <Jar File Name>

            mc.setMapClassDefinition(fExtensionInfoMapClass);
            mc.setMapClassDefinition(fServiceExtensionMapClass);
            mc.setMapClassDefinition(fMonitorExtensionMapClass);

            STAXExtension ext = null;
            String extJarFileName = parseResult.optionValue("EXTENSIONJARFILE");

            synchronized (fExtensionsJarList)
            {
                Iterator iter = fExtensionsJarList.iterator();

                while (iter.hasNext())
                {
                    STAXExtension extension = (STAXExtension)iter.next();

                    if (extension.getJarFileName().equals(extJarFileName))
                    {
                        ext = extension;
                        break;
                    }
                }
            }

            if (ext == null)
            {
                return new STAFResult(STAFResult.DoesNotExist, extJarFileName);
            }

            mc.setRootObject(queryExtension(ext));

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else if (parseResult.optionTimes("EXTENSIONJARFILES") != 0)
        {
            // QUERY EXTENSIONSJARFILES

            mc.setMapClassDefinition(fExtensionInfoMapClass);
            mc.setMapClassDefinition(fServiceExtensionMapClass);
            mc.setMapClassDefinition(fMonitorExtensionMapClass);

            List extensionJarFileList = new ArrayList();

            synchronized (fExtensionsJarList)
            {
                Iterator iter = fExtensionsJarList.iterator();

                while (iter.hasNext())
                {
                    STAXExtension ext = (STAXExtension)iter.next();

                    extensionJarFileList.add(queryExtension(ext));
                }
            }

            mc.setRootObject(extensionJarFileList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }

        STAXJob job = null;

        try
        {
            job = (STAXJob)fJobMap.get(
                   new Integer(parseResult.optionValue("JOB")));

            if (job == null)
            {
                return new STAFResult(STAFResult.DoesNotExist,
                                      parseResult.optionValue("JOB"));
            }
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(STAFResult.InvalidValue,
                                  parseResult.optionValue("JOB"));
        }

        if ((parseResult.optionTimes("THREAD") != 0) &&
            (parseResult.optionTimes("VAR") == 0))
        {
            // QUERY JOB <Job ID> THREAD <ThreadID>

            Integer threadID;

            try
            {
                threadID = new Integer(parseResult.optionValue("THREAD"));
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(STAFResult.InvalidValue,
                                      parseResult.optionValue("THREAD"));
            }

            STAXThread thread = job.getThread(threadID);
            
            if (thread == null)
            {
                return new STAFResult(STAFResult.DoesNotExist,
                                      parseResult.optionValue("THREAD"));
            }

            Map threadMap = fQueryThreadMapClass.createInstance();

            threadMap.put("threadID", threadID.toString());

            if (threadID.intValue() != 1)
            {
                threadMap.put(
                    "parentTID", String.valueOf(
                        thread.getParentThread().getThreadNumber()));
                threadMap.put(
                    "parentHierarchy", thread.getParentHierarchy());
            }

            threadMap.put(
                "startTimestamp", thread.getStartTimestamp().
                getTimestampString());

            // Generate the call stack

            threadMap.put("callStack", thread.getCallStack());

            // Generate the condition stack
            
            threadMap.put("conditionStack", thread.getConditionStack());

            mc.setRootObject(threadMap);
            mc.setMapClassDefinition(fQueryThreadMapClass);
        }
        else if ((parseResult.optionTimes("THREAD") != 0) &&
                 (parseResult.optionTimes("VAR") != 0))
        {
            Integer threadID;

            try
            {
                threadID = new Integer(parseResult.optionValue("THREAD"));
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(STAFResult.InvalidValue,
                                      parseResult.optionValue("THREAD"));
            }

            STAXThread thread = job.getThread(threadID);
            
            if (thread == null)
            {
                return new STAFResult(STAFResult.DoesNotExist,
                                      parseResult.optionValue("THREAD"));
            }

            String variableName = parseResult.optionValue("VAR");

            String queryType = "LONG";

            if (parseResult.optionTimes("SHORT") != 0)
            {
                queryType = "SHORT";
            }

            Map locals = thread.getLocals();
            List variableList;

            if (queryType.equals("SHORT"))
            {
                if (!(locals.containsKey(variableName)))
                {
                    return new STAFResult(STAFResult.DoesNotExist,
                                          variableName);
                }

                Map variableMap = new LinkedHashMap();
                variableMap.put(variableName, locals.get(variableName));

                variableList = createShortMarshalledList(variableMap);
            }
            else // LONG
            {
                if (!(locals.containsKey(variableName)))
                {
                    return new STAFResult(STAFResult.DoesNotExist,
                                          variableName);
                }

                Map variableMap = new LinkedHashMap();
                variableMap.put(variableName, locals.get(variableName));

                variableList = createLongMarshalledList(variableMap);
            }

            mc.setRootObject(variableList);
            mc.setMapClassDefinition(fThreadVariableMapClass);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else if (parseResult.numInstances() == 2)
        {
            // QUERY JOB <Job ID>

            mc.setMapClassDefinition(fQueryJobMapClass);

            Map jobInfoMap = fQueryJobMapClass.createInstance();

            jobInfoMap.put("jobID", String.valueOf(job.getJobNumber()));

            if (!job.getJobName().equals(""))
                jobInfoMap.put("jobName", job.getJobName());

            jobInfoMap.put("startTimestamp",
                           job.getStartTimestamp().getTimestampString());
            jobInfoMap.put("xmlFileName", job.getXmlFile());
            jobInfoMap.put("fileMachine", job.getXmlMachine());
            jobInfoMap.put("function",    job.getStartFunction());
            jobInfoMap.put("arguments",
                           STAFUtil.maskPrivateData(job.getStartFuncArgs()));
            jobInfoMap.put("scriptList", job.getScripts());
            jobInfoMap.put("scriptFileList", job.getScriptFiles());

            if (!job.getScriptFileMachine().equals(""))
                jobInfoMap.put("scriptMachine", job.getScriptFileMachine());

            jobInfoMap.put("sourceMachine", job.getSourceMachine());
            jobInfoMap.put("clearLogs", job.getClearLogsAsString());
            jobInfoMap.put("logTCElapsedTime",
                           job.getLogTCElapsedTimeAsString());
            jobInfoMap.put("logTCNumStarts", job.getLogTCNumStartsAsString());
            jobInfoMap.put("logTCStartStop", job.getLogTCStartStopAsString());
            jobInfoMap.put("notifyOnEnd", job.getNotifyOnEndAsString());
            jobInfoMap.put("pythonOutput",
                           STAXPythonOutput.getPythonOutputAsString(
                               job.getPythonOutput()));

            jobInfoMap.put("pythonLogLevel", job.getPythonLogLevel());

            int numThreads = job.getNumThreads();

            jobInfoMap.put("numThreadsRunning", "" + numThreads);

            jobInfoMap.put("state", job.getStateAsString());

            // Get information from registered query job request handlers

            STAFResult handlerResult = null;

            STAXRequestSettings settings = new STAXRequestSettings(lineSep);

            synchronized (fQueryJobRequestList)
            {
                Iterator listIter = fQueryJobRequestList.iterator();

                while(listIter.hasNext())
                {
                    handlerResult = ((STAXQueryRequestHandler)listIter.next()).
                        handleQueryJobRequest(job, settings);

                    if (handlerResult.rc != STAFResult.Ok)
                        return handlerResult;

                    // XXX: How should info really be added to jobInfoMap
                    //      without STAX.java being aware of the fields ?
                    //      (That is, without defining numBlocksRunning, etc.
                    //      in the STAFMapClassDefinition for jobInfoMap)

                    STAFMarshallingContext mc2 =
                        STAFMarshallingContext.unmarshall(
                            handlerResult.result);

                    Object rootObject = mc2.getRootObject();

                    if (rootObject instanceof Map)
                    {
                        Map blockInfoForJobMap = (Map)rootObject;
                        jobInfoMap.putAll(blockInfoForJobMap);
                    }
                }
            }

            mc.setRootObject(jobInfoMap);
        }
        else
        {
            // QUERY JOB <Job ID> <TypeName> <TypeValue>

            int typeInstanceNum = 0;   // 0 indicates type option not found

            for (int i = 1; i <= 3; i++)
            {
                if (!parseResult.instanceName(i).equalsIgnoreCase("QUERY") &&
                    !parseResult.instanceName(i).equalsIgnoreCase("JOB"))
                {   // Found the type option
                   typeInstanceNum = i;
                   break;
                }
            }

            // Check to see if there is a handler for the type option.

            String typeName = parseResult.instanceName(typeInstanceNum);

            STAXQueryRequestHandler handler = null;

            synchronized (fQueryRequestMap)
            {
                if (!fQueryRequestMap.containsKey(typeName.toUpperCase()))
                    return new STAFResult(STAFResult.DoesNotExist, typeName);
                else
                    handler = (STAXQueryRequestHandler)fQueryRequestMap.
                               get(typeName.toUpperCase());
            }

            STAXRequestSettings settings = new STAXRequestSettings(lineSep);

            return handler.handleQueryRequest(typeName,
                parseResult.instanceValue(typeInstanceNum), job, settings);
        }

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private Map queryExtension(STAXExtension ext)
    {
        Map extMap = fExtensionInfoMapClass.createInstance();

        extMap.put("extensionJarFile", ext.getJarFileName());

        if (!ext.getExtVersion().equals(""))
            extMap.put("version", ext.getExtVersion());

        if (!ext.getExtDescription().equals(""))
            extMap.put("description", ext.getExtDescription());

        // Get any parameters specified for the extension

        List parameterList = new ArrayList();
        HashMap parmMap = ext.getParmMap();
        Iterator iter = parmMap.keySet().iterator();
        int num = 1;

        while (iter.hasNext())
        {
            String name = (String)iter.next();
            String value = (String)parmMap.get(name);
            parameterList.add(name + "=" + value);
        }

        extMap.put("parameterList", parameterList);

        // Get any service extensions

        String serviceVersion = ext.getRequiredServiceVersion();

        if ((serviceVersion.equals(sNONE)) &&
            (ext.getSupportedElementMap().size() == 0) &&
            (ext.getUnsupportedElementMap().size() == 0))
        {
            // Don't assign if no service version is a prereq or if the
            // extension provides no service extensions (e.g. elements)
            extMap.put("serviceExtensions", null);
        }
        else
        {
            Map serviceExtMap = fServiceExtensionMapClass.createInstance();

            serviceExtMap.put("requiredServiceVersion", serviceVersion);

            // Get the list of supported elements names for the extension

            List elementNames = new ArrayList();
            iter = ext.getSupportedElementMap().keySet().iterator();

            while (iter.hasNext())
            {
                elementNames.add((String)iter.next());
            }

            serviceExtMap.put("includedElementList", elementNames);

            // Get the list of excluded element names for the extension

            List excludedElementNames = new ArrayList();
            iter = ext.getUnsupportedElementMap().keySet().iterator();

            while (iter.hasNext())
            {
                String name = (String)iter.next();
                excludedElementNames.add(name);
            }

            serviceExtMap.put("excludedElementList", excludedElementNames);

            extMap.put("serviceExtensions", serviceExtMap);
        }

        // Get any monitor extensions

        String monitorVersion = ext.getRequiredMonitorVersion();

        if ((monitorVersion.equals(sNONE)) &&
            (ext.getMonitorExtensionMap().size() == 0))
        {
            // Don't assign if no monitor version is a prereq or if the
            // extension provides no moniter extensions

            extMap.put("monitorExtensions", null);
        }
        else
        {
            Map monitorExtMap = fMonitorExtensionMapClass.createInstance();

            monitorExtMap.put("requiredMonitorVersion", monitorVersion);

            // Get the monitor extension names

            List extNameList = new ArrayList();
            iter = ext.getMonitorExtensionMap().keySet().iterator();

            while (iter.hasNext())
            {
                extNameList.add((String)iter.next());
            }

            monitorExtMap.put("extensionNameList", extNameList);

            extMap.put("monitorExtensions", monitorExtMap);
        }

        return extMap;
    }

    private String getFormattedExtensionInfo(STAXExtension ext)
    {
        StringBuffer result = new StringBuffer("");

        result.append("Jar File Name         : " + ext.getJarFileName() +
                      lineSep);

        String version = ext.getExtVersion();
        if (version.equals("")) version = sNOT_PROVIDED;
        result.append("Version               : " + version + lineSep);

        String description = ext.getExtDescription();
        if (description.equals("")) description = sNOT_PROVIDED;
        result.append("Description           : " + description + lineSep);

        HashMap parmMap = ext.getParmMap();
        Iterator iter = parmMap.keySet().iterator();
        int num = 1;

        while (iter.hasNext())
        {
            String name = (String)iter.next();
            String value = (String)parmMap.get(name);

            result.append("Parameter #" + num);

            if (num < 10)
                result.append("          : ");
            else if (num < 100)
                result.append("         : ");
            else
                result.append("        : ");

            result.append(name + "=" + value + lineSep);
            num++;
        }

        String serviceVersion = ext.getRequiredServiceVersion();

        if ((serviceVersion.equals(sNONE)) &&
            (ext.getSupportedElementMap().size() == 0) &&
            (ext.getUnsupportedElementMap().size() == 0))
        {
            // Don't show the "Service Version Prereq" line if no elements
        }
        else
        {
            result.append("Service Version Prereq: " + serviceVersion +
                          lineSep);

            iter = ext.getSupportedElementMap().keySet().iterator();
            num = 1;

            while (iter.hasNext())
            {
                String name = (String)iter.next();

                result.append("Element Name #" + num);

                if (num < 10)
                    result.append("       : ");
                else if (num < 100)
                    result.append("      : ");
                else
                    result.append("     : ");

                result.append(name + lineSep);
                num++;
            }

            iter = ext.getUnsupportedElementMap().keySet().iterator();

            while (iter.hasNext())
            {
                String name = (String)iter.next();

                result.append("Element Name #" + num);

                if (num < 10)
                    result.append("       : ");
                else if (num < 100)
                    result.append("      : ");
                else
                    result.append("     : ");

                result.append(sNA + " " + name + lineSep);
                num++;
            }
        }

        String monitorVersion = ext.getRequiredMonitorVersion();

        if ((monitorVersion.equals(sNONE)) &&
            (ext.getMonitorExtensionMap().size() == 0))
        {
            // Don't show the "Monitor Version Prereq" line
        }
        else
        {
            result.append("Monitor Version Prereq: " +
                          monitorVersion + lineSep);

            iter = ext.getMonitorExtensionMap().keySet().iterator();
            num = 1;

            while (iter.hasNext())
            {
                String name = (String)iter.next();

                result.append("Monitor Extension #" + num);

                if (num < 10)
                    result.append("  : ");
                else if (num < 100)
                    result.append(" : ");
                else
                    result.append(": ");

                result.append(name + lineSep);
                num++;
            }
        }
        
        return result.toString();
    }

    private STAFResult handleVersion(STAFServiceInterfaceLevel30.
                                     RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "VERSION", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fVersionParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("JYTHON") == 0)
        {
            // Return the version of the service
            return new STAFResult(STAFResult.Ok, fVersion);
        }
        else
        {
            // Return the version of Python packaged with the service
            return new STAFResult(STAFResult.Ok, fJythonVersion);
        }
    }

    private STAFResult handleGet(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "GET", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fGetParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("DTD") > 0)
        {
            // GET DTD

            if (parseResult.optionTimes("STAX-EXTENSIONS-FILE") > 0)
            {
                STAXExtensionFileParser parser = null;

                try
                {
                    parser = new STAXExtensionFileParser(this);
                }
                catch (Exception e)
                {
                    return new STAFResult(
                        STAFResult.JavaError, // XXX: What kind of error?
                        "Caught Exception: " + e.getMessage());
                }

                return new STAFResult(STAFResult.Ok, parser.getDTD());
            }

            return new STAFResult(STAFResult.Ok, fDTD);
        }

        // GET RESULT JOB <Job ID> [DETAILS]

        // Resolve the value specified for JOB and make sure the value
        // specified is an integer > 0

        STAFResult result = STAFUtil.resolveRequestVarAndCheckInt(
            "JOB", parseResult.optionValue("JOB"),
            fHandle, info.requestNumber);

        if (result.rc != 0)
        {
            return new STAFResult(result.rc, result.result);
        }

        int jobID = Integer.parseInt(result.result);

        if (jobID < 1)
        {
            return new STAFResult(
                STAFResult.InvalidValue,
                "Invalid JOB option value.  Must be an integer greater than 0.");
        }

        // Make sure that the STAX job is complete

        STAXJob job = (STAXJob)fJobMap.get(new Integer(jobID));

        if (job != null)
        {
            return new STAFResult(
                STAX.JobNotComplete,
                "Job " + jobID + " has not completed, so no results are " +
                "available yet");
        }
        
        // Determine the results file name for this job based on whether the
        // DETAILS option was specified or not

        String resultFileName = null;

        if (parseResult.optionTimes("DETAILS") == 0)
            resultFileName = getResultFileName(jobID);
        else
            resultFileName = getDetailedResultFileName(jobID);

        // Get the results file for this job
        //
        // Note:  Need to specify the "ASIS" format so that the line endings
        // in the marshalled results file are not changed as that can result
        // in invalid marshalled data if the string length no longer matches
        // the Colon-Length-Colon length within the marshalled data 

        result = fHandle.submit2(
            "local", "FS", "GET FILE " + STAFUtil.wrapData(resultFileName) +
            " TEXT FORMAT ASIS");

        if (result.rc == STAFResult.DoesNotExist)
        {
            return new STAFResult(
                result.rc, "No result file exists for job " + jobID);
        }
        else if (result.rc != STAFResult.Ok)
        {
            return new STAFResult(
                result.rc, "Error getting results for job " + jobID +
                " from file " + resultFileName);
        }
            
        // Return the marshalled string contained in the results file
        
        return new STAFResult(STAFResult.Ok, result.result);
    }

    private STAFResult handleSet(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 5

        STAFResult trustResult = STAFUtil.validateTrust(
            5, fServiceName, "SET", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fSetParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        STAFResult resolvedValue;

        // Handle CLEARLOGS setting

        if (parseResult.optionTimes("CLEARLOGS") > 0)
        {
            // Resolve the value specified for CLEARLOGS
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("CLEARLOGS"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                fClearLogs = true;
            else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                fClearLogs = false;
            else
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "CLEARLOGS value must be ENABLED or DISABLED.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }

        // Handle LOGTCELAPSEDTIME setting

        if (parseResult.optionTimes("LOGTCELAPSEDTIME") > 0)
        {
            // Resolve the value specified for LOGTCELAPSEDTIME
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("LOGTCELAPSEDTIME"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                fLogTCElapsedTime = true;
            else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                fLogTCElapsedTime = false;
            else
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "LOGTCELAPSEDTIME value must be ENABLED or DISABLED.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }

        // Handle LOGTCNUMSTARTS setting

        if (parseResult.optionTimes("LOGTCNUMSTARTS") > 0)
        {
            // Resolve the value specified for LOGTCNUMSTARTS
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("LOGTCNUMSTARTS"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                fLogTCNumStarts = true;
            else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                fLogTCNumStarts = false;
            else
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "LOGTCNUMSTARTS value must be ENABLED or DISABLED.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }

        // Handle LOGTCSTARTSTOP setting

        if (parseResult.optionTimes("LOGTCSTARTSTOP") > 0)
        {
            // Resolve the value specified for LOGTCSTARTSTOP
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("LOGTCSTARTSTOP"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                fLogTCStartStop = true;
            else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                fLogTCStartStop = false;
            else
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "LOGTCSTARTSTOP value must be ENABLED or DISABLED.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }

        // Handle PYTHONOUTPUT setting

        if (parseResult.optionTimes("PYTHONOUTPUT") > 0)
        {
            // Resolve the value specified for PYTHONOUTPUT
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("PYTHONOUTPUT"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != STAFResult.Ok)
                return resolvedValue;

            STAFResult result = STAXPythonOutput.isValidPythonOutput(
                resolvedValue.result);
            
            if (result.rc != STAFResult.Ok)
                return result;
            
            fPythonOutput = Integer.parseInt(result.result);
        }
        
        // Handle PYTHONLOGLEVEL setting

        if (parseResult.optionTimes("PYTHONLOGLEVEL") > 0)
        {
            // Resolve the value specified for PYTHONLOGLEVEL
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("PYTHONLOGLEVEL"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != STAFResult.Ok)
                return resolvedValue;

            STAFResult result = STAXPythonOutput.isValidLogLevel(
                resolvedValue.result);
            
            if (result.rc != STAFResult.Ok)
                return result;
            
            fPythonLogLevel = result.result;
        }
        
        // Handle FILECACHING setting
        
        if (parseResult.optionTimes("FILECACHING") > 0)
        {
            // Resolve the value specified for FILECACHING
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("FILECACHING"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            if (resolvedValue.result.equalsIgnoreCase("ENABLED"))
                fFileCaching = true;
            else if (resolvedValue.result.equalsIgnoreCase("DISABLED"))
                fFileCaching = false;
            else
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "FILECACHING value must be ENABLED or DISABLED.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }
        
        // Handle MAXFILECACHESIZE setting
        
        if (parseResult.optionTimes("MAXFILECACHESIZE") > 0)
        {
            // Resolve the value specified for MAXFILECACHESIZE
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("MAXFILECACHESIZE"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            try
            {
                int maxFileCacheSize = Integer.parseInt(
                    resolvedValue.result);
                
                if (maxFileCacheSize < -1)
                {
                    return new STAFResult(
                        STAFResult.InvalidValue,
                        "MAXFILECACHESIZE value must be an integer >= -1.  " +
                        "Invalid value: " + resolvedValue.result);
                }
                
                STAXFileCache.get().setMaxCacheSize(maxFileCacheSize);
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "MAXFILECACHESIZE value must be an integer >= -1.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }

        // Handle FILECACHEALGORITHM setting

        if (parseResult.optionTimes("FILECACHEALGORITHM") > 0)
        {
            // Resolve the value specified for FILECACHEALORITHM"
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("FILECACHEALGORITHM"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            STAFResult result = STAXFileCache.get().setAlgorithm(
                resolvedValue.result);

            if (result.rc != 0)
                return result;
        }

        // Handle MAXFILECACHEAGE setting

        if (parseResult.optionTimes("MAXFILECACHEAGE") > 0)
        {
            // Resolve the value specified for MAXFILECACHEAGE and verify
            // that it is a valid max age value, and convert it to seconds
            // if needed

            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("MAXFILECACHEAGE"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            STAFResult result = STAXFileCache.get().setMaxAge(
                resolvedValue.result);

            if (result.rc != 0)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid value for MAXFILECACHEAGE: " +
                    resolvedValue.result + "\n\n" + result.result);
            }
        }
        
        // Handle MAXMACHINECACHESIZE setting
        
        if (parseResult.optionTimes("MAXMACHINECACHESIZE") > 0)
        {
            // Resolve the value specified for MAXMACHINECACHESIZE

            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("MAXMACHINECACHESIZE"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            try
            {
                int maxMachineCacheSize = Integer.parseInt(
                    resolvedValue.result);
                
                if (maxMachineCacheSize < -1)
                {
                    throw new NumberFormatException();
                }
                
                STAXMachineCache.get().setMaxCacheSize(maxMachineCacheSize);
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "MAXMACHINECACHESIZE value must be an integer >= -1.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }
        
        // Handle MAXRETURNFILESIZE setting
        
        if (parseResult.optionTimes("MAXRETURNFILESIZE") > 0)
        {
            // Resolve the value specified for MAXRETURNFILESIZE

            resolvedValue = STAFUtil.resolveRequestVarAndConvertSize(
                "MAXRETURNFILESIZE",
                parseResult.optionValue("MAXRETURNFILESIZE"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            try
            {
                fMaxReturnFileSize = Long.parseLong(resolvedValue.result);
            }
            catch (NumberFormatException e)
            {
                // Should never happen because resolveRequestVarAndConvertSize
                // should have returned an error previously

                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid MAXRETURNFILESIZE value: " +
                    resolvedValue.result);
            }
        }

        // Handle MAXGETQUEUEMESSAGES setting
        
        if (parseResult.optionTimes("MAXGETQUEUEMESSAGES") > 0)
        {
            // Resolve the value specified for MAXGETQUEUEMESSAGES

            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("MAXGETQUEUEMESSAGES"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            try
            {
                int maxGetQueueMessages = Integer.parseInt(
                    resolvedValue.result);
                
                if ((maxGetQueueMessages < 1) || (maxGetQueueMessages > 100))
                {
                    throw new NumberFormatException();
                }

                fMaxGetQueueMessages = maxGetQueueMessages;
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "MAXGETQUEUEMESSAGES value must be an integer > 0 " +
                    "and < 101.  Invalid value: " + resolvedValue.result);
            }
        }

        if (parseResult.optionTimes("MAXSTAXTHREADS") > 0)
        {
            // Resolve the value specified for MAXSTAXTHREADS

            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("MAXSTAXTHREADS"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            try
            {
                int maxSTAXThreads = Integer.parseInt(resolvedValue.result);
                
                if (maxSTAXThreads < 0)
                {
                    throw new NumberFormatException();
                }

                fMaxSTAXThreads = maxSTAXThreads;
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "MAXSTAXTHREADS value must be an integer >= 0 and " +
                    "<= 2147483647.  Invalid value: " + resolvedValue.result);
            }
        }

        return new STAFResult(STAFResult.Ok, "");
    }


    private STAFResult handleNotify(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Parse the result

        STAFCommandParseResult parseResult= fNotifyParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("REGISTER") > 0)
        {
            // Verify the requesting machine/user has at least trust level 3

            STAFResult trustResult = STAFUtil.validateTrust(
                3, fServiceName, "NOTIFY REGISTER", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Resolve the value specified for ONENDOFJOB

            STAFResult resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                "ONENDOFJOB", parseResult.optionValue("ONENDOFJOB"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;
            
            Integer jobID = new Integer(resolvedValue.result);
            
            // Check if the job is currently running

            STAXJob job = (STAXJob)fJobMap.get(jobID);

            if (job == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist, jobID.toString());
            }
            
            int priority = 5;  // Default value for priority

            if (parseResult.optionTimes("PRIORITY") > 0)
            {
                // Resolve the value specified for PRIORITY and make sure
                // the value specified is an integer

                resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                    "PRIORITY", parseResult.optionValue("PRIORITY"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;

                priority = Integer.parseInt(resolvedValue.result);
            }
            
            // Add to the list of notifiees registered for this job

            int notifyBy = STAXJob.NOTIFY_ONEND_BY_HANDLE;

            if (parseResult.optionTimes("BYNAME") > 0)
                notifyBy = STAXJob.NOTIFY_ONEND_BY_NAME;
            
            STAXJobCompleteNotifiee notifiee = new STAXJobCompleteNotifiee(
                notifyBy, info.endpoint, info.handle, info.handleName,
                priority, null);

            STAFResult result = job.addCompletionNotifiee2(notifiee);

            if (result.rc != STAFResult.Ok)
                return result;

            return new STAFResult(STAFResult.Ok, "");
        }
        else if (parseResult.optionTimes("UNREGISTER") > 0)
        {
            // Verify the requesting machine/user has at least trust level 3

            STAFResult trustResult = STAFUtil.validateTrust(
                3, fServiceName, "NOTIFY UNREGISTER", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;
            
            // Resolve the value specified for ONENDOFJOB

            STAFResult resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
                "ONENDOFJOB", parseResult.optionValue("ONENDOFJOB"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;
            
            Integer jobID = new Integer(resolvedValue.result);
            
            // Check if the job is currently running

            STAXJob job = (STAXJob)fJobMap.get(jobID);

            if (job == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist, jobID.toString());
            }

            // Remove entry from notification list if in list.

            STAXJobCompleteNotifiee notifiee = new STAXJobCompleteNotifiee(
                STAXJob.NOTIFY_ONEND_BY_HANDLE, info.endpoint, info.handle,
                info.handleName, 5, null);

            STAFResult removeResult = job.removeCompletionNotifiee(notifiee);

            // If the machine/handle/handleName is not already in the
            // notification list, return an error
            
            if (removeResult.rc != STAFResult.Ok)
            {
                return removeResult;
            }

            return new STAFResult(STAFResult.Ok, "");
        }
        else
        {
            // NOTIFY LIST Request

            // Verify the requesting machine/user has at least trust level 2

            STAFResult trustResult = STAFUtil.validateTrust(
                2, fServiceName, "NOTIFY LIST", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            STAFMarshallingContext mc = new STAFMarshallingContext();
            List resultList = new ArrayList();

            if (parseResult.optionTimes("JOB") > 0)
            {
                // NOTIFY LIST JOB <Job ID>

                // Resolve the value specified for JOB

                STAFResult resolvedValue = 
                    STAFUtil.resolveRequestVarAndCheckInt(
                        "JOB", parseResult.optionValue("JOB"), fHandle,
                        info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;
            
                Integer jobID = new Integer(resolvedValue.result);

                // Check if the job is currently running

                STAXJob job = (STAXJob)fJobMap.get(jobID);

                if (job == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist, jobID.toString());
                }
            
                // List all notifiees for this job
            
                mc.setMapClassDefinition(fNotifieeMapClass);

                LinkedList notifieeList = job.getCompletionNotifiees();
                Iterator iter = notifieeList.iterator();

                while (iter.hasNext())
                {
                    Object notifieeObj = iter.next();
              
                    if (notifieeObj instanceof
                        com.ibm.staf.service.stax.STAXJobCompleteNotifiee)
                    {
                        STAXJobCompleteNotifiee notifiee =
                            (STAXJobCompleteNotifiee)notifieeObj;

                        Map notifieeMap = fNotifieeMapClass.createInstance();

                        notifieeMap.put("machine", notifiee.getMachine());
                        notifieeMap.put("notifyBy",
                                        notifiee.getNotifyByString());
                        notifieeMap.put("handle",
                                        String.valueOf(notifiee.getHandle()));
                        notifieeMap.put("handleName",
                                        notifiee.getHandleName());
                        notifieeMap.put("priority",
                                        String.valueOf(notifiee.getPriority()));

                        resultList.add(notifieeMap);
                    }
                }
            }
            else
            {
                // NOTIFY LIST (for all jobs)

                mc.setMapClassDefinition(fAllNotifieeMapClass);

                synchronized (fJobMap)
                {
                    Iterator jobIter = fJobMap.values().iterator();

                    while (jobIter.hasNext())
                    {
                        STAXJob job = (STAXJob)jobIter.next();
                        LinkedList notifieeList = job.getCompletionNotifiees();
                        Iterator notifieeIter = notifieeList.iterator();

                        while (notifieeIter.hasNext())
                        {
                            Object notifieeObj = notifieeIter.next();

                            if (notifieeObj instanceof
                                com.ibm.staf.service.stax.STAXJobCompleteNotifiee)
                            {
                                STAXJobCompleteNotifiee notifiee =
                                    (STAXJobCompleteNotifiee)notifieeObj;

                                Map notifieeMap = fAllNotifieeMapClass.
                                    createInstance();

                                notifieeMap.put(
                                    "jobID",
                                    String.valueOf(job.getJobNumber()));
                                notifieeMap.put(
                                    "machine", notifiee.getMachine());
                                notifieeMap.put(
                                    "notifyBy", notifiee.getNotifyByString());
                                notifieeMap.put(
                                    "handle",
                                    String.valueOf(notifiee.getHandle()));
                                notifieeMap.put(
                                    "handleName", notifiee.getHandleName());
                                notifieeMap.put(
                                    "priority", 
                                    String.valueOf(notifiee.getPriority()));

                                resultList.add(notifieeMap);
                            }
                        }
                    }
                } // end synchronized (fJobMap)
            }

            mc.setRootObject(resultList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
    }
    
    private STAFResult handlePurge(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 5

        STAFResult trustResult = STAFUtil.validateTrust(
            5, fServiceName, "PURGE", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
        
        // Parse the result

        STAFCommandParseResult parseResult= fPurgeParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }
       
        int numPurged = 0;

        STAFMarshallingContext mc = new STAFMarshallingContext();
        
        if (parseResult.optionTimes("FILECACHE") > 0)
        {
            numPurged = STAXFileCache.get().purge();
        }
        else if (parseResult.optionTimes("MACHINECACHE") > 0)
        {
            numPurged = STAXMachineCache.get().purge();
        }
            
        mc.setMapClassDefinition(fPurgeStatsMapClass);
        Map purgeMap = fPurgeStatsMapClass.createInstance();
        purgeMap.put("numPurged", String.valueOf(numPurged));
        purgeMap.put("numRemaining", "0");
            
        mc.setRootObject(purgeMap);
        
        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private STAFResult handleStop(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Parse the result

        STAFCommandParseResult parseResult= fStopParser.parse(info.request);

        if (parseResult.optionTimes("THREAD") == 0)
        {
            return handleGenericRequest(info);
        }

        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, fServiceName, "STOP", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        STAXJob job = null;

        try
        {
            job = (STAXJob)fJobMap.get(
                   new Integer(parseResult.optionValue("JOB")));

            if (job == null)
            {
                return new STAFResult(STAFResult.DoesNotExist,
                                      parseResult.optionValue("JOB"));
            }
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(STAFResult.InvalidValue,
                                  parseResult.optionValue("JOB"));
        }

        Integer threadID;

        try
        {
            threadID = new Integer(parseResult.optionValue("THREAD"));
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(STAFResult.InvalidValue,
                                  parseResult.optionValue("THREAD"));
        }

        STAXThread thread = job.getThread(threadID);

        if (thread == null)
        {
            return new STAFResult(STAFResult.DoesNotExist,
                                  parseResult.optionValue("THREAD"));
        }

        thread.setBreakpointCondition(true);

        return new STAFResult(STAFResult.Ok, "");
    }

    private STAFResult handlePyExec(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, fServiceName, "PYEXEC", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fPyExecParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        STAXJob job = null;

        try
        {
            job = (STAXJob)fJobMap.get(
                new Integer(parseResult.optionValue("JOB")));

            if (job == null)
            {
                return new STAFResult(STAFResult.DoesNotExist,
                                      parseResult.optionValue("JOB"));
            }
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(STAFResult.InvalidValue,
                                  parseResult.optionValue("JOB"));
        }

        Integer threadID;

        try
        {
            threadID = new Integer(parseResult.optionValue("THREAD"));
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(STAFResult.InvalidValue,
                                  parseResult.optionValue("THREAD"));
        }

        STAXThread thread = job.getThread(threadID);

        if (thread == null)
        {
            return new STAFResult(STAFResult.DoesNotExist,
                                  parseResult.optionValue("THREAD"));
        }

        String pythonCode = parseResult.optionValue("CODE");

        try
        {
          thread.pyExec(pythonCode);
        }
        catch (STAXPythonEvaluationException ex)
        {
            return new STAFResult(STAFResult.InvalidValue, ex.toString());
        }

        return new STAFResult(STAFResult.Ok, "");
    }

    private STAFResult handleGenericRequest(STAFServiceInterfaceLevel30.
                                            RequestInfo info)
    {
        // Find first generic request handler registered for this request with
        // a matching parser.

        STAXGenericRequest genReq = null;
        STAXGenericRequestHandler handler = null;

        synchronized (fGenericRequestList)
        {
            Iterator listIter = fGenericRequestList.iterator();

            while(listIter.hasNext())
            {
                // XXX: When add a new Service Interface Level class (other than
                // STAFServiceInterfaceLevel30), need to add code here to pass
                // the right level of information to handleRequest.

                // Attempt to parse and handle the request with this handler
                STAFResult parseResult = ((STAXGenericRequest)listIter.next()).
                                         fHandler.handleRequest(info, this);

                // Check if found matching parser and handler.
                // If parser returns an non-blank result, that indicates
                // it is the right parser, just an invalid request.
                if ((parseResult.rc != STAFResult.InvalidRequestString) ||
                    (! parseResult.result.equals("")))
                {
                    // Found matching parser and handler
                    return parseResult;
                }
            }
        }

        // Determine the command request (the first word in the request)

        String action;
        int spaceIndex = info.request.indexOf(" ");

        if (spaceIndex != -1)
            action = info.request.substring(0, spaceIndex);
        else
            action = info.request;

        return new STAFResult(
            STAFResult.InvalidRequestString,
            "'" + action + "' is not a valid command request for the " +
            fServiceName + " service" + lineSep + lineSep + sHelpMsg);

    }

    private STAFResult handleHelp(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requesting machine/user has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "HELP", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fHelpParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Return help text

        return new STAFResult(STAFResult.Ok, sHelpMsg);
    }

    private STAFResult handleParms(STAFServiceInterfaceLevel30.InitInfo info)
        throws STAFException
    {
        STAFCommandParseResult parseResult= fParmsParser.parse(info.parms);

        String errmsg = "ERROR:  Service Configuration Error for Service " +
                        fServiceName + lineSep + "STAX::handleParms() - ";

        if (parseResult.rc != STAFResult.Ok)
        {
            System.out.println(
                errmsg + "PARMS parsing failed with RC=" +
                STAFResult.InvalidRequestString +
                " Result=" + parseResult.errorBuffer);

            return new STAFResult(
                STAFResult.InvalidRequestString, parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("EVENTSERVICEMACHINE") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("EVENTSERVICEMACHINE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving EVENTSERVICEMACHINE.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            fEventServiceMachine = resolvedResult.result;
        }

        if (parseResult.optionTimes("EVENTSERVICENAME") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("EVENTSERVICENAME"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving EVENTSERVICENAME.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            fEventServiceName = resolvedResult.result;
        }

        if (parseResult.optionTimes("NUMTHREADS") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVarAndCheckInt(
                "NUMTHREADS", parseResult.optionValue("NUMTHREADS"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Invalid NUMTHREADS value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
                
            fNumThreads = Integer.parseInt(resolvedResult.result);

            if (fNumThreads < 2)
            {
                String msg = "NUMTHREADS value must be > 1.  " +
                    "NUMTHREADS=" + fNumThreads;
                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("EXTENSIONXMLFILE") > 0)
        {
            STAFResult res = STAFUtil.resolveInitVar(
                parseResult.optionValue("EXTENSIONXMLFILE"), fHandle);

            if (res.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving EXTENSIONXMLFILE value. RC=" +
                    res.rc + " Result=" + res.result);

                return res;
            }

            String extFileName = res.result;
            fExtensionFile = res.result;

            res = fHandle.submit2("local", "fs", "get file " + fExtensionFile);

            if (res.rc != 0)
            {   // There was an error in the FS service submission

                String msg = "EXTENSIONXMLFILE " + fExtensionFile +
                    " does not exist.";
                System.out.println(errmsg + msg);

                return res;
            }

            String extFileContents = res.result;
            STAXExtensionFileParser parser = null;

            try
            {
                parser = new STAXExtensionFileParser(this);

                parser.parse(new InputSource(
                    new StringReader(extFileContents)));
            }
            catch (STAXException e)
            {
                String msg = "Error parsing EXTENSIONXMLFILE " +
                    fExtensionFile + lineSep +
                    "Caught " + e.getClass().getName() + ": " + lineSep +
                    e.getMessage();

                System.out.println(errmsg + msg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, msg);
            }
            catch (SAXNotRecognizedException e)
            {
                String msg = "Error parsing EXTENSIONXMLFILE " +
                    fExtensionFile + lineSep +
                    "Caught SAXNRE: " + e.getMessage();

                System.out.println(errmsg + msg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, msg);
            }
            catch (SAXNotSupportedException e)
            {
                String msg = "Error parsing EXTENSIONXMLFILE " +
                    fExtensionFile + lineSep +
                    "Caught SAXNSE: " + e.getMessage();

                System.out.println(errmsg + msg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, msg);
            }
            catch (Exception e)
            {
                String msg = "Error parsing EXTENSIONXMLFILE " +
                    fExtensionFile + lineSep +
                    "Caught Exception: " + e.getMessage();

                System.out.println(errmsg + msg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, msg);
            }
        }

        if (parseResult.optionTimes("EXTENSIONFILE") > 0)
        {   
            STAFResult res = STAFUtil.resolveInitVar(
                parseResult.optionValue("EXTENSIONFILE"), fHandle);
            
            if (res.rc != STAFResult.Ok)
            {
                String msg = "Error resolving EXTENSIONFILE value.  RC=" +
                    res.rc + " Result=" + res.result;

                System.out.println(errmsg + msg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, msg);
            }

            fExtensionFile = res.result;

            res = fHandle.submit2("local", "fs", "get file " + fExtensionFile);

            if (res.rc != 0)
            {   // There was an error in the FS service submission
                String msg = "EXTENSIONFILE " + fExtensionFile +
                    " does not exist, RC: " + res.rc;

                System.out.println(errmsg + msg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, msg);
            }

            String extFileContents = res.result;

            res = readExtensionData(extFileContents);

            if (res.rc != 0)
            {
                System.out.println(res.result);
                return res;
            }
        }

        if (parseResult.optionTimes("EXTENSION") > 0)
        {
            for (int i = 1; i <= parseResult.optionTimes("EXTENSION"); ++i)
            {
                STAFResult res = STAFUtil.resolveInitVar(
                    parseResult.optionValue("EXTENSION", i), fHandle);
                
                if (res.rc != STAFResult.Ok)
                {
                    System.out.println(
                        errmsg + "Error resolving EXTENSION value.  RC=" +
                        res.rc + " Result=" + res.result);

                    return res;
                }

                String extValue = res.result;
                fExtensionList.add(extValue);

                res = readExtensionData(extValue);

                if (res.rc != 0)
                {
                    System.out.println(res.result);
                    return res;
                }
            }
        }

        if (parseResult.optionTimes("PROCESSTIMEOUT") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("PROCESSTIMEOUT"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving PROCESSTIMEOUT value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            try
            {
                fProcessTimeout = Integer.parseInt(resolvedResult.result);

                if (fProcessTimeout < 1000)
                {
                    String msg = "PROCESSTIMEOUT value must be at least " +
                        "1000 (1 second).  PROCESSTIMEOUT=" + fProcessTimeout;

                    System.out.println(errmsg + msg);

                    return new STAFResult(STAFResult.InvalidValue, msg);
                }
            }
            catch (NumberFormatException e)
            {
                String msg = "PROCESSTIMEOUT value must be numeric.  " +
                    "PROCESSTIMEOUT=" + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("CLEARLOGS") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("CLEARLOGS"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving CLEARLOGS value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equalsIgnoreCase("ENABLED"))
            {
                fClearLogs = true;
            }
            else if (resolvedResult.result.equalsIgnoreCase("DISABLED"))
            {
                fClearLogs = false;
            }
            else
            {
                String msg = "CLEARLOGS must be set to Enabled or Disabled. " +
                    " Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("LOGTCELAPSEDTIME") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("LOGTCELAPSEDTIME"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving LOGTCELAPSEDTIME value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equalsIgnoreCase("ENABLED"))
            {
                fLogTCElapsedTime = true;
            }
            else if (resolvedResult.result.equalsIgnoreCase("DISABLED"))
            {
                fLogTCElapsedTime = false;
            }
            else
            {
                String msg = "LOGTCELAPSEDTIME must be set to Enabled or " +
                    "Disabled.  Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("LOGTCNUMSTARTS") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("LOGTCNUMSTARTS"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving LOGTCNUMSTARTS value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equalsIgnoreCase("ENABLED"))
            {
                fLogTCNumStarts = true;
            }
            else if (resolvedResult.result.equalsIgnoreCase("DISABLED"))
            {
                fLogTCNumStarts = false;
            }
            else
            {
                String msg = "LOGTCNUMSTARTS must be set to Enabled or " +
                    "Disabled.  Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("LOGTCSTARTSTOP") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("LOGTCSTARTSTOP"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving LOGTCSTARTSTOP value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equalsIgnoreCase("ENABLED"))
            {
                fLogTCStartStop = true;
            }
            else if (resolvedResult.result.equalsIgnoreCase("DISABLED"))
            {
                fLogTCStartStop = false;
            }
            else
            {
                String msg = "LOGTCSTARTSTOP must be set to Enabled or " +
                    "Disabled.  Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("PYTHONOUTPUT") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("PYTHONOUTPUT"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving PYTHONOUTPUT value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            STAFResult result = STAXPythonOutput.isValidPythonOutput(
                resolvedResult.result);
            
            if (result.rc != STAFResult.Ok)
            {
                System.out.println(errmsg + result.result);

                return result;
            }

            fPythonOutput = Integer.parseInt(result.result);
        }

        if (parseResult.optionTimes("PYTHONLOGLEVEL") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("PYTHONLOGLEVEL"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving PYTHONLOGLEVEL value.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            STAFResult result = STAXPythonOutput.isValidLogLevel(
                resolvedResult.result);
            
            if (result.rc != STAFResult.Ok)
            {
                System.out.println(errmsg + result.result);

                return result;
            }

            fPythonLogLevel = result.result;
        }

        if (parseResult.optionTimes("FILECACHING") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("FILECACHING"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving FILECACHING.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equalsIgnoreCase("ENABLED"))
            {
                fFileCaching = true;
            }
            else if (resolvedResult.result.equalsIgnoreCase("DISABLED"))
            {
                fFileCaching = false;
            }
            else
            {
                String msg = "FILECACHING must be set to Enabled or " +
                    "Disabled.  Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }
        
        if (parseResult.optionTimes("MAXFILECACHESIZE") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAXFILECACHESIZE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving MAXFILECACHESIZE.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
            
            try
            {
                int maxFileCacheSize = Integer.parseInt(resolvedResult.result);
                
                if (maxFileCacheSize < -1)
                {
                    String msg = "MAXFILECACHESIZE must be an integer >= -1.  " +
                        "Invalid value: " + maxFileCacheSize;

                    System.out.println(errmsg + msg);

                    return new STAFResult(STAFResult.InvalidValue, msg);
                }
                
                STAXFileCache.get().setMaxCacheSize(maxFileCacheSize);
                
            }
            catch (NumberFormatException e)
            {
                String msg = "MAXFILECACHESIZE must be an integer >= -1.  " +
                    "Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("FILECACHEALGORITHM") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("FILECACHEALGORITHM"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                resolvedResult.result = "Error resolving FILECACHEALGORITHM." +
                    "\n\n" + resolvedResult.result;

                System.out.println(
                    errmsg + "RC=" + resolvedResult.rc +
                    " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            STAFResult result = STAXFileCache.get().setAlgorithm(
                resolvedResult.result);

            if (result.rc != STAFResult.Ok)
            {
                System.out.println(errmsg + result.result);

                return result;
            }
        }

        if (parseResult.optionTimes("MAXFILECACHEAGE") > 0)
        {
            if (STAXFileCache.get().getAlgorithm() != STAXFileCache.LFU)
            {
                return new STAFResult(
                    STAFResult.InvalidRequestString,
                    "The MAXFILECACHEAGE parameter can only be set if " +
                    "the FILECACHEALGORITHM parameter's value is LFU.");
            }

            // Resolve the value specified for MAXFILECACHEAGE and verify
            // that it is a valid max age value, and convert it to seconds
            // if needed

            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAXFILECACHEAGE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                resolvedResult.result = "Error resolving MAXFILECACHEAGE." +
                    "\n\n" + resolvedResult.result;

                System.out.println(
                    errmsg + "RC=" + resolvedResult.rc +
                    " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            STAFResult result = STAXFileCache.get().setMaxAge(
                resolvedResult.result);

            if (result.rc != 0)
            {
                result.result = "Invalid value for MAXFILECACHEAGE: " +
                    resolvedResult.result + "\n\n" + result.result;

                System.out.println(errmsg + result.result);

                return result;
            }
        }

        if (parseResult.optionTimes("MAXMACHINECACHESIZE") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAXMACHINECACHESIZE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving MAXMACHINECACHESIZE.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
            
            try
            {
                int maxMachineCacheSize = Integer.parseInt(
                    resolvedResult.result);
                
                if (maxMachineCacheSize < -1)
                {
                    throw new NumberFormatException();
                }
                
                STAXMachineCache.get().setMaxCacheSize(maxMachineCacheSize);
                
            }
            catch (NumberFormatException e)
            {
                String msg = "MAXMACHINECACHESIZE must be an integer >= -1.  "+
                    "Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }
        
        if (parseResult.optionTimes("MAXRETURNFILESIZE") > 0)
        {
            // Resolve the value specified for MAXRETURNFILESIZE
           
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAXRETURNFILESIZE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving MAXRETURNFILESIZE.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            STAFResult sizeResult = STAFUtil.convertSizeString(
                resolvedResult.result);

            if (sizeResult.rc != 0)
            {
                String msg = "Invalid MAXRETURNFILESIZE value: " +
                    resolvedResult.result + "\n\n" + sizeResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }

            try
            {
                fMaxReturnFileSize = Long.parseLong(sizeResult.result);
            }
            catch (NumberFormatException e)
            {
                // Should never happen because convertSizeString should have
                // returned an error previously

                String msg = "Invalid MAXRETURNFILESIZE value: " +
                    resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("MAXGETQUEUEMESSAGES") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAXGETQUEUEMESSAGES"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving MAXGETQUEUEMESSAGES.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
            
            try
            {
                int maxGetQueueMessages = Integer.parseInt(
                    resolvedResult.result);
                
                if ((maxGetQueueMessages < 1) || (maxGetQueueMessages > 100))
                {
                    throw new NumberFormatException();
                }
                
                fMaxGetQueueMessages = maxGetQueueMessages;
            }
            catch (NumberFormatException e)
            {
                String msg = "MAXGETQUEUEMESSAGES must be an integer > 0 " +
                    "and < 101.  Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        if (parseResult.optionTimes("MAXSTAXTHREADS") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAXSTAXTHREADS"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving MAXSTAXTHREADS.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
            
            try
            {
                int maxSTAXThreads = Integer.parseInt(resolvedResult.result);
                
                if (maxSTAXThreads < 0)
                {
                    throw new NumberFormatException();
                }
                
                fMaxSTAXThreads = maxSTAXThreads;
            }
            catch (NumberFormatException e)
            {
                String msg = "MAXSTAXTHREADS must be an integer >= 0 and " +
                    "<= 2147483647.  Invalid value: " +
                    resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }
        
        if (parseResult.optionTimes("CACHEPYTHONCODE") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("CACHEPYTHONCODE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving CACHEPYTHONCODE.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equalsIgnoreCase("ENABLED"))
            {
                CACHE_PYTHON_CODE = true;
            }
            else if (resolvedResult.result.equalsIgnoreCase("DISABLED"))
            {
                CACHE_PYTHON_CODE = false;
            }
            else
            {
                String msg = "CACHEPYTHONCODE must be set to Enabled or " +
                    "Disabled.  Invalid value: " + resolvedResult.result;

                System.out.println(errmsg + msg);

                return new STAFResult(STAFResult.InvalidValue, msg);
            }
        }

        return new STAFResult(STAFResult.Ok);
    }

    // STAXJobCompleteListener method

    public void jobComplete(STAXJob job)
    {
        String jobNumber = Integer.toString(job.getJobNumber());
        
        synchronized (fJobMap)
        {
            fJobMap.remove(job.getJobNumberAsInteger());
        }

        synchronized (fExecuteWaitList)
        {
            if (fExecuteWaitList.contains(jobNumber))
            {
                fExecuteWaitList.remove(fExecuteWaitList.indexOf(jobNumber));

                STAFResult Result = fHandle.submit2(
                    "local", "QUEUE", "QUEUE TYPE " +
                    sQueueTypeJobWaitComplete + jobNumber +
                    " MESSAGE " + STAFUtil.wrapData(""));
            }
        }
    }

    private STAFResult readExtensionData(String value)
    {
        String line;
        BufferedReader br = new BufferedReader(new StringReader(value));
        StringTokenizer st;

        try
        {
            while ((line = br.readLine()) != null)
            {
                String jarFileName;
                int pos = line.indexOf('#');
                HashMap includeElementMap = new HashMap();

                if (pos == -1)
                {
                    jarFileName = line.trim();
                }
                else if (pos == 0)
                {
                    continue;  // Skip comment indicated by # in first position
                }
                else
                {
                    jarFileName = line.substring(0, pos - 1).trim();

                    st = new StringTokenizer(line.substring(pos + 1));

                    while (st.hasMoreTokens())
                    {
                        includeElementMap.put(st.nextToken(), null);
                    }
                }

                if (jarFileName.equals(""))
                    continue;  // Skip blank lines (& lines without a jar file)

                if (!jarFileName.toLowerCase().endsWith(".jar"))
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::readExtensionData() - " +
                        "Extension jar file: " + jarFileName + lineSep +
                        "Extension jar file name must end in .jar");
                }

                fExtensionsJarList.add(new STAXExtension(
                    jarFileName, includeElementMap));
            }
        }
        catch (Exception e)
        {
            return new STAFResult(STAFResult.InvalidValue, e.toString());
        }
        finally
        {
            try
            {
                br.close();
            }
            catch (IOException e)
            { /* Do nothing */ }
        }

        return new STAFResult(STAFResult.Ok, "");
    }

    private STAFResult loadExtensions()
    {
        // Map used to make sure no duplicate extensions are loaded
        Map extElementMap = new HashMap();

        // Iterate through the Jar File Extensions List and load
        // the specified extension elements

        Iterator iter = fExtensionsJarList.iterator();

        while (iter.hasNext())
        {
            STAXExtension ext = (STAXExtension)iter.next();

            JarFile jarFile = null;
            String jarFileName = ext.getJarFileName();

            try
            {
                jarFile = new JarFile(jarFileName);
            }
            catch (IOException e)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "ERROR:  Service Configuration Error for Service=" +
                    fServiceName + lineSep +
                    "STAX::loadExtensions() - Extension jar file: " +
                    jarFileName + lineSep +
                    "Unable to access extension jar file." + lineSep +
                    e.toString());
            }

            Manifest manifest = null;

            try
            {
                manifest = jarFile.getManifest();
            }
            catch (IOException e)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "ERROR:  Service Configuration Error for Service=" +
                    fServiceName + lineSep +
                    "STAX::loadExtensions() - ERROR: " +
                    "Extension jar file: " + jarFileName + lineSep +
                    "Unable to access manifest in the extension jar file." +
                    lineSep + e.toString());
            }

            Map manifestEntryMap = manifest.getEntries();

            boolean allElements = true;

            Map includeElementMap = ext.getIncludeElementMap();
            Map excludeElementMap = ext.getExcludeElementMap();

            if (includeElementMap.size() != 0)
            {
                allElements = false;

                // Verify that all specified elements have a manifest entry
                // called staf/stax/extension/<element> in the jar file

                Iterator elemIter = includeElementMap.keySet().iterator();
                while (elemIter.hasNext())
                {
                    String elementName = (String)elemIter.next();
                    if (!manifestEntryMap.containsKey(
                        STAX_EXTENSION + elementName))
                    {
                        return new STAFResult(
                            STAFResult.ServiceConfigurationError,
                            "ERROR:  Service Configuration Error for Service=" +
                            fServiceName + lineSep +
                            "STAX::loadExtensions() - Extension jar file: " +
                            jarFileName + lineSep +
                            "Include element '" + elementName +
                            "' does not have a " + STAX_EXTENSION +
                            " manifest entry in the extension jar file.");
                    }
                }
            }
            else if (excludeElementMap.size() != 0)
            {
                allElements = false;

                // Verify that all specified elements have a manifest entry
                // called staf/stax/extension/<element> in the jar file

                Iterator elemIter = ext.getExcludeElementMap().
                    keySet().iterator();

                while (elemIter.hasNext())
                {
                    String elementName = (String)elemIter.next();
                    if (!manifestEntryMap.containsKey(
                        STAX_EXTENSION + elementName))
                    {
                        return new STAFResult(
                            STAFResult.ServiceConfigurationError,
                            "ERROR:  Service Configuration Error for Service=" +
                            fServiceName + lineSep +
                            "STAX::loadExtensions() - Extension jar file: " +
                            jarFileName + lineSep +
                            "Exclude element '" + elementName +
                            "' does not have a " + STAX_EXTENSION +
                            " manifest entry in the extension jar file.");
                    }
                }
            }

            // Create a class loader used to load STAX so it can be
            // passed to the STAXExtensionClassLoader to load STAX classes

            Class c = this.getClass();
            ClassLoader parentClassLoader = c.getClassLoader();

            // Iterate through the staf/stax/extension entries in the manifest

            Iterator mfIter = manifestEntryMap.keySet().iterator();

            // Load the extension class

            ClassLoader loader = new STAXExtensionClassLoader(jarFile,
                                                     parentClassLoader);

            while (mfIter.hasNext())
            {
                String elementName = (String)mfIter.next();

                if (elementName.startsWith(STAX_EXTENSION_INFO))
                {
                    Attributes attrs = manifest.getAttributes(
                        STAX_EXTENSION_INFO);

                    String extVersion = attrs.getValue("Extension-Version");

                    if (extVersion != null)
                    {
                        ext.setExtVersion(extVersion);
                    }

                    String description = attrs.getValue(
                        "Extension-Description");

                    if (description != null)
                    {
                        ext.setExtDescription(description);
                    }

                    String serviceVersion = attrs.getValue(
                        "Required-Service-Version");

                    if (serviceVersion != null)
                    {
                        ext.setRequiredServiceVersion(serviceVersion);

                        STAFVersion staxVersion = new STAFVersion(fVersion);
                        STAFVersion requiredVersion;

                        try
                        {
                            requiredVersion = new STAFVersion(serviceVersion);

                            if (staxVersion.compareTo(requiredVersion) < 0)
                            {
                                return new STAFResult(
                                    STAFResult.ServiceConfigurationError,
                                    "ERROR:  Service Configuration Error " +
                                    "for Service=" + fServiceName + lineSep +
                                    "STAX::loadExtensions() - " +
                                    "Extension jar file: " + jarFileName +
                                    lineSep + "Required-Service-Version: " +
                                    requiredVersion + lineSep +
                                    "STAX Service Version: " + staxVersion +
                                    lineSep + "The required STAX service " +
                                    "version is not installed.");
                            }
                        }
                        catch (NumberFormatException e)
                        {
                            return new STAFResult(
                                STAFResult.ServiceConfigurationError,
                                "ERROR:  Service Configuration Error for " +
                                "Service=" + fServiceName + lineSep +
                                "STAX::loadExtensions() - " +
                                "Extension jar file: " + jarFileName +
                                lineSep + "Invalid value specified for " +
                                "Required-Service-Version: " +
                                serviceVersion + lineSep + e.toString());
                        }
                    }

                    String monitorVersion = attrs.getValue(
                        "Required-Monitor-Version");

                    if (monitorVersion != null)
                    {
                        ext.setRequiredMonitorVersion(monitorVersion);
                    }

                    continue;
                }
                else if (elementName.startsWith(STAX_MONITOR_EXTENSION))
                {
                    String monitorExtName = elementName.substring(
                        STAX_MONITOR_EXTENSION.length());

                    Attributes attrs = manifest.getAttributes(
                        STAX_MONITOR_EXTENSION + monitorExtName);

                    String monitorExtClass = attrs.getValue(
                        "Extension-Class");

                    ext.setMonitorExtension(monitorExtName, monitorExtClass);

                    continue;  // No further processing is done on
                               // monitor extensions in STAX
                }
                else if (!elementName.startsWith(STAX_EXTENSION))
                    continue; // Ignore these entries

                elementName = elementName.substring(STAX_EXTENSION.length());

                // Put excluded elements in the unsupported element map
                // and continue

                if (!allElements)
                {
                    if (includeElementMap.size() != 0)
                    {
                        if (!includeElementMap.containsKey(elementName))
                        {
                            ext.setUnsupportedElement(elementName);
                            continue;  // Skip this element
                        }
                    }
                    else if (excludeElementMap.size() != 0)
                    {
                        if (excludeElementMap.containsKey(elementName))
                        {
                            ext.setUnsupportedElement(elementName);
                            continue;  // Skip this element
                        }
                    }
                }

                Attributes attrs = manifest.getAttributes(STAX_EXTENSION +
                                                          elementName);

                if (!attrs.containsKey(new Attributes.Name("Factory-Class")))
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Element '" + elementName + "' is missing attribute " +
                        "Factory-Class in the manifest for the " +
                        "extension jar file.");
                }

                String factoryClassName = attrs.getValue("Factory-Class");

                if (extElementMap.containsKey(elementName))
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Extension element '" + elementName +
                        "' is already registered with STAX and cannot " +
                        "be registerd twice.");
                }

                if (fActionFactoryMap.containsKey(elementName))
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Extension element '" + elementName + "'" +
                        " is already defined by STAX and cannot be replaced.");
                }

                extElementMap.put(elementName, factoryClassName);

                Class extFactoryClass = null;
                Object extFactoryObj = null;

                try
                {
                    extFactoryClass = loader.loadClass(factoryClassName);

                    // Try using a constructor that accepts a STAX object
                    // and a Map object

                    Class [] parameterTypes = new Class[2];
                    parameterTypes[0] = this.getClass();
                    parameterTypes[1] = Class.forName("java.util.Map");

                    try
                    {
                        Constructor construct =
                            extFactoryClass.getConstructor(parameterTypes);
                        Object [] initArgs = new Object[2];
                        initArgs[0] = this;
                        initArgs[1] = ext.getParmMap();
                        extFactoryObj = construct.newInstance(initArgs);
                    }
                    catch (NoSuchMethodException e)
                    {
                        if (ext.getParmMap().size() != 0)
                        {
                            return new STAFResult(
                                STAFResult.ServiceConfigurationError,
                                "ERROR:  Service Configuration Error for " +
                                "Service=" + fServiceName + lineSep +
                                "STAX::loadExtensions() - " +
                                "Extension jar file: " + jarFileName + lineSep +
                                "InvocationTargetException instantiating " +
                                "class: " + factoryClassName + lineSep +
                                "The factory class constructor does not " +
                                "support passing in a parameter map: " +
                                ext.getParmMap().toString() +
                                lineSep + e.toString());
                        }

                        // Try using a constructor that accepts a STAX object

                        parameterTypes = new Class[1];
                        parameterTypes[0] = this.getClass();

                        try
                        {
                            Constructor construct = extFactoryClass.
                                getConstructor(parameterTypes);
                            Object [] initArgs = new Object[1];
                            initArgs[0] = this;
                            extFactoryObj = construct.newInstance(initArgs);
                        }
                        catch (NoSuchMethodException e2)
                        {
                            // Extension does not have a constructor that
                            // accepts a STAX object, so try constructor
                            // without parameters
                            extFactoryObj = extFactoryClass.newInstance();
                        }
                        catch (InvocationTargetException e2)
                        {
                            String errorMsg = "ERROR:  Service Configuration" +
                                " Error for Service=" + fServiceName + lineSep +
                                "STAX::loadExtensions() - " +
                                "Extension jar file: " + jarFileName + lineSep +
                                "InvocationTargetException instantiating " +
                                "class: " + factoryClassName;

                            if (e2.getTargetException() != null)
                            {
                                errorMsg += lineSep + "Caught " +
                                    e2.getTargetException().toString();
                            }

                            return new STAFResult(
                                STAFResult.ServiceConfigurationError, errorMsg);
                        }
                        catch (IllegalArgumentException e2)
                        {
                            return new STAFResult(
                                STAFResult.ServiceConfigurationError,
                                "ERROR:  Service Configuration Error for " +
                                "Service=" + fServiceName + lineSep +
                                "STAX::loadExtensions() - Extension jar file: " +
                                jarFileName + lineSep +
                                "IllegalArgumentException instantiating " +
                                "class: " + factoryClassName + lineSep +
                                e2.toString());
                        }
                    }
                    catch (InvocationTargetException e)
                    {
                        String errorMsg = "ERROR:  Service Configuration " +
                            "Error for Service=" + fServiceName + lineSep +
                            "STAX::loadExtensions() - " +
                            "Extension jar file: " + jarFileName + lineSep +
                            "Exception instantiating class: " +
                            factoryClassName;

                        if (e.getTargetException() != null)
                        {
                            errorMsg += lineSep + "Caught " +
                                        e.getTargetException().toString();
                        }

                        return new STAFResult(
                            STAFResult.ServiceConfigurationError, errorMsg);
                    }
                    catch (IllegalArgumentException e)
                    {
                        return new STAFResult(
                            STAFResult.ServiceConfigurationError,
                            "ERROR:  Service Configuration Error for Service=" +
                            fServiceName + lineSep +
                            "STAX::loadExtensions() - Extension jar file: " +
                            jarFileName + lineSep +
                            "IllegalArgumentException instantiating " +
                            "class: " + factoryClassName + lineSep +
                            e.toString());
                    }
                }
                catch (ClassNotFoundException e)
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Cannot find class " + factoryClassName + lineSep +
                        e.toString());
                }
                catch (InstantiationException e)
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Could not instantiate class: " + factoryClassName +
                        lineSep + e.toString());
                }
                catch (IllegalAccessException e)
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Illegal access to class: " + factoryClassName +
                        lineSep + e.toString());
                }

                // Add to fActionFactoryMap
                String taskName = ((STAXActionFactory)extFactoryObj).
                                  getDTDTaskName();

                if ((taskName != null) && (!taskName.equals(elementName)))
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "ERROR:  Service Configuration Error for Service=" +
                        fServiceName + lineSep +
                        "STAX::loadExtensions() - Extension jar file: " +
                        jarFileName + lineSep +
                        "Extension element '" + elementName +
                        "' does not match its getDTDTaskName() of '" +
                        taskName + "'.");
                }

                fActionFactoryMap.put(elementName, extFactoryObj);

                // Add to the support element list for this extension
                ext.setSupportedElement(elementName, factoryClassName);

                // Add to the map of all the registered extension elements
                fExtensionElementMap.put(elementName, jarFileName);
            }
        }

        return new STAFResult(STAFResult.Ok, "");
    }

    private STAFResult logToServiceLog(String level, String message)
    {
        String logName = fServiceName.toUpperCase() + "_Service";

        String logRequest = "LOG MACHINE LOGNAME " +
                            STAFUtil.wrapData(logName) +
                            " LEVEL " + level +
                            " MESSAGE " + STAFUtil.wrapData(message);

        STAFResult result = fHandle.submit2(STAFHandle.ReqSync,
                                            "LOCAL", "LOG", logRequest);

        // Check if the result was unsuccesful (except ignore RC 2 and 33)
        if (result.rc != 0 && result.rc != 2 && result.rc != 33)
        {
            System.out.println("STAX::logToServiceLog failed with RC " +
                               result.rc + " and Result " + result.result +
                               "  level: " + level +
                               "  logRequest: " + logRequest);
        }

        return result;
    }

    private List createShortMarshalledList(Map variableMap)
    {
        List marshalledList = new ArrayList();
        try
        {
            Set keys = variableMap.keySet();
            Iterator keysIter = keys.iterator();

            while (keysIter.hasNext())
            {
                String name = (String)keysIter.next();
                Object value = variableMap.get(name);
                String stringValue = value.toString();
                String type = value.getClass().getName();

                if (value instanceof PyString)
                {
                    stringValue = "'" + stringValue + "'";
                }

                Map breakpointVariableMap =
                    fThreadVariableMapClass.createInstance();
                breakpointVariableMap.put("name", name);
                breakpointVariableMap.put("type", type);
                breakpointVariableMap.put("value", stringValue);

                marshalledList.add(breakpointVariableMap);
            }

            return marshalledList;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return marshalledList;
        }
    }

    private List createLongMarshalledList(Map variableMap)
    {
        List marshalledList = new ArrayList();
        try
        {
            Set keys = variableMap.keySet();
            Iterator keysIter = keys.iterator();

            while (keysIter.hasNext())
            {
                String name = (String)keysIter.next();
                Object value = variableMap.get(name);

                marshalledList.add(
                    addVariable(name, value.getClass().getName(), value));
            }

            return marshalledList;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return marshalledList;
        }
    }

    private Map addVariable(String name, String type, Object obj)
    {
        if ((obj instanceof PyList) ||
            (obj instanceof PyTuple))
        {
            if (((PySequence)obj).__len__() == 0)
            {
                Map breakpointVariableMap =
                    fThreadVariableMapClass.createInstance();
                breakpointVariableMap.put("name", name);
                breakpointVariableMap.put("type", type);
                breakpointVariableMap.put("value", new ArrayList());

                return breakpointVariableMap;
            }
            else
            {
                List varList = new ArrayList();

                for (int i = 0; i < ((PySequence)obj).__len__(); i++)
                {
                    PyObject childObj = ((PySequence)obj).__finditem__(i);
                    varList.add(addVariable(" ",
                                            childObj.getClass().getName(),
                                            childObj));
                }

                Map breakpointVariableMap =
                    fThreadVariableMapClass.createInstance();
                breakpointVariableMap.put("name", name);
                breakpointVariableMap.put("type", type);
                breakpointVariableMap.put("value", varList);

                return breakpointVariableMap;
            }
        }
        else if (obj instanceof PyDictionary)
        {
            // Get a copy of the dicitonary, since we will be calling popitem
            obj = ((PyDictionary)obj).copy();

            List varList = new ArrayList();

            while (((PyDictionary)obj).__len__() > 0)
            {
                PyTuple dictTuple = (PyTuple)((PyDictionary)obj).popitem();
                PyObject dictObj = dictTuple.__getitem__(1);

                if ((dictObj instanceof PyDictionary) ||
                    (dictObj instanceof PyList) ||
                    (dictObj instanceof PyTuple))
                {
                    varList.add(addVariable(
                        dictTuple.__getitem__(0).toString(),
                        dictTuple.__getitem__(1).getClass().getName(),
                        dictTuple.__getitem__(1)));
                }
                else
                {
                    Map breakpointVariableMap =
                        fThreadVariableMapClass.createInstance();
                    breakpointVariableMap.put("name",
                        dictTuple.__getitem__(0).toString());
                    breakpointVariableMap.put("type",
                        dictTuple.__getitem__(1).getClass().getName());

                    if (dictObj instanceof PyString)
                    {
                        breakpointVariableMap.put("value", "'" +
                                                  dictObj.toString() + "'");
                    }
                    else
                    {
                        breakpointVariableMap.put("value", dictObj);
                    }

                    varList.add(breakpointVariableMap);
                }
            }

            Map breakpointVariableMap =
                fThreadVariableMapClass.createInstance();
            breakpointVariableMap.put("name", name);
            breakpointVariableMap.put("type", type);
            breakpointVariableMap.put("value", varList);

            return breakpointVariableMap;
        }
        else
        {
            Map breakpointVariableMap =
                fThreadVariableMapClass.createInstance();
            breakpointVariableMap.put("name", name);
            breakpointVariableMap.put("type", type);

            if (obj instanceof PyString)
            {
                breakpointVariableMap.put("value", "'" + obj.toString() + "'");
            }
            else
            {
                breakpointVariableMap.put("value", obj);
            }

            return breakpointVariableMap;
        }
    }

    // Register error codes for the STAX Service with the HELP service

    private void registerHelpData(int errorNumber, String info,
                                 String description)
    {
        STAFResult res = fHandle.submit2("local", "HELP",
                         "REGISTER SERVICE " + fServiceName +
                         " ERROR " + errorNumber +
                         " INFO " + STAFUtil.wrapData(info) +
                         " DESCRIPTION " + STAFUtil.wrapData(description));
    }

    // Un-register error codes for the STAX Service with the HELP service

    private void unregisterHelpData(int errorNumber)
    {
        STAFResult res = fHandle.submit2("local", "HELP",
                         "UNREGISTER SERVICE " + fServiceName +
                         " ERROR " + errorNumber);
    }

    public void registerJobManagementHandler(STAXJobManagementHandler handler)
    {
        synchronized (fJobManagementHandlerSet)
        {
            fJobManagementHandlerSet.add(handler);
        }
    }

    public void unregisterJobManagementHandler(STAXJobManagementHandler handler)
    {
        synchronized (fJobManagementHandlerSet)
        {
            fJobManagementHandlerSet.remove(handler);
        }
    }

    public void visitJobManagementHandlers(STAXVisitor visitor)
    {
        synchronized (fJobManagementHandlerSet)
        {
            Iterator iter = fJobManagementHandlerSet.iterator();

            while (iter.hasNext())
            {
                visitor.visit(iter.next(), iter);
            }
        }
    }

    public void registerListHandler(String type, STAXListRequestHandler handler)
    {
        synchronized (fListRequestMap)
        {
            if (fListRequestMap.containsKey(type))
            {
                System.out.println("In STAX.registerListHandler: type=" +
                    type + " is already registered with handler " +
                    fListRequestMap.get(type));
            }

            fListRequestMap.put(type.toUpperCase(), handler);

            String optionHelp = " | " + type.toUpperCase();

            // Add line separators to the LIST JOB options to keep each line
            // in the help text <= 78 characters for "prettier" formatting
            // 54 is 78 minus the length of "          JOB <Job ID> [" which
            // will be in the first line of help text for the job options

            int maxLength;

            if (fHelpListJobOptions.indexOf(lineSep) == -1)
                maxLength = 54;
            else
                maxLength = 78;

            fHelpListJobOptions = addJobOptionToHelpText(
                fHelpListJobOptions, optionHelp, maxLength);
        }
    }

    public void registerQueryHandler(String type, String typeValue,
                                     STAXQueryRequestHandler handler)
    {
        synchronized (fQueryRequestMap)
        {
            if (fQueryRequestMap.containsKey(type))
            {
                System.out.println("In STAX.registerQueryHandler: type=" +
                    type + " is already registered with handler " +
                    fQueryRequestMap.get(type));
            }

            fQueryRequestMap.put(type.toUpperCase(), handler);

            if (typeValue.equals("") || typeValue.equals(null))
                typeValue = "Value";

            String optionHelp = " | " + type.toUpperCase() +
                " <" + typeValue + ">";

            // Add line separators to the QUERY JOB options to keep each line
            // in the help text <= 78 characters for "prettier" formatting
            // 54 is 78 minus the length of "          JOB <Job ID> [" which
            // will be in the first line of help text for the job options

            int maxLength;

            if (fHelpQueryJobOptions.indexOf(lineSep) == -1)
                maxLength = 54;
            else
                maxLength = 78;

            fHelpQueryJobOptions = addJobOptionToHelpText(
                fHelpQueryJobOptions, optionHelp, maxLength);
        }
    }

    /**
     *  Add line separators to the Query Job Options to keep each line in
     * the help text <= 78 characters for "prettier" formatting
     * 
     * @param helpText the help text generated so far for the options
     * @param optionHelp the help text to add for the new option
     * @param maxLength the maximum length for the help text line
     */ 
    private String addJobOptionToHelpText(String helpText, String optionHelp,
                                          int maxLength)
    {
        String indentString = "                       ";  // 23 spaces
        int lineSepLength = lineSep.length();
        String result = "";
        String lastLine = ""; 

        int index = helpText.lastIndexOf(lineSep);

        if (index != -1)
        {
            result = helpText.substring(0, index + lineSepLength);
            lastLine = helpText.substring(index + lineSepLength);
        }
        else
        {
            lastLine = helpText;
        }

        lastLine += optionHelp;

        while (lastLine.length() > maxLength)
        {
            index = lastLine.substring(0, maxLength).lastIndexOf(" |");

            if (index != -1)
            {
                result += lastLine.substring(0, index + 2) +
                    lineSep + indentString;
                lastLine = lastLine.substring(index + 2);
            }
            else
            {
                break;
            }
        }

        return (result + lastLine);
    }

    public void registerQueryJobHandler(STAXQueryRequestHandler handler)
    {
        synchronized (fQueryJobRequestList)
        {
            fQueryJobRequestList.add(handler);
        }
    }

    public int registerGenericRequestHandler(STAXGenericRequestHandler handler,
                                             Class serviceInterfaceClass)
    {
        synchronized (fGenericRequestList)
        {
            // Check if supports service interface level class specified

            if (!serviceInterfaceClass.getName().equals(INTERFACE_LEVEL_30))
            {
                return 1;
            }

            fGenericRequestList.add(new STAXGenericRequest(handler,
                                                    serviceInterfaceClass));

            fHelpGenericRequests += handler.getHelpInfo(lineSep) +
                                    lineSep + lineSep;
        }

        return 0;
    }

    // Helper class for processing the Generic Request List

    class STAXGenericRequest
    {
        STAXGenericRequest(STAXGenericRequestHandler handler,
                           Class serviceInterfaceClass)
        {
            fHandler = handler;
            fServiceInterfaceClass = serviceInterfaceClass;
        }

        private STAXGenericRequestHandler fHandler;
        private Class fServiceInterfaceClass;
    }

    private String fServiceName;
    private String fDataDir;
    private String fInstanceUUID = "";
    private String fEventServiceMachine = "";
    private String fEventServiceName = "";
    private String fLocalMachineName = "";
    private String fLocalMachineNickname = "";
    private String fJythonVersion = "";
    private int fNumThreads;
    private int fProcessTimeout = 60000;
    private long fMaxReturnFileSize = 0;  // 0 = No maximum return file size
    private int fMaxGetQueueMessages = 25;
    private int fMaxSTAXThreads = 0;      // 0 = No maximum # of STAX-Threads
    private boolean fClearLogs = false;
    private boolean fLogTCElapsedTime = true;
    private boolean fLogTCNumStarts = true;
    private boolean fLogTCStartStop = false;
    private int fPythonOutput = STAXPythonOutput.JOBUSERLOG;
    private String fPythonLogLevel = STAXPythonOutput.LOGLEVEL_DEFAULT;
    private boolean fFileCaching = true;
    private List fExtensionList = new ArrayList();
    private String fExtensionFile = null;
    private STAFHandle fHandle = null;
    private STAFCommandParser fParmsParser = new STAFCommandParser();
    private STAFCommandParser fExecuteParser = new STAFCommandParser();
    private STAFCommandParser fListParser = new STAFCommandParser();
    private STAFCommandParser fQueryParser = new STAFCommandParser();
    private STAFCommandParser fVersionParser = new STAFCommandParser();
    private STAFCommandParser fGetParser = new STAFCommandParser();
    private STAFCommandParser fSetParser = new STAFCommandParser();
    private STAFCommandParser fNotifyParser = new STAFCommandParser();
    private STAFCommandParser fPurgeParser = new STAFCommandParser();
    private STAFCommandParser fStopParser = new STAFCommandParser();
    private STAFCommandParser fPyExecParser = new STAFCommandParser();
    private STAFCommandParser fHelpParser = new STAFCommandParser();
    private STAFMapClassDefinition fJobResultMapClass;
    private STAFMapClassDefinition fJobDetailedResultMapClass;
    private STAFMapClassDefinition fJobDetailsMapClass;
    private STAFMapClassDefinition fFunctionInfoMapClass;
    private STAFMapClassDefinition fArgInfoMapClass;
    private STAFMapClassDefinition fArgPropertyInfoMapClass;
    private STAFMapClassDefinition fArgPropertyDataInfoMapClass;
    private STAFMapClassDefinition fJobInfoMapClass;
    private STAFMapClassDefinition fFileCacheMapClass;
    private STAFMapClassDefinition fFileCacheSummaryMapClass;
    private STAFMapClassDefinition fMachineCacheMapClass;
    private STAFMapClassDefinition fPurgeStatsMapClass;
    private STAFMapClassDefinition fQueryJobMapClass;
    private STAFMapClassDefinition fThreadInfoMapClass;
    private STAFMapClassDefinition fThreadLongInfoMapClass;
    private STAFMapClassDefinition fQueryThreadMapClass;
    private STAFMapClassDefinition fThreadVariableMapClass;
    private STAFMapClassDefinition fSettingsMapClass;
    private STAFMapClassDefinition fNotifieeMapClass;
    private STAFMapClassDefinition fAllNotifieeMapClass;
    private STAFMapClassDefinition fExtensionElementMapClass;
    private STAFMapClassDefinition fExtensionJarFileMapClass;
    private STAFMapClassDefinition fExtensionInfoMapClass;
    private STAFMapClassDefinition fServiceExtensionMapClass;
    private STAFMapClassDefinition fMonitorExtensionMapClass;
    protected STAFMapClassDefinition fResultMapClass;
    protected STAFMapClassDefinition fDetailedResultMapClass;
    protected static STAFMapClassDefinition fExecuteErrorResultMapClass;
    private Object fNextJobNumberSynch = new Object();
    private int fNextJobNumber = 1;
    private HashMap fActionFactoryMap = new HashMap();
    private TreeMap fJobMap = new TreeMap();
    private STAXThreadQueue fThreadQueue = null;
    private STAXTimedEventQueue fTimedEventQueue = new STAXTimedEventQueue();
    private String fDTD = "";
    private HashMap fListRequestMap = new HashMap();
    private HashMap fQueryRequestMap = new HashMap();
    private ArrayList fQueryJobRequestList = new ArrayList();
    private ArrayList fGenericRequestList = new ArrayList();
    private TreeSet fJobManagementHandlerSet =
                    new TreeSet(new STAXObjectComparator());
    private String fHelpListJobOptions =
        "THREADS [LONG] | < THREAD <Thread ID> VARS [SHORT] >";
    private String fHelpQueryJobOptions =
        "THREAD <Thread ID> [ VAR <VarName> [SHORT] ]";
    private String fHelpGenericRequests = "";
    private String fJythonLibName = "";  // Jython Library directory name
    private List fExtensionsJarList = new ArrayList();
    private Map fExtensionElementMap = new TreeMap();
    private ArrayList fExecuteWaitList = new ArrayList();
}
