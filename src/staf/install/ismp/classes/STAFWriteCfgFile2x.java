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

public class STAFWriteCfgFile2x extends WizardAction
{
    public void execute(WizardBeanEvent event)
    {
        String defaultSLS;

        String installLocation = resolveString("$P(absoluteInstallLocation)");        
        String fileSep = System.getProperty("file.separator");
        String cfgFileName = installLocation +
            fileSep + "bin" + fileSep + "STAF.cfg";
        
        defaultSLS = "STAFDSLS";
            
        File stafCfgFile = new File(cfgFileName);
        
        if (stafCfgFile.exists())
        {
            File existingBackup = new File(cfgFileName + ".ismp.bak");
            
            if (existingBackup.exists())
            {
                existingBackup.delete();
            }
            
            stafCfgFile.renameTo(new File(cfgFileName + ".ismp.bak"));
            stafCfgFile.delete();
        }
        
        FileWriter writer = null;
        String lineSep = System.getProperty("line.separator");
        
        try
        {
            writer = new FileWriter(stafCfgFile);

            writer.write("# Enable TCP/IP connections" + lineSep);
            writer.write("interface tcpip" + lineSep + lineSep);
            writer.write("# Turn on tracing of internal errors and deprecated options" + lineSep);
            writer.write("trace on error deprecated" + lineSep + lineSep);
                
            
            writer.write("# Default Service Loader Service" + lineSep);
            
            writer.write("serviceloader Library " + defaultSLS + lineSep);

            writer.close();           
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }        
    }
}