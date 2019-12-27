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

#include "zlib.h"

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipFileHeader.h"
#include "STAFZipCentralDirEndRecord.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipLocalFileHeader.h"
#include "STAFZipCentralDir.h"


STAFZipCentralDir::STAFZipCentralDir()
{
    cder = new STAFZipCentralDirEndRecord();

}

// write Central Dir data into zip archive
STAFRC_t STAFZipCentralDir::flush(FILE *zf, STAFString *result)
{
    STAFRC_t rc = kSTAFOk;

    std::vector<STAFZipFileHeader*>::iterator i;

    STAFZipUtil util = STAFZipUtil();

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::flush_CP1"));

    for (i = fileHeaderList.begin(); i != fileHeaderList.end(); i++)
    {

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP2")
                         + " signature ["
                         + (*i)->signature
                         + "]");

        // local header signature
        rc = util.putValue(zf, (uLong)(*i)->signature, 4);

        // version made by
        if (rc == kSTAFOk)
        {

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP3")
                             + " versionMadeBy ["
                             + (*i)->versionMadeBy
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->versionMadeBy, 2);
        }

        // version needed to extract
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP4")
                             + " versionNeededToExtract ["
                             + (*i)->versionNeededToExtract
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->versionNeededToExtract, 2);
        }

        // general purpose bit flag, 2 for Maximum (-exx/-ex) compression option
        // was used for method 8 and 9 - deflating
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP5")
                             + " generalPurposeBitFlag ["
                             + (*i)->generalPurposeBitFlag
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->generalPurposeBitFlag, 2);
        }

        // compression method
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP6")
                             + " compressionMethod ["
                             + (*i)->compressionMethod
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->compressionMethod, 2);
        }

        // last mod time date
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP7")
                             + " lastModifiedTimeDate ["
                             + (*i)->lastModifiedTimeDate
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->lastModifiedTimeDate, 4);
        }

        // crc
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP8")
                             + " crc ["
                             + (*i)->crc
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->crc, 4);
        }

        // compressed size
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP9")
                             + " compressedSize ["
                             + (*i)->compressedSize
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->compressedSize, 4);
        }

        // uncompressed size
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP10")
                             + " uncompressedSize ["
                             + (*i)->uncompressedSize
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->uncompressedSize, 4);
        }

        // fileName length
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP11")
                             + " fileNameLength ["
                             + (*i)->fileNameLength
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->fileNameLength, 2);
        }
        
        // fileName length
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP12")
                             + " extraFieldLength ["
                             + (*i)->extraFieldLength
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->extraFieldLength, 2);
        }
        
        // file comment length
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP13")
                             + " fileCommentLength ["
                             + (*i)->fileCommentLength
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->fileCommentLength, 2);
        }
        
        // disk number start
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP14")
                             + " diskNumberStart ["
                             + (*i)->diskNumberStart
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->diskNumberStart, 2);
        }
        
        // internal file attributes
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP15")
                             + " internalFileAttributes ["
                             + (*i)->internalFileAttributes
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->internalFileAttributes, 2);
        }
        
        // external file attributes
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP16")
                             + " externalFileAttributes ["
                             + (*i)->externalFileAttributes
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->externalFileAttributes, 4);
        }
        
        // local header offset
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP17")
                             + " localFileHeaderOffset ["
                             + (*i)->localFileHeaderOffset
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->localFileHeaderOffset, 4);
        }
        
        // file name
        if (rc == kSTAFOk && (*i)->fileNameLength > 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP18")
                             + " fileName ["
                             + (*i)->fileName
                             + "]");

            if (fwrite((*i)->fileName, (uInt)(*i)->fileNameLength, 1, zf) != 1)
            {
                *result = STAFString("STAFZipCentralDir::flush: ")
                          + "Error writing filename to file header ["
                          + (*i)->fileName
                          + "].\n";

                rc = kZIPGeneralZipError;
            }
        }
        
        // extra field
        if (rc == kSTAFOk && (*i)->extraFieldLength > 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP19")
                             + " extraField offset ["
                             + util.tell(zf)
                             + "]");

            if (fwrite((*i)->extraField, (uInt)(*i)->extraFieldLength,
                       1, zf) != 1)
            {
                *result = STAFString("STAFZipCentralDir::flush: ")
                          + "Error writing extraField to file header ["
                          + (char*)((*i)->extraField)
                          + "].\n";

                rc = kZIPGeneralZipError;
            }
        }
        
        // file comment
        if (rc == kSTAFOk && (*i)->fileCommentLength > 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::flush_CP20")
                             + " fileComment ["
                             + (*i)->fileComment
                             + "]");

            if (fwrite((*i)->fileComment, (uInt)(*i)->fileCommentLength, 1,
                       zf) != 1)
            {
                *result = STAFString("STAFZipCentralDir::flush: ")
                          + "Error writing fileComment to file header ["
                          + (*i)->fileComment
                          + "].\n";

                rc = kZIPGeneralZipError;
            }
        }
        
        if (rc != kSTAFOk)
        {
            if ((*result).length() == 0)
            {
                *result = STAFString("STAFZipCentralDir::flush: ")
                          + "Error writing data to file header.\n";
            }

            break;
        }

    } // flush file headers

    // flush end of central dir

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::flush_CP21")
                     + " signature ["
                     + cder->signature
                     + "]");

    // end of central dir signature
    rc = util.putValue(zf, (uLong)cder->signature, 4);
    
    // number of disk
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP22")
                         + " numberDisk ["
                         + cder->numberDisk
                         + "]");

        rc = util.putValue(zf, (uLong)cder->numberDisk, 2);
    }
    
    // number of disk with central dir
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP23")
                         + " numberDiskWithCentralDir ["
                         + cder->numberDiskWithCentralDir
                         + "]");

        rc = util.putValue(zf, (uLong)cder->numberDiskWithCentralDir, 2);
    }
    
    // number of entries on this disk
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP24")
                         + " numberEntry ["
                         + STAFString(cder->numberEntry)
                         + "]");

        rc = util.putValue(zf, (uLong)cder->numberEntry, 2);
    }
    
    // number of entries
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP25")
                         + " numberEntryWithCentralDir ["
                         + STAFString(cder->numberEntryWithCentralDir)
                         + "]");

        rc = util.putValue(zf, (uLong)cder->numberEntryWithCentralDir, 2);
    }
    
    // central dir size
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP26")
                         + " centralDirSize ["
                         + cder->centralDirSize
                         + "]");

        rc = util.putValue(zf, (uLong)cder->centralDirSize, 4);
    }
    
    // central dir offset
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP27")
                         + " centralDirOffset ["
                         + cder->centralDirOffset
                         + "]");

        rc = util.putValue(zf, (uLong)cder->centralDirOffset, 4);
    }
    
    // comment length
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP28")
                         + " commentLength ["
                         + cder->commentLength
                         + "]");

        rc = util.putValue(zf, (uLong)cder->commentLength, 2);
    }
    
    // comment
    if (rc == kSTAFOk && cder->commentLength > 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::flush_CP29")
                         + " comment ["
                         + cder->comment
                         + "]");

        if (fwrite(cder->comment, (uInt)cder->commentLength, 1, zf) != 1)
        {
            *result = STAFString("STAFZipCentralDir::flush: ")
                          + "Error writing comment to end record ["
                          + cder->comment
                          + "].\n";

            rc = kZIPGeneralZipError;
        }
    }

    if (rc != kSTAFOk)
    {
        if ((*result).length() == 0)
        {
            *result = STAFString("STAFZipCentralDir::flush: ")
                      + "Error writing data.\n";
        }
    }

    return rc;
}

