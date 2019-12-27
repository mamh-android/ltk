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

public class STAXHoldActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Hold Element ============================= -->\n" +
"<!--\n" +
"     The hold element specifies to hold a block in the job.\n" +
"     If an if attribute is specified and it evaluates via Python to\n" +
"     false, the hold element is ignored.\n" +
"\n" +
"     The default timeout is 0 which specifies to hold the block\n" +
"     indefinitely.  A non-zero timeout value specifies the maximum time\n" +
"     that the block will be held,  The timeout can be expressed in\n" +
"     milliseconds, seconds, minutes, hours, days, or weeks.\n" +
"     It is evaluated via Python.\n" +
"       Examples:  timeout=\"'1000'\"   (1000 milliseconds or 1 second)\n" +
"                  timeout=\"'5s'\"     (5 seconds)\n" +
"                  timeout=\"'1m'\"     (1 minute)\n" +
"                  timeout=\"'2h'\"     (2 hours)\n" +
"                  timeout=\"0\"        (hold indefinitely)\n" +
"-->\n" +
"<!ELEMENT hold       EMPTY>\n" +
"<!ATTLIST hold\n" +
"          block      CDATA    #IMPLIED\n" +
"          if         CDATA    \"1\"\n" +
"          timeout    CDATA    \"0\"\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "hold";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXHoldAction action = new STAXHoldAction();

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
            else if (thisAttr.getNodeName().equals("timeout"))
            {
                action.setTimeout(STAXUtil.parseAndCompileForPython(
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
