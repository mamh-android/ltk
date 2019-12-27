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

public class STAXTestcaseStatusActionFactory implements STAXActionFactory
{
    static final String STAX_TESTCASE_STATUS = new String("TestcaseStatus");

    private static String fDTDInfo =
"\n" +
"<!--================= The Testcase Status Element ================== -->\n" +
"<!--\n" +
"     Marks status result ('pass' or 'fail' or 'info') for a testcase\n" +
"     and allows additional information to be specified.  The status\n" +
"     result and the additional info is evaluated via Python.\n" +
"-->\n" +
"<!ELEMENT tcstatus   (#PCDATA)>\n" +
"<!ATTLIST tcstatus\n" +
"          result     CDATA  #REQUIRED\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "tcstatus";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXTestcaseStatusAction action = new STAXTestcaseStatusAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("result"))
            {                
                action.setStatus(STAXUtil.parseAndCompileForPython(
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
                action.setElementInfo(new STAXElementInfo(
                    root.getNodeName()));

                action.setMessage(STAXUtil.parseAndCompileForPython(
                    thisChild.getNodeValue(), action));
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

        return action;
    }
}
