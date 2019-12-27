/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/* Java STAF Client */

import com.ibm.staf.*;

class JavaSTAF2
{
    public static void main(String args[])
    {
        if (args.length != 3)
        {
            System.out.println("Usage: JAVA JavaSTAF2 <TCPIP HostName | " +
                               "LOCAL>  <Service> <Request>");
            System.exit(1);
        }

        try
        {
            STAFHandle handle = new STAFHandle("STAF_Java_Client");
            STAFResult result = handle.submit2(args[0], args[1], args[2]);

            if (result.rc == 0)
            {
                System.out.println("Response");
                System.out.println("--------");
                System.out.println(result.result);
            }
            else
            {
                System.out.println("Error submitting request to STAF, RC: " +
                                   result.rc);
                System.out.println(result.result);
            }

            handle.unRegister();
        }
        catch (STAFException e)
        {
            System.out.println("Error (un)registering with staf, RC:" + e.rc);
        }

    }  // End of main
}
