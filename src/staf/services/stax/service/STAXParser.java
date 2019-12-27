/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
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

public class STAXParser implements ErrorHandler, EntityResolver
{
    /*
     * Creates a new STAXParser instance passing in a STAX object which
     * represents the STAX service that is constructing the STAXParser.
     */
    public STAXParser(STAX staxService) throws SAXNotRecognizedException,
                                               SAXNotSupportedException
    {
      fSTAX = staxService;

      fParser.setErrorHandler(this);

      fParser.setFeature(
          "http://apache.org/xml/features/dom/include-ignorable-whitespace",
          false);
        
      // Use a custom EntityResolver to intercept the external DTD entity
      //  and replace the DTD with a STAX DTD held in a memory buffer.
      fParser.setEntityResolver(this);
    }


    /**
     * Parses a STAX XML document.
     * 
     * The xmlFile and xmlMachine arguments are needed so that they can be set
     * in STAXJob before the STAXActionFactory parseAction() methods are called
     * so that if an error occurs parsing, these methods have access to this
     * data.
     * 
     * @param  xmlSource  A String containing the XML document
     * @param  xmlFile    A String containing the name of the XML file
     * @param  xmlMachine A String containing the machine where the XML file
     *                    resides
     * @param  job        A STAXJob object (or null if one has not yet been
     *                    created
     * @return an instance of a STAXJob object.
     */
    public STAXJob parse(String xmlSource, String xmlFile, String xmlMachine,
                         STAXJob job) throws STAXException
    {
        long totalStart = 0; // For debugging purposes
        long start = 0;      // For debugging purposes

        if (sDebug)
        {
            totalStart = System.currentTimeMillis();
            System.out.println("Parse file " + xmlFile + " on machine " +
                               xmlMachine);
        }

        try
        {
            // Step 1: Parse with validation to be sure that the XML document
            //         has valid syntax

            if (sDebug)
                start = System.currentTimeMillis();

            fParser.setFeature("http://xml.org/sax/features/validation", true);
            fParser.parse(new InputSource(new StringReader(xmlSource)));

            if (sDebug)
            {
                System.out.println("  - Step 1: Parse with validation: " +
                                   (System.currentTimeMillis() - start));
                start = System.currentTimeMillis();
            }
            
            // Step 2: Transform the XML document to a new XML document that
            //         includes debug information like a _ln attribute for
            //         each element.  Don't do validation.
            
            STAXXMLTransformer fXMLTransformer = new STAXXMLTransformer();
            StringReader transformedXML =
                (StringReader)fXMLTransformer.transform(
                    new StringReader(xmlSource));

            if (sDebug)
            {
                System.out.println("  - Step 2: Transform using XSLT: " +
                                   (System.currentTimeMillis() - start));
                start = System.currentTimeMillis();
            }
            
            // Step 3: Parse the new XML document without validation
            
            fParser.setFeature("http://xml.org/sax/features/validation", false);
            
            fParser.parse(new InputSource(transformedXML));

            if (sDebug)
            {
                System.out.println(
                    "  - Step 3: Parse new XML without validation: " +
                    (System.currentTimeMillis() - start));
                start = System.currentTimeMillis();
            }
            
            if (job == null)
            {
                job = new STAXJob(fSTAX);
                
                job.setXmlFile(xmlFile);
                job.setXmlMachine(xmlMachine);
            }
            
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
                    (thisChild.getNodeName().equals("defaultcall")))
                {
                    handleDefaultFunction(job, thisChild);
                }
                else if (thisChild.getNodeType() == Node.COMMENT_NODE)
                {
                    // Do nothing
                }
                else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
                {
                    STAXActionFactory factory = 
                        fSTAX.getActionFactory(thisChild.getNodeName());

                    if (factory == null)
                    {
                        STAXActionDefaultImpl defaultCallAction =
                            new STAXActionDefaultImpl();
                        defaultCallAction.setLineNumber(thisChild);
                        defaultCallAction.setXmlFile(job.getXmlFile());
                        defaultCallAction.setXmlMachine(job.getXmlMachine());
                        defaultCallAction.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            "No action factory for element type \"" +
                            thisChild.getNodeName() + "\""));

                        throw new STAXInvalidXMLElementCountException(
                            STAXUtil.formatErrorMessage(defaultCallAction),
                            defaultCallAction);
                    }

                    STAXAction action = factory.parseAction(fSTAX, job,
                                                            thisChild);

