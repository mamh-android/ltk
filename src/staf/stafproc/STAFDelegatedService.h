/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DelegatedService
#define STAF_DelegatedService

#include "STAFService.h"

// STAFDelegatedService - Handles service requests via TCP/IP

class STAFDelegatedService : public STAFService
{
public:

    STAFDelegatedService(STAFString name, STAFString machine, 
                         STAFString toName);

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;
    virtual STAFString getLibName() const { return STAFString("<Delegated>"); }

    // XXX: In the future we'd like to provide additional information such as
    //     "Delegated to <fToName> on <fMachine>" (for a LIST SERVICES request)

    virtual ~STAFDelegatedService();

private:

    // Don't allow copy construction or assignment
    STAFDelegatedService(const STAFDelegatedService &);
    STAFDelegatedService &operator=(const STAFDelegatedService &);

    STAFString fMachine;
    STAFString fToName;
};

#endif
