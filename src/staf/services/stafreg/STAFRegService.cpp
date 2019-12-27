/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_fstream.h"
#include <cstring>
#include "STAFRefPtr.h"
#include "STAFUtil.h"
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFTimestamp.h"

struct RegisterServiceData
{
    unsigned int fDebugMode;              // Debug Mode flag
    STAFString    fShortName;              // Short service name
    STAFString    fName;                   // Registered service name
    STAFHandlePtr fHandlePtr;              // Service's STAF handle
    STAFCommandParserPtr fRegisterParser;  // REGISTER command parser
};

typedef STAFRefPtr<RegisterServiceData> RegisterServiceDataPtr;

static const STAFString sNameString = STAFString(";name:");
static const STAFString sEmailString = STAFString(";email:");
static const STAFString sOrgString = STAFString(";org:");
static const STAFString sSkipName1 = STAFString("ITSAS");
static const STAFString sSkipName2 = STAFString("Bryan Osenbach");

static const char *gFileNamePtr = "C:/STAFRegistrationData/STAFReg.dat";
static STAFMutexSem gFileSem;
static STAFString sHelpMsg;

STAFRC_t handleRegister(STAFString &, STAFString &, unsigned int,
                        STAFString &, STAFString &, RegisterServiceData *);

STAFRC_t handleHelp(STAFString &, STAFString &);

STAFRC_t STAFServiceGetLevelBounds(unsigned int levelID,
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

        RegisterServiceData data;
        data.fDebugMode = 0;
        data.fShortName = pInfo->name;
        data.fName = "STAF/Service/";
        data.fName += pInfo->name;

        *pServiceHandle = new RegisterServiceData(data);

        rc = kSTAFOk;
    }
    catch (STAFException &e)
    { e.write();       }
    catch (...)
    { /* Do Nothing */ }

    return rc;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
                         void *pInitInfo, unsigned int initLevel,
                         STAFString_t *pErrorBuffer)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        if (initLevel != 30) return kSTAFInvalidAPILevel;

        RegisterServiceData *pData =
            reinterpret_cast<RegisterServiceData *>(serviceHandle);

        STAFServiceInitLevel30 *pInfo =
            reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);

        rc = STAFHandle::create(pData->fName, pData->fHandlePtr);

        if (rc != kSTAFOk)
            return rc;

        pData->fRegisterParser = STAFCommandParserPtr(new STAFCommandParser,
           STAFCommandParserPtr::INIT);
        pData->fRegisterParser->addOption("REGISTER", 1,
            STAFCommandParser::kValueNotAllowed);

        // Note:  The optional TYPE option is not currently being used, though
        // the STAFReg program includes it in the REGISTER request it submits
        // and passes STAF for its value.  The intent was to allow the
        // registration service could be used for other types of registration
        // in the future.
        pData->fRegisterParser->addOption("TYPE",     1,
            STAFCommandParser::kValueRequired);

        pData->fRegisterParser->addOption("DATA",     1,
            STAFCommandParser::kValueRequired);

        pData->fRegisterParser->addOptionGroup("REGISTER", 1, 1);
        pData->fRegisterParser->addOptionGroup("DATA",     1, 1);

        // Assign the help text string for the service

        sHelpMsg = "*** " + pData->fShortName + " Service Help ***\n\n" +
            "REGISTER [TYPE <Name>] DATA <Data>\n\n" +
            "HELP";
    }
    catch (STAFException &e)
    { e.write();       }
    catch (...)
    { /* Do Nothing */ }

    return rc;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                  void *pRequestInfo, unsigned int reqLevel,
                                  STAFString_t *pResultBuffer)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        if (reqLevel != 30) return kSTAFInvalidAPILevel;

        STAFServiceRequestLevel30 *pInfo =
            reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

        RegisterServiceData *pData =
            reinterpret_cast<RegisterServiceData *>(serviceHandle);

        STAFHandlePtr handlePtr;
        STAFString machine(pInfo->machine);
        STAFString handleName(pInfo->handleName);
        STAFHandle::create(pInfo->handle, handlePtr);
        unsigned int trustLevel = pInfo->trustLevel;
        STAFString request(pInfo->request);

        if (pData->fDebugMode)
        {
            cout << "Machine      : "
                 << machine.toCurrentCodePage()->buffer() << endl;
            cout << "Handle Name : "
                 << handleName.toCurrentCodePage()->buffer() << endl;
            cout << "Handle       : "
                 << pInfo->handle << endl;
            cout << "Trust Level  : "
                 << trustLevel << endl;
            cout << "Request      : "
                 << request.toCurrentCodePage()->buffer() << endl;
        }

        STAFString action = request.subWord(0, 1).toLowerCase();
        STAFString result;

        if (pData->fDebugMode) cout << "ACTION = " << action << endl;

        if (action == "register") 
            rc = handleRegister(machine, handleName, pInfo->handle, request,
                                result, pData);
        else if (action == "help")
            rc = handleHelp(machine, result);

        *pResultBuffer = result.adoptImpl();
    }
    catch (STAFException &e)
    { e.write();       }
    catch (...)
    { /* Do Nothing */ }

    return rc;
}


STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                         void *pTermInfo, unsigned int termLevel,
                         STAFString_t *pErrorBuffer)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        if (termLevel != 0) return kSTAFInvalidAPILevel;

        rc = kSTAFOk;
    }
    catch (STAFException &e)
    { e.write();       }
    catch (...)
    { /* Do Nothing */ }

    return rc;
}


STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                             void *pDestructInfo,
                             unsigned int destructLevel,
                             STAFString_t *pErrorBuffer)
{
    STAFRC_t rc = kSTAFUnknownError;

    try
    {
        if (destructLevel != 0) return kSTAFInvalidAPILevel;

        RegisterServiceData *pData =
            reinterpret_cast<RegisterServiceData *>
            (*serviceHandle);

        delete pData;

        *serviceHandle = 0;

        rc = kSTAFOk;
    }
    catch (STAFException &e)
    { e.write();       }
    catch (...)
    { /* Do Nothing */ }

    return rc;
}

STAFRC_t handleRegister(STAFString &machine, STAFString &handleName,
                        unsigned int handle, STAFString &request,
                        STAFString &result, RegisterServiceData *pData)
{
    STAFRC_t rc = kSTAFOk;

    STAFCommandParseResultPtr parsedResult =
        pData->fRegisterParser->parse(request);

    if (parsedResult->rc != kSTAFOk)
    {
        result = parsedResult->errorBuffer;
        return kSTAFInvalidRequestString;
    }

    STAFString data("machine: " + machine + ";");
    data += parsedResult->optionValue("DATA");

    STAFMutexSemLock theLock(gFileSem);

    fstream theFile(gFileNamePtr, ios::out | ios::app); 

    if (!theFile)
    {
        result = "File " + STAFString(gFileNamePtr) + " not found";
        rc = kSTAFFileOpenError;
    }
    else
    {
        if (pData->fDebugMode)
            cout << "DATA = " << data.toCurrentCodePage()->buffer() << endl;

        theFile << "#" << STAFTimestamp().asString()
                << "##########################################" << endl
                << data.toCurrentCodePage()->buffer() << endl;

        /*
           Examples of data:
        
           machine: xxx;version: 3.4.5;osname: xxx;osmajor: xxx;osminor: xxx;osrev: xxx;name: ;email: ;org: ;
           machine: xxx;version: 3.2.5;osname: xxx;osmajor: xxx;osminor: xxx;osrev: xxx;name:;email:;org:;
           machine: xxx;version: 3.4.0;osname: xxx;osmajor: xxx;osminor: xxx;osrev: xxx;name:svt;email:xxxx@cn.ibm.com;org:ibm;
         */

        // Only generate a STAF Registration Data event if a name or email
        // is specified in the registration data and if it wasn't generated
        // by any of the following sources:
        // - ITSAS which is indicated by:
        //     name: ITSAS;email: SWG_RTP_ITsecurity@atenotes.raleigh.ibm.com
        // - Lotus Automator install of STAF which is indicated by:
        //     name: Bryan Osenbach;email: osenbach@us.ibm.com
        
        bool skip = false;
        bool nameFound = false;
        bool emailFound = false;

        // Check if a non-blank name value was specified in the data

        unsigned int namePos = data.find(sNameString);
             
        if (namePos != STAFString::kNPos)
        {
            unsigned int emailPos = data.find(sEmailString);
            
            if (emailPos == STAFString::kNPos)
                emailPos = data.length() - 1;

            unsigned int startIndex = namePos + sNameString.length();
            unsigned int nameLength = emailPos - startIndex;
                
            if (nameLength > 0)
            {
                STAFString name = data.subString(startIndex, nameLength);
                name = name.strip();

                if (name.length() > 0)
                {
                    if ((name == sSkipName1) || (name == sSkipName2))
                        skip = true;
                    else
                        nameFound = true;
                }
            }

            if (!skip && !nameFound)
            {
                // Check if a non-blank email value was specified

                unsigned int orgPos = data.find(sOrgString);

                if (orgPos == STAFString::kNPos)
                    orgPos = data.length() - 1;
                
                startIndex = emailPos + sEmailString.length();
                unsigned int emailLength = orgPos - startIndex;

                if (emailLength > 0)
                {
                    STAFString email = data.subString(startIndex, emailLength);
                    email = email.strip();

                    if (email.length() > 0)
                        emailFound = true;
                }
            }
        }
        
        if (nameFound || emailFound)
        {
            STAFString property = "data=";
            property += data.toCurrentCodePage()->buffer();

            pData->fHandlePtr->submit(
                "local", "EVENT",
                "GENERATE TYPE STAFREGISTRATIONDATA SUBTYPE STAFREG PROPERTY " +
                STAFHandle::wrapData(property));
        }
    }

    return rc;
}

STAFRC_t handleHelp(STAFString &machine, STAFString &result)
{
    result = sHelpMsg;

    return kSTAFOk;
}
