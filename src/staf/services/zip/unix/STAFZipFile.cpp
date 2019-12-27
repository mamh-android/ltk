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


// constructor
STAFZipFile::STAFZipFile(STAFHandlePtr h, FILE *file, STAFRC_t *rc, 
                         STAFString *result, int fileExist,
                         STAFInt64_t end, STAFInt64_t start,
                         const STAFString &zfName)
{
    handle = h;
    zf = file;
    zipFileName = zfName;
    util = new STAFZipUtil(handle);
    
    if (end == -1)
    {
        // Determine the size of the file

        if (util->seek(file, 0, SEEK_END) != 0)
        {
            *result = STAFString(
                "STAFZipUtil(): Error determining size of zip file [") +
                zipFileName + "] due to a seek error";

            *rc = kSTAFFileOpenError;
            return;
        }

        STAFInt64_t size = util->tell(file);
        
        if (size == -1)
        {
            *result = STAFString(
                "STAFZipUtil(): Error determining size of zip file [") +
                zipFileName + "] due to a tell error";

            *rc = kSTAFFileOpenError;
            return;
        }

        endPos = size;
    }
    else
    {
        endPos = end;
    }

    if (start == -1)
    {
        if (util->seek(file, 0, SEEK_SET) != 0)
        {
            *result = STAFString(
                "STAFZipUtil(): Error moving current position to the "
                "beginning of zip file [") + zipFileName +
                "] due to a seek error";

            *rc = kSTAFFileOpenError;
            return;
        }

        startPos = util->tell(file);

        if (startPos == -1)
        {
            *result = STAFString(
                "STAFZipUtil(): Error moving current position to the "
                "beginning of zip file [") + zipFileName +
                "] due to a tell error";

            *rc = kSTAFFileOpenError;
            return;
        }
    }
    else
    {
        startPos = start;
    }
        
    newZipFile = (fileExist) ? 0 : 1; 

    cde = new STAFZipCentralDirExtension();
    centralDirPtr = new STAFZipCentralDir();

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::STAFZipFile_CP1")
                     + " startPos [" + startPos + "]"
                     + " endPos [" + endPos + "]"
                     + " newZipFile [" + newZipFile + "]");

    if (!newZipFile)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::STAFZipFile_CP2"));

        // file exists, try to read in central dir data
        if ((*rc = centralDirPtr->readInData(zf, (uLong)startPos, 
            (uLong)endPos, result)) == kSTAFOk)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::STAFZipFile_CP3"));

            if ((*rc = readInData(result)) == kSTAFOk)
            {
                STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::STAFZipFile_CP3.5"));

                // get the last local file header in zip archive

                STAFZipLocalFileHeader *lfh = find(
                    centralDirPtr->findLastFileHeader()->fileName);
                
                // get central dir extension offset

                uLong endOfLocalFileHeaderOffset = lfh->offset + lfh->size;

                STAFTrace::trace(kSTAFTraceServiceResult,
                                 STAFString("STAFZipFile::STAFZipFile_CP4")
                                 + " endOfLocalFileHeaderOffset ["
                                 + endOfLocalFileHeaderOffset
                                 + "]");

                *rc = cde->readInData(
                    zf, centralDirPtr->getOffset() + startPos,
                    endOfLocalFileHeaderOffset + startPos, result);

            } // end local file header readInData
        } // end central dir readInData
    } // end if newZipFile
}

// zipping the file

