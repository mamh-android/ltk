/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFCreateSTAFEnv extends CustomCodeAction
{
    String scriptFile = "";
    String scriptFileName = "";
    boolean unix = true;

    public void install(InstallerProxy ip)
    {
        String fileSep = System.getProperty("file.separator");
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");
        String osname = ip.substitute("$prop.os.name$");
        String instanceName = ip.substitute("$STAF_INSTANCE_NAME$");

        String libraryVariable = "";
        
        if (osname.indexOf("Linux") > -1)
        {
            libraryVariable = "LD_LIBRARY_PATH";
            unix = true;
        }
        else if (osname.indexOf("AIX") > -1)
        {
            libraryVariable = "LIBPATH";
            unix = true;
        }
        else if (osname.indexOf("SunOS") > -1)
        {
            libraryVariable = "LD_LIBRARY_PATH";
            unix = true;
        }
        else if (osname.indexOf("FreeBSD") > -1)
        {
            libraryVariable = "LD_LIBRARY_PATH";
            unix = true;
        }
        else if (osname.indexOf("HP-UX") > -1)
        {
            libraryVariable = "SHLIB_PATH";
            unix = true;
        }
        else if (osname.indexOf("Mac OS X") > -1)
        {
            libraryVariable = "DYLD_LIBRARY_PATH";
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
                writer.write("# STAF environment variables" + lineSep);
                writer.write("PATH=" + installLocation + fileSep +
                    "bin:${PATH:-}" + lineSep);
                writer.write(libraryVariable + "=" + installLocation +
                    fileSep + "lib:${" + libraryVariable + ":-}" + lineSep);
                writer.write("CLASSPATH=" + installLocation + fileSep +
                    "lib" + fileSep + "JSTAF.jar:" + installLocation + fileSep +
                    "samples" + fileSep + "demo" + fileSep +
                    "STAFDemo.jar:${CLASSPATH:-}" + lineSep);
                writer.write("STAFCONVDIR=" + installLocation + fileSep +
                    "codepage" + lineSep);
                writer.write("if [ $# = 0 ]" + lineSep);
                writer.write("then" + lineSep);
                writer.write("    STAF_INSTANCE_NAME=" + instanceName + lineSep);
                writer.write("else" + lineSep);
                writer.write("    if [ $1 != \"start\" ]" + lineSep);
                writer.write("    then" + lineSep);
                writer.write("        STAF_INSTANCE_NAME=$1" + lineSep);
                writer.write("    else" + lineSep);
                writer.write("        # Ignore \"start\" STAF instance name" +
                             lineSep);
                writer.write("        STAF_INSTANCE_NAME=" + instanceName +
                             lineSep);
                writer.write("    fi" + lineSep);
                writer.write("fi" + lineSep);
                writer.write("export PATH " + libraryVariable +
                    " CLASSPATH STAFCONVDIR STAF_INSTANCE_NAME" +
                    lineSep);
            }
            else
            {
                writer.write("@echo off" + lineSep);
                writer.write("REM STAF environment variables" + lineSep);
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
    }

    public void uninstall(UninstallerProxy up)
    {
    }

    public String getInstallStatusMessage()
    {
        return "Creating the STAFEnv script";
    }

    public String getUninstallStatusMessage()
    {
        return "Deleting the STAFEnv script";
    }
}