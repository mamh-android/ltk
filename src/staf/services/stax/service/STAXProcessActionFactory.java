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
import com.ibm.staf.service.*;
import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;

public class STAXProcessActionFactory implements STAXActionFactory,
                                                 STAXSTAFQueueListener,
                                                 STAXListRequestHandler,
                                                 STAXQueryRequestHandler,
                                                 STAXGenericRequestHandler,
                                                 STAXJobManagementHandler
{
    static boolean debug = false;

    static final String STAX_PROCESS_EVENT = new String("Process");

    static final String sProcessInfoMapClassName = new String(
        "STAF/Service/STAX/ProcessInfo");
    static final String sQueryProcessMapClassName = new String(
        "STAF/Service/STAX/QueryProcess");

    // Constructor
 
    public STAXProcessActionFactory(STAX staxService)
    {
        staxService.registerListHandler("PROCESSES", this);
        staxService.registerQueryHandler("PROCESS", "Location:Handle", this);
        staxService.registerJobManagementHandler(this);

        // Set up parser that supports stopping a process or a testcase to
        // determine if it is a STOP PROCESS or a STOP TESTCASE request

        fStopGenericParser.addOption("STOP", 1,
                                     STAFCommandParser.VALUENOTALLOWED);
        fStopGenericParser.addOption("JOB", 1,
                                     STAFCommandParser.VALUEREQUIRED);
        fStopGenericParser.addOption("PROCESS", 1,
                                     STAFCommandParser.VALUEREQUIRED);
        fStopGenericParser.addOption("TESTCASE", 1,
                                     STAFCommandParser.VALUEREQUIRED);
        fStopGenericParser.addOption("KEY", 1,
                                     STAFCommandParser.VALUEREQUIRED);

        fStopGenericParser.addOptionNeed("STOP", "JOB");
        fStopGenericParser.addOptionNeed("JOB", "PROCESS TESTCASE");
        fStopGenericParser.addOptionNeed("KEY", "TESTCASE");

        fStopGenericParser.addOptionGroup("PROCESS TESTCASE", 0, 1);

        // Set up Process Stop parser

        fStopParser.addOption("STOP", 1,
                              STAFCommandParser.VALUENOTALLOWED);
        fStopParser.addOption("JOB", 1,
                              STAFCommandParser.VALUEREQUIRED);
        fStopParser.addOption("PROCESS", 1,
                              STAFCommandParser.VALUEREQUIRED);

        fStopParser.addOptionNeed("STOP", "JOB");
        fStopParser.addOptionNeed("JOB", "PROCESS");

        // Construct map-class for list process information

        fProcessInfoMapClass = new STAFMapClassDefinition(
            sProcessInfoMapClassName);

        fProcessInfoMapClass.addKey("processName", "Process Name");
        fProcessInfoMapClass.addKey("location",    "Location");
        fProcessInfoMapClass.addKey("handle",      "Handle");
        fProcessInfoMapClass.addKey("command",     "Command");
        fProcessInfoMapClass.addKey("parms",       "Parms");

        // Construct map-class for query process information

        fQueryProcessMapClass = new STAFMapClassDefinition(
            sQueryProcessMapClassName);

        fQueryProcessMapClass.addKey("processName",  "Process Name");
        fQueryProcessMapClass.addKey("location",     "Location");
        fQueryProcessMapClass.addKey("handle",       "Handle");
        fQueryProcessMapClass.addKey("blockName",    "Block Name");
        fQueryProcessMapClass.addKey("threadID",     "Thread ID");
        fQueryProcessMapClass.addKey("startTimestamp", "Start Date-Time");
        fQueryProcessMapClass.addKey("command",      "Command");
        fQueryProcessMapClass.addKey("commandMode",  "Command Mode");
        fQueryProcessMapClass.addKey("commandShell", "Command Shell");
        fQueryProcessMapClass.addKey("parms",        "Parms");
        fQueryProcessMapClass.addKey("title",        "Title");
        fQueryProcessMapClass.addKey("workdir",      "Workdir");
        fQueryProcessMapClass.addKey("workload",     "Workload");
        fQueryProcessMapClass.addKey("varList",      "Vars");
        fQueryProcessMapClass.addKey("envList",      "Envs");
        fQueryProcessMapClass.addKey("useProcessVars", "Use Process Vars");
        fQueryProcessMapClass.addKey("userName",     "User Name");
        fQueryProcessMapClass.addKey("password",     "Password");
        fQueryProcessMapClass.addKey("disabledAuth", "Disabled Auth");
        fQueryProcessMapClass.addKey("stdin",        "Stdin");
        fQueryProcessMapClass.addKey("stdoutMode",   "Stdout Mode");
        fQueryProcessMapClass.addKey("stdoutFile",   "Stdout File");
        fQueryProcessMapClass.addKey("stderrMode",   "Stderr Mode");
        fQueryProcessMapClass.addKey("stderrFile",   "Stderr File");
        fQueryProcessMapClass.addKey("returnStdout", "Return Stdout");
        fQueryProcessMapClass.addKey("returnStderr", "Return Stderr");
        fQueryProcessMapClass.addKey("returnFileList", "Returned Files");
        fQueryProcessMapClass.addKey("stopUsing",    "Stop Using");
        fQueryProcessMapClass.addKey("console",      "Console");
        fQueryProcessMapClass.addKey("focus",        "Focus");
        fQueryProcessMapClass.addKey("staticHandleName", "Static Handle Name");
        fQueryProcessMapClass.addKey("other",        "Other");
        
        // Register as a GenericRequestHandler
        try
        {
            // Assign STAFServiceInterfaceLevel class that this handler uses

            Class serviceInterfaceClass = Class.forName(STAX.INTERFACE_LEVEL_30);

            int rc = staxService.registerGenericRequestHandler(
                this, serviceInterfaceClass);

            if (rc != 0)
            {
                System.out.println("STAXProcessActionFactory." +
                                   "registerGenericRequestHandler() failed");
            }
        }
        catch (ClassNotFoundException e)
        {
            System.out.println("STAXProcessActionFactory: " +
                               "registerGenericRequestHandler: " + e);
        }
    }

    public String getName() { return fName; }

    // STAXActionFactory methods

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "process";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXProcessAction process = new STAXProcessAction();

        process.setActionFactory(this);
        process.setLineNumber(root);
        process.setXmlFile(job.getXmlFile());
        process.setXmlMachine(job.getXmlMachine());
        
        NamedNodeMap rootAttrs = root.getAttributes();

        for (int i = 0; i < rootAttrs.getLength(); ++i)
        {
            Node thisAttr = rootAttrs.item(i);
            
            process.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("name"))
            {
                process.setName(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), process));
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
                if (thisChild.getNodeName().equals("envs"))
                    process.setLineNumber(thisChild, "env");
                else if (thisChild.getNodeName().equals("vars"))
                    process.setLineNumber(thisChild, "var");
                else if (thisChild.getNodeName().equals("returnfiles"))
                    process.setLineNumber(thisChild, "returnfile");
                else
                    process.setLineNumber(thisChild);

                if (thisChild.getNodeName().equals("location"))
                {
                    process.setLocation(handleChild(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("command"))
                {
                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);
                        
                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("mode"))
                        {
                            String mode = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), process);
                                                        
                            process.setCommandMode(mode);
                        }
                        else if (thisAttr.getNodeName().equals("shell"))
                        {
                            String shell = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), process);
                                                        
                            process.setCommandShell(shell);
                        }
                    }

                    process.setCommand(handleChild(thisChild, process));
                } 
                else if (thisChild.getNodeName().equals("workload"))
                {
                    process.setWorkload(
                        handleChild(thisChild, process), 
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("title"))
                {
                    process.setTitle(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("parms"))
                {
                    process.setParms(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("workdir"))                
                {
                    process.setWorkdir(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                } 
                else if (thisChild.getNodeName().equals("vars"))                
                {
                    process.setVars(
                        handleChild(thisChild, process, "var"),
                        handleIfAttribute(thisChild, process, "var"));
                }
                else if (thisChild.getNodeName().equals("var"))                
                {
                    process.setVars(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("envs"))                
                {
                    process.setEnvs(
                        handleChild(thisChild, process, "env"),
                        handleIfAttribute(thisChild, process, "env"));
                } 
                else if (thisChild.getNodeName().equals("env"))                
                {
                    process.setEnvs(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                } 
                else if (thisChild.getNodeName().equals("useprocessvars"))
                {
                    process.setUseprocessvars(
                        "useprocessvars",
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("stopusing"))
                {
                    process.setStopusing(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                } 
                else if (thisChild.getNodeName().equals("console"))
                { 
                    String ifValue = new String();
                    String useValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                      thisAttr.getNodeValue(), process);
                        else if (thisAttr.getNodeName().equals("use"))
                            useValue = STAXUtil.parseAndCompileForPython(
                                       thisAttr.getNodeValue(), process);
                    }

                    process.setConsole(useValue, ifValue);
                }
                else if (thisChild.getNodeName().equals("focus"))
                { 
                    String ifValue = new String();
                    String modeValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                      thisAttr.getNodeValue(), process);
                        else if (thisAttr.getNodeName().equals("mode"))
                            modeValue = STAXUtil.parseAndCompileForPython(
                                        thisAttr.getNodeValue(), process);
                    }

                    process.setFocus(modeValue, ifValue);
                }
                else if (thisChild.getNodeName().equals("username"))
                {
                    process.setUsername(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("password"))
                {
                    process.setPassword(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("disabledauth"))
                {
                    String ifValue = new String();
                    String actionValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                      thisAttr.getNodeValue(), process);
                        else if (thisAttr.getNodeName().equals("action"))
                            actionValue = STAXUtil.parseAndCompileForPython(
                                          thisAttr.getNodeValue(), process);
                    }

                    process.setDisabledauth(actionValue, ifValue);
                }
                else if (thisChild.getNodeName().equals("statichandlename"))
                {
                    process.setStatichandlename(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("stdin"))
                {
                    process.setStdin(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("stdout"))
                {
                    String ifValue = new String();
                    String modeValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                      thisAttr.getNodeValue(), process);
                        else if (thisAttr.getNodeName().equals("mode"))
                            modeValue = STAXUtil.parseAndCompileForPython(
                                        thisAttr.getNodeValue(), process);
                    }

                    process.setStdout(handleChild(thisChild, process),
                                      modeValue, ifValue);
                }
                else if (thisChild.getNodeName().equals("stderr"))
                {
                    String ifValue = new String();
                    String modeValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                      thisAttr.getNodeValue(), process);
                        else if (thisAttr.getNodeName().equals("mode"))
                            modeValue = STAXUtil.parseAndCompileForPython(
                                        thisAttr.getNodeValue(), process);
                    }

                    process.setStderr(handleChild(thisChild, process),
                                      modeValue, ifValue);
                }
                else if (thisChild.getNodeName().equals("returnstdout"))
                {
                    process.setReturnStdout(
                        "returnstdout",
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("returnstderr"))
                {
                    process.setReturnStderr(
                        "returnstderr",
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("returnfile"))                
                {
                    process.setReturnFiles(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("returnfiles"))                
                {
                    process.setReturnFiles(
                        handleChild(thisChild, process, "returnfile"),
                        handleIfAttribute(thisChild, process, "returnfile"));
                }
                else if (thisChild.getNodeName().equals("other"))
                {
                    process.setOther(
                        handleChild(thisChild, process),
                        handleIfAttribute(thisChild, process));
                }
                else if (thisChild.getNodeName().equals("process-action"))
                {
                    String ifValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);
                        
                        process.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                      thisAttr.getNodeValue(), process);                        
                    }                

                    NodeList processActionChildren = thisChild.getChildNodes();

                    STAXAction processAction = null;

                    for (int j = 0; j < processActionChildren.getLength(); ++j)
                    {
                        Node processActionChild = 
                            processActionChildren.item(j);

                        if (processActionChild.getNodeType() == 
                            Node.COMMENT_NODE)
                        {
                            /* Do nothing */
                        }
                        else if (processActionChild.getNodeType() == 
                            Node.ELEMENT_NODE)
                        {
                            if (processAction != null)
                            {
                                process.setElementInfo(
                                    new STAXElementInfo(
                                        thisChild.getNodeName(),
                                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                                        STAXElementInfo.LAST_ELEMENT_INDEX,
                                        "Invalid element type \"" +
                                        processActionChild.getNodeName() +
                                        "\""));

                                throw new STAXInvalidXMLElementCountException(
                                    STAXUtil.formatErrorMessage(process),
                                    process);
                            }
                        
                            STAXActionFactory factory = 
                                staxService.getActionFactory(
                                processActionChild.getNodeName());

                            if (factory == null)
                            {
                                process.setElementInfo(new STAXElementInfo(
                                    thisChild.getNodeName(),
                                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                                    STAXElementInfo.LAST_ELEMENT_INDEX,
                                    "No action factory for element type \"" +
                                    processActionChild.getNodeName() + "\""));

                                throw new STAXInvalidXMLElementException(
                                    STAXUtil.formatErrorMessage(process),
                                    process);
                            }

                            processAction = factory.parseAction(staxService, 
                                job, processActionChild);
                        }
                    }
                        
                    process.setProcessAction(processAction, ifValue);
                }
            }
            else
            {
                process.setElementInfo(new STAXElementInfo(
                    root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(process), process);
            }
        }

        return process;
    }

    private String handleIfAttribute(Node thisChild, STAXProcessAction action)
                                     throws STAXException
    {
        return handleIfAttribute(thisChild,action, thisChild.getNodeName());
    }

    private String handleIfAttribute(Node thisChild, STAXProcessAction action,
                                     String elementName)
                                     throws STAXException
    {
        String ifValue = new String();

        NamedNodeMap attrs = thisChild.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                elementName, thisAttr.getNodeName(),
                STAXElementInfo.LAST_ELEMENT_INDEX));

            if (thisAttr.getNodeName().equals("if"))
                ifValue = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
        }

        return ifValue;
    }  

    private String handleChild(Node root, STAXProcessAction action)
                               throws STAXException
    {
        return handleChild(root, action, root.getNodeName());
    }

    private String handleChild(Node root, STAXProcessAction action,
                               String elementName)
                               throws STAXException
    {
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            // XXX: Should I be able to have a COMMENT_NODE here?

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.TEXT_NODE)
            {
                action.setElementInfo(new STAXElementInfo(
                    elementName, STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX));

                return STAXUtil.parseAndCompileForPython(
                    thisChild.getNodeValue(), action);
            }
            else if (thisChild.getNodeType() == Node.CDATA_SECTION_NODE)
            {
                /* Do nothing */
            }
            else
            {
                action.setElementInfo(
                    new STAXElementInfo(
                        elementName, STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);
            }
        }

        return new String();
    }


    // STAXSTAFQueueListener methods

    public void handleQueueMessage(STAXSTAFMessage message, STAXJob job)
    {
        // XXX: How to make sure that the machine they registered
        //      with is the machine on the queue (long vs. short
        //      names)?  Maybe try
        //          a.startsWith(b) || b.startsWith(a)

        String machine = message.getMachine();
        int handle = message.getProcessHandle();
        
        String key = "";
        
        if (!message.getProcessKey().equals(""))
        {
            key = message.getProcessKey();
        }
        else
        {
            // XXX Does lower casing the machine name cause any other problems?
            key = (machine + "/" + handle).toLowerCase();
        }

        @SuppressWarnings("unchecked")
        HashMap<String, Object> processMap =
            (HashMap<String, Object>)job.getData("processMap");

        if (processMap == null)
        {
            // Could occur if job is being terminated
            return;
        }

        synchronized (processMap)
        {
            STAXProcessCompleteListener listener = 
                (STAXProcessCompleteListener)processMap.get(key);

            if (listener != null)
            {
                if (debug)
                {
                    System.out.println(
                        "ProcessActionFactory::handleQueueMessages() - " +
                        "Remove from processMap");
                }

                listener.processComplete(
                    machine, handle, message.getProcessRC(),
                    message.getProcessResultAsList(),
                    message.getProcessTimestamp());

                processMap.remove(key);
            }
            else
            {
                // Log a warning message in the job log

                if (debug)
                {
                    String msg = "STAXProcessActionFactory.handle" +
                        "QueueMessage: No listener found for message:\n" +
                        STAFMarshallingContext.unmarshall(
                            message.getResult()).toString();

                    msg += "\nCurrentKeys=";
                
                    Iterator iter = processMap.keySet().iterator();
                
                    while (iter.hasNext())
                    {
                        msg += iter.next() + ";";
                    }
                
                    job.log(STAXJob.JOB_LOG, "warning", msg);
                
                    System.out.println(
                        "ProcessActionFactory::handleQueueMessages() - " +
                        "Add to processMap");
                }

                // If we received the "process end" message before we placed
                // the machine/handle in the process map, just save the key
                // and the message in the process map.  When STAXProcessAction
                // receives the requestComplete message, since the key already
                // exists, it will just remove it.

                processMap.put(key.toLowerCase(), message);
            }
        }        
    }


    // STAXJobManagement methods

    public void initJob(STAXJob job)
    {
        boolean result = job.setData("processMap", new HashMap<String, Object>());

        if (!result)
        {
            String msg = "STAXProcessActionFactory.initJob: setData for " +
                         "processMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        } 

        result = job.setData("processRequestMap",
                             new TreeMap<String, STAXProcessAction>()); 

        if (!result)
        {
            String msg = "STAXProcessActionFactory.initJob: setData for " +
                         "processRequestMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        } 

        job.registerSTAFQueueListener("STAF/Process/End", this);
    }

    public void terminateJob(STAXJob job)
    { /* Do Nothing */ }


    // STAXListRequestHandler method

    public STAFResult handleListRequest(String type, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("processes"))
        {
            // LIST JOB <Job ID> PROCESSES

            // Create the marshalling context and set its map class definitions
            // and create an empty list to contain the block map entries

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fProcessInfoMapClass);
            List<Map<String, Object>> processOutputList =
                new ArrayList<Map<String, Object>>();

            // Iterate through the process map, generating the output list

            TreeMap processes = (TreeMap)job.getData("processRequestMap");

            if (processes != null)
            {
                synchronized (processes)
                {
                    Iterator iter = processes.values().iterator();

                    while (iter.hasNext())
                    {
                        STAXProcessAction process =
                            (STAXProcessAction)iter.next();
                        
                        Map<String, Object> processMap =
                            new TreeMap<String, Object>();
                        processMap.put("staf-map-class-name",
                                       fProcessInfoMapClass.name());
                        processMap.put("processName", process.getName());
                        processMap.put("location", process.getLocation());
                        processMap.put(
                            "handle", "" + process.getProcessHandle());
                        processMap.put(
                            "command", STAFUtil.maskPrivateData(
                                process.getCommand()));
                        processMap.put(
                            "parms", STAFUtil.maskPrivateData(
                                process.getParms()));
                        
                        processOutputList.add(processMap);
                    }
                }
            }

            mc.setRootObject(processOutputList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    // STAXQueryRequestHandler methods

    public STAFResult handleQueryRequest(String type, String key, STAXJob job,
                                         STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("process"))
        {
            // QUERY JOB <Job ID> PROCESS <Location:Handle>

            // Create the marshalling context and set its map class definition

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fQueryProcessMapClass);

            key = key.toLowerCase();

            TreeMap processes = (TreeMap)job.getData("processRequestMap");

            if (processes == null)
                return new STAFResult(STAFResult.DoesNotExist, key);

            STAXProcessAction process = null;

            synchronized (processes)
            {
                process = (STAXProcessAction)processes.get(key);
            }

            if (process == null)
                return new STAFResult(STAFResult.DoesNotExist, key);

            Map<String, Object> processMap = new TreeMap<String, Object>();
            processMap.put("staf-map-class-name",
                           fQueryProcessMapClass.name());
            processMap.put("processName", process.getName());
            processMap.put("location", process.getLocation());
            processMap.put("handle", "" + process.getProcessHandle());
            processMap.put("blockName", process.getCurrentBlockName());
            processMap.put("threadID",
                           "" + process.getThread().getThreadNumber());
            processMap.put("startTimestamp", process.getStartTimestamp().
                           getTimestampString());
            processMap.put("command",
                           STAFUtil.maskPrivateData(process.getCommand()));

            // The rest of the process elements are optional.

            if (!process.getCommandMode().equals(""))
                processMap.put("commandMode", process.getCommandMode());

            if (!process.getCommandShell().equals(""))
                processMap.put("commandShell", process.getCommandShell());
            
            if (!process.getParms().equals(""))
                processMap.put("parms",
                               STAFUtil.maskPrivateData(process.getParms()));

            if (!process.getTitle().equals(""))
                processMap.put("title", process.getTitle());

            if (!process.getWorkdir().equals(""))
                processMap.put("workdir", process.getWorkdir());

            if (!process.getWorkload().equals(""))
                processMap.put("workload", process.getWorkload());

            List<String> varList = new ArrayList<String>();
            Vector<String> vars = process.getVars();

            for (int i = 0; i < vars.size(); i++)
            {
                varList.add(vars.elementAt(i));
            }

            processMap.put("varList", varList);

            List<String> envList = new ArrayList<String>();
            Vector<String> envs = process.getEnvs();

            for (int i = 0; i < envs.size(); i++)
            {
                envList.add(envs.elementAt(i));
            }

            processMap.put("envList", envList);

            if (!process.getUseprocessvars().equals(""))
                processMap.put("useProcessVars", "true");

            if (!process.getUsername().equals(""))
                processMap.put("userName", process.getUsername()); 
            
            // Mask the true value for the password 
            if (!process.getPassword().equals(""))
                processMap.put("password", "*******");

            if (!process.getDisabledauth().equals(""))
                processMap.put("disabledAuth", process.getDisabledauth());

            if (!process.getStdin().equals(""))
                processMap.put("stdin", process.getStdin()); 
            
            if (!process.getStdout().equals(""))
            {
                processMap.put("stdoutMode", process.getStdoutMode());
                processMap.put("stdoutFile", process.getStdout());
            }

            if (!process.getStderr().equals(""))
                processMap.put("stderrFile", process.getStderr());

            if (!process.getStderrMode().equals(""))
                processMap.put("stderrMode", process.getStderrMode());

            if (!process.getReturnStdout().equals(""))
                processMap.put("returnStdout", "true");

            if (!process.getReturnStderr().equals(""))
                processMap.put("returnStderr", "true");

            List<String> returnFileList = new ArrayList<String>();
            Vector<String> returnfiles = process.getReturnFiles();

            for (int i = 0; i < returnfiles.size(); i++)
            {
                returnFileList.add(returnfiles.elementAt(i));
            }

            processMap.put("returnFileList", returnFileList);

            if (!process.getStopusing().equals(""))
                processMap.put("stopUsing", process.getStopusing());

            if (!process.getConsole().equals(""))
                processMap.put("console", process.getConsole()); 
            
            if (!process.getFocus().equals(""))
                processMap.put("focus", process.getFocus());

            if (!process.getStatichandlename().equals(""))
                processMap.put("staticHandleName",
                               process.getStatichandlename());
            
            if (!process.getOther().equals(""))
                processMap.put("other", process.getOther());
            
            mc.setRootObject(processMap);

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
    
    // STAXGenericRequestHandler Interface Methods

    public STAFResult handleRequest(Object infoObject, STAX staxService)
    {
        STAFServiceInterfaceLevel30.RequestInfo info =
            (STAFServiceInterfaceLevel30.RequestInfo)infoObject;

        String lowerRequest = info.request.toLowerCase();

        if (lowerRequest.startsWith("stop"))
        {
            // Need to make sure that the STAXTestcaseActionFactory gets a
            // chance to handle a STOP TESTCASE request so using a parser
            // that supports either a STOP PROCESS or STOP TESTCASE request.

            // XXX: A better way of determining if it is a STOP PROCESS or
            // STOP TESTCASE request would be nice because now if the syntax
            // of a STOP TESTCASE request changes, have to make the same
            // change to the fStopGenericParser here.

            STAFCommandParseResult parseResult = fStopGenericParser.parse(
                info.request);

            if (parseResult.rc != STAFResult.Ok)
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parseResult.errorBuffer);
            }

            if (parseResult.optionTimes("TESTCASE") > 0)
            {
                // This is a STOP TESTCASE request, not a STOP PROCESS request. 
                // Returning nothing in the result indicates that this parser
                // does not support this request.
                return new STAFResult(STAFResult.InvalidRequestString, "");
            }
            
            return handleStopRequest(info, staxService);
        }
        else
        {   // Returning nothing in the result indicates that this parser
            // does not support this request.
            return new STAFResult(STAFResult.InvalidRequestString, "");
        }
    }
    
    private STAFResult handleStopRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 5

        STAFResult trustResult = STAFUtil.validateTrust(
            4, staxService.getServiceName(), "STOP",
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

        // Resolve the value specified for PROCESS

        res = STAFUtil.resolveRequestVar(
            parseResult.optionValue("PROCESS"),
            staxService.getSTAFHandle(), info.requestNumber);

        if (res.rc != 0) return res;

        String processKey = res.result;
        
        // Verify that the process is a currently active process
        // (according to STAX)

        STAXProcessAction processAction = null;
        TreeMap processes = (TreeMap)job.getData("processRequestMap");

        if (processes != null)
        {
            synchronized (processes)
            {
                processAction = (STAXProcessAction)processes.get(processKey);
            }
        }

        if (processAction == null)
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "The specified process (" + processKey + ") does not exist." +
                "  Note its format must be: <location>:<handle#>");
        }

        String location = processAction.getLocation();
        int processHandle = processAction.getProcessHandle();

        long processRC = 0;  // Assume 0 if cannot obtain
        String stopTimestamp = "";

        // Stop the STAF process (if still running) by submitting a
        // PROCESS STOP HANDLE request on the machine where the process is
        // running.
        
        res = job.submitSync(
            location, "PROCESS", "QUERY HANDLE " + processHandle);
        
        if (debug)
        {
            System.out.println(
                "STAXProcessActionFactory::handleStopRequest: " +
                "PROCESS QUERY HANDLE " + processHandle + ", RC=" + res.rc);
        }

        if (res.rc == STAFResult.Ok)
        {
            // If the process is complete (indicated by rc = null),
            // get the process's completion info (RC and stopTimestamp)

            Map processMap = (Map)res.resultObj;
            String processRCString = (String)processMap.get("rc");

            if (processRCString != null)
            {
                processRC = Long.parseLong(processRCString);
                stopTimestamp = (String)processMap.get("endTimestamp");
            }
            else
            {
                // Process is not yet complete, so stop it

                res = job.submitSync(
                    location, "PROCESS", "STOP HANDLE " + processHandle);

                if (debug)
                {
                    System.out.println(
                        "STAXProcessActionFactory::handleStopRequest: " +
                        "PROCESS STOP HANDLE " + processHandle +
                        ", RC=" + res.rc);
                }

                if (res.rc == STAFResult.Ok)
                {
                    // Delay to allow the STAF/Process/End message (generated
                    // by the PROCESS STOP HANDLE request) to be sent to the
                    // STAX service machine and processed.  

                    try
                    {
                        Thread.sleep(1000);   // Sleep 1 second
                    }
                    catch (InterruptedException ex)
                    { }

                    // Now check if the handle exists (just in case the
                    // STAF/Process/End message was not successfully sent)

                    res = job.submitSync(
                        location, "PROCESS", "QUERY HANDLE " + processHandle);

                    if (debug)
                    {
                        System.out.println(
                            "STAXProcessActionFactory::handleStopRequest: " +
                            "PROCESS QUERY HANDLE " + processHandle +
                            ", RC=" + res.rc);
                    }

                    if (res.rc == STAFResult.Ok)
                    {
                        // Get the process's completion info (RC and
                        // stopTimestamp)
                        
                        processMap = (Map)res.resultObj;
                        processRCString = (String)processMap.get("rc");
                        
                        if (processRCString != null)
                        {
                            processRC = Long.parseLong(processRCString);
                            stopTimestamp = (String)processMap.get(
                                "endTimestamp");
                        }
                    }
                    else if (res.rc == STAFResult.HandleDoesNotExist)
                    {
                        // The process handle no longer exists so the
                        // STAF/Process/End message was successfully sent to
                        // the STAX service machine and processed so we can
                        // return.

                        return new STAFResult(STAFResult.Ok);
                    }
                }
            }
        }

        if (stopTimestamp.length() == 0)
        {
            // Assign the current timestamp as the time the process was stopped

            stopTimestamp = (new STAXTimestamp()).getTimestampString(); 
        }

        // Assign an null fileList since no file data returned by the process
        // is available to us

        List fileList = null;

        // Note that if this process has an active process-action, stopping
        // the process does not "terminate" the process-action.,

        // Notify STAX that the process is complete

        processAction.processComplete(
            location, processHandle, processRC, fileList, stopTimestamp);

        return new STAFResult(STAFResult.Ok);
    }

    public String getHelpInfo(String lineSep)
    {
        return "STOP      JOB <Job ID> PROCESS <Location:Handle>";
    }

    private String fName;
    private STAFMapClassDefinition fProcessInfoMapClass;
    private STAFMapClassDefinition fQueryProcessMapClass;
    private STAFCommandParser fStopParser = new STAFCommandParser();
    private STAFCommandParser fStopGenericParser = new STAFCommandParser();

    private static String fDTDInfo =
