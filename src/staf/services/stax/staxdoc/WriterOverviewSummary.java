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
* Generates main overview (overview-summary.html).
******************************************************************************/
public class WriterOverviewSummary extends HtmlWriter
{
  private static String filename = "overview-summary.html";

  /////////////////////////////////////////////////////////////////////////////
  private WriterOverviewSummary() throws IOException
  {
    super(Main.options.destDir + File.separator + filename, null);
  }


  /////////////////////////////////////////////////////////////////////////////
  public static void generate() throws IOException
  {
    if (Main.options.verbose) System.out.println("Generating " + filename);

    WriterOverviewSummary framegen;

    framegen = new WriterOverviewSummary();
    framegen.generateFrameFile();
    framegen.close();
  }


  /////////////////////////////////////////////////////////////////////////////
  protected void generateFrameFile() throws IOException
  {
    println(ResourceHandler.getText("staxdoc.html.overview-summary1", Main.options.windowtitle, Main.options.doctitle));

    print("<TABLE BORDER=\"1\" CELLPADDING=\"3\" CELLSPACING=\"0\" WIDTH=\"100%\">");
    print("<TR BGCOLOR=\"#CCCCFF\">");
    print("  <TD COLSPAN=\"1\"><FONT SIZE=\"+2\"><B>Packages Summary</B></FONT></TD>");
    print("</TR>");

    Iterator itPackage = Main.srcFiles.getPackageIterator();
    while (itPackage.hasNext())
    {
      StaxPackage staxPackage = (StaxPackage)itPackage.next();
      print("<TR>");
      println("<TD><CODE><B><A HREF=\"" + staxPackage.getPath() + "/package-overview.html\" TARGET=\"filesFrame\">" + staxPackage.getName() + "</A></B></CODE></TD>");
      print("</TR>");
    }

    print("</TABLE>");
    print("<BR></BR>");

    if (Main.options.overviewFilename != null)
      insertBodySection(Main.options.srcPathString + File.separator + Main.options.overviewFilename);

    println(ResourceHandler.getText("staxdoc.html.overview-summary2", Main.options.windowtitle));
  }

}
