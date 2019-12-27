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

public class STAFAcceptLicense extends WizardAction
{
    private String selection = "";

    String message = "You must accept the STAF license in order to proceed " +
       "with the installation.  To accept the license add the following " +
       "option:\n\n-W license.selection=\"Accept\"";

    public void execute(WizardBeanEvent event)
    {
        if ((event.getUserInterface() == null) &&
            !(selection.equalsIgnoreCase("Accept")))
        {
            System.out.println(message);
            logEvent(this, Log.ERROR, message);
            getWizard().exit(1000);
        }
    }

    public String getSelection()
    {
        return this.selection;
    }

    public void setSelection(String selection)
    {
        this.selection = selection;
    }
}