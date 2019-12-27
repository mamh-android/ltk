/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DiagManager
#define STAF_DiagManager

#include <map>
#include "STAF.h"
#include "STAFString.h"
#include "STAFUtil.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFTimestamp.h"

class STAFDiagManager
{
public:

    // Public types

    struct DiagData
    {
        DiagData() { /* Do Nothing */ }

        DiagData(STAFString aTrigger, STAFString aSource)
            : lowerCaseTrigger(aTrigger.toLowerCase()),
              trigger(aTrigger),
              source(aSource),
              count(1)
        { /* Do Nothing */ }

        STAFString lowerCaseTrigger;
        STAFString trigger;
        STAFString source;
        unsigned int count;
    };
    
    typedef std::map<STAFString, DiagData> DiagMap;

    STAFDiagManager(unsigned int enabled = 0) :
        fEnabled(enabled),
        fDisabledTimestamp(STAFTimestamp::now()),
        fResetTimestamp(0)
    { /* Do Nothing */ }
    
    STAFRC_t record(const STAFString &trigger,
                    const STAFString &source);
    
    STAFRC_t record(const STAFString &trigger,
                    const STAFString &handleName,
                    const STAFString &machine,
                    STAFHandle_t handle);

    STAFRC_t reset();

    STAFRC_t enable();

    STAFRC_t disable();

    unsigned int getEnabled();
    STAFString getEnabledAsString();
    STAFTimestamp getDisabledTimestamp();
    STAFTimestamp getResetTimestamp();
    
    DiagMap getDiagMapCopy();

private:

    // Don't allow copy construction or assignment
    STAFDiagManager(const STAFDiagManager &);
    STAFDiagManager &operator=(const STAFDiagManager &);

    unsigned int fEnabled;

    // Last disabled timestamp
    STAFTimestamp fDisabledTimestamp;

    // Last reset (or first enabled) timestamp
    STAFTimestamp fResetTimestamp;

    STAFMutexSem fDiagMapSem;
    DiagMap fDiagMap;
};


#endif
