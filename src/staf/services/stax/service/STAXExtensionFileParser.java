/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2003                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.xml.sax.ErrorHandler;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.SAXNotRecognizedException;
import org.xml.sax.SAXNotSupportedException;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import java.io.*;
import java.util.ArrayList;
import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFUtil;

public class STAXExtensionFileParser implements ErrorHandler, EntityResolver
{
    public STAXExtensionFileParser(STAX staxService)
           throws SAXNotRecognizedException, SAXNotSupportedException
    {
      fSTAX = staxService;

      fParser.setErrorHandler(this);

      fParser.setFeature(
          "http://apache.org/xml/features/dom/include-ignorable-whitespace",
          false);

      fParser.setFeature("http://xml.org/sax/features/validation", true);

      // Use a custom EntityResolver to intercept the external DTD entity
      //  and replace the DTD with a STAX Extension File DTD held in a
      //  memory buffer.
      fParser.setEntityResolver(this);
    }

    public String getDTD() { return fExtensionFileDTD; }

    public void parse(InputSource xmlSource) throws STAXException
    {
        ArrayList extJarList = new ArrayList();

    try
    {
        fParser.parse(xmlSource);

            Node root = fParser.getDocument().getDocumentElement();

            root.normalize();

            NamedNodeMap attrs = root.getAttributes();

            for (int i = 0; i < attrs.getLength(); ++i)
            {
                Node thisAttr = attrs.item(i);
            }

            NodeList children = root.getChildNodes();

            for (int i = 0; i < children.getLength(); ++i)
            {
                Node thisChild = children.item(i);

                if ((thisChild.getNodeType() == Node.ELEMENT_NODE) &&
                    (thisChild.getNodeName().equals("extension")))
                {
                    fSTAX.setExtension(handleExtension(thisChild));
                }
                else if (thisChild.getNodeType() == Node.COMMENT_NODE)
                {
                    /* Do nothing */
                }
                else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
                {
                    throw new STAXInvalidXMLElementException(
                              thisChild.getNodeName());
                }
                else
                {
                    throw new STAXInvalidXMLNodeTypeException(
                              Integer.toString(thisChild.getNodeType()));
                }
            }
    }
    catch (java.io.IOException e)
    {
            throw new STAXException("IOError: " + e.getMessage());
    }
    catch (SAXException e)
    {
            throw new STAXXMLParseException(e.getMessage());
    }
    }

    private STAXExtension handleExtension(Node root) throws STAXException
    {
        STAXExtension ext = new STAXExtension();

        NamedNodeMap rootAttrs = root.getAttributes();

        for (int i = 0; i < rootAttrs.getLength(); ++i)
        {
            Node thisAttr = rootAttrs.item(i);

            if (thisAttr.getNodeName().equals("jarfile"))
            {
                // Resolve any STAF variables specified for jarfile

                String jarFileName = thisAttr.getNodeValue();

                // Resolve the machine name variable for the local machine

                STAFResult result = STAFUtil.resolveInitVar(
                    jarFileName, fSTAX.getSTAFHandle());

                if (result.rc != STAFResult.Ok)
                {
                    throw new STAXException(
                        "Error resolving jarfile attribute value.  " +
                        "RC=" + result.rc + " Result=" + result.result);
                }

                jarFileName = result.result;

                ext.setJarFileName(jarFileName);
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
                if (thisChild.getNodeName().equals("parameter"))
                {
                    String name = null;
                    String value = null;

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();

                        }
                        else if (thisAttr.getNodeName().equals("value"))
                        {
                            value = thisAttr.getNodeValue();

                        }
                    }

                    if (!ext.setParm(name, value))
                    {
                        throw new STAXInvalidXMLElementException(
                            "<parameter name=\"" + name + "\" value=\"" +
                            value + "\"/> for extension jar file " +
                            ext.getJarFileName() + fSTAX.lineSep +
                            "Cannot specify parameters with the same " +
                            "name for an extension");
                    }
                }
                else if (thisChild.getNodeName().equals("include-element"))
                {
                    String name = null;

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();
                        }
                    }

