/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2010                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFRegisterRecordedVariables extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        // Register the custom variables that will be recorded in the
        // response file

        ReplayVariableService rvs =
            (ReplayVariableService)ip.getService(ReplayVariableService.class);

        rvs.register("ACCEPT_LICENSE",
                     "Acceptance of the license agreement");
        rvs.register("UPDATE_ENVIRONMENT",
                     "Scope of environment/menu updates");
        rvs.register("START_ON_LOGIN",
                     "Whether to start STAF on Login (Windows only)");
        rvs.register("CREATE_START_MENU_ICONS",
                     "Whether to create Start menu icons/programs folder for STAF (Windows only)");
        rvs.register("START_STAFPROC",
                     "Whether to start the STAFProc window in normal state (visible on the desktop) or minimized (Windows only)");
        rvs.register("USE_TCP_VERSION",
                     "The default TCP/IP version");
        rvs.register("USE_PERL_VERSION",
                     "The default Perl version");
        rvs.register("USE_PERL_SYSTEM_PATH",
                     "Whether to determine the version of Perl in the system PATH at install-time (if possible) and use that version of Perl by default");
        rvs.register("USE_PYTHON_VERSION",
                     "The default Python version");
        rvs.register("USE_PYTHON_SYSTEM_PATH",
                     "Whether to determine the version of Python in the system PATH at install-time (if possible) and use that version of Python by default");
        rvs.register("STAF_INSTANCE_NAME",
                     "The default STAF instance name");
        rvs.register("USER_REQUESTED_RESTART",
                     "Whether to automatically restart the Windows operating system if target files are in use during the STAF installation");
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