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

public class STAFVersion extends WizardAction
{
    private String version = "3.2.4";
    private String shortVersion = "324";

    public void execute(WizardBeanEvent event)
    {
        // Do nothing
    }
    
    public String getVersion()
    {
        return version;
    }

    public String getShortVersion()
    {
        return shortVersion;
    }

}