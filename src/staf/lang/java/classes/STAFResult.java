/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

/**
 * This class is used to return the result from the <code>STAFHandle</code>
 * class's <code>submit2</code> method. It contains both the STAF return code
 * as well as the result string.
 * <p>
 * In addition, if auto-unmarshalling is enabled for the handle (which it is
 * by default), it also contains the marshalling context for the result (that
 * is, the unmarshalled result) and the result object (that is, the root
 * object of the marshalling context). Otherwise, if auto-unmarshalling is
 * disabled for the handle that called the <code>submit2</code> method, the
 * <code>resultContext</code> and <code>resultObj</code> fields will be set to
 * <code>null</code>.
 * <p>
 * It is typically used in places where you wish to avoid catching exceptions
 * when using STAF or where you want the result to be auto-unmarshalled.
 * <p>
 * This class also contains the constant definitions for all the STAF return
 * codes. These return codes are common to STAFResult and STAFException.
 */
public class STAFResult
{
    /**
     * This constructs a STAF result with return code 0 and an empty result
     * buffer. 
     */ 
    public STAFResult() { rc = STAFResult.Ok; result = new String(); }

    /**
     * This constructs a STAF result with the specified return code and an
     * empty result buffer.
     * 
     * @param  theRC      the return code
     */ 
    public STAFResult(int theRC) { rc = theRC; result = new String(); }

    /**
     * This constructs a STAF result with the specified return code and result
     * buffer.
     * 
     * @param  theRC      the return code
     * @param  theResult  the result buffer
     */ 
    public STAFResult(int theRC, String theResult)
    {
        rc = theRC;
        result = theResult;
    }
    
    /**
     * This constructs a STAF result with the specified return code and result
     * buffer and allows you to specify a flag indicating whether to unmarshall
     * the result automatically.
     * 
     * @param  theRC               the return code
     * @param  theResult           the result buffer
     * @param  doUnmarshallResult  a flag indicating whether to unmarshall the
     *                             result automatically
     */
    public STAFResult(int theRC, String theResult, boolean doUnmarshallResult)
    {
        rc = theRC;
        result = theResult;
        
        if (doUnmarshallResult)
        {
            resultContext = STAFMarshallingContext.unmarshall(theResult);
            resultObj = resultContext.getRootObject();
        }
    }

    /**
     * The return code
     */ 
    public int rc;

    /**
     * The result buffer
     */ 
    public String result;

    /**
     * The result object (that is, the root object of the marshalling context).
     * It is initialized to <code>null</code> and will only be assigned if
     * auto-unmarshalling is enabled.
     */ 
    public Object resultObj = null;

    /**
     * The marshalling context for the result (that is, the unmarshalled result).
     * It is initialized to <code>null</code> and will only be assigned if
     * auto-unmarshalling is enabled.
     */
    public STAFMarshallingContext resultContext = null;

    /**
     * A return code indicating success.
     */ 
    public static final int Ok = 0;

    /**
     * A return code indicating that a process has tried to call an invalid
     * internal STAF API. If this error occurs, report it to the authors.
     */ 
    public static final int InvalidAPI = 1;

    /**
     * A return code indicating you have tried to submit a request to a
     * service that is unknown to STAFProc. Verify that you have correctly
     * registered the service. 
     */ 
    public static final int UnknownService = 2;

    /**
     * A return code indicating you are passing an invalid handle to a STAF
     * API.  Ensure that you are using the handle you received when you
     * registered with STAF. 
     */ 
    public static final int InvalidHandle = 3;

    /**
     * A return code indicating you are trying to register a process with one
     * name when that process has already been registered with a different
     * name. If you register the same process multiple times, ensure that you
     * use the same name on each registration call.
     * <p>
     * Note: If you receive this error code when trying to perform an
     * operation other than registering a service, report it to the authors.
     */ 
    public static final int HandleAlreadyExists = 4;

