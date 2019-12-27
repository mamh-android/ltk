/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <stdlib.h>
#include "STAFConnectionProvider.h"
#include "STAFString.h"
#include "STAFException.h"
#include "STAFUtil.h"
#include "STAFMutexSem.h"

static const STAFString sBang(kUTF8_BANG);
static const STAFString sAt(kUTF8_AT);
static const STAFString sCaret(kUTF8_CARET);

// Opening Privacy Delimiter, !!@
static const STAFString sOpenPD(sBang + sBang + sAt);

// Closing Privacy Delimiter, @!!
static const STAFString sClosePD(sAt + sBang + sBang);

// Escaped Opening Privacy Delimiter, ^!!@
static const STAFString sEscOpenPD(sCaret + sOpenPD);

// Escaped Closing Privacy Delimiter, ^@!!
static const STAFString sEscClosePD(sCaret + sClosePD);

// Our callers may be in a different runtime environment so, make sure we
// register our exception handlers

// #pragma handler(STAFRegister(char *, STAFHandle_t *))
// #pragma handler(STAFRegisterUTF8(char *, STAFHandle_t *))
// #pragma handler(STAFUnRegister(STAFHandle_t))
// #pragma handler(STAFSubmit(STAFHandle_t, char *, char *, char *, unsigned int,\
//                            char **, unsigned int *))
// #pragma handler(STAFSubmitUTF8(STAFHandle_t, char *, char *, char *,\
//                                unsigned int, char **, unsigned int *))
// #pragma handler(STAFFree(STAFHandle_t, char *))

// These are real hacks to get around a bug in the VAC++ compiler dealing
// with exceptions being thrown from a function that has a #pragma handler
//
// So, we basically call these functions to perform the real work for the
// intended APIs

static STAFRC_t RealSTAFRegister(char *processName, STAFHandle_t *handle);
static STAFRC_t RealSTAFRegisterUTF8(char *processName, STAFHandle_t *handle);
static STAFRC_t RealSTAFUnRegister(STAFHandle_t handle);
static STAFRC_t RealSTAFSubmit(STAFHandle_t handle, 
                               STAFSyncOption_t syncOption,
                               char *where, char *service, char *request,
                               unsigned int requestLength, char **resultPtr,
                               unsigned int *resultLength);
static STAFRC_t RealSTAFSubmitUTF8(STAFHandle_t handle, 
                                   STAFSyncOption_t syncOption,
                                   const char *where, const char *service,
                                   const char *request,
                                   unsigned int requestLength,
                                   char **resultPtr,
                                   unsigned int *resultLength);
static STAFRC_t RealSTAFFree(STAFHandle_t handle, char *result);
static void createCResult(STAFString result, char **resultPtr,
                          unsigned int *resultLength);
static unsigned int findNextUnescapedOpeningPD(const STAFString &data,
                                               unsigned int index);
static unsigned int findNextUnescapedClosingPD(const STAFString &data,
                                               unsigned int index);

STAFRC_t makeConnection(STAFConnectionPtr &connection, STAFString &errorBuffer)
{
    try
    {
        static STAFMutexSem sConnProvSem;
        static STAFConnectionProvider *sConnProv = 0;
        static bool sConnProvIsInited = false;
        static STAFString endpoint = "local";

        if (sConnProvIsInited == false)
        {
            STAFMutexSemLock lock(sConnProvSem);

            if (sConnProvIsInited == false)
            {
                STAFConnectionProviderConstructInfoLevel1 constructInfo =
                { kSTAFConnectionProviderOutbound, 0 };

                constructInfo.mode = kSTAFConnectionProviderOutbound;

                sConnProv = STAFConnectionProvider::create("local", "STAFLIPC",
                                                           &constructInfo, 1);
                sConnProvIsInited = true;
            }
        }

        connection = sConnProv->connect(endpoint);

        return kSTAFOk;
    }
    catch (STAFConnectionProviderException &se)
    {
        errorBuffer = se.getText();
        return kSTAFNotRunning;
    }
    catch (STAFException &se)
    {
        errorBuffer = se.getText();
        return se.getErrorCode();
    }
    catch (...)
    {
        errorBuffer = "Caught unknown exception in makeConnection()";
    }

    return kSTAFUnknownError;
}


