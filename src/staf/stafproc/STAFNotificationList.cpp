/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFNotificationList.h"
#include "STAFProc.h"
#include "STAFThreadManager.h"
#include "STAFTrace.h"

STAFNotificationList::STAFNotificationList()
{
    /* Do Nothing */
}


STAFNotificationList::~STAFNotificationList()
{
    /* Do Nothing */
}


STAFRC_t STAFNotificationList::reg(const STAFString &machine,
                                   const STAFHandle_t handle,
                                   unsigned int priority)
{
    STAFString alternateEndpoint = "";

    return STAFNotificationList::reg(
        machine, alternateEndpoint, handle, priority);
}

STAFRC_t STAFNotificationList::reg(const STAFString &machine,
                                   const STAFString &process,
                                   unsigned int priority)
{
    STAFString alternateEndpoint = "";

    return STAFNotificationList::reg(
        machine, alternateEndpoint, process, priority);
}


STAFRC_t STAFNotificationList::reg(const STAFString &machine,
                                   const STAFString &altEndpoint,
                                   const STAFHandle_t handle,
                                   unsigned int priority)
{
    unsigned int foundIt = 0;
    STAFMutexSemLock semLock(fNotifieeListSem);
    NotifieeList::iterator listIter;

    for(listIter = fNotifieeList.begin();
        !foundIt && (listIter != fNotifieeList.end());)
    {
        Notifiee notifiee(*listIter);

        if ((notifiee.nameOrHandle == Notifiee::kHandle) &&
            (notifiee.machine == machine) && (notifiee.handle == handle) &&
            (notifiee.priority == priority))
        {
            foundIt = 1;
        }
        else
        {
            ++listIter;
        }
    }

    if (!foundIt)
    {
        fNotifieeList.push_back(
            Notifiee(machine, altEndpoint, handle, priority));
    }

    return kSTAFOk;
}


STAFRC_t STAFNotificationList::reg(const STAFString &machine,
                                   const STAFString &altEndpoint,
                                   const STAFString &process,
                                   unsigned int priority)
{
    unsigned int foundIt = 0;
    STAFMutexSemLock semLock(fNotifieeListSem);
    NotifieeList::iterator listIter;

    for(listIter = fNotifieeList.begin();
        !foundIt && (listIter != fNotifieeList.end());)
    {
        Notifiee notifiee(*listIter);

        if ((notifiee.nameOrHandle == Notifiee::kName) &&
            (notifiee.machine == machine) && (notifiee.process == process) &&
            (notifiee.priority == priority))
        {
            foundIt = 1;
        }
        else
        {
            ++listIter;
        }
    }

    if (!foundIt)
    {
        fNotifieeList.push_back(
            Notifiee(machine, altEndpoint, process, priority));
    }

    return kSTAFOk;
}


STAFRC_t STAFNotificationList::unregister(const STAFString &machine,
                                          const STAFHandle_t handle,
                                          unsigned int priority)
{
    unsigned int foundIt = 0;
    STAFRC_t rc = kSTAFOk;
    STAFMutexSemLock semLock(fNotifieeListSem);
    NotifieeList::iterator listIter;

    for(listIter = fNotifieeList.begin();
        !foundIt && (listIter != fNotifieeList.end());)
    {
        Notifiee notifiee(*listIter);

        if ((notifiee.nameOrHandle == Notifiee::kHandle) &&
            (notifiee.machine == machine) && (notifiee.handle == handle) &&
            (notifiee.priority == priority))
        {
            foundIt = 1;
        }
        else
        {
            ++listIter;
        }
    }

    if (foundIt) fNotifieeList.erase(listIter);
    else rc = kSTAFNotifieeDoesNotExist;

    return rc;
}


STAFRC_t STAFNotificationList::unregister(const STAFString &machine,
                                          const STAFString &process,
                                          unsigned int priority)
{
    unsigned int foundIt = 0;
    STAFRC_t rc = kSTAFOk;
    STAFMutexSemLock semLock(fNotifieeListSem);
    NotifieeList::iterator listIter;

    for(listIter = fNotifieeList.begin();
        !foundIt && (listIter != fNotifieeList.end());)
    {
        Notifiee notifiee(*listIter);

        if ((notifiee.nameOrHandle == Notifiee::kName) &&
            (notifiee.machine == machine) && (notifiee.process == process) &&
            (notifiee.priority == priority))
        {
            foundIt = 1;
        }
        else
        {
            ++listIter;
        }
    }

    if (foundIt) fNotifieeList.erase(listIter);
    else rc = kSTAFNotifieeDoesNotExist;

    return rc;
}


