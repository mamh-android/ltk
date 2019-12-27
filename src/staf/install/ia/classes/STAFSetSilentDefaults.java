/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFSetSilentDefaults extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        String osname = ip.substitute("$prop.os.name$");
        String osarch = ip.substitute("$prop.os.arch$");

        String updateEnvironment = ip.substitute("$UPDATE_ENVIRONMENT$");

        if (updateEnvironment.equals(""))
        {
            ip.setVariable("$UPDATE_ENVIRONMENT$", "System");
        }

        String startSTAFonLogin = ip.substitute("$START_ON_LOGIN$");

        if (startSTAFonLogin.equals(""))
        {
            ip.setVariable("$START_ON_LOGIN$", "1");
        }

        String createStartMenuIcons = ip.substitute("$CREATE_START_MENU_ICONS$");

        if (createStartMenuIcons.equals("") &&
            !(updateEnvironment.equals("None")))
        {
            ip.setVariable("$CREATE_START_MENU_ICONS$", "1");
        }

        String useTCP = ip.substitute("$USE_TCP_VERSION$");

        if (useTCP.equals(""))
        {
            ip.setVariable("$USE_TCP_VERSION$", "IPV4");
        }

        String usePerl = ip.substitute("$USE_PERL_VERSION$");

        if (usePerl.equals(""))
        {
            ip.setVariable("$USE_PERL_VERSION$", "5.8");
        }

        String usePython = ip.substitute("$USE_PYTHON_VERSION$");

        if (usePython.equals(""))
        {
            if ((osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
                (osname.indexOf("SunOS") > -1) ||
                (osname.indexOf("Darwin") > -1))
            {
                ip.setVariable("$USE_PYTHON_VERSION$", "2.2");
            }
            else if ((osname.indexOf("Windows") > -1) && (osarch.equals("ia64")))
            {
                ip.setVariable("$USE_PYTHON_VERSION$", "2.4");
            }
            else if ((osname.indexOf("Windows") > -1) && (osarch.equals("amd64")))
            {
                ip.setVariable("$USE_PYTHON_VERSION$", "2.5");
            }
            else if (osname.indexOf("FreeBSD") > -1)
            {
                ip.setVariable("$USE_PYTHON_VERSION$", "2.4");
            }
            else if (osname.indexOf("Darwin") > -1)
            {
                ip.setVariable("$USE_PYTHON_VERSION$", "2.3");
                ip.setVariable("$USE_PYTHON_SYSTEM_PATH$", "0");
            }
        }

        String useTcl = ip.substitute("$USE_TCL_VERSION$");

        if (useTcl.equals(""))
        {
            if ((osname.indexOf("Windows") > -1) && (osarch.equals("x86")))
            {
                ip.setVariable("$USE_TCL_VERSION$", "8.3");
            }
            else if ((osname.indexOf("Windows") > -1) &&
                     (osarch.equals("amd64")))
            {
                ip.setVariable("$USE_TCL_VERSION$", "8.5");
            }
        }

        String stafInstanceName = ip.substitute("$STAF_INSTANCE_NAME$");

        if (stafInstanceName.equals(""))
        {
            ip.setVariable("$STAF_INSTANCE_NAME$", "STAF");
        }

        String allowSTAFToRegister = ip.substitute("$REGISTER$");

        if (allowSTAFToRegister.equals(""))
        {
            ip.setVariable("$REGISTER$", "1");
        }

        String userRequestedRestart = ip.substitute("$USER_REQUESTED_RESTART$");

        if (userRequestedRestart.equals(""))
        {
            ip.setVariable("$USER_REQUESTED_RESTART$", "NO");
        }

        String startSTAFProc = ip.substitute("$START_STAFPROC$");

        if (startSTAFProc.equals(""))
        {
            ip.setVariable("$START_STAFPROC$", "Normal");
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