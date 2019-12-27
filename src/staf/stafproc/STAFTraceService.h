/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_TraceService
#define STAF_TraceService

#include "STAFService.h"
#include "STAFTrace.h"
#include <map>

class STAFTraceService : public STAFService
{
public:

    STAFTraceService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFTraceService();

    typedef std::map<STAFTracePoint_t, STAFString> STAFTracepointMap;
    typedef std::map<STAFString, STAFTracePoint_t> STAFTracepointNameMap;
    static STAFTracepointMap fillSTAFTracepointMap();
    static STAFTracepointNameMap fillSTAFTracepointNameMap();
    static STAFTracepointNameMap kSTAFTracepointNameMap;
    static STAFTracepointMap kSTAFTracepointMap;

private:

    // Don't allow copy construction or assignment
    STAFTraceService(const STAFTraceService &);
    STAFTraceService &operator=(const STAFTraceService &);

    STAFServiceResult handleEnableDisable(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleSet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handlePurge(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFString spaces(int numSpaces);

    STAFCommandParser fEnableDisableParser;
    STAFCommandParser fListParser;
    STAFCommandParser fSetParser;

    STAFMapClassDefinitionPtr fTraceInfoClass;
    STAFMapClassDefinitionPtr fTracepointClass;
    STAFMapClassDefinitionPtr fServiceClass;
};

#endif
