package com.ibm.staf.service.fsext;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Author: Chris Alkov                                                       */
/* Date: 12/2001                                                             */
/* Revisions:                                                                */
/*   01/31/2005 - Updated for STAF 3.0 marshalled results                    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: FSEXT                                                              */
/* Description: This class provides the STAFService Interface and implements */
/*              the majority of the service function                         */
/*                                                                           */
/*****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import com.ibm.staf.service.utility.ServiceUtilities;
import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.FileNotFoundException;
import java.text.SimpleDateFormat;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Calendar;
import java.util.Date;
import java.util.Enumeration;
import java.util.Vector;

public class FSExt implements STAFServiceInterfaceLevel30
{
    private STAFHandle sHandle;
    private String fServiceName;
    private String fLocalMachineName = "";
    private static String sHelpMsg;
    private static String sLineSep;

    /* STAF Parsers */

    private STAFCommandParser procFileParser;
    private STAFCommandParser compDirParser;
    private STAFCommandParser fileContainsParser;
    private STAFCommandParser lineContainsParser;
    private STAFCommandParser waitForFileParser;
    private STAFCommandParser substituteParser;

    private STAFMapClassDefinition fFileContainsErrorMapClass;
    private STAFMapClassDefinition fLineContainsMapClass;

    /* String Constants */

    private static final String FSEXTVERSION = "3.0.2";

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");
    
    private static final String PROCESSFILE = "PROCESSFILE";
    private static final String MODE    = "MODE";
    private static final String CAPTURE = "CAPTURE";
    private static final String COMPARE = "COMPARE";
    private static final String FILE1   = "FILE1";
    private static final String FILE2   = "FILE2";
    private static final String SORT    = "SORT";

    private static final String COMPAREDIR = "COMPAREDIR";
    private static final String DIR     = "DIR";
    private static final String EXISTS  = "EXISTS";
    private static final String ATTEMPTS = "ATTEMPTS";
    private static final String POLL    = "INTERVAL";

    private static final String FILECONTAINS = "FILECONTAINS";
    private static final String LINECONTAINS = "LINECONTAINS";
    private static final String FILE    = "FILE";
    private static final String STRING = "STRING";
    private static final String NOT     = "NOT";
    private static final String IGNORECASE = "IGNORECASE";
    private static final String SAVEONFAILURE = "SAVEONFAILURE";
    private static final String LINENUMBER = "LINENUMBER";

    private static final String WAITFORFILE = "WAITFORFILE";
    private static final String TIMEOUT = "TIMEOUT";

    private static final String HELP = "HELP";
    private static final String VERSION = "VERSION";

    private static final String SUBSTITUTE = "SUBSTITUTE";
    private static final String TOFILE = "TOFILE";
    private static final String TOMACHINE = "TOMACHINE";
    private static final String FAILIFEXISTS = "FAILIFEXISTS";
    private static final String FAILIFNEW = "FAILIFNEW";

    /* Return Codes */

    private static final int COMPAREFAIL = 4001;
    private static final String COMPAREFAILInfo = "File Comparison Failed";
    private static final String COMPAREFAILDesc =
        "The file comparison failed.";

    private static final int NOTDIRECTORY = 4002;
    private static final String NOTDIRECTORYInfo = "Not A Directory";
    private static final String NOTDIRECTORYDesc =
        "The value specified for the DIR parameter is not a directory.";

    private static final int FILENOTFOUND = 4003;
    private static final String FILENOTFOUNDInfo = "File Not Found";
    private static final String FILENOTFOUNDDesc =
        "The specified file was not found on the file system.";

    private static final int FILEEXISTS = 4004;
    private static final String FILEEXISTSInfo = "File Exists";
    private static final String FILEEXISTSDesc =
        "The specified file exists. (The NOT parameter was specified.)";

    private static final int EXTRAFILES = 4005;
    private static final String EXTRAFILESInfo = "Extraneous Files Found";
    private static final String EXTRAFILESDesc =
        "Extraneous files were found in the specified directory.";

    private static final int STRINGNOTFOUND = 4006;
    private static final String STRINGNOTFOUNDInfo = "String(s) Not Found";
    private static final String STRINGNOTFOUNDDesc =
        "The specified String(s) were not found.";

    private static final int STRINGFOUND = 4007;
    private static final String STRINGFOUNDInfo = "String(s) Found";
    private static final String STRINGFOUNDDesc =
        "The specified String(s) were found. " +
        "(The NOT parameter was specified.)";

    private static final int LINEDOESNOTEXIST = 4008;
    private static final String LINEDOESNOTEXISTInfo =
        "Line Number Does Not Exist";
    private static final String LINEDOESNOTEXISTDesc =
        "The line number specified by the LINENUMBER parameter " +
        "does not exist in the file.";

    private static final int IOERROR = 4100;
    private static final String IOERRORInfo = "Java IOError";
    private static final String IOERRORDesc =
        "A Java IOError occurred during command execution. Check the result " +
        "buffer for more information.";

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/*                                                                           */
/*****************************************************************************/

