/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.TreeSet;
import java.util.Comparator;

public class STAXTimedEventQueue extends Thread
{
    public STAXTimedEventQueue()
    {
        start();
    }

    public synchronized void end()
    {
        fComplete = true;
        notify();
    }

    public synchronized void addTimedEvent(STAXTimedEvent timedEvent)
    {
        fTimedEvents.add(timedEvent);
        notify();
    }

    public synchronized void removeTimedEvent(STAXTimedEvent timedEvent)
    {
        fTimedEvents.remove(timedEvent);
        notify();
    }

    public void run()
    {
        synchronized (this)
        {
            while (true)
            {
                if (fComplete) return;

                if (fTimedEvents.size() == 0)
                {
                    try
                    { wait(); continue; }
                    catch (InterruptedException e)
                    { /* Do Nothing */ }
                }

                STAXTimedEvent timedEvent = fTimedEvents.first();

                long timeout = timedEvent.getNotificationTime() -
                    System.currentTimeMillis();

                if (timeout <= 0)
                {
                    timedEvent.getTimedEventListener().timedEventOccurred(
                        timedEvent);
                    fTimedEvents.remove(timedEvent);
                }
                else
                {
                    try
                    {
                        wait(timeout);
                    }
                    catch (InterruptedException e)
                    { /* Do Nothing */ }
                }
            }
        }
    }

    // This class is used to sort the conditions in the condition set
    
    class TimedEventComparator implements Comparator<STAXTimedEvent>
    {
        public int compare(STAXTimedEvent t1, STAXTimedEvent t2)
        {
            if (t1.getNotificationTime() == t2.getNotificationTime())
            {
                if (t1.hashCode() == t2.hashCode()) return 0;
                else if (t1.hashCode() < t2.hashCode()) return -1;
            }
            else if (t1.getNotificationTime() < t2.getNotificationTime())
            {
                return -1;
            }

            return 1;
        }
    }

    boolean fComplete = false;
    TreeSet<STAXTimedEvent> fTimedEvents =
        new TreeSet<STAXTimedEvent>(new TimedEventComparator());
}
