/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;
import com.ibm.staf.service.*;

/**
 * This class provides STAF utility functions.
 */
public class STAFUtil
{
    /**
     * Opening privacy delimiter
     */ 
    public static String privacyDelimiter1 = "!!@";

    /**
     * Closing privacy delimiter
     */ 
    public static String privacyDelimiter2 = "@!!";

    /**
     * Escaped opening privacy delimiter
     */ 
    public static String escapedPrivacyDelimiter1 = "^!!@";

    /**
     * Escaped closing privacy delimiter
     */ 
    public static String escapedPrivacyDelimiter2 = "^@!!";

    /**
     * Escape character for privacy delimiters
     */ 
    public static String privacyDelimiterEscape = "^";

    /**
     * Number of milliseconds in a millisecond.  Used as a multiplier to get
     * the duration in milliseconds.
     */ 
    public static int sMILLISECOND = 1;

    /**
     * Number of milliseconds in a second.  Used as a multiplier to get the
     * duration in milliseconds.
     */ 
    public static int sSECOND_IN_MS = 1000;

    /**
     * Number of milliseconds in a minute.  Used as a multiplier to get the
     * duration in milliseconds.
     */ 
    public static int sMINUTE_IN_MS = 60000;

    /**
     * Number of milliseconds in an hour.  Used as a multiplier to get the
     * duration in milliseconds.
     */ 
    public static int sHOUR_IN_MS = 3600000;

    /**
     * Number of milliseconds in a day.  Used as a multiplier to get the
     * duration in milliseconds.
     */ 
    public static int sDAY_IN_MS = 24 * 3600000;

    /**
     * Number of milliseconds in a week.  Used as a multiplier to get the
     * duration in milliseconds.
     */ 
    public static int sWEEK_IN_MS = 7 * 24 * 3600000;

    /**
     * Maximum duration when specified in milliseconds.  Made the maximum
     * 4294967294 because we use 4294967295 to indicate an indefinite wait
     * on 32-bit machine (in C++).  For example:
     *   STAF_EVENT_SEM_INDEFINITE_WAIT = (unsigned int)-1
     * and because the duration value is often passed on to a C++ function such
     * as STAFThreadManager::sleepCurrentThread(unsigned int milliseconds).
     */ 
    public static long sMAX_MILLISECONDS = 4294967294L;

    /**
     * Maximum duration when specified in seconds.
     */ 
    public static int sMAX_SECONDS = 4294967;

    /**
     * Maximum duration when specified in minutes.
     */
    public static int sMAX_MINUTES = 71582;

    /**
     * Maximum duration when specified in hours
     */ 
    public static int sMAX_HOURS = 1193;

    /**
     * Maximum duration when specified in days
     */ 
    public static int sMAX_DAYS = 49;

    /**
     * Maximum duration when specified in weeks
     */ 
    public static int sMAX_WEEKS = 7;

    /**
     * Number of bytes in a byte.  Used as a multiplier to get the size in
     * bytes.
     */ 
    public static int sBYTES = 1;

    /**
     * Number of bytes in a kilobyte.  Used as a multiplier to get the size in
     * bytes.
     */ 
    public static int sBYTES_IN_KILOBYTES = 1024;

    /**
     * Number of bytes in a megabyte.  Used as a multiplier to get the size in
     * bytes.
     */ 
    public static int sBYTES_IN_MEGABYTES = 1048576;

    /**
     * Maximum size when specified in bytes.  Made the maximum 4294967294
     * because that's the maximum size of UINT_MAX on 32-bit machines in C++
     * and because the size value is often passed on to a C++ function.
     */ 
    public static long sMAX_BYTES = 4294967294L;

    /**
     * Maximum size when specified in kilobytes.
     */ 
    public static int sMAX_KILOBYTES = 4194303;

    /**
     * Maximum size when specified in megabytes
     */ 
    public static int sMAX_MEGABYTES = 4095;

    /**
     * This method returns a length delimited representation of a string,
     * the <code>:length:</code> format for the string (known as the colon
     * length colon format).
     * <p>
     * For example, if passed in "Hello world", this method would return
     * ":11:Hello world".
     * 
     * @param  data        the input string to be "wrapped"
     * @return             the string in a colon length colon format 
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDROVFORM">
     *      Section "7.2 Option Value Formats" in the STAF User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_wrapData">
     *      Section "3.4.1 Static Method STAFUtil.wrapData" in the STAF Java
     *      User's Guide</a>
     */ 
    public static String wrapData(String data)
    {
        return ":" + data.length() + ":" + data;
    }

    /**
     * This method returns the input string without the <code>:length:</code>
     * prefix, if present.
     * <p>
     * For example, if passed in ":11:Hello world", this method would return
     * "Hello world".
     * 
     * @param  data       the input string in a colon length colon format
     * @return            the string without the colon length colon format
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_unwrapData">
     *      Section "3.4.2 Static Method STAFUtil.unwrapData" in the STAF Java
     *      User's Guide</a>
     */ 
    public static String unwrapData(String data)
    {
        if (data != null)
        {
            int colon1Pos = data.indexOf(":");

            if (colon1Pos == 0)
            {
                int colon2Pos = data.indexOf(":", 1);

                if (colon2Pos > -1)
                {
                    try
                    {
                        // Verify that an integer was specified between the two
                        // colons to make sure the value has a colonLengthColon
                        // format, and just doesn't happen to contain two colons

                        int length = (new Integer(
                            data.substring(1, colon2Pos))).intValue();

                        String newValue = data.substring(colon2Pos + 1);

                        if (length == newValue.length())
                            return newValue;
                    }
                    catch (NumberFormatException e)
                    {
                        // Not a CLC format
                    }
                }
            }
        }

        return data;
    }

