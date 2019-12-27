/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////

#include "STAF.h"
#include "STAF_fstream.h"
#include "STAF_iostream.h"
#include "STAFUtil.h"
#include "STAFConverter.h"

///////////////////////////////////////////////////////////////////////////////

Node::Node()
{
    for (unsigned i = 0; i < 256; i++)
        index[i] = 0;
}

Node::Node(Node *nextNode)
{
    for (unsigned i = 0; i < 256; i++)
        node[i] = nextNode;
}

Node::Node(Leaf *nextLeaf)
{
    for (unsigned i = 0; i < 256; i++)
        leaf[i] = nextLeaf;
}

///////////////////////////////////////////////////////////////////////////////

CompactTree::CompactTree()
{ 
    // if this constructor is used, then we set its mode to unknown since 
    // no memory has been allocated. using this constructor does not gua-
    // rantee that the user will ever deserialize the tree,  so we should 
    // not let the destructor delete memory that has not been allocated.

    fMode      = kUnknown;
    fNodeSize  = sizeof(Node);
    fLeafSize  = 0; // set by deserialize
}

CompactTree::CompactTree(unsigned int sizeOfKey, unsigned int sizeOfVal,
                         const unsigned char *defValPtr)
{
    // if this constructor is used, then we set its mode as  serialize. the
    // destructor will walk down the levels and  delete each node/leaf when
    // the tree is in serialize mode.  note that even if somebody used this
    // constructor but never writes into the tree or serializes the tree it
    // will still be ok since we allocate 1 node/leaf for each level as the
    // default. 

    fMode      = kSerialize;
    fSizeOfKey = sizeOfKey;
    fSizeOfVal = sizeOfVal;
    fNodeSize  = sizeof(Node);
    fLeafSize  = 256 * fSizeOfVal;

    int i = 0;

    for (i = 0; i < fSizeOfKey; i++)
        fLevelIndex[i] = 0;

    // allocate 1 leaf of 256 Chars (not chars) and initialize
    // with default value or zeroes if no default specified

    Leaf *leafPtr = new Leaf[fLeafSize];

    if (defValPtr != 0)
    {
        Leaf *nextValPtr = leafPtr;
        for (i = 0; i < 256; i++, nextValPtr += fSizeOfVal)
            memcpy(nextValPtr, defValPtr, fSizeOfVal);
    }
    else memset(leafPtr, 0, fLeafSize);

    // create the default nodes for each level and initialize

    for (i = 0; i < fSizeOfKey - 1; i++)
        fLevelVector[i].push_back((void *)new Node());

    fLevelVector[i].push_back((void *)leafPtr);
}

CompactTree::~CompactTree()
{
    switch (fMode)
    {
        case kSerialize:
        break;

        case kDeserialize:
        delete[] pNodeBuffer;
        return;
 
        default:
        return;
    }

    // this is reached only when the tree was in serialized mode.
    // in serialized mode, we have allocated individual nodes/leaves
    // for each level (at least 1 per level for the default) so we
    // must now delete them.

    int i = 0, j = 0;

    // walk down each level and delete each node/leaf

    for (i = 0; i < fSizeOfKey - 1; i++)
    {
        for (j = 0; j < fLevelIndex[i]; j++)
        {
            Node *nextNodePtr = (Node *)fLevelVector[i][j];
            delete nextNodePtr;
        }
    }

    for (j = 0; j < fLevelIndex[i]; j++)
    {
        Leaf *nextLeafPtr = (Leaf *)fLevelVector[i][j];
        delete nextLeafPtr;
    }
}

int CompactTree::serialize(fstream &outStream)
{
    // write the sizes of both the key and the values to
    // the binary file

    outStream.write((char *)&fSizeOfKey, sizeof(fSizeOfKey));
    outStream.write((char *)&fSizeOfVal, sizeof(fSizeOfVal));

    int i = 0;

    // write the count of nodes/leaves per level

    for (i = 0; i < fSizeOfKey; i++)
    {
        unsigned int count = fLevelVector[i].size();
        outStream.write((char *)&count, sizeof(count));
    }

    int j = 0;

    // write each node traversing the tree in width-first order

    for (i = 0; i < fSizeOfKey - 1; i++)
    {
        for (j = 0; j < fLevelVector[i].size(); j++)
        {      
            Node *nextPtr = (Node *)fLevelVector[i][j];
            outStream.write((char *)nextPtr, fNodeSize);
        }
    }

    // write each leaf as well (i = fSizeOfKey - 1)

    for (j = 0; j < fLevelVector[i].size(); j++)
    {
        Leaf *leafPtr = (Leaf *)fLevelVector[i][j];
        outStream.write((char *)leafPtr, fLeafSize);
    }

    return 0;
}

