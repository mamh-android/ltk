/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFLocalConnection.h"
#include "STAFException.h"

#ifdef DEBUG_LOCAL_CONNECTION
#undef sleep
#include "STAF_iostream.h"
#include "STAFThread.h"
#define MYTID STAFThreadGetCurrentThreadID()
#endif

#define CHECKCONNECT(string) if (rc) {\
        STAFConnectionConnectException connectError(string, rc);\
        THROW_STAF_EXCEPTION(connectError);}

#define CHECKIO(string) if (rc) {\
        STAFConnectionIOException ioError(string, rc);\
        THROW_STAF_EXCEPTION(ioError);}

STAFConnAllocInfo STAFLocalConnection::create(STAFString localName)
{
    return STAFConnAllocInfo(new STAFLocalConnection(localName),
                             &freeLocalConnection);
}


STAFLocalConnection::STAFLocalConnection(HPIPE pipeHandle)
{
    return STAFConnAllocInfo(new STAFLocalConnection(pipeHandle),
                             &freeLocalConnection);
}


STAFLocalConnection::STAFLocalConnection(STAFString localName)
    : STAFConnection(kLocal, kClient)
{
    APIRET rc = 0;
    ULONG openAction;
    STAFString pipeName(STAFString("\\PIPE\\") + localName);
    STAFStringBufferPtr pipeNameBuf = pipeName.toCurrentCodePage();

    rc = DosOpen((PSZ)pipeNameBuf->buffer(), &fPipeHandle, &openAction, 0, 0,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, 0);

    while ((rc == ERROR_PIPE_BUSY) || (rc == ERROR_SEM_TIMEOUT))
    {
        rc = DosWaitNPipe((PSZ)pipeNameBuf->buffer(), 100);

        if (!rc)
        {
            rc = DosOpen((PSZ)pipeNameBuf->buffer(), &fPipeHandle, &openAction,
                         0, 0, OPEN_ACTION_FAIL_IF_NEW |
                         OPEN_ACTION_OPEN_IF_EXISTS,
                         OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, 0);
        }
    }

    CHECKCONNECT("DosOpen");
}


STAFLocalConnection::STAFLocalConnection(HPIPE pipeHandle)
    : STAFConnection(kLocal, kServer), fPipeHandle(pipeHandle)
{
    /* Do Nothing */
}


void STAFLocalConnection::read(void *buffer, unsigned int size)
{
    APIRET rc = 0;
    ULONG actual = 0;

    for(ULONG current = 0; current < size; current += actual)
    {
        actual = 0;
        CHECKIO("DosRead");

        #ifdef DEBUG_LOCAL_CONNECTION
        cout << MYTID << ": Reading: " << (ULONG)(size - current) << " @ "
             << current << " of " << size << endl;
        #endif

        rc = DosRead(fPipeHandle, (PVOID)((char *)buffer + current),
                     (ULONG)(size - current), &actual);

        #ifdef DEBUG_LOCAL_CONNECTION
        cout << MYTID << ": Read   : " << actual << " @ " << current
             << " of " << size << " RC: " << rc << endl;
        #endif

        if ((actual == 0) && (size != 0))
        {
            STAFConnectionIOException noData("No data available on pipe");
            THROW_STAF_EXCEPTION(noData);
        }
    }

    #ifdef DEBUG_LOCAL_CONNECTION
    if (size == 4) cout << MYTID << ": RNum   : " << *((unsigned int *)buffer)
                        << endl;
    else if (size) cout << MYTID << ": RData  : "
                        << IString((char *)buffer, size) << endl;
    else           cout << MYTID << ": RZero" << endl;
    #endif
}


void STAFLocalConnection::write(void *buffer, unsigned int size)
{
    APIRET rc = 0;
    ULONG actual = 0;

    for(ULONG current = 0; current < size; current += actual)
    {
        actual = 0;
        #ifdef DEBUG_LOCAL_CONNECTION
        cout << MYTID << ": Writing: " << (ULONG)(size - current) << " @ "
             << current << " of " << size << endl;
        #endif

        rc = DosWrite(fPipeHandle, (PVOID)((char *)buffer + current),
                      (ULONG)(size - current), &actual);

        #ifdef DEBUG_LOCAL_CONNECTION
        cout << MYTID << ": Wrote  : " << actual << " @ " << current
             << " of " << size << " RC: " << rc << endl;
        #endif

        CHECKIO("DosWrite");

        rc = DosResetBuffer(fPipeHandle);

        // This if statement is here due to a bug (?) in Warp V3, where
        // DosResetBuffer returns ERROR_BROKEN_PIPE or ERROR_PIPE_NOT_CONNECTED
        // when the other side of the pipe has read the data and closed its end
        // of the pipe.  Warp V4 does not seem to exhibit this problem.
        //
        // This check should be fairly harmless, as, if this is the last
        // write then it doesn't matter if the pipe is now broken.  If
        // it isn't the last write then the error will be returned on
        // the next call, due to a failure on the DosWrite call.

        if ((rc != ERROR_BROKEN_PIPE) && (rc != ERROR_PIPE_NOT_CONNECTED))
        {
            CHECKIO("DosResetBuffer");
        }
    }

    #ifdef DEBUG_LOCAL_CONNECTION
    if (size == 4) cout << MYTID << ": WNum   : " << *((unsigned int *)buffer)
                        << " RC: " << rc << endl;
    else if (size) cout << MYTID << ": WData  : "
                        << IString((char *)buffer, size) << " RC: " << rc
                        << endl;
    else           cout << MYTID << ": WZero RC: " << rc << endl;
    #endif
}


void STAFLocalConnection::freeLocalConnection(STAFConnection *pConn)
{
    delete pConn;
}


STAFLocalConnection::~STAFLocalConnection()
{
    if (connectionMode() == kServer)
        DosDisConnectNPipe(fPipeHandle);

    DosClose(fPipeHandle);
}
