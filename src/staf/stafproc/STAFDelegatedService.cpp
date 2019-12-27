/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFDelegatedService.h"
#include "STAFConnectionManager.h"

STAFDelegatedService::STAFDelegatedService(STAFString name, 
                                           STAFString machine,
                                           STAFString toName)
    : STAFService(name), fMachine(machine), fToName(toName.toUpperCase())
{
    /* Do Nothing */
}


STAFDelegatedService::~STAFDelegatedService()
{
    /* Do Nothing */
}


STAFString STAFDelegatedService::info(unsigned int) const
{
    return name() + ": Delegated to " + fToName + " on " + fMachine;
}

STAFServiceResult STAFDelegatedService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFConnectionPtr connection;
    STAFString result;
    STAFRC_t rc = gConnectionManagerPtr->makeConnection(
        fMachine, connection, result);

    if (rc == kSTAFOk)
    {
        connection->writeUInt(kSTAFRemoteServiceRequestAPI2);  // API Number
        connection->writeUInt(0);      // API Level

        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());

        if (ack != kSTAFOk) return ack;

        // Now find out the specific level to use
        unsigned int minLevel = 2;
        unsigned int maxLevel = 2;

        connection->writeUInt(minLevel);
        connection->writeUInt(maxLevel);

        unsigned int levelToUse = connection->readUInt();

        if (levelToUse == 0) return kSTAFInvalidAPILevel;

        connection->writeString(requestInfo.fPort);
        connection->writeString(requestInfo.fSTAFInstanceUUID);
        connection->writeUInt(requestInfo.fHandle);
        connection->writeString(requestInfo.fHandleName);
        connection->writeString(fToName); // Target service name
        connection->writeString(requestInfo.fRequest);
        connection->writeString(requestInfo.fAuthenticator);
        connection->writeString(requestInfo.fUserIdentifier);
        connection->writeString(requestInfo.fAuthenticationData);
        connection->writeString(requestInfo.fMachineNickname);

        // Write originator's request var pool

        STAFVariablePool::VariableMap requestVarMap =
            requestInfo.fRequestVarPool->getVariableMapCopy();

        connection->writeUInt(requestVarMap.size());

        for (STAFVariablePool::VariableMap::iterator requestIter = 
             requestVarMap.begin(); requestIter != requestVarMap.end();
             ++requestIter)
        {
            connection->writeString(requestIter->second.name);
            connection->writeString(requestIter->second.value);
        }

        // Write originator's shared var pool

        STAFVariablePool::VariableMap sharedVarMap =
            requestInfo.fSourceSharedVarPool->getVariableMapCopy();

        connection->writeUInt(sharedVarMap.size());

        for (STAFVariablePool::VariableMap::iterator sharedIter = 
             sharedVarMap.begin(); sharedIter != sharedVarMap.end();
             ++sharedIter)
        {
            connection->writeString(sharedIter->second.name);
            connection->writeString(sharedIter->second.value);
        }
        
        rc = static_cast<STAFRC_t>(connection->readUInt());
        result = connection->readString();
    }

    return STAFServiceResult(rc, result);
}