int CompactTree::deserialize(fstream &inStream)
{
    // read the sizes of both the key and the values from
    // the binary file

    inStream.read((char *)&fSizeOfKey, sizeof(fSizeOfKey));
    inStream.read((char *)&fSizeOfVal, sizeof(fSizeOfVal));

    fLeafSize = 256 * fSizeOfVal; 

    int i = 0;

    // read the count of nodes/leaves per level

    for (i = 0; i < fSizeOfKey; i++)
    {
        inStream.read((char *)&fLevelIndex[i], sizeof(unsigned int));
    }

    unsigned int totalNodes = 0;
    unsigned int totalLeaves = 0;

    // count up the total number of inner nodes and leaves

    for (i = 0; i < fSizeOfKey - 1; i++)
        totalNodes += fLevelIndex[i]; 

    totalLeaves = fLevelIndex[i];

    try
    {
        // since we allocate memory here then we set the mode
        // as deserialize in order to let the destructor know
        // what it needs to clean up
         
        fMode = kDeserialize;

        // allocate one common buffer (for cache efficiency)
        // to store both inner nodes and leaves

        char *pBuffer = new char[fNodeSize * totalNodes +
                                 fLeafSize * totalLeaves];

        // Note: pNodeBuffer + totalNodes increment in Node-
        // size bytes, so there is no need to scale by 256

        pNodeBuffer = (Node *)(pBuffer);
        pLeafBuffer = (Leaf *)(pNodeBuffer + totalNodes);

        // special case for key size = 1, there must be only
        // 1 leaf and no nodes so just read it in and return

        if (fSizeOfKey == 1)
        {
            inStream.read((char *)pLeafBuffer, fLeafSize);
            return 0;
        }

        // read the entire nodes. while deserializing, convert
        // all node contained indeces into actual pointers to
        // their respective next-level nodes

        inStream.read((char *)pNodeBuffer, fNodeSize * totalNodes);

        Node *nextNodePtr = pNodeBuffer;

        int j = 0, k = 0;
        
        // for each level ...
        for (i = 0; i < fSizeOfKey - 2; i++)
        {
            // for each node in the level ...
            for (j = 0; j < fLevelIndex[i]; j++, nextNodePtr++)
            {
                // for each entry in the node ...
                for (k = 0; k < 256; k++)
                {
                    // point to its corresponding next level node
                    nextNodePtr->node[k] = nextNodePtr + 
                                           nextNodePtr->index[k] +
                                           fLevelIndex[i] - j;
                }
            }
        }

        // read the entire leaves. while deserializing, convert
        // all node contained indeces into actual pointers to
        // their respective data leaves
        
        inStream.read((char *)pLeafBuffer, fLeafSize * totalLeaves);

        // for each node in the previous-to-last level ...
        for (j = 0; j < fLevelIndex[i]; j++, nextNodePtr++)
        {
            // for each entry in the node ...
            for (k = 0; k < 256; k++)
            {
                // point to its corresponding leaf
                nextNodePtr->leaf[k] = pLeafBuffer + 
                                       nextNodePtr->index[k] * 
                                       256 * fSizeOfVal;
            }
        }
    }
    catch (...)
    {
        cerr << "Caught unknown exception in CompactTree::deserialize()"
             << endl;

        return 2;
    }

    return 0;
}

void CompactTree::put(const unsigned char *key, const unsigned char *val)
{
    Leaf *leafPtr;

    // XXX: do real exception handling

    if (key == 0 || val == 0)
    {
        cerr << "CompactTree::put(), key or val = NULL" << endl;
        return;
    }

    // special case for key size 1

    if (fSizeOfKey == 1)
    {
        leafPtr = (Leaf *)fLevelVector[0][0];
        memcpy((char *)&leafPtr[ key[0] * fSizeOfVal ], val, fSizeOfVal);
        return;
    }

    int i = 0;
    Node *prevPtr = 0;
    Node *nextPtr = (Node *)fLevelVector[0][0];
    bool needsNodeBranch = false;

    // check if a node branch is needed

    for (i = 0; i < fSizeOfKey - 2 && !needsNodeBranch; i++)
    {
        if (nextPtr->index[key[i]] == 0)
        {
            needsNodeBranch = true;
            break;
        }

        prevPtr = nextPtr;
        nextPtr = (Node *)fLevelVector[i + 1][nextPtr->index[key[i]]];
    }

    if (needsNodeBranch)
    {
        // i has the index of last non-default level
        // prevPtr points to last non-default level,
        // nextPtr points to first default level

        // create internal nodes

        for (; i < fSizeOfKey - 2; i++)
        {
            nextPtr->index[key[i]] = fLevelVector[i + 1].size();
            prevPtr = nextPtr;
            nextPtr = new Node();
            fLevelVector[i + 1].push_back((void *)nextPtr);
        }
    }

    // check if a leaf branch is needed, otherwise get the
    // leaf

    if (nextPtr->index[key[i]] == 0)
    {
        nextPtr->index[key[i]] = fLevelVector[i + 1].size();
        leafPtr = new Leaf[fLeafSize];
        memcpy(leafPtr, fLevelVector[fSizeOfKey - 1][0], fLeafSize);
        fLevelVector[fSizeOfKey - 1].push_back((void *)leafPtr);
    }
    else 
    {
        leafPtr = (Leaf *)fLevelVector[fSizeOfKey - 1][nextPtr->index[key[i]]];
    }

    // copy value into leaf and we are done!!!

    memcpy((char *)&leafPtr[ key[fSizeOfKey - 1] * fSizeOfVal ], 
           val, fSizeOfVal);
}

