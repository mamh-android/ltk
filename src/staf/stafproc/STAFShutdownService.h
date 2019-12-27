/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ShutdownService
#define STAF_ShutdownService

#include "STAFService.h"
#include "STAFCommandParser.h"

class STAFShutdownService : public STAFService
{
public:

    STAFShutdownService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFShutdownService();

private:

    // Don't allow copy construction or assignment
    STAFShutdownService(const STAFShutdownService &);
    STAFShutdownService &operator=(const STAFShutdownService &);

    STAFServiceResult handleShutdown(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleNotify(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fShutdownParser;
    STAFCommandParser fNotifyParser;

    STAFMapClassDefinitionPtr fNotifieeClass;
};

#endif
