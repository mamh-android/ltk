/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_MutexSemInlImpl
#define STAF_MutexSemInlImpl

#include "STAF.h"
#include "STAFMutexSem.h"
#include "STAFRefPtr.h"
#include "STAFException.h"

STAF_INLINE STAFMutexSem::STAFMutexSem() : fMutexImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFMutexSemConstruct(&fMutexImpl, 0, &osRC);

    STAFException::checkRC(rc, "STAFMutexSemConstruct", osRC);
}


STAF_INLINE STAFRC_t STAFMutexSem::request(unsigned int timeout)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFMutexSemRequest(fMutexImpl, timeout, &osRC);

    if ((rc != kSTAFOk) && (rc != kSTAFTimeout))
        STAFException::checkRC(rc, "STAFMutexSemRequest", osRC);

    return rc;
}


STAF_INLINE void STAFMutexSem::release()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFMutexSemRelease(fMutexImpl, &osRC);

    STAFException::checkRC(rc, "STAFMutexSemRelease", osRC);
}


STAF_INLINE STAFMutexSem::~STAFMutexSem()
{
  unsigned int osRC = 0;
  STAFRC_t rc = STAFMutexSemDestruct(&fMutexImpl, &osRC);
}

#endif
