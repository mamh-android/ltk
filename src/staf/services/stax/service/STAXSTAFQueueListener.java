/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

/**
 * An interface which listens for queued messages.
 * The class that is interested in knowing when a message is queued implements
 * this interface, and the object created with that class is registered by
 * passing itself as the listener argument when calling the job's
 * registerSTAFQueueListener method.  When registering, it also passes in the
 * type of messages that it is interested in. When a message is queued,
 * that object's handleQueueMessage method is invoked.  
 *
 * @see STAXJob
 */
public interface STAXSTAFQueueListener
{
    /**
     * Invoked when a queued message matching the specified type is received.
     * The STAF message received on the queue and the job object are passed in.
     * <p>
     * For example, the STAXProcessActionFactory's handleQueueMessage checks
     * for messages with type "STAF/Process/End" so that it knows when to
     * remove a process from the map of active processes that it maintains.
     *
     * @param stafMessage the object represeting the message
     * @param job         the object representing the job
     */
    public void handleQueueMessage(STAXSTAFMessage stafMessage, STAXJob job);
}
