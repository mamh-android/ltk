/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFCheckSilentLicenseAccept extends CustomCodeAction
{
    String message = "You must accept the STAF license in order to proceed " +
       "with the installation.  To accept the license add the following " +
       "option:\n\n-DACCEPT_LICENSE=1\n\n";

    public void install(InstallerProxy ip)
    {
        String licenseAccept = ip.substitute("$ACCEPT_LICENSE$");

        if (!(licenseAccept.equals("1")))
        {
            //System.out.println(message);
            PrintStream out = IASys.out;
            out.println(message);
            CustomError error = (CustomError)ip.getService(CustomError.class);
            error.appendError(message, CustomError.ERROR);
            error.setLogDescription("STAFCheckSilentLicenseAccept");
            error.log();
            ip.abortInstallation(200);
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