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

public class STAXTryActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--=============== The Try / Catch / Finally Elements ============= --> \n" +
"<!-- \n" +
"     The try element allows you to perform a task and to catch \n" +
"     exceptions that are thrown.  Also, if a finally element is \n" +
"     specified, then the finally task is executed, no matter whether \n" +
"     the try task completes normally or abruptly, and no matter whether \n" +
"     a catch element is first given control. \n" +
"--> \n" +
"<!ELEMENT try        ((%task;), ((catch+) | ((catch*), finally)))> \n" +
"<!-- \n" +
"     The catch element performs a task when the specified exception is \n" +
"     caught.  The var attribute specifies the name of the variable to \n" +
"     receive the data specified within the throw element.  The typevar \n" +
"     attribute specifies the name of the variable to receive the type \n" +
"     of the exception.  The sourcevar attribute specifies the name\n" +
"     of the variable to receive the source information for the exception.\n" +
" \n" +
"--> \n" +
"<!ELEMENT catch      (%task;)> \n" +
"<!ATTLIST catch \n" +
"          exception  CDATA        #REQUIRED \n" +
"          var        CDATA        #IMPLIED \n" +
"          typevar    CDATA        #IMPLIED \n" +
"          sourcevar  CDATA        #IMPLIED \n" +
"> \n" +
"<!ELEMENT finally    (%task;)> \n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "try";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXTryAction action = new STAXTryAction();
        
        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        STAXAction tryAction = null;
        List<STAXCatchAction> catchList = new ArrayList<STAXCatchAction>();
        
        STAXFinallyAction finallyAction = null;

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

                if (thisChild.getNodeName().equals("catch"))
                {
                    catchList.add(handleCatch(staxService, job, thisChild));
                }
                else if (thisChild.getNodeName().equals("finally"))
                {
                    finallyAction = handleFinally(staxService, job, thisChild);
                }
                else
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

                    tryAction = factory.parseAction(staxService, job, thisChild);
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

        action.setTryAction(tryAction);
        action.setCatchList(catchList);

        if (finallyAction == null)
        {
            return action;
        }
        else
        {
            finallyAction.setTryAction(action);
            return finallyAction;
        }
    }

    private STAXCatchAction handleCatch(STAX staxService, STAXJob job,
                                        Node root) throws STAXException
    {
        STAXCatchAction action = new STAXCatchAction();
        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("exception"))
            {
                action.setExceptionName(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
            else if (thisAttr.getNodeName().equals("var"))
            {
                action.setVarName(thisAttr.getNodeValue());
            }
            else if (thisAttr.getNodeName().equals("typevar"))
            {
                action.setTypeVarName(thisAttr.getNodeValue());
            }
            else if (thisAttr.getNodeName().equals("sourcevar"))
            {
                action.setSourceVarName(thisAttr.getNodeValue());
            }
        }

        STAXAction catchAction = null;

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

                catchAction = factory.parseAction(staxService, job, thisChild);
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

        action.setAction(catchAction);

        return action;
    }

    private STAXFinallyAction handleFinally(STAX staxService, STAXJob job,
                                            Node root) throws STAXException
    {
        STAXFinallyAction action = new STAXFinallyAction();
        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        STAXAction finallyAction = null;
        
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

                finallyAction = factory.parseAction(
                    staxService, job, thisChild);
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

        action.setFinallyAction(finallyAction);

        return action;
    }
}
