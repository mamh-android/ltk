/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_SemService
#define STAF_SemService

#include <map>
#include <list>
#include "STAFTimestamp.h"
#include "STAFMutexSem.h"
#include "STAFEventSem.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFService.h"
#include "STAFRefPtr.h"

// STAFSemService - Handles manipulating a handle's Sem

class STAFSemService : public STAFService
{
public:

    STAFSemService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFSemService();

    struct MutexRequest
    {
        MutexRequest(STAFString aSTAFInstanceUUID = "",
                     STAFString aMachine = "",
                     STAFString aHandleName = "", STAFHandle_t aHandle = 0,
                     STAFString aUser = gUnauthenticatedUser,
                     STAFString aEndpoint = "",
                     bool aGarbageCollect = true)
            : stafInstanceUUID(aSTAFInstanceUUID), machine(aMachine),
              handleName(aHandleName), handle(aHandle),
              user(aUser), endpoint(aEndpoint), timeDate(STAFTimestamp::now()),
              avail(new STAFEventSem(), STAFEventSemPtr::INIT),
              garbageCollectedPtr(new bool, STAFRefPtr<bool>::INIT),
              garbageCollect(aGarbageCollect),
              retCode(kSTAFOk)
        { *garbageCollectedPtr = false; }

        STAFString      stafInstanceUUID;
        STAFString      machine;
        STAFString      handleName;
        STAFHandle_t    handle;
        STAFString      user;
        STAFString      endpoint;
        STAFTimestamp   timeDate;
        STAFEventSemPtr avail;
        STAFRefPtr<bool> garbageCollectedPtr;
        bool            garbageCollect; // true means do garbage collection
                                        // false means no garbage collection
        STAFRC_t        retCode;        // Return code indicating if request
                                        // was successful (0) or canceled (56)
        STAFString      resultBuffer;   // Error message if retCode != 0
    };

    typedef STAFRefPtr<MutexRequest> MutexRequestPtr;

    // MutxRequestList - Ordered list of pending requests for a mutex
    typedef std::list<MutexRequestPtr> MutexRequestList;

    struct MutexSem
    {
        // Note: we have 2 constructors here due to a GNU C++
        //       compiler internal error when we actually
        //       assign a default MutexRequest to owner

        MutexSem(STAFString aName = "", unsigned int owned = 0)
            : name(aName), uppercaseName(aName.toUpperCase()),
              isOwned(owned), acquireTimeDate(STAFTimestamp::now()),
              lockPtr(new STAFMutexSem(), STAFMutexSemPtr::INIT)
        {  /* Do Nothing */  }

        MutexSem(STAFString aName, unsigned int owned,
                 const MutexRequest &anOwner)
            : name(aName), uppercaseName(aName.toUpperCase()),
              isOwned(owned), owner(anOwner), 
              acquireTimeDate(STAFTimestamp::now()),
              lockPtr(new STAFMutexSem(), STAFMutexSemPtr::INIT)
        {  /* Do Nothing */ }

        STAFString name;
        STAFString uppercaseName;
        unsigned int isOwned;
        MutexRequest owner;
        STAFTimestamp acquireTimeDate;
        STAFMutexSemPtr lockPtr;
        MutexRequestList requestList;
    };

    typedef STAFRefPtr<MutexSem> MutexSemPtr;
    typedef std::map<STAFString, MutexSemPtr> MutexSemList;

    struct EventUnit
    {
        EventUnit(STAFString aSTAFInstanceUUID = "",
                  STAFString aMachine = "",
                  STAFString aHandleName = "",
                  STAFHandle_t aHandle = 0,
                  STAFString aUser = gUnauthenticatedUser,
                  STAFString aEndpoint = "")
            : stafInstanceUUID(aSTAFInstanceUUID), machine(aMachine),
              handleName(aHandleName), handle(aHandle),
              user(aUser), endpoint(aEndpoint), timeDate(STAFTimestamp::now())
        { /* Do Nothing */ }

        STAFString stafInstanceUUID;
        STAFString machine;
        STAFString handleName;
        STAFHandle_t handle;
        STAFString user;
        STAFString endpoint;
        STAFTimestamp timeDate;
    };

    typedef std::list<EventUnit> EventWaiterList;

    struct EventSem
    {
        EventSem(STAFString aName = "") : name(aName),
            uppercaseName(aName.toUpperCase()),
            eventSem(new STAFEventSem(), STAFEventSemPtr::INIT),
            lastPost(*gSTAFInstanceUUIDPtr, *gMachinePtr, "STAF_Process",
                     gSTAFProcHandle, gUnauthenticatedUser, "local://local"),
            lastReset(*gSTAFInstanceUUIDPtr, *gMachinePtr, "STAF_Process",
                      gSTAFProcHandle, gUnauthenticatedUser, "local://local"),
            lockPtr(new STAFMutexSem(), STAFMutexSemPtr::INIT)
        { /* Do Nothing */ }

        STAFString name;
        STAFString uppercaseName;
        STAFEventSemPtr eventSem;
        EventUnit lastPost;
        EventUnit lastReset;
        STAFMutexSemPtr lockPtr;
        EventWaiterList waiterList;
    };

    typedef STAFRefPtr<EventSem> EventSemPtr;
    typedef std::map<STAFString, EventSemPtr> EventSemList;

private:

    // Don't allow copy construction or assignment
    STAFSemService(const STAFSemService &);
    STAFSemService &operator=(const STAFSemService &);

    STAFServiceResult handleRequest(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleRelease(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleCancel(const STAFServiceRequest &requestInfo);
    STAFServiceResult handlePostPulse(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleReset(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleWait(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDelete(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleSTAFCallback(
        const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    MutexSemList fMutexSemList;
    STAFMutexSem fMutexSemListSem;
    EventSemList fEventSemList;
    STAFMutexSem fEventSemListSem;

    STAFCommandParser fRequestParser;
    STAFCommandParser fReleaseParser;
    STAFCommandParser fCancelParser;
    STAFCommandParser fPostPulseParser;
    STAFCommandParser fResetParser;
    STAFCommandParser fWaitParser;
    STAFCommandParser fDeleteParser;
    STAFCommandParser fQueryParser;
    STAFCommandParser fListParser;
    STAFCommandParser fSTAFCallbackParser;
    
    STAFMapClassDefinitionPtr fMutexInfoClass;
    STAFMapClassDefinitionPtr fEventInfoClass;
    STAFMapClassDefinitionPtr fQueryMutexClass;
    STAFMapClassDefinitionPtr fQueryEventClass;
    STAFMapClassDefinitionPtr fEventRequesterClass;
    STAFMapClassDefinitionPtr fMutexOwnerClass;
    STAFMapClassDefinitionPtr fPendingRequestClass;
};

#endif