public FSExt()
{
    super();
}

/*****************************************************************************/
/*                                                                           */
/* Method: acceptRequest                                                     */
/* Description: required by interface STAFServiceInterfaceLevel30             */
/*              performs first parse of request                              */
/*                                                                           */
/*****************************************************************************/

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

        String actionUC = action.toUpperCase();

        // Call the appropriate method to handle the command request

        if (actionUC.equals(PROCESSFILE))
            return processFile(info);
        else if (actionUC.equals(COMPAREDIR))
            return compareDir(info);
        else if (actionUC.equals(FILECONTAINS))
            return fileContains(info);
        else if (actionUC.equals(LINECONTAINS))
            return lineContains(info);
        else if (actionUC.equals(WAITFORFILE))
            return waitForFile(info);
        else if (actionUC.equals(SUBSTITUTE))
            return substitute(info);
        else if (actionUC.equals(HELP))
            return handleHelp(info);
        else if (actionUC.equals(VERSION))
            return handleVersion(info);
        else
        {
            return new STAFResult(
                STAFResult.InvalidRequestString,
                "'" + action + "' is not a valid command request for the " +
                fServiceName + " service" + sLineSep + sLineSep + sHelpMsg);
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

        StringWriter sr = new StringWriter();
        t.printStackTrace(new PrintWriter(sr));

        if (t.getMessage() != null)
        {
            return new STAFResult(
                STAFResult.JavaError,
                t.getMessage() + sLineSep + sr.toString());
        }
        else
        {
            return new STAFResult(
                STAFResult.JavaError, sr.toString());
        }
    }
}

/*****************************************************************************/
/*                                                                           */
/* Method: compareDir                                                        */
/* Description: performs compareDir service function                         */
/* Parameters: info - RequestInfo passed to acceptRequest                    */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/

