/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.*;

public class TestQueue
{
    // This is the main command line entry point

    public static void main(String [] argv)
    {
        // Verify the command line arguments

        if (argv.length != 0)
        {
            System.out.println();
            System.out.println("Usage: java TestQueue");
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

        System.out.println("Please send a queue message to handle " +
                           handle.getHandle() + " now");

        STAFResult queueResult = handle.submit2("local", "queue", "get wait");

        if (queueResult.rc != 0)
        {
            System.out.println("Error getting message from queue, RC: " +
                               queueResult.rc + ", Additional Info: " +
                               queueResult.result);
            System.exit(1);
        }

        STAFQueueMessage message = new STAFQueueMessage(queueResult.result);

        System.out.println("Priority   : " + message.priority);
        System.out.println("Timestamp  : " + message.timestamp);
        System.out.println("Machine    : " + message.machine);
        System.out.println("Handle name: " + message.handleName);
        System.out.println("Handle     : " + message.handle);
        System.out.println("Type       : " + message.type);
        System.out.println("Message    : " + message.message);
    }

    private static STAFHandle handle;
}
