/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.wrapper;
import com.ibm.staf.*;

// STAFMonitor - This class is a wrapper around the STAF monitor service.
//               It currently only provides a means to monitor locally.
//               There are instance and static methods to perform the logging.

public class STAFMonitor
{
    public STAFMonitor(STAFHandle stafHandle) { handle = stafHandle; }

    public STAFResult log(String message)
    {
        return handle.submit2("LOCAL", "MONITOR", "LOG MESSAGE :" +
                              message.length() + ":" + message);
    }

    public static STAFResult log(STAFHandle theHandle, String message)
    {
        return theHandle.submit2("LOCAL", "MONITOR", "LOG MESSAGE :" +
                                 message.length() + ":" + message);
    }

    private STAFHandle handle;
}
