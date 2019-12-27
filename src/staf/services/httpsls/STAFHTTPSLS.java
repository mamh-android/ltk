/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.httpsls;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import com.ibm.staf.STAFException;
import com.ibm.staf.STAFHandle;
import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFUtil;
import com.ibm.staf.service.STAFCommandParseResult;
import com.ibm.staf.service.STAFCommandParser;
import com.ibm.staf.service.STAFServiceInterfaceLevel30;

/**
 * The STAF HTTP Service Loader enables STAF to load services that can be
 * downloaded from a web server.  The HTTP Service Loader Service (HTTPSLS)
 * uses a configuration file to determine what services it can load.  The
 * location of the HTTPSLS configuration file is specified via a parameter
 * when registering the HTTP Service Loader in the STAF.cfg file and it can
 * reside on the local machine or on a web server.  The HTTPSLS configuration
 * file contains a line for each STAF Java service which specifies the location
 * of the service's jar file (or a zip file that contains the service's jar
 * file).
 * 
 * When STAF encounters a request for a service that isn't currently
 * registered, it checks each service loader that's registered in the
 * STAF.cfg file to see if it handles this service.  The HTTP Service Loader
 * reads its HTTPSLS configuration file (first downloading it from a web
 * server if needed) to see if it can handle this service.  If so, it
 * downloads the service from the specified location to a temporary
 * directory on the local machine (and unzips it if necessary to access the
 * STAF service jar file) and submits an ADD request to the SERVICE service
 * to register the service.  This way, STAF is able to load whatever Java
 * services you want.  Also, note that each time the HTTP Service Loader
 * attempts to load a service, it reads its HTTPSLS configuration file, so
 * you can update the HTTPSLS configuration file and it will read it the
 * next time it attempts to load a service (without restarting STAFProc).
 * 
 * Note that each time STAF is restarted, it deletes the tmp directory in
 * the STAF data directory so the temporary jar files that the HTTP Service
 * Loader downloaded will be deleted since they are stored in the STAF tmp
 * data directory's service/<Serviceloader Name> sub-directory.
 * 
 * The STAF HTTP Service Loader service is installed as part of the Java
 * support that STAF installs by default with a typical installation of STAF.
 * To use the STAF HTTP Service Loader, you must have a Java runtime installed
 * on your machine and you must add a SERVICELOADER configuration statement to
 * the STAF.cfg file including a parameter specifying the location of the
 * HTTPSLS cnfiguration file it will use.
 * 
 * The syntax for registering the HTTP Service loader is:
 * 
 * SERVICELOADER LIBRARY JSTAF
 *               EXECUTE {STAF/Config/STAFRoot}/lib/STAFHTTPSLS.jar
 *               [OPTION <Name[=Value]>]...  
 *               PARMS "CONFIGFILE <ConfigFileLocation>
 *                      [DOWNLOADATTEMPTS <NumDownloadAttempts>]
 *                      [DOWNLOADRETRYDELAY <DelayInSeconds>]"
 * 
 * OPTION specifies a configuration that will be passed to JSTAF to
 * further control the interface to the JVM use for this service loader.
 * You may specify multiple OPTIONs.  See section "4.4.2 JSTAF service proxy
 * library" in the STAF User's Guide for acceptable options to the JSTAF
 * service proxy library.
 * 
 * Also, you can override the version of Java that you want the HTTP Service
 * Loader to use via the OPTION JVM=<Java Path> option if you create a new 
 * JVM by also using OPTION JVMName=HTTPSLS.  For example:
 * 
 * OPTION JVMName=HTTPSLS OPTION JVM=C:\java1.5.0_12\bin\java
 * 
 * PARMS specifies the parameters that will be passed to the service loader
 * during initialization where:
 * 
 * - CONFIGFILE specifies the location of the HTTPSLS configuration file.
 *   This can be a fully-qualified path to a file on the local machine or
 *   it can be a url to the location of the HTTPSLS configuration file on
 *   a web server.  The contents of this file must follow the syntax
 *   specified in section "HTTPSLS Configuration File" below or the HTTP
 *   Service Loader Service registration will fail.  This parameter is
 *   required.  This option will resolve STAF variables.  For example:
 * 
 *   CONFIGFILE http://server1.company.com/staf/httpsls.cfg
 *   CONFIGFILE C:/tests/staf/myhttpsls.cfg
 *   CONFIGFILE {STAF/Config/STAFRoot}/bin/httpsls.cfg
 * 
 * - DOWNLOADATTEMPTS specifies the maximum number of attempts that the
 *   HTTP Service Loader will make to try to download a file.  The default
 *   is 3.  This option will resolve STAF variables.
 * 
 * - DOWNLOADRETRYDELAY specifies the number of seconds that the HTTP
 *   Service Loader will delay before retrying to download a file if the
 *   previous download attempt failed.  The default is 5.  This option will
 *   resolve STAF variables.
 * 
 * Examples:
 * 
 * # Add HTTP Service Loader Service
 * SERVICELOADER LIBRARY JSTAF \
 *               EXECUTE {STAF/Config/STAFRoot}/lib/STAFHTTPSLS.jar \
 *               OPTION JVMName=HTTPSLS \
 *               PARMS "CONFIGFILE http://server.ibm.com/project/httpsls.cfg"
 * 
 * # Add HTTP Service Loader Service
 * SERVICELOADER LIBRARY JSTAF \
 *               EXECUTE {STAF/Config/STAFRoot}/lib/STAFHTTPSLS.jar \
 *               OPTION JVMName=HTTPSLS OPTION JVM=C:\java1.5.0_12\bin\java \
 *               PARMS "CONFIGFILE http://server.ibm.com/project/httpsls.cfg \
 *                      DOWNLOADATTEMPTS 3 DOWNLOADRETRYDELAY 7"
 * 
 * Note that you can register the HTTP Service Loader Service multiple times
 * specifying different CONFIGFILE parameters.  For example, you may have
 * one HTTP service loader service that downloads STAF Java services like STAX,
 * Event, Cron, and Email from the SourceForge website and you may have a
 * second HTTP service loader service that downloads your custom STAF Java
 * services from your own web server.
 * 
 * * The HTTP Service Loader Service (HTTPSLS) is configured through a text file
 * called the HTTPSLS Configuration File.  This file may have any name you
 * desire and can be located on a web server or on the local machine.  The
 * HTTPSLS Configuration File is read and processed line by line.  Whitespace
 * at the front and end of each line is removed before processing.  Blank
 * lines, or lines containing only whitespace, are ignored.  You may continue
 * a configuration statement onto the next line by placing a "\" as the last
 * character of the line.
 * 
 * Each service that you want the HTTP Service Loader to load must have a
 * SERVICE entry in the HTTPSLS configuration file.  The syntax for each
 * service is somewhat similar to the SERVICE syntax used when registering a
 * STAF service in the STAF Configuration File.
 * 
 * Comments:
 * 
 * You specify a comment by placing a pound sign, #, as the first non-blank
 * character in the line.  Comment lines are ignored.  For example:
 * 
 * # This is a comment line
 * 
 * Service Registration:
 * 
 * External Java services are registed with the SERVICE configuration
 * statement.  The syntax is:
 * 
 * SERVICE <Name> LIBRARY JSTAF EXECUTE <Location of Service Jar or Zip File>
 *                [ZIPPATH <Path to Service Jar File>]
 *                [OPTION <Name[=Value]>]...
 *                [PARMS "<Service Parameters>"]
 * 
 * <Name> is the name by which this service will be known on this machine.
 * 
 * LIBRARY must always be JSTAF which is the STAF Java service proxy library
 * as the HTTP Service Loader only supports loading Java services currently.
 * 
 * EXECUTE specifies the url for the Java jar file which implements the
 * service.  Or, it can specify the url of a zip file that contains the Java
 * jar file if the ZIPPATH option is also specified.  Or, it can specify the
 * fully-qualified name of the Java jar file that resides on the local
 * machine.  This option will resolve STAF variables.  For example:
 * 
 *   http://server1.company.com/staf/myservice.jar
 *   http://prdownloads.sourceforge.net/staf/STAXV333.zip
 *   C:/STAF/services/stax/STAX.jar
 *   {STAF/Config/STAFRoot}/services/email/STAFEmail.jar
 * 
 * ZIPPATH specifies the path to the service jar file in the zip file
 * specified by the EXECUTE option.  Note that this option should only be
 * used if the EXECUTE option specifies a url for a zip file that can be
 * unzipped using the STAF ZIP service.  This option does not resolve STAF
 * variables.  For example:
 * 
 *   stax/STAX.jar
 *   myService.jar
 * 
 * OPTION specifies a configuration option that will be passed on to JSTAF to
 * further control the interface to the JVM where this service will be run.
 * You may specify multiple OPTIONs for a given service.  See section "4.4.2
 * JSTAF service proxy library" in the STAF User's Guide for valid options
 * that can be specified for the JSTAF service proxy library.
 * 
 * PARMS specifies parameters that will be passed to the service during
 * initialization.
 * 
 * Examples:
 * 
 * # Register the custom SERVICE1 service, downloading it from a web server
 * SERVICE SERVICE1 LIBRARY JSTAF EXECUTE http://www.company.com/service1.jar
 * 
 * # Register the custom SERVICE2 service, downloading it from a web server
 * SERVICE SERVICE2 LIBRARY JSTAF EXECUTE http://www.company.com/service2.jar \
 *                  OPTION JVMName=SERVICE2
 * 
 * # Register the STAX V3.3.0 service, downloading it from the SourceForge
 * # website
 * SERVICE STAX LIBRARY JSTAF \
 *              EXECUTE http://prdownloads.sourceforge.net/staf/STAXV333.zip \
 *              ZIPPATH stax/STAX.jar \
 *              OPTION JVMName=STAX OPTION J2=-Xmx1024m \
 *              PARMS "EXTENSIONXMLFILE C:/staf/services/extensions.xml"
 * 
 * # Register the Cron V3.3.2 service, downloading it from the SourceForge
 * # website
 * SERVICE CRON LIBRARY JSTAF
 *              EXECUTE http://server.company.com/staf/CronV332.zip \
 *              ZIPPATH cron/STAFCron.jar OPTION JVMName=CRON
 * 
 * # Register the Email service (which resides in a jar file on the local
 * # machine)
 * SERVICE EMAIL LIBRARY JSTAF EXECUTE C:/STAF/services/email/STAFEmail.jar \
 *               PARMS "MAILSERVER NA.relay.ibm.com \
 *               BACKUPMAILSERVERS \"LA.relay.ibm.com EMEA.relay.ibm.com\""
 * 
 * We provide a sample HTTP Service Loader Service Configuration File
 * on SourceForge that can be used to download the latest versions of
 * STAF Java services available on SourceForge such as STAX, Event,
 * Cron, EventManager, etc.  It is located at:
 * 
 *   http://staf.sourceforge.net/current/STAFHTTPSLS.cfg
 * 
 * Note that this HTTPSLS configuration file may not work for you as you may
 * need to customize it for the services that you use by changing the
 * MAILSERVER and BACKUPMAILSERVERS parameters for the Email service and/or
 * running STAX in its own JVM with larger maximum heap size by adding the
 * following options:
 * 
 *   OPTION JVMNAME=STAX OPTION J2=-Xmx1024m
 * 
 * or you may not use use some of the services so you may want to remove
 * them, or you may want to download the STAF Java service zip files and
 * put them on your own web server for faster download times, etc.
 */ 
