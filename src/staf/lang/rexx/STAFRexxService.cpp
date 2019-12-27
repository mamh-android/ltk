/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_fstream.h"
#include <string>
#include "STAFServiceInterface.h"
#include "STAFRefPtr.h"
#include "STAFUtil.h"
#include "STAF_rexx.h"

struct RexxServiceData
{
    unsigned int fDebugMode;
    unsigned int fInterfaceLevel;
    STAFString fCommand;
    STAFString fName;
    std::string fTokenImage;
};

typedef STAFRefPtr<RexxServiceData> RexxServiceDataPtr;

static unsigned int callRexx(RexxServiceData *pData, std::string function,
                             unsigned int numParms, RXSTRING *parms,
                             STAFString &result);


unsigned int STAFServiceGetLevelBounds(unsigned int levelID,
                                       unsigned int *minimum,
                                       unsigned int *maximum)
{
    switch (levelID)
    {
        case kServiceInfo:
        case kServiceInit:
        case kServiceAcceptRequest:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceTerm:
        case kServiceDestruct:
        {
            *minimum = 0;
            *maximum = 0;
            break;
        }
        default:
        {
            return kSTAFInvalidAPILevel;
        }
    }

    return 0;
}

unsigned int STAFServiceConstruct(STAFServiceHandle_t *pServiceHandle,
                                  void *pServiceInfo, unsigned int infoLevel,
                                  STAFString_t *pErrorBuffer)
{

    try
    {
        if (infoLevel != 30) return kSTAFInvalidAPILevel;

        STAFServiceInfoLevel30 *pInfo =
            reinterpret_cast<STAFServiceInfoLevel30 *>(pServiceInfo);
        RexxServiceData data;

        data.fDebugMode = 0;
        data.fCommand = pInfo->exec;
        data.fName = pInfo->name;
        data.fInterfaceLevel = 0;

        // Walk through and verify the config options

        for (unsigned int i = 0; i < pInfo->numOptions; ++i)
        {
            if (STAFString(pInfo->pOptionName[i]).upperCase() == "DEBUG")
            {
                data.fDebugMode = 1;
            }
            else
            {
                STAFString optionError(pInfo->pOptionName[i]);
                *pErrorBuffer = optionError.adoptImpl();
                return kSTAFServiceConfigurationError;
            }
        }

        // Verify that it is using the official interface level specification

        ifstream fileText(data.fCommand.toCurrentCodePage()->buffer());

        if (!fileText)
        {
            STAFString openError("Error opening file: " + data.fCommand);
            *pErrorBuffer = openError.adoptImpl();
            return kSTAFServiceConfigurationError;
        }

        unsigned int done = 0;

        for(; !done && (data.fInterfaceLevel == 0);)
        {
            char buffer[256] = { 0 };

            fileText.getline(buffer, 255);

            STAFString line = STAFString(buffer).strip();

            // Make sure this is a comment line

            if (line.find("/*") != 0)
            {
                done = 1;
                continue;
            }

            // Make sure there are at least two words on the first line

            if (line.numWords() < 2) continue;

            // Make sure the second word is in the format <string>:<string>

            STAFString specifier = line.subWord(1, 1);
            unsigned int colonPos = specifier.find(kUTF8_COLON);

            if (colonPos == STAFString::kNPos) continue;
            if (specifier.length() < (colonPos + 1)) continue;

            // Make sure the correct marker is there and the second string is a
            // number

            STAFString interfaceString =
                       specifier.subString(0, colonPos).toUpperCase();
            STAFString interfaceLevelString = specifier.subString(colonPos + 1);

            if (interfaceString != "STAF_SERVICE_INTERFACE_LEVEL") continue;

            try
            {
                data.fInterfaceLevel = interfaceLevelString.asUInt();
            }
            catch (STAFException)
            {
                continue;
            }
        }

        // Check to see if we found a valid REXX inteface level specifier

        if (done)
        {
            STAFString levelError("No valid REXX service interface level "
                                  "specifier found");
            *pErrorBuffer = levelError.adoptImpl();
            return kSTAFInvalidAPILevel;
        }

        // Make sure it specifies a supported interface level

        if ((data.fInterfaceLevel != 30))
        {
            STAFString levelError("Unsupported REXX interface level");
            *pErrorBuffer = levelError.adoptImpl();
            return kSTAFInvalidAPILevel;
        }

        if (!data.fDebugMode)
        {
            SHORT rexxRC = 0;
            LONG rc = 0;
            RXSTRING arg = { 3, "//T" };
            RXSTRING result = { 0 };
            RXSTRING inStore[2] = { 0 };

            // First open the file

            fstream sourceFile(data.fCommand.toCurrentCodePage()->buffer(),
                               ios::binary | ios::in);
            if (!sourceFile)
            {
                STAFString openError("Error opening file: " + data.fCommand);
                *pErrorBuffer = openError.adoptImpl();
                return kSTAFFileOpenError;
            }

            // Figure out how big it is

            sourceFile.seekg(0, std::ios::end);
            unsigned int fileLength = (unsigned int)sourceFile.tellg();

            // Initialize the source buffer

            inStore[0].strlength = fileLength;
            inStore[0].strptr = new char[fileLength];
            sourceFile.seekg(0, std::ios::beg);
            sourceFile.read(inStore[0].strptr, fileLength);

            // Now call RexxStart to tokenize the source

            rc = RexxStart(1, &arg, const_cast<char *>(
                           data.fCommand.toCurrentCodePage()->buffer()),
                           inStore, 0, RXCOMMAND, 0, &rexxRC, &result);

            // Free up the source image

            delete [] inStore[0].strptr;

            if (rc != 0)
            {
                STAFString tokenizeError("Error tokenizing file: " +
                                         data.fCommand);
                *pErrorBuffer = tokenizeError.adoptImpl();
                 return kSTAFREXXError;
            }

            // Copy the image into an std::string for ease of use later

            data.fTokenImage = std::string(inStore[1].strptr,
                                           (unsigned int)inStore[1].strlength);

            // Free up the sytem memory for the tokenized image

            STAFUtilFreeSystemMemory(inStore[1].strptr);
        }

        // Set service handle

        *pServiceHandle = new RexxServiceData(data);

        return kSTAFOk;
    }
    catch (STAFException &e)
    { e.trace(); }
    catch (...)
    { /* Do Nothing */ }

    return kSTAFUnknownError;
}


