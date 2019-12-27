/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <vector>
#include <map>
#include "STAFExecProxyLib.h"
#include "STAFServiceInterface.h"
#include "STAFConnectionProvider.h"
#include "STAFUtil.h"
#include "STAFThread.h"
#include "STAFEventSem.h"
#include "STAFMutexSem.h"
#include "STAFProcess.h"
#include "STAFFileSystem.h"
#include "STAFTrace.h"
#include "STAF_fstream.h"

struct ExecProxyData
{
    STAFString fName;
    STAFString fExec;
    STAFString fOptions;
    STAFConnectionProviderPtr fConnProv;
    STAFEventSemPtr fExecProxyExitedSem;
    STAFProcessID_t fExecProxy_PID;
    unsigned int fNumServices;
};

static STAFString sLocal("local");
static STAFString sIPCName("IPCNAME");
static STAFString sJSTAF("JSTAF");

typedef std::map<STAFString, STAFString> STAFEnvMap;

unsigned int fEnvVarCaseSensitive = 1;

#if defined(STAF_OS_NAME_SOLARIS) || defined(STAF_OS_NAME_ZOS) || \
    defined(STAF_OS_NAME_FREEBSD)
    extern char **environ;
#elif defined(STAF_OS_NAME_MACOSX)
#    include <crt_externs.h>
#    define environ (*_NSGetEnviron())
#endif

