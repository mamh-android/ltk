/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFFileSystem.h"
#include "STAFString.h"
#include "STAFThreadManager.h"
#include "STAF_iostream.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <filename> <hold time in millis>"
             << endl;
        return 1;
    }

    STAFFSPath path(argv[1]);
    unsigned int sleepTime = STAFString(argv[2]).asUInt();

    if (!path.exists())
    {
        cout << argv[1] << " does not exist" << endl;
        return 1;
    }

    STAFFSEntryPtr entry = path.getEntry();

    cout << "Getting lock on " << argv[1] << endl;

    STAFFSEntryRLock rlock(entry);

    cout << "Sleeping for " << argv[2] << " milliseconds" << endl;

    STAFThreadManager::sleepCurrentThread(sleepTime);

    cout << "Releasing lock on " << argv[1] << endl;

    return 0;
}
