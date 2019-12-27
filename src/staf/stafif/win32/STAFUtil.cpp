/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAFUtil.h"
#include "STAFUtilWin32.h"
#include "STAFMutexSem.h"
#include "STAFTrace.h"
#include <time.h>

// Older versions of the MS Visual Studio C/C++ compiler don't define
// MEMORYSTATUSEX, so we'll define our own so we can call the
// GlobalMemoryStatusEx API to get the physical memory
typedef struct _MEMORY_STATUS_EX {
         DWORD dwLength;
         DWORD dwMemoryLoad;
         DWORDLONG ullTotalPhys;
         DWORDLONG ullAvailPhys;
         DWORDLONG ullTotalPageFile;
         DWORDLONG ullAvailPageFile;
         DWORDLONG ullTotalVirtual;
         DWORDLONG ullAvailVirtual;
         DWORDLONG ullAvailExtendedVirtual;
} lMEMORYSTATUSEX, *LPMEMORY_STATUS_EX;

typedef BOOL (WINAPI * GlobalMemoryStatusEx_Proc) (LPMEMORY_STATUS_EX);


char *STAFUtilGetCurrentProcessCodePage(char *codepage)
{
    if (codepage != 0)
        sprintf(codepage, "%s%d", "IBM-", GetOEMCP());

    return codepage;
}


