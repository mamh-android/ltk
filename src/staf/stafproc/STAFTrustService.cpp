/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFTrustService.h"
#include "STAFCommandParser.h"
#include "STAFTrustManager.h"
#include "STAFConnectionManager.h"

static STAFString sHelpMsg;

STAFTrustService::STAFTrustService() : STAFService("TRUST")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** TRUST Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SET <MACHINE <Machine> | USER <User> | DEFAULT> LEVEL <Level>" +
        *gLineSeparatorPtr +
        "GET MACHINE <Machine> [USER <User>]" +
        *gLineSeparatorPtr +
        "DELETE MACHINE <Machine> | USER <User>" +
        *gLineSeparatorPtr +
        "LIST" +
        *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // get options
 
    fGetParser.addOption("GET",     1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("MACHINE", 1,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("USER", 1, 
        STAFCommandParser::kValueRequired);
                                           
    // get groups
 
    fGetParser.addOptionGroup("MACHINE", 1, 1);
    fGetParser.addOptionNeed("USER", "MACHINE");
 
    // set options

    fSetParser.addOption("SET",     1, 
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("MACHINE", 1, 
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("USER", 1, 
        STAFCommandParser::kValueRequired);
    fSetParser.addOption("DEFAULT", 1, 
        STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("LEVEL",   1, 
        STAFCommandParser::kValueRequired);
 
    // set groups
 
    fSetParser.addOptionGroup("MACHINE USER DEFAULT", 1, 1);
    fSetParser.addOptionGroup("LEVEL", 1, 1);
 
    // delete options
 
    fDeleteParser.addOption("DELETE",  1, 
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("MACHINE", 1, 
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("USER", 1, 
        STAFCommandParser::kValueRequired);
 
    // delete groups
 
    fDeleteParser.addOptionGroup("MACHINE USER", 1, 1);
 
    // list options
 
    fListParser.addOption("LIST", 1,
        STAFCommandParser::kValueNotAllowed);

    // Construct map class for all trust information

    fTrustEntryClass = STAFMapClassDefinition::create(
        "STAF/Service/Trust/Entry");

    fTrustEntryClass->addKey("type", "Type");
    fTrustEntryClass->addKey("entry", "Entry");
    fTrustEntryClass->addKey("trustLevel", "Trust Level");
}


STAFTrustService::~STAFTrustService()
{
    /* Do Nothing */
}


STAFString STAFTrustService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFTrustService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();
 
    if (action == "set")
        return handleSet(requestInfo);
    else if (action == "get")
        return handleGet(requestInfo);
    else if (action == "delete")
        return handleDelete(requestInfo);
    else if (action == "list")
        return handleList(requestInfo);
    else if (action == "help") 
        return handleHelp(requestInfo);
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


STAFServiceResult STAFTrustService::handleSet(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "SET");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fSetParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
 
    // Resolve any variables in the LEVEL option and convert to an
    // unsigned integer in the range of 0 - maxTrustLevel

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    unsigned int level;

    STAFRC_t rc = RESOLVE_UINT_OPTION_RANGE(
        "LEVEL", level, 0, gTrustManagerPtr->getMaxTrustLevel());

    if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);
    
    // Set the trust level

    if (parsedResult->optionTimes("MACHINE") != 0)
    {
        // Resolve any variables in the MACHINE option

        STAFString machine;
        rc = RESOLVE_STRING_OPTION("MACHINE", machine);

        if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);

        // Set the trust level for the machine

        return STAFServiceResult(
            gTrustManagerPtr->setMachineTrusteeLevel(machine, level));
    }
    else if (parsedResult->optionTimes("USER") != 0)
    {
        // Resolve any variables in the USER option

        STAFString user;
        rc = RESOLVE_STRING_OPTION("USER", user);

        if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);

        // Set the trust level for the user

        return STAFServiceResult(
            gTrustManagerPtr->setUserTrusteeLevel(user, level));
    }
    else
    {
        return STAFServiceResult(
            gTrustManagerPtr->setDefaultTrusteeLevel(level));
    }
}


STAFServiceResult STAFTrustService::handleGet(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "GET");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fGetParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
    
    // Resolve any variables in the MACHINE option

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString machine;
    
    STAFRC_t rc = RESOLVE_STRING_OPTION("MACHINE", machine);

    if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);
    
    // If an interface is not specified, prepend the machine value
    // with the default interface name and "://"

    unsigned int sepPos = machine.find(gSpecSeparator);

    if (sepPos == STAFString::kNPos)
    {
        machine = gConnectionManagerPtr->getDefaultConnectionProvider() +
            gSpecSeparator + machine;
    }

    if (parsedResult->optionTimes("USER") != 0)
    {
        // Resolve any variables in the USER option

        STAFString user;
        rc = RESOLVE_STRING_OPTION("USER", user);

        if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);

        if (user.hasWildcard())
        {
            return STAFServiceResult(kSTAFInvalidValue,
                   STAFString("User " + user +
                              " cannot contain wildcard characters"));
        }
        else
        {
            return STAFServiceResult(kSTAFOk, gTrustManagerPtr->getTrustLevel(
                                     machine, user));
        }
    }

    else
    {
        return STAFServiceResult(kSTAFOk,
                                 gTrustManagerPtr->getTrustLevel(machine));
    }
}


