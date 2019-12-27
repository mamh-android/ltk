/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
#include "STAF.h"
#include "STAFString.h"
#include "STAFTrace.h"
#include "STAFFileSystem.h"


#include <sys/stat.h>
#include <vector>
#include <map>

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipFileHeader.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipLocalFileHeader.h"
#include "STAFZipCentralDirEndRecord.h"
#include "STAFZipCentralDir.h"

#include "STAFZipFile.h"


// Delete file

STAFRC_t STAFZipFile::deleteFile(const char *filename, STAFInt64_t *newsize,
                                 STAFString *result)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::deleteFile_CP1")
                     + " filename ["
                     + filename
                     + "]");

    if (newZipFile != 0)
    {
        *result = STAFString("STAFZipFile::deleteFile: ")
                  + STAFString("Invalid format in zip archive");
        return kZIPInvalidZipFile;
    }

    // get the file size

    STAFInt64_t filesize = endPos - startPos;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::deleteFile_CP4")
                     + " filesize ["
                     + filesize
                     + "]");

    STAFZipFileHeader *fh;
    STAFZipLocalFileHeader *lfh = find(filename);

    if (lfh == NULL)
    {
        *result = STAFString("STAFZipFile::deleteFile: ")
                  + "File name not found in zip ["
                  + filename
                  + "].\n";
        return kSTAFDoesNotExist;
    }

    // remove file header from central dir

    fh = centralDirPtr->remove(filename, lfh);

    // the last file in the zip archive

    if (centralDirPtr->getCentralDirEndRecord()->numberEntry <= 0)
    {
        // if this is the last file to be removed from zip archive
        // set the new size to be 0

        *newsize = 0;

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::deleteFile_CP6"));
        return kSTAFOk;
    }

    // remove local file header from zip file
    
    int c;
    fpos_t from, to;

    // move to the end of current local file header

    if (util->seek(zf, lfh->offset + lfh->size, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipFile::deleteFile: ")
                  + "Error in seek to end of local header ["
                  + (lfh->offset + lfh->size)
                  + "].\n";
        return kZIPGeneralZipError;
    }

    // set the from position
    
    fgetpos(zf, &from);

    // move to the beginning of current local file header
    
    if (util->seek(zf, lfh->offset, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipFile::deleteFile: ")
                  + "Error in seek to beginning of local header ["
                  + lfh->offset
                  + "].\n";
        return kZIPGeneralZipError;
    }

    // set the to position
    
    fgetpos(zf, &to);
    fsetpos(zf, &from);

    // move data from from position to to position

    while ((c = fgetc(zf))!= EOF)
    {
        fgetpos(zf, &from);
        fsetpos(zf, &to);

        fputc(c, zf);

        fgetpos(zf, &to);
        fsetpos(zf, &from);
    }

    // move to the beginning of central dir

    if (util->seek(zf, centralDirPtr->getOffset(), SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipFile::deleteFile: ")
                  + "Error in seek to beginning of central dir ["
                  + centralDirPtr->getOffset()
                  + "].\n";
        return kZIPGeneralZipError;
    }

    // flush central dir

    STAFRC_t rc = centralDirPtr->flush(zf, result);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::deleteFile_CP10")
                     + " rc ["
                     + rc
                     + "]");

    if (rc == kSTAFOk)
    {
        *newsize = filesize - fh->size - lfh->size;

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::deleteFile_CP11")
                     + " *newsize ["
                     + *newsize
                     + "]");
    }

    delete fh;

    return rc;

}

// list the content of zip archive

STAFRC_t STAFZipFile::listFile(STAFString *buf, STAFString *result)
{

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::listFile_CP1"));

    if (newZipFile != 0)
    {
        *result = STAFString("STAFZipFile::listFile: ")
                  + STAFString("Invalid format in zip archive");
        return kZIPInvalidZipFile;
    }

    return centralDirPtr->list(buf, result);
}

// Read in local file header data from Zip archive

