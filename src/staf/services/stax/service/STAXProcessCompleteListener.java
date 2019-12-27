/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.List;

/**
 * An interface which listens for notification that a STAF process it submitted
 * has completed so that it can get information like the process RC and result
 * (which includes files returned by the process).
 *
 * The class that is interested in knowing when a STAF process it submits has
 * completed implements this interface, and the object created with that class
 * adds itself to the processMap. When a submitted STAF process completes, that
 * object's processComplete method is invoked.
 * <p>
 * For example, STAXSTAFProcessAction implements this interface so that it
 * is notified when the STAF process it submitted has completed.
 *
 * @see STAXProcessAction
 * @see STAXProcessActionFactory
 */
public interface STAXProcessCompleteListener
{
    /**
     * Invoked when a queued message is received indicating that a submitted
     * STAF start process request has completed.  The machine and handle of
     * the STAF start process request that completed is passed in as well as
     * its RC and STAFResult and completion timestamp.
     * <p>
     * For example, STAXProcessAction implements this interface so that it
     * is notified when the STAF start process request it submitted has
     * completed. STAXProcessAction's processComplete method frees the process
     * handle, removes the process from the processRequestMap, sets Python
     * variables RC and STAXResult with the result from the submitted STAF
     * start process request and removes the hold thread condition and
     * schedules the thread to run.
     * <p>
     * Note that you may need to synchronize this method for some actions.
     * For example, STAXProcessAction synchronizes this method because its
     * state can be changed on another thread (e.g. via its execute and
     * handleCondition methods).
     *
     * @param  machine   the machine to which the STAF process was submitted
     * @param  handle    the handle of the submitted STAF process
     * @param  rc        the return code from the submitted STAF process request
     * @param  result    the result from the submitted STAF process request
     *                   which includes any returned file data
     * @param  timestamp the process completion timestamp
     */
    public void processComplete(String machine, int handle, long rc,
                                List result, String timestamp);
}