unsigned int STAFUtilWin32GetWinType()
{
    static STAFMutexSem helperSem;
    static bool haveChecked = false;
    static unsigned int type;

    if (!haveChecked)
    {
        STAFMutexSemLock lock(helperSem);

        if (!haveChecked)
        {
            OSVERSIONINFO osVersionInfo = { sizeof(OSVERSIONINFO), 0 };

            if (!GetVersionEx(&osVersionInfo))
            {
                STAFTrace::trace(
                    kSTAFTraceError,
                    "GetVersionEx() failed in STAFUtilWin32GetWinType()");
                type = 0;
            }
            else if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            {
                if (osVersionInfo.dwMinorVersion == 0)
                    type = kSTAFWin95;
                else if (osVersionInfo.dwMinorVersion == 10)
                    type = kSTAFWin98;
                else if (osVersionInfo.dwMinorVersion == 90)
                    type = kSTAFWinMe;
                else
                    type = kSTAFWin9XUnknown;
            }
            else if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
            {
                if (osVersionInfo.dwMajorVersion == 4)
                    type = kSTAFWinNT;
                else if ((osVersionInfo.dwMajorVersion == 5) &&
                         (osVersionInfo.dwMinorVersion == 0))
                    type = kSTAFWin2K;
                else if ((osVersionInfo.dwMajorVersion == 5) &&
                         (osVersionInfo.dwMinorVersion == 1))
                    type = kSTAFWinXP;
                else if ((osVersionInfo.dwMajorVersion == 5) &&
                         (osVersionInfo.dwMinorVersion == 2))
                    type = kSTAFWin2K3;
                else if ((osVersionInfo.dwMajorVersion == 6) &&
                         (osVersionInfo.dwMinorVersion == 0))
                {
                    // Because the version numbers for Windows Server 2008 and
                    // Windows Vista are identical (6.0), we must also test if
                    // the productType is 1 (VER_NT_WORKSTATION).  If so, then
                    // it's Windows Vista, otherwise it's Windows Server 2008.

                    BYTE productType = 1;  // Default to Workstation type

                    // Call GetVersionEx using the OSVERSIONINFOEX structure
                    // to get the wProductType field.
                    
                    // Note:  Using the OSVERSIONINFOEX structure requires
                    // Windows 2000 or later which is why we had to call
                    // GetVersionEx earlier using the OSVERSIONINFO structure
                    // as it works with earlier Windows versions.

                    OSVERSIONINFOEX osVersionInfoEx =
                        { sizeof(OSVERSIONINFOEX), 0 };

                    if (GetVersionEx((OSVERSIONINFO *) &osVersionInfoEx))
                    {
                        // However, the wProductType field is not available
                        // in the OSVERSIONINFOEX structures until VC++ 7.0.
                        // In VC++ 6.0, the wReserved field is defined as:
                        //   WORD wReserved[2].
                        // In VC++ 7.0 and later versions, these last 4 bytes
                        // are defined as:
                        //   WORD wSuiteMask;
                        //   BYTE wProductType;
                        //   BYTE wReserved;
                        #ifdef _MSC_VER
                        #if _MSC_VER < 1300
                            // Compiling using VC++ 6.0 or earlier as
                            // 1300 is the version number for VC++ 7.0

                            // Get productType from the 1st byte in the 2nd
                            // word of the wReserved field
                            union
                            {
                                BYTE half[2];
                                WORD whole;
                            } data;

                            data.whole = osVersionInfoEx.wReserved[1];
                            productType = data.half[0];
                        #else
                            // Compiling using VC++ 7.0 or later
                            productType = osVersionInfoEx.wProductType;
                        #endif
                        #endif
                    }
                    
                    // VER_NT_WORKSTATION = 1
                    // However, it's not defined until VC++ 7.0 so can't use

                    if (productType == 1)
                        type = kSTAFWinVista;
                    else
                        type = kSTAFWinSrv2008;
                }
                else if ((osVersionInfo.dwMajorVersion == 6) &&
                         (osVersionInfo.dwMinorVersion == 1))
                {
                    // Because the version numbers for Windows Server 2008 R2
                    // and Windows 7 are identical (6.1), we must also test if
                    // the productType is 1 (VER_NT_WORKSTATION).  If so, then
                    // it's Windows 7, otherwise it's Windows Server 2008 R2.

                    BYTE productType = 1;  // Default to Workstation type

                    // Call GetVersionEx using the OSVERSIONINFOEX structure
                    // to get the wProductType field.
                    
                    // Note:  Using the OSVERSIONINFOEX structure requires
                    // Windows 2000 or later which is why we had to call
                    // GetVersionEx earlier using the OSVERSIONINFO structure
                    // as it works with earlier Windows versions.

                    OSVERSIONINFOEX osVersionInfoEx =
                        { sizeof(OSVERSIONINFOEX), 0 };

                    if (GetVersionEx((OSVERSIONINFO *) &osVersionInfoEx))
                    {
                        // However, the wProductType field is not available
                        // in the OSVERSIONINFOEX structures until VC++ 7.0.
                        // In VC++ 6.0, the wReserved field is defined as:
                        //   WORD wReserved[2].
                        // In VC++ 7.0 and later versions, these last 4 bytes
                        // are defined as:
                        //   WORD wSuiteMask;
                        //   BYTE wProductType;
                        //   BYTE wReserved;
                        #ifdef _MSC_VER
                        #if _MSC_VER < 1300
                            // Compiling using VC++ 6.0 or earlier as
                            // 1300 is the version number for VC++ 7.0

                            // Get productType from the 1st byte in the 2nd
                            // word of the wReserved field
                            union
                            {
                                BYTE half[2];
                                WORD whole;
                            } data;

                            data.whole = osVersionInfoEx.wReserved[1];
                            productType = data.half[0];
                        #else
                            // Compiling using VC++ 7.0 or later
                            productType = osVersionInfoEx.wProductType;
                        #endif
                        #endif
                    }

                    // VER_NT_WORKSTATION = 1
                    // However, it's not defined until VC++ 7.0 so can't use

                    if (productType == 1)
                        type = kSTAFWin7;
                    else
                        type = kSTAFWinSrv2008R2;
                }
                else if ((osVersionInfo.dwMajorVersion == 6) &&
                         (osVersionInfo.dwMinorVersion == 2))
                {
                    // Because the version numbers for Windows Server 2012
                    // and Windows 8 are identical (6.2), we must also test if
                    // the productType is 1 (VER_NT_WORKSTATION).  If so, then
                    // it's Windows 8, otherwise it's Windows Server 2012.

                    BYTE productType = 1;  // Default to Workstation type

                    // Call GetVersionEx using the OSVERSIONINFOEX structure
                    // to get the wProductType field.
                    
                    // Note:  Using the OSVERSIONINFOEX structure requires
                    // Windows 2000 or later which is why we had to call
                    // GetVersionEx earlier using the OSVERSIONINFO structure
                    // as it works with earlier Windows versions.

                    OSVERSIONINFOEX osVersionInfoEx =
                        { sizeof(OSVERSIONINFOEX), 0 };

                    if (GetVersionEx((OSVERSIONINFO *) &osVersionInfoEx))
                    {
                        // However, the wProductType field is not available
                        // in the OSVERSIONINFOEX structures until VC++ 7.0.
                        // In VC++ 6.0, the wReserved field is defined as:
                        //   WORD wReserved[2].
                        // In VC++ 7.0 and later versions, these last 4 bytes
                        // are defined as:
                        //   WORD wSuiteMask;
                        //   BYTE wProductType;
                        //   BYTE wReserved;
                        #ifdef _MSC_VER
                        #if _MSC_VER < 1300
                            // Compiling using VC++ 6.0 or earlier as
                            // 1300 is the version number for VC++ 7.0

                            // Get productType from the 1st byte in the 2nd
                            // word of the wReserved field
                            union
                            {
                                BYTE half[2];
                                WORD whole;
                            } data;

                            data.whole = osVersionInfoEx.wReserved[1];
                            productType = data.half[0];
                        #else
                            // Compiling using VC++ 7.0 or later
                            productType = osVersionInfoEx.wProductType;
                        #endif
                        #endif
                    }

                    // VER_NT_WORKSTATION = 1
                    // However, it's not defined until VC++ 7.0 so can't use

                    if (productType == 1)
                        type = kSTAFWin8;
                    else
                        type = kSTAFWinSrv2012;
                }
                else
                    type = kSTAFWinNTUnknown;
            }
            else
            {
                type = kSTAFWin32Unknown;
            }

            haveChecked = true;
        }
    }

    return type;
}


