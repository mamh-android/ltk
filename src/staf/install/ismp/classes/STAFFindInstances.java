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
import com.installshield.util.*;
import java.util.*;
import java.io.*;

public class STAFFindInstances extends WizardAction
{
    private String uid = "f0cdd2f9b6b390509acd38f3769a665b";
    public String getKey() {     return uid;   }
    public void setKey(String str) {    uid = str;   }
    
    private String isUpgrade = "false";
    public String getIsUpgrade() {     return isUpgrade;   }
    public void setIsUpgrade(String str) {    isUpgrade = str;   }
    
    private String twoXInstalled = "false";
    public String getTwoXInstalled() {     return twoXInstalled;   }
    public void setTwoXInstalled(String str) {    twoXInstalled = str;   }
    
    private String twoXInstallLocation = "";
    public String getTwoXInstallLocation() {     return twoXInstallLocation;   }
    public void setTwoXInstallLocation(String str) 
        { twoXInstallLocation = str; }
    
    private String upgradeVersion = "";
    public String getUpgradeVersion() {     return upgradeVersion;   }
    public void setUpdateVersion(String str) {    upgradeVersion = str;   }
    
    public String installedVersions = "";
    public String getInstalledVersions() { return installedVersions; }
    public void setInstalledVersions(String str) { installedVersions = str; }
    
    private String otherVersionsExist = "false";
    public String getOtherVersionsExist() {     return otherVersionsExist;   }
    public void setOtherVersionsExist(String str) { otherVersionsExist = str; }

    private String installedToOtherLocation = "";
    public String getInstalledToOtherLocation() 
        {     return installedToOtherLocation;   }
    public void setInstalledToOtherLocation(String str) 
        { installedToOtherLocation = str; }
    
    private HashMap currentVersions = new HashMap();
    
    public void execute(WizardBeanEvent evnt)
    {
        isUpgrade = "false";
        twoXInstalled = "false";
        twoXInstallLocation = "";
        upgradeVersion = "";
        installedToOtherLocation = "";
        installedVersions = "";
        currentVersions.clear();

        // Construct appropriate SoftwareObjectKey for the product in question
        SoftwareObjectKey sok = new SoftwareObjectKey();
        sok.setUID(uid);
        SoftwareObject myProduct = null;
        int productInstance = 0;

        try
        {
            // Get the instance of RegistryService
            RegistryService regserv =
                (RegistryService)getService(RegistryService.NAME);

            // Get the product's instance in the VPD
            productInstance = regserv.getNewestInstance(sok);
            SoftwareObject [] objs = regserv.getSoftwareObjects(uid);
            
            String targetInstallLocation = 
                resolveString("$P(absoluteInstallLocation)");
            
            // Determine all versions of STAF installed on the machine
            for (int i = 0; i < objs.length; i++)
            {
                String major = objs[i].getKey().getVersion().getMajor();
                String minor = objs[i].getKey().getVersion().getMinor();
                String update = objs[i].getKey().getVersion().getUpdate();
                String version = "";
                String installLocation = objs[i].getInstallLocation();

                // Make sure that bin/STAFProc(.exe) exists in
                // the install location

                String platform = Platform.currentPlatform.toString();
                String stafProcBinary = "";

                if (platform.indexOf("Windows") > -1)
                {
                    stafProcBinary = "STAFProc.exe";
                }
                else
                {
                    stafProcBinary = "STAFProc";
                }

                String fileSep = System.getProperty("file.separator");
                String stafProcFileName = installLocation +
                    fileSep + "bin" + fileSep + stafProcBinary;

                File stafProcFile = new File(stafProcFileName);

                if (!(stafProcFile.exists()))
                {
                    continue;
                }

                if (major.equals("2"))
                {
                    twoXInstalled = "true";
                    version = "a pre-V3.0 version of STAF";
                    currentVersions.put(installLocation, version);
                    installedVersions += installLocation + " (" + version +
                        ")<br>";
                    twoXInstallLocation = installLocation;
                }
                else
                    {
                        version = major + "." + 
                            minor.substring(0, 1) +
                            "." + minor.substring(1, 2) + " " + update;
                        currentVersions.put(installLocation, version);
                        installedVersions += installLocation + " (" + version +
                        ")<br>";
                    }
            }
            
            Set set = currentVersions.keySet();
            Iterator iter = set.iterator();
            
            while (iter.hasNext())
            {
                String location = (String)iter.next();
                String version = (String)currentVersions.get(location);
                
                // make sure install locations match
                if (targetInstallLocation.equalsIgnoreCase(location))
                {
                    isUpgrade = "true";
                    
                    if (version.equals("a pre-V3.0 version of STAF"))
                    {
                        upgradeVersion = version;
                    }
                    else
                    {
                        upgradeVersion = "STAF Version " + version;
                    }
                }
                else
                {
                    otherVersionsExist = "true";
                    String newVersion = 
                        resolveString("$W(stafVersion.version)");
                    
                    if (version.equals(newVersion))
                    {
                        installedToOtherLocation = location;
                    }
                }
            }
        }
        catch (ServiceException se)
        {
            se.printStackTrace();
            System.out.println(se.getData());
        }
    }
}
