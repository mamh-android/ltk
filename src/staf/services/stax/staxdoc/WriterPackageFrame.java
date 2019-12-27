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
*
******************************************************************************/
public class WriterPackageFrame extends HtmlWriter
{
  private static String filename = "package-frame.html";
  private static StaxPackage staxPackage = null;

  private WriterPackageFrame() throws IOException
  {
    super(staxPackage.getDestAbsolutePackageName() + File.separator + filename, null);
  }

  public static void generate(StaxPackage staxPackage) throws IOException
  {
    if (Main.options.verbose) System.out.println("Generating " + filename);

    WriterPackageFrame.staxPackage = staxPackage;

    WriterPackageFrame doc;
    doc = new WriterPackageFrame();
    doc.generateDoc();
    doc.close();
  }


  private void generateDoc() throws IOException
  {
    print(ResourceHandler.getText("staxdoc.html.package-frame1", staxPackage.getName()));

    Iterator itFile = staxPackage.getFilesIterator();
    while (itFile.hasNext())
    {
      StaxFile staxFile = (StaxFile)itFile.next();
      println("<BR><A HREF=\"" + staxFile.getHtmlFilename() + "\" TARGET=\"filesFrame\">" + staxFile.getFileName() + "</A>");
    }
    print(ResourceHandler.getText("staxdoc.html.package-frame2"));
  }

}