    /**
     * A return code indicating you are trying to perform an operation on a
     * handle that does not exist. For example, you may be trying to stop a
     * process, but you are specifying the wrong handle. 
     */ 
    public static final int HandleDoesNotExist = 5;

    /**
     * A return code indicating an unknown error has occurred. This error is
     * usually an indication of an internal STAF error. If this error occurs,
     * report it the authors. 
     */ 
    public static final int UnknownError = 6;

    /**
     * A return code indicating you have submitted an improperly formatted
     * request to a service. See the service's User Guide for for the syntax
     * of the service's requests, or contact the provider of the service. 
     * <p>
     * Note: Additional information regarding the exact syntax error may be
     * provided in the result passed back from the submit call.
     */ 
    public static final int InvalidRequestString = 7;

    /**
     * A return code indicating an internal error with the service to which
     * a request was submitted. If this error occurs, report it to the
     * authors and the service provider.
     */ 
    public static final int InvalidServiceResult = 8;

    /**
     * A return code indicating an internal error in an external Rexx service.
     * If this error occurs, report it to the authors and the service
     * provider. 
     * <p>
     * Note: The actual Rexx error code will be returned in the result passed
     * back from the submit call.
     */ 
    public static final int REXXError = 9;

    /**
     * A return code indicating that a base operating system error was
     * encountered. 
     * <p>
     * Note: The actual base operating system error code, and possibly
     * additional information about the error, will be returned in the result
     * passed back from the submit call. 
     */ 
    public static final int BaseOSError = 10;

    /**
     * A return code indicating you are trying to perform an invalid operation
     * on a process that has already completed. For example, you may be trying
     * to stop the process or register for a process end notification. 
     */ 
    public static final int ProcessAlreadyComplete = 11;

    /**
     * A return code indicating you are trying to free process information for
     * a process that is still executing. 
     */ 
    public static final int ProcessNotComplete = 12;

    /**
     * A return code indicating you are trying to get, remove, or resolve a
     * variable that does not exist. Remember that variables are case
     * sensitive. The name of the variable that does not exist will be in the
     * result passed back from the submit call. 
     */ 
    public static final int VariableDoesNotExist = 13;

    /**
     * A return code indicating you have requested to resolve a string that
     * cannot be resolved. This indicates that you have exceeded the
     * resolution depth of the VAR service. The most common cause of this is
     * recursive variables definitions. 
     */ 
    public static final int UnResolvableString = 14;

    /**
     * A return code indicating the string you requested to be resolved has a
     * non-matching left or right curly brace. Ensure that all variable
     * references have both left and right curly braces. 
     */ 
    public static final int InvalidResolveString = 15;

    /**
     * A return code indicating that STAFProc was not able to submit the
     * request to the requested endpoint (i.e. target machine). This error
     * usually indicates one or more of the following:
     * <ul>
     * <li>STAFProc is not running on the target machine.
     * <li>The requested endpoint is not valid.
     * <li>The network interface or port for the requested endpoint is not
     * supported.
     * <li>A firewall is blocking communication via the port for the requested
     * endpoint. 
     * <li>A secure network interface is being used to communicate to a
     * machine that doesn't have a secure network interface configured with
     * the same certificate.
     * </ul>
     * <p>
     * Alternatively, you may need to increase your CONNECTTIMEOUT value for
     * the network interface and/or increase your CONNECTATTEMPTS value in
     * your STAF.cfg file. 
     */ 
    public static final int NoPathToMachine = 16;

    /**
     * A return code indicating that there was an error opening the requested
     * file. Some possible explanations are that the file/path does not exist,
     * contains invalid characters, or is locked.
     * <p>
     * Note: Additional information regarding which file could not be opened
     * may be provided in the result passed back from the submit call.
     */ 
    public static final int FileOpenError = 17;

