/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.staxdoc;

import java.io.*;

/******************************************************************************
* Helper class to write HTML files.
******************************************************************************/
public class HtmlWriter extends PrintWriter
{
  protected final String htmlFilename;


  public HtmlWriter(String filename, String docencoding) throws IOException, UnsupportedEncodingException
  {
    super(genWriter(filename, docencoding));
    htmlFilename = filename;
  }

  public static Writer genWriter(String filename, String docencoding) throws IOException, UnsupportedEncodingException
  {
    FileOutputStream fos;
    fos = new FileOutputStream(filename);

    if (docencoding == null)
    {
      OutputStreamWriter oswriter = new OutputStreamWriter(fos);
      docencoding = oswriter.getEncoding();
      return oswriter;
    }
    else
    {
      return new OutputStreamWriter(fos, docencoding);
    }
  }

  /****************************************************************************
  * Copies the content of the BODY section of the specified file
  * without the BODY start/end tags.
  ****************************************************************************/
  public void insertBodySection(String filename) throws IOException
  {
    String str;
    File packageCommentFile = new File(filename);
    Reader r = new FileReader(packageCommentFile);
    BufferedReader in = new BufferedReader(r);

    while ((str = in.readLine()) != null)
    {
      if ((str.trim()).equalsIgnoreCase("<BODY>")) break;
    }
    if (str == null)
    {
      System.err.println("Cannot find <BODY> tag into package.html file");
      System.err.println("The <BODY> tag must be alone on a single line");
      throw new IOException("Cannot find <BODY> tag into package.html file");
    }

    while ((str = in.readLine()) != null)
    {
      if ((str.trim()).equalsIgnoreCase("</BODY>")) break;
      println(str);
    }
    if (str == null)
    {
      System.err.println("Cannot find </BODY> tag into package.html file");
      System.err.println("The </BODY> tag must be alone on a single line");
      throw new IOException("Cannot find </BODY> tag into package.html file");
    }
  }


}
