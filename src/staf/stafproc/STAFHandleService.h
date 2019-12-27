/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_HandleService
#define STAF_HandleService

#include "STAFService.h"
#include "STAFCommandParser.h"

// STAFHandleService - Handles requests about process handles

class STAFHandleService : public STAFService
{
public:

    STAFHandleService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFHandleService();
    
    static STAFHandleService *getInstance()  
    { 
        return sHandleService; 
    }

private:

    // Don't allow copy construction or assignment
    STAFHandleService(const STAFHandleService &);
    STAFHandleService &operator=(const STAFHandleService &);
    
    static STAFHandleService *sHandleService;

    STAFServiceResult handleCreate(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDelete(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleAuthenticate(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleUnAuthenticate(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleSet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleNotify(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleUnregister(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fCreateParser;
    STAFCommandParser fDeleteParser;
    STAFCommandParser fNotifyParser;
    STAFCommandParser fQueryParser;
    STAFCommandParser fAuthParser;
    STAFCommandParser fUnAuthParser;
    STAFCommandParser fSetParser;
    STAFCommandParser fListParser;
    STAFCommandParser fUnregisterParser;

    STAFMapClassDefinitionPtr fQueryHandleClass;
    STAFMapClassDefinitionPtr fHandleInfoClass;
    STAFMapClassDefinitionPtr fHandleInfoLongClass;
    STAFMapClassDefinitionPtr fHandleSummaryClass;
    STAFMapClassDefinitionPtr fNotificationClass;
    STAFMapClassDefinitionPtr fNotificationLongClass;
};

#endif