private STAFResult compareDir(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "COMPAREDIR", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse request string

    STAFCommandParseResult pResult = compDirParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Determine which options which specified in the request

    File dir;
    String[] expectedFiles;

    // Parse DIR & FILES options

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(DIR), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String dirName = res.result;
    dir = new File(dirName);

    if (!dir.isDirectory())
    {
        // Value for DIR option specified is not a directory
        return new STAFResult(NOTDIRECTORY, dirName);
    }

    // Create a String array of fully qualified filenames from list of
    // relative filenames passed in via the FILE parm.

    int numFiles = pResult.optionTimes(FILE);
    expectedFiles = new String[numFiles];

    for (int i = 0; i < numFiles; i++)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(FILE, i+1), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String shortName = res.result;
        File file = new File(dir, shortName);
        expectedFiles[i] = file.getPath();
    }

    // Parse EXISTS parm

    boolean exact = true;

    if (pResult.optionTimes(EXISTS) == 1)
    {
        exact = false;
    }

    // Parse RETRY parm

    int attempts = 1;
    int poll = 15000;

    if (pResult.optionTimes(ATTEMPTS) == 1)
    {
        res = STAFUtil.resolveRequestVarAndCheckInt(
            ATTEMPTS, pResult.optionValue(ATTEMPTS),
            sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        attempts = Integer.parseInt(res.result);
    }

    if (pResult.optionTimes(POLL) == 1)
    {
        res = STAFUtil.resolveRequestVarAndCheckInt(
            POLL, pResult.optionValue(POLL), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        poll = Integer.parseInt(res.result);
    }

    // Start a large "for" loop to handle retries

    for (int loop = 1; loop <= attempts; loop++)
    {
        boolean success = true;

        // Create a HashMap of all files in the specified directory.  The
        // filenames shall be the keys for the Map and the values will all
        // initially be set to false.  As a filename in the FILES list is
        // matched with a file in the file system, the value will be set to
        // true.

        HashMap files = ServiceUtilities.createMapOfFiles(dir);

        // Loop through the list of expected files and verify that all of them
        // exist in the specified directory.

        for (int i = 0; i < expectedFiles.length; i++)
        {
            if (files.containsKey(expectedFiles[i]))
            {
                // File exists - set value to true
                files.put(expectedFiles[i], new Boolean(true));
            }
            else if (loop == attempts)
            {
                // File does not exist - Last retry so return failure
                return new STAFResult(FILENOTFOUND, expectedFiles[i]);
            }
            else
            {
                // File does not exist - Set success to false for this attempt
                success = false;
            }
        }

        // If all the expected files were found and EXISTS was not specified,
        // check that no extraneous files exist.

        if (exact && success)
        {
            if (files.containsValue(new Boolean(false)))
            {
                // At least one extra file found

                if (loop == attempts)
                {
                    // Last retry so return failure and list of extra files

                    List fileList = new ArrayList();
                    Iterator keys = files.keySet().iterator();

                    while(keys.hasNext())
                    {
                        // Loop thru map and return all files with false value

                        String key = (String)keys.next();

                        boolean failed = !((Boolean)files.get(key)).
                            booleanValue();

                        if (failed) fileList.add(key);
                    }

                    // Create a marshalled list of the exta files

                    STAFMarshallingContext mc = new STAFMarshallingContext();
                    mc.setRootObject(fileList);

                    return new STAFResult(EXTRAFILES, mc.marshall());
                }
                else
                {
                    // Extra files found. Set success to false for this attempt
                    success = false;
                }
            } // end if file.containsValue
        } // end if (exact && success)

        // If all checks are successful break out of big loop and return OK.
        // Otherwise, delay and start check again at top of loop.

        if (success)
            break;
        else
            sHandle.submit2("local", "DELAY", "DELAY " + poll);
        
    } // end big for loop

    return new STAFResult(STAFResult.Ok);
}

/*****************************************************************************/
/*                                                                           */
/* Method: fileContains                                                      */
/* Description: performs fileContains service function                       */
/* Parameters: info - RequestInfo passed to acceptRequest                    */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/

private STAFResult fileContains(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "FILECONTAINS", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse request String

    STAFCommandParseResult pResult = fileContainsParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get FILE and STRING options

    File file;
    Enumeration strings;

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(FILE), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String filename = res.result;
    file = new File(filename);

    // Get all instances of STRING, store in a Vector and get the Enumeration

    int numStrings = pResult.optionTimes(STRING);
    Vector stringVect = new Vector();

    for (int i = 1; i <= numStrings; i++)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(STRING, i), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String tempString = res.result;
        stringVect.add(tempString);
    }

    strings = stringVect.elements();

    // Check if NOT option specified

    boolean fileContainsString = true;

    if (pResult.optionTimes(NOT) == 1)
        fileContainsString = false;
    
    // Check if IGNORECASE option specified

    boolean ignoreCase = false;

    if (pResult.optionTimes(IGNORECASE) == 1)
        ignoreCase = true;
    
    // Check if SAVEONFAILURE option specified

    boolean saveOnFail = false;

    if (pResult.optionTimes(SAVEONFAILURE) == 1)
        saveOnFail = true;
    
    // Read entire file and store as a String

    FileReader in = null;
    String data = null;

    try
    {
        in = new FileReader(file);
        int numBytes = (int) file.length();
        char[] fileData = new char[numBytes];
        in.read(fileData);
        data = new String(fileData);

        if (ignoreCase) data = data.toLowerCase();
    }
    catch(IOException ioe)
    {
        return new STAFResult(IOERROR, ioe.toString());
    }
    finally
    {
        try
        {
            in.close();
        }
        catch(Throwable t) {} //don't care
    }

    // Scan entire file for presence or absence of the specified Strings

    if (fileContainsString)
    {
        // Check that file contains list of Strings

        while(strings.hasMoreElements())
        {
            String tempS = (String) strings.nextElement();

            if (ignoreCase)
            {
                // IGNORECASE specified
                tempS = tempS.toLowerCase();
            }

            if (data.indexOf(tempS) == -1)
            {
                // String not found

                if (saveOnFail)
                {
                    // Capture the file for later comparison
                    
                    ExtFile capture = new ExtFile(file);
                    try
                    {
                        capture.captureFile(file.getPath() + ".fail");
                    }
                    catch(IOException ioe)
                    {
                        return new STAFResult(IOERROR, ioe.toString());
                    }
                }

                return new STAFResult(STRINGNOTFOUND, tempS);
            }
        }
    }
    else
    {
        // Check that the file does NOT contain the list of Strings

        while (strings.hasMoreElements())
        {
            String tempS = (String)strings.nextElement();

            if (ignoreCase) tempS = tempS.toLowerCase();
            
            int index = data.indexOf(tempS);

            if (index != -1)
            {
                // String was found in the file

                if (saveOnFail)
                {
                    // Capture the file for later comparison

                    ExtFile capture = new ExtFile(file);

                    try
                    {
                        capture.captureFile(file.getPath() + ".fail");
                    }
                    catch(IOException ioe)
                    {
                        return new STAFResult(IOERROR, ioe.toString());
                    }
                }

                Map resultMap = fFileContainsErrorMapClass.createInstance();
                resultMap.put("offset", String.valueOf(index));
                resultMap.put("string", tempS);

                STAFMarshallingContext mc = new STAFMarshallingContext();
                mc.setRootObject(resultMap);
                mc.setMapClassDefinition(fFileContainsErrorMapClass);

                return new STAFResult(STRINGFOUND, mc.marshall());
            }
        }
    }

    return new STAFResult(STAFResult.Ok);
}

