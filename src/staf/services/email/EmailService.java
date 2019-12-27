/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.email;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.*;
import java.util.*;
import java.net.*;
import java.text.SimpleDateFormat;
import org.apache.commons.codec.binary.Base64;

public class EmailService implements STAFServiceInterfaceLevel30
{
    private STAFHandle fHandle;
    private final String kVersion = "3.3.8";
    private String fServiceName = "";
    private String fLocalMachineName = "";
    private static boolean DEBUG = true;
    private STAFCommandParser fSendParser;
    private STAFCommandParser fListParser;
    private STAFCommandParser fParmsParser;
    private STAFCommandParser fSetParser;
    static private String sHelpMsg;
    static private String fLineSep;
    private STAFMapClassDefinition fSettingsMapClass;
    private static String fHostname;
    private Base64 base64 = new Base64();
    private static String fMailserver = "";
    private static int fMailport = 25;
    private static List fBackupMailServerList = new ArrayList();
    private static boolean fResolveMessage = true;
    private static int fSocketTimeout = 60000;

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");
    static final String sSettingsMapClassName = new String(
        "STAF/Service/Email/Settings");
    static final String sTextPlainContentType = "text/plain";
    static final String sTextHtmlContentType = "text/html";
    private static String fContentType = sTextPlainContentType;
    private static String fLineEnd = "\r\n";
    private final String sPlainHeader =
        "******************************************************************\n" +
        "* DO NOT RESPOND TO THE SERVICE MACHINE THAT GENERATED THIS NOTE *\n" +
        "*****************************************************" +
        "*************\n";

    private final String sHtmlHeader =
        "<pre>*****************************************************" +
        "*************\n"+
        "* DO NOT RESPOND TO THE SERVICE MACHINE THAT GENERATED THIS NOTE *\n" +
        "*****************************************************" +
        "*************\n</pre>";

    public static final int IOEXCEPTION = 4001;
    private static final String IOEXCEPTIONInfo = "IO Exception";
    private static final String IOEXCEPTIONDesc =
       "An IO Exception occured while sending the email.";

    public EmailService() {}

    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        int rc = STAFResult.Ok;

