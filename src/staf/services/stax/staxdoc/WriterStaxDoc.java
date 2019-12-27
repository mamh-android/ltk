/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.staxdoc;

import java.io.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.*;
import javax.xml.transform.sax.SAXSource;
import org.xml.sax.helpers.XMLReaderFactory;
import org.xml.sax.InputSource;
import org.xml.sax.EntityResolver;
import org.xml.sax.XMLReader;
import org.xml.sax.SAXException;


/******************************************************************************
* Transforms STAX source files to html using an XSLT transformation
******************************************************************************/
public class WriterStaxDoc extends HtmlWriter
{
  private static StaxFile staxFile = null;
  private static String destFilename = null;
  private static EntityResolver resolver = null;
  
  private WriterStaxDoc() throws IOException
  {
    super(destFilename, null);

    // If an XML document being transformed contains a DOCTYPE line like:
    //    <!DOCTYPE stax SYSTEM "stax.dtd">
    // then when transforming the XML document, the XML parser will try to
    // pull in the DTD from the current directory (even if you turn off 
    // validation and tell it not to resolve external entities).
    // So, instead of requiring a stax.dtd file in every package directory,
    // we're using our own EntityResolver to intercept the external DTD
    // entity in the XML document and replace it with an empty string, which
    // means no validation is done either.
    //
    // XXX: Instead, we could replace the DTD with a location of a valid DTD
    // or could add an option to provide a STAX service/machine and then could
    // submit a STAX GET DTD request to get a string containing the STAX DTD.

    resolver = new EntityResolver()
    {
        public InputSource resolveEntity(String publicId, String systemId) 
        {
            if (systemId.startsWith("file:"))
            {
                // Intercept the external DTD entity in the XML document
                // and replace it with an empty string.
                // Note that if SYSTEM "stax.dtd" is specified, the systemId
                // will be set to file:/<currentDirectory>/stax.dtd

                StringReader reader = new StringReader("");
                return new InputSource(reader); 
            }
            else
            {
                // Use the normal default processing
                return null;
            }
        }
    };
  }

  public static void generate(StaxFile staxFile) throws IOException
  {
    WriterStaxDoc.staxFile = staxFile;
    destFilename = staxFile.getStaxPackage().getDestAbsolutePackageName() +
        File.separator + staxFile.getHtmlFilename();

    WriterStaxDoc doc;
    doc = new WriterStaxDoc();
    doc.generateDoc();
    doc.close();
  }


  private void generateDoc() throws IOException
  {
    if (Main.options.verbose)
        System.out.println("Generating " + staxFile.getAbsoluteHtmlFilename());

    print(ResourceHandler.getText(
        "staxdoc.html.doc1", staxFile.getFileName(),
        staxFile.getStaxPackage().getRelativeOverviewPath()));

    javax.xml.transform.TransformerFactory tFactory =
        javax.xml.transform.TransformerFactory.newInstance();

    try
    {
      // Determine which stylesheet to use (GsxDoc1.xsl or GsxDoc1a.xsl)

      String stylesheetName = "GsxDoc1.xsl";

      if (Main.options.functionSummary.equalsIgnoreCase("All"))
          stylesheetName = "GsxDoc1a.xsl";

      // Open the stylesheet from the jar file

      Transformer transformer;
      
      java.net.URL url = (this.getClass()).getResource(
          "/resources/" + stylesheetName);
      StreamSource s = new StreamSource(url.openStream());
      transformer = tFactory.newTransformer(s);

      if (Main.options.verbose)
          System.out.println("Transforming " + staxFile.getAbsoluteFileName());

      // We want to use our special EntityResolver during the transformation
      // of the XML file to HTML, so we need to create an XMLReader for which
      // we set our EntityResolver.  We then create a SAXSource using our
      // special XMLReader for the XML file, and pass the SAXSource object to
      // the transformer.
      
      XMLReader xmlReader = XMLReaderFactory.createXMLReader();
      xmlReader.setEntityResolver(resolver);
      SAXSource saxSource = new SAXSource(
          xmlReader, new InputSource(staxFile.getAbsoluteFileName()));

      // Transform the XML file HTML using a stylesheet
      transformer.transform(saxSource, new StreamResult(this.out));

      java.net.URL url2 = (this.getClass()).getResource("/resources/GsxDoc2.xsl");
      StreamSource s2 = new StreamSource(url2.openStream());
      transformer = tFactory.newTransformer(s2);

      transformer.transform(saxSource, new StreamResult(this.out));
    }
    catch(Exception e)
    {
      System.err.println("XSLT transformation failed");
      e.printStackTrace();
      System.err.println();
      throw(new IOException());
    }
    print("</body>");
    print("</html>");

  }

}
