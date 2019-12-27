/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_QueueService
#define STAF_QueueService

#include "STAFService.h"
#include "STAFCommandParser.h"

// STAFQueueService - Handles manipulating a handle's queue

class STAFQueueService : public STAFService
{
public:

    STAFQueueService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFQueueService();

private:

    // Don't allow copy construction or assignment
    STAFQueueService(const STAFQueueService &);
    STAFQueueService &operator=(const STAFQueueService &);

    STAFServiceResult handleQueue(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleGetPeek(const STAFServiceRequest &requestInfo,
                                    bool isGet);
    STAFServiceResult handleDelete(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fQueueParser;
    STAFCommandParser fGetParser;
    STAFCommandParser fDeleteParser;
    STAFCommandParser fListParser;

    // Map class definitions for marshalled results
    STAFMapClassDefinitionPtr fQueueEntryMapClass;
    STAFMapClassDefinitionPtr fQueueFullMapClass;
    STAFMapClassDefinitionPtr fQueueErrorMapClass;
};

#endif
