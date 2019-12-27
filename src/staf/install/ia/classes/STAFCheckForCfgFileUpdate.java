/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;
public class STAFCheckForCfgFileUpdate extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");
        String fileSep = System.getProperty("file.separator");
        String installPropertiesFileName = installLocation + fileSep +
            "install.properties";
        String osname = ip.substitute("$prop.os.name$");
        String osarch = ip.substitute("$prop.os.arch$");

        File installPropertiesFile = new File(installPropertiesFileName);

        // When a secure TCP/IP interface is provided for Windows IA64 in a
        // future STAF release, remove the OR check of osname/osarch.
        if (installPropertiesFile.exists() ||
            ((osname.indexOf("Windows") > -1) && osarch.equals("ia64")))
        {
            ip.setVariable("$UPDATE_CFG_FILE$", "0");
        }
        else
        {
            ip.setVariable("$UPDATE_CFG_FILE$", "1");
        }
    }

    public void uninstall(UninstallerProxy up)
    {
    }

    public String getInstallStatusMessage()
    {
        return "";
    }

    public String getUninstallStatusMessage()
    {
        return "";
    }
}