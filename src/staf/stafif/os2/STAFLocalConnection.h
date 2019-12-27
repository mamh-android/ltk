/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_LocalConnection
#define STAF_LocalConnection

#include "STAFConnection.h"
#include "STAFString.h"

// STAFLocalConnection - Derives from STAFConnection to provide support for
//                       Local IPC on OS/2 (using Named Pipes).

class STAFLocalConnection : public STAFConnection
{
public:

    STAFConnAllocInfo create(STAFString localName);
    STAFConnAllocInfo create(HPIPE pipeHandle);

    // Reads size bytes into buffer
    virtual void read(void *buffer, unsigned int size);

    // Writes size bytes from buffer
    virtual void write(void *buffer, unsigned int size);

    static void freeLocalConnection(STAFConnection *pConn);

    virtual ~STAFLocalConnection();

private:

    // Don't allow copy construction or assignment
    STAFLocalConnection(const STAFLocalConnection &);
    STAFLocalConnection &operator=(const STAFLocalConnection &);

    // User's must use the create functions
    STAFLocalConnection(STAFString localName);
    STAFLocalConnection(HPIPE pipeHandle);

    HPIPE fPipeHandle;
};

#endif
