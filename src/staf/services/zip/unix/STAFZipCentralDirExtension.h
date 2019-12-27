/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPCentralDirExtension
#define STAF_ZIPCentralDirExtension


class STAFZipCentralDirExtension
{
public:

    // Size of fields in a file attribute record in the central dir extension
    static const int sFileNameLengthSize = 2;
    static const int sModeSize = 4;
    static const int sUidSize = 4;
    static const int sGidSize = 4;

    std::vector<STAFZipFileAttribute*> falist;

    std::map<STAFString, STAFZipFileAttribute*> falistSorted;

    uLong offset;

    uLong size;

    uLong signature;

    // methods

    STAFZipCentralDirExtension();

    STAFRC_t readInData(FILE*, uLong, uLong, STAFString*);

    STAFRC_t flush(FILE*, STAFString*);

    STAFRC_t addFileAttribute(const char*, int, STAFString*);

    STAFZipFileAttribute* find(char*);

    ~STAFZipCentralDirExtension();
};


#endif