// Read in data from Zip archive
STAFRC_t STAFZipCentralDir::readInData(
    FILE *zf, STAFInt64_t startPos, STAFInt64_t endPos, STAFString *result)
{
    STAFZipUtil util = STAFZipUtil();

    unsigned char* buf;
    STAFInt64_t uSizeFile;
    STAFInt64_t uBackRead;
    uLong uMaxBack = 0xffff; /* maximum size of global comment */
    STAFInt64_t uPosFound = 0;

    // go to the end of stream

    if (util.seek(zf, endPos, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error in seek to the end of zip stream: "
                  + endPos;

        return kZIPGeneralZipError;
    }

    // get stream size

    uSizeFile = endPos - startPos;
    
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::readInData_CP2")
                     + " startPos [" + startPos + "]"
                     + " endPos [" + endPos + "]"
                     + " uSizeFile [" + uSizeFile + "]"
                     );
    
    if (uMaxBack > uSizeFile)
    {
        uMaxBack = uSizeFile;
    }

    buf = (unsigned char*)malloc(BUFREADCOMMENT + 4);

    if (buf == NULL)
    {
        *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error allocating buffer read comment memory ["
                  + (BUFREADCOMMENT + 4)
                  + "].\n";
        return kZIPNotEnoughMemory;
    }

    uBackRead = 4;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::readInData_CP4")
                     + " uBackRead ["
                     + uBackRead
                     + "] uMaxBack ["
                     + uMaxBack
                     + "]");

    // search offset of central dir end record

    while (uBackRead < uMaxBack)
    {
        STAFInt64_t uReadSize, uReadPos;
        STAFInt64_t i;

        if (uBackRead + BUFREADCOMMENT > uMaxBack)
        {
            uBackRead = uMaxBack;
        }
        else
        {
            uBackRead += BUFREADCOMMENT;
        }

        uReadPos = uSizeFile - uBackRead;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP5")
                         + " uReadSize ["
                         + uReadSize
                         + "] uReadPos ["
                         + uReadPos
                         + "]");

        // move to uReadPos of the file

        if (util.seek(zf, uReadPos + startPos, SEEK_SET) != 0)
        {
            *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error seek zip file uReadPos + startPos ["
                  + uReadPos + " + " + startPos + "].\n";

            break;
        }

        // read in a block of data from the file

        if (fread(buf, uReadSize, 1, zf) != 1)
        {
            *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error read zip file size ["
                  + uReadSize
                  + "].\n";

            break;
        }

        // search for the signature of central dir end record

        for (i = uReadSize - 3; (i--) > 0;)
        {
            if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) &&
                ((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3))== 0x06))
            {
                uPosFound = uReadPos + i;
                break;
            }
        }

        if (uPosFound != 0)
        {
            break;
        }
    }

    free(buf);

    if (uPosFound <= 0)
    {
        if((*result).length() == 0)
        {
            *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Can't find central dir end record.\n";
        }

        return kZIPGeneralZipError;
    }

    // get attributes of central dir end record

    STAFRC_t rc;

    if (util.seek(zf, uPosFound + startPos, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error in seek central dir end record ["
                  + uPosFound + " + + " + startPos
                  + "].\n";

        return kZIPGeneralZipError;
    }

    // get signature
    rc = util.getLong(zf, &cder->signature);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::readInData_CP10")
                     + " signature ["
                     + cder->signature
                     + "]");

    // get number of this disk
    if (rc == kSTAFOk)
    {
        uLong iL;

        rc = util.getShort(zf, &iL);

        cder->numberDisk = (unsigned short)iL;

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP11")
                         + " numberDisk ["
                         + cder->numberDisk
                         + "]");
    }

    // get number of the disk with the start of central directory
    if (rc == kSTAFOk)
    {
        uLong iL;

        rc = util.getShort(zf, &iL);

        cder->numberDiskWithCentralDir = (unsigned short)iL;

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP12")
                         + " numberDiskWithCentralDir ["
                         + cder->numberDiskWithCentralDir
                         + "]");
    }

    // get total number of entries in the central directory on this disk
    if (rc == kSTAFOk)
    {
        uLong iL;

        rc = util.getShort(zf, &iL);

        cder->numberEntry = (unsigned short)iL;

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP13")
                         + " numberEntry ["
                         + STAFString(cder->numberEntry)
                         + "]");
    }

    // get total number of entries in the central directory
    if (rc == kSTAFOk)
    {
        uLong iL;

        rc = util.getShort(zf, &iL);

        cder->numberEntryWithCentralDir = (unsigned short)iL;

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP14")
                         + " numberEntryWithCentralDir ["
                         + STAFString(cder->numberEntryWithCentralDir)
                         + "]");
    }

    // check
    if (rc == kSTAFOk && (cder->numberEntryWithCentralDir != cder->numberEntry
        || cder->numberDiskWithCentralDir != 0
        || cder->numberDisk != 0
        || cder->numberEntry <= 0))
    {
        *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Check integrity error: number disk"
                  + STAFString(cder->numberDisk)
                  + "] number disk with central dir ["
                  + STAFString(cder->numberDiskWithCentralDir)
                  + "] number entry with central dir ["
                  + STAFString(cder->numberEntryWithCentralDir)
                  + "] number entry ["
                  + STAFString(cder->numberEntry)
                  + "].\n";

        rc = kZIPGeneralZipError;
    }

    // get size of the central directory
    if (rc == kSTAFOk)
    {
        rc = util.getLong(zf, &cder->centralDirSize);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP15")
                         + " centralDirSize ["
                         + cder->centralDirSize
                         + "]");
    }

    // get offset of start of central directory with respect to the starting
    // disk number
    if (rc == kSTAFOk)
    {
        rc = util.getLong(zf, &cder->centralDirOffset);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP16")
                         + " centralDirOffset ["
                         + cder->centralDirOffset
                         + "]");
    }

    // get zipfile comment length
    if (rc == kSTAFOk)
    {
        uLong iL;

        rc = util.getShort(zf, &iL);

        cder->commentLength = (unsigned short)iL;

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP17")
                         + " commentLength ["
                         + cder->commentLength
                         + "]");
    }

    // get zipfile comment
    if (rc == kSTAFOk && cder->commentLength > 0)
    {
        cder->comment = (char*)calloc(cder->commentLength + 1, 1);

        if (cder->comment == NULL)
        {
            *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error allocating memory for comment length ["
                  + (cder->commentLength + 1)
                  + "].\n";

            return kZIPNotEnoughMemory;
        }

        if (fread(cder->comment, (uInt)cder->commentLength, 1, zf) !=1 )
        {
            *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error reading comment.\n";

            rc = kSTAFFileReadError;
        }

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP20")
                         + " comment ["
                         + cder->comment
                         + "]");
    }

    // check data integrity
    if ((uPosFound != cder->centralDirOffset + startPos + cder->centralDirSize) &&
        (rc == kSTAFOk))
    {
        *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error in checking data integrity: uPosFound ["
                  + uPosFound
                  + "] central dir offset ["
                  + cder->centralDirOffset
                  + "] central dir size ["
                  + cder->centralDirSize
                  + "].\n";

        rc = kZIPGeneralZipError;
    }

    if (rc != kSTAFOk)
    {
        if ((*result).length() == 0)
        {
            *result = STAFString("STAFZipCentralDir::readInData: ")
                      + "Error reading central dir end record.\n";
        }

        return rc;
    }

    // move to central dir
    if (util.seek(zf, cder->centralDirOffset + startPos, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipCentralDir::readInData: ")
                  + "Error seek central dir offset ["
                  + cder->centralDirOffset + " + " + startPos
                  + "].\n";

        return kZIPGeneralZipError;
    }

    // get file headers
    for(int i = 0; i < cder->numberEntry; i++)
    {
        STAFZipFileHeader *fh = new STAFZipFileHeader();

        // get signature
        rc = util.getLong(zf, &fh->signature);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::readInData_CP24")
                         + " signature ["
                         + fh->signature
                         + "]");

        // get version made by
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->versionMadeBy = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP25")
                             + " versionMadeBy ["
                             + fh->versionMadeBy
                             + "]");
        }

        // get version needed to extract
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->versionNeededToExtract = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP26")
                             + " versionNeededToExtract ["
                             + fh->versionNeededToExtract
                             + "]");
        }
        
        // get general purpose bit flag
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->generalPurposeBitFlag = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP27")
                             + " generalPurposeBitFlag ["
                             + fh->generalPurposeBitFlag
                             + "]");
        }
        
        // get compression method
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->compressionMethod = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP28")
                             + " compressionMethod ["
                             + fh->compressionMethod
                             + "]");
        }
        
        // get last mod time date
        if (rc == kSTAFOk)
        {
            rc = util.getLong(zf, &fh->lastModifiedTimeDate);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP29")
                             + " lastModifiedTimeDate ["
                             + fh->lastModifiedTimeDate
                             + "]");
        }

        // get CRC32
        if (rc == kSTAFOk)
        {
            rc = util.getLong(zf, &fh->crc);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP30")
                             + " crc ["
                             + fh->crc
                             + "]");
        }
        
        // get compressed size
        if (rc == kSTAFOk)
        {
            rc = util.getLong(zf, &fh->compressedSize);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP31")
                             + " compressedSize ["
                             + fh->compressedSize
                             + "]");
        }
        
        // get uncompressed size
        if (rc == kSTAFOk)
        {
            rc = util.getLong(zf, &fh->uncompressedSize);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP32")
                             + " uncompressedSize ["
                             + fh->uncompressedSize
                             + "]");
        }
        
        // get file name length
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->fileNameLength = (unsigned short)iL;
            fh->size += fh->fileNameLength;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP33")
                             + " fileNameLength ["
                             + fh->fileNameLength
                             + "] "
                             + "size ["
                             + fh->size
                             + "]");
        }
        
        // get extra field length
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->extraFieldLength = (unsigned short)iL;
            fh->size += fh->extraFieldLength;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP34")
                             + " extraFieldLength ["
                             + fh->extraFieldLength
                             + "] "
                             + "size ["
                             + fh->size
                             + "]");
        }

        // get file comment length
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->fileCommentLength = (unsigned short)iL;
            fh->size += fh->fileCommentLength;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP35")
                             + " fileCommentLength ["
                             + fh->fileCommentLength
                             + "] "
                             + "size ["
                             + fh->size
                             + "]");
        }

        // get disk number start
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->diskNumberStart = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP36")
                             + " diskNumberStart ["
                             + fh->diskNumberStart
                             + "]");
        }

        // get internal file attributes
        if (rc == kSTAFOk)
        {
            uLong iL;
            rc = util.getShort(zf, &iL);
            fh->internalFileAttributes = (unsigned short)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP37")
                             + " internalFileAttributes ["
                             + fh->internalFileAttributes
                             + "]");
        }

        // get external file attributes
        if (rc == kSTAFOk)
        {
            rc = util.getLong(zf, &fh->externalFileAttributes);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP38")
                             + " externalFileAttributes ["
                             + fh->externalFileAttributes
                             + "]");
        }

        // get local header offset
        if (rc == kSTAFOk)
        {
            rc = util.getLong(zf, &fh->localFileHeaderOffset);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP39")
                             + " localFileHeaderOffset ["
                             + fh->localFileHeaderOffset
                             + "]");
        }
        
        // get file name
        if (rc == kSTAFOk && fh->fileNameLength > 0)
        {
            buf = (unsigned char*)calloc(fh->fileNameLength + 1, 1);

            if (buf == NULL)
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error allocating memory for file name length ["
                          + (fh->fileNameLength + 1)
                          + "].\n";

                return kZIPNotEnoughMemory;
            }

            if (fread(buf, (uInt)fh->fileNameLength, 1, zf) !=1 )
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error reading filename.\n";

                rc = kZIPGeneralZipError;
            }

            fh->fileName = (char*)buf;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP42")
                             + " fileName ["
                             + fh->fileName
                             + "]");
        }
        
        // get extra field
        if (rc == kSTAFOk && fh->extraFieldLength > 0)
        {
            buf = (unsigned char*)calloc(fh->extraFieldLength, 1);

            if (buf == NULL)
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error allocating memory for extra field length ["
                          + fh->extraFieldLength
                          + "].\n";

                return kZIPNotEnoughMemory;
            }

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP45")
                             + " extraField offset ["
                             + util.tell(zf)
                             + "]");

            if (fread(buf, (uInt)fh->extraFieldLength, 1, zf) !=1 )
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error reading extra field.\n";

                rc = kZIPGeneralZipError;
            }

            fh->extraField = (void*)buf;
        }
        
        // get file comment
        if (rc == kSTAFOk && fh->fileCommentLength > 0)
        {
            buf = (unsigned char*)calloc(fh->fileCommentLength + 1, 1);

            if (buf == NULL)
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error allocating memory for file comment length ["
                          + (fh->fileCommentLength + 1)
                          + "].\n";

                return kZIPNotEnoughMemory;
            }

            if (fread(buf, (uInt)fh->fileCommentLength, 1, zf) !=1 )
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error reading file comment.\n";

                rc = kZIPGeneralZipError;
            }

            fh->fileComment = (char*)buf;

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::readInData_CP48")
                             + " fileComment ["
                             + fh->fileComment
                             + "]");
        }

        if (rc != kSTAFOk)
        {
            if ((*result).length() != 0)
            {
                *result = STAFString("STAFZipCentralDir::readInData: ")
                          + "Error reading central dir data.\n";
            }

            break;
        }

        fileHeaderList.push_back(fh);

        fileHeaderListSorted[STAFString(fh->fileName)] = fh;        
    }

    return rc;
}