    /**
     * A return code indicating that there was an error while trying to read
     * data from a file.
     * <p>
     * Note: Additional information regarding which file could not be read
     * from may be provided in the result passed back from the submit call.
     */ 
    public static final int FileReadError = 18;

    /**
     * A return code indicating that there was an error while trying to write
     * data to a file.
     * <p>
     * Note: Additional information regarding which file could not be written
     * to may be provided in the result passed back from the submit call.
     */ 
    public static final int FileWriteError = 19;

    /**
     * A return code indicating that there was an error while trying to delete
     * a file or directory.
     * <p>
     * Note: Additional information regarding which file or directory could not
     * be deleted may be provided in the result passed back from the submit
     * call.
     */ 
    public static final int FileDeleteError = 20;

    /**
     * A return code indicating that STAFProc is not running on the local
     * machine with the same STAF_INSTANCE_NAME (and/or the same STAF_TEMP_DIR
     * if on a Unix machine).
     * <p>
     * Notes:
     * <ul>
     * <li>If the STAF_INSTANCE_NAME environment variable is not set, it
     * defaults to "STAF".
     * <li>On Unix, if the STAF_TEMP_DIR environment variable is not set, it
     * defaults to "/tmp". This environment variable is not used on Windows.
     * <li>This error can also occur when submitting a request using the local
     * IPC interface on a Unix machine if the socket file that the local
     * interface uses has been inadvertently deleted.
     * <li>To get more information on this error, set special environment
     * variable STAF_DEBUG_21=1 and resubmit your local STAF service request.
     * </ul>
     */ 
    public static final int STAFNotRunning = 21;

    /**
     * A return code indicating an error transmitting data across the network,
     * or to the local STAF process. For example, you would receive this error
     * if STAFProc.exe was terminated in the middle of a service request, or
     * if a bridge went down in the middle of a remote service request. This
     * can also indicate that the requested endpoint is not valid (e.g. it has
     * an invalid network interface and port combination such as a non-secure
     * tcp interface with the port for a secure ssl interface). 
     */ 
    public static final int CommunicationError = 22;

    /**
     * A return code indicating you have requested to delete a trustee, and the
     * trustee does not exist. Verify that you have specified the correct
     * trustee. 
     */ 
    public static final int TrusteeDoesNotExist = 23;

    /**
     * A return code indicating You have attempted to set a machine or default
     * trust level to an invalid level. The valid trust levels are from zero
     * to five.
     */ 
    public static final int InvalidTrustLevel = 24;

    /**
     * A return code indicating you have submitted a request for which you do
     * not have the required trust level to perform the request.
     * <p>
     * Note: Additional information regarding the required trust level may be
     * provided in the result passed back from the submit call.
     */ 
    public static final int AccessDenied = 25;

    /**
     * A return code indicating that an external service encountered a problem
     * when trying to register with STAF. Ensure that STAF has been properly
     * installed and configured. 
     */ 
    public static final int STAFRegistrationError = 26;

    /**
     * A return code indicating an error with the configuration of an external
     * service. One possible explanation is that the LIBRARY you specified
     * when configuring the service does not exist. Or, if you specified the
     * EXECUTE option, verify that the executable exists and has the execute
     * permission. Or, if you specified the PARMS option, verify that all of
     * the service configuration are valid. Consult the appropriate
     * documentation for the service to verify whether you have configured the
     * service properly, or contact the service provider.
     * <p>
     * Note: Additional information regarding why the service configuration
     * failed may be provided in the result passed back from the submit call.
     */ 
    public static final int ServiceConfigurationError = 27;

    /**
     * A return code indicating that you are trying to queue a message to a
     * handle's queue, but the queue is full. The maximum queue size can be
     * increased by using the MAXQUEUESIZE statement in the STAF Configuration
     * File. 
     */ 
    public static final int QueueFull = 28;

    /**
     * A return code indicating that you tried to GET or PEEK a particular
     * element in a queue, but no such element exists, or the queue is empty. 
     */ 
    public static final int NoQueueElement = 29;

