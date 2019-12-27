/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_MiscService
#define STAF_MiscService

#include "STAFService.h"
#include "STAFCommandParser.h"

#if defined(STAF_OS_TYPE_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

class STAFMiscService : public STAFService
{
public:

    STAFMiscService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFMiscService();

private:

    // Don't allow copy construction or assignment
    STAFMiscService(const STAFMiscService &);
    STAFMiscService &operator=(const STAFMiscService &);

    STAFServiceResult handleVersion(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleWhoAmI(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleWhoAreYou(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleThread(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handlePurge(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleSet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleTest(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDebug(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fThreadParser;
    STAFCommandParser fListParser;
    STAFCommandParser fQueryParser;
    STAFCommandParser fPurgeParser;
    STAFCommandParser fSetParser;
    STAFCommandParser fTestParser;
    STAFCommandParser fDebugParser;

    STAFMapClassDefinitionPtr fLogRecordClass;
    STAFObjectPtr fMapOfLists;
    STAFObjectPtr fListOfMaps;
    STAFMapClassDefinitionPtr fInterfaceClass;
    STAFMapClassDefinitionPtr fSettingsClass;
    STAFMapClassDefinitionPtr fPropertiesClass;
    STAFMapClassDefinitionPtr fListCacheClass;
    STAFMapClassDefinitionPtr fPurgeCacheClass;
    STAFMapClassDefinitionPtr fWhoamiClass;
    STAFMapClassDefinitionPtr fWhoAreYouClass;
    STAFMapClassDefinitionPtr fThreadClass;
    
    #if defined(STAF_OS_TYPE_WIN32) && defined(_DEBUG)
    _CrtMemState fFromMemState;
    #endif
};

#endif
