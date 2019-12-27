/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2003, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.authsample;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.util.Properties;
import java.util.Calendar;
import java.io.FileInputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.SimpleDateFormat;

// Sample Authenticator Service

public class AuthSample implements STAFServiceInterfaceLevel30
{
    public AuthSample() {}
    
    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        int rc = STAFResult.Ok;

        try
        {
            fServiceName = info.name;
            fServiceType = info.serviceType;

            // Get the services STAF handle
            fHandle = new STAFHandle("STAF/Authenticator/" + info.name);
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  "Error registering handle");
        }

        // Parse PARMS if provided when registering the authenticator

        if (info.parms != null)
        {
            // instantiate parser as not case sensitive
            fParmsParser = new STAFCommandParser(0, false);

            fParmsParser.addOption("USERPROPERTIESFILE", 1,
                                   STAFCommandParser.VALUEREQUIRED);

            STAFResult res = handleParms(info);

            if (res.rc != STAFResult.Ok)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Error validating parameters: RC=" + res.rc +
                    ", Result=" + res.result);
            }
        }

        if (fUserPropertiesFile == null)
        {
            System.out.println(
                "ERROR: AuthSample.init() failed for authenticator " +
                fServiceName +
                "\nThe USERPROPERTIESFILE parameter must be specified.");

            return new STAFResult(
                STAFResult.ServiceConfigurationError,
                "The USERPROPERTIESFILE parameter must be specified.");
        }

        // Load the User Properties File

        try
        {
            fUserProperties.load(new FileInputStream(fUserPropertiesFile));
        }
        catch (Exception e)
        {
            System.out.println(
                "ERROR: AuthSample.init() failed for authenticator " +
                fServiceName +
                "\nException occurred accessing User Properties File\n" +
                e.toString());

            return new STAFResult(
                STAFResult.ServiceConfigurationError,
                "Exception occurred accessing User Properties File\n" +
                e.toString());
        }

        // Create AUTHENTICATE Parser

        // AUTHENTICATE USER (CREDENTIALS <Credentials> | DATA <Data>)

        fAuthParser = new STAFCommandParser(0, false);

        fAuthParser.addOption("AUTHENTICATE", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fAuthParser.addOption("USER", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fAuthParser.addOption("CREDENTIALS", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fAuthParser.addOption("DATA", 1,
                             STAFCommandParser.VALUEREQUIRED);

        // You must specify a USER
        fAuthParser.addOptionNeed("AUTHENTICATE", "USER");

        // You must specify CREDENTIALS or DATA for a USER
        fAuthParser.addOptionNeed("USER", "CREDENTIALS DATA");

        // You must have CREDENTIALS or DATA, but not both
        fAuthParser.addOptionGroup("CREDENTIALS DATA", 0, 1);
        
        // XXX: The VAR service isn't registered yet, so can't resolve
        //      {STAF/Config/Machine}
        fLocalMachineName = "Unavailable";
        
        return new STAFResult(STAFResult.Ok);
    }

    public STAFResult acceptRequest(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
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

            if (actionLC.equals("authenticate"))
            {
                return handleAuthenticate(info);
            }
            else
            {
                return new STAFResult(
                    STAFResult.InvalidRequestString,
                    "'" + action +
                    "' is not a valid command request for the " +
                    fServiceName + " service.  AUTHENTICATE is the only " +
                    "valid command request supported.");
            }
        }
        catch (Throwable t)
        {
            // Write the Java stack trace to the JVM log for the service

            System.out.println(
                sTimestampFormat.format(Calendar.getInstance().getTime()) +
                " ERROR: Exception on " + fServiceName +
                " service request:\n\n" + info.request + "\n");

            t.printStackTrace();

            // And also return the Java stack trace in the result

            StringWriter sr = new StringWriter();
            t.printStackTrace(new PrintWriter(sr));

            if (t.getMessage() != null)
            {
                return new STAFResult(
                    STAFResult.JavaError,
                    t.getMessage() + "\n" + sr.toString());
            }
            else
            {
                return new STAFResult(
                    STAFResult.JavaError, sr.toString());
            }
        }

    }

    private STAFResult handleParms(STAFServiceInterfaceLevel30.InitInfo info)
    {
        STAFCommandParseResult parseResult= fParmsParser.parse(info.parms);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("USERPROPERTIESFILE") > 0)
        {
            STAFResult result = STAFUtil.resolveInitVar(
                parseResult.optionValue("USERPROPERTIESFILE"), fHandle);

            if (result.rc != STAFResult.Ok) return result;
            
            fUserPropertiesFile = result.result;
        }

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult handleAuthenticate(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Don't need to check trust level if service was registered as an
        // Authenticator.

        if (fServiceType !=
            STAFServiceInterfaceLevel30.serviceTypeAuthenticator)
        {
            // Verify the request has at least trust level 5

            STAFResult trustResult = STAFUtil.validateTrust(
                5, fServiceName, "AUTHENTICATE", fLocalMachineName, info);

            if (trustResult.rc != STAFResult.Ok) return trustResult;
        }

        STAFResult result = new STAFResult(STAFResult.Ok, "");
        String resultString = "";

        // Parse the authenticate request

        STAFCommandParseResult parsedRequest = fAuthParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        String userValue;
        String credentialsValue = null;
        String dataValue = null;

        // Resolve any STAF variables in USER option value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("USER"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        userValue = res.result;

        // Get the CREDENTIALS or DATA option value

        if (parsedRequest.optionTimes("CREDENTIALS") != 0)
            credentialsValue = parsedRequest.optionValue("credentials");
        else
            dataValue = parsedRequest.optionValue("data");

        // If CREDENTIALS specified, verify match in fUserProperties
        if (parsedRequest.optionTimes("CREDENTIALS") != 0)
        {
            if (fUserProperties.getProperty(userValue) == null)
            {
                return new STAFResult(
                    STAFResult.HandleAuthenticationDenied,
                    "User " + userValue + " is not a valid user");
            }
            else if (!(fUserProperties.getProperty(userValue)).equals(
                      credentialsValue))
            {
                return new STAFResult(
                    STAFResult.HandleAuthenticationDenied,
                    "Invalid credentials for user " + userValue);
            }
            else
            {
                // Authenticated successfully.

                // XXX: Could "encrypt" the credentials and return and
                // store the encrypted credentials as the authentication
                // data instead of returning the actual credentials.

                result.result = credentialsValue;
            }
        }
        else
        {   // DATA specified, so verify match in fUserProperties

            // XXX: If encrypted the credentials, would verify match with
            // encrypted credentials instead.

            if (fUserProperties.getProperty(userValue) == null)
            {
                return new STAFResult(
                    STAFResult.HandleAuthenticationDenied,
                    "User " + userValue + " is not a valid user");
            }
            else if (!(fUserProperties.getProperty(userValue)).equals(
                      dataValue))
            {
                return new STAFResult(
                    STAFResult.HandleAuthenticationDenied,
                    "Invalid data for user " + userValue);
            }
        }

        return result;
    }

    public STAFResult term()
    {
        try
        {
            fHandle.unRegister();
        }
        catch (STAFException ex)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  "Error unregistering handle.");
        }

        return new STAFResult(STAFResult.Ok);
    }

    private String fServiceName;  // Authenticator service name
    private int fServiceType;
    private STAFHandle fHandle;
    private String fLocalMachineName = "";  // Local service machine name
    private String fUserPropertiesFile = null;
    private Properties fUserProperties = new Properties();
    private STAFCommandParser fAuthParser;
    private STAFCommandParser fParmsParser;

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");
}
