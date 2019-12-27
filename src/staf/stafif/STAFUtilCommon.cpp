/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFUtil.h"
#include "STAFMutexSem.h"
#include <stdarg.h>
#include <stdlib.h>

static STAFMutexSem sPIDSem;

STAFProcessID_t STAFUtilGetPID()
{
    static STAFProcessID_t thePID = 0;

    if (thePID == 0)
    {
        STAFMutexSemLock lock(sPIDSem);

        if (thePID == 0) thePID = getpid();
    }

    return thePID;
}

inline unsigned int internalSwapUInt(unsigned int theUInt)
{
    unsigned int newInt = (theUInt & 0x000000ff) << 24;

    newInt |= (theUInt & 0x0000ff00) << 8;
    newInt |= (theUInt & 0x00ff0000) >> 8;
    newInt |= (theUInt & 0xff000000) >> 24;

    return newInt;
}


unsigned int STAFUtilSwapUInt(unsigned int theUInt)
{
    return internalSwapUInt(theUInt);
}


unsigned int STAFUtilConvertNativeUIntToLE(unsigned int theUInt)
{
    static unsigned short test = 0xdead;
    static unsigned char *lead = (unsigned char *)&test;
 
    // If big-endian platform swap, otherwise, just return
    return (*lead == 0xde) ? internalSwapUInt(theUInt) : theUInt;
}


unsigned int STAFUtilConvertLEUIntToNative(unsigned int theUInt)
{
    static unsigned short test = 0xdead;
    static unsigned char *lead = (unsigned char *)&test;
 
    // If big-endian platform swap, otherwise, just return
    return (*lead == 0xde) ? internalSwapUInt(theUInt) : theUInt;
}

unsigned int STAFUtilFormatString(STAFStringConst_t formatString,
                                  STAFString_t *outputString, ...)
{
    if (formatString == 0) return kSTAFInvalidParm;
    if (outputString == 0) return kSTAFInvalidParm;

    va_list theArgs;

    va_start(theArgs, outputString);

    return STAFUtilFormatString2(formatString, outputString, theArgs);
}


unsigned int STAFUtilFormatString2(STAFStringConst_t formatString,
                                   STAFString_t *outputString, va_list args)
{
    static STAFString sPercent(kUTF8_PERCENT);
    static STAFString sUIntChar("d");
    static STAFString sStringChar("s");
    static STAFString sCLCStringChar("C");

    if (formatString == 0) return kSTAFInvalidParm;
    if (outputString == 0) return kSTAFInvalidParm;

    STAFString format(formatString);
    STAFString output;
    unsigned int lastPos = 0;

    for (unsigned int currPos = format.find(sPercent);
         currPos != STAFString::kNPos; currPos = format.find(sPercent, lastPos))
    {
        output += format.subString(lastPos, currPos - lastPos);

        STAFString formatChar = format.subString(currPos + 1, 1,
                                                 STAFString::kChar);

        lastPos = currPos + 1 + formatChar.length();

        if (formatChar == sUIntChar)
            output += STAFString(va_arg(args, unsigned int));
        else if (formatChar == sStringChar)
            output += va_arg(args, STAFStringConst_t);
        else if (formatChar == sCLCStringChar)
            output += STAFHandle::wrapData(va_arg(args, STAFStringConst_t));
        else if (formatChar == sPercent)
            output += sPercent;
    }

    output += format.subString(lastPos);

    va_end(args);
    *outputString = output.adoptImpl();

    return kSTAFOk;
}


STAFRC_t STAFUtilStripPortFromEndpoint(STAFStringConst_t endpoint,
                                       STAFString_t *strippedEndpoint)
{
    if (endpoint == 0) return kSTAFInvalidParm;
    if (strippedEndpoint == 0) return kSTAFInvalidParm;

    // Strip the port from the endpoint, if present.

    STAFString endpointStr = STAFString(endpoint);

    unsigned int portIndex = endpointStr.find(kUTF8_AT);

    if (portIndex != STAFString::kNPos)
    {
        // If the characters following the "@" are numeric, then assume
        // it's a valid port and strip the @ and the port number from
        // the endpoint.

        STAFString port = endpointStr.subString(portIndex + 1);
        
        if (port.isDigits())
        {
            endpointStr = endpointStr.subString(0, portIndex);
        }
    }

    *strippedEndpoint = endpointStr.adoptImpl();

    return kSTAFOk;
}


