/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_LifeCycleService
#define STAF_LifeCycleService

#include <map>
#include <vector>
#include "STAFVariablePool.h"
#include "STAFMutexSem.h"
#include "STAFService.h"

// STAFLifeCycleService - Provides the ability to run or more STAF service
// requests when STAFProc starts up or shuts down on a machine

class STAFLifeCycleService : public STAFService
{
public:

    STAFLifeCycleService();

    virtual STAFString info(unsigned int raw = 0) const;

    virtual STAFServiceResult acceptRequest(
        const STAFServiceRequest &requestInfo);

    virtual ~STAFLifeCycleService();

private:

    // Don't allow copy construction or assignment
    STAFLifeCycleService(const STAFLifeCycleService &);
    STAFLifeCycleService &operator=(const STAFLifeCycleService &);

    STAFServiceResult handleRegister(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleUnregister(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleUpdate(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleTrigger(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleEnable(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDisable(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fRegisterParser;
    STAFCommandParser fUnregisterParser;
    STAFCommandParser fUpdateParser;
    STAFCommandParser fListParser;
    STAFCommandParser fQueryParser;
    STAFCommandParser fTriggerParser;
    STAFCommandParser fEnableParser;
    STAFCommandParser fDisableParser;

    STAFMapClassDefinitionPtr fRegMapClass;
    STAFMapClassDefinitionPtr fRegDetailsMapClass;
    STAFMapClassDefinitionPtr fRegQueryMapClass;
    STAFMapClassDefinitionPtr fTriggerIdMapClass;
    STAFMapClassDefinitionPtr fTriggerIdsMapClass;

    STAFMutexSem fRegistrationMapSem;
    STAFString   fServiceLogName;
};

#endif
