/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAFUtil.h"
#include "STAFConverter.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    /*
      Tests code page conversion.  Created when testing fix for bug #617232.
      Tests the roundtrip codepage conversion of characters (from original
      codepage to UTF8 and back to the original codepage).
     */

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <string>" << endl;
        return 1;
    }
    
    char *cpName = STAFConverter::determineCodePage();
    cout << "Codepage: " << cpName << endl;

    cout << "Input string                    : " << argv[1] << endl;
    cout << "Input string in hex             : ";

    for (unsigned int x = 0; x < strlen(argv[1]); ++x)
    {
        unsigned int currChar = static_cast<unsigned char>(argv[1][x]);
        if (currChar < 16) cout << "0";
        cout << hex << currChar << dec << " ";
    }
    
    cout << endl << "Input string in UTF-8           : ";

    STAFString testString(argv[1]);
    const char *utfbuffer = testString.buffer();
    
    for (unsigned int y = 0; y < testString.length(); ++y)
    {
        unsigned int currChar = static_cast<unsigned char>(utfbuffer[y]);
        if (currChar < 16) cout << "0";
        cout << hex << currChar << dec << " ";
    }
    
    cout << endl << "Input string back in original cp: ";

    unsigned int totalLength = testString.length();

    STAFStringBufferPtr testStringPtr = testString.toCurrentCodePage();
    char *buffer = const_cast<char *>(testStringPtr->buffer());

    for (unsigned int z = 0; z < testStringPtr->length(); ++z)
    {
        unsigned int currChar = static_cast<unsigned char>(buffer[z]);
        if (currChar < 16) cout << "0";
        cout << hex << currChar << dec << " ";
    }

    cout << endl;

    return 0;
}
