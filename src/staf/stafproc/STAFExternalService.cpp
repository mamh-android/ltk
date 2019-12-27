/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFExternalService.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFTrustManager.h"

STAFExternalService::STAFExternalService(STAFString name, STAFString library,
                                         STAFString exec, OptionList options,
                                         STAFString parms,
                                         STAFServiceType_t serviceType,
                                         STAFString loadedBySLS)
    : STAFService(name, 1, serviceType, loadedBySLS),
      fLibName(library),
      fImplLib(library.toCurrentCodePage()->buffer()), fExec(exec),
      fParms(parms)
{
    // Obtain service's interface addresses and levels

    fGetBounds = (STAFServiceGetLevelBounds_t)fImplLib.getAddress(
        "STAFServiceGetLevelBounds");

    fLevelConstruct = getLevel(kServiceInfo, "ServiceInfo", 30, 30);
    fConstruct = (STAFServiceConstruct_t)fImplLib.getAddress(
        "STAFServiceConstruct");

    fLevelInit = getLevel(kServiceInit, "ServiceInit", 30, 30);
    fInit = (STAFServiceInit_t)fImplLib.getAddress("STAFServiceInit");

    fLevelAcceptRequest = getLevel(kServiceAcceptRequest,
                                   "ServiceAcceptRequest", 30, 30);
    fAcceptRequest = (STAFServiceAcceptRequest_t)fImplLib.getAddress(
                     "STAFServiceAcceptRequest");

    fLevelTerm = getLevel(kServiceTerm, "ServiceTerm", 0, 0);
    fTerm = (STAFServiceTerm_t)fImplLib.getAddress("STAFServiceTerm");

    fLevelDestruct = getLevel(kServiceDestruct, "ServiceDestruct", 0, 0);
    fDestruct = (STAFServiceDestruct_t)fImplLib.getAddress(
                "STAFServiceDestruct");

    // Build option name and value arrays

    STAFString *optionNameStrings = new STAFString[options.size()];
    STAFString *optionValueStrings = new STAFString[options.size()];
    unsigned int i = 0;
    fOptions = STAFObject::createList();

    for (OptionList::iterator iter = options.begin(); iter != options.end();
         ++iter, ++i)
    {
        fOptions->append(iter->subString(0));
        unsigned int equalPos = iter->find(kUTF8_EQUAL);
        optionNameStrings[i] = iter->subString(0, equalPos);

        if (equalPos != STAFString::kNPos)
            optionValueStrings[i] = iter->subString(equalPos + 1);
    }

    STAFString_t *optionNames = new STAFString_t[options.size()];
    STAFString_t *optionValues = new STAFString_t[options.size()];

    for (i = 0; i < options.size(); ++i)
    {
        optionNames[i] = optionNameStrings[i].getImpl();
        optionValues[i] = optionValueStrings[i].getImpl();
    }

    // Construct service

    STAFString_t errorBuffer = 0;
    unsigned int constructRC = kSTAFUnknownError;

    if (fLevelConstruct == 30)
    {
        STAFServiceInfoLevel30 info = { name.getImpl(), exec.getImpl(),
                                        gSTAFWriteLocationPtr->getImpl(),
                                        serviceType,
                                        options.size(), optionNames,
                                        optionValues };

        constructRC = fConstruct(&fServiceHandle, &info, 30, &errorBuffer);
    }

    // Cleanup

    delete [] optionNameStrings;
    delete [] optionValueStrings;
    delete [] optionNames;
    delete [] optionValues;

    if (constructRC != 0)
    {
        STAFString errorString;

        if (errorBuffer)
            errorString = STAFString(errorBuffer, STAFString::kShallow);

        STAFServiceCreateException
            error((STAFString("Error constructing service, ")
                  + fLibName + ", Result: "
                  + errorString).toCurrentCodePage()->buffer(), 
                  constructRC);

        THROW_STAF_EXCEPTION(error);
    }
}