    /**
     * This method returns the endpoint without the port (strips @nnnn if
     * present from the end of the endpoint string).
     * <p>
     * For example, if passed in "tcp://client1.company.com@6500", would
     * return "tcp://client1.company.com".
     * 
     * @param  endpoint  an endpoint in the format of:
     *                   [&lt;Interface>://]&lt;System identifier>[@&lt;Port>]
     * @return           the endpoint without the port
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_stripPort">
     *      Section "3.4.3 Static Method STAFUtil.stripPortFromEndpoint" in
     *      the STAF Java User's Guide</a>
     */ 
    public static String stripPortFromEndpoint(String endpoint)
    {
        // Strip the port from the endpoint, if present.

        String endpointNoPort = endpoint;
        int portIndex = endpoint.lastIndexOf("@");

        if (portIndex != -1)
        {
            // If the characters following the "@" are numeric, then assume
            // it's a valid port and strip the @ and the port number from
            // the endpoint.

            try
            {
                int port = new Integer(endpoint.substring(portIndex + 1)).
                    intValue();
                endpointNoPort = endpoint.substring(0, portIndex);
            }
            catch (NumberFormatException e)
            {
                // Do nothing - Not valid port so don't remove from endpoint
            }
        }

        return endpointNoPort;
    }
    
    /**
     * This method validates that the requesting machine has the required
     * trust to submit a service request.  This method is intended for use
     * by writers of STAF Java services. 
     * <p>
     * Note:  Each time a new service interface level is added, must add
     * another <code>validateTrust</code> method overloaded to support the
     * new service interface level.
     * 
     * @param  requiredTrustLevel the required trust level for this service
     *                            request
     * @param  service            the registered name of the service 
     * @param  request            specifies the first word (or two) that
     *                            uniquely identifies the request if an error
     *                            occurs
     * @param  localMachine       specifies the logical identifier for the
     *                            service machine which will be used in the
     *                            error message if the requesting machine has
     *                            insufficient trust. 
     * @param  info               the STAF service interface request
     *                            information
     * @return                    a <code>STAFResult</code> object whose
     *                            <code>rc</code> field contains the return
     *                            code (0 if successful) and whose
     *                            <code>result</code> field contains if
     *                            successful or a detailed error message if
     *                            not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_validateTrust">
     *      Section "3.4.4 Static Method STAFUtil.validateTrust" in the STAF
     *      Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     */ 
    public static STAFResult validateTrust(
        int requiredTrustLevel, String service, String request,
        String localMachine, STAFServiceInterfaceLevel30.RequestInfo info)
    {
        if (info.trustLevel < requiredTrustLevel)
        {
            // Strip the port from the machine's endpoint, if present.

            String endpoint = stripPortFromEndpoint(info.endpoint);

            return new STAFResult(
                STAFResult.AccessDenied,
                "Trust level " + requiredTrustLevel + " required for the " +
                service + " service's " + request + " request\n" +
                "Requester has trust level " + info.trustLevel +
                " on machine " + localMachine + "\nRequesting machine: " +
                 endpoint + " (" + info.physicalInterfaceID + ")" +
                "\nRequesting user   : " + info.user);
        }
    
        return new STAFResult(STAFResult.Ok);
    }

