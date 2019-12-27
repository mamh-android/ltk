/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2009                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;
public class STAFCheckPerlVersion extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        String usePerlSystemPath = ip.substitute("$USE_PERL_SYSTEM_PATH$");

        if (usePerlSystemPath.equals("1"))
        {
            Process p = null;
            java.util.Properties envVars = new java.util.Properties();
            Runtime r = Runtime.getRuntime();
            String line = "";
            String perlVersion = "";

            try
            {
                String command = "perl -v";

                p = r.exec(command);

                BufferedReader br = new
                    BufferedReader(new InputStreamReader(p.getInputStream()));

                // Look for a line like the following:
                //
                // This is perl, v5.6.1 built ...
                // This is perl, v5.8.9 built ...
                // This is perl, v5.10.0 built ...
                // This is perl 5, version 12, subversion 4 (v5.12.4) built ...
                // This is perl 5, version 14, subversion 2 (v5.14.2) built ...
                    
                while ((line = br.readLine()) != null)
                {                          
                    if (line.startsWith("This is perl"))
                    {
                        String findString = "v5.";
                        int versionIndex = line.indexOf(findString);
                        
                        if (versionIndex != -1)
                        {
                            // Find 2nd period (separates subversion from version)
                            int secondPeriod = line.indexOf(
                                '.', versionIndex + findString.length());
                            
                            if (secondPeriod != -1)
                            {    
                                // Assign the Perl version, e.g. 5.6, 5.8, 5.10,
                                // 5.12, 5.14, etc
                                perlVersion = line.substring(
                                    versionIndex + 1, secondPeriod);
                            }
                        }
                        
                        break;  // Break out of loop
                    }
                }
                
                if (perlVersion.equals("5.8") ||
                    perlVersion.equals("5.10") ||
                    perlVersion.equals("5.12") ||
                    perlVersion.equals("5.14") ||
                    perlVersion.equals("5.6"))
                {
                    ip.setVariable("$USE_PERL_VERSION$", perlVersion);
                }
            }
            catch (IOException e) { }
        }
    }

    public void uninstall(UninstallerProxy up)
    {
    }

    public String getInstallStatusMessage()
    {
        return "";
    }

    public String getUninstallStatusMessage()
    {
        return "";
    }
}