// Add file header
void STAFZipCentralDir::addFileHeader(STAFZipLocalFileHeader *lfh)
{
    STAFZipFileHeader *fh = new STAFZipFileHeader(lfh);

    fileHeaderList.push_back(fh);
    fileHeaderListSorted[STAFString(fh->fileName)] = fh;        

    cder->numberEntry++;
    cder->numberEntryWithCentralDir = cder->numberEntry;
    cder->centralDirSize += fh->size;
    cder->centralDirOffset += lfh->size;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::addFileHeader_CP1")
                     + " cder->numberEntry ["
                     + STAFString(cder->numberEntry)
                     + "] cder->numberEntryWithCentralDir ["
                     + STAFString(cder->numberEntryWithCentralDir)
                     + "] cder->centralDirSize ["
                     + cder->centralDirSize
                     + "] cder->centralDirOffset ["
                     + cder->centralDirOffset
                     + "]");
}


// get central dir offset
uLong STAFZipCentralDir::getOffset()
{
    return cder->centralDirOffset;
}


// get number of entries in central dir
unsigned short STAFZipCentralDir::getNumberEntry()
{
    return cder->numberEntry;
}


// get central dir size
uLong STAFZipCentralDir::getSize()
{
    return cder->centralDirSize;
}


// get central dir end record
STAFZipCentralDirEndRecord* STAFZipCentralDir::getCentralDirEndRecord()
{
    return cder;
}