    /**
     * This method verifies that the timeout duration is valid and returns
     * the timeout duration converted to milliseconds.
     * <p>
     * Examples of input timeout duration strings are: "100", "1s", "5m",
     * "1h", "1d", "1w"
     * 
     * @param  durationString  a string containing a timeout duration in the
     *                         format: &lt;Number>[s|m|h|d|w]
     * @return                 a <code>STAFResult</code> object whose
     *                         <code>rc</code> field contains the return code
     *                         (0 if successful) and whose <code>result</code>
     *                         field contains the converted duration in
     *                         milliseconds if successful or an error message
     *                         if not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_convertDuration">
     *      Section "3.4.5 Static Method STAFUtil.convertDurationString" in
     *      the STAF Java User's Guide</a>
     * @since  STAF 3.3.2
     */ 
    public static STAFResult convertDurationString(String durationString)
    {
        int rc = STAFResult.Ok;
        String durationStr = durationString;
        long duration = 0;

        // Asume duration is specified in milliseconds if numeric
        int multiplier = 1;

        if ((durationStr == null) || (durationStr.length() == 0))
        {
            rc = STAFResult.InvalidValue;
        }
        else
        {
            // Check if the duration string is not all digits

            try
            {
                duration = (new Long(durationStr)).longValue();

                if (duration < 0)
                {
                    rc = STAFResult.InvalidValue;
                }
            }
            catch (NumberFormatException e)
            {
                // Get duration type (last character of duration string)

                String durationType = durationStr.substring(
                    durationStr.length() - 1).toLowerCase();

                if (durationType.equals("s"))
                    multiplier = sSECOND_IN_MS;
                else if (durationType.equals("m"))
                    multiplier = sMINUTE_IN_MS;
                else if (durationType.equals("h"))
                    multiplier = sHOUR_IN_MS;
                else if (durationType.equals("d"))
                    multiplier = sDAY_IN_MS;
                else if (durationType.equals("w"))
                    multiplier = sWEEK_IN_MS;
                else
                    rc = STAFResult.InvalidValue;

                if (rc == STAFResult.Ok)
                {
                    // Assign numeric duration value (all characters
                    // except the last one)
                    durationStr = durationString.substring(
                        0, durationString.length() - 1);
                        
                    try
                    {
                        duration = (new Long(durationStr)).longValue();

                        if (duration < 0)
                        {
                            rc = STAFResult.InvalidValue;
                        }
                    }
                    catch (NumberFormatException e2)
                    {
                        rc = STAFResult.InvalidValue;
                    }
                }
            }
        }
        
        if (rc != STAFResult.Ok)
        {
            return new STAFResult(
                rc,
                "This value may be expressed in milliseconds, seconds, " +
                "minutes, hours, days, or weeks.  Its format is " +
                "<Number>[s|m|h|d|w] where <Number> is an integer >= 0 and " +
                "indicates milliseconds unless one of the following " +
                "case-insensitive suffixes is specified:  s (for seconds), " +
                "m (for minutes), h (for hours), d (for days), or " +
                "w (for weeks).  The calculated value cannot exceed " +
                "4294967294 milliseconds.\n\nExamples: \n" +
                "  100 specifies 100 milliseconds, \n" +
                "  10s specifies 10 seconds, \n" +
                "  5m specifies 5 minutes, \n" +
                "  2h specifies 2 hours, \n" +
                "  1d specifies 1 day, \n" +
                "  1w specifies 1 week.");
        }

        // Because the maximum duration in milliseconds cannot exceed
        // 4294967294:
        // - Duration specified in seconds cannot exceed 4294967 seconds
        // - Duration specified in minutes cannot exceed 71582 minutes
        // - Duration specified in hours cannot exceed 1193 hours
        // - Duration specified in days cannot exceed 49 days
        // - Duration specified in weeks cannot exceed 7 weeks

        // Note: The maximum is 4294967294 because we use 4294967295 to
        // indicate an indefinite wait on 32-bit machine (in C++).
        // For example:  STAF_EVENT_SEM_INDEFINITE_WAIT = (unsigned int)-1
        // Alsso, because the duration value is often passed on to a C++
        // function such as:
        //   STAFThreadManager::sleepCurrentThread(unsigned int milliseconds)

        if (((multiplier == sMILLISECOND) && (duration > sMAX_MILLISECONDS)) ||
            ((multiplier == sSECOND_IN_MS) && (duration > sMAX_SECONDS)) ||
            ((multiplier == sMINUTE_IN_MS) && (duration > sMAX_MINUTES)) ||
            ((multiplier == sHOUR_IN_MS) && (duration > sMAX_HOURS)) ||
            ((multiplier == sDAY_IN_MS) && (duration > sMAX_DAYS)) ||
            ((multiplier == sWEEK_IN_MS) && (duration > sMAX_WEEKS)))
        {
            rc = STAFResult.InvalidValue;
        }

        if (rc == STAFResult.Ok)
        {
            duration *= multiplier;

            return new STAFResult(
                STAFResult.Ok, (new Long(duration)).toString());
        }
        
        String errorMsg = "";

        if (multiplier == sMILLISECOND)
            errorMsg = "Cannot exceed " + sMAX_MILLISECONDS + " milliseconds.";
        else if (multiplier == sSECOND_IN_MS)
            errorMsg = "Cannot exceed " + sMAX_SECONDS + " seconds.";
        else if (multiplier == sMINUTE_IN_MS)
            errorMsg = "Cannot exceed " + sMAX_MINUTES + " minutes.";
        else if (multiplier == sHOUR_IN_MS)
            errorMsg = "Cannot exceed " + sMAX_HOURS + " hours.";
        else if (multiplier == sDAY_IN_MS)
            errorMsg = "Cannot exceed " + sMAX_DAYS + " days.";
        else if (multiplier == sWEEK_IN_MS)
            errorMsg = "Cannot exceed " + sMAX_WEEKS + " weeks.";
        
        return new STAFResult(rc, errorMsg);
    }

    /**
     * This method verifies that the input file size string is valid and is in
     * format &lt;Number>[k|m] and returns the size converted to bytes.
     * <p>
     * Examples of valid input size strings are:  "100000", "500k", "5m"
     * 
     * @param  sizeString  a string containing a size in the format:
     *                     &lt;Number>[k|m]
     * @return             a <code>STAFResult</code> object whose
     *                     <code>rc</code> field contains the return code
     *                     (0 if successful) and whose <code>result</code>
     *                     field contains the converted size in bytes if
     *                     successful or an error message if not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_convertSize">
     *      Section "3.4.6 Static Method STAFUtil.convertSizeString" in
     *      the STAF Java User's Guide</a>
     * @since  STAF 3.3.4
     */ 
    public static STAFResult convertSizeString(String sizeString)
    {
        int rc = STAFResult.Ok;
        String sizeStr = sizeString;
        long size = 0;

        // Assume size is specified in bytes if numeric
        int multiplier = sBYTES;

        if ((sizeStr == null) || (sizeStr.length() == 0))
        {
            rc = STAFResult.InvalidValue;
        }
        else
        {
            // Check if the size string is not all digits

            try
            {
                size = (new Long(sizeStr)).longValue();

                if (size < 0)
                {
                    rc = STAFResult.InvalidValue;
                }
            }
            catch (NumberFormatException e)
            {
                // Get size type (last character of size string)

                String sizeType = sizeStr.substring(
                    sizeStr.length() - 1).toLowerCase();

                if (sizeType.equals("k"))
                    multiplier = sBYTES_IN_KILOBYTES;
                else if (sizeType.equals("m"))
                    multiplier = sBYTES_IN_MEGABYTES;
                else
                    rc = STAFResult.InvalidValue;

                if (rc == STAFResult.Ok)
                {
                    // Assign numeric size value (all characters
                    // except the last one)
                    sizeStr = sizeString.substring(
                        0, sizeString.length() - 1);
                        
                    try
                    {
                        size = (new Long(sizeStr)).longValue();

                        if (size < 0)
                        {
                            rc = STAFResult.InvalidValue;
                        }
                    }
                    catch (NumberFormatException e2)
                    {
                        rc = STAFResult.InvalidValue;
                    }
                }
            }
        }
        
        if (rc != STAFResult.Ok)
        {
            return new STAFResult(
                rc,
                "This value may be expressed in bytes, kilobytes, or " +
                "megabytes.  Its format is " +
                "<Number>[k|m] where <Number> is an integer >= 0 and " +
                "indicates bytes unless one of the following " +
                "case-insensitive suffixes is specified:  " +
                "k (for kilobytes) or m (for megabytes).  " +
                "The calculated value cannot exceed 4294967294 bytes.\n\n" +
                "Examples: \n" +
                "  100000 specifies 100,000 bytes, \n" +
                "  500k specifies 500 kilobytes (or 512,000 bytes), \n" +
                "  5m specifies 5 megabytes (or 5,242,880 bytes), \n" +
                "  0 specifies no maximum size limit");
        }

        // Because the maximum size in bytes cannot exceed 4294967294:
        // - Size specified in kilobytes cannot exceed 4294967 seconds
        // - Size specified in megabytes cannot exceed 71582 minutes

        if (((multiplier == sBYTES) && (size > sMAX_BYTES)) ||
            ((multiplier == sBYTES_IN_KILOBYTES) && (size > sMAX_KILOBYTES)) ||
            ((multiplier == sBYTES_IN_MEGABYTES) && (size > sMAX_MEGABYTES)))
        {
            rc = STAFResult.InvalidValue;
        }

        if (rc == STAFResult.Ok)
        {
            size *= multiplier;

            return new STAFResult(
                STAFResult.Ok, (new Long(size)).toString());
        }
        
        String errorMsg = "";

        if (multiplier == sBYTES)
            errorMsg = "Cannot exceed " + sMAX_BYTES + " bytes.";
        else if (multiplier == sBYTES_IN_KILOBYTES)
            errorMsg = "Cannot exceed " + sMAX_KILOBYTES + " kilobytes.";
        else if (multiplier == sBYTES_IN_MEGABYTES)
            errorMsg = "Cannot exceed " + sMAX_MEGABYTES + " megabytes.";
        
        return new STAFResult(rc, errorMsg);
    }

