/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.whoami;

import com.ibm.staf.*;
import com.ibm.staf.service.*;

public class WhoAmI implements STAFServiceInterfaceLevel30
{
    public WhoAmI() { /* Do Nothing */ }

    public STAFResult init(STAFServiceInterfaceLevel30.InitInfo info)
    {
        try
        {
            fHandle = new STAFHandle("STAF/Service/" + info.name);
        }
        catch (STAFException e)
        {
            return new STAFResult(
                STAFResult.STAFRegistrationError, e.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }

    public STAFResult acceptRequest(STAFServiceInterfaceLevel30.RequestInfo info)
    {
        STAFResult res = fHandle.submit2(info.endpoint, "MISC", "WHOAMI");

        if (res.rc != 0)
            return res;

        return new STAFResult(STAFResult.Ok, res.result);
    }

    public STAFResult term()
    {
        // Un-register the service handle

        try
        {
            fHandle.unRegister();
        }
        catch (STAFException e)
        {
            return new STAFResult(
                STAFResult.STAFRegistrationError, e.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }

    private STAFHandle fHandle;
}