const unsigned char *CompactTree::get(const unsigned char *key)
{
    Leaf *leafPtr = pLeafBuffer;
    Node *nextPtr = pNodeBuffer;

    // XXX: do real exception handling

    if (key == 0) 
    {
        cerr << "CompactTree::get(), key = NULL" << endl;
        return 0;
    }

    switch (fSizeOfKey)
    {
        case 1:
        
        switch (fSizeOfVal)
        {
            case 1:
            return &leafPtr[key[0]];

            case 2:
            return &leafPtr[key[0] << 1];

            case 4:
            return &leafPtr[key[0] << 2];

            default:
            break;
        }

        break;

        case 2:

        switch (fSizeOfVal)
        {
            case 1:
            leafPtr = nextPtr->leaf[key[0]];
            return &leafPtr[key[1]];

            case 2:
            leafPtr = nextPtr->leaf[key[0]];
            return &leafPtr[key[1] << 1];

            case 4:
            leafPtr = nextPtr->leaf[key[0]];
            return &leafPtr[key[1] << 2];

            default:
            break;
        }

        break;

        case 4:
    
        switch (fSizeOfVal)
        {
            case 1:
            nextPtr = nextPtr->node[key[0]];
            nextPtr = nextPtr->node[key[1]];
            leafPtr = nextPtr->leaf[key[2]];
            return &leafPtr[key[3]];
            
            case 2:
            nextPtr = nextPtr->node[key[0]];
            nextPtr = nextPtr->node[key[1]];
            leafPtr = nextPtr->leaf[key[2]];
            return &leafPtr[key[3] << 1];

            case 4:
            nextPtr = nextPtr->node[key[0]];
            nextPtr = nextPtr->node[key[1]];
            leafPtr = nextPtr->leaf[key[2]];
            return &leafPtr[key[3] << 2];

            default:
            break;

        }

        break;

        default:
        break;
    }

    int i = 0;

    // uncommon cases get here

    // walk down until last node (one level before the leaf)
    for (i = 0; i < fSizeOfKey - 2; i++)
        nextPtr = nextPtr->node[key[i]];

    // now get the leaf ...
    leafPtr = nextPtr->leaf[key[i]];

    // scale by size of value and return
    return &leafPtr[ key[fSizeOfKey - 1] * fSizeOfVal ];
}

///////////////////////////////////////////////////////////////////////////////

const unsigned int SIGNATURE = 0xDEADC0DE;        // bin file signature
const unsigned int INVALID_STRING  = 1;           // error code
const unsigned int NOT_IMPLEMENTED = 2;           // error code

static const char SIZE_TABLE[] =
{
    // This table allows for O(1) lookup of a char size.
 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};

///////////////////////////////////////////////////////////////////////////////

const char *kAliasNamePtr          = "alias.txt";
const char *kDefaultCodePagePtr    = "LATIN_1";

#ifdef STAF_OS_NAME_MACOSX
    const char *kDefaultConvDirPtr = "/Library/staf/codepage";
#elif STAF_OS_NAME_WIN32
    const char *kDefaultConvDirPtr = "C:/STAF/codepage";
#else
    const char *kDefaultConvDirPtr = "/usr/local/staf/codepage";
#endif

bool  STAFConverter::sAliasCreated = false;
char *STAFConverter::sConvDirPtr   = 0;
std::map<std::string, std::string> *STAFConverter::sAliasTable;

///////////////////////////////////////////////////////////////////////////////

STAFConverter::STAFConverter(char *cpName, FileType fileType)
{ 
    pC2UData = pU2CData = 0;

    memset(&fHeader, 0, sizeof(fHeader));
    memset((char *)&fCharSize, 0, sizeof(fCharSize));

    fHeader.max_uni_size = MAX_UNI_CHAR_SIZE;
    fHeader.def_uni_char[MAX_UNI_CHAR_SIZE - 1] = 0x2f;

    if (fileType == kUCM) 
    {
        fromUCMFile(std::string(cpName));
        return;
    }

    if (cpName == 0)
    {
        cpName = determineCodePage();
    }
        
    if (sConvDirPtr == 0)
    {
        sConvDirPtr = determineConvDir();
    } 

    if (sAliasCreated == false)
    { 
        createAliasTable();
        sAliasCreated = true;
    }

    std::string lowerCPName(cpName);

    for(int i = 0; i < lowerCPName.length(); ++i)
        lowerCPName[i] = tolower(lowerCPName[i]);

    // Note: we need the 'C' string from lowerCPName, otherwise
    // the hashtable key is a different type and would not find
    // the value being looked up

    std::string convName = (*sAliasTable)[lowerCPName.c_str()];
    std::string finalName;

    if (convName == "LATIN_1")
    {
        fHeader.conv_class = kLATIN1;
        fC2UFunc = &STAFConverter::fromLATIN1;
        fU2CFunc = &STAFConverter::toLATIN1;
        return;
    }
    else if (convName == "UTF8")
    {
        fHeader.conv_class = kUTF8;
        fC2UFunc = &STAFConverter::fromUTF8;
        fU2CFunc = &STAFConverter::toUTF8;
        return;
    }
    else if (convName == "")
    { finalName = lowerCPName; }
    else
    { finalName = convName;    }

    if (fromBINFile(finalName))
    {
        fHeader.conv_class = kLATIN1;
        fC2UFunc = &STAFConverter::fromLATIN1;
        fU2CFunc = &STAFConverter::toLATIN1;

        cerr << "WARNING: Defaulting to " << kDefaultCodePagePtr << endl;
    }
}

STAFConverter::~STAFConverter()
{ 
    if (pU2CData) delete pU2CData;
    if (pC2UData) delete pC2UData;
}

char *STAFConverter::determineCodePage()
{
    // try to determine the codepage

    static char codePageBuffer[32];

    // STAFCODEPAGEOVERRIDE is only used to force a codepage to be
    // selected. It should not be documented since it is for inter-
    // nal use only.

    char *cpName = getenv("STAFCODEPAGEOVERRIDE");

    // if not set, then ask the OS which codepage we are operating
    // on

    if (cpName == 0 || strlen(cpName) == 0)
    {
        cpName = STAFUtilGetCurrentProcessCodePage(codePageBuffer);
    }

    // if not set, then check if the STAFCODEPAGE env variable is
    // set

    if (cpName == 0 || strlen(cpName) == 0)
    {
        cpName = getenv("STAFCODEPAGE");
    }

    // if not set, then put a warning and assume the system we are
    // working on used the default codepage (LATIN_1)

    if (cpName == 0 || strlen(cpName) == 0)
    {
        // if no system code page found, set it to default and warn user

        cerr << "WARNING: Could not determine codepage." << endl
             << "env STAFCODEPAGE not set; defaulting to "
             << kDefaultCodePagePtr << endl;

        cpName = (char *)kDefaultCodePagePtr;
    }

    return cpName;
}