    /**
     * This method resolves any STAF variables that are contained within the
     * string passed in by submitting a "RESOLVE REQUEST request# STRING
     * value" request to the VAR service on the local machine. The variables
     * will be resolved using the originating handle's pool associated with
     * the specified request number, the local machine's shared variable pool,
     * and the local machine's system variable pool.
     * <p>
     * This method should be used by writers of STAF Java services as most
     * option values in a service request should be resolved. 
     * 
     * @param  value         the value of an option in a service request string
     *                       that may contain STAF variables, such as
     *                       {STAF/Config/Machine}
     * @param  handle        a <code>STAFHandle</code> used to submit the
     *                       VAR RESOLVE request
     * @param  requestNumber the request number
     * @return               a <code>STAFResult</code> object whose
     *                       <code>rc</code> field contains the return code
     *                       (0 if successful) and whose <code>result</code>
     *                       field contains the resolved option's value if
     *                       successful or an error message if not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_resRequestVar">
     *      Section "3.4.7 Static Method STAFUtil.resolveRequestVar" in
     *      the STAF Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     */ 
    public static STAFResult resolveRequestVar(
        String value, STAFHandle handle, int requestNumber)
    {
        if (value.indexOf("{") != -1)
        {
            // The string may contains STAF variables
            
            STAFResult resolvedResult = handle.submit2(
                "local", "VAR", "RESOLVE REQUEST " + requestNumber +
                " STRING " + STAFUtil.wrapData(value));

            return resolvedResult;
        }

        return new STAFResult(STAFResult.Ok, value);
    }
    
    /**
     * This method resolves any STAF variables that are contained within
     * the option value passed in and then checks if the resolved value is an
     * integer.  It resolves STAF variables by submitting a
     * "RESOLVE REQUEST request# STRING value" request to the VAR service
     * on the local system.  STAF variables will be resolved using the
     * originating handle's pool associated with the specified request number,
     * the local system's shared pool, and the local system's system pool.
     * <p>
     * This method should be used by writers of STAF Java services as most
     * option values in a service request should be resolved. 
     *
     * @param  option        the name of an option in a service request string
     *                       whose value is being resolved
     * @param  value         the value of an option in a service request string
     *                       that may contain STAF variables to be resolved,
     *                       such as {STAF/Config/Machine}
     * @param  handle        a <code>STAFHandle</code> used to submit the
     *                       VAR RESOLVE request
     * @param  requestNumber the request number
     * @return               a <code>STAFResult</code> object whose
     *                       <code>rc</code> field contains the return code
     *                       (0 if successful) and whose <code>result</code>
     *                       field contains the resolved option's value if
     *                       successful or an error message if not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_resRequestVarAndCheckInt">
     *      Section "3.4.8 Static Method STAFUtil.resolveRequestVarAndCheckInt"
     *      in the STAF Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     */ 
    public static STAFResult resolveRequestVarAndCheckInt(
        String option, String value, STAFHandle handle, int requestNumber)
    {
        STAFResult resolvedValue = resolveRequestVar(
            value, handle, requestNumber);

        if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;

        try
        {
            Integer.parseInt(resolvedValue.result);
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(
                STAFResult.InvalidValue, option +
                " value must be an Integer.  " +
                option + "=" + resolvedValue.result);
        }

        return resolvedValue;
    }

