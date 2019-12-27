/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

/**
 * This class is the primary class used to communicate with STAF.
 * It is used to register with STAF, submit service requests to STAF, and
 * unregister with STAF.
 * <p>
 * Each Java application using STAF should generally create one and only one
 * <code>STAFHandle</code> object.  The act of creating this object registers
 * the Java application with STAF.
 * <p>
 * There are two constuctors for the <code>STAFHandle</code> class:
 * <ul compact>
 * <li>The first (and standard) constructor allows you to specify a string
 * containing the name by which your handle should be known.
 * <li>The second constructor allows you to specify an integer which is the
 * number of an existing STAF static handle.  It returns an instance of a
 * <code>STAFHandle</code> object referencing this existing static handle.
 * </ul>
 * <p>
 * Once you have a valid <code>STAFHandle</code> instance object, you can
 * begin submitting requests to STAF services by one of two methods:
 * <ul compact>
 * <li>The <code>submit</code> method works in the traditional Java fashion
 * in that it throws an exception, a <code>STAFException</code> in particular,
 * if it encounters an error and it returns a result string on success.
 * <li>The <code>submit2</code> method returns a <code>STAFResult</code>
 * object in all cases.  This object contains the real STAF return code as
 * well as the result string.  In addition, if auto-unmarshalling is enabled
 * for the handle that called the <code>submit2</code> method, the
 * <code>STAFResult</code> object also contains the marshalling context for
 * the result (the unmarshalled result) in the <code>resultContext</code> field
 * and the result object (the root object of the marshalling context) in the
 * <code>resultObj</code> field.  Otherwise, if auto-unmarshalling is disabled,
 * the <code>resultContext</code> and <code>resultObj</code> fields in the
 * <code>STAFResult</code> object will be set to <code>null</code>.
 * </ul>
 * <p>
 * Note that a <code>STAFHandle</code> instance object has an auto-unmarshall
 * result member variable that defaults to true when a <code>STAFHandle</code>
 * instance object is created, but can be set to false via its
 * <code>setDoUnmarshallResult</code> method.  When set to true, this causes
 * the STAF result to be automatically unmarshalled when using the
 * <code>submit2</code> method.
 * <p>
 * Before the Java application exits, it should unregister with STAF by calling
 * the <code>unRegister</code> method. 
 * <p>
 * This class is fully re-entrant.  In general, only one <code>STAFHandle</code>
 * object should be created for use by the entire Java application.
 */ 
public class STAFHandle
{
    /**
     * This indicates the request should be submitted synchronously so that
     * the request will not return until the request has completed..
     * This is equivalent to calling the <code>submit</code> method. 
     */ 
    public static final int ReqSync = 0;

    /**
     * This indicates the request should be submitted asynchronously with
     * no means to determine if the request was successful or not.
     * The request number will be passed back in the result buffer.
     * The request's results will not be sent to the submitter's queue nor
     * will they be retained by the Service service. 
     */
    public static final int ReqFireAndForget = 1; 

    /**
     * This indicates the request should be submitted asynchronously with
     * the request number passed back in the result buffer and when the
     * request completes, the results will be placed on the submitter's
     * queue.
     */ 
    public static final int ReqQueue = 2;

    /**
     * This indicates the request should be submitted asynchronously with
     * the request number passed back in the result buffer and when the
     * request completes, the submitter can determine the results of the
     * request by using the FREE command of the Service service.
     * 
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRSRVREQF">
     * Section "8.16.6 FREE" in the STAF User's Guide</a>
     */ 
    public static final int ReqRetain = 3;

    /**
     * This indicates the request should be submitted asynchronously with
     * the request number passed back in the result buffer and when the
     * request completes, the results will be placed on the submitter's queue
     * The submitter should also free the results of the request by using the
     * FREE command of the Service service.
     * 
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRSRVREQF">
     * Section "8.16.6 FREE" in the STAF User's Guide</a>
     */ 
    public static final int ReqQueueRetain = 4;

    // Constructors
    /**
     * This is the standard constructor used to create a STAF handle.
     * It allows you to assign a name by which this handle should be known.
     * 
     * @param  handleName     the name by which this handle should should be
     *                        known
     * @throws STAFException  thrown if an error occurs creating a STAF handle
     */
    public STAFHandle(String handleName) throws STAFException
    {
        STAFRegister(handleName);
    }

    /**
     * This constructs a STAF handle that references an existing static handle.
     * 
     * @param  staticHandleNumber the number of the existing STAF static handle
     *                            that you want to reference
     */
    public STAFHandle(int staticHandleNumber)
    {
        handle = staticHandleNumber;
    }

    /**
     * Submits a request to a STAF service on a specified system and waits for
     * it to complete.  It throws an <code>STAFException</code> if it
     * encounters an error and it returns a result string on success.
     * 
     * @param  where          the system to submit the STAF service request to
     * @param  service        the name of the STAF service to submit the
     *                        request to
     * @param  request        the request to submit to the STAF service
     * @return                a string containing the result from the STAF
     *                        service request if successful
     * @throws STAFException  if the STAF service request does not complete
     *                        successfully (that is, with a non-zero return
     *                        code)
     */ 
    public String submit(String where, String service, String request)
                  throws STAFException
    {
        return STAFSubmit(ReqSync, where, service, request);
    }

