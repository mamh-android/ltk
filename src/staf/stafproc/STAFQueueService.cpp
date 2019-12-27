/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFQueueService.h"
#include "STAFHandleQueue.h"
#include "STAFHandleManager.h"
#include "STAFVariablePool.h"
#include "STAFUtil.h"
// XXX: Remove STAFServiceManager.h if pass in default authenticator via
//      LocalRequest()
#include "STAFServiceManager.h"

static const STAFString sColon(kUTF8_COLON);
static STAFString sHelpMsg;

STAFQueueService::STAFQueueService() : STAFService("QUEUE")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** QUEUE Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "QUEUE  MESSAGE <Message>" +
        *gLineSeparatorPtr +
        "       [HANDLE <Handle>] | [NAME <Name>] [PRIORITY <Priority>] [TYPE <Type>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "GET    [PRIORITY <Priority>]... [MACHINE <Endpoint>]... [NAME <Name>]..." +
        *gLineSeparatorPtr +
        "       [HANDLE <Handle>]... [USER <User>]... [TYPE <Type>]..." +
        *gLineSeparatorPtr +
        "       [CONTAINS <String>]... [ICONTAINS <String>]... " +
        *gLineSeparatorPtr +
        "       [FIRST <Number> | ALL]" +
        *gLineSeparatorPtr +
        "       [WAIT [<Number>[s|m|h|d|w]]]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "PEEK   [PRIORITY <Priority>]... [MACHINE <Endpoint>]... [NAME <Name>]..." +
        *gLineSeparatorPtr +
        "       [HANDLE <Handle>]... [USER <User>]... [TYPE <Type>]..." +
        *gLineSeparatorPtr +
        "       [CONTAINS <String>]... [ICONTAINS <String>]... " +
        *gLineSeparatorPtr +
        "       [FIRST <Number> | ALL]" +
        *gLineSeparatorPtr +
        "       [WAIT [<Number>[s|m|h|d|w]]]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DELETE [PRIORITY <Priority>]... [MACHINE <Endpoint>]... [NAME <Name>]..." +
        *gLineSeparatorPtr +
        "       [HANDLE <Handle>]... [USER <User>]... [TYPE <Type>]..." +
        *gLineSeparatorPtr +
        "       [CONTAINS <String>]... [ICONTAINS <String>]... " +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST   [HANDLE <Handle>]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // queue options

    fQueueParser.addOption("QUEUE",    1,
        STAFCommandParser::kValueNotAllowed);
    fQueueParser.addOption("HANDLE",   1,
        STAFCommandParser::kValueRequired);
    fQueueParser.addOption("NAME",     1,
        STAFCommandParser::kValueRequired);
    fQueueParser.addOption("PRIORITY", 1,
        STAFCommandParser::kValueRequired);
    fQueueParser.addOption("MESSAGE",  1,
        STAFCommandParser::kValueRequired);
    fQueueParser.addOption("TYPE",  1,
        STAFCommandParser::kValueRequired);

    // queue option groups

    fQueueParser.addOptionGroup("HANDLE NAME", 0, 1);
    fQueueParser.addOptionGroup("MESSAGE",     1, 1);

    // get / peek options

    fGetParser.addOption("GET",       1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("PEEK",      1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("PRIORITY",  0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("MACHINE",   0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("NAME",      0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("HANDLE",    0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("USER",      0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("TYPE",      0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("CONTAINS",  0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("ICONTAINS", 0,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("WAIT",      1,
        STAFCommandParser::kValueAllowed);
    fGetParser.addOption("FIRST",      1,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("ALL",      1,
        STAFCommandParser::kValueNotAllowed);
    
    fGetParser.addOptionGroup("FIRST ALL", 0, 1);

    // delete options

    fDeleteParser.addOption("DELETE",    1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("PRIORITY",  0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("MACHINE",   0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("NAME",      0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("HANDLE",    0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("USER",      0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("TYPE",      0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("CONTAINS",  0,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("ICONTAINS", 0,
        STAFCommandParser::kValueRequired);

    // list options

    fListParser.addOption("LIST",   1,
        STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("HANDLE", 1,
        STAFCommandParser::kValueRequired);

    // Construct map class for an entry in a queue

    fQueueEntryMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Queue/Entry");

    fQueueEntryMapClass->addKey("priority",   "Priority");
    fQueueEntryMapClass->setKeyProperty("priority", "display-short-name", "P");
    fQueueEntryMapClass->addKey("timestamp",  "Date-Time");
    fQueueEntryMapClass->addKey("machine",    "Machine");
    fQueueEntryMapClass->addKey("handleName", "Handle Name");
    fQueueEntryMapClass->setKeyProperty(
        "handleName", "display-short-name", "Name");
    fQueueEntryMapClass->addKey("handle",     "Handle");
    fQueueEntryMapClass->setKeyProperty("handle", "display-short-name", "H#");
    fQueueEntryMapClass->addKey("user",       "User");
    fQueueEntryMapClass->addKey("type",       "Type");
    fQueueEntryMapClass->addKey("message",    "Message");

    // Construct map classes for handles with a full queue error

    fQueueFullMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Queue/FullInfo");
    fQueueFullMapClass->addKey("handle", "Handle");
    fQueueFullMapClass->addKey("queueSize", "Queue Size");

    fQueueErrorMapClass = STAFMapClassDefinition::create(
        "STAF/Service/Queue/Error");
    fQueueErrorMapClass->addKey("numberQueued", "Number Queued");
    fQueueErrorMapClass->addKey("fullQueueList", "Handles with Full Queues");
}


STAFQueueService::~STAFQueueService()
{
    /* Do Nothing */
}


STAFString STAFQueueService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFQueueService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();

    if      (action == "queue")  return handleQueue(requestInfo);
    else if (action == "get")    return handleGetPeek(requestInfo, true);
    else if (action == "peek")   return handleGetPeek(requestInfo, false);
    else if (action == "delete") return handleDelete(requestInfo);
    else if (action == "list")   return handleList(requestInfo);
    else if (action == "help")   return handleHelp(requestInfo);
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


STAFServiceResult STAFQueueService::handleQueue(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 3

    IVALIDATE_TRUST(3, "QUEUE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fQueueParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFString errorBuffer;

    // HANDLE may only default if the request came from this machine

    if ((parsedResult->optionTimes("HANDLE") == 0) &&
        (parsedResult->optionTimes("NAME") == 0) &&
        !requestInfo.fIsLocalRequest)
    {
        return STAFServiceResult(kSTAFInvalidRequestString);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString name;
    STAFHandle_t theHandle = requestInfo.fHandle;
    unsigned int priority = 5;
    STAFString type;

    STAFRC_t rc = RESOLVE_OPTIONAL_STRING_OPTION("NAME", name);

    if (!rc) rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
        "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (!rc) rc = RESOLVE_OPTIONAL_UINT_OPTION("PRIORITY", priority);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("TYPE", type);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Currently we won't resolve variables in MESSAGE

    STAFHandleQueuePtr handleQueue;
    STAFString machine = requestInfo.fEndpoint;

    STAFString result;

    STAFHandleQueue::Message message(priority, machine,
                                     requestInfo.fAuthenticator,
                                     requestInfo.fUserIdentifier,
                                     requestInfo.fHandleName,
                                     requestInfo.fHandle,
                                     parsedResult->optionValue("MESSAGE"),
                                     type);

    if (parsedResult->optionTimes("NAME") == 0)
    {
        rc = gHandleManagerPtr->handleQueue(theHandle, handleQueue);

        if (rc) return STAFServiceResult(rc);

        STAFMutexSemLock queueLock(handleQueue->fQueueSem);

        if (handleQueue->fQueue.size() >= gMaxQueueSize)
        {
            return STAFServiceResult(kSTAFQueueFull,
                                     STAFString(handleQueue->fQueue.size()));
        }

        handleQueue->fQueue.insert(STAFHandleQueue::HandleQueue::value_type(
            message.priority, message));
        handleQueue->fNotify->post();
    }
    else
    {
        std::vector<STAFHandle_t> handles =
                                  gHandleManagerPtr->handlesWithName(name);

        std::vector<STAFHandle_t>::iterator iter;

        unsigned int numberQueued = handles.size();

        STAFObjectPtr fullQueueErrorList = STAFObject::createList();

        for (iter = handles.begin(); iter != handles.end(); iter++)
        {
            if (gHandleManagerPtr->handleQueue(*iter, handleQueue))
                --numberQueued;
            else
            {
                STAFMutexSemLock queueLock(handleQueue->fQueueSem);

                if (handleQueue->fQueue.size() >= gMaxQueueSize)
                {
                    --numberQueued;

                    // Add information about the handle's full queue to the
                    // error list

                    STAFObjectPtr errorMap = fQueueFullMapClass->createInstance();
                    errorMap->put("handle", STAFString(*iter));
                    errorMap->put("queueSize", handleQueue->fQueue.size());
                    fullQueueErrorList->append(errorMap);
                }
                else
                {
                    handleQueue->fQueue.insert(
                    STAFHandleQueue::HandleQueue::value_type(
                        message.priority, message));
                    handleQueue->fNotify->post();
                }
            }
        }
        
        if (fullQueueErrorList->size() > 0)
        {
            // At least one message was not successfully queued

            // Create a marshalled map containing error information including
            // the number of handles that the message was successfully queued
            // to and a list of the handles where the message could not be
            // queued because the queues were full

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            mc->setMapClassDefinition(fQueueErrorMapClass->reference());
            mc->setMapClassDefinition(fQueueFullMapClass->reference());
            STAFObjectPtr errorMap = fQueueErrorMapClass->createInstance();
            errorMap->put("numberQueued", STAFString(numberQueued));
            errorMap->put("fullQueueList", fullQueueErrorList);

            mc->setRootObject(errorMap);

            return STAFServiceResult(kSTAFQueueFull, mc->marshall());
        }
        else if (numberQueued == 0)
        {
            // No handles exist with the specified handle name

            return STAFServiceResult(kSTAFHandleDoesNotExist);
        }

        result = STAFString(numberQueued);
    }

    return STAFServiceResult(kSTAFOk, result);
}


STAFServiceResult STAFQueueService::handleGetPeek(
    const STAFServiceRequest &requestInfo, bool isGet)
{
    // Verify that this request came from the local machine
    // No check of trust is needed since these commands are only valid
    // with respect to the submitting process' queue so just set
    // required trust level to 0.

    if (isGet)
    {
        IVALIDATE_LOCAL_TRUST(0, "GET");
    }
    else
    {
        IVALIDATE_LOCAL_TRUST(0, "PEEK");
    }

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fGetParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFHandleQueuePtr handleQueue;
    STAFRC_t rc = gHandleManagerPtr->handleQueue(requestInfo.fHandle,
                                                 handleQueue);
    if (rc) return STAFServiceResult(rc);

    // Check if the ALL or FIRST option was specified

    bool first = false;  // Indicates if the FIRST option is specified
    bool all = false;    // Indicates if the ALL option is specified
    unsigned int firstNum = 0;  // Number of queue entries to obtain

    if (parsedResult->optionTimes("ALL") > 0)
    {
        all = true;
    }
    else if (parsedResult->optionTimes("FIRST") > 0)
    {
        rc = RESOLVE_UINT_OPTION_RANGE("FIRST", firstNum, 1, UINT_MAX);
        
        if (rc) return STAFServiceResult(rc, errorBuffer);

        first = true;
    }
    else
    {
        // Default to FIRST 1 if neither the ALL or FIRST option is
        // specified

        firstNum = 1;
    }

    std::vector<unsigned int> priorityList;
    std::vector<unsigned int> handleList;
    std::vector<STAFString> machineList;
    std::vector<STAFString> nameList;
    std::vector<STAFString> userList;
    std::vector<STAFString> typeList;
    std::vector<STAFString> containsList;
    std::vector<STAFString> icontainsList;

    unsigned int index = 0;
    unsigned int optionCount = 0;
    unsigned int priority = 0;

    for (index = 1, optionCount = parsedResult->optionTimes("PRIORITY");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_UINT_OPTION("PRIORITY", index, priority);
        if (!rc) priorityList.push_back(priority);
    }

    STAFHandle_t theHandle = 0;

    for (index = 1, optionCount = parsedResult->optionTimes("HANDLE");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_UINT_OPTION_RANGE(
            "HANDLE", index, theHandle,
            gHandleManagerPtr->getMinHandleNumber(),
            gHandleManagerPtr->getMaxHandleNumber());

        if (!rc) handleList.push_back(theHandle);
    }

    STAFString machine;

    for (index = 1, optionCount = parsedResult->optionTimes("MACHINE");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("MACHINE", index, machine);
        if (!rc) machineList.push_back(machine.toLowerCase());
    }

    STAFString name;

    for (index = 1, optionCount = parsedResult->optionTimes("NAME");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("NAME", index, name);
        if (!rc) nameList.push_back(name.toLowerCase());
    }

    STAFString user;

    for (index = 1, optionCount = parsedResult->optionTimes("USER");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("USER", index, user);

        if (!rc)
        {
            // Check if authenticator was specified in the user value

            unsigned int sepIndex = user.find(gSpecSeparator);

            if (sepIndex == STAFString::kNPos)
            {
                // No authenticator specified
                // Use lower-cased default authenticator
                user = gServiceManagerPtr->getDefaultAuthenticator().
                    toLowerCase() + gSpecSeparator + user;
            }
            else
            {
                // User specified in form of Authenticator://UserIdentifier
                // Change authenticator to lower-case.
                user = user.subString(0, sepIndex).toLowerCase() +
                       user.subString(sepIndex);
            }

            userList.push_back(user);
        }
    }

    STAFString type;

    for (index = 1, optionCount = parsedResult->optionTimes("TYPE");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("TYPE", index, type);
        if (!rc) typeList.push_back(type.toLowerCase());
    }
    
    STAFString contains;

    for (index = 1, optionCount = parsedResult->optionTimes("CONTAINS");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("CONTAINS", index, contains);
        if (!rc) containsList.push_back(contains);
    }

    STAFString icontains;

    for (index = 1, optionCount = parsedResult->optionTimes("ICONTAINS");
         (index <= optionCount) && !rc; ++index)
    {
        rc = RESOLVE_INDEXED_STRING_OPTION("ICONTAINS", index, icontains);
        if (!rc) icontainsList.push_back(icontains.toLowerCase());
    }

    // Set the timeout variable.  If no WAIT option is specified, don't wait
    // for a message to appear on the queue.

    unsigned int timeout = 0;

    if (!rc) rc = RESOLVE_DEFAULT_DURATION_OPTION(
        "WAIT", timeout, STAF_EVENT_SEM_INDEFINITE_WAIT);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFHandleQueue::Message message;
    unsigned int doWait = 0;
    STAFEventSemPtr event;

    if (parsedResult->optionTimes("WAIT") != 0)
    {
        STAFMutexSemLock queueLock(handleQueue->fQueueSem);

        event = handleQueue->fNotify;
        doWait = 1;
    }

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fQueueEntryMapClass->reference());

    STAFObjectPtr resultList;

    if (all | first)
    {
        // A list of queue elements that match the search criteria will
        // be returned

        resultList = STAFObject::createList();
    }

    // The number of elements in the queue that match the search criteria
    unsigned int numFound = 0;
 
    STAFTimestamp startTime = STAFTimestamp::now();
    unsigned int currTimeout = timeout;
    bool retry = true;

    // We enter into a loop here waiting for one or more queue element(s)
    // that match the search criteria to become available
    
    while (retry)
    {
        // Create a block so that the lock on the handle's queue is only
        // held while within this block
        {
            // Get a lock on the handle's queue while getting/peeking
            // messages from it

            STAFMutexSemLock queueLock(handleQueue->fQueueSem);

            bool done = false;  // Indicates if done iterating the queue

            // Used if isGet to keep track of the number of entries in the
            // queue to skip since they have already been processed (to speed
            // up things if not all queue elements match the criteria
            unsigned int entriesToSkip = 0;

            while (!done && (all || (numFound < firstNum)))
            {
                unsigned int foundIt = 0;

                STAFHandleQueue::HandleQueue::iterator iter;
                unsigned int i = 0;
                bool readQueueFromBeginning = true;

                for (iter = handleQueue->fQueue.begin();
                     (iter != handleQueue->fQueue.end()) &&
                      ((isGet && !foundIt) ||
                       (!isGet && (all || (numFound < firstNum)))); ++iter)
                {
                    if (isGet)
                    {
                        // Skip entries that we already know don't match

                        if (readQueueFromBeginning)
                        {
                            if (i < entriesToSkip)
                            {
                                ++i;
                                continue;
                            }
                            else
                            {
                                readQueueFromBeginning = false;
                                entriesToSkip = 0;
                            }
                        }
                    }

                    foundIt = 1;

                    if (priorityList.size() != 0)
                    {
                        unsigned int checkPriority = iter->second.priority;

                        for (foundIt = 0, index = 0;
                             index < priorityList.size(); ++index)
                        {
                            if (priorityList[index] == checkPriority)
                                foundIt = 1;
                        }
                    }

                    if (foundIt && (handleList.size() != 0))
                    {
                        STAFHandle_t checkHandle = iter->second.handle;

                        for (foundIt = 0, index = 0;
                             index < handleList.size(); ++index)
                        {
                            if (handleList[index] == checkHandle)
                                foundIt = 1;
                        }
                    }

                    if (foundIt && (machineList.size() != 0))
                    {
                        STAFString lowerMachine = iter->second.machine.
                            toLowerCase();

                        for (foundIt = 0, index = 0;
                             index < machineList.size(); ++index)
                        {
                            // Do a wildcard match

                            if (lowerMachine.matchesWildcards(
                                machineList[index]))
                            {
                                foundIt = 1;
                            }
                        }
                    }

                    if (foundIt && (nameList.size() != 0))
                    {
                        STAFString lowerName =
                            iter->second.process.toLowerCase();

                        for (foundIt = 0, index = 0;
                             index < nameList.size(); ++index)
                        {
                            if (nameList[index] == lowerName)
                                foundIt = 1;
                        }
                    }

                    if (foundIt && (userList.size() != 0))
                    {
                        STAFString user =
                            iter->second.authenticator.toLowerCase()+
                            gSpecSeparator + iter->second.userIdentifier;

                        for (foundIt = 0, index = 0;
                             index < userList.size(); ++index)
                        {
                            // Do a wildcard match

                            // if (user.matchesWildcards(userList[index]))
                            if (userList[index] == user)
                                foundIt = 1;
                        }
                    }

                    if (foundIt && (typeList.size() != 0))
                    {
                        STAFString lowerType = iter->second.type.toLowerCase();

                        for (foundIt = 0, index = 0;
                             index < typeList.size(); ++index)
                        {
                            if (typeList[index] == lowerType)
                                foundIt = 1;
                        }
                    }

                    if (foundIt && (containsList.size() != 0))
                    {
                        STAFString &msg = iter->second.message;
                    
                        for (foundIt = 0, index = 0;
                             index < containsList.size(); ++index)
                        {   
                            if (msg.find(containsList[index]) !=
                                STAFString::kNPos)
                            {
                                foundIt = 1;
                            }
                        }
                    }

                    if (foundIt && (icontainsList.size() != 0))
                    {
                       STAFString lowerMsg = iter->second.message.
                           toLowerCase();

                        for (foundIt = 0, index = 0;
                             index < icontainsList.size(); ++index)
                        {
                            if (lowerMsg.find(icontainsList[index]) !=
                                STAFString::kNPos)
                            {
                                foundIt = 1;
                            }
                        }
                    }
                
                    if (!foundIt)
                    {
                        // This queue element doesn't match search criteria
                        // so skip it next time if doing a GET request

                        if (isGet) ++entriesToSkip;

                        // Continue to process the next element in the queue
                        continue;
                    }
                    
                    // This queue element matches the search criteria

                    numFound++;

                    // Create the queueEntryMap for the queued message

                    STAFObjectPtr queueEntryMap = fQueueEntryMapClass->
                        createInstance();

                    queueEntryMap->put("priority",
                                       STAFString(iter->second.priority));
                    queueEntryMap->put("timestamp",
                                       iter->second.timestamp.asString());
                    queueEntryMap->put("machine", iter->second.machine);

                    if (iter->second.process.length() == 0)
                    {
                        queueEntryMap->put("handleName",
                                           STAFObject::createNone());
                    }
                    else
                    {
                        queueEntryMap->put("handleName",
                                           iter->second.process);
                    }

                    queueEntryMap->put("handle",
                                       STAFString(iter->second.handle));
                    queueEntryMap->put("user",
                                       iter->second.authenticator +
                                       gSpecSeparator +
                                       iter->second.userIdentifier);
                    
                    if (iter->second.type.length() != 0)
                        queueEntryMap->put("type", iter->second.type);
                    
                    queueEntryMap->put("message", iter->second.message);
                        
                    if (all || first)
                        resultList->append(queueEntryMap);
                    else
                        mc->setRootObject(queueEntryMap);
                    
                    if (isGet)
                    {
                        // Remove the message from the queue
                        handleQueue->fQueue.erase(iter);
                    }

                    if (!all && !first)
                    {
                        // Return information about this queue element
                        return STAFServiceResult(kSTAFOk, mc->marshall());
                    }

                    // If GET, break out of the for loop so that iter won't be
                    // incremented at the end of the loop as that can cause an
                    // exception since an element was removed from the
                    // iterated queue list

                    if (isGet) break;

                } // end for loop

                if (!isGet)
                {
                    // Peek - Finished iterating through the queue
                    done = true;
                }
                else
                {
                    // Get - Have to keep checking for matching messages
                    //       if found a message last time since the message
                    //       was removed and can't continue using the iterator
                    //       for a list that has changed

                    if (!foundIt)
                    {
                        // No more matching messages in the queue
                        done = true;
                    }
                }
            } // end while loop

            if (doWait && (numFound == 0))
            {
                // WAIT option was specified and no matching queue elements
                // have been found yet.  Check if should continue waiting.

                unsigned int deltaTime = (STAFTimestamp::now() - startTime) *
                    1000;

                if (deltaTime >= timeout)
                {
                    return STAFServiceResult(kSTAFTimeout);
                }
                else
                {
                    currTimeout = timeout - deltaTime;
                    event->reset();
                }
            }
            else
            {
                retry = false;
            }

        } // end lock on handle's queue

        if (retry)
        {
            // Check if the handle still exists in case it has been
            // unregistered/deleted in which case we want to return a
            // Request Cancelled error and stop waiting
            
            STAFHandleQueuePtr queuePtr;

            rc = gHandleManagerPtr->handleQueue(requestInfo.fHandle,
                                                queuePtr);

            if (rc == kSTAFHandleDoesNotExist)
                return STAFServiceResult(kSTAFRequestCancelled);
            
            // Wait until an element is placed on the queue or times out

            if (event->wait(currTimeout))
            {
                return STAFServiceResult(kSTAFTimeout);
            }
        }
    }

    if (numFound == 0)
    {
        return STAFServiceResult(kSTAFNoQueueElement);
    }
        
    mc->setRootObject(resultList);
    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFQueueService::handleDelete(
    const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine
    // No check of trust is needed since these commands are only valid
    // with respect to the submitting process' queue so just set
    // required trust level to 0.

    IVALIDATE_LOCAL_TRUST(0, "DELETE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fDeleteParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    STAFString errorBuffer;

    unsigned int checkPriority  = parsedResult->optionTimes("PRIORITY");
    unsigned int checkMachine   = parsedResult->optionTimes("MACHINE");
    unsigned int checkName      = parsedResult->optionTimes("NAME");
    unsigned int checkHandle    = parsedResult->optionTimes("HANDLE");
    unsigned int checkUser      = parsedResult->optionTimes("USER");
    unsigned int checkType      = parsedResult->optionTimes("TYPE");
    unsigned int checkContains  = parsedResult->optionTimes("CONTAINS");
    unsigned int checkIContains = parsedResult->optionTimes("ICONTAINS");

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFHandleQueuePtr handleQueue;
    STAFRC_t rc = gHandleManagerPtr->handleQueue(requestInfo.fHandle,
                                                 handleQueue);
    if (rc) return STAFServiceResult(rc);

    unsigned int numDeleted = 0;
    STAFMutexSemLock queueLock(handleQueue->fQueueSem);
    STAFHandleQueue::HandleQueue::iterator iter;
    int foundIt = 0;

    for (iter = handleQueue->fQueue.begin();
         iter != handleQueue->fQueue.end();)
    {
        foundIt = 1;

        if (checkPriority)
        {
            unsigned int priority = 0;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("PRIORITY")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_UINT_OPTION("PRIORITY", i, priority);
                if (rc) return STAFServiceResult(rc, errorBuffer);
                if (priority == iter->second.priority) foundIt = 1;
            }
        }

        if (foundIt && checkMachine)
        {
            STAFString machine;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("MACHINE")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("MACHINE", i, machine);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                machine.lowerCase();

                // Do a wildcard match

                STAFString lowerMachine = iter->second.machine.toLowerCase();

                if (lowerMachine.matchesWildcards(machine))
                {
                    foundIt = 1;
                }
            }
        }

        if (foundIt && checkName)
        {
            STAFString name;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("NAME")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("NAME", i, name);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                name.lowerCase();

                if (name == iter->second.process.toLowerCase()) foundIt = 1;
            }
        }

        if (foundIt && checkHandle)
        {
            unsigned int theHandle = 0;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("HANDLE")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_UINT_OPTION_RANGE(
                    "HANDLE", i, theHandle,
                    gHandleManagerPtr->getMinHandleNumber(),
                    gHandleManagerPtr->getMaxHandleNumber());

                if (rc) return STAFServiceResult(rc, errorBuffer);

                if (theHandle == iter->second.handle) foundIt = 1;
            }
        }

        if (foundIt && checkUser)
        {
            STAFString user;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("USER")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("USER", i, user);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                // Check if authenticator was specified in the user value

                unsigned int sepIndex = user.find(gSpecSeparator);

                if (sepIndex == STAFString::kNPos)
                {
                    // No authenticator specified
                    // Use lower-case default authenticator
                    user = gServiceManagerPtr->getDefaultAuthenticator().
                        toLowerCase() + gSpecSeparator + user;
                }
                else
                {
                    // User specified in form of Authenticator://UserIdentifier
                    // Change authenticator to lower-case.
                    user = user.subString(0, sepIndex).toLowerCase() +
                           user.subString(sepIndex);
                }

                // Do a wildcard match

                STAFString matchString(
                    iter->second.authenticator.toLowerCase() +
                    gSpecSeparator + iter->second.userIdentifier);

                // if (matchString.matchesWildcards(user)) foundIt = 1;
                if (matchString == user) foundIt = 1;
            }
        }

        if (foundIt && checkType)
        {
            STAFString type;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("TYPE")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("TYPE", i, type);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                type.lowerCase();

                if (type == iter->second.type.toLowerCase()) foundIt = 1;
            }
        }

        if (foundIt && checkContains)
        {
            STAFString contains;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("CONTAINS")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("CONTAINS", i, contains);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                if (iter->second.message.find(contains) != STAFString::kNPos)
                    foundIt = 1;
            }
        }

        if (foundIt && checkIContains)
        {
            STAFString checkMessage = iter->second.message.toLowerCase();
            STAFString icontains;
            foundIt = 0;

            for (int i = 1; (i <= parsedResult->optionTimes("ICONTAINS")) &&
                 !foundIt; ++i)
            {
                rc = RESOLVE_INDEXED_STRING_OPTION("ICONTAINS", i, icontains);

                if (rc) return STAFServiceResult(rc, errorBuffer);

                if (checkMessage.find(icontains.toLowerCase()) !=
                    STAFString::kNPos)
                {
                    foundIt = 1;
                }
            }
        }

        if (foundIt)
        {
            ++numDeleted;
            handleQueue->fQueue.erase(iter);
            iter = handleQueue->fQueue.begin();
        }
        else
        {
            ++iter;
        }
    }

    return STAFServiceResult(kSTAFOk, STAFString(numDeleted));
}


STAFServiceResult STAFQueueService::handleList(
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
                                 parsedResult->errorBuffer, 0);
    }

    STAFString errorBuffer;

    // HANDLE may only default if the request came from this machine

    if ((parsedResult->optionTimes("HANDLE") == 0) &&
        !requestInfo.fIsLocalRequest)
    {
        return STAFServiceResult(kSTAFInvalidRequestString);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    unsigned int theHandle = requestInfo.fHandle;

    STAFRC_t rc = RESOLVE_OPTIONAL_UINT_OPTION_RANGE(
        "HANDLE", theHandle, gHandleManagerPtr->getMinHandleNumber(),
        gHandleManagerPtr->getMaxHandleNumber());

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFHandleQueuePtr handleQueue;

    rc = gHandleManagerPtr->handleQueue(theHandle, handleQueue);

    if (rc) return STAFServiceResult(rc);

    // Create a marshalled list of maps containing information for entries
    // found in the queue

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fQueueEntryMapClass->reference());
    STAFObjectPtr queueList = STAFObject::createList();

    // Iterate through the queue

    STAFMutexSemLock queueLock(handleQueue->fQueueSem);

    STAFHandleQueue::HandleQueue::iterator iter;

    for (iter = handleQueue->fQueue.begin();
         iter != handleQueue->fQueue.end(); iter++)
    {
        STAFHandleQueue::Message message = iter->second;

        // Create a map for each entry in the queue

        STAFObjectPtr queueEntryMap = fQueueEntryMapClass->createInstance();

        queueEntryMap->put("priority", STAFString(message.priority));
        queueEntryMap->put("timestamp", message.timestamp.asString());
        queueEntryMap->put("machine", message.machine);

        if (message.process.length() == 0)
            queueEntryMap->put("handleName", STAFObject::createNone());
        else
            queueEntryMap->put("handleName", message.process);

        queueEntryMap->put("handle", STAFString(message.handle));
        queueEntryMap->put("user", message.authenticator +
                           gSpecSeparator + message.userIdentifier);

        if (message.type.length() != 0)
            queueEntryMap->put("type", message.type);

        queueEntryMap->put("message",
                           STAFHandle::maskPrivateData(message.message));

        queueList->append(queueEntryMap);
    }

    // Set the marshalling context's root object

    mc->setRootObject(queueList);

    return STAFServiceResult(rc, mc->marshall());
}


STAFServiceResult STAFQueueService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
