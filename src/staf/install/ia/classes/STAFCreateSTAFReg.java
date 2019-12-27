/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFCreateSTAFReg extends CustomCodeAction
{
    String registrationCompletionFileName = "STAFReg.cmp";
    String registrationInfoFileName = "STAFReg.inf";

    public void install(InstallerProxy ip)
    {
        String fileSep = System.getProperty("file.separator");
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");

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

            String name = ip.substitute("$REGISTRATION_NAME$");
            String email = ip.substitute("$REGISTRATION_EMAIL$");
            String org = ip.substitute("$REGISTRATION_ORG$");

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

    public void uninstall(UninstallerProxy up)
    {
    }

    public String getInstallStatusMessage()
    {
        return "Creating the STAFReg.inf file";
    }

    public String getUninstallStatusMessage()
    {
        return "Deleting the STAFReg.inf";
    }
}