STAFServiceResult STAFTrustService::handleDelete(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "DELETE");
 
    STAFCommandParseResultPtr parsedResult = fDeleteParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    
    if (parsedResult->optionTimes("MACHINE") != 0)
    {
        // Resolve any variables in the MACHINE option

        STAFString machine;
        STAFRC_t rc = RESOLVE_STRING_OPTION("MACHINE", machine);

        if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);
        
        // Delete the machine trust

        return STAFServiceResult(
            gTrustManagerPtr->deleteMachineTrustee(machine));
    }
    else
    {
        // Resolve any variables in the USER option

        STAFString user;
        STAFRC_t rc = RESOLVE_STRING_OPTION("USER", user);

        if (rc != kSTAFOk) return STAFServiceResult(rc, errorBuffer);

        // Delete the user trust

        return STAFServiceResult(
            gTrustManagerPtr->deleteUserTrustee(user));
    }
}


STAFServiceResult STAFTrustService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    // Create a marshalled list of maps containing trust entries

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fTrustEntryClass->reference());
    STAFObjectPtr trustEntryList = STAFObject::createList();

    // Add an entry for the default trust to the list
        
    STAFObjectPtr trustEntryMap = fTrustEntryClass->createInstance();
    trustEntryMap->put("type", "Default");
    trustEntryMap->put("entry",  STAFObject::createNone());
    trustEntryMap->put("trustLevel", STAFString(gTrustManagerPtr->
                                                getDefaultTrusteeLevel()));
    trustEntryList->append(trustEntryMap);

    // Iterate through the list of machine trusts and add an entry
    // for each machine to the trustEntryList

    STAFTrustManager::TrustMap trustMap = 
        gTrustManagerPtr->getMachineTrustMapCopy();
    STAFTrustManager::TrustMap::iterator iter;

    if (trustMap.size() != 0)
    {
        for (iter = trustMap.begin(); iter != trustMap.end(); iter++)
        {
            STAFObjectPtr trustEntryMap = fTrustEntryClass->createInstance();
            
            trustEntryMap->put("type", "Machine");
            trustEntryMap->put("entry",
                               iter->second.group + gSpecSeparator +
                               iter->second.entity);
            trustEntryMap->put("trustLevel",
                               STAFString(iter->second.trustLevel));

            trustEntryList->append(trustEntryMap);
        }
    }

    // Iterate through the list of user trusts and add an entry
    // for each user to the trustEntryList

    trustMap = gTrustManagerPtr->getUserTrustMapCopy();

    if (trustMap.size() != 0)
    {
        for (iter = trustMap.begin(); iter != trustMap.end(); iter++)
        {
            STAFObjectPtr trustEntryMap = fTrustEntryClass->createInstance();

            trustEntryMap->put("type", "User");
            trustEntryMap->put("entry",
                               iter->second.group + gSpecSeparator +
                               iter->second.entity);
            trustEntryMap->put("trustLevel",
                               STAFString(iter->second.trustLevel));

            trustEntryList->append(trustEntryMap);
        }
    }

    mc->setRootObject(trustEntryList);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFTrustService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