/*****************************************************************************/
/*                                                                           */
/* Method: help                                                              */
/* Description: returns service help information                             */
/* Parameters: none                                                          */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleHelp(
    STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 1

    STAFResult trustResult = STAFUtil.validateTrust(
        1, fServiceName, "HELP", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Return help text for the service

    return new STAFResult(STAFResult.Ok, sHelpMsg);
}

/*****************************************************************************/
/*                                                                           */
/* Method: init                                                              */
/* Description: required by interface STAFServiceInterfaceLevel30            */
/*              creates parsers and registers with STAF                      */
/*                                                                           */
/*****************************************************************************/

public STAFResult init(STAFServiceInterfaceLevel30.InitInfo initInfo)
{
    /* Generate STAF Parsers */

    /* Generate PROCESSFILE parser */

    procFileParser = new STAFCommandParser();

    procFileParser.addOption(PROCESSFILE, 1,
        STAFCommandParser.VALUENOTALLOWED);
    procFileParser.addOption(MODE, 1, STAFCommandParser.VALUEREQUIRED);
    procFileParser.addOption(FILE1, 1, STAFCommandParser.VALUEREQUIRED);
    procFileParser.addOption(FILE2, 1, STAFCommandParser.VALUEREQUIRED);
    procFileParser.addOption(SORT, 1, STAFCommandParser.VALUENOTALLOWED);
    procFileParser.addOption(SAVEONFAILURE, 1,
        STAFCommandParser.VALUENOTALLOWED);

    procFileParser.addOptionGroup(FILE1, 1, 1);
    procFileParser.addOptionGroup(FILE2, 1, 1);

    /* Generete COMPAREDIR parser */

    compDirParser = new STAFCommandParser();
    compDirParser.addOption(COMPAREDIR, 1, STAFCommandParser.VALUENOTALLOWED);
    compDirParser.addOption(DIR, 1, STAFCommandParser.VALUEREQUIRED);
    compDirParser.addOption(FILE, 0, STAFCommandParser.VALUEREQUIRED);
    compDirParser.addOption(EXISTS, 1, STAFCommandParser.VALUENOTALLOWED);
    compDirParser.addOption(ATTEMPTS, 1, STAFCommandParser.VALUEREQUIRED);
    compDirParser.addOption(POLL, 1, STAFCommandParser.VALUEREQUIRED);

    compDirParser.addOptionGroup(DIR, 1, 1);
    compDirParser.addOptionGroup(FILE, 1, 1);

    compDirParser.addOptionNeed(POLL, ATTEMPTS);

    /* Generate FILECONTAINS parser */

    fileContainsParser = new STAFCommandParser();

    fileContainsParser.addOption(FILECONTAINS, 1,
        STAFCommandParser.VALUENOTALLOWED);
    fileContainsParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    fileContainsParser.addOption(STRING, 0, STAFCommandParser.VALUEREQUIRED);
    fileContainsParser.addOption(NOT, 1, STAFCommandParser.VALUENOTALLOWED);
    fileContainsParser.addOption(IGNORECASE, 1,
        STAFCommandParser.VALUENOTALLOWED);
    fileContainsParser.addOption(SAVEONFAILURE, 1,
        STAFCommandParser.VALUENOTALLOWED);

    fileContainsParser.addOptionGroup(FILE, 1, 1);
    fileContainsParser.addOptionGroup(STRING, 1, 1);

    /* Generate LINECONTAINS parser */

    lineContainsParser = new STAFCommandParser();

    lineContainsParser.addOption(LINECONTAINS, 1,
        STAFCommandParser.VALUENOTALLOWED);
    lineContainsParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    lineContainsParser.addOption(STRING, 0, STAFCommandParser.VALUEREQUIRED);
    lineContainsParser.addOption(IGNORECASE, 1,
        STAFCommandParser.VALUENOTALLOWED);
    lineContainsParser.addOption(SAVEONFAILURE, 1,
        STAFCommandParser.VALUENOTALLOWED);
    lineContainsParser.addOption(LINENUMBER, 1,
        STAFCommandParser.VALUEREQUIRED);

    lineContainsParser.addOptionGroup(FILE, 1, 1);
    lineContainsParser.addOptionGroup(STRING, 1, 1);

    /* Generate WAITFORFILE parser */

    waitForFileParser = new STAFCommandParser();

    waitForFileParser.addOption(WAITFORFILE, 1,
        STAFCommandParser.VALUENOTALLOWED);
    waitForFileParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    waitForFileParser.addOption(TIMEOUT, 1, STAFCommandParser.VALUEREQUIRED);
    waitForFileParser.addOption(POLL, 1, STAFCommandParser.VALUEREQUIRED);
    waitForFileParser.addOption(NOT, 1, STAFCommandParser.VALUENOTALLOWED);

    waitForFileParser.addOptionGroup(FILE, 1, 1);

    /* Generate SUBSTITUTE parser */

    substituteParser = new STAFCommandParser();

    substituteParser.addOption(SUBSTITUTE, 1,
                               STAFCommandParser.VALUENOTALLOWED);
    substituteParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    substituteParser.addOption(TOFILE, 1, STAFCommandParser.VALUEREQUIRED);
    substituteParser.addOption(TOMACHINE, 1, STAFCommandParser.VALUEREQUIRED);
    substituteParser.addOption(FAILIFEXISTS, 1,
                               STAFCommandParser.VALUENOTALLOWED);
    substituteParser.addOption(FAILIFNEW, 1,
                               STAFCommandParser.VALUENOTALLOWED);

    substituteParser.addOptionGroup(FILE, 1, 1);
    substituteParser.addOptionGroup(FAILIFEXISTS + " " + FAILIFNEW, 0, 1);

    // Construct map class for the result from an unsuccessful FILECONTAINS NOT
    // request

    fFileContainsErrorMapClass = new STAFMapClassDefinition(
        "STAF/Service/FSExt/FileContainsNotError");
    fFileContainsErrorMapClass.addKey("offset", "Byte Offset");
    fFileContainsErrorMapClass.addKey("string", "Found String");
    
    // Construct map class for the result from a LINECONTAINS request

    fLineContainsMapClass = new STAFMapClassDefinition(
        "STAF/Service/FSExt/LineContainsResult");
    fLineContainsMapClass.addKey("lineNumber", "Line Number");
    fLineContainsMapClass.addKey("line", "Line");

    /* Set name */

    fServiceName = initInfo.name;

    /* Create STAFHandle */

    try
    {
        sHandle = new STAFHandle(initInfo.name);
    }
    catch(STAFException se)
    {
        se.printStackTrace();
        return new STAFResult(STAFResult.STAFRegistrationError,
                              se.toString());
    }

    // Resolve the machine name variable for the local machine

    STAFResult res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    fLocalMachineName = res.result;
    
    // Resolve the line separator variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    sLineSep = res.result;

    // Assign the help text string for the service

    sHelpMsg = "*** " + fServiceName + " Service Help ***" +
        sLineSep + sLineSep +
        "COMPAREDIR DIR <Directory> FILE <File> [FILE <File>]... [EXISTS]" +
        sLineSep +
        "     [ATTEMPTS <Num Attempts> [INTERVAL <Polling Interval>]]" +
        sLineSep + sLineSep +
        "FILECONTAINS FILE <File> STRING <String> [STRING <String>]... [NOT]" +
        sLineSep +
        "     [IGNORECASE] [SAVEONFAILURE]" +
        sLineSep + sLineSep +
        "LINECONTAINS FILE <File> STRING <String> [STRING <String>]... [IGNORECASE]" +
        sLineSep +
        "     [SAVEONFAILURE] [LINENUMBER <Line #>]" +
        sLineSep + sLineSep +
        "PROCESSFILE [MODE <capture | compare>] FILE1 <File1> FILE2 <File2> [SORT]" +
        sLineSep +
        "     [SAVEONFAILURE]" +
        sLineSep + sLineSep +
        "SUBSTITUTE FILE <File> [TOFILE <File>] [TOMACHINE <Machine>]" +
        sLineSep +
        "     [FAILIFNEW | FAILIFEXISTS]" +
        sLineSep + sLineSep +
        "WAITFORFILE FILE <File> [TIMEOUT <Max Wait Time>]" +
        sLineSep +
        "     [INTERVAL <Polling Interval>] [NOT]" +
        sLineSep + sLineSep +
        "VERSION" +
        sLineSep + sLineSep +
        "HELP";

    /* Register RCs with the HELP service */

    int rc = this.registerHelp(initInfo.name);

    if (rc != STAFResult.Ok)
        return new STAFResult(
            rc, "Error registering RCs with the HELP service.");

    /* Finished */

    return new STAFResult(rc);

}

/*****************************************************************************/
/*                                                                           */
/* Method: lineContains                                                      */
/* Description: performs lineContains service function                       */
/* Parameters: info - RequestInfo passed to acceptRequest                    */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/

private STAFResult lineContains(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "LINECONTAINS", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse request string

    STAFCommandParseResult pResult = lineContainsParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get FILE and STRING options

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(FILE), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String filename = res.result;
    File file = new File(filename);

    // Get all instances of STRING and store in a Vector

    Vector stringList = new Vector();

    for (int i = 1; i <= pResult.optionTimes(STRING); i++)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(STRING, i), sHandle, info.requestNumber);

        if (res.rc != 0) return res;
           
        String tempString = res.result;
        stringList.add(tempString);
    }

    // Check if IGNORECASE option is specified

    boolean ignoreCase = false;

    if (pResult.optionTimes(IGNORECASE) == 1)
        ignoreCase = true;
    
    // Check if SAVEONFAILURE option is specified

    boolean saveOnFail = false;

    if (pResult.optionTimes(SAVEONFAILURE) == 1)
        saveOnFail = true;
    
    // Parse LINENUMBER option

    boolean strictLine = false;
    int specificLineNumber = 0;

    if (pResult.optionTimes(LINENUMBER) == 1)
    {
        strictLine = true;
            
        res = STAFUtil.resolveRequestVarAndCheckInt(
            LINENUMBER, pResult.optionValue(LINENUMBER),
            sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        specificLineNumber = Integer.parseInt(res.result);

        if (specificLineNumber == 0)
        {
            return new STAFResult(
                STAFResult.InvalidRequestString, "LINENUMBER cannot be 0.");
        }
    }

    // Read file one line at a time and search each line for string list.

    BufferedReader in = null;
    int numStrings = stringList.size();
    int lineNumber = 0;
    Vector lineStore = new Vector();

    try
    {
        in = new BufferedReader(new FileReader(file));

        while(true)
        {
            lineNumber++;

            // Read next line from file
            String line = in.readLine();

            if (line == null)
            {
                // Reached end of file
                break;
            }

            if (ignoreCase)
            {
                line = line.toLowerCase();
            }

            if (strictLine)
            {
                // Simply store line for later comparison
                lineStore.add(line);
                continue;
            }

            boolean searchResult = ServiceUtilities.lineContainsStrings(
                line, stringList, ignoreCase);

            if (searchResult)
            {
                // Found all strings in a single line

                Map resultMap = fLineContainsMapClass.createInstance();
                resultMap.put("lineNumber", String.valueOf(lineNumber));
                resultMap.put("line", line);

                STAFMarshallingContext mc = new STAFMarshallingContext();
                mc.setRootObject(resultMap);
                mc.setMapClassDefinition(fLineContainsMapClass);

                return new STAFResult(STAFResult.Ok, mc.marshall());
            }
        }
    }
    catch(IOException ioe)
    {
        return new STAFResult(IOERROR, ioe.toString());
    }
    finally
    {
        try
        {
            in.close();
        }
        catch(Throwable t) {} // Don't care
    }

    STAFResult result = new STAFResult(STRINGNOTFOUND); // Default from here

    if (strictLine)
    {
        // Now do comparison

        int totalLines = lineStore.size();

        if (Math.abs(specificLineNumber) > totalLines)
        {
            // Line not in file
            result = new STAFResult(LINEDOESNOTEXIST, 
                                    String.valueOf(totalLines));
        }

        STAFMarshallingContext mc = new STAFMarshallingContext();
        mc.setMapClassDefinition(fLineContainsMapClass);

        Map resultMap = fLineContainsMapClass.createInstance();

        if (specificLineNumber > 0)
        {
            String line = (String)lineStore.get(specificLineNumber - 1);

            resultMap.put("lineNumber",
                          String.valueOf(specificLineNumber));
            resultMap.put("line", line);
            mc.setRootObject(resultMap);

            if (ServiceUtilities.lineContainsStrings(
                line, stringList, ignoreCase))
            {
                return new STAFResult(STAFResult.Ok, mc.marshall());
            }
            else
            {
                result = new STAFResult(STRINGNOTFOUND, mc.marshall());
            }
        }
        else
        {
            int lineNum = totalLines + specificLineNumber;
            String line = (String)lineStore.get(lineNum);

            resultMap.put("lineNumber", String.valueOf(lineNum + 1));
            resultMap.put("line", line);
            mc.setRootObject(resultMap);

            if (ServiceUtilities.lineContainsStrings(
                line, stringList, ignoreCase))
            {
                return new STAFResult(STAFResult.Ok, mc.marshall());
            }
            else
            {
                result = new STAFResult(STRINGNOTFOUND, mc.marshall());
            }
        }
    }

    // If we get here query failed

    if (saveOnFail)
    {
        // Capture the file for later comparison

        ExtFile capture = new ExtFile(file);

        try
        {
            capture.captureFile(file.getPath() + ".fail");
        }
        catch(IOException ioe)
        {
            return new STAFResult(IOERROR, ioe.toString());
        }
    }

    return result;

}

