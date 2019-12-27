/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;
public class STAFCreateInstallProperties extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        String installLocation = ip.substitute("$USER_INSTALL_DIR$");
        String fileSep = System.getProperty("file.separator");
        String installPropertiesFileName = installLocation + fileSep +
            "install.properties";
        String versionProperty = "";
        final String versionKey = "version=";
        String platformProperty = "";
        final String platformKey = "platform=";
        String installerProperty = "";
        final String installerKey = "installer=";
        String architectureProperty = "";
        final String architectureKey = "architecture=";
        String osnameProperty = "";
        final String osnameKey = "osname=";
        String osversionProperty = "";
        final String osversionKey = "osversion=";
        String osarchProperty = "";
        final String osarchKey = "osarch=";
        String fileProperty = "";
        final String fileKey = "file=";

        try
        {
            FileReader fileReader = new FileReader(installPropertiesFileName);
            BufferedReader reader = new BufferedReader(fileReader);
            String line;

            while ((line = reader.readLine()) != null)
            {
                if (line.startsWith(versionKey))
                {
                    versionProperty = line;
                }
                else if (line.startsWith(platformKey))
                {
                    platformProperty = line;
                }
                else if (line.startsWith(installerKey))
                {
                    installerProperty = line;
                }
                else if (line.startsWith(fileKey))
                {
                    fileProperty = line;
                }
                else if (line.startsWith(architectureKey))
                {
                    architectureProperty = line;
                }
                else if (line.startsWith(osnameKey))
                {
                    osnameProperty = line;
                }
                else if (line.startsWith(osversionKey))
                {
                    osversionProperty = line;
                }
                else if (line.startsWith(osarchKey))
                {
                   osarchProperty = line;
                }
            }

            fileReader.close();

            String installer = ip.substitute("$EXTRACTOR_EXECUTABLE$");
            File extractorExecutable = new File(installer);
            String executableFileName = extractorExecutable.getName();
            fileProperty = "file=" + executableFileName;

            if (executableFileName.indexOf("NoJVM") > -1)
            {
                installerProperty = "installer=IA_NoJVM";
            }
            else if (platformProperty.startsWith("platform=macosx"))
            {
                // macosx only provides a NOJVM installer
                installerProperty = "installer=IA_NoJVM";
            }
            else
            {
                installerProperty = "installer=IA";
            }

            File installPropertiesFile = new File(installPropertiesFileName);
            installPropertiesFile.delete();

            FileWriter writer = null;
            writer = new FileWriter(installPropertiesFile);
            String lineSep = System.getProperty("line.separator");
            writer.write(versionProperty + lineSep);
            writer.write(platformProperty + lineSep);
            writer.write(architectureProperty + lineSep);
            writer.write(installerProperty + lineSep); 
            writer.write(fileProperty + lineSep);
            writer.write(osnameProperty + lineSep);
            writer.write(osversionProperty + lineSep);
            writer.write(osarchProperty + lineSep);
            writer.close();
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