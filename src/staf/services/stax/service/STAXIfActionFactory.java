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
import java.util.List;
import java.util.ArrayList;

public class STAXIfActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Conditional Element (if-then-else) ======= -->\n" +
"<!--\n" +
"     Allows you to write an if or a case construct with zero or more\n" +
"     elseifs and one or no else statements.\n" +
"\n" +
"     The expr attribute value is evaluated via Python and must evaluate\n" +
"     to a boolean value.\n" +
"-->\n" +
"<!ELEMENT if         ((%task;), elseif*, else?)>\n" +
"<!ATTLIST if\n" +
"          expr       CDATA   #REQUIRED\n" +
">\n" +
"<!ELEMENT elseif     (%task;)>\n" +
"<!ATTLIST elseif\n" +
"          expr       CDATA   #REQUIRED\n" +
">\n" +
"<!ELEMENT else       (%task;)>\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "if";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXIfAction action = new STAXIfAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        STAXAction ifAction = null;
        List<STAXElseIf> elseifs = new ArrayList<STAXElseIf>();
        STAXAction elseAction = null;
 
        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);
            
            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("expr"))
            {
                action.setIfExpression(STAXUtil.parseAndCompileForPython(
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
                action.setLineNumber(thisChild);

                if (thisChild.getNodeName().equals("elseif"))
                {
                    // Add STAXElseIf object to elseifs List
                    elseifs.add(handleElseifChild(
                        staxService, job, thisChild, action));
                } 
                else if (thisChild.getNodeName().equals("else"))
                {
                    elseAction = handleElseChild(
                        staxService, job, thisChild, action);
                }
                else  // If Action  
                {
                    STAXActionFactory factory = 
                        staxService.getActionFactory(thisChild.getNodeName());

                    if (factory == null)
                    {
                        action.setElementInfo(new STAXElementInfo(
                            root.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "No action factory for element type \"" +
                            thisChild.getNodeName() + "\""));

                        throw new STAXInvalidXMLElementException(
                            STAXUtil.formatErrorMessage(action), action);
                    }

                    ifAction = factory.parseAction(
                        staxService, job, thisChild);
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

        action.setIfAction(ifAction);
        action.setElseifs(elseifs);
        action.setElseAction(elseAction);

        return action;
    }

    private STAXElseIf handleElseifChild(STAX staxService, STAXJob job, 
                                         Node root, STAXIfAction action)
                                         throws STAXException
    {
        String elseifExpression = new String();
        STAXAction elseifAction = null;
 
        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName(),
                STAXElementInfo.LAST_ELEMENT_INDEX));

            if (thisAttr.getNodeName().equals("expr"))
            {
                elseifExpression = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
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
                STAXActionFactory factory = 
                    staxService.getActionFactory(thisChild.getNodeName());

                if (factory == null)
                {
                    action.setElementInfo(new STAXElementInfo(
                        root.getNodeName(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        "No action factory for element type \"" +
                        thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                elseifAction = factory.parseAction(
                    staxService, job, thisChild);
            }
        }

        return new STAXElseIf(elseifExpression, elseifAction);
    }

    private STAXAction handleElseChild(STAX staxService, STAXJob job,
                                       Node root, STAXIfAction action)
                                       throws STAXException
    {
        STAXAction elseAction = null;

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
                STAXActionFactory factory = 
                    staxService.getActionFactory(thisChild.getNodeName());

                if (factory == null)
                {
                    action.setElementInfo(new STAXElementInfo(
                        root.getNodeName(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        "No action factory for element type \"" +
                        thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementException(
                        STAXUtil.formatErrorMessage(action), action);
                } 

                elseAction = factory.parseAction(staxService, job, thisChild);
            }
        }

        return elseAction;
    }
}