STAFRC_t STAFZipFile::zipFile(const char *pathname, int prefixlen,
                              int recursive, STAFString *result)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::zipFile_CP1")
                     + " pathname ["
                     + pathname
                     + "] prefixlen ["
                     + prefixlen
                     + "] recursive ["
                     + recursive
                     + "]");

    // check to make sure the pathname exists

    STAFFSPath path(pathname);

    if (!path.exists())
    {
        *result = STAFString("STAFZipFile::zipFile: ")
                  + "File does not exist ["
                  + pathname
                  + "]\n";

        return kSTAFDoesNotExist;
    }
    
    STAFRC_t rc;

    // build localFileHeaderListNew

    rc = zipDir(pathname, prefixlen, recursive, result);

    // if zip dir was unsuccessful or local file header list is empty

    if (rc != kSTAFOk || localFileHeaderListNew.empty())
    {
        if (rc != kSTAFOk)
        {
            return rc;
        }
        else
        {
            *result = STAFString("STAFZipFile::zipFile: ")
                      + STAFString("No file entries read.");

            return kZIPGeneralZipError;
        }
    }

    std::vector<STAFZipLocalFileHeader*>::iterator i;

    if (newZipFile)
    {
        // adding files to a new zip archive

        for (i = localFileHeaderListNew.begin(); i != localFileHeaderListNew.end();
             i++)
        {
            // save local file header into zip archive

            if ((rc = (*i)->flush(zf, result)) != kSTAFOk)
            {
                break;
            }

            // add file header to central dir

            centralDirPtr->addFileHeader(*i);
        }
    }
    else
    {
        // appending files to existing zip file

        if (cde->offset > 0)
        {
            // start appending local file header from the beginning of
            // central dir extension

            if (util->seek(zf, cde->offset, SEEK_SET) != 0)
            {
                *result = STAFString("STAFZipFile::zipFile: ")
                              + "Can't seek central dir extention ["
                              + cde->offset
                              + "].\n";
                return kZIPGeneralZipError;
            }
        }
        else
        {
            // start appending local file header from the beginning of
            // central dir

            if (util->seek(zf, centralDirPtr->getOffset(), SEEK_SET) != 0)
            {
                *result = STAFString("STAFZipFile::zipFile: ")
                              + STAFString("Can't seek central dir [")
                              + centralDirPtr->getOffset()
                              + STAFString("].\n");
                return kZIPGeneralZipError;
            }
        }

        // appending local file headers one by one

        for (i = localFileHeaderListNew.begin(); i != localFileHeaderListNew.end();
            i++)
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::zipFile_CP7")
                             + " (*i)->fileName ["
                             + (*i)->fileName
                             + "]");

            // search if there is any existing entry

            if (centralDirPtr->find((*i)->fileName) != NULL)
            {
                *result = STAFString("STAFZipFile::zipFile: ")
                              + "File name exists in zip ["
                              + (*i)->fileName
                              + "].\n";
                rc = kSTAFAlreadyExists;

                break;
            }

            // save local file header into zip archive

            if ((rc = (*i)->flush(zf, result)) != kSTAFOk)
            {
                break;
            }

            // append file header to central dir

            centralDirPtr->addFileHeader(*i);

        } // iterate all local file headers
    } // newZipFile

    // flush central dir extension to zip file

    if (rc == kSTAFOk)
    {
        rc = cde->flush(zf, result);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::zipFile_CP10")
                         + " rc ["
                         + rc
                         + "]");
    }

    // flush central dir to zip file

    if (rc == kSTAFOk)
    {
        centralDirPtr->cder->centralDirOffset = util->tell(zf);
        rc = centralDirPtr->flush(zf, result);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::zipFile_CP11")
                         + " rc ["
                         + rc
                         + "]");
    }

    return rc;
}

// zip directory

