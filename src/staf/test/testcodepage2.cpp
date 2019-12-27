/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    /*
       Tests code page conversion.  Created when testing fix for bug #617232.
       Input is provided in a file and tests converting the data from the
       current code page to UTF-8 and from UTF-8 to the current code page.
       
       Set the STAFCODEPAGEOVERRIDE environment variable before running this
       program to change the codepage. e.g. SET STAFCDOEPAGEOVERRIDE=ibm-949
       Make sure that STAFCONVDIR is set correctly.
       
       You can use REXX to create an input file with the character(s) you want
       to test.  For example:
         D:\>rexx rexxtry call charout "cp1.out", x2c('825C')
         D:\>testcodepage2 c2u D:/test1.out
         Input data           : 82 5c
         UTF-8 data           : 5c 5c
         
         D:\>rexx rexxtry call charout "test2.out", x2c('e282a9efbfa5')
         D:\>testcodepage2 u2c D:/dev/src/stax/test2.out
         Input data           : e2 82 a9 ef bf a5
         Current codepage data: 5c a1 cd
    */

    if (argc != 3)
    {
        cout << "Usage:   " << argv[0] << " <C2U | U2C> <filename>" << endl;
        cout << "where C2U means from current code page to UTF-8" << endl;
        cout << "  and U2C means from UTF-8 to current code page" << endl;
        cout << "Example: " << argv[0] << " C2U C:/test/codepage.txt" << endl;
        cout << "Example: " << argv[0] << " U2C C:/test/utf8.txt" << endl;
        return 1;
    }

    STAFString direction = STAFString(argv[1]);
    direction.upperCase();

    if ((direction != "C2U") && (direction != "U2C"))
    {
        cout << "Usage: " << argv[0] << " <C2U | U2C> <filename>" << endl;
        return 1;
    }

    fstream inFile(argv[2], ios::in | STAF_ios_binary);

    if (!inFile)
    {
        cout << "ERROR: Cannot open file" << argv[2] << endl;
        return 1;
    }

    // Figure out how big the file is

    inFile.seekg(0, ios::end);
    unsigned int fileLength = (unsigned int)inFile.tellg();

    // Initialize the output buffer and read in the file

    STAFRefPtr<char> buffer(new char[fileLength + 1], STAFRefPtr<char>::INIT, 
                            STAFRefPtr<char>::ARRAY);

    inFile.seekg(0, ios::beg);
    inFile.read(buffer, fileLength);

    buffer[fileLength] = 0;

    cout << "Input data           : ";
    
    for (unsigned int x = 0; x < fileLength; ++x)
    {
        unsigned int currChar = static_cast<unsigned char>(buffer[x]);
        if (currChar < 16) cout << "0";
        cout << hex << currChar << dec << " ";
    }
    
    if (direction == "C2U")
    {
        STAFString testString(buffer, fileLength);

        // testString = testString.replace(kUTF8_NULL, kUTF8_NULL2);

        cout << endl << "UTF-8 data           : ";

        const char *utfbuffer = testString.buffer();
    
        for (unsigned int y = 0; y < testString.length(); ++y)
        {
            unsigned int currChar = static_cast<unsigned char>(utfbuffer[y]);
            if (currChar < 16) cout << "0";
            cout << hex << currChar << dec << " ";
        }
    }
    else if (direction == "U2C")
    {
        STAFString testString(buffer, fileLength, STAFString::kUTF8);
        
        cout << endl << "Current codepage data: ";
    
        STAFStringBufferPtr cpbuffer = testString.toCurrentCodePage();
    
        for (unsigned int y = 0; y < cpbuffer->length(); ++y)
        {
            unsigned int currChar = static_cast<unsigned char>(cpbuffer->buffer()[y]);
            if (currChar < 16) cout << "0";
            cout << hex << currChar << dec << " ";
        }
    }
    
    cout << endl;

    return 0;
}


