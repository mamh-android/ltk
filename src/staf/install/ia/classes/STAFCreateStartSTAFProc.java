/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFCreateStartSTAFProc extends CustomCodeAction
{
    String scriptFile = "";
    String scriptFileName = "";
    boolean unix = true;

    public void install(InstallerProxy ip)
    {
        String fileSep = System.getProperty("file.separator");
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");
        String osname = ip.substitute("$prop.os.name$");
        String stafVersion = ip.substitute("$STAF_VERSION$");
        String startSTAFProc = ip.substitute("$START_STAFPROC$");
        String startSTAFProcScript = "";
        String startSTAFProcScriptFileName = "";
        String stafEnvFileName = "";
        String stafProcExe = "";
        String startOption = " ";

        if (osname.indexOf("Windows") > -1)
        {
            startSTAFProcScript = "startSTAFProc.bat";
            stafProcExe = installLocation + fileSep + "bin" + fileSep +
                "STAFProc.exe";

            if (startSTAFProc.equalsIgnoreCase("Minimized"))
            {
                startOption = " /min ";
            }

            stafEnvFileName = "STAFEnv.bat";
        }
        else
        {
            startSTAFProcScript = "startSTAFProc.sh";
            stafProcExe = installLocation + fileSep + "bin" + fileSep +
                "STAFProc";
            stafEnvFileName = "STAFEnv.sh";
        }

        startSTAFProcScriptFileName =
            installLocation + fileSep + startSTAFProcScript;

        File startSTAFProcScriptFile = new File(startSTAFProcScriptFileName);

        if (startSTAFProcScriptFile.exists())
        {
            startSTAFProcScriptFile.delete();
        }

        FileWriter writer = null;
        String lineSep = System.getProperty("line.separator");

        try
        {
            if (osname.indexOf("Windows") > -1)
            {
                writer = new FileWriter(startSTAFProcScriptFile);
                writer.write("REM Sets up the STAF environment variables and " +
                             "starts STAFProc " + lineSep);
                writer.write("call \"" + installLocation + fileSep +
                             stafEnvFileName + "\"" + lineSep);
                writer.write("start \"Start STAF " + stafVersion + "\""  +
                             startOption + "\"" + stafProcExe + "\"" + lineSep);
                writer.close();
            }
            else
            {
                writer = new FileWriter(startSTAFProcScriptFile);
                writer.write("#!/bin/sh" + lineSep);
                writer.write("# Sets up the STAF environment variables and " +
                             "starts STAFProc " + lineSep + "# in the " +
                             "background, logging STAFProc output to " +
                             "nohup.out" + lineSep);
                writer.write(". " + installLocation + fileSep +
                             stafEnvFileName + lineSep);
                writer.write("nohup " + stafProcExe + " &" + lineSep);
                writer.close();

                String command = "chmod 777 " + startSTAFProcScriptFileName;
                Process pid = Runtime.getRuntime().exec(command);
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }

    public void uninstall(UninstallerProxy up)
    {
    }

    public String getInstallStatusMessage()
    {
        return "Creating the startSTAFProc script";
    }

    public String getUninstallStatusMessage()
    {
        return "Deleting the startSTAFProc script";
    }
}