STAFRC_t STAFZipFile::zipDir(const char *pathname, int prefixlen,
                             int recursive, STAFString *result)
{
    char filename[MAXFILENAME] = "";
    int i, rc = kSTAFOk;

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::zipDir_CP1")
                     + " pathname ["
                     + pathname
                     + "] prefixlen ["
                     + prefixlen
                     + "] recursive ["
                     + recursive
                     + "]");

    // Assign the zip file backup name

    STAFString zipFileBackupName = STAFString("");

    if (zipFileName != 0)
        zipFileBackupName = zipFileName + ".ZIP";

    STAFString path(pathname);
    STAFFSPath directoryPath(path);
    
    // Do so that any // will be converted to a /, etc. and so that
    // any trailing slashes are removed
    directoryPath.setRoot(directoryPath.root());

    STAFFSEntryPtr entry;

    try
    {
        entry = directoryPath.getEntry();
    }
    catch (STAFBaseOSErrorException &e)
    {
        *result = STAFString("STAFZipFile:zipDir: ")
                  + e.getText() + STAFString(": ")
                  + e.getErrorCode()
                  + "\nError on Source Directory: "
                  + path;
        return kSTAFDoesNotExist;
    }

    // if the value of path name is a file

    if (entry->type() == kSTAFFSFile)
    {
        // this is a file

        if(*(pathname + strlen(pathname) - 1) == '/' ||
           *(pathname + strlen(pathname) - 1) == '\\')
        {
            // invalid file name

            *result = STAFString("STAFZipFile:zipDir: ")
                  + "Invalid file name: ["
                  + pathname
                  + "].\n";

            return kSTAFInvalidValue;
        }

        STAFString entryName = entry->path().asString();

        if ((entryName == zipFileName) ||
            (entryName == zipFileBackupName))
        {
            *result = STAFString("STAFZipFile:zipDir: ") +
                "The name of the file to be zipped cannot be the same as "
                "the name of the zip file being created/updated";

            return kSTAFInvalidValue;
        }
        else
        {
            // Add the entry to the localFileHeaderListNew list

            util->convertSTAFStringBuffer(entryName.toCurrentCodePage(),
                                          filename);

            // create a local file header for the current file
        
            STAFZipLocalFileHeader *lfh = new STAFZipLocalFileHeader(
                filename, prefixlen);

            localFileHeaderListNew.push_back(lfh);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::zipDir_CP3"));

            // add file attribute into central dir extension

            rc = cde->addFileAttribute(pathname, prefixlen, result);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::zipDir_CP4") +
                             " rc [" + rc + "]");
        }
    }
    else if (entry->type() == kSTAFFSDirectory)
    {
        // This is a directory

        if (*(pathname + strlen(pathname) - 1) != '/' &&
            *(pathname + strlen(pathname) - 1) != '\\')
        {
            *result = STAFString("STAFZipFile:zipDir: ")
                  + "Invalid directory name: ["
                  + pathname
                  + "].\n";

            return kSTAFInvalidValue;
        }

        // Number of entries in the directory
        i = 0;

        unsigned int entryTypesUInt = kSTAFFSFile | kSTAFFSDirectory;

        STAFFSEnumPtr entryEnum = entry->enumerate();

        // zip the directory

        for (; entryEnum->isValid(); entryEnum->next())
        {
            STAFString entryName = entryEnum->entry()->path().asString().
                replace(kUTF8_BSLASH, kUTF8_SLASH);

            if (entryName.length() > MAXFILENAME - 1)
            {
                *result = STAFString("STAFZipFile:zipDir: ")
                          + "File name length exceeds "
                          + MAXFILENAME
                          + " charactors: ["
                          + entryName
                          + "].\n";

                return kSTAFInvalidValue;                
            }
            
            if ((entryName == zipFileName) ||
                (entryName == zipFileBackupName))
            {
                // Don't add an entry for the zip file or backup zip file
                // being created/updated 
                continue;
            }
            
            // increase number of entries by one
            i++;

            util->convertSTAFStringBuffer(entryName.toCurrentCodePage(),
                                          filename);

            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::zipDir_CP6")
                     + " filename ["
                     + filename
                     + "]");

            if (entryEnum->entry()->type() == kSTAFFSDirectory)
            {
                // it is a dir, add ending "/" to the pathname

                char *p;               
                p = (char*)filename;
                
                while (*p != '\0') p++;
                
                if (*(p - 1) != '\\' && *(p - 1) != '/')
                {
                    *p = '/';                    
                    *(p + 1) = '\0';
                }
            } 
            else if (entryEnum->entry()->type() == kSTAFFSFile)
            {
                // it is a file, remove any possible ending "/" from the pathname

                char *p;                
                p = (char*)filename;
                
                while (*p != '\0') p++;
                
                if (*(p - 1) == '\\' || *(p - 1) == '/')
                {
                    *(p - 1) = '\0';
                }
            }

            // zip dir recursively

            if (recursive)
            {
                STAFTrace::trace(kSTAFTraceServiceResult,
                                 STAFString("STAFZipFile::zipDir_CP8")
                                 + " filename ["
                                 + filename
                                 + "]");

                // zip dir recursively

                if ((rc = zipDir(filename, prefixlen, recursive, result))
                    != kSTAFOk)
                {
                    break;
                }
            }
            else
            {
                STAFTrace::trace(kSTAFTraceServiceResult,
                                 STAFString("STAFZipFile::zipDir_CP10")
                                 + " filename ["
                                 + filename
                                 + "]");

                // create local file header for the current file

                STAFZipLocalFileHeader *lfh =
                        new STAFZipLocalFileHeader(filename, prefixlen);

                localFileHeaderListNew.push_back(lfh);

                // add file attribute of the current file

                if ((rc = cde->addFileAttribute(filename, prefixlen, result))
                    != kSTAFOk)
                {
                    break;
                }
            } // if recurse
        } // loop to read dir entries

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::zipDir_CP12")
                         + " filename ["
                         + filename
                         + "]");

        if ((i == 0) && (rc == kSTAFOk))
        {
            // Zip the empty diretory

            STAFZipLocalFileHeader *lfh = new STAFZipLocalFileHeader(
                pathname, prefixlen);

            if (lfh->fileNameLength == 0)
            {
                *result = STAFString("STAFZipFile:zipDir: ") +
                    "The directory to zip is empty and the relativeto value "
                    "is the same as the directory path so there's nothing "
                    "to zip";

                return kZIPGeneralZipError;
            }

            localFileHeaderListNew.push_back(lfh);

            rc = cde->addFileAttribute(pathname, prefixlen, result);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::zipDir_CP13")
                             + " pathname ["
                             + pathname
                             + "] rc ["
                             + rc
                             + "]");
        }
    }
    else
    {
        *result = STAFString("STAFZipFile:zipDir: ")
                          + "Pathname is neither a file nor a directory: ["
                          + pathname
                          + "].\n";

        return kZIPGeneralZipError;
    }

    return rc;
}

