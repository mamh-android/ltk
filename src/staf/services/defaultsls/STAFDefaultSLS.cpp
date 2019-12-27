/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFOSTypes.h"
#include <map>
#include "STAF_fstream.h"
#include "STAF_iostream.h"
#include "STAFString.h"
#include "STAFError.h"
#include "STAFException.h"
#include "STAFRefPtr.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFUtil.h"
#include "STAFInternalUtil.h"
#include "STAFDefaultSLS.h"

typedef STAFRefPtr<STAFCommandParser> STAFCommandParserPtr;

static STAFString sLOAD("LOAD");
static STAFString sSERVICE("SERVICE");
static STAFString sVAR("VAR");
static STAFString sLOG("LOG");
static STAFString sSTAFLog("STAFLog");
static STAFString sMONITOR("MONITOR");
static STAFString sSTAFMon("STAFMon");
static STAFString sRESPOOL("RESPOOL");
static STAFString sSTAFPool("STAFPool");
static STAFString sZIP("ZIP");
static STAFString sSTAFZip("STAFZip");
static STAFString sLOCAL("LOCAL");

struct DefaultServiceLoaderServiceData
{
    unsigned int  fDebugMode;              // Debug Mode flag
    STAFString    fShortName;              // Short service name
    STAFString    fName;                   // Registered service name    
    STAFHandlePtr fHandlePtr;              // Service's STAF handle
    STAFCommandParserPtr fLoadParser;      // LOAD command parser
};

typedef STAFRefPtr<DefaultServiceLoaderServiceData> 
    DefaultServiceLoaderServiceDataPtr;

static STAFString lineSep;

static STAFResultPtr handleLoad(STAFServiceRequestLevel30 *,
                                DefaultServiceLoaderServiceData *);

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
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        if (infoLevel != 30) return kSTAFInvalidAPILevel;

        STAFServiceInfoLevel30 *pInfo =
            reinterpret_cast<STAFServiceInfoLevel30 *>(pServiceInfo);
            
        DefaultServiceLoaderServiceData data;
        data.fDebugMode = 0;
        data.fShortName = pInfo->name;
        data.fName = "STAF/Service/";
        data.fName += pInfo->name;
        
        // Set service handle

        *pServiceHandle = new DefaultServiceLoaderServiceData(data);
       
        return kSTAFOk;
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
                "STAFDefaultServiceLoader.cpp: STAFServiceConstruct").
                adoptImpl();
    }
    catch (...)
    {
        STAFString error(
            "STAFDefaultServiceLoader.cpp: STAFServiceConstruct: Caught "
            "unknown exception in STAFServiceAcceptRequest()");
        *pErrorBuffer = error.adoptImpl();
    }    

    return kSTAFUnknownError;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
                         void *pInitInfo, unsigned int initLevel,
                         STAFString_t *pErrorBuffer)
{
    STAFRC_t retCode = kSTAFUnknownError;

    try
    {    
        if (initLevel != 30) return kSTAFInvalidAPILevel;

        DefaultServiceLoaderServiceData *pData =
            reinterpret_cast<DefaultServiceLoaderServiceData *>(serviceHandle);
        
        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);        

        retCode = STAFHandle::create(pData->fName, pData->fHandlePtr);

        if (retCode != kSTAFOk)
            return retCode;

        pData->fLoadParser = STAFCommandParserPtr(new STAFCommandParser,
            STAFCommandParserPtr::INIT);
        pData->fLoadParser->addOption(sLOAD, 1, 
            STAFCommandParser::kValueNotAllowed);
        pData->fLoadParser->addOption(sSERVICE, 1, 
            STAFCommandParser::kValueRequired);
            
        pData->fLoadParser->addOptionNeed(sLOAD, sSERVICE);
    }

    catch (STAFException &e)
    {
        STAFString result;
        
        result += 
            STAFString("In DefaultServiceLoaderService.cpp: STAFServiceInit")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error(
            "DefaultServiceLoaderService.cpp: STAFServiceInit: Caught unknown "
            "exception in STAFServiceServiceInit()");
        *pErrorBuffer = error.adoptImpl();
    }    

    return retCode;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                  void *pRequestInfo, unsigned int reqLevel,
                                  STAFString_t *pResultBuffer)
{
    if (reqLevel != 30) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    STAFResultPtr result(new STAFResult(), STAFResultPtr::INIT);

    try
    {
        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);
            
        DefaultServiceLoaderServiceData *pData =
            reinterpret_cast<DefaultServiceLoaderServiceData *>(serviceHandle);

        STAFString request(pInfo->request);
        STAFString action = request.subWord(0, 1).toUpperCase();
        
        if (action == sLOAD)  
        {
            result = handleLoad(pInfo, pData);
        }
    }
    catch (STAFException &e)
    {
        *pResultBuffer = getExceptionString(e,
                         "STAFDefaultServiceLoader.cpp: "
                         "STAFServiceAcceptRequest").adoptImpl();
    }
    catch (...)
    {
        STAFString error("STAFDefaultServiceLoader.cpp: STAFServiceAcceptRequest: Caught "
                         "unknown exception in STAFServiceAcceptRequest()");
        *pResultBuffer = error.adoptImpl();
    }
        
    *pResultBuffer = result->result.adoptImpl();

    return kSTAFOk;
}


STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                         void *pTermInfo, unsigned int termLevel,
                         STAFString_t *pErrorBuffer)
{
    if (termLevel != 0) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    try
    {
        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    { 
        *pErrorBuffer = getExceptionString(e,
            "STAFDefaultSericeLoader.cpp: STAFServiceTerm").adoptImpl();
    }
    catch (...)
    {
        STAFString error(
            "STAFDefaultServiceLoader.cpp: STAFServiceTerm: Caught unknown "
            "exception in STAFServiceAcceptRequest()");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}


STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                             void *pDestructInfo, unsigned int destructLevel,
                             STAFString_t *pErrorBuffer)
{
    if (destructLevel != 0) return kSTAFInvalidAPILevel;

    STAFRC_t retCode = kSTAFUnknownError;

    try
    {
        DefaultServiceLoaderServiceData *pData =
            reinterpret_cast<DefaultServiceLoaderServiceData *>
            (*serviceHandle);

        delete pData;
        *serviceHandle = 0;

        retCode = kSTAFOk;
    }
    catch (STAFException &e)
    { 
        STAFString result;
        
        result += STAFString
            ("In DefaultServiceLoaderService.cpp: STAFServiceDestruct")
            + kUTF8_SCOLON;
            
        result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
        result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
        result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
        result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;
        
        *pErrorBuffer = result.adoptImpl();
    }
    catch (...)
    {
        STAFString error(
            "DefaultServiceLoaderService.cpp: STAFServiceDestruct: Caught "
            "unknown exception in STAFServiceDestruct()");
        *pErrorBuffer = error.adoptImpl();
    }

    return retCode;
}

STAFResultPtr handleLoad(STAFServiceRequestLevel30 *pInfo,
                         DefaultServiceLoaderServiceData *pData)
{
    STAFCommandParseResultPtr parsedResult =
        pData->fLoadParser->parse(pInfo->request);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
            parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
    
    STAFString serviceName = parsedResult->optionValue(sSERVICE);
    STAFString libraryName;
    
    if (serviceName.toUpperCase() == sLOG)
    {
        libraryName = sSTAFLog;
    }
    else if (serviceName.toUpperCase() == sMONITOR)
    {
        libraryName = sSTAFMon;
    }
    else if (serviceName.toUpperCase() == sRESPOOL)
    {
        libraryName = sSTAFPool;
    }
    else if (serviceName.toUpperCase() == sZIP)
    {
        libraryName = sSTAFZip;
    }
    else
    {
        return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
    }
    
    STAFString request = "ADD SERVICE " + serviceName +
        " LIBRARY " + libraryName;

    #if defined(STAF_OS_NAME_FREEBSD)
    
        if (libraryName == sSTAFZip)
        {
            // Determine if the operating system on the local machine is
            // FreeBSD 4.x (vs a later version of FreeBSD) because want to
            // use EXECPROXY when registering the ZIP service because a ADD
            // request to the ZIP service on FreeBSD 4.x may fail and cause
            // STAFProc to crash if not using EXECPROXY.

            STAFResultPtr result = pData->fHandlePtr->submit(
                sLOCAL, sVAR, "RESOLVE STRING {STAF/Config/OS/MajorVersion}");

            if ((result->rc == kSTAFOk) &&
                (result->result.length(STAFString::kChar) > 0) && 
                (result->result.subString(0, 1) == "4"))
            {
                request = "ADD SERVICE " + serviceName +
                    " LIBRARY STAFEXECPROXY EXECUTE " + libraryName;
            }
        }

    #endif
    
    STAFResultPtr result = pData->fHandlePtr->submit(
        sLOCAL, sSERVICE, request);

    return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
}
