/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <pthread.h>
#include <langinfo.h>
#include <ctype.h>
#include <cstdio>
#include <stdlib.h>
#include <sys/utsname.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// Linux/Solaris libraries
#if HAVE_UNISTD_H
#include <cstdlib>
#endif

// FreeBSD and OSX libraries
#if defined(STAF_OS_NAME_FREEBSD) || defined(STAF_OS_NAME_MACOSX)
#include <sys/sysctl.h>
#endif

// AIX 5.x libraries
#if defined(STAF_OS_NAME_AIX) || HAVE_SYS_SYSTEMCFG_H
#include <sys/systemcfg.h>
#endif

// HP-UX libraries
#if defined(STAF_OS_NAME_HPUX)
#include <sys/pstat.h>
#endif

// XXX: Why is STAF.h after the other headers
#include "STAF.h"
#include "STAF_fstream.h"
#include "STAFString.h"
#include "STAFUtil.h"
#include "STAFMutexSem.h"

STAFRC_t STAFUtilUnixGetFilePath (STAFStringConst_t theFile,
                                  STAFString_t *thePathname,
                                  unsigned int *osRC)
{
    if (theFile == 0) return kSTAFInvalidValue;

    if (thePathname == 0) return kSTAFInvalidValue;

    STAFString file(theFile);
    STAFString pathname;

    unsigned int start = 0;
    unsigned int index = 0;
    STAFString envpath = getenv("PATH");

    // append a ":" at the end if none exists (for algorithm correctness)

    if (envpath.findLastOf(kUTF8_COLON, STAFString::kChar) !=
        envpath.length(STAFString::kChar) - 1)
    {
        envpath += kUTF8_COLON;
    }

    // loop until either the path of file is found or no path is found
    for (;;)
    {
        index = envpath.find(kUTF8_COLON, start);
        pathname = envpath.subString(start, index - start);

        // append a "/" at then end if none exists
        if (pathname.findLastOf(kUTF8_SLASH, STAFString::kChar) !=
            pathname.length(STAFString::kChar) - 1)
        {
            pathname += kUTF8_SLASH;
        }

        // append the filename
        pathname += file;

        ifstream testFile(pathname.toCurrentCodePage()->buffer());

        if (testFile)
        {
            testFile.close();

            // extract the "/filename" part so that we return only
            // the path without an ending "/"
            *thePathname = pathname.subString(0, pathname.findLastOf(
                                              kUTF8_SLASH)).adoptImpl();
            return kSTAFOk;
        }
        else
        {
            start = index + envpath.sizeOfChar(index);
            if (start >= envpath.length()) return kSTAFDoesNotExist;
        }
    }
}


