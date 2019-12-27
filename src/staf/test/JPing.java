/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

//===========================================================================
// JPing - A multi-threaded STAF PING test
//===========================================================================
// Accepts: Where to PING
//          Optionally, the number of threads to use (default = 5)
//          Optionally, the number of loops per thread (default = 10000)
//          Optionally, the display modulus (default = 250)
//
// Returns: 0 , on success
//          >0, if an error is encountered
//===========================================================================
// Date        Who           Comment
// ----------  ------------  ------------------------------------------
// 04/25/1998  C. Rankin     File Created
//===========================================================================

import com.ibm.staf.*;
import java.util.Date;
import java.util.Calendar;
import java.text.DateFormat;

public class JPing implements Runnable
{
    // Constructor

    public JPing(int numLoops, int displayModulus, int myThreadNum)
    {
        loopCount = numLoops;
        modulus = displayModulus;
        threadNum = myThreadNum;
        errors = 0;
    }

    // This is the main command line entry point

    public static void main(String [] argv)
    {
        // Verify the command line arguments

        if ((argv.length < 1) || (argv.length > 4))
        {
            System.out.println();
            System.out.println("Usage: java JPing <Where> [# Threads] " +
                               "[# Loops per thread] [Display Modulus]");
            System.out.println();
            System.out.println("Defaults:");
            System.out.println();
            System.out.println("  # Threads          = 5");
            System.out.println("  # Loops per thread = 10000");
            System.out.println("  Display Modulus    = 250");
            System.out.println();
            System.out.println("Examples:");
            System.out.println();
            System.out.println("java JPing local");
            System.out.println("java JPing SomeServer 3 1000 100");
            System.exit(1);
        }

        // Register with STAF

        try
        {
            handle = new STAFHandle("Java_Ping_Test");
        }
        catch (STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            System.exit(1);
        }

        // Initialize variables

        timeFormatter = DateFormat.getTimeInstance(DateFormat.MEDIUM);

        where = argv[0];
        int numThreads = 5;
        int numLoops = 10000;
        int displayModulus = 250;

        if (argv.length > 1) numThreads = Integer.parseInt(argv[1]);
        if (argv.length > 2) numLoops = Integer.parseInt(argv[2]);
        if (argv.length > 3) displayModulus = Integer.parseInt(argv[3]);

        JPing [] pingers = new JPing[numThreads];
        Thread [] threads = new Thread[numThreads];

        System.out.println("(0)" + timeFormatter.format(new Date()) +
                           " - Started");
        long startSecs = (new Date()).getTime();

        // Start the threads

        for(int i = 0; i < numThreads; ++i)
        {
            pingers[i] = new JPing(numLoops, displayModulus, i + 1);
            threads[i] = new Thread(pingers[i]);
            threads[i].start();
        }

        // Wait for all the threads to finish

        for(int i = 0; i < numThreads; ++i)
        {
            try
            {
                threads[i].join();
            }
            catch (Exception e)
            {
                System.out.println("Exception: " + e);
                System.out.println(e.getMessage());
            }
        }

        // Output final pings/sec

        long stopSecs = (new Date()).getTime();
        System.out.println("(0)" + timeFormatter.format(new Date()) +
                           " - Ended");

        System.out.println("Average: " + ((numLoops * numThreads * 1000) /
                           (stopSecs - startSecs)) + " pings/sec");

        // Unregister with STAF

        try
        {
            handle.unRegister();
        }
        catch (STAFException e)
        {
            System.out.println("Error unregistering with STAF, RC: " + e.rc);
            System.exit(1);
        }
    }

    // This is the method called when each thread starts

    public void run()
    {
        for(int i = 1; i <= loopCount; ++i)
        {
            STAFResult result = handle.submit2(where, "PING", "PING");

            // If we get a non-zero return code, or a response of something
            // other than "PONG", display an error

            if (result.rc != 0)
            {
                System.out.println("(" + threadNum + ")" +
                                   timeFormatter.format(new Date()) +
                                   " - Loop #" + i + ", Error #" +
                                   ++errors + ", RC: " + result.rc);
            }
            else if (result.result.compareTo("PONG") != 0)
            {
                System.out.println("(" + threadNum + ")" +
                                   timeFormatter.format(new Date()) +
                                   " - Loop #" + i + ", Error #" +
                                   ++errors + ", RESULT = " + result.result);
            }

            // If we are at our display modulus display a status message

            if ((i % modulus) == 0)
            {
                System.out.println("(" + threadNum + ")" +
                                   timeFormatter.format(new Date()) +
                                   " - Ended Loop #" + i + ", Errors = " +
                                   errors);
            }
        }
    }

    private static STAFHandle handle;
    private static String where;
    private static DateFormat timeFormatter;

    private int loopCount;
    private int modulus;
    private int threadNum;
    private int errors;
}