static const STAFString sZipInfoClassName = "STAF/Service/Zip/ZipInfo";


// list the content of Zip archive
STAFRC_t STAFZipCentralDir::list(STAFString *buf, STAFString *result)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::list_CP1"));

    // Construct map class for the marshalled  zip information

    STAFMapClassDefinitionPtr zipInfoClass;
 
    zipInfoClass = STAFMapClassDefinition::create("STAF/Service/Zip/ZipInfo");
 
    zipInfoClass->addKey("length", "Length");
    zipInfoClass->addKey("method", "Method");
    zipInfoClass->addKey("size", "Size");
    zipInfoClass->addKey("ratio", "Ratio");
    zipInfoClass->addKey("date", "Date");
    zipInfoClass->addKey("time", "Time");
    zipInfoClass->addKey("crc-32", "CRC-32");
    zipInfoClass->addKey("name", "Name");

    // Create a marshalled list of maps containing zip file information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(zipInfoClass->reference());
    STAFObjectPtr outputList = STAFObject::createList();
    
    char fieldBuf[MAXMESSAGE] = "";
    std::vector<STAFZipFileHeader*>::iterator i;
    STAFZipUtil util = STAFZipUtil();

    for (i = fileHeaderList.begin(); i != fileHeaderList.end(); i++)
    {
        uLong ratio = 0;
        const char *string_method;
        struct tm filedate;

        util.fileTime((*i)->lastModifiedTimeDate, &filedate);

        if ((*i)->uncompressedSize > 0)
        {
            ratio = ((*i)->compressedSize * 100) / (*i)->uncompressedSize;
        }

        if ((*i)->compressionMethod == 0)
        {
            string_method="Stored";
        }
        else
        {
            if ((*i)->compressionMethod == Z_DEFLATED)
            {
                uInt iLevel = (uInt)(((*i)->generalPurposeBitFlag & 0x6)
                                      / 2);
                if (iLevel == 0)
                {
                    string_method = "Defl:N";
                }
                else if (iLevel == 1)
                {
                    string_method = "Defl:X";
                }
                else if ((iLevel == 2) || (iLevel == 3))
                {
                    string_method = "Defl:F"; /* 2:fast , 3 : extra fast*/
                }
            }
            else
            {
                string_method = "Unkn. ";
            }
        }
        
        STAFObjectPtr zipInfoMap = zipInfoClass->createInstance();

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%7lu", (*i)->uncompressedSize);
        zipInfoMap->put("length", STAFString(fieldBuf));

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%6s", string_method);
        zipInfoMap->put("method", STAFString(fieldBuf));

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%7lu", (*i)->compressedSize);
        zipInfoMap->put("size", STAFString(fieldBuf));

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%3lu%%", ratio);
        zipInfoMap->put("ratio", STAFString(fieldBuf));

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%2.2lu-%2.2lu-%2.2lu", 
                (uLong)filedate.tm_mon + 1,
                (uLong)filedate.tm_mday,
                (uLong)filedate.tm_year % 100);
        zipInfoMap->put("date", STAFString(fieldBuf));

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%2.2lu:%2.2lu", 
                (uLong)filedate.tm_hour,
                (uLong)filedate.tm_min);
        zipInfoMap->put("time", STAFString(fieldBuf));

        memset(fieldBuf, 0, MAXMESSAGE);
        sprintf(fieldBuf, "%8.8lx", (uLong)(*i)->crc);
        zipInfoMap->put("crc-32", STAFString(fieldBuf));

        zipInfoMap->put("name", STAFString((*i)->fileName));

        outputList->append(zipInfoMap);
    }
    
    mc->setRootObject(outputList);
    *buf = mc->marshall();

    return kSTAFOk;
}