unsigned int STAFServiceInit(STAFServiceHandle_t serviceHandle,
                             void *pInitInfo, unsigned int initLevel,
                             STAFString_t *pErrorBuffer)
{
    try
    {
        if (initLevel != 30) return kSTAFInvalidAPILevel;

        RexxServiceData *pData =
                        reinterpret_cast<RexxServiceData *>(serviceHandle);
        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);
        STAFString result;
        unsigned int rc = kSTAFUnknownError;
        RXSTRING initArgs[3] = { 0 };

        // XXX: Check use of pInfo->parms here

        STAFStringBufferPtr serviceName = pData->fName.toCurrentCodePage();
        STAFStringBufferPtr initParms =
                            STAFString(pInfo->parms).toCurrentCodePage();
        STAFStringBufferPtr command = pData->fCommand.toCurrentCodePage();

        initArgs[0].strptr = const_cast<char *>(serviceName->buffer());
        initArgs[0].strlength = serviceName->length();
        initArgs[1].strptr = const_cast<char *>(initParms->buffer());
        initArgs[1].strlength = initParms->length();
        initArgs[2].strptr = const_cast<char *>(command->buffer());
        initArgs[2].strlength = command->length();

        if (pData->fInterfaceLevel == 30)
            rc = callRexx(pData, "STAFServiceInit", 30, initArgs, result);

        if (rc) *pErrorBuffer = result.adoptImpl();

        return rc;
    }
    catch (STAFException &e)
    { e.trace(); }
    catch (...)
    { /* Do Nothing */ }

    return kSTAFUnknownError;
}


unsigned int STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                      void *pRequestInfo, unsigned int reqLevel,
                                      STAFString_t *pResultBuffer)
{
    try
    {
        if (reqLevel != 30) return kSTAFInvalidAPILevel;

        RexxServiceData *pData =
                        reinterpret_cast<RexxServiceData *>(serviceHandle);
        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);
        STAFString result;    
        unsigned int rc = kSTAFUnknownError;

        STAFStringBufferPtr serviceName = pData->fName.toCurrentCodePage();
        STAFStringBufferPtr client =
                            STAFString(pInfo->machine).toCurrentCodePage();
        STAFStringBufferPtr machineNickname =
            STAFString(pInfo->machineNickname).toCurrentCodePage();
        STAFStringBufferPtr clientTrustLevel =
                            STAFString(pInfo->trustLevel).toCurrentCodePage();
        STAFStringBufferPtr handleName =
                            STAFString(pInfo->handleName).toCurrentCodePage();
        STAFStringBufferPtr handleString =
                            STAFString(pInfo->handle).toCurrentCodePage();
        STAFStringBufferPtr request =
                            STAFString(pInfo->request).toCurrentCodePage();

        if (pData->fInterfaceLevel == 30)
        {
            // XXX: Changed to remove localMachine and changed effective
            //      machine to machineNickname.  Still need to add user,
            //      endpoint, stafInstanceUUID, isLocalRequest, and
            //      physicalInterfaceID fields and then change whatever
            //      uses these requestArgs to use the new indexes for fields.
            RXSTRING requestArgs[7] = { 0 };

            requestArgs[0].strptr = const_cast<char *>(serviceName->buffer());
            requestArgs[0].strlength = serviceName->length();
            requestArgs[1].strptr = const_cast<char *>(client->buffer());
            requestArgs[1].strlength = client->length();
            requestArgs[2].strptr =
                const_cast<char *>(machineNickname->buffer());
            requestArgs[2].strlength = machineNickname->length();
            requestArgs[3].strptr =
                const_cast<char *>(clientTrustLevel->buffer());
            requestArgs[3].strlength = clientTrustLevel->length();
            requestArgs[4].strptr = const_cast<char *>(handleName->buffer());
            requestArgs[4].strlength = handleName->length();
            requestArgs[5].strptr = const_cast<char *>(handleString->buffer());
            requestArgs[5].strlength = handleString->length();
            requestArgs[6].strptr = const_cast<char *>(request->buffer());
            requestArgs[6].strlength = request->length();

            rc = callRexx(pData, "STAFServiceAcceptRequest", 8, requestArgs,
                          result);
        }

        *pResultBuffer = result.adoptImpl();

        return rc;
    }
    catch (STAFException &e)
    { e.trace(); }
    catch (...)
    { /* Do Nothing */ }

    return kSTAFUnknownError;
}


