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

public class STAFUninstallExistingSTAFProc extends WizardAction
{
    private String uninstaller = "";
    
    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String platform = Platform.currentPlatform.toString();
        String installLocation = resolveString("$P(absoluteInstallLocation)");
        
        // Try the uninstall this many times using _uninst, _uninst1, _uninst2,
        // _uninst3, _uninst4, _uinnst5, etc. until you have no exceptions
        int maxTries = 5;
        boolean attemptNextUninstall = true;

        int index = 0;
        while (attemptNextUninstall && (index <= maxTries))
        {
            attemptNextUninstall = false;
            String uninstDir = "_uninst";

            if (index > 0)
            {
                uninstDir = uninstDir + (new Integer(index)).toString();
            }

            if (platform.indexOf("name=Windows") > -1)
            {
                uninstaller =  installLocation +
                    fileSep + uninstDir + fileSep + "uninstaller.exe -silent";
            }
            else
            {
                uninstaller = resolveString("$P(absoluteInstallLocation)") + 
                    fileSep + uninstDir + fileSep + "uninstaller.bin -silent";
            }

            try
            {
                Process pid = Runtime.getRuntime().exec(uninstaller);
                pid.waitFor();
            }
            catch (InterruptedException ex)
            {
                ex.printStackTrace();
                attemptNextUninstall = true;
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
                attemptNextUninstall = true;
            }

            index++;
        }
    }

}