STAFRC_t STAFRegister(char *processName, STAFHandle_t *handle)
{
    return RealSTAFRegister(processName, handle);
}

STAFRC_t RealSTAFRegister(char *processName, STAFHandle_t *handle)
{
    STAFRC_t rc = kSTAFRegistrationError;
    STAFString errorBuffer;

    try
    {
        STAFConnectionPtr connection;

        rc = makeConnection(connection, errorBuffer);

        if (rc != kSTAFOk)
        {
            if (rc == kSTAFNotRunning)
            {
                // Added to be able to provide more information on why getting
                // RC 21 (STAF Not Running) when submitting a request using
                // the local IPC interface
                // XXX: If we changed RealSTAFRegister and STAFRegisterUTF8 to
                //      provide the errorBuffer, then this information would
                //      be provided in the result and we could remove the need
                //      for the STAF_DEBUG_RC_21 environment variable.

                if (getenv("STAF_DEBUG_RC_21") != NULL)
                    cout << errorBuffer << endl;
            }

            return rc;
        }

        connection->writeUInt(2);     // API Number
        connection->writeUInt(0);     // API Level

        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());

        if (ack != kSTAFOk) return ack;

        connection->writeUInt(STAFUtilGetPID());
        connection->writeString(processName);

        rc = (STAFRC_t)connection->readUInt();

        connection->readUInt(*handle);
    }
    catch (STAFConnectionProviderConnectException)
    {
        rc = kSTAFNotRunning;
    }
    catch (STAFConnectionIOException)
    {
        rc = kSTAFCommunicationError;
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}


STAFRC_t STAFRegisterUTF8(char *processName, STAFHandle_t *handle)
{
    return RealSTAFRegisterUTF8(processName, handle);
}

STAFRC_t RealSTAFRegisterUTF8(char *processName, STAFHandle_t *handle)
{
    STAFRC_t rc = kSTAFRegistrationError;
    STAFString errorBuffer;

    try
    {
        STAFConnectionPtr connection;

        rc = makeConnection(connection, errorBuffer);

        if (rc != kSTAFOk)
        {
            if (rc == kSTAFNotRunning)
            {
                // Added to be able to provide more information on why getting
                // RC 21 (STAF Not Running) when submitting a request using
                // the local IPC interface
                // XXX: If we changed RealSTAFRegister and STAFRegisterUTF8 to
                //      provide the errorBuffer, then this information would
                //      be provided in the result and we could remove the need
                //      for the STAF_DEBUG_RC_21 environment variable.

                if (getenv("STAF_DEBUG_RC_21") != NULL)
                    cout << errorBuffer << endl;
            }

            return rc;
        }

        connection->writeUInt(2);     // API Number
        connection->writeUInt(0);     // API Level

        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());

        if (ack != kSTAFOk) return ack;

        connection->writeUInt(STAFUtilGetPID());

        unsigned int processNameLength = strlen(processName);
        connection->writeUInt(processNameLength);
        connection->write(processName, processNameLength);

        rc = (STAFRC_t)connection->readUInt();

        connection->readUInt(*handle);
    }
    catch (STAFConnectionProviderConnectException)
    {
        rc = kSTAFNotRunning;
    }
    catch (STAFConnectionIOException)
    {
        rc = kSTAFCommunicationError;
    }
    catch (STAFException &se)
    {
        rc = se.getErrorCode();
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}


STAFRC_t STAFUnRegister(STAFHandle_t handle)
{
    return RealSTAFUnRegister(handle);
}

