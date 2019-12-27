/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import com.ibm.staf.*;
import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import com.ibm.staf.service.*;

public class STAXTestcaseActionFactory implements STAXActionFactory,
                                                  STAXListRequestHandler,
                                                  STAXQueryRequestHandler,
                                                  STAXGenericRequestHandler,
                                                  STAXJobManagementHandler
{
    static final String STAX_TESTCASE = new String("Testcase");

    static List<String> VALID_STATUS_LIST = new ArrayList<String>();

    static final String sTestcaseInfoMapClassName = new String(
        "STAF/Service/STAX/TestcaseInfo");
    static final String sQueryTestcaseMapClassName = new String(
        "STAF/Service/STAX/QueryTestcase");
    static final String sTestcaseTotalsMapClassName = new String(
        "STAF/Service/STAX/TestcaseTotals");

    private static String fDTDInfo =
"\n" +
"<!--================= The Testcase Element ========================= -->\n" +
"<!--\n" +
"     Defines a testcase.  Used in conjunction with the tcstatus\n" +
"     element to mark the status for a testcase.\n" +
"     The name attribute value is evaluated via Python.\n" +
"-->\n" +
"<!ELEMENT testcase   (%task;)>\n" +
"<!ATTLIST testcase\n" +
"          name       CDATA    #REQUIRED\n" +
"          mode       CDATA    \"'default'\"\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "testcase";
    }

    public STAXTestcaseActionFactory(STAX staxService)
    {
        VALID_STATUS_LIST.add("pass");
        VALID_STATUS_LIST.add("fail");
        VALID_STATUS_LIST.add("info");

        staxService.registerListHandler("TESTCASES", this);
        staxService.registerQueryHandler("TESTCASE", "Test Name", this);

        staxService.registerJobManagementHandler(this);

        // Set up Testcase Status Update parser

        fUpdateParser.addOption("UPDATE", 1,
                                STAFCommandParser.VALUENOTALLOWED);
        fUpdateParser.addOption("JOB", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fUpdateParser.addOption("TESTCASE", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fUpdateParser.addOption("STATUS", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fUpdateParser.addOption("MESSAGE", 1,
                                STAFCommandParser.VALUEREQUIRED);
        fUpdateParser.addOption("FORCE", 1,
                                STAFCommandParser.VALUENOTALLOWED);
        fUpdateParser.addOption("PARENT", 1,
                                STAFCommandParser.VALUEREQUIRED);

        fUpdateParser.addOptionNeed("UPDATE", "JOB");
        fUpdateParser.addOptionNeed("JOB", "TESTCASE");
        fUpdateParser.addOptionNeed("TESTCASE", "STATUS");
        fUpdateParser.addOptionNeed("STATUS", "TESTCASE");
        fUpdateParser.addOptionNeed("MESSAGE", "STATUS");
        fUpdateParser.addOptionNeed("PARENT", "FORCE");

        // Set up Testcase Start parser

        fStartParser.addOption("START", 1,
                               STAFCommandParser.VALUENOTALLOWED);
        fStartParser.addOption("JOB", 1,
                               STAFCommandParser.VALUEREQUIRED);
        fStartParser.addOption("TESTCASE", 1,
                               STAFCommandParser.VALUEREQUIRED);
        fStartParser.addOption("KEY", 1,
                               STAFCommandParser.VALUEREQUIRED);
        fStartParser.addOption("PARENT", 1,
                               STAFCommandParser.VALUEREQUIRED);

        fStartParser.addOptionNeed("START", "JOB");
        fStartParser.addOptionNeed("JOB", "TESTCASE");
        fStartParser.addOptionNeed("KEY", "TESTCASE");
        fStartParser.addOptionNeed("PARENT", "TESTCASE");

        // Set up Testcase Stop parser

        fStopParser.addOption("STOP", 1,
                              STAFCommandParser.VALUENOTALLOWED);
        fStopParser.addOption("JOB", 1,
                              STAFCommandParser.VALUEREQUIRED);
        fStopParser.addOption("TESTCASE", 1,
                              STAFCommandParser.VALUEREQUIRED);
        fStopParser.addOption("KEY", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fStopParser.addOptionNeed("STOP", "JOB");
        fStopParser.addOptionNeed("JOB", "TESTCASE");
        fStopParser.addOptionNeed("KEY", "TESTCASE");
        
        // Construct map class for job results testcase totals

        fTestcaseTotalsMapClass = new STAFMapClassDefinition(
            sTestcaseTotalsMapClassName);
        fTestcaseTotalsMapClass.addKey("numTests", "Tests");
        fTestcaseTotalsMapClass.addKey("numPasses", "Passes");
        fTestcaseTotalsMapClass.addKey("numFails", "Fails");

        // Construct map-class for testcase list information

        fTestcaseInfoMapClass = new STAFMapClassDefinition(
            sTestcaseInfoMapClassName);

        fTestcaseInfoMapClass.addKey("testcaseName", "Testcase Name");
        fTestcaseInfoMapClass.setKeyProperty(
            "testcaseName", "display-short-name", "Name");
        fTestcaseInfoMapClass.addKey("numPasses",    "Passes");
        fTestcaseInfoMapClass.addKey("numFails",     "Fails");
        fTestcaseInfoMapClass.addKey("elapsedTime",  "Elapsed Time");
        fTestcaseInfoMapClass.setKeyProperty(
            "elapsedTime", "display-short-name", "Elapsed");
        fTestcaseInfoMapClass.addKey("numStarts",    "Starts");
        fTestcaseInfoMapClass.addKey("information",  "Information");
        fTestcaseInfoMapClass.setKeyProperty(
            "information", "display-short-name", "Info");

        // Construct map-class for testcase query information

        fQueryTestcaseMapClass = new STAFMapClassDefinition(
            sQueryTestcaseMapClassName);

        fQueryTestcaseMapClass.addKey("testcaseName", "Testcase Name");
        fQueryTestcaseMapClass.addKey("numPasses",    "Passes");
        fQueryTestcaseMapClass.addKey("numFails",     "Fails");
        fQueryTestcaseMapClass.addKey("startedTimestamp",
                                      "Start Date-Time");
        fQueryTestcaseMapClass.addKey("elapsedTime",  "Elapsed Time");
        fQueryTestcaseMapClass.addKey("numStarts",    "Starts");
        fQueryTestcaseMapClass.addKey("lastStatusTimestamp",
                                      "Status Date-Time");
        fQueryTestcaseMapClass.addKey("lastStatus",   "Last Status");
        fQueryTestcaseMapClass.addKey("information",  "Information");
        fQueryTestcaseMapClass.addKey("testcaseStack", "Testcase Stack");

        // Register as a GenericRequestHandler
        try
        {
            // Assign STAFServiceInterfaceLevel class that this handler uses

            Class serviceInterfaceClass = Class.forName(STAX.INTERFACE_LEVEL_30);

            int rc = staxService.registerGenericRequestHandler(this,
                                                      serviceInterfaceClass);
            if (rc != 0)
            {
                System.out.println("STAXTestcaseActionFactory." +
                                   "registerGenericRequestHandler() failed");
            }
        }
        catch (ClassNotFoundException e)
        {
            System.out.println("STAXTestcaseActionFactory: " +
                               "registerGenericRequestHandler: " + e);
        }
    }

    public void initJob(STAXJob job)
    {
        if (!job.setData("testcaseMap", new TreeMap<String, STAXTestcase>()))
        {
            String msg = "STAXTestcaseActionFactory.initJob: setData for " +
                         "testcaseMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        }

        if (!job.setData("activeTestcaseMap",
                         new HashMap<String, STAXActiveTestcase>()))
        {
            String msg = "STAXTestcaseActionFactory.initJob: setData for " +
                         "activeTestcaseMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        }
    }

    public void terminateJob(STAXJob job)
    {
        // Check if any entries are left in the activeTestcaseMap and call
        // the stop testcase method to calculate the elapsed time.

        HashMap activeTestcaseMap = (HashMap)job.getData("activeTestcaseMap");

        TreeMap testcaseMap = (TreeMap)job.getData("testcaseMap");

        if (activeTestcaseMap != null)
        {
            synchronized (activeTestcaseMap)
            {
                Iterator iter = activeTestcaseMap.values().iterator();

                while (iter.hasNext())
                {
                    STAXActiveTestcase activeTest =
                        (STAXActiveTestcase)iter.next();

                    String name = activeTest.getName();
                    STAXTimestamp startTime = activeTest.getStartTime();

                    // Stop the Testcase

                    STAXTestcase theTest = (STAXTestcase)testcaseMap.get(name);

                    if (theTest != null)
                    {
                        theTest.stop(job, startTime);
                    }
                    else
                    {
                        job.log(STAXJob.JOB_LOG, "warning",
                                "STAXTestcaseActionFactory::terminateJob()" +
                                " - Could not stop testcase: " + name +
                                ".  Not in testcase map.");
                    }
                }
            }
        }

        // Log a summary of the testcase status for the job in the STAX Job
        // Log and set the testcase results information for the job to be
        // used in generating the marshalledResults.txt file

        int totalTests = 0;
        int totalPasses = 0;
        int totalFails = 0;
        List<Map<String, Object>> testcaseList =
            new ArrayList<Map<String, Object>>();

        if (testcaseMap != null)
        {
            synchronized (testcaseMap)
            {
                Iterator it = testcaseMap.entrySet().iterator();
                
                while (it.hasNext())
                {
                    Map.Entry entry = (Map.Entry)it.next();
                    String testName = (String)entry.getKey();
                    STAXTestcase test = (STAXTestcase)entry.getValue();

                    int passes = test.getNumPass();
                    int fails  = test.getNumFail();

                    if (passes == 0 && fails == 0 &&
                        test.getMode() == STAXTestcase.DEFAULT_MODE)
                    {
                        // Do not log a status record for the testcase
                    }
                    else
                    {
                        // Log a status record for the testcase

                        String msg = "Testcase: " + testName +
                            ", Pass: " + passes + ", Fail: " + fails;

                        if (job.getLogTCElapsedTime())
                        {
                            msg += ", ElapsedTime: " + test.getElapsedTime();
                        }

                        if (job.getLogTCNumStarts())
                        {
                            msg += ", NumStarts: " + test.getNumStarts();
                        }

                        job.log(STAXJob.JOB_LOG, "status", msg);

                        totalTests += 1;
                        totalPasses += passes;
                        totalFails += fails;
                
                        // Add testcase info to testcase list for job results

                        Map<String, Object> tcMap =
                            new TreeMap<String, Object>();
                        tcMap.put("staf-map-class-name",
                                  fQueryTestcaseMapClass.name());
                        tcMap.put("testcaseName", test.getName());
                        tcMap.put("numPasses", "" + test.getNumPass());
                        tcMap.put("numFails", "" + test.getNumFail());
                        tcMap.put("startedTimestamp", "" +
                            test.getStartedTimestamp().getTimestampString());

                        if (!test.getLastStatus().equals(""))
                            tcMap.put("lastStatus", test.getLastStatus());

                        tcMap.put("information", test.getLastMessage());

                        STAXTimestamp lastStatusTimestamp =
                            test.getLastStatusTimestamp();
                
                        if (lastStatusTimestamp != null)
                        {
                            tcMap.put(
                                "lastStatusTimestamp",
                                lastStatusTimestamp.getTimestampString());
                        }

                        tcMap.put("elapsedTime", test.getElapsedTime());
                        tcMap.put("numStarts", "" + test.getNumStarts());
                        tcMap.put("testcaseStack", test.getTestcaseStack());

                        testcaseList.add(tcMap);
                    }
                }
            }

            // Log testcase totals in the STAX Job Log

            String msg = "Testcase Totals: Tests: " + totalTests +
                         ", Pass: " + totalPasses + ", Fail: " + totalFails;
            job.log(STAXJob.JOB_LOG, "status", msg);

            // Create testcase totals map for job results

            Map<String, String> tcTotalsMap = new TreeMap<String, String>();
            tcTotalsMap.put("staf-map-class-name",
                            fTestcaseTotalsMapClass.name());
            tcTotalsMap.put("numTests", "" + totalTests);
            tcTotalsMap.put("numPasses", "" + totalPasses);
            tcTotalsMap.put("numFails", "" + totalFails);

            // Set the testcase list and testcase totals so that the
            // testcase results are available for the job

            job.setTestcaseList(testcaseList);
            job.setTestcaseTotalsMap(tcTotalsMap);
        }
    }

    public STAFResult handleListRequest(String type, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("testcases"))
        {
            // LIST JOB <Job ID> TESTCASES

            // Create the marshalling context and set its map class definitions
            // and create an empty list to contain the block map entries

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fTestcaseInfoMapClass);
            List<Map<String, Object>> tcOutputList =
                new ArrayList<Map<String, Object>>();

            // Iterate though the testcase map, generating the output list

            TreeMap testcaseMap = (TreeMap)job.getData("testcaseMap");

            if (testcaseMap != null)
            {
                synchronized (testcaseMap)
                {
                    Iterator it = testcaseMap.values().iterator();

                    while (it.hasNext())
                    {
                        STAXTestcase testcase = (STAXTestcase)it.next();
                        int numPass = testcase.getNumPass();
                        int numFail = testcase.getNumFail();
                        String message = testcase.getLastMessage();

                        if (testcase.getMode() == STAXTestcase.DEFAULT_MODE &&
                            numPass == 0 && numFail == 0 && message.equals(""))
                        {
                            // Don't show in list
                        }
                        else
                        {
                            Map<String, Object> tcMap =
                                new TreeMap<String, Object>();
                            tcMap.put("staf-map-class-name",
                                      fTestcaseInfoMapClass.name());
                            tcMap.put("testcaseName", testcase.getName());
                            tcMap.put("numPasses", "" + numPass);
                            tcMap.put("numFails", "" + numFail);
                            tcMap.put("elapsedTime",
                                      testcase.getElapsedTime());
                            tcMap.put("numStarts",
                                      "" + testcase.getNumStarts());
                            tcMap.put("information", message);

                            tcOutputList.add(tcMap);
                        }
                    }
                }
            }

            mc.setRootObject(tcOutputList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryRequest(String type, String key, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("testcase"))
        {
            // QUERY JOB <Job ID> TESTCASE <Testcase Name>

            // Create the marshalling context and set its map class definition

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fQueryTestcaseMapClass);

            TreeMap testcaseMap = (TreeMap)job.getData("testcaseMap");

            if (testcaseMap == null)
                return new STAFResult(STAFResult.DoesNotExist, key);

            synchronized (testcaseMap)
            {
                STAXTestcase testcase = (STAXTestcase)testcaseMap.get(key);
                
                if (testcase == null)
                    return new STAFResult(STAFResult.DoesNotExist, key);
                
                Map<String, Object> tcMap = new TreeMap<String, Object>();
                tcMap.put("staf-map-class-name",
                          fQueryTestcaseMapClass.name());
                tcMap.put("testcaseName", testcase.getName());
                tcMap.put("numPasses", "" + testcase.getNumPass());
                tcMap.put("numFails", "" + testcase.getNumFail());
                tcMap.put("startedTimestamp", "" +
                          testcase.getStartedTimestamp().getTimestampString());

                if (!testcase.getLastStatus().equals(""))
                    tcMap.put("lastStatus", testcase.getLastStatus());
                    
                tcMap.put("information", testcase.getLastMessage());

                STAXTimestamp lastStatusTimestamp =
                    testcase.getLastStatusTimestamp();

                if (lastStatusTimestamp != null)
                {
                    tcMap.put("lastStatusTimestamp",
                              lastStatusTimestamp.getTimestampString());
                }

                tcMap.put("elapsedTime", testcase.getElapsedTime());
                tcMap.put("numStarts", "" + testcase.getNumStarts());
                tcMap.put("testcaseStack", testcase.getTestcaseStack());
               
                mc.setRootObject(tcMap);

                return new STAFResult(STAFResult.Ok, mc.marshall());
            }
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryJobRequest(STAXJob job,
                                            STAXRequestSettings settings)
    {
        return new STAFResult(STAFResult.Ok, "");
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXTestcaseAction action = new STAXTestcaseAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        STAXAction testcaseAction = null;

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("name"))
            {
                action.setName(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
            else if (thisAttr.getNodeName().equals("mode"))
            {
                action.setMode(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
        }

        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                if (testcaseAction != null)
                {
                    action.setElementInfo(
                        new STAXElementInfo(
                            root.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementCountException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                STAXActionFactory factory = staxService.getActionFactory(
                    thisChild.getNodeName());

                if (factory == null)
                {
                    action.setElementInfo(new STAXElementInfo(
                        root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        "No action factory for element type \"" +
                        thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                testcaseAction = factory.parseAction(
                    staxService, job, thisChild);
            }
            else
            {
                action.setElementInfo(new STAXElementInfo(
                    root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);
            }
        }

        action.setTestcaseAction(testcaseAction);

        return action;
    }

    // STAXGenericRequestHandler Interface Methods

    public STAFResult handleRequest(Object infoObject, STAX staxService)
    {
        STAFServiceInterfaceLevel30.RequestInfo info =
            (STAFServiceInterfaceLevel30.RequestInfo)infoObject;

        String lowerRequest = info.request.toLowerCase();

        if (lowerRequest.startsWith("update"))
            return handleUpdateRequest(info, staxService);
        else if (lowerRequest.startsWith("start"))
            return handleStartRequest(info, staxService);
        else if (lowerRequest.startsWith("stop"))
            return handleStopRequest(info, staxService);
        else
        {   // Returning nothing in the result indicates that this parser
            // does not support this request.
            return new STAFResult(STAFResult.InvalidRequestString, "");
        }
    }

    private STAFResult handleUpdateRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, staxService.getServiceName(), "UPDATE", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fUpdateParser.parse(info.request);

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

        // Resolve the value specified for Testcase name

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("TESTCASE"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String testcaseName = res.result;

        // Resolve the value specified for the Status

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("STATUS"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String status = res.result;

        // Make sure that the status is valid

        if (!VALID_STATUS_LIST.contains(status.toLowerCase()))
        {
            return new STAFResult(STAFResult.InvalidValue, status);
        }

        String message = new String("");

        if (parseResult.optionTimes("MESSAGE") != 0)
        {
            // Resolve the value specified for Message

            res = STAFUtil.resolveRequestVar(
                parseResult.optionValue("MESSAGE"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (res.rc != 0) return res;

            message = res.result;
        }

        boolean force = false;

        if (parseResult.optionTimes("FORCE") != 0)
        {
            force = true;
        }

        String parentName = null;

        if (parseResult.optionTimes("PARENT") > 0)
        {
            // Resolve the value specified for the parent if needed

            res = STAFUtil.resolveRequestVar(
                parseResult.optionValue("PARENT"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (res.rc != 0) return res;

            parentName = res.result;

            // Verify that the parent testcase name is a valid
            // parent for this testcase

            if (!testcaseName.startsWith(parentName + ".") ||
                testcaseName.equals(parentName + "."))
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid parent testcase name.  Testcase '" +
                    parentName + "' is not a parent of testcase '" +
                    testcaseName + "'.");
            }
        }

        // Check if the testcase exists.  If not, and force is not specified,
        // return an error message. If not, and force is specified, add it
        // to the testcaseMap.

        @SuppressWarnings("unchecked")
        TreeMap<String, STAXTestcase> testcaseMap =
            (TreeMap<String, STAXTestcase>)job.getData("testcaseMap");

        if (testcaseMap == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Testcase " + testcaseName + " does not exist." +
                "  The job's testcaseMap is null, possibly because the job " +
                "is terminating.");
        }

        STAXTestcase theTest;

        synchronized (testcaseMap)
        {
            theTest = testcaseMap.get(testcaseName);

            if (theTest == null)
            {
                if (! force)
                {
                    return new STAFResult(
                        STAFResult.DoesNotExist,
                        "Testcase " + testcaseName + " does not exist.");
                }
                else
                {
                    // Create a new testcase and add it to the testcaseMap

                    if (parentName == null)
                    {
                        theTest = new STAXTestcase(
                            testcaseName, STAXTestcase.STRICT_MODE);
                    }
                    else
                    {
                        // Verify that the parent testcase specified exists in
                        // the testcaseMap and get its testcase stack.  Add
                        // the testcase name to the stack and pass it to the
                        // STAXTestcase constructor.

                        STAXTestcase parentTest = testcaseMap.get(parentName);

                        if (parentTest == null)
                        {
                            return new STAFResult(
                                STAFResult.DoesNotExist,
                                "Parent testcase '" + parentName +
                                "' does not exist.");
                        }

                        List<String> testcaseStack =
                            parentTest.getTestcaseStack();

                        testcaseStack.add(testcaseName);

                        theTest = new STAXTestcase(
                            testcaseName, STAXTestcase.STRICT_MODE,
                            testcaseStack);
                    }

                    testcaseMap.put(testcaseName, theTest);
                }
            }
        }

        if (!theTest.isActive())
        {
            // Check if the testcase name is in the activeTestcaseMap.
            // If not, add it to the activeTestcaseMap, storing its start
            // timestamp, and start the testcase.

            @SuppressWarnings("unchecked")
            HashMap<String, STAXActiveTestcase> activeTestcaseMap =
                (HashMap<String, STAXActiveTestcase>)job.getData(
                    "activeTestcaseMap");

            if (activeTestcaseMap != null)
            {
                synchronized (activeTestcaseMap)
                {
                    if (!activeTestcaseMap.containsKey(testcaseName))
                    {
                        STAXActiveTestcase activeTest = new STAXActiveTestcase(
                            testcaseName, new STAXTimestamp());

                        activeTestcaseMap.put(testcaseName, activeTest);

                        theTest.start(job);
                    }
                }
            }
        }

        // Update Testcase status

        theTest.updateStatus(status, message, job);

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult handleStartRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, staxService.getServiceName(), "START",
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the result

        STAFCommandParseResult parseResult= fStartParser.parse(info.request);

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

        // Resolve the value specified for Testcase name

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("TESTCASE"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String testcaseName = res.result;

        String key = new String("");

        if (parseResult.optionTimes("KEY") > 0)
        {
            // Resolve the value specified for the key if needed

            res = STAFUtil.resolveRequestVar(
                parseResult.optionValue("KEY"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (res.rc != 0) return res;

            key = res.result;
        }

        String parentName = null;

        if (parseResult.optionTimes("PARENT") > 0)
        {
            // Resolve the value specified for the parent if needed

            res = STAFUtil.resolveRequestVar(
                parseResult.optionValue("PARENT"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (res.rc != 0) return res;

            parentName = res.result;

            // Verify that the parent testcase name is a valid
            // parent for this testcase

            if (!testcaseName.startsWith(parentName + ".") ||
                testcaseName.equals(parentName + "."))
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid parent testcase name.  Testcase '" +
                    parentName + "' is not a parent of testcase '" +
                    testcaseName + "'.");
            }
        }

        // Check if the testcase[:key] is in the activeTestcaseMap.  If not,
        // add it to the activeTestcaseMap, storing its start timestamp.

        String testcaseKey = testcaseName;

        if (!key.equals(""))
        {
            testcaseKey = testcaseName + ":" + key;
        }

        @SuppressWarnings("unchecked")
        HashMap<String, STAXActiveTestcase> activeTestcaseMap =
            (HashMap<String, STAXActiveTestcase>)job.getData(
                "activeTestcaseMap");

        if (activeTestcaseMap == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Job " + jobID + " is not currently running " +
                "(activeTestcaseMap is null, possibly because the job is " +
                "terminating)");
        }

        synchronized (activeTestcaseMap)
        {
            if (activeTestcaseMap.containsKey(testcaseKey))
            {
                if (key.equals(""))
                {
                    return new STAFResult(
                        STAFResult.AlreadyExists,
                        "Testcase " + testcaseKey + " already exists");
                }
                else
                {
                    return new STAFResult(
                        STAFResult.AlreadyExists,
                        "Testcase/key " + testcaseKey + " already exists");
                }
            }

            STAXActiveTestcase activeTest =
                new STAXActiveTestcase(testcaseName, new STAXTimestamp());

            activeTestcaseMap.put(testcaseKey, activeTest);
        }

        // Check if the testcase name is already in the testcaseMap.
        // If not, add it to the testcaseMap.  If the testcase already
        // exists, set its mode to strict if its not already strict.

        @SuppressWarnings("unchecked")
        TreeMap<String, STAXTestcase> testcaseMap =
            (TreeMap<String, STAXTestcase>)job.getData("testcaseMap");

        if (testcaseMap == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Job " + jobID + " is not currently running " +
                "(testcaseMap is null, possibly because the job is " +
                "terminating)");
        }

        STAXTestcase theTest;

        synchronized (testcaseMap)
        {
            theTest = testcaseMap.get(testcaseName);

            if (theTest == null)
            {
                // Create a new testcase and add it to the testcaseMap

                if (parentName == null)
                {
                    theTest = new STAXTestcase(
                        testcaseName, STAXTestcase.STRICT_MODE);
                }
                else
                {
                    // Verify that the parent testcase specified exists in
                    // the testcaseMap and get its testcase stack.  Add the
                    // testcase name to the stack and pass it to the
                    // STAXTestcase constructor.
                    
                    STAXTestcase parentTest = testcaseMap.get(parentName);

                    if (parentTest == null)
                    {
                        activeTestcaseMap.remove(testcaseKey);

                        return new STAFResult(
                            STAFResult.DoesNotExist,
                            "Parent testcase '" + parentName +
                            "' does not exist.");
                    }

                    List<String> testcaseStack = parentTest.getTestcaseStack();

                    testcaseStack.add(testcaseName);

                    theTest = new STAXTestcase(
                        testcaseName, STAXTestcase.STRICT_MODE,
                        testcaseStack);
                }
                
                testcaseMap.put(testcaseName, theTest);
            }
            else if (theTest.getMode() != STAXTestcase.STRICT_MODE)
            {
                theTest.setMode(STAXTestcase.STRICT_MODE);
            }
        }

        // Start the testcase
        theTest.start(job);

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult handleStopRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, staxService.getServiceName(), "STOP",
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fStopParser.parse(info.request);

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

        // Resolve the value specified for Testcase name

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("TESTCASE"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String testcaseName = res.result;

        String key = new String("");

        if (parseResult.optionTimes("KEY") > 0)
        {
            // Resolve the value specified for the key if needed

            res = STAFUtil.resolveRequestVar(
                parseResult.optionValue("KEY"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (res.rc != 0) return res;

            key = res.result;
        }

        // Check if the testcase:[key] is in the activeTestcaseMap.  If so,
        // get its start timestamp and remove it from the activeTestcaseMap.
        // If not, return a DoesNotExist error.

        String testcaseKey = testcaseName;

        if (!key.equals(""))
        {
            testcaseKey = testcaseName + ":" + key;
        }

        STAXTimestamp startTimestamp;

        HashMap activeTestcaseMap = (HashMap)job.getData("activeTestcaseMap");

        if (activeTestcaseMap == null)
            return new STAFResult(STAFResult.DoesNotExist, testcaseKey);

        synchronized (activeTestcaseMap)
        {
            STAXActiveTestcase activeTest =
                (STAXActiveTestcase)activeTestcaseMap.get(testcaseKey);

            if (activeTest == null)
            {
                return new STAFResult(STAFResult.DoesNotExist, testcaseKey);
            }

            startTimestamp = activeTest.getStartTime();

            activeTestcaseMap.remove(testcaseKey);
        }

        // Stop the Testcase

        STAXTestcase theTest = null;
        TreeMap testcaseMap = (TreeMap)job.getData("testcaseMap");

        if (testcaseMap != null)
        {
            synchronized (testcaseMap)
            {
                theTest = (STAXTestcase)testcaseMap.get(testcaseName);
            }
        }

        if (theTest == null)
        {
            return new STAFResult(STAFResult.DoesNotExist, testcaseName);

        }

        theTest.stop(job, startTimestamp);

        return new STAFResult(STAFResult.Ok);
    }

    public String getHelpInfo(String lineSep)
    {
        return "START     JOB <Job ID> TESTCASE <Testcase name> [KEY <Key>]" +
               lineSep +
               "          [PARENT <Testcase name>]" +
               lineSep + lineSep +
               "STOP      JOB <Job ID> TESTCASE <Testcase name> [KEY <Key>]" +
               lineSep + lineSep +
               "UPDATE    JOB <Job ID> TESTCASE <Testcase name> " +
               "STATUS <Status>" + lineSep +
               "          [MESSAGE <Message text>] " +
               "[FORCE [PARENT <Testcase name>]]";
    }


    private STAFCommandParser fUpdateParser = new STAFCommandParser();
    private STAFCommandParser fStartParser  = new STAFCommandParser();
    private STAFCommandParser fStopParser   = new STAFCommandParser();
    private STAFMapClassDefinition fTestcaseInfoMapClass;
    static protected STAFMapClassDefinition fQueryTestcaseMapClass;
    static protected STAFMapClassDefinition fTestcaseTotalsMapClass;


    // STAXActiveTestcase class is the value stored in the activeTestcaseMap

    class STAXActiveTestcase
    {
        STAXActiveTestcase(String name, STAXTimestamp startTime)
        {
            fName = name;
            fStartTime = startTime;
        }

        String getName() { return fName; }
        STAXTimestamp getStartTime() { return fStartTime; }

        private String fName;
        private STAXTimestamp fStartTime;

    } // end class STAXActiveTestcase
}
