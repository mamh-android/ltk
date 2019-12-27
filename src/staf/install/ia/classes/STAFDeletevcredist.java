/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;
public class STAFDeletevcredist extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");
        String fileSep = System.getProperty("file.separator");
        String vcredistFileName = installLocation + fileSep +
            "vcredist_x64.exe";

        File vcredistFile = new File(vcredistFileName);

        if (vcredistFile.exists())
        {
            vcredistFile.delete();
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