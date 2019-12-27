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
import java.io.*;

public class STAFDeleteUninstallScript extends WizardAction
{
    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String scriptFileName = resolveString("$P(absoluteInstallLocation)") +
            fileSep + "STAFUninst";    
            
        File scriptFile = new File(scriptFileName);
        
        if (scriptFile.exists())
        {
            scriptFile.delete();            
        }                            
    }
}