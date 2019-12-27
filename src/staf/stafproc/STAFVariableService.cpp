/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
 
#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFUtil.h"
#include "STAFVariableService.h"
#include "STAFVariablePool.h"
#include "STAFHandleManager.h"
#include "STAFRequestManager.h"

static STAFString sHelpMsg;

// XXX: Might want to think about being able to create other "system" variable
//      pools by name, and then adding a POOL <Name> option to the above.

STAFVariableService::STAFVariableService() : STAFService("VAR")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** VAR Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SET [SYSTEM | SHARED | HANDLE <Handle>] [FAILIFEXISTS]" +
        *gLineSeparatorPtr +
        "    VAR <Name=Value> [VAR <Name=Value>]..." +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "GET [SYSTEM | SHARED | HANDLE <Handle>] VAR <Name>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DELETE [SYSTEM | SHARED | HANDLE <Handle>] VAR <Name> [VAR <Name>]..." +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST [SYSTEM | SHARED | HANDLE <Handle> | ASHANDLE <Handle> | " +
        *gLineSeparatorPtr +
        "      REQUEST [<Request Number>] ] " +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "RESOLVE [SYSTEM | SHARED | HANDLE <Handle> | ASHANDLE <Handle> | " +
        *gLineSeparatorPtr +
        "         REQUEST [<Request Number>] ] " +
        *gLineSeparatorPtr +
        "        STRING <String> [STRING <String>]... [IGNOREERRORS]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parser

    // var options

    fVarParser.addOption("SET",        1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("GET",        1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("DELETE",     1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("LIST",       1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("RESOLVE",    1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("SYSTEM",     1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("SHARED",     1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("HANDLE",     1,
                         STAFCommandParser::kValueRequired);
    fVarParser.addOption("ASHANDLE",   1,
                         STAFCommandParser::kValueRequired);
    fVarParser.addOption("REQUEST",    1,
                         STAFCommandParser::kValueAllowed); 
    fVarParser.addOption("VAR",        0,
                         STAFCommandParser::kValueRequired); 
    fVarParser.addOption("STRING",     0,
                         STAFCommandParser::kValueRequired);
    fVarParser.addOption("IGNOREERRORS", 1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("FAILIFEXISTS", 1,
                         STAFCommandParser::kValueNotAllowed);
    fVarParser.addOption("HELP",       1,
                         STAFCommandParser::kValueNotAllowed);

    // var groups

    fVarParser.addOptionGroup("SET GET DELETE LIST RESOLVE HELP", 1, 1);
    fVarParser.addOptionGroup("SYSTEM SHARED HANDLE ASHANDLE REQUEST", 0, 1);

    // var needs

    fVarParser.addOptionNeed("SET GET DELETE", "VAR");
    fVarParser.addOptionNeed("VAR", "SET GET DELETE");
    fVarParser.addOptionNeed("REQUEST ASHANDLE", "RESOLVE LIST");
    fVarParser.addOptionNeed("RESOLVE", "STRING");
    fVarParser.addOptionNeed("STRING", "RESOLVE");
    fVarParser.addOptionNeed("IGNOREERRORS", "STRING");
    fVarParser.addOptionNeed("FAILIFEXISTS", "SET");

    // Construct map-class used when an error occurs setting/deleting
    // multiple variables

    fErrorClass = STAFMapClassDefinition::create(
        "STAF/Service/Var/ErrorInfo");
 
    fErrorClass->addKey("name", "Name");
    fErrorClass->addKey("rc", "RC");
    fErrorClass->addKey("result", "Result");

    // Construct map-class for resolve var info

    fResolveStringClass = STAFMapClassDefinition::create(
        "STAF/Service/Var/ResolveString");
 
    fResolveStringClass->addKey("rc", "RC");
    fResolveStringClass->addKey("result", "Result");
}


STAFVariableService::~STAFVariableService()
{
    /* Do Nothing */
}


STAFString STAFVariableService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFVariableService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFCommandParseResultPtr parsedResult = fVarParser.parse(
        requestInfo.fRequest);
 
    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;                  
    STAFRC_t rc = kSTAFOk;

    STAFHandle_t theHandle = 0;    
    STAFRequestNumber_t theRequest = 0;    

    STAFVariablePoolPtr handleVarPoolPtr; 

    // Maximum number of pools that could be used
    const unsigned int MAX_VARIABLE_POOLS = 6;  

    const STAFVariablePool *varPoolListMain[MAX_VARIABLE_POOLS];
    
    unsigned int varPoolListSizeMain = 0;

    if (parsedResult->optionTimes("HANDLE") != 0)
    {
        // Use handle pool only, for all commands    

        rc = RESOLVE_UINT_OPTION_RANGE(
            "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
            gHandleManagerPtr->getMaxHandleNumber());

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = gHandleManagerPtr->variablePool(theHandle, handleVarPoolPtr);
            
        if (rc != kSTAFOk) return STAFServiceResult(rc);
            
        varPoolListMain[varPoolListSizeMain] = handleVarPoolPtr;
        varPoolListSizeMain++;
    }
    else if (parsedResult->optionTimes("SYSTEM") != 0)
    {
        // Use local system pool only, for all commands

        varPoolListMain[varPoolListSizeMain] = requestInfo.fLocalSystemVarPool;
        varPoolListSizeMain++;
    }
    else if (parsedResult->optionTimes("SHARED") != 0)
    {
        // Use local shared pool only, for all commands

        varPoolListMain[varPoolListSizeMain] = requestInfo.fLocalSharedVarPool;
        varPoolListSizeMain++;
    }
    else if (parsedResult->optionTimes("ASHANDLE") != 0)
    {
        // For LIST and resolve commands only
        // Use local handle, local shared, local system pools

        rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
            "ASHANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
            gHandleManagerPtr->getMaxHandleNumber());

        if (rc) return STAFServiceResult(rc, errorBuffer);

        rc = gHandleManagerPtr->variablePool(theHandle, handleVarPoolPtr);

        if (rc != kSTAFOk) return STAFServiceResult(rc);

        varPoolListMain[varPoolListSizeMain] = handleVarPoolPtr;
        varPoolListSizeMain++;

        varPoolListMain[varPoolListSizeMain] = requestInfo.fLocalSharedVarPool;
        varPoolListSizeMain++;

        varPoolListMain[varPoolListSizeMain] = requestInfo.fLocalSystemVarPool;
        varPoolListSizeMain++;
    }
    else if (parsedResult->optionTimes("REQUEST") != 0 
            || (parsedResult->optionTimes("REQUEST") == 0
               && (parsedResult->optionTimes("LIST") != 0 
                  || parsedResult->optionTimes("RESOLVE") != 0)))
    {
        // For LIST and RESOLVE commands only
        // If request was from local system,
        // use local handle, shared, system pools;
        // Else if request was from remote system,
        // use local request, remote shared, local shared, local system pools;
        // If no value or none of the option was specified, 
        // use the request's own request number, itself.

        rc = RESOLVE_OPTIONAL_UINT_OPTION("REQUEST", theRequest);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        if (theRequest == 0)
            theRequest = requestInfo.fRequestNumber;

        STAFRequestManager::RequestMap requestMap = 
                          gRequestManagerPtr->getRequestMapCopy();

        STAFServiceRequestPtr serviceRequest;

        if (requestMap.find(theRequest) != requestMap.end())
        {
            serviceRequest = requestMap[theRequest];
        }
        else
        {
           return STAFServiceResult(
               kSTAFRequestNumberNotFound,
               STAFString("Request number ") + theRequest + " not found");
        }

        // If the request came from local system, 
        // the source shared pool and local shared pool are the same
                
        varPoolListMain[varPoolListSizeMain] = serviceRequest->fRequestVarPool;
        varPoolListSizeMain++;

        varPoolListMain[varPoolListSizeMain] = 
                                          serviceRequest->fSourceSharedVarPool;
        varPoolListSizeMain++;

        varPoolListMain[varPoolListSizeMain] = 
                                           serviceRequest->fLocalSharedVarPool;
        varPoolListSizeMain++;

        varPoolListMain[varPoolListSizeMain] = 
                                           serviceRequest->fLocalSystemVarPool;
        varPoolListSizeMain++;
    }
    else 
    {
        // For SET, GET, DELETE commands only, 
        // (RESOLVE and LIST commands have been handled earlier)
        // When none of the options was specified,
        // If request from local system,
        // Use local handle pool on the current handle number;
        // Else if request from remote system,
        // Use local system pool

        // XXX: May need to re-consider the exact behavior in this case.

        if (requestInfo.fIsLocalRequest)
        {
            // If this is a local system, use the handle's variable pool

            rc = gHandleManagerPtr->variablePool(requestInfo.fHandle, 
                                                 handleVarPoolPtr);

            if (rc != kSTAFOk) return STAFServiceResult(rc);

            varPoolListMain[varPoolListSizeMain] = handleVarPoolPtr;
            varPoolListSizeMain++;
        }
        else
        {
            // If this is a remote system, use the system pool
                
            varPoolListMain[varPoolListSizeMain] = 
                requestInfo.fLocalSystemVarPool;
            varPoolListSizeMain++;
        }
    }

    STAFString result;

    if (parsedResult->optionTimes("SET") != 0)
    {
        // Verify that the requesting machine/user has at least trust level 3

        IVALIDATE_TRUST(3, "SET");

        bool failIfExists = false;

        if (parsedResult->optionTimes("FAILIFEXISTS") != 0)
            failIfExists = true;

        unsigned int numVars = parsedResult->optionTimes("VAR");

        if (numVars == 1)
        {
            STAFString nameAndValue = parsedResult->optionValue("VAR", 1);
            unsigned int equalPos = nameAndValue.find(kUTF8_EQUAL);

            if (equalPos != STAFString::kNPos)
            {
                // For SET command, there is only one pool involved
                rc = ((STAFVariablePool*)varPoolListMain[0])->set(
                    nameAndValue.subString(0, equalPos),
                    nameAndValue.subString(equalPos +
                                           nameAndValue.sizeOfChar(equalPos)),
                    result, failIfExists);
            }
            else
            {
                rc = kSTAFInvalidValue;
                result = "The VAR option's value does not have format "
                    "Name=Value";
            }
        }
        else
        {
            // Create a marshalled list of maps for the result if an error
            // occurs setting one of more variables

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            mc->setMapClassDefinition(fErrorClass->reference());
            STAFObjectPtr outputList = STAFObject::createList();
            STAFRC_t setRC;
            
            for (int i = 1; i <= numVars; ++i)
            {
                STAFString nameAndValue = parsedResult->optionValue("VAR", i);
                unsigned int equalPos = nameAndValue.find(kUTF8_EQUAL);
                STAFString name;

                if (equalPos != STAFString::kNPos)
                {
                    result = "";
                    name = nameAndValue.subString(0, equalPos);

                    // For SET command, there is only one pool involved
                    setRC = ((STAFVariablePool*)varPoolListMain[0])->set(
                        name,
                        nameAndValue.subString(
                            equalPos + nameAndValue.sizeOfChar(equalPos)),
                        result, failIfExists);
                }
                else
                {
                    name = nameAndValue;
                    setRC = kSTAFInvalidValue;
                    result = "The VAR option's value does not have format "
                        "Name=Value";
                }
                
                STAFObjectPtr resultMap = fErrorClass->createInstance();
                resultMap->put("name", name);
                resultMap->put("rc", STAFString(setRC));
                resultMap->put("result", result);

                // Set the overall return code to the first non-zero RC
                if ((setRC != kSTAFOk) && (rc == kSTAFOk))
                    rc = setRC;

                outputList->append(resultMap);
            }

            if (rc != kSTAFOk)
            {
                // An error occurred setting one or more variables so return
                // a marshalled list of the error information in the result

                mc->setRootObject(outputList);
                result = mc->marshall();
            }
        }
    }
    else if (parsedResult->optionTimes("GET") != 0)
    {
        // Verify that the requesting machine/user has at least trust level 2

        IVALIDATE_TRUST(2, "GET");

        if (parsedResult->optionTimes("VAR") == 1)
        {
            STAFString varName = parsedResult->optionValue("VAR");

            // For GET command, there is only one pool involved
        
            rc = varPoolListMain[0]->get(varName, result);

            if (rc == kSTAFVariableDoesNotExist)
            {
                result = "This variable does not exist: " + varName;
            }
        }
        else
        {
            rc = kSTAFInvalidRequestString;
            result = "You may have no more than 1 instance of option VAR";
        }
    }
    else if (parsedResult->optionTimes("DELETE") != 0)
    {
        // Verify that the requesting machine/user has at least trust level 3

        IVALIDATE_TRUST(3, "DELETE");

        unsigned int numVars = parsedResult->optionTimes("VAR");

        if (numVars == 1)
        {
            STAFString varName = parsedResult->optionValue("VAR", 1);

            // For DELETE command, there is only one pool involved
            
            rc = ((STAFVariablePool*)varPoolListMain[0])->del(varName);

            if (rc == kSTAFVariableDoesNotExist)
            {
                result = "This variable does not exist: " + varName;
            }
        }
        else
        {
            // Create a marshalled list of maps for the result if an error
            // occurs deleting one of more variables

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            mc->setMapClassDefinition(fErrorClass->reference());
            STAFObjectPtr outputList = STAFObject::createList();
            STAFRC_t delRC;

            for (int i = 1; (i <= parsedResult->optionTimes("VAR")); ++i)
            {
                STAFString varName = parsedResult->optionValue("VAR", i);
            
                // For DELETE command, there is only one pool involved
            
                delRC = ((STAFVariablePool*)varPoolListMain[0])->del(varName);

                STAFObjectPtr resultMap = fErrorClass->createInstance();
                resultMap->put("name", varName);
                resultMap->put("rc", STAFString(delRC));

                if (delRC == kSTAFVariableDoesNotExist)
                    resultMap->put("result", "This variable does not exist");
                else
                    resultMap->put("result", "");

                // Set the overall return code to the first non-zero RC

                if ((delRC != kSTAFOk) && (rc == kSTAFOk))
                    rc = delRC;

                outputList->append(resultMap);
            }

            if (rc != kSTAFOk)
            {
                // An error occurred deleting one or more variables so return
                // a marshalled list of the error information in the result

                mc->setRootObject(outputList);
                result = mc->marshall();
            }
        }
    }
    else if (parsedResult->optionTimes("LIST") != 0)
    {
        // Verify that the requesting machine/user has at least trust level 2

        IVALIDATE_TRUST(2, "LIST");

        // We construct a result variable pool
        // based on variable pools we constructed before

        STAFVariablePoolPtr resultVarPoolPtr = STAFVariablePoolPtr(
            new STAFVariablePool, STAFVariablePoolPtr::INIT);

        for (int i = varPoolListSizeMain - 1; i >= 0; i--)
        {
            // Put values into result variable pool.
            // Variables in a pool earlier in the list
            // override variables in a pool later in the list

            STAFVariablePool::VariableMap theVarMap;

            theVarMap = ((STAFVariablePool*)varPoolListMain[i])->
                getVariableMapCopy();

            STAFVariablePool::VariableMap::iterator theIter;

            for (theIter = theVarMap.begin();
                 theIter != theVarMap.end(); theIter++)
            {
                const STAFVariablePool::Variable &var = theIter->second;

                resultVarPoolPtr->set(var.name, var.value);    
            }
        }            

        // Build the output based on the result variable pool

        STAFVariablePool::VariableMap theVarMap = 
            resultVarPoolPtr->getVariableMapCopy();

        STAFVariablePool::VariableMap::iterator theIter;
                                   
        // Construct map-class for listing variables - its keys will be
        // dynamically generated

        STAFMapClassDefinitionPtr varInfoClass =
            STAFMapClassDefinition::create("STAF/Service/Var/VarInfo");
                                   
        // Create a marshalled map to contain the STAF variables

        STAFObjectPtr mc = STAFObject::createMarshallingContext();
        mc->setMapClassDefinition(varInfoClass->reference());
        STAFObjectPtr varInfoMap = varInfoClass->createInstance();

        for (theIter = theVarMap.begin();
             theIter != theVarMap.end(); theIter++)
        {
            const STAFVariablePool::Variable &var = theIter->second;

            // Dynamically generate the map keys for the map class definition
            varInfoClass->addKey(STAFString(var.name), STAFString(var.name));

            varInfoMap->put(STAFString(var.name), STAFString(var.value));
        }        

        mc->setRootObject(varInfoMap);
        result = mc->marshall();
    }
    else if (parsedResult->optionTimes("RESOLVE") != 0)
    {
        // Verify that the requesting machine/user has at least trust level 3

        IVALIDATE_TRUST(3, "RESOLVE");

        // Determine whether to ignore variable resolution errors such as
        // if the variable does not exist, or a { without a matching } 

        bool ignoreErrors = false;

        if (parsedResult->optionTimes("IGNOREERRORS") > 0)
            ignoreErrors = true;

        unsigned int numTimes = parsedResult->optionTimes("STRING");

        if (numTimes == 1)
        {
            rc = STAFVariablePool::resolve(
                parsedResult->optionValue("STRING"), varPoolListMain,
                varPoolListSizeMain, result, ignoreErrors);
        }
        else
        {
            // Create a marshalled list of maps containing resolved variable
            // data

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            mc->setMapClassDefinition(fResolveStringClass->reference());
            STAFObjectPtr outputList = STAFObject::createList();

            for (int i = 1; i <= numTimes; ++i)
            {
                STAFObjectPtr resolveVarMap =
                    fResolveStringClass->createInstance();

                rc = STAFVariablePool::resolve(
                    parsedResult->optionValue("STRING", i), varPoolListMain,
                    varPoolListSizeMain, result, ignoreErrors);

                resolveVarMap->put("rc", STAFString(rc));
                resolveVarMap->put("result", result);

                outputList->append(resolveVarMap);
            }

            rc = kSTAFOk;

            mc->setRootObject(outputList);
            result = mc->marshall();
        }
    }
    else
    {
        // Verify that the requesting machine/user has at least trust level 1

        IVALIDATE_TRUST(1, "HELP");

        return STAFServiceResult(kSTAFOk, sHelpMsg);
    }

    return STAFServiceResult(rc, result);
}
