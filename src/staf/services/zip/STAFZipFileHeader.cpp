/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
#include "STAF.h"
#include "STAFString.h"
#include "STAFTrace.h"

#include <vector>
#include <map>

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipFileHeader.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipLocalFileHeader.h"


// constructor
STAFZipFileHeader::STAFZipFileHeader()
{
    signature = 0x02014b50;

    versionMadeBy = 0x0;

    versionNeededToExtract = 0;

    generalPurposeBitFlag = 0;

    compressionMethod = 0;

    lastModifiedTimeDate = 0;

    crc = 0;

    compressedSize = 0;

    uncompressedSize = 0;

    fileNameLength = 0;

    extraFieldLength = 0;

    fileCommentLength = 0;

    diskNumberStart = 0;

    internalFileAttributes = 0;  // file contains binary data

    externalFileAttributes = 0;  // external attribute set to 0

    localFileHeaderOffset = 0;

    fileName = NULL;

    extraField = NULL;

    fileComment = NULL;

    size = 46;
}


// constructor based on local file header
STAFZipFileHeader::STAFZipFileHeader(void *vlfh)
{
    signature = 0x02014b50;

    STAFZipLocalFileHeader *lfh = (STAFZipLocalFileHeader*)vlfh;

    versionMadeBy = 0x0;

    versionNeededToExtract = lfh->versionNeededToExtract;

    generalPurposeBitFlag = lfh->generalPurposeBitFlag;

    compressionMethod = lfh->compressionMethod;

    lastModifiedTimeDate = lfh->lastModifiedTimeDate;

    crc = lfh->crc;

    compressedSize = lfh->compressedSize;

    uncompressedSize = lfh->uncompressedSize;

    fileNameLength = lfh->fileNameLength;

    extraFieldLength = lfh->extraFieldLength;

    fileCommentLength = 0;

    diskNumberStart = 0;

    internalFileAttributes = 0;  // file contains binary data

    externalFileAttributes = 0;  // external attribute set to 0

    localFileHeaderOffset = lfh->offset;

    fileName = NULL;

    extraField = NULL;

    fileComment = NULL;

    if (fileNameLength > 0)
    {
        if ((fileName = (char*)calloc(fileNameLength + 1, 1)) == NULL)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFileHeader::STAFZipFileHeader_CP1")
                     + "Error allocating memory for file name length ["
                     + (fileNameLength + 1)
                     + "].\n");
        }
        else
        {
            strcpy(fileName, lfh->fileName);
        }
    }


    if (extraFieldLength > 0)
    {
        if ((extraField = (void*)calloc(extraFieldLength, 1)) == NULL)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFileHeader::STAFZipFileHeader_CP2")
                     + "Error allocating memory for extra field length ["
                     + (extraFieldLength + 1)
                     + "].\n");
        }
        else
        {
            memcpy(extraField, lfh->extraField, extraFieldLength);
        }
    }


    if (fileCommentLength > 0)
    {
        if ((fileComment = (char*)calloc(fileCommentLength + 1, 1)) == NULL)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFileHeader::STAFZipFileHeader_CP3")
                     + "Error allocating memory for file comment length ["
                     + (fileCommentLength + 1)
                     + "].\n");
        }
        else
        {
            strcpy(fileComment, ".");
        }
    }


    size = 46 + fileNameLength + extraFieldLength + fileCommentLength;
}


// destructor
STAFZipFileHeader::~STAFZipFileHeader()
{
    if (fileName != NULL)
    {
        free(fileName);
    }

    if (fileComment != NULL)
    {
        free(fileComment);
    }

    if (extraField != NULL)
    {
        free(extraField);
    }
}
