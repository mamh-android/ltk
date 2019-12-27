/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFRWSem.h"

STAFRC_t STAFRWSemConstruct(STAFRWSem_t *pRWSem, const char *name,
                            unsigned int *osRC)
{
    return STAFRWSemConstructCommon(pRWSem, name, osRC);
}


STAFRC_t STAFRWSemQuery(STAFRWSem_t rwsem, STAFRWSemInfo *rwSemInfo,
                        unsigned int *osRC)
{
    return STAFRWSemQueryCommon(rwsem, rwSemInfo, osRC);
}


STAFRC_t STAFRWSemReadLock(STAFRWSem_t rwsem, unsigned int timeout,
                           unsigned int *osRC)
{
    return STAFRWSemReadLockCommon(rwsem, timeout, osRC);
}


STAFRC_t STAFRWSemReadUnlock(STAFRWSem_t rwsem, unsigned int *osRC)
{
    return STAFRWSemReadUnlockCommon(rwsem, osRC);
}


STAFRC_t STAFRWSemWriteLock(STAFRWSem_t rwsem, unsigned int timeout,
                            unsigned int *osRC)
{
    return STAFRWSemWriteLockCommon(rwsem, timeout, osRC);
}


STAFRC_t STAFRWSemWriteUnlock(STAFRWSem_t rwsem, unsigned int *osRC)
{
    return STAFRWSemWriteUnlockCommon(rwsem, osRC);
}


STAFRC_t STAFRWSemDestruct(STAFRWSem_t *pRWSem, unsigned int *osRC)
{
    return STAFRWSemDestructCommon(pRWSem, osRC);
}
