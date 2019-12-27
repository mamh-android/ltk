/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.ftp;

import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.io.PrintWriter;
import java.io.StringWriter;

import com.ibm.staf.STAFException;
import com.ibm.staf.STAFHandle;
import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFUtil;
import com.ibm.staf.service.STAFCommandParseResult;
import com.ibm.staf.service.STAFCommandParser;
import com.ibm.staf.service.STAFServiceInterfaceLevel30;

public class FTPService implements STAFServiceInterfaceLevel30
{
    private static final String kVersion = "1.0.3";
    protected static final int kHostConnectError = 4001;

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    private static String sHelpMsg;
    private static String fLineSep = null;

    private String fServiceName = null;
    private STAFHandle fHandle;
    private String fLocalMachineName = "";
    
    private HelpRequestProcessor fHelpRequestProcessor;
    private VersionRequestProcessor fVersionRequestProcessor;
    private GetRequestProcessor fGetRequestProcessor;
    private PutRequestProcessor fPutRequestProcessor;
    private DirRequestProcessor fDirRequestProcessor;

    /**
     * FTPService contructor.
     */
    public FTPService() {}

    /**
     * This is the STAF Service initialization method that is run when the
     * service is registered.  It performs initialization functions such as:
     * <ul>
     * <li>Creates a STAF handle to use to submit requests to STAF services
     * <li>Resolves any local variables needed by the service
     * <li>Creates request parsers
     * <li>Registers help data for the service
     * </ul>
     * 
     * @param info STAF service initialization information
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the service initialized successfully.
     */ 
    public STAFResult init(InitInfo initInfo)
    {
        try
        {
            // initialize STAF service related stuff here
            
            this.fServiceName = initInfo.name;
            this.fHandle = new STAFHandle("STAF/Service/" + initInfo.name);
            STAFResult res = new STAFResult();
            
            // Resolve the line separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", fHandle);
            
            if (res.rc != STAFResult.Ok) return res;
            
            this.fLineSep = res.result;
            
            // Resolve the machine name variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);
            
            if (res.rc != STAFResult.Ok) return res;
            
            this.fLocalMachineName = res.result;
            
            fHelpRequestProcessor = new HelpRequestProcessor();
            fVersionRequestProcessor = new VersionRequestProcessor();
            fGetRequestProcessor = new GetRequestProcessor();
            fPutRequestProcessor = new PutRequestProcessor();
            fDirRequestProcessor = new DirRequestProcessor();
            
            // Assign the help text string for the service

            sHelpMsg = "*** " + fServiceName + " Service Help ***" +
                fLineSep + fLineSep +
                "GET HOST <Host> [PORT <Port>] URLPATH <FTP URL File Path> FILE <Name>" +
                fLineSep +
                "    [USER <Name> PASSWORD <Password>]" +
                fLineSep + fLineSep +
                "PUT HOST <Host> [PORT <Port>] URLPATH <FTP URL File Path> FILE <Name>" +
                fLineSep +
                "    [USER <Name> PASSWORD <Password>]" +
                fLineSep + fLineSep +
                "DIR HOST <Host> [PORT <Port>] URLPATH <FTP URL Directory Path>" +
                fLineSep +
                "    [USER <Name> PASSWORD <Password>]" +
                fLineSep + fLineSep +
                "VERSION" +
                fLineSep + fLineSep +
                "HELP";

            // Register help data for the service

            registerHelpData(
                kHostConnectError,
                "FTP host connection error",
                "Could not open a connection to the specified FTP host. " +
                "Additional information about the error is provided in " +
                "the result.");

        }
        catch (Exception e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  e.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }// end init

    /**
     * This is the STAF Service method that accepts requests that are
     * submitted to this service.  Based on the first option specifed in
     * the request, it calls the appropriate method that handles that request.
     * @param info STAF Service request information
     * @return An instance of STAFResult which contains the return code
     * and result buffer.
     */ 
    public STAFResult acceptRequest(RequestInfo info)
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

            if (actionLC.equals("get"))
            {
                return fGetRequestProcessor.handleGetRequest(info);
            }
            else if (actionLC.equals("put"))
            {
                return fPutRequestProcessor.handlePutRequest(info);
            }
            else if (actionLC.equals("dir"))
            {
                return fDirRequestProcessor.handleDirRequest(info);
            }
            else if (actionLC.equals("help"))
            {
                return fHelpRequestProcessor.handleHelpRequest(info);
            }
            else if (actionLC.equals("version"))
            {
                return fVersionRequestProcessor.handleVersionRequest(info);
            }
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
            
            // And also return the Java stack trace in the result

            StringWriter sr = new StringWriter();
            t.printStackTrace(new PrintWriter(sr));

            if (t.getMessage() != null)
            {
                return new STAFResult(
                    STAFResult.JavaError,
                    t.getMessage() + fLineSep + sr.toString());
            }
            else
            {
                return new STAFResult(
                    STAFResult.JavaError, sr.toString());
            }
        }
    }// end acceptRequest
    
    /**
     * This is the STAF Service termination method that is run when the
     * service is unregistered.  It performs termination functions such as:
     * <ul>
     * <li>Unregisters help data for the service
     * <li>Unregisters the STAF handle it used to submit requests to STAF
     * services
     * * </ul>
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the service terminated successfully.
     */ 
    public STAFResult term()
    {
        try
        {
            // Un-register help data for the service

            unregisterHelpData(kHostConnectError);

            // Un-register the service handle

            fHandle.unRegister();
        }
        catch (STAFException ex)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  ex.toString());
        }
        
        return new STAFResult(STAFResult.Ok);
    }// end term


    private class HelpRequestProcessor
    {
        private HelpRequestProcessor() {}
        
        private STAFResult handleHelpRequest(RequestInfo info)
        {
            // Verify the requester has at least trust level 1

            STAFResult trustResult = STAFUtil.validateTrust(
                1, fServiceName, "HELP", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;

            // Return help text for the service
            
            return new STAFResult(STAFResult.Ok, sHelpMsg);
        }
    }// end class HelpRequestProcessor
    
    private class VersionRequestProcessor
    {
        private VersionRequestProcessor() {}
        
        private STAFResult handleVersionRequest(RequestInfo info)
        {
            // Verify the requester has at least trust level 1

            STAFResult trustResult = STAFUtil.validateTrust(
                1, fServiceName, "VERSION", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;
            
            // Return the service's version

            return new STAFResult(STAFResult.Ok, kVersion);
        }
    }// end class fVersionRequestProcessor
        
    private class GetRequestProcessor
    {
        private STAFCommandParser commandParser;
                
        private GetRequestProcessor()
        {
            this.commandParser = defineOptionsForRequest();
        }
        
        private STAFCommandParser defineOptionsForRequest()
        {
            STAFCommandParser commandParser = new STAFCommandParser();
            
            commandParser.addOption(
                "GET", 1, STAFCommandParser.VALUENOTALLOWED);
            commandParser.addOption(
                "HOST", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "PORT", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "URLPATH", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "FILE", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "USER", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "PASSWORD", 1, STAFCommandParser.VALUEREQUIRED);
            
            // If you specify GET, the HOST, URLPATH and FILE options
            // are required
            
            commandParser.addOptionNeed("GET", "HOST");
            commandParser.addOptionNeed("GET", "URLPATH");
            commandParser.addOptionNeed("GET", "FILE");
            
            // If you specify PORT, the "HOST" option is required

            commandParser.addOptionNeed("PORT", "HOST");

            // If you specify USER, the PASSWORD option is required

            commandParser.addOptionNeed("USER", "PASSWORD");
            
            return commandParser;
        }

        private STAFResult handleGetRequest(RequestInfo info)
        {
            // Verify the requester has at least trust level 4

            STAFResult trustResult = STAFUtil.validateTrust(
                4, fServiceName, "GET", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;
            
            try
            {
                // Parse the request

                STAFCommandParseResult parsedRequest = commandParser.parse(
                    info.request);
                   
                if (parsedRequest.rc != STAFResult.Ok)
                {
                    return new STAFResult(STAFResult.InvalidRequestString,
                                          parsedRequest.errorBuffer);
                }

                STAFResult res = null;

                // Resolve any STAF variables in the HOST option value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("HOST"),
                    fHandle, info.requestNumber);
                        
                if (res.rc != STAFResult.Ok) return res;
                        
                String host = res.result;
                
                // If no PORT option is specified, leave set to null which
                // indicates to use the default port

                String port = null;

                if (parsedRequest.optionTimes("PORT") > 0)
                {
                    // Resolve any STAF variables in the PORT option's value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("PORT"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;
                
                    port = res.result;
                }

                // Resolve any STAF variables in the URLPATH option value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("URLPATH"),
                    fHandle, info.requestNumber);
                
                if (res.rc != STAFResult.Ok) return res;

                String urlPath = res.result;
                        
                // Resolve any STAF variables in the FILE option value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("FILE"),
                    fHandle, info.requestNumber);
                        
                if (res.rc != STAFResult.Ok) return res;

                String localFile = res.result;
                
                // If no USER option is specified, do an anonymous login

                String user = "anonymous";
                String password = fServiceName + "@" + info.machine;

                if (parsedRequest.optionTimes("USER") > 0)
                {
                    // Resolve any STAF variables in the USER option value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("USER"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;

                    user = res.result;
                        
                    // Resolve any STAF variables in the PASSWORD option value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("PASSWORD"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;
                        
                    // Remove privacy delimiters

                    password = STAFUtil.removePrivacyDelimiters(res.result);
                }
                        
                // Perform FTP Get command

                try
                {
                    new FTPGet(host, port, user, password, localFile, urlPath);
                }
                catch (STAFException se)
                {
                    return new STAFResult(se.rc, se.getMessage());
                }

                return new STAFResult(STAFResult.Ok);
           }
           catch (Throwable t)
           {
               StringWriter sw = new StringWriter();
               PrintWriter p = new PrintWriter(sw, true);
               t.printStackTrace(p);
               return new STAFResult(STAFResult.JavaError, sw.toString());
           }
        }
    }// end class GetRequestProcessor

    private class PutRequestProcessor
    {
        private STAFCommandParser commandParser;
                
        private PutRequestProcessor()
        {
            this.commandParser = defineOptionsForRequest();
        }
        
        private STAFCommandParser defineOptionsForRequest()
        {
            STAFCommandParser commandParser = new STAFCommandParser();
            
            commandParser.addOption(
                "PUT", 1, STAFCommandParser.VALUENOTALLOWED);
            commandParser.addOption(
                "HOST", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "PORT", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "URLPATH", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "FILE", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "USER", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "PASSWORD", 1, STAFCommandParser.VALUEREQUIRED);
            
            // If you specify GET, the HOST, URLPATH and FILE options
            // are required
            
            commandParser.addOptionNeed("GET", "HOST");
            commandParser.addOptionNeed("GET", "URLPATH");
            commandParser.addOptionNeed("GET", "FILE");
            
            // If you specify PORT, the "HOST" option is required

            commandParser.addOptionNeed("PORT", "HOST");

            // If you specify USER, the PASSWORD option is required

            commandParser.addOptionNeed("USER", "PASSWORD");

            return commandParser;
        }

        private STAFResult handlePutRequest(RequestInfo info)
        {
            // Verify the requester has at least trust level 4

            STAFResult trustResult = STAFUtil.validateTrust(
                4, fServiceName, "PUT", fLocalMachineName, info);
            
            if (trustResult.rc != STAFResult.Ok) return trustResult;
            
            try
            {
                // Parse the request

                STAFCommandParseResult parsedRequest = commandParser.parse(
                    info.request);
                
                if (parsedRequest.rc != STAFResult.Ok)
                {
                    return new STAFResult(STAFResult.InvalidRequestString,
                                          parsedRequest.errorBuffer);
                }
                
                STAFResult res = null;
                
                // Resolve any STAF variables in the HOST option value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("HOST"),
                    fHandle, info.requestNumber);
                    
                if (res.rc != STAFResult.Ok) return res;
                    
                String host = res.result;
                
                // If no PORT option is specified, leave set to null which
                // indicates to use the default port

                String port = null;

                if (parsedRequest.optionTimes("PORT") > 0)
                {
                    // Resolve any STAF variables in the PORT option's value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("PORT"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;
                
                    port = res.result;
                }

                // Resolve any STAF variables in the URLPATH option value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("URLPATH"),
                    fHandle, info.requestNumber);
                    
                if (res.rc != STAFResult.Ok) return res;
                
                String urlPath = res.result;

                // Resolve any STAF variables in the FILE option value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("FILE"),
                    fHandle, info.requestNumber);
                    
                if (res.rc != STAFResult.Ok) return res;
                    
                String localFile = res.result;

                // If no USER option is specified, do an anonymous login

                String user = "anonymous";
                String password = fServiceName;

                if (parsedRequest.optionTimes("USER") > 0)
                {
                    // Resolve any STAF variables in the USER option value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("USER"),
                        fHandle, info.requestNumber);
                    
                    if (res.rc != STAFResult.Ok) return res;

                    user = res.result;

                    // Resolve any STAF variables in the PASSWORD option value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("PASSWORD"), 
                        fHandle, info.requestNumber);
                    
                    if (res.rc != STAFResult.Ok) return res;
                    
                    // Remove privacy delimiters

                    password = STAFUtil.removePrivacyDelimiters(res.result);
                }
                    
                // Perform the FTP put command

                try
                {
                    new FTPPut(host, port, user, password, localFile, urlPath);
                }
                catch (STAFException se)
                {
                    return new STAFResult(se.rc, se.getMessage());
                }

                return new STAFResult(STAFResult.Ok);
            }
            catch (Throwable t)
            {
                StringWriter sw = new StringWriter();
                PrintWriter p = new PrintWriter(sw, true);
                t.printStackTrace(p);
                return new STAFResult(STAFResult.JavaError, sw.toString());
            }
        }
    }// end class PutRequestProcessor    

    private class DirRequestProcessor
    {
        private STAFCommandParser commandParser;
        
        private DirRequestProcessor()
        {
            this.commandParser = defineOptionsForRequest();
        }
        
        private STAFCommandParser defineOptionsForRequest()
        {
            STAFCommandParser commandParser = new STAFCommandParser();
            
            commandParser.addOption(
                "DIR", 1, STAFCommandParser.VALUENOTALLOWED);
            commandParser.addOption(
                "HOST", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "PORT", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "USER", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "PASSWORD", 1, STAFCommandParser.VALUEREQUIRED);
            commandParser.addOption(
                "URLPATH", 1, STAFCommandParser.VALUEREQUIRED);
            
            // If you specify GET, the HOST and URLPATH options are
            // required
            
            commandParser.addOptionNeed("GET", "HOST");
            commandParser.addOptionNeed("GET", "URLPATH");
            
            // If you specify PORT, the "HOST" option is required

            commandParser.addOptionNeed("PORT", "HOST");

            // If you specify USER, the PASSWORD option is required

            commandParser.addOptionNeed("USER", "PASSWORD");
            
            return commandParser;
        }
        
        private STAFResult handleDirRequest(RequestInfo info)
        {
            // Verify the requester has at least trust level 2

            STAFResult trustResult = STAFUtil.validateTrust(
                2, fServiceName, "DIR", fLocalMachineName, info);
            
            if (trustResult.rc != STAFResult.Ok) return trustResult;
            
            try
            {
                // Parse the request

                STAFCommandParseResult parsedRequest = commandParser.parse(
                    info.request);
                
                if (parsedRequest.rc != STAFResult.Ok)
                {
                    return new STAFResult(STAFResult.InvalidRequestString,
                                          parsedRequest.errorBuffer);
                }
                
                STAFResult res = null;

                // Resolve any STAF variables in the HOST option's value

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("HOST"),
                    fHandle, info.requestNumber);
                
                if (res.rc != STAFResult.Ok) return res;
                
                String host = res.result;
                
                // If no PORT option is specified, leave set to null which
                // indicates to use the default port

                String port = null;

                if (parsedRequest.optionTimes("PORT") > 0)
                {
                    // Resolve any STAF variables in the PORT option's value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("PORT"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;
                
                    port = res.result;
                }

                // If no USER option is specified, do an anonymous login

                String user = "anonymous";
                String password = fServiceName;

                if (parsedRequest.optionTimes("USER") > 0)
                {
                    // Resolve any STAF variables in the USER option value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("USER"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;
                
                    user = res.result;

                    // Resolve any STAF variables in the PASSWORD option value

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("PASSWORD"),
                        fHandle, info.requestNumber);
                
                    if (res.rc != STAFResult.Ok) return res;
                
                    // Remove privacy delimiters

                    password = STAFUtil.removePrivacyDelimiters(res.result);
                }

                // Resolve any STAF variables in the URLPATH option

                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("URLPATH"),
                    fHandle, info.requestNumber);
                
                if (res.rc != STAFResult.Ok) return res;
                
                String urlPath = res.result;
                
                // Perform the FTP Dir command

                try
                {
                    FTPDir ftpdir = new FTPDir(
                        host, port, user, password, urlPath);

                    return new STAFResult(STAFResult.Ok,
                                          ftpdir.getResult().toString());
                }
                catch (STAFException se)
                {
                    return new STAFResult(se.rc, se.getMessage());
                }
            }
            catch (Throwable t)
            {
                StringWriter sw = new StringWriter();
                PrintWriter p = new PrintWriter(sw, true);
                t.printStackTrace(p);
                return new STAFResult(STAFResult.JavaError, sw.toString());
            }
        }
    }// end class DirRequestProcessor
    
    /**
     *  Register error codes for the FTP Service with the HELP service
     */
    private void registerHelpData(int errorNumber, String info,
                                  String description)
    {
        STAFResult res = fHandle.submit2(
            "local", "HELP", "REGISTER SERVICE " + fServiceName +
            " ERROR " + errorNumber +
            " INFO " + STAFUtil.wrapData(info) +
            " DESCRIPTION " + STAFUtil.wrapData(description));
    }

    /**
     *  Un-register error codes for the FTP Service with the HELP service
     */ 
    private void unregisterHelpData(int errorNumber)
    {
        STAFResult res = fHandle.submit2(
            "local", "HELP", "UNREGISTER SERVICE " + fServiceName +
            " ERROR " + errorNumber);
    }
}// end class

