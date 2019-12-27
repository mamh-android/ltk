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

public class STAXCallWithListActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Call-With-List Element =================== -->\n" +
"<!--\n" +
"     Perform a function with the referenced name with any number of\n" +
"     arguments in the form of a list.  The function attribute value\n" +
"     and argument values are evaluated via Python.\n" +
"-->\n" +
"<!ELEMENT call-with-list      (call-list-arg*)>\n" +
"<!ATTLIST call-with-list\n" +
"          function   CDATA    #REQUIRED\n" +
">\n" +
"\n" +
"<!ELEMENT call-list-arg       (#PCDATA)>\n" +
"\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "call-with-list";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        String functionName = new String();
        STAXCallAction action = new STAXCallAction();
        
        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        NamedNodeMap rootAttrs = root.getAttributes();

        for (int i = 0; i < rootAttrs.getLength(); ++i)
        {
            Node thisAttr = rootAttrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("function"))
            {
                action.setFunction(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
        }

        // Determine which call element is being processed
        
        action.setCallType(STAXCallAction.CALL_LIST_ARGS);

        // Iterate its children nodes to get arguments passed in (if any)

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
                action.setLineNumber(thisChild);

                if (thisChild.getNodeName().equals("call-list-arg"))
                {
                    // Add argument to argument list
                    action.addListArg(handleChild(thisChild, action));
                }
            }
            else
            {
                action.setElementInfo(new STAXElementInfo(
                    root.getNodeName(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);
            }
        }

        return action;
    }


    String handleChild(Node root, STAXCallAction action) throws STAXException
    {
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
                    root.getNodeName(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX));

                return STAXUtil.parseAndCompileForPython(
                    thisChild.getNodeValue(), action);
            }
        }

        return null;
    }
}
