/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2009                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;
public class STAFCheckPythonVersion extends CustomCodeAction
{
    public void install(InstallerProxy ip)
    {
        PrintStream out = IASys.out;
        String usePythonSystemPath = ip.substitute("$USE_PYTHON_SYSTEM_PATH$");

        if (usePythonSystemPath.equals("1"))
        {
            Process p = null;
            java.util.Properties envVars = new java.util.Properties();
            Runtime r = Runtime.getRuntime();
            String line = "";
            String pythonVersion = "";

            try
            {
                String command = "python -V";

                p = r.exec(command);

                // Note that Python outputs the version to standard error, not
                // standard output, so we need to read the error stream
                BufferedReader br = new
                    BufferedReader(new InputStreamReader(p.getErrorStream()));

                while ((line = br.readLine()) != null)
                {
                    if (line.startsWith("Python "))
                    {
                        // Check for "Python 2.7" (i.e. not "Python 2.7.0",
                        // so don't remove the ".0")
                        if (line.length() == 10)
                        {
                            pythonVersion = line.substring(7);
                        }
                        else
                        {
                            String version = line.substring(7);
                            int lastPeriod = version.lastIndexOf('.');
                            pythonVersion = version.substring(0, lastPeriod);
                        }
                    }
                }

                if (pythonVersion.equals("2.2"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "2.2");
                }
                else if (pythonVersion.equals("2.3"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "2.3");
                }
                else if (pythonVersion.equals("2.4"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "2.4");
                }
                else if (pythonVersion.equals("2.5"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "2.5");
                }
                else if (pythonVersion.equals("2.6"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "2.6");
                }
                else if (pythonVersion.equals("2.7"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "2.7");
                }
                else if (pythonVersion.equals("3.0"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "3.0");
                }
                else if (pythonVersion.equals("3.1"))
                {
                    ip.setVariable("$USE_PYTHON_VERSION$", "3.1");
                }
            }
            catch (IOException e) {}
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