STAFRC_t STAFZipFile::readInData(STAFString *result)
{
    STAFInt64_t curPos = util->tell(zf);

    if (curPos == -1)
    {
        *result = STAFString("STAFZipFile::readInData: ") +
            "Error getting current position using tell";
        return kZIPGeneralZipError;
    }

    STAFRC_t rc;

    std::vector<STAFZipFileHeader*>::iterator i;

    // get local file headers
    
    for (i = centralDirPtr->fileHeaderList.begin(); 
         i != centralDirPtr->fileHeaderList.end();
         i++)
    {
        STAFZipLocalFileHeader *lfh = new STAFZipLocalFileHeader();

        lfh->offset = (*i)->localFileHeaderOffset;

        // move to the local file header
        
        if (util->seek(zf, lfh->offset + startPos, SEEK_SET) != 0)
        {
            *result = STAFString("STAFZipFile::readInData: ") +
                "Error in seek to local file header";
            return kZIPGeneralZipError;
        }

        // get signature

        rc = util->getLong(zf, &lfh->signature);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::readInData_CP1")
                         + " signature ["
                         + lfh->signature
                         + "]");

        // get version needed to extract

        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util->getShort(zf, &iL);

            lfh->versionNeededToExtract = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP2")
                             + " versionNeededToExtract ["
                             + lfh->versionNeededToExtract
                             + "]");
        }

        // get general purpose bit flag

        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util->getShort(zf, &iL);

            lfh->generalPurposeBitFlag = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP3")
                             + " generalPurposeBitFlag ["
                             + lfh->generalPurposeBitFlag
                             + "]");
        }

        // get compression method

        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util->getShort(zf, &iL);

            lfh->compressionMethod = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP4")
                             + " compressionMethod ["
                             + lfh->compressionMethod
                             + "]");
        }

        // get last mod time date

        if (rc == kSTAFOk)
        {
            rc = util->getLong(zf, &lfh->lastModifiedTimeDate);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP5")
                             + " lastModifiedTimeDate ["
                             + lfh->lastModifiedTimeDate
                             + "]");
        }

        // get CRC32
        
        if (rc == kSTAFOk)
        {
            rc = util->getLong(zf, &lfh->crc);

            if (rc == kSTAFOk && lfh->crc == 0 && (*i)->crc != 0)
            {
                lfh->crc = (*i)->crc;
            }

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP6")
                             + " crc ["
                             + lfh->crc
                             + "]");
        }

        // get compressed size
        
        if (rc == kSTAFOk)
        {
            rc = util->getLong(zf, &lfh->compressedSize);

            if (rc == kSTAFOk && lfh->compressedSize == 0 && (*i)->compressedSize != 0)
            {
                lfh->compressedSize = (*i)->compressedSize;
            }

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP7")
                             + " compressedSize ["
                             + lfh->compressedSize
                             + "]");
        }

        // get uncompressed size

        if (rc == kSTAFOk)
        {
            rc = util->getLong(zf, &lfh->uncompressedSize);

            if (rc == kSTAFOk && lfh->uncompressedSize == 0 && (*i)->uncompressedSize != 0)
            {
                lfh->uncompressedSize = (*i)->uncompressedSize;
            }

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP8")
                             + " uncompressedSize ["
                             + lfh->uncompressedSize
                             + "]");
        }

        // get file name length

        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util->getShort(zf, &iL);

            lfh->fileNameLength = (unsigned short)iL;

            lfh->size += lfh->fileNameLength;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP9")
                             + " fileNameLength ["
                             + lfh->fileNameLength
                             + "] "
                             + "size ["
                             + lfh->size
                             + "]");
        }

        // get extra field length

        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util->getShort(zf, &iL);

            lfh->extraFieldLength = (unsigned short)iL;

            lfh->size += lfh->extraFieldLength;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::readInData_CP10")
                             + " extraFieldLength ["
                             + lfh->extraFieldLength
                             + "] "
                             + "size ["
                             + lfh->size
                             + "]");
        }

        // get file name

        unsigned char* buf;

        if (rc == kSTAFOk && lfh->fileNameLength > 0)
        {
            buf = (unsigned char*)calloc(lfh->fileNameLength + 1, 1);
            if (buf == NULL)
            {
                *result = STAFString("STAFZipFile::readInData: ")
                          + "Error allocating memory for file name length ["
                          + (lfh->fileNameLength + 1)
                          + "].\n";

                return kZIPNotEnoughMemory;
            }

            if (fread(buf, (uInt)lfh->fileNameLength, 1, zf) !=1 )
            {
                *result = STAFString("STAFZipFile::readInData: ")
                          + "Error reading filename.\n";

                rc = kZIPGeneralZipError;
            }

            lfh->fileName = (char*)buf;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP11")
                             + " fileName ["
                             + lfh->fileName
                             + "]");
        }

        // get extra field

        if (rc == kSTAFOk && lfh->extraFieldLength > 0)
        {
            buf = (unsigned char*)calloc(lfh->extraFieldLength, 1);
            if(buf == NULL)
            {
                *result = STAFString("STAFZipFile::readInData: ")
                          + "Error allocating memory for extra field length ["
                          + lfh->extraFieldLength
                          + "].\n";

                return kZIPNotEnoughMemory;
            }

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP12")
                             + " extraField offset ["
                             + util->tell(zf)
                             + "]");

            if (fread(buf, (uInt)lfh->extraFieldLength, 1, zf) != 1)
            {
                *result = STAFString("STAFZipFile::readInData: ")
                          + "Error reading extra field.\n";

                rc = kZIPGeneralZipError;
            }

            lfh->extraField = (void*)buf;
                        
        }

        if (rc != kSTAFOk)
        {
            if ((*result).length() != 0)
            {
                *result = STAFString("STAFZipFile::readInData: ")
                          + "Error reading local file header data.\n";
            }

            break;
        }

        lfh->size = 30 + lfh->fileNameLength + lfh->extraFieldLength +
            lfh->compressedSize;

        localFileHeaderListCurrent.push_back(lfh);
        localFileHeaderListCurrentSorted[STAFString(lfh->fileName)] = lfh;
    }


    if (util->seek(zf, curPos, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipFile::readInData: ")
                  + STAFString("Error in seek to original position");

        return kZIPGeneralZipError;
    }

    return rc;
}


