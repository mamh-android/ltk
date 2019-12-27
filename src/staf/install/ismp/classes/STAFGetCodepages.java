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

public class STAFGetCodepages extends WizardAction
{    
    private String acp = "";
    private String oemcp = "";
      
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
                
            String key = 
                "SYSTEM\\CurrentControlSet\\Control\\Nls\\Codepage";                
            
            setAcp(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key, "ACP", false));
                    
            setOemcp(wrs.getStringValue(
                     GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                     key, "OEMCP", false));            
        }
        catch (ServiceException ex)
        {
            ex.printStackTrace();
        }
    }    
    
    public String getAcp()
    {
        return acp;     
    }
    
    public void setAcp(String str)
    {       
        acp = str;
    }    
    
    public String getOemcp()
    {
        return oemcp;     
    }
    
    public void setOemcp(String str)
    {       
        oemcp = str;
    }  
}