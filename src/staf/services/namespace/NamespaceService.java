/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.util.Calendar;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.TreeMap;
import java.util.Iterator;
import java.text.SimpleDateFormat;
import java.io.File;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.Date;

/**
 * Represents the Namespace service which is a STAF service whose purpose
 * is to provide a namespace hierarchy for storing and retrieving a
 * persistent repository of variables.  The Namespace service allows
 * namespaces to hold a set of key/value pairs (e.g. variables).
 * Namespaces may inherit variables from another namespace, thus creating
 * a namespace hierarchy.  Variable look-ups will be done within a namespace
 * scope.  If a variable cannot be found in the given namespace, resolution
 * will be attempted in the parent namespace, and so on up the hierarchy.
 * <p>
 * Unlike the VAR service, any variables set will persist across stops and
 * restarts of STAF with no additional steps required by the user.  This is
 * done by immediately updating the Namespaces XML file when any updates
 * are made to namespaces.
 */ 
public class NamespaceService implements STAFServiceInterfaceLevel30
{
    // Namespace service version
    private final String kVersion = "1.0.3";

    // Version of STAF (or later) required for this service
    private final String kRequiredSTAFVersion = "3.0.0";

    // Namespace Service Error Codes
    /**
     * Error code for when an error occurs writing the Namespaces data to
     * persistent storage
     */ 
    public static final int kDataStorageError = 4001;

    private static String sHelpMsg;

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    private String fServiceName;
    private STAFHandle fHandle;
    private String fLocalMachineName = "";
    private String fLineSep;
    private String fFileSep;
    private String fDataDir = null;
    private String fShortFileName = null;
    private String fTempDir;
    private File fDataFile;    // Namespaces.xml file
    private File fBackupFile;  // Namespaces.xml.backup file

    private NamespaceManager fNamespaceManager = new NamespaceManager();
    private StorageManager fStorageManager = null;
    
    // STAFCommandParsers for each request
    private STAFCommandParser fCreateParser;
    private STAFCommandParser fModifyParser;
    private STAFCommandParser fDeleteParser;
    private STAFCommandParser fListParser;
    private STAFCommandParser fQueryParser;
    private STAFCommandParser fSetParser;
    private STAFCommandParser fGetParser;
    private STAFCommandParser fVersionParser;
    private STAFCommandParser fHelpParser;
    private STAFCommandParser fParmsParser;
    
    // Map Class Definitions used to create marshalled results
    private static STAFMapClassDefinition fListNamespacesMapClass;
    private static STAFMapClassDefinition fListVariablesMapClass;
    private static STAFMapClassDefinition fSettingsMapClass;
    private static STAFMapClassDefinition fQueryfNamespaceMapClass;
    private static STAFMapClassDefinition fQueryTreeMapClass;

    /**
     * Creates a new NamespaceService instance
     */ 
    public NamespaceService() {}

    /**
     * This is the STAF Service initialization method that is run when the
     * service is registered.  It performs initialization functions such as:
     * <ul>
     * <li>Creates a STAF handle to use to submit requests to STAF services
     * <li>Resolves any local variables needed by the service
     * <li>Creates request parsers
     * <li>Creates map classes uses in list/query results
     * <li>Creates the data directory for the service (if it doesn't already 
     * exist
     * <li>Creates the storage manager which loads the namespaces from the
     * Namespaces xml file, if the file exists.
     * </ul>
     * 
     * @param info STAF service initialization information
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the service initialized successfully.
     */ 
    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        try
        {
            fServiceName = info.name;
            fHandle = new STAFHandle("STAF/Service/" + info.name);
            STAFResult res = new STAFResult();
            
            // Resolve the machine name variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLocalMachineName = res.result;
            
            // Resolve the line separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fLineSep = res.result;

            // Resolve the file separator variable for the local machine

            res = STAFUtil.resolveInitVar("{STAF/Config/Sep/File}", fHandle);

            if (res.rc != STAFResult.Ok) return res;

            fFileSep = res.result;

            // CREATE parser
        
            fCreateParser = new STAFCommandParser();

            fCreateParser.addOption(
                "CREATE", 1, STAFCommandParser.VALUENOTALLOWED);

            fCreateParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);
            
            fCreateParser.addOption(
                "DESCRIPTION", 1, STAFCommandParser.VALUEREQUIRED);

            fCreateParser.addOption(
                "PARENT", 1, STAFCommandParser.VALUEREQUIRED);
            
            // if you specify CREATE, the NAMESPACE option is required
            fCreateParser.addOptionNeed("CREATE", "NAMESPACE");

            // if you specify CREATE, the DESCRIPTION option is required
            fCreateParser.addOptionNeed("CREATE", "DESCRIPTION");

            // MODIFY parser
        
            fModifyParser = new STAFCommandParser();

            fModifyParser.addOption(
                "MODIFY", 1, STAFCommandParser.VALUENOTALLOWED);

            fModifyParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);
            
            fModifyParser.addOption(
                "DESCRIPTION", 1, STAFCommandParser.VALUEREQUIRED);

            fModifyParser.addOption(
                "PARENT", 1, STAFCommandParser.VALUEREQUIRED);
            
            // if you specify MODIFY, the NAMESPACE option is required
            fModifyParser.addOptionNeed("MODIFY", "NAMESPACE");

            // DELETE parser

            fDeleteParser = new STAFCommandParser();

            fDeleteParser.addOption(
                "DELETE", 1, STAFCommandParser.VALUENOTALLOWED);

            fDeleteParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);

            fDeleteParser.addOption(
                "VAR", 0, STAFCommandParser.VALUEREQUIRED);

            fDeleteParser.addOption(
                "CONFIRM", 1, STAFCommandParser.VALUENOTALLOWED);

            // If you specify DELETE, the NAMESPACE option is required
            fDeleteParser.addOptionNeed("DELETE", "NAMESPACE");

            // If you specify the VAR option, the NAMESPACE option is required
            fDeleteParser.addOptionNeed("VAR", "NAMESPACE");

            // If you specify the CONFIRM option, the NAMESPACE option is
            // required
            fDeleteParser.addOptionNeed("CONFIRM", "NAMESPACE");

            // You must specify either VAR or CONFIRM, but not both
            fDeleteParser.addOptionGroup("VAR CONFIRM", 1, 1);

            // LIST parser

            fListParser = new STAFCommandParser();

            fListParser.addOption(
                "LIST", 1, STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption(
                "NAMESPACES", 1, STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);

            fListParser.addOption(
                "ONLY", 1, STAFCommandParser.VALUENOTALLOWED);

            fListParser.addOption(
                "SETTINGS", 1, STAFCommandParser.VALUENOTALLOWED);

            // You can specify only 1 of NAMESPACES or NAMESPACE or SETTINGS
            fListParser.addOptionGroup("NAMESPACES NAMESPACE SETTINGS", 0, 1);

            fListParser.addOptionNeed("NAMESPACES", "LIST");
            fListParser.addOptionNeed("NAMESPACE", "LIST");
            fListParser.addOptionNeed("SETTINGS", "LIST");
            fListParser.addOptionNeed("ONLY", "NAMESPACE");
            
            // QUERY parser

            fQueryParser = new STAFCommandParser();

            fQueryParser.addOption(
                "QUERY", 1, STAFCommandParser.VALUENOTALLOWED);

            fQueryParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);

            fQueryParser.addOption(
                "TREE", 1, STAFCommandParser.VALUENOTALLOWED);

            // If you specify QUERY, the NAMESPACE option is required
            fQueryParser.addOptionNeed("QUERY", "NAMESPACE");

            // SET parser

            fSetParser = new STAFCommandParser();

            fSetParser.addOption(
                "SET", 1, STAFCommandParser.VALUENOTALLOWED);

            fSetParser.addOption(
                "VAR", 0, STAFCommandParser.VALUEREQUIRED);

            fSetParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);

            // If you specify SET, at least one VAR option is required and
            // the NAMESPACE option is required
            fSetParser.addOptionNeed("SET", "VAR");
            fSetParser.addOptionNeed("SET", "NAMESPACE");

            // GET parser

            fGetParser = new STAFCommandParser();

            fGetParser.addOption(
                "GET", 1, STAFCommandParser.VALUENOTALLOWED);

            fGetParser.addOption(
                "VAR", 1, STAFCommandParser.VALUEREQUIRED);

            fGetParser.addOption(
                "NAMESPACE", 1, STAFCommandParser.VALUEREQUIRED);

            // If you specify GET, the VAR and NAMESPACE options are required
            fGetParser.addOptionNeed("GET", "VAR");
            fGetParser.addOptionNeed("GET", "NAMESPACE");

            // VERSION parser

            fVersionParser = new STAFCommandParser();
            fVersionParser.addOption(
                "VERSION", 1, STAFCommandParser.VALUENOTALLOWED);

            // HELP parser

            fHelpParser = new STAFCommandParser();
            fHelpParser.addOption(
                "HELP", 1, STAFCommandParser.VALUENOTALLOWED);

            // Construct map class for the result from a LIST NAMESPACES request

            fListNamespacesMapClass = new STAFMapClassDefinition(
                "STAF/Service/Namespace/NamespaceInfo");
            fListNamespacesMapClass.addKey("name",        "Name");
            fListNamespacesMapClass.addKey("description", "Description");
            fListNamespacesMapClass.addKey("parent",      "Parent");

            // Construct map class for the result from a LIST VARIABLES request

            fListVariablesMapClass = new STAFMapClassDefinition(
                "STAF/Service/Namespace/VarInfo");
            fListVariablesMapClass.addKey("key",       "Key");
            fListVariablesMapClass.addKey("value",     "Value");
            fListVariablesMapClass.addKey("namespace", "Namespace");

            // Construct map class for the result from a LIST SETTINGS request

            fSettingsMapClass = new STAFMapClassDefinition(
                "STAF/Service/Namespace/Settings");
            fSettingsMapClass.addKey("directory", "Directory");
            fSettingsMapClass.addKey("filename",  "File Name");

            // Construct map class for the result from a QUERY request

            fQueryfNamespaceMapClass = new STAFMapClassDefinition(
                "STAF/Service/Namespace/Query");
            fQueryfNamespaceMapClass.addKey("name",        "Name");
            fQueryfNamespaceMapClass.addKey("description", "Description");
            fQueryfNamespaceMapClass.addKey("parent",      "Parent");
            fQueryfNamespaceMapClass.addKey("children",    "Children");

            // Construct map class for the result from a QUERY TREE request

            fQueryTreeMapClass = new STAFMapClassDefinition(
                "STAF/Service/Namespace/QueryTree");
            fQueryTreeMapClass.addKey("name",     "Name");
            fQueryTreeMapClass.addKey("children", "Children");
            
            // Assign the help text string for the service

            sHelpMsg = "*** " + fServiceName + " Service Help ***" +
                fLineSep + fLineSep +
                "CREATE  NAMESPACE <Name> DESCRIPTION <Description> [PARENT <Name>]" +
                fLineSep +
                "MODIFY  NAMESPACE <Name> [DESCRIPTION <Description>] [PARENT <Name>]" +
                fLineSep +
                "DELETE  NAMESPACE <Name> < VAR <Key>... | CONFIRM >" +
                fLineSep +
                "LIST    [NAMESPACES | <NAMESPACE <Name> [ONLY]> | SETTINGS]" +
                fLineSep +
                "QUERY   NAMESPACE <Name> [TREE]" +
                fLineSep +
                "SET     VAR <Key=Value> [VAR <Key=Value>]... NAMESPACE <Name>" +
                fLineSep +
                "GET     VAR <Key> NAMESPACE <Name>" +
                fLineSep +
                "VERSION" +
                fLineSep +
                "HELP";

            // Register Help Data

            registerHelpData(
                kDataStorageError,
                "Data storage error",
                "An error occurred saving namespaces data to persistent " +
                "storage. Additional information about the error is put " +
                "into the STAF Result.");

            // Create service data directory if it doesn't already exist:
            //   <STAFDataDir>/service/<service name (lower-case)>

            fDataDir = info.writeLocation + fFileSep + "service" + fFileSep +
                fServiceName.toLowerCase();

            // Check if service data directory and/or filename is being
            // overridden in the PARMS for this service

            if (info.parms != null && info.parms.length() > 0)
            {
                res = handleParms(info);

                if (res.rc != STAFResult.Ok)
                {
                    return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                        "Error validating parameters: RC=" + res.rc +
                        ", Result=" + res.result);
                }
            }

            File dir = new File(fDataDir);
            
            if (!dir.exists())
            {
                if (!dir.mkdirs())
                {
                    return new STAFResult(
                        STAFResult.ServiceConfigurationError,
                        "Error creating service data directory: " + fDataDir);
                }
            }
            else if (!dir.isDirectory())
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Service data directory \"" + fDataDir +
                    "\" is not a directory");
            }
            
            // Create service directory in the STAF temporary data directory,
            // if it doesn't already exist:
            //   <STAFDataDir>/tmp/service/<Service Name (lower-case)>;

            fTempDir = info.writeLocation + fFileSep + "tmp" + fFileSep +
                "service" + fFileSep + fServiceName.toLowerCase();

            dir = new File(fTempDir);
            
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
            else if (!dir.isDirectory())
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Service temporary data directory \"" + fTempDir +
                    "\" is not a directory");
            }

            // Create the storage manager (which loads the namespaces from
            // the xml file if it exists)

            try
            {
                if (fShortFileName == null)
                {
                    fStorageManager = new StorageManager(
                        fDataDir, fNamespaceManager);
                }
                else
                {
                    fStorageManager = new StorageManager(
                        fDataDir, fShortFileName, fNamespaceManager);
                }
            }
            catch (Exception e)
            {
                // Log to service log and print stack trace to JVM log

                String errorMsg = "Error loading namespaces from persistent" +
                    " storage. " + fLineSep + e;
                logMessage("Error", errorMsg, false, null);

                e.printStackTrace();

                return new STAFResult(STAFResult.STAFRegistrationError,
                                      e.toString());
            }
            
            fDataFile = fStorageManager.getFile();

            fBackupFile = new File(
                fDataDir + fFileSep + fStorageManager.getShortFileName() +
                ".backup");
            
            logMessage("Start", "Initialized the " + fServiceName + " service",
                       false, null);

            // Save a copy of the Namespaces file in a temporary directory

            copyData("init");
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  e.toString());
        }
        catch (Throwable t)
        {
            /* Unexpected Exception. Log to JVM Log */
            t.printStackTrace();
            return new STAFResult(STAFResult.JavaError, t.getMessage());
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Gets the instance of the NamespaceManager used by the Namespace service
     * @return an instance of the NamespaceManager
     */ 
    public NamespaceManager getNamespaceManager()
    {
        return fNamespaceManager;
    }

    /**
     * This is the STAF Service method that accepts requests that are
     * submitted to this service.  Based on the first option specifed in
     * the request, it calls the appropriate method that handles that request.
     * @param info STAF Service request information
     * @return An instance of STAFResult which contains the return code
     * and result buffer.
     */ 
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

            if (actionLC.equals("create"))
                return handleCreate(info);
            else if (actionLC.equals("modify"))
                return handleModify(info);
            else if (actionLC.equals("delete"))
                return handleDelete(info);
            else if (actionLC.equals("list"))
                return handleList(info);
            else if (actionLC.equals("query"))
                return handleQuery(info);
            else if (actionLC.equals("set"))
                return handleSet(info);
            else if (actionLC.equals("get"))
                return handleGet(info);
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

    /**
     * Handle a HELP request
     */ 
    private STAFResult handleHelp(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "HELP", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fHelpParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Return help text for the service

        return new STAFResult(STAFResult.Ok, sHelpMsg);
   }

    /**
     * Handle a VERSION request
     */ 
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

        // Return the service's version

        return new STAFResult(STAFResult.Ok, kVersion);
    }

    /**
     * Handle a CREATE request
     */ 
    private STAFResult handleCreate(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, fServiceName, "CREATE", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fCreateParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Resolve any STAF variables in the NAMESPACE option's value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("NAMESPACE"),
            fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;

        String namespace = res.result;

        // Don't resolve STAF variables in the DESCRIPTION option's value

        String description = parsedRequest.optionValue("DESCRIPTION");

        String parent = Namespace.sNONE;

        if (parsedRequest.optionTimes("PARENT") > 0)
        {
            // Resolve any STAF variables in the PARENT option's value

            res = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("PARENT"),
                fHandle, info.requestNumber);

            if (res.rc != STAFResult.Ok) return res;
        
            parent = res.result;
        }
        
        Namespace parentNS = null;

        // Synchronized on the Namespace service so that only one request
        // to the Namespace service can run at a time (to ensure that nothing
        // else can change the Namespace data while this code is running).

        synchronized(this)
        {
            // Check if the namespace already exists

            if (fNamespaceManager.get(namespace) != null)
            {
                return new STAFResult(
                    STAFResult.AlreadyExists,
                    "Namespace '" + namespace + "' already exists");
            }

            if (!parent.equalsIgnoreCase(Namespace.sNONE))
            {
                // Get the parent namespace

                parentNS = fNamespaceManager.get(parent);

                if (parentNS == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Parent namespace '" + parent + "' does not exist"); 
                }

                // Get the right casing for the parent name

                parent = parentNS.getName();
            }
        
            // Create a new namespace and add to the namespace map

            Namespace ns = new Namespace(namespace, description, parent);

            fNamespaceManager.create(namespace, ns);

            if (parentNS != null)
            {
                parentNS.addChild(namespace, ns);
            }

            // Save namespaces to persistent data storage

            res = storeData(info);

            if (res.rc != STAFResult.Ok) return res;
        }                

        return new STAFResult(STAFResult.Ok);
    }
    
    /**
     * Handle a MODIFY request
     */ 
    private STAFResult handleModify(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, fServiceName, "MODIFY", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fModifyParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Resolve any STAF variables in the NAMESPACE option's value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("NAMESPACE"),
            fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String namespace = res.result;

        String description = null;

        if (parsedRequest.optionTimes("DESCRIPTION") > 0)
        {
            // Don't resolve STAF variables in the DESCRIPTION option's value

            description = parsedRequest.optionValue("DESCRIPTION");
        }
        
        String parent = null;

        if (parsedRequest.optionTimes("PARENT") > 0)
        {
            // Resolve any STAF variables in the PARENT option's value

            res = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("PARENT"),
                fHandle, info.requestNumber);

            if (res.rc != STAFResult.Ok) return res;
        
            parent = res.result;
        }

        if ((parsedRequest.optionTimes("DESCRIPTION") == 0) &&
            (parsedRequest.optionTimes("PARENT") == 0))
        {
            return new STAFResult(
                STAFResult.InvalidRequestString,
                "You must have at least 1 of the option(s), " +
                "DESCRIPTION PARENT");
        }

        // Synchronized on the Namespace service so that only one request
        // to the Namespace service can run at a time (to ensure that nothing
        // else can change the Namespace data while this code is running).

        synchronized(this)
        {
            // Make sure the namespace already exists

            Namespace ns = fNamespaceManager.get(namespace);

            if (ns == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "Namespace '" + namespace + "' does not exist");
            }
        
            namespace = ns.getName();

            if (parent != null)
            {
                // Make sure the parent namespace already exists

                if (!parent.equalsIgnoreCase(Namespace.sNONE))
                {
                    // Get the parent namespace

                    Namespace parentNS = fNamespaceManager.get(parent);

                    if (parentNS == null)
                    {
                        return new STAFResult(
                            STAFResult.DoesNotExist,
                            "Parent namespace '" + parent +
                            "' does not exist"); 
                    }
                    
                    parent = parentNS.getName();
            
                    // Verify the change of parent will not create a
                    // cyclic dependency by walking up the parent tree
                    // and making sure that the namespace name is not found
                    
                    String nsName = parent;

                    while (!nsName.equalsIgnoreCase(Namespace.sNONE))
                    {
                        if (nsName.equalsIgnoreCase(namespace))
                        {
                            return new STAFResult(
                                STAFResult.InvalidValue,
                                "Parent namespace '" + nsName +
                                "' is invalid due to a cyclic dependency.");
                        }
                        else
                        {
                            nsName = fNamespaceManager.get(nsName).
                                getParent();
                        }
                    }
                    
                    // Add namespace to new parent's children

                    parentNS.addChild(namespace, ns);
                }

                // Remove namespace from previous parent's children

                String prevParent = ns.getParent();

                if (!prevParent.equalsIgnoreCase(Namespace.sNONE))
                {
                    Namespace prevParentNS = fNamespaceManager.
                        get(prevParent);

                    prevParentNS.removeChild(namespace);
                }

                // Update the namespace's parent

                ns.setParent(parent);
            }

            if (description != null)
            {
                // Update the description for the namespace

                ns.setDescription(description);
            }

            // Save namespaces to persistent data storage

            res = storeData(info);

            if (res.rc != STAFResult.Ok) return res;
        }
        
        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Handle a LIST request
     */ 
    private STAFResult handleList(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "LIST", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fListParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        STAFMarshallingContext mc = new STAFMarshallingContext();

        // Create an empty result list to contain the result
        List resultList = new ArrayList();

        // Check if specified namespace

        if (parsedRequest.optionTimes("SETTINGS") > 0)
        {
            // LIST SETTINGS
            
            mc.setMapClassDefinition(fSettingsMapClass);

            Map settingsMap = fSettingsMapClass.createInstance();
            settingsMap.put("directory", fDataDir);
            settingsMap.put("filename", fStorageManager.getShortFileName());

            mc.setRootObject(settingsMap);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else if (parsedRequest.optionTimes("NAMESPACE") > 0)
        {
            // LIST NAMESPACE [ONLY]

            // Resolve any STAF variables in the NAMESPACE option's value
    
            STAFResult res = STAFUtil.resolveRequestVar(
                parsedRequest.optionValue("NAMESPACE"),
                fHandle, info.requestNumber);

            if (res.rc != STAFResult.Ok) return res;
        
            String namespace = res.result;

            boolean only = false;

            if (parsedRequest.optionTimes("ONLY") > 0)
                only = true;

            mc.setMapClassDefinition(fListVariablesMapClass);
            
            // Synchronized on the Namespace service so that only one request
            // to the Namespace service can run at a time (to ensure that
            // nothing else can change the Namespace data while this code is
            // running).

            synchronized(this)
            {
                // Make sure the namespace exists

                Namespace ns = fNamespaceManager.get(namespace);

                if (ns == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Namespace '" + namespace + "' does not exist");
                }

                String name = ns.getName();
                Map variableMap = ns.getVariables();
                Iterator iter = variableMap.values().iterator();

                while (iter.hasNext())
                {
                    // Add an entry for each variable to the result list

                    Variable variable = (Variable)iter.next();

                    String key = variable.getKey();
                    String value = variable.getValue();

                    Map resultMap = fListVariablesMapClass.createInstance();

                    resultMap.put("key", key);
                    resultMap.put("value", value);

                    if (!name.equalsIgnoreCase(Namespace.sNONE))
                        resultMap.put("namespace", name);

                    resultList.add(resultMap);
                }

                if (!only)
                {
                    // List variables from the parent namespaces

                    String parent = ns.getParent();

                    while (!parent.equalsIgnoreCase(Namespace.sNONE))
                    {
                        Namespace parentNS = fNamespaceManager.get(parent);

                        variableMap = parentNS.getVariables();
                        iter = variableMap.values().iterator();

                        while (iter.hasNext())
                        {
                            // Add an entry for each variable to result list

                            Variable variable = (Variable)iter.next();

                            String key = variable.getKey();
                            
                            // Don't list any variables from parent namespaces
                            // that were overridden in the specified namespace

                            if (ns.hasVariable(key))
                            {
                                continue;
                            }
                            
                            String value = variable.getValue();

                            Map resultMap = fListVariablesMapClass.
                                createInstance();
                            
                            resultMap.put("key", key);
                            resultMap.put("value", value);

                            if (!parent.equalsIgnoreCase(Namespace.sNONE))
                                resultMap.put("namespace", parent);

                            resultList.add(resultMap);
                        }

                        parent = parentNS.getParent();
                    }
                }
            }
        }
        else
        {
            // LIST [NAMESPACES]

            mc.setMapClassDefinition(fListNamespacesMapClass);

            // Synchronized on the Namespace service so that only one request
            // to the Namespace service can run at a time (to ensure that
            // nothing else can change the Namespace data while this code is
            // running).

            synchronized(this)
            {
                // Add an entry for each namespace to the result list

                Iterator iter = fNamespaceManager.getNamespaceMapCopy().
                    values().iterator();

                while (iter.hasNext())
                {
                    Namespace ns = (Namespace)iter.next();

                    Map resultMap = fListNamespacesMapClass.createInstance();

                    resultMap.put("name", ns.getName());
                    resultMap.put("description", ns.getDescription());

                    String parent = ns.getParent();

                    if (!parent.equalsIgnoreCase(Namespace.sNONE))
                    {
                        resultMap.put("parent", ns.getParent());
                    }

                    resultList.add(resultMap);
                }
            }
        }

        // Set the result list as the root object for the marshalling context
        // and return the marshalled result

        mc.setRootObject(resultList);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    /**
     * Handle a QUERY request
     */ 
    private STAFResult handleQuery(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "QUERY", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fQueryParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }
        
        // Resolve any STAF variables in the NAMESPACE option's value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("NAMESPACE"),
            fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String namespace = res.result;

        STAFMarshallingContext mc = new STAFMarshallingContext();

        boolean tree = false;

        if (parsedRequest.optionTimes("TREE") > 0)
            tree = true;

        if (!tree)
        {
            // QUERY NAMESPACE <Namespace>

            // Create a marshalling context and set any map classes (if any).

            mc.setMapClassDefinition(fQueryfNamespaceMapClass);

            // Create an empty result map to contain the result
        
            Map resultMap = fQueryfNamespaceMapClass.createInstance();

            // Synchronized on the Namespace service so that only one request
            // to the Namespace service can run at a time (to ensure that
            // nothing else can change the Namespace data while this code is
            // running).

            synchronized(this)
            {
                // Find the specified namespace and add its info to the
                // result map

                Namespace ns = fNamespaceManager.get(namespace);

                if (ns == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Namespace '" + namespace + "' does not exist");
                }
                
                resultMap.put("name", ns.getName());
                resultMap.put("description", ns.getDescription());

                String parent = ns.getParent();

                if (!parent.equalsIgnoreCase(Namespace.sNONE))
                {
                    resultMap.put("parent", ns.getParent());
                }

                resultMap.put("children",
                              new ArrayList(ns.getChildren().keySet()));
            }

            mc.setRootObject(resultMap);
        }
        else
        {
            // QUERY NAMESPACE <Namespace> TREE

            // Create a marshalling context and set any map classes (if any).

            mc.setMapClassDefinition(fQueryTreeMapClass);

            // Create an empty result map to contain the result
        
            Map resultMap = fQueryTreeMapClass.createInstance();

            // Synchronized on the Namespace service so that only one request
            // to the Namespace service can run at a time (to ensure that
            // nothing else can change the Namespace data while this code is
            // running).

            synchronized(this)
            {
                // Find the specified namespace and add its info to the
                // result map

                Namespace ns = fNamespaceManager.get(namespace);

                if (ns == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Namespace '" + namespace + "' does not exist");
                }
                
                resultMap.put("name", ns.getName());

                // Iterate thru the children
                
                List resultList = visitChildren(ns.getChildren());
                resultMap.put("children", resultList);
            }

            mc.setRootObject(resultMap);
        }

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }
    
    /**
     * Helper method for a QUERY TREE request to build up the output that
     * shows all of the child namespaces in the hierarchy
     */ 
    private List visitChildren(Map children)
    {
        List resultList = new ArrayList();
        Iterator iter = children.values().iterator();

        while (iter.hasNext())
        {
            Namespace childNS = (Namespace)iter.next();

            Map childResultMap = fQueryTreeMapClass.createInstance();
            childResultMap.put("name", childNS.getName());
            childResultMap.put("children",
                               visitChildren(childNS.getChildren()));
            resultList.add(childResultMap);
        }

        return resultList;
    }

    /**
     * Handle a SET request
     */ 
    private STAFResult handleSet(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, fServiceName, "SET", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fSetParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Resolve any STAF variables in the NAMESPACE option's value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("NAMESPACE"),
            fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String namespace = res.result;

        // Don't resolve STAF variables in the VAR option values.
        // Make sure the variables have format Key=Value

        int numVars = parsedRequest.optionTimes("VAR");
        HashMap vars = new HashMap();

        for (int i = 1; i <= numVars; ++i)
        {
            String varString = parsedRequest.optionValue("VAR", i);

            int equalPos = varString.indexOf("=");

            if (equalPos == -1)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Variable format must be Key=Value.  VAR: " + varString);
            }

            String key = varString.substring(0, equalPos);
            String value = varString.substring(equalPos + 1);

            vars.put(key, value);
        }

        // Synchronized on the Namespace service so that only one request
        // to the Namespace service can run at a time (to ensure that nothing
        // else can change the Namespace data while this code is running).

        synchronized(this)
        {
            // Make sure the namespace exists
            
            Namespace ns = fNamespaceManager.get(namespace);

            if (ns == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "Namespace '" + namespace + "' does not exist");
            }
            
            // Set the variable(s) in the namespace: 
            // - If the variable key already exists, update its value.
            // - If the variable key does not exist, add it.

            ns.addVariables(vars);
            
            // Save namespaces to persistent data storage

            res = storeData(info);

            if (res.rc != STAFResult.Ok) return res;
        }
        
        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Handle a GET request
     */ 
    private STAFResult handleGet(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "GET", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fGetParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Resolve any STAF variables in the NAMESPACE option's value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("NAMESPACE"),
            fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String namespace = res.result;

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("VAR"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;

        String varKey = res.result;
        String varValue = "";

        // Synchronized on the Namespace service so that only one request
        // to the Namespace service can run at a time (to ensure that nothing
        // else can change the Namespace data while this code is running).

        synchronized(this)
        {
            // Make sure the namespace exists

            Namespace ns = fNamespaceManager.get(namespace);

            if (ns == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "Namespace '" + namespace + "' does not exist");
            }

            // Check if the variable exists in the specified namespace.
            // If so, return it's value.

            if (ns.hasVariable(varKey))
            {
                varValue = ns.getVariable(varKey);
            }
            else
            {
                // If not found in the specified namespace, check the parent
                // namespace, and so on, up the hierarchy, until the variable
                // key is found or until a namespace has no parent.
                // Return the variable's value in the result buffer for the
                // first instance of the variable found or return an error if
                // not found.

                String parent = ns.getParent();

                while (!parent.equalsIgnoreCase(Namespace.sNONE))
                {
                    ns = fNamespaceManager.get(parent);

                    if (ns.hasVariable(varKey))
                    {
                        varValue = ns.getVariable(varKey);
                        return new STAFResult(STAFResult.Ok, varValue);
                    }
                    else
                    {
                        parent = ns.getParent();
                    }
                }
                
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "Variable with key '" + varKey + "' does not exist");
            }
        }
        
        return new STAFResult(STAFResult.Ok, varValue);
    }

    /**
     * Handle a DELETE request
     */ 
    private STAFResult handleDelete(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Parse the request

        STAFCommandParseResult parsedRequest = fDeleteParser.parse(
            info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        int numVars = parsedRequest.optionTimes("VAR");

        // Check trust level

        STAFResult trustResult;

        if (numVars > 0)
        {
            // Delete var from a namespace request requires trust level 3
            trustResult = STAFUtil.validateTrust(
                3, fServiceName, "DELETE", fLocalMachineName, info);
        }
        else
        {
            // Delete namespace request requires trust level 4

            trustResult = STAFUtil.validateTrust(
                4, fServiceName, "DELETE", fLocalMachineName, info);
        }

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Resolve any STAF variables in the NAMESPACE option's value

        STAFResult res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("NAMESPACE"),
            fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String namespace = res.result;

        if (numVars == 0)
        {
            // DELETE NAMESPACE

            if (parsedRequest.optionTimes("CONFIRM") == 0)
            {
                return new STAFResult(
                    STAFResult.InvalidRequestString,
                    "When specifying option DELETE without any VAR options," +
                    " you must also specify option CONFIRM to delete the " +
                    "namespace");
            }

            // Synchronized on the Namespace service so that only one request
            // to the Namespace service can run at a time (to ensure that
            // nothing else can change the Namespace data while this code is
            // running).

            synchronized(this)
            {
                // Check if the namespace exists

                Namespace ns = fNamespaceManager.get(namespace);

                if (ns == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Namespace '" + namespace + "' does not exist");
                }
                
                // If the namespace has any child namespaces, their parents
                // will be changed to be the parent of the deleted namespace
                // or null if it has no parent

                namespace = ns.getName();
                String parent = ns.getParent();
                Namespace parentNS = null;

                if (!parent.equalsIgnoreCase(Namespace.sNONE))
                {
                    parentNS = fNamespaceManager.get(parent);
                }
                
                Map children = ns.getChildren();
                Iterator iter = children.keySet().iterator();

                while (iter.hasNext())
                {
                    String childNSName = (String)iter.next();
                    Namespace childNS = (Namespace)children.get(childNSName);

                    // Set new parent

                    childNS.setParent(parent);

                    // Add this namespace's children to the new parent's
                    // children

                    if (parentNS != null)
                    {
                        parentNS.addChild(childNSName, childNS);
                    }
                }

                if (parentNS != null)
                {
                    // Delete this namespace from its parent's children list
                
                    parentNS.removeChild(namespace);
                }
                
                // Save a copy of the Namespaces file in a temporary directory
                // before deleting the namespace
                
                copyData("delete");

                // Delete the namespace

                fNamespaceManager.delete(namespace);

                // Save namespaces to persistent data storage

                res = storeData(info);

                if (res.rc != STAFResult.Ok) return res;
            }
        }
        else
        {
            // DELETE Variable(s) in a NAMESPACE
            
            List varKeyList = new ArrayList();

            for (int i = 1; i <= numVars; ++i)
            {
                res = STAFUtil.resolveRequestVar(
                    parsedRequest.optionValue("VAR", i),
                    fHandle, info.requestNumber);

                if (res.rc != STAFResult.Ok) return res;
        
                varKeyList.add(res.result);
            }

            // Synchronized on the Namespace service so that only one request
            // to the Namespace service can run at a time (to ensure that
            // nothing else can change the Namespace data while this code is
            // running).

            synchronized(this)
            {
                // Remove variables from the namespace

                Namespace ns = fNamespaceManager.get(namespace);

                if (ns == null)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Namespace '" + namespace + "' does not exist");
                }
                
                Iterator iter = varKeyList.iterator();

                while (iter.hasNext())
                {
                    String key = (String)iter.next();

                    if (!ns.hasVariable(key))
                    {
                        return new STAFResult(
                            STAFResult.DoesNotExist,
                            "Variable with key '" + key + "' does not exist");
                    }
                    
                    // Delete the variable from the namespace

                    ns.removeVariable(key);
                }
                
                // Save namespaces to persistent data storage

                res = storeData(info);

                if (res.rc != STAFResult.Ok) return res;
            }
        }
        
        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Parse the PARMS value specified when registering the Namespace service
     */

    private STAFResult handleParms(STAFServiceInterfaceLevel30.InitInfo info)
    {
        // Parse PARMS if provided when registering this service

        fParmsParser = new STAFCommandParser();

        fParmsParser.addOption("DIRECTORY", 1,
                               STAFCommandParser.VALUEREQUIRED);
        fParmsParser.addOption("FILENAME", 1,
                               STAFCommandParser.VALUEREQUIRED);

        STAFCommandParseResult parseResult= fParmsParser.parse(info.parms);

        String errorPrefix = "ERROR:  Service Configuration Error for " +
            "Service " + fServiceName + fLineSep +
            "NamespaceService::handleParms() - ";

        if (parseResult.rc != STAFResult.Ok)
        {
            System.out.println(
                errorPrefix + "PARMS parsing failed with RC=" +
                STAFResult.InvalidRequestString +
                " Result=" + parseResult.errorBuffer);

            return new STAFResult(
                STAFResult.InvalidRequestString, parseResult.errorBuffer);
        }

        if (parseResult.optionTimes("DIRECTORY") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("DIRECTORY"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errorPrefix + "Error resolving DIRECTORY.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            if (resolvedResult.result.length() == 0)
            {
                String errorMsg = "The PARMS DIRECTORY option value cannot " +
                    "be blank"; 
                System.out.println(errorPrefix + errorMsg);

                return new STAFResult(STAFResult.InvalidValue, errorMsg);
            }

            fDataDir = resolvedResult.result;
        }

        if (parseResult.optionTimes("FILENAME") > 0)
        {
            STAFResult resolvedResult = STAFUtil.resolveInitVar(
                parseResult.optionValue("FILENAME"), fHandle);

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println(
                    errorPrefix + "Error resolving FILENAME.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }
            
            if (resolvedResult.result.length() == 0)
            {
                String errorMsg = "The PARMS FILENAME option value cannot " +
                    "be blank"; 
                System.out.println(errorPrefix + errorMsg);

                return new STAFResult(STAFResult.InvalidValue, errorMsg);
            }

            fShortFileName = resolvedResult.result;
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * This is the STAF Service termination method that is run when the
     * service is unregistered.  It performs termination functions such as:
     * <ul>
     * <li>Unregisters any serivce help messages
     * <li>Unregisters the STAF handle it used to submit requests to STAF
     * services
     * * </ul>
     * @return An instance of STAFResult which contains the return code and
     * result buffer indicating if the service terminated successfully.
     */ 
    public STAFResult term()
    {
        logMessage("Stop", "Terminating the " + fServiceName + " service",
                   false, null);
        try
        {
            // Save a copy of the Namespaces file in a temporary directory

            copyData("term");

            // Un-register Help Data

            unregisterHelpData(kDataStorageError);

            // Un-register the service handle

            fHandle.unRegister();
        }
        catch (STAFException ex)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  ex.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Rename a file
     */
    private boolean renameFile(File oldFile, File newFile)
    {
        newFile.delete();

        return oldFile.renameTo(newFile);
    }

    /**
     * Save the namespaces data currently in memory to persistent storage.
     * If an error occurs writing to persistent storage, restore the
     * namespaces data to it's previous state to keep the namespaces data
     * in memory in sync with the data in persistent storage.
     */ 
    private STAFResult storeData(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Backup the Namespaces.xml file

        boolean haveBackup = renameFile(fDataFile, fBackupFile);

        // Write the namespaces data to persistent storage

        try
        {
            fStorageManager.saveNamespaces();
        }
        catch (NSException e)
        {
            // Writing the namespaces data to persistent storage failed.
            // Use the Namespaces backup file to restore namespaces data in
            // memory to keep them in sync (and to back out the change just
            // made).

            // Log to service log and print stack trace to JVM log

            String errorMsg = "Saving namespaces data to persistent " +
                "storage failed. " + fLineSep + e + fLineSep +
                "See the service log for more information and contact the " +
                "system administrator for the service machine.";

            logMessage("Error", errorMsg, true, info);

            e.printStackTrace();

            if (haveBackup)
            {
                // Restore to the backup Namespaces xml file

                if (renameFile(fBackupFile, fDataFile))
                {
                    try
                    {
                        // Reload namespace data from backup file

                        fStorageManager.loadNamespaces();

                        logMessage(
                            "Info", "Successfully reloaded namespaces data " +
                            "from backup file.", false, null);
                    }
                    catch (NSException nse)
                    {
                        logMessage(
                            "Error", "Namespaces data in memory is out of " +
                            "sync with data stored in persistent storage " +
                            "because loadNamespaces() failed to restore " +
                            "Namespaces data from backup file. " + fLineSep +
                            nse, false, null);
                    }
                }
                else
                {
                    logMessage(
                        "Error", "Namespaces data in memory is out of sync " +
                        "with data stored in persistent storage because " +
                        " renameFile(" + fBackupFile + ", " + fDataFile +
                        ") failed.", false, null);
                }
            }
            else
            {
                logMessage("Error", "Namespaces data in memory is out of " +
                           "sync with data stored in persistent storage " +
                           "because a backup file is not available.",
                           false, null);
            }

            return new STAFResult(kDataStorageError, errorMsg);
        }

        logMessage("Info", "", true, info);

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Copy the current Namespaces.xml file to a temporary file that is
     * available for backup purposes
     */ 
    private void copyData(String phase)
    {
        // Create a unique file name to copy the Namespace file to using an
        // extension containing the current time in milli-seconds.  The
        // format for this file name (stored in a temporary directory) is:
        //
        // <Short xml file name>.<phase>.<current time in milli-seconds>

        long currentTime = new Date().getTime();  // Get current time
        String toFile = fTempDir + fFileSep +
            fStorageManager.getShortFileName() + "." + phase + "." +
            currentTime;

        // Copy the file

        String copyRequest = "COPY FILE " +
            STAFUtil.wrapData(fStorageManager.getFileName()) +
            " TOFILE " + STAFUtil.wrapData(toFile) +
            " TOMACHINE local";

        STAFResult res = fHandle.submit2("local", "FS", copyRequest);

        if (res.rc != STAFResult.Ok)
        {
            logMessage(
                "Error", "Error copying Namespaces data file.  RC=" +
                res.rc + ", Result=" + res.result +
                ", Request=" + copyRequest, false, null);
        }
    }

    /**
     * Log a message to the global service log
     */ 
    private void logMessage(String level, String msg, boolean logRequestInfo,
                            STAFServiceInterfaceLevel30.RequestInfo info)
    {
        String message = msg;

        if (logRequestInfo)
        {
            message = "[" + info.endpoint + " " + info.handleName + " " +
                info.handle + "] " + info.request;

            if (!msg.equals(""))
            {
                message = message + " - " + msg;
            }
        }

        fHandle.submit2(
            "local", "LOG", "LOG GLOBAL LOGNAME " + fServiceName +
            " LEVEL " + level + " MESSAGE " + STAFUtil.wrapData(message));
    }

    /**
     *  Register error codes for the Namespace Service with the HELP service
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
     *  Un-register error codes for the Namespace Service with the HELP service
     */ 
    private void unregisterHelpData(int errorNumber)
    {
        STAFResult res = fHandle.submit2(
            "local", "HELP", "UNREGISTER SERVICE " + fServiceName +
            " ERROR " + errorNumber);
    }
}
