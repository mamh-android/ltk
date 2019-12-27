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
import com.installshield.product.service.desktop.*;
import java.io.*;

public class STAFGetRegistry extends WizardAction
{
    private boolean previousVersionInstalled = false;
    private String previousVersionNumber = "";
    private String previousVersionDirectory = "";
    private String previousVersionInstallType = "";
    
    private Win32RegistryService wrs;
    String key = "SOFTWARE\\IBM\\STAF - Software Testing Automation Framework";
      
    public void build(WizardBuilderSupport support)
    {
        support.putRequiredService(Win32RegistryService.NAME);
    }

    public void execute(WizardBeanEvent event)
    {
        try
        {
            wrs = (Win32RegistryService)getService(Win32RegistryService.NAME);                
            
            boolean exists = wrs.keyExists(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key);
                
            setPreviousVersionInstalled(exists);
            
            if (exists)
            {
                checkPrevious2xVersions();                            
                
                boolean exists231 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.3.1");
                            
                if (exists231)
                {
                    setPreviousVersionNumber("2.3.1");                        
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.3.1", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.3.1", "Install Type", false));
                }

                boolean exists232 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.3.2");
                            
                if (exists232)
                {
                    setPreviousVersionNumber("2.3.2");                        
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.3.2", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.3.2", "Install Type", false));
                }
                   
                boolean exists230 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.3.0");
                            
                if (exists230)
                {
                    setPreviousVersionNumber("2.3.0");                        
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.3.0", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.3.0", "Install Type", false));
                }

                boolean exists240 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.4.0");
                            
                if (exists240)
                {
                    setPreviousVersionNumber("2.4.0");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.0", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.0", "Install Type", false));
                }

                boolean exists241 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.4.1");
                            
                if (exists241)
                {
                    setPreviousVersionNumber("2.4.1");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.1", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.1", "Install Type", false));
                }

                boolean exists242 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.4.2");
                            
                if (exists242)
                {
                    setPreviousVersionNumber("2.4.2");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.2", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.2", "Install Type", false));
                }

                boolean exists243 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.4.3");
                            
                if (exists243)
                {
                    setPreviousVersionNumber("2.4.3");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.3", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.3", "Install Type", false));
                }

                boolean exists244 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.4.4");
                            
                if (exists244)
                {
                    setPreviousVersionNumber("2.4.4");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.4", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.4", "Install Type", false));
                }

                boolean exists245 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.4.5");

                if (exists245)
                {
                    setPreviousVersionNumber("2.4.5");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.5", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.4.5", "Install Type", false));
                }

                boolean exists250 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.5.0");

                if (exists250)
                {
                    setPreviousVersionNumber("2.5.0");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.5.0", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.5.0", "Install Type", false));
                }

                boolean exists251 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.5.1");

                if (exists251)
                {
                    setPreviousVersionNumber("2.5.1");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.5.1", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.5.1", "Install Type", false));
                }

                boolean exists252 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.5.2");

                if (exists252)
                {
                    setPreviousVersionNumber("2.5.2");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.5.2", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.5.2", "Install Type", false));
                }
                
                boolean exists260 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.6.0");

                if (exists260)
                {
                    setPreviousVersionNumber("2.6.0");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.0", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.0", "Install Type", false));
                }
                
                boolean exists261 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.6.1");

                if (exists261)
                {
                    setPreviousVersionNumber("2.6.1");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.1", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.1", "Install Type", false));
                }
                
                boolean exists262 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.6.2");

                if (exists262)
                {
                    setPreviousVersionNumber("2.6.2");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.2", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.2", "Install Type", false));
                }

                boolean exists263 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.6.3");

                if (exists263)
                {
                    setPreviousVersionNumber("2.6.3");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.3", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.3", "Install Type", false));
                }
                
                boolean exists264 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.6.4");

                if (exists264)
                {
                    setPreviousVersionNumber("2.6.4");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.4", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.4", "Install Type", false));
                }
                
                boolean exists265 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.6.5");

                if (exists265)
                {
                    setPreviousVersionNumber("2.6.5");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.5", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\2.6.5", "Install Type", false));
                }
                
                boolean exists300beta1 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\3.0.0 Beta 1");

                if (exists300beta1)
                {
                    setPreviousVersionNumber("3.0.0 Beta 1");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 1", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 1", "Install Type", false));
                }
                
                boolean exists300beta2 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\3.0.0 Beta 2");

                if (exists300beta2)
                {
                    setPreviousVersionNumber("3.0.0 Beta 2");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 2", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 2", "Install Type", false));
                }

                boolean exists300beta3 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\3.0.0 Beta 3");

                if (exists300beta3)
                {
                    setPreviousVersionNumber("3.0.0 Beta 3");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 3", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 3", "Install Type", false));
                }
                
                boolean exists300beta4 = wrs.keyExists(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\3.0.0 Beta 4");

                if (exists300beta4)
                {
                    setPreviousVersionNumber("3.0.0 Beta 4");
                    
                    setPreviousVersionDirectory(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 4", "Directory", false));
                
                    setPreviousVersionInstallType(wrs.getStringValue(
                        GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                        key + "\\3.0.0 Beta 4", "Install Type", false));
                }
            }
        }
        catch (ServiceException ex)
        {
            ex.printStackTrace();
        }
    }
    
    public void checkPrevious2xVersions()
    {      
        try
        {            
            String[] subKeys = wrs.getSubkeyNames(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key);
                
                // Pre-STAF2.3.0, there will only be one entry
            if (subKeys[0].equals("2.00"))
            {
                setPreviousVersionNumber("2.0.0");
            
                setPreviousVersionDirectory(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.00", "Directory", false));
                
                setPreviousVersionInstallType(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, 
                    key + "\\2.00", "Install Type", false));
            }
            else if (subKeys[0].equals("2.1.0"))
            {
                setPreviousVersionNumber("2.1.0");
            
                setPreviousVersionDirectory(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key,
                    "Directory", false));
                
                setPreviousVersionInstallType(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key,
                    "Install Type", false));
            }
            else if (subKeys[0].equals("2.2.0"))
            {
                setPreviousVersionNumber("2.2.0");
            
                setPreviousVersionDirectory(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key,
                    "Directory", false));
                
                setPreviousVersionInstallType(wrs.getStringValue(
                    GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key,
                    "Install Type", false));
            }
        }
        catch(ServiceException ex)
        {
            ex.printStackTrace();
        }
    }
    
    public boolean getPreviousVersionInstalled()
    {
        return previousVersionInstalled;     
    }
    
    public void setPreviousVersionInstalled(boolean bool)
    {       
        previousVersionInstalled = bool;
    }
    
    public String getPreviousVersionNumber()
    {
        return previousVersionNumber;     
    }
    
    public void setPreviousVersionNumber(String str)
    {       
        previousVersionNumber = str;
    }
    
    public String getPreviousVersionDirectory()
    {
        return previousVersionDirectory;     
    }
    
    public void setPreviousVersionDirectory(String str)
    {       
        previousVersionDirectory = str;
    }

    public String getPreviousVersionInstallType()
    {
        return previousVersionInstallType;     
    }
    
    public void setPreviousVersionInstallType(String str)
    {       
        previousVersionInstallType = str;
    }
}