/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.Set;
import java.util.List;
import java.util.LinkedList;
import com.ibm.staf.service.stax.*;
import com.ibm.staf.*;
import com.ibm.staf.service.*;
import org.python.core.*;

public class STAXBreakpointActionFactory implements STAXActionFactory,
                                                    STAXJobManagementHandler,
                                                    STAXListRequestHandler,
                                                    STAXGenericRequestHandler
{
    public static final String BREAKPOINT = new String("breakpoint");

    static final String sShort = new String("short");
    static final String sLong = new String("long");
    //static final String sBreakpointInfoMapClassName = new String(
    //    "STAF/Service/STAX/BreakpointInfo");
    //static final String sQueryBreakpointMapClassName = new String(
    //    "STAF/Service/STAX/QueryBreakpoint");
    //static final String sBreakpointVariableMapClassName = new String(
    //    "STAF/Service/STAX/BreakpointVariable");
    static final String sListBreakpointsMapClassName = new String(
        "STAF/Service/STAX/BreakpointInfo");

    //public STAFMapClassDefinition fBreakpointVariableMapClass;
    public STAFMapClassDefinition fListBreakpointsMapClass;

    private static String fDTDInfo =
"\n" +
"<!--================= The Breakpoint Element ============================ -->\n" +
"<!--\n" +
"     The breakpoint element allows you to denote a breakpoint.\n" +
"-->\n" +
"<!ELEMENT breakpoint     EMPTY>\n";

    public STAXBreakpointActionFactory() { }

    public STAXBreakpointActionFactory(STAX staxService)
    {
        // Note that in order for it's initJob method to be run at the start
        // of a job, you must register the factory as a Job Management handler.

        staxService.registerJobManagementHandler(this);
        staxService.registerListHandler("BREAKPOINTS", this);

        // Construct map-class for list breakpoints information

        /*fBreakpointInfoMapClass = new STAFMapClassDefinition(
            sBreakpointInfoMapClassName);

        fBreakpointInfoMapClass.addKey("id", "ID");
        fBreakpointInfoMapClass.addKey("lineNumber", "Line #");
        fBreakpointInfoMapClass.addKey("xmlFile", "XML File");
        fBreakpointInfoMapClass.addKey("xmlMachine", "XML Machine");

        fQueryBreakpointMapClass = new STAFMapClassDefinition(
            sQueryBreakpointMapClassName);

        fQueryBreakpointMapClass.addKey("id", "ID");
        fQueryBreakpointMapClass.addKey("lineNumber", "Line #");
        fQueryBreakpointMapClass.addKey("xmlFile", "XML File");
        fQueryBreakpointMapClass.addKey("xmlMachine", "XML Machine");
        fQueryBreakpointMapClass.addKey("vars", "Variables");
        fQueryBreakpointMapClass.addKey("blockName", "Block Name");

        fBreakpointVariableMapClass = new STAFMapClassDefinition(
            sBreakpointVariableMapClassName);

        fBreakpointVariableMapClass.addKey("name", "Name");
        fBreakpointVariableMapClass.addKey("value", "Value");
        fBreakpointVariableMapClass.addKey("type", "Type");*/

        // Add parser

        fAddParser.addOption("ADD", 1,
                             STAFCommandParser.VALUENOTALLOWED);
        fAddParser.addOption("JOB", 1,
                             STAFCommandParser.VALUEREQUIRED);
        fAddParser.addOption("BREAKPOINT", 1,
                             STAFCommandParser.VALUENOTALLOWED);
        fAddParser.addOption("FUNCTION", 1,
                             STAFCommandParser.VALUEREQUIRED);
        fAddParser.addOption("LINE", 1,
                             STAFCommandParser.VALUEREQUIRED);
        fAddParser.addOption("FILE", 1,
                             STAFCommandParser.VALUEREQUIRED);
        fAddParser.addOption("MACHINE", 1,
                             STAFCommandParser.VALUEREQUIRED);

        fAddParser.addOptionNeed("BREAKPOINT", "ADD");
        fAddParser.addOptionNeed("JOB", "BREAKPOINT");
        fAddParser.addOptionNeed("FUNCTION", "BREAKPOINT");
        fAddParser.addOptionNeed("LINE", "BREAKPOINT");
        fAddParser.addOptionNeed("BREAKPOINT", "FUNCTION LINE");
        fAddParser.addOptionGroup("FUNCTION LINE", 0, 1);
        fAddParser.addOptionGroup("FUNCTION FILE", 0, 1);
        fAddParser.addOptionGroup("FUNCTION MACHINE", 0, 1);
        fAddParser.addOptionNeed("FILE", "LINE");

        // Remove parser

        fRemoveParser.addOption("REMOVE", 1,
                                STAFCommandParser.VALUENOTALLOWED);
        fRemoveParser.addOption("JOB", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fRemoveParser.addOption("BREAKPOINT", 1,
                                STAFCommandParser.VALUEREQUIRED);

        fRemoveParser.addOptionNeed("BREAKPOINT", "REMOVE");
        fRemoveParser.addOptionNeed("JOB", "REMOVE");
        fRemoveParser.addOptionNeed("JOB", "BREAKPOINT");

        // Resume Parser

        fResumeParser.addOption("RESUME", 1,
                                STAFCommandParser.VALUENOTALLOWED);
        fResumeParser.addOption("JOB", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fResumeParser.addOption("THREAD", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fResumeParser.addOptionNeed("JOB", "RESUME");
        fResumeParser.addOptionNeed("THREAD", "RESUME");

        // Step Parser

        fStepParser.addOption("STEP", 1,
                              STAFCommandParser.VALUENOTALLOWED);
        fStepParser.addOption("JOB", 1,
                              STAFCommandParser.VALUEREQUIRED);
        fStepParser.addOption("THREAD", 1,
                               STAFCommandParser.VALUEREQUIRED);
        fStepParser.addOption("INTO", 1,
                               STAFCommandParser.VALUENOTALLOWED);
        fStepParser.addOption("OVER", 1,
                               STAFCommandParser.VALUENOTALLOWED);
        fStepParser.addOptionNeed("JOB", "STEP");
        fStepParser.addOptionNeed("THREAD", "STEP");
        fStepParser.addOptionGroup("INTO OVER", 0, 1);

        // List Breakpoints

        fListBreakpointsMapClass = new STAFMapClassDefinition(
            sListBreakpointsMapClassName);

        fListBreakpointsMapClass.addKey("ID", "ID");
        fListBreakpointsMapClass.addKey("function",  "Function Name");
        fListBreakpointsMapClass.addKey("line", "Line #");
        fListBreakpointsMapClass.addKey("file", "XML File");
        fListBreakpointsMapClass.addKey("machine", "Machine");

        // Register as a GenericRequestHandler
        try
        {
            // Assign STAFServiceInterfaceLevel class that this handler uses

            Class serviceInterfaceClass = Class.forName(STAX.INTERFACE_LEVEL_30);

            int rc = staxService.registerGenericRequestHandler(this,
                                                      serviceInterfaceClass);

            if (rc != 0)
            {
                System.out.println("STAXBreakpointActionFactory." +
                                   "registerGenericRequestHandler() failed");
            }
        }
        catch (ClassNotFoundException e)
        {
            System.out.println("STAXBreakpointActionFactory: " +
                               "registerGenericRequestHandler: " + e);
        }
    }

    public STAXBreakpointActionFactory(STAX staxService, Map parmMap)
        throws STAXExtensionInitException
    {
        staxService.registerJobManagementHandler(this);
    }


    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "breakpoint";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXBreakpointAction breakpoint = new STAXBreakpointAction();
        breakpoint.setActionFactory(this);

        breakpoint.setLineNumber(root);
        breakpoint.setXmlFile(job.getXmlFile());
        breakpoint.setXmlMachine(job.getXmlMachine());

        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.CDATA_SECTION_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.TEXT_NODE)
            {
                breakpoint.setElementInfo(new STAXElementInfo(root.getNodeName()));
            }
        }

        return breakpoint;
    }

    public STAXAction createStepBreakpoint(STAXAction action)
    {
        STAXBreakpointAction breakpoint = new STAXBreakpointAction();
        breakpoint.setActionFactory(this);

        breakpoint.setLineNumber(BREAKPOINT,
            ((STAXActionDefaultImpl)action).getLineNumber());
        breakpoint.setXmlFile(
            ((STAXActionDefaultImpl)action).getXmlFile());
        breakpoint.setXmlMachine(
            ((STAXActionDefaultImpl)action).getXmlMachine());

        StringBuffer actionInfo = new
            StringBuffer(((STAXActionDefaultImpl)action).getElement());

        if ((((STAXActionDefaultImpl)action).getInfo() != null) &&
            (((STAXActionDefaultImpl)action).getInfo().length() > 0))
        {
            actionInfo.append(": ").
                append(((STAXActionDefaultImpl)action).getInfo());
        }

        breakpoint.setInfo(actionInfo.toString());

        return breakpoint;
    }

        // STAXGenericRequestHandler Interface Methods

    public STAFResult handleRequest(Object infoObject, STAX staxService)
    {
        STAFServiceInterfaceLevel30.RequestInfo info =
            (STAFServiceInterfaceLevel30.RequestInfo)infoObject;

        String lowerRequest = info.request.toLowerCase();

        if (lowerRequest.startsWith("add"))
            return handleAddRequest(info, staxService);
        else if (lowerRequest.startsWith("remove"))
            return handleRemoveRequest(info, staxService);
        else if (lowerRequest.startsWith("resume"))
            return handleResumeRequest(info, staxService);
        else if (lowerRequest.startsWith("step"))
            return handleStepRequest(info, staxService);
        else
        {   // Returning nothing in the result indicates that this parser
            // does not support this request.
            return new STAFResult(STAFResult.InvalidRequestString, "");
        }
    }

    private STAFResult handleAddRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, staxService.getServiceName(), "UPDATE", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fAddParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Resolve the value specified for JOB and get its integer value

        STAFResult resolvedValue = STAFUtil.resolveRequestVarAndCheckInt(
            "JOB", parseResult.optionValue("JOB"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (resolvedValue.rc != 0) return resolvedValue;

        Integer jobID = new Integer(resolvedValue.result);

        // Verify that the JOB ID is a currently active job

        STAXJob job = null;

        job = (STAXJob)staxService.getJobMap().get(jobID);

        if (job == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Job " + jobID + " is not currently running.");
        }

        // Handle BREAKPOINT

        String function = "";
        String line = "";
        String file = "";
        String machine = "";

        int breakpointID = 0;

        if (parseResult.optionTimes("FUNCTION") > 0)
        {

            // Resolve the value specified for FUNCTION
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("FUNCTION"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            function = resolvedValue.result;

            breakpointID = job.addBreakpointFunction(function);
        }
        else if (parseResult.optionTimes("LINE") > 0)
        {
            // Resolve the value specified for LINE
            resolvedValue = STAFUtil.resolveRequestVar(
                parseResult.optionValue("LINE"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (resolvedValue.rc != 0)
                return resolvedValue;

            line = resolvedValue.result;

            if (parseResult.optionTimes("FILE") > 0)
            {
                // Resolve the value specified for FILE
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("FILE"),
                    staxService.getSTAFHandle(), info.requestNumber);

                if (resolvedValue.rc != 0)
                    return resolvedValue;

                file = resolvedValue.result;
            }
            else
            {
                file = job.getXmlFile();
            }

            if (parseResult.optionTimes("MACHINE") > 0)
            {
                // Resolve the value specified for MACHINE
                resolvedValue = STAFUtil.resolveRequestVar(
                    parseResult.optionValue("MACHINE"),
                    staxService.getSTAFHandle(), info.requestNumber);

                if (resolvedValue.rc != 0)
                    return resolvedValue;

                machine = resolvedValue.result;
            }

            String breakpointMachineName = machine;

            if (machine.equals(""))
            {
                breakpointMachineName = info.endpoint;
            }

            String fileSep = "";

            if (STAXFileCache.get().isLocalMachine(breakpointMachineName))
            {
                // Assign the file separator for the local STAX machine
                fileSep = STAX.fileSep;
            }
            else
            {
                if (STAXMachineCache.get().checkCache(
                    breakpointMachineName))
                {
                    fileSep = STAXMachineCache.get().getFileSep(
                        breakpointMachineName);
                }
                else
                {
                    STAFResult result = staxService.getSTAFHandle().submit2(
                        breakpointMachineName, "VAR",
                        "RESOLVE STRING {STAF/Config/Sep/File}");

                    if (result.rc == STAFResult.Ok)
                    {
                        fileSep = result.result;
                    }
                    else
                    {
                        // XXX
                    }
                }
            }

            file = STAXUtil.normalizeFilePath(file, fileSep);

            String breakpointLine = line + " " + machine + " " + file;

            breakpointID = job.addBreakpointLine(line, file, machine);
        }

        HashMap<String, String> triggerMap = new HashMap<String, String>();
        triggerMap.put("type", "breakpoint");
        triggerMap.put("status", "add");
        triggerMap.put("id", String.valueOf(breakpointID));
        triggerMap.put("function", function);
        triggerMap.put("line", line);
        triggerMap.put("file", file);
        triggerMap.put("machine", machine);

        job.generateEvent(STAXBreakpointActionFactory.BREAKPOINT,
            triggerMap);

        return new STAFResult(STAFResult.Ok,
                              String.valueOf(breakpointID));
    }

    private STAFResult handleRemoveRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, staxService.getServiceName(), "UPDATE", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fRemoveParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Resolve the value specified for JOB and get its integer value

        STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
            "JOB", parseResult.optionValue("JOB"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        Integer jobID = new Integer(res.result);

        // Verify that the JOB ID is a currently active job

        STAXJob job = null;

        job = (STAXJob)staxService.getJobMap().get(jobID);

        if (job == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Job " + jobID + " is not currently running.");
        }

        // Resolve the value specified for BreakpointTrigger

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("BREAKPOINT"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String breakpointID = res.result;

        STAFResult removeResult =
            job.removeBreakpoint(breakpointID);

        if (removeResult.rc == STAFResult.Ok)
        {
            HashMap<String, String> triggerMap = new HashMap<String, String>();
            triggerMap.put("type", "breakpoint");
            triggerMap.put("status", "remove");
            triggerMap.put("id", String.valueOf(breakpointID));

            job.generateEvent(STAXBreakpointActionFactory.BREAKPOINT,
                triggerMap);
        }

        return removeResult;
    }

    private STAFResult handleResumeRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, staxService.getServiceName(), "UPDATE", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fResumeParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Resolve the value specified for JOB and get its integer value

        STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
            "JOB", parseResult.optionValue("JOB"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        Integer jobID = new Integer(res.result);

        // Verify that the JOB ID is a currently active job

        STAXJob job = null;

        job = (STAXJob)staxService.getJobMap().get(jobID);

        if (job == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Job " + jobID + " is not currently running.");
        }

        // Resolve the value specified for Breakpoint

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("THREAD"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String breakpoint = res.result;

        TreeMap breakpointMap = (TreeMap)job.getData("breakpointMap");

        if (!breakpointMap.containsKey(breakpoint))
            return new STAFResult(STAFResult.DoesNotExist, breakpoint);

        STAXBreakpointAction theBreakpoint =
            (STAXBreakpointAction)breakpointMap.get(breakpoint);

        STAFResult result = staxService.getSTAFHandle().submit2(
             "local", staxService.getServiceName(),
             "QUERY JOB " + jobID + " THREAD " + breakpoint);

        STAFMarshallingContext mc =
                STAFMarshallingContext.unmarshall(result.result);

        Map threadInfoMap = (Map)mc.getRootObject();

        List conditionStack = (List)(threadInfoMap.get("conditionStack"));

        if (conditionStack != null)
        {
            Iterator conditionStackIter = conditionStack.iterator();

            while (conditionStackIter.hasNext())
            {
                if (conditionStackIter.next().toString().equals(
                    "HoldThread: Source=Block, Priority=1000"))
                {
                    return new STAFResult(STAX.BreakpointBlockHeld, "");
                }
            }
        }

        theBreakpoint.resumeBreakpoint();

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult handleStepRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 4

        STAFResult trustResult = STAFUtil.validateTrust(
            4, staxService.getServiceName(), "UPDATE", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fStepParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  parseResult.errorBuffer);
        }

        // Resolve the value specified for JOB and get its integer value

        STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
            "JOB", parseResult.optionValue("JOB"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        Integer jobID = new Integer(res.result);

        // Verify that the JOB ID is a currently active job

        STAXJob job = null;

        job = (STAXJob)staxService.getJobMap().get(jobID);

        if (job == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Job " + jobID + " is not currently running.");
        }

        // Resolve the value specified for Breakpoint

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("THREAD"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String breakpoint = res.result;

        TreeMap breakpointMap = (TreeMap)job.getData("breakpointMap");

        if (!breakpointMap.containsKey(breakpoint))
            return new STAFResult(STAFResult.DoesNotExist, breakpoint);

        STAXBreakpointAction theBreakpoint =
            (STAXBreakpointAction)breakpointMap.get(breakpoint);
         STAFResult result = staxService.getSTAFHandle().submit2(
             "local", staxService.getServiceName(),
             "QUERY JOB " + jobID + " THREAD " + breakpoint);

        STAFMarshallingContext mc =
                STAFMarshallingContext.unmarshall(result.result);

        Map threadInfoMap = (Map)mc.getRootObject();

        List conditionStack = (List)(threadInfoMap.get("conditionStack"));

        if (conditionStack != null)
        {
            Iterator conditionStackIter = conditionStack.iterator();

            while (conditionStackIter.hasNext())
            {
                if (conditionStackIter.next().toString().equals(
                    "HoldThread: Source=Block, Priority=1000"))
                {
                    return new STAFResult(STAX.BreakpointBlockHeld, "");
                }
            }
        }

        if (parseResult.optionTimes("OVER") > 0)
        {
            theBreakpoint.stepOverBreakpoint();
        }
        else
        {
            theBreakpoint.stepIntoBreakpoint();
        }

        return new STAFResult(STAFResult.Ok);
    }

    // STAXListRequestHandler method

    public STAFResult handleListRequest(String type, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("BREAKPOINTS"))
        {
            // LIST BREAKPOINTS

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fListBreakpointsMapClass);

            List<Map<String, Object>> jobList =
                new ArrayList<Map<String, Object>>();

            TreeMap<String, STAXBreakpoint> breakpointsMap =
                job.getBreakpointsMap();

            synchronized (breakpointsMap)
            {
                for (Map.Entry<String, STAXBreakpoint> entry :
                    job.getBreakpointsMap().entrySet())
                {
                    String id = entry.getKey();
                    STAXBreakpoint breakpoint = entry.getValue();

                    Map<String, Object> breakpointMap =
                        new TreeMap<String, Object>();
                    breakpointMap.put("staf-map-class-name",
                                      fListBreakpointsMapClass.name());
                    breakpointMap.put("ID", id);

                    if (!(breakpoint.getFunction().equals("")))
                    {
                        breakpointMap.put("function",
                            breakpoint.getFunction());
                    }

                    if (!(breakpoint.getLine().equals("")))
                    {
                        breakpointMap.put("line",
                            breakpoint.getLine());
                    }

                    if (!(breakpoint.getFile().equals("")))
                    {
                        breakpointMap.put("file",
                            breakpoint.getFile());
                    }

                    if (!(breakpoint.getMachine().equals("")))
                    {
                        breakpointMap.put("machine",
                            breakpoint.getMachine());
                    }

                    jobList.add(breakpointMap);
                }
            } // end synchronized (fBreakpointsMap)

            mc.setRootObject(jobList);
            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryJobRequest(STAXJob job,
                                            STAXRequestSettings settings)
    {
        return new STAFResult(STAFResult.Ok, "");
    }

    /*public List createShortMarshalledList(TreeMap variableMap)
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
                    fBreakpointVariableMapClass.createInstance();
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
    }*/

    /*public List createLongMarshalledList(TreeMap variableMap)
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

                marshalledList.add(addVariable(name,
                                             value.getClass().getName(),
                                             value));
            }

            return marshalledList;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return marshalledList;
        }
    }*/

    /*public Map addVariable(String name,
                           String type,
                           Object obj)
    {
        if ((obj instanceof PyList) ||
            (obj instanceof PyTuple))
        {
            if (((PySequence)obj).__len__() == 0)
            {
                Map breakpointVariableMap =
                    fBreakpointVariableMapClass.createInstance();
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
                    fBreakpointVariableMapClass.createInstance();
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
                        fBreakpointVariableMapClass.createInstance();
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
                fBreakpointVariableMapClass.createInstance();
            breakpointVariableMap.put("name", name);
            breakpointVariableMap.put("type", type);
            breakpointVariableMap.put("value", varList);

            return breakpointVariableMap;
        }
        else
        {
            Map breakpointVariableMap =
                fBreakpointVariableMapClass.createInstance();
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
    }*/

    // STAXJobManagement methods

    public void initJob(STAXJob job)
    {
        if (!job.setData("breakpointMap", new TreeMap()))
        {
            String msg = "STAXBreakpointActionFactory.initJob: setData for " +
                         "breakpointMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        }
    }

    public void terminateJob(STAXJob job)
    { /* Do Nothing */ }

    public String getHelpInfo(String lineSep)
    {
        return "ADD       JOB <Job ID> BREAKPOINT" + lineSep +
               "          < FUNCTION <Function Name> |" + lineSep +
               "          LINE <Line Number> FILE <XML File> [MACHINE <Machine Name>] >"
               + lineSep + lineSep +
               "REMOVE    JOB <Job ID> BREAKPOINT <Breakpoint ID>" +
               lineSep + lineSep +
               "RESUME    JOB <Job ID> THREAD <Thread ID>" + lineSep +
               lineSep +
               "STEP      JOB <Job ID> THREAD <Thread ID> [INTO | OVER]";
    }

    private STAFMapClassDefinition fBreakpointInfoMapClass;
    private STAFMapClassDefinition fQueryBreakpointMapClass;
    private STAFCommandParser fAddParser = new STAFCommandParser();
    private STAFCommandParser fRemoveParser = new STAFCommandParser();
    private STAFCommandParser fResumeParser = new STAFCommandParser();
    private STAFCommandParser fStepParser = new STAFCommandParser();
}
