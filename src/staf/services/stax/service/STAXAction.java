/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

/**
 * An interface that defines the action to take when processing a STAX XML
 * element.  All STAX action classes must implement this interface.
 *  
 * @see STAXActionFactory
 */
public interface STAXAction
{
    /**
     * Gets information about an action.
     * <p>
     * For example, STAXSTAFCommandAction's getInfo() method contains:
     * <pre>
     * return fName;
     * </pre>
     *
     * @return information about this action
     */
    public String getInfo();

    /**
     * Gets detailed information about an action. 
     * <p>
     * For example, STAXSTAFCommandAction's getDetails() method contains:
     * <pre>
     * return "Name:" + fName + 
     *        ";Location:" + fLocation +
     *        ";Service:" + fService + 
     *        ";Request:" + fRequest +
     *        ";State:" + getStateAsString() + 
     *        ";BlockName:" + fCurrentBlockName +
     *        ";StartTimestamp:" + fStartTimestamp +
     *        ";RequestNumber:" + fRequestNumber +
     *        ";RequestRC:" + fRequestRC +
     *        ";RequestResult" + fRequestResult +
     *        ";HoldThreadCondition:" + fHoldCondition;
     * </pre>
     *
     * @return detailed information about this action
     */
    public String getDetails();

    /**
     * Called by STAXThread to execute an action under normal conditions (that
     * is, when there are no pending conditions).
     * <p>
     * Note that you may need to synchronize this method for some actions.
     * For example, STAXSTAFCommandAction synchronizes this method because its
     * state can be changed on another thread (e.g. via the requestComplete
     * method).
     *
     * @param  thread  the STAX-Thread instance where this action is executed
     */
    public void execute(STAXThread thread);

    /**
     * Called by STAXThread if there are any pending conditions, passing the
     * highest priority condition.
     * <p>
     * Note that you may need to synchronize this method for some actions.
     * For example, STAXSTAFCommandAction synchronizes this method because its
     * state can be changed on another thread (e.g. via the requestComplete
     * method).
     *
     * @param  thread  the STAX-Thread instance where this action is executed
     * @param  cond    the condition that has been raised
     */
    public void handleCondition(STAXThread thread, STAXCondition condition);

    /**
     * Returns a clone of an action. Called when need to push an action
     * onto a thread's action stack.  For example, STAXParallelAction needs to
     * generate child threads pushing a clone of its action onto each child
     * thread's action stack.
     *
     * @return  a clone of this action
     */
    public STAXAction cloneAction();
}
