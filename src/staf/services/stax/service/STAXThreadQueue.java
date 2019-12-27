/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.LinkedList;

// XXX: This thread should probably have a list of all STAXThreads being run as
// well as those on the queue.  This would allow us to make sure that we don't
// try to run the same STAXThread on two real threads at the same time, which
// I think actually happens now.

public class STAXThreadQueue
{
    public STAXThreadQueue(int numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            QueueThread thread = new QueueThread();
            fThreads.add(thread);
            thread.start();
        }
    }

    public void add(STAXThread thread)
    {
        synchronized (fQueue)
        {
            fQueue.addLast(thread);
            fQueue.notify();
        }
    }

    class QueueThread extends Thread
    {
        public void run()
        {
            STAXThread thread = null;

            for (;;)
            {
                try
                {
                    synchronized (fQueue)
                    {
                        while (fQueue.size() == 0) fQueue.wait();

                        thread = fQueue.removeFirst();
                    }

                    thread.execute();
                }
                catch (InterruptedException e)
                {
                    // XXX: What to do?  What does this really mean?
                }
            }
        }
    }

    LinkedList<STAXThread> fQueue = new LinkedList<STAXThread>();
    LinkedList<QueueThread> fThreads = new LinkedList<QueueThread>();
}
