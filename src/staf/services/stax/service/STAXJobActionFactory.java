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
import com.ibm.staf.*;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

/**
 * A factory class for creating STAXJobAction objects based on
 * information obtained by parsing an XML node from a DOM tree for a
 * &lt;job&gt; element.
 *
 *@see STAXJobAction
 */
public class STAXJobActionFactory implements STAXActionFactory,
                                             STAXListRequestHandler,
                                             STAXJobManagementHandler
{
    static final String STAX_SUBJOB_EVENT = new String("SubJob");

    static final String sSubjobInfoMapClassName = new String(
        "STAF/Service/STAX/SubjobInfo");
    
    /** 
     * Creates a new STAXJobActionFactory instance and registers
     * with the STAX service to handle requests to LIST the active 
     * sub-jobs in a job and to manage a map of currently active
     * sub-jobs for a job.
     */
    public STAXJobActionFactory(STAX staxService)
    {
        staxService.registerListHandler("SUBJOBS", this);
        staxService.registerJobManagementHandler(this);

        // Construct map-class for list subjob information

        fSubjobInfoMapClass = new STAFMapClassDefinition(
            sSubjobInfoMapClassName);

        fSubjobInfoMapClass.addKey("jobID",          "Job ID");
        fSubjobInfoMapClass.addKey("jobName",        "Job Name");
        fSubjobInfoMapClass.addKey("startTimestamp", "Start Date-Time");
        fSubjobInfoMapClass.addKey("function",       "Function");
        fSubjobInfoMapClass.addKey("blockName",      "Block Name");
    }

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "job";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXJobAction subjob = new STAXJobAction();

        subjob.setActionFactory(this);

        String element = root.getNodeName();
        subjob.setLineNumber(root);
        subjob.setXmlFile(job.getXmlFile());
        subjob.setXmlMachine(job.getXmlMachine());

        NamedNodeMap rootAttrs = root.getAttributes();

        for (int i = 0; i < rootAttrs.getLength(); ++i)
        {
            Node thisAttr = rootAttrs.item(i);

            subjob.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));
            
            if (thisAttr.getNodeName().equals("name"))
            {                
                subjob.setName(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("clearlogs"))
            {                
                subjob.setClearlogs(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("monitor"))
            {                
                subjob.setMonitor(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("logtcelapsedtime"))
            {
                subjob.setLogTCElapsedTime(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("logtcnumstarts"))
            {
                subjob.setLogTCNumStarts(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("logtcstartstop"))
            {
                subjob.setLogTCStartStop(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("pythonoutput"))
            {
                subjob.setPythonOutput(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
            }
            else if (thisAttr.getNodeName().equals("pythonloglevel"))
            {
                subjob.setPythonLogLevel(
                    STAXUtil.parseAndCompileForPython(
                        thisAttr.getNodeValue(), subjob));
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
                subjob.setLineNumber(thisChild);
                
                if (thisChild.getNodeName().equals("job-file"))
                {
                    String machine = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));
                        
                        if (thisAttr.getNodeName().equals("machine"))
                        {
                            machine = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        }
                    }

                    subjob.setJobFile(
                        handleChild(thisChild, subjob), machine);
                }
                else if (thisChild.getNodeName().equals("job-data"))
                {
                    String eval = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("eval"))
                        {
                            eval = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        }
                    }

                    subjob.setJobData(
                        handleChild(thisChild, subjob), eval);
                }
                else if (thisChild.getNodeName().equals("job-function"))
                {
                    String ifValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                        {
                            ifValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        }
                    }

                    subjob.setFunction(handleChild(thisChild, subjob),
                                       ifValue);
                }
                else if (thisChild.getNodeName().equals("job-function-args"))
                {
                    String ifValue = new String();
                    String eval    = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        else if (thisAttr.getNodeName().equals("eval"))
                            eval = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                    }

                    subjob.setFunctionArgs(
                        handleChild(thisChild, subjob), eval, ifValue);
                }
                else if (thisChild.getNodeName().equals("job-script"))
                {
                    String ifValue = new String();
                    String eval    = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName(),
                            STAXElementInfo.LAST_ELEMENT_INDEX));
                        
                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        else if (thisAttr.getNodeName().equals("eval"))
                            eval = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                    }

                    subjob.setScripts(
                        handleChild(thisChild, subjob), eval, ifValue);
                }
                else if (thisChild.getNodeName().equals("job-scriptfile") ||
                         thisChild.getNodeName().equals("job-scriptfiles"))
                {
                    // Either job-scriptfile or job-scriptfiles can be
                    // specified, so need to set which was used so the
                    // action can get it's linenumber if a run-time error
                    // occurs.
                    subjob.setScriptFileElementName(thisChild.getNodeName());

                    String ifValue  = new String();
                    String machine = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        else if (thisAttr.getNodeName().equals("machine"))
                            machine = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                    }

                    subjob.setScriptFiles(
                        handleChild(thisChild, subjob), machine, ifValue);
                }
                else if (thisChild.getNodeName().equals("job-hold"))
                {
                    String ifValue = new String();
                    String holdTimeout = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));

                        if (thisAttr.getNodeName().equals("if"))
                        {
                            ifValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        }
                        else if (thisAttr.getNodeName().equals("timeout"))
                        {
                            holdTimeout = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                        }
                    }

                    subjob.setHold(holdTimeout, ifValue);
                }
                else if (thisChild.getNodeName().equals("job-action"))
                {
                    String ifValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        subjob.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), thisAttr.getNodeName()));
                        
                        if (thisAttr.getNodeName().equals("if"))
                            ifValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), subjob);
                    }                

                    NodeList jobActionChildren = thisChild.getChildNodes();

                    STAXAction jobAction = null;

                    for (int j = 0; j < jobActionChildren.getLength(); ++j)
                    {
                        Node jobActionChild = jobActionChildren.item(j);

                        if (jobActionChild.getNodeType() == Node.COMMENT_NODE)
                        {
                            /* Do nothing */
                        }
                        else if (jobActionChild.getNodeType() == 
                                 Node.ELEMENT_NODE)
                        {
                            if (jobAction != null)
                            {
                                subjob.setElementInfo(
                                    new STAXElementInfo(
                                        thisChild.getNodeName(),
                                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                                        STAXElementInfo.LAST_ELEMENT_INDEX,
                                        jobActionChild.getNodeName() + "\""));

                                throw new STAXInvalidXMLElementCountException(
                                    STAXUtil.formatErrorMessage(subjob),
                                    subjob);
                            }
                        
                            STAXActionFactory factory = 
                                staxService.getActionFactory(
                                    jobActionChild.getNodeName());

                            if (factory == null)
                            {
                                subjob.setElementInfo(new STAXElementInfo(
                                    thisChild.getNodeName(),
                                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                                    STAXElementInfo.LAST_ELEMENT_INDEX,
                                    "No action factory for element type \"" +
                                    jobActionChild.getNodeName() + "\""));

                                throw new STAXInvalidXMLElementException(
                                    STAXUtil.formatErrorMessage(subjob),
                                    subjob);
                            }

                            jobAction = factory.parseAction(
                                staxService, job, jobActionChild);
                        }
                    }

                    subjob.setJobAction(jobAction, ifValue);
                }
            }
            else
            {
                subjob.setElementInfo(new STAXElementInfo(
                    root.getNodeName(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(subjob), subjob);
            }
        }

        return subjob;
    }
    
    private String handleChild(Node root, STAXJobAction action)
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
                    root.getNodeName(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX));

                if (!root.getNodeName().equals("job-data"))
                {
                    return STAXUtil.parseAndCompileForPython(
                        thisChild.getNodeValue(), action);
                }
                else
                {
                    // Call the parseForPython method for this element instead
                    // of parseAndCompileForPython to remove the leading 
                    // whitespace but don't compile the value.  This is
                    // because it has an "eval" attribute which if set to
                    // false means don't evaluate it via Python in the parent
                    // job as its value may be XML, not a Python string.
                    return STAXUtil.parseForPython(thisChild.getNodeValue());
                }
            }
            else if (thisChild.getNodeType() == Node.CDATA_SECTION_NODE)
            {
                /* Do nothing */
            }
            else
            {
                action.setElementInfo(
                    new STAXElementInfo(
                        root.getNodeName(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        "Contains invalid node type: " +
                        Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);
            }
        }

        return new String();
    }


    // STAXJobManagement methods

    public void initJob(STAXJob job)
    {
        boolean result = job.setData(
            "subJobMap", new TreeMap<String, STAXJobAction>()); 

        if (!result)
        {
            String msg = "STAXJobActionFactory.initJob: setData for " +
                         "subJobMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        } 
    }

    public void terminateJob(STAXJob job)
    {
        // Remove from map of currently executing sub-jobs

        TreeMap subJobs = (TreeMap)job.getData("subJobMap");

        if (subJobs != null)
        {
            synchronized (subJobs)
            {
                subJobs.remove(String.valueOf(job.getJobNumber()));
            }
        }
    }
    

    // STAXListRequestHandler method

    public STAFResult handleListRequest(String type, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("subjobs"))
        {
            // LIST JOB <Job ID> SUBJOBS

            // Create the marshalling context and set its map class definitions
            // and create an empty list to contain the block map entries

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fSubjobInfoMapClass);
            List<Map<String, Object>> subjobOutputList =
                new ArrayList<Map<String, Object>>();

            // Iterate through the subjob map, generating the output list

            TreeMap subjobs = (TreeMap)job.getData("subJobMap");

            if (subjobs != null)
            {
                synchronized (subjobs)
                {
                    Iterator iter = subjobs.values().iterator();

                    while (iter.hasNext())
                    {
                        STAXJobAction subjob = (STAXJobAction)iter.next();

                        Map<String, Object> subjobMap =
                            new TreeMap<String, Object>();
                        subjobMap.put("staf-map-class-name",
                                      fSubjobInfoMapClass.name());

                        if (!subjob.getName().equals(""))
                            subjobMap.put("jobName", subjob.getName());

                        subjobMap.put("jobID", "" + subjob.getJobID());
                        subjobMap.put(
                            "startTimestamp",
                            subjob.getStartTimestamp().getTimestampString());
                        subjobMap.put("function", subjob.getStartFunction());
                        subjobMap.put(
                            "blockName", subjob.getCurrentBlockName());

                        subjobOutputList.add(subjobMap);
                    }
                }
            }

            mc.setRootObject(subjobOutputList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    private STAFMapClassDefinition fSubjobInfoMapClass;

    private static String fDTDInfo =
"\n" +
"<!--================== The STAX Job Element ===================== -->\n" +
"<!--\n" +
"     Specifies a STAX sub-job to be executed.  This element is equivalent\n" +
"     to a STAX EXECUTE request.\n" +
"\n" +
"     The name attribute specifies the name of the job. The job name\n" +
"     defaults to the value of the function name called to start the job.\n" +
"     Its name and all of its element values are evaluated via Python.\n" +
"     The job element must contain a location element and either a\n" +
"     file or data element.  This attribute is equivalent to the\n" +
"     JOBNAME option for a STAX EXECUTE command.\n" +
"\n" +
"     The clearlogs attribute specifies to delete the STAX Job and Job\n" +
"     User logs before the job is executed to ensure that only one job's\n" +
"     contents are in the log.  This attribute is equivalent to the\n" +
"     CLEARLOGS option for a STAX EXECUTE command.  The default is the\n" +
"     same option that was specified for the parent job.  Valid values\n" +
"     include 'parent', 'default', 'enabled', and 'disabled'.\n" +
"\n" +
"     The monitor attribute specifies whether to automatically monitor the\n" +
"     subjob.  Note that 'Automatically monitor recommended sub-jobs' must\n" +
"     be selected in the STAX Job Monitor properties in order for it to be\n" +
"     used.  The default value for the monitor attribute is 0, a false\n" +
"     value.\n" +
"\n" +
"     The logtcelapsedtime attribute specifies to log the elapsed time\n" +
"     for a testcase in the summary record in the STAX Job log and on a\n" +
"     LIST TESTCASES request.  This attribute is equivalent to the\n" +
"     LOGTCELAPSEDTIME option for a STAX EXECUTE command.  The default is\n" +
"     the same option that was specified for the parent job.  Valid values\n" +
"     include 'parent', 'default', 'enabled', and 'disabled'.\n" +
"\n" +
"     The logtcnumstarts attribute specifies to log the number of starts\n" +
"     for a testcase in the summary record in the STAX Job log and on a\n" +
"     LIST TESTCASES request.  This attribute is equivalent to the\n" +
"     LOGNUMSTARTS option for a STAX EXECUTE command.  The default is\n" +
"     the same option that was specified for the parent job.  Valid values\n" +
"     include 'parent', 'default', 'enabled', and 'disabled'.\n" +
"\n" +
"     The logtcstartstop attribute specifies to log start/stop records\n" +
"     for testcases in the STAX Job log.  This attribute is equivalent to\n" +
"     the LOGTCSTARTSTOP option for a STAX EXECUTE command.  The default\n" +
"     is the same option that was specified for the parent job.  Valid\n" +
"     values include 'parent', 'default', 'enabled', and 'disabled'.\n" +
"\n" +
"     The pythonoutput attribute specifies where to write Python stdout/stderr\n" +
"     (e.g. from a print statement in a script element).  This attribute\n" +
"     is equivalent to the PYTHONOUTPUT option for a STAX EXECUTE command.\n" +
"     The default is the same option that was specified for the parent job.\n" +
"     Valid values include 'parent', 'default', 'jobuserlog', 'message',\n" +
"     'jobuserlogandmsg', and 'jvmlog'.\n" +
"\n" +
"     The pythonloglevel attribute specifies the log level to use when writing\n" +
"     Python stdout (e.g. from a print statement in a script element) if the\n" +
"     python output is written to the STAX Job User Log.  This attribute is\n" +
"     equivalent to the PYTHONLOGLEVEL option for a STAX EXECUTE command.\n" +
"     The default is the same option that was specified for the parent job.\n" +
"     Valid values include 'parent', 'default', or a valid STAF log level\n" +
"     such as 'Info', 'Trace', 'User1', etc.\n" +
"\n" +
"     The job element must contain either a job-file or job-data element.\n" +
"\n" +
"     The job element has the following optional elements:\n" +
"       job-function, job-function-args, job-scriptfile(s), job-script,\n" +
"       job-hold, and job-action\n" +
"\n" +
"     Each of these optional elements may specify an if attribute.\n" +
"     The if attribute must evaluate via Python to a true or false value.\n" +
"     If it does not evaluate to a true value, the element is ignored.\n" +
"     The default value for the if attribute is 1, a true value.\n" +
"     Note that in Python, true means any nonzero number or nonempty\n" + 
"     object; false means not true, such as a zero number, an empty\n" +
"     object, or None. Comparisons and equality tests return 1 or 0\n" +
"     (true or false).\n" + 
"-->\n" +
"<!ELEMENT job        ((job-file | job-data),\n" +
"                      job-function?, job-function-args?,\n" +
"                      (job-scriptfile | job-scriptfiles)?,\n" +
"                      job-script*, job-hold?, job-action?)>\n" +
"<!ATTLIST job\n" +
"          name              CDATA   #IMPLIED\n" +
"          clearlogs         CDATA   \"'parent'\"\n" +
"          monitor           CDATA   #IMPLIED\n" +
"          logtcelapsedtime  CDATA   \"'parent'\"\n" +
"          logtcnumstarts    CDATA   \"'parent'\"\n" +
"          logtcstartstop    CDATA   \"'parent'\"\n" +
"          pythonoutput      CDATA   \"'parent'\"\n" +
"          pythonloglevel    CDATA   \"'parent'\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-file element specifies the fully qualified name of a file\n" +
"     containing the XML document for the STAX job to be executed.\n" +
"     The job-file element is equivalent to the FILE option for a STAX\n" +
"     EXECUTE command.\n" +
"\n" +
"     The machine attribute specifies the name of the machine where the\n" +
"     xml file is located.  If not specified, it defaults to Python\n" +
"     variable STAXJobXMLMachine.  The machine attribute is equivalent\n" +
"     to the MACHINE option for a STAX EXECUTE command.\n" +
"  -->\n" +
"<!ELEMENT job-file           (#PCDATA)>\n" +
"<!ATTLIST job-file\n" +
"          machine    CDATA   \"STAXJobXMLMachine\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-data element specifies a string containing the XML document\n" +
"     for the job to be executed.  This element is equivalent to the\n" +
"     DATA option for a STAX EXECUTE command.\n" +
"\n" + 
"     The eval attribute specifies whether the data is be evaluated by\n" +
"     Python in the parent job.  For example, if the job-data information\n" +
"     is dynamically generated and assigned to a Python variable, rather\n" +
"     than just containing the literal XML information, then you would\n" +
"     need to set the eval attribute to true (e.g. eval=\"1\").\n" +
"     The default for the eval attribute is false (\"0\").\n" +
"  -->\n" +
"<!ELEMENT job-data           (#PCDATA)>\n" +
"<!ATTLIST job-data\n" +
"          eval       CDATA   \"0\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-function element specifies the name of the function element\n" +
"     to call to start the job, overriding the defaultcall element, if any,\n" +
"     specified in the XML document. The <function name> must be the name\n" +
"     of a function element specified in the XML document. This element is\n" +
"     equivalent to the FUNCTION option for a STAX EXECUTE command.\n" +
"-->\n" +
"<!ELEMENT job-function       (#PCDATA)>\n" +
"<!ATTLIST job-function\n" +
"          if         CDATA   \"1\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-function-args element specifies arguments to pass to the\n" +
"     function element called to start the job, overriding the arguments,\n" +
"     if any, specified for the defaultcall element in the XML document.\n" +
"     This element is equivalent to the ARGS option for a STAX EXECUTE\n" +
"     command.\n" +
"\n" +
"     The eval attribute specifies whether the data is to be evaluated\n" +
"     by Python in the parent job.  The default for the eval attribute\n" +
"     is false (\"0\").\n" +
"-->\n" +
"<!ELEMENT job-function-args  (#PCDATA)>\n" +
"<!ATTLIST job-function-args\n" +
"          if         CDATA   \"1\"\n" +
"          eval       CDATA   \"0\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-script element specifies Python code to be executed.\n" +
"     This element is equivalent to the SCRIPT option for a STAX\n" +
"     EXECUTE command.  Multiple job-script elements may be specified.\n" +
"\n" +
"     The eval attribute specifies whether the data is to be evaluated\n" +
"     by Python in the parent job.  The default for the eval attribute\n" +
"     is false (\"0\").\n" +
"-->\n" +
"<!ELEMENT job-script         (#PCDATA)>\n" +
"<!ATTLIST job-script\n" +
"          if         CDATA   \"1\"\n" +
"          eval       CDATA   \"0\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-scriptfile element (equivalent to the job-scriptfiles\n" +
"     element) specifies the fully qualified name of a file containing\n" +
"     Python code to be executed, or a list of file names containing\n" +
"     Python code to be executed. The value must evaluate via Python to\n" +
"     a string or a list of strings. This element is equivalent to the\n" +
"     SCRIPTFILE option for a STAX EXECUTE command.\n" +
"\n" +
"     Specifying only one scriptfile could look like either:\n" +
"       ['C:/stax/scriptfiles/scriptfile1.py']      or \n" +
"       'C:/stax/scriptfiles/scriptfiel1.py'\n" +
"     Specifying a list containing 3 scriptfiles could look like:\n" +
"       ['C:/stax/scriptfiles/scriptfile1.py',\n" +
"        'C:/stax/scriptfiles/scriptfile2.py',\n" +
"         C:/stax/scriptfiles/scriptfile2.py' ]\n" +
"\n" +
"     The machine attribute specifies the name of the machine where the\n" +
"     SCRIPTFILE(s) are located. If not specified, it defaults to Python\n" +
"     variable STAXJobScriptFileMachine.  This attribute is equivalent\n" +
"     to the SCRIPTFILEMACHINE option for a STAX EXECUTE command.\n" +
"-->\n" +
"<!ELEMENT job-scriptfile     (#PCDATA)>\n" +
"<!ATTLIST job-scriptfile\n" +
"          if         CDATA   \"1\"\n" +
"          machine    CDATA   \"STAXJobScriptFileMachine\"\n" +
">\n" +
"\n" +
"<!ELEMENT job-scriptfiles    (#PCDATA)>\n" +
"<!ATTLIST job-scriptfiles\n" +
"          if         CDATA   \"1\"\n" +
"          machine    CDATA   \"STAXJobScriptFileMachine\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-hold element specifies to hold the job.  This element is\n" +
"     equivalent to the HOLD option for a STAX EXECUTE command,\n" +
"\n" +
"     The default timeout is 0 which specifies to hold the job indefinitely.\n" +
"     A non-zero timeout value specifies the maximum time that the job\n" +
"     will be held,  The timeout can be expressed in milliseconds, seconds,\n" +
"     minutes, hours, days, weeks, or years.  It is evaluated via Python.\n" +
"       Examples:  timeout=\"'1000'\"   (1000 milliseconds or 1 second)\n" +
"                  timeout=\"'5s'\"     (5 seconds)\n" +
"                  timeout=\"'1m'\"     (1 minute)\n" +
"                  timeout=\"'2h'\"     (2 hours)\n" +
"                  timeout=\"'0'\"      (hold indefinitely)\n" +
"-->\n" +
"<!ELEMENT job-hold           (#PCDATA)>\n" +
"<!ATTLIST job-hold\n" +
"          if         CDATA   \"1\"\n" +
"          timeout    CDATA   \"0\"\n" +
">\n" +
"\n" +
"<!--\n" +
"     The job-action element specifies a task to be executed after the\n" +
"     sub-job has started. This task will be executed in parallel with\n" +
"     the sub-job via a new STAX-Thread. The task will be able to use the\n" +
"     STAXSubJobID variable to obtain the sub-job ID in order to interact\n" +
"     with the job. If the job completes before the task completes, the\n" +
"     job will remain in a non-complete state until the task completes.\n" +
"     If the job cannot be started, the job-action task is not executed.\n" +
"-->\n" +
"<!ELEMENT job-action         (%task;)>\n" +
"<!ATTLIST job-action\n" +
"          if        CDATA    \"1\"\n" +
">\n";

}
