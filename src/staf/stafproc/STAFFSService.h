/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_FileSystemService
#define STAF_FileSystemService

#include "STAFService.h"
#include "STAFCommandParser.h"

// STAFFileSystemService - Handles manipulating file system objects


typedef enum STAFFSCopy_e
{
    kSTAFFSContinueCopy = 0,
    kSTAFFSStopCopy = 1,
    kSTAFFSFinishedCopy = 0xffffffff
} STAFFSCopy_t;

typedef enum STAFFSTransferType_e
{
    kSTAFFSBinary = 0,         // Binary transfer
    kSTAFFSTextNoConvert = 1,  // Text transfer, no codepage conversion
    kSTAFFSTextConvert = 2     // Text transfer, codepage conversion
} STAFFSTransferType_t;

class STAFFSService : public STAFService
{
public:

    STAFFSService();

    virtual STAFServiceResult acceptRequest(
                              const STAFServiceRequest &requestInfo);

    virtual STAFString info(unsigned int raw = 0) const;

    virtual ~STAFFSService();

private:

    // Don't allow copy construction or assignment
    STAFFSService(const STAFFSService &);
    STAFFSService &operator=(const STAFFSService &);

    STAFServiceResult handleCopyFile(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleCopyDir(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleMove(const bool fileSpecified,
                                 const STAFServiceRequest &requestInfo);
    STAFServiceResult handleGet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleList(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleListCopyRequests(
        const STAFServiceRequest &requestInfo);
    STAFServiceResult handleCreate(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleDelete(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleQuery(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleSet(const STAFServiceRequest &requestInfo);
    STAFServiceResult handleHelp(const STAFServiceRequest &requestInfo);

    STAFCommandParser fCopyFileParser;
    STAFCommandParser fCopyDirParser;
    STAFCommandParser fMoveFileParser;
    STAFCommandParser fMoveDirParser;
    STAFCommandParser fGetParser;
    STAFCommandParser fListParser;
    STAFCommandParser fListCopyRequestsParser;
    STAFCommandParser fCreateParser;
    STAFCommandParser fDeleteParser;
    STAFCommandParser fQueryParser;
    STAFCommandParser fSetParser;

    STAFMapClassDefinitionPtr fErrorInfoClass;
    STAFMapClassDefinitionPtr fCopyRequestClass;
    STAFMapClassDefinitionPtr fCopyFileClass;
    STAFMapClassDefinitionPtr fFileCopyStateClass;
    STAFMapClassDefinitionPtr fCopyDirectoryClass;
    STAFMapClassDefinitionPtr fDirectoryCopyStateClass;
    STAFMapClassDefinitionPtr fSettingsClass;
    STAFMapClassDefinitionPtr fQueryInfoClass;
    STAFMapClassDefinitionPtr fEntrySizeInfoClass;
};

#endif
