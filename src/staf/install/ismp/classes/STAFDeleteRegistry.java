/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.util.*;
import com.installshield.wizard.platform.win32.*;

public class STAFDeleteRegistry extends WizardAction
{      
    public void build(WizardBuilderSupport support)
    {
        support.putRequiredService(Win32RegistryService.NAME);
    }

    public void execute(WizardBeanEvent event)
    {
        try
        {
            Win32RegistryService wrs = 
                (Win32RegistryService)getService(Win32RegistryService.NAME);
            
            String parentKey = "SOFTWARE\\IBM";  
            String stafKey = "STAF - Software Testing Automation Framework";
            String versionKey = "3.0.0 Beta 4";
            
            boolean exists = wrs.keyExists(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                parentKey + "\\" + stafKey + "\\" + versionKey);         
            
            if (exists)
            {
                wrs.deleteKey(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                              parentKey + "\\" + stafKey, versionKey, true);
            }
            
            String subkeyNames[] = wrs.getSubkeyNames(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey);
                
            if (subkeyNames.length == 0)
            {
                wrs.deleteKey(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                              parentKey, stafKey, true);
            }
        }
        catch (ServiceException ex)
        {
            ex.printStackTrace();
        }
    }    
}