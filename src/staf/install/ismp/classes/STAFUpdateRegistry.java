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
import com.installshield.product.wizardbeans.SetupTypePanel;

public class STAFUpdateRegistry extends WizardAction
{      
    public void build(WizardBuilderSupport support)
    {
        support.putRequiredService(Win32RegistryService.NAME);
    }

    public void execute(WizardBeanEvent event)
    {
        // NOTE: If you are updating the version number in this file, you 
        // also need to update STAFDeleteRegistry.java, STAFGetRegistry.java,
        // and STAFOlderVersionPanelSwingImpl

        try
        {
            Win32RegistryService wrs = 
                (Win32RegistryService)getService(Win32RegistryService.NAME);
            
            String parentKey = "SOFTWARE\\IBM";  
            String stafKey = "STAF - Software Testing Automation Framework";
            String versionKey = "3.0.0 Beta 4";
                
            boolean exists = wrs.keyExists(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                parentKey + stafKey);         
            
            if (!exists)
            {
                wrs.createKey(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                              parentKey, stafKey);
            }

            wrs.createKey(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                          parentKey + "\\" + stafKey, versionKey);       
                         
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Directory", false, 
                resolveString("$P(absoluteInstallLocation)"));
                
            SetupTypePanel setupPanel = 
                (SetupTypePanel)getWizardTree().findWizardBean("setupTypes");
                
            String setupType = setupPanel.getSelectedSetupTypeId();
            
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Install Type", false, setupType);
            
            String services = "";
            boolean firstService = true;
               
            if (resolveString("$P(logService.active)").equals("true"))
            {               
                if (!firstService) services += ";";
                firstService = false;
                
                services += resolveString("$P(logService.description)");
            }
            
            if (resolveString("$P(monitorService.active)").equals("true"))
            {
                if (!firstService) services += ";";
                firstService = false;
                
                services += 
                    resolveString("$P(monitorService.description)");
            }
            
            if (resolveString("$P(resourcePoolService.active)").equals("true"))
            {
                if (!firstService) services += ";";
                firstService = false;
                
                services += 
                    resolveString("$P(resourcePoolService.description)");
            }
            
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Services", false, services);     
            
            String languages = "";
            boolean firstLanguage = true;
             
            if (resolveString("$P(cSupport.active)").equals("true"))
            {
                if (!firstLanguage) languages += ";";
                firstLanguage = false;         
                
                languages += resolveString("$P(cSupport.description)");
            }
   
            if (resolveString("$P(javaSupport.active)").equals("true"))
            {
                if (!firstLanguage) languages += ";";
                firstLanguage = false;         
                
                languages += resolveString("$P(javaSupport.description)");
            }
            
            if (resolveString("$P(rexxSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinuxOrAix)").equals("true"))
                {
                    if (!firstLanguage) languages += ";";
                    firstLanguage = false;         
                
                    languages += resolveString("$P(rexxSupport.description)");
                }
            }
            
            if (resolveString("$P(tclSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinux)").equals("true"))                
                {
                    if (!firstLanguage) languages += ";";
                    firstLanguage = false;         
                
                    languages += resolveString("$P(tclSupport.description)");
                }
            }

            if (resolveString("$P(pythonSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinux)").equals("true"))                
                {
                    if (!firstLanguage) languages += ";";
                    firstLanguage = false;         
                
                    languages += resolveString("$P(pythonSupport.description)");
                }
            }
            
            if (resolveString("$P(perlSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinux)").equals("true"))                
                {
                    if (!firstLanguage ) languages += ";";
                    firstLanguage = false;         
                
                    languages += 
                        resolveString("$P(perlSupport.description)");
                }
            }
            
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Languages", false, languages);
            
            String docs;    
            
            if (resolveString("$P(documentation.active)").equals("true"))
            {
                docs = "yes";
            }
            else
            {
                docs = "no";
            }
            
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Documentation", false, docs);
     
            String serviceDev;           
            
            if (resolveString("$P(serviceDeveloper.active)").equals("true"))
            {
                serviceDev = "yes";
            }
            else
            {
                serviceDev = "no";
            }
            
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Service Developement support", false, serviceDev);
             
            String codepages;   
            
            if (resolveString("$P(codepageSupport.active)").equals("true"))
            {
                codepages = "yes";
            }
            else 
            {
                codepages = "no";
            }
            
            wrs.setStringValue(GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                parentKey + "\\" + stafKey + "\\" + versionKey,
                "Full codepage support", false, codepages);

        }
        catch (ServiceException ex)
        {
            ex.printStackTrace();
        }
    }    
}