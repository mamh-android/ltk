/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <min length> <max length>" << endl;
        return 1;
    }

    STAFString minString(argv[1]);
    STAFString maxString(argv[2]);

    if (!minString.isDigits())
    {
        cout << "<min length> must be a positive integer" << endl
             << "You specified: " << minString << endl;
        return 2;
    }

    if (!maxString.isDigits())
    {
        cout << "<max length> must be a positive integer" << endl
             << "You specified: " << maxString << endl;
        return 2;
    }

    unsigned int minLength = minString.asUInt();
    unsigned int maxLength = maxString.asUInt();

    if (minLength == 0)
    {
        cout << "<min length> must be a positive integer" << endl
             << "You specified: " << minLength << endl;
        return 3;
    }

    if (maxLength == 0)
    {
        cout << "<max length> must be a positive integer" << endl
             << "You specified: " << maxLength << endl;
        return 3;
    }

    if (maxLength < minLength)
    {
        cout << "<max length> must be greater than or equal to minlength" << endl
             << "You specified <min length>: " << minLength << endl
             << "You specified <min length>: " << maxLength << endl;
        return 4;
    }

    STAFHandle_t handle = 0;
    unsigned int rc = 0;

    rc = STAFRegister("STAF/Test/bufflen", &handle);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    char *requestStart = "log global logname LogBuffLenTest level info message ";
    unsigned int requestStartLength = strlen(requestStart);
    char *fillString = new char[maxLength + 1];

    for (unsigned int i = 1; i < minLength; ++i)
        fillString[i - 1] = '*';

    fillString[minLength] = 0;

    cout << "Length:";

    for (unsigned int currLength = minLength; currLength < maxLength + 1;
         ++currLength)
    {
        char *request = new char[currLength + requestStartLength + 1];

        fillString[currLength - 1] = '*';
        fillString[currLength] = 0;

        strcpy(request, requestStart);
        strcat(request, fillString);

        cout << " " << currLength;

        char *result = 0;
        unsigned resultLength = 0;

        rc = STAFSubmit(handle, "local", "log", request,
                        currLength + requestStartLength,
                        &result, &resultLength);

        if (rc != 0)
        {
            cout << endl << "Length: " << currLength << endl
                 << "Error submitting request, RC: " << rc << endl;

            if (resultLength != 0)
                cout << "Additional info: " << result << endl;

            return 5;
        }

        delete [] request;
        STAFFree(handle, result);
    }

    return 0;
}
