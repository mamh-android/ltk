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

public class STAXLoopActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Loop Element ============================= -->\n" +
"<!--\n" +
"     The loop element performs a task a specified number of times,\n" +
"     allowing specification of an upper and lower bound with an\n" +
"     increment value and where the index counter is available to\n" +
"     sub-tasks.  Also, while and/or until expressions can be\n" +
"     specified.\n" +
"-->\n" +
"<!ELEMENT loop       (%task;)>\n" +
"<!-- var      is the name of a variable which will contain the loop\n" +
"              index variable.  It is a literal.\n" +
"     from     is the starting value of the loop index variable.\n" +
"              It must evaluate to an integer value via Python.\n" +
"     to       is the maximum value of the loop index variable\n" +
"              It must evaluate to an integer value via Python.\n" +
"     by       is the increment value for the loop index variable\n" +
"              It must evaluate to an integer value via Python.\n" +
"     while    is an expression that must evaluate to a boolean value\n" +
"              and is performed at the top of each loop.  If it\n" +
"              evaluates to false, it breaks out of the loop.\n" +
"     until    is an expression that must evaluate to a boolean value\n" +
"              and is performed at the bottom of each loop.  If it\n" +
"              evaluates to false, it breaks out of the loop.\n" +
"-->\n" +
"<!ATTLIST loop\n" +
"          var        CDATA    #IMPLIED\n" +
"          from       CDATA    '1'\n" +
"          to         CDATA    #IMPLIED\n" +
"          by         CDATA    '1'\n" +
"          while      CDATA    #IMPLIED\n" +
"          until      CDATA    #IMPLIED\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "loop";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXLoopAction action = new STAXLoopAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        String indexvar   = null;
        String from       = null;
        String to         = null;
        String by         = null;
        String in_while   = null;
        String until      = null;
        STAXAction loopAction = null;

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("var"))
            {
                indexvar = thisAttr.getNodeValue();
            }
            else if (thisAttr.getNodeName().equals("from"))
            {
                from = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
            }
            else if (thisAttr.getNodeName().equals("to"))
            {
                to = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
            } 
            else if (thisAttr.getNodeName().equals("by"))
            {
                by = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
            }
            else if (thisAttr.getNodeName().equals("while"))
            {
                in_while = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
            } 
            else if (thisAttr.getNodeName().equals("until"))
            {
                until = STAXUtil.parseAndCompileForPython(
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

                loopAction = factory.parseAction(staxService, job, thisChild);
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

        action.setIndexVar(indexvar);
        action.setFrom(from);
        action.setTo(to);
        action.setBy(by);
        action.setWhile(in_while);
        action.setUntil(until);
        action.setAction(loopAction);

        return action;
    }
}