STAFRC_t RealSTAFUnRegister(STAFHandle_t handle)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        STAFConnectionPtr connection;
        STAFString errorBuffer;

        rc = makeConnection(connection, errorBuffer);

        if (rc != kSTAFOk) return rc;

        connection->writeUInt(3);     // API Number
        connection->writeUInt(0);     // API Level

        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());

        if (ack != kSTAFOk) return ack;

        connection->writeUInt(STAFUtilGetPID());
        connection->writeUInt(handle);

        rc = (STAFRC_t)connection->readUInt();
    }
    catch (STAFConnectionProviderConnectException)
    {
        rc = kSTAFNotRunning;
    }
    catch (STAFConnectionIOException)
    {
        rc = kSTAFCommunicationError;
    }
    catch (STAFException &se)
    {
        rc = se.getErrorCode();
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}


STAFRC_t STAFSubmit(STAFHandle_t handle, char *where, char *service,
                    char *request, unsigned int requestLength,
                    char **resultPtr, unsigned int *resultLength)
{
    return STAFSubmit2(handle, kSTAFReqSync, where, service, request, 
                       requestLength, resultPtr, resultLength);
}

STAFRC_t STAFSubmit2(STAFHandle_t handle, STAFSyncOption_t syncOption,
                     char *where, char *service,
                     char *request, unsigned int requestLength,
                     char **resultPtr, unsigned int *resultLength)
{
    return RealSTAFSubmit(handle, syncOption, where, service, request, 
                          requestLength, resultPtr, resultLength);
}


