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

public class STAFDeleteRegFile extends WizardAction
{
    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String regFileName = resolveString("$P(absoluteInstallLocation)") +
            fileSep + "STAFReg.inf";
            
        File stafRegFile = new File(regFileName);
        
        if (stafRegFile.exists())
        {
            stafRegFile.delete();
        }       
        
        // Note: Cannot delete STAFReg.cmp file since it's stored in the
        // {STAF/DataDir}/registration directory and the {STAF/DataDir}
        // variable is not available since STAF is not running during it's
        // un-install.
    }
}