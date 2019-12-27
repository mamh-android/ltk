/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFServiceInterface.h"
#include "STAFString.h"

char message[] = "Hello World";

STAFRC_t STAFServiceGetLevelBounds(unsigned int levelID,
                                   unsigned int *minimum,
                                   unsigned int *maximum)
{
    switch (levelID)
    {
        case kServiceInfo:
        case kServiceInit:
        case kServiceAcceptRequest:
        *minimum = 1;
        *maximum = 1;
            break;
        case kServiceTerm:
        case kServiceDestruct:
            *minimum = 0;
        *maximum = 0;
            break;
        default:
            return kSTAFInvalidAPILevel;
    }

    return kSTAFOk;
}

STAFRC_t STAFServiceConstruct(STAFServiceHandle_t *pServiceHandle,
                              void *pServiceInfo, unsigned int infoLevel,
                              STAFString_t *pErrorBuffer)
{
    return kSTAFOk;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
                         void *pInitInfo, unsigned int initLevel,
                         STAFString_t *pErrorBuffer)
{
    return kSTAFOk;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                  void *pRequestInfo, unsigned int reqLevel,
                                  STAFString_t *pResultBuffer)
{
    unsigned int osRC = 0;
    return STAFStringConstructFromCurrentCodePage(pResultBuffer, message,
                                                  sizeof(message), &osRC);
}


STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                         void *pTermInfo, unsigned int termLevel,
                         STAFString_t *pErrorBuffer)
{
    return kSTAFOk;
}


STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                             void *pDestructInfo, unsigned int destructLevel,
                             STAFString_t *pErrorBuffer)
{
    return kSTAFOk;
}
