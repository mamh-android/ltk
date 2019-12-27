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

    STAFHandlePtr handlePtr;
    unsigned int rc = 0;

    rc = STAFHandle::create("STAF/Test/bufflen", handlePtr);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    STAFString local("local");
    STAFString echo("echo");
    STAFString echoHeader("echo ");
    STAFString star(kUTF8_STAR);
    STAFString testString;

    for (unsigned int i = 1; i < minLength; ++i)
        testString += star;

    cout << "Length:";

    for (unsigned int currLength = minLength; currLength < maxLength + 1;
         ++currLength)
    {
        STAFString request = echoHeader;

        for (unsigned int i = 0; i < currLength; ++i)
            request += star;

        testString += star;

        cout << " " << currLength;

        STAFResultPtr result = handlePtr->submit(local, echo, request);

        if (result->rc != 0)
        {
            cout << endl << "Length: " << currLength << endl
                 << "Error submitting request, RC: " << result->rc << endl;

            if (result->result.length() != 0)
                cout << "Additional info: " << result->result << endl;

            return 5;
        }

        if (result->result != testString)
        {
            cout << endl << "Length: " << currLength << endl
                 << "Mismatched result" << endl
                 << "Actual result length: " << result->result.length() << endl
                 << "Actual result: " << result->result << endl;
            return 6;
        }
    }

    return 0;
}