unsigned int STAFUtilGetConfigInfo(STAFConfigInfo *configInfo,
                                   STAFString_t *errorBuffer,
                                   unsigned int *osRC)
{
    static STAFMutexSem helperSem;
    static bool haveChecked = false;

    static STAFUInt64_t physicalMemory = 0;
    static STAFString bootDrive;
    static STAFString exePath;
    static STAFString osMajorVersion;
    static STAFString osMinorVersion;
    static STAFString osRevision;
    static STAFString osName;
    static STAFString crlfString;
    static STAFString fileSeparator;
    static STAFString pathSeparator;
    static STAFString commandSeparator;
    static unsigned int numProcessors = 0;
    static STAFString currentDrive;

    if (!haveChecked)
    {
        STAFMutexSemLock lock(helperSem);

        if (!haveChecked)
        {
            // Get boot drive

            char systemPathBuffer[MAX_PATH];
            
            if (GetSystemDirectory(systemPathBuffer, MAX_PATH) == 0)
            {
                if (errorBuffer)
                    *errorBuffer = STAFString(
                        "GetSystemDirectory()").adoptImpl();
                
                if (osRC) *osRC = static_cast<unsigned int>(GetLastError());
                
                return kSTAFBaseOSError;
            }

            bootDrive = STAFString(systemPathBuffer).subString(0, 2);

            // Get STAFRoot

            char buffer[256] = { 0 };
            char *filePart = 0;

            if (SearchPath(0, "STAFPROC.EXE", 0, sizeof(buffer), buffer,
                &filePart) == 0)
            {
                if (errorBuffer)
                    *errorBuffer = STAFString("SearchPath()").adoptImpl();

                if (osRC) *osRC = static_cast<unsigned int>(GetLastError());

                return kSTAFBaseOSError;
            }

            exePath = STAFString(buffer).subWord(0);

            exePath = exePath.subString(0, exePath.findLastOf(kUTF8_BSLASH));
            exePath = exePath.subString(0, exePath.findLastOf(kUTF8_BSLASH));

            // Get OS Name, Major and Minor Version, and Revision

            OSVERSIONINFO osVersionInfo = { sizeof(OSVERSIONINFO), 0 };

            if (GetVersionEx(&osVersionInfo) != TRUE)
            {
                if (errorBuffer)
                    *errorBuffer = STAFString("GetVersionEx()").adoptImpl();

                if (osRC) *osRC = 1;

                return kSTAFBaseOSError;
            }

            osMajorVersion = STAFString(osVersionInfo.dwMajorVersion);
            osMinorVersion = STAFString(osVersionInfo.dwMinorVersion);
            osRevision = STAFString(osVersionInfo.dwBuildNumber & 0x0000FFFF);
            
            switch (STAFUtilWin32GetWinType())
            {
                case kSTAFWin95:        osName = "Win95"; break;
                case kSTAFWin98:        osName = "Win98"; break;
                case kSTAFWinMe:        osName = "WinMe"; break;
                case kSTAFWin9XUnknown: osName = "Unknown Win9X"; break;
                case kSTAFWinNT:        osName = "WinNT"; break;
                case kSTAFWin2K:        osName = "Win2000"; break;
                case kSTAFWinXP:        osName = "WinXP"; break;
                case kSTAFWin2K3:       osName = "Win2003"; break;
                case kSTAFWinVista:     osName = "WinVista"; break;
                case kSTAFWinSrv2008:   osName = "WinSrv2008"; break;
                case kSTAFWin7:         osName = "Win7"; break;
                case kSTAFWinSrv2008R2: osName = "WinSrv2008R2"; break;
                case kSTAFWin8:         osName = "Win8"; break;
                case kSTAFWinSrv2012:   osName = "WinSrv2012"; break;
                case kSTAFWinNTUnknown: osName = "Unknown WinNT"; break;
                case kSTAFWin32Unknown: osName = "Unknown Win32"; break;
            }

            crlfString = STAFString(kUTF8_CR) + STAFString(kUTF8_LF);
            fileSeparator  = STAFString(kUTF8_BSLASH);
            pathSeparator  = STAFString(kUTF8_SCOLON);
            commandSeparator = STAFString(kUTF8_AMP);

            // Get the number of available processors from the
            // NUMBER_OF_PROCESSORS environment variable

            char *numProcessorsStr = getenv("NUMBER_OF_PROCESSORS");

            if (numProcessorsStr != NULL)
                numProcessors = atoi(numProcessorsStr);
            
            // Get total physical memory

            if (STAFUtilWin32GetWinType() & kSTAFWin2KPlus)
            {
                // Try to use GlobalMemoryStatusEx as it supports memory >=
                // 2GB, where GlobalMemoryStatus does not

                HMODULE h;

                if (h = GetModuleHandle("kernel32.dll"))
                {
                    GlobalMemoryStatusEx_Proc s_pfn_GlobalMemoryStatusEx =
                        (GlobalMemoryStatusEx_Proc)GetProcAddress(
                            h, "GlobalMemoryStatusEx");
                
                    if (s_pfn_GlobalMemoryStatusEx != NULL)
                    {
                        lMEMORYSTATUSEX memoryStatusEx;
                        memoryStatusEx.dwLength = sizeof memoryStatusEx;
                    
                        if (s_pfn_GlobalMemoryStatusEx(&memoryStatusEx))
                        {
                            physicalMemory = memoryStatusEx.ullTotalPhys;
                        }
                    }
                }
            }

            if (physicalMemory == 0)
            {
                // Fall back to GlobalMemoryStatus which is always available
                // but returns wrong results for physical memory >= 2GB
                
                MEMORYSTATUS memoryStatus = { sizeof(MEMORYSTATUS), 0 };
                GlobalMemoryStatus(&memoryStatus);
                physicalMemory = memoryStatus.dwTotalPhys;
            }
            
            // Get the drive (e.g. C:) of the current directory (the directory
            // from which STAFProc was started

            char myBuffer[MAX_PATH] = { 0 };

            if (GetCurrentDirectory(sizeof(myBuffer), myBuffer) > 0)
            {
                STAFString currentDir = STAFString(myBuffer);

                if ((currentDir.length() > 1) &&
                    (currentDir.subString(1, 1) == STAFString(kUTF8_COLON)))
                {
                    currentDrive = currentDir.subString(0, 2);
                }
            }

            haveChecked = true;
        }
    }

    configInfo->physicalMemory = physicalMemory;
    configInfo->numProcessors = numProcessors;
    configInfo->lineSeparator  = crlfString.getImpl();
    configInfo->fileSeparator  = fileSeparator.getImpl();
    configInfo->pathSeparator  = pathSeparator.getImpl();
    configInfo->commandSeparator = commandSeparator.getImpl();
    configInfo->osName         = osName.getImpl();
    configInfo->osMajorVersion = osMajorVersion.getImpl();
    configInfo->osMinorVersion = osMinorVersion.getImpl();
    configInfo->osRevision     = osRevision.getImpl();
    configInfo->exePath        = exePath.getImpl();
    configInfo->bootDrive      = bootDrive.getImpl();
    configInfo->defaultProcessStopMethod = kSTAFProcessStopWithSigKillAll;
    configInfo->defaultProcessConsoleMode = kSTAFProcessNewConsole;

    // The maximum value for a PID must not exceed the maximum value for
    // the STAFProcessID_t type which on Windows is unsigned int
    configInfo->maxPid = UINT_MAX;

    configInfo->envVarCaseSensitive = 0;  // Case insensitive
    configInfo->currentDrive = currentDrive.getImpl();

    return 0;
}


