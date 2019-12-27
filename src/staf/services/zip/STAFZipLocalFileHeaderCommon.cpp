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
#include "zutil.h"

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"
#include "STAFZipLocalFileHeader.h"
#include "STAFZipFileHeader.h"


// constructor
STAFZipLocalFileHeader::STAFZipLocalFileHeader()
{
    signature = 0x04034b50;

    // 2 bytes, zip 2.0
    versionNeededToExtract = 20;

    // 2 bytes, general purpose bit flag bit 2, 1 set to be 1 0 for
    // maximum compression
    generalPurposeBitFlag = 2;

    // 2 bytes, method 8 - deflating
    compressionMethod = Z_DEFLATED;

    // 2 bytes time, 2 bytes date
    lastModifiedTimeDate = 0;

    // 4 bytes
    crc = 0;

    // 4 bytes, not initialized yet
    compressedSize = 0;

    // 4 bytes, not initialized yet
    uncompressedSize = 0;

    // 2 bytes
    fileNameLength = 0;

    // 2 bytes
    extraFieldLength = 0;

    fileName = NULL;

    extraField = NULL;

    fullFileName = NULL;

    size = 30;

    offset = 0;

}


// constructor based on pathname and prefix length
STAFZipLocalFileHeader::STAFZipLocalFileHeader(const char *pathname,
                                               int prefixlen)
{
    STAFZipUtil util = STAFZipUtil();

    signature = 0x04034b50;


    fileName = NULL;

    fullFileName = NULL;

    // 2 bytes
    fileNameLength = 0;

    // 2 bytes time, 2 bytes date
    lastModifiedTimeDate = 0;


    fullFileName = (char*)calloc(strlen(pathname) + 1, 1);

    if (fullFileName != NULL)
    {
        strcpy(fullFileName, pathname);

        fileName = util.calculateFileNameInZip(fullFileName, prefixlen);

        fileNameLength = strlen(fileName);

        lastModifiedTimeDate = util.fileTime(fullFileName);
    }
    else
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
             STAFString("STAFZipLocalFileHeader::STAFZipLocalFileHeader1_CP1")
                     + "Error allocating memory for full file name.\n");
    }

    // 2 bytes, zip 2.0
    versionNeededToExtract = 20;

    // 2 bytes, general purpose bit flag bit 2, 1 set to be 1 0 for
    // maximum compression
    generalPurposeBitFlag = 2;

    // 2 bytes, method 8 - deflating
    compressionMethod = Z_DEFLATED;

    // 4 bytes
    crc = 0;

    // 4 bytes, not initialized yet
    compressedSize = 0;

    // 4 bytes, not initialized yet
    uncompressedSize = 0;

    // 2 bytes
    extraFieldLength = 0;

    extraField = NULL;


    size = 30 + fileNameLength;

    offset = 0;

}


