/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.util.LinkedList;
import java.util.List;
import java.util.Iterator;

public class STAFUpdateCfgFile extends CustomCodeAction
{
    static String sslInterfaceLine =
        "interface ssl library STAFTCP option Secure=Yes option Port=6550";

    private String fSTAFRoot;
    private String fLineSep;
    private String fFileSep;
    private boolean fUseIA = false;
    private boolean fDebug = false;

    public STAFUpdateCfgFile(String stafRoot)
    {
        fSTAFRoot = stafRoot;
    }

    public STAFUpdateCfgFile() { /* Do Nothing */ }

    public void install(InstallerProxy ip)
    {
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");

        // Update the STAF config file

        this.setUseIA(true);
        this.setSTAFRoot(installLocation);
        this.setUseIA(true);
        int rc = this.update();

        // if rc == 0, the STAF.cfg file was successfully updated
        // if rc == 1, the STAF.cfg file did not need to be updated
        // if rc == 2, an error occurred updating the STAF.cfg file
        ip.setVariable("$UPDATE_CFG_FILE_RC$", new Integer(rc).toString());
    }

    public void uninstall(UninstallerProxy up)
    {
    }

    public String getInstallStatusMessage()
    {
        return "Updating the STAF.cfg file";
    }

    public String getUninstallStatusMessage()
    {
        return "";
    }

    private void setSTAFRoot(String stafRoot)
    {
        fSTAFRoot = stafRoot;
    }

    private void setUseIA(boolean useIA)
    {
        fUseIA = useIA;
    }

    private void printMsg(String msg)
    {
        if (fUseIA)
        {
            //IASys.err.println(msg);
        }
        else
        {
            //System.out.println(msg);
        }
    }

    // Updates the STAF.cfg by adding a secure tcp interface as the default
    // if a secure interface doesn't already exist and if there are no
    // interfaces already configured with port 6550.
    // If changes are made to the STAF.cfg file, the original STAF.cfg file
    // is backed up as file STAF_ia_backup.cfg in the STAF bin directory.
    //
    // Returns rc:
    //   if rc == 0, the STAF.cfg file was successfully updated
    //   if rc == 1, the STAF.cfg file was not changed
    //   if rc == 2, an error occurred updating the STAF.cfg file