public class STAFHTTPSLS implements STAFServiceInterfaceLevel30
{
    private static final int BUFFER_SIZE = 1024;
    private static final String COMMENT_MARKER = "#";
    private static final String CONTINUATION_LINE_MARKER = "\\";
    private static final String EMPTY_STRING = "";
    
    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    private String fServiceName;
    private STAFHandle fHandle;
    private String fFileSep;
    private String fTempDir;
    private String fConfigFile;

    // Default number of times to retry downloading a file is 3
    private int fDownloadAttempts = 3;

    // Default seconds to delay before retrying to download a file is 5
    private int fDownloadRetryDelay = 5;

    private STAFCommandParser fParmsParser;
    private STAFCommandParser fServiceParser;
    private STAFCommandParser fLoadParser;
    
    public STAFHTTPSLS() {}
     
    /**
     * This is the STAF Service initialization method that is run when the
     * STAF HTTP Service Loader Service (SLS) is registered.
     * It performs initialization functions such as:
     * <ul>
     * <li>Creates a STAF handle to use to submit requests to STAF services
     * <li>Resolves any local variables needed by the service
     * <li>Creates request parsers
     * <li>Creates map classes uses in list/query results
     * <li>Creates the data directory for the service (if it doesn't already 
     * exist
     * </ul>
     * 
     * @param info STAF service initialization information
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the service initialized successfully.
     */ 
    public STAFResult init(InitInfo info)
    {
        try
        {
            fServiceName = info.name;
            fHandle = new STAFHandle("STAF/Service/" + info.name);

            // Resolve the file separator variable for the local machine

            STAFResult res = STAFUtil.resolveInitVar(
                "{STAF/Config/Sep/File}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fFileSep = res.result;

            // Parse the PARMS provided for the Http Service Loader service to
            // get the name of the configuration file.

            String errmsg = "Error validating parameters for STAFHTTPSLS: ";

            fParmsParser = new STAFCommandParser();
            fParmsParser.addOption("CONFIGFILE", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fParmsParser.addOption("DOWNLOADATTEMPTS", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fParmsParser.addOption("DOWNLOADRETRYDELAY", 1,
                                   STAFCommandParser.VALUEREQUIRED);
            fParmsParser.addOptionGroup("CONFIGFILE", 1, 1);

            res = handleParms(info);

            if (res.rc != STAFResult.Ok)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Error validating parameters for STAFHTTPSLS: RC=" +
                    res.rc + ", Result=" + res.result);
            }

            // LOAD parser
            
            fLoadParser = new STAFCommandParser();
            
            fLoadParser.addOption(
                "LOAD", 1, STAFCommandParser.VALUENOTALLOWED);
            fLoadParser.addOption(
                "SERVICE", 1, STAFCommandParser.VALUEREQUIRED);

            fLoadParser.addOptionNeed("LOAD", "SERVICE");
            
            // SERVICE parser
            
            fServiceParser = new STAFCommandParser();
            
            fServiceParser.addOption(
                "SERVICE", 1, STAFCommandParser.VALUEREQUIRED);
            fServiceParser.addOption(
                "LIBRARY", 1, STAFCommandParser.VALUEREQUIRED);
            fServiceParser.addOption(
                "EXECUTE", 1, STAFCommandParser.VALUEREQUIRED);
            fServiceParser.addOption(
                "OPTION", Integer.MAX_VALUE, STAFCommandParser.VALUEREQUIRED);
            fServiceParser.addOption(
                "PARMS", 1, STAFCommandParser.VALUEREQUIRED);
            fServiceParser.addOption(
                "ZIPPATH", 1, STAFCommandParser.VALUEREQUIRED);

            fServiceParser.addOptionNeed("SERVICE", "LIBRARY");
            fServiceParser.addOptionNeed("LIBRARY", "SERVICE");
            fServiceParser.addOptionNeed("SERVICE", "EXECUTE");
            fServiceParser.addOptionNeed("EXECUTE", "SERVICE");
            fServiceParser.addOptionNeed("OPTION", "SERVICE");
            fServiceParser.addOptionNeed("PARMS", "SERVICE");
            fServiceParser.addOptionNeed("ZIPPATH", "SERVICE");

            // Create a temporary directory for the STAF Http Service Loader
            // Service, if it doesn't already exist:
            //   <STAFDataDir>/tmp/service/<Service Name (lower-case)>;

            fTempDir = info.writeLocation + fFileSep + "tmp" + fFileSep +
                "service" + fFileSep + fServiceName.toLowerCase();

            File dir = new File(fTempDir);
            
            if (!dir.exists())
            {
                if (!dir.mkdirs())
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "Error creating service temporary data directory: " +
                        fTempDir);
                }
            }

            // Verify that the configuration file exists and that the syntax
            // is valid for the SERVICE entries that it contains

            ServiceLoadInfo serviceInfo = readConfigFile();

            if (serviceInfo.fRC != STAFResult.Ok)
            {
                errmsg += "Verifying CONFIGFILE parameter: RC=" +
                    serviceInfo.fRC + ", Result=" + serviceInfo.fResult;

                System.out.println(errmsg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, errmsg);
            }
        }
        catch (Exception e)
        {
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  e.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * This is the STAF Service method that accepts requests that are
     * submitted to this service.  Based on the first option specifed in
     * the request, it calls the appropriate method that handles that request.
     * Since this is a STAF Service Loader service, it must accept LOAD
     * requests with syntax:
     *   LOAD SERVICE <Service>
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

            // Call the appropriate method to handle the command request

            if (action.equalsIgnoreCase("LOAD"))
            {
                return handleLoadRequest(info);
            }
            else
            {
                return new STAFResult(
                    STAFResult.InvalidRequestString,
                    "'" + action + "' is not a valid command request for the " +
                    fServiceName + " service.  " +
                    "LOAD is the only valid command request.");
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

            StringWriter sw = new StringWriter();
            t.printStackTrace(new PrintWriter(sw));

            if (t.getMessage() != null)
            {
                return new STAFResult(
                    STAFResult.JavaError,
                    t.getMessage() + "\n" + sw.toString());
            }
            else
            {
                return new STAFResult(
                    STAFResult.JavaError, sw.toString());
            }
        }
    }
    