                    if (!ext.setIncludeElement(name))
                    {
                        throw new STAXInvalidXMLElementException(
                            "<include-element name=\"" + name +
                            "\"/> for extension jar file " +
                            ext.getJarFileName() + fSTAX.lineSep +
                            "Cannot specify include elements with the " +
                            "same name for an extension");
                    }
                }
                else if (thisChild.getNodeName().equals("exclude-element"))
                {
                    String name = null;

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();
                        }
                    }

                    if (!ext.setExcludeElement(name))
                    {
                        throw new STAXInvalidXMLElementException(
                            "<exclude-element name=\"" + name +
                            "\"/> for extension jar file " +
                            ext.getJarFileName() + fSTAX.lineSep +
                            "Cannot specify exclude elements with the " +
                            "same name for an extension");
                    }
                }
            }
            else
            {
                throw new STAXInvalidXMLNodeTypeException(
                          Integer.toString(thisChild.getNodeType()));
            }
        }

        return ext;
    }

    // ErrorHandler interface methods

    public void warning(SAXParseException e)
    {
        // Ignore warnings
    }

    public void error(SAXParseException e) throws SAXException
    {
        throw new SAXException(fSTAX.lineSep + "Line " + e.getLineNumber() +
                               ": " + e.getMessage());
    }

    public void fatalError(SAXParseException e) throws SAXException
    {
        throw new SAXException(fSTAX.lineSep + "Line " + e.getLineNumber() +
                               ": " + e.getMessage());
    }

    // EntityResolver interface methods

    public InputSource resolveEntity(String publicId, String systemId)
                                 throws java.io.FileNotFoundException
    {
        // Intercept the external DTD entity in the XML document
        // and replace it with a common DTD in memory.
        StringReader reader = new StringReader(fExtensionFileDTD);
        return new InputSource(reader);
    }

    private STAX fSTAX;

    private org.apache.xerces.parsers.DOMParser fParser =
    new org.apache.xerces.parsers.DOMParser();

    private String fExtensionFileDTD =
"<!--\n" +
"   STAX Extensions Document Type Definition (DTD)\n" +
"\n" +
"   This DTD module is identified by the SYSTEM identifier:\n" +
"\n" +
"     SYSTEM 'stax-extensions.dtd'\n" +
"\n" +
"   This DTD is used for files specified using the EXTENSIONXMLFILE\n" +
"   parameter when registering the STAX service with extensions.\n" +
"\n" +
"-->\n" +
"\n" +
"<!--================= STAX Extension File Definition =============== -->\n" +
"<!--\n" +
"     The root element extensions contains all other elements.  It\n" +
"     consists of one or more extension elements.\n" +
"-->\n" +
"<!ELEMENT stax-extensions    (extension+)>\n" +
"\n" +
"<!--================= The Extension Element ======================== -->\n" +
"<!--\n" +
"     Specifies a STAX extension.  It can consist of 0 or more\n" +
"     parameter elements, followed by 0 or more include-element or\n" +
"     0 or more exclude-element elements.\n" +
"-->\n" +
"<!ELEMENT extension          (parameter*,\n" +
"                              (include-element* | exclude-element*))>\n" +
"<!ATTLIST extension\n" +
"          jarfile            CDATA   #REQUIRED\n" +
">\n" +
"\n" +
"<!--================= The Parameter Element ======================== -->\n" +
"<!--\n" +
"     Specifies a parameter for a STAX extension.\n" +
"-->\n" +
"<!ELEMENT parameter          EMPTY>\n" +
"<!ATTLIST parameter\n" +
"          name               CDATA   #REQUIRED\n" +
"          value              CDATA   #REQUIRED\n" +
">\n" +
"\n" +
"<!--================= The Include Element ========================= -->\n" +
"<!--\n" +
"     Specifies to only register this element for a STAX extension\n" +
"     instead of registering all elements specified in the extension\n" +
"     jar file's manifest file.\n" +
"-->\n" +
"<!ELEMENT include-element    EMPTY>\n" +
"<!ATTLIST include-element\n" +
"          name               CDATA   #REQUIRED\n" +
">\n" +
"\n" +
"<!--================= The Exclude Element ========================= -->\n" +
"<!--\n" +
"     Specifies to excluce registering this element for a STAX extension\n" +
"     instead of registering all elements specified in the extension\n" +
"     jar file's manifest file.\n" +
"-->\n" +
"<!ELEMENT exclude-element    EMPTY>\n" +
"<!ATTLIST exclude-element\n" +
"          name               CDATA   #REQUIRED\n" +
">\n" +
"\n";

}
