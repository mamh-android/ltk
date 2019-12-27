/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"

int main(int argc, char **argv)
{
    STAFHandlePtr handlePtr;

    unsigned int rc = STAFHandle::create("STAF/TestPrivateData", handlePtr);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    unsigned int numTests = 15;

    STAFString testData[15][8] =
    {
        {
         "secret", "secret", "!!@secret@!!",
         "secret", "secret", "secret",
         "secret", "!!@secret@!!" 
        },
        {
         "!!@secret@!!", "************", "!!@secret@!!",
         "secret", "secret", "secret",
         "^!!@secret^@!!", "!!@^^!!@secret^^@!!@!!"
        },
        {
         "Pw: !!@pw@!!", "Pw: ********", "!!@Pw: ^!!@pw^@!!@!!",
         "Pw: !!@pw@!!", "Pw: pw", "Pw: pw",
         "Pw: ^!!@pw^@!!", "!!@Pw: ^^!!@pw^^@!!@!!"
        },
        {
         "^!!@secret@!!", "^!!@secret@!!", "!!@^^!!@secret^@!!@!!",
         "!!@secret@!!", "!!@secret@!!", "!!@secret@!!",
         "^^!!@secret^@!!", "!!@^^^!!@secret^^@!!@!!"
        },
        {
         "^!!@secret^@!!",
         "^!!@secret^@!!",
         "!!@^^!!@secret^^@!!@!!",
         "!!@secret@!!",
         "!!@secret@!!",
         "!!@secret@!!",
         "^^!!@secret^^@!!",
         "!!@^^^!!@secret^^^@!!@!!"
        },
        {
         "!!@secret", "!!@secret", "!!@^!!@secret@!!",
         "!!@secret", "!!@secret", "!!@secret",
         "^!!@secret", "!!@^^!!@secret@!!"
        },
        {
         "!!@secret^@!!", "!!@secret^@!!", "!!@^!!@secret^^@!!@!!",
         "!!@secret@!!", "!!@secret@!!", "!!@secret@!!",
         "^!!@secret^^@!!", "!!@^^!!@secret^^^@!!@!!"
        },
        {
         "Pw1=!!@a@!!, Pw2=!!@pw@!!.",
         "Pw1=*******, Pw2=********.",
         "!!@Pw1=^!!@a^@!!, Pw2=^!!@pw^@!!.@!!",
         "Pw1=!!@a@!!, Pw2=!!@pw@!!.",
         "Pw1=a, Pw2=pw.", "Pw1=a, Pw2=pw.",
         "Pw1=^!!@a^@!!, Pw2=^!!@pw^@!!.",
         "!!@Pw1=^^!!@a^^@!!, Pw2=^^!!@pw^^@!!.@!!"
        },
        {
         "^!!@a@!!^@!!b!!@",
         "^!!@a@!!^@!!b!!@",
         "!!@^^!!@a^@!!^^@!!b^!!@@!!",
         "!!@a@!!@!!b!!@",
         "!!@a@!!@!!b!!@",
         "!!@a@!!@!!b!!@",
         "^^!!@a^@!!^^@!!b^!!@",
         "!!@^^^!!@a^^@!!^^^@!!b^^!!@@!!"
        },
        {
         "Pw1=!!@secret, !!@pw@!!.",
         "Pw1=*******************.",
         "!!@Pw1=^!!@secret, ^!!@pw^@!!.@!!",
         "Pw1=!!@secret, !!@pw@!!.",
         "Pw1=secret, !!@pw.",
         "Pw1=secret, !!@pw.",
         "Pw1=^!!@secret, ^!!@pw^@!!.",
         "!!@Pw1=^^!!@secret, ^^!!@pw^^@!!.@!!"
        },
        {
         "Pw1=!!@secret@!!, !!@pw.",
         "Pw1=************, !!@pw.",
         "!!@Pw1=^!!@secret^@!!, ^!!@pw.@!!",
         "Pw1=!!@secret@!!, !!@pw.",
         "Pw1=secret, !!@pw.",
         "Pw1=secret, !!@pw.",
         "Pw1=^!!@secret^@!!, ^!!@pw.",
         "!!@Pw1=^^!!@secret^^@!!, ^^!!@pw.@!!"
        },
        {
         "Msg: !!@Pw: ^!!@pw^@!!@!!",
         "Msg: ********************",
         "!!@Msg: ^!!@Pw: ^^!!@pw^^@!!^@!!@!!",
         "Msg: !!@Pw: ^!!@pw^@!!@!!",
         "Msg: Pw: !!@pw@!!",
         "Msg: Pw: pw",
         "Msg: ^!!@Pw: ^^!!@pw^^@!!^@!!",
         "!!@Msg: ^^!!@Pw: ^^^!!@pw^^^@!!^^@!!@!!"
        },
        {
         "@!!a!!@b@!!",
         "@!!a*******", 
         "!!@^@!!a^!!@b^@!!@!!",
         "@!!a!!@b@!!",
         "@!!ab",
         "@!!ab",
         "^@!!a^!!@b^@!!",
         "!!@^^@!!a^^!!@b^^@!!@!!"
        },
        {
         "Msg: !!@Pw is ^^!!@secret^^@!!.@!!",
         "Msg: *****************************",
         "!!@Msg: ^!!@Pw is ^^^!!@secret^^^@!!.^@!!@!!",
         "Msg: !!@Pw is ^^!!@secret^^@!!.@!!",
         "Msg: Pw is !!@secret@!!.",
         "Msg: Pw is !!@secret@!!.",
         "Msg: ^!!@Pw is ^^^!!@secret^^^@!!.^@!!",
         "!!@Msg: ^^!!@Pw is ^^^^!!@secret^^^^@!!.^^@!!@!!"
        },
        {
         "", "", "", "", "", "", "", ""
        }
    };

    cout << "KEY:"
         << endl << "  apd() = STAFHandle::addPrivacyDelimiters()"
         << endl << "  mpd() = STAFHandle::maskPrivateData()"
         << endl << "  rpd() = STAFHandle::removePrivacyDelimiters()"
         << endl << "  epd() = STAFHandle::escapePrivacyDelimiters()"
         << endl;

    unsigned int numErrors = 0;
    
    for (int i = 0; i < numTests; ++i)
    {
        STAFString data(testData[i][0]);
        cout << endl << STAFString(i+1) << ")  data: " << data << endl << endl;

        // 1

        STAFString maskedData = STAFHandle::maskPrivateData(data);
        cout << "mpd(" << data << "): " << maskedData << endl;

        if (maskedData != testData[i][1])
        {
            cout << "ERROR(" << i << ", 1):  mpd(" << data << "): " << maskedData << endl
                 << "        Expected: " << testData[i][1] << endl;
            numErrors++;
        }
        
        // 2

        STAFString dataWithPrivacy = STAFHandle::addPrivacyDelimiters(data);
        cout << "apd(" << data << "): " << dataWithPrivacy << endl;

        if (dataWithPrivacy != testData[i][2])
        {
            cout << "ERROR(" << i << ", 2):  apd(" << data << "): " << dataWithPrivacy << endl
                 << "        Expected: " << testData[i][2] << endl;
            numErrors++;
        }

        // 3

        STAFString dataWithPrivacyRemoved =
            STAFHandle::removePrivacyDelimiters(dataWithPrivacy, 1);
        cout << "rpd(" << dataWithPrivacy << ", 1): " << dataWithPrivacyRemoved << endl;

        if (dataWithPrivacyRemoved != testData[i][3])
        {
            cout << "ERROR(" << i << ", 3):  rpd(" << dataWithPrivacy << "): "
                 << dataWithPrivacyRemoved << endl
                 << "        Expected: " << testData[i][3] << endl;
            numErrors++;
        }

        // 4

        STAFString dataWithPrivacyRemoved2 =
            STAFHandle::removePrivacyDelimiters(dataWithPrivacy, 2);
        cout << "rpd(" << dataWithPrivacy << ", 2): " << dataWithPrivacyRemoved2 << endl;
        
        if (dataWithPrivacyRemoved2 != testData[i][4])
        {
            cout << "ERROR(" << i << ", 4):  rpd(" << dataWithPrivacy << "): "
                 << dataWithPrivacyRemoved2 << endl
                 << "        Expected: " << testData[i][4] << endl;
            numErrors++;
        }

        // 5

        STAFString dataWithAllPrivacyRemoved =
            STAFHandle::removePrivacyDelimiters(dataWithPrivacy, 0);
        cout << "rpd(" << dataWithPrivacy << ", 0): " << dataWithAllPrivacyRemoved << endl;

        if (dataWithAllPrivacyRemoved != testData[i][5])
        {
            cout << "ERROR(" << i << ", 5):  rpd(" << dataWithPrivacy << "): "
                 << dataWithAllPrivacyRemoved << endl
                 << "        Expected: " << testData[i][5] << endl;
            numErrors++;
        }
        
        // 6

        STAFString escapedData = STAFHandle::escapePrivacyDelimiters(data);
        cout << "epd(" << data << "): " << escapedData << endl;

        if (escapedData != testData[i][6])
        {
            cout << "ERROR(" << i << ", 6):  epd(" << data << "): " << escapedData << endl
                 << "        Expected: " << testData[i][6] << endl;
            numErrors++;
        }
        
        // 7

        dataWithPrivacy = STAFHandle::addPrivacyDelimiters(
            escapedData);
        cout << "apd(" << escapedData << "): " << dataWithPrivacy << endl;

        if (dataWithPrivacy != testData[i][7])
        {
            cout << "ERROR(" << i << ", 7):  apd(" << escapedData << "): " << dataWithPrivacy << endl
                 << "        Expected: " << testData[i][7] << endl;
            numErrors++;
        }

        // 8

        dataWithPrivacyRemoved =
            STAFHandle::removePrivacyDelimiters(dataWithPrivacy, 1);
        cout << "rpd(" << dataWithPrivacy << ", 1): " << dataWithPrivacyRemoved << endl;
        
        if (dataWithPrivacyRemoved != data)
        {
            cout << "ERROR(" << i << ", 8):  rpd(" << dataWithPrivacy << ", 1): "
                 << dataWithPrivacyRemoved << endl
                 << "        Expected: " << data << endl;
            numErrors++;
        }

        // 9

        dataWithAllPrivacyRemoved =
            STAFHandle::removePrivacyDelimiters(dataWithPrivacy);
        cout << "rpd(" << dataWithPrivacy << "): " << dataWithAllPrivacyRemoved << endl;

        if (dataWithAllPrivacyRemoved != data)
        {
            cout << "ERROR(" << i << ", 8):  rpd(" << dataWithPrivacy << "): "
                 << dataWithAllPrivacyRemoved << endl
                 << "        Expected: " << data << endl;
            numErrors++;
        }
    }

    if (numErrors == 0)
    {
        cout << endl << "Test completed successfully" << endl;
    }
    else
    {
        cout << endl << "Test failed with " << STAFString(numErrors)
             << " errors" << endl;
    }

    return 0;
}
