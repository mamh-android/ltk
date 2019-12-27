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

#include "STAFZip.h"
#include "STAFZipFileAttribute.h"

// constructor
STAFZipFileAttribute::STAFZipFileAttribute()
{
    filename_length = 0;

    filename = NULL;

    mode = 0;

    owner = 0;

    group = 0;
}


// constructor
STAFZipFileAttribute::STAFZipFileAttribute(char *name, struct stat *s)
{
    filename_length = strlen(name);

    filename = NULL;

    if (filename_length > 0)
    {
        filename = (char*)calloc(filename_length + 1, 1);

        strcpy(filename, name);
    }


    mode = s->st_mode;

    owner = s->st_uid;

    group = s->st_gid;

}


// set outputdir file attribute
STAFRC_t STAFZipFileAttribute::set(const char *outputdir)
{

    char buffer[MAXFILENAME] = "";

    strcpy(buffer, outputdir);

    strcat(buffer, filename);

    STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFileAttribute::set_CP1")
                     + " outputdir ["
                     + outputdir
                     + " filename ["
                     + filename
                     + " buffer ["
                     + buffer
                     + "]");

    // try to set owner group
    if (chown(buffer, owner, group) != 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFileAttribute::set_CP2")
                     + "Invalid Owner Group");

//        return kZIPInvalidOwnerGroup;
    }

    // try to set permission
    if (chmod(buffer, mode) != 0)
    {
        STAFTrace::trace(kSTAFTraceServiceResult,
                     STAFString("STAFZipFileAttribute::set_CP3")
                     + "Invalid File Permission Mode");

//        return kZIPInvalidFileMode;
    }

    return kSTAFOk;
}


// destructor
STAFZipFileAttribute::~STAFZipFileAttribute()
{
    if (filename != NULL)
    {
        free(filename);
    }

}

