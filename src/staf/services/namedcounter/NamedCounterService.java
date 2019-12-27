/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namedcounter;

import com.ibm.staf.*;
import com.ibm.staf.service.*;

import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.text.SimpleDateFormat;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.StringWriter;

/** STAF Named Counter Serivce, providing a mechanism for managing
 * Named Counters across a testing environment.
 * @author Karl Donald
 * @version 1.1.0
 */

public class NamedCounterService implements STAFServiceInterfaceLevel30
{
    /** The STAF Handle that the service uses itself
     */
    private STAFHandle stafHandle;

    /** Help text for the service
     */ 
    private static String sHelpMsg;

    /** Line separator for the local service machine
     */
    private static String sLineSep;

    /** Error code for a request for a counter which doesn't exist
     */
    static final int CounterNotExist = 4001;

    /** Return code for a counter which is at Zero and as such
     * can't be decremented
     */
    static final int CounterAlreadyZero = 4002;

    /** Return code for is for when a counter hits the maximum
     * value and is unable to increment
     */
    static final int CounterAtMax = 4003;
    
    /** The structure in which all of the named counters are stored
     */
    private HashMap counters;

    /** Flag to check whether to persist the named counters or not
     */
    private boolean persist;

    /** The name that the service has been assigned
     */
    private String fServiceName;

    /** The local machine's logical identifier
     */
    private String fLocalMachineName;

    /** Name of the directory to which the service can write persistent data
     */
    private String fDataDir;

    /** Fully-qualified name of file containing the service's persistent data
     */
    private String fNamedCounterFileName;

    /** The Parser which is used to process the STAF request
     */
    STAFCommandParser parser = null;

    private final String serviceVersion = "3.0.2";

    /** The format for the timestamp used when logging an error message
     */ 
    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    /** Construct an instance of the NamedCounterService
     */
    public NamedCounterService() {
        counters = new HashMap();
    }

    /** Loads the counters from the file that they were persisted to
     */
    private void loadCounters()
    {
        try
        {
            FileInputStream fi = new FileInputStream(fNamedCounterFileName);
            ObjectInputStream si = new ObjectInputStream(fi);
            counters = (HashMap) si.readObject();
        }
        catch (FileNotFoundException e1)
        {
            System.err.println(
                "NamedCounterService:Error: Named Counters " +
                "File not Found for service: " + fNamedCounterFileName);
            // Not a problem, will create a new one on exit.
            // The object has already been initialised
        }
        catch (Exception e2)
        {
            System.err.println(
                "NamedCounterService:Error: Exception caught " +
                "whilst loading counters");
            System.err.println("                         : " +
                e2.getMessage());
            e2.printStackTrace();
        }
    }

    /** Save the counters to a file
     * @return The STAFResult object which is the result of the request
     */
    private boolean saveCounters()
    {
        boolean result = true;

        try
        {
            FileOutputStream fo = new FileOutputStream(fNamedCounterFileName);
            ObjectOutputStream so = new ObjectOutputStream(fo);
            so.writeObject(counters);
            so.flush();
        }
        catch (Exception e)
        {
            result = false;
            System.err.println(
                "NamedCounterService:Error: Exception caught " +
                "whilst saving counters in file " + fNamedCounterFileName);
            System.err.println("                         : " +
                e.getMessage());
            e.printStackTrace();
        }

        return result;
    }


    // Register error codes for the service with the HELP service

    private void registerHelpData(int errorNumber, String info,
                                 String description)
    {
        STAFResult res = stafHandle.submit2("local", "HELP",
                         "REGISTER SERVICE " + fServiceName +
                         " ERROR " + errorNumber +
                         " INFO " + STAFUtil.wrapData(info) +
                         " DESCRIPTION " + STAFUtil.wrapData(description));
    }

    // Un-register error codes for the service with the HELP service

    private void unregisterHelpData(int errorNumber)
    {
        STAFResult res = stafHandle.submit2("local", "HELP",
                         "UNREGISTER SERVICE " + fServiceName +
                         " ERROR " + errorNumber);
    }


    /***************************************************************/
    /* init - Initializes NamedCounter Service with STAF.          */
    /* Instantiates all parsers.                                   */
    /* Registers help for service error codes.                     */
    /*                                                             */
    /* accepts: STAFServiceInterfaceLevel30 initialization info    */
    /*                                                             */
    /* Returns: STAFResult.Ok or STAFResult.STAFRegistrationError  */
    /***************************************************************/
    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        fServiceName = info.name;

