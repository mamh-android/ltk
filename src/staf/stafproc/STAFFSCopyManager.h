/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_FSCopyManager
#define STAF_FSCopyManager

#include <map>
#include "STAF.h"
#include "STAFString.h"
#include "STAFUtil.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFTimestamp.h"


typedef enum STAFFSCopyType_e
{
    kSTAFFSFileCopy = 1,       // File copy
    kSTAFFSDirectoryCopy = 2   // Directory copy
} STAFFSCopyType_t;

typedef enum STAFFSCopyDirection_e
{
    kSTAFFSCopyFrom = 0,
    kSTAFFSCopyTo   = 1
} STAFFSCopyDirection_t;

class STAFFSCopyManager
{
public:

    // Public types

    struct FSCopyData
    {
        FSCopyData() { /* Do Nothing */ }
        
        FSCopyData(unsigned int aMapID, STAFString aName, STAFString aMachine,
                   unsigned int aDirection, unsigned int aType,
                   unsigned int aMode, unsigned int aFileSize) :
            mapID(aMapID),
            name(aName),
            machine(aMachine),
            direction(aDirection),
            type(aType),
            mode(aMode),
            fileSize(aFileSize),
            bytesCopied(0),
            entryName("")
        {
            startTimestamp = STAFTimestamp::now().asString();
        }

        unsigned int mapID;
        STAFString startTimestamp;
        STAFString name;
        STAFString machine;
        unsigned int direction;   // 0=From, 1=To
        unsigned int type;        // 1=File, 2=Directory
        unsigned int mode;        // 0=Binary File
                                  // 1=Text File, no codepage conversion
                                  // 2=Text File, codepage conversion
        unsigned int fileSize;    // Total size of file in bytes 
        unsigned int bytesCopied; // Number of bytes copied so far
        STAFString entryName;     // Name of entry in directory being copied
    };

    typedef STAFRefPtr<FSCopyData> FSCopyDataPtr;
    typedef std::map<unsigned int, FSCopyDataPtr> FSCopyMap;

    STAFFSCopyManager(unsigned int mapID = 0) :
        fMapID(mapID)
    { /* Do Nothing */ }
        
    STAFRC_t add(
        const STAFString &name, const STAFString &machine,
        unsigned int direction, unsigned int type, unsigned int mode,
        unsigned int fileSize, FSCopyDataPtr &copyDataPtr);
    
    STAFRC_t updateFileCopy1(FSCopyDataPtr &copyDataPtr, unsigned int fileSize,
                             unsigned int mode);

    STAFRC_t updateFileCopy(FSCopyDataPtr &copyDataPtr, unsigned int bytesCopied);

    STAFRC_t updateDirectoryCopy(
        FSCopyDataPtr &copyDataPtr, const STAFString &name, unsigned int mode,
        unsigned int fileSize);

    STAFRC_t remove(FSCopyDataPtr &copyDataPtr);

    unsigned int getMapID();
    
    FSCopyMap getFSCopyMapCopy();

private:

    // Don't allow copy construction or assignment
    STAFFSCopyManager(const STAFFSCopyManager &);
    STAFFSCopyManager &operator=(const STAFFSCopyManager &);

    unsigned int fMapID;

    STAFMutexSem fFSCopyMapSem;
    FSCopyMap fFSCopyMap;
};


#endif
