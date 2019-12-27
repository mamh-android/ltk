package com.ibm.staf.service.timer;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/****************************************************************************/
//
// Class: Timer
//
// Logical Name: Timer.java
//
// Description: This class accepts requests from STAF.
//
//
// History:
//
// DATE       DEVELOPER   CHANGE DESCRIPTION
// ---------- ----------- -----------------------------------
// 02/01/2000 C Alkov     Original Program
//
/****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.wrapper.STAFLog;
import com.ibm.staf.service.STAFServiceInterfaceLevel30;
import java.io.File;

public class Timer implements STAFServiceInterfaceLevel30
{
    TimerRequestHandler reqHandler;
    private TimerCmdParser cmdParser;
    STAFHandle sHandle;
    STAFHandle wHandle;
    String sServiceName;
    String fDataDir;
    String localMachine;
    static String sLineSep;
    STAFLog log;
    String tListFileName;
    String wListFileName;
    private String FS;

/****************************************************************************/
//
// Method:
//   Constructor
//
// Description:
//   Constructor method for Timer class.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public Timer()
{
    /* Create command parser */

    cmdParser = new TimerCmdParser(this);

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
    STAFResult result = cmdParser.parse(info);

    return result;
}

/***************************************************************/
/* init - Initializes Timer Service with STAF.                 */
/*                                                             */
/* accepts: STAFServiceInterfaceLevel30 initialization info    */
/*          Parms includes a string representation of the      */
/*            directory to use for persistent storage.         */
/*                                                             */
/* Returns: STAFResult.Ok or STAFResult.STAFRegistrationError  */
/***************************************************************/
public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
{
    /* Set service name */

    sServiceName = info.name;

    /* Register with STAF */

    try
    {
        sHandle = new STAFHandle("STAF/Service/" + sServiceName);
    }
    catch (STAFException e)
    {
        return new STAFResult(e.rc, e.toString());
    }

    /* Create a process for watch manager, this process will only be used as an exclusive
       queue for messages, all other requests will be handled by the sHandle process */

    try
    {
        wHandle = new STAFHandle("STAF/Service/" + sServiceName + "/Watch");
    }
    catch (STAFException e)
    {
        return new STAFResult(e.rc, e.toString());
    }

    /* Create STAFlog instance */

    log = new STAFLog(STAFLog.MACHINE, "STAFSERVICE_" + sServiceName, sHandle);

    // Resolve the file separator variable for the local machine

    STAFResult res = STAFUtil.resolveInitVar("{STAF/Config/Sep/File}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    FS = res.result;

    // Create String to represent filenames for persistent storage
        
    if (info.parms.equals(new String()))
    {
        // No parm set.  Store data for the service in directory:
        //   <STAF Write Location>/service/<service name (lower-case)>

        fDataDir = info.writeLocation;

        if (!fDataDir.endsWith(FS))
        {
            fDataDir += FS;
        }

        fDataDir = fDataDir + "service" + FS + sServiceName.toLowerCase();

        File dir = new File(fDataDir);
        
        if (!dir.exists())
        {
            dir.mkdirs();
        }

        tListFileName = fDataDir + FS + "tlist.ser";
        wListFileName = fDataDir + FS + "wlist.ser";
    }
    else
    {
        if (info.parms.indexOf("{") != -1)
        {
            // The string may contains STAF variables

            STAFResult resolvedResult = sHandle.submit2(
                "local", "VAR", "RESOLVE STRING " +
                STAFUtil.wrapData(info.parms));

            if (resolvedResult.rc != STAFResult.Ok)
            {
                System.out.println("Error resolving DIRECTORY.  RC=" +
                    resolvedResult.rc + " Result=" + resolvedResult.result);

                return resolvedResult;
            }

            fDataDir = resolvedResult.result;
        }
        else
        {
            fDataDir = info.parms;
        }

        tListFileName = fDataDir + FS + "tlist.ser";
        wListFileName = fDataDir + FS + "wlist.ser";
    }

    // Resolve the machine name variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    localMachine = res.result;

    // Resolve the line separator variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    sLineSep = res.result;
    
    // Create request handler

    reqHandler = new TimerRequestHandler(sServiceName, sHandle, this);

    // Get Timer variables from STAF using VAR service

    STAFResult result = reqHandler.refresh();

    return new STAFResult(result.rc);

}

/****************************************************************************/
//
// Method:
//   term
//
// Description:
//   The termination method for the Timer service.
//
// Input:
//   none
//
// Exceptions Thrown:
//   none
//
// Notes:
//   none
//
/****************************************************************************/

public STAFResult term()
{
    int rc = reqHandler.term();

    if (rc != STAFResult.Ok)
        return new STAFResult(rc, "Error terminating request handler.");

    return new STAFResult(rc);

}



    // Register error codes for the service with the HELP service

    private void registerHelpData(int errorNumber, String info,
                                 String description)
    {
        STAFResult res = sHandle.submit2(
            "local", "HELP", "REGISTER SERVICE " + sServiceName +
            " ERROR " + errorNumber + " INFO " + STAFUtil.wrapData(info) +
            " DESCRIPTION " + STAFUtil.wrapData(description));
    }

    // Un-register error codes for the service with the HELP service

    private void unregisterHelpData(int errorNumber)
    {
        STAFResult res = sHandle.submit2(
            "local", "HELP", "UNREGISTER SERVICE " + sServiceName +
            " ERROR " + errorNumber);
    }
}