unsigned int STAFUtilGetConfigInfo(STAFConfigInfo *configInfo,
                                   STAFString_t *errorBuffer,
                                   unsigned int *osRC)
{
    static STAFMutexSem helperSem;
    static bool haveChecked = false;

    static STAFUInt64_t physicalMemory = 0;
    static unsigned int numProcessors = 0;
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
    static STAFString currentDrive;

    if (!haveChecked)
    {
        STAFMutexSemLock lock(helperSem);

        if (!haveChecked)
        {
            STAFString exeName = "STAFProc";
            STAFString_t relPathImpl = 0;
            STAFRC_t rc = 0;

            // get path of exeName as specified in the PATH environment variable

            rc = STAFUtilUnixGetFilePath(exeName.getImpl(), &relPathImpl, osRC);

            if (rc != kSTAFOk)
            {
                if (errorBuffer) *errorBuffer = STAFString(
                    "STAF bin directory not found in PATH").adoptImpl();
                return rc;
            }

            STAFString relPath(relPathImpl, STAFString::kShallow);
            char cwdBuf[256];

            // if relPath is the CWD then get its name
            if (relPath == ".")
                exePath = getcwd(cwdBuf, sizeof(cwdBuf));
            else
                exePath = relPath;

            // truncate "/bin" out of the returned path
            exePath = exePath.subString(0, exePath.findLastOf(kUTF8_SLASH));

            struct utsname name;

            int unameRC = uname(&name);

            #if defined(STAF_OS_NAME_HPUX)
            if ((unameRC < 0) && (errno == EOVERFLOW))
            {
                // An EOVERFLOW error can occur using HP-UX i11 v3 or later
                // if expanded_node_host_name=1 and uname_eoverflow=1 and if
                // the node name length >= 8 because the nodename field in the
                // utsname structure is defined with length 9 (which includes
                // the null terminated character so it really contains up to
                // 8 characters.
                //
                // If we ever change to compile using compiler option:
                //    -D_HPUX_API_LEVEL=20040821
                // the nodename field length would be defined as 257 and we
                // can remove this code that checks for EOVERFLOW.
                // See Feature #2974748 for more information.

                // If name.nodename length >= 8, assume this EOVERFLOW error
                // was caused by name.nodename overflowing (and not by another
                // field overflowing) and ignore this error since we do not
                // use the name.nodename field.

                if (STAFString(name.nodename).length() >= 8)
                    unameRC = 0;  // Ignore this error
            }
            #endif
            
            if (unameRC < 0)
            {
                if (osRC) *osRC = errno;
                if (errorBuffer)
                    *errorBuffer = STAFString("uname()").adoptImpl();
                return kSTAFBaseOSError;
            }

            osName         = STAFString(name.sysname);
            osMajorVersion = STAFString(name.release);
            osMinorVersion = STAFString(name.version);
            osRevision     = STAFString(name.machine);
            
            bootDrive      = STAFString(kUTF8_SLASH);
            crlfString     = STAFString(kUTF8_LF);
            fileSeparator  = STAFString(kUTF8_SLASH);
            pathSeparator  = STAFString(kUTF8_COLON);
            commandSeparator = STAFString(kUTF8_SCOLON);
            currentDrive   = STAFString(kUTF8_SLASH);

            // For unix systems, each OS has a different way of fetching the
            // physical memory and number of processors.
            // Note: For z/OS and AIX Version 4 or lower, the memory and
            // number of processors will be 0 because we couldn't find APIs to
            // access this information for these operating systems.

            #if (defined(STAF_OS_NAME_LINUX) || defined(STAF_OS_NAME_SOLARIS))
            
                // Linux, zLinux, and Solaris 

                // Get physical memory
                STAFUInt64_t numPages = sysconf(_SC_PHYS_PAGES);
                unsigned long pageSize = sysconf(_SC_PAGESIZE);
                physicalMemory = numPages * pageSize;

                // Get number of available processors
                numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
                
            #elif defined(STAF_OS_NAME_AIX)
            
                if (osName == "AIX")
                {     
                    // AIX V5 or later

                    // Get physical memory
                    physicalMemory = _system_configuration.physmem;

                    // Get number of available processors
                    numProcessors = _system_configuration.ncpus;
                }
                else if (osName == "OS400")
                {
                    // OS400 (iSeries)
                    // Note: we compile the OS400 code through AIX.

                    // Get physical memory
                    STAFUInt64_t numPages = sysconf(_SC_PHYS_PAGES);
                    unsigned long pageSize = sysconf(_SC_PAGESIZE);
                    physicalMemory = numPages * pageSize;

                    // Get number of available processors
                    numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
                }

            #elif defined(STAF_OS_NAME_HPUX)
            
                // HP-UX

                // Get physical memory
                struct pst_static ps;
                pstat_getstatic(&ps, sizeof(ps), 1, 0);
                STAFUInt64_t numPages = ps.physical_memory;
                unsigned int pageSize = ps.page_size;
                physicalMemory = numPages * pageSize;

                // Get number of available processors
                struct pst_dynamic psd;
                if (pstat_getdynamic(&psd, sizeof(psd), 1, 0) != -1)
                    numProcessors = psd.psd_proc_cnt;
                
            #elif defined(STAF_OS_NAME_FREEBSD) || defined(STAF_OS_NAME_MACOSX)
                
                // FreeBSD and OSX

                // Get physical memory
                int mib[2];
                STAFUInt64_t totalMem;
                size_t len = sizeof(totalMem);
                mib[0] = CTL_HW;
                mib[1] = HW_PHYSMEM;
                sysctl(mib, 2, &totalMem, &len, NULL, 0);
                physicalMemory = totalMem;                        

                // Get number of available processors
                unsigned int ncpu;
                len = sizeof(ncpu);
                mib[1] = HW_NCPU;
                sysctl(mib, 2, &ncpu, &len, NULL, 0);
                numProcessors = ncpu;
                
            #endif
            
            haveChecked = true;
        }
    }
    
    configInfo->physicalMemory  = physicalMemory;
    configInfo->numProcessors   = numProcessors;
    configInfo->lineSeparator   = crlfString.getImpl();
    configInfo->fileSeparator   = fileSeparator.getImpl();
    configInfo->pathSeparator   = pathSeparator.getImpl();
    configInfo->commandSeparator = commandSeparator.getImpl();
    configInfo->osName          = osName.getImpl();
    configInfo->osMajorVersion  = osMajorVersion.getImpl();
    configInfo->osMinorVersion  = osMinorVersion.getImpl();
    configInfo->osRevision      = osRevision.getImpl();
    configInfo->exePath         = exePath.getImpl();
    configInfo->bootDrive       = bootDrive.getImpl();
    configInfo->defaultProcessStopMethod = kSTAFProcessStopWithSigKillAll;
    configInfo->defaultProcessConsoleMode = kSTAFProcessSameConsole;

    // The maximum value for a PID must not exceed the maximum value for
    // the STAFProcessID_t type which on Unix is int
    configInfo->maxPid = INT_MAX;

    configInfo->envVarCaseSensitive = 1;  // Case sensitive

    return 0;
}