STAFRC_t STAFUtilValidateTrust(unsigned int actualTrustLevel,
                               unsigned int requiredTrustLevel,
                               STAFStringConst_t service,
                               STAFStringConst_t request,
                               STAFStringConst_t localMachine,
                               STAFStringConst_t requestingEndpoint,
                               STAFStringConst_t physicalInterfaceID,
                               STAFStringConst_t requestingUser,
                               STAFString_t *errorBuffer)
{
    if (service == 0) return kSTAFInvalidParm;
    if (request == 0) return kSTAFInvalidParm;
    if (localMachine == 0) return kSTAFInvalidParm;
    if (requestingEndpoint == 0) return kSTAFInvalidParm;
    if (physicalInterfaceID == 0) return kSTAFInvalidParm;
    if (requestingUser == 0) return kSTAFInvalidParm;
    if (errorBuffer == 0) return kSTAFInvalidParm;

    if (actualTrustLevel < requiredTrustLevel)
    {
        // Strip the port from the machine's endpoint, if present.
        
        STAFString_t strippedEndpoint = 0;

        STAFUtilStripPortFromEndpoint(requestingEndpoint, &strippedEndpoint);
        
        *errorBuffer = STAFString(
            "Trust level " + STAFString(requiredTrustLevel) +
            " required for the " + service + " service's " + request +
            " request\nRequester has trust level " +
            STAFString(actualTrustLevel) + " on machine " + localMachine +
            "\nRequesting machine: " +
            STAFString(strippedEndpoint, STAFString::kShallow) +
            " (" + physicalInterfaceID + ")\nRequesting user   : " +
            requestingUser).adoptImpl();

        return kSTAFAccessDenied;
    }

    return kSTAFOk;
}


STAFRC_t STAFUtilConvertStringToUInt(STAFStringConst_t theString_t,
                                     STAFStringConst_t optionName_t,
                                     unsigned int *theUInt,
                                     STAFString_t *errorBuffer_t,
                                     unsigned int minValue,
                                     unsigned int maxValue)
{
    static STAFString asUIntErrorMsgFormat1(
        "The value for the %s option must be an unsigned integer in "
        "range %d to %d.  Invalid value: %s");

    static STAFString asUIntErrorMsgFormat2(
        "The value must be an unsigned integer in range %d to %d.  "
        "Invalid value: %s");

    if (theString_t == 0) return kSTAFInvalidParm;
    if (optionName_t == 0)  return kSTAFInvalidParm;
    if (errorBuffer_t == 0) return kSTAFInvalidParm;

    STAFString theString = STAFString(theString_t);
    STAFString optionName = STAFString(optionName_t);

    // Convert a string to an unsigned integer in the range of 0 to UINT_MAX
    // (e.g. 4294967295)

    bool asUIntError = false;

    try
    {
        *theUInt = theString.asUInt();
    }
    catch (...)
    {
        asUIntError = true;
    }
    
    if (asUIntError || (*theUInt < minValue) || (*theUInt > maxValue))
    {
        STAFString errorBuffer;

        if (optionName.length() != 0)
        {
            errorBuffer = STAFHandle::formatString(
                asUIntErrorMsgFormat1.getImpl(), optionName.getImpl(),
                minValue, maxValue, theString.getImpl());
        }
        else
        {
            errorBuffer = STAFHandle::formatString(
                asUIntErrorMsgFormat2.getImpl(),
                minValue, maxValue, theString.getImpl());
        }

        *errorBuffer_t = errorBuffer.adoptImpl();

        return kSTAFInvalidValue;
    }

    return kSTAFOk;
}


