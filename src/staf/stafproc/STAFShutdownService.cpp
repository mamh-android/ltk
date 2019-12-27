/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFShutdownService.h"
#include "STAFNotificationList.h"
#include "STAFHandleManager.h"
#include "STAFUtil.h"

static STAFString sHelpMsg;

STAFShutdownService::STAFShutdownService() : STAFService("SHUTDOWN")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** SHUTDOWN Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SHUTDOWN" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "NOTIFY REGISTER   [MACHINE <Machine>] [HANDLE <Handle> | NAME <Name>]" +
        *gLineSeparatorPtr +
        "                  [PRIORITY <Priority>]" +
        *gLineSeparatorPtr +
        "NOTIFY UNREGISTER [MACHINE <Machine>] [HANDLE <Handle> | NAME <Name>]" +
        *gLineSeparatorPtr +
        "                  [PRIORITY <Priority>]" +
        *gLineSeparatorPtr +
        "NOTIFY LIST" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // Create the SHUTDOWN command request parser

    fShutdownParser.addOption("SHUTDOWN", 1,
                              STAFCommandParser::kValueNotAllowed);

    fShutdownParser.addOptionGroup("SHUTDOWN", 1, 1);

    // Create the NOTIFY command request parser

    fNotifyParser.addOption("NOTIFY", 1,
                            STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("REGISTER", 1,
                            STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("UNREGISTER", 1,
                            STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("LIST", 1,
                            STAFCommandParser::kValueNotAllowed);
    fNotifyParser.addOption("MACHINE", 1,
                            STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("HANDLE", 1,
                            STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("NAME", 1,
                            STAFCommandParser::kValueRequired);
    fNotifyParser.addOption("PRIORITY",   1,
                            STAFCommandParser::kValueRequired);

    fNotifyParser.addOptionGroup("NOTIFY", 1, 1);
    fNotifyParser.addOptionGroup("HANDLE NAME", 0, 1);

    fNotifyParser.addOptionNeed("NOTIFY", "REGISTER UNREGISTER LIST");
    fNotifyParser.addOptionNeed("REGISTER UNREGISTER LIST", "NOTIFY");
    fNotifyParser.addOptionNeed("MACHINE HANDLE NAME", "REGISTER UNREGISTER");

    // Construct map class for shutdown notifiee information

    fNotifieeClass = STAFMapClassDefinition::create(
        "STAF/Service/Shutdown/Notifiee");
 
    fNotifieeClass->addKey("priority", "Priority");
    fNotifieeClass->addKey("machine", "Machine");
    fNotifieeClass->addKey("notifyBy", "Notify By");
    fNotifieeClass->addKey("notifiee", "Notifiee");
}


STAFString STAFShutdownService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFShutdownService::~STAFShutdownService()
{
    /* Do Nothing */
}


STAFServiceResult STAFShutdownService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();
 
    if (action == "shutdown") return handleShutdown(requestInfo);
    else if (action == "notify") return handleNotify(requestInfo);
    else if (action == "help") return handleHelp(requestInfo);
    else
    {
        STAFString errMsg = STAFString("'") +
            requestInfo.fRequest.subWord(0, 1) +
            "' is not a valid command request for the " + name() +
            " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
            sHelpMsg;

        return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
    }
}


STAFServiceResult STAFShutdownService::handleShutdown(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "SHUTDOWN");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fShutdownParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    return STAFServiceResult(kSTAFOk, "", 1);
}

STAFServiceResult STAFShutdownService::handleNotify(
    const STAFServiceRequest &requestInfo)
{
    // Parse the request

    STAFCommandParseResultPtr parsedResult =
        fNotifyParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    if (parsedResult->optionTimes("LIST") > 0)
    {
        // Verify that the requesting machine/user has at least trust level 2

        IVALIDATE_TRUST(2, "NOTIFY LIST");

        // Create a marshalled list of maps containing information about
        // the shutdown notifiees.

        STAFObjectPtr mc = STAFObject::createMarshallingContext();
        mc->setMapClassDefinition(fNotifieeClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();

        STAFNotificationList::NotifieeList notifieeList(
            gNotifyOnShutdownPtr->getNotifieeListCopy());

        STAFNotificationList::NotifieeList::iterator iter;

        for (iter = notifieeList.begin(); iter != notifieeList.end(); iter++)
        {
            STAFNotificationList::Notifiee notifiee = *iter;

            STAFObjectPtr notifieeMap = fNotifieeClass->createInstance();
            notifieeMap->put("priority", STAFString(notifiee.priority));
            notifieeMap->put("machine", STAFString(notifiee.machine));

            if (notifiee.nameOrHandle ==
                STAFNotificationList::Notifiee::kName)
            {
                notifieeMap->put("notifyBy", STAFString("Name"));
                notifieeMap->put("notifiee", STAFString(notifiee.process));
            }
            else
            {
                notifieeMap->put("notifyBy", STAFString("Handle"));
                notifieeMap->put("notifiee", STAFString(notifiee.handle));
            }

            outputList->append(notifieeMap);
        }

        mc->setRootObject(outputList);

        return STAFServiceResult(kSTAFOk, mc->marshall());
    }
    
    // Handle REGISTER and UNREGISTER requests

    bool isRegister;

    // Verify that the requesting machine/user has at least trust level 3

    if (parsedResult->optionTimes("REGISTER") > 0)
    {
        isRegister = true;
        IVALIDATE_TRUST(3, "REGISTER");
    }
    else
    {
        isRegister = false;
        IVALIDATE_TRUST(3, "UNREGISTER");
    }
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);

    // Set the default values for handle#, machine, and priority

    unsigned int theHandle = requestInfo.fHandle;
    STAFString machine = requestInfo.fEndpoint;
    unsigned int priority = 5;

    STAFString errorBuffer;
    STAFRC_t rc = kSTAFOk;

    unsigned int useName = 0;  // Default to using handle# not handle name
    STAFString name;

    // Check if the HANDLE option or NAME option were specified and if so,
    // resolve any variables in the the option values

    if (parsedResult->optionTimes("HANDLE") > 0)
    {
        // Get the handle number value and verify that it is valid

        rc = RESOLVE_UINT_OPTION_RANGE(
            "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
             gHandleManagerPtr->getMaxHandleNumber());
    }
    else if (parsedResult->optionTimes("NAME") > 0)
    {
        useName = 1;

        rc = RESOLVE_STRING_OPTION("NAME", name);
    }

    if (!rc) rc = RESOLVE_OPTIONAL_UINT_OPTION("PRIORITY", priority);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("MACHINE", machine);

    if (rc) return STAFServiceResult(rc, errorBuffer);
    
    if (isRegister)
    {
        if (useName)
            rc = gNotifyOnShutdownPtr->reg(machine, name, priority);
        else
            rc = gNotifyOnShutdownPtr->reg(machine, theHandle, priority);
    }
    else
    {
        if (useName)
        {
            rc = gNotifyOnShutdownPtr->unregister(machine, name, priority);

            if (rc == kSTAFNotifieeDoesNotExist)
            {
                errorBuffer = "A notification does not exist in the shutdown "
                    "notification list for machine '" + machine +
                    "', handle name '" + name + "' and priority '" +
                    STAFString(priority) + "'";
            }
        }
        else
        {
            rc = gNotifyOnShutdownPtr->unregister(machine, theHandle, 
                                                  priority);

            if (rc == kSTAFNotifieeDoesNotExist)
            {
                errorBuffer = "A notification does not exist in the shutdown "
                    "notification list for machine '" + machine +
                    "', handle number '" + STAFString(theHandle) +
                    "', and priority " + STAFString(priority) + "'";
            }
        }
    }

    return STAFServiceResult(rc, errorBuffer);
}


STAFServiceResult STAFShutdownService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
