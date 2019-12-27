/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
#include "STAF.h"
#include "STAFTrace.h"

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipCentralDirEndRecord.h"


// constructor
STAFZipCentralDirEndRecord::STAFZipCentralDirEndRecord()
{

    signature = 0x06054b50;

    numberDisk = 0;

    numberDiskWithCentralDir = 0;

    numberEntry = 0;

    numberEntryWithCentralDir = 0;

    centralDirSize = 0;

    centralDirOffset = 0;

    commentLength = 0;

    comment = NULL;

}


// destructor
STAFZipCentralDirEndRecord::~STAFZipCentralDirEndRecord()
{
    /* Do Nothing */
}
