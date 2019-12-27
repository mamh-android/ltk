/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_HandleQueue
#define STAF_HandleQueue

#include <map>
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAF.h"
#include "STAFTimestamp.h"
#include "STAFService.h"
#include "STAFMutexSem.h"
#include "STAFEventSem.h"

class STAFHandleQueue;
typedef STAFRefPtr<STAFHandleQueue> STAFHandleQueuePtr;

class STAFHandleQueue
{
public:

    struct Message
    {
        Message(unsigned int aPriority = 1,
                const STAFString &aMachine = STAFString(),
                const STAFString &aAuthenticator = STAFString(),
                const STAFString &aUserIdentifier = STAFString(),
                const STAFString &aProcess = STAFString(),
                STAFHandle_t aHandle = 0,
                const STAFString &aMessage = STAFString(),
                const STAFString &aType = STAFString())
            : priority(aPriority), handle(aHandle),
              timestamp(STAFTimestamp::now()), machine(aMachine),
              authenticator(aAuthenticator), userIdentifier(aUserIdentifier),
              process(aProcess), message(aMessage), type(aType)
        { /* Do Nothing */ }

        unsigned int priority;
        STAFHandle_t handle;
        STAFTimestamp timestamp;
        STAFString machine;
        STAFString authenticator;
        STAFString userIdentifier;
        STAFString process;
        STAFString message;
        STAFString type;
    };

    typedef std::multimap<unsigned int, Message> HandleQueue;

    STAFHandleQueue(unsigned int maxMessages = 100)
        : fMaxMessages(maxMessages), fNotify(new STAFEventSem(),
                                             STAFEventSemPtr::INIT)
    { /* Do Nothing */ }


    unsigned int fMaxMessages;
    HandleQueue fQueue;
    STAFMutexSem fQueueSem;
    STAFEventSemPtr fNotify;

private:

    // Don't allow copy construction or assignment
    STAFHandleQueue(const STAFHandleQueue &);
    STAFHandleQueue &operator=(const STAFHandleQueue &);
};

#endif
