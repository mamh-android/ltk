/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.util.*;
import com.installshield.product.service.product.*;
import java.io.*;

public class STAFCreateScripts extends WizardAction
{
    String scriptFile = "";
    String scriptFileName = "";
    boolean unix = true;

    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String installLocation = resolveString("$P(absoluteInstallLocation)");
        String instanceName =
            resolveString("$W(instanceNameInput.instanceName)");

        String libraryVariable = "";
        String platformDetails = Platform.currentPlatform.toString();

        if (platformDetails.indexOf("Linux") > -1)
        {
            libraryVariable = "LD_LIBRARY_PATH";
            unix = true;
        }
        else if (platformDetails.indexOf("AIX") > -1)
        {
            libraryVariable = "LIBPATH";
            unix = true;
        }
        else if (platformDetails.indexOf("SunOS") > -1)
        {
            libraryVariable = "LD_LIBRARY_PATH";
            unix = true;
        }
        else if (platformDetails.indexOf("HP-UX") > -1)
        {
            libraryVariable = "SHLIB_PATH";
            unix = true;
        }
        else
        {
            unix = false;
        }

        if (unix)
        {
            scriptFile = "STAFEnv.sh";
        }
        else
        {
            scriptFile = "STAFEnv.bat";
        }

        scriptFileName =  installLocation + fileSep + scriptFile;

        File scriptFile = new File(scriptFileName);

        if (scriptFile.exists())
        {
            scriptFile.delete();
        }

        FileWriter writer = null;
        String lineSep = System.getProperty("line.separator");

        try
        {
            writer = new FileWriter(scriptFile);

            if (unix)
            {
                writer.write("#!/bin/sh" + lineSep);
                writer.write("# STAF environment variables for " +
                    resolveString("$W(stafVersion.version)") + lineSep);
                writer.write("PATH=" + installLocation + fileSep +
                    "bin:$PATH" + lineSep);
                writer.write(libraryVariable + "=" + installLocation +
                    fileSep + "lib:$" + libraryVariable + lineSep);
                writer.write("CLASSPATH=" + installLocation + fileSep +
                    "lib" + fileSep + "JSTAF.jar:" + installLocation + fileSep +
                    "samples" + fileSep + "demo" + fileSep +
                    "STAFDemo.jar:$CLASSPATH" + lineSep);
                writer.write("STAFCONVDIR=" + installLocation + fileSep +
                    "codepage" + lineSep);
                writer.write("if [ $# = 0 ]" + lineSep);
                writer.write("then" + lineSep);
                writer.write("    STAF_INSTANCE_NAME=" + instanceName + lineSep);
                writer.write("else" + lineSep);
                writer.write("    STAF_INSTANCE_NAME=$1" + lineSep);
                writer.write("fi" + lineSep);
                writer.write("export PATH " + libraryVariable +
                    " CLASSPATH STAFCONVDIR STAF_INSTANCE_NAME" +
                    lineSep);
            }
            else
            {
                writer.write("@echo off" + lineSep);
                writer.write("REM STAF environment variables for " +
                    resolveString("$W(stafVersion.version)") + lineSep);
                writer.write("set PATH=" + installLocation + fileSep +
                    "bin;%PATH%" + lineSep);
                writer.write("set CLASSPATH=" + installLocation + fileSep +
                    "bin" + fileSep + "JSTAF.jar;" + installLocation + fileSep +
                    "samples" + fileSep + "demo" + fileSep +
                    "STAFDemo.jar;%CLASSPATH%" + lineSep);
                writer.write("set STAFCONVDIR=" + installLocation + fileSep +
                    "codepage" + lineSep);
                writer.write("if \"%1\" EQU \"\" set STAF_INSTANCE_NAME=" +
                    instanceName + lineSep);
                writer.write("if \"%1\" NEQ \"\" set STAF_INSTANCE_NAME=%1" +
                    lineSep);
            }

            writer.close();

            if (unix)
            {
                String command = "chmod 777 " + scriptFileName;
                Process pid = Runtime.getRuntime().exec(command);
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        
        // XXX This next section is commented out because InstallShield 10.5
        // does not recognize STAF installations that used pre-10.5 versions
        // of InstallShield (which STAF 2.x does).  If this is ever resovled
        // in a new version of InstallShield 10.5+, then this code can be
        // uncommented.

        // Create 2x script, if 2x is installed and this is not an upgrade
        // of 2x.  Do not modify the script if it already exists.

        /*String twoXInstalled = resolveString("$W(findInstances.twoXInstalled");
        String isUpgrade = resolveString("$W(findInstances.isUpgrade");
        String upgradeVersion =
            resolveString("$W(findInstances.upgradeVersion");

        if (twoXInstalled.equals("false"))
        {
            return;
        }

        if (isUpgrade.equals("true") &&
            (upgradeVersion.equals("a pre-V3.0 version of STAF")))
        {
            return;
        }

        String twoXInstallLocation =
            resolveString("$W(findInstances.twoXInstallLocation)");

        try
        {
            File twoXScriptFile = new File(twoXInstallLocation + fileSep +
                "STAFEnv.bat");

            if (twoXScriptFile.exists()) return;

            writer = new FileWriter(twoXScriptFile);

            writer.write("@echo off" + lineSep);
            writer.write("REM STAF environment variables" + lineSep);
            writer.write("set PATH=" + twoXInstallLocation + fileSep +
                "bin;%PATH%" + lineSep);
            writer.write("set CLASSPATH=" + twoXInstallLocation + fileSep +
                "bin" + fileSep + "JSTAF.jar;" + twoXInstallLocation + fileSep +
                "samples" + fileSep + "demo" + fileSep +
                "STAFDemo.jar;%CLASSPATH%" + lineSep);
            writer.write("set STAFCONVDIR=" + twoXInstallLocation + fileSep +
                "codepage" + lineSep);

            writer.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }*/
    }

    public String getScriptFileName()
    {
        return scriptFileName;
    }

    public void setScriptFileName(String str)
    {
        scriptFileName = str;
    }
}