/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Converter
#define STAF_Converter

///////////////////////////////////////////////////////////////////////////////
// STAFConverter
// This STAFConverter serves as the mechanism to convert from/to the local
// codepage into UTF-8 which is the encoding that STAF uses internally when
// dealing with strings. The STAFConverter uses a compact data storage in
// order to allow multiple mappings to map to the same data in memory, thus
// saving space. 
///////////////////////////////////////////////////////////////////////////////

#include "STAF.h"
#include "STAF_fstream.h"
#include <map>
#include <string>
#include <vector>
#include <ctype.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

const unsigned int MAX_CHAR_SIZE     = 4;           // may be changed
const unsigned int MAX_UNI_CHAR_SIZE = 2;           // may be changed
const unsigned int MAX_CPG_CHAR_SIZE = 4;           // may be changed
const unsigned int MAX_UTF8_CHAR_SIZE = 3;

///////////////////////////////////////////////////////////////////////////////
// Forward declaration

typedef unsigned char Leaf;
class Node;
class CompactTree;

///////////////////////////////////////////////////////////////////////////////

class STAFConverter
{
public:

    enum FileType { kUCM = 0, kBIN = 1 };
    
    STAFConverter(char *cpName = 0, FileType type = kBIN);

    ~STAFConverter();

    unsigned int convertFromUTF8(const unsigned char **src, 
                                 unsigned int *srclen,
                                 unsigned char *trg, 
                                 unsigned int *trglen);

    unsigned int convertToUTF8(const unsigned char **src, 
                               unsigned int *srclen,
                               unsigned char *trg, 
                               unsigned int *trglen);

    static char *determineCodePage();
    
private:

    enum ConvClass { kUNKNOWN = 0, kSBCS   = 1, kDBCS = 2, kMBCS = 3, 
                     kEBCDIC  = 4, kLATIN1 = 5, kUTF8 = 6 };

    typedef unsigned int (STAFConverter::*ConvFunc) (const unsigned char **,
                                                     unsigned int *,
                                                     unsigned char *,
                                                     unsigned int *);
    // inner data structures

    struct Header;
    friend struct Header;

    struct Header
    {
        unsigned int  signature;
        unsigned char code_set_name[12];
        ConvClass     conv_class;
        unsigned int  max_uni_size;
        unsigned int  max_cpg_size;
        unsigned char def_uni_char[MAX_UNI_CHAR_SIZE];
        unsigned char def_cpg_char[MAX_CPG_CHAR_SIZE];
    };

    Header fHeader;

    // static data members

    static char *sConvDirPtr;
    static bool  sAliasCreated;
    static std::map<std::string, std::string> *sAliasTable;

    // static methods

    static char *determineConvDir();    
    static unsigned int createAliasTable();

    // private data members

    CompactTree *pC2UData;
    CompactTree *pU2CData;
    
    unsigned char fCharSize[256];  // contains the size of chars
                                   // using lead-byte (byte[0])
                                   // as index
    // private methods

    unsigned int fromUCMFile(std::string converterName);
    unsigned int fromBINFile(std::string converterName);

    unsigned int encodeUTF8(const unsigned char *uniChar, 
                            unsigned char *buffer);

    unsigned int decodeUTF8(const unsigned char *utfChar,
                            unsigned char *buffer);

    unsigned int fromLATIN1(const unsigned char **src, 
                            unsigned int *srclen,
                            unsigned char *trg, 
                            unsigned int *trglen);

    unsigned int fromUTF8(const unsigned char **src, 
                          unsigned int *srclen,
                          unsigned char *trg, 
                          unsigned int *trglen);

    unsigned int fromSBCS(const unsigned char **src, 
                          unsigned int *srclen,
                          unsigned char *trg, 
                          unsigned int *trglen);

    unsigned int fromDBCS(const unsigned char **src, 
                          unsigned int *srclen,
                          unsigned char *trg, 
                          unsigned int *trglen);

    unsigned int fromMBCS(const unsigned char **src, 
                          unsigned int *srclen,
                          unsigned char *trg, 
                          unsigned int *trglen);

    unsigned int fromEBCDIC(const unsigned char **src, 
                            unsigned int *srclen,
                            unsigned char *trg, 
                            unsigned int *trglen);

    unsigned int toLATIN1(const unsigned char **src, 
                          unsigned int *srclen,
                          unsigned char *trg, 
                          unsigned int *trglen);

    unsigned int toUTF8(const unsigned char **src, 
                        unsigned int *srclen,
                        unsigned char *trg, 
                        unsigned int *trglen);

    unsigned int toSBCS(const unsigned char **src, 
                        unsigned int *srclen,
                        unsigned char *trg, 
                        unsigned int *trglen);

    unsigned int toDBCS(const unsigned char **src, 
                        unsigned int *srclen,
                        unsigned char *trg, 
                        unsigned int *trglen);

    unsigned int toMBCS(const unsigned char **src, 
                        unsigned int *srclen,
                        unsigned char *trg, 
                        unsigned int *trglen);

    unsigned int toEBCDIC(const unsigned char **src, 
                          unsigned int *srclen,
                          unsigned char *trg, 
                          unsigned int *trglen);
    ConvFunc fC2UFunc;
    ConvFunc fU2CFunc;

    STAFConverter(const STAFConverter &);

};

///////////////////////////////////////////////////////////////////////////////

class Node
{
public:

    union
    {
        size_t   index[256];
        Node     *node[256];
        Leaf     *leaf[256];
    };

    Node();
    Node(Node *next);
    Node(Leaf *next);
};

///////////////////////////////////////////////////////////////////////////////

class CompactTree
{
public:

    CompactTree();

    CompactTree(unsigned int sizeOfKey, unsigned int sizeOfVal,
                const unsigned char *defVal = 0);

    ~CompactTree();

    void put(const unsigned char *key, const unsigned char *val);
    const unsigned char *get(const unsigned char *key);

    int serialize(fstream &outStream);
    int deserialize(fstream &inStream);

private:

    // this enum type is used so that the destructor knows what to delete
    enum TreeMode { kUnknown = 0, kSerialize = 1, kDeserialize = 2 };

    // these structures help create the tree initially
    unsigned int        fNodeSize;
    unsigned int        fLeafSize;
    unsigned int        fLevelIndex [MAX_CHAR_SIZE];
    std::vector<void*>  fLevelVector[MAX_CHAR_SIZE];
    
    TreeMode fMode;                 // this tree's mode (serial./deserial.)

    Node *pNodeBuffer;              // only this pointer is allocated,
    Leaf *pLeafBuffer;              // this one points within pNodeBuffer's

    int fSizeOfKey;                 // equiv. to tree's height
    int fSizeOfVal;                 // byte-size of stored element
};

///////////////////////////////////////////////////////////////////////////////

#endif
