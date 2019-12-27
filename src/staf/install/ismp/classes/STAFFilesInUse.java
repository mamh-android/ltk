/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.util.*;
import com.installshield.product.service.product.*;

public class STAFFilesInUse extends WizardAction
{
    String title = "Reboot system";
    String message = "To process locked files, please restart the " +
       "system now.  After the system reboots, restart the STAF " +
       "installer to complete the installation.";
   
    public void execute(WizardBeanEvent event)
    {
        if (event.getUserInterface() != null)
        {
	        // This is not silent mode, so we can display a message	 
	        event.getUserInterface().displayUserMessage(title, message,
	            UserInputRequest.MESSAGE);
        }
        else
        {
            System.out.println(message);
        }
        
        getWizard().exit(1000);
    }
}