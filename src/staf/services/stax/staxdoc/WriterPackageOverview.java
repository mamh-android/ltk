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
public class WriterPackageOverview extends HtmlWriter
{
  private static String filename = "package-overview.html";
  private static StaxPackage staxPackage = null;

  private WriterPackageOverview() throws IOException
  {
    super(staxPackage.getDestAbsolutePackageName() + File.separator + filename, null);
  }

  public static void generate(StaxPackage staxPackage) throws IOException
  {
    if (Main.options.verbose) System.out.println("Generating " + filename + " - " + staxPackage.getPath());

    WriterPackageOverview.staxPackage = staxPackage;

    WriterPackageOverview doc;
    doc = new WriterPackageOverview();
    doc.generateDoc();
    doc.copyDocFiles();
    doc.close();
  }


  private void generateDoc() throws IOException
  {
    print(ResourceHandler.getText("staxdoc.html.package-overview1", staxPackage.getName(), staxPackage.getRelativeOverviewPath()));

    println("<TABLE BORDER=\"1\" CELLPADDING=\"3\" CELLSPACING=\"0\" WIDTH=\"100%\">");
    println("  <TR BGCOLOR=\"#CCCCFF\">");
    println("    <TD COLSPAN=\"1\"><FONT SIZE=\"+2\"><B>Files Summary</B></FONT></TD>");
    println("  </TR>");

    Iterator itFile = staxPackage.getFilesIterator();
    while (itFile.hasNext())
    {
      StaxFile staxFile = (StaxFile)itFile.next();
      println("  <TR>");
      println("    <TD><CODE><B><A HREF=\"" + staxFile.getHtmlFilename() + "\" TARGET=\"filesFrame\">" + staxFile.getFileName() + "</A></B></CODE></TD>");
      println("  </TR>");
    }

    println("</TABLE>");
    println("<BR>");

    String packageCommentFile = staxPackage.getSourcePath() + File.separator +
                                staxPackage.getPath() + File.separator +
                                "package.html";
    File f = new File(packageCommentFile);
    if (f.exists())
      insertBodySection(packageCommentFile);

    print(ResourceHandler.getText("staxdoc.html.package-overview2"));
  }

  /****************************************************************************
  * Copy the "doc-files" directory (if exists) contents from the source
  * package directory to the generated documentation directory.
  ****************************************************************************/
  private void copyDocFiles() throws IOException
  {
    String srcDocFilesDir = staxPackage.getAbsolutePackageName() + File.separator + "doc-files";
    File fileSrcDocFilesDir = new File(srcDocFilesDir);

    if (!fileSrcDocFilesDir.exists()) return;

    String destDocFilesDir = staxPackage.getDestAbsolutePackageName() + File.separator + "doc-files";
    File fileDestDocFilesDir = new File(destDocFilesDir);

    try
    {
      if (!fileDestDocFilesDir.exists())
      {
        if (!fileDestDocFilesDir.mkdirs())
        {
          System.out.println("Unable to create " + destDocFilesDir + " directory");
          throw new IOException();
        }
      }

      String[] files = fileSrcDocFilesDir.list();
      for (int i = 0; i < files.length; i++)
      {
        File srcfile = new File(fileSrcDocFilesDir, files[i]);
        File destfile = new File(fileDestDocFilesDir, files[i]);
        if (srcfile.isFile())
        {
          if (Main.options.verbose) System.out.println("Copying_File " + srcfile.toString() + " to directory " + fileDestDocFilesDir.toString());
          copyFile(destfile, srcfile);
        }
      }
    }
    catch (SecurityException exc)
    {
      throw new IOException();
    }
  }
  /****************************************************************************
  * Copies a file.
  ****************************************************************************/
  private void copyFile(File destFile, File srcFile) throws IOException
  {
    byte[] bytearr = new byte[512];
    int len = 0;
    FileInputStream input = new FileInputStream(srcFile);
    FileOutputStream output = new FileOutputStream(destFile);
    try
    {
      while ((len = input.read(bytearr)) != -1)
      {
        output.write(bytearr, 0, len);
      }
    }
    finally
    {
      input.close();
      output.close();
    }
  }

}
