/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/* Java STAF Client */

import com.ibm.staf.*;

class JavaSTAF
{
    public static void main(String args[])
    {
        if (args.length != 3)
        {
            System.out.println("Usage: JAVA JavaSTAF <TCPIP HostName | " +
                               "LOCAL>  <Service> <Request>");
            System.exit(1);
        }

        try
        {
            STAFHandle handle = new STAFHandle("STAF_Java_Client");

            try
            {
                String result = handle.submit(args[0], args[1], args[2]);

                System.out.println("Response");
                System.out.println("--------");
                System.out.println(
                    STAFMarshallingContext.unmarshall(result).toString());
            }
            catch (STAFException e)
            {
                System.out.println("Error submitting request to STAF, RC: " +
                                   e.rc);
                System.out.println(e.getMessage());
            }
            finally
            {
                handle.unRegister();
            }
        }
        catch (STAFException e)
        {
            System.out.println("Error (un)registering with staf, RC:" + e.rc);
        }

    }  // End of main
}
