/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_NotificationList
#define STAF_NotificationList

#include <list>
#include "STAF.h"
#include "STAFString.h"
#include "STAFService.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"

class STAFNotificationList;
typedef STAFRefPtr<STAFNotificationList> STAFNotificationListPtr;

class STAFNotificationList
{
public:

    class Notifiee
    {
    public:

        enum NameOrHandle { kHandle, kName };

        Notifiee(STAFString aMachine = STAFString(),
                 STAFString aAltEndpoint = STAFString(),
                 STAFHandle_t aHandle = 0,
                 unsigned int aPriority = 0)
            : nameOrHandle(kHandle),
              machine(aMachine), altEndpoint(aAltEndpoint),
              process(STAFString()), handle(aHandle), priority(aPriority)
        { /* Do Nothing */ }

        Notifiee(STAFString aMachine, STAFString aAltEndpoint,
                 STAFString aName, unsigned int aPriority)
            : nameOrHandle(kName),
              machine(aMachine), altEndpoint(aAltEndpoint),
              process(aName), handle(0), priority(aPriority)
        { /* Do Nothing */ }

        NameOrHandle nameOrHandle;
        STAFString machine;     // Endpoint of machine to send notification
        STAFString altEndpoint; // Alternate endpoint (or "" if no alternate)
        STAFString process;
        STAFHandle_t handle;
        unsigned int priority;
    };

    typedef std::list<Notifiee> NotifieeList;

    STAFNotificationList();

    STAFRC_t reg(const STAFString &machine,
                 const STAFHandle_t handle, unsigned int priority);
    STAFRC_t reg(const STAFString &machine,
                 const STAFString &process, unsigned int priority);
    STAFRC_t reg(const STAFString &machine, const STAFString &altEndpoint,
                 const STAFHandle_t handle, unsigned int priority);
    STAFRC_t reg(const STAFString &machine, const STAFString &altEndpoint,
                 const STAFString &process, unsigned int priority);

    STAFRC_t unregister(const STAFString &machine, const STAFHandle_t handle, 
                        unsigned int priority);
    STAFRC_t unregister(const STAFString &machine, const STAFString &process, 
                        unsigned int priority);

    unsigned int sendNotification(const STAFString &type, 
                                  const STAFString &message);

    NotifieeList getNotifieeListCopy();

    ~STAFNotificationList();

private:

    // Don't allow copy construction or assignment
    STAFNotificationList(const STAFNotificationList &);
    STAFNotificationList &operator=(const STAFNotificationList &);

    STAFMutexSem fNotifieeListSem;
    NotifieeList fNotifieeList;
};

#endif