                    if (thisChild.getNodeName().equals("function"))
                    {
                        job.addFunction((STAXFunctionAction)action);
                    }
                    else if (thisChild.getNodeName().equals("signalhandler"))
                    {
                        job.addDefaultAction(action);
                    }
                    else if (thisChild.getNodeName().equals("script"))
                    {
                        job.addDefaultAction(action);
                    }
                }
            }

            if (sDebug)
            {
                System.out.println("  - Step 4: Call parseActions: " +
                                   (System.currentTimeMillis() - start));
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

        if (sDebug)
        {
            System.out.println("  - Parse Total: " +
                               (System.currentTimeMillis() - totalStart));
        }

        return job;
    }

    public STAXJob parse(String xmlSource, String xmlFile, String xmlMachine) 
                         throws STAXException
    {
        return parse(xmlSource, xmlFile, xmlMachine, null);
    }

    /**
     * Parses a STAX XML document.
     * Called by the above parse method so that debug information like line
     * number is available when a run-time error occurs.
     * @param  xmlSource  InputSource object containing the XML document
     * @return an instance of a STAXJob object
     */
    public STAXJob parse(InputSource xmlSource) throws STAXException
    {
        STAXJob job = null;

        try
        {
            fParser.setFeature("http://xml.org/sax/features/validation", true);

            fParser.parse(xmlSource);

            job = new STAXJob(fSTAX);
            
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
                    (thisChild.getNodeName().equals("defaultcall")))
                {
                    handleDefaultFunction(job, thisChild);
                }
                else if (thisChild.getNodeType() == Node.COMMENT_NODE)
                {
                    // Do nothing
                }
                else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
                {
                    STAXActionFactory factory = 
                        fSTAX.getActionFactory(thisChild.getNodeName());

                    if (factory == null)
                    {
                        throw new STAXInvalidXMLElementException(
                                  thisChild.getNodeName());
                    }

                    STAXAction action = factory.parseAction(fSTAX, job,
                                                            thisChild);

                    if (thisChild.getNodeName().equals("function"))
                    {
                        job.addFunction((STAXFunctionAction)action);
                    }
                    else if (thisChild.getNodeName().equals("signalhandler"))
                    {
                        job.addDefaultAction(action);
                    }
                    else if (thisChild.getNodeName().equals("script"))
                    {
                        job.addDefaultAction(action);
                    }
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

        return job;
    }
    
    private void handleDefaultFunction(STAXJob job,
                                       Node root) throws STAXException
    {
        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            if (thisAttr.getNodeName().equals("function"))
            {
                job.setStartFunction(thisAttr.getNodeValue());

                // Set function arguments to null by default
                job.setStartFuncArgs(null);
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
                STAXCallAction defaultCallAction = new STAXCallAction();
                defaultCallAction.setLineNumber(root);
                defaultCallAction.setXmlFile(job.getXmlFile());
                defaultCallAction.setXmlMachine(job.getXmlMachine());
                defaultCallAction.setElementInfo(new STAXElementInfo(
                    "defaultcall"));

                job.setStartFuncArgs(STAXUtil.parseAndCompileForPython(
                    thisChild.getNodeValue(), defaultCallAction));

                job.setDefaultCallAction(defaultCallAction);
            }
        }
    }

    /**
     *  ErrorHandler interface methods
     */
    public void warning(SAXParseException e)
    {
        // Ignore warnings
    }

    public void error(SAXParseException e) throws SAXException
    {
        throw new SAXException("\nLine " + e.getLineNumber() +
                               ": " + e.getMessage());
    }

    public void fatalError(SAXParseException e) throws SAXException
    {
        throw new SAXException("\nLine " + e.getLineNumber() +
                               ": " + e.getMessage());
    }

    /**
     *  EntityResolver interface methods
     */
    public InputSource resolveEntity(String publicId, String systemId) 
                                 throws java.io.FileNotFoundException,
                                        SAXException
    {
        if ((systemId != null) && systemId.toLowerCase().endsWith("stax.dtd"))
        {
            // Intercept the external DTD entity in the XML document
            // and replace it with a common DTD in memory

            StringReader reader = new StringReader(fSTAX.getDTD());
            return new InputSource(reader); 
        }
        else
        {
            // STAX doesn't support references to external entities (other
            // than the stax.dtd).  The reason why STAX can't support
            // handling external entities without a re-design is because
            // STAX currently does the final parsing of STAX xml documents
            // without validation turned on because it first uses an XSLT
            // transformer to add line numbers/file names to the document so
            // that run-time errors can provide the line number of the
            // element that contains an error and the name of the xml file
            // where it resides.  So, STAX can't parse the final STAX xml
            // document with validation turned on (because line number
            // attributes have been added which aren't in the STAX DTD).
            // When parsing is performed without validation turned on, then
            // external entities are not resolved.
            // Also, STAX supports caching xml documents.  Even if STAX
            // allows resolving external entities, if a STAX xml file
            // contains external entities and it has already been executed and
            // is cached, then if the STAX file is executed again, it won't be
            // re-parsed if it's in the cache (a performance savings).
            // However, if the external entities it references have since
            // changed, they wouldn't be picked up since the cached parsed
            // document would be run (which may not be the expected result)
            
            throw new SAXParseException(
                "References to external entities are not supported.  " +
                "An entity with system identifier '" + systemId +
                "' is referenced by the xml document.", null);
        }
    } 

    STAX fSTAX;
    org.apache.xerces.parsers.DOMParser fParser =
        new org.apache.xerces.parsers.DOMParser();

    private static boolean sDebug = false;
}
