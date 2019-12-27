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

public class STAFCreateUninstallScript extends WizardAction
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
        
        FileWriter writer = null;
        String lineSep = System.getProperty("line.separator");
        String installLocation = resolveString("$P(absoluteInstallLocation)");
        
        try
        {
            writer = new FileWriter(scriptFile);

            writer.write("#!/bin/sh" + lineSep);
            writer.write(installLocation + "/_uninst/uninstaller.bin" + 
                         lineSep);
            
            writer.close();           
            
            String command = "chmod 777 " + scriptFileName;
            Process pid = Runtime.getRuntime().exec(command);
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }        
    }
}