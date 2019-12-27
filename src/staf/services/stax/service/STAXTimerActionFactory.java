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

public class STAXTimerActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Timer Element ============================ -->\n" +
"<!--\n" +
"     The timer element runs a task for a specified duration.\n" +
"     If the task is still running at the end of the specified duration,\n" +
"     then the RC variable is set to 1, else if the task ended before\n" +
"     the specified duration, the RC variable is set to 0, else if the\n" +
"     timer could not start due to an invalid duration, the RC variable\n" +
"     is set to -1.\n" +
"-->\n" +
"<!ELEMENT timer     (%task;)>\n" +
"<!-- duration is the maximum length of time to run the task.\n" +
"       Time can be expressed in milliseconds, seconds, minutes,\n" +
"       hours, days, weeks, or years.  It is evaluated via Python.\n" +
"         Examples:  duration='50'    (50 milliseconds)\n" +
"                    duration='90s'   (90 seconds)\n" +
"                    duration='5m'    ( 5 minutes)\n" +
"                    duration='36h'   (36 hours)\n" +
"                    duration='3d'    ( 3 days)\n" +
"                    duration='1w'    ( 1 week)\n" +
"                    duration='1y'    ( 1 year)\n" +
"-->\n" +
"<!ATTLIST timer\n" +
"          duration   CDATA        #REQUIRED\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "timer";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXTimerAction action = new STAXTimerAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        STAXAction timerAction = null;

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("duration"))
            {
                action.setDuration(STAXUtil.parseAndCompileForPython(
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
                if (timerAction != null)
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

                timerAction = factory.parseAction(
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

        action.setTimerAction(timerAction);

        return action;
    }
}