// remove a file entry from central dir
STAFZipFileHeader* STAFZipCentralDir::remove(const char *fileName,
                                             STAFZipLocalFileHeader *lfh)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::remove_CP1"));

    STAFZipFileHeader *fh = NULL;

    STAFZipUtil util = STAFZipUtil();

    std::vector<STAFZipFileHeader*>::iterator i;
    
    // search for file entry in the central dir
    for (i = fileHeaderList.begin(); i != fileHeaderList.end(); i++)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::remove_CP2")
                         + " fileName ["
                         + fileName
                         + "] (*i)->fileName ["
                         + (*i)->fileName
                         + "]");

        if (util.myStrCmp((*i)->fileName, fileName) == 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::remove_CP3")
                             + " fileName ["
                             + fileName
                             + "] (*i)->fileName ["
                             + (*i)->fileName
                             + "]");

            // store file header
            fh = (*i);

            // remove the file header from central dir
            fileHeaderList.erase(i);
            fileHeaderListSorted.erase(STAFString(fh->fileName));

            break;
        }
    }

    // if file header is found
    if (fh != NULL)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::remove_CP4"));

        // change local file header offset to the new offset in central dir
        for (i = fileHeaderList.begin(); i != fileHeaderList.end(); i++)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDir::remove_CP5")
                             + " (*i)->localFileHeaderOffset ["
                             + (*i)->localFileHeaderOffset
                             + "] fh->localFileHeaderOffset ["
                             + fh->localFileHeaderOffset
                             + "]");
            
            // if local file header is after the removed file header
            if ((*i)->localFileHeaderOffset > fh->localFileHeaderOffset)
            {
                // change local file header offset to the new offset
                (*i)->localFileHeaderOffset -= lfh->size;

                STAFTrace::trace(kSTAFTraceServiceResult,
                                 STAFString("STAFZipCentralDir::remove_CP6")
                                 + " (*i)->localFileHeaderOffset ["
                                 + (*i)->localFileHeaderOffset
                                 + "] fh->localFileHeaderOffset ["
                                 + fh->localFileHeaderOffset
                                 + "] lfh->size ["
                                 + lfh->size
                                 + "]");
            }
        }

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::remove_CP7")
                         + " cder->centralDirOffset ["
                         + cder->centralDirOffset
                         + "] cder->centralDirSize ["
                         + cder->centralDirSize
                         + "] cder->numberEntry ["
                         + STAFString(cder->numberEntry)
                         + "] cder->numberEntryWithCentralDir ["
                         + STAFString(cder->numberEntryWithCentralDir)
                         + "] lfh->size ["
                         + lfh->size
                         + "]");

        // change central dir offset
        cder->centralDirOffset -= lfh->size;

        // change central dir size
        cder->centralDirSize -= fh->size;

        // reduce number of entries in central dir by 1
        cder->numberEntry--;

        // reduce number of entries with central dir by 1
        cder->numberEntryWithCentralDir--;

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::remove_CP8")
                         + " cder->centralDirOffset ["
                         + cder->centralDirOffset
                         + "] cder->centralDirSize ["
                         + cder->centralDirSize
                         + "] cder->numberEntry ["
                         + STAFString(cder->numberEntry)
                         + "] cder->numberEntryWithCentralDir ["
                         + STAFString(cder->numberEntryWithCentralDir)
                         + "] lfh->size ["
                         + lfh->size
                         + "]");
    }

    return fh;
}

