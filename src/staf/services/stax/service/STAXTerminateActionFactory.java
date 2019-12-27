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

public class STAXTerminateActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Terminate Element ======================== -->\n" +
"<!--\n" +
"     The terminate element specifies to terminate a block in the job.\n" +
"     If an if attribute is specified and it evaluates via Python to\n" +
"     false, the terminate element is ignored.\n" +
"-->\n" +
"<!ELEMENT terminate  EMPTY>\n" +
"<!ATTLIST terminate\n" +
"          block      CDATA    #IMPLIED\n" +
"          if         CDATA    \"1\"\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "terminate";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXTerminateAction action = new STAXTerminateAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        String blockName = null;
        String ifAttr = "1";

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));


            if (thisAttr.getNodeName().equals("block"))
            {
                action.setBlockName(STAXUtil.parseAndCompileForPython(
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
            else
            {
                action.setElementInfo(new STAXElementInfo(
                    root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);
            }
        }

        return action;
    }
}
