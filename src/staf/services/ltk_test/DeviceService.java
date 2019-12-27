/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.deviceservice;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.util.Calendar;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.TreeMap;
import java.util.Iterator;
import java.text.SimpleDateFormat;
import java.io.PrintWriter;
import java.io.StringWriter;

public class DeviceService implements STAFServiceInterfaceLevel30
{
    private final String kVersion = "3.4.0";

    private String fServiceName;
    private STAFHandle fHandle;
    private String fLocalMachineName = "";

    // Define any error codes unique to this service
    private static final int kDeviceInvalidSerialNumber = 4001;

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    // STAFCommandParsers for each request
    private STAFCommandParser fListParser;
    private STAFCommandParser fQueryParser;
    private STAFCommandParser fAddParser;
    private STAFCommandParser fDeleteParser;

    // Map Class Definitions used to create marshalled results
    public static STAFMapClassDefinition fListDeviceMapClass;
    public static STAFMapClassDefinition fQueryDeviceMapClass;

    private static String sHelpMsg;
    private String fLineSep;
    private TreeMap fPrinterMap = new TreeMap();
    private TreeMap fModemMap = new TreeMap();

    public DeviceService() {}

    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        try
        {
            fServiceName = info.name;
            fHandle = new STAFHandle("STAF/Service/" + info.name);
        }
        catch (STAFException e)
        {
            return new STAFResult(STAFResult.STAFRegistrationError,
                                  e.toString());
        }

        // ADD parser
        
        fAddParser = new STAFCommandParser();

        fAddParser.addOption("ADD", 1,
                             STAFCommandParser.VALUENOTALLOWED);

