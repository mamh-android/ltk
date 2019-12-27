/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <errno.h>
#include "STAF.h"
#include "STAFUtil.h"
#include "STAFConnection.h"
#include "STAFException.h"

STAFConnection::STAFConnection(ConnectionType conType, ConnectionMode conMode)
    : fConnectionType(conType), fConnectionMode(conMode)
{ /* Do Nothing */ }


STAFConnection::~STAFConnection()
{ /* Do Nothing */ }


STAFString STAFConnection::readString()
{
    // First read in the UTF-8 data

    unsigned int dataSize = readUInt();
    char *inputData = new char[dataSize];

    read((void *)inputData, dataSize);

    STAFString result;

    try
    {
        // Indicate UTF-8 data since default is Current Code Page
        result = STAFString(inputData, dataSize, STAFString::kUTF8);
    }
    catch(...)
    {
        delete [] inputData;
        throw;
    }

    delete [] inputData;

    return result;
}


void STAFConnection::readString(STAFString &theString)
{
    theString = readString();
}


void STAFConnection::writeString(const STAFString &theString)
{
    // If it is a zero length string, simply short-circuit and write
    // zero-length string

    if (theString.length() == 0)
    {
        writeUInt(0);
        return;
    }

    unsigned int len;
    const char *buffer = theString.buffer(&len);

    writeUInt(len);
    write((void *)buffer, len);
}
