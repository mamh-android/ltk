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

public class STAFWriteRegFile extends WizardAction
{
    String registrationCompletionFileName = "STAFReg.cmp";
    String registrationInfoFileName = "STAFReg.inf";

    public void execute(WizardBeanEvent event)
    {
        String fileSep = System.getProperty("file.separator");
        String installLocation = resolveString("$P(absoluteInstallLocation)");
        String regFileName =  installLocation + fileSep +
            registrationInfoFileName;

        File stafRegFile = new File(regFileName);

        if (stafRegFile.exists())
        {
            stafRegFile.delete();
        }

        FileWriter writer = null;
        String lineSep = System.getProperty("line.separator");

        try
        {
            writer = new FileWriter(stafRegFile);

            String name = resolveString("$W(stafOptions.registrationName)");
            String email = resolveString("$W(stafOptions.registrationEmail)");
            String org =
                resolveString("$W(stafOptions.registrationOrganization)");

            writer.write("name:" + name + lineSep);
            writer.write("email:" + email + lineSep);
            writer.write("org:" + org + lineSep);

            writer.close();

            // Now we need to delete any STAFReg.cmp files residing in the
            // installation directory
            removeRegistrationCompletionFiles(new File(installLocation));
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }

    public void removeRegistrationCompletionFiles(File root)
    {
        if (root != null && root.isDirectory())
        {
            File[] contents = root.listFiles();

            for (int i = 0; i < contents.length; i++)
            {
                if (contents[i].isDirectory())
                {
                    removeRegistrationCompletionFiles(contents[i]);
                }
                else
                {
                    if (contents[i].getName().equals(
                        registrationCompletionFileName))
                    {
                        File completionFile = new
                            File(contents[i].getAbsolutePath());

                        completionFile.delete();
                    }
                }
            }
        }
    }
}