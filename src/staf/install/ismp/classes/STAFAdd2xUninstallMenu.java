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
import com.installshield.wizard.platform.win32.*;
import com.installshield.product.service.desktop.*;
import java.io.*;

public class STAFAdd2xUninstallMenu extends WizardAction
{
    private Win32RegistryService wrs;
    String key = "SOFTWARE\\IBM\\STAF - Software Testing Automation Framework";
    
    private String add2xUninstallMenu = "false";
    public String getAdd2xUninstallMenu() {     return add2xUninstallMenu;   }
    public void setAdd2xUninstallMenu(String str) { add2xUninstallMenu = str; }
    
    private String uninstallerLocation = "";
    public String getUninstallerLocation() {     return uninstallerLocation;   }
    public void setUninstallerLocation(String str) 
        {    uninstallerLocation = str;   }
    
    public void execute(WizardBeanEvent event)
    {
        try
        {
            wrs = (Win32RegistryService)getService(Win32RegistryService.NAME);                
            
            boolean exists = wrs.keyExists(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key);
                
            if (!exists)
                return;

            String subKeys[] = wrs.getSubkeyNames(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key);
            
            // There will always only be 1 subKey
            String subKey = subKeys[0];
            
            String location = wrs.getStringValue(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                key + "\\" + subKey, "Directory", false);
                
            location += "\\_uninst\\uninstaller.exe";
            uninstallerLocation = location;
            
            key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\" +
                "Explorer\\Shell Folders";
            
            exists = wrs.keyExists(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE, key);
                
            if (!exists)
                return;
                
            String programsLocation = wrs.getStringValue(
                GenericWin32RegistryService.HKEY_LOCAL_MACHINE,
                key , "Common Programs", false);
            
            programsLocation += "\\STAF - Software Testing Automation Framework";
            
            File programsFile = new File(programsLocation);
        
            if (programsFile.exists())
            {
                add2xUninstallMenu = "true";
            }
        }
        catch (ServiceException ex)
        {
            ex.printStackTrace();
        }
    }
}