// flush local file header to zip archive and deflating data by
// using of zlib utility
STAFRC_t STAFZipLocalFileHeader::flush(FILE *zf, STAFString *result)
{
    STAFZipUtil util = STAFZipUtil();

    STAFRC_t rc;
    int err;

    // save the current file offset
    offset = util.tell(zf);

    // write the local header

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP1")
                     + " offset ["
                     + offset
                     + "] signature ["
                     + signature
                     + "]");

    // local header magic
    rc = util.putValue(zf, (uLong)signature, 4);

    // version needed to extract
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP2")
                     + " versionNeededToExtract ["
                     + versionNeededToExtract
                     + "]");

        rc = util.putValue(zf, (uLong)versionNeededToExtract, 2);
    }

    // general purpose bit flag, 2 for Maximum (-exx/-ex) compression option was
    // used for method 8 and 9 - deflating
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP3")
                     + " generalPurposeBitFlag ["
                     + generalPurposeBitFlag
                     + "]");

        rc = util.putValue(zf, (uLong)generalPurposeBitFlag, 2);
    }

    // compression method
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP4")
                     + " compressionMethod ["
                     + compressionMethod
                     + "]");

        rc = util.putValue(zf, (uLong)compressionMethod, 2);
    }

    // file time in DOS format
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP5")
                     + " lastModifiedTimeDate ["
                     + lastModifiedTimeDate
                     + "]");

        rc = util.putValue(zf, (uLong)lastModifiedTimeDate, 4);
    }

    // CRC 32, unknown
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP6")
                     + " crc ["
                     + crc
                     + "]");

        rc = util.putValue(zf, (uLong)crc, 4);
    }

    // compressed size, unknown
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP7")
                     + " compressedSize ["
                     + compressedSize
                     + "]");

        rc = util.putValue(zf, (uLong)compressedSize, 4);
    }

    // uncompressed size, unknown
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP7")
                     + " uncompressedSize ["
                     + uncompressedSize
                     + "]");

        rc = util.putValue(zf, (uLong)uncompressedSize, 4);
    }

    // file name size
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP8")
                     + " fileNameLength ["
                     + fileNameLength
                     + "]");

        rc = util.putValue(zf, (uLong)fileNameLength, 2);
    }

    // extra field size
    if (rc == kSTAFOk)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP9")
                     + " extraFieldLength ["
                     + extraFieldLength
                     + "]");

        rc = util.putValue(zf, (uLong)extraFieldLength, 2);
    }

    // file name
    if (rc == kSTAFOk && fileNameLength > 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP10")
                     + " fileName ["
                     + fileName
                     + "]");

        if (fwrite(fileName, (uInt)fileNameLength, 1, zf)!=1)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error writting file name ["
                          + fileName
                          + "].\n";

            rc = kZIPGeneralZipError;
        }
    }

    // extra field
    if (rc == kSTAFOk && extraFieldLength > 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP12")
                     + " extraField offset ["
                     + util.tell(zf)
                     + "]");

        if (fwrite(extraField, (uInt)extraFieldLength, 1, zf)!=1)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error writting extra field ["
                          + fileName
                          + "].\n";

            rc = kZIPGeneralZipError;
        }
    }


    if (rc != kSTAFOk)
    {

        if ((*result).length() == 0)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error writing data.\n";
        }

        return rc;
    }


    // start deflating the file
    if (*(fileName + fileNameLength - 1) != '/'
        && *(fileName + fileNameLength - 1) != '\\')
    {

        void *outbuf = NULL;
        void *inbuf = NULL;


        // allocate output buffer
        outbuf = (void*)malloc(Z_BUFSIZE);
        if(outbuf == NULL)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error allocating memory for zip output buffer ["
                          + Z_BUFSIZE
                          + "].\n";

            return kZIPNotEnoughMemory;
        }


        // initialize zLib stream structure for deflate
        z_stream stream;


        stream.total_in = 0;

        stream.total_out = 0;

        stream.zalloc = (alloc_func)0;

        stream.zfree = (free_func)0;

        stream.opaque = (voidpf)0;

        uInt level = 9;

        err = deflateInit2(&stream, level,
               Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, 0);


        if (err != Z_OK)
        {
            free(outbuf);

            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error in zlib deflate init ["
                          + err
                          + "].\n";

            return kZIPGeneralZipError;
        }


        // allocate input buffer
        inbuf = (void*)malloc(Z_BUFSIZE);
        if(inbuf == NULL)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error allocating memory for zip input buffer ["
                          + Z_BUFSIZE
                          + "].\n";

            return kZIPNotEnoughMemory;
        }



        FILE *fin;

        // open the to be zipped file
        fin = fopen(fullFileName, "rb");
        if (fin == NULL)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error in open file ["
                          + fullFileName
                          + "].\n";

            free(inbuf);
            free(outbuf);

            return kSTAFFileOpenError;
        }


        uInt compressed_bytes;

        stream.avail_in = (uInt)0;


        // loop to read data from original file
        while (err == Z_OK)
        {

            // prepare zip buffer

            stream.next_out = (Bytef*)outbuf;

            stream.avail_out = (uInt)Z_BUFSIZE;


            // set compressed bytes to 0
            compressed_bytes = 0;


            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP19")
                     + " stream.avail_out ["
                     + stream.avail_out
                     + "] stream.avail_in ["
                     + stream.avail_in
                     + "]");

            // loop to zip data
            while (stream.avail_out > 0 && err == Z_OK)
            {

                if (stream.avail_in == 0)
                {

                    // read Z_BUFSIZE of bytes from file
                    stream.avail_in = fread(inbuf, 1, Z_BUFSIZE, fin);

                    STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipLocalFileHeader::flush_CP20")
                             + " stream.avail_in ["
                             + stream.avail_in
                             + "]");

                    // if size read < Z_BUFSIZE and not end of file, return err
                    if (stream.avail_in < Z_BUFSIZE)
                    {
                        if (feof(fin) == 0)
                        {
                            *result =
                                STAFString("STAFZipLocalFileHeader::flush: ")
                                + "Error in reading file ["
                                + fullFileName
                                + "].\n";

                            rc = kSTAFFileReadError;

                            break;
                        }
                    }


                    // if size read > 0
                    if (stream.avail_in > 0)
                    {
                        // set next in
                        stream.next_in = (Bytef*)inbuf;

                        // calculate crc
                        crc = crc32(crc, stream.next_in, stream.avail_in);
                    }

                } // read in raw data from file


                // save the total out before delfating
                uLong uTotalOutBefore = stream.total_out;

                STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipLocalFileHeader::flush_CP22")
                             + " stream.total_out ["
                             + stream.total_out
                             + "] stream.avail_in ["
                             + stream.avail_in
                             + "]");


                if (stream.avail_in == 0)
                {
                    // do delate with Z_FINISH
                    err = deflate(&stream,  Z_FINISH);
                }
                else
                {
                    // do delate with Z_NO_FLUSH
                    err = deflate(&stream,  Z_NO_FLUSH);
                }


                // calculate compressed size
                compressed_bytes += (uInt)(stream.total_out - uTotalOutBefore);

                STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipLocalFileHeader::flush_CP23")
                             + " stream.total_out ["
                             + stream.total_out
                             + "] uTotalOutBefore ["
                             + uTotalOutBefore
                             + "] compressed_bytes ["
                             + compressed_bytes
                             + "]");

            } // loop to zip data


            // if error, break
            if (rc != kSTAFOk || (err != Z_STREAM_END && err != Z_OK))
            {
                if ((*result).length() == 0)
                {
                    *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error in deflating ["
                          + err
                          + "].\n";
                }

                rc = kZIPGeneralZipError;

                break;
            }


            // write the unzipped buffer to file

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipLocalFileHeader::flush_CP25")
                             + " compressed_bytes ["
                             + compressed_bytes
                             + "]");

            if (compressed_bytes > 0)
            {
                if (fwrite(outbuf, compressed_bytes, 1, zf)!=1)
                {
                    *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error writting compressed bytes ["
                          + compressed_bytes
                          + "].\n";

                    rc = kSTAFFileWriteError;

                    break;
                }

            }


        } // loop to read data from original file


        fclose(fin);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::flush_CP27")
                         + " err ["
                         + err
                         + "] rc ["
                         + rc
                         + "]");

        // end the deflate
        if (err == Z_STREAM_END && rc == kSTAFOk)
        {
            err = deflateEnd(&stream);
        }


        free(inbuf);

        free(outbuf);


        if (err != Z_OK)
        {
            if ((*result).length() == 0)
            {
                *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error finishing deflating ["
                          + err
                          + "].\n";
            }

            rc = kZIPGeneralZipError;
        }


        if (rc != kSTAFOk)
        {
            return rc;
        }


        compressedSize = stream.total_out;

        uncompressedSize = stream.total_in;



        // save current end of local file header position
        STAFInt64_t curpos = util.tell(zf);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::flush_CP30")
                         + " curpos ["
                         + curpos
                         + "]");

        // update crc, compressed size, uncompressed size
        if (util.seek(zf, offset + 14, SEEK_SET) != 0)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error seek crc, compressed uncompressed size ["
                          + (offset + 14)
                          + "].\n";

            rc = kZIPGeneralZipError;
        }


        // save crc32
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::flush_CP32")
                         + " crc ["
                         + crc
                         + "]");

            rc = util.putValue(zf, (uLong)crc, 4);
        }

        // save compressed size
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::flush_CP33")
                         + " compressedSize ["
                         + compressedSize
                         + "]");

            rc = util.putValue(zf, (uLong)compressedSize, 4);
        }


        // save uncompressed size
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::flush_CP34")
                         + " uncompressedSize ["
                         + uncompressedSize
                         + "]");

            rc = util.putValue(zf, (uLong)uncompressedSize, 4);
        }


        // restore the previous end of local file header position
        if (util.seek(zf, curpos, SEEK_SET) != 0 && rc == kSTAFOk)
        {
            *result = STAFString("STAFZipLocalFileHeader::flush: ")
                          + "Error restore end of local header pos ["
                          + curpos
                          + "].\n";

            rc = kZIPGeneralZipError;
        }


    } // zip the file

    size = util.tell(zf) - offset;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::flush_CP36")
                     + " size ["
                     + size
                     + "]");

    return rc;
}


