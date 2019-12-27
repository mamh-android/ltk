/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPLocalFileHeader
#define STAF_ZIPLocalFileHeader


class STAFZipLocalFileHeader
{

public:

    // attributes

    uLong signature;

    unsigned short versionNeededToExtract;

    unsigned short generalPurposeBitFlag;

    unsigned short compressionMethod;

    uLong lastModifiedTimeDate;

    uLong crc;

    uLong compressedSize;

    uLong uncompressedSize;

    unsigned short fileNameLength;

    unsigned short extraFieldLength;

    char *fileName;

    void *extraField;


    char *fullFileName;

    uLong offset;

    uLong size;



    // methods

    STAFZipLocalFileHeader();

    STAFZipLocalFileHeader(const char*, int);

    STAFRC_t flush(FILE*, STAFString*);

    STAFRC_t extract(FILE*, uLong, const char*, STAFString*);

    STAFRC_t extract(FILE*, uLong, const char*, STAFZipCentralDirExtension*,
                     void*, STAFString*);

    STAFRC_t doExtract(FILE*, uLong, FILE*, STAFString*);

    ~STAFZipLocalFileHeader();
};

#endif
