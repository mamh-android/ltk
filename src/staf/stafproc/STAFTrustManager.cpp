/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFTrustManager.h"
#include "STAFServiceManager.h"

static const STAFString sStar(kUTF8_STAR);

STAFTrustManager::TrustData::TrustData(const STAFString &theGroup,
                                       const STAFString &theEntity,
                                       unsigned int theTrustLevel)
    : group(theGroup), entity(theEntity), trustLevel(theTrustLevel)
{
    if (group.hasWildcard())
    {
        if (entity.hasWildcard())
            matchType = kTrustMatchWildcard;
        else
            matchType = kTrustMatchEntityExact;
    }
    else if (entity.hasWildcard())
    {
        matchType = kTrustMatchGroupExact;
    }
    else
    {
        matchType = kTrustMatchExact;
    }
}

unsigned int STAFTrustManager::getTrustLevel(const STAFString &theInterface,
                                             const STAFString &logicalID,
                                             const STAFString &physicalID,
                                             const STAFString &authenticator,
                                             const STAFString &userID)
{
    // First, let's try to find a user trust match

    unsigned int theTrustLevel = 0;
    unsigned int matchLevel = TrustData::kTrustMatchNoMatch;
    bool matchFound = false;
    TrustMap::iterator iter;

    for (iter = fUserTrustMap.begin(); iter != fUserTrustMap.end(); ++iter)
    {
        const TrustData &trustData = iter->second;

        if (trustData.matchType > matchLevel) continue;

        if (authenticator.matchesWildcards(trustData.group,
                                           kSTAFStringCaseInsensitive) &&
            userID.matchesWildcards(trustData.entity,
                                    kSTAFStringCaseSensitive))
        {
            matchFound = true;

            if (matchLevel > trustData.matchType)
            {
                theTrustLevel = trustData.trustLevel;
                matchLevel = trustData.matchType;
            }
            else
            {
                theTrustLevel = STAF_MIN(theTrustLevel, trustData.trustLevel);
            }
        }
    }

    if (matchFound) return theTrustLevel;

    // No user trust match was found, so let's search for a machine trust match

    bool matchesPhysical = false;

    for (iter = fMachineTrustMap.begin(); iter != fMachineTrustMap.end(); ++iter)
    {
        const TrustData &trustData = iter->second;

        if (trustData.matchType > matchLevel) continue;

        bool thisMatches = false;
        bool thisMatchesPhysical = false;

        if (theInterface.matchesWildcards(trustData.group,
                                          kSTAFStringCaseInsensitive))
        {
            if (physicalID.matchesWildcards(trustData.entity,
                                            kSTAFStringCaseInsensitive))
            {
                thisMatches = true; 
                thisMatchesPhysical = true;
            }
            else if (logicalID.matchesWildcards(trustData.entity,
                                                kSTAFStringCaseInsensitive))
            {
                thisMatches = true;
            }
        }

        if (thisMatches)
        {
            matchFound = true;

            if (matchLevel > trustData.matchType)
            {
                theTrustLevel = trustData.trustLevel;
                matchesPhysical = thisMatchesPhysical;
                matchLevel = trustData.matchType;
            }
            else if (!matchesPhysical && thisMatchesPhysical)
            {
                theTrustLevel = trustData.trustLevel;
                matchesPhysical = true;
            }
            else if (matchesPhysical == thisMatchesPhysical)
            {
                // Note: This matches if they are either both physical or
                //       both logical, but not if they are one of each

                theTrustLevel = STAF_MIN(theTrustLevel, trustData.trustLevel);
            }
        }
    }

    // XXX: Might need an explicit check for local access here
    
    if (!matchFound) theTrustLevel = fDefaultTrustLevel;
    
    return theTrustLevel;
}


unsigned int STAFTrustManager::getTrustLevel(const STAFString &machine,
                                             const STAFString &user)
{
    STAFString theInterface;
    STAFString theMachine;

    splitSpecification(machine, sStar, theInterface, theMachine);

    // If a port was specified as part of the machine, remove it from the
    // machine as trust is based on machine identifier only, without the port

    theMachine = STAFHandle::stripPortFromEndpoint(theMachine);

    STAFString theAuthenticator;
    STAFString theUserID;

    splitSpecification(user,
                       gServiceManagerPtr->getDefaultAuthenticator(),
                       theAuthenticator,
                       theUserID);

    return getTrustLevel(theInterface, theMachine, theMachine,
                         theAuthenticator, theUserID);
}


unsigned int STAFTrustManager::getTrustLevel(const STAFString &machine)
{
    return getTrustLevel(machine, gUnauthenticatedUser);
}

