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
* Handle the list of STAX source files to be processed.
******************************************************************************/
public class StaxSource
{
  private String sourcePath = null;

  private SortedSet packageList = new TreeSet();
  private SortedSet fileListAll = new TreeSet();


  /////////////////////////////////////////////////////////////////////////////
  public StaxSource(String sourcePath)
  {
    this.sourcePath = sourcePath;
  }

  /////////////////////////////////////////////////////////////////////////////
  public void addSource(String packagePath, String packageName) throws IOException
  {
    File currentFile = new File(sourcePath + File.separator + packagePath);
    if (!currentFile.exists())
    {
      System.out.println("File not found " + currentFile.getPath());
      throw (new IOException());
    }

    if (currentFile.isDirectory())
    {
      if (Main.options.verbose) System.out.println("Package:" + packagePath);
      StaxPackage sp = new StaxPackage(Main.options.srcPathString, packagePath, packageName);
      packageList.add(sp);

      File[] files = currentFile.listFiles();
      for(int j=0; j<files.length; j++)
      {
        if(files[j].getName().endsWith(".xml"))
        {
          if (Main.options.verbose) System.out.println("  File:" + packagePath + File.separator + files[j].getName());
          StaxFile sf = new StaxFile(sp, files[j].getName());
          sp.addFile(sf);
          fileListAll.add(sf);
        }
      }
    }
    else
    {
      System.out.println("Only packages are allowed: " + packagePath);
      throw (new IOException());
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  public Iterator getFilesIterator()
  {
    return fileListAll.iterator();
  }

  /////////////////////////////////////////////////////////////////////////////
  public Iterator getPackageIterator()
  {
    return packageList.iterator();
  }

}
