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

public class STAFDeleteScripts extends WizardAction
{
    String scriptFileName = "";
    
    public void execute(WizardBeanEvent event)
    {
        
        String fileSep = System.getProperty("file.separator");
        String installLocation = resolveString("$P(absoluteInstallLocation)");
        String platformDetails = Platform.currentPlatform.toString();
        
        if (platformDetails.indexOf("name=Windows") > -1)
        {
            scriptFileName = installLocation + fileSep + "STAFEnv.bat";
        }
        else
        {
            scriptFileName = installLocation + fileSep + "STAFEnv.sh";
        }
        
        File scriptFile = new File(scriptFileName);
        
        if (scriptFile.exists())
        {
            scriptFile.delete();
        }
    }
}