    /**
     * This method resolves any STAF variables that are contained within
     * the option value passed in and then checks if the resolved value is a
     * valid timeout duration and if so, converts it into milliseconds.
     * It resolves STAF variables by submitting a
     * "RESOLVE REQUEST request# STRING value" request to the VAR service
     * on the local system.  STAF variables will be resolved using the
     * originating handle's pool associated with the specified request number,
     * the local system's shared pool, and the local system's system pool.
     * <p>
     * This method may be used by writers of STAF Java services to resolve a
     * duration timeout option that needs to be converted to milliseconds. 
     *
     * @param  option        the name of an option in a service request string
     *                       whose value is being resolved
     * @param  value         the value of an option in a service request string
     *                       that may contain STAF variables to be resolved.
     *                       This value specifies the timeout duration in the
     *                       format <code>&lt;Number>[s|m|h|d|w]</code>. For
     *                       example: "100", "1s", "5m", "1h".
     * @param  handle        a <code>STAFHandle</code> used to submit the
     *                       VAR RESOLVE request
     * @param  requestNumber the request number
     * @return               a <code>STAFResult</code> object whose
     *                       <code>rc</code> field contains the return code
     *                       (0 if successful) and whose <code>result</code>
     *                       field contains the resolved option's timeout
     *                       duration in milliseconds if successful or an
     *                       error message if not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_resRequestVarAndConvertDuration">
     *      Section "3.4.9 Static Method STAFUtil.resolveRequestVarAndConvertDuration"
     *      in the STAF Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     * @since  STAF 3.3.2
     */ 
    public static STAFResult resolveRequestVarAndConvertDuration(
        String option, String value, STAFHandle handle, int requestNumber)
    {
        STAFResult resolvedValue = resolveRequestVar(
            value, handle, requestNumber);

        if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;

        STAFResult result = convertDurationString(resolvedValue.result);
        
        if (result.rc != STAFResult.Ok)
        {
            result.result = "Invalid value for the " + option + " option: " +
                resolvedValue.result + " \n\n" + result.result;
        }

        return result;
    }
    
    /**
     * This method resolves any STAF variables that are contained within
     * the option value passed in and then checks if the resolved value is a
     * valid size and if so, converts the size into bytes.
     * It resolves STAF variables by submitting a
     * "RESOLVE REQUEST request# STRING value" request to the VAR service
     * on the local system.  STAF variables will be resolved using the
     * originating handle's pool associated with the specified request number,
     * the local system's shared pool, and the local system's system pool.
     * <p>
     * This method may be used by writers of STAF Java services to resolve a
     * size option that needs to be converted to bytes. 
     *
     * @param  option        the name of an option in a service request string
     *                       whose value is being resolved
     * @param  value         the value of an option in a service request string
     *                       that may contain STAF variables to be resolved,
     *                       This value specifies the size in the format
     *                       <code>&lt;Number>[k|m]</code>. For example:
     *                       "1000000", "500k", "5m".
     * @param  handle        a <code>STAFHandle</code> used to submit the
     *                       VAR RESOLVE request
     * @param  requestNumber the request number
     * @return               a <code>STAFResult</code> object whose
     *                       <code>rc</code> field contains the return code
     *                       (0 if successful) and whose <code>result</code>
     *                       field contains the resolved option's size in
     *                       bytes if successful or an error message if not
     *                       successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_resRequestVarAndConvertSize">
     *      Section "3.4.10 Static Method STAFUtil.resolveRequestVarAndConvertSize"
     *      in the STAF Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     * @since  STAF 3.3.4
     */ 
    public static STAFResult resolveRequestVarAndConvertSize(
        String option, String value, STAFHandle handle, int requestNumber)
    {
        STAFResult resolvedValue = resolveRequestVar(
            value, handle, requestNumber);

        if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;

        STAFResult result = convertSizeString(resolvedValue.result);
        
        if (result.rc != STAFResult.Ok)
        {
            result.result = "Invalid value for the " + option + " option: " +
                resolvedValue.result + " \n\n" + result.result;
        }

        return result;
    }

    /**
     * This method resolves any STAF variables that are contained within
     * the string passed in the <code>value</code> parameter.  It does this
     * by submitting a "RESOLVE STRING value" request to the VAR service on
     * the local system.
     * <p>
     * This method should be used by writers of STAF Java services if they
     * need to resolve a variable in the init() method for a Java service. 
     * 
     * @param  value         a string that may contain STAF variables to be
     *                       resolved, such as {STAF/Config/Machine}
     * @param  handle        a <code>STAFHandle</code> used to submit the
     *                       VAR RESOLVE request
     * @return               a <code>STAFResult</code> object whose
     *                       <code>rc</code> field contains the return code
     *                       (0 if successful) and whose <code>result</code>
     *                       field contains a string with all the variables
     *                       resolved if successful, or an error message if
     *                       not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_resInitVar">
     *      Section "3.4.11 Static Method STAFUtil.resolveInitVar"
     *      in the STAF Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     */ 
    public static STAFResult resolveInitVar(String value, STAFHandle handle)
    {
        if (value.indexOf("{") != -1)
        {
            // The string may contains STAF variables

            STAFResult resolvedResult = handle.submit2(
                "local", "VAR", "RESOLVE STRING " + STAFUtil.wrapData(value));

            return resolvedResult;
        }

        return new STAFResult(STAFResult.Ok, value);
    }

