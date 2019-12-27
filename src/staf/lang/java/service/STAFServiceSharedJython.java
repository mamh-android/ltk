/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service;

import com.ibm.staf.*;
import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.util.jar.*;
import java.util.zip.*;
import java.util.Enumeration;
import java.util.Properties;

public class STAFServiceSharedJython
{
    static public final boolean DEBUG = false;

    // Service Manifest file Entries
    //   Name: staf/service/shared_jython
    //   Jython-Version: 2.1-staf-v4

    static final String JYTHON_ENTRY = new String("staf/service/shared_jython");
    static final String JYTHON_VERSION = new String("Jython-Version");

    static final String JYTHON_INF = "JYTHON-INF/";
    static final String JYTHON_JAR_ENTRY = "STAF-INF/jars/jython.jar";

    static String fileSep;  // {STAF/Config/Sep/File}

    static private JarFile jarFile = null;
    static private STAFHandle handle = null;
    static private String fJythonDirName = new String("");
    static private String fJythonVersion = new String("");

    static public String getJythonDirName()
    {
        return fJythonDirName;
    }
    
    static public String getJythonVersion()
    {
        return fJythonVersion;
    }
    
    /***********************************************************************/
    /* setupJython - Checks if the special complete.txt file exists.       */
    /* If not, extracts jar entries and makes files out of them            */
    /* and puts them into the specified directory.  Sets the classpath to  */
    /* point to the jython.jar and initializes the PythonInterpreter's     */
    /* python.home to point to the shared jython installation.             */
    /*                                                                     */
    /* accepts:                                                            */
    /*   JarFile           - Service's jar file                            */
    /*   STAFHandle        - Service's handle                              */
    /*   String            - STAF Write Location                           */
    /*                                                                     */
    /* returns:                                                            */
    /*   STAFResult        - RC and result                                 */
    /***********************************************************************/
    static public STAFResult setupJython(JarFile inJarFile,
                                         STAFHandle inHandle,
                                         String writeLocation)
    {
        jarFile = inJarFile;
        handle = inHandle;

        STAFResult result = new STAFResult(STAFResult.Ok, "");

        // Add service jar to the classpath

        Properties props = System.getProperties();

        props.setProperty("java.class.path",
                          props.getProperty("java.class.path") +
                          props.getProperty("path.separator") +
                          jarFile.getName());

        // Get the File separator
        fileSep = props.getProperty("file.separator");

        // Get Jython version number from manifest file
        Manifest manifest = null;

        try
        {
            manifest = jarFile.getManifest();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "Error accessing manifest file from " +
                                  jarFile.getName() + "\n" +
                                  ex.getMessage());
        }

