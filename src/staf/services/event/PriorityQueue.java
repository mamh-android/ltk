/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.event;

import java.util.*;
import java.io.*;

public class PriorityQueue
{
    // inner classes

    public class PriorityQueueEntry
    {
        public PriorityQueueEntry()
        {
            this(-1, new Object());
        }

        public PriorityQueueEntry(long aPriority)
        {
            this(aPriority, new Object());
        }

        public PriorityQueueEntry(long aPriority, Object anObject)
        { 
            priority = aPriority;
            object   = anObject;
        }
		
        public String toString()
        {
            StringBuffer result = new StringBuffer();

            result.append("Priority: " + priority + " Object: " + object);

            return result.toString();
        }

        public long priority;
        public Object object;
    }

    // constructors

    public PriorityQueue()
    {
        this(10000);
    }

    public PriorityQueue(int maxsize)
    {
        fHeap  = new PriorityQueueEntry[maxsize];
        fCount = 0;
    }

    // public methods

    synchronized public void enqueue(long prior, Object obj)
    {
        int i;

        // find slot for new entry in O(lg fCount) time

        for (i = fCount; i > 0 && fHeap[parent(i)].priority > prior;
             i = parent(i))
        {
            fHeap[i] = fHeap[parent(i)];
        }        

        fHeap[i] = new PriorityQueueEntry(prior, obj);
        fCount++;        
    }

    synchronized public Object dequeue()
    {
        // if no entries error
        if (fCount == 0) return null;

        // get first entry
        Object obj = fHeap[0].object;

        fCount--;        

        // move last entry to root
        fHeap[0] = fHeap[fCount];

        // fix queue
        sort(0);

        return obj;
    }

    synchronized public Object front()
    {
        // if no entries error
        if (fCount == 0) return null;

        // get first entry
        return fHeap[0].object;
    }

    synchronized public long topPriority()
    {
        // if no entries error
        if (fCount == 0) return -1;

        // get priority of first entry
        return fHeap[0].priority;
    }

    synchronized public Object get(long priority)
    {
        // if no entries error
        if (fCount == 0) return null;
        
        int i = 0;

        while (i < fCount)
        {
            if (fHeap[i++].priority == priority)
                return fHeap[i-1].object;
        }

        return null;
    }

    synchronized public PriorityQueueEntry[] getQueueCopy()
    {
        PriorityQueueEntry[] aCopy = new PriorityQueueEntry[fCount];

        System.arraycopy(fHeap, 0, aCopy, 0, fCount);

        return aCopy;
    }

    public int count()
    {
        return fCount;
    }

    public String toString()
    {
        StringBuffer result = new StringBuffer();

        result.append("PRIORITY QUEUE\n-----------------\n");

        for (int i = 0; i < fCount; i++)
            result.append(fHeap[i].toString() + "\n");
		
        return result.toString();
    }

    // private methods

    synchronized private void sort(int n)
    {
        // Note: this is a recursive method, so it would
        //       not be a good idea for large queues but
        //       it actually takes O(lg fCount) calls so
        //       it is really not a big deal.

        int l = left(n);
        int r = right(n);
        int s = n;

        // pick smallest key among n, l, r
        if (l < fCount && fHeap[l].priority < fHeap[s].priority) s = l;

        if (r < fCount && fHeap[r].priority < fHeap[s].priority) s = r;

        // if child is smaller swap and continue sorting
        if (s != n)
        {
            PriorityQueueEntry temp = fHeap[n];

            fHeap[n] = fHeap[s];
            fHeap[s] = temp;

            // fix heap again
            sort(s);
        }
    }
 
    synchronized private int parent(int n)
    {
        // return priority of parent of nth entry
        return (n - 1) >> 1;
    }

    synchronized private int left(int n)
    {
        // return priority of left child of nth entry
        return (n << 1) + 1;
    }

    synchronized private int right(int n)
    {
        // return priority of right child of nth entry
        return (n << 1) + 2;
    }

    // private data members

    private PriorityQueueEntry[] fHeap;
    private int fCount;
}