unsigned int STAFNotificationList::sendNotification(const STAFString &type,
                                                    const STAFString &message)
{
    unsigned int numSent = 0;
    NotifieeList notifieeList;

    // Lock the list temporarily to get a copy
    {
        STAFMutexSemLock semLock(fNotifieeListSem);
        notifieeList = fNotifieeList;
    }

    // Send the message to each notifiee in the list

    NotifieeList::iterator listIter;

    for(listIter = notifieeList.begin(); listIter != notifieeList.end();
        ++listIter)
    {
        Notifiee notifiee = *listIter;

        // Construct the QUEUE service request to send the message to the
        // notifiee

        STAFString requestWithoutMsg("QUEUE");

        if (notifiee.nameOrHandle == Notifiee::kHandle)
        {
            // Send the message to the notifiee's handle number
            requestWithoutMsg += " HANDLE " + STAFString(notifiee.handle);
        }
        else
        {
            // Send the message to the notifiee's handle name
            requestWithoutMsg += " NAME :" +
                STAFString(notifiee.process.length(STAFString::kChar)) + ":" +
                notifiee.process;
        }

        if (type.length() != 0)
        {
            requestWithoutMsg += " TYPE :" +
                STAFString(type.length(STAFString::kChar)) + ":" + type;
        }

        requestWithoutMsg += " PRIORITY " + STAFString(notifiee.priority);

        STAFString request = requestWithoutMsg + " MESSAGE :" +
            STAFString(message.length(STAFString::kChar)) + ":" + message;

        // Send the message to the notifiee

        STAFResultPtr result = gSTAFProcHandlePtr->submit(
            notifiee.machine, "QUEUE", request);

        if (result->rc != kSTAFOk)
        {
            // If the QUEUE request failed with RC 22, retry after a delay

            if (result->rc == kSTAFCommunicationError)
            {
                // Sleep a random number of milliseconds between 1 and the
                // gConnectionRetryDelay

                int delayTime = rand() % gConnectionRetryDelay;
                STAFThreadManager::sleepCurrentThread(delayTime);

                // Retry sending the process completion message to the
                // notifiee's "machine" endpint

                result = gSTAFProcHandlePtr->submit(
                    notifiee.machine, "QUEUE", request);
            }

            // If the QUEUE request failed with RC 16 using the notifiee's
            // "machine" as the endpoint (e.g. with "Error resolving host
            // name"), then try using the notifiee's altEndpoint if it isn't
            // blank.  The "altEndpoint" uses the physical interface ID
            // (e.g. IP address if using the STAF TCP Connection Provider)
            // instead of the logical interface ID (host name).

            bool useAltEndpoint = false;

            if ((result->rc == kSTAFNoPathToMachine) &&
                (notifiee.altEndpoint.length() != 0))
            {
                useAltEndpoint = true;

                // Try sending the message to the notifiee's altEndpoint

                result = gSTAFProcHandlePtr->submit(
                    notifiee.altEndpoint, "QUEUE", request);
            }

            if (result->rc != kSTAFOk)
            {
                // Log a trace warning message

                // Log the request in the warning message with the MESSAGE
                // option value formatted in a readable way..
                // XXX: If add support to the tracing infrastructure to
                //      support handling "nested" marshalled data, then should
                //      remove here.

                STAFString unmarshalledRequest = requestWithoutMsg +
                    " MESSAGE :" +
                    STAFString(message.length(STAFString::kChar)) + ":" +
                    STAFObject::unmarshall(message)->asFormattedString();

                STAFString errorBuffer = STAFString(
                    "STAFNotificationList::sendNotification failed with RC=") +
                    result->rc + ", Result=" + result->result +
                    ", where=" + notifiee.machine;

                if (useAltEndpoint)
                    errorBuffer += " (" + notifiee.altEndpoint + ")";

                errorBuffer += ", service=QUEUE, request=" +
                    unmarshalledRequest;

                STAFTrace::trace(kSTAFTraceWarning, errorBuffer);
            }
        }

        if (result->rc == kSTAFOk)
            ++numSent;
    }

    return numSent;
}


STAFNotificationList::NotifieeList STAFNotificationList::getNotifieeListCopy()
{
    STAFMutexSemLock semLock(fNotifieeListSem);

    return fNotifieeList;
}
