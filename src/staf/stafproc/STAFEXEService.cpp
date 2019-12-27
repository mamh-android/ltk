/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFThread.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFException.h"
#include "STAFLocalConnection.h"
#include "STAFEXEService.h"
#include "STAFProcess.h"
#include "STAFEventSem.h"
#include "STAFThreadManager.h"

// This is necessary because <types.h> defines a sleep macro which
// interferes with ICurrentThread::sleep()

#undef sleep

// XXX: Should there be a thread that monitors a termination queue and
//      checks to see if these services die?  If so, what should it
//      do if it determines a service has died?

STEXEService::STEXEService(IString name, IString command, IString parms)
    : STService(name), fLocalName("STAFProc_Service_" + name), fParms(parms)
{
    STProcessManager::ProcessInitInfo initInfo;

    initInfo.command = command;
    initInfo.parms = name;

    STProcessID pid = 0;
    unsigned int osRC = 0;
    STError::ID rc = STAFProcess::startProcess(initInfo, pid, osRC, 0);
    if (rc != STError::kOk)
    {
        STServiceCreateException startError("Base OS Error", rc);
        ITHROW(startError);
    }

    STEventSemPtr procInit;

    try
    {
        procInit = STEventSemPtr(new STEventSem("STAFProc_Service_" +
                                                name + "_Start"), IINIT);
    }
    catch (...)
    {
        STServiceCreateException startError("Error creating synch "
                                            "semaphore");
        ITHROW(startError);
    }

    if (procInit->wait(15000) != 0)
    {
        STServiceCreateException startError("Timeout waiting for process");
        ITHROW(startError);
    }

    // Wait for one second and then contact service to determine its
    // interface level

    gThreadManagerPtr->sleepCurrentThread(1000);

    IString errorString;
    unsigned int foundError = 0;

    try
    {
        STLocalConnection connection(fLocalName);

        connection.writeUInt(3);               // API Number
        connection.writeUInt(0);               // API Level

        unsigned int ack = connection.readUInt();

        if (ack)
        {
            foundError = 1;
            errorString = "Unsupported API level: " + 3;
        }
        else
        {
            unsigned int interfaceLevel = connection.readUInt();

            if (interfaceLevel != 1)
            {
                foundError = 1;
                errorString = "Unsupported interface level: " +
                              IString(interfaceLevel);
                connection.writeUInt(STError::kInvalidAPILevel);
            }
        }
    }
    catch (...)
    {
        foundError = 1;
        errorString = "Error communicating with service to determine "
                      "interface level";
    }

    if (foundError)
    {
        STServiceCreateException startError(errorString);
        ITHROW(startError);
    }
}


STEXEService::~STEXEService()
{
    /* Do Nothing */
}


IString STEXEService::info(unsigned int) const
{
    return name() + IString(": External EXE");
}


unsigned int STEXEService::init()
{
    STLocalConnection connection(fLocalName);

    connection.writeUInt(1);               // API Number
    connection.writeUInt(0);               // API Level

    unsigned int ack = connection.readUInt();

    if (ack) return ack;

    connection.writeIString(fParms);

    return connection.readUInt();
}


unsigned int STEXEService::term()
{
    STLocalConnection connection(fLocalName);

    connection.writeUInt(2);               // API Number
    connection.writeUInt(0);               // API Level

    unsigned int ack = connection.readUInt();

    if (ack) return ack;

    return connection.readUInt();
}


STServiceResult STEXEService::acceptRequest(IString client, STAFHandle handle,
                                            IString process, IString request)
{
    STLocalConnection connection(fLocalName);

    connection.writeUInt(0);               // API Number
    connection.writeUInt(0);               // API Level

    unsigned int ack = connection.readUInt();

    if (ack) return ack;

    connection.writeIString(client);
    connection.writeIString(process);
    connection.writeUInt(handle);
    connection.writeIString(request);

    unsigned int rc = connection.readUInt();
    IString result = connection.readIString();

    return STServiceResult(rc, result);
}
