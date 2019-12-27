/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFString.h"

STAFResultPtr logIt(const STAFString &type, const STAFString &name,
                    const STAFString &level, const STAFString &message);

// This could be a handy macro.
// Just set gLogType and gLogName to the appropriate values.
// Then you can use the macro as follows
//    LOGIT("INFO", "This is some data I want to log");

#define LOGIT(level, message) logIt(gLogType, gLogName, level, message)

STAFString gLogType("GLOBAL");
STAFString gLogName("MyLog");
STAFString gRegName("Logit");
STAFHandlePtr gHandle;

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cout << "Usage: LogIt <Type> <Name> <Level> <Message>" << endl;
        return 1;
    }

    unsigned int rc = STAFHandle::create(gRegName, gHandle);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    STAFResultPtr result = logIt(argv[1], argv[2], argv[3], argv[4]);

    if (result->rc != 0)
    {
        cout << "Error logging to STAF, RC: " << result->rc
             << " RESULT: " << result->result << endl;
    }

    return result->rc;
}

STAFResultPtr logIt(const STAFString &type, const STAFString &name,
                    const STAFString &level, const STAFString &message)
{
    static STAFString where("LOCAL");
    static STAFString service("LOG");

    STAFString request("LOG " + type + " LOGNAME " + name + " LEVEL " +
                       level + " MESSAGE " + STAFHandle::wrapData(message));

    return gHandle->submit(where, service, request);
}