STAFRC_t RealSTAFSubmit(STAFHandle_t handle, STAFSyncOption_t syncOption,
                        char *where, char *service,
                        char *request, unsigned int requestLength,
                        char **resultPtr, unsigned int *resultLength)
{
    STAFRC_t rc = kSTAFUnknownError;
    *resultLength = 0;
    *resultPtr = 0;
    STAFString response;
    char *theResultPtr = 0;

    try
    {
        // Basically, we just piggy back on top of the UTF-8 version of
        // STAFSubmit

        STAFString whereUTF8(where);
        STAFString serviceUTF8(service);
        STAFString requestUTF8(request, requestLength);

        // Note: We don't need to add a null to requestUTF8 since we pass it's
        //       length to RealSTAFSubmitUTF8

        whereUTF8 += kUTF8_NULL;
        serviceUTF8 += kUTF8_NULL;

        unsigned int theResultLength = 0;
        rc = RealSTAFSubmitUTF8(handle, syncOption, whereUTF8.buffer(),
                                serviceUTF8.buffer(), requestUTF8.buffer(),
                                requestUTF8.length(), &theResultPtr,
                                &theResultLength);

        response = STAFString(theResultPtr, theResultLength, STAFString::kUTF8);
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    createCResult(response, resultPtr, resultLength);

    if (theResultPtr != 0)
    {
        // Free the buffer pointed to by theResultPtr to prevent a memory leak
        STAFFree(handle, theResultPtr);
    }

    return rc;
}


STAFRC_t STAFSubmitUTF8(STAFHandle_t handle, char *where,
                        char *service, char *request,
                        unsigned int requestLength,
                        char **resultPtr,
                        unsigned int *resultLength)
{
    return STAFSubmit2UTF8(handle, kSTAFReqSync, where, service, request,
                           requestLength, resultPtr, resultLength);
}

STAFRC_t STAFSubmit2UTF8(STAFHandle_t handle, STAFSyncOption_t syncOption,
                         char *where, char *service, char *request,
                         unsigned int requestLength,
                         char **resultPtr,
                         unsigned int *resultLength)
{
    return RealSTAFSubmitUTF8(handle, syncOption, where, service, request, 
                              requestLength, resultPtr, resultLength);
}


STAFRC_t RealSTAFSubmitUTF8(STAFHandle_t handle, STAFSyncOption_t syncOption,
                            const char *where, const char *service,
                            const char *request, unsigned int requestLength,
                            char **resultPtr, unsigned int *resultLength)

{
    if ((syncOption != kSTAFReqSync) && (syncOption != kSTAFReqQueue) &&
        (syncOption != kSTAFReqRetain) && 
        (syncOption != kSTAFReqQueueRetain) &&
        (syncOption != kSTAFReqFireAndForget))
    {
        return kSTAFInvalidAsynchOption;
    }

    STAFRC_t rc = kSTAFUnknownError;
    *resultLength = 0;
    *resultPtr = 0;
    STAFString response;
    char *buffer2 = 0;

    // Note: I tried using a stack buffer of various sizes (64, 128, and 256
    //       bytes, to try to avoid doing most memory allocation, but that
    //       actually made performance worse.

    try
    {
        STAFConnectionPtr connection;
        STAFString errorBuffer;

        rc = makeConnection(connection, errorBuffer);

        if (rc != kSTAFOk)
        {
            *resultLength = errorBuffer.length();

            if (*resultLength != 0)
            {
                *resultPtr = new char[*resultLength + 1];
                (*resultPtr)[*resultLength] = 0;
                memcpy(*resultPtr, errorBuffer.buffer(), *resultLength);
            }

            return rc;
        }

        unsigned int whereLength = strlen(where);
        unsigned int serviceLength = strlen(service);
        unsigned int buffer[2];

        buffer[0] = 0;                                 // API Number
        buffer[1] = STAFUtilConvertNativeUIntToLE(2);  // API Level

        connection->write(buffer, sizeof(buffer));

        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());

        if (ack != kSTAFOk) return ack;

        unsigned int buffSize = (6 * sizeof(unsigned int)) + whereLength +
                                serviceLength + requestLength;

        buffer2 = new char[buffSize];
        int *buffer2int = reinterpret_cast<int *>(buffer2);

        buffer2int[0] = STAFUtilConvertNativeUIntToLE(syncOption);
        buffer2int[1] = STAFUtilConvertNativeUIntToLE(STAFUtilGetPID());
        buffer2int[2] = STAFUtilConvertNativeUIntToLE(handle);
        buffer2int[3] = STAFUtilConvertNativeUIntToLE(whereLength);
        buffer2int[4] = STAFUtilConvertNativeUIntToLE(serviceLength);
        buffer2int[5] = STAFUtilConvertNativeUIntToLE(requestLength);

        char *buffer2offset = buffer2 + (6 * sizeof(unsigned int));

        memcpy(buffer2offset, where, whereLength);
        buffer2offset += whereLength;
        memcpy(buffer2offset, service, serviceLength);
        buffer2offset += serviceLength;
        memcpy(buffer2offset, request, requestLength);

        connection->write(buffer2, buffSize);

        rc = static_cast<STAFRC_t>(connection->readUInt());

        *resultLength = connection->readUInt();

        if (*resultLength != 0)
        {
            *resultPtr = new char[*resultLength + 1];
            (*resultPtr)[*resultLength] = 0;
            connection->read(*resultPtr, *resultLength);
        }
    }
    catch (STAFConnectionProviderConnectException &se)
    {
        rc = kSTAFNotRunning;
        response = STAFString(se.getText());
        response += STAFString(": ");
        response += STAFString(se.getErrorCode());
    }
    catch (STAFConnectionIOException &se)
    {
        rc = kSTAFCommunicationError;
        response = STAFString(se.getText());
        response += STAFString(": ");
        response += STAFString(se.getErrorCode());
    }
    catch (STAFException &se)
    {
        rc = se.getErrorCode();
        response = STAFString(se.getText());
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
        response = STAFString("Unknown Error");
    }

    if (response.length() != 0)
    {
        // We must have thrown an exception

        try
        {
            // Free memory if we had already allocated it
            if (*resultPtr != 0) delete [] *resultPtr;

            *resultLength = response.length();

            if (*resultLength != 0)
            {
                *resultPtr = new char[*resultLength + 1];
                memcpy(*resultPtr, response.buffer(), *resultLength);
                (*resultPtr)[*resultLength] = 0;
            }
        }
        catch (...)
        {
            delete [] *resultPtr;
            *resultLength = 0;
        }
    }

    delete [] buffer2;

    return rc;
}


STAFRC_t STAFFree(STAFHandle_t handle, char *result)
{
    return RealSTAFFree(handle, result);
}

STAFRC_t RealSTAFFree(STAFHandle_t, char *result)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        delete [] result;
        rc = kSTAFOk;
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}

