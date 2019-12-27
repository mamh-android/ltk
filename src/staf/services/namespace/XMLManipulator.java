/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.FileReader;
import org.w3c.dom.Document;
import org.apache.xerces.parsers.DOMParser;
import org.apache.xml.serialize.XMLSerializer;
import org.apache.xml.serialize.OutputFormat;
import org.xml.sax.ErrorHandler;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.SAXNotRecognizedException;
import org.xml.sax.SAXNotSupportedException;

import com.ibm.staf.service.namespace.NSException;

/**
 * This abstract class may be extended by any classes
 * which need to read and/or write XML files. This
 * class will provide load and save methods as well
 * as entity resolution for DTDs contained in the
 * service jar file.
 * <p>
 * This implementation uses the Xerces-J XML APIs.
 * <p>
 * Subclasses must set the DTD and XML file before calling loadDocument().
 * <p>
 * Subclasses must set the XML file before calling saveDocument().
 */
public abstract class XMLManipulator implements EntityResolver, ErrorHandler
{
    private File fXmlFile;
    private String fDtdFile;
    protected Document fDoc;

    // XML Settings
    private static final String XML_VERSION  = "1.0";
    private static final String XML_ENCODING = "UTF-8";
    private static final int    XML_INDENT   = 2;
    
    /**
     * Loads the XML file and returns an DOM XML Document.
     * This method is synchronized to prevent two different
     * threads from attempting to load and save the document
     * at the same time.
     * @throws NSException if an error occurred loading the XML file.
     */
    public synchronized Document loadDocument() throws NSException
    {
        if (fDtdFile == null || fXmlFile == null) 
        {
            throw new NSException(
                "fDtdFile and fXmlFile must be set before loading the " +
                "Document. Please contact service administrators.");
        }
                
        Document doc = null;
        DOMParser fParser = new DOMParser();
        fParser.setErrorHandler(this);
        fParser.setEntityResolver(this);

        try
        {
            fParser.setFeature(
                "http://xml.org/sax/features/validation", true);
            
            fParser.parse(new InputSource(new FileReader(fXmlFile)));
            
            doc = fParser.getDocument();
        }
        catch (IOException ioe)
        {
            throw new NSException(
                "IOException loading XML file '" + fXmlFile.toString() +
                "'. " + ioe.getMessage());
        }
        catch (SAXNotRecognizedException snre)
        {
            throw new NSException(
                "SAXNotRecognizedException loading Namespaces XML file '" +
                fXmlFile.toString() + "'. " + snre.getMessage());
        }
        catch (SAXNotSupportedException snse)
        {
            throw new NSException(
                "SAXNotSupportedException loading Namespaces XML file '" +
                fXmlFile.toString() + "'. " + snse.getMessage());
        }
        catch (SAXException je)
        {
            throw new NSException(
                "XML parse error in Namespaces XML file.  " +
                "Please correct following error in XML file '" +
                fXmlFile.toString() + "': " + je.getMessage());
        }

        this.fDoc = doc;
        return doc;
    }
    
    
    /**
     * Saves the Document in an XML file.
     * This method is synchronized to prevent two different
     * threads from attempting to load and save the document
     * at the same time.
     * @throws NSException if errors ocurred during the save
     */
    public synchronized void saveDocument() throws NSException
    {
        saveDocument(fDoc);
    }
    
    
    /**
     * Saves the Document as an XML file
     * This method is synchronized to prevent two different
     * threads from attempting to load and save the document
     * at the same time.
     * @param doc the Document to save
     * @throws NSException if errors ocurred during the save
     */
    public synchronized void saveDocument(Document doc) throws
        NSException
    {
        if (fXmlFile == null) 
        {
            throw new NSException(
                "fXmlFile must be set before saving the Document. " +
                "Please contact service administrators.");
        }
        
        FileWriter writer = null;

        try
        {
            // Define a Writer
            writer = new FileWriter(fXmlFile);
        }
        catch(IOException ioe)
        {
            throw new NSException(
                "IOException opening Namespaces XML file '" +
                fXmlFile.toString() + "'. " + ioe.getMessage());
        }
        
        try
        {
            // Setup format settings
            OutputFormat outFormat = new OutputFormat(
                doc, XML_ENCODING, true);
            outFormat.setVersion(XML_VERSION);
            outFormat.setDoctype(null, fDtdFile);
            outFormat.setIndent(XML_INDENT);

            // Define Serializer for the XML file using the format settings
            XMLSerializer xmlSerializer = new XMLSerializer(
                writer, outFormat);

            // Serialize XML Document
            xmlSerializer.serialize(doc);
        }
        catch(IOException ioe)
        {
            throw new NSException(
                "IOException saving Namespaces XML file '" +
                fXmlFile.toString() + "'. " + ioe.getMessage());
        }
        finally
        {            
            try 
            {
                writer.close();                
            }
            catch(IOException ioe)
            {
                ioe.printStackTrace();
            }
        }
        
    }
    
    // EntityResolver interface method

    /**
     * Maps the dtd to the xml input.
     * This works for loading XML files and using a DTD contained in the .jar
     * file.
     * @see org.xml.sax.EntityResolver#resolveEntity(java.lang.String, 
     *                                               java.lang.String)
     */
    public InputSource resolveEntity(String publicId, String systemId) throws
        SAXException, IOException
    {
        // Instead of using a DTD file in the jar file, could use a DTD file
        // stored im memory as follows:
        // StringReader in = new StringReader(getDtdFile());

        ClassLoader cl = this.getClass().getClassLoader();
        InputStream in = cl.getResourceAsStream(fDtdFile);     
        
        if (in == null) 
        {            
            throw new IOException("Could not locate " + fDtdFile);
        }

        return new InputSource(in);
    }
    
    // ErrorHandler interface methods

    /**
     * Ignores any warnings during parsing of the XML document
     */ 
    public void warning(SAXParseException e)
    {
        // Ignore warnings
    }

    /**
     * Throws a SAX Exception with the error information including
     * the line number where the parse error occurred
     */ 
    public void error(SAXParseException e) throws SAXException
    {
        throw new SAXException("\nLine " + e.getLineNumber() +
                               ": " + e.getMessage());
    }

    /**
     * Throws a SAX Exception with the error information including
     * the line number where the parse error occurred
     */ 
    public void fatalError(SAXParseException e) throws SAXException
    {
        throw new SAXException("\nLine " + e.getLineNumber() +
                               ": " + e.getMessage());
    }
    
    // Getter / Setters
    
    /**
     * @return Returns the fDtdFile.
     */
    public String getDtdFile()
    {
        return fDtdFile;
    }
    
    /**
     * @param fDtdFile The fDtdFile to set.
     */
    public void setDtdFile(String fDtdFile)
    {
        this.fDtdFile = fDtdFile;
    }
    
    
    /**
     * @return Returns the fXmlFile.
     */
    public File getXmlFile()
    {
        return fXmlFile;
    }
    
    /**
     * @param fXmlFile The fXmlFile to set.
     */
    public void setXmlFile(String fXmlFile)
    {
        this.fXmlFile = new File(fXmlFile);
    }

    /**
     * @return Returns the fDoc.
     */
    public Document getDoc()
    {
        return fDoc;
    }
    /**
     * @param doc The doc to set.
     */
    public void setDoc(Document doc)
    {
        this.fDoc = doc;
    }
}