// find a file name in central dir
STAFZipFileHeader* STAFZipCentralDir::find(const char *fileName)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDir::find_CP1")
                         + " fileName ["
                         + fileName
                         + "]");

    std::map<STAFString, STAFZipFileHeader*>::iterator i;

    i = fileHeaderListSorted.find(
        STAFString(fileName).replace(kUTF8_BSLASH, kUTF8_SLASH));

    if (i != fileHeaderListSorted.end())
        return i->second;
    else
        return NULL;
}


// find the last file header
STAFZipFileHeader* STAFZipCentralDir::findLastFileHeader()
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::findLastFileHeader_CP1"));

    STAFZipFileHeader *fh = *fileHeaderList.begin();


    std::vector<STAFZipFileHeader*>::iterator i;

    for (i = fileHeaderList.begin(); i != fileHeaderList.end(); i++)
    {

        if ((*i)->localFileHeaderOffset > fh->localFileHeaderOffset)
        {
            fh = *i;
        }
    }

    return fh;
}


// destructor
STAFZipCentralDir::~STAFZipCentralDir()
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDir::destructor_CP1"));

    std::vector<STAFZipFileHeader*>::iterator i;

    for (i = fileHeaderList.begin(); i != fileHeaderList.end(); i++)
    {
        delete *i;
    }

    delete cder;
}