char *STAFConverter::determineConvDir()
{
    char *convDir = getenv("STAFCONVDIR");

    // if STAFCONVDIR is not set, we have no means to know were the code-
    // page files and the alias file are located, so assume a directory.

    if (convDir == 0 || strlen(convDir) == 0)
    {
        /* 
        WE SHOULD NOT SPIT OFF THIS WARNING
        cerr << "WARNING: Could not determine codepage directory."
             << endl << "env STAFCONVDIR not set; defaulting to "
             << kDefaultConvDirPtr << endl;
        */

        convDir = (char *)kDefaultConvDirPtr;
    }

    return convDir;
}

unsigned int STAFConverter::encodeUTF8(const unsigned char *uniChar, 
                                       unsigned char *buffer)
{
    if (uniChar == 0 || buffer == 0) return 0;

    //  Table of UTF-8 Encoding (this is how we encode UCS2)
    //  ---------------------------------------------------- 
    //  Bytes   Bits    Encoding
    //  1       7       0bbbbbbb 
    //  2       11      110bbbbb 10bbbbbb 
    //  3       16      1110bbbb 10bbbbbb 10bbbbbb 
    //  ---------------------------------------------------- 
    // e.g. UNI 0x00B8 == UTF-8 11000010 10111000
    //                             c   2    b   8
    //      UNI 0x001C == UTF-8 00011100
    //                             1   c
    //  ---------------------------------------------------- 

    unsigned short encChar = (unsigned short)((uniChar[0] << 8) |
                                              (uniChar[1]));

    /* XXX: remove this
    cout << "UNICODE CHAR = 0x" << std::hex << encChar << endl; 
    cout << "UNI[0] = " << (unsigned)(uniChar[0] & 0xff) << " " 
              << "UNI[1] = " << (unsigned)(uniChar[1] & 0xff) << endl; 
    */

    // do conversion

    if (encChar < 0x0080)
    {
        buffer[0] = uniChar[1];
        return 1;
    }
    else if (encChar < 0x0800)
    {
        buffer[0] = (0xc0 | (uniChar[0] << 2) | (uniChar[1] >> 6));
        buffer[1] = (0x80 | (uniChar[1] & 0x3f));
        return 2;
    }
    else
    {
        buffer[0] = (0xe0 |  (uniChar[0] >> 4));
        buffer[1] = (0x80 | ((uniChar[0] & 0x0f) << 2) | 
                             (uniChar[1] >> 6)); 
        buffer[2] = (0x80 |  (uniChar[1] & 0x3f));
        return 3;
    }
}

unsigned int STAFConverter::decodeUTF8(const unsigned char *utfChar,
                                       unsigned char *buffer)
{
    unsigned int size = SIZE_TABLE[utfChar[0]];
    
    if (size == 1)
    {
        buffer[0] = 0;
        buffer[1] = utfChar[0];
        return 2;
    }

    if (size == 2)
    {
        buffer[0] = ((utfChar[0] & 0x1c) >> 2);
        buffer[1] = ((utfChar[0] & 0x03) << 6) | (utfChar[1] & 0x3f);
        return 2;
    }

    if (size == 3)
    {
        buffer[0] = ((utfChar[0] & 0x0f) << 4) | ((utfChar[1] & 0x3c) >> 2);        
        buffer[1] = ((utfChar[1] & 0x03) << 6) |  (utfChar[2] & 0x3f);
        return 2;
    }

    return 0;
}

