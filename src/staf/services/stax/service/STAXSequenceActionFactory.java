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

public class STAXSequenceActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Sequence Element ========================= -->\n" +
"<!--\n" +
"     The sequence element performs one or more tasks in sequence.\n" +
"-->\n" +
"<!ELEMENT sequence   (%task;)+>\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "sequence";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXSequenceAction action = new STAXSequenceAction();
        
        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        NodeList children = root.getChildNodes();
        ArrayList<STAXAction> actionList = new ArrayList<STAXAction>();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                STAXActionFactory factory = 
                    staxService.getActionFactory(thisChild.getNodeName());

                if (factory == null)
                {
                    action.setElementInfo(new STAXElementInfo(
                        root.getNodeName(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "No action factory for element type \"" +
                        thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                actionList.add(
                    actionList.size(),
                    factory.parseAction(staxService, job, thisChild));
            }
        }

        action.setActionList(actionList);

        return action;
    }
}