// extract one file from zip archive by using zlib utility
STAFRC_t STAFZipLocalFileHeader::doExtract(FILE *zf, uLong startPos, 
                                           FILE *outfile,
                                           STAFString *result)
{
    STAFZipUtil util = STAFZipUtil();
    z_stream stream;
    stream.total_out = 0;

    int err = Z_OK;
    STAFRC_t rc = kSTAFOk;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::doExtract_CP1")
                     + " rc ["
                     + rc
                     + "]");

    if (compressionMethod != 0)
    {
        stream.zalloc = (alloc_func)0;
        stream.zfree = (free_func)0;
        stream.opaque = (voidpf)0;

        err = inflateInit2(&stream, -MAX_WBITS);
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */

        if (err != Z_OK)
        {
            *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error init zlib inflate ["
                          + err
                          + "].\n";

            return kZIPGeneralZipError;
        }
    }

    // allocate input buffer

    void *inbuf = (void*)malloc(Z_BUFSIZE);

    if(inbuf == NULL)
    {
        *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error allocating memory for input buffer ["
                          + Z_BUFSIZE
                          + "].\n";
        return kZIPNotEnoughMemory;
    }

    // allocate output buffer

    void *outbuf = (void*)malloc(Z_BUFSIZE);

    if(outbuf == NULL)
    {
        *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error allocating memory for output buffer ["
                          + Z_BUFSIZE
                          + "].\n";
        return kZIPNotEnoughMemory;
    }

    uLong pos = offset + size - compressedSize;
    uLong rest_uncompressed, rest_compressed;

    rest_uncompressed = uncompressedSize;
    rest_compressed = compressedSize;

    uInt uncompressed_bytes;
    uLong newcrc32 = 0;

    stream.avail_in = (uInt)0;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::doExtract_CP4.5")
                     + " pos ["
                     + pos
                     + "] stream.avail_in ["
                     + stream.avail_in
                     + "] rest_uncompressed ["
                     + rest_uncompressed
                     + "] rest_compressed ["
                     + rest_compressed
                     + "]");

    // loop to read compressed data from zip file

    while (rest_uncompressed > 0)
    {
        // unzip buffer

        stream.next_out = (Bytef*)outbuf;
        stream.avail_out = (uInt)Z_BUFSIZE;

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::doExtract_CP5")
                     + " stream.avail_out ["
                     + stream.avail_out
                     + "] rest_uncompressed ["
                     + rest_uncompressed
                     + "]");

        if (stream.avail_out > rest_uncompressed)
        {
            stream.avail_out = (uInt)rest_uncompressed;
        }

        uncompressed_bytes = 0;

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::doExtract_CP6")
                     + " stream.avail_out ["
                     + stream.avail_out
                     + "]");

        // loop to unzip compressed data

        while (stream.avail_out > 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::doExtract_CP7")
                     + " stream.avail_in ["
                     + stream.avail_in
                     + "] rest_compressed ["
                     + rest_compressed
                     + "]");

            if (stream.avail_in == 0 && rest_compressed != 0)
            {
                // set available in

                stream.avail_in = Z_BUFSIZE;

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP8")
                         + " stream.avail_in ["
                         + stream.avail_in
                         + "] rest_compressed ["
                         + rest_compressed
                         + "]");

                if (stream.avail_in > rest_compressed)
                {
                    stream.avail_in = (uInt)rest_compressed;
                }

                // set file position to the next compressed data

                if (util.seek(zf, pos + startPos, SEEK_SET) != 0)
                {
                    *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error set pos to next compressed data ["
                          + pos + " + " + startPos + "].\n";
                    rc =  kZIPGeneralZipError;

                    break;
                }

                // read stream.avail_in bytes of data from file to inbuf

                if (fread(inbuf, stream.avail_in, 1, zf) != 1)
                {
                    *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error reading file bytes ["
                          + stream.avail_in
                          + "].\n";
                    rc = kSTAFFileReadError;

                    break;
                }

                // prepare file position to next compressed data

                pos += stream.avail_in;

                // reduce rest_compressed data size

                rest_compressed -= stream.avail_in;

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP11")
                         + " stream.avail_in ["
                         + stream.avail_in
                         + "] rest_compressed ["
                         + rest_compressed
                         + "] pos ["
                         + pos
                         + "]");

                // set zip stream to point to inbuf

                stream.next_in = (Bytef*)inbuf;
            }

            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP12")
                         + " compressionMethod ["
                         + compressionMethod
                         + "]");

            // if no compression
            if (compressionMethod == 0)
            {
                uLong processed_bytes;

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP13")
                         + " stream.avail_in ["
                         + stream.avail_in
                         + "] stream.avail_out ["
                         + stream.avail_out
                         + "]");

                if (stream.avail_out < stream.avail_in)
                {
                    processed_bytes = stream.avail_out;
                }
                else
                {
                    processed_bytes = stream.avail_in;
                }

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP14")
                         + " processed_bytes ["
                         + processed_bytes
                         + "]");

                for (int i = 0; i < processed_bytes; i++)
                {
                    *(stream.next_out + i) = *(stream.next_in + i);
                }

                newcrc32 = crc32(newcrc32, stream.next_out, processed_bytes);

                stream.avail_in -= processed_bytes;
                stream.avail_out -= processed_bytes;

                stream.next_out += processed_bytes;
                stream.next_in += processed_bytes;

                stream.total_out += processed_bytes;

                uncompressed_bytes += processed_bytes;

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP15")
                         + " stream.avail_in ["
                         + stream.avail_in
                         + " stream.avail_out ["
                         + stream.avail_out
                         + " stream.total_out ["
                         + stream.total_out
                         + " uncompressed_bytes ["
                         + uncompressed_bytes
                         + " newcrc32 ["
                         + newcrc32
                         + "]");
            }
            else
            {
                // start deflating the file

                uLong total_out_before, processed_bytes;
                total_out_before = stream.total_out;

                const Bytef *bufBefore;
                bufBefore = stream.next_out;

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP16")
                         + " total_out_before ["
                         + total_out_before
                         + "]");

                // unzip

                err = inflate(&stream, Z_SYNC_FLUSH);

                processed_bytes = stream.total_out - total_out_before;

                // calculate crc

                newcrc32 = crc32(newcrc32, bufBefore, (uInt)(processed_bytes));

                // calculate uncompressed bytes

                uncompressed_bytes += (uInt)processed_bytes;

                STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP17")
                         + " err ["
                         + err
                         + "] stream.total_out ["
                         + stream.total_out
                         + "] processed_bytes ["
                         + processed_bytes
                         + "] newcrc32 ["
                         + newcrc32
                         + "] uncompressed_bytes ["
                         + uncompressed_bytes
                         + "]");

                if (err != Z_OK)
                {
                    *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error inflate file, err ["
                          + err
                          + "].\n";

                    break;
                }
            } // compressionMethod == 0?
        } // while stream.avail_out > 0

        if (rc != kSTAFOk || (err != Z_OK && err != Z_STREAM_END))
        {
            if ((*result).length() == 0)
            {
                *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error deflating file, err ["
                          + err
                          + "].\n";
            }

            rc = kZIPGeneralZipError;

            break;
        }

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP20")
                         + " uncompressed_bytes ["
                         + uncompressed_bytes
                         + "]");

        // write the unzipped buffer to file

        if (uncompressed_bytes > 0)
        {
            if (fwrite(outbuf, uncompressed_bytes, 1, outfile) != 1)
            {
                *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Error writing file bytes ["
                          + uncompressed_bytes
                          + "].\n";

                rc = kSTAFFileWriteError;

                break;
            }

            rest_uncompressed -= uncompressed_bytes;

            STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP22")
                         + " rest_uncompressed ["
                         + rest_uncompressed
                         + "]");
        }
    } // loop to read compressed data from zip file

    if (compressionMethod != 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipLocalFileHeader::doExtract_CP23")
                         + " compressionMethod ["
                         + compressionMethod
                         + "]");

        // inflate end

        inflateEnd(&stream);
    }

    // compare crc

    if (rc == kSTAFOk)
    {
        if (newcrc32 != crc)
        {
           *result = STAFString("STAFZipLocalFileHeader::doExtract: ")
                          + "Bad CRC new crc ["
                          + newcrc32
                          + "] old crc ["
                          + crc
                          + "].\n";
            rc = kZIPBadCRC;
        }
    }

    free(inbuf);
    free(outbuf);

    return rc;
}


