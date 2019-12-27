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
* Generates upper-left frame overview (overview-frame.html).
******************************************************************************/
public class WriterOverviewFrame extends HtmlWriter
{
  private static String filename = "overview-frame.html";

  /////////////////////////////////////////////////////////////////////////////
  private WriterOverviewFrame() throws IOException
  {
    super(Main.options.destDir + File.separator + filename, null);
  }

  /////////////////////////////////////////////////////////////////////////////
  public static void generate() throws IOException
  {
    if (Main.options.verbose) System.out.println("Generating " + filename);

    WriterOverviewFrame doc;
    doc = new WriterOverviewFrame();
    doc.generateDoc();
    doc.close();
  }


  /////////////////////////////////////////////////////////////////////////////
  private void generateDoc() throws IOException
  {
    print(ResourceHandler.getText("staxdoc.html.overview-frame1"));

    Iterator packageFilesIt = Main.srcFiles.getPackageIterator();
    while (packageFilesIt.hasNext())
    {
      StaxPackage staxFile = (StaxPackage)packageFilesIt.next();
      println("<BR><A HREF=\"" + staxFile.getPath() + "/package-frame.html\" TARGET=\"packageFrame\">" + staxFile.getName() + "</A>");
    }
    print(ResourceHandler.getText("staxdoc.html.overview-frame2"));
  }

}
