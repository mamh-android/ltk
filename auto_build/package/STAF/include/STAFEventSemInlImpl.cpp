/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_EventSemInlImpl
#define STAF_EventSemInlImpl

#include "STAF.h"
#include "STAFEventSem.h"
#include "STAFRefPtr.h"
#include "STAFString.h"
#include "STAFException.h"

STAF_INLINE STAFEventSem::STAFEventSem(const char *name) : fEventImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFEventSemConstruct(&fEventImpl, name, &osRC);

    STAFException::checkRC(rc, "STAFEventSemConstruct", osRC);
}


STAF_INLINE void STAFEventSem::post()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFEventSemPost(fEventImpl, &osRC);

    STAFException::checkRC(rc, "STAFEventSemPost", osRC);
}


STAF_INLINE void STAFEventSem::reset()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFEventSemReset(fEventImpl, &osRC);

    STAFException::checkRC(rc, "STAFEventSemReset", osRC);
}


STAF_INLINE STAFRC_t STAFEventSem::wait(unsigned int timeout)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFEventSemWait(fEventImpl, timeout, &osRC);

    if ((rc != kSTAFOk) && (rc != kSTAFTimeout))
        STAFException::checkRC(rc, "STAFEventSemWait", osRC);

    return rc;
}


STAF_INLINE STAFEventSemState_t STAFEventSem::query()
{
    unsigned int osRC = 0;
    STAFEventSemState_t state = kSTAFEventSemReset;
    STAFRC_t rc = STAFEventSemQuery(fEventImpl, &state, &osRC);

    STAFException::checkRC(rc, "STAFEventSemQuery: %d", osRC);

    return state;
}


STAF_INLINE STAFEventSem::~STAFEventSem()
{
  unsigned int osRC = 0;
  STAFRC_t rc = STAFEventSemDestruct(&fEventImpl, &osRC);
}

#endif