    /**
     * Submits a request to a STAF service on a specified system in either a
     * synchronous or non-synchronous manner based on the specified
     * <code>syncOption</code> argument. When and what it returns in the
     * result string depends on the <code>syncOption</code> argument.
     * It throws an <code>STAFException</code> if it encounters an error and
     * it returns a result string on success.
     * 
     * @param  syncOption     specifies whether to submit the STAF service
     *                        request synchronously or in an asyncronous
     *                        manner.  Specify one of the following:
     *                        {@link #ReqSync ReqSync},
     *                        {@link #ReqFireAndForget ReqFireAndForget},
     *                        {@link #ReqQueue ReqQueue},
     *                        {@link #ReqRetain ReqRetain},
     *                        {@link #ReqQueueRetain ReqQueueRetain}
     * @param  where          the system to submit the STAF service request to
     * @param  service        the name of the STAF service to submit the
     *                        request to
     * @param  request        the request to submit to the STAF service
     * @return                a string containing the result from the STAF
     *                        service request if successful
     * @throws STAFException  if the STAF service request does not complete
     *                        successfully (that is, with a non-zero return
     *                        code)
     */ 
    public String submit(int syncOption, String where, 
                         String service, String request)
                  throws STAFException
    {
        return STAFSubmit(syncOption, where, service, request);
    }

    /**
     * Submits a request to a STAF service on a specified system and waits
     * for the request to complete and returns a <code>STAFResult</code>
     * object containing its return code and result string.
     * 
     * @param  where          the system to submit the STAF service request to
     * @param  service        the name of the STAF service to submit the
     *                        request to
     * @param  request        the request to submit to the STAF service
     * @return                a STAFResult object containing the return code
     *                        and result string from the STAF service request
     * @see    com.ibm.staf.STAFResult
     */
    public STAFResult submit2(String where, String service, String request)
    {
        STAFResult result = STAFSubmit2(ReqSync, where, service, request);
        
        return new STAFResult(result.rc, result.result, doUnmarshallResult); 
    }

    /**
     * Submits a request to a STAF service on a specified system in either a
     * synchronous or non-synchronous manner based on the specified
     * <code>syncOption</code> argument. When it returns and what it returns
     * in the <code>STAFResult</code> object depends on the
     * <code>syncOption</code> argument.
     * 
     * @param  syncOption     specifies whether to submit the STAF service
     *                        request synchronously or in an asyncronous
     *                        manner.  Specify one of the following:
     *                        {@link #ReqSync ReqSync},
     *                        {@link #ReqFireAndForget ReqFireAndForget},
     *                        {@link #ReqQueue ReqQueue},
     *                        {@link #ReqRetain ReqRetain},
     *                        {@link #ReqQueueRetain ReqQueueRetain}
     * @param  where          the system to submit the STAF service request to
     * @param  service        the name of the STAF service to submit the
     *                        request to
     * @param  request        the request to submit to the STAF service
     * @return                a STAFResult object containing the return code
     *                        and result string from the STAF service request
     * @see    com.ibm.staf.STAFResult
     */ 
    public STAFResult submit2(int syncOption, String where, 
                              String service, String request)
    {
        STAFResult result = STAFSubmit2(syncOption, where, service, request);

        return new STAFResult(result.rc, result.result, doUnmarshallResult); 
    }

    /**
     * Unregisters this handle with STAF.
     * 
     * @throws STAFException  if the handle is not unregistered successfully
     */ 
    public void unRegister() throws STAFException
    {
        STAFUnRegister();
    }

    /**
     * Sets a flag to indicates whether the result should be auto-unmarshalled.
     * 
     * @param flag  <code>true</code> if the result should be auto-unmarshalled
     *              or <code>false</code> if the result should not be
     *              auto-unmarshalled
     */ 
    public void setDoUnmarshallResult(boolean flag)
    {
        doUnmarshallResult = flag;
    }

    /**
     * Retrieves the auto-unmarshall result flag.
     * 
     * @return  the auto-unmarshall result flag
     */ 
    public boolean getDoUnmarshallResult()
    {
        return doUnmarshallResult;
    }

    /**
     * Retrieves the internal STAF handle number.
     *
     * @return  the internal STAF handle number
     */ 
    public int getHandle() { return handle; }

    // Instance variable to keep the STAF Handle for this class
    private int handle;

    // Flag to indicate whether the result should be auto-unmarshalled
    boolean doUnmarshallResult = true;

    /************************/
    /* All the native stuff */
    /************************/

    private static native void initialize();
    private native void STAFRegister(String handleName);
    private native void STAFUnRegister();
    private native String STAFSubmit(int syncOption, String where, 
                                     String service, String request);
    private native STAFResult STAFSubmit2(int syncOption, String where, 
                                          String service, String request);

    // Static initializer - called first time class is loaded.
    static
    {
        if ((System.getProperty("os.name").toLowerCase().indexOf("aix") == 0) ||
           (System.getProperty("os.name").toLowerCase().indexOf("linux") == 0) ||
           ((System.getProperty("os.name").toLowerCase().indexOf("sunos") == 0)
             && (System.getProperty("os.arch").equals("sparc"))))
        {
            System.loadLibrary("STAF");
        }

        System.loadLibrary("JSTAF");
        initialize();
    }
}
