/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFCreateLinks extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");

        try
        {
            String command = "ln -s " + installLocation + "/bin/STAF " +
                             installLocation + "/bin/staf";
            Process pid = Runtime.getRuntime().exec(command);
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        try
        {
            String command = "ln -s " + installLocation + "/bin/FmtLog " +
                             installLocation + "/bin/fmtlog";
            Process pid = Runtime.getRuntime().exec(command);
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        /*try
        {
            if (resolveString("$P(perlSupport.active)").equals("true") &&
                resolveString("$W(stafPlatform.linux)").equals("true"))
            {
                String defaultPerlSupport =
                    resolveString("$W(stafOptions.defaultPerlVersion)");

                if (defaultPerlSupport.equals("5.8"))
                {
                    defaultPerlSupport = "perl58";
                }
                else  if (defaultPerlSupport.equals("5.6"))
                {
                    defaultPerlSupport = "perl56";
                }
                else
                {
                    defaultPerlSupport = "perl50";
                }

                String command = "ln -s " + installLocation +
                                 "/lib/" + defaultPerlSupport +
                                 "/libPLSTAF.so " + installLocation +
                                 "/lib/libPLSTAF.so";
                Process pid = Runtime.getRuntime().exec(command);
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }*/

        /*try
        {
            if (resolveString("$W(stafPlatform.aix)").equals("true"))
            {
                String command = "ln -s " + installLocation +
                    "/lib/libSTAF.so "
                    + installLocation + "/lib/libSTAF.a";
                Process pid = Runtime.getRuntime().exec(command);
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }*/
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