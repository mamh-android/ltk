/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
#include "STAF.h"
#include "STAFString.h"
#include "STAFTrace.h"

#include <sys/stat.h>
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


// extract files with restore permission
STAFRC_t STAFZipLocalFileHeader::extract(FILE *zf, uLong startPos,
                                         const char *outputdir,
                                         STAFZipCentralDirExtension* cde, 
                                         void* vfh, STAFString *result)
{
    STAFRC_t rc;

    STAFZipUtil util = STAFZipUtil();

    STAFZipFileHeader *fh = (STAFZipFileHeader*)vfh;
    
    STAFTrace::trace(
        kSTAFTraceServiceResult,
        STAFString("STAFZipLocalFileHeader::extract_CP1 outputdir [") +
        outputdir + "]");

    fullFileName = (char*)calloc(strlen(outputdir) + fileNameLength + 1, 1);

    if (fullFileName == NULL)
    {
        *result = STAFString("STAFZipLocalFileHeader::extract: ") +
            "Error allocating memory for fullFileName [" +
            (strlen(outputdir) + fileNameLength + 1) + "].\n";

        return kZIPNotEnoughMemory;
    }

    strcpy(fullFileName, outputdir);
    strcat(fullFileName, fileName);

    fileName = fullFileName + strlen(outputdir);

    STAFTrace::trace(
        kSTAFTraceServiceResult,
        STAFString("STAFZipLocalFileHeader::extract_CP3") +
        " fileName [" + fileName + "] fullFileName [" + fullFileName + "]");

    if (*(fileName + strlen(fileName) - 1) == '\\'
        || *(fileName + strlen(fileName) - 1) == '/')
    {
        STAFTrace::trace(
            kSTAFTraceServiceResult,
            STAFString("STAFZipLocalFileHeader::extract_CP4"));

        // This is a directory, so create it then return

        if ((rc = util.makeDir(fullFileName, cde, outputdir)) != kSTAFOk)
        {
            *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                "Error making dir [" + fullFileName + "].\n";
        }
        else
        {
            // HACK: not sure why the attribute bytes are on high end

            uLong externalAttribute = fh->externalFileAttributes>>16;
            
            if (externalAttribute != 0)
            {
                if (chmod(fullFileName, externalAttribute) != 0)
                {
                    STAFTrace::trace(
                        kSTAFTraceServiceResult,
                        STAFString("STAFZipLocalFileHeader::extract_CP4.5") +
                        "Invalid File Permission Mode");
                }
            }
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

        STAFTrace::trace(
            kSTAFTraceServiceResult,
            STAFString("STAFZipLocalFileHeader::extract_CP5") +
            " filename_withoutpath [" + filename_withoutpath + "]");

        // store the char before the file name without path

        char c = *(filename_withoutpath - 1);

        // to form a string which contains only the directory

        *(filename_withoutpath - 1) = '\0';

        STAFTrace::trace(
            kSTAFTraceServiceResult,
            STAFString("STAFZipLocalFileHeader::extract_CP6") +
            " fullFileName [" + fullFileName + "]");

        // make the directory

        if ((rc = util.makeDir(fullFileName, cde, outputdir)) != kSTAFOk)
        {
            *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                "Error making directory: [" + fullFileName + "].\n";
            return rc;
        }

        *(filename_withoutpath - 1) = c;


        if ((outfile = fopen(fullFileName, "wb")) == NULL)
        {
            *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                "Error creating file [" + fullFileName + "].\n";

            return kZIPGeneralZipError;
        }
    }

    rc = doExtract(zf, startPos, outfile, result);

    STAFTrace::trace(
        kSTAFTraceServiceResult,
        STAFString("STAFZipLocalFileHeader::extract_CP8 rc [") + rc + "]");

    fclose(outfile);

    if (rc == kSTAFOk)
    {
        tm filedate;

        util.fileTime(lastModifiedTimeDate, &filedate);
        util.changeFileDate(fullFileName, lastModifiedTimeDate, filedate);

        // XXX HACK: not sure why the attribute bytes are on high end
        uLong externalAttribute = fh->externalFileAttributes >> 16;

        if (S_ISLNK(externalAttribute))
        {
            FILE *lf;

            if ((lf = fopen(fullFileName, "rb")) == NULL) 
            {
                *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                    "Error open link file name: [" +
                    STAFString(fullFileName) + "]";

                return kZIPGeneralZipError;
            }
            
            if (util.seek(lf, 0, SEEK_END) == -1)
            {
                fclose(lf);

                *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                    "Error determining length of link target file [ " +
                    STAFString(fullFileName) + "] due to a seek error";

                return kZIPGeneralZipError;
            }
            
            STAFInt64_t targetFilenameLen = 0;
            targetFilenameLen = util.tell(lf);

            if (targetFilenameLen == -1)
            {
                fclose(lf);

                *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                    " Error determining length of link target file [ " +
                    STAFString(fullFileName) + "] due to a tell error";

                return kZIPGeneralZipError;
            }

            if (util.seek(lf, 0, SEEK_SET) == -1)
            {
                fclose(lf);

                *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                    "Error moving current position to the beginning of "
                    "link target file [" + STAFString(fullFileName) +
                    "] due to a seek error";

                return kZIPGeneralZipError;
            }

            char targetFilename[MAXFILENAME + 1] = "";
                       
            if (fread(targetFilename, targetFilenameLen, 1, lf) != 1)
            {
                fclose(lf);

                *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                    "Error link target file name, size: [" +
                    targetFilenameLen + "]";

                return kZIPGeneralZipError;
            }

            fclose(lf);

            remove(fullFileName);

            if (symlink(targetFilename, fullFileName))  /* create the real link */
            {
                *result = STAFString("STAFZipLocalFileHeader::extract: ") +
                    "Error creating link to: [" + STAFString(targetFilename) +
                    "]";
                return kZIPGeneralZipError;
            }
        }
            
        // change file attributes

        STAFZipFileAttribute *fa;
        fa = cde->find(fileName);

        if (fa != NULL)
        {
            rc = fa->set(outputdir);

            if (rc != kSTAFOk)
            {
                *result = STAFString("STAFZipLocalFileHeader::extract ") +
                    "Error set file attribute [" + fileName + "].\n";
            }
        } 
        else
        {            
            if (externalAttribute != 0)
            {
                if (chmod(fullFileName, externalAttribute) != 0)
                {
                    STAFTrace::trace(
                        kSTAFTraceServiceResult,
                        STAFString("STAFZipLocalFileHeader::extract_CP9") +
                        "Invalid File Permission Mode");
                }
            }
        }            
    }

    return rc;
}