        fAddParser.addOption("PRINTER", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fAddParser.addOption("MODEL", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fAddParser.addOption("SN", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fAddParser.addOption("MODEM", 1,
                             STAFCommandParser.VALUEREQUIRED);

        // this means you can have PRINTER or MODEM, but not both
        fAddParser.addOptionGroup("PRINTER MODEM", 0, 1);

        // if you specify ADD, MODEL is required
        fAddParser.addOptionNeed("ADD", "MODEL");

        // if you specify ADD, SN is required
        fAddParser.addOptionNeed("ADD", "SN");

        // if you specify PRINTER or MODEM, ADD is required
        fAddParser.addOptionNeed("PRINTER MODEM", "ADD");

        // if you specify ADD, PRINTER or MODEM is required
        fAddParser.addOptionNeed("ADD", "PRINTER MODEM");

        // LIST parser

        fListParser = new STAFCommandParser();

        fListParser.addOption("LIST", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fListParser.addOption("PRINTERS", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        fListParser.addOption("MODEMS", 1,
                              STAFCommandParser.VALUENOTALLOWED);

        // QUERY parser

        fQueryParser = new STAFCommandParser();

        fQueryParser.addOption("QUERY", 1,
                               STAFCommandParser.VALUENOTALLOWED);

        fQueryParser.addOption("PRINTER", 1,
                               STAFCommandParser.VALUEREQUIRED);

        fQueryParser.addOption("MODEM", 1,
                               STAFCommandParser.VALUEREQUIRED);

        // This means you can have PRINTER or MODEM, but not both
        fQueryParser.addOptionGroup("PRINTER MODEM", 0, 1);

        // If you specify PRINTER or MODEM, QUERY is required
        fQueryParser.addOptionNeed("PRINTER MODEM", "QUERY");

        // If you specify QUERY, PRINTER or MODEM is required
        fQueryParser.addOptionNeed("QUERY", "PRINTER MODEM");

        // DELETE parser

        fDeleteParser = new STAFCommandParser();

        fDeleteParser.addOption("DELETE", 1,
                               STAFCommandParser.VALUENOTALLOWED);

        fDeleteParser.addOption("PRINTER", 1,
                                STAFCommandParser.VALUEREQUIRED);

        fDeleteParser.addOption("MODEM", 1,
                                STAFCommandParser.VALUEREQUIRED);

        fDeleteParser.addOption("CONFIRM", 1,
                               STAFCommandParser.VALUENOTALLOWED);

        // This means you must have PRINTER or MODEM, but not both
        fDeleteParser.addOptionGroup("PRINTER MODEM", 0, 1);

        // If you specify PRINTER or MODEM, DELETE is required
        fDeleteParser.addOptionNeed("PRINTER MODEM", "DELETE");

        // If you specify DELETE, PRINTER or MODEM is required
        fDeleteParser.addOptionNeed("DELETE", "PRINTER MODEM");

        // If you specify DELETE, CONFIRM is required
        fDeleteParser.addOptionNeed("DELETE", "CONFIRM");

        // Construct map class for the result from a LIST request.

        fListDeviceMapClass = new STAFMapClassDefinition(
            "STAF/Service/Device/ListDevice");
        fListDeviceMapClass.addKey("name",    "Name");
        fListDeviceMapClass.addKey("type",    "Type");
        fListDeviceMapClass.addKey("model",   "Model");
        fListDeviceMapClass.addKey("serial#", "Serial Number");
        fListDeviceMapClass.setKeyProperty(
            "serial#", "display-short-name", "Serial #");

        // Construct map class for the result from a QUERY request.

        fQueryDeviceMapClass = new STAFMapClassDefinition(
            "STAF/Service/Device/QueryDevice");
        fQueryDeviceMapClass.addKey("model",   "Model");
        fQueryDeviceMapClass.addKey("serial#", "Serial Number");

        STAFResult res = new STAFResult();

        // Resolve the line separator variable for the local machine

        res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", fHandle);

        if (res.rc != STAFResult.Ok) return res;
        
        fLineSep = res.result;
         
        // Resolve the machine name variable for the local machine

        res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", fHandle);

        if (res.rc != STAFResult.Ok) return res;
        
        fLocalMachineName = res.result;

        // Assign the help text string for the service

        sHelpMsg = "*** " + fServiceName + " Service Help ***" +
            fLineSep + fLineSep +
            "ADD     < PRINTER <PrinterName> | MODEM <ModemName> > MODEL <Model> SN <Serial#>" +
            fLineSep +
            "DELETE  < PRINTER <PrinterName> | MODEM <ModemName> > CONFIRM" +
            fLineSep +
            "LIST    [PRINTERS] [MODEMS]" +
            fLineSep +
            "QUERY   PRINTER <PrinterName> | MODEM <ModemName>" +
            fLineSep +
            "VERSION" +
            fLineSep +
            "HELP";

        // Register Help Data

        registerHelpData(
            kDeviceInvalidSerialNumber,
            "Invalid serial number",
            "A non-numeric value was specified for serial number");

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

            if (actionLC.equals("list"))
                return handleList(info);
            else if (actionLC.equals("query"))
                return handleQuery(info);
            else if (actionLC.equals("add"))
                return handleAdd(info);
            else if (actionLC.equals("delete"))
                return handleDelete(info);
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

        // Return help text for the service

        return new STAFResult(STAFResult.Ok, sHelpMsg);
    }

    private STAFResult handleVersion(
        STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 1

        STAFResult trustResult = STAFUtil.validateTrust(
            1, fServiceName, "VERSION", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Return the service's version

        return new STAFResult(STAFResult.Ok, kVersion);
    }

    private STAFResult handleAdd(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, fServiceName, "ADD", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fAddParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Resolve any STAF variables in the printer option's value

        STAFResult res = new STAFResult();

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("printer"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String printer = res.result;

        // Resolve any STAF variables in the modem option's value

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("modem"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String modem = res.result;

        // Resolve any STAF variables in the model option's value

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("model"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String model = res.result;

        // Resolve any STAF variables in the sn option's value

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("sn"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;

        String sn = res.result;

        // Verify that the serial number is numeric

        try
        {
            new Integer(sn);
        }
        catch (NumberFormatException e)
        {
            // Note that instead of creating a new error code specific for
            // this service, should use STAFResult.InvalidValue instead, but
            // wanted to demonstrate the registration of a service error code.
            return new STAFResult(kDeviceInvalidSerialNumber, sn);
        }

        // Add the device to the printer map or the modem map and
        // write an informational message to the service log

        if (!printer.equals(""))
        {
            synchronized (fPrinterMap)
            {
                fPrinterMap.put(printer, new DeviceData(model, sn));
            }

            String logMsg = "ADD PRINTER request.  Name=" + printer +
                            " Model=" + model + " Serial#=" + sn;

            fHandle.submit2(
                "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
                " LEVEL info MESSAGE " + STAFUtil.wrapData(logMsg));
        }
        else if (!modem.equals(""))
        {
            synchronized (fModemMap)
            {
                fModemMap.put(modem, new DeviceData(model, sn));
            }

            String logMsg = "ADD MODEM request.  Name=" + modem +
                            " Model=" + model + " Serial#=" + sn;

            fHandle.submit2(
                "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
                " LEVEL info MESSAGE " + STAFUtil.wrapData(logMsg));
        }

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult handleList(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "LIST", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fListParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Check if specified printers or modems

        int printersOption = parsedRequest.optionTimes("printers");
        int modemsOption = parsedRequest.optionTimes("modems");

        boolean defaultList = false;

        if (printersOption == 0 && modemsOption == 0)
        {
            defaultList = true;
        }

        // Create a marshalling context and set any map classes (if any).

        STAFMarshallingContext mc = new STAFMarshallingContext();
        mc.setMapClassDefinition(fListDeviceMapClass);

        // Create an empty result list to contain the result

        List resultList = new ArrayList();

        // Add printer entries to the result list

        if (defaultList || printersOption > 0)
        {
            Iterator iter = fPrinterMap.keySet().iterator();

            while (iter.hasNext())
            {
                String key = (String)iter.next();

                DeviceData data = (DeviceData)fPrinterMap.get(key);

                Map resultMap = fListDeviceMapClass.createInstance();

                resultMap.put("name", key);
                resultMap.put("type", "Printer");
                resultMap.put("model", data.model);
                resultMap.put("serial#", data.sn);

                resultList.add(resultMap);
            }
        }

        // Add modem entries to the result list

        if (defaultList || modemsOption > 0)
        {
            Iterator iter = fModemMap.keySet().iterator();

            while (iter.hasNext())
            {
                String key = (String)iter.next();

                DeviceData data = (DeviceData)fModemMap.get(key);

                Map resultMap = fListDeviceMapClass.createInstance();

                resultMap.put("name", key);
                resultMap.put("type", "Modem");
                resultMap.put("model", data.model);
                resultMap.put("serial#", data.sn);

                resultList.add(resultMap);
            }
        }

        // Set the result list as the root object for the marshalling context
        // and return the marshalled result

        mc.setRootObject(resultList);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private STAFResult handleQuery(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 2

        STAFResult trustResult = STAFUtil.validateTrust(
            2, fServiceName, "QUERY", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fQueryParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }
        
        // Resolve any STAF variables in the printer option's value

        STAFResult res = new STAFResult();

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("printer"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String printer = res.result;

        // Resolve any STAF variables in the modem option's value

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("modem"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String modem = res.result;

        // Create a marshalling context and set any map classes (if any).

        STAFMarshallingContext mc = new STAFMarshallingContext();
        mc.setMapClassDefinition(fQueryDeviceMapClass);

        // Create an empty result map to contain the result
        
        Map resultMap = fQueryDeviceMapClass.createInstance();

        // Find the specified printer/modem and add its info to the result map

        if (!printer.equals(""))
        {
            if (fPrinterMap.containsKey(printer))
            {
                DeviceData data = (DeviceData)fPrinterMap.get(printer);
                
                resultMap.put("model", data.model);
                resultMap.put("serial#", data.sn);
            }
            else
            {
                return new STAFResult(STAFResult.DoesNotExist, printer);
            }
        }
        else if (!modem.equals(""))
        {
            if (fModemMap.containsKey(modem))
            {
                DeviceData data = (DeviceData)fModemMap.get(modem);
                
                resultMap.put("model", data.model);
                resultMap.put("serial#", data.sn);
            }
            else
            {
                return new STAFResult(STAFResult.DoesNotExist, modem);
            }
        }

        // Set the result map as the root object for the marshalling context
        // and return the marshalled result

        mc.setRootObject(resultMap);

        return new STAFResult(STAFResult.Ok, mc.marshall());
    }

    private STAFResult handleDelete(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        // Verify the requester has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, fServiceName, "DELETE", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parsedRequest = fDeleteParser.parse(info.request);

        if (parsedRequest.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parsedRequest.errorBuffer);
        }

        // Resolve any STAF variables in the print option's value

        STAFResult res = new STAFResult();

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("printer"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String printer = res.result;
        
        // Resolve any STAF variables in the modem option's value

        res = STAFUtil.resolveRequestVar(
            parsedRequest.optionValue("modem"), fHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;
        
        String modem = res.result;

        // Find the device in the printer or modem map and remove it and
        // write an informational message to the service log

        if (!printer.equals(""))
        {
            synchronized (fPrinterMap)
            {
                if (fPrinterMap.containsKey(printer))
                    fPrinterMap.remove(printer);
                else
                    return new STAFResult(STAFResult.DoesNotExist, printer);
            }

            String logMsg = "DELETE PRINTER request.  Name=" + printer;

            fHandle.submit2(
                "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
                " LEVEL info MESSAGE " + STAFUtil.wrapData(logMsg));
        }
        else if (!modem.equals(""))
        {
            synchronized (fModemMap)
            {
                if (fModemMap.containsKey(modem))
                    fModemMap.remove(modem);
                else
                    return new STAFResult(STAFResult.DoesNotExist, modem);
            }

            String logMsg = "DELETE MODEM request.  Name=" + modem;

            fHandle.submit2(
                "local", "LOG", "LOG MACHINE LOGNAME " + fServiceName +
                " LEVEL info MESSAGE " + STAFUtil.wrapData(logMsg));
        }

        return new STAFResult(STAFResult.Ok);
    }

    public STAFResult term()
    {
        try
        {
            // Un-register Help Data

            unregisterHelpData(kDeviceInvalidSerialNumber);

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

    // Register error codes for this service with the HELP service

    private void registerHelpData(int errorNumber, String info,
                                  String description)
    {
        STAFResult res = fHandle.submit2(
            "local", "HELP", "REGISTER SERVICE " + fServiceName +
            " ERROR " + errorNumber +
            " INFO " + STAFUtil.wrapData(info) +
            " DESCRIPTION " + STAFUtil.wrapData(description));
    }

    // Un-register error codes for this service with the HELP service

    private void unregisterHelpData(int errorNumber)
    {
        STAFResult res = fHandle.submit2(
            "local", "HELP", "UNREGISTER SERVICE " + fServiceName +
            " ERROR " + errorNumber);
    }

    public class DeviceData
    {
        public String model = "";
        public String sn = "";

        public DeviceData(String model, String sn)
        {
            this.model = model;
            this.sn = sn;
        }
    }
}
