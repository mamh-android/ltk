/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
package com.ibm.staf.service.stax.staxdoc;

import java.io.*;

/******************************************************************************
* Generates main page (index.html) that contains three frames.
******************************************************************************/
public class WriterMainFrame extends HtmlWriter
{
  private static String filename = "index.html";

  /////////////////////////////////////////////////////////////////////////////
  private WriterMainFrame() throws IOException
  {
    super(Main.options.destDir + File.separator + filename, null);
  }

  /////////////////////////////////////////////////////////////////////////////
  public static void generate() throws IOException
  {
    if (Main.options.verbose) System.out.println("Generating " + filename);

    WriterMainFrame framegen;

    framegen = new WriterMainFrame();
    framegen.generateFrameFile();
    framegen.close();

  }

  /////////////////////////////////////////////////////////////////////////////
  protected void generateFrameFile()
  {
    println(ResourceHandler.getText("staxdoc.html.main_frame", Main.options.windowtitle));
  }


}