// find a file name in local file header list

STAFZipLocalFileHeader* STAFZipFile::find(const char *fileName)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::find_CP1")
                         + " fileName ["
                         + fileName
                         + "]");

    std::map<STAFString, STAFZipLocalFileHeader*>::iterator i;

    i = localFileHeaderListCurrentSorted.find(
        STAFString(fileName).replace(kUTF8_BSLASH, kUTF8_SLASH));

    if (i != localFileHeaderListCurrentSorted.end())
        return i->second;
    else
        return NULL;
}


// find all matching file names in local file header list

std::vector<STAFString> STAFZipFile::findAll(const char *fileName)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::findAll_CP1")
                         + " fileName ["
                         + fileName
                         + "]");

    std::vector<STAFString> nameList;

    std::vector<STAFZipLocalFileHeader*>::iterator i;
    
    for (i = localFileHeaderListCurrent.begin(); i != localFileHeaderListCurrent.end();
         i++)
    {
        STAFString fileNameInZip = STAFString((*i)->fileName);

        if (fileNameInZip.find(STAFString(fileName)) != STAFString::kNPos)
            nameList.push_back(fileNameInZip);            
    }

    return nameList;        
}


// find dir name in local file header list

std::vector<STAFString> STAFZipFile::findDir(const char *dirName)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::findDir_CP1")
                         + " dirName ["
                         + dirName
                         + "]");

    std::vector<STAFString> nameList;

    std::vector<STAFZipLocalFileHeader*>::iterator i;
    
    STAFString theDirName = STAFString(dirName).replace(kUTF8_BSLASH, kUTF8_SLASH);
    
    if (theDirName.findLastOf(STAFString(kUTF8_SLASH)) != (theDirName.length() - 1))
    {
        theDirName += STAFString(kUTF8_SLASH);
    }
    
    for (i = localFileHeaderListCurrent.begin(); i != localFileHeaderListCurrent.end();
         i++)
    {
        STAFString fileNameInZip = STAFString((*i)->fileName);

        if (fileNameInZip.find(theDirName) == 0)
            nameList.push_back(fileNameInZip);            
    }

    return nameList;        
}
