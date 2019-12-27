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

public class STAXParallelIterateActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Parallel Iterate Element ================ -->\n" +
"<!--\n" +
"     The parallel iterate element iterates through a list of items,\n" +
"     performing its contained task while substituting each item in\n" +
"     the list.  The iterated tasks are performed in parallel.\n" +
"-->\n" +
"<!ELEMENT paralleliterate  (%task;)>\n" +
"<!-- var      is the name of a variable which will contain the current\n" +
"              item in the list or tuple being iterated.\n" +
"              It is a literal.\n" +
"     in       is the list or tuple to be iterated.  It is evaluated\n" +
"              via Python and must evaluate to be a list or tuple.\n" +
"     indexvar is the name of a variable which will contain the index of\n" +
"              the current item in the list or tuple being iterated.\n" +
"              It is a literal.  The value of the first index is 0.\n" +
"     maxthreads is the maximum number of STAX-Threads that can be running\n" +
"              simultaneously at a time.  It must evaluate to an integer\n" +
"              value >= 0 via Python.  The default is 0 which means\n" +
"              there is no maximum.\n" +
"-->\n" +
"<!ATTLIST paralleliterate\n" +
"          var        CDATA    #REQUIRED\n" +
"          in         CDATA    #REQUIRED\n" +
"          indexvar   CDATA    #IMPLIED\n" +
"          maxthreads CDATA    \"0\"\n" +
">\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "paralleliterate";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXParallelIterateAction action = new STAXParallelIterateAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        String itemvar  = new String();
        String in       = new String();
        String indexvar = new String();
        String maxthreads = null;
        STAXAction iterateAction = null;
        
        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("var"))
            {
                itemvar = thisAttr.getNodeValue();
            }
            else if (thisAttr.getNodeName().equals("in"))
            {
                in = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action);
            }
            else if (thisAttr.getNodeName().equals("indexvar"))
            {
                indexvar = thisAttr.getNodeValue();
            }
            else if (thisAttr.getNodeName().equals("maxthreads"))
            {
                maxthreads = STAXUtil.parseAndCompileForPython(
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

                iterateAction = factory.parseAction(
                    staxService,job,thisChild);
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

        action.setItemVar(itemvar);
        action.setIn(in);
        action.setIndexVar(indexvar);
        action.setMaxThreads(maxthreads);
        action.setAction(iterateAction);

        return action;
    }
}
