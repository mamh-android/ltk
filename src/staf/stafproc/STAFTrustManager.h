/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_TrustManager
#define STAF_TrustManager

#include <map>
#include "STAF.h"
#include "STAFString.h"
#include "STAFUtil.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFFileSystem.h"


class STAFTrustManager
{
public:

    // Public types

    struct TrustData
    {
        enum TrustMatchType
        {
            kTrustMatchExact       = 0,
            kTrustMatchGroupExact  = 1,
            kTrustMatchEntityExact = 2,
            kTrustMatchWildcard    = 3,
            kTrustMatchNoMatch     = 4
        };

        TrustData() { /* Do Nothing */ }

        TrustData(const STAFString &theGroup, const STAFString &theEntity,
                  unsigned int theTrustLevel);

        STAFString group;
        STAFString entity;
        unsigned int trustLevel;
        TrustMatchType matchType;
    };

    typedef std::map<STAFString, TrustData> TrustMap;

    STAFTrustManager(unsigned int maxTrustLevel,
                     unsigned int defaultTrustLevel = 3)
        : fMaxTrustLevel(maxTrustLevel), fDefaultTrustLevel(defaultTrustLevel)
    { /* Do Nothing */ }

    unsigned int getTrustLevel(const STAFString &theInterface,
                               const STAFString &logicalID,
                               const STAFString &physicalID,
                               const STAFString &authenticator,
                               const STAFString &userID);
    unsigned int getTrustLevel(const STAFString &machine,
                               const STAFString &user);
    unsigned int getTrustLevel(const STAFString &machine);

    STAFRC_t setMachineTrusteeLevel(const STAFString &machine,
                                    unsigned int trustLevel);
    STAFRC_t deleteMachineTrustee(const STAFString &machine);
    TrustMap getMachineTrustMapCopy();

    STAFRC_t setUserTrusteeLevel(STAFString user,
                                 unsigned int trustLevel);
    STAFRC_t deleteUserTrustee(STAFString user);
    TrustMap getUserTrustMapCopy();

    unsigned int getDefaultTrusteeLevel();
    STAFRC_t setDefaultTrusteeLevel(unsigned int trustLevel);

    unsigned int getMaxTrustLevel();

private:

    // Don't allow copy construction or assignment
    STAFTrustManager(const STAFTrustManager &);
    STAFTrustManager &operator=(const STAFTrustManager &);

    void splitSpecification(const STAFString &source,
                            const STAFString &defaultGroup,
                            STAFString &group,
                            STAFString &entity);

    unsigned int fMaxTrustLevel;
    unsigned int fDefaultTrustLevel;
    STAFMutexSem fTrustDataSem;
    TrustMap fMachineTrustMap;
    TrustMap fUserTrustMap;
};

#endif