    /**
     * A return code indicating that you are trying to remove a message
     * notification for a machine/process/priority combination which does not
     * exist in the notification list. 
     */ 
    public static final int NotifieeDoesNotExist = 30;

    /**
     * A return code indicating that a process has tried to call an invalid
     * level of an internal STAF API. If this error occurs, report it to the
     * authors. 
     */ 
    public static final int InvalidAPILevel = 31;

    /**
     * A return code indicating that you are trying to unregister a service
     * that is not unregisterable. Note that internal services are not
     * unregisterable. 
     */ 
    public static final int ServiceNotUnregisterable = 32;

    /**
     * A return code indicating that the service you requested is not
     * currently able to accept requests. The service may be in the process of
     * initializing or terminating. 
     */ 
    public static final int ServiceNotAvailable = 33;

    /**
     * A return code indicating that you are trying to release, query, or
     * delete a semaphore that does not exist. 
     */ 
    public static final int SemaphoreDoesNotExist = 34;

    /**
     * A return code indicating that you are trying to release a semaphore for
     * which your process is not the current owner. 
     */ 
    public static final int NotSemaphoreOwner = 35;

    /**
     * A return code indicating that you are trying to delete either a mutex
     * semaphore that is currently owned or an event semaphore that has
     * waiting processes. 
     */ 
    public static final int SemaphoreHasPendingRequests = 36;

    /**
     * A return code indicating that you submitted a request with a timeout
     * value and the request did not complete within the requested time. 
     */ 
    public static final int Timeout = 37;

    /**
     * A return code indicating an error performing a Java native method call.
     * A description of the error will be returned in the result passed back
     * from the submit call. 
     */ 
    public static final int JavaError = 38;

    /**
     * A return code indicating an error performing a codepage conversion.
     * The most likely cause of this error is that STAF was not properly
     * installed. However, it is possible that you are currently using a
     * codepage that was not present or specified during STAF installation. 
     */ 
    public static final int ConverterError = 39;

    /**
     * A return code indicating that there was an error while trying to move a
     * file or directory.
     * <p>
     * Note: Additional information regarding the error may be provided in the
     * result passed back from the submit call.
     */ 
    public static final int MoveError = 40;

    /**
     * A return code indicating that an invalid object was specified to a STAF
     * API. If you receive this return code via a standard STAFSubmit call,
     * report it to the authors and the service provider. 
     */ 
    public static final int InvalidObject = 41;

    /**
     * A return code indicating that an invalid parameter was specified to a
     * STAF API. If you receive this return code via a standard STAFSubmit
     * call, report it to the authors and the service provider. 
     */ 
    public static final int InvalidParm = 42;

    /**
     * A return code indicating that the specified Request Number was not
     * found. The specified Request Number may be invalid, or the request's
     * information may no longer be available from the Service Service (for
     * example, if the SERVICE FREE command had previously been issued for
     * the request number). 
     */ 
    public static final int RequestNumberNotFound = 43;

    /**
     * A return code indicating that an invalid Asynchronous submit option was
     * specified. 
     */ 
    public static final int InvalidAsynchOption = 44;

    /**
     * A return code indicating that the specified request is not complete.
     * This error code would be returned, for example, if you requested the
     * result of a request which has not yet completed. 
     */ 
    public static final int RequestNotComplete = 45;

    /**
     * A return code indicating that the userid/password you specified could
     * not be authenticated. The userid/password may not be valid or
     * authentication may be disabled. 
     */ 
    public static final int ProcessAuthenticationDenied = 46;

    /**
     * A return code indicating that an invalid value was specified. This is
     * closely related to the Invalid Request String return code, but
     * indicates that a specific value in the request is invalid. For example,
     * you may not have specified a number where a number was expected.
     * <p>
     * Note: Additional information regarding which value is invalid may be
     * provided in the result passed back from the submit call.
     */ 
    public static final int InvalidValue = 47;

