/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.zerog.ia.api.pub.*;
import java.io.*;

public class STAFConsoleRegistration extends CustomCodeConsoleAction
{
    public boolean setup()
    {
        return true;
    }

    public void executeConsoleAction() throws PreviousRequestException
    {
        ConsoleUtils cu = (ConsoleUtils)cccp.getService(ConsoleUtils.class);

        String allowToRegister = cu.promptAndGetValueWithDefaultValue(
            "Allow STAF to Register?", "1");

        cccp.setVariable("$REGISTER$", allowToRegister);

        if (allowToRegister.equals("1"))
        {
            String name = cu.promptAndGetValue(
                "Registration Name");

            cccp.setVariable("$REGISTRATION_NAME$", name);

            String email = cu.promptAndGetValue(
                "Registration Email");

            cccp.setVariable("$REGISTRATION_EMAIL$", email);

            String organization = cu.promptAndGetValue(
                "Registration Organization");

            cccp.setVariable("$REGISTRATION_ORG$", organization);
        }
    }

    public String getTitle()
    {
        return "Registration Information";
    }
}