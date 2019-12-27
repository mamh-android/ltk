/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
package com.ibm.staf.service.stax.staxdoc;

import java.util.*;
import java.io.*;

/******************************************************************************
* Generates lower-left frame files list (allfiles-frame.html).
******************************************************************************/
public class WriterAllFilesFrame extends HtmlWriter
{
  private static String filename = "allfiles-frame.html";

  /////////////////////////////////////////////////////////////////////////////
  private WriterAllFilesFrame() throws IOException
  {
    super(Main.options.destDir + File.separator + filename, null);
  }

  /////////////////////////////////////////////////////////////////////////////
  public static void generate() throws IOException
  {
    if (Main.options.verbose) System.out.println("Generating " + filename);

    WriterAllFilesFrame doc;

    doc = new WriterAllFilesFrame();
    doc.generateDoc();
    doc.close();

  }


  /////////////////////////////////////////////////////////////////////////////
  private void generateDoc() throws IOException
  {
    print(ResourceHandler.getText("staxdoc.html.allfiles-frame1"));

    Iterator itFile = Main.srcFiles.getFilesIterator();
    while (itFile.hasNext())
    {
      StaxFile staxFile = (StaxFile)itFile.next();
      println("<BR><A HREF=\"" + staxFile.getStaxPackage().getPath() + "/" + staxFile.getHtmlFilename() + "\" TARGET=\"filesFrame\">" + staxFile.getFileName() + "</A>");
    }
    print(ResourceHandler.getText("staxdoc.html.allfiles-frame2"));
  }

}
