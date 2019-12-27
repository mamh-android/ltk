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
import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFUtil;
import com.ibm.staf.service.*;
import java.util.HashMap;

public class STAXLogActionFactory implements STAXActionFactory,
                                             STAXGenericRequestHandler
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Log Element ============================== -->\n" +
"<!--\n" +
"     Writes a message and its log level to a STAX Job User Log file.\n" +
"     The message must evaluate via Python to a string.\n\n" +
"     The log level specified defaults to 'info'.  If specified, it\n" +
"     must evaluate via Python to a string containing one of the\n" +
"     following STAF Log Service Log levels:\n" +
"       fatal, warning, info, trace, trace2, trace3, debug, debug2,\n" +
"       debug3, start, stop, pass, fail, status, user1, user2, user3,\n" +
"       user4, user5, user6, user7, user8\n" +
"     The message attribute is evaluated via Python.  If it evaluates\n" +
"     to true, the message text will also be sent to the STAX Job Monitor.\n" +
"     The message attribute defaults to the STAXMessageLog variable whose\n" +
"     value defaults to 0 (false) but can by changed within the STAX job\n" +
"     to turn on messaging.\n\n" +
"     If an if attribute is specified and it evaluates via Python to\n" +
"     false, then the log element is ignored.\n" +  
"-->\n" +
"<!ELEMENT log         (#PCDATA)>\n" +
"<!ATTLIST log\n" +
"          level       CDATA       \"'info'\"\n" +
"          message     CDATA       \"STAXMessageLog\"\n" +
"          if          CDATA       \"1\"\n" +
">\n";

    
    private STAFCommandParser fLogParser = new STAFCommandParser();

    public STAXLogActionFactory(STAX staxService)
    {
        // Set up LOG parser

        fLogParser.addOption("LOG", 1, STAFCommandParser.VALUENOTALLOWED);
        fLogParser.addOption("JOB", 1, STAFCommandParser.VALUEREQUIRED);
        fLogParser.addOption("MESSAGE", 1, STAFCommandParser.VALUEREQUIRED);
        fLogParser.addOption("LEVEL", 1, STAFCommandParser.VALUEREQUIRED);
        fLogParser.addOption("SEND", 1, STAFCommandParser.VALUENOTALLOWED);

        fLogParser.addOptionNeed("LOG", "JOB");
        fLogParser.addOptionNeed("JOB", "LOG");
        fLogParser.addOptionNeed("MESSAGE", "LOG");
        fLogParser.addOptionNeed("LOG", "MESSAGE");
        fLogParser.addOptionNeed("LEVEL", "LOG");
        fLogParser.addOptionNeed("SEND", "LOG");

        // Register as a GenericRequestHandler
        try
        {
            // Assign STAFServiceInterfaceLevel class that this handler uses

            Class serviceInterfaceClass = Class.forName(
                STAX.INTERFACE_LEVEL_30);

            int rc = staxService.registerGenericRequestHandler(
                this, serviceInterfaceClass);

            if (rc != 0)
            {
                System.out.println("STAXLogActionFactory." +
                                   "registerGenericRequestHandler() failed");
            }
        }
        catch (ClassNotFoundException e)
        {
            System.out.println("STAXLogActionFactory: " +
                               "registerGenericRequestHandler: " + e);
        }
    }

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "log";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXLogAction action = new STAXLogAction();
        action.setLogfile(STAXJob.USER_JOB_LOG);

        String element = root.getNodeName();
        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);
            String attrName = thisAttr.getNodeName();
            
            action.setElementInfo(new STAXElementInfo(element, attrName));

            if (attrName.equals("level"))
            {
                if (thisAttr.getNodeValue().equals(""))
                {
                    action.setElementInfo(new STAXElementInfo(
                        element, attrName,
                        "The value for the \"level\" attribute cannot be " +
                        "blank"));

                    throw new STAXInvalidXMLAttributeException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                action.setLevel(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
            else if (thisAttr.getNodeName().equals("message"))
            {
                action.setMessageAttr(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
            else if (thisAttr.getNodeName().equals("if"))
            {
                action.setIf(STAXUtil.parseAndCompileForPython(
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
            else if (thisChild.getNodeType() == Node.TEXT_NODE)
            {
                action.setElementInfo(new STAXElementInfo(element));
                action.setMessage(STAXUtil.parseAndCompileForPython(
                    thisChild.getNodeValue(), action));
            }
        }

        return action;
    }


    // STAXGenericRequestHandler Interface Methods

    public STAFResult handleRequest(Object infoObject, STAX staxService)
    {
        STAFServiceInterfaceLevel30.RequestInfo info =
            (STAFServiceInterfaceLevel30.RequestInfo)infoObject;

        String lowerRequest = info.request.toLowerCase();

        if (lowerRequest.startsWith("log"))
            return handleLogRequest(info, staxService);
        else
        {   // Returning nothing in the result indicates that this parser
            // does not support this request.
            return new STAFResult(STAFResult.InvalidRequestString, "");
        }
    }

    private STAFResult handleLogRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, staxService.getServiceName(), "LOG", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fLogParser.parse(info.request);

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
        
        // Get the value specified for MESSAGE.  Don't resolve variables.

        String message = parseResult.optionValue("MESSAGE");

        // If LEVEL is not specified, it defaults to Info

        String level = new String("Info");

        if (parseResult.optionTimes("LEVEL") != 0)
        {
            // Resolve the value specified for LEVEL

            res = STAFUtil.resolveRequestVar(
                parseResult.optionValue("LEVEL"),
                staxService.getSTAFHandle(), info.requestNumber);

            if (res.rc != 0) return res;

            level = res.result;
        }

        // Log the message in the STAX Job user log
        
        res = job.log(STAXJob.USER_JOB_LOG, level, message);

        if (res.rc == 4004)
        {
            // Returning an RC for the LOG service is confusing since the
            // request was submitted to the STAX service, so doing this
            // to try to clarify the error.
            res.rc = STAFResult.InvalidValue;
            res.result = "Invalid log level: " + level;
        }

        if (res.rc != STAFResult.Ok)
        {
            return res;
        }

        if (parseResult.optionTimes("SEND") > 0)
        {
            // Send the message to the STAX Monitor (via an event)
            
            HashMap<String, String> messageMap = new HashMap<String, String>();
            STAXTimestamp timestamp = new STAXTimestamp();

            messageMap.put("messagetext",
                           timestamp.getTimestampString() + " " +
                           STAFUtil.maskPrivateData(message));
            job.generateEvent(
                STAXMessageActionFactory.STAX_MESSAGE, messageMap);
        }

        return new STAFResult(STAFResult.Ok);
    }

    public String getHelpInfo(String lineSep)
    {
        return "LOG       JOB <Job ID> MESSAGE <Message> " +
               "[LEVEL <Level>] [SEND]";
    }
}
