/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPFileHeader
#define STAF_ZIPFileHeader


class STAFZipFileHeader
{
public:

    uLong signature;

    unsigned short versionMadeBy;

    unsigned short versionNeededToExtract;

    unsigned short generalPurposeBitFlag;

    unsigned short compressionMethod;

    uLong lastModifiedTimeDate;

    uLong crc;

    uLong compressedSize;

    uLong uncompressedSize;

    unsigned short fileNameLength;

    unsigned short extraFieldLength;

    unsigned short fileCommentLength;

    unsigned short diskNumberStart;

    unsigned short internalFileAttributes;

    uLong externalFileAttributes;

    uLong localFileHeaderOffset;

    char *fileName;

    void *extraField;

    char *fileComment;

    uLong size;


    // methods

    STAFZipFileHeader();

    STAFZipFileHeader(void*);


    ~STAFZipFileHeader();
};

#endif