/*****************************************************************************/
/*                                                                           */
/* Method: processFile                                                       */
/* Description: performs processFile service function                        */
/* Parameters: info - RequestInfo passed to acceptRequest                    */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/

private STAFResult processFile(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "PROCESSFILE", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    /* Parse request string */

    STAFCommandParseResult pResult = procFileParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    /* Get parameters */

    boolean compare = true; //if true compare, if false capture
    boolean sort = false;
    String mode, file1, file2;

    /* Get FILE names */

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(FILE1), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    file1 = res.result;

    res = STAFUtil.resolveRequestVar(
        pResult.optionValue(FILE2), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    file2 = res.result;

    /* Get MODE */

    if (pResult.optionTimes(MODE) == 1)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(MODE), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        mode = res.result;

        if (mode.equalsIgnoreCase(CAPTURE))
        {
            compare = false;
        }
        else if (mode.equalsIgnoreCase(COMPARE))
        {
            compare = true;
        }
        else
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                "Invalid value specified for MODE");
        }
    }

    /* Get SORT */

    if (pResult.optionTimes(SORT) == 1)
    {
        sort = true;
    }

    /* Parse SAVEONFAILURE parameter */

    boolean saveOnFail = false;

    if (pResult.optionTimes(SAVEONFAILURE) == 1)
    {
        saveOnFail = true;
    }

    /* Perform capture or compare */

    ExtFile sourceFile = new ExtFile(file1);
    sourceFile.setSaveFailures(saveOnFail);

    try
    {
        if (compare)
        {
            if (!sourceFile.compareFile(file2, sort))
            {
                return new STAFResult(COMPAREFAIL);
            }
        }
        else
        {
            sourceFile.captureFile(file2);
        }
    }
    catch(IOException ioe)
    {
        return new STAFResult(IOERROR, ioe.toString());
    }

    return new STAFResult(STAFResult.Ok);

}