/* XXX: Commented out until get UINT64_MAX working on Solaris
STAFRC_t STAFUtilConvertStringToUInt64(STAFStringConst_t theString_t,
                                       STAFStringConst_t optionName_t,
                                       STAFUInt64_t *theUInt,
                                       STAFString_t *errorBuffer_t,
                                       STAFUInt64_t minValue,
                                       STAFUInt64_t maxValue)
{
    static STAFString asUIntErrorMsgFormat1(
        "The value for the %s option must be an unsigned integer in "
        "range %s to %s.  Invalid value: %s");

    static STAFString asUIntErrorMsgFormat2(
        "The value must be an unsigned integer in range %u to %u.  "
        "Invalid value: %s");

    if (theString_t == 0) return kSTAFInvalidParm;
    if (optionName_t == 0)  return kSTAFInvalidParm;
    if (errorBuffer_t == 0) return kSTAFInvalidParm;

    STAFString theString = STAFString(theString_t);
    STAFString optionName = STAFString(optionName_t);

    // Convert a string to an unsigned integer in the range of 0 to UINT64_MAX
    // (e.g. 18446744073709551615)

    bool asUIntError = false;

    try
    {
        *theUInt = theString.asUInt64();
    }
    catch (...)
    {
        asUIntError = true;
    }
    
    if (asUIntError || (*theUInt < minValue) || (*theUInt > maxValue))
    {
        STAFString errorBuffer;

        if (optionName.length() != 0)
        {
            errorBuffer = STAFHandle::formatString(
                asUIntErrorMsgFormat1.getImpl(), optionName.getImpl(),
                STAFString(minValue).getImpl(),
                STAFString(maxValue).getImpl(), theString.getImpl());
        }
        else
        {
            errorBuffer = STAFHandle::formatString(
                asUIntErrorMsgFormat2.getImpl(),
                STAFString(minValue).getImpl(),
                STAFString(maxValue).getImpl(), theString.getImpl());
        }
        
        *errorBuffer_t = errorBuffer.adoptImpl();

        return kSTAFInvalidValue;
    }

    return kSTAFOk;
}
*/


