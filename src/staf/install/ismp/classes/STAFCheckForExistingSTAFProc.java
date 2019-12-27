/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.wizard.service.system.*;
import com.installshield.util.*;
import com.installshield.product.service.product.*;
import java.io.*;

public class STAFCheckForExistingSTAFProc extends WizardAction
{
    private boolean stafProcExists = false;
    private boolean stafProcRunning = false;
    private String stafProcBinary = "";
    private String stafClientBinary = "";
    
    public void execute(WizardBeanEvent event)
    {
        String platform = Platform.currentPlatform.toString();

        if (platform.indexOf("name=Windows") > -1)
        {
            stafProcBinary = "STAFProc.exe";
            stafClientBinary = "STAF.exe";
        }
        else
        {
            stafProcBinary = "STAFProc";
            stafClientBinary = "STAF";
        }
        
        String installLocation = resolveString("$P(absoluteInstallLocation)");
        String fileSep = System.getProperty("file.separator");
        String stafProcFileName = installLocation
            + fileSep + "/bin" + fileSep + stafProcBinary;
            
        File stafProcFile = new File(stafProcFileName);
        
        setStafProcExists(stafProcFile.exists());

        String windows = resolveString("$W(stafPlatform.Windows)");
        
        if (stafProcExists && (platform.indexOf("name=Windows") > -1))
        {
            try
            {           
                String command = installLocation + "/bin/" + stafClientBinary +
                    " local ping ping";
                Process pid = Runtime.getRuntime().exec(command);
                
                BufferedReader br = new BufferedReader(new
                    InputStreamReader(pid.getInputStream()));
                
                String str = null;
                while ((str = br.readLine()) != null)
                {
                    // Do nothing
                }
                
                int rc = pid.exitValue();
                
                if (rc == 0)
                {
                    setStafProcRunning(true);
                }
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            } 
        }
    }
    
    public boolean getStafProcExists()
    {
        return stafProcExists;     
    }
    
    public void setStafProcExists(boolean bool)
    {       
        stafProcExists = bool;
    }
    
    public boolean getStafProcRunning()
    {
        return stafProcRunning;
    }
    
    public void setStafProcRunning(boolean bool)
    {       
        stafProcRunning = bool;
    }
}