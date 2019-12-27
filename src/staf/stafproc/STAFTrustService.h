/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_TrustService
#define STAF_TrustService

#include "STAFService.h"

// STAFTrustService - Handles requests dealing with Trust/Access

class STAFTrustService : public STAFService
{
public:

    STAFTrustService();

    virtual STAFString info(unsigned int raw = 0) const;

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual ~STAFTrustService();

private:

    // Don't allow copy construction or assignment
    STAFTrustService(const STAFTrustService &);
    STAFTrustService &operator=(const STAFTrustService &);

    STAFServiceResult handleSet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleGet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDelete(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fGetParser;
    STAFCommandParser fSetParser;
    STAFCommandParser fDeleteParser;
    STAFCommandParser fListParser;

    // Map class definitions for marshalled output
    STAFMapClassDefinitionPtr fTrustEntryClass;
};

#endif
