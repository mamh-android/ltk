/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.util.*;
import com.installshield.product.service.product.*;
import java.io.*;

public class STAFGetWindowsStartupFolder extends WizardAction
{

    String windowsStartupFolder = "";

    public void execute(WizardBeanEvent event)
    {
        // Obtain the Windows Registry value for the startup folder, which is
        // the full path to the folder.  Then use File.getName() to determine
        // the name of the start folder directory, which is specific to the
        // operating system language, and set windowsStartupFolder to this
        // value.  For example, on an English system, the registry value could
        // be:  "C:\Documents and Settings\Adminstrator\Start Menu\Programs\Startup".
        // windowsStartupFolder would be set to "Startup" in this example.

        String startupFolderValue =
            resolveString("$W(WindowsGetRegistryStartupFolder.value)");

        File startupFolder = new File(startupFolderValue);
        setWindowsStartupFolder(startupFolder.getName());
    }

    public String getWindowsStartupFolder()
    {
        return windowsStartupFolder;
    }

    public void setWindowsStartupFolder(String str)
    {
        windowsStartupFolder = str;
    }
}
