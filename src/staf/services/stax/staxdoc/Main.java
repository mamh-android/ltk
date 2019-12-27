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
* Main program of StaxDoc tool.
******************************************************************************/
public final class Main
{
  static final String program = "staxdoc";

  static Options options = new Options();
  static StaxSource srcFiles = null;


  /////////////////////////////////////////////////////////////////////////////
  public static void main(String argv[])
  {
    //-------------------------------------------------------------------------
    // Parse arguments

    // first looks for 'verbose' option
    for (int i = 0; i < argv.length; i++)
    {
      String arg = argv[i];
      if (argv[i].equals("-verbose"))
      {
        options.verbose = true;
        System.out.println("Option set: verbose=true");
      }
    }

    for (int i = 0; i < argv.length; i++)
    {
      String arg = argv[i];
      if (arg.equals("-d"))
      {
        i++;
        options.destDir = argv[i];
        if (options.verbose) System.out.println("Option set: d=" + options.destDir);

        File destDir = new File(argv[i]);
        if (!destDir.exists())
        {
          //System.out.println("Destination directory not found " + destDir.getPath());
          if (!destDir.mkdirs())
          {
            System.err.println("Unable to create " + destDir.getName() + " directory");
            exit();
          }
          if (options.verbose) System.out.println("Destination directory created: " + destDir.getName());
        }
        else if (!destDir.isDirectory())
        {
          System.err.println("Destination directory is not a directory" + destDir.getPath());
          exit();
        }
        else if (!destDir.canWrite())
        {
          System.err.println("Destination directory not writable" + destDir.getPath());
          exit();
        }
      }
      else if (arg.equals("-doctitle"))
      {
        i++;
        options.doctitle = argv[i];
        if (options.verbose) System.out.println("Option set: doctitle=" + options.doctitle);
      }
      else if (arg.equals("-help"))
      {
        printUsage();
        exit();
      }
      else if (arg.equals("-overview"))
      {
        i++;
        options.overviewFilename = argv[i];
        if (options.verbose) System.out.println("Option set: overview=" + options.overviewFilename);
      }
      else if (arg.equals("-sourcepath"))
      {
        i++;
        options.srcPathString = argv[i];
        if (options.verbose) System.out.println("Option set: sourcepath=" + options.srcPathString);
      }
      else if (arg.equals("-functionsummary"))
      {
        i++;
        options.functionSummary = argv[i];

        if (!options.validFunctionSummaryValue(options.functionSummary))
        {
            System.err.println(
                "\nInvalid value for option -functionsummary: " +
                options.functionSummary +
                "\nValid value list for option -functionsummary: " +
                options.getFunctionSummaryValues());

            printUsage();
            exit();
        }

        if (options.verbose) System.out.println("Option set: functionsummary=" + options.functionSummary);
      }
      else if (arg.equals("-verbose"))
      {
        // nothing to do
      }
      else if (arg.equals("-windowtitle"))
      {
        i++;
        options.windowtitle = argv[i];
        if (options.verbose) System.out.println("Option set: windowtitle=" + options.windowtitle);
      }
      else if ( arg.startsWith("-") )
      {
        usageError("Invalid option: " + arg);
      }
      else
      {
        StringTokenizer st = new StringTokenizer(arg, "=");
        String pkgPath = st.nextToken();
        String pkgName = pkgPath;
        if (st.hasMoreTokens())
        {
          pkgName = st.nextToken();
        }

        options.packagePathsList.add(pkgPath);
        options.packageNamesList.add(pkgName);
        if (options.verbose) System.out.println("STAX source package:" + arg);
      }
    }

    if (options.packagePathsList.isEmpty())
    {
      usageError("No packages specified");
    }

    //-------------------------------------------------------------------------
    // put all STAX source files into global StaxFiles object
    srcFiles = new StaxSource(options.srcPathString);
    try
    {
      System.out.println();
      for(int i=0; i<options.packagePathsList.size(); i++)
      {
        srcFiles.addSource((String)options.packagePathsList.get(i), (String)options.packageNamesList.get(i));
      }
      System.out.println();
    }
    catch (IOException e)
    {
      exit();
    }

    //-------------------------------------------------------------------------
    // generate documentation files
    try
    {
      WriterMainFrame.generate();
      WriterOverviewSummary.generate();
      WriterOverviewFrame.generate();
      WriterAllFilesFrame.generate();

      // generate packages directories structure and navigation HTML files
      Iterator itDir = Main.srcFiles.getPackageIterator();
      while (itDir.hasNext())
      {
        new PackageHandler((StaxPackage)itDir.next());
      }

      // generate files
      Iterator itFile = Main.srcFiles.getFilesIterator();

      while (itFile.hasNext())
      {
        WriterStaxDoc.generate((StaxFile)itFile.next());
      }

    }
    catch(IOException ioe)
    {
      System.err.println("IOException encountered");
      System.err.println(ioe.toString());
      exit();
    }


    if (options.verbose) System.out.println("STAXDoc ended with success");

    exit();
  }


