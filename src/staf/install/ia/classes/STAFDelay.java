/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

/*
  This CustomCodeAction is a workaround for an InstallAnywhere problem where
  on an upgrade of STAF, when the existing version of STAF is uninstalled
  during the upgrade, the Uninstaller for STAF will sometimes not be created.

  The problem is that when the uninstaller is launched, the process that is
  spawned will recursively check the uninstall folder and remove anything in
  it.  This process seems to linger around for a bit when the install sequence
  starts.  So when the new uninstaller is created, that process will remove it.

  The workaround is to have a CustomCodeAction delay for 5 seconds in the
  install sequence, after the uninstall.

  This problem was reported to InstallAnywhere support as incident
  #SIOA-000116325, and the work order for the fix is #IOA-000031200.

  Increased the delay to 30 seconds for this problem:
  Bug #2972267 - jre sometimes not installed during an upgrade install

  On fast systems, if you are doing an upgrade install of STAF, the jre
  directory (i.e. C:/STAF/jre) may be missing after the install completes.
  All of the STAF files are installed correctly, it's just he bundled JVM which
  is missing.  This appears to be a timing issue, which is why it only happens
  on fast systems.  If you are doing an upgrade install, before any files are
  installed, the uninstaller for the existing version of STAF is executed (and
  its bundled JVM is used to run the uninstaller).  Then (after the uninstall
  is complete but the JVM used to run the uninstaller may still be shutting
  down) the STAF files are installed, and at the end of the install the bundled
  JVM is installed to the jre directory.  On a fast system, the files can get
  installed very quickly, and when the installer tries to install the bundled
  JVM, the JVM that was used for the uninstall is still in use, so the
  installer does not put the bundled JVM in the jre directory.  Then when the
  uninstaller JVM really does exit, it removes the jre directory.  So, you end
  up without a jre directory.

  This problem was reported to InstallAnywhere support as Issue #BZ-17127
*/

public class STAFDelay extends CustomCodeAction
{

    public void install(InstallerProxy ip)
    {
        try
        {
            Thread.sleep(30000);
        }
        catch (InterruptedException ex)
        {
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