        try
        {
            parser = new STAFCommandParser(2, false);

            // Add some options that are permitted in command statements
            parser.addOption("INC", 1, STAFCommandParser.VALUEREQUIRED);
            parser.addOption("DEC", 1, STAFCommandParser.VALUEREQUIRED);
            parser.addOption("GET", 1, STAFCommandParser.VALUEREQUIRED);
            parser.addOption("RESET", 1, STAFCommandParser.VALUEREQUIRED);
            parser.addOption("DELETE", 1, STAFCommandParser.VALUEREQUIRED);
            parser.addOption("LIST", 1, STAFCommandParser.VALUENOTALLOWED);
            parser.addOption("HELP", 1, STAFCommandParser.VALUENOTALLOWED);
            parser.addOption("VERSION", 1, STAFCommandParser.VALUENOTALLOWED);

            // At least one and only one of these must appear
            parser.addOptionGroup("INC DEC GET RESET DELETE LIST HELP VERSION",
                1, 1);

            // Get a STAFHandle for NamedCounterService <name>
            stafHandle = new STAFHandle("STAF/Service/" + fServiceName);
            STAFResult persistRes =
                stafHandle.submit2(
                    "local",
                    "var",
                    "get system var STAF/Service/" + fServiceName + "/Persist");

            if ((persistRes.rc == 0)
                & (persistRes.result.toUpperCase().equals("TRUE")))
            {
                persist = true;

                // Get the file separator for the local machine

                String fileSep = "/";

                STAFResult res = STAFUtil.resolveInitVar(
                    "{STAF/Config/Sep/File}", stafHandle);

                if (res.rc == STAFResult.Ok) fileSep = res.result;

                // Store data for the service in directory:
                //   <STAF writeLocation>/service/<service name (lower-case)>

                fDataDir = info.writeLocation;

                if (!fDataDir.endsWith(fileSep))
                {
                    fDataDir += fileSep;
                }

                fDataDir = fDataDir + "service" + fileSep +
                    fServiceName.toLowerCase();

                File dir = new File(fDataDir);

                if (!dir.exists())
                {
                    dir.mkdirs();
                }

                fNamedCounterFileName = fDataDir + fileSep + "namedCounter.ser";

                loadCounters();
            }
            else
            {
                persist = false;
            }

            // Resolve the machine name variable for the local machine

            STAFResult res = STAFUtil.resolveInitVar(
                "{STAF/Config/Machine}", stafHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLocalMachineName = res.result;
            
            // Resolve the line separator for the local machine

            res = STAFUtil.resolveInitVar(
                "{STAF/Config/Sep/Line}", stafHandle);

            if (res.rc != STAFResult.Ok) return res;

            sLineSep = res.result;
            
            // Assign the help text string for the service

            sHelpMsg = "*** " + fServiceName + " Service Help ***" +
                sLineSep + sLineSep +
                "INC <Name>" +
                sLineSep +
                "DEC <Name>" +
                sLineSep +
                "GET <Name>" +
                sLineSep +
                "RESET <Name>" +
                sLineSep +
                "DELETE <Name>" +
                sLineSep +
                "LIST" +
                sLineSep +
                "VERSION" +
                sLineSep +
                "HELP";

            // Register Help Data

            registerHelpData(
                CounterNotExist,
                "Counter Does Not Exist",
                "The specified named counter could not be found.");

            registerHelpData(
                CounterAlreadyZero,
                "Counter Already Zero",
                "The named counter is currently at Zero.");

            registerHelpData(
                CounterAtMax,
                "Counter At Max",
                "The named counter is at its maximum possible value.");

            return new STAFResult(STAFResult.Ok);
        }
        catch (STAFException e)
        {
            System.err.println(
                "NamedCounterService:Error: Exception caught " +
                "whilst initializing service");
            System.err.println("                         : " +
                e.getMessage());
            e.printStackTrace();

            return new STAFResult(STAFResult.STAFRegistrationError,
                                  e.toString());
        }
    }