unsigned int STAFConverter::fromUCMFile(std::string converterName)
{
    std::string ucmName = converterName + ".ucm";
    fstream ucmFile(ucmName.c_str(), ios::in);
 
    if (!ucmFile) 
    {
        cerr << "Could not open file " << ucmName.c_str() << endl;
        return 1;
    }

    static const unsigned int SIZE = 1024;

    char fileLine[SIZE];
    bool inCharMap = false;

    // map files are required to be in .ucm format (ucm format is the format
    // provided by the IBM Classes for Unicode), where lines starting with
    // # are comments, lines containing <Uabcd> XY are value pairs that are
    // used to construct the binary file; other lines MAY simply be ignored.

    while (!ucmFile.eof())
    {
        fileLine[0] = 0;
        ucmFile.getline(fileLine, SIZE);

        std::string line(fileLine);
        
        for (int i = 0; (i < SIZE) && (fileLine[i] != 0); i++)
        {
            if (isspace(fileLine[i])) continue;

            if (fileLine[i] == '#') break;

            if (strncmp(fileLine, "CHARMAP", 7) == 0)
            {
                inCharMap = true;
                break;
            }

            if (fileLine[i] == '<')
            {
                char var[32];
                char val[32];                    
                
                i++;  // skip opening <

                if (!inCharMap)
                {
                    // while it is not a closing > or a space or a new
                    // line/carriage return, set var
                    int j = 0;

                    for (; (i < SIZE) && (fileLine[i] != 0) && 
                         (fileLine[i] != '>') && (!isspace(fileLine[i]));
                         i++, j++) 
                        var[j] = fileLine[i];

                    var[j] = 0;

                    // while it is a space or a closing > or an opening "
                    // skip
                    while ((i < SIZE) && (fileLine[i] != 0) &&
                           ((isspace(fileLine[i])) || (fileLine[i] == '"') ||
                            (fileLine[i] == '>'))) i++;

                    // while it is not a space and not a closing " and
                    // not a new line/carriage return, set val
                    int k = 0;

                    for (; (i < SIZE) && (fileLine[i] != 0) &&
                         (!isspace(fileLine[i])) && (fileLine[i] != '"');
                         i++, k++)
                        val[k] = fileLine[i];

                    val[k] = 0;

                    // now we actually set the values in the static file 
                    // header so that we can later write it to the binary
                    // file

                    if (strcmp(var, "code_set_name") == 0)
                    {
                        strncpy((char *)fHeader.code_set_name, val, 11); 
                    }
                    else if (strcmp(var, "uconv_class") == 0)
                    {
                        fHeader.conv_class = kUNKNOWN;

                        if (strncmp(val, "SBCS", 4) == 0) 
                        {
                            fHeader.conv_class = kSBCS;
                            fC2UFunc = &STAFConverter::fromSBCS;
                            fU2CFunc = &STAFConverter::toSBCS;
                        }
                        else if (strncmp(val, "DBCS", 4) == 0)
                        {
                            fHeader.conv_class = kDBCS;
                            fC2UFunc = &STAFConverter::fromDBCS;
                            fU2CFunc = &STAFConverter::toDBCS;
                        }
                        else if (strncmp(val, "MBCS", 4) == 0)
                        {
                            fHeader.conv_class = kMBCS;
                            fC2UFunc = &STAFConverter::fromMBCS;
                            fU2CFunc = &STAFConverter::toMBCS;
                        }
                        else if (strncmp(val, "EBCD", 4) == 0)
                        {
                            fHeader.conv_class = kEBCDIC;
                            fC2UFunc = &STAFConverter::fromEBCDIC;
                            fU2CFunc = &STAFConverter::toEBCDIC;
                        }
                    }
                    else if (strcmp(var, "mb_cur_min") == 0)
                    {
                        /* ignore, we never use this field */
                    }
                    else if (strcmp(var, "mb_cur_max") == 0)
                    {
                        fHeader.max_cpg_size = atoi(val);
                    }
                    else if (strcmp(var, "subchar") == 0)
                    {
                        // at this point we are still reading the ucm file's
                        // header. we are going to setup the default charac-
                        // ter block for the codepage represented
                        
                        j = 0, k = 0;
                        int charLen = 0;
                    
                        while (val[j] == '\\')
                        {
                            j += 2;
                            val[k++] = val[j++];
                            val[k++] = val[j++];
                            charLen++;
                        }

                        val[k] = 0;

                        // we have the default codepage character as a hex 
                        // string in 'val' (length of val depends on class
                        // of char: e.g. if class is SBCS, len of val is 2
                        // (like "A2" which represents 1 byte) for DBCS is
                        // 4 (like "E5F1" which represents 2 bytes), etc. 

                        unsigned long defCpgVal = strtoul(val, 0, 16);

                        for (k = charLen - 1; k >= 0; k--, defCpgVal >>= 8)
                        {
                            fHeader.def_cpg_char[k] = defCpgVal & 0xff;
                        }

                        fCharSize[fHeader.def_cpg_char[0]] = 
                            (unsigned char)charLen;

                    }  // end of else if "subchar"
                }
                else    // in Char Map
                {
                    if (pC2UData == 0)
                    {
                        pC2UData = new CompactTree(fHeader.max_cpg_size, 
                                                   fHeader.max_uni_size,
                                                   (const unsigned char *)
                                                   &fHeader.def_uni_char);

                        pU2CData = new CompactTree(fHeader.max_uni_size, 
                                                   fHeader.max_cpg_size,
                                                   (const unsigned char *)
                                                   &fHeader.def_cpg_char);
                    }

                    i++;  // skip U from <Uabcd>

                    // get hex unicode value into var
                    
                    var[0] = fileLine[i++];
                    var[1] = fileLine[i++];
                    var[2] = fileLine[i++];
                    var[3] = fileLine[i++];
                    var[4] = 0;

                    // while it is a space or a closing '>' skip
                    while ((i < SIZE) && (fileLine[i] != 0) &&
                           (isspace(fileLine[i]) || (fileLine[i] == '>')))
                        i++;

                    // assumption is that we have encountered the cpg char so
                    // get 2 hex digits of each, repeat until space is found
                   
                    int j = 0;
                    int charLen = 0;

                    while (fileLine[i] == '\\')
                    {
                        i += 2;
                        val[j++] = fileLine[i++];
                        val[j++] = fileLine[i++];
                        charLen++;
                    }

                    val[j] = 0;

                    unsigned long uniVal = strtoul(var, 0, 16);
                    unsigned long cpgVal = strtoul(val, 0, 16);

                    unsigned long saveCpgVal = cpgVal;
                    unsigned long saveUniVal = uniVal;

                    unsigned char uniBuffer   [MAX_UNI_CHAR_SIZE] = { 0 };
                    unsigned char cpgKeyBuffer[MAX_CPG_CHAR_SIZE] = { 0 };
                    unsigned char cpgValBuffer[MAX_CPG_CHAR_SIZE] = { 0 };
                    
                    int k;
                    
                    for (k = MAX_UNI_CHAR_SIZE - 1; k >= 0; k--, uniVal >>= 8)
                    {
                        uniBuffer[k] = (unsigned char)(uniVal & 0xff);
                    }
                    
                    // BYTE-ORDER MANIPULATION:
                    // Note: this creates the codepage key and value which
                    // are different because it truly impacts in the size
                    // of the binary file being created. Different strate-
                    // gies for 1 particular file gave a 20Mb, a 10Mb, and
                    // ultimately a 256kb binary file. What we do here is
                    // rearrange the codepage character bytes when it acts
                    // as a key and when it acts as the value being stored.
                    // For example, if we are dealing with MBCS of size 4,
                    // and a given character consists of only 2 bytes, the
                    // ls-bytes (i.e. byte[0] and byte[1]) must be zeroes
                    // when the char is acting as key, so that it can take
                    // the same path as other 2-byte keys. This does not
                    // happen for unicode since they are all 2 bytes.

                    // Lets review this manipulation of bytes with an ex-
                    // ample where again the size of the MBCS char is at
                    // most 4 bytes:

                    // MBCS char = { 0x2e, 0xff }
                    // key representation = { 0x00, 0x00, 0x2e, 0xff }
                    // val representation = { 0x2e, 0xff, 0x00, 0x00 }

                    // The val representation helps determine in faster
                    // time the exact size of the char, since we use the
                    // ls-byte as an index into the fCharLen table, and 
                    // the key representation helps all other keys whose
                    // size is also 2 take the same path in the compact
                    // tree being used for storage.

                    int m;
                    
                    for (k = fHeader.max_cpg_size - 1, m = charLen - 1; 
                         k >= 0; k--, m--, cpgVal >>= 8)
                    {
                        cpgKeyBuffer[k] = (unsigned char)(cpgVal & 0xff);
                        if (m >= 0)
                        cpgValBuffer[m] = (unsigned char)(cpgVal & 0xff);
                    }

                    fCharSize[cpgValBuffer[0]] = (unsigned char)charLen;
                    
                    // Note: This makes Won/Yen map to "\"

                    static const unsigned char sCpgValBuffer5c
                        [MAX_CPG_CHAR_SIZE] = { 0x5c, 0x00 };
                    static const unsigned char sUniBuffer005c
                        [MAX_UNI_CHAR_SIZE] = { 0x00, 0x5c };
                    
#if STAF_OS_NAME_ZOS
                    // Worrying about backslashes (0x5c) in the conversion
                    // is only an issue on non-EBCDIC systems.

                    pC2UData->put(cpgKeyBuffer, uniBuffer);
                    pU2CData->put(uniBuffer, cpgValBuffer);
#else
                    if (saveCpgVal == 0x5c)
                        pC2UData->put(cpgKeyBuffer, sUniBuffer005c);
                    else
                        pC2UData->put(cpgKeyBuffer, uniBuffer);

                    if (saveUniVal == 0x005C)
                        pU2CData->put(uniBuffer, sCpgValBuffer5c);
                    else
                        pU2CData->put(uniBuffer, cpgValBuffer);
#endif

                }  // end of in Char Map

            }  //  end of in '<'

        }  // end of for loop

    }  // end of while != eof

    // sign the header and serialize the necessary data 
    // in a binary file which must be read in same order

    fHeader.signature = SIGNATURE;

    std::string binName = converterName + ".bin";
    fstream binFile(binName.c_str(), ios::out | STAF_ios_binary);

    if (!binFile)
    {
        cerr << "Could not open file " << binName.c_str() << endl;
        return 1;
    }

    binFile.write((char *)&fHeader, sizeof(fHeader));
    pC2UData->serialize(binFile);
    pU2CData->serialize(binFile);
    
    if (fHeader.conv_class == kMBCS)
    {
        binFile.write((char *)&fCharSize, sizeof(fCharSize));
    }

    binFile.close();

    return 0;
}
 
