/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ServiceService
#define STAF_ServiceService

#include "STAFService.h"

// STAFServiceService - Handles commands relating to services

class STAFServiceService : public STAFService
{
public:

    STAFServiceService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFServiceService();

private:

    // Don't allow copy construction or assignment
    STAFServiceService(const STAFServiceService &);
    STAFServiceService &operator=(const STAFServiceService &);

    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleFree(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleAdd(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleReplace(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleRename(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleRemove(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);
};

#endif
