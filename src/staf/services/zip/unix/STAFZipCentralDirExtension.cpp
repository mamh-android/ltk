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

#include "STAFZip.h"
#include "STAFZipUtil.h"
#include "STAFZipFileAttribute.h"
#include "STAFZipCentralDirExtension.h"


// constructor
STAFZipCentralDirExtension::STAFZipCentralDirExtension()
{

    signature = 0x01024653;  // STAF

    offset = 0;

    size = 4;

}


// read in central dir entension data
STAFRC_t STAFZipCentralDirExtension::readInData(
    FILE *zf, uLong centralDirOffset, uLong endOfLastLocalFileHeaderOffset,
    STAFString *result)
{
    // search for central dir extension section
    
    uLong cdeSize = centralDirOffset - endOfLastLocalFileHeaderOffset;
    
    if (cdeSize == 0)
    {
        return kSTAFOk;
    }

    unsigned char* buf;

    buf = (unsigned char*)malloc(cdeSize);
    if (buf == NULL)
    {
        *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                  + "Error allocating memory for reading in central dir extension ["
                  + (cdeSize)
                  + "].\n";

        return kZIPNotEnoughMemory;
    }

    STAFZipUtil util = STAFZipUtil();

    if (util.seek(zf, endOfLastLocalFileHeaderOffset, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                  + "Error in seek to the end of last local file header offset ["
                  + endOfLastLocalFileHeaderOffset
                  + "]\n";

        return kZIPGeneralZipError;
    }


    // read in a block of data from the file
    if (fread(buf, (uInt)cdeSize, 1, zf) != 1)
    {
        *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                  + "Error reading central dir extension data size ["
                  + cdeSize
                  + "].\n";

        return kZIPGeneralZipError;
    }


    // search for central dir extension signature
    for (int i = (int)cdeSize - 3; (i--) > 0;)
    {
        if (((*(buf + i)) == 0x53) && ((*(buf + i + 1)) == 0x46) &&
            ((*(buf + i + 2)) == 0x02) && ((*(buf + i + 3))== 0x01))
        {
            offset = endOfLastLocalFileHeaderOffset + i;
            
            size = centralDirOffset - offset;
            
            break;
        }
    }

    if (offset == 0)
    {
        return kSTAFOk;
    }


    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDirExtension::readInData_CP1")
                     + " offset ["
                     + offset
                     + " size ["
                     + size
                     + "]");

    uLong sig;

    STAFRC_t rc;


    if (util.seek(zf, offset, SEEK_SET) != 0)
    {
        *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                  + "Error in seek to the beginning of central dir extension ["
                  + offset
                  + "]\n";

        return kZIPGeneralZipError;
    }


    // get signature
    rc = util.getLong(zf, &sig);

    if (rc == kSTAFOk)
    {
        if (sig != signature)
        {
            *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                          + "Wrong signature ["
                          + sig
                          + "].\n";

            return kZIPGeneralZipError;
        }
    }
    else
    {
        *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                          + "Error getting signature ["
                          + sig
                          + "].\n";

        return rc;
    }


    long ul = (long)size;

    ul -= 4;

    // while there is still data in central dir extension block
    while (ul > 0)
    {
        STAFZipFileAttribute *fa = new STAFZipFileAttribute();

        // get filename length
        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util.getShort(zf, &iL);

            fa->filename_length = (unsigned short)iL;
        }

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipCentralDirExtension::readInData_CP4")
                         + " ul=" + ul + " fa->filename_length ["
                         + fa->filename_length + "] rc [" + rc + "]");

        if (rc == kSTAFOk && fa->filename_length <= 0)
        {
            *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                          + "filename_length less than zero ["
                          + fa->filename_length + "].\n";

            rc = kZIPGeneralZipError;
        }


        // get filename
        if (rc == kSTAFOk)
        {
            if ((fa->filename = (char*)calloc(fa->filename_length + 1, 1))
                 != NULL)
            {
                if (fread(fa->filename, (uInt)fa->filename_length, 1, zf) != 1 )
                {
                    *result = STAFString(
                         "STAFZipCentralDirExtension::readInData: ")
                          + "Error getting file name.\n";

                    rc = kZIPGeneralZipError;
                }
                else
                {
                    STAFTrace::trace(kSTAFTraceServiceResult,
                            STAFString(
                            "STAFZipCentralDirExtension::readInData_CP7")
                             + " fa->filename [" + fa->filename + "]");
                }
            }
            else
            {
                *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                          + "Error allocating memory for file name length ["
                          + (fa->filename_length + 1) + "].\n";

                rc = kZIPNotEnoughMemory;
            }
        }


        // get mode
        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util.getLong(zf, &iL);

            fa->mode = (mode_t)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                           STAFString(
                           "STAFZipCentralDirExtension::readInData_CP9")
                           + " fa->mode [" + fa->mode + "] rc" + rc + "]");
        }


        // get owner
        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util.getLong(zf, &iL);

            fa->owner = (uid_t)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                           STAFString(
                           "STAFZipCentralDirExtension::readInData_CP10")
                           + " fa->owner [" + fa->owner + "] rc" + rc + "]");
        }


        // get group
        if (rc == kSTAFOk)
        {
            uLong iL;

            rc = util.getLong(zf, &iL);

            fa->group = (gid_t)iL;

            STAFTrace::trace(kSTAFTraceServiceResult,
                           STAFString(
                           "STAFZipCentralDirExtension::readInData_CP11")
                           + " fa->group [" + fa->group + "] rc" + rc + "]");
        }

        if (rc != kSTAFOk)
        {
            delete fa;

            if ((*result).length() == 0)
            {
                *result = STAFString("STAFZipCentralDirExtension::readInData: ")
                          + "Error getting data.\n";
            }

            break;
        }

        // Reduce the size counter by one record
        // Note: A file attribute record in the central dir extension block
        //       consists of the following fields:
        //       Filename length, File name, Mode, Owner (uid), Group (gid)

        ul -= sFileNameLengthSize + fa->filename_length +
              sModeSize + sUidSize + sGidSize;

        /* Uncomment for debugging
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString(
                         "STAFZipCentralDirExtension::readInData_CP13")
                         + " ul=" + ul + " after decrementing by "
                         + (sFileNameLengthSize + fa->filename_length +
                            sModeSize + sUidSize + sGidSize));
        */

        falist.push_back(fa);
        falistSorted[STAFString(fa->filename)] = fa;
    }

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString(
                     "STAFZipCentralDirExtension::readInData_CP14")
                     + " ul=" + ul + " (should be 0)"
                     + " after reading central dir extension block");

    return rc;
}