    /**
     * This method resolves any STAF variables that are contained within
     * the option value passed in and then checks if the resolved value is an
     * integer and returns an error if it is not an integer.  It resolves
     * STAF variables by submitting a "RESOLVE STRING value" request to the
     * VAR service on the local system.
     * <p>
     * This method should be used by writers of STAF Java services if they
     * need to resolve a variable in the init() method for a Java service. 
     *
     * @param  option        the name of an option in a service request string
     *                       whose value is being resolved
     * @param  value         an option value in a service request string that
     *                       may contain STAF variables, such as
     *                       {STAF/Config/Machine}
     * @param  handle        a <code>STAFHandle</code> used to submit the
     *                       VAR RESOLVE request
     * @return               a <code>STAFResult</code> object whose
     *                       <code>rc</code> field contains the return code
     *                       (0 if successful) and whose <code>result</code>
     *                       field contains the resolved option value if
     *                       successful or an error message if not successful
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_resInitVarAndCheckInt">
     *      Section "3.4.12 Static Method STAFUtil.resolveInitVarAndCheckInt"
     *      in the STAF Java User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/stafsdg.html">
     *      STAF Service Developer's Guide</a>
     */ 
    public static STAFResult resolveInitVarAndCheckInt(
        String option, String value, STAFHandle handle)
    {
        STAFResult resolvedValue = resolveInitVar(value, handle);

        if (resolvedValue.rc != STAFResult.Ok) return resolvedValue;

        try
        {
            Integer.parseInt(resolvedValue.result);
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(
                STAFResult.InvalidValue, option + " value must be numeric.  " +
                option + "=" + resolvedValue.result);
        }

        return resolvedValue;
    }

    /**
     * This method gets the version of STAF running on a specified machine
     * and then checks if the version meets the minimum required version
     * specified in the <code>minRequiredVersion</code> parameter.  It gets
     * the STAF version by submitting a VERSION request to the MISC service
     * on the specified machine.  If the version is at or above the required
     * version, a <code>STAFResult</code> object is returned with its
     * <code>rc</code> field set to 0 and its <code>result</code> field set to
     * the version running on the specified machine.  If the version is lower
     * than the mininum required version, a <code>STAFResult</code> object is
     * returned with its <code>rc</code> field set to
     * <code>STAFResult.InvalidSTAFVersion</code> and an error message in its
     * <code>result</code> field.  If another error occurs (e.g. RC 16 if
     * the machine is not currently running STAF, etc.), that return code
     * and possibly an error message will be returned in the
     * <code>STAFResult</code> object.
     * <p>
     * This method should be used by writers of STAF Java services if the
     * service they are writing needs a particular version of STAF. 
     * <p>
     * The versions being compared must have the following format unless it is
     * blank or "<N/A>", which equates to "no version" and is internally
     * represented as 0.0.0.0:
     * <p>
     *   <code>a[.b[.c[.d]]] [text]</code>
     * <p>
     * where:
     * <ul>
     *   <li>a, b, c, and d (if specified) are numeric
     *   <li>text is separated by one or more spaces from the version numbers
     * </ul>
     * <p>
     * Versions are compared as follows:
     * <ol type="a">
     * <li> The numeric versions (a[.b[.c[.d]]]) are numerically compared.
     * <li> If the numeric versions are "equal", then the [text] values are
     *      compared using a case-insensitive string compare.  Except, note
     *      that no text is considered GREATER than any text.  For example,
     *      "3.1.0" > "3.1.0 Beta 1").
     * </ol>
     * <p>
     * Examples:
     * <ul>
     *   <li>"3" = "3.0" = "3.0.0" = "3.0.0.0"
     *   <li>"3.0.0" < "3.1.0"
     *   <li>"3.0.2" < "3.0.3"
     *   <li>"3.0.0" < "3.1"
     *   <li>"3.0.9" < "3.0.10"
     *   <li>"3.1.0 Beta 1" < "3.1.0"
     *   <li>"3.1.0 Alpha 1" < "3.1.0 Beta 1"
     * </ul>
     * 
     * @param  machine  endpoint of the machine whose STAF version is to be
     *                  compared
     * @param  handle   STAF handle to use to submit the request
     * @param  minRequiredVersion  The minimum required version.
     *         The version must have the following format unless it is blank
     *         or "<N/A>", which equates to "no version" and is internally
     *         represented as 0.0.0.0:  <code>a[.b[.c[.d]]] [text]</code>
     * @return          a <code>STAFResult</code> object.  If the version
     *                  is at or above the required version, its
     *                  <code>rc</code> field set to 0 and its
     *                  <code>result</code> field set to the version
     *                  running on the specified machine.  If the version is
     *                  lower than the mininum required version, its
     *                  <code>rc</code> field is set to
     *                  <code>STAFResult.InvalidSTAFVersion</code> and its
     *                  <code>result</code> field contains an error message.
     *                  Other errors may occur as well such as RC 16 if the
     *                  machine is not currently running STAF, etc.), and
     *                  then its <code>rc</code> field will be set to it
     *                  and the <code>result</code> field will possibly
     *                  contain an error message.
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_compareSTAFVersion">
     *      Section "3.4.13 Static Method STAFUtil.compareSTAFVersion"
     *      in the STAF Java User's Guide</a>
     * @since  STAF 3.1.0
     */
    public static STAFResult compareSTAFVersion(
        String machine, STAFHandle handle, String minRequiredVersion)
    {
        // The default is to check the version of STAF running (which is
        // done by submitting a VERSION request to the MISC service)
        return compareSTAFVersion(
            machine, handle, minRequiredVersion, "MISC");
    }

