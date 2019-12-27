/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_HelpService
#define STAF_HelpService

#include "STAFService.h"
#include "STAFCommandParser.h"
#include "STAFMutexSem.h"
#include <map>

class STAFHelpService : public STAFService
{
public:

    STAFHelpService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFHelpService();

    struct ServiceHelpData
    {
        ServiceHelpData(const STAFString &theInfo, const STAFString &theDetail)
            : info(theInfo), detail(theDetail)
        { /* Do Nothing */ }

        ServiceHelpData()
        { /* Do Nothing */ }

        STAFString info;
        STAFString detail;
    };

    typedef std::map<STAFString, ServiceHelpData> HelpServiceToInfoMap;
    typedef std::map<unsigned int, ServiceHelpData> HelpErrorToInfoMap;
    typedef std::map<STAFString, HelpErrorToInfoMap> HelpServiceMap;
    typedef std::map<unsigned int, HelpServiceToInfoMap> HelpErrorMap;

private:

    // Don't allow copy construction or assignment
    STAFHelpService(const STAFHelpService &);
    STAFHelpService &operator=(const STAFHelpService &);

    STAFServiceResult handleRegister(bool isRegister,
                                     const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleError(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fRegParser;
    STAFCommandParser fListParser;
    STAFCommandParser fErrorParser;

    STAFMutexSem fMapSem;
    HelpServiceMap fServiceMap;
    HelpErrorMap fErrorMap;
};

#endif