unsigned int STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                             void *pTermInfo, unsigned int termLevel,
                             STAFString_t *pErrorBuffer)
{
    try
    {
        if (termLevel != 0) return kSTAFInvalidAPILevel;

        STAFString result;
        RXSTRING termArgs[1] = { 0 };
        RexxServiceData *pData =
                        reinterpret_cast<RexxServiceData *>(serviceHandle);
        STAFStringBufferPtr serviceName = pData->fName.toCurrentCodePage();

        termArgs[0].strptr = const_cast<char *>(serviceName->buffer());
        termArgs[0].strlength = serviceName->length();

        return callRexx(pData, "STAFServiceTerm", 1, termArgs, result);
    }
    catch (STAFException &e)
    { e.trace(); }
    catch (...)
    { /* Do Nothing */ }

    return kSTAFUnknownError;
}


unsigned int STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                                 void *pDestructInfo,
                                 unsigned int destructLevel,
                                 STAFString_t *pErrorBuffer)
{
    try
    {
        if (destructLevel != 0) return kSTAFInvalidAPILevel;

        // XXX: Should we accept a pointer to the handle here, so that we can 0
        //      it out after the delete?

        RexxServiceData *pData =
            reinterpret_cast<RexxServiceData *>(*serviceHandle);

        delete pData;
        *serviceHandle = 0;

        return 0;
    }
    catch (STAFException &e)
    { e.trace(); }
    catch (...)
    { /* Do Nothing */ }

    return kSTAFUnknownError;
}


unsigned int callRexx(RexxServiceData *pData, std::string function,
                      unsigned int numParms, RXSTRING *parms,
                      STAFString &result)
{
    SHORT rexxRC = 0;
    LONG rc = 0;
    RXSTRING rexxResult = { 0 };
    RXSTRING inStore[2] = { 0 };

    if (pData->fDebugMode)
    {
        // First open the file

        ifstream sourceFile(pData->fCommand.toCurrentCodePage()->buffer(),
                            ios::binary | ios::in);
        if (!sourceFile)
        {
            result = "Error opening file: " + pData->fCommand;
            return kSTAFFileOpenError;
        }

        // Figure out how big it is

        sourceFile.seekg(0, ios::end);
        unsigned int fileLength = (unsigned int)sourceFile.tellg();

        // Initialize the source buffer

        inStore[0].strlength = fileLength;
        inStore[0].strptr = new char[fileLength];
        sourceFile.seekg(0, std::ios::beg);
        sourceFile.read(inStore[0].strptr, fileLength);
    }
    else
    {
        inStore[1].strptr = const_cast<char *>(pData->fTokenImage.data());
        inStore[1].strlength = pData->fTokenImage.length();
    }

    rc = RexxStart(numParms, parms, const_cast<char *>(function.c_str()),
                   inStore, 0, RXFUNCTION, 0, &rexxRC, &rexxResult);

    if (pData->fDebugMode)
    {
        // Free up the source image

        delete [] inStore[0].strptr;

        // Free up the sytem memory for the tokenized image

        STAFUtilFreeSystemMemory(inStore[1].strptr);
    }

    std::string resultString(rexxResult.strptr,
                             (unsigned int)rexxResult.strlength);

    if (rexxResult.strptr != 0)
        STAFUtilFreeSystemMemory(rexxResult.strptr);

    // XXX: We might want/need to add a STAFString constructor which takes
    //      a signed int

    if (rc != 0) result = STAFString(rc);

    // XXX: Not sure what to do about tracing

    if (rc < 0)
    {
        result = "-" + STAFString(static_cast<unsigned int>(-rc));

        return kSTAFREXXError;
    }
    else if (rc > 0)
    {
        result = STAFString(static_cast<unsigned int>(rc));

        return kSTAFBaseOSError;
    }
    else if (resultString.length() < 4)
    {
        return kSTAFInvalidServiceResult;
    }

    result = STAFString(&resultString.data()[4],
                        resultString.length() - 4);

    unsigned int realRC = *reinterpret_cast<unsigned int *>(
                          const_cast<char *>(resultString.substr(0,4).data()));

    // REXX Services assume the byte ordering is little-endian

    return STAFUtilConvertLEUIntToNative(realRC);
}
