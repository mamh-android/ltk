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
public class StaxFile implements Comparable
{
  private StaxPackage staxPackage = null;
  private String name = null;

  /////////////////////////////////////////////////////////////////////////////

  StaxFile(StaxPackage staxPackage, String fileName)
  {
    this.staxPackage = staxPackage;
	  this.name = fileName;
  }

  /////////////////////////////////////////////////////////////////////////////

  public String getFileName()
  {
    return name;
  }

  public StaxPackage getStaxPackage()
  {
    return staxPackage;
  }

  /////////////////////////////////////////////////////////////////////////////

  public String getRelativePath()
  {
    return staxPackage.getPath();
  }

  public String getRelativeFileName()
  {
    return getRelativePath() + File.separator + name;
  }

  public String getAbsolutePath()
  {
    int lastsepidx = getAbsoluteFileName().lastIndexOf(File.separator);
    return getAbsoluteFileName().substring(0, lastsepidx);
  }

  public String getAbsoluteFileName()
  {
    return staxPackage.getAbsolutePackageName() + File.separator + name;
  }

  /////////////////////////////////////////////////////////////////////////////

  public String getHtmlFilename()
  {
    return name.substring(0, name.indexOf(".xml")) + ".html";
  }

  public String getAbsoluteHtmlFilename()
  {
    return getAbsolutePath() + File.separator + getHtmlFilename();
  }

  /////////////////////////////////////////////////////////////////////////////

  public int compareTo(Object o)
  {
    String s1 = ((StaxFile)this).staxPackage.getPath() + ((StaxFile)this).name;
    String s2 = ((StaxFile)o).staxPackage.getPath() + ((StaxFile)o).name;
    return s1.compareTo(s2);
  }

}
