/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_VariableService
#define STAF_VariableService

#include "STAFService.h"
#include "STAFCommandParser.h"

// STAFVariableService - Handles setting, querying, and resolving variables

class STAFVariableService : public STAFService
{
public:

    STAFVariableService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFVariableService();

private:

    // Don't allow copy construction or assignment
    STAFVariableService(const STAFVariableService &);
    STAFVariableService &operator=(const STAFVariableService &);

    STAFString fCommand;
    STAFString fParms;

    STAFCommandParser fVarParser;

    STAFMapClassDefinitionPtr fErrorClass;
    STAFMapClassDefinitionPtr fResolveStringClass;
};

#endif
