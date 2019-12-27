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

public class STAFUnixExistingSTAFProc extends WizardAction
{
    private boolean stafProcExists = false;
    
    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String stafProcFileName = resolveString("$P(absoluteInstallLocation)")
            + fileSep + "/bin" + fileSep + "STAFProc";
            
        File stafProcFile = new File(stafProcFileName);
        
        setStafProcExists(stafProcFile.exists());
    }
    
    public boolean getStafProcExists()
    {
        return stafProcExists;     
    }
    
    public void setStafProcExists(boolean bool)
    {       
        stafProcExists = bool;
    }
}