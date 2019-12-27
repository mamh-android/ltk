/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2012                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ConfigService
#define STAF_ConfigService

#include "STAFService.h"
#include "STAFCommandParser.h"

class STAFConfigService : public STAFService
{
public:

    STAFConfigService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFConfigService();

private:

    // Don't allow copy construction or assignment
    STAFConfigService(const STAFConfigService &);
    STAFConfigService &operator=(const STAFConfigService &);

    STAFServiceResult handleSave(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fSaveParser;
};

#endif