unsigned int STAFConverter::createAliasTable() 
{
    std::string aliasName = std::string(sConvDirPtr) + "/" + 
                            std::string(kAliasNamePtr);
    fstream aliasFile(aliasName.c_str(), ios::in);
 
    if (!aliasFile) 
    {
        cerr << "Could not open file " << aliasName.c_str() << endl;
        return 1;
    }

    // insert the default codepage first to map to itself
    sAliasTable = new std::map<std::string, std::string>();
    (*sAliasTable)[std::string(kDefaultCodePagePtr)] = std::string(
        kDefaultCodePagePtr);

    const unsigned int SIZE = 1024;
    char fileLine[SIZE];

    while (!aliasFile.eof())
    {
        fileLine[0] = 0;
        aliasFile.getline(fileLine, SIZE);

        std::string line(fileLine);
        
        for (int i = 0; (i < SIZE) && (fileLine[i] != 0); i++)
        {
            if (isspace(fileLine[i])) continue;

            if (fileLine[i] == '#') break;

            // read the value, then all the different keys
            int j;
            char val[32];

            for (j = 0; !isspace(fileLine[i]); j++)
                val[j] = fileLine[i++];
            val[j] = 0;

            // add the value as a key to itself
            std::string theKey = val;
            std::string theVal = val;

			(*sAliasTable)[theKey] = theVal;

            char key[32];

            // add the keys found in the same text line
            for (; (i < SIZE) && (fileLine[i] != 0); i++)
            {
                if (isspace(fileLine[i])) continue;

                if (fileLine[i] == '#') break;

                for (j = 0; (fileLine[i] != 0) && (!isspace(fileLine[i]));
                     i++, j++)
                    key[j] = fileLine[i];
                key[j] = 0;

                i--;  // adjust (won't put as on infinite loop)

                theKey = key;
                theVal = val;
                
                (*sAliasTable)[theKey] = theVal;
            }

            break;
        }
    }

    aliasFile.close();

    return 0;
}

