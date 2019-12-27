/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.wizard.service.security.*;
import com.installshield.util.*;
import com.installshield.product.service.product.*;
import java.io.*;

public class STAFAdminOrRoot extends WizardAction
{

    private boolean isAdminOrRoot = false;

    public void execute(WizardBeanEvent event)
    {
        try
        {            
            SecurityService secService = 
                (SecurityService)getService(SecurityService.NAME);
               
            setIsAdminOrRoot(secService.isCurrentUserAdmin());
       }    
       catch(ServiceException e) 
       {
       }
    }
    
    public void build(WizardBuilderSupport support)
    {
        support.putRequiredService(SecurityService.NAME);
    }

    public boolean getIsAdminOrRoot()
    {
        return isAdminOrRoot;     
    }
    
    public void setIsAdminOrRoot(boolean bool)
    {       
        isAdminOrRoot = bool;
    }   

}