"\n" +
"<!--================= The STAF Process Element ===================== -->\n" +
"<!--\n" +
"     Specifies a STAF process to be started.\n" +
"     All of its non-empty element values are evaluated via Python.\n" +
"-->\n" +
"<!ENTITY % procgroup1 '((parms?, workdir?) | (workdir?, parms?))'>\n" +
"<!ENTITY % procgroup2 '((title?, workload?) | (workload?, title?))'>\n" +
"<!ENTITY % procgroup1a '((parms?, workload?) | (workload?, parms?))'>\n" +
"<!ENTITY % procgroup2a '((title?, workdir?) | (workdir?, title?))'>\n" +
"<!ENTITY % procgroup3 '(((vars | var | envs | env)*, useprocessvars?) |\n" +
"                        (useprocessvars?, (vars | var | envs | env)*))'>\n" +
"<!ENTITY % procgroup4 '(((username, password?)?, disabledauth?) |\n" +
"                        ((disabledauth?, (username, password?)?)))'>\n" +
"<!ENTITY % procgroup5 '((stdin?, stdout?, stderr?) |\n" + 
"                        (stdout?, stderr?, stdin?) |\n" +
"                        (stderr?, stdin?, stdout?) |\n" +
"                        (stdin?, stderr?, stdout?) |\n" +
"                        (stdout?, stdin?, stderr?) |\n" +
"                        (stderr?, stdout?, stdin?))'>\n" +
"<!ENTITY % returnfileinfo '(returnfiles | returnfile)*'>\n" +
"<!ENTITY % procgroup5a '((%returnfileinfo;, returnstdout?, returnstderr?) |\n" +
"                        (returnstdout?, returnstderr?, %returnfileinfo;) |\n" +
"                        (returnstderr?, %returnfileinfo;, returnstdout?) |\n" +
"                        (%returnfileinfo;, returnstderr?, returnstdout?) |\n" +
"                        (returnstdout?, %returnfileinfo;, returnstderr?) |\n" +
"                        (returnstderr?, returnstdout?, %returnfileinfo;))'>\n" +
//"<!ENTITY % procgroup6 '((stopusing?, console?, statichandlename?) |\n" +
//"                        (stopusing?, statichandlename?, console?) |\n" +
//"                        (console?, stopusing?, statichandlename?) |\n" +
//"                        (console?, statichandlename?, stopusing?) |\n" +
//"                        (statichandlename?, stopusing?, console?) |\n" +
//"                        (statichandlename?, console?, stopusing?))'>\n" +
"<!ENTITY % procgroup6 '((stopusing?, console?, focus?, statichandlename?) |\n" +
"                        (stopusing?, console?, statichandlename?, focus?) |\n" +
"                        (stopusing?, focus?, console?, statichandlename?) |\n" +
"                        (stopusing?, focus?, statichandlename?, console?) |\n" +
"                        (stopusing?, statichandlename?, console?, focus?) |\n" +
"                        (stopusing?, statichandlename?, focus?, console?) |\n" +
"                        (console?, focus?, stopusing?, statichandlename?) |\n" +
"                        (console?, focus?, statichandlename?, stopusing?) |\n" +
"                        (console?, stopusing?, focus?, statichandlename?) |\n" +
"                        (console?, stopusing?, statichandlename?, focus?) |\n" +
"                        (console?, statichandlename?, focus?, stopusing?) |\n" +
"                        (console?, statichandlename?, stopusing?, focus?) |\n" +
"                        (focus?, console?, stopusing?, statichandlename?) |\n" +
"                        (focus?, console?, statichandlename?, stopusing?) |\n" +
"                        (focus?, stopusing?, console?, statichandlename?) |\n" +
"                        (focus?, stopusing?, statichandlename?, console?) |\n" +
"                        (focus?, statichandlename?, console?, stopusing?) |\n" +
"                        (focus?, statichandlename?, stopusing?, console?) |\n" +
"                        (statichandlename?, stopusing?, console?, focus?) |\n" +
"                        (statichandlename?, stopusing?, focus?, console?) |\n" +
"                        (statichandlename?, console?, focus?, stopusing?) |\n" +
"                        (statichandlename?, console?, stopusing?, focus?) |\n" +
"                        (statichandlename?, focus?, console?, stopusing?) |\n" +
"                        (statichandlename?, focus?, stopusing?, console?))'>\n" +
"<!ELEMENT process    (location, command,\n" +
"                      ((%procgroup1;, %procgroup2;) |\n" + 
"                       (%procgroup2;, %procgroup1;) |\n" +
"                       (%procgroup1a;, %procgroup2a;) |\n" +
"                       (%procgroup2a;, %procgroup1a;)),\n" +
"                      %procgroup3;,\n" + 
"                      ((%procgroup4;, %procgroup5;, %procgroup5a;, %procgroup6;) |\n" +
"                       (%procgroup4;, %procgroup6;, %procgroup5;, %procgroup5a;) |\n" +
"                       (%procgroup5;, %procgroup5a;, %procgroup4;, %procgroup6;) |\n" +
"                       (%procgroup5;, %procgroup5a;, %procgroup6;, %procgroup4;) |\n" +
"                       (%procgroup6;, %procgroup4;, %procgroup5;, %procgroup5a;) |\n" +
"                       (%procgroup6;, %procgroup5;, %procgroup5a;, %procgroup4;)),\n" +
"                      other?, process-action?)>\n" +
"<!ATTLIST process\n" +
"          name        CDATA   #IMPLIED\n" +
">\n" +
"\n" +
"<!--\n" +
"     The process element must contain a location element and a\n" +
"     command element.\n" + 
"-->\n" +
"<!ELEMENT location            (#PCDATA)>\n" +
"<!ELEMENT command             (#PCDATA)>\n" +
"<!ATTLIST command\n" +
"          mode        CDATA   \"'default'\"\n" +
"          shell       CDATA   #IMPLIED\n" +
">\n" +
"\n" +
"<!--\n" +
"     The parms element specifies any parameters that you wish to\n" +
"     pass to the command.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT parms               (#PCDATA)>\n" +
"<!ATTLIST parms\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The workload element specifies the name of the workload for\n" +
"     which this process is a member.  This may be useful in\n" +
"     conjunction with other process elements.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT workload            (#PCDATA)>\n" +
"<!ATTLIST workload\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The title element specifies the program title of the process.\n" +
"     Unless overridden by the process, the title will be the text\n" +
"     that is displayed on the title bar of the application.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT title               (#PCDATA)>\n" +
"<!ATTLIST title\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The workdir element specifies the directory from which the\n" +
"     command should be executed.  If you do not specify this\n" +
"     element, the command will be started from whatever directory\n" +
"     STAFProc is currently in.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT workdir             (#PCDATA)>\n" +
"<!ATTLIST workdir\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The vars (and var) elements specify STAF variables that go into the\n" +
"     process specific STAF variable pool.\n" +
"     The value must evaluate via Python to a string or a list of \n" +
"     strings. Multiple vars elements may be specified for a process.\n" +
"     The format for each variable is:\n" +
"       'varname=value'\n" +
"     So, a list containing 3 variables could look like:\n" +
"       ['var1=value1', 'var2=value2', 'var3=value3']\n" +
"     Specifying only one variable could look like either:\n" +
"       ['var1=value1']      or \n" +
"       'var1=value1'\n" +
"-->\n" +
"<!ELEMENT vars                (#PCDATA)>\n" +
"<!ATTLIST vars\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!ELEMENT var                 (#PCDATA)>\n" +
"<!ATTLIST var\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The envs (and env) elements specify environment variables that will\n" +
"     be set for the process.  Environment variables may be mixed case,\n" +
"     however most programs assume environment variable names will\n" +
"     be uppercase, so, in most cases, ensure that your environment\n" +
"     variable names are uppercase.\n" +
"     The value must evaluate via Python to a string or a list of \n" +
"     strings. Multiple envs elements may be specified for a process.\n" +
"     The format for each variable is:\n" +
"       'varname=value'\n" +
"     So, a list containing 3 variables could look like:\n" +
"       ['ENV_VAR_1=value1', 'ENV_VAR_2=value2', 'ENV_VAR_3=value3']\n" +
"     Specifying only one variable could look like either:\n" +
"       ['ENV_VAR_1=value1']      or \n" +
"       'ENV_VAR_1=value1'\n" +
"-->\n" +
"<!ELEMENT envs                (#PCDATA)>\n" +
"<!ATTLIST envs\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!ELEMENT env                 (#PCDATA)>\n" +
"<!ATTLIST env\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"<!--\n" +
"     The useprocessvars element specifies that STAF variable\n" +
"     references should try to be resolved from the STAF variable\n" +
"     pool associated with the process being started first.\n" +
"     If the STAF variable is not found in this pool, the STAF\n" +
"     global variable pool should then be searched.\n" +
"-->\n" +
"<!ELEMENT useprocessvars      EMPTY>\n" +
"<!ATTLIST useprocessvars\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The stopusing element allows you to specify the method by\n" +
"     which this process will be STOPed, if not overridden on the\n" +
"     STOP command.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT stopusing           (#PCDATA)>\n" +
"<!ATTLIST stopusing\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The console element allows you to specify if the process should\n" +
"     get a new console window or share the STAFProc console.\n" +
"\n" +
"     use    Must evaluate via Python to a string containing either:\n" +
"            - 'new' specifies that the process should get a new console\n" +
"              window.  This option only has effect on Windows systems.\n" +
"              This is the default for Windows systems.\n" +
"            - 'same' specifies that the process should share the\n" +
"              STAFProc console.  This option only has effect on Windows\n" +
"              systems.  This is the default for Unix systems.\n" +
"-->\n" +
"<!ELEMENT console             EMPTY>\n" +
"<!ATTLIST console\n" +
"          if        CDATA     \"1\"\n" +
"          use       CDATA     #REQUIRED\n" +
">\n" +
"\n" +
"<!--\n" +
"     The focus element allows you to specify the focus that is to be\n" +
"     given to any new windows opened when starting a process on a Windows\n" +
"     system.  The window(s) it effects depends on whether you are using\n" +
"     the 'default' or the 'shell' command mode:\n" +
"       - Default command mode (no SHELL option):  The focus specified is\n" +
"         given to any new windows opened by the command specified.\n" +
"       - Shell command mode:  The focus specified is given only to the\n" +
"         new shell command window opened, not to any windows opened by\n" +
"         the specified command.\n" +
"\n" +         
"     The focus element only has effect on Windows systems and requires\n" +
"     STAF V3.1.4 or later on the machine where the process is run.\n" +
"\n" +
"     mode   Must evaluate via Python to a string containing one of the\n" +
"            following values:\n" +
"            - 'background' specifies to display a window in the background\n" +
"              (e.g. not give it focus) in its most recent size and position.\n" +
"              This is the default.\n" +
"            - 'foreground' specifies to display a window in the foreground\n" +
"              (e.g. give it focus) in its most recent size and position.\n" +
"            - 'minimized' specifies to display a window as minimized.\n" +
"-->\n" +
"<!ELEMENT focus               EMPTY>\n" +
"<!ATTLIST focus\n" +
"          if        CDATA     \"1\"\n" +
"          mode      CDATA     #REQUIRED\n" +
">\n" +
"\n" +
"<!--\n" +
"     The username element specifies the username under which \n" +
"     the process should be started.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT username            (#PCDATA)>\n" +
"<!ATTLIST username\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The password element specifies the password with which to\n" +
"     authenticate the user specified with the username element.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT password            (#PCDATA)>\n" +
"<!ATTLIST password\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!-- The disabledauth element specifies the action to take if a\n" +
"     username/password is specified but authentication has been disabled.\n" +
"\n" +
"     action  Must evaluate via Python to a string containing either:\n" +
"             - 'error' specifies that an error should be returned.\n" +
"             - 'ignore'  specifies that any username/password specified\n" +
"               is ignored if authentication is desabled.\n" +
"             This action overrides any default specified in the STAF\n" +
"             configuration file.\n" +
"-->\n" +
"<!ELEMENT disabledauth        EMPTY>\n" +
"<!ATTLIST disabledauth\n" +
"          if        CDATA     \"1\"\n" +
"          action    CDATA     #REQUIRED\n" +
">\n" +
"\n" +
"<!--\n" +
"     Specifies that a static handle should be created for this process.\n" +
"     The value is evaluated via Python to a string.  It will be the\n" +
"     registered name of the static handle.  Using this option will also\n" +
"     cause the environment variable STAF_STATIC_HANDLE to be set\n" +
"     appropriately for the process.\n" +
"-->\n" +
"<!ELEMENT statichandlename    (#PCDATA)>\n" +
"<!ATTLIST statichandlename\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The stdin element specifies the name of the file from which\n" +
"     standard input will be read.  The value is evaluated via\n" +
"     Python to a string.\n" +
"-->\n" +
"<!ELEMENT stdin               (#PCDATA)>\n" +
"<!ATTLIST stdin\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The stdout element specifies the name of the file to which\n" +
"     standard output will be redirected.\n" +
"     The mode and filename are evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT stdout              (#PCDATA)>\n" +
"<!--  mode  specifies what to do if the file already exists.\n" +
"            The value must evaluate via Python to one of the\n" +
"            following:\n" +
"            'replace' - specifies that the file will be replaced.\n" +
"            'append'  - specifies that the process' standard\n" +
"                        output will be appended to the file.\n" +
"-->\n" +
"<!ATTLIST stdout\n" +
"          if        CDATA     \"1\"\n" +
"          mode      CDATA     \"'replace'\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The stderr element specifies the file to which standard error will\n" +
"     be redirected. The mode and filename are evaluated via Python to a\n" +
"     string.\n" +
"-->\n" +
"<!ELEMENT stderr              (#PCDATA)>\n" +
"<!-- mode   specifies what to do if the file already exists or to\n" +
"            redirect standard error to the same file as standard output.\n" +
"            The value must evaluate via Python to one of the following:\n" +
"            'replace' - specifies that the file will be replaced.\n" +
"            'append'  - specifies that the process's standard error will\n" +
"                        be appended to the file.\n" +
"            'stdout'  - specifies to redirect standard error to the\n" +
"                        same file to which standard output is redirected.\n" +
"                        If a file name is specified, it is ignored.\n" +
"-->\n" +
"<!ATTLIST stderr\n" +
"          if        CDATA     \"1\"\n" +
"          mode      CDATA     \"'replace'\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The returnstdout element specifies to return in STAXResult\n" +
"     the contents of the file where standard output was redirected\n" +
"     when the process completes.\n" +
"-->\n" +
"<!ELEMENT returnstdout        EMPTY>\n" +
"<!ATTLIST returnstdout\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The returnstderr element specifies to return in STAXResult\n" +
"     the contents of the file where standard error was redirected\n" +
"     when the process completes.\n" +
"-->\n" +
"<!ELEMENT returnstderr        EMPTY>\n" +
"<!ATTLIST returnstderr\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The returnfiles (and returnfile) elements specify that the\n" +
"     contents of the specified file(s) should be returned in\n" +
"     STAXResult when the process completes.  The value must evaluate\n" +
"     via Python to a string or a list of strings.  Multiple returnfile(s)\n" +
"     elements may be specified for a process.\n" +
"-->\n" +
"<!ELEMENT returnfiles         (#PCDATA)>\n" +
"<!ATTLIST returnfiles\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!ELEMENT returnfile          (#PCDATA)>\n" +
"<!ATTLIST returnfile\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The process-action element specifies a task to be executed\n" +
"     when a process has started.\n" +
"-->\n" +
"<!ELEMENT process-action      (%task;)>\n" +
"<!ATTLIST process-action\n" +
"          if        CDATA     \"1\"\n" +
">\n" +
"<!--\n" +
"     The other element specifies any other STAF parameters that\n" +
"     may arise in the future.  It is used to pass additional data\n" +
"     to the STAF PROCESS START request.\n" +
"     The value is evaluated via Python to a string.\n" +
"-->\n" +
"<!ELEMENT other               (#PCDATA)>\n" +
"<!ATTLIST other\n" +
"          if        CDATA     \"1\"\n" +
">\n";

} // end Class
