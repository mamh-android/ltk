/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.staxdoc;

import java.io.*;
import java.util.Iterator;

/******************************************************************************
*
******************************************************************************/
public class PackageHandler
{

  PackageHandler(StaxPackage staxPackage) throws IOException
  {
    File dir = new File(Main.options.destDir + File.separator + staxPackage.getPath());
    if (!dir.exists())
    {
      if (!dir.mkdirs())
      {
        System.out.println("Unable to create destination directory");
        throw new IOException();
      }
    }

    WriterPackageOverview.generate(staxPackage);
    WriterPackageFrame.generate(staxPackage);
  }
}
