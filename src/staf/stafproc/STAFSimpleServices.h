/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_InternalServices
#define STAF_InternalServices

#include <map>
#include "STAFService.h"
#include "STAFEventSem.h"
#include "STAFCommandParser.h"

class STAFPingService : public STAFService
{
public:

    STAFPingService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFPingService();

private:

    // Don't allow copy construction or assignment
    STAFPingService(const STAFPingService &);
    STAFPingService &operator=(const STAFPingService &);

    static STAFString sPing;
    static STAFString sPong;
    static STAFString sHelpMsg;
};


class STAFEchoService : public STAFService
{
public:

    STAFEchoService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFEchoService();

private:

    // Don't allow copy construction or assignment
    STAFEchoService(const STAFEchoService &);
    STAFEchoService &operator=(const STAFEchoService &);

    static STAFString sHelpMsg;
};


class STAFDelayService : public STAFService
{
public:

    STAFDelayService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFDelayService();

    typedef std::map<STAFString, STAFEventSemPtr> DelayRequestMap;

private:

    // Don't allow copy construction or assignment
    STAFDelayService(const STAFDelayService &);
    STAFDelayService &operator=(const STAFDelayService &);

    // DELAY STAF_CALLBACK command parser
    STAFCommandParser fSTAFCallbackParser;
    
    DelayRequestMap fDelayRequestMap;
    STAFMutexSem fDelayRequestMapSem;

    static STAFString sHelpMsg;
};

#endif