/*****************************************************************************/
/*                                                                           */
/* Method: registerHelp                                                      */
/* Description: registers all FSEXT return codes with the HELP service       */
/* Parameters: serviceName - name which this service is registered as        */
/* Returns: int representation of STAFResult                                 */
/*                                                                           */
/*****************************************************************************/

private int registerHelp(String serviceName)
{
    try
    {
        String request = "REGISTER SERVICE "+serviceName+" ERROR "+COMPAREFAIL+
          " INFO "+COMPAREFAILInfo+" DESCRIPTION "+COMPAREFAILDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+NOTDIRECTORY+
          " INFO "+NOTDIRECTORYInfo+" DESCRIPTION "+NOTDIRECTORYDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+FILENOTFOUND+
          " INFO "+FILENOTFOUNDInfo+" DESCRIPTION "+FILENOTFOUNDDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+FILEEXISTS+
          " INFO "+FILEEXISTSInfo+" DESCRIPTION "+FILEEXISTSDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+EXTRAFILES+
          " INFO "+EXTRAFILESInfo+" DESCRIPTION "+EXTRAFILESDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+STRINGNOTFOUND+
          " INFO "+STRINGNOTFOUNDInfo+" DESCRIPTION "+STRINGNOTFOUNDDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+STRINGFOUND+
          " INFO "+STRINGFOUNDInfo+" DESCRIPTION "+STRINGFOUNDDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+LINEDOESNOTEXIST+
          " INFO "+LINEDOESNOTEXISTInfo+" DESCRIPTION "+LINEDOESNOTEXISTDesc;
        sHandle.submit("local", "HELP", request);

        request = "REGISTER SERVICE "+serviceName+" ERROR "+IOERROR+
          " INFO "+IOERRORInfo+" DESCRIPTION "+IOERRORDesc;
        sHandle.submit("local", "HELP", request);

    }
    catch(STAFException se)
    {
        return se.rc;
    }

    return STAFResult.Ok;

}