void *STAFUtilGetSystemMemory(unsigned long size, unsigned int *osRC)
{
    void *theMem = GlobalAlloc(GMEM_FIXED, size);

    if ((theMem == 0) && osRC) *osRC = 0;

    return theMem;
}


void STAFUtilFreeSystemMemory(void *ptr)
{
    GlobalFree(ptr);
}


unsigned int STAFUtilIsValidSocket(STAFSocket_t theSocket)
{
    return (theSocket != INVALID_SOCKET);
}


unsigned int STAFUtilGetNonInheritableSocket(STAFSocket_t oldSocket,
                                             STAFSocket_t *newSocket,
                                             unsigned int *osRC)
{
    HANDLE newHandle;

    int dupHandleRC = DuplicateHandle(
        GetCurrentProcess(), (HANDLE)oldSocket, 
        GetCurrentProcess(), &newHandle, 0,
        FALSE, // The new handle cannot be inherited
        DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS);

    if (dupHandleRC)
    {
        *newSocket = (SOCKET)newHandle;
        return kSTAFOk;  // To indicate success
    }
    else
    {
        // DuplicateHandle failed
        *osRC = static_cast<unsigned int>(GetLastError());
        *newSocket = oldSocket;
        return 1;        // To indicate failure
    }
}


unsigned int STAFUtilWin32CreateNullSD(PSECURITY_DESCRIPTOR *sd)
{
    // If on WinNT or above, set up security to use on a file mapping to
    // allow access to everyone.

    if (STAFUtilWin32GetWinType() & kSTAFWinNTPlus)
    {
        // Allocate the Security Descriptor (SD)
        if (!(*sd = static_cast<PSECURITY_DESCRIPTOR>(
                    new char[SECURITY_DESCRIPTOR_MIN_LENGTH])))
            return 1;

        if (!InitializeSecurityDescriptor(*sd, SECURITY_DESCRIPTOR_REVISION))
            return 2;

        // Set NULL DACL on the Security Descriptor
        if (!SetSecurityDescriptorDacl(*sd, TRUE, (PACL)NULL, FALSE))
            return 3;
    }

    return 0;
}


