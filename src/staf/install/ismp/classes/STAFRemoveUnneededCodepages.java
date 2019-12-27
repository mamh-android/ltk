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
import java.util.*;

public class STAFRemoveUnneededCodepages extends WizardAction
{
    public void execute(WizardBeanEvent event)
    {
        if (resolveString(
            "$W(setupTypes.selectedSetupTypeId)").equalsIgnoreCase("Full"))
        {
            return;
        }

        String fileSep = System.getProperty("file.separator");
        String installLoc = resolveString("$P(absoluteInstallLocation)");
        String codepageDir = installLoc + fileSep + "codepage";
        String aliasFileName = codepageDir + fileSep + "alias.txt";

        String acp = resolveString("$W(stafCodepages.acp)");
        String oemcp = resolveString("$W(stafCodepages.oemcp)");

        String acpIBM = "ibm-" + acp;
        String oemcpIBM = "ibm-" + oemcp;

        String acpFile = codepageDir + fileSep + acpIBM + ".bin";
        String oemcpFile = codepageDir + fileSep + oemcpIBM + ".bin";

        Vector requiredCodepages = new Vector();
        requiredCodepages.add(acpFile);
        requiredCodepages.add(oemcpFile);
        requiredCodepages.add(codepageDir + fileSep + "alias.txt");

        File aliasFile = new File(aliasFileName);

        if (aliasFile.exists())
        {
            // Parse the alias.txt file to determine the codepage file aliases
            try
            {
                FileReader fileReader = new FileReader(aliasFileName);
                BufferedReader reader = new BufferedReader(fileReader);
                String line;

                while ((line = reader.readLine()) != null)
                {
                    // Strip off any trailing comments
                    if (line.indexOf("#") > -1)
                    {
                        line = line.substring(0, line.indexOf("#"));
                    }

                    // Skip blank lines
                    if (line.equals(""))
                        continue;

                    StringTokenizer tokenizer = new StringTokenizer(line);
                    // Format is:  <actual-file-name> <alias1> <alias2> ...
                    String actualFileName = tokenizer.nextToken();
                    Vector aliases = new Vector();

                    while (tokenizer.hasMoreTokens())
                    {
                        String alias = tokenizer.nextToken();
                        aliases.add(alias);
                    }

                    if (aliases.contains(acpIBM) || aliases.contains(oemcpIBM))
                    {
                        requiredCodepages.add(codepageDir + fileSep +
                                              actualFileName + ".bin");
                    }
                }

                fileReader.close();
            }
            catch (FileNotFoundException ex)
            {
                ex.printStackTrace();
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }
        }

        File codepageDirFile = new File(codepageDir);
        File [] codepageFiles = codepageDirFile.listFiles();

        for (int i = 0; i < codepageFiles.length; i++)
        {
            String codePageFileName = codepageFiles[i].toString();

            if (!requiredCodepages.contains(codePageFileName))
            {
                codepageFiles[i].delete();
            }
        }
    }

    public void copy(InputStream in, OutputStream out)
    {
        synchronized(in)
        {
            synchronized(out)
            {
                try
                {
                    byte [] buffer = new byte[256];
                    while (true)
                    {
                        int bytesRead = in.read(buffer);

                        if (bytesRead == -1)
                        {
                            break;
                        }

                        out.write(buffer, 0, bytesRead);
                    }
                }
                catch (IOException ex)
                {
                    ex.printStackTrace();
                }
            }
        }
    }
}