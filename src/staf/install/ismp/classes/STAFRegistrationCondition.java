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

public class STAFRegistrationCondition extends WizardBeanCondition
{
    public boolean evaluateTrueCondition()
    {
        String selectedOptions = "";        
        
        if (this.getWizardBean().getServices().resolveString(
                "$W(stafPlatform.windows)").equals("true"))
        {
            selectedOptions = this.getWizardBean().getServices(). 
                resolveString("$W(windowsOptions.windowsChoices)");
        }
        else
        {
            selectedOptions = this.getWizardBean().getServices().
                resolveString("$W(unixOptions.unixChoices)");
        }        

        if (selectedOptions.indexOf("Allow STAF to Register") > -1)
        {
            return true;
        }        
        else
        {        
            return false;
        }
    }
    
    public String defaultName()
    {
        return "STAF Registration condition";
    }
    
    public String describe()
    {
        return "Returns true if the registration panel is to be displayed.";
    }
}