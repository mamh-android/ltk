/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.util.*;
import com.installshield.wizard.platform.win32.*;
import com.installshield.product.service.desktop.*;
import java.io.*;

public class STAFRemoveLinks extends WizardAction
{      
    public void execute(WizardBeanEvent event)
    {
        String installLocation = 
            resolveString("$P(absoluteInstallLocation)");
            
        try
        {                          
            String command = "rm -f " + installLocation + "/bin/staf";
            Process pid = Runtime.getRuntime().exec(command);
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        
        try
        {                          
            String command = "rm -f /usr/bin/staf";
            Process pid = Runtime.getRuntime().exec(command);        
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        
        try
        {           
            String command = "rm -f " + installLocation + "/lib/libPLSTAF.so";
            Process pid = Runtime.getRuntime().exec(command);
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        try
        {           
            String command = "rm -f " + installLocation + "/lib/libSTAF.a";
            Process pid = Runtime.getRuntime().exec(command);
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }        
    }   
}