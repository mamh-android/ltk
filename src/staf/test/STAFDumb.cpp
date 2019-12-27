/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFThread.h"

int main(int argc, char **argv)
{
    if ((argc < 2) || (argc > 3))
    {
        cout << "Usage: STAFDumb <time to wait in millis> [multi-handle]"
             << endl;
        return 1;
    }

    STAFHandlePtr handlePtr;
    STAFHandlePtr handlePtr2;
    STAFHandle_t handle3;

    unsigned int rc = STAFHandle::create("STAF/DumbClient", handlePtr);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    if ((argc > 2) && (atoi(argv[2]) > 0))
    {
        rc = STAFHandle::create("STAF/DumbClient/Second-handle", handlePtr2);

        if (rc != 0)
        {
            cout << "Error registering with STAF, RC: " << rc << endl;
            return rc;
        }

        rc = STAFRegister("STAF/DumbClient/Third-handle", &handle3);

        if (rc != 0)
        {
            cout << "Error registering with STAF, RC: " << rc << endl;
            return rc;
        }
    }

    int timeToWait = atoi(argv[1]);

    // cout << "Sleeping for " << timeToWait << " milliseconds" << endl;

    STAFThreadSleepCurrentThread(timeToWait, 0);

    return 0;
}
