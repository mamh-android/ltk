/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ExternalService
#define STAF_ExternalService

#include <list>
#include "STAFService.h"
#include "STAFServiceInterface.h"
#include "STAFDynamicLibrary.h"

// STAFExternalService - Handles interfacing with all services that aren't
//                       delegated and don't exist within STAFProc itself.
//                       Essentially, this class interfaces with DLLs that
//                       implement services or act as proxies for services.

class STAFExternalService : public STAFService
{
public:

    typedef std::list<STAFString> OptionList;

    STAFExternalService(STAFString name, STAFString implLib, STAFString exec,
                        OptionList options, STAFString parms,
                        STAFServiceType_t serviceType,
                        STAFString loadedBySLS = STAFString(""));
    
    virtual STAFServiceResult init();
    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);
    virtual STAFServiceResult term();

    virtual STAFString info(unsigned int raw = 0) const;
    virtual STAFString getLibName() const;
    virtual STAFString getExecutable() const;
    virtual STAFString getParameters() const;
    virtual STAFObjectPtr getOptions() const;

    virtual ~STAFExternalService();

private:

    // Don't allow copy construction or assignment
    STAFExternalService(const STAFExternalService &);
    STAFExternalService &operator=(const STAFExternalService &);

    unsigned int getLevel(STAFServiceLevelID levelID, const char *errorString,
                          unsigned int stafMin, unsigned int stafMax);

    STAFServiceHandle_t fServiceHandle;
    STAFString fLibName;
    STAFDynamicLibrary fImplLib;
    STAFString fExec;
    STAFString *optionNames;
    STAFString *optionValues;
    STAFString fParms;
    STAFObjectPtr fOptions;

    STAFServiceGetLevelBounds_t fGetBounds;
    unsigned int fLevelConstruct;
    STAFServiceConstruct_t fConstruct;
    unsigned int fLevelInit;
    STAFServiceInit_t fInit;
    unsigned int fLevelAcceptRequest;
    STAFServiceAcceptRequest_t fAcceptRequest;
    unsigned int fLevelTerm;
    STAFServiceTerm_t fTerm;
    unsigned int fLevelDestruct;
    STAFServiceDestruct_t fDestruct;
};

#endif

