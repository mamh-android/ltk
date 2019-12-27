/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFRestartNeeded extends CustomCodeConsoleAction
{
    public boolean setup()
    {
        return true;
    }

    public void executeConsoleAction() throws PreviousRequestException
    {
        ConsoleUtils cu = (ConsoleUtils)cccp.getService(ConsoleUtils.class);

        String restartNeeded = cccp.substitute("$RESTART_NEEDED$");

        if (restartNeeded.startsWith("YES"))
        {
            String allowToReboot = cu.promptAndGetValueWithDefaultValue(
                "The system must be rebooted to complete the install. " +
                "Would you like to reboot the system now?", "YES");

            if (allowToReboot.equalsIgnoreCase("YES"))
            {
                cccp.setVariable("$USER_REQUESTED_RESTART$", "YES");
            }
            else
            {
                cccp.setVariable("$USER_REQUESTED_RESTART$", "NO");
            }
        }
    }

    public String getTitle()
    {
        return "";
    }
}