// unzip one file

STAFRC_t STAFZipFile::unzipFile(const char *filename,
                                char *outputdir, int replace,
                                int restorepermission,
                                STAFString *result)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipFile_CP1")
                     + " filename ["
                     + filename
                     + "] outputdir ["
                     + outputdir
                     + "] replace ["
                     + replace
                     + "] restorepermission ["
                     + restorepermission
                     + "]");

    int rc = kSTAFOk;
    char fullname[MAXFILENAME] = "";

    // construct full file name

    strcpy(fullname, outputdir);
    strcat(fullname, filename);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipFile_CP3")
                     + " fullname ["
                     + fullname
                     + "]");

    // search central dir for file name

    STAFZipFileHeader *fh = centralDirPtr->find(filename);

    if (fh == NULL)
    {
        // Did not find the file in the zipfile

        *result = STAFString("STAFZipFile::unzipFile: ") +
            "File name not found in zip archive [" + filename + "].\n";

        return kSTAFDoesNotExist;
    }

    // Found the file

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipFile_CP4")
                     + " filename ["
                     + filename
                     + "]");

    if (*(filename + strlen(filename) - 1) == '\\' ||
        *(filename + strlen(filename) - 1) == '/')
    {
        // this is a directory

        STAFString path(fullname);
        STAFFSPath directoryPath(path);

        if (directoryPath.exists())
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::unzipFile_CP5"));

            // dir exists, do nothing
            
            return kSTAFOk;
        }
    }
    else
    {
        FILE *f;

        // this is a file, test if the file exists

        if ((f = fopen(fullname, "rb")) != NULL)
        {
            // file exists

            fclose(f);

            // if not replace

            if (!replace)
            {
                *result = STAFString("STAFZipFile::unzipFile: ") +
                    "File already exists [" + fullname + "].\n";

                return kSTAFAlreadyExists;
            }
        }
    }

    // find the local file header based on the file header

    STAFZipLocalFileHeader *lfh = find(fh->fileName);

    if (restorepermission)
    {
        // extract the file and restore permission

        rc = lfh->extract(zf, (uLong)startPos, outputdir, cde, fh, result);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::unzipFile_CP7")
                         + " rc [" + rc + "]");
    }
    else
    {
        // extract the file, don't restore permission

        rc = lfh->extract(zf, (uLong)startPos, outputdir, result);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::unzipFile_CP8")
                         + " rc [" + rc + "]");
    }

    return rc;
}

// unzip all files from zip archive