STAFRC_t STAFTrustManager::setMachineTrusteeLevel(const STAFString &machine,
                                                  unsigned int trustLevel)
{
    if (trustLevel > fMaxTrustLevel)
        return kSTAFInvalidTrustLevel;

    STAFString theInterface;
    STAFString theMachine;

    splitSpecification(machine, sStar, theInterface, theMachine);
    
    // If a port was specified as part of the machine, remove it from the
    // machine as trust is based on machine identifier only, without the port

    theMachine = STAFHandle::stripPortFromEndpoint(theMachine);

    STAFString theSpec = theInterface.toLowerCase() + gSpecSeparator +
                         theMachine.toLowerCase();

    STAFMutexSemLock trustLock(fTrustDataSem);

    if (fMachineTrustMap.find(theSpec) == fMachineTrustMap.end())
    {
        fMachineTrustMap[theSpec] = TrustData(theInterface, theMachine,
                                              trustLevel);
    }
    else
    {
        fMachineTrustMap[theSpec].trustLevel = trustLevel;
    }

    return kSTAFOk;
}


STAFRC_t STAFTrustManager::deleteMachineTrustee(const STAFString &machine)
{
    STAFString theInterface;
    STAFString theMachine;

    splitSpecification(machine, sStar, theInterface, theMachine);
    
    // If a port was specified as part of the machine, remove it from the
    // machine as trust is based on machine identifier only, without the port

    theMachine = STAFHandle::stripPortFromEndpoint(theMachine);

    STAFString theSpec = theInterface.toLowerCase() + gSpecSeparator +
                         theMachine.toLowerCase();

    STAFMutexSemLock trustLock(fTrustDataSem);

    if (fMachineTrustMap.find(theSpec) == fMachineTrustMap.end())
        return kSTAFTrusteeDoesNotExist;

    fMachineTrustMap.erase(theSpec);

    return kSTAFOk;
}


STAFTrustManager::TrustMap STAFTrustManager::getMachineTrustMapCopy()
{
    STAFMutexSemLock trustLock(fTrustDataSem);

    return fMachineTrustMap;
}


STAFRC_t STAFTrustManager::setUserTrusteeLevel(STAFString user,
                                               unsigned int trustLevel)
{
    if (trustLevel > fMaxTrustLevel)
        return kSTAFInvalidTrustLevel;

    STAFString theAuthenticator;
    STAFString theUserID;

    splitSpecification(user,
                       gServiceManagerPtr->getDefaultAuthenticator(),
                       theAuthenticator,
                       theUserID);

    STAFString theSpec = theAuthenticator.toLowerCase() + gSpecSeparator +
                         theUserID;

    // XXX: if (theSpec == gUnauthenticatedUser) return kSTAFInvalidValue;

    // XXX: Don't return an error if no authenticators are registered
    //if (theAuthenticator.toLowerCase() == gNoneString) return kSTAFInvalidValue;

    STAFMutexSemLock trustLock(fTrustDataSem);

    if (fUserTrustMap.find(theSpec) == fUserTrustMap.end())
    {
        fUserTrustMap[theSpec] = TrustData(theAuthenticator, theUserID,
                                           trustLevel);
    }
    else
    {
        fUserTrustMap[theSpec].trustLevel = trustLevel;
    }

    return kSTAFOk;
}


STAFRC_t STAFTrustManager::deleteUserTrustee(STAFString user)
{
    STAFString theAuthenticator;
    STAFString theUserID;

    splitSpecification(user,
                       gServiceManagerPtr->getDefaultAuthenticator(),
                       theAuthenticator,
                       theUserID);

    STAFString theSpec = theAuthenticator.toLowerCase() + gSpecSeparator +
                         theUserID;

    STAFMutexSemLock trustLock(fTrustDataSem);

    if (fUserTrustMap.find(theSpec) == fUserTrustMap.end())
        return kSTAFTrusteeDoesNotExist;

    fUserTrustMap.erase(theSpec);

    return kSTAFOk;
}


STAFTrustManager::TrustMap STAFTrustManager::getUserTrustMapCopy()
{
    STAFMutexSemLock trustLock(fTrustDataSem);

    return fUserTrustMap;
}


unsigned int STAFTrustManager::getDefaultTrusteeLevel()
{
    return fDefaultTrustLevel;
}


STAFRC_t STAFTrustManager::setDefaultTrusteeLevel(unsigned int trustLevel)
{
    if (trustLevel > fMaxTrustLevel)
        return kSTAFInvalidTrustLevel;

    STAFMutexSemLock trustLock(fTrustDataSem);

    if (trustLevel > fMaxTrustLevel)
        return kSTAFInvalidTrustLevel;

    fDefaultTrustLevel = trustLevel;

    return kSTAFOk;
}

unsigned int STAFTrustManager::getMaxTrustLevel()
{
    return fMaxTrustLevel;
}


void STAFTrustManager::splitSpecification(const STAFString &source,
                                          const STAFString &defaultGroup,
                                          STAFString &group,
                                          STAFString &entity)
{
    unsigned int sepPos = source.find(gSpecSeparator);

    if (sepPos == STAFString::kNPos)
    {
        group = defaultGroup;
        entity = source;
    }
    else
    {
        group = source.subString(0, sepPos);
        entity = source.subString(sepPos + gSpecSeparator.length());
    }
}
