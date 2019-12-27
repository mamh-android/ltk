/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.*;

public class JStatic
{
    // This is the main command line entry point

    public static void main(String [] argv)
    {
        // Verify the command line arguments

        if (argv.length != 0)
        {
            System.out.println();
            System.out.println("Usage: java JStatic");
            System.exit(1);
        }

        // Register with STAF

        try
        {
            handle = new STAFHandle("Queue_Class_Test");
        }
        catch (STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            System.exit(1);
        }

        STAFResult staticResult = handle.submit2("local", "handle",
                                                 "create handle name JStatic");

        if (staticResult.rc != 0)
        {
            System.out.println("Error creating static handle, RC: " +
                               staticResult.rc);
            if (staticResult.result.length() != 0)
                System.out.println("Additional info: " + staticResult.result);

            System.exit(1);
        }

        System.out.println("Successfully created static handle: " +
                           staticResult.result);

        try
        {
            staticHandle = new STAFHandle(Integer.parseInt(staticResult.result));
        }
        catch (NumberFormatException e)
        {
            System.out.println("Invalid static handle received from STAF: " +
                               staticResult.result);
            System.exit(1);
        }

        System.out.println("Successfully created Java STAFHandle for static " +
                           "handle");
        System.out.println("Attempting PING request via static handle");

        STAFResult pingResult = staticHandle.submit2("local", "ping", "ping");

        System.out.println("Expected RC    : 0");
        System.out.println("Actual RC      : " + pingResult.rc);
        System.out.println("Expected Result: PONG");
        System.out.println("Actual Result  : " + pingResult.result);

        staticResult = handle.submit2("local", "handle", "delete handle " +
                                      staticResult.result);

        if (staticResult.rc != 0)
        {
            System.out.println("Error deleting static handle, RC: " +
                               staticResult.rc);
            if (staticResult.result.length() != 0)
                System.out.println("Additional info: " + staticResult.result);

            System.exit(1);
        }

        System.out.println("Successfully deleted static handle: " +
                           staticHandle.getHandle());
    }

    private static STAFHandle handle;
    private static STAFHandle staticHandle;
}