unsigned int STAFExternalService::getLevel(STAFServiceLevelID levelID,
                                           const char *levelString,
                                           unsigned int stafMin,
                                           unsigned int stafMax)
{
    unsigned int serviceMin = 0;
    unsigned int serviceMax = 0;
    unsigned int rc = 0;

    // Get the service's supported levels

    if ((rc = fGetBounds(levelID, &serviceMin, &serviceMax) != 0))
    {
        STAFString message(STAFString("Error getting level bounds for ") +
                           levelString);
        STAFServiceCreateException error(message.toCurrentCodePage()->buffer());
        THROW_STAF_EXCEPTION(error);
    }

    // Check for non-overlapping ranges

    if ((serviceMin > stafMax) || (stafMin > serviceMax))
    {
        STAFString message(levelString);
        message += STAFString(" level validation failure, RC = " + rc) +
                   ".  Service supports "        +
                   serviceMin + ":" + serviceMax +
                   ".  STAF supports " + stafMin +
                   ":" + stafMax + ".";
        STAFServiceCreateException
            error(message.toCurrentCodePage()->buffer());
        THROW_STAF_EXCEPTION(error);
    }

    return STAF_MIN(serviceMax, stafMax);
}


STAFExternalService::~STAFExternalService()
{
    STAFString_t errorBuffer = 0;
    unsigned int rc = fDestruct(&fServiceHandle, 0, 0, &errorBuffer);
    unsigned int osRC = 0;

    // As a fix for Bug #1851096 "Removing multiple STAF EXECPROXY services
    // causes trap on AIX 32-bit", changed to "delete fServiceHandle" and
    // "fServiceHandle = 0" in this destructor instead of in
    // STAFExecProxyLib::STAFServiceDestruct() on AIX machines

#ifdef STAF_OS_NAME_AIX
    if (fServiceHandle != 0)
    {
        delete fServiceHandle;
        fServiceHandle = 0;
    }
#endif

    if (errorBuffer) STAFStringDestruct(&errorBuffer, &osRC);
}


STAFString STAFExternalService::info(unsigned int) const
{
    STAFString data(name() + ": External library " + fLibName);

    if (fExec.length() != 0)
        data += ", exec " + fExec;

    return data;
}


STAFString STAFExternalService:: getLibName() const
{
    return fLibName;
}


STAFString STAFExternalService:: getExecutable() const
{
    return fExec;
}


STAFString STAFExternalService:: getParameters() const
{
    return fParms;
}


STAFObjectPtr STAFExternalService:: getOptions() const
{
    return fOptions->reference();
}


STAFServiceResult STAFExternalService::init()
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc = kSTAFUnknownError;

    if (fLevelInit == 30)
    {
        STAFServiceInitLevel30 initData = { fParms.getImpl(),
                                            gSTAFWriteLocationPtr->getImpl() };
        rc = fInit(fServiceHandle, &initData, 30, &errorBuffer);
    }
    
    STAFString result = STAFString(errorBuffer, STAFString::kShallow);
    
    if (rc) result = STAFString("Error initializing service, ")
                     + fLibName + STAFString(", Result: ")+ result;

    return STAFServiceResult(rc, result);
}


STAFServiceResult STAFExternalService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString_t resultBuffer = 0;
    STAFRC_t rc = kSTAFUnknownError;
    
    if (fLevelAcceptRequest == 30)
    {
        STAFServiceRequestLevel30 requestData = {
            requestInfo.fSTAFInstanceUUID.getImpl(),
            requestInfo.fMachine.getImpl(),
            requestInfo.fMachineNickname.getImpl(),
            requestInfo.fHandleName.getImpl(),
            requestInfo.fHandle,
            requestInfo.fTrustLevel,
            requestInfo.fIsLocalRequest,
            requestInfo.fDiagEnabled,
            requestInfo.fRequest.getImpl(),
            requestInfo.fRequestNumber,
            requestInfo.fUser.getImpl(),
            requestInfo.fEndpoint.getImpl(),
            requestInfo.fPhysicalInterfaceID.getImpl()
        };

        rc = fAcceptRequest(fServiceHandle, &requestData, 30, &resultBuffer);
    }
   
    return STAFServiceResult(rc, STAFString(resultBuffer,
                                            STAFString::kShallow));
}


STAFServiceResult STAFExternalService::term()
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fTerm(fServiceHandle, 0, 0, &errorBuffer);

    STAFString result = STAFString(errorBuffer, STAFString::kShallow);
    
    if (rc) result = STAFString("Error terminating service, ")
                     + fLibName + STAFString(", Result: ")+ result;

    return STAFServiceResult(rc, result);
}