    /**
     * A return code indicating that the item you specified does not exist.
     * <p>
     * Note: Additional information regarding which item could not be found
     * may be provided in the result passed back from the submit call.
     */ 
    public static final int DoesNotExist = 48;

    /**
     * A return code indicating that the item you specified already exists.
     * <p>
     * Note: Additional information regarding which item already exists may
     * be provided in the result passed back from the submit call.
     */ 
    public static final int AlreadyExists = 49;

    /**
     * A return code indicating that you have tried to delete a directory, but
     * that directory is not empty.
     * <p>
     * Note: Additional information specifying the directory which could not
     * be deleted may be provided in the result passed back from the submit
     * call.
     */ 
    public static final int DirectoryNotEmpty = 50;

    /**
     * A return code indicating that you have tried to copy a directory, but
     * errors occurred during the copy.
     * <p>
     * Note: Additional information specifying the entries which could not be
     * copied may be provided in the result passed back from the submit call.
     */ 
    public static final int DirectoryCopyError = 51;

    /**
     * A return code indicating that you tried to record diagnostics data, but
     * diagnostics have not been enabled. You must enable diagnostics before
     * you can record diagnostics data. 
     */ 
    public static final int DiagnosticsNotEnabled = 52;

    /**
     * A return code indicating that the user, credentials, and/or
     * authenticator you specified could not be authenticated. The
     * user/credentials may not be valid or the authenticator may not be
     * registered.
     * <p>
     * Note: Additional information specifying why authentication was denied
     * may be provided in the result passed back from the submit call.
     */ 
    public static final int HandleAuthenticationDenied = 53;

    /**
     * A return code indicating that the handle is already authenticated. The
     * handle must be unauthenticated in order to be authenticated. 
     */ 
    public static final int HandleAlreadyAuthenticated = 54;

    /**
     * A return code indicating that the version of STAF (or the version of
     * a STAF service) is lower than the minimum required version. 
     */ 
    public static final int InvalidSTAFVersion = 55;

    /**
     * A return code indicating that the request has been cancelled.
     * <p>
     * Note: Additional information specifying why the request was cancelled 
     * may be provided in the result passed back from the submit call.
     */ 
    public static final int RequestCancelled = 56;

    /**
     * A return code indicating that a problem occurred creating a new thread.
     * One possible explanation is that there's not enough memory available to
     * create a new thread.
     * <p>
     * Note: Additional information specifying why creating a new thread failed
     * may be provided in the result passed back from the submit call.
     */ 
    public static final int CreateThreadError = 57;

    /**
     * A return code indicating that the size of a file exceeded the maximum
     * size allowed (e.g. per the MAXRETURNFILESIZE operational parameter or
     * per the MAXRETURNFILESIZE setting for the STAX service). A maximum file
     * size is usually set to prevent the creation of result strings that
     * require more memory than is available which can cause errors or crashes.
     * <p>
     * Note: Additional information specifying why this error occurred may be
     * provided in the result passed back from the submit call.
     */ 
    public static final int MaximumSizeExceeded = 58;

    /**
     * A return code indicating that a new handle could not be created or
     * registered because the maximum number of active handles allowed by
     * STAF has been exceeded. You need to delete one or more handles that
     * are no longer being used. The Handle service's LIST HANDLES SUMMARY
     * request provides information on the maximum number of active STAF
     * handles and this may be helpful in better understanding why this error
     * occurred. 
     */ 
    public static final int MaximumHandlesExceeded = 59;

    /**
     * A return code indicating that you cannot cancel a pending request your
     * handle did not submit unless you specify the FORCE option. 
     */ 
    public static final int NotRequester = 60;

    /**
     * Error codes of 4000 and beyond are service specific error codes. Either
     * see the appropriate section in this document for the syntax of the
     * service's requests, or contact the provider of the service. 
     */ 
    public static final int UserDefined = 4000;
}