// flush the central dir extension to zip archive
STAFRC_t STAFZipCentralDirExtension::flush(FILE *zf, STAFString *result)
{

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDirExtension::flush_CP1")
                     + " size ["
                     + size
                     + "]");

    if (size == 0)
    {
        return kSTAFOk;
    }

    STAFRC_t rc = kSTAFOk;

    std::vector<STAFZipFileAttribute*>::iterator i;

    STAFZipUtil util = STAFZipUtil();


    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipCentralDirExtension::flush_CP2")
                     + " signature ["
                     + signature
                     + "]");

    // signature
    rc = util.putValue(zf, (uLong)signature, 4);


    // write file attribute record into zip archive
    for (i = falist.begin(); i != falist.end(); i++)
    {

        // filenamelength
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDirExtension::flush_CP3")
                             + " (*i)->filename_length ["
                             + (*i)->filename_length
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->filename_length,
                               sFileNameLengthSize);
        }


        // file name
        if (rc == kSTAFOk && (*i)->filename_length > 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDirExtension::flush_CP4")
                             + " (*i)->filename ["
                             + (*i)->filename
                             + "]");

            if (fwrite((*i)->filename, (uInt)(*i)->filename_length, 1, zf) != 1)
            {
                *result = STAFString("STAFZipCentralDirExtension::flush: ")
                          + "Error writing filename ["
                          + (*i)->filename
                          + "].\n";

                rc = kSTAFFileWriteError;
            }
        }


        // mode
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDirExtension::flush_CP6")
                             + " (*i)->mode ["
                             + (*i)->mode
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->mode, sModeSize);
        }


        // owner
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDirExtension::flush_CP7")
                             + " (*i)->owner ["
                             + (*i)->owner
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->owner, sUidSize);
        }


        // group
        if (rc == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipCentralDirExtension::flush_CP8")
                             + " (*i)->group ["
                             + (*i)->group
                             + "]");

            rc = util.putValue(zf, (uLong)(*i)->group, sGidSize);
        }


        if (rc != kSTAFOk)
        {
            if ((*result).length() == 0)
            {
                *result = STAFString("STAFZipCentralDirExtension::flush: ")
                          + "Error writing data.\n";
            }

            break;
        }
    }

    return rc;
}


