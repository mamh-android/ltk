/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DiagService
#define STAF_DiagService

#include "STAFService.h"

// STAFDiagService - Handles requests dealing with Diagnostics

typedef enum STAFDiagSortBy_e
{
    kSTAFDiagSortByCount = 0,
    kSTAFDiagSortByTrigger = 1,
    kSTAFDiagSortBySource = 2
} STAFDiagSortBy_t;

class STAFDiagService : public STAFService
{
public:

    STAFDiagService();

    virtual STAFString info(unsigned int raw = 0) const;

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual ~STAFDiagService();

private:

    // Don't allow copy construction or assignment
    STAFDiagService(const STAFDiagService &);
    STAFDiagService &operator=(const STAFDiagService &);

    STAFServiceResult handleRecord(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleReset(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleEnable(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDisable(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fRecordParser;
    STAFCommandParser fListParser;
    STAFCommandParser fResetParser;
    STAFCommandParser fEnableParser;
    STAFCommandParser fDisableParser;

    STAFMapClassDefinitionPtr fSettingsClass;
    STAFMapClassDefinitionPtr fAllDiagInfoClass;
    STAFMapClassDefinitionPtr fTriggerInfoClass;
    STAFMapClassDefinitionPtr fTriggersInfoClass;
    STAFMapClassDefinitionPtr fSourceInfoClass;
    STAFMapClassDefinitionPtr fSourcesInfoClass;
    STAFMapClassDefinitionPtr fComboCountClass;
    STAFMapClassDefinitionPtr fTriggerCountClass;
    STAFMapClassDefinitionPtr fSourceCountClass;
};

#endif
