/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_LocalConnectionFactory
#define STAF_LocalConnectionFactory

#include "STAFString.h"
#include "STAFConnectionFactory.h"
#include "STAFLocalConnection.h"

class STAFLocalConnectionFactory : public STAFConnectionFactory
{
public:

    STAFLocalConnectionFactory(STAFString localName,
                               unsigned char instanceCount = 0xFF);

    virtual STAFConnAllocInfo acceptConnection();

    virtual ~STAFLocalConnectionFactory();

private:

    // Don't allow copy construction or assignment
    STAFLocalConnectionFactory(const STAFLocalConnectionFactory &);
    STAFLocalConnectionFactory &operator=(const STAFLocalConnectionFactory &);

    STAFString fPipeName;
    HPIPE fPipeHandle;
    unsigned char fInstanceCount;
};

#endif
