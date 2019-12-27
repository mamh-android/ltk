/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFTimestamp.h"

int main(void)
{
    STAFRelativeTime begin;

    const char *data[] =
    {
        "20001031-09:27:16",
        "10/31/00-09:27:16",
        "10/31/2000-09:27:16",
        "20001031-09:27",
        "20001031-09",
        "20001031-",
        "20001031",
        "-09:27:16",
        "-09:27",
        "-09",
        "-",
        "",
        "10:11:12",
        "20000229",     // Begin errors
        "20001131",
        "19990229",
        "20010229",
        "06/29/69",     // Non-obvious, 69 == 2069 which is > sizeof(time_t)
        "-24:00:00",
        "-10:70:00",
        "-10:20:70",
        "-1:00:00",     // Must be 01:xx:yy
        "-10:1:00",
        "-10:01:1"
    };

    cout << "BEGIN STRING TESTS" << endl;

    int i = 0;

    for (i = 0; i < (sizeof(data) / sizeof(char *)); ++i)
    {
        try
        {
            cout << endl << "Test     : " << data[i] << endl;

            if (STAFTimestamp::isValidTimestampString(data[i]))
                cout << "Valid    : Yes" << endl;
            else
                cout << "Valid    : No" << endl;

            STAFTimestamp aTime(data[i]);

            cout << "Formatted: " << aTime.asString() << endl;
        }
        catch (STAFException &se)
        {
            cout << "Caught exception: " << se.getName() << endl;
        }
        catch (...)
        { cout << "Caught unknown exception in main()" << endl; }
    }


    unsigned int data2[][6] =
    {
        { 2000, 10, 31, 9 , 27, 16 },
        { 99  , 10, 31, 9 , 27, 16 },
        { 02  , 10, 31, 9 , 27, 16 },
        { 102 , 10, 31, 9 , 27, 16 },
        { 1999, 2 , 29, 9 , 27, 16 },     // Begin errors
        { 2000, 2 , 29, 9 , 27, 16 },
        { 2001, 2 , 29, 9 , 27, 16 },
        { 69  , 6 , 29, 9 , 27, 16 },     // Again non-obvious
        { 2000, 10, 31, 24, 27, 16 },
        { 2000, 10, 31, 9 , 60, 16 },
        { 2000, 10, 31, 9 , 27, 60 },
    };

    cout << endl << "BEGIN NUMERIC TESTS" << endl;

    for (i = 0; i < (sizeof(data2) / sizeof(unsigned int [6])); ++i)
    {
        try
        {
            cout << endl << "Test     : "
                 << data2[i][0] << ", " << data2[i][1] << ", "
                 << data2[i][2] << ", " << data2[i][3] << ", "
                 << data2[i][4] << ", " << data2[i][5] << endl;

            if (STAFTimestamp::isValidTimestampData(data2[i][0], data2[i][1],
                                                    data2[i][2], data2[i][3],
                                                    data2[i][4], data2[i][5]))
            {
                cout << "Valid    : Yes" << endl;
            }
            else
            {
                cout << "Valid    : No" << endl;
            }

            STAFTimestamp aTime(data2[i][0], data2[i][1], data2[i][2],
                                data2[i][3], data2[i][4], data2[i][5]);

            cout << "Formatted: " << aTime.asString() << endl;
        }
        catch (STAFException &se)
        {
            cout << "Caught exception: " << se.getName() << endl;
        }
        catch (...)
        { cout << "Caught unknown exception in main()" << endl; }
    }

    const char *data3[] = 
    {
        "20000101-00:00:00",
        "19991231-23:59:59"
    };

    cout << endl << "BEGIN ACCESSOR TESTS" << endl;

    for (i = 0; i < (sizeof(data3) / sizeof(char *)); ++i)
    {
        try
        {
            cout << endl << "Test     : " << data3[i] << endl;

            STAFTimestamp aTime(data3[i]);

            cout << "Year  : " << aTime.getYear() << endl;
            cout << "Month : " << aTime.getMonth() << endl;
            cout << "Day   : " << aTime.getDay() << endl;
            cout << "Hour  : " << aTime.getHour() << endl;
            cout << "Minute: " << aTime.getMinute() << endl;
            cout << "Second: " << aTime.getSecond() << endl;
        }
        catch (STAFException &se)
        {
            cout << "Caught exception: " << se.getName() << endl;
        }
        catch (...)
        { cout << "Caught unknown exception in main()" << endl; }
    }


    const char *data4[][2] = 
    {
        { "20001108"         , "20001109"          },
        { "20001108-10:11:12", "20001108-10:11:01" },
        { "10:11:12"         , "10:11:01"          },
        { "20001108-10:11:12", "20001108-10:11:12" },
        { "19991231-23:59:59", "20000101-00:00:00" }
    };

    cout << endl << "BEGIN RELATIONAL TESTS" << endl;

    for (i = 0; i < (sizeof(data4) / sizeof(char *[2])); ++i)
    {
        try
        {
            cout << endl << data4[i][0] << " == " << data4[i][1] << " : ";

            if (STAFTimestamp(data4[i][0]) == STAFTimestamp(data4[i][1]))
                cout << "Yes" << endl;
            else
                cout << "No" << endl;

            cout << data4[i][0] << " != " << data4[i][1] << " : ";

            if (STAFTimestamp(data4[i][0]) != STAFTimestamp(data4[i][1]))
                cout << "Yes" << endl;
            else
                cout << "No" << endl;

            cout << data4[i][0] << " <  " << data4[i][1] << " : ";

            if (STAFTimestamp(data4[i][0]) < STAFTimestamp(data4[i][1]))
                cout << "Yes" << endl;
            else
                cout << "No" << endl;

            cout << data4[i][0] << " <= " << data4[i][1] << " : ";

            if (STAFTimestamp(data4[i][0]) <= STAFTimestamp(data4[i][1]))
                cout << "Yes" << endl;
            else
                cout << "No" << endl;

            cout << data4[i][0] << " >  " << data4[i][1] << " : ";

            if (STAFTimestamp(data4[i][0]) > STAFTimestamp(data4[i][1]))
                cout << "Yes" << endl;
            else
                cout << "No" << endl;

            cout << data4[i][0] << " >= " << data4[i][1] << " : ";

            if (STAFTimestamp(data4[i][0]) >= STAFTimestamp(data4[i][1]))
                cout << "Yes" << endl;
            else
                cout << "No" << endl;
        }
        catch (STAFException &se)
        {
            cout << "Caught exception: " << se.getName() << endl;
        }
        catch (...)
        { cout << "Caught unknown exception in main()" << endl; }
    }

    STAFRelativeTime end;

    cout << endl << "Runtime: " << ((end - begin) / 1000) << "."
         << setw(3) << setfill('0') << ((end - begin) % 1000)
         << " seconds" << endl;

    return 0;
}