char *STAFUtilGetCurrentProcessCodePage(char *codepage)
{
    static bool isLocaleSet = false;

    if (codepage != 0)
    {
        if (!isLocaleSet)
        {
            // Before nl_langinfo is called to get the codepage, must call
            // setlocale and pass locale "", so that LC_ALL is set according
            // to the environment variables.  Otherwise, if no locale has been
            // selected for the appropriate category, nl_langinfo  returns a
            // pointer to the corresponding string in the "C" locale.  The
            // locale "C"  or "POSIX" is a portable locale; its LC_CTYPE part
            // corresponds to the 7-bit ASCII character set (ANSI_X3.4-1968).

            setlocale(LC_ALL, "");

            isLocaleSet = true;
        }

        sprintf(codepage, "%s", nl_langinfo(CODESET));
    }

    return codepage;
}


void *STAFUtilGetSystemMemory(unsigned long size, unsigned int *osRC)
{
    void *ptr = malloc(size);

    // if no memory could be allocate set error
    if ((ptr == 0) && osRC)
        *osRC = errno;

    return ptr;
}


void STAFUtilFreeSystemMemory(void* ptr)
{
    free(ptr);
}


unsigned int STAFUtilIsValidSocket(STAFSocket_t theSocket)
{
    return !(theSocket < 0);
}


unsigned int STAFUtilGetNonInheritableSocket(STAFSocket_t oldSocket,
                                             STAFSocket_t *newSocket,
                                             unsigned int *osRC)
{
    *newSocket = oldSocket;
    
    // Set the close-on-exec (aka no-inherit) flag for the socket file
    // descriptor.  This call also ensures that the file descriptor is
    // closed when we fork() and exec().
    
    int flags = fcntl(*newSocket, F_GETFD);

    if (flags == -1)
    {
        *osRC = errno;
        return 1;    // To indicate failure 
    }    
    
    flags |= FD_CLOEXEC;

    if (fcntl(*newSocket, F_SETFD, flags) == -1)
    {
        *osRC = errno;
        return 1;    // To indicate failure
    }
    
    return kSTAFOk;  // To indicate success
}


#ifdef STAF_OS_NAME_HPUX

