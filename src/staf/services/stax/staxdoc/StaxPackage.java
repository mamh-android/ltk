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
* Represents a source package: directory and contained suorce files.
******************************************************************************/
public class StaxPackage implements Comparable
{
  private String sourcePath = null;
  private String path = null;
  private String name = null;
  private SortedSet srcList = new TreeSet();


  StaxPackage(String sourcePath, String pkgPath, String pkgName)
  {
    pkgPath = pkgPath.replace('\\', '/');
    if(pkgPath.endsWith("/") || pkgPath.endsWith("\\"))
      pkgPath = pkgPath.substring(0, pkgPath.length()-1);

    this.sourcePath = sourcePath;
    this.path = pkgPath;
    this.name = pkgName;
  }

  /////////////////////////////////////////////////////////////////////////////
  public String getPath()
  {
    return path;
  }

  /////////////////////////////////////////////////////////////////////////////
  public String getName()
  {
    return name;
  }

  public String getSourcePath()
  {
    return sourcePath;
  }

  /////////////////////////////////////////////////////////////////////////////

  public String getRelativeOverviewPath()
  {
    String ret = "../";
    for(int i=0; i<name.length(); i++)
    {
      if(name.charAt(i) == '/' || name.charAt(i) == '\\')
        ret += "../";
    }
    return ret;
  }

  /////////////////////////////////////////////////////////////////////////////

  public String getAbsolutePackageName()
  {
    return sourcePath + File.separator + path;
  }

  public String getDestAbsolutePackageName()
  {
    return Main.options.destDir + File.separator + path;
  }

  /////////////////////////////////////////////////////////////////////////////

  public void addFile(StaxFile f)
  {
    srcList.add(f);
  }

  public Iterator getFilesIterator()
  {
    return srcList.iterator();
  }

  /////////////////////////////////////////////////////////////////////////////

  public int compareTo(Object o)
  {
    return (((StaxPackage)this).path).compareTo(((StaxPackage)o).path);
  }


}
