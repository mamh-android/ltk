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

public class STAFPlatform extends WizardAction
{
    public boolean windows = false;
    public boolean linux = false;
    public boolean aix = false;
    public boolean solaris = false;
    public boolean unix = false;    
    public boolean hpux = false;   
    public String platformDetails = "";    

    public void execute(WizardBeanEvent event)
    {
        platformDetails = Platform.currentPlatform.toString();

        if (platformDetails.indexOf("Windows") > -1)
        {
            windows = true;
        }
        else if (platformDetails.indexOf("Linux") > -1)
        {
            linux = true;
            unix = true;
        }
        else if (platformDetails.indexOf("AIX") > -1)
        {
            aix = true;
            unix = true;
        }
        else if (platformDetails.indexOf("SunOS") > -1)
        {
            solaris = true;
            unix = true;
        }
        else if (platformDetails.indexOf("HP-UX") > -1)
        {
            hpux = true;
            unix = true;
        }
        else
        {
            unix = true;
        }
    }
    
    public boolean getWindows()
    {
        return windows;
    }
    
    public void setWindows(boolean bool)
    {
        windows = bool;
    }   
    
    public boolean getUnix()
    {
        return unix;
    }
    
    public void setUnix(boolean bool)
    {
        unix = bool;
    }
    
    public boolean getLinux()
    {
        return linux;
    }
    
    public void setLinux(boolean bool)
    {
        linux = bool;
    }

    public boolean getAix()
    {
        return aix;
    }
    
    public void setAix(boolean bool)
    {
        aix = bool;
    }
    
    public boolean getSolaris()
    {
        return solaris;
    }
    
    public void setSolaris(boolean bool)
    {
        solaris = bool;
    }
    
    public boolean getHpux()
    {
        return hpux;
    }
    
    public void setHpux(boolean bool)
    {
        hpux = bool;
    }
    
    public boolean getWinOrLinux()
    {        
        return windows || linux;
    }
    
    public boolean getWinOrLinuxOrAix()
    {        
        return windows || linux || aix;
    }
    
    public String getPlatformDetails()
    {
        return platformDetails;
    }
    
    public void setPlatformDetails(String details)
    {
        platformDetails = details;
    }

}