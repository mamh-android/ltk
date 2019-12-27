/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFUtil.h"

char *STAFUtilGetCurrentProcessCodePage(char *codepage)
{
    unsigned long codePageList[8] = { 0 };
    unsigned long listSize = 0;
    APIRET rc = DosQueryCp(sizeof(codePageList), codePageList, &listSize);


    if (codepage != 0)
        sprintf(codepage, "%s%d", "IBM-", codePageList[0]);

    return codepage;
}


unsigned int STAFUtilGetConfigInfo(STAFConfigInfo *configInfo,
                                   STAFString_t *errorBuffer,
                                   unsigned int *osRC)
{
    unsigned long sysInfo[QSV_MAX] = { 0 };
    APIRET rc = DosQuerySysInfo(1, QSV_MAX, (void *)sysInfo,
                                sizeof(unsigned long) * QSV_MAX);

    if (rc)
    {
        if (errorBuffer)
            *errorBuffer = STAFString("DosQuerySysInfo()").adoptImpl();

        if (osRC) *osRC = static_cast<unsigned int>(rc;

        return kSTAFBaseOSError;
    }


    PIB *stafPIB;
    TIB *stafTIB;

    rc = DosGetInfoBlocks(&stafTIB, &stafPIB);

    if (rc)
    {
        if (errorBuffer)
            *errorBuffer = STAFString("DosGetInfoBlocks()").adoptImpl();

        if (osRC) *osRC = static_cast<unsigned int>(rc);

        return kSTAFBaseOSError;
    }

    char stafPath[256];

    rc = DosQueryModuleName(stafPIB->pib_hmte, sizeof(stafPath), stafPath);

    if (rc)
    {
        if (errorBuffer)
            *errorBuffer = STAFString("DosQueryModuleName()").adoptImpl();

        if (osRC) *osRC = static_cast<unsigned int>(rc);

        return kSTAFBaseOSError;
    }

    STAFString stafPathString(stafPath);

    // Get rid of \BIN\STAFPROC.EXE
    // This, obviously, assumes they have installed STAF as per the
    // instructions

    stafPathString = stafPathString.subString(0,
                     stafPathString.findLastOf(kUTF8_BSLASH));
    stafPathString = stafPathString.subString(0,
                     stafPathString.findLastOf(kUTF8_BSLASH));

    STAFString bootDrive(static_cast<char>((sysInfo[QSV_BOOT_DRIVE - 1] - 1 +
                         static_cast<unsigned long>('A'))));

    bootDrive += kUTF8_COLON;

    configInfo->bootDrive = bootDrive.adoptImpl();
    configInfo->osName = STAFString("OS2").adoptImplt();
    configInfo->osMajorVersion =
        STAFString(sysInfo[QSV_VERSION_MAJOR - 1]).adoptImpl();
    configInfo->osMinorVersion =
        STAFString(sysInfo[QSV_VERSION_MINOR - 1]).adoptImpl();
    configInfo->osRevision =
        STAFString(sysInfo[QSV_VERSION_REVISION - 1]).adoptImpl();
    configInfo->physicalMemory = sysInfo[QSV_TOTPHYSMEM - 1];

    configInfo->exePath = stafPathString.adoptImpl();

    STAFString crlfString = STAFString(kUTF8_CR) + STAFString(kUTF8_LF);

    configInfo.lineSeparator = crlfString.adoptImpl();
    configInfo.fileSeparator = STAFString(kUTF8_BSLASH).adoptImpl();
    configInfo.pathSeparator = STAFString(kUTF8_SCOLON).adoptImpl();
    configInfo.commandSeparator = STAFString(kUTF8_SCOLON).adoptImpl();

    // This is spec'd in the Warp V3 Control Program Refrence, but
    // doesn't seem to be defined in the os2 headers
    //
    // gGlobalVariablePool->set("NumberOfProcessors",
    //                          IString(sysInfo[QSV_NUMPROCESSORS]));


    return kSTAFOk;
}


void *STAFUtilGetSystemMemory(unsigned long size, unsigned int *osRC)
{
    void *theMem = 0;
    APIRET rc = DosAllocMem(&theMem, size, PAG_COMMIT | PAG_READ | PAG_WRITE);

    if (rc != 0)
    {
        if (osRC) *osRC = static_cast<unsigned int>(rc);

        return 0;
    }

    return theMem;
}


void STAFUtilFreeSystemMemory(void *ptr)
{
    DosFreeMem(ptr);
}


unsigned int STAFUtilIsValidSocket(STAFSocket_t theSocket)
{
    return !(theSocket < 0);
}