    /**
     * Handle a LOAD request for a service
     * 
     * @param info STAF Service request information
     * @return An instance of STAFResult which contains the return code
     * and result buffer.
     */
    private STAFResult handleLoadRequest(RequestInfo info)
    {
        // Parse the LOAD request

        STAFCommandParseResult parsedRequest = fLoadParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }
        
        String serviceToLoad = parsedRequest.optionValue("SERVICE");

        ServiceLoadInfo serviceInfo = readConfigFile(serviceToLoad, info);

        if (serviceInfo == null)
        {
            // The service was not found in the config file

            return new STAFResult(STAFResult.Ok);
        }
        else if (serviceInfo.fRC != STAFResult.Ok)
        {
            // An error occurred

            return new STAFResult(
                serviceInfo.fRC, serviceInfo.fResult);
        }

        String executable = serviceInfo.fExecute;

        try
        {
            URL url = new URL(executable);

            // Download the url to a temporary file in directory
            // fTempDir named <ServiceName>.jar  (or <ServiceName.zip
            // if the ZIPPATH option is specified)

            executable = fTempDir + fFileSep + serviceInfo.fService;

            if (serviceInfo.fZipPath == null)
                executable += ".jar";
            else
                executable += ".zip";

            File tempFile = new File(executable);

            STAFResult downloadResult = downloadFileWithRetries(
                url, tempFile);

            if (downloadResult.rc != STAFResult.Ok)
            {
                return downloadResult;
            }

            if (serviceInfo.fZipPath != null)
            {
                String toDir = fTempDir + fFileSep + serviceInfo.fService;

                // Unzip the downloaded file

                String unzipRequest = "UNZIP ZIPFILE " +
                    STAFUtil.wrapData(executable) +
                    " FILE " + STAFUtil.wrapData(serviceInfo.fZipPath) +
                    " TODIRECTORY " + STAFUtil.wrapData(toDir) +
                    " REPLACE";

                STAFResult unzipResult = fHandle.submit2(
                    "local", "ZIP", unzipRequest);

                if (unzipResult.rc != STAFResult.Ok)
                {
                    return new STAFResult(
                        unzipResult.rc,
                        "The file contained in url '" + executable +
                        "' specified by the EXECUTE option cannot be " +
                        "unzipped by the ZIP service.  " +
                        "'STAF local ZIP " + unzipRequest +
                        "' failed with RC=" + unzipResult.rc + ", Result=" +
                        unzipResult.result +
                        "  The STAFHTTPSLS configuration file '" +
                        fConfigFile +
                        "' probably contains an invalid value for " +
                        "the EXECUTE option for service " +
                        serviceInfo.fService);
                }

                // Delete the temporary zipfile

                try
                {
                    tempFile.delete();
                }
                catch (Exception e)
                {
                    // Ignore any exceptions that could occur
                }

                executable = toDir + fFileSep + serviceInfo.fZipPath;
            }
        }
        catch (MalformedURLException e)
        {
            // The EXECUTE value did not specify a URL.
            // Check if the executable exists on the local machine.

            if (!(new File(executable)).exists())
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "File '" + executable + "' does not exist.  " +
                    "The STAFHTTPSLS configuration file '" +
                    fConfigFile + "' contains an invalid value for the " +
                    "EXECUTE option for service " + serviceInfo.fService);
            }
        }

        String requestString = "ADD SERVICE " + serviceInfo.fService +
            " LIBRARY JSTAF EXECUTE " + STAFUtil.wrapData(executable);

        if ((serviceInfo.fParms != null) &&
            !serviceInfo.fParms.equals(EMPTY_STRING))
        {
            requestString = requestString + " PARMS " +
                STAFUtil.wrapData(serviceInfo.fParms);
        }

        if ((serviceInfo.fOptions != null) &&
            !serviceInfo.fOptions.equals(EMPTY_STRING))
        {
            requestString = requestString + serviceInfo.fOptions;
        }
        
        return fHandle.submit2("local", "SERVICE", requestString);
    }
    
    /**
     * Reads the configuration file for the HTTP Service Loader Service and
     * does one of two things:
     * 
     * 1) If serviceToLoad is not null, searches the config file for this
     * service and, if found, loads the service by submitting an ADD request
     * to the SERVICE service and returns an instance of ServiceLoadInfo
     * with the information provided to register the service and return rc 0.
     * If the service is not found in the config file, will return null.
     * 
     * 2) If serviceToLoad is null, this means that it's being called during
     * the init() phase and is simply verifying that the syntax for each
     * SERVICE statement in the configuration file is valid.  The
     * loadRequestInfo will also be null in this case.  No attempt will be
     * made to download a url specified in an EXECUTE option and it won't
     * try to load the service.  An instance of ServiceLoadInfo will be
     * returned with the rc set to 0 if the syntax in the config file is valid
     * or the rc will be set to non-zero rc if the syntax in the config file
     * is not valid and the result will contain an error message.
     * 
     * @param serviceToLoad The name of the service to load or null if
     * being called during the init() phase to verify the syntax for each
     * SERVICE statement in the config file.
     * @param loadRequestInfo The RequestInfo instance representing who
     * submitted the load request.  It will be null if serviceToLoad is null.
     *
     * @return ServiceLoadInfo
     */

    private ServiceLoadInfo readConfigFile()
    {
        return readConfigFile(null, null);
    }

    private ServiceLoadInfo readConfigFile(String serviceToLoad,
                                           RequestInfo loadRequestInfo)
    {
        boolean verifyOnly = false;

        if (serviceToLoad == null)
            verifyOnly = true;    
        
        BufferedReader configFileReader = null;
        
        // Download the configuration file if needed

        STAFResult res = downloadConfigFile(fConfigFile, serviceToLoad);

        if (res.rc != STAFResult.Ok)
        {
            return new ServiceLoadInfo(
                res.rc, "Invalid CONFIGFILE for STAFHTTPSLS: RC=" +
                res.rc + ", Result=" + res.result);
        }

        STAFCommandParseResult parsedRequest;
        String configFile = res.result;
        String line = EMPTY_STRING;
        
        try
        {
            configFileReader = new BufferedReader(new FileReader(
                new File(configFile)));
            
            String thisLine = null;
            
            while ((thisLine = configFileReader.readLine()) != null)
            {
                thisLine = thisLine.trim();
                
                // Skip empty lines and comment lines in the config file

                if (thisLine.equals(EMPTY_STRING) ||
                    thisLine.startsWith(COMMENT_MARKER))
                {
                    continue;
                }
                
                if (line.length() > 0)
                    line += " ";

                // Check if this line continues on (ends in a \)

                if (thisLine.substring(thisLine.length() - 1,
                                       thisLine.length()).
                    equals(CONTINUATION_LINE_MARKER))
                {
                    line += thisLine.substring(0, thisLine.length() - 1).
                        trim();

                    continue;
                }
                
                line += thisLine;

                parsedRequest = fServiceParser.parse(line);
                    
                if (parsedRequest.rc != STAFResult.Ok)
                {
                    return new ServiceLoadInfo(
                        STAFResult.InvalidRequestString,
                        parsedRequest.errorBuffer +
                        "  The STAFHTTPSLS configuration file '" +
                        fConfigFile +
                        "' contains invalid syntax in the following line: " +
                        line);
                }
                    
                String service = parsedRequest.optionValue("SERVICE");
                    
                if (!verifyOnly)
                {
                    if (!service.equalsIgnoreCase(serviceToLoad))
                    {
                        // Reset line for the next statement
                        line = EMPTY_STRING;

                        // Skip lines for other services in the config file
                        continue;
                    }
                }
                
                String library = parsedRequest.optionValue("LIBRARY");

                if (!library.equalsIgnoreCase("JSTAF"))
                {
                    // Return an error if not loading a Java service

                    return new ServiceLoadInfo(
                        STAFResult.InvalidValue,
                        "The LIBRARY option value is not JSTAF.  The " +
                        "STAFHTTPSLS configuration file '" +
                        fConfigFile + "' contains an invalid value for " +
                        "LIBRARY in the following line: " + line);
                }

                String executable = parsedRequest.optionValue("EXECUTE");

                if (loadRequestInfo != null)
                {
                    // Resolve the value specified for EXECUTE

                    res = STAFUtil.resolveRequestVar(
                        parsedRequest.optionValue("EXECUTE"),
                        fHandle, loadRequestInfo.requestNumber);

                    if (res.rc != 0)
                    {
                        return new ServiceLoadInfo(
                            res.rc, res.result +
                            "  The STAFHTTPSLS configuration file '" +
                            fConfigFile +
                            "' contains an invalid value for EXECUTE in the" +
                            " following line: " + line);
                    }

                    executable = res.result;
                }

                String zipPath = null;

                if (parsedRequest.optionTimes("ZIPPATH") > 0)
                    zipPath = parsedRequest.optionValue("ZIPPATH");
                
                String parms = parsedRequest.optionValue("PARMS");
                        
                String options = null;

                if (parsedRequest.optionTimes("OPTION") > 0)
                {
                    options = EMPTY_STRING;

                    for (int i = 0; i < parsedRequest.optionTimes("OPTION");
                         i++)
                    {
                        options = options + " OPTION " +
                            STAFUtil.wrapData(parsedRequest.optionValue(
                                "OPTION", i+1));
                    }
                }

                if (verifyOnly)
                {
                    // Reset line for the next SERVICE statement
                    line = EMPTY_STRING;

                    // Verify the syntax for the next SERVICE statement
                    continue;
                }

                return new ServiceLoadInfo(
                    service, executable, zipPath, parms, options);
            }

            if (!verifyOnly)
            {
                // Service was not found in the configFile

                return null;
            }
            else
            {
                // Verified that the syntax for all services in the config
                // file is valid

                return new ServiceLoadInfo(STAFResult.Ok, "");
            }
        }
        catch (FileNotFoundException e)
        {
            return new ServiceLoadInfo(
                STAFResult.DoesNotExist, e.toString());
        }
        catch (IOException e)
        {
            return new ServiceLoadInfo(
                STAFResult.FileReadError, e.toString());
        }
        finally
        {
            // Close the config file

            if (configFileReader != null)
            {
                try
                {
                    configFileReader.close();
                }
                catch (Exception e)
                {
                    // Ignore
                }
            }

            // If a url is specified for the STAFHTTSLS CONFIGFILE, delete
            // the temporary config file where the url was downloaded to

            if (configFile != fConfigFile)
            {
                try
                {
                    (new File(configFile)).delete();
                }
                catch (Exception e)
                {
                    // Ignore any exceptions that could occur
                }
            }
        }
    }
    
    /**
     * This is the STAF Service termination method that is run when the
     * service loader service is unregistered.  It performs termination
     * functions such as unregistering the STAF handle it used to submit
     * requests to STAF services
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the service terminated successfully.
     */ 
    public STAFResult term()
    {
        try
        {
            // Un-register the service handle

            fHandle.unRegister();
        }
        catch (STAFException ex)
        {
            return new STAFResult(
                STAFResult.STAFRegistrationError, ex.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }
    
    /**
     * Downloads the specified url into the specified file on the local
     * machine.  If a download attempt fails, it will retry up to the
     * maximum number of download attempts, sleeping for the specified
     * number of seconds before retrying.
     * @param url  A URL specifying a file to be downloaded
     * @param targetFile  A File on the local machine where the url is to be
     *                    downloaded to
     * @return A STAFResult containing the return code and result string
     *         where a return code of 0 indicates the file was downloaded
     *         successfully
     */
    private STAFResult downloadFileWithRetries(URL url, File targetFile)
    {
        STAFResult res = new STAFResult();
            
        for (int i = 0; i < fDownloadAttempts + 1; i++)
        {
            res = downloadFile(url, targetFile);
                
            if (res.rc != STAFResult.Ok)
            {
                if ((i + 1) < fDownloadAttempts)
                {
                    try
                    {
                        Thread.sleep(fDownloadRetryDelay * 1000);
                    }
                    catch (Exception e)
                    {
                        return new STAFResult(
                            STAFResult.FileReadError,
                            "Error while sleeping between attempts to " +
                            "download " + url + ".  " + e.toString());
                    }
                }
            }
            else
            {
                // Downloaded successfully
                return res;
            }
        }
            
        // Return error
        return res;
    }
    
    /**
     * Downloads the specified url into the specified file on the local
     * machine.
     * @param url  A URL specifying a file to be downloaded
     * @param targetFile  A File on the local machine where the url is to be
     *                    downloaded to
     * @return A STAFResult containing the return code and result string
     *         where a return code of 0 indicates the file was downloaded
     *         successfully
     */
    private STAFResult downloadFile(URL url, File targetFile)
    {
        OutputStream out = null;
        InputStream in = null;

        try
        {
            String parentOfTarget = targetFile.getParent();
            
            if (parentOfTarget != null)
            {
                (new File(parentOfTarget)).mkdirs();
            }
            
            out = new BufferedOutputStream(new FileOutputStream(targetFile));
            in = new BufferedInputStream(url.openStream());

            byte[] aByte = new byte[BUFFER_SIZE];
            int totalBytesRead = 0;
            int count;
            
            while ((count = in.read(aByte, 0, aByte.length)) > 0)
            {
                totalBytesRead += count;
                out.write(aByte, 0, count);
            }
            
            // Check if there was an authorization issue to access a url.
            // Currently, the only way we know how to do this is to check if
            // no bytes were read.

            if (totalBytesRead == 0)
            {
                return new STAFResult(
                    STAFResult.FileReadError,
                    "Downloading url " + url + " failed.  Verify you have " +
                    "read access for this url and that this url does not " +
                    "point to an empty file.");
            }

            return new STAFResult(STAFResult.Ok);
        }
        catch (FileNotFoundException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.toString());
        }
        catch (Exception e)
        {
            return new STAFResult(STAFResult.FileReadError, e.toString());
        }
        finally
        {
            if (in != null)
            {
                try
                {
                    in.close();
                }
                catch (IOException e)
                {
                    // Ignore
                }
            }
            if (out != null)
            {
                try
                {
                    out.close();
                }
                catch (IOException e)
                {
                    // Ignore
                }
            }
        }
    }

    /**
     * Downloads the specified configuration file into a temporary file on the
     * local machine.
     * @param configFile  A string specifying the name of a url or file to be 
     *                    downloaded
     * @param serviceName  The name of the service to be loaded or null if
     *                     it's being called during the init() phase and is
     *                     simply verifying that the syntax for each SERVICE
     *                     statement in the configuration file is valid.
     * @return An instance of STAFResult containing the return code and result
     *         string where a return code of 0 indicates the file was
     *         downloaded successfully and the result then contains the name
     *         of the temporary file where the config file was downloaded to
     */
    private STAFResult downloadConfigFile(String configFile,
                                          String serviceName)
    {
        try
        {
            URL url = new URL(configFile);

            String tempConfigFile = fTempDir + fFileSep;

            if (serviceName != null)
            {
                // Download the config file to <fTempDir>/<ServiceName>.cfg
                tempConfigFile += serviceName + ".cfg";
            }
            else
            {
                // Download the config file to <fTempDir>/STAFHTTPSLS.cfg
                tempConfigFile += "STAFHTTPSLS.cfg";
            }

            STAFResult downloadResult = downloadFile(
                url, new File(tempConfigFile));

            if (downloadResult.rc != STAFResult.Ok)
            {
                return downloadResult;
            }

            configFile = tempConfigFile;
        }
        catch (MalformedURLException e)
        {
            // The EXECUTE value did not specify a URL.
            // Check if the executable exists on the local machine.

            if (!(new File(configFile)).exists())
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "File '" + configFile + "' does not exist.");
            }
        }

        return new STAFResult(STAFResult.Ok, configFile);
    }

    /**
     * Parses the PARMS value specified for the HTTP Service Loader and
     * verifies that the parameters specified are valid.
     * @param info STAF service initialization information
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the parameters are valid.
     */
    private STAFResult handleParms(InitInfo info)
    {
        STAFCommandParseResult parseResult= fParmsParser.parse(info.parms);
        
        String errmsg = "ERROR:  Service Configuration Error for Service " +
            fServiceName + ": ";

        if (parseResult.rc != STAFResult.Ok)
        {
            errmsg += "PARMS parsing failed with RC=" +
                STAFResult.InvalidRequestString +
                " Result=" + parseResult.errorBuffer;

            System.out.println(errmsg);

            return new STAFResult(
                STAFResult.ServiceConfigurationError, errmsg);
        }

        STAFResult res = STAFUtil.resolveInitVar(
            parseResult.optionValue("CONFIGFILE"), fHandle);

        if (res.rc != STAFResult.Ok)
        {
            errmsg += "Error resolving CONFIGFILE.  RC=" + res.rc +
                " Result=" + res.result;

            System.out.println(errmsg);

            return new STAFResult(
                STAFResult.ServiceConfigurationError, errmsg);
        }
                
        fConfigFile = res.result;

        if (parseResult.optionTimes("DOWNLOADATTEMPTS") > 0)
        {
            res = STAFUtil.resolveInitVarAndCheckInt(
                "DOWNLOADATTEMPTS",
                parseResult.optionValue("DOWNLOADATTEMPTS"), fHandle);

            if (res.rc != STAFResult.Ok)
            {
                errmsg += "Invalid DOWNLOADATTEMPTS value.  RC=" +
                    res.rc + " Result=" + res.result;

                System.out.println(errmsg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, errmsg);
            }

            fDownloadAttempts = Integer.parseInt(res.result);
        }

        if (parseResult.optionTimes("DOWNLOADRETRYDELAY") > 0)
        {
            res = STAFUtil.resolveInitVarAndCheckInt(
                "DOWNLOADRETRYDELAY",
                parseResult.optionValue("DOWNLOADRETRYDELAY"), fHandle);

            if (res.rc != STAFResult.Ok)
            {
                errmsg += "Invalid DOWNLOADRETRYDELAY value.  RC=" +
                    res.rc + " Result=" + res.result;

                System.out.println(errmsg);

                return new STAFResult(
                    STAFResult.ServiceConfigurationError, errmsg);
            }

            fDownloadRetryDelay = Integer.parseInt(res.result);
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     *  Helper class for storing data for a Java service to be registered
     */
    class ServiceLoadInfo
    {
        private ServiceLoadInfo()
        {
            // Do nothing
        }
        
        ServiceLoadInfo(int rc, String result)
        {
            fRC = rc;
            fResult = result;
        }

        ServiceLoadInfo(String service, String execute, String zipPath,
                        String parms, String options)
        {
            fService = service;
            fExecute = execute;
            fZipPath = zipPath;
            fParms = parms;
            fOptions = options;
            fRC = 0;
            fResult = "";
        }

        private String fService = null;
        private String fExecute = null;
        private String fZipPath = null;
        private String fParms = null;
        private String fOptions = null;
        private int fRC = STAFResult.Ok;
        private String fResult = "";
    }
}
