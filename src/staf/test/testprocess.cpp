/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFProcess.h"
#include "STAFThread.h"
#include "STAFString.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <program>" << endl;
        return 1;
    }

    STAFString command(argv[1]);

    /* Part 1:  Show how to start a process and unmarshall its result to get
                the process RC 
     */

    cout << "Part 1: " << endl << endl;

    STAFHandlePtr handlePtr;
    unsigned int returnCode = 0;

    returnCode = STAFHandle::create("STAF/TestProcess", handlePtr);

    if (returnCode != 0)
    {
        cout << "Error registering with STAF, RC: " << returnCode << endl;
        return returnCode;
    }

    STAFString machine = STAFString("local");

    STAFResultPtr res = handlePtr->submit(
        machine, "PROCESS", "START COMMAND " + command + " WAIT 60000");

    if (res->rc != kSTAFOk)
    {
        cout << "PROCESS START request failed with RC=" << STAFString(res->rc)
             << " Result=" << res->result << endl;
        return res->rc;
    }

    cout << "Marshalled Data Result: " << endl << res->result << endl << endl;

    cout << "Pretty Printed Unmarshalled Data: " << endl
         << STAFObject::unmarshall(res->result)->asFormattedString()
         << endl << endl;

    // Unmarshall the data to get the marshalling context
    STAFObjectPtr mc = STAFObject::unmarshall(res->result);

    // Get the root object for the marshalling context (which is a map as a
    // PROCESS START WAIT request result returns a marshalled map)
    STAFObjectPtr processResultMap = mc->getRootObject();

    // Get the "rc" key from the map containing the process result data
    cout << "Process Return Code: " << processResultMap->get("rc")->asString()
         << endl;

    // Check if the process RC is 0

    if (processResultMap->get("rc")->asString() == "0")
        cout << "Process completed successfully" << endl;
    else
        cout << "Process failed" << endl;

    /* Part 2:  Using STAFProcessStart and STAFProcessStop to start/stop
                a process
     */

    cout << endl << endl << "Part 2: " << endl << endl;

    STAFProcessStartInfoLevel1 startInfo = { 0 };

    startInfo.command = command.getImpl();

    STAFProcessID_t pid = 0;
    STAFProcessHandle_t procHandle = 0;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFProcessStart(&pid, &procHandle, &startInfo, 1, &osRC);

    if (rc != kSTAFOk)
    {
        cout << "Error starting process, RC: " << rc << ", OS RC: " << osRC
             << endl;
        return 1;
    }

    cout << "Process ID: " << pid << endl
         << "Process handle: " << procHandle << endl << endl;

    cout << "Sleeping for 5 seconds" << endl;

    STAFThreadSleepCurrentThread(5000, 0);

    rc = STAFProcessStop(pid, kSTAFProcessStopWithSigKill, &osRC);

    if (rc != kSTAFOk)
    {
        cout << "Error stopping process, RC: " << rc << ", OS RC: " << osRC
             << endl;
        return 1;
    }

    /* Part 3:  Using STAFProcessStart2 and STAFProcessStop to start/stop
                a process
     */

    cout << endl << endl << "Part 3: " << endl << endl;

    startInfo.command = command.getImpl();

    pid = 0;
    procHandle = 0;
    osRC = 0;
    STAFString_t errorBuffer = 0;
    rc = STAFProcessStart2(&pid, &procHandle, &startInfo, 1, &osRC, &errorBuffer);

    if (rc != kSTAFOk)
    {
        cout << "Error starting process, RC: " << rc << ", OS RC: " << osRC
             << ", Result: " << STAFString(errorBuffer, STAFString::kShallow) << endl;
        return 1;
    }

    cout << "Process ID: " << pid << endl
         << "Process handle: " << procHandle << endl << endl;

    cout << "Sleeping for 5 seconds" << endl;

    STAFThreadSleepCurrentThread(5000, 0);

    rc = STAFProcessStop(pid, kSTAFProcessStopWithSigKill, &osRC);

    if (rc != kSTAFOk)
    {
        cout << "Error stopping process, RC: " << rc << ", OS RC: " << osRC
             << endl;
        return 1;
    }

    /* Part 4:  Using STAFProcessStart2 to verify get error message in result
                when an error occurs starting a process.     */

    cout << endl << endl << "Part 4: " << endl << endl;

    // Make the command invalid
    command += "XXX";

    startInfo.command = command.getImpl();
    
    pid = 0;
    procHandle = 0;
    osRC = 0;
    errorBuffer = 0;
    rc = STAFProcessStart2(&pid, &procHandle, &startInfo, 1, &osRC, &errorBuffer);

    if (rc != kSTAFOk)
    {
        cout << "Got expected error starting process" << endl
             << "  RC: " << rc << endl
             << "  OS RC: " << osRC << endl
             << "  Result: " << STAFString(errorBuffer, STAFString::kShallow) << endl;

        return 0;
    }
    else
    {
        cout << "Error:  Part 4 - Process started when it should have failed" << endl;
    }

    return 0;
}
