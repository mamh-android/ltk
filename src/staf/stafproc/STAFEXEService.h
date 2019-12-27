/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_EXEService
#define STAF_EXEService

#include "STAFService.h"

// STAFEXEService - Handles service requests for services residing in
//                  externel executables that communicate via local ipc

class STAFEXEService : public STAFService
{
public:

    STAFEXEService(STAFString name, STAFString command,
                   STAFString parms = "");

    virtual STAFServiceResult acceptRequest(STAFString client, 
                                            STAFHandle handle,
                                            STAFString process, 
                                            STAFString request);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFEXEService();

private:

    // Don't allow copy construction or assignment
    STAFEXEService(const STAFEXEService &);
    STAFEXEService &operator=(const STAFEXEService &);

    virtual unsigned int init();
    virtual unsigned int term();

    STAFString fLocalName;
    STAFString fParms;
};

#endif