  /////////////////////////////////////////////////////////////////////////////
  static void exit()
  {
    System.exit(0);
  }


  /////////////////////////////////////////////////////////////////////////////
  static void printUsage()
  {
    System.out.println();
    System.out.println("Usage: java -jar STAXDoc.jar [-options] packagename(s)...");
    System.out.println();
    System.out.println("where options include:");
    System.out.println("    -d <directory>          Specifies the destination directory for output");
    System.out.println("                            files.  The current directory is the default.");
    System.out.println("    -doctitle <html-code>   Specifies to include the title for the package");
    System.out.println("                            index (first) page.");
    System.out.println("    -functionsummary <FirstSentence | All>");
    System.out.println("                            Specifies what to include for the description on");
    System.out.println("                            each \"Function Summary\" page:");
    System.out.println("                            - FirstSentence: Include only the first sentence");
    System.out.println("                              of the function-prolog.  This is the default.");
    System.out.println("                            - All: Include the entire contents of the");
    System.out.println("                              function-prolog.");
    System.out.println("    -help                   Specifies to display the help.");
    System.out.println("    -overview <file>        Specifies to read overview documentation from the");
    System.out.println("                            HTML file.");
    System.out.println("    -sourcepath <directory> Specifies the root directory of the packages.");
    System.out.println("                            The current directory is the default.");
    System.out.println("    -verbose                Specifies to output messages about what STAXDoc is");
    System.out.println("                            doing.");
    System.out.println("    -windowtitle <title>    Specifies the title to be placed in the HTML");
    System.out.println("                            <title> tag.");
    System.out.println();
    System.out.println("where packagename(s)... are the names of one or more subdirectories in");
    System.out.println("    the -sourcepath containing STAX xml files that you want to document.");
    System.out.println("    The subdirectory names must be separated by one or more spaces.");
    System.out.println("    You must separately specify each package (subdirectory) that you want to");
    System.out.println("    document as subdirectories are not recursively traversed.");
    System.out.println("    Package names can be overridden using the = keyword.  For example, if you");
    System.out.println("    specify src1=P1 src2 in the command line, the first package will appear ");
    System.out.println("    named P1 in the generated documentation.");
    System.out.println();
    System.out.println("Examples:");
    System.out.println("  java -jar STAXDoc.jar -d C:\\stax\\mydocs -sourcepath C:\\stax\\xml src1 src2");
    System.out.println("  java -jar STAXDoc.jar -sourcepath C:\\stax\\xml -verbose libraries");
    System.out.println("  java -jar STAXDoc.jar -sourcepath C:\\user\\src utils/memory");
    System.out.println();
  }
  

  /////////////////////////////////////////////////////////////////////////////
  static void usageError(String key)
  {
    System.err.println(key);
    printUsage();
    exit();
  }

}
