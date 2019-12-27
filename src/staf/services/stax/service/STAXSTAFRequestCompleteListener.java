/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;

/**
 * An interface which listens for a queued message indicating that a
 * submitted STAF command request has completed.
 * The class that is interested in knowing when a STAF command it submits
 * has completed implements this interface, and the object created
 * with that class is registered by passing itself as the listener argument
 * when calling the job's submitAsync method.  When a submitted STAF command
 * completes, that object's requestComplete method is invoked.
 * <p>
 * For example, STAXSTAFCommandAction implements this interface so that it
 * is notified when the STAF command request it submitted has completed.
 *
 * @see STAXSTAFCommandActionFactory
 * @see STAXJob
 */
public interface STAXSTAFRequestCompleteListener
{
    /**
     * Invoked when a queued message is received indicating that a submitted
     * STAF command request has completed.  The request number of the STAF
     * command request that completed is passed in as well as its STAFResult
     * (containing the RC an result from running the STAF command).
     * <p>
     * For example, STAXSTAFCommandAction's requestComplete method sets
     * Python variables RC and STAFResult with the result from the submitted
     * STAF command request and removes the hold thread condition and schedules
     * the thread to run.  It also generates an event to indicate that the
     * submitted STAF command request has completed.
     * <p>
     * Note that you may need to synchronize this method for some actions.
     * For example, STAXSTAFCommandAction synchronizes this method because its
     * state can be changed on another thread (e.g. via its execute and
     * handleCondition methods).
     *
     * @param  requestNumber  the request number of the submitted STAF command
     *                        request
     * @param  result         the result from the submitted STAF command request
     */
    public void requestComplete(int requestNumber, STAFResult result);
}