unsigned int STAFConverter::fromBINFile(std::string converterName)
{
    std::string binName = std::string(sConvDirPtr) + "/" + converterName + 
                          ".bin";
    fstream binFile(binName.c_str(), ios::in | STAF_ios_binary);

    if (!binFile)
    {
        cerr << "Could not open file " << binName.c_str() << endl;
        return 1;
    }

    binFile.read((char *)&fHeader, sizeof(fHeader));

    if (fHeader.signature != SIGNATURE)
    {
        cerr << "File " << binName.c_str() << " is corrupted" << endl;
        return 1;
    }

    pC2UData = new CompactTree();
    pU2CData = new CompactTree();
    
    pC2UData->deserialize(binFile);
    pU2CData->deserialize(binFile);
    
    if (fHeader.conv_class == kMBCS)
    {
        binFile.read((char *)&fCharSize, sizeof(fCharSize));
    }

    binFile.close();

    switch (fHeader.conv_class)
    {
        case kSBCS:
        fC2UFunc = &STAFConverter::fromSBCS;
        fU2CFunc = &STAFConverter::toSBCS;
        break;

        case kDBCS:
        fC2UFunc = &STAFConverter::fromDBCS;
        fU2CFunc = &STAFConverter::toDBCS;
        break;

        case kMBCS:
        fC2UFunc = &STAFConverter::fromMBCS;
        fU2CFunc = &STAFConverter::toMBCS;
        break;

        case kEBCDIC:
        fC2UFunc = &STAFConverter::fromEBCDIC;
        fU2CFunc = &STAFConverter::toEBCDIC;
        break;

        default:
        cerr << "Unknown format for file " << binName.c_str() << endl;
        return 2;
    }

    return 0;
}

unsigned int STAFConverter::convertToUTF8(const unsigned char **src,
                                          unsigned int *srclen,
                                          unsigned char *trg, 
                                          unsigned int *trglen)
{
    // srclen points to the number of bytes taken up by src, and trglen
    // points to the number of bytes allocated for trg. on return, src
    // will point to the first byte in src that was not converted, src-
    // len will contain the number of bytes left to be converted, and 
    // trglen will contain the number of bytes used from trg.

    if (fHeader.conv_class == kUNKNOWN)
        fC2UFunc = &STAFConverter::fromLATIN1;

    return (this->*fC2UFunc)(src, srclen, trg, trglen);
}

unsigned int STAFConverter::convertFromUTF8(const unsigned char **src,
                                            unsigned int *srclen,
                                            unsigned char *trg, 
                                            unsigned int *trglen)
{
    // srclen points to the number of bytes taken up by src, and trglen
    // points to the number of bytes allocated for trg. on return, src
    // will point to the first byte in src that was not converted, src-
    // len will contain the number of bytes left to be converted, and 
    // trglen will contain the number of bytes stored in trg.

    if (fHeader.conv_class == kUNKNOWN)
        fU2CFunc = &STAFConverter::toLATIN1;

    return (this->*fU2CFunc)(src, srclen, trg, trglen);
}