    /** Process a HELP request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionHELP(STAFCommandParseResult myParsedRequest)
    {
        return new STAFResult(STAFResult.Ok, sHelpMsg);
    }

    /** Process a VERSION request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionVERSION(STAFCommandParseResult myParsedRequest)
    {
        return new STAFResult(STAFResult.Ok, serviceVersion);
    }

    /** Process a GET request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionGET(STAFCommandParseResult myParsedRequest)
    {
        synchronized(counters)
        {
            if (counters.containsKey(myParsedRequest.optionValue("GET")))
            {
                Integer i = (Integer) counters.get(
                    myParsedRequest.optionValue("GET"));

                return new STAFResult(STAFResult.Ok, i.toString());
            }
            else
            {
                return new STAFResult(CounterNotExist,
                    "Counter does not exist");
            }
        }
    }

    /** Process a LIST request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionLIST(STAFCommandParseResult myParsedRequest)

    {
        STAFMarshallingContext mc = new STAFMarshallingContext();

        synchronized(counters)
        {
            // List all counters by creating returning a marshalled map of
            // the counters
            
            mc.setRootObject(counters);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
    }

    /** Process an INC request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionINC(STAFCommandParseResult myParsedRequest)
    {
        synchronized(counters)
        {
            if (counters.containsKey(myParsedRequest.optionValue("INC")))
            {
                Integer i = (Integer) counters.get(
                    myParsedRequest.optionValue("INC"));

                if (Integer.MAX_VALUE == i.intValue())
                {
                    return new STAFResult(CounterAtMax,
                        "Counter already at maximum");
                }
                else
                {
                    i = new Integer(i.intValue() + 1);
                    counters.put(myParsedRequest.optionValue("INC"), i);

                    return new STAFResult(STAFResult.Ok, i.toString());
                }
            }
            else
            {
                Integer i = new Integer(1);
                counters.put(myParsedRequest.optionValue("INC"), i);

                return new STAFResult(STAFResult.Ok, i.toString());
            }
        }
    }

    /** Process a DEC request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionDEC(STAFCommandParseResult myParsedRequest)
    {
        synchronized(counters)
        {
            if (counters.containsKey(myParsedRequest.optionValue("DEC")))
            {
                Integer i = (Integer) counters.get(
                    myParsedRequest.optionValue("DEC"));

                if (i.intValue() >= 1)
                {
                    i = new Integer(i.intValue() - 1);
                    counters.put(myParsedRequest.optionValue("DEC"), i);

                    return new STAFResult(STAFResult.Ok, i.toString());
                }
                else
                {
                    return new STAFResult(CounterAlreadyZero,
                        "Counter already at zero");
                }
            }
            else
            {
                return new STAFResult(CounterNotExist, "Counter does not exist");
            }
        }
    }

    /** Process a RESET request
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionRESET(STAFCommandParseResult myParsedRequest)
    {
        synchronized(counters)
        {
            Integer i = new Integer(0);
            counters.put(myParsedRequest.optionValue("RESET"), i);

            return new STAFResult(STAFResult.Ok, i.toString());
        }
    }

    /** Process a DELETE reqest
     * @param myParsedRequest The pre-parsed result
     * @return The STAFResult object which is the result of the request
     */
    private STAFResult optionDELETE(STAFCommandParseResult myParsedRequest)
    {
        synchronized(counters)
        {
            if (counters.containsKey(myParsedRequest.optionValue("DELETE")))
            {
                counters.remove(myParsedRequest.optionValue("DELETE"));
            }

            return new STAFResult(STAFResult.Ok);
        }
    }