unsigned int STAFUtilWin32DeleteNullSD(PSECURITY_DESCRIPTOR *sd)
{
    // If on WinNT or above, delete the Security Descriptor

    if (STAFUtilWin32GetWinType() & kSTAFWinNTPlus)
    {
        if (*sd != 0)
        {
            delete [] static_cast<char *>(*sd);
            *sd = 0;
        }
    }

    return 0;
}


// The Win32 API does not contain a functional equivalent of the standard C
// mkstemp() function.  The Microsoft C runtime implementation does not even
// provide support for the function, although it does provide an
// implementation of mktemp().  However, it is strongly advised not to use
// the mktemp() function on either Unix or Windows.
//
// The Win32 API does provide a function, GetTempFileName(), that will
// generate a temporary filename, but that is all that it does; it does not
// open the file for you.  So, we are not using this function.
// Instead of using GetTempPath() to obtain the current user's setting for the
// location to place temporary files, the location to store temporary files is 
// being passed in (and should be *gSTAFWriteLocationPtr/tmp), the STAF data
// directory).
// 
// The STAFUtilCreateTempFile() method generates our own random filename, 
// using the system time and request number to seed the rand() function.
// The format of the unique filename is:  
//   <tempDir>\STAFTemp??????[.<suffix>]
// where <tempDir> contains the name of the directory to store the temp file
//       ??????    represents 6 randomly generated characters.
// Also, if a suffix is specified, e.g. "out", it is added as the file
// extension.
// We attempt to create the file.  If not successful, we repeat the process,
// generating another random temporary filename and attempting to create the
// file until the file is successfully created or some kind of fatal error
// occurs.