// The best solution for creating a temporary file on Unix is to use the
// mkstemp() function in the standard C runtime library.  However, on some
// operating systems, such as HP-UX, mkstemp() uses the process id and
// prepends one letter to yield temporary file names like STAFTempa13928
// and STAFTempb13928.  But having only one letter limits the number of
// temporary file names that this algorithm can generate to 26 files.
//
// So, instead, we are generating our own random filename, using the system
// time and request number to seed the rand() function.  The format of the
// unique filename is:
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
    static char fileNameChars[63] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVYXWZabcdefghijklmnopqrstuvwxyz";
    
    const unsigned int BUFSIZE = 4096;    // Maximum buffer size
    char tmpPathBuffer[BUFSIZE] = { 0 };  // Buffer for temp path name
    char randomBuffer[7] = { 0 };         // Buffer for random values
    unsigned int characterRange = strlen(fileNameChars) - 1;
    char cCharacter;
    unsigned int i;
    int tmpfd;    // Temporary file descriptor
    
    // Create a unique temporary file name with format:
    //   <tempDir>/STAFTemp??????[.<suffix>]
    // where <tempDir> contains the name of the directory to store the
    // temporary file and ?????? is a randomly assigned unique value.
    // Also, if a suffix is specified, e.g. out, it is added as the file
    // extension.

    STAFString tempNamePrefix = STAFString(tempDir) + STAFString(kUTF8_SLASH) +
        "STAFTemp";
    
    STAFString suffix = STAFString(tempFileSuffix);

    // Note:  We're using the request number which is unique per request on
    // each machine in additon to the time function to set the random seed.
    srand((unsigned)time(NULL) * requestNumber);

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

        tmpfd = open(tmpPathBuffer, O_CREAT | O_EXCL | O_WRONLY,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                     S_IROTH | S_IWOTH);

        if ((tmpfd == -1) && (errno != EEXIST))
        {
            // Fatal error - Don't retry

            *errorBuffer = STAFString("Creating temp file failed").adoptImpl();
            *osRC = errno;

            return kSTAFBaseOSError;
        }
    } while (tmpfd == -1);

    // Temporary file was successfully created and opened.  Close it.
    close(tmpfd);

    *tempFileName = STAFString(tmpPathBuffer).adoptImpl();

    return kSTAFOk;
}
#else
// The best solution for creating a temporary file on Unix is to use the
// mkstemp() function in the standard C runtime library.  This function
// generates a random filename, attempts to create it, and repeats the
// whole process until it is successful, thus guaranteeing that a unique
// file is created.  The file created by mkstemp() will be readable and
// writable by the owner, but not by anyone else.
// The mkstemp() function works by specifying a template from which a
// random filename can be generated.  From the end of the template,
// "X" characters are replaced with random characters.  The template is
// modified in place, so the specified buffer must be writable.  The
// return value from mkstemp() is -1 if an error occurs; otherwise, it
// is the file descriptor to the file that was created.

STAFRC_t STAFUtilCreateTempFile(STAFStringConst_t tempDir,
                                STAFStringConst_t tempFileSuffix,
                                unsigned int requestNumber,
                                STAFString_t *tempFileName, 
                                STAFString_t *errorBuffer,
                                unsigned int *osRC)
{
    // Note: 20 is the length of "<tempDir>/STAFTempXXXXXX" plus a null term char
    //char myTempName[20] = { 0 };

    const unsigned int BUFSIZE = 4096;    // Maximum buffer size
    char tempNameBuffer[BUFSIZE] = { 0 };  // Buffer for temp file name

    // The format of the unique filename is:  
    //   <tempDir>/STAFTempXXXXXX
    // where <tempDir> contains the name of the directory to store the temp file
    //       XXXXXX    represents 6 randomly generated characters.

    STAFString tempNameString = STAFString(tempDir) + STAFString(kUTF8_SLASH) +
        "STAFTempXXXXXX";

    (void) strcpy(tempNameBuffer, tempNameString.toCurrentCodePage()->buffer());

    int tmpfd = mkstemp(tempNameBuffer);
     
    if (tmpfd == -1)
    {
        *osRC = errno;
        *errorBuffer = STAFString("Temp file creation failed").adoptImpl();

        return kSTAFBaseOSError;
    }
    
    // Temporary file was successfully created and opened.  Close it.
    close(tmpfd);

    *tempFileName = STAFString(tempNameBuffer).adoptImpl();
    
    return kSTAFOk;
}
#endif
