/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;
import java.util.*;

public class STAFConsoleAdvancedOptions extends CustomCodeConsoleAction
{
    public boolean setup()
    {
        return true;
    }

    public void executeConsoleAction() throws PreviousRequestException
    {
        ConsoleUtils cu = (ConsoleUtils)cccp.getService(ConsoleUtils.class);

        String osname = cccp.substitute("$prop.os.name$");
        String osarch = cccp.substitute("$prop.os.arch$");

        Vector updateEnvironmentOptions = new Vector();
        updateEnvironmentOptions.add("System");
        updateEnvironmentOptions.add("User");
        updateEnvironmentOptions.add("None");

        int updateEnvironmentSelection =
            cu.createChoiceListAndGetValue("Update Environment/Menus for",
                                           updateEnvironmentOptions,
                                           0);

        String updateEnvironmentSelectionText = (String)
            updateEnvironmentOptions.elementAt(updateEnvironmentSelection);

        cccp.setVariable("$UPDATE_ENVIRONMENT$",
            updateEnvironmentSelectionText);

        cu.wprintln("\n");

        if ((osname.indexOf("Windows") > -1) &&
            !(updateEnvironmentSelectionText.equals("None")))
        {
            String startSTAF = cu.promptAndGetValueWithDefaultValue(
                "Start STAF on user login?", "1");

            cccp.setVariable("$START_ON_LOGIN$", startSTAF);

            cu.wprintln("\n");

            String createStartMenuIcons = cu.promptAndGetValueWithDefaultValue(
                "Create Start menu icons?", "1");

            cccp.setVariable("$CREATE_START_MENU_ICONS$", createStartMenuIcons);

            cu.wprintln("\n");

            Vector startSTAFProcOptions = new Vector();
            startSTAFProcOptions.add("Normal");
            startSTAFProcOptions.add("Minimized");

            int startSTAFProcSelection =
                cu.createChoiceListAndGetValue("Start STAFProc",
                                               startSTAFProcOptions,
                                               0);

            String startSTAFProcSelectionText = (String)
                startSTAFProcOptions.elementAt(startSTAFProcSelection);

            cccp.setVariable("$START_STAFPROC$", startSTAFProcSelectionText);

            cu.wprintln("\n");
        }

        Vector tcpOptions = new Vector();
        tcpOptions.add("IPV4");
        tcpOptions.add("IPV4_IPV6");

        int tcpSelection =
            cu.createChoiceListAndGetValue("Default TCP version",
                                           tcpOptions,
                                           0);

        cccp.setVariable("$USE_TCP_VERSION$",
            tcpOptions.elementAt(tcpSelection));

        cu.wprintln("\n");

        if ((osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Windows") > -1) && (osarch.equals("amd64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
            ((osname.indexOf("SunOS") > -1) && (osarch.equals("sparc"))) ||
            (osname.indexOf("AIX") > -1) ||
            (osname.indexOf("Mac OS X") > -1))
        {
            Vector perlOptions = new Vector();
            perlOptions.add("5.8");

            if (!((osname.indexOf("Linux") > -1) && (osarch.equals("ia64"))))
            {
                perlOptions.add("5.10");
                
                // Currently only support Perl 5.12/5.14 on Windows/Linux x86
                // and amd64
                if ((osname.indexOf("Windows") > -1) ||
                    (osname.indexOf("Linux") > -1))
                {
                    perlOptions.add("5.12");
                    perlOptions.add("5.14");
                }
            }

            if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")))
            {
                perlOptions.add("5.6");
            }

            int perlSelection =
                cu.createChoiceListAndGetValue("Default Perl version",
                                               perlOptions,
                                               0);

            cccp.setVariable("$USE_PERL_VERSION$",
                perlOptions.elementAt(perlSelection));

            cu.wprintln("\n");

            String usePerlSystemPath = cu.promptAndGetValueWithDefaultValue(
                "Use Perl version in System Path?", "0");

            cccp.setVariable("$USE_PERL_SYSTEM_PATH$", usePerlSystemPath);

            cu.wprintln("\n");
        }

        if ((osname.indexOf("Windows") > -1) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
            (osname.indexOf("FreeBSD") > -1) ||
            (osname.indexOf("SunOS") > -1) ||
            (osname.indexOf("Mac OS X") > -1))
        {
            Vector pythonOptions = new Vector();

            if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
                (osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("FreeBSD") > -1) ||
                (osname.indexOf("SunOS") > -1))
            {
                pythonOptions.add("2.2");
                pythonOptions.add("2.3");
            }

            if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
                (osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Windows") > -1) && (osarch.equals("ia64")) ||
                (osname.indexOf("FreeBSD") > -1) ||
                (osname.indexOf("SunOS") > -1))
            {
                pythonOptions.add("2.4");
            }

            if (osname.indexOf("Mac OS X") == -1)
            {
                pythonOptions.add("2.5");
            }

            if ((osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ppc64")) ||
                (osname.indexOf("Linux") > -1) && (osarch.equals("ia64")) ||
                (osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
                (osname.indexOf("Windows") > -1) && (osarch.equals("amd64")) ||
                (osname.indexOf("FreeBSD") > -1) ||
                (osname.indexOf("SunOS") > -1))
            {
                pythonOptions.add("2.6");
                pythonOptions.add("2.7");
                pythonOptions.add("3.0");
                pythonOptions.add("3.1");
            }

            if (osname.indexOf("Mac OS X") > -1)
            {
                // XXX When Bug #2115056 (InstallAnywhere variable
                // $EXTRACTOR_EXECUTABLE$ is blank on Mac OS X) is fixed, we can
                // check to see if the filename includes the text "universal",
                // and only display the Python versions supported for the installer
                pythonOptions.add("2.3 (supported on macosx-i386, macosx-ppc)");
                pythonOptions.add("2.6 (supported on macosx-i386, macosx-ppc)");
                pythonOptions.add("2.7 (supported on macosx-universal)");
                pythonOptions.add("3.1 (supported on macosx-universal)");
            }

            int pythonSelection = 0;

            if (osname.indexOf("FreeBSD") > -1)
            {
                pythonSelection =
                    cu.createChoiceListAndGetValue("Default Python version",
                                                   pythonOptions,
                                                   2);
            }
            else
            {
                pythonSelection =
                    cu.createChoiceListAndGetValue("Default Python version",
                                                   pythonOptions,
                                                   0);
            }

            String pythonSelectionText = (String)
                pythonOptions.elementAt(pythonSelection);

            if (pythonSelectionText.indexOf("(supported on") > -1)
            {
                pythonSelectionText = pythonSelectionText.substring(0, 3);
            }

            cccp.setVariable("$USE_PYTHON_VERSION$", pythonSelectionText);

            cu.wprintln("\n");

            String usePythonSystemPath = cu.promptAndGetValueWithDefaultValue(
                "Use Python version in System Path?", "0");

            cccp.setVariable("$USE_PYTHON_SYSTEM_PATH$", usePythonSystemPath);

            cu.wprintln("\n");
        }

        if ((osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Windows") > -1) && (osarch.equals("amd64")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("x86")) ||
            (osname.indexOf("Linux") > -1) && (osarch.equals("amd64")))
        {
            Vector tclOptions = new Vector();

           if ((osname.indexOf("Windows") > -1) && (osarch.equals("x86")))
            {
                tclOptions.add("8.3");
            }

            if ((osname.indexOf("Windows") > -1) && (osarch.equals("x86")) ||
               (osname.indexOf("Linux") > -1) && (osarch.equals("x86")))
            {
                tclOptions.add("8.4");
            }

            tclOptions.add("8.5");
            tclOptions.add("8.6");

            int tclSelection =
                cu.createChoiceListAndGetValue("Default TCL version",
                                               tclOptions,
                                               0);

            cccp.setVariable("$USE_TCL_VERSION$",
                tclOptions.elementAt(tclSelection));

            cu.wprintln("\n");

            String useTclSystemPath = cu.promptAndGetValueWithDefaultValue(
                "Use TCL version in System Path?", "0");

            cccp.setVariable("$USE_TCL_SYSTEM_PATH$", useTclSystemPath);

            cu.wprintln("\n");
        }

        // Prompt for the "Default STAF Instance Name".  If an invalid
        // value is specified, display an error message and prompt again
        // to input the "Default STAF Instance Name".  Do not allow the
        // user to continue the installation until a valid STAF Instance
        // Name is specified.

        boolean isValid = false;
        String stafInstanceName = "STAF";

        while (!isValid)
        {
            isValid = true;

            stafInstanceName = cu.promptAndGetValueWithDefaultValue(
                "Default STAF Instance Name", "STAF");

            if (!stafInstanceName.equals("STAF"))
            {
                // Verify that the STAF Instance Name specified is valid.
                // A STAF instance name is invalid if it contains any of the
                // following special characters: ~!#$%^&*+={}[]|;':"?/<>\
                // or if it contains any whitespace at the beginning or end
                // or if it only contains whitespace.

                String errorMsg = "";
                String trimmedInstanceName = stafInstanceName.trim();
                
                if (trimmedInstanceName.length() == 0)
                {
                    errorMsg = "It cannot be empty or just spaces.";
                }
                else if (trimmedInstanceName != stafInstanceName)
                {
                    errorMsg = "It cannot contain any leading or trsiling " +
                        "whitespace.";
                }
                else
                {
                    boolean found = false;
                    char[] invalidChars = new char[] {
                        '~', '!', '#', '$', '%', '^', '&', '*', '+', '=', '{',
                        '}', '[', ']', '|', ';', '\'', ':', '"', '?', '/',
                        '<', '>', '\\' };
                
                    for (int i = 0; i < stafInstanceName.length() && !found; i++)
                    {
                        char ch = stafInstanceName.charAt(i);
                    
                        for (int j = 0; j < invalidChars.length && !found; j++)
                        {
                            if (invalidChars[j] == ch)
                            {
                                found = true;
                                errorMsg = "It cannot contain any of the " +
                                    "following characters: " +
                                    "~!#$%^&*+={}[]|;':\"?/<>\\";
                            }
                        }
                    }
                }

                if (errorMsg.length() != 0)
                {
                    isValid = false;

                    // Display an error message

                    cu.wprintln("You entered an invalid STAF Instance Name.  " +
                                errorMsg);
                    cu.wprintln("\n");
                }
            }
        }

        cccp.setVariable("$STAF_INSTANCE_NAME$", stafInstanceName);

        cu.wprintln("\n");
    }

    public String getTitle()
    {
        return "Advanced Options";
    }
}