STAFRC_t STAFServiceGetLevelBounds(unsigned int levelID,
                                   unsigned int *minimum,
                                   unsigned int *maximum)
{
    switch (levelID)
    {
        case kServiceInfo:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceInit:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
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

    return kSTAFOk;
}

STAFRC_t STAFServiceConstruct(STAFServiceHandle_t *pServiceHandle,
                              void *pServiceInfo, unsigned int infoLevel,
                              STAFString_t *pErrorBuffer)
{
    try
    {
        if (infoLevel != 30) return kSTAFInvalidAPILevel;

        ExecProxyData data;
        STAFServiceInfoLevel30 *pInfo =
            static_cast<STAFServiceInfoLevel30 *>(pServiceInfo);
        STAFString serviceName = pInfo->name;
        STAFString serviceExec = pInfo->exec;
        STAFString execProxyExec = "STAFExecProxy";
        data.fName = pInfo->name;
        data.fExec = pInfo->exec;

        STAFString proxyLibrary = data.fExec;
        bool proxyLibrarySpecified = false;
        int proxyLibraryIndex = -1;
        unsigned int numProxyEnvOptions = 0;

        STAFString options = ""; // All OPTIONs other than PROXYLIBRARY

        // Create the environment data from the global environment map.
        // Need to create even if no environment variables are being added or
        // changed because if a process is started as another user on Windows
        // and then if a process is started as the user who was logged on when
        // STAFProc was started, the USERPROFILE environment shows the wrong
        // value if set startData->environment = 0 instead of creating the
        // environment data.

        // First, create a STAFProcessEnvMap from the global environment map so
        // that the key (the environment variable name) can be case-insensitive
        // if designated by the operating system.

        STAFConfigInfo configInfo;
        STAFString_t errorBufferT = 0;
        unsigned int configInfoOSRC = 0;

        if (STAFUtilGetConfigInfo(&configInfo, &errorBufferT, &configInfoOSRC)
            != kSTAFOk)
        {
            STAFString errorMsg = STAFString(errorBufferT, STAFString::kShallow)
                + ", RC: " + STAFString(configInfoOSRC);
                STAFTrace::trace(
                kSTAFTraceError, errorMsg.toCurrentCodePage()->buffer());
        }
        else
        {
            fEnvVarCaseSensitive = configInfo.envVarCaseSensitive;
        }

        STAFProcessEnvMap envMap;

        for (int envi = 0; environ[envi] != 0; ++envi)
        {
            STAFString envVar(environ[envi]);
            unsigned int equalPos = envVar.find(kUTF8_EQUAL);
            STAFString envName(envVar.subString(0, equalPos));
            STAFString envNameKey = envName;

            if (!fEnvVarCaseSensitive)
                envNameKey.upperCase();

            STAFString envValue;

            if (equalPos != STAFString::kNPos)
            {
                envValue = envVar.subString(equalPos + 1);
            }

            envMap[envNameKey] = STAFProcessEnvData(envName, envValue);
        }

        // Walk through and verify the config options

        for (int k = 0; k < pInfo->numOptions; ++k)
        {
            STAFString upperOptionName =
                       STAFString(pInfo->pOptionName[k]).upperCase();

            STAFString optionValue(pInfo->pOptionValue[k]);

            if (upperOptionName == "PROXYLIBRARY")
            {
                proxyLibrary = optionValue;

                if (proxyLibrary.upperCase() == sJSTAF)
                {
                    STAFString error("Using the STAFEXECPROXY library for "
                        "Java STAF services is not supported");

                    *pErrorBuffer = error.adoptImpl();

                    return kSTAFServiceConfigurationError;
                }

                proxyLibraryIndex = k;
                proxyLibrarySpecified = true;
            }
            else if (upperOptionName == "PROXYENV")
            {
                // Update the envMap with the variable

                numProxyEnvOptions = numProxyEnvOptions + 1;

                STAFString envVar = optionValue;

                unsigned int equalPos = envVar.find(kUTF8_EQUAL);
                STAFString envName(envVar.subString(0, equalPos));
                STAFString envNameKey = envName;

                if (!fEnvVarCaseSensitive)
                    envNameKey.upperCase();

                STAFString envValue;

                if (equalPos != STAFString::kNPos)
                    envValue = envVar.subString(equalPos + 1);

                // Retain original env variable name if already exists

                if (envMap.find(envNameKey) != envMap.end())
                    envMap[envNameKey].envValue = envValue;
                else
                    envMap[envNameKey] = STAFProcessEnvData(envName, envValue);
            }
        }

        // Start a process for STAFExecProxy

        STAFProcessStartInfoLevel1 startInfo = { 0 };


        startInfo.command     = execProxyExec.getImpl();

        startInfo.parms       = serviceName.getImpl();
        startInfo.consoleMode = kSTAFProcessSameConsole;

        int bufSize = 0;
        std::deque<STAFStringBufferPtr> envList;

        for (STAFProcessEnvMap::iterator iter2 = envMap.begin();
            iter2 != envMap.end(); ++iter2)
        {
            STAFProcessEnvData envData = iter2->second;
            STAFString envCombo = envData.envName + kUTF8_EQUAL +
                envData.envValue;
            STAFStringBufferPtr envComboPtr = envCombo.toCurrentCodePage();

            bufSize += envComboPtr->length() + 1;  // Add one for the null byte
            envList.push_back(envComboPtr);
        }

        // Allocate the buffer

        bufSize += 1;  // Add one for the trailing null byte
        char *envBuf = new char[bufSize];

        // Walk the list and add the entries to the buffer

        bufSize = 0;

        for (std::deque<STAFStringBufferPtr>::iterator iter = envList.begin();
             iter != envList.end(); ++iter)
        {
            memcpy(envBuf + bufSize, (*iter)->buffer(), (*iter)->length());
            envBuf[bufSize + (*iter)->length()] = 0;
            bufSize += (*iter)->length() + 1;
        }

        // Add the trailing null byte

        envBuf[bufSize] = 0;

        if (bufSize > 0)
        {
            startInfo.environment = (char *)envBuf;
        }

        unsigned int osRC = 0;
        STAFString_t errorBuffer = 0;

        STAFRC_t rc = STAFProcessStart2(
            0, 0, &startInfo, 1, &osRC, &errorBuffer);

        if (rc != kSTAFOk)
        {
            STAFString startError(
                "Error starting a process for STAFExecProxy , RC: " +
                STAFString(rc) + ", Result: " +
                STAFString(errorBuffer, STAFString::kShallow));

            *pErrorBuffer = startError.adoptImpl();

            return kSTAFServiceConfigurationError;
        }

        // Create the connection provider for the STAFExecProxy

        STAFString ipcName = serviceName;
        STAFStringConst_t optionData[] = { sIPCName.getImpl(),
                                           ipcName.getImpl() };
        STAFConnectionProviderConstructInfoLevel1 constructInfo =
        {
            kSTAFConnectionProviderOutbound,
            1,
            optionData,
            &optionData[1]
        };

        data.fConnProv =
            STAFConnectionProvider::createRefPtr(ipcName, "STAFLIPC",
                                                 &constructInfo, 1);

        // Now we need to wait for it to start

        bool execProxyReady = false;

        for (int i = 0; (i < 30) & !execProxyReady; ++i)
        {
            try
            {
                // First connect to the STAFExecProxy
                STAFConnectionPtr connPtr =
                    data.fConnProv->connect(sLocal);

                // Now see if it's alive

                connPtr->writeUInt(STAFEXECPROXY_PING);
                STAFRC_t execProxyRC = connPtr->readUInt();
                STAFString execProxyResultString =
                    connPtr->readString();

                if (execProxyRC != kSTAFOk)
                {
                    execProxyResultString =
                        "Error starting STAFExecProxy: " +
                        execProxyResultString;
                    *pErrorBuffer = execProxyResultString.adoptImpl();
                    return kSTAFServiceConfigurationError;
                }

                execProxyReady = true;
            }
            catch (STAFException e)
            {
                // Do nothing - STAFException may occur if the
                // STAFExecProxy has not started yet
            }

            if (!execProxyReady)
                STAFThreadSleepCurrentThread(1000, 0);
        }

        if (!execProxyReady)
        {
            *pErrorBuffer = STAFString(
                "Unable to connect to STAFExecProxy").adoptImpl();
            return kSTAFServiceConfigurationError;
        }

        STAFConnectionPtr connPtr =
            data.fConnProv->connect(sLocal);

        #if defined(STAF_OS_NAME_SOLARIS)
            // Needed to resolve timing issues on Solaris 64-bit
            STAFThreadSleepCurrentThread(1000, 0);
        #endif

        connPtr->writeUInt(STAFEXECPROXY_LOAD);
        connPtr->writeString(data.fName);
        connPtr->writeString(data.fExec);
        connPtr->writeString(proxyLibrary);
        connPtr->writeString(pInfo->writeLocation);
        connPtr->writeUInt(pInfo->serviceType);

        unsigned int numOptions = 0;

        if (proxyLibrarySpecified)
            numOptions = pInfo->numOptions - 1;
        else
            numOptions = pInfo->numOptions;

        numOptions = numOptions - numProxyEnvOptions;

        connPtr->writeUInt(numOptions);

        for (int j = 0; j < pInfo->numOptions; ++j)
        {
            STAFString upperOptionName =
                       STAFString(pInfo->pOptionName[j]).upperCase();

            if ((proxyLibraryIndex != j) &&
                (upperOptionName != "PROXYENV"))
            {
                connPtr->writeString(pInfo->pOptionName[j]);
                connPtr->writeString(pInfo->pOptionValue[j]);
            }
        }

        unsigned int constructRC = connPtr->readUInt();
        STAFString constructResult = connPtr->readString();

        if (constructRC != kSTAFOk)
        {
            *pErrorBuffer = constructResult.adoptImpl();
            return constructRC;
        }

        // The service is now loaded

        ExecProxyData *pService = new ExecProxyData(data);

        *pServiceHandle = pService;

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        e.trace("STAFExecProxy.STAFServiceConstruct()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFExecProxy.STAFServiceConstruct()");
    }

    return kSTAFUnknownError;
}

STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                             void *pDestructInfo,
                             unsigned int destructLevel,
                             STAFString_t *pErrorBuffer)
{
    try
    {
        if (destructLevel != 0) return kSTAFInvalidAPILevel;

        ExecProxyData *pData =
            reinterpret_cast<ExecProxyData *>(*serviceHandle);

        STAFConnectionPtr connPtr = pData->fConnProv->connect(sLocal);

        connPtr->writeUInt(STAFEXECPROXY_DESTRUCT);
        connPtr->writeString(pData->fName);

        unsigned int termRC = connPtr->readUInt();
        STAFString termResult = connPtr->readString();

        // As a fix for Bug #1851096 "Removing multiple STAF EXECPROXY
        // services causes trap on AIX 32-bit", commented out next two lines
        // if on AIX, and instead, do them in the ~STAFExternalService
        // destructor if fServiceHandle != 0
#ifndef STAF_OS_NAME_AIX
        delete pData;
        *serviceHandle = 0;
#endif

        if (termRC != kSTAFOk)
        {
            *pErrorBuffer = termResult.adoptImpl();
            return termRC;
        }

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        e.trace("STAFExecProxy.STAFServiceDestruct()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFExecProxy.STAFServiceDestruct()");
    }

    return kSTAFUnknownError;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
                         void *pInitInfo, unsigned int initLevel,
                         STAFString_t *pErrorBuffer)
{
    try
    {
        if (initLevel != 30) return kSTAFInvalidAPILevel;

        ExecProxyData *pData =
            static_cast<ExecProxyData *>(serviceHandle);
        STAFServiceInitLevel30 *pInfo =
            static_cast<STAFServiceInitLevel30 *>(pInitInfo);
        STAFConnectionPtr connPtr =
            pData->fConnProv->connect(sLocal);

        #if defined(STAF_OS_NAME_SOLARIS)
            // Needed to resolve timing issues on Solaris 64-bit
            STAFThreadSleepCurrentThread(1000, 0);
        #endif

        connPtr->writeUInt(STAFEXECPROXY_INIT);
        connPtr->writeString(pData->fName);
        connPtr->writeString(pInfo->parms);
        connPtr->writeString(pInfo->writeLocation);

        unsigned int initRC = connPtr->readUInt();
        STAFString initResult = connPtr->readString();

        if (initRC != kSTAFOk)
        {
            *pErrorBuffer = initResult.adoptImpl();
            return initRC;
        }

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        e.trace("STAFExecProxy.STAFServiceInit()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFExecProxy.STAFServiceInit()");
    }

    return kSTAFUnknownError;
}

STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                         void *pTermInfo, unsigned int termLevel,
                         STAFString_t *pErrorBuffer)
{
    try
    {
        if (termLevel != 0) return kSTAFInvalidAPILevel;

        ExecProxyData *pData =
            static_cast<ExecProxyData *>(serviceHandle);
        STAFConnectionPtr connPtr =
            pData->fConnProv->connect(sLocal);

        connPtr->writeUInt(STAFEXECPROXY_TERM);
        connPtr->writeString(pData->fName);

        unsigned int termRC = connPtr->readUInt();
        STAFString termResult = connPtr->readString();

        if (termRC != kSTAFOk)
        {
            *pErrorBuffer = termResult.adoptImpl();
            return termRC;
        }

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        e.trace("STAFExecProxy.STAFServiceTerm()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFExecProxy.STAFServiceTerm()");
    }

    return kSTAFUnknownError;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                  void *pRequestInfo, unsigned int reqLevel,
                                  STAFString_t *pResultBuffer)
{
    try
    {
        if (reqLevel != 30) return kSTAFInvalidAPILevel;

        ExecProxyData *pData =
            static_cast<ExecProxyData *>(serviceHandle);
        STAFServiceRequestLevel30 *pInfo =
            static_cast<STAFServiceRequestLevel30 *>(pRequestInfo);
        STAFConnectionPtr connPtr =
            pData->fConnProv->connect(sLocal);

        unsigned int  machineLength          = 0;
        const char   *machineBuffer          = 0;
        unsigned int  machineNicknameLength  = 0;
        const char   *machineNicknameBuffer  = 0;
        unsigned int  handleNameLength       = 0;
        const char   *handleNameBuffer       = 0;
        unsigned int  requestLength          = 0;
        const char   *requestBuffer          = 0;
        unsigned int  userLength             = 0;
        const char   *userBuffer             = 0;
        unsigned int  endpointLength         = 0;
        const char   *endpointBuffer         = 0;
        unsigned int  stafInstanceUUIDLength = 0;
        const char   *stafInstanceUUIDBuffer = 0;
        unsigned int  physicalInterfaceIDLength = 0;
        const char   *physicalInterfaceIDBuffer = 0;

        STAFStringGetBuffer(pInfo->machine, &machineBuffer,
                            &machineLength, 0);
        STAFStringGetBuffer(pInfo->machineNickname, &machineNicknameBuffer,
                            &machineNicknameLength, 0);
        STAFStringGetBuffer(pInfo->handleName, &handleNameBuffer,
                            &handleNameLength, 0);
        STAFStringGetBuffer(pInfo->request, &requestBuffer,
                            &requestLength, 0);
        STAFStringGetBuffer(pInfo->user, &userBuffer, &userLength, 0);
        STAFStringGetBuffer(pInfo->endpoint, &endpointBuffer,
                            &endpointLength, 0);
        STAFStringGetBuffer(pInfo->stafInstanceUUID, &stafInstanceUUIDBuffer,
                            &stafInstanceUUIDLength, 0);
        STAFStringGetBuffer(pInfo->physicalInterfaceID,
                            &physicalInterfaceIDBuffer,
                            &physicalInterfaceIDLength, 0);

        // IMPORTANT:  Increase the numFields value if add a field to the
        // ServiceRequest class for a new STAFServiceInterfaceLevel.

        unsigned int numFields = 16;

        unsigned int bufferLength = (numFields * sizeof(unsigned int)) +
            pData->fName.length() + machineLength + machineNicknameLength +
            handleNameLength + requestLength + userLength + endpointLength +
            stafInstanceUUIDLength + physicalInterfaceIDLength;

        STAFBuffer<char>
            buffer(new char[bufferLength], STAFBuffer<char>::INIT,
                   STAFBuffer<char>::ARRAY);
        unsigned int *uintBuffer =
                     reinterpret_cast<unsigned int *>((char *)buffer);

        uintBuffer[0] =
            STAFUtilConvertNativeUIntToLE(STAFEXECPROXY_ACCEPT_REQUEST);
        uintBuffer[1] = STAFUtilConvertNativeUIntToLE(
            bufferLength - (2 * sizeof(unsigned int)));
        uintBuffer[2] = pData->fName.length();
        uintBuffer[3] = pInfo->handle;
        uintBuffer[4] = pInfo->trustLevel;
        uintBuffer[5] = machineLength;
        uintBuffer[6] = machineNicknameLength;
        uintBuffer[7] = handleNameLength;
        uintBuffer[8] = requestLength;
        uintBuffer[9] = pInfo->diagEnabled;
        uintBuffer[10] = pInfo->requestNumber;
        uintBuffer[11] = userLength;
        uintBuffer[12] = endpointLength;
        uintBuffer[13] = stafInstanceUUIDLength;
        uintBuffer[14] = pInfo->isLocalRequest;
        uintBuffer[15] = physicalInterfaceIDLength;

        char *currBuffer = buffer + (numFields * sizeof(unsigned int));

        memcpy(currBuffer, pData->fName.buffer(), pData->fName.length());
        currBuffer += pData->fName.length();
        memcpy(currBuffer, machineBuffer, machineLength);
        currBuffer += machineLength;
        memcpy(currBuffer, machineNicknameBuffer, machineNicknameLength);
        currBuffer += machineNicknameLength;
        memcpy(currBuffer, handleNameBuffer, handleNameLength);
        currBuffer += handleNameLength;
        memcpy(currBuffer, requestBuffer, requestLength);
        currBuffer += requestLength;
        memcpy(currBuffer, userBuffer, userLength);
        currBuffer += userLength;
        memcpy(currBuffer, endpointBuffer, endpointLength);
        currBuffer += endpointLength;
        memcpy(currBuffer, stafInstanceUUIDBuffer, stafInstanceUUIDLength);
        currBuffer += stafInstanceUUIDLength;
        memcpy(currBuffer, physicalInterfaceIDBuffer,
               physicalInterfaceIDLength);
        currBuffer += physicalInterfaceIDLength;

        connPtr->write(buffer, bufferLength);

        unsigned int reqRC = connPtr->readUInt();
        STAFString reqResult = connPtr->readString();

        *pResultBuffer = reqResult.adoptImpl();
        return reqRC;
    }
    catch (STAFException &e)
    {
        e.trace("STAFExecProxy.STAFServiceAcceptRequest");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in "
            "STAFExecProxy.STAFServiceAcceptRequest()");
    }

    *pResultBuffer = STAFString().adoptImpl();

    return kSTAFUnknownError;
}