unsigned int STAFConverter::fromLATIN1(const unsigned char **src,
                                       unsigned int *srclen,
                                       unsigned char *trg,
                                       unsigned int *trglen)
{
    // Note: this function converts from the local filesystem character
    // set to the corresponding USC code and then converts the USC character
    // code to the UTF-8 encoding.  For Latin1 (ISO 8859-1:1998), to convert
    // the local character set to the corresponding USC code, it simply
    // creates a 2-character key: key[0] contains x00 and key[1] contains
    // the local character set character 
    //    e.g. 0x00 -> 0x0000, 0x01 -> 0x0001, ..., 0x0FF -> 0x00FF
    
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;

    *trglen = 0;

    unsigned char key[MAX_UNI_CHAR_SIZE] = { 0 };

    while (convlen > 0 && (roomLeftInBuffer > (MAX_UTF8_CHAR_SIZE - 1)))
    {
        key[1] = **src;

        register unsigned len = encodeUTF8(key, trg);
        
        if (len == 0) return INVALID_STRING;
        
        (*src)++;
        trg += len;
        (*srclen)--;
        (*trglen) += len;
        convlen--;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::fromUTF8(const unsigned char **src,
                                     unsigned int *srclen,
                                     unsigned char *trg,
                                     unsigned int *trglen)
{
    // Note: this function simply copies the UTF-8 chars from
    //       src into trg and modifies the counts as appropri-
    //       ate.

    int convlen = STAF_MIN(*srclen, *trglen);
    
    *trglen = 0;

    memcpy(trg, *src, convlen);

    *src += convlen;
    *srclen -= convlen;
    *trglen += convlen;

    return 0;
}

unsigned int STAFConverter::fromSBCS(const unsigned char **src,
                                     unsigned int *srclen,
                                     unsigned char *trg,
                                     unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;

    *trglen = 0;

    while (convlen > 0 && (roomLeftInBuffer > (MAX_UTF8_CHAR_SIZE - 1)))
    {
        register unsigned len = encodeUTF8(pC2UData->get(*src), trg);
        
        if (len == 0) return INVALID_STRING;
        
        (*src)++;
        trg += len;
        (*srclen)--;
        (*trglen) += len;
        convlen--;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::fromDBCS(const unsigned char **src,
                                     unsigned int *srclen,
                                     unsigned char *trg,
                                     unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;
    
    *trglen = 0;

    while (convlen > 0 && (roomLeftInBuffer > (MAX_UTF8_CHAR_SIZE - 1)))
    {
        register unsigned len = encodeUTF8(pC2UData->get(*src), trg);

        if (len == 0) return INVALID_STRING;
        
        (*src) += 2;
        trg += len;
        (*srclen) -= 2;
        (*trglen) += len;
        convlen -= 2;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::fromMBCS(const unsigned char **src,
                                     unsigned int *srclen,
                                     unsigned char *trg,
                                     unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;
    
    *trglen = 0;

    register unsigned m = fHeader.max_cpg_size;

    while (convlen > 0 && (roomLeftInBuffer > (MAX_UTF8_CHAR_SIZE - 1)))
    {
        // this must be initialized with zeroes on each iteration.
        // this only happens in fromMBCS and the reason for this
        // has to do with the way we use cpg characters as keys in
        // order to save space. a note on this is provided above.
        // Grep on BYTE-ORDER MANIPULATION for more info.

        unsigned char key[MAX_CPG_CHAR_SIZE] = { 0 };

        register unsigned size = fCharSize[**src];

        if (size == 0) return INVALID_STRING;
        
        register unsigned i = m - size;

        while (i < m) key[i++] = *((*src)++);

        register unsigned len = encodeUTF8(pC2UData->get(key), trg);

        trg += len;
        (*srclen) -= size;
        (*trglen) += len;
        convlen -= size;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::fromEBCDIC(const unsigned char **src,
                                       unsigned int *srclen,
                                       unsigned char *trg,
                                       unsigned int *trglen)
{
    // XXX: Note yet implemented!!!
    return NOT_IMPLEMENTED;
}

unsigned int STAFConverter::toLATIN1(const unsigned char **src,
                                     unsigned int *srclen,
                                     unsigned char *trg,
                                     unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);

    *trglen = 0;

    unsigned char key[MAX_UNI_CHAR_SIZE] = { 0 };

    while (convlen > 0)
    {
        register unsigned len = SIZE_TABLE[**src];

        if (len == 0) return INVALID_STRING;

        decodeUTF8(*src, key);

        // If the first character is 0, assign the second character.
        // If the first character is not 0, assign ? since only support 1 char
        *(trg++)= (key[0] == 0) ? key[1] : 0x3F;  // 0x3F = Question mark
        (*src) += len;
        (*srclen) -= len;
        (*trglen)++;
        convlen -= len;
    }

    return 0;
}

unsigned int STAFConverter::toUTF8(const unsigned char **src,
                                   unsigned int *srclen,
                                   unsigned char *trg,
                                   unsigned int *trglen)
{
    // Note: we simply call fromUTF8, it's the same algorithm!!!
    return fromUTF8(src, srclen, trg, trglen);
}

unsigned int STAFConverter::toSBCS(const unsigned char **src,
                                   unsigned int *srclen,
                                   unsigned char *trg,
                                   unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;

    *trglen = 0;

    unsigned char key[MAX_UNI_CHAR_SIZE] = { 0 };

    while (convlen > 0 && (roomLeftInBuffer > (MAX_CPG_CHAR_SIZE - 1)))
    {
        register unsigned len = SIZE_TABLE[**src];

        if (len == 0) return INVALID_STRING;

        decodeUTF8(*src, key);

        register const unsigned char *cpgChar = pU2CData->get(key);
        
        *(trg++) = *(cpgChar);

        (*src) += len;
        (*srclen) -= len;
        (*trglen)++;
        convlen -= len;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::toDBCS(const unsigned char **src,
                                   unsigned int *srclen,
                                   unsigned char *trg,
                                   unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;

    *trglen = 0;

    unsigned char key[MAX_UNI_CHAR_SIZE] = { 0 };

    while (convlen > 0 && (roomLeftInBuffer > (MAX_CPG_CHAR_SIZE - 1)))
    {
        register unsigned len = SIZE_TABLE[**src];

        if (len == 0) return INVALID_STRING;

        decodeUTF8(*src, key);

        register const unsigned char *cpgChar = pU2CData->get(key);
        
        *(trg++) = *(cpgChar++);
        *(trg++) = *(cpgChar);

        (*src) += len;
        (*srclen) -= len;
        (*trglen) += 2;
        convlen -= len;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::toMBCS(const unsigned char **src,
                                   unsigned int *srclen,
                                   unsigned char *trg,
                                   unsigned int *trglen)
{
    int convlen = STAF_MIN(*srclen, *trglen);
    int roomLeftInBuffer = *trglen;
    
    *trglen = 0;

    unsigned char key[MAX_UNI_CHAR_SIZE] = { 0 };

    while (convlen > 0 && (roomLeftInBuffer > (MAX_CPG_CHAR_SIZE - 1)))
    {
        register unsigned len = SIZE_TABLE[**src];
        
        decodeUTF8(*src, key);

        register const unsigned char *cpgChar = pU2CData->get(key);
        register unsigned size = fCharSize[cpgChar[0]];

        if (size == 0) return INVALID_STRING;
        
        memcpy(trg, cpgChar, size);

        trg += size;
        (*src) += len;
        (*srclen) -= len;
        (*trglen) += size;
        convlen -= len;
        roomLeftInBuffer -= len;
    }

    return 0;
}

unsigned int STAFConverter::toEBCDIC(const unsigned char **src,
                                     unsigned int *srclen,
                                     unsigned char *trg,
                                     unsigned int *trglen)
{
    // XXX: not yet implemented
    return NOT_IMPLEMENTED;
}

///////////////////////////////////////////////////////////////////////////////