    public int update()
    {
        // Assumes STAF config file is named <STAFRoot>/bin/STAF.cfg

        // Edits STAF config file adding a new secure tcp interface before
        // any other "interface" lines as follows:
        // interface ssl library STAFTCP option Secure=Yes Port=6550

        // If no interfaces are configured, then add this line as the first
        // line.

        // If the STAF.cfg file does not exist or a problem occurs reading
        // the STAF.cfg file, return rc 2.

        // If the STAF.cfg file exists but contains no lines, return rc 2.

        // If an interface is already defined named ssl or if an interface
        // is already defined that uses port 6550, don't make any changes
        // and return 1.

        // If adds a new secure tcp interface, also changes any
        // "set defaultinterface xxx" lines to "set defaultinterface ssl".

        // If don't have access to write to STAF.cfg file or the
        // STAF_ia_tmp.cfg (or if can't delete the STAF_ia_backup.cfg file),
        // return 2.

        // If a problem occurs writing to the STAF_ia_tmp.cfg file, return 2

        LinkedList cfgFileList = new LinkedList();
        BufferedReader inputStream = null;
        PrintWriter outputStream = null;

        fLineSep = System.getProperty("line.separator");
        fFileSep = System.getProperty("file.separator");

        String cfgFileName = fSTAFRoot + fFileSep + "bin" + fFileSep +
            "STAF.cfg";

        printMsg("Updating STAF config file: " + cfgFileName);

        int lineNumberToAdd = 0;
        boolean doNotAddSecureInterface = false;

        try
        {
            inputStream = new BufferedReader(new FileReader(cfgFileName));

            int lineNumber = 0;
            String line;
            boolean firstInterfaceFound = false;

            while ((line = inputStream.readLine()) != null)
            {
                String upperCaseLine = line.toUpperCase().trim();

                String[] lineTokens = upperCaseLine.split("\\s");

                if (lineTokens[0].equals("INTERFACE"))
                {
                    if (fDebug)
                    {
                        printMsg("Found interface line: ");
                        printMsg("  " + line);
                    }

                    if (!firstInterfaceFound)
                    {
                        lineNumberToAdd = lineNumber;
                        firstInterfaceFound = true;
                    }

                    // Check if interface name is ssl

                    if ((lineTokens.length > 2) &&
                        (lineTokens[1]).equals("SSL"))
                    {
                        printMsg("An interface named ssl is already " +
                                 "configured.");
                        doNotAddSecureInterface = true;
                    }
                    else
                    {
                        // Make sure options PORT=6550 and SECURE=YES are
                        // not specified

                        for (int i = 2; i < lineTokens.length; i++)
                        {
                            if (lineTokens[i].equals("PORT=6550"))
                            {
                                printMsg("An interface with port 6550 is " +
                                         "already configured.");
                                doNotAddSecureInterface = true;
                            }
                            else if (lineTokens[i].equals("SECURE=YES"))
                            {
                                printMsg("A secure interface is already " +
                                         "configured.");
                                doNotAddSecureInterface = true;
                            }
                        }
                    }
                }
                else if (lineTokens[0].equals("SET") && lineTokens.length > 1)
                {
                    if (lineTokens[1].equals("DEFAULTINTERFACE"))
                    {
                        // Change the default interface to be ssl
                        // Note:  This change will only actually be made to
                        // the STAF.cfg file if inserting a ssl interface line

                        if ((lineTokens.length > 2) &&
                            !lineTokens[2].equals("SSL"))
                        {
                            if (fDebug)
                            {
                                printMsg("Found defaultinterface line: " +
                                         line);
                            }

                            line = lineTokens[0] + " " + lineTokens[1] +
                                " ssl";
                        }
                    }
                }

                cfgFileList.add(line);

                lineNumber++;
            }
        }
        catch (java.io.FileNotFoundException e)
        {
            printMsg("ERROR: The STAF config file, " + cfgFileName +
                     ", does not exist." + fLineSep + e.toString());
            return 2;
        }
        catch (java.io.IOException e)
        {
            printMsg("ERROR: IOException reading the STAF config file, " +
                     cfgFileName + fLineSep + e.toString());
            return 2;
        }
        finally
        {
            if (inputStream != null)
            {
                try
                {
                    inputStream.close();
                }
                catch (java.io.IOException ex)
                {
                    // Ignore
                }
            }
        }

        if (cfgFileList.size() == 0)
        {
            printMsg("ERROR: The STAF config file, " + cfgFileName +
                     ", is empty.");
            return 2;
        }

        if (doNotAddSecureInterface)
        {
            // Don't update the STAF.cfg file
            printMsg("No changes made to STAF config file: " + cfgFileName);
            return 1;
        }

        // Insert the ssl interface line at the proper place in the array

        printMsg("Inserting following ssl interface line at line #:" +
                 lineNumberToAdd);
        printMsg("  " + sslInterfaceLine);

        if (lineNumberToAdd == 0)
        {
            // Insert ssl interfaces line at the beginning

            cfgFileList.addFirst(sslInterfaceLine);
        }
        else
        {
            cfgFileList.add(lineNumberToAdd, sslInterfaceLine);
        }

        // Write the new STAF.cfg file contents to a new file (STAF_ia_tmp.cfg)

        String newCfgFileName = fSTAFRoot + fFileSep + "bin" + fFileSep +
            "STAF_ia_tmp.cfg";

        try
        {
            outputStream = new PrintWriter(new FileWriter(newCfgFileName));

            // Iterate through the list, writing each line to the new STAF.cfg
            // file

            Iterator iter = cfgFileList.iterator();

            while (iter.hasNext())
            {
                outputStream.println((String)iter.next());
            }
        }
        catch (java.io.IOException e)
        {
            printMsg("ERROR: IOException writing to the temporary STAF " +
                     "config file, " + cfgFileName + fLineSep + e.toString());
            return 2;
        }
        finally
        {
            if (outputStream != null)
            {
                outputStream.close();
            }
        }

        // Rename the old STAF.cfg file to STAF_ia_backup.cfg (as a backup).
        // First delete STAF_ia_backup.cfg if it already exists.

        String backupCfgFileName = fSTAFRoot + fFileSep + "bin" + fFileSep +
            "STAF_ia_backup.cfg";

        File backupFile = new File(backupCfgFileName);

        if (backupFile.exists())
        {
            // Delete the previous backup file

            if (!backupFile.delete())
            {
                printMsg("ERROR: Deleting file " + backupCfgFileName +
                         " failed.");
                return 2;
            }
        }

        File cfgFile = new File(cfgFileName);

        if (!cfgFile.renameTo(new File(backupCfgFileName)))
        {
            printMsg("ERROR: Renaming file " + cfgFileName + " to " +
                     backupCfgFileName + " failed.");
            return 2;
        }

        // Rename the new STAF_ia_tmp.cfg file to STAF.cfg

        File newCfgFile = new File(newCfgFileName);

        if (!newCfgFile.renameTo(new File(cfgFileName)))
        {
            printMsg("ERROR: Renaming file " + newCfgFileName + " to " +
                     cfgFileName + " failed.");
            return 2;
        }

        return 0;
    }

    public static void main(String[] args)
    {
        if (args.length != 1)
        {
            System.out.println("Usage: java STAFUpdateCfgFile <STAFRoot>");
            System.out.println("");
            System.out.println("For example:");
            System.out.println("  java STAFUpdateCfgFile C:\\STAF");
            System.out.println("  java STAFUpdateCfgFile /usr/local/staf");
            System.exit(1);
        }

        // Get the STAFRoot directory from first argument
        String stafRoot = args[0];

        // Update the STAF config file

        STAFUpdateCfgFile updateCfgFile = new STAFUpdateCfgFile(stafRoot);

        updateCfgFile.setUseIA(false);

        int rc = updateCfgFile.update();

        System.exit(rc);
    }
}