        if (manifest == null)
        {
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "No manifest file in " + jarFile.getName());
        }

        if (!manifest.getEntries().containsKey(JYTHON_ENTRY))
        {
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "Manifest file in " + jarFile.getName() +
                                  " is missing key " + JYTHON_ENTRY);
        }

        Attributes attrs = manifest.getAttributes(JYTHON_ENTRY);

        if (!attrs.containsKey(new Attributes.Name(JYTHON_VERSION)))
        {
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "Manifest file in " + jarFile.getName() +
                                  " is missing attribute name " +
                                  JYTHON_VERSION + " for entry " +
                                  JYTHON_ENTRY);
        }

        fJythonVersion = attrs.getValue(JYTHON_VERSION);

        // Assign the target directory name
        // e.g. <writeLocation>/lang/java/shared_jython/2.1-staf-v4
        fJythonDirName = writeLocation + fileSep + "lang" + fileSep + "java" +
            fileSep + "shared_jython" + fileSep + fJythonVersion;

        String jythonLibName = fJythonDirName + fileSep + "Lib";
        File jythonDir = new File(fJythonDirName);
        File jythonCompleteFile = new File(fJythonDirName + fileSep +
                                           "complete.txt");

        // Check if the Jython complete file exists
        if (!jythonCompleteFile.exists())
        {
            if (DEBUG)
            {
                System.out.println("Creating Jython Directory: " +
                                   fJythonDirName);
            }

            // Extract jar entries that begin with JYTHON_INF and make files
            // out of them placed into the specified target directory.

            result = extractFromJar(jythonDir, JYTHON_INF);

            if (result.rc != 0) return result;

            // Now, extract jython.jar

            result = extractFileFromJar(JYTHON_JAR_ENTRY,
                                      fJythonDirName + fileSep + "jython.jar");

            if (result.rc != 0) return result;

            // Write the special file that indicates that the shared Jython
            // release directory was successfully installed

            result = writeStringToFile(jythonCompleteFile,
                                       "Shared Jython installed");

            if (result.rc != 0) return result;
        }

        // Set the classpath to point to the jython.jar

        props = System.getProperties();

        props.setProperty("java.class.path",
                           props.getProperty("java.class.path") +
                           props.getProperty("path.separator") +
                           fJythonDirName +
                           props.getProperty("file.separator") +
                           "jython.jar");
        if (DEBUG)
        {
            System.out.println("java.class.path=" +
                               props.getProperty("java.class.path"));
        }

        return result;
    }

    /***********************************************************************/
    /* extractFromJar - Extracts jar entries for a specified directory     */
    /*   and makes files out of them placed into the target directory.     */
    /*                                                                     */
    /* accepts:                                                            */
    /*   targetDir   - the directory to place extracted files into         */
    /*   searchEntry - the name of the directory whose files are to be     */
    /*                 extracted                                           */
    /*                                                                     */
    /* returns:                                                            */
    /*   STAFResult        - RC and result                                 */
    /***********************************************************************/
    static private STAFResult extractFromJar(File targetDir, String searchEntry)
    {
        STAFResult result = new STAFResult(STAFResult.Ok, "");

        ZipEntry ze = null;
        InputStream in = null;
        FileOutputStream fos = null;

        // Create path name
        String targetPath = targetDir.getAbsolutePath() + "/";

        try
        {
            boolean searchEntryFound = false;
            int seLen = searchEntry.length();

            // Loop through each element in the jar file and if it matches
            // the search criteria, write it to the target directory. Check to
            // ensure the path exists before writing.
            for (Enumeration e = jarFile.entries(); e.hasMoreElements();)
            {
                // ZipEntry class is used to represent zip file entry
                ze = (ZipEntry)e.nextElement();

                // Search for a particular entry
                if (ze.getName().indexOf(searchEntry) == 0)
                {
                    searchEntryFound = true;

                    String targetName = targetPath +
                                        ze.getName().substring(seLen);
                    File entryFile = new File(targetName);

                    if (ze.isDirectory() && !entryFile.exists())
                        entryFile.mkdirs();

                    if (entryFile.isDirectory())
                        continue;

                    in = jarFile.getInputStream(ze);
                    fos = new FileOutputStream(entryFile);
                    byte [] buffer = new byte[1024];

                    for (int bytesRead = 0; bytesRead != -1;)
                    {
                        bytesRead = in.read(buffer, 0, 1024);

                        if (bytesRead != -1)
                            fos.write(buffer, 0, bytesRead);
                    }

                    fos.close();
                }
                else if (searchEntryFound)
                    break;  // Found all entries, stop searching
            }

            if (!searchEntryFound)
            {
                return new STAFResult(STAFResult.ServiceConfigurationError,
                                      "Error: No entries for " + searchEntry +
                                      " found in " + jarFile.getName());
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "Error: " + ex.getMessage());
        }

        return result;
    }

    /***********************************************************************/
    /* extractFileFromJar - Extracts a single file from a Jar file.        */
    /*                                                                     */
    /* accepts:                                                            */
    /*   targetJarFileName - the name of the file in the jar file          */
    /*   targetFileName    - the name of the target file                   */
    /*                                                                     */
    /* returns:                                                            */
    /*   STAFResult        - RC and result                                 */
    /***********************************************************************/
    static private STAFResult extractFileFromJar(String targetJarFileName,
                                          String targetFileName)
    {
        STAFResult result = new STAFResult(STAFResult.Ok, "");

        ZipEntry ze = null;
        InputStream in = null;
        FileOutputStream fos = null;

        try
        {
            // ZipEntry class is used to represent zip file entry
            ze = jarFile.getEntry(targetJarFileName);

            if (ze == null)
            {
                return new STAFResult(STAFResult.ServiceConfigurationError,
                                      "Error: Could not find " +
                                      targetJarFileName + " in " +
                                      jarFile.getName());
            }

            in = jarFile.getInputStream(ze);
            fos = new FileOutputStream(targetFileName);
            byte [] buffer = new byte[1024];

            for (int bytesRead = 0; bytesRead != -1;)
            {
                bytesRead = in.read(buffer, 0, 1024);

                if (bytesRead != -1)
                    fos.write(buffer, 0, bytesRead);
            }

            fos.close();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "Error: " + ex.getMessage());
        }

        return result;
    }

    /***********************************************************************/
    /* writeStringToFile - Writes a string to a file                       */
    /*                                                                     */
    /* accepts:                                                            */
    /*   targetFile   - the File object to write to                        */
    /*   output       - the string to write to the file                    */
    /*                                                                     */
    /* returns:                                                            */
    /*   STAFResult   - RC and result                                      */
    /***********************************************************************/
    static private STAFResult writeStringToFile(File targetFile,
                                                String output)
    {
        STAFResult result = new STAFResult(STAFResult.Ok, "");

        try
        {
            FileWriter fw = new FileWriter(targetFile);
            fw.write(output, 0, output.length());
            fw.close();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return new STAFResult(STAFResult.ServiceConfigurationError,
                                  "Error: " + ex.getMessage());
        }

        return result;
    }
}