STAFRC_t STAFUtilCreateTempFile(STAFStringConst_t tempDir,
                                STAFStringConst_t tempFileSuffix,
                                unsigned int requestNumber,
                                STAFString_t *tempFileName,
                                STAFString_t *errorBuffer,
                                unsigned int *osRC)
{
    static char fileNameChars[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVYXWZ";
    const unsigned int BUFSIZE = 4096;    // Maximum buffer size
    char tmpPathBuffer[BUFSIZE] = { 0 };  // Buffer for temp path name
    char tmpFileBuffer[MAX_PATH] = { 0 }; // Buffer for temp file name
    char randomBuffer[7] = { 0 };         // Buffer for random values
    unsigned int characterRange = lstrlen(fileNameChars) - 1;
    char cCharacter;
    unsigned int i;
    HANDLE hFile;    // Temporary file's handle

    // Create a unique temporary file name with format:
    //   <tempDir>\STAFTemp??????[.<suffix>]
    // where <tempDir> contains the name of the directory to store the
    // temporary file and ?????? is a randomly assigned unique value.
    // Also, if a suffix is specified, e.g. out, it is added as the file
    // extension.

    STAFString tempNamePrefix = STAFString(tempDir) + STAFString(kUTF8_BSLASH) +
        "STAFTemp";
    
    STAFString suffix = STAFString(tempFileSuffix);

    // Note:  Using the only the time function is not enough to set the
    // random seed because on Windows, it would set the same seed for all
    // temporary files created within the same second.  We also use the
    // request number since it is unique per request on each machine.
    srand((unsigned)(time(NULL) * requestNumber));

    // Repeat the process of generating a random temporary file name and
    // attempting to create the file, until the file is successfully created
    // or some kind of fatal error occurs.
    do
    {
        // Add 6 randomly assigned values to the temporary file name

        for (i = 0; i < 6; i++)
        {
            cCharacter = fileNameChars[rand() % characterRange];
            randomBuffer[i] = cCharacter;
        }

        randomBuffer[i] = 0;
        
        STAFString tempNameString = tempNamePrefix + STAFString(randomBuffer);
        
        // If a suffix was specified, add it to the temporary file name

        if (suffix.length() > 0)
        {
            tempNameString = tempNameString + STAFString(".") + suffix;
        }

        // Attempt to create the file to make sure doesn't already exist

        strncpy(tmpPathBuffer, tempNameString.toCurrentCodePage()->buffer(),
                sizeof(tmpPathBuffer));
        
        hFile = CreateFile(tempNameString.toCurrentCodePage()->buffer(),
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           0, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, 0);

        if ((hFile == INVALID_HANDLE_VALUE) &&
            (GetLastError() != ERROR_FILE_EXISTS))
        {
            // Fatal error - Don't retry

            *errorBuffer = STAFString("Creating temp file failed").adoptImpl();
            *osRC = GetLastError();

            return kSTAFBaseOSError;
        }
    } while (hFile == INVALID_HANDLE_VALUE);

    // Temporary file was successfully created and opened.  Close it.
    CloseHandle(hFile);

    *tempFileName = STAFString(tmpPathBuffer).adoptImpl();

    return kSTAFOk;
}


/*******************************************************************************/
/* STAFUtilWin32LookupSystemErrorMessage - This function looks up the error    */
/* message for the specified Windows system error code.                        */
/*                                                                             */
/* Accepts: (IN)  System error code                                            */
/*          (OUT) Error message                                                */
/*                                                                             */
/* Returns: Nothing                                                            */
/*******************************************************************************/

void STAFUtilWin32LookupSystemErrorMessage(unsigned int errorCode,
                                           STAFString_t *errorMsg) 
{
    LPVOID lpMsgBuffer;
    
    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuffer, 0, NULL))
    {
        if (errorMsg)
        {
            *errorMsg = (STAFString(
                reinterpret_cast<char *>(lpMsgBuffer))).adoptImpl();
        }
    }

    LocalFree(lpMsgBuffer);
}

