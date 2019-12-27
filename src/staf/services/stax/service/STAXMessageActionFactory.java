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

public class STAXMessageActionFactory implements STAXActionFactory,
                                                 STAXGenericRequestHandler
{
    static final String STAX_MESSAGE = new String("Message");

    private static String fDTDInfo =
"\n" +
"<!--================= The Message Element ========================== -->\n" +
"<!--\n" +
"     Generates an event and makes the message value available to the\n" +
"     STAX Job Monitor.  The message must evaluate via Python to a string.\n\n" +
"     The log attribute is evaluated via Python to a boolean.  If it\n" +
"     evaluates to true, the message text will also be logged in the STAX\n" +
"     Job User log.  The log attribute defaults to the STAXLogMessage\n" +
"     variable whose value defaults to 0 (false) but can by changed within\n" +
"     the STAX job to turn on logging.\n\n" +
"     The log level is ignored if the log attribute does not evaluate to\n" +
"     true.  It defaults to 'info'.  If specified, it must evaluate via\n" +
"     Python to a string containing one of the following STAF Log Service\n" +
"     logging levels:\n" +
"       fatal, warning, info, trace, trace2, trace3, debug, debug2,\n" +
"       debug3, start, stop, pass, fail, status, user1, user2, user3,\n" +
"       user4, user5, user6, user7, user8\n\n" +
"     If an if attribute is specified and it evaluates via Python to\n" +
"     false, the message element is ignored.\n" +
"-->\n" +
"<!ELEMENT message     (#PCDATA)>\n" +
"<!ATTLIST message\n" +
"          log         CDATA       \"STAXLogMessage\"\n" +
"          level       CDATA       \"'info'\"\n" +
"          if          CDATA       \"1\"\n" +
">\n";

    private STAFCommandParser fSendParser = new STAFCommandParser();

    public STAXMessageActionFactory(STAX staxService)
    {
        // Set up SEND parser

        fSendParser.addOption("SEND", 1, STAFCommandParser.VALUENOTALLOWED);
        fSendParser.addOption("JOB", 1, STAFCommandParser.VALUEREQUIRED);
        fSendParser.addOption("MESSAGE", 1, STAFCommandParser.VALUEREQUIRED);

        fSendParser.addOptionNeed("SEND", "JOB");
        fSendParser.addOptionNeed("JOB", "SEND");
        fSendParser.addOptionNeed("MESSAGE", "SEND");
        fSendParser.addOptionNeed("SEND", "MESSAGE");

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
                System.out.println("STAXMessageActionFactory." +
                                   "registerGenericRequestHandler() failed");
            }
        }
        catch (ClassNotFoundException e)
        {
            System.out.println("STAXMessageActionFactory: " +
                               "registerGenericRequestHandler: " + e);
        }
    }

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "message";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXMessageAction action = new STAXMessageAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("log"))
            {
                action.setLogAttr(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
            else if (thisAttr.getNodeName().equals("level"))
            {
                if (thisAttr.getNodeValue().equals(""))
                {
                    action.setElementInfo(new STAXElementInfo(
                        root.getNodeName(), thisAttr.getNodeName(),
                        "The value for the \"level\" attribute cannot be " +
                        "blank"));

                    throw new STAXInvalidXMLAttributeException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                action.setLevel(STAXUtil.parseAndCompileForPython(
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
                action.setElementInfo(
                    new STAXElementInfo(root.getNodeName()));
                action.setMessageValue(STAXUtil.parseAndCompileForPython(
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

        if (lowerRequest.startsWith("send"))
            return handleSendRequest(info, staxService);
        else
        {   // Returning nothing in the result indicates that this parser
            // does not support this request.
            return new STAFResult(STAFResult.InvalidRequestString, "");
        }
    }

    private STAFResult handleSendRequest(
        STAFServiceInterfaceLevel30.RequestInfo info, STAX staxService)
    {
        // Verify the requesting machine/user has at least trust level 3

        STAFResult trustResult = STAFUtil.validateTrust(
            3, staxService.getServiceName(), "SEND", 
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        // Parse the request

        STAFCommandParseResult parseResult= fSendParser.parse(info.request);

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

        // Send the message to the STAX Monitor (via an event)
            
        HashMap<String, String> messageMap = new HashMap<String, String>();
        STAXTimestamp timestamp = new STAXTimestamp();

        messageMap.put("messagetext",
                       timestamp.getTimestampString() + " " +
                       STAFUtil.maskPrivateData(message));

        job.generateEvent(STAXMessageActionFactory.STAX_MESSAGE, messageMap);
        
        return new STAFResult(STAFResult.Ok);
    }

    public String getHelpInfo(String lineSep)
    {
        return "SEND      JOB <Job ID> MESSAGE <Message>";
    }
}