void createCResult(STAFString result, char **resultPtr,
                   unsigned int *resultLength)
{
    *resultPtr = 0;
    *resultLength = 0;

    try
    {
        // We use a temporary to hold the result so that it will not
        // be set if we throw an exception allocating memory

        STAFStringBufferPtr theResult = result.toCurrentCodePage();
        unsigned int theResultLength = theResult->length();

        if (theResultLength != 0)
        {
            *resultPtr = new char[theResultLength + 1];
            memcpy(*resultPtr, theResult->buffer(), theResultLength);
            (*resultPtr)[theResultLength] = 0;
            *resultLength = theResultLength;
        }
    }
    catch (...)
    {
    }
}

STAFRC_t STAFAddPrivacyDelimiters(STAFStringConst_t inData,
                                  STAFString_t *result)
{
    if (inData == 0) return kSTAFInvalidObject;

    STAFRC_t rc = kSTAFOk;
    
    try
    {
        STAFString data(inData);

        if (data.length() == 0)
        {
            *result = data.adoptImpl();
            return rc;
        }

        // Check if data already has privacy delimiters at beginning and end

        if (data.find(sOpenPD) == 0)
        {
            int index = data.length() - sClosePD.length();

            if (index >= sOpenPD.length())
            {
                if ((data.subString(index) == sClosePD) &&
                    (data.subString(index - 1) != sEscClosePD))
                {
                    // Don't add additional privacy delimiters.
                    *result = data.adoptImpl();
                    return rc;
                }
            }
        }

        // 1) Add an opening privacy delimiter, !!@, to the beginning of the
        //    data.
        //
        // 2) Escape all occurrences of the opening and closing privacy
        //    delimiters.  That is, replace all occurrences of !!@ with
        //    ^!!@ and replace all occurrences of @!! with ^@!!.
        //
        //    Note:  Escape the closing privacy delimiters before escaping the
        //    opening privacy delimiters to avoid a problem if the data
        //    includes back to back closing privacy delimiters, ^@!!@!!, which
        //    could be mistaken for an opening privacy delimiter, !!@, if the
        //    closing privacy delimiters were not escaped first.
        //
        // 3) Add a closing privacy delimiter, @!!, to the end of the data.

        data = STAFString(sOpenPD +
                          (data.replace(sClosePD, sEscClosePD)).
                           replace(sOpenPD, sEscOpenPD) +
                          sClosePD);
        
        *result = data.adoptImpl();
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}

STAFRC_t STAFRemovePrivacyDelimiters(STAFStringConst_t data,
                                     unsigned int numLevels,
                                     STAFString_t *outString)
{
    if (data == 0) return kSTAFInvalidObject;

    STAFRC_t rc = kSTAFOk;
    
    try
    {
        STAFString result(data);
        
        if ((result.length() == 0) ||
            (result.find(sOpenPD) == STAFString::kNPos))
        {
            *outString = result.adoptImpl();
            return rc;
        }

        unsigned int index = 0;
        unsigned int nextIndex = 0;
        unsigned int openIndex = 0;
        unsigned int closeIndex = 0;
        unsigned int level = 0;
        
        // numLevels = 0 means to remove all privacy masks.
        // numLevels = 1+ means to remove up to the specified number of levels
        //             of privacy data.

        bool noMoreLevels = false;

        if (numLevels == 0)
            noMoreLevels = true;

        // Remove privacy delimiters for the specified number of levels

        for (; ((numLevels == 0) || (level < numLevels)); ++level)
        {
            // Check if any more unescaped opening privacy delimiters.

            nextIndex = 0;
            
            openIndex = findNextUnescapedOpeningPD(result, nextIndex);

            // If no more unescaped opening privacy delimiters, break out of
            // the loop since no more levels of private data

            if (openIndex == STAFString::kNPos)
            {
                noMoreLevels = true;
                break;
            }

            // Check if any more unescaped closing privacy delimiters after
            // the position of the opening unescaped privacy delimiter.
            
            nextIndex = openIndex + sOpenPD.length();
            
            closeIndex = findNextUnescapedClosingPD(result, nextIndex);

            // If no more unescaped closing privacy delimiters, break out of
            // the loop since no more levels of private data

            if (closeIndex == STAFString::kNPos)
            {
                noMoreLevels = true;
                break;
            }

            // Handle all opening and closing privacy delimiters at this level

            while (true)
            {
                // If there are any escaped privacy delimiters between this
                // opening privacy delimiter and the closing privacy delimiter,
                // remove the escape character (^) from them.

                unsigned int nextOpenIndex = result.find(
                   sEscOpenPD, openIndex + sOpenPD.length());

                for (; ((nextOpenIndex != STAFString::kNPos) &&
                        (nextOpenIndex < closeIndex));)
                {
                    result = result.subString(0, nextOpenIndex) +
                             result.subString(nextOpenIndex + 1);

                    closeIndex--;

                    nextOpenIndex = result.find(
                        sEscOpenPD, nextOpenIndex + sOpenPD.length());
                }

                unsigned int nextCloseIndex = result.find(
                    sEscClosePD, openIndex + sOpenPD.length());

                for (; ((nextCloseIndex != STAFString::kNPos) &&
                        (nextCloseIndex < closeIndex));)
                {
                    result = result.subString(0, nextCloseIndex) +
                        result.subString(nextCloseIndex + 1);

                    closeIndex--;

                    nextCloseIndex = result.find(
                        sEscClosePD, nextCloseIndex + sClosePD.length());
                }

                // Remove these opening and closing privacy delimiters

                unsigned int beginPos = openIndex + sOpenPD.length();

                if (openIndex == 0)
                {
                    result = result.subString(beginPos, closeIndex - beginPos) +
                             result.subString(closeIndex + sClosePD.length());
                }
                else
                {
                    result = result.subString(0, openIndex) +
                             result.subString(beginPos, closeIndex - beginPos) +
                             result.subString(closeIndex + sClosePD.length());
                }
                
                // Check if any more unescaped opening privacy delimiters after
                // this closing delimiter.

                nextIndex = closeIndex;

                openIndex = findNextUnescapedOpeningPD(result, nextIndex);

                if (openIndex == STAFString::kNPos)
                {
                    break;
                }

                // Check if any more unescaped closing privacy delimiters after
                // the position of the opening unescaped privacy delimiter.

                nextIndex = openIndex + sOpenPD.length();

                closeIndex = findNextUnescapedClosingPD(result, nextIndex);

                if (closeIndex == STAFString::kNPos)
                {
                    break;
                }
            }
        }

        // Check if there are any more levels of privacy delimiters

        if (!noMoreLevels)
        {
            // Check if any more unescaped opening privacy delimiters.

            nextIndex = 0;

            openIndex = findNextUnescapedOpeningPD(result, nextIndex);

            if (openIndex == STAFString::kNPos)
            {
                noMoreLevels = true;
            }
            else
            {
                // Check if any more unescaped closing privacy delimiters after
                // the position of the opening unescaped privacy delimiter.

                nextIndex = openIndex + sOpenPD.length();

                closeIndex = findNextUnescapedClosingPD(result, nextIndex);

                if (closeIndex == STAFString::kNPos)
                {
                    noMoreLevels = true;
                }
            }
        }
        
        if (noMoreLevels)
        {
            // Replace any escaped closing privacy delimiters, ^@!!, with
            // unescaped closing privacy delimiters, @!!.
            result = result.replace(sEscClosePD, sClosePD);

            // Replace any escaped opening privacy delimiters, ^!!@, with
            // unescaped opening privacy delimiters, !!@.
            result = result.replace(sEscOpenPD, sOpenPD);
        }

        *outString = result.adoptImpl();
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}

STAFRC_t STAFMaskPrivateData(STAFStringConst_t inData, STAFString_t *result)
{
    if (inData == 0) return kSTAFInvalidObject;

    STAFRC_t rc = kSTAFOk;
    
    try
    {
        STAFString data(inData);
        
        if ((data.length() == 0) ||
            (data.find(sOpenPD) == STAFString::kNPos))
        {
            *result = data.adoptImpl();
            return rc;
        }

        STAFString outString("");
        unsigned int index = 0;
        unsigned int openIndex = 0;
        unsigned int closeIndex = 0;

        // Find all unescaped opening privacy delimiters with matching 
        // unescaped closing privacy delimiters and replace all data between 
        // these privacy delimiters with asterisks.

        while ((openIndex = data.find(sOpenPD, closeIndex)) !=
               STAFString::kNPos)
        {
            if ((openIndex > 0) && (data.sizeOfChar(openIndex - 1) == 1) &&
                (data.subString(openIndex - 1, 1) == sCaret))
            {
                // Found an escaped privacy delimiter
                closeIndex = openIndex + sOpenPD.length();
                continue;
            }

            // Find position of first unescaped closing privacy delimiter
            // after this unescaped opening delimiter.

            closeIndex = data.find(sClosePD, openIndex + sOpenPD.length());

            while (closeIndex != STAFString::kNPos)
            {
                if ((data.sizeOfChar(closeIndex - 1) == 1) &&
                    (data.subString(closeIndex - 1, 1) == sCaret))
                {
                    // Escaped; Find next unescaped closing privacy delimiter

                    closeIndex = data.find(
                        sClosePD, closeIndex + sClosePD.length()); 
                }
                else
                {
                    break;  // Found unescaped closing privacy delimiter
                }
            }

            if (closeIndex == STAFString::kNPos)
            {
                break;  // Exit since no unescaped closing @!! strings left
            }

            outString += data.subString(index, openIndex - index);

            // Replace the privacy delimiters and data with "*"s

            unsigned int replaceLength = closeIndex - openIndex +
                sClosePD.length();

            for (unsigned int i = 1; i <= replaceLength; i++)
            {
                outString += "*";
            }

            index = closeIndex + sClosePD.length();

            if (index >= data.length()) break;
        }

        if (index < data.length())
        {
            outString += data.subString(index);
        }

        *result = outString.adoptImpl();
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}

STAFRC_t STAFEscapePrivacyDelimiters(STAFStringConst_t inData,
                                     STAFString_t *result)
{
    if (inData == 0) return kSTAFInvalidObject;

    STAFRC_t rc = kSTAFOk;
    
    try
    {
        STAFString data(inData);
        
        if (data.length() != 0)
        {
            // Escape all opening or closing delimiters.

            data = data.replace(sClosePD, sEscClosePD);
            data = data.replace(sOpenPD, sEscOpenPD);
        }

        *result = data.adoptImpl();
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
    }

    return rc;
}


// Returns the index of the next unescaped opening privacy delimiter
// found at or after the specified index.  If no more unescaped opening
// privacy delimiters are found, returns STAFString::kNPos.
//
// Input:
//   data  - A string that may contain opening privacy delimiters
//   index - The index to begin searching the data for an unescaped
//           opening privacy delimiter

unsigned int findNextUnescapedOpeningPD(const STAFString &data, unsigned int index)
{
    unsigned int i;
    
    while ((i = data.find(sOpenPD, index)) != STAFString::kNPos)
    {
        if ((i > 0) && (data.sizeOfChar(i - 1) == 1) &&
            (data.subString(i - 1, 1) == sCaret))
        {
            index = i + sOpenPD.length();
        }
        else
        {
            break;
        }
    }

    return i;
}


// Returns the index of the next unescaped closing privacy delimiter
// found at or after the specified index.  If no more unescaped closing
// privacy delimiters are found, returns STAFString::kNPos.
//
// Input:
//   data  - A string that may contain closing privacy delimiters
//   index - The index to begin searching the data for an unescaped
//           closing privacy delimiter

unsigned int findNextUnescapedClosingPD(const STAFString &data, unsigned int index)
{
    unsigned int i;
    
    while ((i = data.find(sClosePD, index)) != STAFString::kNPos)
    {
        if ((i > 0) && (data.sizeOfChar(i - 1) == 1) &&
            (data.subString(i - 1, 1) == sCaret))
        {
            index = i + sClosePD.length();
        }
        else
        {
            break;
        }
    }

    return i;
}
