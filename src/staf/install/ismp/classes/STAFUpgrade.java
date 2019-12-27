/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.product.service.registry.*;
import com.installshield.wizard.service.*;
import com.installshield.product.*;
import com.installshield.wizard.awt.MessageDialog;
import com.installshield.util.Log;
import com.installshield.wizard.platform.win32.*;
import com.installshield.product.service.product.*;
import com.installshield.util.*;

public class STAFUpgrade extends WizardAction
{
    public void build(WizardBuilderSupport support)
    {
        support.putRequiredService(Win32RegistryService.NAME);
    }
    
    public void execute(WizardBeanEvent evnt)
    {
        String upgradeVersion =
            resolveString("$W(findInstances.upgradeVersion)");
        String windows = resolveString("$W(stafPlatform.Windows)");

        if (upgradeVersion.equals("a pre-V3.0 version of STAF"))
        {
            String platform = Platform.currentPlatform.toString();

            if (platform.indexOf("name=Windows") > -1)
            {
                // Remove the Windows registry
                try
                {
                    Win32RegistryService wrs = (Win32RegistryService)getService
                        (Win32RegistryService.NAME);

                    String parentKey = "SOFTWARE\\IBM";
                    String stafKey =
                        "STAF - Software Testing Automation Framework";

                    boolean exists = wrs.keyExists(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        parentKey + "\\" + stafKey);

                    if (exists)
                    {
                        wrs.deleteKey(
                            GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                            parentKey, stafKey, true);
                    }
                }
                catch (ServiceException ex)
                {
                    ex.printStackTrace();
                }
            }
        }
    }
}
