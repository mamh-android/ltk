/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFException.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAFTimestamp.h"

STAFString massageArgument(char *theArg)
{
    STAFString arg(theArg);

    if (getenv("STAF_OLDCLI") != 0) return arg;

    if (arg.find(kUTF8_SPACE) != STAFString::kNPos)
    {
        if ((arg.find(kUTF8_DQUOTE) != 0) ||
            (arg.findLastOf(kUTF8_DQUOTE, STAFString::kNPos,
                               STAFString::kChar) !=
             arg.length(STAFString::kChar)))
        {
            arg = kUTF8_COLON + STAFString(arg.length(STAFString::kChar)) +
                  kUTF8_COLON + arg;
        }
    }

    return arg;
}


int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cout << "Usage: STAFLoop <# loops> <Where> <Service> "
             << "<Request>" << endl;
        return 1;
    }

    try
    {
        unsigned int staticHandle = 0;

        if (getenv("STAF_STATIC_HANDLE") != 0)
            staticHandle = strtoul(getenv("STAF_STATIC_HANDLE"), 0, 10);

        STAFHandlePtr handlePtr;
        unsigned int rc = 0;

        if (staticHandle != 0)
            rc = STAFHandle::create(staticHandle, handlePtr);
        else
            rc = STAFHandle::create("STAF/LoopTest", handlePtr);

        if (rc != 0)
        {
            cout << "Error registering with STAF, RC: " << rc << endl;
            return rc;
        }

        int loops = atoi(argv[1]);
        int numError = 0;
        STAFString where(argv[2]);
        STAFString service(argv[3]);
        STAFString request;
        int i = 0;

        for (i = 4; i < (argc - 1); i++)
        {
            request += massageArgument(argv[i]) + STAFString(" ");
        }

        request += massageArgument(argv[argc - 1]);

        STAFRelativeTime begin;

        for (i = 0; i < loops; i++)
        {
            STAFResultPtr result = handlePtr->submit(where, service, request);

            if (result->rc != 0) 
            {
                cout << "OOPS(" << i << "): RC     = " << result->rc << endl;
                cout << "           RESULT = " << result->result << endl;
                if (++numError > 3) break;
            }
        }

        unsigned int totTime = STAFRelativeTime() - begin;
        unsigned int avgTime = totTime/loops;
        double loopsPerSec = (static_cast<double>(loops) * 1000) / totTime;

        cout << "Total loops: " << loops << endl
             << "Total time : " << (totTime / 1000) << "." << setw(3)
             << setfill('0') << (totTime % 1000) << endl
             << "Avg. time  : " << (avgTime / 1000) << "." << setw(3)
             << setfill('0') << (avgTime % 1000) << endl
             << "Loops/sec  : " << loopsPerSec << endl;
    }
    catch (STAFException &se)
    { se.write("main()"); }
    catch (...)
    { cout << "Caught unknown exception" << endl; }

    return 0;
}