STAFRC_t STAFUtilConvertDurationString(STAFStringConst_t durationString,
                                       unsigned int *duration,
                                       STAFString_t *errorBuffer)
{
    static unsigned int sMILLISECOND = 1;
    static unsigned int sSECOND_IN_MS = 1000;
    static unsigned int sMINUTE_IN_MS = 60000;
    static unsigned int sHOUR_IN_MS = 3600000;
    static unsigned int sDAY_IN_MS = 24 * 3600000;
    static unsigned int sWEEK_IN_MS = 7 * 24 * 3600000;

    // Made the maximum 4294967294 because we use 4294967295 (UINT_MAX) to
    // indicate an indefinite wait on 32-bit machine.  For example:
    //   STAF_EVENT_SEM_INDEFINITE_WAIT = (unsigned int)-1
    static unsigned int sMAX_MILLISECONDS = 4294967294;
    static unsigned int sMAX_SECONDS = 4294967;
    static unsigned int sMAX_MINUTES = 71582;
    static unsigned int sMAX_HOURS = 1193;
    static unsigned int sMAX_DAYS = 49;
    static unsigned int sMAX_WEEKS = 7;

    if (durationString == 0) return kSTAFInvalidParm;
    if (errorBuffer == 0) return kSTAFInvalidParm;

    unsigned int rc = kSTAFOk;
    STAFString durationStr = STAFString(durationString);

    // Assume duration is specified in milliseconds if numeric
    unsigned int multiplier = 1;

    if (durationStr.length() > 0)
    {
        if (!durationStr.isDigits())
        {
            if (durationStr.length() > 1)
            {
                // Get duration type (last character of duration string)
                STAFString durationType = durationStr.subString(
                    durationStr.length() - 1, 1).toLowerCase();

                if (durationType == "s")
                    multiplier = sSECOND_IN_MS;
                else if (durationType == "m")
                    multiplier = sMINUTE_IN_MS;
                else if (durationType == "h")
                    multiplier = sHOUR_IN_MS;
                else if (durationType == "d")
                    multiplier = sDAY_IN_MS;
                else if (durationType == "w")
                    multiplier = sWEEK_IN_MS;
                else
                    rc = kSTAFInvalidValue;

                if (rc == kSTAFOk)
                {
                    // Assign to numeric duration value (all characters
                    // except the last one)
                    durationStr = durationStr.subString(
                        0, durationStr.length() - 1);
        
                    if (!durationStr.isDigits()) rc = kSTAFInvalidValue;
                }
            }
            else
            {
                rc = kSTAFInvalidValue;
            }
        }
    }
    else
    {
        rc = kSTAFInvalidValue;
    }

    if (rc != kSTAFOk)
    {
        *errorBuffer = STAFString(
            "This value may be expressed in milliseconds, seconds, minutes, "
            "hours, days, or weeks.  Its format is <Number>[s|m|h|d|w] "
            "where <Number> is an integer >= 0 and indicates milliseconds "
            "unless one of the following case-insensitive suffixes is "
            "specified:  s (for seconds), m (for minutes), h (for hours), "
            "d (for days), or w (for weeks).  The calculated value cannot "
            "exceed 4294967294 milliseconds.\n\nExamples: \n"
            "  100 specifies 100 milliseconds, \n"
            "  10s specifies 10 seconds, \n"
            "  5m specifies 5 minutes, \n"
            "  2h specifies 2 hours, \n"
            "  1d specifies 1 day, \n"
            "  1w specifies 1 week.").adoptImpl();
        return rc;
    }
    
    // Convert duration from a string to an unsigned integer in range
    // 0 to 04294967294

    try
    {
        *duration = durationStr.asUInt();
    }
    catch (STAFException)
    {
        rc = kSTAFInvalidValue;
    }
            
    if (rc == kSTAFOk)
    {
        // Because the maximum duration in milliseconds cannot exceed
        // 4294967294:
        // - Duration specified in seconds cannot exceed 4294967 seconds
        // - Duration specified in minutes cannot exceed 71582 minutes
        // - Duration specified in hours cannot exceed 1193 hours
        // - Duration specified in days cannot exceed 49 days
        // - Duration specified in weeks cannot exceed 7 weeks

        if (((multiplier == sMILLISECOND) && (*duration > sMAX_MILLISECONDS)) ||
            ((multiplier == sSECOND_IN_MS) && (*duration > sMAX_SECONDS)) ||
            ((multiplier == sMINUTE_IN_MS) && (*duration > sMAX_MINUTES)) ||
            ((multiplier == sHOUR_IN_MS) && (*duration > sMAX_HOURS)) ||
            ((multiplier == sDAY_IN_MS) && (*duration > sMAX_DAYS)) ||
            ((multiplier == sWEEK_IN_MS) && (*duration > sMAX_WEEKS)))
        {
            rc = kSTAFInvalidValue;
        }
            
        if (rc == kSTAFOk)
        {
            *duration = *duration * multiplier;
            return kSTAFOk;
        }
    }
    
    if (multiplier == sMILLISECOND)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_MILLISECONDS +
            " milliseconds.").adoptImpl();
    }
    else if (multiplier == sSECOND_IN_MS)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_SECONDS +
            " seconds.").adoptImpl();;
    }
    else if (multiplier == sMINUTE_IN_MS)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_MINUTES +
            " minutes.").adoptImpl();
    }
    else if (multiplier == sHOUR_IN_MS)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_HOURS +
            " hours.").adoptImpl();
    }
    else if (multiplier == sDAY_IN_MS)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_DAYS +
            " days.").adoptImpl();
    }
    else if (multiplier == sWEEK_IN_MS)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_WEEKS +
            " weeks.").adoptImpl();
    }
    
    return rc;
}