STAFRC_t STAFZipFile::unzipFile(char *outputdir, int replace,
                                int restorepermission, STAFString *result)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipFile2_CP1")
                     + " outputdir ["
                     + outputdir
                     + "] replace ["
                     + replace
                     + "] restorepermission ["
                     + restorepermission
                     + "]");

    STAFRC_t rc = kSTAFOk;
    char fullname[MAXFILENAME] = "";
    std::vector<STAFZipLocalFileHeader*>::iterator i;

    // loop through all file headers in central dir

    for (i = localFileHeaderListCurrent.begin();
         i != localFileHeaderListCurrent.end(); i++)
    {
        // construct full file name

        strcpy(fullname, outputdir);
        strcat(fullname, (*i)->fileName);

        STAFTrace::trace(kSTAFTraceServiceResult,
                         STAFString("STAFZipFile::unzipFile2_CP3")
                         + " fullname [" + fullname + "]");

        if (*((*i)->fileName + (*i)->fileNameLength - 1) == '\\' ||
            *((*i)->fileName + (*i)->fileNameLength - 1) == '/')
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipFile2_CP4"));

            // this is a directory

            STAFString path(fullname);
            STAFFSPath directoryPath(path);

            if (directoryPath.exists())
            {
                STAFTrace::trace(kSTAFTraceServiceResult,
                                 STAFString("STAFZipFile::unzipFile2_CP5"));

                // dir exists, do nothing

                continue;
            }
        }
        else
        {
            STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipFile2_CP6"));

            FILE *f;

            // this is a file, test to see if the file exists

            if ((f = fopen(fullname, "rb")) != NULL)
            {
                // file exists

                fclose(f);

                if (!replace)
                {
                    *result = STAFString("STAFZipFile::unzipFile2: ")
                              + "File already exists ["
                              + fullname
                              + "].\n";
                    return kSTAFAlreadyExists;
                }
            }
        }

        if (restorepermission)
        {
            // search central dir for file name

            STAFZipFileHeader *fh = centralDirPtr->find((*i)->fileName);

            // extract file and restore permission

            rc = (*i)->extract(zf, (uLong)startPos, outputdir, cde, fh, result);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::unzipFile2_CP8")
                             + " rc ["
                             + rc
                             + "]");
        }
        else
        {
            // extract file, don't restore permission

            rc = (*i)->extract(zf, (uLong)startPos, outputdir, result);

            STAFTrace::trace(kSTAFTraceServiceResult,
                             STAFString("STAFZipFile::unzipFile2_CP9")
                             + " rc ["
                             + rc
                             + "]");
        }

        if (rc != kSTAFOk)
        {
            break;
        }
    }

    return rc;
}

// unzip dir

STAFRC_t STAFZipFile::unzipDir(const char *dirname, char *outputdir,
                               int replace, int restorepermission,
                               STAFString *result)
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::unzipDir_CP1")
                     + " dirname ["
                     + dirname
                     + "] outputdir ["
                     + outputdir
                     + "] replace ["
                     + replace
                     + "] restorepermission ["
                     + restorepermission
                     + "]");

    int rc = kSTAFOk;

    std::vector<STAFString> nameList = findDir(dirname);

    if (nameList.size() > 0)
    {
        std::vector<STAFString>::iterator i;
    
        for (i = nameList.begin(); i != nameList.end(); i++) 
        {
            // unzip the file entry

            if ((rc = unzipFile(((*i)+STAFString(kUTF8_NULL)).buffer(), outputdir, 
                 replace, restorepermission, result)) != kSTAFOk)
            {
                // if fails, exit the loop

                break;
            }
        }
    }
    else
    {
        *result = STAFString("STAFZipFile::unzipDir_CP3: ")
                  + STAFString("Directory ")
                  + STAFString(dirname)
                  + STAFString(" does not exist in zip archive");
        rc = kSTAFDoesNotExist;
    }

    return rc;
}

// destructor

STAFZipFile::~STAFZipFile()
{
    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFile::destructor_CP1"));

    std::vector<STAFZipLocalFileHeader*>::iterator i;

    for (i = localFileHeaderListNew.begin(); i != localFileHeaderListNew.end(); i++)
    {
        delete *i;
    }

    for (i = localFileHeaderListCurrent.begin(); i != localFileHeaderListCurrent.end(); i++)
    {
        delete *i;
    }

    delete centralDirPtr;
    delete util;
    delete cde;
}



