/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPCentralDirEndRecord
#define STAF_ZIPCentralDirEndRecord


class STAFZipCentralDirEndRecord
{
public:

    // attributes

    uLong signature;

    unsigned short numberDisk;

    unsigned short numberDiskWithCentralDir;

    // Must be unsigned short so can contain 0 to 65,534 entries (where
    // if was a short could only contain (-32,767 to 32,767 entries)

    unsigned short numberEntry;

    unsigned short numberEntryWithCentralDir;

    uLong centralDirSize;

    uLong centralDirOffset;

    unsigned short commentLength;

    char *comment;
    
    // methods
    STAFZipCentralDirEndRecord();

    ~STAFZipCentralDirEndRecord();
};

#endif