// add a file attribute record into central dir extension
STAFRC_t STAFZipCentralDirExtension::addFileAttribute(const char *fullname,
                                             int prefix_len, STAFString *result)
{
    char *buffer ;
    char *p;
    int len = strlen(fullname);
    STAFRC_t rc;

    int err;


    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString(
                     "STAFZipCentralDirExtension::addFileAttribute_CP1")
                     + " len ["
                     + len
                     + " fullname ["
                     + fullname
                     + " prefix_len ["
                     + prefix_len
                     + "]");

    if (len <= 0)
    {
        return kSTAFOk;
    }

    // allocate buffer to make a copy of fullname
    if ((buffer = (char*)calloc(len + 1, 1)) == NULL)
    {
        *result = STAFString("STAFZipCentralDirExtension::addFileAttribute: ")
                          + "Error allocating memory for fullname ["
                          + (len + 1)
                          + "].\n";

        return kZIPNotEnoughMemory;
    }

    strcpy(buffer, fullname);


    // if this fullname is a directory, remove the ending slash
    if (buffer[len - 1] == '/' || buffer[len - 1] == '\\')
    {
        buffer[len-1] = '\0';
    }

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString(
                     "STAFZipCentralDirExtension::addFileAttribute_CP3")
                     + " buffer ["
                     + buffer
                     + "]");

    // setup a pointer to point to the first char after prefix
    // i.e. given full name: /opt/staf/mydata.dat,
    // prefix /opt/
    // we will start processing from staf/mydata.dat
    p = buffer + prefix_len + 1;

    while (1)
    {
        char hold;

        // get the first level of directory
        while(*p && *p != '\\' && *p != '/')
        {
          p++;
        }

        hold = *p;

        *p = 0;

        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString(
                     "STAFZipCentralDirExtension::addFileAttribute_CP4")
                     + " buffer ["
                     + buffer
                     + "] hold ["
                     + hold
                     + "]");

        struct stat s;

        if ((err = lstat(buffer, &s)) != 0)
        {
            *result =
                STAFString("STAFZipCentralDirExtension::addFileAttribute: ")
                + "Error getting file status ["
                + buffer
                + "] err ["
                + err
                + "].\n";

            return kZIPGeneralZipError;
        }


        STAFZipFileAttribute *fa;


        // search the current central dir extension for existing file attribute
        if ((fa = find(buffer + prefix_len)) == NULL)
        {
            // if not found, create a new file attribute entry

            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString(
                     "STAFZipCentralDirExtension::addFileAttribute_CP6")
                     + " buffer+prefix_len ["
                     + (buffer + prefix_len)
                     + "]");

            fa = new STAFZipFileAttribute(buffer + prefix_len, &s);

            falist.push_back(fa);
            falistSorted[STAFString(fa->filename)] = fa;

            size += sFileNameLengthSize + fa->filename_length +
                    sModeSize + sUidSize + sGidSize;
        }
        else
        {
            // if found, replace the existing entry with new value

            fa->mode = s.st_mode;

            fa->owner = s.st_uid;

            fa->group = s.st_gid;

            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString(
                     "STAFZipCentralDirExtension::addFileAttribute_CP7")
                     + " buffer+prefix_len ["
                     + (buffer + prefix_len)
                     + "] fa->mode ["
                     + fa->mode
                     + "] fa->owner ["
                     + fa->owner
                     + "] fa->group ["
                     + fa->group
                     + "]");
        }



        if (hold == 0)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                    STAFString(
                    "STAFZipCentralDirExtension::addFileAttribute_CP8"));

            // if it is the end of the fullname
            break;
        }

        // restore the char
        *p++ = hold;
    }


    return kSTAFOk;
}


// find filename in the central dir extension section
STAFZipFileAttribute* STAFZipCentralDirExtension::find(char *filename)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                    STAFString("STAFZipCentralDirExtension::find_CP1")
                    + " filename ["
                    + filename
                    + "]");

    std::map<STAFString, STAFZipFileAttribute*>::iterator i;

    i = falistSorted.find(
        STAFString(filename).replace(kUTF8_BSLASH, kUTF8_SLASH));

    if (i != falistSorted.end())
        return i->second;
    else
        return NULL;
}


// destructor
STAFZipCentralDirExtension::~STAFZipCentralDirExtension()
{

    std::vector<STAFZipFileAttribute*>::iterator i;

    for (i = falist.begin(); i != falist.end(); i++)
    {
        delete *i;
    }

}





