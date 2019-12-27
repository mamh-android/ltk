/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include "STAF.h"

unsigned int logIt(char *type, char *name, char *level, char *message);

// This could be a handy macro.
// Just set gLogType and gLogName to the appropriate values.
// Then you can use the macro as follows
//    LOGIT("INFO", "This is some data I want to log");

#define LOGIT(level, message) logIt(gLogType, gLogName, level, message)

char *gLogType = "GLOBAL";
char *gLogName = "MyLog";

char *gRegName = "Logit";
STAFHandle_t gHandle = 0;

int main(int argc, char **argv)
{
    unsigned int rc = 0;

    if (argc != 5)
    {
        printf("Usage: %s <Type> <Name> <Level> <Message>\n", argv[0]);
        return 1;
    }

    if ((rc = STAFRegister(gRegName, &gHandle)) != 0)
    {
        printf("Error registering with STAF, RC: %d\n", rc);
        return rc;
    }

    if ((rc = logIt(argv[1], argv[2], argv[3], argv[4])) != 0)
    {
        printf("Error logging data to STAF, RC: %d\n", rc);
        return rc;
    }

    if ((rc = STAFUnRegister(gHandle)) != 0)
    {
        printf("Error unregistering with STAF, RC: %d\n", rc);
        return rc;
    }

    return rc;
}

unsigned int logIt(char *type, char *name, char *level, char *message)
{
    static char *where = "LOCAL";
    static char *service = "LOG";
    static char buffer[4000] = { 0 };
    char *resultPtr = 0;
    unsigned int resultLength = 0;
    unsigned int rc = 0;

    sprintf(buffer, "LOG %s LOGNAME %s LEVEL %s MESSAGE :%d:%s", type, name,
            level, strlen(message), message);

    rc = STAFSubmit(gHandle, where, service, buffer, strlen(buffer),
                    &resultPtr, &resultLength);

    STAFFree(gHandle, resultPtr);

    return rc;
}
