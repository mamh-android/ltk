/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFResult;

/**
 * An interface that provides the ability to perform tasks that only need
 * to be done once at the beginning or end of a job.
 *
 * For example, the STAXSTAFCommandActionFactory implements this interface
 * so that when before a job begins execution, it can create a map that
 * is used to keep track of active STAF commands in a STAX job.  This data
 * is needed so that the STAX service can list and query active STAF
 * commands for a job.
 *
 * @see STAXJob
 */
public interface STAXJobManagementHandler
{
    /**
     * Called by STAXJob when starting the execution of a STAX job.
     * It can be used to perform tasks that only need to be done once at the
     * beginning of a job.  For example, it could initialize data needed
     * throughout the lifetime of a STAX job and/or to register a message
     * handler for certain messages received on the queue.
     *
     * @param job the STAX job
     */
    public void initJob(STAXJob job);

    /**
     * Called by STAXJob when terminating a STAX job.
     * It can be used to do things that only need to be done once at the
     * end of a job.  For example, delete data that is no longer needed.
     * 
     * @param job the STAX job
     */
    public void terminateJob(STAXJob job);
}
