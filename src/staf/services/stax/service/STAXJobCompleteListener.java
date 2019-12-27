/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

/**
 * An interface which listens for notification that a STAX job is terminating.
 *
 * The class that is interested in knowing when a STAF job it submits has
 * completed implements this interface, and the object created with that class
 * is registered by passing itself as the listener argument when calling the
 * job's addCompletionNotifee method.  When a job completes, that object's
 * jobComplete method is invoked.
 * <p>
 * For example, STAX implements this interface so that it is notified when
 * the STAX jobs it submits for execution have completed so it can remove
 * the job from its map of active jobs.
 *
 * @see    STAXJob
 */
public interface STAXJobCompleteListener
{
    /**
     * Called by STAXJob when terminating the execution of a STAX job.
     * The job object representing the job that completed is passed in.
     * <p>
     *
     * @param job an object representing the STAX job being terminated
     */
    public void jobComplete(STAXJob job);
}