    /**
     * This method gets the version of STAF (or of a STAF service)
     * running on a specified machine and then checks if the version meets
     * the minimum required version specified in the
     * <code>minRequiredVersion</code> parameter.  It gets the version by
     * submitting a VERSION request to either the MISC service (to get the
     * version of STAF) or to another STAF service on the specified machine.
     * If the version is at or above the required version, a
     * <code>STAFResult</code> object is returned with its <code>rc</code>
     * field set to 0 and its <code>result</code> field set to the version
     * running on the specified machine.  If the version is lower than the
     * mininum required version, a <code>STAFResult</code> object is returned
     * with its <code>rc</code> field set to
     * <code>STAFResult.InvalidSTAFVersion</code> and an error message in its
     * <code>result</code> field.  If another error occurs (e.g. RC 16 if
     * the machine is not currently running STAF, etc.), that return code
     * and possibly an error message will be returned in the
     * <code>STAFResult</code> object.
     * <p>
     * This method should be used by writers of STAF Java services if the
     * service they are writing needs a particular version of STAF. 
     * <p>
     * The versions being compared must have the following format unless it is
     * blank or "<N/A>", which equates to "no version" and is internally
     * represented as 0.0.0.0:
     * <p>
     *   <code>a[.b[.c[.d]]] [text]</code>
     * <p>
     * where:
     * <ul>
     *   <li>a, b, c, and d (if specified) are numeric
     *   <li>text is separated by one or more spaces from the version numbers
     * </ul>
     * <p>
     * Versions are compared as follows:
     * <ol type="a">
     * <li> The numeric versions (a[.b[.c[.d]]]) are numerically compared.
     * <li> If the numeric versions are "equal", then the [text] values are
     *      compared using a case-insensitive string compare.  Except, note
     *      that no text is considered GREATER than any text.  For example,
     *      "3.1.0" > "3.1.0 Beta 1").
     * </ol>
     * <p>
     * Examples:
     * <ul>
     *   <li>"3" = "3.0" = "3.0.0" = "3.0.0.0"
     *   <li>"3.0.0" < "3.1.0"
     *   <li>"3.0.2" < "3.0.3"
     *   <li>"3.0.0" < "3.1"
     *   <li>"3.0.9" < "3.0.10"
     *   <li>"3.1.0 Beta 1" < "3.1.0"
     *   <li>"3.1.0 Alpha 1" < "3.1.0 Beta 1"
     * </ul>
     * 
     * @param  machine  endpoint of the machine whose STAF or STAF service
     *                  version is to be compared
     * @param  handle   STAF handle to use to submit the request
     * @param  minRequiredVersion  The minimum required version.
     *         The version must have the following format unless it is blank
     *         or "<N/A>", which equates to "no version" and is internally
     *         represented as 0.0.0.0:  <code>a[.b[.c[.d]]] [text]</code>
     * @param  service  Name of the service for which you want the
     *                  version of.  Optional.  Defaults to "MISC"
     *                  which means that you want to compare the
     *                  version of STAF running on the machine.
     *                  Or, you can specify the name of a STAF 
     *                  service (such as STAX, Event, Cron, etc.)
     *                  that implements a VERSION request and
     *                  follows the STAF version format requirements.
     * @return          a <code>STAFResult</code> object.  If the version
     *                  is at or above the required version, its
     *                  <code>rc</code> field set to 0 and its
     *                  <code>result</code> field set to the version
     *                  running on the specified machine.  If the version is
     *                  lower than the mininum required version, its
     *                  <code>rc</code> field is set to
     *                  <code>STAFResult.InvalidSTAFVersion</code> and its
     *                  <code>result</code> field contains an error message.
     *                  Other errors may occur as well such as RC 16 if the
     *                  machine is not currently running STAF, etc.), and
     *                  then its <code>rc</code> field will be set to it
     *                  and the <code>result</code> field will possibly
     *                  contain an error message.
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_compareSTAFVersion">
     *      Section "3.4.13 Static Method STAFUtil.compareSTAFVersion"
     *      in the STAF Java User's Guide</a>
     * @since  STAF 3.1.0
     */
    public static STAFResult compareSTAFVersion(
        String machine, STAFHandle handle, String minRequiredVersion,
        String service)
    {
        STAFResult res = handle.submit2(
            machine, service, "VERSION");

        if (res.rc != STAFResult.Ok)
        {
            return new STAFResult(
                res.rc,
                "Request VERSION submitted to the " + service +
                " service on machine " + machine + " failed." +
                "  Additional info: " + res.result);
        }
        else
        {
            STAFVersion version;
            STAFVersion minReqVersion;

            try
            {
                version = new STAFVersion(res.result);
            }
            catch (NumberFormatException e)
            {
                // Should never happen
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid value specified for the version: " +
                    res.result + ", Exception info: " + e.toString());
            }

            try
            {
                minReqVersion = new STAFVersion(minRequiredVersion);
            }
            catch (NumberFormatException e)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid value specified for the minimum required " +
                    "version: " + minRequiredVersion +
                    ", Exception info: " + e.toString());
            }

            if (version.compareTo(minReqVersion) < 0)
            {
                String servMsg = service + " service";

                if (service.equalsIgnoreCase("MISC"))
                    servMsg = "STAF";

                return new STAFResult(
                    STAFResult.InvalidSTAFVersion,
                    "Machine " + machine + " is running " + servMsg +
                    " Version " + version + ".  Version " +
                    minRequiredVersion + " or later is required.");
            }
            else
            {
                return new STAFResult(STAFResult.Ok, version.toString());
            }
        }
    }
    
    /**
     * This method adds privacy delimiters to a string and returns the updated
     * string.
     * <p>
     * This method should be used by anyone who wants to protect private data
     * specified in a STAF command option that supports handling private data.
     * <p>
     * Examples: 
     * <ul>
     * <li>If the data is "passw0rd", this method would return "!!@passw0rd@!!".
     * <li>If the data is "Password: !!@secret@!!", this method would return
     *     "!!@Password: ^!!@secret^@!!@!!".
     * </ul>
     * @param  data  a string containing data you want to protect
     * @return       a string containing the data with opening and closing
     *               privacy delimiters added and escapes any privacy
     *               delimiters already contained in the string with a caret
     *               (^). If the string has length 0 or already has an
     *               unescaped opening privacy delimiter at the beginning and
     *               an unescaped closing privacy delimiter at the end,
     *               privacy delimiters are not added.
     * @since STAF 3.1.0
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRPRIVATEDATA">
     *      Section "7.3 Private Data" in the STAF User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_addPrivacyDelimiters">
     *      Section "3.3.1 Static Method STAFUtil.addPrivacyDelimiters" in the
     *      STAF Java User's Guide</a>
     */ 
    public static String addPrivacyDelimiters(String data)
    {
        return STAFUtilAddPrivacyDelimiters(data);
    }
    
    /**
     * This method removes all privacy delimiters from the input data and
     * returns the updated string.
     * <p>
     * Examples: 
     * <ul>
     * <li>If the data is "!!@passw0rd@!!", this method would return
     *     "passw0rd".
     * <li>If the data is "!!@passw^@!!d@!!", this method would return
     *     "passw@!!d".
     * <li>If the data is "!!@Password=^!!@secret^@!!.@!!", this method
     *     would return "Password=secret".
     * </ul>
     * 
     * @param  data  a string containing data that may contain privacy
     *               delimiters
     * @return       a string with the privacy delimiters removed
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRPRIVATEDATA">
     *      Section "7.3 Private Data" in the STAF User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_removePrivacyDelimiters">
     *      Section "3.3.3 Static Method STAFUtil.removePrivacyDelimiters" in the
     *      STAF Java User's Guide</a>
     * @since STAF 3.1.0
     */ 
    public static String removePrivacyDelimiters(String data)
    {
        return STAFUtil.removePrivacyDelimiters(data, 0);
    }
    
    /**
     * This method removes privacy delimiters from the input data (allowing
     * removal of only a specified number of privacy levels) and returns the
     * updated string.
     * <p>
     * Examples: 
     * <ul>
     * <li>If the data is "!!@passw0rd@!!", this method would return
     *     "passw0rd".
     * <li>If the data is "!!@passw^@!!d@!!", this method would return
     *     "passw@!!d".
     * <li>If the data is "!!@Password=^!!@secret^@!!.@!!" and the numLevels
     *     is 0, this method would return "Password=secret".
     * <li>If the data is "!!@Password=^!!@secret^@!!.@!!" and the numLevels
     *     is 1, this method would return "Password=!!@secret@!!".
     * </ul>
     * 
     * @param  data       a string containing data that may be contain
     *                    privacy delimiters
     * @param  numLevels  the number of levels of privacy data to remove.
     *                    The default is 0 which indicates to remove all
     *                    levels of privacy data.  Note that, generally,
     *                    you'll want to remove all levels of privacy
     *                    delimiters. 
     * @return            a string with the specified levels of privacy
     *                    delimiters removed
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRPRIVATEDATA">
     *      Section "7.3 Private Data" in the STAF User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_removePrivacyDelimiters">
     *      Section "3.3.3 Static Method STAFUtil.removePrivacyDelimiters" in the
     *      STAF Java User's Guide</a>
     * @since STAF 3.1.0
     */ 
    public static String removePrivacyDelimiters(String data, int numLevels)
    {
        return STAFUtilRemovePrivacyDelimiters(data, numLevels);
    }
    
    /**
     * This method masks any private data indicated by the privacy delimiters
     * by replacing the private data with asterisks and returns the updated
     * string.
     * <p>
     * Examples:
     * <ul>
     * <li>If the data is "!!@passw0rd@!!", this method would
     *     return "**************".
     * <li>If the data is "testA -password !!@secret@!!", this method would
     *     return "testA -password ************".
     * </ul>
     * 
     * @param  data       a string containing data that may contain privacy
     *                    delimiters
     * @return            a string with any private data masked
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRPRIVATEDATA">
     *      Section "7.3 Private Data" in the STAF User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_maskPrivateData">
     *      Section "3.3.4 Static Method STAFUtil.maskPrivateData" in the
     *      STAF Java User's Guide</a>
     * @since STAF 3.1.0
     */ 
    public static String maskPrivateData(String data)
    {
        return STAFUtilMaskPrivateData(data);
    }
    
    /**
     * This method escapes all privacy delimiters (!!@ and @!!) found in the input data,
     * and returns the updated string.
     * This method escapes all privacy delimiters (!!@ and @!!) found in the
     * data with a caret (^) and returns the updated string.
     * <p>
     * This method should be used before calling the
     * <code>addPrivacyDelimiters</code> method for data that needs to be
     * protected but may contain substrings !!@ and/or @!! that should not be
     * mistaken for privacy delimiters.
     * <p>
     * For example, if the data is "passw@!!d", this method would return
     * "passw^@!!d". 
     * 
     * @param  data       a string containing data that may contin substrings
     *                    !!@ and/or @!! that should not be mistaken for
     *                    privacy delimiters
     * @return            a string with all privacy delimiters escaped with
     *                    the escape character (^)
     * @see <a href="http://staf.sourceforge.net/current/STAFUG.htm#HDRPRIVATEDATA">
     *      Section "7.3 Private Data" in the STAF User's Guide</a>
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_escapePrivacyDelimiters">
     *      Section "3.3.2 Static Method STAFUtil.escapePrivacyDelimiters" in the
     *      STAF Java User's Guide</a>
     * @since  STAF 3.1.0
     */ 
    public static String escapePrivacyDelimiters(String data)
    {
        return STAFUtilEscapePrivacyDelimiters(data);
    }

    /************************/
    /* All the native stuff */
    /************************/

    private static native void initialize();
    private static native String STAFUtilAddPrivacyDelimiters(String data);
    private static native String STAFUtilRemovePrivacyDelimiters(
        String data, int numLevels);
    private static native String STAFUtilMaskPrivateData(String data);
    private static native String STAFUtilEscapePrivacyDelimiters(String data);

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