STAFRC_t STAFUtilConvertSizeString(STAFStringConst_t sizeString,
                                   unsigned int *size,
                                   STAFString_t *errorBuffer)
{
    static unsigned int sBYTES = 1;
    static unsigned int sBYTES_IN_KILOBYTES = 1024;
    static unsigned int sBYTES_IN_MEGABYTES = 1048576;

    // Made the maximum 4294967295 because that's the maximum size of
    // UINT_MAX on 32-bit machines

    static unsigned int sMAX_BYTES = 4294967295;
    static unsigned int sMAX_KILOBYTES = 4194303;
    static unsigned int sMAX_MEGABYTES = 4095;

    if (sizeString == 0) return kSTAFInvalidParm;
    if (errorBuffer == 0) return kSTAFInvalidParm;

    unsigned int rc = kSTAFOk;
    STAFString sizeStr = STAFString(sizeString);

    // Assume size is specified in bytes if numeric
    unsigned int multiplier = sBYTES;

    if (sizeStr.length() > 0)
    {
        if (!sizeStr.isDigits())
        {
            unsigned int strLength = sizeStr.length(STAFString::kChar);

            if (strLength > 1)
            {
                // Get the size type (last character of size string)
                STAFString sizeType = sizeStr.subString(
                    strLength - 1, 1, STAFString::kChar).toLowerCase();

                if (sizeType == "k")
                    multiplier = sBYTES_IN_KILOBYTES;
                else if (sizeType == "m")
                    multiplier = sBYTES_IN_MEGABYTES;
                else
                    rc = kSTAFInvalidValue;

                if (rc == kSTAFOk)
                {
                    // Assign to numeric size value (all characters
                    // except the last one)
                    sizeStr = sizeStr.subString(
                        0, strLength - 1, STAFString::kChar);
        
                    if (!sizeStr.isDigits())
                        rc = kSTAFInvalidValue;
                }
            }
            else
            {
                rc = kSTAFInvalidValue;
            }
        }
    }
    else
    {
        rc = kSTAFInvalidValue;
    }

    if (rc != kSTAFOk)
    {
        *errorBuffer = STAFString(
            "This value may be expressed in bytes, kilobytes, or megabytes."
            "  Its format is <Number>[k|m] "
            "where <Number> is an integer >= 0 and indicates bytes "
            "unless one of the following case-insensitive suffixes is "
            "specified:  k (for kilobytes) or m (for megabytes).  "
            "The calculated value cannot exceed 4294967295 bytes.\n\n"
            "Examples: \n"
            "  100000 specifies 100,000 bytes, \n"
            "  500k specifies 500 kilobytes (or 512,000 bytes), \n"
            "  5m specifies 5 megabytes (or 5,242,880 bytes), \n"
            "  0 specifies no maximum size limit").adoptImpl();
        return rc;
    }
        
    // Convert size from a string to an unsigned integer in the range of
    // 0 to 4294967295

    try
    {
        *size = sizeStr.asUInt();
    }
    catch (STAFException)
    {
        rc = kSTAFInvalidValue;
    }
            
    if (rc == kSTAFOk)
    {
        // Because the maximum size in bytes cannot exceed 4294967295:
        // - Size specified in kilobytes cannot exceed 4194303 kilobytes
        // - Size specified in megabytes cannot exceed 4095 megabytes

        if (multiplier == sBYTES)
        {
            // No additional maximum size checks needed
        }
        else
        if (((multiplier == sBYTES_IN_KILOBYTES) && (*size > sMAX_KILOBYTES)) ||
            ((multiplier == sBYTES_IN_MEGABYTES) && (*size > sMAX_MEGABYTES)))
        {
            rc = kSTAFInvalidValue;
        }
            
        if (rc == kSTAFOk)
        {
            *size = *size * multiplier;
            return kSTAFOk;
        }
    }
    
    if (multiplier == sBYTES)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_BYTES +
            " bytes.").adoptImpl();
    }
    else if (multiplier == sBYTES_IN_KILOBYTES)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_KILOBYTES +
            " kilobytes.").adoptImpl();;
    }
    else if (multiplier == sBYTES_IN_MEGABYTES)
    {
        *errorBuffer = STAFString(
            STAFString("Cannot exceed ") + sMAX_MEGABYTES +
            " megabytes.").adoptImpl();
    }
    
    return rc;
}

