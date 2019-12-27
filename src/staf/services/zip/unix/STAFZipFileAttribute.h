/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPFileAttribute
#define STAF_ZIPFileAttribute

class STAFZipFileAttribute
{
public:

    unsigned short filename_length;

    char *filename;

    mode_t mode;  //uLong

    uid_t owner;  // uLong

    gid_t group;  // uLong


    // methods

    STAFZipFileAttribute(char*, struct stat*);

    STAFZipFileAttribute();

    STAFRC_t set(const char*);

    ~STAFZipFileAttribute();
};


#endif
