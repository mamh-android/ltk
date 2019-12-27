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

public class STAFDefaultInstallLocation extends WizardAction
{
    private String defaultInstallLocation = "";    

    public void execute(WizardBeanEvent event)
    {
        // Do not override the default install location if the user
        // has already specified the location during a silent install
     
        if (defaultInstallLocation.equals(""))
        {         
            if (resolveString("$W(stafPlatform.windows)").equals("true"))
            {
                defaultInstallLocation = "C:\\STAF";
            }
            else
            {
                defaultInstallLocation = "/usr/local/staf";
            }
        }
                
        ProductService service = null;

        try
        {
            service = (ProductService)getService(ProductService.NAME);
            
            service.setProductBeanProperty(
                        ProductService.DEFAULT_PRODUCT_SOURCE,
                        null, "installLocation", defaultInstallLocation);
        }
        catch (ServiceException ex)
        {
            ex.printStackTrace();
        }
    }
    
    public String getDefaultInstallLocation()
    {
        return defaultInstallLocation;
    }
    
    public void setDefaultInstallLocation(String location)
    {
        defaultInstallLocation = location;
    }    
    
}