// destructor

STAFZipLocalFileHeader::~STAFZipLocalFileHeader()
{
    if (fullFileName != NULL)
    {
        free(fullFileName);
    }

    if (extraField != NULL)
    {
        free(extraField);
    }
}


// extract zip archive to output dir

STAFRC_t STAFZipLocalFileHeader::extract(FILE *zf, uLong startPos,
                                         const char *outputdir,
                                         STAFString *result)
{
    STAFRC_t rc;
    STAFZipUtil util = STAFZipUtil();

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::extract_CP1")
                     + " outputdir ["
                     + outputdir
                     + "]");

    fullFileName = (char*)calloc(strlen(outputdir) + fileNameLength + 1, 1);

    if (fullFileName == NULL)
    {
        *result = STAFString("STAFZipLocalFileHeader::extract: ")
                          + "Error allocating memory for fullFileName ["
                          + (strlen(outputdir) + fileNameLength + 1)
                          + "].\n";
        return kZIPNotEnoughMemory;
    }

    strcpy(fullFileName, outputdir);
    strcat(fullFileName, fileName);

    fileName = fullFileName + strlen(outputdir);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::extract_CP3")
                     + " fileName ["
                     + fileName
                     + "] fullFileName ["
                     + fullFileName
                     + "]");

    if (*(fileName + strlen(fileName) - 1) == '\\'
        || *(fileName + strlen(fileName) - 1) == '/')
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::extract_CP4"));

        // create dir

        if ((rc = util.makeDir(fullFileName)) != kSTAFOk)
        {
            *result = STAFString("STAFZipLocalFileHeader::extract: ")
                          + "Error making dir ["
                          + fullFileName
                          + "].\n";
        }

        return rc;
    }

    FILE *outfile;

    // make sure the target directories exist before extracting files

    if ((outfile = fopen(fullFileName, "wb")) == NULL)
    {
        // finding fileName without path

        char *p, *filename_withoutpath;
        p = filename_withoutpath = fullFileName;

        while ((*p) != '\0')
        {
            if (((*p) == '/') || ((*p) == '\\'))
            {
                filename_withoutpath = p + 1;
            }

            p++;
        }

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::extract_CP5")
                     + " filename_withoutpath ["
                     + filename_withoutpath
                     + "]");

        // store the char before the file name without path

        char c = *(filename_withoutpath - 1);

        // to form a string which contains only the directory

        *(filename_withoutpath - 1) = '\0';

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::extract_CP6")
                     + " fullFileName ["
                     + fullFileName
                     + "]");

        // make the directory

        if ((rc = util.makeDir(fullFileName)) != kSTAFOk)
        {
            *result = STAFString("STAFZipLocalFileHeader::extract: ")
                          + "Error making directory: ["
                          + fullFileName
                          + "].\n";
            return rc;
        }

        *(filename_withoutpath - 1) = c;

        if ((outfile = fopen(fullFileName, "wb")) == NULL)
        {
            *result = STAFString("STAFZipLocalFileHeader::extract: ")
                          + "Error creating file ["
                          + fullFileName
                          + "].\n";

            return kZIPGeneralZipError;
        }
    }

    rc = doExtract(zf, startPos, outfile, result);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipLocalFileHeader::extract_CP8")
                     + " rc ["
                     + rc
                     + "]");

    fclose(outfile);

    if (rc == kSTAFOk)
    {
        tm filedate;

        util.fileTime(lastModifiedTimeDate, &filedate);
        util.changeFileDate(fullFileName, lastModifiedTimeDate, filedate);
    }

    return rc;
}





