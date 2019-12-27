/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFException.h"
#include "STAFLocalConnectionFactory.h"

#define CHECKCONNECT(string) if (rc) {\
        STAFConnectionConnectException connectError(string, rc);\
        THROW_STAF_EXCEPTION(connectError);}

const unsigned int kPipeSize = 0x00000400;

// Note, that between the Factory constructor and the acceptConnection()
// method, there is always one outstanding Pipe.  This is so that a
// client can never try to open a non-existant pipe.  In other words, all
// of the servers pipes will not be closed at the same time.


STAFLocalConnectionFactory::STAFLocalConnectionFactory(STAFString localName,
    unsigned char instanceCount) : fPipeName("\\PIPE\\" + localName),
                                   fInstanceCount(instanceCount)
{
    APIRET rc = DosCreateNPipe((PSZ)fPipeName.toCurrentCodePage()->buffer(),
                               &fPipeHandle,
                               NP_NOINHERIT | NP_ACCESS_DUPLEX,
                               NP_WAIT | NP_TYPE_BYTE | NP_READMODE_BYTE |
                               fInstanceCount, kPipeSize, kPipeSize, 0);

    CHECKCONNECT("DosCreateNPipe");
}


STAFConnAllocInfo STAFLocalConnectionFactory::acceptConnection()
{
    HPIPE oldPipe = fPipeHandle;
    APIRET rc = DosCreateNPipe((PSZ)fPipeName.toCurrentCodePage()->buffer(),
                               &fPipeHandle,
                               NP_NOINHERIT | NP_ACCESS_DUPLEX,
                               NP_WAIT | NP_TYPE_BYTE | NP_READMODE_BYTE |
                               fInstanceCount, kPipeSize, kPipeSize, 0);

    CHECKCONNECT("DosCreateNPipe");

    rc = DosConnectNPipe(oldPipe);

    CHECKCONNECT("DosConnectNPipe");

    return STAFLocalConnection::create(oldPipe);
}


STAFLocalConnectionFactory::~STAFLocalConnectionFactory()
{
    DosClose(fPipeHandle);
}