    /***************************************************************************/
    /* acceptRequest- Calls appropriate methods to process a request from a    */
    /*                client process.                                          */
    /*                                                                         */
    /* accepts: STAFServiceInterfaceLevel30 request information                */
    /*                                                                         */
    /* returns: STAFResult.rc = STAFResult.Ok, if successful; STAFResult.      */
    /*          InvalidRequestString, if unsuccessful;                         */
    /*                                                                         */
    /*          STAFResult.result contains the result returned by the called   */
    /*          method, if successful;                                         */
    /*          STAFResult.result contains the command, if unsuccessful        */
    /***************************************************************************/
    public STAFResult acceptRequest(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        STAFResult result;

        try
        {
            // Create a STAFCommandParseResult object to evaluate the
            // arguments passed to the service. This is done using the
            // STAFCommandParser, which was created in init().

            STAFCommandParseResult myParsedRequest = parser.parse(info.request);

            // If STAFEResult.rc is not 0, the argument string
            // contained an invalid argument

            if (myParsedRequest.rc != STAFResult.Ok)
            {
                result = new STAFResult(STAFResult.InvalidRequestString,
                    myParsedRequest.errorBuffer);
            }
            else
            {
                if (myParsedRequest.optionTimes("HELP") == 1)
                {
                    // Verify the requester has at least trust level 1

                    STAFResult trustResult = STAFUtil.validateTrust(
                        1, fServiceName, "HELP", fLocalMachineName, info);

                    if (trustResult.rc != STAFResult.Ok) return trustResult;

                    result = optionHELP(myParsedRequest);
                }
                else if (myParsedRequest.optionTimes("GET") == 1)
                {
                    // Verify the requester has at least trust level 1

                    STAFResult trustResult = STAFUtil.validateTrust(
                        1, fServiceName, "GET", fLocalMachineName, info);

                    if (trustResult.rc != STAFResult.Ok) return trustResult;

                    result = optionGET(myParsedRequest);
                }
                else if (myParsedRequest.optionTimes("LIST") == 1)
                {
                    // Verify the requester has at least trust level 1

                    STAFResult trustResult = STAFUtil.validateTrust(
                        1, fServiceName, "LIST", fLocalMachineName, info);

                    if (trustResult.rc != STAFResult.Ok) return trustResult;

                    result = optionLIST(myParsedRequest);
                }
                else if (myParsedRequest.optionTimes("VERSION") == 1)
                {
                    // Verify the requester has at least trust level 1

                    STAFResult trustResult = STAFUtil.validateTrust(
                        1, fServiceName, "VERSION", fLocalMachineName, info);

                    if (trustResult.rc != STAFResult.Ok) return trustResult;

                    result = optionVERSION(myParsedRequest);
                }
                else
                {
                    String action = "";

                    if (myParsedRequest.optionTimes("INC") == 1)
                        action = "INC";
                    else if (myParsedRequest.optionTimes("RESET") == 1)
                        action = "RESET";
                    else if (myParsedRequest.optionTimes("DELETE") == 1)
                        action = "DELETE";
                    else if (myParsedRequest.optionTimes("DEC") == 1)
                        action = "DEC";
                    else
                    {
                        // This should never be reached because it
                        // would mean that the parse had failed,
                        // however the following line is required
                        // so that the code can be compiled.
                        result = new
                            STAFResult(STAFResult.UnknownError);
                    }

                    // Verify the requester has at least trust level 4

                    STAFResult trustResult = STAFUtil.validateTrust(
                        4, fServiceName, action, fLocalMachineName, info);

                    if (trustResult.rc != STAFResult.Ok) return trustResult;

                    if (action.equals("INC"))
                        result = optionINC(myParsedRequest);
                    else if (action.equals("RESET"))
                        result = optionRESET(myParsedRequest);
                    else if (action.equals("DELETE"))
                        result = optionDELETE(myParsedRequest);
                    else
                        result = optionDEC(myParsedRequest);
                }
            }
        }
        catch (Throwable t)
        {
            // Write the Java stack trace to the JVM log for the service

            System.out.println(
                sTimestampFormat.format(Calendar.getInstance().getTime()) +
                " ERROR: Exception on " + fServiceName + " service request:" +
                sLineSep + sLineSep + info.request + sLineSep);

            t.printStackTrace();
            
            // And also return the Java stack trace in the result

            StringWriter sw = new StringWriter();
            t.printStackTrace(new PrintWriter(sw));

            if (t.getMessage() != null)
            {
                return new STAFResult(
                    STAFResult.JavaError,
                    t.getMessage() + sLineSep + sw.toString());
            }
            else
            {
                return new STAFResult(
                    STAFResult.JavaError, sw.toString());
            }
        }

        return result;
    }

    /** Terminate the service, and save any existing counters if
     * the option to persist them is set
     * @return Ok if saving the counters succeeded, or JavaError if
     * the save failed (this is assuming that the service has
     * been setup to persist the counters)
     */
    public STAFResult term()
    {
        boolean saveResult = true;

        // Un-register Help Data

        unregisterHelpData(CounterNotExist);
        unregisterHelpData(CounterAlreadyZero);
        unregisterHelpData(CounterAtMax);

        // If persisting counters
        if (persist)
        {
            saveResult = saveCounters();
        }

        // Un-register the service handle

        try
        {
            stafHandle.unRegister();
        }
        catch (STAFException ex)
        {
            if (saveResult == true)
            {
                return new STAFResult(STAFResult.STAFRegistrationError,
                                      ex.toString());
            }
        }

        // If the counters saved ok, (or we aren't persisting them)
        if (saveResult == true)
        {
            return new STAFResult(STAFResult.Ok);
            // Else something went wrong whilst saving the counters
        } else
        {
            return new STAFResult(STAFResult.JavaError, "Error saving counters.");
        }
    }
}
