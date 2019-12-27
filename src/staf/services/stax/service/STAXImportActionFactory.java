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

public class STAXImportActionFactory implements STAXActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Import Element =========================== -->\n" +
"<!--\n" +
"     Allows importing of functions from other STAX XML job file(s).\n" +
"     Either the file or directory attribute must be specified.\n" +
"     All attributes and sub-elements are evaluated via Python.\n" +
"-->\n" +
"<!ELEMENT import         (import-include?, import-exclude?)?>\n" +
"<!ATTLIST import\n" +
"          file           CDATA       #IMPLIED\n" +
"          directory      CDATA       #IMPLIED\n" +
"          machine        CDATA       #IMPLIED\n" +
"          replace        CDATA       \"0\"\n" +
"          mode           CDATA       \"'error'\"\n" +
">\n" + 
"<!ELEMENT import-include (#PCDATA)>\n" +
"<!ELEMENT import-exclude (#PCDATA)>\n" +
"\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "import";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXImportAction staxImport = new STAXImportAction();
            
        staxImport.setLineNumber(root);
        staxImport.setXmlFile(job.getXmlFile());
        staxImport.setXmlMachine(job.getXmlMachine());

        String file = null;
        String directory = null;
        String machine = null;
        
        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            staxImport.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));
            
            if (thisAttr.getNodeName().equals("machine"))
            {
                machine = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), staxImport);
            }
            else if (thisAttr.getNodeName().equals("file"))
            {
                file = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), staxImport);
            }
            else if (thisAttr.getNodeName().equals("directory"))
            {
                directory = STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), staxImport);
            }
            else if (thisAttr.getNodeName().equals("mode"))
            {
                staxImport.setMode(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), staxImport));
            }
            else if (thisAttr.getNodeName().equals("replace"))
            {
                staxImport.setReplace(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), staxImport));
            }
        }

        if ((file != null) && (directory != null))
        {
            staxImport.setElementInfo(new STAXElementInfo(
                root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                "Only one of the following attributes are allowed: " +
                "file, directory"));

            throw new STAXInvalidXMLAttributeException(
                STAXUtil.formatErrorMessage(staxImport), staxImport);
        }
        else if ((file == null) && (directory == null))
        {
            staxImport.setElementInfo(new STAXElementInfo(
                root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                "One of the following attributes must be specified: " +
                "file, directory"));

            throw new STAXInvalidXMLAttributeException(
                STAXUtil.formatErrorMessage(staxImport), staxImport);
        }

        staxImport.setFile(file);
        staxImport.setDirectory(directory);
        staxImport.setMachine(machine);
        
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
                staxImport.setLineNumber(thisChild);

                if (thisChild.getNodeName().equals("import-include"))
                {
                    if (directory != null)
                    {
                        staxImport.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            "Cannot specify \"import-include\" element if " +
                            "the \"directory\" attribute is specified for " +
                            "the \"import\" element."));

                        throw new STAXInvalidXMLElementException(
                            STAXUtil.formatErrorMessage(staxImport), staxImport);
                    }

                    staxImport.setImportInclude(
                        handleChild(thisChild, staxImport));
                }
                else if (thisChild.getNodeName().equals("import-exclude"))
                {
                    if (directory != null)
                    {
                        staxImport.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            "Cannot specify \"import-exclude\" element if " +
                            "the \"directory\" attribute is specified for " +
                            "the \"import\" element."));

                        throw new STAXInvalidXMLElementException(
                            STAXUtil.formatErrorMessage(staxImport), staxImport);
                    }

                    staxImport.setImportExclude(
                        handleChild(thisChild, staxImport));
                }
            }
            else
            {
                staxImport.setElementInfo(new STAXElementInfo(
                    root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(staxImport), staxImport);
            }
        }

        return staxImport;
    }
    
    private String handleChild(Node root, STAXImportAction action)
                               throws STAXException
    {
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            // XXX: Should I be able to have a COMMENT_NODE here?

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
            else if (thisChild.getNodeType() == Node.CDATA_SECTION_NODE)
            {
                /* Do nothing */
            }
        }

        return new String();
    }

}