        try
        {
            fHandle = new STAFHandle("STAF/SERVICE/" + info.name);
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError, e.toString());
        }

        fServiceName = info.name;

        // Send parser

        fSendParser = new STAFCommandParser(0, false);

        fSendParser.addOption("SEND", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fSendParser.addOption("TO", 0,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("CC", 0,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("BCC", 0,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("FROM", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("SUBJECT", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("MESSAGE", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("FILE", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("MACHINE", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("NOHEADER", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fSendParser.addOption("CONTENTTYPE", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("TEXTATTACHMENT", 0,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("BINARYATTACHMENT", 0,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("ATTACHMENTMACHINE", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("AUTHUSER", 1,
                              STAFCommandParser.VALUEREQUIRED);
        fSendParser.addOption("AUTHPASSWORD", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOption("RESOLVEMESSAGE", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fSendParser.addOption("NORESOLVEMESSAGE", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fSendParser.addOptionGroup("RESOLVEMESSAGE NORESOLVEMESSAGE", 0, 1);

        fSendParser.addOptionGroup("MESSAGE FILE", 1, 1);

        fSendParser.addOptionNeed("MACHINE", "FILE");
        fSendParser.addOptionNeed("AUTHUSER", "AUTHPASSWORD");
        fSendParser.addOptionNeed("AUTHPASSWORD", "AUTHUSER");

        fSendParser.addOptionNeed("ATTACHMENTMACHINE", 
                                  "TEXTATTACHMENT BINARYATTACHMENT");

        fSendParser.addOptionNeed("SEND",
                                  "TO CC BCC");

        // List parser

        fListParser = new STAFCommandParser(0, false);

        fListParser.addOption("LIST", 1, STAFCommandParser.VALUENOTALLOWED);
        fListParser.addOption("SETTINGS", 1, STAFCommandParser.VALUENOTALLOWED);
        fListParser.addOptionNeed("SETTINGS", "LIST");

        // Set parser

        fSetParser = new STAFCommandParser(0, false);

        fSetParser.addOption("SET", 1,
                             STAFCommandParser.VALUENOTALLOWED);

        fSetParser.addOption("MAILSERVER", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fSetParser.addOption("PORT", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fSetParser.addOption("SOCKETTIMEOUT", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fSetParser.addOption("BACKUPMAILSERVERS", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fSetParser.addOption("CONTENTTYPE", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fSetParser.addOption("RESOLVEMESSAGE", 1,
                             STAFCommandParser.VALUENOTALLOWED);

        fSetParser.addOption("NORESOLVEMESSAGE", 1,
                             STAFCommandParser.VALUENOTALLOWED);

        fSetParser.addOptionGroup("RESOLVEMESSAGE NORESOLVEMESSAGE", 0, 1);

        // Resolve the line separator variable for the local machine
         
        STAFResult res = STAFUtil.resolveInitVar(
            "{STAF/Config/Sep/Line}", fHandle);

        if (res.rc != STAFResult.Ok) return res;
        
        fLineSep = res.result;
         
        // Resolve the machine name variable for the local machine

        res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);

        if (res.rc != STAFResult.Ok) return res;
        
        fLocalMachineName = res.result;

        // Parse the parameters specified when registering the service

        if (info.parms == null)
        {
            return new STAFResult(
                STAFResult.ServiceConfigurationError,
                "MAILSERVER is a required parameter for the STAFEmail service");
        }
        else
        {
            fParmsParser = new STAFCommandParser(0, false);

            fParmsParser.addOption("MAILSERVER", 1,
                STAFCommandParser.VALUEREQUIRED);

            fParmsParser.addOption("PORT", 1,
                STAFCommandParser.VALUEREQUIRED);

            fParmsParser.addOption("SOCKETTIMEOUT", 1,
                STAFCommandParser.VALUEREQUIRED);

            fParmsParser.addOption("BACKUPMAILSERVERS", 1,
                STAFCommandParser.VALUEREQUIRED);

            fParmsParser.addOption("CONTENTTYPE", 1,
                STAFCommandParser.VALUEREQUIRED);

            fParmsParser.addOption("RESOLVEMESSAGE", 1,
                STAFCommandParser.VALUENOTALLOWED);

            fParmsParser.addOption("NORESOLVEMESSAGE", 1,
                STAFCommandParser.VALUENOTALLOWED);

            // DEBUG is deprecated
            fParmsParser.addOption("DEBUG", 1,
                STAFCommandParser.VALUENOTALLOWED);

            fParmsParser.addOptionGroup("RESOLVEMESSAGE NORESOLVEMESSAGE", 0, 1);

            res = handleParms(info);

            if (res.rc != STAFResult.Ok)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Error validating parameters: RC=" + res.rc +
                    ", Result=" + res.result);
            }

            String message = fServiceName + " service initialized, using " +
                "mailserver " + fMailserver + " port " + fMailport +
                " backupmailservers " + fBackupMailServerList;
            fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                        fServiceName + " LEVEL info MESSAGE " +
                        STAFUtil.wrapData(message));
        }
        
        // Assign the help text string for the service

        sHelpMsg = "*** " + fServiceName + " Service Help ***" +
            fLineSep + fLineSep +
            "SEND < TO <Address> | CC <Address> | BCC <Address> >..." + fLineSep +
            "     [FROM <user@company.com>] [CONTENTTYPE <ContentType>]" +
            fLineSep +
            "     < MESSAGE <Message> | FILE <File> [MACHINE <Machine>] >" +
            fLineSep +
            "     [SUBJECT <Subject>] [NOHEADER] [TEXTATTACHMENT <File>]..." +
            fLineSep +
            "     [BINARYATTACHMENT <File>]... [ATTACHMENTMACHINE <Machine>]" +
            fLineSep +
            "     [RESOLVEMESSAGE | NORESOLVEMESSAGE]" +
            fLineSep +
            "     [AUTHUSER <User> AUTHPASSWORD <Password>]" +
            fLineSep + fLineSep +
            "LIST SETTINGS" +
            fLineSep + fLineSep +
            "SET  [MAILSERVER <MailServer>]" +
            fLineSep +
            "     [PORT <MailPort>]" +
            fLineSep +
            "     [SOCKETTIMEOUT <Number>[s|m|h|d|w]]" +
            fLineSep +
            "     [BACKUPMAILSERVERS <Space-separated list of backup mailservers>]" +
            fLineSep +
            "     [CONTENTTYPE <ContentType>]" +
            fLineSep +
            "     [RESOLVEMESSAGE | NORESOLVEMESSAGE]" +
            fLineSep + fLineSep +
            "VERSION" +
            fLineSep + fLineSep +
            "HELP";

        /* Register RCs with the HELP service */

        rc = this.registerHelp(info.name);

        if (rc != STAFResult.Ok)
            return new STAFResult(rc, 
                                   "Error registering RCs with HELP service.");

        // Construct map-class for list settings information

        fSettingsMapClass = new STAFMapClassDefinition(
            sSettingsMapClassName);

        fSettingsMapClass.addKey("mailServer", "Mail Server");
        fSettingsMapClass.addKey("port", "Port");
        fSettingsMapClass.addKey("socketTimeout", "Socket Timeout");
        fSettingsMapClass.addKey("backupMailServers", "Backup Mail Servers");
        fSettingsMapClass.addKey("contentType", "Content Type");
        fSettingsMapClass.addKey("resolveMessage", "Resolve Message");

        return new STAFResult(rc);
    }
    
    private int registerHelp(String name)
    {
        try
        {
            String request = "REGISTER SERVICE " + name + " ERROR " +
                IOEXCEPTION + " INFO \"" + IOEXCEPTIONInfo + "\" DESCRIPTION \""
                + IOEXCEPTIONDesc + "\"";
            fHandle.submit("LOCAL", "HELP", request);
        }
        catch(STAFException se)
        {
            return se.rc;
        }

        return STAFResult.Ok;
    }

    private STAFResult handleParms(STAFServiceInterfaceLevel30.InitInfo info)
    {
        STAFCommandParseResult parseResult= fParmsParser.parse(info.parms);

        String errmsg = "ERROR:  Service Configuration Error for Service " +
                        fServiceName + fLineSep +
                        "EmailService::handleParms() - ";

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("MAILSERVER") == 0)
        {
            String errorMsg = errmsg +
                "MAILSERVER is a required parameter for the STAFEmail service";

            return new STAFResult(
                STAFResult.ServiceConfigurationError, errorMsg);
        }
        else
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("MAILSERVER"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving MAILSERVER.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.equals(""))
            {
                String errorMsg = errmsg +
                    "The value for MAILSERVER cannot be blank";

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, errorMsg);
            }

            fMailserver = resolvedResult.result;
        }

        if (parseResult.optionTimes("RESOLVEMESSAGE") > 0)
        {
            fResolveMessage = true;
        }
        else if (parseResult.optionTimes("NORESOLVEMESSAGE") > 0)
        {
            fResolveMessage = false;
        }

        // DEBUG parm is deprecated

        if (parseResult.optionTimes("PORT") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("PORT"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving PORT.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            try
            {
                fMailport = (new Integer(resolvedResult.result).intValue());
            }
            catch (NumberFormatException ex)
            {
                String errorMsg = errmsg +
                    "PORT must be numeric.  Invalid value: " +
                    resolvedResult.result;

                System.out.println(errorMsg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, errorMsg);
            }
        }

        if (parseResult.optionTimes("SOCKETTIMEOUT") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("SOCKETTIMEOUT"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving SOCKETTIMEOUT parameter." +
                    "  RC=" + resolvedResult.rc +
                    " Result=" + resolvedResult.result);

                resolvedResult.result = "Error resolving SOCKETTIMEOUT " +
                    "parameter. " + resolvedResult.result;

                return resolvedResult;
            }

            String timeoutString = resolvedResult.result;

            // Verify the socket timeout value is a valid timeout value, and
            // convert it to milliseconds if needed
            
            STAFResult result = STAFUtil.convertDurationString(timeoutString);

            if (result.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Invalid value for SOCKETTIMEOUT parameter." +
                    "  RC=" + result.rc + " Result=" + result.result);

                result.result = "Invalid value for SOCKETTIMEOUT parameter. " +
                    result.result;

                return result;
            }

            // Convert timeout string to a number

            try
            {
                long socketTimeoutLong = Long.parseLong(timeoutString);

                // The socket timeout must be a positive integer

                if ((socketTimeoutLong < 0) ||
                    (socketTimeoutLong > (long)Integer.MAX_VALUE))
                {
                    String errorMsg = errmsg +
                        "Invalid value (in milliseconds) for the " +
                        "SOCKETTIMEOUT parameter: " + timeoutString +
                        "\n\nIts value in milliseconds must be an integer " +
                        "> 0 and cannot exceed " + Integer.MAX_VALUE +
                        " milliseconds.";
                    
                    System.out.println(errorMsg);

                    return new STAFResult(
                        STAFResult.ServiceConfigurationError, errorMsg);
                }
                
                fSocketTimeout = (int)(new Long(socketTimeoutLong)).intValue();
            }
            catch (NumberFormatException nfe)
            {
                // Should never happen because convertDurationString should
                // have already found this

                String errorMsg = errmsg +
                    "Invalid value (in milliseconds) for the SOCKETTIMEOUT " +
                    "parameter: " + timeoutString +
                    "\n\nNumberFormatException: " + nfe.getMessage();

                System.out.println(errorMsg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, errorMsg);
            }
        }

        if (parseResult.optionTimes("BACKUPMAILSERVERS") > 0)
        {
            // The value can be a space separated list of mail servers

            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("BACKUPMAILSERVERS"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving BACKUPMAILSERVERS.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            // Takes an input string and creates a list containing each of
            // the words in the input as a separate element in the ArrayList.

            // Create a stringtokenizer to read through the input string one
            // word at a time.  It works like an iterator that just iterates
            // over whitespace separated words.
            StringTokenizer st = new StringTokenizer(resolvedResult.result);

            while(st.hasMoreTokens())
            {
                fBackupMailServerList.add(st.nextToken());
            }
        }

        if (parseResult.optionTimes("CONTENTTYPE") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("CONTENTTYPE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errmsg + "Error resolving CONTENTTYPE.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
 
            if (!(resolvedResult.result.equals(sTextPlainContentType)) &&
                !(resolvedResult.result.equals(sTextHtmlContentType)))
            {
                return new STAFResult(STAFResult.InvalidValue,
                                       resolvedResult.result);
            }

            fContentType = resolvedResult.result;
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

            if (actionLC.equals("send"))
                return handleSend(info);
            else if (actionLC.equals("list"))
                return handleList(info);
            else if (actionLC.equals("set"))
                return handleSet(info);
            else if (actionLC.equals("help"))
                return handleHelp(info);
            else if (actionLC.equals("version"))
                return handleVersion(info);
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

    private STAFResult handleVersion(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "VERSION", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        return new STAFResult(STAFResult.Ok, kVersion);
    }

    private STAFResult handleList(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "LIST", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parsedRequest = fListParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }
          
        // LIST SETTINGS

        // Create the marshalling context

        STAFMarshallingContext mc = new STAFMarshallingContext();

        mc.setMapClassDefinition(fSettingsMapClass);

        Map outputMap = fSettingsMapClass.createInstance();
        outputMap.put("mailServer", fMailserver);
        outputMap.put("port", new Integer(fMailport));
        outputMap.put("socketTimeout", new Integer(fSocketTimeout));
        
        synchronized (fBackupMailServerList)
        {
            outputMap.put("backupMailServers", fBackupMailServerList);
        }

        outputMap.put("contentType", fContentType);
        
        if (fResolveMessage)
        {
            outputMap.put("resolveMessage", "Enabled");
        }
        else
        {
            outputMap.put("resolveMessage", "Disabled");
        }

        mc.setRootObject(outputMap);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private STAFResult handleSet(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 5

        STAFResult trustResult = STAFUtil.validateTrust(
            5, fServiceName, "SET", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parsedRequest = fSetParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        STAFResult resolvedValue = null;

        fHandle.submit2(
            "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
            " LEVEL info MESSAGE " + STAFUtil.wrapData(
            "[" + info.endpoint + "] " + info.request));

        if (parsedRequest.optionTimes("MAILSERVER") > 0)
        {
            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("MAILSERVER"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            if (fMailserver.equals(""))
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "The value for the MAILSERVER option cannot be blank");
            }

            fMailserver = resolvedValue.result;
        }

        if (parsedRequest.optionTimes("PORT") > 0)
        {
            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("PORT"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != 0) return resolvedValue;

            try
            {
                fMailport = (new Integer(resolvedValue.result).intValue());
            }
            catch (NumberFormatException ex)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "The value for the PORT option must be numeric.  " +
                    "Invalid value: " + resolvedValue.result);
            }
        }
        
        if (parsedRequest.optionTimes("SOCKETTIMEOUT") > 0)
        {
            // Resolve the TIMEOUT value, verify that it is a valid
            // timeout value, and convert it to milliseconds if needed

            resolvedValue = STAFUtil.resolveRequestVarAndConvertDuration(
                "SOCKETTIMEOUT", parsedRequest.optionValue("SOCKETTIMEOUT"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;
            
            String timeoutString = resolvedValue.result;

            // Convert timeout string to a number

            try
            {
                long socketTimeoutLong = Long.parseLong(timeoutString);

                // The socket timeout must be a positive integer

                if ((socketTimeoutLong < 0) ||
                    (socketTimeoutLong > (long)Integer.MAX_VALUE))
                {
                    String errorMsg = "Invalid value (in milliseconds) for " +
                        "the SOCKETTIMEOUT option: " + timeoutString +
                        "\n\nIts value in milliseconds must be an integer " +
                        "> 0 and cannot exceed " + Integer.MAX_VALUE +
                        " milliseconds.";

                    return new STAFResult(
                        STAFResult.InvalidValue, errorMsg);
                }
                
                fSocketTimeout = (int)(new Long(socketTimeoutLong)).intValue();
            }
            catch (NumberFormatException nfe)
            {
                // Should never happen because already check by
                // STAFUtil.resolveRequestVarAndConvertDuration()

                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid value (in milliseconds) for the SOCKETTIMEOUT " +
                    "option: " + timeoutString +
                    "\n\nNumberFormatException: " + nfe.getMessage());
            }
        }

        if (parsedRequest.optionTimes("BACKUPMAILSERVERS") > 0)
        {
            // The value can be a space separated list of mail servers

            resolvedValue = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("BACKUPMAILSERVERS"),
                fHandle, info.requestNumber);

            if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;

            // Takes an input string and creates a list containing each of
            // the words in the input as a separate element in the list.

            // Create a stringtokenizer to read through the input string
            // one word at a time.  It works like an iterator that just
            // iterates over whitespace separated words.
            StringTokenizer st = new StringTokenizer(resolvedValue.result);

            synchronized (fBackupMailServerList)
            {
                // Set the backup mail server list to be empty since setting
                // BACKUPMAILSERVERS replaces the backup mail servers
                fBackupMailServerList = new ArrayList();

                while(st.hasMoreTokens())
                {
                    fBackupMailServerList.add(st.nextToken());
                }
            }
        }

        if (parsedRequest.optionTimes("CONTENTTYPE") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parsedRequest.optionValue("CONTENTTYPE"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                return resolvedResult;
            }
 
            if (!(resolvedResult.result.equals(sTextPlainContentType)) &&
                !(resolvedResult.result.equals(sTextHtmlContentType)))
            {
                return new STAFResult(STAFResult.InvalidValue,
                                       resolvedResult.result);
            }

            fContentType = resolvedResult.result;
        }

        if (parsedRequest.optionTimes("RESOLVEMESSAGE") > 0)
        {
            fResolveMessage = true;
        }
        else if (parsedRequest.optionTimes("NORESOLVEMESSAGE") > 0)
        {
            fResolveMessage = false;
        }

        return new STAFResult(STAFResult.Ok, "");
    }

    private STAFResult handleSend(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, fServiceName, "SEND", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        String resultString = "";
        Vector toList = new Vector();
        Vector ccList = new Vector();
        Vector bccList = new Vector();
        Vector combinedSendList = new Vector();
        String messageOption = "";
        String contentType = fContentType;
        String subjectOption = "";
        STAFResult resolvedValue = null;
        Vector textAttachments = new Vector();
        Vector binaryAttachments = new Vector();
        String attachmentMachine = info.endpoint;
        boolean includeHeader = true;

        try
        {
            STAFCommandParseResult parsedRequest = fSendParser.parse(
                info.request);

            if (parsedRequest.rc != STAFResult.Ok)
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parsedRequest.errorBuffer);
            }
            
            int toTimes = parsedRequest.optionTimes("to");

            for (int i = 0; i < toTimes; i++)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("to", i + 1),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;

                toList.add(resolvedValue.result);
                combinedSendList.add(resolvedValue.result);
            }

            int ccTimes = parsedRequest.optionTimes("cc");

            for (int i = 0; i < ccTimes; i++)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("cc", i + 1),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;

                ccList.add(resolvedValue.result);
                combinedSendList.add(resolvedValue.result);
            }

            int bccTimes = parsedRequest.optionTimes("bcc");

            for (int i = 0; i < bccTimes; i++)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("bcc", i + 1),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;

                bccList.add(resolvedValue.result);
                combinedSendList.add(resolvedValue.result);
            }

            if (parsedRequest.optionTimes("subject") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("subject"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;

                subjectOption = resolvedValue.result;
            }

            if (parsedRequest.optionTimes("noheader") > 0)
            {
                includeHeader = false;
            }

            boolean resolveCurrentMessage = fResolveMessage;

            if (parsedRequest.optionTimes("resolvemessage") > 0)
            {
                resolveCurrentMessage = true;
            }
            else if (parsedRequest.optionTimes("noresolvemessage") > 0)
            {
                resolveCurrentMessage = false;
            }

            if (parsedRequest.optionTimes("message") == 1)
            {
                if (resolveCurrentMessage)
                {
                    resolvedValue = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("message"),
                        fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    messageOption = resolvedValue.result;
                }
                else
                {
                    messageOption = parsedRequest.optionValue("message");
                }
            }
            else if (parsedRequest.optionTimes("file") == 1)
            {
                String fileMachine = info.endpoint;

                if (parsedRequest.optionTimes("machine") == 1)
                {
                    resolvedValue = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("machine"),
                        fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    fileMachine = resolvedValue.result;;
                }

                STAFResult fsGetResult = fHandle.submit2(
                    fileMachine, "FS", "GET FILE " +
                    parsedRequest.optionValue("file"));

                if (fsGetResult.rc == 0)
                {
                    messageOption = fsGetResult.result;
                }
                else
                {
                    String getFileError = "Error retrieving message file. RC=" +
                        fsGetResult.rc + ", Result=" + fsGetResult.result +
                        ", Request=" + fileMachine + " FS GET" +
                        " FILE " + parsedRequest.optionValue("file");

                    fHandle.submit2("local", "LOG", "LOG MACHINE " +
                        "LOGNAME " + fServiceName + " LEVEL ERROR " +
                        "MESSAGE " + STAFUtil.wrapData("[" + info.endpoint + ":" +
                             info.requestNumber + "] " + getFileError));

                    return fsGetResult;
                }
            }

            if (parsedRequest.optionTimes("contenttype") == 1)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("contenttype"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;

                contentType = resolvedValue.result;

                if (!(contentType.equals(sTextPlainContentType)) &&
                    !(contentType.equals(sTextHtmlContentType)))
                {
                    return new STAFResult(STAFResult.InvalidValue, contentType);
                }
            }

            try
            {
                InetAddress ipaddress = InetAddress.getLocalHost();

                if ((fHostname = ipaddress.getHostName()) == null)
                {
                    fHostname = "localhost";
                }
                else
                {
                    ipaddress = InetAddress.getByName(
                        ipaddress.getHostAddress());

                    fHostname = ipaddress.getHostName();
                }
            }
            catch (UnknownHostException uhe)
            {
                fHostname = "localhost";
            }

            String from = "STAFEmailService@" + fHostname;

            if (parsedRequest.optionTimes("from") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("from"),
                    fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    from = resolvedValue.result;
            }

            // Convert the occurences of the 2-character line ending
            // string "\n" into true 1-character line ending characters,
            // but ignore any escaped (3-character) line ending character
            // strings ("\\n")

            int index = messageOption.indexOf("\\n");
            int escapedNewLineIndex = messageOption.indexOf("\\\\n");

            while (escapedNewLineIndex == index - 1)
            {
                escapedNewLineIndex = messageOption.indexOf("\\\\n",
                                                            index + 2);
                index = messageOption.indexOf("\\n", index + 2);
            }

            String tmpStr = "";
            String CR = "";

            while (index >= 0)
            {
                tmpStr = tmpStr + CR +
                    messageOption.substring(0, index);
                CR = "\n";
                messageOption = messageOption.substring(index+2);

                index = messageOption.indexOf("\\n");
                escapedNewLineIndex = messageOption.indexOf("\\\\n");

                while (escapedNewLineIndex == index - 1)
                {
                    escapedNewLineIndex = messageOption.indexOf("\\\\n",
                                                                index + 2);
                    index = messageOption.indexOf("\\n", index + 2);
                }
            }

            tmpStr = tmpStr + CR + messageOption;
            messageOption = tmpStr;

            if (parsedRequest.optionTimes("attachmentmachine") > 0)
            {
                resolvedValue = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("attachmentmachine"),
                    fHandle, info.requestNumber);

                if (resolvedValue.rc != 0) return resolvedValue;

                attachmentMachine = resolvedValue.result;
            }

            if (parsedRequest.optionTimes("textattachment") > 0)
            {
                int textAttachmentCount = 
                    parsedRequest.optionTimes("textattachment");

                for (int i = 0; i < textAttachmentCount; i++)
                {
                    resolvedValue = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("textattachment", i + 1),
                        fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    textAttachments.add(resolvedValue.result);
                }
            }

            if (parsedRequest.optionTimes("binaryattachment") > 0)
            {
                int binaryAttachmentCount = 
                    parsedRequest.optionTimes("binaryattachment");

                for (int i = 0; i < binaryAttachmentCount; i++)
                {
                    resolvedValue = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("binaryattachment", 
                        i + 1), fHandle, info.requestNumber);

                    if (resolvedValue.rc != 0) return resolvedValue;

                    binaryAttachments.add(resolvedValue.result);
                }
            }

            String message = "[" + info.endpoint + ":" +
                             info.requestNumber + "] " +
                             "SEND TO " + toList.toString() +
                             " CC " + ccList.toString() +
                             " BCC " + bccList.toString() + " FROM " +
                             from + " SUBJECT " + subjectOption +
                             " CONTENTTYPE " + contentType;

            if (textAttachments.size() > 0)
            {
                message += " TEXTATTACHMENTS " + textAttachments.toString();
            }

            if (binaryAttachments.size() > 0)
            {
                message += " BINARYATTACHMENTS " + binaryAttachments.toString();
            }

            if ((textAttachments.size()) > 0 ||
                (binaryAttachments.size()) > 0)
            {
                message += " ATTACHMENTMACHINE " + attachmentMachine;
            }

            fHandle.submit2(
                "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
                " LEVEL info MESSAGE " + STAFUtil.wrapData(message));

            // Generate a list of the mail servers to use in case need to
            // retry sending an email using a backup mail server.
            // The list will contain the primary mail server as the first
            // entry and then the backup mail servers, if any.
            
            List mailServerList = new ArrayList();
            mailServerList.add(fMailserver);

            synchronized (fBackupMailServerList)
            {
                Iterator backupServerIter = fBackupMailServerList.iterator();

                while (backupServerIter.hasNext())
                {
                    mailServerList.add((String)backupServerIter.next());
                }
            }

            String level = "";
            Socket socket = null;

            // Iterate through the mail server and backup mail server
            // until send message successfully

            Iterator mailServerIter = mailServerList.iterator();

            boolean retry = true;
            int numAttempts = 0;  // Counts number of attempts to send a message
            String theResult = "";

            while (retry && mailServerIter.hasNext())
            {
                numAttempts++;

                String mailServer = (String)mailServerIter.next();
                
                try
                {
                    // Create a socket to communicate with the mail server

                    socket = new Socket(mailServer, fMailport);

                    // Set the SO-TIMEOUT property on the socket so that
                    // reading from the socket only waits up to a maximum of
                    // the specified timeout before a SocketTimeoutException
                    // is raised.  This is to prevent a possible hang when
                    // reading from the socket if the mail server doesn't send
                    // any data.

                    socket.setSoTimeout(fSocketTimeout);

                    int rc = 0;
                    String result = "";

                    BufferedReader inStream =
                        new BufferedReader(
                            new InputStreamReader(socket.getInputStream()));

                    PrintStream outStream =
                        new PrintStream(socket.getOutputStream());

                    String reply = inStream.readLine();

                    if (reply != null)
                    {
                        if (reply.startsWith("4") || reply.startsWith("5"))
                        {
                            level = "error";
                            rc = new Integer("4" + reply.substring(0, 3)).intValue();
                        }
                        else
                        {
                            level = "info";
                            rc = 0;
                        }

                        result = reply;
                    }
                    else
                    {
                        level = "error";
                        rc = IOEXCEPTION;
                        result = "Mail server (" + mailServer + ") reply is null." +
                            " Check if valid mail server.";
                    }

                    fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                        fServiceName + " LEVEL " + level + "  MESSAGE " +
                        STAFUtil.wrapData("[" + info.endpoint + ":" +
                             info.requestNumber + "] " + result));

                    if (level.equals("error"))
                        return new STAFResult(rc, result);

                    outStream.print("HELO " + fHostname);
                    outStream.print(fLineEnd);
                    outStream.flush();

                    reply = inStream.readLine();

                    if (reply != null)
                    {
                        if (reply.startsWith("4") || reply.startsWith("5"))
                        {
                            level = "error";
                            rc = new Integer("4" + reply.substring(0, 3)).intValue();
                        }
                        else
                        {
                            level = "info";
                            rc = 0;
                        }

                        result = reply;
                    }
                    else
                    {
                        level = "error";
                        rc = IOEXCEPTION;
                        result = "Mail server (" + mailServer + ") reply is null.";
                    }

                    fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                        fServiceName + " LEVEL " + level + "  MESSAGE " +
                        STAFUtil.wrapData("[" + info.endpoint + ":" +
                             info.requestNumber + "] " + result));

                    if (level.equals("error"))
                        return new STAFResult(rc, result);

                    if (parsedRequest.optionTimes("AUTHUSER") > 0)
                    {
                        String authUser = "";

                        resolvedValue = STAFUtil.resolveRequestVar(
                            parsedRequest.optionValue("AUTHUSER"),
                            fHandle, info.requestNumber);

                        if (resolvedValue.rc != 0) return resolvedValue;

                        authUser = resolvedValue.result;

                        String authPassword = "";

                        resolvedValue = STAFUtil.resolveRequestVar(
                            parsedRequest.optionValue("AUTHPASSWORD"),
                            fHandle, info.requestNumber);

                        if (resolvedValue.rc != 0) return resolvedValue;

                        String authPasswordWithPrivacyDelimiters =
                            resolvedValue.result;

                        authPassword = STAFUtil.removePrivacyDelimiters(
                            authPasswordWithPrivacyDelimiters);

                        String authMessage = "[" + info.endpoint + ":" +
                             info.requestNumber + "] " +
                             "AUTHUSER=" + authUser + " AUTHPASSWORD=" +
                             authPasswordWithPrivacyDelimiters;

                        fHandle.submit2(
                            "local", "LOG", "LOG MACHINE LOGNAME " +
                            fServiceName + " LEVEL info MESSAGE " +
                            STAFUtil.wrapData(authMessage));

                        outStream.print("AUTH LOGIN");
                        outStream.println(fLineEnd);
                        outStream.flush();

                        reply = inStream.readLine();

                        if (reply != null)
                        {
                            if (reply.startsWith("4") || reply.startsWith("5"))
                            {
                                level = "error";
                                rc = new Integer("4" + reply.substring(0, 3)).intValue();
                            }
                            else
                            {
                                level = "info";
                                rc = 0;
                            }

                            result = reply;
                        }
                        else
                        {
                            level = "error";
                            rc = IOEXCEPTION;
                            result = "Mail server (" + mailServer + ") reply is null.";
                        }
                        
                        fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                            fServiceName + " LEVEL " + level + "  MESSAGE " +
                            STAFUtil.wrapData(result));

                        if (level.equals("error"))
                            return new STAFResult(rc, result);

                        String base64user = "";
                        byte[] userbuf = authUser.getBytes();
                        byte[] base64userbuf = base64.encode(userbuf);

                        for (int i=0; i<base64userbuf.length;i++)
                        {
                            base64user = base64user + (char)base64userbuf[i];
                        }

                        outStream.print(base64user);
                        outStream.println(fLineEnd);
                        outStream.flush();

                        reply = inStream.readLine();

                        if (reply != null)
                        {
                            if (reply.startsWith("4") || reply.startsWith("5"))
                            {
                                level = "error";
                                rc = new Integer("4" + reply.substring(0, 3)).intValue();
                            }
                            else
                            {
                                level = "info";
                                rc = 0;
                            }

                            result = reply;
                        }
                        else
                        {
                            level = "error";
                            rc = IOEXCEPTION;
                            result = "Mail server (" + mailServer + ") reply is null.";
                        }

                        fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                            fServiceName + " LEVEL " + level + "  MESSAGE " +
                            STAFUtil.wrapData(result));

                        if (level.equals("error"))
                            return new STAFResult(rc, result);

                        String base64pass = "";
                        byte[] passbuf = authPassword.getBytes();
                        byte[] base64passbuf = base64.encode(passbuf);

                        for (int i=0; i<base64passbuf.length;i++)
                        {
                            base64pass = base64pass + (char)base64passbuf[i];
                        }

                        outStream.print(base64pass);
                        outStream.println(fLineEnd);
                        outStream.flush();

                        reply = inStream.readLine();

                        if (reply != null)
                        {
                            if (reply.startsWith("4") || reply.startsWith("5"))
                            {
                                level = "error";
                                rc = new Integer("4" + reply.substring(0, 3)).intValue();
                            }
                            else
                            {
                                level = "info";
                                rc = 0;
                            }

                            result = reply;
                        }
                        else
                        {
                            level = "error";
                            rc = IOEXCEPTION;
                            result = "Mail server (" + mailServer + ") reply is null.";
                        }

                        fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                            fServiceName + " LEVEL " + level + "  MESSAGE " +
                            STAFUtil.wrapData(result));

                        if (level.equals("error"))
                            return new STAFResult(rc, result);
                    }

                    outStream.print("MAIL FROM:  " + from);
                    outStream.print(fLineEnd);
                    outStream.flush();

                    reply = inStream.readLine();
                    
                    if (reply != null)
                    {
                        if (reply.startsWith("4") || reply.startsWith("5"))
                        {
                            level = "error";
                            rc = new Integer("4" + reply.substring(0, 3)).intValue();
                        }
                        else
                        {
                            level = "info";
                            rc = 0;
                        }

                        result = reply;
                    }
                    else
                    {
                        level = "error";
                        rc = IOEXCEPTION;
                        result = "Mail server (" + mailServer + ") reply is null.";
                    }

                    fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                        fServiceName + " LEVEL " + level + "  MESSAGE " +
                        STAFUtil.wrapData("[" + info.endpoint + ":" +
                             info.requestNumber + "] " + result));

                    if (level.equals("error"))
                        return new STAFResult(rc, result);

                    for (int j = 0; j < combinedSendList.size(); j++)
                    {
                        String to = (String)combinedSendList.elementAt(j);

                        outStream.print("RCPT TO:  " + to);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        reply = inStream.readLine();

                        if (reply != null)
                        {
                            if (reply.startsWith("4") || reply.startsWith("5"))
                            {
                                level = "error";
                                rc = new Integer("4" + reply.substring(0, 3)).intValue();
                            }
                            else
                            {
                                level = "info";
                                rc = 0;
                            }

                            result = reply;
                        }
                        else
                        {
                            level = "error";
                            rc = IOEXCEPTION;
                            result = "Mail server (" + mailServer + ") reply is null.";
                        }

                        fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                            fServiceName + " LEVEL " + level + "  MESSAGE " +
                            STAFUtil.wrapData("[" + info.endpoint + ":" +
                            info.requestNumber + "] " + result));

                        if (level.equals("error"))
                            return new STAFResult(rc, result);
                    }

                    outStream.print("DATA");
                    outStream.print(fLineEnd);
                    outStream.flush();

                    // Some external SMTP servers require the following line
                    inStream.readLine();

                    outStream.print("MIME-Version: 1.0");
                    outStream.print(fLineEnd);
                    outStream.flush();

                    String to = "";

                    for (int j = 0; j < toList.size(); j++)
                    {
                        to = to + ", " + (String)toList.elementAt(j);
                    }

                    if (!(to.equals("")))
                    {
                        outStream.print("To: " + to);
                        outStream.print(fLineEnd);
                        outStream.flush();
                    }

                    String cc = "";

                    for (int j = 0; j < ccList.size(); j++)
                    {
                        cc = cc + ", " + (String)ccList.elementAt(j);
                    }

                    if (!(cc.equals("")))
                    {
                        outStream.print("Cc: " + cc);
                        outStream.print(fLineEnd);
                        outStream.flush();
                    }

                    String bcc = "";

                    for (int j = 0; j < bccList.size(); j++)
                    {
                        bcc = bcc + ", " + (String)bccList.elementAt(j);
                    }

                    if (!(bcc.equals("")))
                    {
                        outStream.print("Bcc: " + bcc);
                        outStream.print(fLineEnd);
                        outStream.flush();
                    }

                    outStream.print("Subject: " + subjectOption);
                    outStream.print(fLineEnd);
                    outStream.flush();

                    outStream.print("From: " + from);
                    outStream.print(fLineEnd);
                    outStream.flush();

                    String strBoundary = "DataSeparatorString";

                    outStream.print("Content-Type: multipart/mixed; " + 
                        "boundary=\"" + strBoundary + "\"");
                    outStream.print(fLineEnd);
                    outStream.flush();
                    
                    outStream.print("--" + strBoundary);
                    outStream.print(fLineEnd);
                    outStream.flush();

                    outStream.print("Content-Type: " + contentType +
                                    "; charset=us-ascii");
                    outStream.print(fLineEnd);
                    outStream.flush();
                    
                    outStream.print("Content-Transfer-Encoding: 7bit");
                    outStream.print(fLineEnd);
                    outStream.flush();
                    
                    outStream.print("");
                    outStream.print(fLineEnd);
                    outStream.flush();

                    if (includeHeader)
                    {
                        if (contentType.equals(sTextPlainContentType))
                        {
                            outStream.print(sPlainHeader);
                        }
                        else
                        {
                            outStream.print(sHtmlHeader);
                        }

                        outStream.print(fLineEnd);
                        outStream.flush();
                    }

                    outStream.print(messageOption);
                    outStream.print(fLineEnd);
                    outStream.flush();

                    outStream.print(fLineEnd);
                    outStream.flush();

                    for (int k = 0; k < textAttachments.size(); k++)
                    {
                        String attachmentFile =
                            (String)textAttachments.elementAt(k);

                        int forwardSlashIndex = attachmentFile.lastIndexOf("/");
                        int backwardSlashIndex =
                            attachmentFile.lastIndexOf("\\");
                        int filenameIndex;

                        if (forwardSlashIndex > backwardSlashIndex)
                            filenameIndex = forwardSlashIndex;
                        else
                            filenameIndex = backwardSlashIndex;

                        String attachmentFileName =
                            attachmentFile.substring(filenameIndex);

                        STAFResult fileResult =
                            fHandle.submit2(attachmentMachine,
                                            "FS", "GET FILE " + attachmentFile +
                                            " TEXT");

                        if (fileResult.rc != STAFResult.Ok)
                        {
                            String getFileError = "Error retrieving text " +
                                "attachment file. RC=" + fileResult.rc +
                                ", Result=" + fileResult.result +
                                ", Request=" + attachmentMachine + " FS GET" +
                                " FILE " + attachmentFile + " TEXT";

                            fHandle.submit2("local", "LOG", "LOG MACHINE " +
                                "LOGNAME " + fServiceName + " LEVEL ERROR " +
                                "MESSAGE " + STAFUtil.wrapData(
                                "[" + info.endpoint + ":" +
                                info.requestNumber + "] " + getFileError));

                            return fileResult;
                        }

                        outStream.print("--" + strBoundary);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Type: text/plain; name=\"" +
                            attachmentFileName + "\"");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Transfer-Encoding: 7bit");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Disposition: attachment; " +
                            "filename=\"" + attachmentFileName + "\"");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Description: Form Results");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print(fileResult.result);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("--" + strBoundary);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("");
                        outStream.print(fLineEnd);
                        outStream.flush();
                    }

                    for (int k = 0; k < binaryAttachments.size(); k++)
                    {
                        String attachmentFile =
                            (String)binaryAttachments.elementAt(k);

                        int forwardSlashIndex = attachmentFile.lastIndexOf("/");
                        int backwardSlashIndex =
                            attachmentFile.lastIndexOf("\\");
                        int filenameIndex;

                        if (forwardSlashIndex > backwardSlashIndex)
                            filenameIndex = forwardSlashIndex;
                        else
                            filenameIndex = backwardSlashIndex;

                        String attachmentFileName =
                            attachmentFile.substring(filenameIndex);

                        STAFResult fileResult =
                            fHandle.submit2(attachmentMachine,
                                            "FS", "GET FILE " + attachmentFile +
                                            " BINARY");

                        if (fileResult.rc != STAFResult.Ok)
                        {
                            String getFileError = "Error retrieving binary " +
                                "attachment file. RC=" + fileResult.rc +
                                ", Result=" + fileResult.result +
                                ", Request=" + attachmentMachine + " FS GET" +
                                " FILE " + attachmentFile + " BINARY";

                            fHandle.submit2("local", "LOG", "LOG MACHINE " +
                                "LOGNAME " + fServiceName + " LEVEL ERROR " +
                                "MESSAGE " + STAFUtil.wrapData(
                                "[" + info.endpoint + ":" +
                                info.requestNumber + "] " + getFileError));

                            return fileResult;
                        }

                        outStream.print("--" + strBoundary);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Type: " +
                            "application/octet-stream; name=\"" +
                            attachmentFileName + "\"");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Transfer-Encoding: base64");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Disposition: attachment; " +
                            "filename=\"" + attachmentFileName + "\"");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("Content-Description: Form Results");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("");
                        outStream.print(fLineEnd);
                        outStream.flush();

                        String getFileResult = fileResult.result;

                        byte[] fileContent = 
                            new byte[getFileResult.length() / 2];

                        fileContent[0] = (byte)Integer.parseInt(
                            getFileResult.substring(0, 2), 16);

                        for (int i = 1; i < fileContent.length; i++)
                        {
                            fileContent[i] = (byte)
                                Integer.parseInt(getFileResult.substring(
                                                 i*2, i*2 + 2), 16);
                        }

                        byte[] base64FileContent = 
                            base64.encodeBase64Chunked(fileContent);

                        outStream.write(base64FileContent, 0,
                                        base64FileContent.length);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("--" + strBoundary);
                        outStream.print(fLineEnd);
                        outStream.flush();

                        outStream.print("");
                        outStream.print(fLineEnd);
                        outStream.flush();
                    }

                    outStream.print(fLineEnd);

                    outStream.print(".");
                    outStream.print(fLineEnd);
                    outStream.print("");
                    outStream.print(fLineEnd);
                    outStream.flush();

                    outStream.print("QUIT");
                    outStream.print(fLineEnd);
                    outStream.flush();

                    reply = inStream.readLine();

                    if (reply != null)
                    {
                        if (reply.startsWith("4") || reply.startsWith("5"))
                        {
                            level = "error";
                            rc = new Integer("4" + reply.substring(0, 3)).intValue();
                        }
                        else
                        {
                            level = "info";
                            rc = 0;
                        }

                        result = reply;
                    }
                    else
                    {
                        level = "error";
                        rc = IOEXCEPTION;
                        result = "Mail server (" + mailServer + ") reply is null.";
                    }

                    fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                        fServiceName + " LEVEL " + level + "  MESSAGE " +
                        STAFUtil.wrapData("[" + info.endpoint + ":" +
                             info.requestNumber + "] " + result));

                    if (level.equals("error"))
                        return new STAFResult(rc, result);

                    retry = false;
                }
                catch ( IOException ioe )
                {
                    theResult = "Socket IO Exception: " + ioe +
                        ", Mailserver: " + mailServer + ", Port: " + fMailport;

                    fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                        fServiceName + " LEVEL warning MESSAGE " +
                        STAFUtil.wrapData("[" + info.endpoint + ":" +
                             info.requestNumber + "] Send attempt #" +
                             numAttempts + " failed with " + theResult));

                    // Retry using backup mail servers

                    retry = true;
                }
                finally
                {
                    try
                    {
                        socket.close();
                    }
                    catch(Exception e) {}
                }
            } // end while

            if (retry)
            {
                // All attempts to send a message using the primary mail server
                // and the backup mail servers failed.  Log an error message
                // and return IOEXCEPTION error. 

                if (numAttempts == 1)
                {
                    theResult = numAttempts +
                         " attempt to send the message failed";
                }
                else
                {
                    theResult = numAttempts +
                        " attempts to send the message failed";
                }

                fHandle.submit2("local", "LOG", "LOG MACHINE LOGNAME " +
                    fServiceName + " LEVEL error MESSAGE " +
                    STAFUtil.wrapData("[" + info.endpoint + ":" +
                         info.requestNumber + "] " + theResult));

                return new STAFResult(IOEXCEPTION, theResult);
            }
        }
        catch (Exception e)
        {
            if (DEBUG)
            {
                e.printStackTrace();
            }
            return new STAFResult(STAFResult.JavaError,
                                  "Internal Java error.");
        }

        return new STAFResult(STAFResult.Ok, resultString);
    }

    public STAFResult term()
    {
        // Un-register the service handle

        try
        {
            fHandle.unRegister();
        }
        catch (STAFException ex)
        {
            return new STAFResult(
                STAFResult.STAFRegistrationError, ex.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }
}
