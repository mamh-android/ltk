/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFThread.h"
#include "STAFConnectionProvider.h"
#include "STAFDynamicLibrary.h"
#include "STAFEventSem.h"
#include "STAFTrace.h"

static STAFEventSem sStopServerSem;

STAFRC_t TestConnAcceptConnection(
    const STAFConnectionProvider *provider,
    STAFConnectionPtr &connection);

static int gOptionStartIndex = 4;

int main(int argc, char **argv)
{
    if (argc < gOptionStartIndex)
    {
        cout << "Usage: " << argv[0]
             << " <interface library> <server connect string | SERVER> "
             << "<text string> [option=value]..." << endl << endl
             << "Use a <text string> of \"end\" to terminate the server"
             << endl;
        return 1;
    }

    STAFTrace::traceOn(kSTAFTraceError);

    // Parse the connection provider options

    STAFStringPtr optionNames  = STAFStringPtr(new STAFString[argc -
                                                             gOptionStartIndex],
                                               STAFStringPtr::INIT,
                                               STAFStringPtr::ARRAY);
    STAFStringPtr optionValues = STAFStringPtr(new STAFString[argc -
                                                             gOptionStartIndex],
                                               STAFStringPtr::INIT,
                                               STAFStringPtr::ARRAY);
    STAFBuffer<STAFStringConst_t> implOptionNames(
        new STAFStringConst_t[argc - gOptionStartIndex],
        STAFBuffer<STAFStringConst_t>::INIT);
    STAFBuffer<STAFStringConst_t> implOptionValues(
        new STAFStringConst_t[argc - gOptionStartIndex],
        STAFBuffer<STAFStringConst_t>::INIT);

    for (int i = 0; i < (argc - gOptionStartIndex); ++i)
    {
        STAFString thisOption = argv[gOptionStartIndex + i];
        unsigned int equalPos = thisOption.find(kUTF8_EQUAL);
        STAFString thisOptionName = thisOption.subString(0, equalPos);
        STAFString thisOptionValue = (equalPos == STAFString::kNPos) ?
            STAFString() : thisOption.subString(equalPos + 1);

        optionNames[i]  = thisOptionName;
        implOptionNames[i] = optionNames[i].getImpl();
        optionValues[i] = thisOptionValue;
        implOptionValues[i] = optionValues[i].getImpl();
    }

    // Setup the connection provider structure

    STAFString mode = argv[2];
    STAFConnectionProviderConstructInfoLevel1 cpInfo =
        { mode.toUpperCase() == "SERVER" ? kSTAFConnectionProviderOutbound :
          kSTAFConnectionProviderInbound, 0 };

    cpInfo.mode                      = kSTAFConnectionProviderDuplex;
    cpInfo.numOptions                = argc - gOptionStartIndex;
    cpInfo.optionNames               = implOptionNames;
    cpInfo.optionValues              = implOptionValues;

    try
    {
        // Create the connection provider

        STAFConnectionProviderPtr provider =
            STAFConnectionProvider::createRefPtr("test", argv[1], &cpInfo, 1);

        // Do the work

        if (mode.toUpperCase() == "SERVER")
        {
            provider->start(TestConnAcceptConnection);

            cout << "Waiting for connections" << endl;

            sStopServerSem.wait();
        }
        else
        {
            STAFConnectionPtr conn = provider->connect(argv[2]);

            cout << "Made connection" << endl;

            STAFString testString(argv[3]);

            conn->writeString(testString);

            cout << "Wrote " << testString << endl;

            STAFString reply = conn->readString();

            cout << "Read " << reply << endl;
        }
    }
    catch (STAFException &e)
    { e.write("main()"); }
    catch (...)
    { cout << "Caught unknown exception in main()" << endl; }

    cout << "Done" << endl;

    return 0;
}


STAFRC_t TestConnAcceptConnection(
    const STAFConnectionProvider *provider,
    STAFConnectionPtr &conn)
{
    try
    {
        cout << "Received connection" << endl;

        STAFString request = conn->readString();

        cout << "Read " << STAFString(request) << endl;

        conn->writeString(request);

        cout << "Wrote " << STAFString(request) << endl;

        if (request.lowerCase() == "end") sStopServerSem.post();
    }
    catch (STAFException &e)
    { e.write("TestConnAcceptConnection"); }
    catch (...)
    { cout << "Caught unknown exception in TestConnAcceptConnection()" << endl; }

    return kSTAFOk;
}
