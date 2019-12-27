/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.apache.xalan.processor.TransformerFactoryImpl;
import org.apache.xalan.transformer.TransformerImpl;
import org.apache.xalan.transformer.XalanProperties;
import javax.xml.transform.*;
import javax.xml.transform.stream.*;
import javax.xml.transform.sax.SAXSource;
import javax.xml.parsers.SAXParserFactory;
import org.xml.sax.helpers.XMLReaderFactory;
import org.xml.sax.XMLReader;
import org.xml.sax.InputSource;
import org.xml.sax.EntityResolver;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Reader;

public class STAXXMLTransformer implements EntityResolver
{
    /**
     * Create a transformer to transform a XML document into a new XML
     * document where each element now has a _lineno attribute containing
     * the line number where the element is located in the XML document.
     */
    public STAXXMLTransformer() throws STAXException
    {
        // Directly instantiate the Xalan transformer instead of doing:
        //    fFactory = TransformerFactory.newInstance();
        // to ensure that the Xalan transformer is used and not whatever
        // XSLT transformer is configured for the JVM
        fFactory = new org.apache.xalan.processor.TransformerFactoryImpl();

        fFactoryImpl = (TransformerFactoryImpl)fFactory;
        
        fFactoryImpl.setAttribute(XalanProperties.SOURCE_LOCATION,
                                  Boolean.TRUE);

        // Open the stylesheet in the STAX.jar file to use to transform the
        // XML document

        String stylesheetName = "/resources/STAXGetLineNumber.xsl";

        java.net.URL url = this.getClass().getResource(stylesheetName);

        if (url == null)
        {
            throw new STAXException(
                "TransformerException:  Cannot find stylesheet " +
                stylesheetName + " in the STAX.jar file");
        }

        try
        {
            fTransformer = fFactory.newTransformer(
                new StreamSource(url.openStream()));
        }
        catch (java.io.IOException e)
        {
            throw new STAXException("IOError: " + e.getMessage());
        }
        catch (javax.xml.transform.TransformerException te)
        {
            throw new STAXException(te.toString());
        }

        fImpl = (TransformerImpl)fTransformer;
        fImpl.setProperty(XalanProperties.SOURCE_LOCATION, Boolean.TRUE);

        // Create an XMLReader with a custom EntityResolver to intercept
        // the external DTD entity and replace the DTD with an empty string

        try
        {
            SAXParserFactory pfactory = SAXParserFactory.newInstance();
            pfactory.setNamespaceAware(true);
            pfactory.setValidating(false);
            fXmlReader = pfactory.newSAXParser().getXMLReader();
        }
        catch (javax.xml.parsers.ParserConfigurationException pce)
        {
            throw new STAXException(pce.toString());
        }
        catch (org.xml.sax.SAXException se)
        {
            throw new STAXException("SAXException: " + se.getMessage()); 
        }

        fXmlReader.setEntityResolver(this);
    }

    /**
     * Transform the XML document into a new XML document where each element
     * has a _lineno attribute containing the line number of the element.
     * @param xmlSource A Reader containing the XML document to be transformed
     * @return A Reader containing the transformed XML document
     */
    public Reader transform(Reader xmlSource) throws STAXException
    {
        StringWriter sw = new StringWriter();
        
	try
	{
            // Note:  Using SAXSource object so that we can use an XML
            // reader with an Entity Resolver to ignore the DTD.

            SAXSource saxSource = new SAXSource(
                fXmlReader, new InputSource(xmlSource));
            
            fTransformer.transform(saxSource, new StreamResult(sw));
        }
        catch (javax.xml.transform.TransformerException te)
        {
            throw new STAXException(te.toString());
        }

        if (sw.getBuffer().length() == 0)
        {
            // If using some the GNU libgcj JVM (on Linux machines), the
            // the transform() method doesn't throw an exception but the
            // resulting transformed document is empty and it writes the
            // following error to the JVM log:
            //
            //   SystemId Unknown; Line #1; Column #0; null
            //
            // Since it's not throwing an exception, had to add this hack
            // to check if the transformed document has length 0 so that
            // it won't fail later when parsing the transformed document
            // with the following STAXXMLParserException:
            //   Line -1: Premature end of file.

            String errMsg = "The following JVM is not supported:" +
                "\n  java vendor: " +
                System.getProperty("java.vendor", "<unknown vendor>") +
                "\n  java version: " +
                System.getProperty("java.version", "<unknown version") +
                "\n  java runtime name: " +
                System.getProperty("java.runtime.name",
                                   "<unknown runtime name>") +
                "\n  java runtime version " +
                System.getProperty("java.runtime.version",
                                   "<unknown runtime version>") +
                "\n  java vm vendor: " +
                System.getProperty("java.vm.vendor", "<unknown vm vendor>") +
                "\n  java vm name: " +
                System.getProperty("java.vm.name", "<unknown java vm name>") +
                "\n  java vm version " +
                System.getProperty("java.vm.version", "<unknown vm version>");

            throw new STAXException(errMsg);
        }

        return new StringReader(sw.getBuffer().toString());
    }

    /**
     *  EntityResolver interface methods
     */
    public InputSource resolveEntity(String publicId, String systemId) 
                                     throws java.io.FileNotFoundException
    {
        // Intercept the external DTD entity in the XML document
        // and replace it with an empty string (as no validation is
        // needed since it's already been done).

        StringReader reader = new StringReader("");
        return new InputSource(reader); 
    }

    private TransformerFactory fFactory;
    private TransformerFactoryImpl fFactoryImpl;
    private Transformer fTransformer;
    private TransformerImpl fImpl;
    private XMLReader fXmlReader;
}