/*****************************************************************************/
/*                                                                           */
/* Method: substitute                                                        */
/* Description: performs substitute service function                         */
/* Parameters: info - RequestInfo passed to acceptRequest                    */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/

private STAFResult substitute(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 4

    STAFResult trustResult = STAFUtil.validateTrust(
        4, fServiceName, "SUBSTITUTE", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    /* Parse request string */

    STAFCommandParseResult pResult = substituteParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    /* Get source file */

    STAFResult res = STAFUtil.resolveRequestVar(
        pResult.optionValue(FILE), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String inputFile = res.result;

    /* Read file and resolve VARS then write out to tempFile */

    BufferedReader in = null;
    BufferedWriter out = null;
    File tempFile = null;

    try
    {
        tempFile = File.createTempFile("fsext", null);
        in = new BufferedReader(new FileReader(inputFile));
        out = new BufferedWriter(new FileWriter(tempFile));

        while(true)
        {
            //outside loop to parse file line by line

            String line = in.readLine();

            if (line == null)
            {
                //end of file

                break;
            }

            while(true)
            {
                //inside loop to resolve multiple vars per line
                int beginIndex = line.indexOf("%{");

                if (beginIndex == -1)
                {
                    //no vars to resolve

                    break;
                }

                int endIndex = line.indexOf("}%", beginIndex);
                String varToResolve = line.substring(beginIndex+1, endIndex+1);
                String resolvedVar;

                // Resolve using resolve request

                resolvedVar = sHandle.submit(
                    "local", "VAR", "RESOLVE REQUEST " +
                    String.valueOf(info.requestNumber) + " STRING " +
                    varToResolve);
                
                StringBuffer modString = new StringBuffer(line);
                modString = modString.replace(beginIndex, endIndex+2,
                    resolvedVar);
                line = modString.toString();
            }

            out.write(line);
            out.newLine();
        }
    }
    catch(STAFException se)
    {
        return new STAFResult(se.rc, se.getMessage());
    }
    catch(IOException ioe)
    {
        return new STAFResult(IOERROR, ioe.getMessage());
    }
    finally
    {
        try
        {
            out.close();
            in.close();
        }
        catch(Exception e) {} //don't care
    }

    // Resolve variables that will be required to call FS & generate FS request
    // string

    StringBuffer fsReq = new StringBuffer("COPY FILE "+tempFile.getPath());
    boolean toFileSet = false;

    if ((pResult.optionTimes(TOFILE)) == 1)
    {
        fsReq.append(" TOFILE "+pResult.optionValue(TOFILE));
        toFileSet = true;
    }

    if ((pResult.optionTimes(TOMACHINE)) == 1)
    {
        fsReq.append(" TOMACHINE "+pResult.optionValue(TOMACHINE));
        toFileSet = true;
    }

    if (!toFileSet)
    {
        fsReq.append(" TOFILE "+inputFile);
    }

    if ((pResult.optionTimes(FAILIFEXISTS)) == 1)
    {
        fsReq.append(" FAILIFEXISTS");
    }

    if ((pResult.optionTimes(FAILIFNEW)) == 1)
    {
        fsReq.append(" FAILIFNEW");
    }

    /* Send STAF FS Command */

    try
    {
        String result = sHandle.submit("local", "FS", fsReq.toString());
        return new STAFResult(STAFResult.Ok, result);
    }
    catch(STAFException se)
    {
        return new STAFResult(se.rc, se.getMessage());
    }
    finally
    {
        tempFile.delete();
    }
}

/*****************************************************************************/
/*                                                                           */
/* Method: term                                                              */
/* Description: required by interface STAFServiceInterfaceLevel30            */
/*                                                                           */
/*****************************************************************************/

public STAFResult term()
{
    // Un-register the service handle

    try
    {
        sHandle.unRegister();
    }
    catch (STAFException ex)
    {
        return new STAFResult(STAFResult.STAFRegistrationError,
                              ex.toString());
    }

    return new STAFResult(STAFResult.Ok);
}

/*****************************************************************************/
/*                                                                           */
/* Method: version                                                           */
/* Description: returns service version information                          */
/* Parameters: none                                                          */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleVersion(
    STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 1

    STAFResult trustResult = STAFUtil.validateTrust(
        1, fServiceName, "VERSION", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Return the service's version
    
    return new STAFResult(STAFResult.Ok, FSEXTVERSION);
}

/*****************************************************************************/
/*                                                                           */
/* Method: waitForFile                                                       */
/* Description: performs waitForFile service function                        */
/* Parameters: info - RequestInfo passed to acceptRequest                    */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/

private STAFResult waitForFile(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "WAITFORFILE", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    /* Parse request string */

    STAFCommandParseResult pResult = waitForFileParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    /* Parse FILE, POLL, & TIMEOUT parameters */

    File file;
    String poll = "15000"; //default value
    long timeout =
        Long.MAX_VALUE; // set to max value, for default infinite (almost) wait

    try
    {
        STAFResult res = STAFUtil.resolveRequestVar(
            pResult.optionValue(FILE), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String filename = res.result;
        file = new File(filename);

        if (pResult.optionTimes(TIMEOUT) == 1)
        {   
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(TIMEOUT), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            timeout = Long.parseLong(res.result);
        }
        if (pResult.optionTimes(POLL) == 1)
        {
            res = STAFUtil.resolveRequestVarAndCheckInt(
                POLL, pResult.optionValue(POLL), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            poll = res.result;
        }
    }
    catch(NumberFormatException nfe)
    {
        return new STAFResult(STAFResult.InvalidValue, nfe.toString());
    }

    /* Get not parameter */

    boolean not = false;

    if (pResult.optionTimes(NOT) == 1)
    {
        not = true;
    }

    /* Get starting time */

    long start = (new Date()).getTime();
    Date current;

    /* Perform appropriate file check, if failed wait POLL ms and try again */
    /* Quit after TIMEOUT ms has passed */

    do
    {
        if ((file.exists() && !not) || (!file.exists() && not))
        {
            return new STAFResult(STAFResult.Ok);
        }

        sHandle.submit2("local", "DELAY", "DELAY "+poll);
        current = new Date();

    }
    while ((current.getTime() - start) < timeout);

    /* If we get here timed out without finding file */

    if (not)
    {
        return new STAFResult(FILEEXISTS, file.getPath());
    }
    else
    {
        return new STAFResult(FILENOTFOUND, file.getPath());
    }

}
}
