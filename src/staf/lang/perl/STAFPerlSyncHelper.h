/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef __STAFPERLSYNCHELPER_H__
#define __STAFPERLSYNCHELPER_H__

#include "STAF.h"

typedef struct SyncData_t SyncData;
typedef struct SingleSync_t SingleSync;

SyncData *CreateSyncData();
SingleSync *GetSingleSync(SyncData *sd,
                          unsigned int request_number);
void PostSingleSyncByID(SyncData *sd,
                        unsigned int id,
                        STAFRC_t rc,
                        const char *err_str,
                        unsigned int len);
STAFRC_t WaitForSingleSync(SingleSync *ss,
                           STAFString_t *pErrorBuffer);
void ReleaseSingleSync(SyncData *sd,
                       SingleSync *ss);
void DestroySyncData(SyncData *sd);

#endif

