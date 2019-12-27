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

public class STAFWriteCfgFile extends WizardAction
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
            try
            {
                FileReader fileReader = new FileReader(cfgFileName);
                BufferedReader reader = new BufferedReader(fileReader);
                String line;
                boolean found2XConfigFile = false;

                while ((line = reader.readLine()) != null)
                {
                    line = line.toUpperCase();

                    if (line.startsWith("INTERFACE"))
                    {
                        if (line.indexOf("LIBRARY") == -1)
                        {
                            // We found an INTERFACE statement without
                            // the LIBRARY option, so this must be a 2.x
                            // configuration file
                            found2XConfigFile = true;
                        }
                    }
                }

                fileReader.close();

                if (!found2XConfigFile)
                {
                    // We found a STAF.cfg file that is not a 2x configuration
                    // file, so return since we do not want to over-write it
                    return;
                }
            }
            catch (FileNotFoundException ex)
            {
                ex.printStackTrace();
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }

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
            
            writer.write("# Turn on tracing of internal errors and deprecated options" + lineSep);
            writer.write("trace enable tracepoints \"error deprecated\"" + lineSep + lineSep);

            writer.write("# Enable TCP/IP connections" + lineSep);
            writer.write("interface tcp library STAFTCP" + lineSep + lineSep);
            
            writer.write("# Set default local trust" + lineSep);
            writer.write("trust machine local://local level 5" + lineSep + lineSep);    
            
            writer.write("# Default Service Loader Service" + lineSep);
            writer.write("serviceloader library " + defaultSLS + lineSep);

            writer.close();           
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }        
    }
}
