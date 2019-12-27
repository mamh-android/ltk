/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

/**
 * An interface which listens for notification that a STAX thread has
 * completed.
 *
 * The class that is interested in knowing when a STAX thread completes
 * implements this interface, and the object created with that class is
 * registered by passing itself as the listener argument when calling the
 * thread's addCompletionNotifee method.  When a thread completes, that
 * object's threadComplete method is invoked.
 * <p>
 * For example, STAXParallelAction implements this interface so that it is
 * notified when the STAX threads it submits for execution have completed so
 * it can remove the thread from its map of active threads and know when all
 * of its threads have copmleted so that it can remove hold conditions on its
 * parent threads and schedule the parent thread to run.
 *
 * @see    STAXThread
 */
public interface STAXThreadCompleteListener
{
    /**
     * Called by STAXThread when completing the execution of a STAX thread.
     * The thread object representing the thread that completed is passed in.
     * 
     * @param thread  an object representing the STAX thread that completed
     * @param endCode a code which indicates how the thread ended (e.g.
     *                STAXThread.THREAD_END_OK = 0,
     *                STAXThread.THREAD_END_DUPLICATE_SIGNAL = 1,
     *                STAXThread.THREAD_END_STOPPED_BY_PARENT = 2)
     */
    public void threadComplete(STAXThread thread, int endCode);
}
