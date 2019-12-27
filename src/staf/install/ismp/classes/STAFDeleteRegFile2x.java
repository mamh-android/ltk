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

public class STAFDeleteRegFile2x extends WizardAction
{
    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String regFileName = resolveString("$P(absoluteInstallLocation)") +
            fileSep + "STAFReg.inf";
        String cmpFileName = resolveString("$P(absoluteInstallLocation)") +
            fileSep + "STAFReg.cmp";
            
        File stafRegFile = new File(regFileName);
        
        if (stafRegFile.exists())
        {
            stafRegFile.delete();
        }       
        
        File cmpFile = new File(cmpFileName);
        
        if (cmpFile.exists())
        {
            cmpFile.delete();
        }
    }
}