/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
 
#include "STAF.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAFUtil.h"
#include "STAFFSService.h"
#include "STAFConnectionManager.h"
#include "STAFVariablePool.h"
#include "STAFException.h"
#include "STAFFileSystem.h"
#include "STAFHandleManager.h"
#include "STAFFSCopyManager.h"
#include "STAFTrace.h"
#include "STAFConverter.h"
#include "STAFThreadManager.h"
#include "STAFDataTypes.h"
#include <set>
#include <cstring>
#include <deque>
#include <algorithm>

#ifdef STAF_USE_SSL
#include <openssl/evp.h>
#include <openssl/crypto.h>
#endif

typedef std::set<STAFString> SET_STAFString;
typedef std::deque<STAFFSEntryPtr> STAFFSEntryList;

static STAFMutexSem sStrictFSCopyTrustSem;
static const STAFString sEnabled = "Enabled";
static const STAFString sDisabled = "Disabled";
static const unsigned int sMaxReadAttempts = 20;
static const unsigned int sReadRetryDelay = 500;  // 1/2 second (500ms)
static const STAFString sPeriod(kUTF8_PERIOD);
static const STAFString sDoublePeriod(sPeriod + sPeriod);
static const STAFString sNoneString("<None>");
static const STAFString sStar(kUTF8_STAR);
static const STAFString sDoubleQuote(kUTF8_DQUOTE);
static const STAFString sSpace(kUTF8_SPACE);
static STAFString sHelpMsg;

// Line ending strings for Windows and Unix
static const STAFString sWindowsEOL = STAFString(kUTF8_CR) +
                                      STAFString(kUTF8_LF);
static const STAFString sUnixEOL = STAFString(kUTF8_LF);

// Maximum buffer size for searching for a line ending in a file
static const int sLineEndingBufferSize = 4000;

static STAFMapClassDefinitionPtr fListLongInfoClass;
static STAFMapClassDefinitionPtr fListDetailsInfoClass;
static STAFMapClassDefinitionPtr fListSummaryInfoClass;

// This table allows for lookup of a hex character (in UTF-8)
static const char HEX_TABLE[] =
{
    // Here's the corresponding non-UTF-8 hex representation
    // '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    // 'A', 'B', 'C', 'D', 'E', 'F'

    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46
};

#ifdef STAF_USE_SSL
// The OpenSSL FAQ says: "Multi-threaded applications must provide two
// callback functions to OpenSSL by calling CRYPTO_set_locking_callback()
// and CRYPTO_set_id_callback().  Since the FS service's GET ENTRY CHECKSUM
// request calls OpenSSL apis, the FS service must provide these OpenSSL
// callback functions.

// This is a pointer to an array of mutexes needed by the OpenSSL locking
// callback function
#ifdef STAF_OS_TYPE_WIN32
static HANDLE *lock_cs;
#else
static pthread_mutex_t *lock_cs;
#endif

/*
 * The locking callback function is needed to perform locking on shared data
 * structures used by OpenSSL whenever multiple threads use OpenSSL.
 * This function must be able to handle up to CRYPTO_num_locks() different
 * mutex locks. It sets the n-th lock if mode & CRYPTO_LOCK, and releases it
 * otherwise.  We define one locking callback function for Windows and another
 * for Unix operating systems.
 */
#ifdef STAF_OS_TYPE_WIN32
static void STAF_SSL_Locking_Callback(int mode, int type,
                                      const char *file, int line)
{
    if (mode & CRYPTO_LOCK)
        WaitForSingleObject(lock_cs[type], INFINITE);
    else
        ReleaseMutex(lock_cs[type]);
}

#else
static void STAF_SSL_Locking_Callback(int mode, int type,
                                      const char *file, int line)
{
    if (mode & CRYPTO_LOCK)
        pthread_mutex_lock(&(lock_cs[type]));
    else
        pthread_mutex_unlock(&(lock_cs[type]));
}

/*
 * This callback function returns a thread ID.  It isn't needed on Windows
 * nor on platforms where getpid() returns a different ID for each thread.
 */
static unsigned long STAF_SSL_ThreadID_Callback()
{
    return (unsigned long)pthread_self();
}
#endif

/*
 * This function assigns the callback functions needed for multi-threaded
 * applications that use OpenSSL so that they don't randomly crash.
 */
static void STAF_SSL_Thread_Setup()
{
    // The CRYPTO_set_locking_callback() method requires an array of mutexes
    // that is needed for locking access to global shared data structure
    // used by OpenSSL

#   ifdef STAF_OS_TYPE_WIN32
        lock_cs = (HANDLE *)OPENSSL_malloc(
            CRYPTO_num_locks() * sizeof(HANDLE));
#   else
        lock_cs = (pthread_mutex_t *)OPENSSL_malloc(
            CRYPTO_num_locks() * sizeof(pthread_mutex_t));
#   endif

    for (int i = 0; i < CRYPTO_num_locks(); i++)
    {
#       ifdef STAF_OS_TYPE_WIN32
            lock_cs[i] = CreateMutex(NULL, FALSE, NULL);
#       else
            pthread_mutex_init(&(lock_cs[i]), NULL);
#       endif
    }

    // Assign callback functions needed for multi-threaded applications
    // that use OpenSSL

    CRYPTO_set_locking_callback(STAF_SSL_Locking_Callback);

    // If Unix, assign a callback function for returning a thread id

#   ifndef STAF_OS_TYPE_WIN32
        CRYPTO_set_id_callback(STAF_SSL_ThreadID_Callback);
#   endif
}

static void STAF_SSL_Thread_Cleanup()
{
    // Remove the SSL locking callback function

    CRYPTO_set_locking_callback(NULL);

    // If Unix, Remove the SSL thread id callback function

#   ifndef STAF_OS_TYPE_WIN32
        CRYPTO_set_id_callback(NULL);
#   endif

    // Delete the mutexes used for locking by OpenSSL

    for (int i = 0; i < CRYPTO_num_locks(); i++)
    {
#       ifdef STAF_OS_TYPE_WIN32
            CloseHandle(lock_cs[i]);
#       else
            pthread_mutex_destroy(&(lock_cs[i]));
#       endif
    }

    OPENSSL_free(lock_cs);
}
#endif

struct STAFSortEnumByName
{
    STAFSortEnumByName(STAFFSCaseSensitive_t caseSensitive)
        : fCaseSensitive(caseSensitive)
    { /* Do nothing */ }

    bool operator()(STAFFSEntryPtr lhs, STAFFSEntryPtr rhs)
    {
        unsigned int comp = 0;

        if (fCaseSensitive == kSTAFFSCaseSensitive)
        {
            STAFStringCompareTo(lhs->path().asString().getImpl(),
                                rhs->path().asString().getImpl(), &comp, 0);
        }
        else
        {
            STAFStringCompareTo(lhs->path().asString().toUpperCase().getImpl(),
                                rhs->path().asString().toUpperCase().getImpl(),
                                &comp, 0);
        }

        return (comp == 1);
    }

    STAFFSCaseSensitive_t fCaseSensitive;
};


static bool sortEnumBySize(STAFFSEntryPtr lhs, STAFFSEntryPtr rhs)
{
    return (lhs->size64() < rhs->size64());
}


static bool sortEnumByModTime(STAFFSEntryPtr lhs, STAFFSEntryPtr rhs)
{
    return (lhs->modTime() < rhs->modTime());
}


STAFString getTypeString(STAFFSEntryType_t type)
{
    switch (type)
    {
        case kSTAFFSFile:              return "F";
        case kSTAFFSSpecialDirectory: // Fall through
        case kSTAFFSDirectory:         return "D";
        case kSTAFFSPipe:              return "P";
        case kSTAFFSSocket:            return "S";
        case kSTAFFSSymLink:           return "L";
        case kSTAFFSBlkDev:            return "B";
        case kSTAFFSCharDev:           return "C";
        case kSTAFFSOther:             return "O";
        default:                       return "?";
    }
}

unsigned int getTypeMaskFromString(const STAFString &typeString,
                                   bool includeSpecial)
{
    STAFString upperTypeString = typeString.toUpperCase();
    unsigned int entryTypesUInt = kSTAFFSNone;

    for (int i = 0; i < upperTypeString.length(STAFString::kChar); ++i)
    {
        STAFString thisChar = upperTypeString.subString(i, 1, STAFString::kChar);

        if ((thisChar == "!") && includeSpecial)
        {
            entryTypesUInt |= kSTAFFSSpecialDirectory;
        }
        else if (thisChar == "F") entryTypesUInt |= kSTAFFSFile;
        else if (thisChar == "D") entryTypesUInt |= kSTAFFSDirectory;
        else if (thisChar == "P") entryTypesUInt |= kSTAFFSPipe;
        else if (thisChar == "S") entryTypesUInt |= kSTAFFSSocket;
        else if (thisChar == "L") entryTypesUInt |= kSTAFFSSymLink;
        else if (thisChar == "B") entryTypesUInt |= kSTAFFSBlkDev;
        else if (thisChar == "C") entryTypesUInt |= kSTAFFSCharDev;
        else if (thisChar == "O") entryTypesUInt |= kSTAFFSOther;
    }

    return entryTypesUInt;
}


void updateResultString(STAFObjectPtr &outputList, STAFFSEntryPtr &entry,
                        STAFRC_t rc, unsigned int osRC)
{
    if (rc != kSTAFOk)
    {
        STAFObjectPtr errorInfoMap = STAFObject::createMap();
        errorInfoMap->put("staf-map-class-name",
                          "STAF/Service/FS/ErrorInfo");
        errorInfoMap->put("name", entry->path().asString());
        errorInfoMap->put("rc", STAFString(rc));

        if (rc == kSTAFBaseOSError)
            errorInfoMap->put("osRC", STAFString(osRC));

        // Add remaining map entries here
        outputList->append(errorInfoMap);
    }
}


STAFRC_t readFile(fstream &inFile, char *fileBuffer, unsigned int readLength,
                  const STAFString fromFile, const STAFString toMachine,
                  unsigned int fileLength, unsigned int currentPos)
{
    /* Leave in (commented out) for future debugging purposes.
    // Make sure that currentPos is the correct current position in the file
    // (indicating the number of bytes read so far) now that we're no longer
    // using tellg() because it only returns an int which means it does not
    // support files >= 2G.
    if (currentPos <= (INT_MAX - readLength))
    {
        int currentPosInt = inFile.tellg();// Save current position before read
        unsigned int myCurrentPos = (unsigned int)currentPosInt;

        if (myCurrentPos != currentPos)
        {
            cout << "STAFFSService::readFile() - MISMATCH: myCurrentPos="
                 << myCurrentPos << " currentPos=" << currentPos << endl;
        }
    }
    */
    
    // Read a block of data from the file
    
    inFile.read(fileBuffer, readLength);
    
    if (inFile.good())
        return kSTAFOk;  // Read was successful

    // If get eof condition reading the file, make sure that all bytes have
    // really been read to make sure it's not a "pre-mature" eof condition.
    //
    // Note:  A "pre-mature" eof condition can occur on Windows when copying
    // from a file on a mapped drive (in text mode) and the drive is
    // disconnected.

    if (inFile.eof() && (currentPos + inFile.gcount() == fileLength))
        return kSTAFOk;  // Completed reading the file
    
    // The read failed.  Retry the read up to sMaxReadAttempts times with a
    // delay between each attempt.

    for (int readAttempt = 1;
         !inFile.good() && readAttempt <= sMaxReadAttempts;
         readAttempt++)
    {
        if (inFile.fail())
        {
            // Recoverable read error

            // Log a warning tracepoint message

            STAFString warningMsg(
                "STAFFSService::readFile() - Read attempt #" +
                STAFString(readAttempt) + " failed while copying file " +
                fromFile + " to machine " + toMachine + " after reading " +
                STAFString(currentPos) + " bytes");

            STAFTrace::trace(kSTAFTraceWarning, warningMsg);

            // Delay and retry read after clearing any error flags and
            // repositioning the file pointer

            STAFThreadManager::sleepCurrentThread(
                sReadRetryDelay);
            
            inFile.clear();
            
            if (readAttempt == sMaxReadAttempts)
            {
                // Before the last read attempt, try closing the file and
                // reopening it first to see if that fixes the problem

                inFile.close();
                inFile.open(fromFile.toCurrentCodePage()->buffer(),
                            ios::in | STAF_ios_binary);
            }

            if (currentPos <= INT_MAX)
            {
                inFile.seekg(currentPos, ios::beg);
                inFile.read(fileBuffer, readLength);
            }
            else
            {
                // Reading a large file, so need to do multiple seekg()'s
                // because seekg() only accepts an integer for the offset
                
                inFile.seekg(INT_MAX, ios::beg);

                unsigned int setPos = currentPos - INT_MAX;

                while (setPos > INT_MAX)
                {
                    inFile.seekg(INT_MAX, ios::cur);
                    setPos = setPos - INT_MAX;
                }

                inFile.seekg((int)setPos, ios::cur);
                 
                if (inFile.good())
                    inFile.read(fileBuffer, readLength);
            }
        }
        else if (inFile.bad())
        {
            // Unrecoverable read error.
            break;
        }
    }

    if (!inFile.good())
    {
        // Unrecoverable read failure

        return kSTAFFileReadError;
    }

    return kSTAFOk;
}


STAFServiceResult determineLineEnding(unsigned int fileLength,
                                      fstream &inFile,
                                      const STAFString &fromFile,
                                      const STAFString &toMachine,
                                      const STAFString &defaultEOL)
{
    // Determine current EOL (End Of Line ending) as follows:
    // 1) Look for the first Windows line ending and the first Unix
    //    line ending in the first 4000 characters of the source file.
    //    Whichever line ending is found first will be assumed to be
    //    the line ending in this text file.
    // 2) If no Windows or Unix line ending character is found in the
    //    first 4000 characters, then default to the line ending
    //    character for the type of operating system where the source
    //    source file resides.
    //
    // If successful, the current EOL will be returned in the fResult
    // of the STAFServiceResult returned.

    STAFString currentEOL = STAFString("");

    if (fileLength > 0)
    {
        char *buffer = new char[sLineEndingBufferSize];
        unsigned int readLength = STAF_MIN(sLineEndingBufferSize, fileLength);

        STAFRC_t rc = readFile(inFile, buffer, readLength,
                               fromFile, toMachine, fileLength, 0);

        if (rc == kSTAFOk)
        {
            // Create a STAFString containing up to the first 4000 bytes
            // of the file data

            try
            {
                STAFString fileData = STAFString(buffer, readLength);

                // Check which line ending is found first in the file and
                // assign to currentEOL

                unsigned int windowsEOLIndex = fileData.find(sWindowsEOL);
                unsigned int unixEOLIndex = fileData.find(sUnixEOL);

                if (windowsEOLIndex < unixEOLIndex)
                    currentEOL = sWindowsEOL;
                else if (unixEOLIndex < windowsEOLIndex)
                    currentEOL = sUnixEOL;
            }
            catch (...)
            {
                // Do nothing 
            }
        }

        // Set the file pointer back to the beginning of the file

        inFile.clear();
        inFile.seekg(0, ios::beg);
    }

    if (currentEOL.length() == 0)
    {
        // Could not determine the current EOL by looking for the first EOL in
        // the first 4000 bytes, so set to source machine's OS line ending.

        if (defaultEOL.length() > 0)
        {
            // On a directory copy, the line ending for the source machine
            // has already been determined and is passed in as defaultEOL

            currentEOL = defaultEOL;
        }
        else
        {
            STAFConfigInfo sysInfo;
            STAFString_t errorBuffer;
            unsigned int osRC;

            if (STAFUtilGetConfigInfo(&sysInfo, &errorBuffer, &osRC) !=
                kSTAFOk)
            {
                return STAFServiceResult(
                    kSTAFBaseOSError,
                    STAFString("Get current EOL failure.  ") +
                    STAFString(errorBuffer, STAFString::kShallow) +
                    ", RC: " + osRC);
            }

            currentEOL = sysInfo.lineSeparator;
        }
    }
    
    return STAFServiceResult(kSTAFOk, currentEOL);
}

STAFRC_t copyDirectory(STAFFSEnumPtr childEnum, STAFString fromDir,
    STAFConnectionPtr connection, SET_STAFString textExtList,
    STAFString currentEOL, STAFString newEOL, bool doCodepageConvert,
    int level, STAFFSCaseSensitive_t caseSensitive, STAFObjectPtr &outputList,
    STAFFSCopyManager::FSCopyDataPtr copyDataPtr, const STAFString &toMachine)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFRC_t ack = kSTAFOk;
    STAFString ackResult;
    STAFFSEntryPtr fileEntry;
    STAFString toFile;
    bool textTransfer;
    bool doConvert = doCodepageConvert;
    SET_STAFString::iterator i;

    for (; childEnum->isValid(); childEnum->next())
    {
        fileEntry = childEnum->entry();
        STAFString fromFile = fileEntry->path().asString();
        fromFile = fromFile.replace(kUTF8_BSLASH, kUTF8_SLASH);
        toFile = fromFile.subString(fromDir.length());

        // Open the file to copy

        fstream inFile(fromFile.toCurrentCodePage()->buffer(),
                    ios::in | STAF_ios_binary);

        if (!inFile)
        {
            updateResultString(outputList, fileEntry,
                               kSTAFFileOpenError, osRC);
            continue;
        }

        // Get the size of the file (upperSize and lowerSize).
        // If the file size is < 4G, upperSize will be zero.
        
        unsigned int upperSize = fileEntry->size().first;
        unsigned int lowerSize = fileEntry->size().second;

        if (upperSize > 0)
        {
            // File size exceeds the maximum that the FS service handles

            updateResultString(outputList, fileEntry,
                               kSTAFFileReadError, osRC);
            // XXX: How provide a better error message such as the following?
            //"File size exceeds maximum size ") + UINT_MAX + ") supported"

            inFile.close();
            continue;
        }

        unsigned int fileLength = lowerSize;

        // Determine if this file is to be text or binary

        textTransfer = false;
        unsigned int isMatch;

        for(i = textExtList.begin(); i != textExtList.end(); i++)
        {
            isMatch = STAFFileSystem::matchesWildcards(
                fileEntry->path().extension(), *i, caseSensitive);
            textTransfer |= (isMatch > 0);
        }

        if (textTransfer)
        {
            // Determine what line ending the from file contains

            STAFServiceResult result = determineLineEnding(
                fileLength, inFile, fromFile, toMachine, currentEOL);

            if (result.fRC != kSTAFOk)
            {
                updateResultString(outputList, fileEntry, result.fRC, osRC);
                // XXX: How provide a better error message such as
                // result.fResult?

                continue;
            }
            
            currentEOL = result.fResult;
        }

        doCodepageConvert = doConvert && textTransfer;

        textTransfer &= (!newEOL.isEqualTo(currentEOL));

        connection->writeUInt(kSTAFFSContinueCopy);

        connection->writeUInt(kSTAFFSFile);
        connection->writeString(toFile);

        if (level > 1)
        {
            if (doCodepageConvert)
                connection->writeUInt(kSTAFFSTextConvert);
            else if (textTransfer)
                connection->writeUInt(kSTAFFSTextNoConvert);
            else
                connection->writeUInt(kSTAFFSBinary);
        }

        char fileBuffer[4000] = {0};
        unsigned int writeLength = 0;

        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());
        ackResult = connection->readString();

        if (ack != kSTAFOk)
        {
            updateResultString(outputList, fileEntry, ack, osRC);
            inFile.close();
            continue;
        }

        if (doCodepageConvert)
        {
            // Perform a text transfer with conversion of eol chars and a
            // codepage conversion.  It runs considerably slower than the
            // binary transfer.  This is the type of transfer that will occur
            // if the TEXTEXT option is specified and the codepages are
            // different.  Data is sent as a series of buffers of UTF8 chars.
            
            gFSCopyManagerPtr->updateDirectoryCopy(
                copyDataPtr, fromFile, kSTAFFSTextConvert, fileLength);

            connection->writeString(currentEOL);
            
            if (fileLength > 0)
            {
                STAFStringBufferPtr eolStr =
                    currentEOL.toCurrentCodePage();

                // How many bytes in the end-of-line sequence
                unsigned int eolBufferSize = eolStr->length();

                // Points to a buffer containing end-of-line sequence
                char *eolBuffer = new char[eolBufferSize];

                try
                {
                    memcpy(eolBuffer, eolStr->buffer(), eolBufferSize);
                    
                    // Last byte of end-of-line sequence
                    char eolLastChar = eolBuffer[eolBufferSize - 1];

                    const unsigned int sBufferSize = 4096;
                    
                    STAFRefPtr<char> buffer = STAFRefPtr<char>
                        (new char[sBufferSize], STAFRefPtr<char>::INIT,
                         STAFRefPtr<char>::ARRAY);

                    unsigned int bufferSize = sBufferSize;
                    unsigned int readOffset = 0;  // Buffer read offset
                    unsigned int bytesCopied  = 0;
                    bool done = false;
                    
                    while (!done)
                    {
                        rc = readFile(inFile,
                                      static_cast<char *>(buffer + readOffset),
                                      bufferSize - readOffset,
                                      fromFile, toMachine,
                                      fileLength, bytesCopied);

                        if (rc != kSTAFOk)
                        {
                            updateResultString(outputList, fileEntry,
                                               rc, osRC);
                            break;
                        }

                        unsigned int bytesInBuffer = inFile.gcount() +
                            readOffset;

                        bytesCopied += inFile.gcount();

                        if (bytesInBuffer < bufferSize) done = true;

                        // Find a newline. Make sure we don't underrun the
                        // buffer.

                        unsigned int i = 0;
                        unsigned int guardIndex = eolBufferSize - 1;

                        if (bytesInBuffer > 0)
                        {
                            i = bytesInBuffer - 1;  // Last NewLine index

                            while (((buffer[i] != eolLastChar) ||
                                    !memcmp(buffer + i - eolBufferSize,
                                            eolBuffer, eolBufferSize)) &&
                                   (i > guardIndex))
                            { --i; }
                        }

                        while ((i == guardIndex) && !done)
                        {
                            // We have a line bigger than our buffer.
                            // Note: the beginning of the buffer may be a lone
                            // newline, but we ignore that for this algorithm
                            // (as the next line is likely larger than the
                            // buffer anyway.

                            // First, create a buffer that is double our
                            // current size, and copy our existing buffer data
                            // into it.

                            STAFRefPtr<char> tmpBuffer = STAFRefPtr<char>
                                (new char[bufferSize * 2],
                                 STAFRefPtr<char>::INIT,
                                 STAFRefPtr<char>::ARRAY);

                            memcpy(tmpBuffer, buffer, bufferSize);
                            buffer = tmpBuffer;
                            bufferSize *= 2;

                            // Now, read in data to fill remainder of the
                            // buffer

                            rc = readFile(inFile, buffer + (bufferSize / 2),
                                          bufferSize / 2, fromFile, toMachine,
                                          fileLength, bytesCopied);

                            if (rc != kSTAFOk)
                            {
                                updateResultString(outputList, fileEntry,
                                                   rc, osRC);
                                break;
                            }

                            bytesInBuffer += inFile.gcount();
                            bytesCopied += inFile.gcount();

                            // Finally, let's check to make sure that this
                            // buffer was big enough, by finding a newline.
                            // Otherwise, let's run the loop again.
                            
                            if (bytesInBuffer < bufferSize) done = true;
                            
                            i = 0;

                            if (bytesInBuffer > 0)
                            {
                                i = bytesInBuffer - 1;  // Last NewLine index
                                guardIndex = (bufferSize / 2) - eolBufferSize;

                                while (((buffer[i] != eolLastChar) ||
                                        !memcmp(buffer + i - eolBufferSize,
                                                eolBuffer, eolBufferSize)) &&
                                       (i > guardIndex))
                                { --i; }
                            }
                        } // while ((i == guardIndex) && !done)

                        // We now have the last newline in the buffer

                        if (!done)
                        {
                            // Need to see if can create a STAFString (e.g. 
                            // without getting a codepage converter exception,
                            // before writing kSTAFFSContinueCopy and the data

                            STAFString data = STAFString(
                                buffer, i + 1, STAFString::kCurrent);

                            connection->writeUInt(kSTAFFSContinueCopy);
                            connection->writeString(data);
                        
                            memmove(buffer, buffer + i + 1,
                                    bufferSize - i - 1);

                            readOffset = bufferSize - i - 1;
                        }
                        else
                        {
                            // Need to see if can create a STAFString (e.g. 
                            // without getting a codepage converter exception,
                            // before writing kSTAFFSContinueCopy and the data

                            STAFString data = STAFString(
                                buffer, bytesInBuffer, STAFString::kCurrent);

                            connection->writeUInt(kSTAFFSContinueCopy);
                            connection->writeString(data);
                        }

                        gFSCopyManagerPtr->updateFileCopy(copyDataPtr,
                                                          bytesCopied);
                    } // while (!done)
                }
                catch (STAFException &e)
                {
                    // XXX: It would be nice if we could provide more
                    //      information on the error using e.getText().

                    rc = e.getErrorCode();
                    updateResultString(outputList, fileEntry, rc, osRC);
                }
                    
                delete[] eolBuffer;

            } // if (fileLength > 0)

            connection->writeUInt(kSTAFFSFinishedCopy);

            inFile.close();

            if (rc == kSTAFOk)
            {
                // Read an ack, so that we know the file is closed
            
                ack = connection->readUInt();

                if (ack != kSTAFOk)
                {
                    updateResultString(outputList, fileEntry, ack, osRC);
                }
            }
        }

        /*  This if clause is used to perform a text transfer with
            conversion of eol chars without a codepage conversion
            It runs considerably faster than the codepage conversion
            transfer.
            This is the type of transfer that will occur if the NOCONVERT
            option is enabled
            This is the type of transfer that will occur if the codepages
            are the same and a text transfer has been specified
        */
        else if (textTransfer)
        {
            gFSCopyManagerPtr->updateDirectoryCopy(
                copyDataPtr, fromFile, kSTAFFSTextNoConvert, fileLength);

            STAFString transferString;
            int bufferSize = 3000;
            unsigned int transferLen = 0;
            char *buffer = new char[bufferSize];
            unsigned int bytesCopied = 0;

            connection->writeString(currentEOL);
            connection->writeUInt(bufferSize);

            while ((fileLength > 0) && (inFile.good()))
            {
                transferLen = STAF_MIN(bufferSize, fileLength);

                rc = readFile(inFile, buffer, transferLen, fromFile,
                              toMachine, fileLength, bytesCopied);

                if (rc != kSTAFOk)
                {
                    updateResultString(outputList, fileEntry, rc, osRC);
                    fileLength = 0;
                    break;
                }

                connection->writeUInt(transferLen);
                connection->write(buffer, transferLen);

                fileLength -= transferLen;
                bytesCopied += transferLen;
                gFSCopyManagerPtr->updateFileCopy(copyDataPtr, bytesCopied);
            }

            connection->writeUInt(kSTAFFSFinishedCopy);

            delete[] buffer;
            inFile.close();

            if (rc == kSTAFOk)
            {
                // Read an ack, so that we know the file is closed

                ack = connection->readUInt();

                if (ack != kSTAFOk)
                {
                    updateResultString(outputList, fileEntry, ack, osRC);
                    fileLength = 0;
                }
            }
        }
        else
        {
            // Perform a binary transfer of a file

            unsigned int bytesCopied = 0;
            
            gFSCopyManagerPtr->updateDirectoryCopy(
                copyDataPtr, fromFile, kSTAFFSBinary, fileLength);
            
            connection->writeUInt(fileLength);

            if (level > 3)
            {
                // Starting with level 4 for the STAFDirectoryCopyAPI, to
                // improve performance, acks are no longer sent/received
                // after each read/write. Instead, a final ack is received
                // after the entire file is processed which indicates if
                // the copy was successful.

                while ((fileLength > 0) && (inFile.good()))
                {
                    writeLength = STAF_MIN(sizeof(fileBuffer), fileLength);

                    rc = readFile(
                        inFile, reinterpret_cast<char *>(fileBuffer),
                        writeLength, fromFile, toMachine, fileLength,
                        bytesCopied);

                    if (rc != kSTAFOk) break;

                    connection->write(fileBuffer, writeLength);
                    fileLength -= writeLength;
                    bytesCopied += writeLength;
                    gFSCopyManagerPtr->updateFileCopy(
                        copyDataPtr, bytesCopied);
                }

                inFile.close();
                
                if (rc == kSTAFOk)
                {
                    // Receive the final acknowledgement that indicates if
                    // the file was copied successfully
                    rc = connection->readUInt();
                }

                if (rc != kSTAFOk)
                {
                    // Update error information in result
                    updateResultString(outputList, fileEntry, rc, osRC);
                    fileLength = 0;
                }
            }
            else
            {
                // Levels < 4 for the STAFDirectoryCopyAPI send/receive
                // acknowledgements after each read/write.

                while ((fileLength > 0) && (inFile.good()))
                {
                    writeLength = STAF_MIN(sizeof(fileBuffer), fileLength);

                    rc = readFile(
                        inFile, reinterpret_cast<char *>(fileBuffer),
                        writeLength, fromFile, toMachine, fileLength,
                        bytesCopied);

                    if (rc != kSTAFOk)
                    {
                        updateResultString(outputList, fileEntry, rc, osRC);
                        fileLength = 0;
                        connection->writeUInt(kSTAFFileReadError);
                        break;
                    }

                    connection->writeUInt(kSTAFOk);
                    connection->write(fileBuffer, writeLength);
                    fileLength -= writeLength;

                    ack = connection->readUInt();

                    if (ack != kSTAFOk)
                    {
                        updateResultString(outputList, fileEntry, ack, osRC);
                        fileLength = 0;
                        break;
                    }

                    bytesCopied += writeLength;
                    gFSCopyManagerPtr->updateFileCopy(
                        copyDataPtr, bytesCopied);
                }

                inFile.close();
            }
        }
    }

    return kSTAFOk;
}


STAFRC_t recurseCopyDir(STAFFSEntryPtr entry, const STAFString &namePattern,
    const STAFString &extPattern, STAFString fromDir, bool keepEmptyDir,
    bool onlyDir, unsigned int entryTypesUInt, STAFConnectionPtr connection,
    STAFFSCaseSensitive_t caseSensitive, SET_STAFString textExtList ,
    STAFString currentEOL, STAFString newEOL, int level, bool doCodepageConvert,
    STAFObjectPtr &outputList, STAFFSCopyManager::FSCopyDataPtr copyDataPtr,
    const STAFString &toMachine)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    // Enumerate the files in the current entry
    STAFFSEnumPtr fileEnum = entry->enumerate(namePattern, extPattern,
        STAFFSEntryType_t(entryTypesUInt & ~kSTAFFSDirectory),
        kSTAFFSNoSort, caseSensitive);

    STAFString thisDir = entry->path().asString();

    // relative toDirectory
    STAFString toDir = thisDir.subString(fromDir.length());
    toDir = toDir.replace(kUTF8_BSLASH, kUTF8_SLASH);

    // If the thisDir is equal to fromDir, then the directory already exists
    // If there is at least one file to be copied or KEEPEMPTYDIRECTORIES
    // or ONLYDIRECTORIES option was used, then create the current directory.
    if ((thisDir != fromDir) &&
        (onlyDir || keepEmptyDir || (fileEnum->isValid())))
    {
        connection->writeUInt(kSTAFFSContinueCopy);
        connection->writeUInt(kSTAFFSDirectory);
        connection->writeString(toDir);

        //...wait for STAFProc's ack
        STAFRC_t ack = static_cast<STAFRC_t>(connection->readUInt());
        STAFString ackResult = connection->readString();

        if (ack != kSTAFOk)
            updateResultString(outputList, entry, ack, osRC);
    }

    // copy current directory
    if (!onlyDir && fileEnum->isValid())
        copyDirectory(fileEnum, fromDir, connection, textExtList, currentEOL,
                      newEOL, doCodepageConvert, level, caseSensitive,
                      outputList, copyDataPtr, toMachine);

    // Enumerate the directories in the current entry
    STAFFSEnumPtr directoryEnum = entry->enumerate(kUTF8_STAR, kUTF8_STAR,
        STAFFSEntryType_t(entryTypesUInt & ~kSTAFFSFile),
        kSTAFFSNoSort, caseSensitive);

    // copy sub-directories in current directory
    for (; directoryEnum->isValid(); directoryEnum->next())
    {
        recurseCopyDir(directoryEnum->entry(), namePattern, extPattern,
                       fromDir, keepEmptyDir, onlyDir, entryTypesUInt,
                       connection, caseSensitive, textExtList, currentEOL,
                       newEOL, level, doCodepageConvert, outputList,
                       copyDataPtr, toMachine);
    }

    return kSTAFOk;
}


STAFRC_t removeChildren(STAFFSEntryPtr entry, const STAFString &namePattern,
                        const STAFString &extPattern,
                        unsigned int entryTypesUInt,
                        STAFFSCaseSensitive_t caseSensitive, STAFObjectPtr &outputList)
{
    STAFFSEnumPtr childEnum = entry->enumerate(namePattern, extPattern,
                                               STAFFSEntryType_t(entryTypesUInt),
                                               kSTAFFSNoSort, caseSensitive);
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    for (; childEnum->isValid(); childEnum->next())
    {
        STAFFSEntryPtr entry = childEnum->entry();
        rc = entry->remove(&osRC);
        updateResultString(outputList, entry, rc, osRC);
    }

    return kSTAFOk;
}


STAFRC_t recurseRemove(STAFFSEntryPtr entry, const STAFString &namePattern,
                       const STAFString &extPattern, unsigned int entryTypesUInt,
                       STAFFSCaseSensitive_t caseSensitive, STAFObjectPtr &outputList)
{
    STAFFSEnumPtr childDirEnum = entry->enumerate(kUTF8_STAR, kUTF8_STAR,
                                                  kSTAFFSDirectory);
    STAFRC_t rc = kSTAFOk;

    for (; childDirEnum->isValid(); childDirEnum->next())
    {
        STAFFSEntryPtr childDirEntry = childDirEnum->entry();
        
        // If child directory entry is a link, skip it

        if (!childDirEntry->isLink())
        {
            rc = recurseRemove(childDirEntry, namePattern, extPattern,
                               entryTypesUInt, caseSensitive, outputList);
        }
    }

    rc = removeChildren(entry, namePattern, extPattern, entryTypesUInt,
                        caseSensitive, outputList);

    return kSTAFOk;
}


STAFRC_t addListDirectoryEntry(
    STAFString rootDir, STAFFSEntryPtr entry, STAFObjectPtr &outputList,
    STAFObjectPtr &mc, bool showLong, bool details)
{
    // Remove the root directory from the name
            
    STAFString theName = entry->path().asString();
    theName = theName.subString(rootDir.length() + 1);

    // Adds an entry in the specified format to the outputList.
    // Used when listing the contents of a directory..

    if (showLong && details)
    {
        STAFObjectPtr fileInfoMap = fListDetailsInfoClass->createInstance();
        fileInfoMap->put("name", theName);
        fileInfoMap->put("type", getTypeString(entry->type()));
        fileInfoMap->put("size", STAFString(entry->size64()));
        fileInfoMap->put("upperSize", STAFString(entry->size().first));
        fileInfoMap->put("lowerSize", STAFString(entry->size().second));
        fileInfoMap->put("lastModifiedTimestamp",
                         entry->modTime().asString());

        if (entry->linkTarget() != "")
            fileInfoMap->put("linkTarget", entry->linkTarget());

        outputList->append(fileInfoMap);
    }
    else if (showLong && !details)
    {
        STAFObjectPtr fileInfoMap =  fListLongInfoClass->createInstance();
        fileInfoMap->put("type", getTypeString(entry->type()));

        STAFString sizeInfo;
        STAFUInt64_t size = entry->size64();

        if (size > (99999 * 1024))
            sizeInfo = STAFString(size / (1024 * 1024)) + "M"; // Megabytes
        else if (size > 99999)
            sizeInfo = STAFString(size / 1024) + "K"; // Kilobytes
        else
            sizeInfo = STAFString(size);  // Bytes

        fileInfoMap->put("size", sizeInfo);

        fileInfoMap->put("lastModifiedTimestamp", entry->modTime().asString());
        fileInfoMap->put("name", theName);

        if (entry->linkTarget() != "")
            fileInfoMap->put("linkTarget", entry->linkTarget());
                  
        outputList->append(fileInfoMap);
    }
    else 
    {
        outputList->append(theName);
    }

    return kSTAFOk;
}


/*****************************************************************************/
/* getDirectorySize - Gets the total size of the entries in a directory      */
/*                    whose name, extension, type, and case-sensitivity      */
/*                    match the specified criteria.  If the recurse argument */
/*                    is true, the size will also include the sizes of the   */
/*                    entries in subdirectories that match the specified     */
/*                    criteria.                                              */
/*                                                                           */
/* Accepts: (In)  File system entry (must be for a directory)                */
/*          (In)  Name pattern (* indicates to match the name of any entry   */
/*                in the directory)                                          */
/*          (In)  Extension pattern (* indicates to match the extension of   */
/*                any entry in the directory)                                */
/*          (In)  Types (types of entries in the directory that match)       */
/*          (In)  Case sensitive (indicates whether to match the name and    */
/*                extension patterns in a case sensitive manner)             */
/*          (In)  Recurse (true indicates to count matching entries in       */
/*                subdirectories)                                            */
/*          (Out) Total size of the directory                                */
/*          (Out) Number of files in the directory                           */
/*          (Out) Number of subdirectories in the directory                  */
/*                                                                           */
/* Returns: Standard return codes                                            */
/*****************************************************************************/
STAFRC_t getDirectorySize(const STAFFSEntryPtr dirEntry,
                          const STAFString &namePattern,
                          const STAFString &extPattern,
                          const unsigned int entryTypesUInt,
                          const STAFFSCaseSensitive_t caseSensitive,
                          const bool recurse,
                          STAFUInt64_t &size, STAFUInt64_t &numFiles,
                          STAFUInt64_t &numDirectories)
{
    // Enumerate all entries in the directory with the specified types,
    // and also all sub-directories if the recurse option was specified
    
    STAFFSEntryType_t entryTypes;

    if (recurse)
        entryTypes = STAFFSEntryType_t(entryTypesUInt | kSTAFFSDirectory);
    else
        entryTypes = STAFFSEntryType_t(entryTypesUInt);

    // Get an enumeration of the entries (specify no sorting since only
    // getting a summary)

    STAFFSEnumPtr dirEnum = dirEntry->enumerate(
        kUTF8_STAR, kUTF8_STAR, entryTypes, kSTAFFSNoSort, caseSensitive);

    // Iterate through the entries

    for (; dirEnum->isValid(); dirEnum->next())
    {
        STAFFSEntryPtr entry = dirEnum->entry();
        
        // Check if name and extension (in the specified case) and type
        // match the specified criteria

        if ((STAFFileSystem::matchesWildcards(entry->path().name(),
                                              namePattern, caseSensitive)) &&
            (STAFFileSystem::matchesWildcards(entry->path().extension(),
                                              extPattern, caseSensitive)) &&
            (entry->type() & STAFFSEntryType_t(entryTypesUInt)))
        {
            // This entry matches the specified criteria

            if (!entry->isLink())
            {
                // If the entry is a link, the size is for the entry it is
                // linked to (which may not even be in the same directory)
                // so don't include the size of a link entry

                size += entry->size64();
            }

            if (entry->type() & kSTAFFSDirectory)
                numDirectories++;
            else
                numFiles++;
        }

        // If recurse was specified and the entry is a directory, get the total
        // size of the directory

        if (recurse && (entry->type() & kSTAFFSDirectory))
        {
            // Skip special directories . and ..

            if ((entry->path().name() == sPeriod) ||
                (entry->path().name() == sDoublePeriod))
               continue;
            
            getDirectorySize(
                entry, namePattern, extPattern, entryTypesUInt, caseSensitive,
                recurse, size, numFiles, numDirectories);
        }
    }

    return kSTAFOk;
}


STAFRC_t recurseListDir(
    STAFFSEntryPtr dirEntry, const STAFString &namePattern,
    const STAFString &extPattern, unsigned int entryTypesUInt,
    STAFFSCaseSensitive_t caseSensitive, STAFFSSortBy_t sortBy,
    STAFFSEntryList &entryList)
{
    // Enumerate all entries whose type matches one specified, as well as
    // enumerate all directory entries
    
    STAFFSEnumPtr dirEnum = dirEntry->enumerate(
        kUTF8_STAR, kUTF8_STAR,
        STAFFSEntryType_t(STAFFSEntryType_t(entryTypesUInt) |
                          kSTAFFSDirectory),
        sortBy, caseSensitive);

    // Iterate through all the entriess

    for (; dirEnum->isValid(); dirEnum->next())
    {
        STAFFSEntryPtr entry = dirEnum->entry();

        // Check if name and extension (in the specifies case) and type
        // match the specified criteria

        if ((STAFFileSystem::matchesWildcards(entry->path().name(),
                                              namePattern, caseSensitive)) &&
            (STAFFileSystem::matchesWildcards(entry->path().extension(),
                                              extPattern, caseSensitive)) &&
            (entry->type() & STAFFSEntryType_t(entryTypesUInt)))
        {
            // This entries matches the specified criteria so add an entry
            // to the matching entry list

            entryList.push_back(entry);
        }

        // Check if entry is a directory, recursively check if any of it's
        // entries match the specified criteria

        if (entry->type() & kSTAFFSDirectory)
        {
            // Skip special directories . and ..

            if ((entry->path().name() == sPeriod) ||
                (entry->path().name() == sDoublePeriod))
               continue;
            
            recurseListDir(entry, namePattern, extPattern, entryTypesUInt,
                           caseSensitive, sortBy, entryList);
        }
    }

    return kSTAFOk;
}


STAFServiceResult convertToHex(const char *buffer,
                               unsigned int fileLength)
{
    // Convert the buffer to a hex format

    STAFBuffer<char> hexBuffer(new char[fileLength * 2],
                               STAFBuffer<char>::INIT,
                               STAFBuffer<char>::ARRAY);

    for (unsigned int i = 0, j = 0; i < fileLength; i++)
    {
        hexBuffer[j++] = HEX_TABLE[(buffer[i] >> 4) & 0xF];
        hexBuffer[j++] = HEX_TABLE[buffer[i] & 0xF];
    }

    // Return new result in hex format
    return STAFServiceResult(kSTAFOk, STAFString(hexBuffer, fileLength * 2,
                                                 STAFString::kUTF8));
}


STAFServiceResult convertLineEndings(const char *buffer,
                                     unsigned int fileLength,
                                     const STAFString &textFormat,
                                     const STAFString &orgMachine,
                                     bool isLocalRequest,
                                     bool testFlag)
{
    // Convert the line ending characters in the buffer

    // Determine the new line ending character(s) to use

    STAFString newEOL = STAFString("");

    if (textFormat.toUpperCase() == "NATIVE")
    {
        if (!isLocalRequest)
        {
            // Try to get the line separator for the originating machine.
            // If VAR resolve request fails (probably due to insufficient
            // trust), default to target system's line separator.

            // XXX: In STAF V3.x, change to get the line separator from
            //      information passed to the service.

            STAFResultPtr lineSepResult = gSTAFProcHandlePtr->submit(
                orgMachine, "VAR", "RESOLVE STRING " +
                STAFHandle::wrapData("{STAF/Config/Sep/Line}"));

            if (lineSepResult->rc == kSTAFOk)
            {
                // XXX: If the originating machine is running STAF v2.x,
                //      using the new VAR resolve string syntax does not
                //      return a 7 due to the parser assuming the rest of
                //      the string should be the value, so it line separator
                //      begins with "STRING " (which is NOT what we want).
                if (lineSepResult->result.find("STRING ") != 0)
                    newEOL = lineSepResult->result;
                else
                {
                    // Try using the old VAR resolve syntax in case the
                    // originating machine is running STAF v2.x.
                    lineSepResult = gSTAFProcHandlePtr->submit(
                        orgMachine, "VAR", "RESOLVE " +
                        STAFHandle::wrapData("{STAF/Config/Sep/Line}"));

                    if (lineSepResult->rc == kSTAFOk)
                        newEOL = lineSepResult->result;
                }
            }
        }

        if (newEOL == STAFString(""))
        {
            // Get the line ending character(s) for the system where the
            // file resides.

            STAFConfigInfo configInfo;
            STAFString_t errorBufferT;
            unsigned int osRC = 0;

            if (STAFUtilGetConfigInfo(&configInfo, &errorBufferT, &osRC)
                != kSTAFOk)
            {
                 return STAFServiceResult(
                     kSTAFBaseOSError,
                     STAFString(errorBufferT, STAFString::kShallow) +
                     ", RC: " + osRC);
            }

            // Default to the target system's line separator if target
            // system is the same as the orginating system (e.g. is local) or
            // if can't get the originating system's line separator.

            newEOL = configInfo.lineSeparator;
        }
    }
    else if (textFormat.toUpperCase() == "WINDOWS")
        newEOL = sWindowsEOL;
    else if (textFormat.toUpperCase() == "UNIX")
        newEOL = sUnixEOL;
    else
        newEOL = textFormat;

    // Create a STAFString containing the file data

    STAFString result = "";

    try
    {
        result = STAFString(buffer, fileLength);
    }
    catch (STAFException &e)
    {
        result = e.getText();

        if (e.getErrorCode() == kSTAFConverterError)
        {
            result += ": The file contains data that is not valid in the "
                "codepage that STAF is using.  To see the codepage that STAF "
                "is using, check the value of STAF variable "
                "STAF/Config/CodePage.";
        }

        return STAFServiceResult(e.getErrorCode(), result);
    }
    catch (...)
    {             
        result = "Caught unknown exception in STAFFSService::"
            "convertLineEndings() when trying to create a STAFString from "
            "the data in the file";
        return STAFServiceResult(kSTAFUnknownError, result);
    }

    // Check which line endings the file actually contains and assign
    // to currentEOL.  The first line-ending found for Windows (0D0A)
    // or Unix (0A) determines the line endings assumed for the file.
    // If no line-endings are found in the file, default to newEOL 
    // so that no line-endings conversion is done.

    STAFString currentEOL = newEOL;

    unsigned int windowsEOLIndex = result.find(sWindowsEOL);
    unsigned int unixEOLIndex = result.find(sUnixEOL);

    if (windowsEOLIndex < unixEOLIndex)
        currentEOL = sWindowsEOL;
    else if (unixEOLIndex < windowsEOLIndex)
        currentEOL = sUnixEOL;

    // If debug flag is true, prints line ending characters in hex

    bool debug = false;

    if (debug)
    {
        cout << "currentEOL in Hex: ";

        const char *buffer = currentEOL.buffer();

        for (unsigned int y = 0; y < currentEOL.length(); ++y)
        {
            unsigned int currChar = static_cast<unsigned char>(buffer[y]);
            if (currChar < 16) cout << "0";
            cout << hex << currChar << dec << " ";
        }

        cout << endl << "newEOL in Hex:  ";

        buffer = newEOL.buffer();

        for (unsigned int i = 0; i < newEOL.length(); ++i)
        {
            unsigned int currChar = static_cast<unsigned char>(buffer[i]);
            if (currChar < 16) cout << "0";
            cout << hex << currChar << dec << " ";
        }

        cout << endl << endl;
    }

    // If conversion of line ending character(s) is needed, replace the
    // current line ending character(s) with the new line ending character(s)

    if (currentEOL != newEOL)
        result = result.replace(currentEOL, newEOL);

    // If testFlag, convert the result to hex to verify that the line-ending
    // characters were converted as expected.

    if (testFlag)
        return convertToHex(result.buffer(), result.length());

    // Return new result in hex format
    return STAFServiceResult(kSTAFOk, result);
}


STAFFSService::STAFFSService() : STAFService("FS")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** FS Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "COPY   FILE <Name> [TOFILE <Name> | TODIRECTORY <Name>] "
        "[TOMACHINE <Machine>]" +
        *gLineSeparatorPtr +
        "       [TEXT [FORMAT <Format>]] [FAILIFEXISTS | FAILIFNEW]" +
        //Remove comment when enabling convert/noconvert option
        //"       [CONVERT | NOCONVERT]]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "COPY   DIRECTORY <Name> [TODIRECTORY <Name>] [TOMACHINE <Machine>]" +
        *gLineSeparatorPtr +
        "       [NAME <Pattern>] [EXT <Pattern>] "
        "[CASESENSITIVE | CASEINSENSITIVE]" +
        *gLineSeparatorPtr +
        "       [TEXTEXT <Pattern>... [FORMAT <Format>]] " +
        // Remove comment when enabling convert/noconvert option
        //"[CONVERT | NOCONVERT]]" +
        *gLineSeparatorPtr +
        "       [RECURSE [KEEPEMPTYDIRECTORIES | ONLYDIRECTORIES]]" +
        *gLineSeparatorPtr +
        "       [IGNOREERRORS] [FAILIFEXISTS | FAILIFNEW]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "MOVE   FILE <Name> <TOFILE <Name> | TODIRECTORY <Name>>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "MOVE   DIRECTORY <Name> TODIRECTORY <Name>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "GET    FILE <Name> [[TEXT | BINARY] [FORMAT <Format>]]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "GET    ENTRY <Name> <TYPE | SIZE | MODTIME | LINKTARGET" +
    #ifdef STAF_USE_SSL
        " | " +
        *gLineSeparatorPtr + 
        "                     CHECKSUM [<Algorithm>]" +
    #endif
        ">" +
        *gLineSeparatorPtr +
        "QUERY  ENTRY <Name>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "CREATE DIRECTORY <Name> [FULLPATH] [FAILIFEXISTS]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST   DIRECTORY <Name> [RECURSE] [LONG [DETAILS] | SUMMARY] "
        "[TYPE <Types>]" +
        *gLineSeparatorPtr +
        "       [NAME <Pattern>] [EXT <Pattern>] "
        "[CASESENSITIVE | CASEINSENSITIVE]" +
        *gLineSeparatorPtr +
        "       [SORTBYNAME | SORTBYSIZE | SORTBYMODTIME]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST   COPYREQUESTS [LONG] [INBOUND] [OUTBOUND]" +
        *gLineSeparatorPtr +
        "       [FILE [[BINARY] [TEXT]]] [DIRECTORY]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "LIST   SETTINGS" + *gLineSeparatorPtr + *gLineSeparatorPtr +
        "DELETE ENTRY <Name> CONFIRM [RECURSE] [IGNOREERRORS]" +
        *gLineSeparatorPtr +
        "       [ CHILDREN [TYPE <Types>] [NAME <Pattern>] [EXT <Pattern>]" +
        *gLineSeparatorPtr +
        "                  [CASESENSITIVE | CASEINSENSITIVE] ]" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "SET    STRICTFSCOPYTRUST <Enabled | Disabled>" +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // COPY FILE parser setup

    fCopyFileParser.addOption("COPY",                 1,
        STAFCommandParser::kValueNotAllowed);
    fCopyFileParser.addOption("FILE",                 1,
        STAFCommandParser::kValueRequired);
    fCopyFileParser.addOption("TOFILE",               1,
        STAFCommandParser::kValueRequired);
    fCopyFileParser.addOption("TODIRECTORY",          1,
        STAFCommandParser::kValueRequired);
    fCopyFileParser.addOption("TOMACHINE",            1,
        STAFCommandParser::kValueRequired);
    fCopyFileParser.addOption("FORMAT",               1,
        STAFCommandParser::kValueRequired);
    fCopyFileParser.addOption("TEXT",                 1,
        STAFCommandParser::kValueNotAllowed);
    // Remove comment when enabling convert/noconvert option
    /*fCopyFileParser.addOption("CONVERT",              1,
        STAFCommandParser::kValueNotAllowed);
    fCopyFileParser.addOption("NOCONVERT",            1,
        STAFCommandParser::kValueNotAllowed);*/
    fCopyFileParser.addOption("FAILIFEXISTS",         1,
        STAFCommandParser::kValueNotAllowed);
    fCopyFileParser.addOption("FAILIFNEW",            1,
        STAFCommandParser::kValueNotAllowed);

    fCopyFileParser.addOptionGroup("FAILIFEXISTS FAILIFNEW", 0, 1);
    fCopyFileParser.addOptionGroup("TOFILE TODIRECTORY", 0, 1);

    // Remove comment when enabling convert/noconvert option
    //fCopyFileParser.addOptionGroup("CONVERT NOCONVERT", 0, 1);

    fCopyFileParser.addOptionNeed("FORMAT", "TEXT");

    // Remove comment when enabling convert/noconvert option
    //fCopyFileParser.addOptionNeed("CONVERT", "TEXT");
    //fCopyFileParser.addOptionNeed("NOCONVERT", "TEXT");
    fCopyFileParser.addOptionNeed("COPY", "FILE");

    // COPY DIRECTORY parser setup

    fCopyDirParser.addOption("COPY",                 1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("DIRECTORY",            1,
        STAFCommandParser::kValueRequired);
    fCopyDirParser.addOption("TODIRECTORY",          1,
        STAFCommandParser::kValueRequired);
    fCopyDirParser.addOption("TOMACHINE",            1,
        STAFCommandParser::kValueRequired);
    fCopyDirParser.addOption("NAME",                 1,
        STAFCommandParser::kValueRequired);
    fCopyDirParser.addOption("EXT",                  1,
        STAFCommandParser::kValueRequired);
    fCopyDirParser.addOption("TEXTEXT",              0,
        STAFCommandParser::kValueRequired);
    fCopyDirParser.addOption("FORMAT",               1,
        STAFCommandParser::kValueRequired);
    // Remove comment when enabling convert/noconvert option
    /*fCopyDirParser.addOption("CONVERT",              1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("NOCONVERT",            1,
        STAFCommandParser::kValueNotAllowed);*/
    fCopyDirParser.addOption("RECURSE",              1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("CASESENSITIVE",        1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("CASEINSENSITIVE",      1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("KEEPEMPTYDIRECTORIES", 1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("ONLYDIRECTORIES",      1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("IGNOREERRORS",         1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("FAILIFEXISTS",         1,
        STAFCommandParser::kValueNotAllowed);
    fCopyDirParser.addOption("FAILIFNEW",            1,
        STAFCommandParser::kValueNotAllowed);

    fCopyDirParser.addOptionGroup("CASESENSITIVE CASEINSENSITIVE", 0, 1);
    fCopyDirParser.addOptionGroup("KEEPEMPTYDIRECTORIES ONLYDIRECTORIES", 0, 1);
    fCopyDirParser.addOptionGroup("FAILIFEXISTS FAILIFNEW", 0, 1);
    // Remove comment when enabling convert/noconvert option
    //fCopyDirParser.addOptionGroup("CONVERT NOCONVERT", 0, 1);

    fCopyDirParser.addOptionNeed("FORMAT", "TEXTEXT");
    // Remove comment when enabling convert/noconvert option
    //fCopyDirParser.addOptionNeed("CONVERT", "TEXTEXT");
    //fCopyDirParser.addOptionNeed("NOCONVERT", "TEXTEXT");
    fCopyDirParser.addOptionNeed("KEEPEMPTYDIRECTORIES ONLYDIRECTORIES",
                              "RECURSE");
    fCopyDirParser.addOptionNeed("COPY", "DIRECTORY");

    // MOVE FILE parser setup

    fMoveFileParser.addOption("MOVE", 1, STAFCommandParser::kValueNotAllowed);
    fMoveFileParser.addOption("FILE", 1, STAFCommandParser::kValueRequired);
    fMoveFileParser.addOption("TOFILE", 1, STAFCommandParser::kValueRequired);
    fMoveFileParser.addOption("TODIRECTORY", 1, STAFCommandParser::kValueRequired);
    fMoveFileParser.addOptionGroup("TOFILE TODIRECTORY", 1, 1);
    fMoveFileParser.addOptionNeed("MOVE", "FILE");

    // MOVE DIRECTORY parser setup

    fMoveDirParser.addOption("MOVE", 1, STAFCommandParser::kValueNotAllowed);
    fMoveDirParser.addOption(
        "DIRECTORY", 1, STAFCommandParser::kValueRequired);
    fMoveDirParser.addOption(
        "TODIRECTORY", 1, STAFCommandParser::kValueRequired);
    fMoveDirParser.addOptionNeed("MOVE", "DIRECTORY");
    fMoveDirParser.addOptionNeed("DIRECTORY", "TODIRECTORY");

    // GET parser setup

    fGetParser.addOption("GET",              1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("FILE",             1,
        STAFCommandParser::kValueRequired);

    fGetParser.addOption("TEXT",             1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("BINARY",           1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("FORMAT",           1,
        STAFCommandParser::kValueRequired);

    // Note that TEST is an undocumented option that we can use for testing
    // that the line ending character really got converted correctly
    fGetParser.addOption("TEST",           1,
     STAFCommandParser::kValueNotAllowed);

    fGetParser.addOption("ENTRY",            1,
        STAFCommandParser::kValueRequired);
    fGetParser.addOption("TYPE",             1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("SIZE",             1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("MODTIME",          1,
        STAFCommandParser::kValueNotAllowed);
    fGetParser.addOption("LINKTARGET",       1,
        STAFCommandParser::kValueNotAllowed);
#ifdef STAF_USE_SSL
    fGetParser.addOption("CHECKSUM",         1,
        STAFCommandParser::kValueAllowed);
#endif

    fGetParser.addOptionGroup("FILE ENTRY", 1, 1);

    fGetParser.addOptionGroup("TEXT BINARY", 0, 1);
    fGetParser.addOptionNeed("TEXT BINARY", "FILE");
    fGetParser.addOptionNeed("FORMAT", "TEXT BINARY");
    fGetParser.addOptionNeed("TEST", "FILE");

    fGetParser.addOptionNeed("GET", "FILE ENTRY");
#ifdef STAF_USE_SSL
    fGetParser.addOptionNeed("TYPE SIZE MODTIME LINKTARGET CHECKSUM", "ENTRY");
    fGetParser.addOptionNeed("ENTRY", "TYPE SIZE MODTIME LINKTARGET CHECKSUM");
#else
    fGetParser.addOptionNeed("TYPE SIZE MODTIME LINKTARGET", "ENTRY");
    fGetParser.addOptionNeed("ENTRY", "TYPE SIZE MODTIME LINKTARGET");
#endif
    
    // LIST parser setup

    fListParser.addOption(
        "LIST", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "DIRECTORY", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption(
        "NAME", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption(
        "EXT", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption(
        "LONG", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "DETAILS", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "SUMMARY", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "SORTBYNAME", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "SORTBYSIZE", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "SORTBYMODTIME", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "CASESENSITIVE", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "CASEINSENSITIVE", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "TYPE", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption(
        "RECURSE", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption(
        "SETTINGS", 1, STAFCommandParser::kValueNotAllowed);

    fListParser.addOptionGroup("DIRECTORY SETTINGS", 1, 1);
    fListParser.addOptionGroup("SORTBYNAME SORTBYSIZE SORTBYTIME", 0, 1);
    fListParser.addOptionGroup("CASESENSITIVE CASEINSENSITIVE", 0, 1);
    fListParser.addOptionGroup("LONG SUMMARY", 0, 1);

    fListParser.addOptionNeed("LIST", "DIRECTORY SETTINGS");
    fListParser.addOptionNeed(
        "NAME EXT LONG SUMMARY SORTBYNAME SORTBYSIZE SORTBYMODTIME TYPE RECURSE",
        "DIRECTORY");
    fListParser.addOptionNeed("DETAILS", "LONG");
    fListParser.addOptionNeed("CASESENSITIVE CASEINSENSITIVE",
                              "NAME EXT SORTBYNAME");

    // LIST COPYREQUESTS parser setup

    fListCopyRequestsParser.addOption(
        "LIST", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "COPYREQUESTS", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "INBOUND", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "OUTBOUND", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "FILE", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "DIRECTORY", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "BINARY", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "TEXT", 1, STAFCommandParser::kValueNotAllowed);
    fListCopyRequestsParser.addOption(
        "LONG", 1, STAFCommandParser::kValueNotAllowed);
    
    fListCopyRequestsParser.addOptionNeed("BINARY TEXT", "FILE");

    // CREATE parser setup

    fCreateParser.addOption("CREATE",           1,
        STAFCommandParser::kValueNotAllowed);
    fCreateParser.addOption("DIRECTORY",        1,
        STAFCommandParser::kValueRequired);
    fCreateParser.addOption("FULLPATH",         1,
        STAFCommandParser::kValueNotAllowed);
    fCreateParser.addOption("FAILIFEXISTS",     1,
        STAFCommandParser::kValueNotAllowed);

    fCreateParser.addOptionNeed("CREATE", "DIRECTORY");

    // DELETE parser setup

    fDeleteParser.addOption("DELETE",           1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("ENTRY",            1,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("CHILDREN",         1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("NAME",             1,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("EXT",              1,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("TYPE",             1,
        STAFCommandParser::kValueRequired);
    fDeleteParser.addOption("CASESENSITIVE",    1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("CASEINSENSITIVE",  1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("RECURSE",          1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("CONFIRM",          1,
        STAFCommandParser::kValueNotAllowed);
    fDeleteParser.addOption("IGNOREERRORS",     1,
        STAFCommandParser::kValueNotAllowed);

    fDeleteParser.addOptionGroup("CASESENSITIVE CASEINSENSITIVE", 0, 1);

    fDeleteParser.addOptionNeed("DELETE", "ENTRY");
    fDeleteParser.addOptionNeed("DELETE", "CONFIRM");
    fDeleteParser.addOptionNeed("NAME EXT TYPE", "CHILDREN");
    fDeleteParser.addOptionNeed("CASESENSITIVE CASEINSENSITIVE", "NAME EXT");
    fDeleteParser.addOptionNeed("IGNOREERRORS", "CHILDREN RECURSE");

    // QUERY parser setup

    fQueryParser.addOption("QUERY",            1,
        STAFCommandParser::kValueNotAllowed);
    fQueryParser.addOption("ENTRY",            1,
        STAFCommandParser::kValueRequired);

    fQueryParser.addOptionNeed("QUERY", "ENTRY");

    // SET parser setup

    fSetParser.addOption("SET", 1,
                         STAFCommandParser::kValueNotAllowed);
    fSetParser.addOption("STRICTFSCOPYTRUST", 1,
                         STAFCommandParser::kValueRequired);

    // set groups/needs

    fSetParser.addOptionGroup("STRICTFSCOPYTRUST", 1, 1);

    // Construct map-class for a LIST DIRECTORY LONG information

    fListLongInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/ListLongInfo");
 
    fListLongInfoClass->addKey("type", "Type");
    fListLongInfoClass->addKey("size", "Size");
    fListLongInfoClass->addKey("lastModifiedTimestamp",
                               "Modified Date-Time");
    fListLongInfoClass->addKey("name", "Name");
    fListLongInfoClass->addKey("linkTarget", "Link Target");
    fListLongInfoClass->setKeyProperty(
        "linkTarget", "display-short-name", "Link");
    
    // Construct map-class for LIST DIRECTORY LONG DETAILS information

    fListDetailsInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/ListDetailsInfo");
 
    fListDetailsInfoClass->addKey("name", "Name");
    fListDetailsInfoClass->addKey("linkTarget", "Link Target");
    fListDetailsInfoClass->setKeyProperty(
        "linkTarget", "display-short-name", "Link");
    fListDetailsInfoClass->addKey("type", "Type");
    fListDetailsInfoClass->addKey("size", "Size");
    fListDetailsInfoClass->addKey("upperSize", "U-Size");
    fListDetailsInfoClass->addKey("lowerSize", "L-Size");
    fListDetailsInfoClass->addKey("lastModifiedTimestamp",
                                  "Modified Date-Time");

    // Construct map-class for a LIST DIRECTORY SUMMARY information

    fListSummaryInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/ListSummaryInfo");
 
    fListSummaryInfoClass->addKey("name", "Name");
    fListSummaryInfoClass->addKey("size", "Size");
    fListSummaryInfoClass->addKey("numFiles", "Files");
    fListSummaryInfoClass->addKey("numDirectories", "Directories");

    // Construct map class for LIST COPYREQUESTS information

    fCopyRequestClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/CopyRequest");

    fCopyRequestClass->addKey("startTimestamp", "Start Date-Time");
    fCopyRequestClass->setKeyProperty(
        "timestamp", "display-short-name", "Date-Time");
    fCopyRequestClass->addKey("io", "In/Out");
    fCopyRequestClass->setKeyProperty(
        "io", "display-short-name", "I/O");
    fCopyRequestClass->addKey("machine", "Machine");
    fCopyRequestClass->addKey("name", "Name");
    fCopyRequestClass->addKey("type", "Type");

    // Construct map class for COPYREQUESTS LONG information for a file copy

    fCopyFileClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/CopyFile");

    fCopyFileClass->addKey("startTimestamp", "Start Date-Time");
    fCopyFileClass->setKeyProperty(
        "timestamp", "display-short-name", "Date-Time");
    fCopyFileClass->addKey("io", "In/Out");
    fCopyFileClass->setKeyProperty(
        "io", "display-short-name", "I/O");
    fCopyFileClass->addKey("machine", "Machine");
    fCopyFileClass->addKey("name", "File Name");
    fCopyFileClass->addKey("type", "Type");
    fCopyFileClass->addKey("mode", "Mode");
    fCopyFileClass->addKey("state", "Transfer State");

    // Construct map class for the state of a file copy request

    fFileCopyStateClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/FileCopyState");
    fFileCopyStateClass->addKey("fileSize", "File Size");
    fFileCopyStateClass->addKey("bytesCopied", "Bytes Copied");

    // Construct map class for COPYREQUESTS LONG information for a directory
    
    fCopyDirectoryClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/CopyDirectory");

    fCopyDirectoryClass->addKey("startTimestamp", "Start Date-Time");
    fCopyDirectoryClass->setKeyProperty(
        "timestamp", "display-short-name", "Date-Time");
    fCopyDirectoryClass->addKey("io", "In/Out");
    fCopyDirectoryClass->setKeyProperty(
        "io", "display-short-name", "I/O");
    fCopyDirectoryClass->addKey("machine", "Machine");
    fCopyDirectoryClass->addKey("name", "Directory Name");
    fCopyDirectoryClass->addKey("type", "Type");
    fCopyDirectoryClass->addKey("state", "Transfer State");

    // Construct map class for the state of a directory copy request

    fDirectoryCopyStateClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/DirectoryCopyState");
    fDirectoryCopyStateClass->addKey("name", "Name");
    fDirectoryCopyStateClass->addKey("mode", "Mode");
    fDirectoryCopyStateClass->addKey("fileSize", "File Size");
    fDirectoryCopyStateClass->addKey("bytesCopied", "Bytes Copied");
    
    // Construct map class for LIST SETTINGS information

    fSettingsClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/Settings");

    fSettingsClass->addKey("strictFSCopyTrust", "Strict FS Copy Trust");

    // Construct map-class for GET ENTRY SIZE information

    fEntrySizeInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/SizeInfo");
 
    fEntrySizeInfoClass->addKey("size", "Size");
    fEntrySizeInfoClass->addKey("upperSize", "Upper 32-bit Size");
    fEntrySizeInfoClass->addKey("lowerSize", "Lower 32-bit Size");

    // Construct map-class for QUERY ENTRY information

    fQueryInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/QueryInfo");
 
    fQueryInfoClass->addKey("name", "Name");
    fQueryInfoClass->addKey("linkTarget", "Link Target");
    fQueryInfoClass->addKey("type", "Type");
    fQueryInfoClass->addKey("size", "Size");
    fQueryInfoClass->addKey("upperSize", "Upper 32-bit Size");
    fQueryInfoClass->addKey("lowerSize", "Lower 32-bit Size");
    fQueryInfoClass->addKey("lastModifiedTimestamp", "Modified Date-Time");

    // Construct map-class for error information on a copy/delete

    fErrorInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/FS/ErrorInfo");
 
    fErrorInfoClass->addKey("name", "Name");
    fErrorInfoClass->addKey("rc", "RC");
    fErrorInfoClass->addKey("osRC", "OS RC");

#ifdef STAF_USE_SSL
    // Perform SSL Thread Setup and assign callback functions needed for
    // multi-threaded applications that use OpenSSL
    // Note:  Need to do this because the FS service uses OpenSSL functions
    //        for determining the checksum of files

    STAF_SSL_Thread_Setup();
#endif
}


STAFFSService::~STAFFSService()
{
#ifdef STAF_USE_SSL
    // Free SSL Thread callbacks and mutexes used by the locking
    // callback function
    // Note:  Need to do this because the FS service uses OpenSSL functions
    //        for determining the checksum of files

    STAF_SSL_Thread_Cleanup();
#endif
}


STAFString STAFFSService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFFSService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).toLowerCase();
    STAFString subAction = requestInfo.fRequest.subWord(1, 1).toLowerCase();

    if (action == "copy")
    {
        if (subAction == "file")
            return handleCopyFile(requestInfo);
        else if (subAction == "directory")
            return handleCopyDir(requestInfo);
        else
        {
            STAFString errMsg = STAFString("'") +
                requestInfo.fRequest.subWord(0, 1) + " " +
                requestInfo.fRequest.subWord(1, 1) +
                "' is not a valid command request for the " + name() +
                " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
                sHelpMsg;

            return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
        }
    }
    else if (action == "move")
    {
        if (subAction == "file")
            return handleMove(true, requestInfo);
        else if (subAction == "directory")
            return handleMove(false, requestInfo);
        else
        {
            STAFString errMsg = STAFString("'") +
                requestInfo.fRequest.subWord(0, 1) + " " +
                requestInfo.fRequest.subWord(1, 1) +
                "' is not a valid command request for the " + name() +
                " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
                sHelpMsg;

            return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
        }
    }
    else if (action == "get")    return handleGet(requestInfo);
    else if (action == "list")
    {
        if (subAction == "copyrequests")
            return handleListCopyRequests(requestInfo);
        else
            return handleList(requestInfo);
    }
    else if (action == "create") return handleCreate(requestInfo);
    else if (action == "delete") return handleDelete(requestInfo);
    else if (action == "query")  return handleQuery(requestInfo);
    else if (action == "set")    return handleSet(requestInfo);
    else if (action == "help")   return handleHelp(requestInfo);
    else
    {
        STAFString errMsg = STAFString("'") +
            requestInfo.fRequest.subWord(0, 1) +
            "' is not a valid command request for the " + name() +
            " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
            sHelpMsg;

        return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
    }
}


STAFServiceResult STAFFSService::handleCopyFile(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "COPY");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fCopyFileParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString   fromMachine = requestInfo.fEndpoint;
    STAFHandle_t handle      = requestInfo.fHandle;
    STAFString   request     = requestInfo.fRequest;
    STAFRC_t     rc          = kSTAFOk;
    STAFString   toMachine   = fromMachine;
    STAFString   toFile;
    STAFString   fromFile;
    STAFString   errorBuffer;

    rc = RESOLVE_STRING_OPTION("FILE", fromFile);

    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("TOMACHINE", toMachine);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // if user specified TOFILE value
    if (parsedResult->optionTimes("TOFILE") != 0)
    {
        toFile = parsedResult->optionValue("TOFILE");
    }
    else if (parsedResult->optionTimes("TODIRECTORY") != 0)
    {
        // Copy the file to this directory using the same file name as
        // the "from" file name
        toFile = parsedResult->optionValue("TODIRECTORY");
    }
    else
    {
        toFile = parsedResult->optionValue("FILE");
    }

    // Note:  We'll resolve any variables in TOFILE on the TOMACHINE not
    //        here on the from machine.

    // Check if the from file exists

    STAFFSPath fromPath(fromFile);

    try
    {
        if (!fromPath.exists())
            return STAFServiceResult(
                kSTAFDoesNotExist, "File " + fromFile + " does not exist");
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = "Error on From File: " + fromFile + "\n" +
            e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }

    // Get the actual path name and remove any unnecessary trailing slashes
    fromFile = fromPath.setRoot(fromPath.root()).asString();

    // If TODIRECTORY is specified, need to add the from file name to the path
    if (parsedResult->optionTimes("TODIRECTORY") != 0)
    {
        toFile = toFile + "/" + fromPath.name();

        if (fromPath.extension() != "")
            toFile = toFile + "." + fromPath.extension();
    }
    
    // determine if transfer is text or binary
    bool doText = false;
    bool doCodepage = false;
    STAFString newEOL = "Native";

    if (parsedResult->optionTimes("TEXT") > 0)
    {
        doText = true;
        doCodepage = true;

        rc = RESOLVE_OPTIONAL_STRING_OPTION("FORMAT", newEOL);
        if (rc) return STAFServiceResult(rc, errorBuffer);

        // If enable NOCONVERT option, uncomment the  following line
        //doCodepage = (parsedResult->optionTimes("NOCONVERT") == 0);
    }

    fstream inFile;

    // open the file as binary
    inFile.open(fromFile.toCurrentCodePage()->buffer(),
                 ios::in | STAF_ios_binary);

    if (!inFile)
        return STAFServiceResult(kSTAFFileOpenError, fromFile);

    // Get the size of the file (upperSize and lowerSize).
    // If the file size is < 4G, upperSize will be zero.
    
    STAFFSEntryPtr entry;

    try
    {
        entry = fromPath.getEntry();
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = "Error on From File: " + fromFile + "\n" +
            e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }

    unsigned int upperSize = entry->size().first;
    unsigned int lowerSize = entry->size().second;

    // Check if file size exceeds the maximum that the FS service handles

    if (upperSize > 0)
    {
        STAFString errMsg = STAFString(
            "File size exceeds the maximum size (") + UINT_MAX +
            ") supported.  File name: " + fromFile; 
        return STAFServiceResult(kSTAFFileReadError, errMsg);
    }

    unsigned int fileLength = lowerSize;

    unsigned char fileBuffer[4000] = { 0 };
    unsigned int writeLength = 0;
    STAFConnectionPtr connection;
    STAFConnectionProviderPtr provider;
    STAFString result;
    unsigned int useNewAPI = 1;
    unsigned int levelToUse = 0;
    unsigned int flags = 0;

    if (parsedResult->optionTimes("FAILIFEXISTS") != 0)
        flags |= 0x00000001;
    else if (parsedResult->optionTimes("FAILIFNEW") != 0)
        flags |= 0x00000002;

    // If interface cycling is not enabled, and toMachine is not 'local',
    // and neither interface nor port are specified in toMachine, get the
    // originator's interface and prepend to the toMachine value so that
    // the connection to the toMachine will use the orginator's interface.

    if ((!gConnectionManagerPtr->getAutoInterfaceCycling()) &&
        (!isLocalMachine(toMachine, 1)) &&
        (toMachine.find(gSpecSeparator == STAFString::kNPos)) &&
        (toMachine.find(kUTF8_AT) == STAFString::kNPos))
    {
        unsigned int specSepPos = requestInfo.fEndpoint.find(gSpecSeparator);

        if (specSepPos != STAFString::kNPos)
        {
            STAFString interface = requestInfo.fEndpoint.subString(
                0, specSepPos);

            if (interface != "local")
            {
                // Prepend the interface from the originator's endpoint
                toMachine = interface + gSpecSeparator + toMachine;
            }
        }
    }
    
    // This flag indicates if copyDataPtr has been initialized yet by calling
    // gFSCopyManagerPtr->add() and passing it copyDataPtr

    bool addedFSCopyData = false;

    STAFFSCopyManager::FSCopyDataPtr copyDataPtr;
    
    try
    {
        // Make a connection to the toMachine

        try
        {
            rc = gConnectionManagerPtr->makeConnection(
                toMachine, provider, connection, result);
        }
        catch (STAFConnectionProviderConnectException &e)
        {
            rc = kSTAFNoPathToMachine;
            result = e.getText() + STAFString(": ") +
                STAFString(e.getErrorCode());
        }

        if ((rc == kSTAFNoPathToMachine) &&
            (toMachine.find(kUTF8_AT) == STAFString::kNPos))
        {
            // Retry connecting to the toMachine, this time using the
            // originator's port

            unsigned int atPos = requestInfo.fEndpoint.find(kUTF8_AT);
        
            if (atPos != STAFString::kNPos)
            {
                toMachine = toMachine + requestInfo.fEndpoint.subString(atPos);

                rc = gConnectionManagerPtr->makeConnection(
                    toMachine, provider, connection, result);
            }
        }
        
        if (rc) return STAFServiceResult(rc, result);
        
        // First, lets try the new API

        connection->writeUInt(kSTAFFileTransferAPI2);  // New API number
        connection->writeUInt(0);    // Dummy level

        STAFRC_t ack = connection->readUInt();

        if (ack != kSTAFOk)
        {
            // They don't support the new API, so try the old API
            useNewAPI = 0;

            rc = gConnectionManagerPtr->makeConnection(
                toMachine, connection, result);

            if (rc) return STAFServiceResult(rc, result);

            connection->writeUInt(kSTAFFileTransferAPI);  // Old API Number
            connection->writeUInt(0);    // API Level

            ack = connection->readUInt();

            if (ack != kSTAFOk) return ack;
        }
        else
        {
            // Now find out the specific level to use
            unsigned int minLevel = 1;
            unsigned int maxLevel = 3;

            connection->writeUInt(minLevel);
            connection->writeUInt(maxLevel);

            levelToUse = connection->readUInt();

            if (levelToUse == 0) return kSTAFInvalidAPILevel;
        }

        // If the other machine can do a text transfer, specify text or binary

        if (useNewAPI && (levelToUse > 1))
            if (doText)
            {
                if (doCodepage)
                    connection->writeUInt(kSTAFFSTextConvert);
                else
                    connection->writeUInt(kSTAFFSTextNoConvert);
            }
            else
            {
                connection->writeUInt(kSTAFFSBinary);
            }

        connection->writeString(*gMachinePtr);
        connection->writeString(requestInfo.fMachine);
        connection->writeString(toFile);

        if (!(useNewAPI && (levelToUse > 1)) && doText)
        {
            // Fail and abort
            flags |= (0x00000001 | 0x00000002);

            // Send bad flags 0x1|0x2 // fail if new | fail if exist
            // to kill other side
            connection->writeUInt(flags);

            // Read kill confirmation
            ack = static_cast<STAFRC_t>(connection->readUInt());
            result = connection->readString();
            inFile.close();
            return kSTAFInvalidAPILevel;
        }
        
        // Add an entry to the FS Copy Map so can list copies in progress

        int copyMode = kSTAFFSBinary;

        if (doCodepage)
            copyMode = kSTAFFSTextConvert;   // Text, cp conversion
        else if (doText)
            copyMode = kSTAFFSTextNoConvert; // Text, no cp conversion

        if (gFSCopyManagerPtr->add(fromFile, toMachine, kSTAFFSCopyFrom,
                                   kSTAFFSFileCopy, copyMode,
                                   fileLength, copyDataPtr))
        {
            return STAFServiceResult(
                kSTAFFileReadError, "Cannot read from a file that another "
                "copy request is currently writing to");
        }

        addedFSCopyData = true;

        // Write the flags

        connection->writeUInt(flags);

        if (useNewAPI && levelToUse > 2)
        {
            connection->writeUInt(requestInfo.fHandle);

            if (requiresSecureConnection(requestInfo.fAuthenticator) &&
                (provider->getProperty(
                    kSTAFConnectionProviderIsSecureProperty) != "1"))
            {
                // Don't send authentication data since non-secure
                // connection.  Instead set authenticator to "none" and
                // user to "anonymous" and set authentication data to ""

                connection->writeString(gNoneString);
                connection->writeString(gAnonymousString);
                connection->writeString("");
            }
            else
            {
                connection->writeString(requestInfo.fAuthenticator);
                connection->writeString(requestInfo.fUserIdentifier);
                connection->writeString(requestInfo.fAuthenticationData);
            }

            connection->writeString(requestInfo.fInterface);
            connection->writeString(requestInfo.fLogicalInterfaceID);
            connection->writeString(requestInfo.fPhysicalInterfaceID);
            connection->writeString(requestInfo.fSTAFInstanceUUID);
            connection->writeString(*gSTAFInstanceUUIDPtr);

            // Write request var pool

            STAFVariablePool::VariableMap requestVarMap =
                requestInfo.fRequestVarPool->getVariableMapCopy();

            connection->writeUInt(requestVarMap.size());

            for (STAFVariablePool::VariableMap::iterator requestIter =
                 requestVarMap.begin();
                 requestIter != requestVarMap.end();
                 ++requestIter)
            {
                connection->writeString(requestIter->second.name);
                connection->writeString(requestIter->second.value);
            }

            // Write source shared var pool

            STAFVariablePool::VariableMap sourceSharedVarMap =
                requestInfo.fSourceSharedVarPool->getVariableMapCopy();

            connection->writeUInt(sourceSharedVarMap.size());

            for (STAFVariablePool::VariableMap::iterator sourceSharedIter =
                     sourceSharedVarMap.begin();
                 sourceSharedIter != sourceSharedVarMap.end();
                 ++sourceSharedIter)
            {
                connection->writeString(sourceSharedIter->second.name);
                connection->writeString(sourceSharedIter->second.value);
            }
        }

        // Check if can open and write to the file

        ack = static_cast<STAFRC_t>(connection->readUInt());
        result = connection->readString();

        if (ack != kSTAFOk)
        {
            gFSCopyManagerPtr->remove(copyDataPtr);

            return STAFServiceResult(ack, result);
        }

        STAFString currentEOL = STAFString("");

        if (doText)
        {
            // Determine what line ending the from file contains

            STAFServiceResult result = determineLineEnding(
                fileLength, inFile, fromFile, toMachine, currentEOL);

            if (result.fRC != kSTAFOk)
            {
                gFSCopyManagerPtr->remove(copyDataPtr);

                return result;
            }
            
            currentEOL = result.fResult;

            // send the new EOL marker across
            connection->writeString(newEOL);

            // verify ok new eol
            if (connection->readUInt() == kSTAFFSStopCopy)
            {
                gFSCopyManagerPtr->remove(copyDataPtr);

                return STAFServiceResult(kSTAFBaseOSError,
                                        "Get current EOL failure");
            }

            newEOL = connection->readString();

            if (newEOL.isEqualTo(currentEOL))
            {
                // Allows changing to use binary transfer (which is faster) if
                // the eol on both sides are the same and no convert is needed.
                
                connection->writeUInt(kSTAFFSBinary);
                doText = false;
            }
            else
                connection->writeUInt(kSTAFFSTextNoConvert);
        }
        
        if (doCodepage)
        {
            // Check if the codepages on both sides are the same because then
            // can use the text noconvert routine which is faster.
            
            // Get this codepage id
            STAFString cPage;
            rc = RESOLVE_STRING("{STAF/Config/CodePage}", cPage);

            // Get other codepage id
            STAFString otherCPage = connection->readString();

            if (cPage.isEqualTo(otherCPage))
            {
                // XXX: Change next line to kSTAFFSTextNoConvert if decide not
                //      to force codepage conversion on all text transfers
                connection->writeUInt(kSTAFFSTextConvert);

                // XXX: Uncomment next line to enable the codepage filter. 
                //      Currently, commented out to force codepage conversion
                //      on all text transfers.
                //doCodepage = false;
            }
            else
                connection->writeUInt(kSTAFFSTextConvert);
        }

        /************** Start transferring the file ***************/

        if (doCodepage)
        {
            // Perform a text transfer with conversion of eol chars and a
            // codepage conversion.  It runs considerably slower than the
            // binary transfer.  This is the type of transfer that will 
            // occur if the TEXT option is specified and the codepages are
            // different.  Data is sent as a series of buffers of UTF8 chars.
            
            gFSCopyManagerPtr->updateFileCopy1(
                copyDataPtr, fileLength, kSTAFFSTextConvert);

            unsigned int bytesCopied = 0;
            
            connection->writeString(currentEOL);
            
            if (fileLength > 0)
            {
                STAFStringBufferPtr eolStr = currentEOL.toCurrentCodePage();

                // How many bytes in the end-of-line sequence
                unsigned int eolBufferSize = eolStr->length();

                // Points to a buffer containing end-of-line sequence
                char *eolBuffer = new char[eolBufferSize];
                memcpy(eolBuffer, eolStr->buffer(), eolBufferSize);

                // Last byte of end-of-line sequence
                char eolLastChar = eolBuffer[eolBufferSize - 1];

                const unsigned int sBufferSize = 4096;

                STAFRefPtr<char> buffer = STAFRefPtr<char>
                    (new char[sBufferSize], STAFRefPtr<char>::INIT,
                     STAFRefPtr<char>::ARRAY);
                unsigned int bufferSize = sBufferSize;
                unsigned int readOffset = 0;  // Buffer read offset
                bool done = false;

                while (!done)
                {
                    rc = readFile(
                        inFile, static_cast<char *>(buffer + readOffset),
                        bufferSize - readOffset, fromFile,
                        toMachine, fileLength, bytesCopied);

                    if (rc != kSTAFOk)
                    {
                        result = "Unrecoverable read error occurred while"
                                 " copying file " + fromFile + " in text mode";
                        break;
                    }
                
                    unsigned int bytesInBuffer = inFile.gcount() + readOffset;

                    bytesCopied += inFile.gcount();
                    
                    if (bytesInBuffer < bufferSize) done = true;
                    
                    // Find a newline.  Make sure we don't underrun the buffer
                    
                    unsigned int i = 0;
                    unsigned int guardIndex = eolBufferSize - 1;
                     
                    if (bytesInBuffer > 0)
                    {
                        i = bytesInBuffer - 1;  // Last NewLine index

                        while (((buffer[i] != eolLastChar) ||
                                !memcmp(buffer + i - eolBufferSize,
                                        eolBuffer, eolBufferSize)) &&
                               (i > guardIndex))
                        { --i; }
                    }

                    while ((i == guardIndex) && !done)
                    {
                        // We have a line bigger than our buffer.
                        // Note: the beginning of the buffer may be a lone
                        // newline, but we ignore that for this algorithm
                        // (as the next line is likely larger than the buffer
                        // anyway.

                        // First, create a buffer that is double our current
                        // size, and copy our existing buffer data into it

                        STAFRefPtr<char> tmpBuffer = STAFRefPtr<char>
                            (new char[bufferSize * 2], STAFRefPtr<char>::INIT,
                             STAFRefPtr<char>::ARRAY);
                        
                        memcpy(tmpBuffer, buffer, bufferSize);
                        buffer = tmpBuffer;
                        bufferSize *= 2;

                        // Now, read in data to fill remainder of the buffer
                    
                        rc = readFile(inFile, buffer + (bufferSize / 2),
                                      bufferSize / 2, fromFile, toMachine,
                                      fileLength, bytesCopied);

                        if (rc != kSTAFOk)
                        {
                            result = "Unrecoverable read error occurred while"
                                " copying file " + fromFile + " in text mode";
                            break;
                        }

                        bytesInBuffer += inFile.gcount();
                        bytesCopied += inFile.gcount();

                        // Finally, let's check to make sure that this buffer
                        // was big enough by finding a newline.  Otherwise,
                        // let's run the loop again.

                        if (bytesInBuffer < bufferSize) done = true;

                        i = 0;
                        
                        if (bytesInBuffer > 0)
                        {
                            i = bytesInBuffer - 1;  // Last NewLine index
                            guardIndex = (bufferSize / 2) - eolBufferSize;

                            while (((buffer[i] != eolLastChar) ||
                                    !memcmp(buffer + i - eolBufferSize,
                                            eolBuffer, eolBufferSize)) &&
                                   (i > guardIndex))
                            { --i; }
                        }

                    } // while ((i == guardIndex) && !done)

                    // We now have the last newline in the buffer

                    if (rc == kSTAFOk)
                    {
                        if (!done)
                        {
                            connection->writeUInt(kSTAFFSContinueCopy);

                            connection->writeString(
                                STAFString(buffer, i + 1,
                                           STAFString::kCurrent));

                            memmove(buffer, buffer + i + 1,
                                    bufferSize - i - 1);

                            readOffset = bufferSize - i - 1;
                        }
                        else
                        {
                            connection->writeUInt(kSTAFFSContinueCopy);

                            connection->writeString(
                                STAFString(buffer, bytesInBuffer, 
                                           STAFString::kCurrent));
                        }

                        gFSCopyManagerPtr->updateFileCopy(
                            copyDataPtr, bytesCopied);
                    }
                } // while (!done)

                delete[] eolBuffer;

            } // fileLength > 0

            connection->writeUInt(kSTAFFSFinishedCopy);

            inFile.close();

            if (rc == kSTAFOk)
            {
                // Read an ack, so that we know the file is closed and
                // if the file was copied successfully
                rc = connection->readUInt();
            }
        }
        else if (doText)
        {
            // Text file copy without codepage conversion
            // Perform a text transfer with conversion of eol chars without a 
            // codepage conversion. It runs considerably faster than the
            // codepage conversion transfer.
            // - This is the type of transfer that will occur if the NOCONVERT
            //   option is enabled
            // - This is the type of transfer that will occur if the codepages
            //   are the same and a text transfer has been specified
            
            gFSCopyManagerPtr->updateFileCopy1(
                copyDataPtr, fileLength, kSTAFFSTextNoConvert);

            STAFString transferString;
            int bufferSize = 3000;
            unsigned int transferLen = 0;
            char *buffer = new char[bufferSize];
            unsigned int bytesCopied = 0;
            
            connection->writeString(currentEOL);
            connection->writeUInt(bufferSize);

            while ((fileLength > 0) && (inFile.good()))
            {
                transferLen = STAF_MIN(bufferSize, fileLength);

                rc = readFile(inFile, buffer, transferLen, fromFile,
                              toMachine, fileLength, bytesCopied);
                    
                if (rc != kSTAFOk)
                {
                    result = "Unrecoverable read error occurred while"
                         " copying file " + fromFile +
                         " in text (no codepage convert) mode";
                    break;
                }

                connection->writeUInt(transferLen);
                connection->write(buffer, transferLen);

                fileLength -= transferLen;
                bytesCopied += transferLen;;
                gFSCopyManagerPtr->updateFileCopy(copyDataPtr, bytesCopied);
            }

            connection->writeUInt(kSTAFFSFinishedCopy);
            delete[] buffer;

            inFile.close();

            if (rc == kSTAFOk)
            {
                // Read an ack, so that we know the file is closed and
                // if the file was copied successfully
                rc = connection->readUInt();
            }
        }
        else if (useNewAPI)
        {
            // Binary file copy (using new API)

            connection->writeUInt(fileLength);
            
            unsigned int bytesCopied = 0;

            gFSCopyManagerPtr->updateFileCopy1(
                copyDataPtr, fileLength, kSTAFFSBinary);
            
            while ((fileLength > 0) && inFile.good())
            {
                writeLength = STAF_MIN(sizeof(fileBuffer), fileLength);
                
                rc = readFile(inFile, reinterpret_cast<char *>(fileBuffer),
                              writeLength, fromFile, toMachine, fileLength,
                              bytesCopied);

                if (rc != kSTAFOk)
                {
                    break;
                }

                connection->write(fileBuffer, writeLength);
                fileLength -= writeLength;
                bytesCopied += writeLength;
                gFSCopyManagerPtr->updateFileCopy(copyDataPtr, bytesCopied);
            }

            inFile.close();

            if (rc == kSTAFOk)
            {
                // Read an ack, so that we know the file is closed and
                // if the file was copied successfully
                rc = connection->readUInt();
            }
        }
        else
        {
            // Binary file copy (using old API)

            gFSCopyManagerPtr->updateFileCopy1(
                copyDataPtr, fileLength, kSTAFFSBinary);

            unsigned bytesCopied = 0;

            do
            {
                rc = readFile(inFile, reinterpret_cast<char *>(fileBuffer),
                              sizeof(fileBuffer), fromFile, toMachine,
                              fileLength, bytesCopied);

                if (rc != kSTAFOk)
                {
                    result = "Unrecoverable read error occurred while"
                         " copying file " + fromFile +
                         " in binary mode";
                    break;
                }

                writeLength = inFile.gcount();
                connection->writeUInt(writeLength);

                if ((writeLength == 0) && !inFile.eof())
                {
                    gFSCopyManagerPtr->remove(copyDataPtr);

                    return STAFServiceResult(kSTAFFileReadError, fromFile);
                }
                else if (writeLength != 0)
                {
                    connection->write(fileBuffer, writeLength);
                    ack = connection->readUInt();
                    if (ack) result = connection->readString();
                    fileLength -= writeLength;
                    bytesCopied += writeLength;
                    gFSCopyManagerPtr->updateFileCopy(copyDataPtr, bytesCopied);
                }
            }while ((writeLength != 0) && !ack && inFile.good());

            inFile.close();

            if (ack != kSTAFOk)
            {
                gFSCopyManagerPtr->remove(copyDataPtr);
                
                return STAFServiceResult(ack, result);
            }
            else if (writeLength != 0)
            {
                 // Tell the other side we are finished

                 connection->writeUInt(0);

                  // Read the final ack

                  try
                  {
                      ack = connection->readUInt();
                  }
                  catch (STAFConnectionIOException)
                  {
                      // Older clients will close the connection instead of
                      // acking so we need to ignore Connection IO exceptions here
                      
                      ack = kSTAFOk;
                  }

                  if (ack != kSTAFOk)
                     rc = ack;
            }
        }
    }
    catch (STAFConnectionProviderConnectException &e)
    {
        rc = kSTAFNoPathToMachine;
        result = e.getText() + STAFString(": ") + STAFString(e.getErrorCode());
    }
    catch (STAFConnectionIOException &e)
    {
        rc = kSTAFCommunicationError;
        result = e.getText() + STAFString(": ") + STAFString(e.getErrorCode());
    }
    catch (STAFException &e)
    {
        rc = e.getErrorCode();

        if (rc == kSTAFConverterError)
        {
            result = e.getText() +
                STAFString(": Caught a STAF Converter Error in "
                           "STAFFSService::handleCopyFile() while copying "
                           "to file ") +
                toFile + " from machine " + fromMachine +
                ". The file contains data that is not valid in the"
                " codepage that STAF is using.  To see the codepage that "
                "STAF is using, check the value of STAF variable "
                "STAF/Config/CodePage.";
        }
        else
        {
            result = e.getText() +
                STAFString(": Caught a STAFException in STAFFSService::"
                           "handleCopyFile() while copying to file ") +
                toFile + " from machine " + fromMachine;
        }
    }
    catch (...)
    {
        rc = kSTAFUnknownError;
        result = "Caught unknown exception in STAFFSService::handleCopyFile() "
            "while copying to file " + toFile + " from machine " +
            fromMachine + "\n";
    }
    
    if (addedFSCopyData)
    {
        // Remove file copy entry from the map
        gFSCopyManagerPtr->remove(copyDataPtr);
    }

    return STAFServiceResult(rc, result);
}


STAFServiceResult STAFFSService::handleCopyDir(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "COPY");

    // Parse the request
    
    STAFCommandParseResultPtr parsedResult = fCopyDirParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString            fromMachine    = requestInfo.fEndpoint;
    STAFHandle_t          handle         = requestInfo.fHandle;
    STAFString            request        = requestInfo.fRequest;
    unsigned int          entryTypesUInt = kSTAFFSFile | kSTAFFSDirectory;
    STAFFSCaseSensitive_t caseSensitive  = kSTAFFSCaseDefault;
    STAFString            toMachine      = fromMachine;
    bool recurse      = (parsedResult->optionTimes("RECURSE") > 0);
    bool keepEmptyDir = (parsedResult->optionTimes("KEEPEMPTYDIRECTORIES") > 0);
    bool onlyDir      = (parsedResult->optionTimes("ONLYDIRECTORIES") > 0);
    bool ignoreErrors = (parsedResult->optionTimes("IGNOREERRORS") > 0);
    STAFString   namePattern(kUTF8_STAR);
    STAFString   extPattern(kUTF8_STAR);
    STAFString   fromDir;
    STAFString   toDir;
    STAFString   typeString;
    STAFString   result;
    STAFString   ackResult;
    STAFString   errorBuffer;

    // Do not want to resolve TODIRECTORY on this machine.  Need to
    // resolve on the target machine

    if (parsedResult->optionTimes("TODIRECTORY") != 0)
        toDir = parsedResult->optionValue("TODIRECTORY");
    else
        toDir = parsedResult->optionValue("DIRECTORY");

    if (parsedResult->optionTimes("CASESENSITIVE") != 0)
        caseSensitive = kSTAFFSCaseSensitive;
    else if (parsedResult->optionTimes("CASEINSENSITIVE") != 0)
        caseSensitive = kSTAFFSCaseInsensitive;

    STAFRC_t rc = RESOLVE_STRING_OPTION("DIRECTORY", fromDir);

    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("TOMACHINE", toMachine);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("NAME", namePattern);
    if (!rc) rc = RESOLVE_OPTIONAL_STRING_OPTION("EXT", extPattern);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Obtain directory path from the value of DIRECTORY
    STAFFSPath directoryPath(fromDir);

    STAFFSEntryPtr entry;

    try
    {
        if (!directoryPath.exists())
        {
            ackResult = "Directory, " + fromDir + ", does Not Exist";
            return STAFServiceResult(kSTAFDoesNotExist, ackResult);
        }

        entry = directoryPath.getEntry();

        // Convert the fromDir to the full long path name
        // (e.g. with correct file separators, correct case if Windows,
        // no unnecessary trailing slashes, etc)

        fromDir = entry->path().asString();
    }
    catch (STAFBaseOSErrorException &e)
    {
        result = "Error on Source Directory: " + fromDir +
            "\n" + e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, result);
    }

    // if the value of DIRECTORY is not a directory, then stop here
    if (entry->type() != kSTAFFSDirectory)
    {
        ackResult = fromDir + " is Not a Directory";
        return STAFServiceResult(kSTAFInvalidValue, ackResult);
    }

    STAFConnectionPtr connection;
    STAFConnectionProviderPtr provider;
    unsigned int levelToUse = 0;
    unsigned int flags = 0;

    if (parsedResult->optionTimes("FAILIFEXISTS") != 0)
        flags |= 0x00000001;
    else if (parsedResult->optionTimes("FAILIFNEW") != 0)
        flags |= 0x00000002;

    STAFString newEOL = "Native";

    rc = RESOLVE_OPTIONAL_STRING_OPTION("FORMAT", newEOL);
    if (rc) return STAFServiceResult(rc, errorBuffer);

    // If interface cycling is not enabled, and toMachine is not 'local',
    // and neither interface nor port are specified in toMachine, get the
    // originator's interface and prepend to the toMachine value so that
    // the connection to the toMachine will use the orginator's interface.

    if ((!gConnectionManagerPtr->getAutoInterfaceCycling()) &&
        (!isLocalMachine(toMachine, 1)) &&
        (toMachine.find(gSpecSeparator == STAFString::kNPos)) &&
        (toMachine.find(kUTF8_AT) == STAFString::kNPos))
    {
        unsigned int specSepPos = requestInfo.fEndpoint.find(gSpecSeparator);

        if (specSepPos != STAFString::kNPos)
        {
            STAFString interface = requestInfo.fEndpoint.subString(
                0, specSepPos);

            if (interface != "local")
            {
                // Prepend the interface from the originator's endpoint
                toMachine = interface + gSpecSeparator + toMachine;
            }
        }
    }

    /************** Start transferring the directory ***************/

    // This flag indicates if copyDataPtr has been initialized yet by calling
    // gFSCopyManagerPtr->add() and passing it copyDataPtr

    bool addedFSCopyData = false;

    STAFFSCopyManager::FSCopyDataPtr copyDataPtr;
    
    try
    {
        // Make a connection to the toMachine

        try
        {
            rc = gConnectionManagerPtr->makeConnection(
                toMachine, provider, connection, result);
        }
        catch (STAFConnectionProviderConnectException &e)
        {
            rc = kSTAFNoPathToMachine;
            result = e.getText() + STAFString(": ") +
                STAFString(e.getErrorCode());
        }

        if ((rc == kSTAFNoPathToMachine) &&
            (toMachine.find(kUTF8_AT) == STAFString::kNPos))
        {
            // Retry connecting to the toMachine, this time appending the
            // originator's port to the toMachine

            unsigned int atPos = requestInfo.fEndpoint.find(kUTF8_AT);
        
            if (atPos != STAFString::kNPos)
            {
                toMachine = toMachine + requestInfo.fEndpoint.subString(atPos);

                rc = gConnectionManagerPtr->makeConnection(
                    toMachine, provider, connection, result);
            }
        }
        
        if (rc) return STAFServiceResult(rc, result);

        // First, lets try the new API

        connection->writeUInt(kSTAFDirectoryCopyAPI);  // API number
        connection->writeUInt(0);    // Dummy level

        STAFRC_t ack = connection->readUInt();

        // The client don't support the new API
        if (ack != kSTAFOk)
            return STAFServiceResult(ack, "No Support for Directory Copy API");

        // Now find out the specific level to use
        unsigned int minLevel = 1;
        unsigned int maxLevel = 4;

        connection->writeUInt(minLevel);
        connection->writeUInt(maxLevel);

        levelToUse = connection->readUInt();

        if (levelToUse == 0)
            return STAFServiceResult(kSTAFInvalidAPILevel,
                                     "Invalid API Level");

        int numTextExtentions = parsedResult->optionTimes("TEXTEXT");

        // This prevents attempts to transfer if invalid api

        if ((levelToUse < 2) && ( numTextExtentions != 0))
            return STAFServiceResult(kSTAFInvalidAPILevel,
                                     "Invalid API Level");

        STAFString currentEOL;
        
        // Determine default current EOL

        STAFConfigInfo sysInfo;
        STAFString_t eBuff;
        unsigned int osRC;

        if (STAFUtilGetConfigInfo(&sysInfo, &eBuff, &osRC) != kSTAFOk)
        {
            // Kill transfer
            return STAFServiceResult(
                kSTAFBaseOSError,
                STAFString("Get current EOL failure.  ") +
                STAFString(eBuff, STAFString::kShallow) +
                ", RC: " + osRC);
        }

        currentEOL = sysInfo.lineSeparator;
        
        // Make sure not doing a cyclic copy (where the source includes the
        // destination) when copying a directory using the RECURSE option
        // to the local machine

        if (recurse)
        {
            // Check if toMachine is the same as the local (from) machine
            
            // XXX: It would be great if we had a better way to check if
            // the toMachine is the same machine as the local machine.
            // Problems with the current method of submitting a MISC WHOAMI
            // request to the toMachine include:
            // 1) It only knows if the toMachine is running the same instance
            //    of STAF since it uses the STAF UUID.  If we stored a Global
            //    UUID in shared memory, then we could compare the toMachine's
            //    Global UUID with the local machine's Global UUID instead.
            // 2) It submits a request to the toMachine.  We already have a
            //    connection to toMachine so we could change it to read the
            //    toMachine's Global UUID via this existing connection.

            bool localToMachine = false;

            if (isLocalMachine(toMachine, 1))
            {
                localToMachine = true;
            }
            else
            {
                // Submit a WhoAmI request to the toMachine to see if it's the
                // local machine

                STAFResultPtr whoamiResult = gSTAFProcHandlePtr->submit(
                    toMachine, "MISC", "WHOAMI");

                if (whoamiResult->rc == kSTAFOk)
                {
                    // Unmarshall the result and get if isLocalRequest is "Yes"

                    if (whoamiResult->rc == kSTAFOk)
                    {
                        STAFString isLocalMachine = whoamiResult->resultObj->
                            get("isLocalRequest")->asString();

                        if (isLocalMachine == "Yes")
                        {
                            localToMachine = true;
                        }
                    }
                }
            }
            
            if (localToMachine)
            {
                // Resolve any STAF variables in toDir on the local machine
                // (since it hasn't been resolved yet)
                
                STAFString resToDir = toDir;

                STAFResultPtr toDirResult = gSTAFProcHandlePtr->submit(
                    "local", "VAR",
                    "RESOLVE STRING " + STAFHandle::wrapData(toDir));

                if (toDirResult->rc == kSTAFOk) resToDir = toDirResult->result;

                // Convert the toDirectory to the full long path name
                // (e.g. with correct file separators, correct case if Windows,
                // no unnecessary trailing slashes, etc)

                STAFFSPath toDirPath(resToDir);
                resToDir = toDirPath.setRoot(toDirPath.root()).asString(); 

                // Compares the from directory and to directory names in a
                // "normalized" form to make sure that they don't specify the
                // same directory name and to make sure that the fromDir
                // doesn't start with (include) the to directory which would
                // mean it's a cyclic copy which we don't support.

                unsigned int compareResult = STAFFileSystem::comparePaths(
                    resToDir, fromDir);

                if (compareResult == kSTAFFSDoesIncludePath)
                {
                    return STAFServiceResult(
                        kSTAFDirectoryCopyError,
                        "Cannot perform a cyclic copy (the source includes the "
                        "destination)");
                }
                else if (compareResult == kSTAFFSSamePath)
                {
                    return STAFServiceResult(
                        kSTAFFileWriteError,
                        STAFString("Cannot write to directory ") + resToDir +
                        " at the same time this request is reading from it");
                }
            }
        }
        
        // Add an entry to the FS Copy Map so can list copies in progress

        if (gFSCopyManagerPtr->add(fromDir, toMachine, kSTAFFSCopyFrom,
                                   kSTAFFSDirectoryCopy, kSTAFFSBinary,
                                   0, copyDataPtr))
        {
            return STAFServiceResult(
                kSTAFFileReadError,
                STAFString( "Cannot read from directory ") + fromDir +
                " at the same time another copy request is writing to it");
        }

        addedFSCopyData = true;
        
        connection->writeString(*gMachinePtr);
        connection->writeString(fromMachine);
        connection->writeString(toDir);   // write the root of toDir
        connection->writeUInt(flags);

        if (levelToUse > 2)
        {
            connection->writeUInt(requestInfo.fHandle);
            
            if (requiresSecureConnection(requestInfo.fAuthenticator) &&
                (provider->getProperty(
                    kSTAFConnectionProviderIsSecureProperty) != "1"))
            {
                // Don't send authentication data since non-secure
                // connection.  Instead set authenticator to "none" and
                // user to "anonymous" and set authentication data to ""

                connection->writeString(gNoneString);
                connection->writeString(gAnonymousString);
                connection->writeString("");
            }
            else
            {
                connection->writeString(requestInfo.fAuthenticator);
                connection->writeString(requestInfo.fUserIdentifier);
                connection->writeString(requestInfo.fAuthenticationData);
            }

            connection->writeString(requestInfo.fInterface);
            connection->writeString(requestInfo.fLogicalInterfaceID);
            connection->writeString(requestInfo.fPhysicalInterfaceID);
            connection->writeString(requestInfo.fSTAFInstanceUUID);
            connection->writeString(*gSTAFInstanceUUIDPtr);
        }

        if (levelToUse > 1)
        {
            connection->writeString(newEOL);
        }

        if (levelToUse > 2)
        {
            // Write request var pool

            STAFVariablePool::VariableMap requestVarMap =
                requestInfo.fRequestVarPool->getVariableMapCopy();

            connection->writeUInt(requestVarMap.size());

            for (STAFVariablePool::VariableMap::iterator requestIter =
                 requestVarMap.begin();
                 requestIter != requestVarMap.end();
                 ++requestIter)
            {
                connection->writeString(requestIter->second.name);
                connection->writeString(requestIter->second.value);
            }

            // Write source shared var pool

            STAFVariablePool::VariableMap sourceSharedVarMap =
                requestInfo.fSourceSharedVarPool->getVariableMapCopy();

            connection->writeUInt(sourceSharedVarMap.size());

            for (STAFVariablePool::VariableMap::iterator sourceSharedIter =
                     sourceSharedVarMap.begin();
                 sourceSharedIter != sourceSharedVarMap.end();
                 ++sourceSharedIter)
            {
                connection->writeString(sourceSharedIter->second.name);
                connection->writeString(sourceSharedIter->second.value);
            }
        }

        ack = static_cast<STAFRC_t>(connection->readUInt());
        ackResult = connection->readString();

        if (ack != kSTAFOk)
        {
            gFSCopyManagerPtr->remove(copyDataPtr);
            return STAFServiceResult(ack, ackResult);
        }

        bool doCodepageConvert = false;

        if (levelToUse > 1)
        {
            newEOL = connection->readString();

            if (numTextExtentions > 0)
            {
                doCodepageConvert = true;
                // XXX: If enable NOCONVERT option, write above line with the
                //      following commented line
                //doCodepageConvert = (parsedResult->optionTimes("NOCONVERT") == 0);
                connection->writeUInt(kSTAFFSTextConvert);
            }
            else
            {
                connection->writeUInt(kSTAFFSBinary);
            }
        }

        if (doCodepageConvert)
        {
            // If the codepages on both sides are the same, use the text
            // noconvert routine which is faster.
            
            // Get this codepage id
            STAFString cPage;
            rc = RESOLVE_STRING("{STAF/Config/CodePage}", cPage);

            // Get other codepage id
            STAFString otherCPage = connection->readString();

            if (cPage.isEqualTo(otherCPage))
            {
                // XXX: Uncomment this line to enable the codepage filter
                //      instead of forcing codepage conversion
                //doCodepageConvert = false;
            }
        }

        // Add the text extensions to a list

        STAFString resExt;
        SET_STAFString TextExtensionList;

        for (int i = 0; i < numTextExtentions; i++)
        {
            rc = RESOLVE_INDEXED_STRING_OPTION("TEXTEXT", i + 1, resExt);
            if (!rc) TextExtensionList.insert(resExt);
        }

        // Create a marshalled list of error information when copying files

        STAFObjectPtr mc = STAFObject::createMarshallingContext();
        mc->setMapClassDefinition(fErrorInfoClass->reference());
        STAFObjectPtr outputList = STAFObject::createList();

        // Start copying files/directories

        if (!recurse)
        {
            STAFFSEnumPtr fileEnum = entry->enumerate(namePattern, extPattern,
                STAFFSEntryType_t(entryTypesUInt & ~kSTAFFSDirectory),
                kSTAFFSNoSort, caseSensitive);

            if (fileEnum->isValid())
            {
                copyDirectory(fileEnum, fromDir, connection,
                            TextExtensionList, currentEOL, newEOL,
                            doCodepageConvert, levelToUse, caseSensitive,
                            outputList, copyDataPtr, toMachine);

                mc->setRootObject(outputList);
                result = mc->marshall();
            }
        }
        else
        {
            recurseCopyDir(entry, namePattern, extPattern, fromDir,
                          keepEmptyDir, onlyDir, entryTypesUInt,
                          connection, caseSensitive, TextExtensionList,
                          currentEOL, newEOL, levelToUse, doCodepageConvert,
                          outputList, copyDataPtr, toMachine);

            mc->setRootObject(outputList);
            result = mc->marshall();
        }

        // Stop copying files/directories

        connection->writeUInt(kSTAFFSStopCopy);

        if (!ignoreErrors && (outputList->size() != 0))
            rc = kSTAFDirectoryCopyError;
        else
        {
            rc = kSTAFOk;
            result = STAFString();
        }
    }
    catch (STAFConnectionProviderException &e)
    {
        rc = kSTAFNoPathToMachine;
        result = e.getText() + STAFString(": ") + STAFString(e.getErrorCode());
    }
    catch (STAFConnectionIOException &e)
    {
        rc = kSTAFCommunicationError;
        result = e.getText() + STAFString(": ") + STAFString(e.getErrorCode());
    }
    catch (...)
    {
        rc = kSTAFDirectoryCopyError;
        result = "Caught unknown exception in STAFFSService::handleCopyDir() "
            "while copying to directory " + toDir + " from machine " +
            fromMachine + "\n";
    }

    if (addedFSCopyData)
    {
        // Remove file copy entry from map keeping track of copies in progress
        gFSCopyManagerPtr->remove(copyDataPtr);
    }

    return STAFServiceResult(rc, result);
}


/*
   If the fileName is not already surrounded in double quotes, enclose it in
   double quotes if it contains one or more spaces (because that's needed for
   the OS "move"/"mv" commands).
   
   For example:  C:\Test 1  -> "C:\Test 1"
   
   However, on Unix, if the fileName contains one or more wildcards, *, in
   addition to one or more spaces, then don't enclose the wildcard characters
   in double quotes because then the shell won't expand them.
   
   For example: /tmp/Test 1/test*         -> "/tmp/Test 1/test"*
                /tmp/Test 1/test*.txt     -> "/tmp/Test 1/test"*".txt"
                /tmp/Test 1/*est*1**.txt* -> "/tmp/Test 1/"*"est"*"1"**".txt"*
                 
   Input:  String containing the file name
           bool indicating whether or not to check for wildcards (*)
   Output: String containing the updated file name
 */  
STAFString quote(STAFString fileName, bool checkForWildCards)
{
    if (fileName.find(sSpace) == STAFString::kNPos)
    {
        // No need to quote since fileName does not contain any spaces
        return fileName;
    }
    else if (fileName.find(sDoubleQuote, 0, STAFString::kChar) == 0)
    {
        // The fileName is already in double quotes (starts with ")
        return fileName;
    }
    
    if (!checkForWildCards)
    {
        // Add a double quote at the beginning and at the end of the fileName
        return sDoubleQuote + fileName + sDoubleQuote;
    }
    
    #ifdef STAF_OS_TYPE_WIN32
    return sDoubleQuote + fileName + sDoubleQuote;
    #else
    // On Unix, if the fileName contains one or more wildcards, *, in addition
    // to one or more spaces, then don't enclose the wildcard characters in
    // double quotes because then the shell won't expand them.

    if (fileName.find(sStar, 0, STAFString::kChar) == STAFString::kNPos)
    {
        return sDoubleQuote + fileName + sDoubleQuote;
    }

    // File name contains wildcards and spaces so add quotes around everything
    // except the wildcards
    
    STAFString newName = "";

    unsigned int wildcardIndex = 0;

    // Find first non-wildcard character
    unsigned int nonWildcardIndex = fileName.findFirstNotOf(
        sStar, STAFString::kChar);

    // Add double quote before next non-wildcard character

    newName += fileName.subString(0, nonWildcardIndex, STAFString::kChar) +
        sDoubleQuote;

    while (nonWildcardIndex != STAFString::kNPos)
    {
        // Find next wildcard

        wildcardIndex = fileName.findFirstOf(
            sStar, nonWildcardIndex + 1, STAFString::kChar);

        if (wildcardIndex == STAFString::kNPos)
        {
            // No more wildcards

            newName += fileName.subString(
                nonWildcardIndex, STAFString::kRemainder, STAFString::kChar) +
                sDoubleQuote;
            break;
        }
        
        newName += fileName.subString(
            nonWildcardIndex, wildcardIndex - nonWildcardIndex,
            STAFString::kChar) +
            sDoubleQuote;

        // Find next non-wildcard character

        nonWildcardIndex = fileName.findFirstNotOf(
            sStar, wildcardIndex + 1, STAFString::kChar);

        // Add the wildcard(s)

        newName += fileName.subString(
            wildcardIndex, nonWildcardIndex - wildcardIndex,
            STAFString::kChar);
        
        if (nonWildcardIndex != STAFString::kNPos)
        {
            wildcardIndex = nonWildcardIndex - 1;
            newName += sDoubleQuote;
        }
    }

    return newName;
    #endif
}

STAFServiceResult STAFFSService::handleMove(
    const bool fileSpecified,
    const STAFServiceRequest &requestInfo)
{
    // This command only supports moving files from one place to another on
    // the same machine.  In order to move files to a different machine,
    // we would need to either:
    // 1) Submit a STAF FS COPY FILE/DIRECTORY request to move the file/
    //    directory and then if successful, submit a STAF FS DELETE ENTRY
    //    request to remove the source file/directory, or
    // 2) Add new STAFProc API(s) to move a file/directory
    //
    // And we may want additional options to indicate if file(s) to be moved
    // are text files in order to handle changing line endings based on the
    // operating systeme the file(s) are being moved to and code page changes.


    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "MOVE");

    STAFCommandParseResultPtr parsedResult;
    STAFString fromOptionName;

    if (fileSpecified)
    {
        // Parse the MOVE FILE request

        parsedResult = fMoveFileParser.parse(requestInfo.fRequest);

        fromOptionName = "FILE";
    }
    else
    {
        // Parse the MOVE DIRECTORY request

        parsedResult = fMoveDirParser.parse(requestInfo.fRequest);

        fromOptionName = "DIRECTORY";
    }

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFRC_t rc = kSTAFOk;
    STAFString errorBuffer;
    STAFString result;
    
    // Resolve the value specified for ENTRY

    STAFString fromEntry;

    rc = RESOLVE_STRING_OPTION(fromOptionName, fromEntry);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    // Check if the "FROM" entry exists unless it contains a "*" wildcard
    // and it resides on a Unix system
    
    bool checkIfFromEntryExists = true;

    #ifndef STAF_OS_TYPE_WIN32
    if (fromEntry.find("*") != STAFString::kNPos)
        checkIfFromEntryExists = false;
    #endif
    
    if (checkIfFromEntryExists)
    {
        STAFFSPath fromEntryPath(fromEntry);
        STAFFSEntryPtr entry;

        try
        {
            if (!fromEntryPath.exists())
            {
                return STAFServiceResult(
                    kSTAFDoesNotExist,
                    fromOptionName + " '" + fromEntry + "' does not exist");
            }

            entry = fromEntryPath.getEntry();

            // Convert the "FROM" entry to the full long path name
            // (e.g. with correct file separators, correct case if Windows,
            // no unnecessary trailing slashes, etc)

            fromEntry = entry->path().asString();
        }
        catch (STAFBaseOSErrorException &e)
        {
            STAFString errMsg = "Error on " + fromOptionName + ": " +
                fromEntry +
                "\n" + e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, errMsg);
        }

        // If the FILE option is specified, verify that its value is a file
         
        if (fileSpecified)
        {
            // Verify that the value of FILE is a file

            if (entry->type() != kSTAFFSFile)
            {
                return STAFServiceResult(
                    kSTAFInvalidValue,
                    "'" + fromEntry + "' is not a File");
            }
        }
        else
        {
            // Verify that the value of DIRECTORY is a directory

            if (entry->type() != kSTAFFSDirectory)
            {
                return STAFServiceResult(
                    kSTAFInvalidValue,
                    "'" + fromEntry + "' is not a Directory");
            }
        }

    }
    
    // Resolve the value specified for the "TO" option

    STAFString toOptionName;

    if (fileSpecified)
    {
        if (parsedResult->optionTimes("TOFILE") != 0)
            toOptionName = "TOFILE";
        else
            toOptionName = "TODIRECTORY";
    }
    else
    {
        toOptionName = "TODIRECTORY";
    }

    STAFString toEntry;

    rc = RESOLVE_STRING_OPTION(toOptionName, toEntry);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if ((fromOptionName == "FILE") && (toOptionName == "TOFILE"))
    {
        // When moving from a file to a file, if the TOFILE already exists,
        // verify that TOFILE is a file

        STAFFSPath toEntryPath(toEntry);
        STAFFSEntryPtr entry;

        try
        {
            if (toEntryPath.exists())
            {
                // Verify that it is a file
                
                entry = toEntryPath.getEntry();

                // Convert the toDir to the full long path name (e.g. with
                // correct file separators, correct case if Windows, no
                // unnecessary trailing slashes, etc)

                toEntry = entry->path().asString();
                
                // Verify that the value of FILE is a file

                if (entry->type() != kSTAFFSFile)
                {
                    return STAFServiceResult(
                        kSTAFInvalidValue, "'" + toEntry + "' is not a File");
                }
            }
        }
        catch (STAFBaseOSErrorException &e)
        {
            result = "Error on " + toOptionName + ": " + toEntry +
                "\n" + e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, result);
        }
    }
    else if ((fromOptionName == "FILE") && (toOptionName == "TODIRECTORY"))
    {
        // Moving from a file to a directory.
        // Verify that TODIRECTORY is an existing directory

        STAFFSPath toDirPath(toEntry);
        STAFFSEntryPtr entry;

        try
        {
            if (!toDirPath.exists())
            {
                return STAFServiceResult(
                    kSTAFDoesNotExist,
                    toOptionName + " '" + toEntry + "' does not exist");
            }

            entry = toDirPath.getEntry();

            // Convert the toDir to the full long path name
            // (e.g. with correct file separators, correct case if Windows,
            // no unnecessary trailing slashes, etc)

            toEntry = entry->path().asString();
            
            // If the value of TODIRECTORY is not a directory, return
            // an error

            if (entry->type() != kSTAFFSDirectory)
            {
                result = "'" + toEntry + "' is not a Directory";
                return STAFServiceResult(kSTAFInvalidValue, result);
            }
        }
        catch (STAFBaseOSErrorException &e)
        {
            result = "Error on " + toOptionName + ": " + toEntry +
                "\n" + e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, result);
        }
    }
    else if ((fromOptionName == "DIRECTORY") &&
             (toOptionName == "TODIRECTORY"))
    {
        // When moving from a directory to a directory, if the TODIRECTORY
        // already exists, verify that TODIRECTORY is a directory

        STAFFSPath toEntryPath(toEntry);
        STAFFSEntryPtr entry;

        try
        {
            if (toEntryPath.exists())
            {
                // Verify that it is a directory
                
                entry = toEntryPath.getEntry();

                // Convert the toDir to the full long path name (e.g. with
                // correct file separators, correct case if Windows, no
                // unnecessary trailing slashes, etc)

                toEntry = entry->path().asString();
                
                // If the value of TODIRECTORY is not a directory, return
                // an error

                if (entry->type() != kSTAFFSDirectory)
                {
                    result = "'" + toEntry + "' is not a Directory";
                    return STAFServiceResult(kSTAFInvalidValue, result);
                }
            }
        }
        catch (STAFBaseOSErrorException &e)
        {
            result = "Error on " + toOptionName + ": " + toEntry +
                "\n" + e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, result);
        }
    }
 
    // To move a file/directory on the same machine, use a "move" command
    // for the operating system so that it is faster and file permissions are
    // retained

    #ifdef STAF_OS_TYPE_WIN32
    STAFString moveCommand = "move /Y";
    bool checkForWildcards = false;
    #else
    STAFString moveCommand = "mv -f";
    bool checkForWildcards = true;     
    #endif
    
    // If the from/to entry names are not already surrounded in double quotes,
    // enclose them in double quotes if they contain one or more spaces
    // (because that's needed for the OS "move"/"mv" commands).

    moveCommand += " "  + quote(fromEntry, checkForWildcards) + " " +
        quote(toEntry, false);
  
    // Use a local PROCESS START WAIT request to run the "move" comamnd

    STAFString request = "START SHELL COMMAND " +
        STAFHandle::wrapData(moveCommand) +
        " RETURNSTDOUT STDERRTOSTDOUT WAIT";

    STAFResultPtr res = gSTAFProcHandlePtr->submit(
        "local", "PROCESS", request);

    if (res->rc != kSTAFOk)
    {
        // Should not have a problem submitting the PROCESS START request
        return STAFServiceResult(res->rc, res->result);
    }

    // Check if the process RC is 0

    if (res->resultObj->type() == kSTAFMapObject)
    {
        STAFString processRC = res->resultObj->get("rc")->asString();

        if (processRC != "0")
        {
            rc = kSTAFMoveError;
        }
        
        STAFObjectPtr fileList = res->resultObj->get("fileList");

        if (fileList->type() == kSTAFListObject)
        {
            // Get stdout/stderr information to return in the result
                
            for (STAFObjectIteratorPtr iter = fileList->iterate();
                 iter->hasNext();)
            {
                STAFObjectPtr fileMap = iter->next();

                if (fileMap->type() == kSTAFMapObject)
                {
                    if (fileMap->get("rc")->asString() == "0")
                    {
                        result += fileMap->get("data")->asString().strip();
                    }
                    else
                    {
                        result += "Error getting stdout/stderr for " +
                            moveCommand + ", rc=" +
                            fileMap->get("rc")->asString();
                    }
                }
            }
        }
    }
    else
    {
        // Should never happen
        rc = kSTAFMoveError;
        result += "Error accessing result from command: " + moveCommand;
    }

    return STAFServiceResult(rc, result);
}

STAFServiceResult STAFFSService::handleGet(
    const STAFServiceRequest &requestInfo)
{
    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fGetParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFRC_t rc = kSTAFOk;
    STAFString errorBuffer;
    STAFString result;

    if (parsedResult->optionTimes("FILE") != 0)
    {
        // GET FILE request

        // Verify that the requesting machine/user has at least trust level 4

        IVALIDATE_TRUST(4, "GET FILE");

        // Determine if Text or Binary

        bool convertToText = true;         // Defaults to text

        if (parsedResult->optionTimes("BINARY") != 0) convertToText = false;

        // Determine the format

        STAFString format;

        if (parsedResult->optionTimes("FORMAT") != 0)
        {
            // Resolve the value specified for the FORMAT option

            STAFString unresFormat = parsedResult->optionValue("FORMAT");

            rc = RESOLVE_STRING_OPTION("FORMAT", format);

            if (rc) return STAFServiceResult(rc, errorBuffer);

            // If Binary, verify that the format specified is Hex

            if (!convertToText && format.toUpperCase() != "HEX")
                return STAFServiceResult(kSTAFInvalidValue, format);
        }
        else
        {
            if (convertToText)
                format = "NATIVE";  // Default Text format is Native
            else
                format = "HEX";     // Default Binary format is Hex
        }

        // Check if the undocumented TEST option was specified

        bool testFlag = false;

        if (parsedResult->optionTimes("TEST") != 0)
            testFlag = true;

        // Resolve the value specified for FILE

        STAFString file;

        rc = RESOLVE_STRING_OPTION("FILE", file);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        // Check if the file exists

        STAFFSPath path(file);

        try
        {
            if (!path.exists())
                return STAFServiceResult(
                    kSTAFDoesNotExist, "File " + file + " does not exist");
        }
        catch (STAFBaseOSErrorException &e)
        {
            STAFString errMsg = "Error on file: " + file + "\n" +
                e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, errMsg);
        }
        
        // Get the actual path name and remove any unnecessary trailing slashes
        file = path.setRoot(path.root()).asString();

        // Open the file to read it

        fstream inFile(file.toCurrentCodePage()->buffer(),
                       ios::in | STAF_ios_binary);

        if (!inFile)
            return STAFServiceResult(kSTAFFileOpenError, file);

        // Get the size of the file (upperSize and lowerSize).
        // If the file size is < 4G, upperSize will be zero.

        STAFFSEntryPtr entry;

        try
        {
            entry = path.getEntry();
        }
        catch (STAFBaseOSErrorException &e)
        {
            STAFString errMsg = "Error on file: " + file + "\n" +
                e.getText() + STAFString(": ") + e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, errMsg);
        }

        unsigned int upperSize = entry->size().first;
        unsigned int lowerSize = entry->size().second;

        // Check if file size exceeds the maximum that the FS service handles

        if (upperSize > 0)
        {
            STAFString errMsg = STAFString(
                "File size exceeds the maximum size (") + UINT_MAX +
                ") supported.  File name: " + file; 
            return STAFServiceResult(kSTAFFileReadError, errMsg);
        }

        unsigned int fileLength = lowerSize;

        // Added for debugging memory issues

        if (gResultWarningSize)
        {
            if (fileLength > gResultWarningSize)
            {
                // Log a warning tracepoint message

                STAFString warningMsg = STAFString(
                    "WARNING: Submitting a FS GET FILE request for a large "
                    "file (") + fileLength + " bytes) uses a lot of memory.  "
                    "STAFFSService::handleGet()";
                    
                warningMsg += " - Client: " + requestInfo.fMachine +
                    ", Handle: " + STAFString(requestInfo.fHandle) +
                    ", Handle Name: " + requestInfo.fHandleName +
                    ", Request: " + requestInfo.fRequest;

                STAFTrace::trace(kSTAFTraceWarning, warningMsg);
            }
        }
        
        // Determine the maximum return file size
        
        unsigned int maxFileSize = gMaxReturnFileSize;  // Default

        // Check if variable STAF/MaxReturnFileSize was set to a non-zero
        // value.  Resolve this variable using only the originating handle's
        // pool associated with this request so create a variable pool that
        // only contains the request variable pool.

        const STAFVariablePool *theVarPoolList[] =
        {
            requestInfo.fRequestVarPool
        };

        unsigned int theVarPoolListSize = sizeof(theVarPoolList) /
            sizeof(const STAFVariablePool *);

        STAFString sizeString;

        STAFRC_t maxSizeRC = STAFVariablePool::resolve(
            "{STAF/MaxReturnFileSize}",
            theVarPoolList, theVarPoolListSize, sizeString);
        
        if (maxSizeRC == kSTAFOk)
        {
            // Variable STAF/MaxReturnFileSize exists
            
            // Verify if its size value is valid and convert it to bytes
            // if needed
            
            STAFString_t errorBuffer = 0;
            unsigned int maxSize = 0; // 0 means no size limit

            STAFRC_t rc = STAFUtilConvertSizeString(
                sizeString.getImpl(), &maxSize, &errorBuffer);

            if (rc != kSTAFOk)
            {
                STAFString errMsg = STAFString(
                    "Variable STAF/MaxReturnFileSize in the originating "
                    "handle's variable pool for this request is set to "
                    "an invalid value: ") + sizeString + " \n" +
                    STAFString(errorBuffer, STAFString::kShallow);

                return STAFServiceResult(kSTAFInvalidValue, errMsg);
            }

            if (maxSize != 0)
            {
                if ((gMaxReturnFileSize == 0) ||
                    (maxSize < gMaxReturnFileSize))
                {
                    // Assign the maximum file size based on the
                    // STAF/MaxReturnFileSize variable
                    maxFileSize = maxSize;
                }
            }
        }

        // Determine if the file size exceeds the maximum allowed size

        if ((maxFileSize != 0) && (fileLength > maxFileSize))
        {
            return STAFServiceResult(
                kSTAFMaximumSizeExceeded,
                STAFString("File '") + file + "' size is " + fileLength +
                " bytes which exceeds " + STAFString(maxFileSize) +
                " bytes, the maximum return file size allowed");
        }
        
        // Initialize the output buffer and read in the file

        STAFBuffer<char> buffer(new char[fileLength], STAFBuffer<char>::INIT,
                                STAFBuffer<char>::ARRAY);
        
        inFile.read(buffer, fileLength);
        inFile.close();

        if (!convertToText)
        {
            // Convert the file contents to a hex format
            return convertToHex(buffer, fileLength);
        }

        // Don't convert line-endings in the text file if format is "AsIs"

        if (format.toUpperCase() == "ASIS")
        {
            // If the undocumented TEST option was specified.

            if (testFlag)
            {
                // Return the result in hex to verify that the line-ending
                // characters were not converted.
                return convertToHex(buffer, fileLength);
            }

            return STAFServiceResult(rc, STAFString(buffer, fileLength));
        }

        // Convert the line ending characters in the text file

        return convertLineEndings(buffer, fileLength, format,
                                  requestInfo.fEndpoint,
                                  requestInfo.fIsLocalRequest, testFlag);
    }
    else
    {
        // GET ENTRY request

        // Verify that the requesting machine/user has at least trust level 2

        IVALIDATE_TRUST(2, "GET ENTRY");
        
        STAFString entryName;

        rc = RESOLVE_STRING_OPTION("ENTRY", entryName);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFFSPath path(entryName);

        try
        {
            if (!path.exists())
            {
                return STAFServiceResult(
                    kSTAFDoesNotExist, "Entry " + entryName +
                    " does not exist");
            }
        }
        catch (STAFBaseOSErrorException &e)
        {
            STAFString errMsg = "Error on entry: " + entryName + "\n" +
                e.getText() + STAFString(": ") + e.getErrorCode();
            
            return STAFServiceResult(kSTAFBaseOSError, errMsg);
        }

        STAFFSEntryPtr entry;

        try
        {
            entry = path.getEntry();
        }
        catch (STAFBaseOSErrorException &e)
        {
            STAFString errMsg = e.getText() + STAFString(": ") +
                e.getErrorCode();

            return STAFServiceResult(kSTAFBaseOSError, errMsg);
        }

        if (parsedResult->optionTimes("TYPE") != 0)
        {
            result = getTypeString(entry->type());
        }
        else if (parsedResult->optionTimes("SIZE") != 0)
        {
            // Create a marshalled map containing entry size information

            STAFObjectPtr mc = STAFObject::createMarshallingContext();
            mc->setMapClassDefinition(fEntrySizeInfoClass->reference());

            STAFObjectPtr entrySizeMap = fEntrySizeInfoClass->createInstance();
            entrySizeMap->put("size", STAFString(entry->size64()));
            entrySizeMap->put("upperSize", STAFString(entry->size().first));
            entrySizeMap->put("lowerSize", STAFString(entry->size().second));

            mc->setRootObject(entrySizeMap);
            result = mc->marshall();
        }
        else if (parsedResult->optionTimes("MODTIME") != 0)
        {
            result = entry->modTime().asString();
        }
        else if (parsedResult->optionTimes("LINKTARGET") != 0)
        {
            if (entry->linkTarget() != "")
                result = entry->linkTarget();
            else
                result = sNoneString;
        }
        else if (parsedResult->optionTimes("CHECKSUM") != 0)
        {
#ifdef STAF_USE_SSL

            // Check if the entry specified is a directory and if so, return
            // an error as you cannot get the checksum for a directory

            if (entry->type() == kSTAFFSDirectory)
            {
                return STAFServiceResult(
                    kSTAFInvalidValue,
                    "Cannot get the checksum for an entry that is a "
                    "directory");
            }

            // Default the checksum alogrithm to MD5 if not specified

            STAFString checksumAlgorithm = "MD5";

            // Get the checksum value, if specified, and resolve any STAF
            // variables in it

            if (parsedResult->optionValue("CHECKSUM") != "")
            {
                rc = RESOLVE_OPTIONAL_STRING_OPTION(
                    "CHECKSUM", checksumAlgorithm);
            }

            // Compute the checksum for the specified cryptographic hashing
            // algorithm (aka digest) using functions provided by OpenSSL

            const EVP_MD *md;

            OpenSSL_add_all_digests();

            md = EVP_get_digestbyname(
                checksumAlgorithm.toUpperCase().toCurrentCodePage()->buffer());

            if (!md)
            {
                return STAFServiceResult(
                    kSTAFInvalidValue,
                    STAFString("Unknown algorithm specified for the "
                               "CHECKSUM option: ") + checksumAlgorithm);
            }

            // Open the file to read it

            fstream inFile(entryName.toCurrentCodePage()->buffer(),
                           ios::in | STAF_ios_binary);

            if (!inFile)
            {
                return STAFServiceResult(
                    kSTAFFileOpenError,
                    STAFString("Cannot open file ") + entryName);
            }

            // Get the size of the file (upperSize and lowerSize).
            // If the file size is < 4G, upperSize will be zero.

            unsigned int upperSize = entry->size().first;
            unsigned int lowerSize = entry->size().second;

            // Check if file size exceeds the maximum that the FS service handles

            if (upperSize > 0)
            {
                inFile.close();

                STAFString errMsg = STAFString(
                    "File size exceeds the maximum size (") + UINT_MAX +
                    ") supported.  File name: " + entryName;

                return STAFServiceResult(kSTAFFileReadError, errMsg);
            }

            unsigned int fileLength = lowerSize;

            EVP_MD_CTX mdctx;
            unsigned char md_value[EVP_MAX_MD_SIZE];
            unsigned int md_len;

            EVP_MD_CTX_init(&mdctx);
            EVP_DigestInit_ex(&mdctx, md, NULL);

            // Read the entire file using a buffer size of 4096 bytes and
            // update the digest with the buffer

            unsigned int bytesCopied = 0;
            char fileBuffer[4096] = {0};
            unsigned int writeLength = 0;
            
            while ((fileLength > 0) && inFile.good())
            {
                writeLength = STAF_MIN(sizeof(fileBuffer), fileLength);
                
                rc = readFile(inFile, reinterpret_cast<char *>(fileBuffer),
                              writeLength, entryName, "local", fileLength,
                              bytesCopied);

                if (rc != kSTAFOk) break;

                EVP_DigestUpdate(&mdctx, fileBuffer, writeLength);
                fileLength -= writeLength;
                bytesCopied += writeLength;
            }
            
            inFile.close();

            if (rc == kSTAFOk)
            {
                // Get the checksum value
                EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
            }

            EVP_MD_CTX_cleanup(&mdctx);

            if (rc == kSTAFOk)
            {
                // Convert the checksum value to a hexadecimal format
                return convertToHex(reinterpret_cast<char *>(md_value), md_len);
            }
            else
            {
                return STAFServiceResult(
                    kSTAFFileReadError,
                    STAFString("Error reading file ") + entryName);
            }
#else
            return STAFServiceResult(
                kSTAFInvalidRequestString, "Cannot specify the CHECKSUM "
                "option because STAF OpenSSL support is not provided");
#endif
        }

        return STAFServiceResult(rc, result);
    }
}


STAFServiceResult STAFFSService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fListParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;

    if (parsedResult->optionTimes("SETTINGS") > 0)
    {
        // LIST SETTINGS

        // Create a marshalling context to represent the settings info

        STAFObjectPtr mc = STAFObject::createMarshallingContext();
        mc->setMapClassDefinition(fSettingsClass->reference());
        STAFObjectPtr settingsMap = fSettingsClass->createInstance();

        if (gStrictFSCopyTrust)
            settingsMap->put("strictFSCopyTrust", "Enabled");
        else
            settingsMap->put("strictFSCopyTrust", "Disabled");

        mc->setRootObject(settingsMap);

        return STAFServiceResult(kSTAFOk, mc->marshall());
    }

    //LIST DIRECTORY <Name> [RECURSE] [LONG [DETAILS] | SUMMARY] [TYPE <Types>]
    //     [NAME <Pattern>] [EXT <Pattern>] [CASESENSITIVE | CASEINSENSITIVE]
    //     [SORTBYNAME | SORTBYSIZE | SORTBYMODTIME]

    STAFFSSortBy_t        sortBy         = kSTAFFSNoSort;
    STAFFSCaseSensitive_t caseSensitive  = kSTAFFSCaseDefault;
    unsigned int          entryTypesUInt = kSTAFFSNormal;
    bool showLong = (parsedResult->optionTimes("LONG") > 0);
    bool details = (parsedResult->optionTimes("DETAILS") > 0);
    bool summary = (parsedResult->optionTimes("SUMMARY") > 0);
    bool recurse = (parsedResult->optionTimes("RECURSE") > 0);

    STAFString namePattern(kUTF8_STAR);
    STAFString extPattern(kUTF8_STAR);
    STAFString dir;
    STAFString typeString;
    STAFRC_t rc = RESOLVE_STRING_OPTION("DIRECTORY", dir);

    if (!rc) RESOLVE_OPTIONAL_STRING_OPTION("NAME", namePattern);
    if (!rc) RESOLVE_OPTIONAL_STRING_OPTION("EXT" , extPattern);
    if (!rc) RESOLVE_OPTIONAL_STRING_OPTION("TYPE", typeString);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (parsedResult->optionTimes("SORTBYNAME") != 0)
        sortBy = kSTAFFSSortByName;
    else if (parsedResult->optionTimes("SORTBYSIZE") != 0)
        sortBy = kSTAFFSSortBySize;
    else if (parsedResult->optionTimes("SORTBYMODTIME") != 0)
        sortBy = kSTAFFSSortByModTime;

    if (parsedResult->optionTimes("CASESENSITIVE") != 0)
        caseSensitive = kSTAFFSCaseSensitive;
    else if (parsedResult->optionTimes("CASEINSENSITIVE") != 0)
        caseSensitive = kSTAFFSCaseInsensitive;

    if (typeString.toUpperCase() == "ALL")
        entryTypesUInt = kSTAFFSAll;
    else if (typeString.length() != 0)
        entryTypesUInt = getTypeMaskFromString(typeString, true);

    STAFFSPath dirPath(dir);

    try
    {
        if (!dirPath.exists())
            return STAFServiceResult(
                kSTAFDoesNotExist, "Directory " + dir + " does not exist");
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = "Error on Directory: " + dir + "\n" +
            e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }

    // Create a marshalled list of maps containing file information

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (showLong && details)
        mc->setMapClassDefinition(fListDetailsInfoClass->reference());
    else if (showLong && !details)
        mc->setMapClassDefinition(fListLongInfoClass->reference());
    else if (summary)
        mc->setMapClassDefinition(fListSummaryInfoClass->reference());

    STAFFSEntryPtr dirPathEntry;

    try
    {
        dirPathEntry = dirPath.getEntry();
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }
    
    // Return an error if the DIRECTORY specified is not a directory

    if (dirPathEntry->type() != kSTAFFSDirectory)
    {
        return STAFServiceResult(
            kSTAFInvalidValue, dir + " is not a directory");
    }

    if (summary)
    {
        // Get a summary of the statistics for the directory such as
        // total size, number of files in the directory, and number of
        // subdirectories

        STAFObjectPtr summaryMap = fListSummaryInfoClass->createInstance();

        STAFUInt64_t dirSize = 0;
        STAFUInt64_t numFiles = 0;
        STAFUInt64_t numDirectories = 0;

        getDirectorySize(dirPathEntry, namePattern, extPattern,
                         entryTypesUInt, caseSensitive, recurse,
                         dirSize, numFiles, numDirectories);

        summaryMap->put("name", dirPathEntry->path().asString());
        summaryMap->put("size", STAFString(dirSize));
        summaryMap->put("numFiles", STAFString(numFiles));
        summaryMap->put("numDirectories", STAFString(numDirectories));

        mc->setRootObject(summaryMap);

        return STAFServiceResult(kSTAFOk, mc->marshall());
    }

    STAFObjectPtr outputList = STAFObject::createList();
    
    // Get the actual directory path name

    STAFString rootDir = dirPath.setRoot(dirPath.root()).asString();

#ifdef STAF_OS_TYPE_WIN32

    // On Windows, need to remove the trailing backslash from a path that
    // is a root directory like C:\

    if (rootDir.findFirstNotOf(kUTF8_BSLASH) != STAFString::kNPos)
    {
        unsigned int lastNonSlashLoc = rootDir.findLastNotOf(kUTF8_BSLASH);

        if (lastNonSlashLoc + 1 != rootDir.length())
        {
            rootDir = rootDir.subString(
                0, lastNonSlashLoc + 1, STAFString::kChar);
        }
    }
#endif

    if (!recurse)
    {
        // List matching entries non-recursively (e.g. don't check 
        // sub-directories for any matching entries)

        // Get a list of all matching entries

        STAFFSEnumPtr dirEnum = dirPathEntry->enumerate(
            namePattern, extPattern, STAFFSEntryType_t(entryTypesUInt),
            sortBy, caseSensitive);

        // Iterate through the sorted matching entries and format each entry

        for (; dirEnum->isValid(); dirEnum->next())
        {
            addListDirectoryEntry(rootDir, dirEnum->entry(),
                                  outputList, mc, showLong, details);
        }
    }
    else
    {
        // List matching entries recursively (e.g. check sub-directories for
        // any matching entries as well)

        // Get a list of all matching entries

        STAFFSEntryList entryList;
        
        recurseListDir(dirPathEntry, namePattern, extPattern,
                       entryTypesUInt, caseSensitive, sortBy, entryList);

        // Sort the entries by name in the specified case, or by size, or by
        // last modification time.

        switch (sortBy)
        {
            case kSTAFFSSortByName:
            {
                std::sort(entryList.begin(), entryList.end(),
                          STAFSortEnumByName(caseSensitive));
                break;
            }

            case kSTAFFSSortBySize:
            {
                std::sort(entryList.begin(), entryList.end(),
                          sortEnumBySize);
                break;
            }

            case kSTAFFSSortByModTime:
            {
                std::sort(entryList.begin(), entryList.end(),
                          sortEnumByModTime);
                break;
            }

            default: break;
        }
        
        // Iterate through the sorted matching entries and format each entry

        for (STAFFSEntryList::iterator iter = entryList.begin();
             iter != entryList.end(); ++iter)
        {
            addListDirectoryEntry(rootDir, *iter, outputList, mc,
                                  showLong, details);
        }                         
    }

    mc->setRootObject(outputList);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFFSService::handleListCopyRequests(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fListCopyRequestsParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;

    // LIST COPYREQUESTS request

    // Create a marshalling context to represent the list of copy requests

    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    bool longFlag = false;
    bool toFlag = false;
    bool fromFlag = false;
    bool fileFlag = false;
    bool directoryFlag = false;
    bool binaryFlag = false;
    bool textFlag = false;

    if (parsedResult->optionTimes("LONG") > 0) longFlag = true;
    if (parsedResult->optionTimes("INBOUND") > 0) toFlag = true;
    if (parsedResult->optionTimes("OUTBOUND") > 0) fromFlag = true;
    if (parsedResult->optionTimes("DIRECTORY") > 0) directoryFlag = true;

    if (parsedResult->optionTimes("FILE") > 0)
    {
        fileFlag = true;

        if (parsedResult->optionTimes("BINARY") > 0) binaryFlag = true;
        if (parsedResult->optionTimes("TEXT") > 0) textFlag = true;
    }

    if (!toFlag && !fromFlag)
    {
        toFlag = true;
        fromFlag = true;
    }

    if (!fileFlag && !directoryFlag)
    {
        fileFlag = true;
        directoryFlag = true;
    }

    if (!binaryFlag && !textFlag)
    {
        binaryFlag = true;
        textFlag = true;
    }

    if (!longFlag)
    {
        mc->setMapClassDefinition(fCopyRequestClass->reference());
    }
    else
    {
        mc->setMapClassDefinition(fCopyFileClass->reference());
        mc->setMapClassDefinition(fFileCopyStateClass->reference());
        mc->setMapClassDefinition(fCopyDirectoryClass->reference());
        mc->setMapClassDefinition(fDirectoryCopyStateClass->reference());
    }

    STAFObjectPtr resultList = STAFObject::createList();

    // Read FS Copy Map
        
    STAFFSCopyManager::FSCopyMap copyMap = gFSCopyManagerPtr->
        getFSCopyMapCopy();

    STAFFSCopyManager::FSCopyMap::iterator iter;

    for (iter = copyMap.begin(); iter != copyMap.end(); ++iter)
    {
        STAFFSCopyManager::FSCopyDataPtr copyData = iter->second;

        STAFObjectPtr resultMap;
         
        if (!longFlag)
            resultMap = fCopyRequestClass->createInstance();
        else if (copyData->type == kSTAFFSFileCopy)
            resultMap = fCopyFileClass->createInstance();
        else
            resultMap = fCopyDirectoryClass->createInstance();
        
        if (copyData->direction == kSTAFFSCopyFrom)
        {
            if (!fromFlag) continue;
                
            resultMap->put("io", "Out");
        }
        else
        {
            if (!toFlag) continue;

            resultMap->put("io", "In");
        }

        if (copyData->type == kSTAFFSFileCopy)
        {
            if (!fileFlag) continue;

            resultMap->put("type", "F");
        }
        else
        {
            if (!directoryFlag) continue;
                
            resultMap->put("type", "D");
        }

        resultMap->put("startTimestamp", copyData->startTimestamp);
        resultMap->put("machine", copyData->machine);
        resultMap->put("name", copyData->name);

        if (longFlag)
        {
            // Assign additional information for the copy in progress

            if (copyData->type == kSTAFFSFileCopy)
            {
                if (copyData->mode == kSTAFFSBinary)
                {
                    if (!binaryFlag) continue;

                    resultMap->put("mode", "Binary");
                }
                else
                {
                    if (!textFlag) continue;

                    resultMap->put("mode", "Text");
                }

                STAFObjectPtr stateMap = fFileCopyStateClass->
                    createInstance();

                if (copyData->fileSize != 0)
                {
                    stateMap->put("fileSize", copyData->fileSize);
                }

                stateMap->put("bytesCopied", copyData->bytesCopied);

                resultMap->put("state", stateMap);
            }
            else
            {
                STAFObjectPtr stateMap;

                stateMap = fDirectoryCopyStateClass->createInstance();

                if (copyData->mode == kSTAFFSBinary)
                {
                    stateMap->put("mode", "Binary");
                }
                else
                {
                    stateMap->put("mode", "Text");
                }

                if (copyData->entryName != STAFString(""))
                {
                    stateMap->put("name", copyData->entryName);

                    if (copyData->fileSize != 0)
                    {
                        stateMap->put("fileSize", copyData->fileSize);
                    }

                    stateMap->put("bytesCopied", copyData->bytesCopied);
                }

                resultMap->put("state", stateMap);
            }
        }
        
        resultList->append(resultMap);
    }

    mc->setRootObject(resultList);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFFSService::handleCreate(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "CREATE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fCreateParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    unsigned int osRC = 0;
    STAFFSDirectoryCreateMode_t createMode = kSTAFFSCreateDirOnly;
    STAFString dir;
    STAFString result;
    STAFString errorBuffer;
    STAFRC_t rc = RESOLVE_STRING_OPTION("DIRECTORY", dir);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (parsedResult->optionTimes("FULLPATH") != 0)
        createMode = kSTAFFSCreatePath;

    rc = STAFFSCreateDirectory(dir.getImpl(), createMode, &osRC);

    if (rc == kSTAFBaseOSError)
    {
        result = STAFString("Could not create directory ") + dir +
            STAFString(", OS RC: ") + osRC;
    }
    else if (rc == kSTAFAlreadyExists)
    {
        if (parsedResult->optionTimes("FAILIFEXISTS") != 0)
        {
            result = dir;
        }
        else
        {
            // The entry specified for the DIRECTORY option already exists.
            // If the entry specified is a directory then don't return an
            // error since the FAILIFEXISTS option wan't specified.
            // However, if it's not a directory, return an error.
            
            STAFFSPath dirPath(dir);
            STAFFSEntryPtr dirPathEntry;

            try
            {
                dirPathEntry = dirPath.getEntry();

                if (dirPathEntry->type() == kSTAFFSDirectory)
                    rc = kSTAFOk;  // Not an error
                else
                    result = dir + " already exists, but is not a directory";
            }
            catch (STAFBaseOSErrorException)
            {
                rc = kSTAFOk;  // Ignore exceptions
            }
        }
    }

    return STAFServiceResult(rc, result);
}


STAFServiceResult STAFFSService::handleDelete(
    const STAFServiceRequest &requestInfo)
{
    // Parse the request
    
    STAFCommandParseResultPtr parsedResult = fDeleteParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    bool children     = (parsedResult->optionTimes("CHILDREN") > 0);
    bool recurse      = (parsedResult->optionTimes("RECURSE") > 0);
    bool ignoreErrors = (parsedResult->optionTimes("IGNOREERRORS") > 0);
    STAFFSCaseSensitive_t caseSensitive  = kSTAFFSCaseDefault;
    unsigned int entryTypesUInt = kSTAFFSAll & ~kSTAFFSSpecialDirectory;
    STAFString namePattern(kUTF8_STAR);
    STAFString extPattern(kUTF8_STAR);
    STAFString entryString;
    STAFString typeString;
    STAFString result;
    STAFString errorBuffer;

    // Verify that the requesting machine/user has at least trust level 4
    // if RECURSE is not specified and trust level 5 if RECURSE is specified

    if (!recurse)
    {
        IVALIDATE_TRUST(4, "DELETE");
    }
    else
    {
        IVALIDATE_TRUST(5, "DELETE RECURSE");
    }
    
    STAFRC_t rc = RESOLVE_STRING_OPTION("ENTRY", entryString);

    if (!rc) RESOLVE_OPTIONAL_STRING_OPTION("NAME", namePattern);
    if (!rc) RESOLVE_OPTIONAL_STRING_OPTION("EXT" , extPattern);
    if (!rc) RESOLVE_OPTIONAL_STRING_OPTION("TYPE", typeString);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (parsedResult->optionTimes("CASESENSITIVE") != 0)
        caseSensitive = kSTAFFSCaseSensitive;
    else if (parsedResult->optionTimes("CASEINSENSITIVE") != 0)
        caseSensitive = kSTAFFSCaseInsensitive;

    if (typeString.toUpperCase() == "ALL")
        entryTypesUInt = kSTAFFSAll & ~kSTAFFSSpecialDirectory;
    else if (typeString.length() != 0)
        entryTypesUInt = getTypeMaskFromString(typeString, false);

    STAFFSPath entryPath(entryString);

    try
    {
        if (!entryPath.exists())
            return STAFServiceResult(
                kSTAFDoesNotExist, "Entry " + entryString + " does not exist");
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = "Error on Entry: " + entryString + "\n" +
            e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }


    STAFFSEntryPtr entry;

    try
    {
        entry = entryPath.getEntry();
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = e.getText() + STAFString(": ") +
            e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }

    unsigned int osRC = 0;

    // Create a marshalled list of error information deleting a directory

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fErrorInfoClass->reference());
    STAFObjectPtr outputList = STAFObject::createList();

    if (recurse)
    {
        if (entry->type() == kSTAFFSDirectory)
        {
            recurseRemove(entry, namePattern, extPattern, entryTypesUInt,
                          caseSensitive, outputList);
        }

        if (!children)
        {
            rc = entry->remove(&osRC);
            updateResultString(outputList, entry, rc, osRC);
        }

        mc->setRootObject(outputList);
        result = mc->marshall();
    }
    else if (children)
    {
        if (entry->type() == kSTAFFSDirectory)
        {
            removeChildren(entry, namePattern, extPattern, entryTypesUInt,
                           caseSensitive, outputList);
        }

        mc->setRootObject(outputList);
        result = mc->marshall();
    }
    else
    {
        rc = entry->remove(&osRC);
        updateResultString(outputList, entry, rc, osRC);

        mc->setRootObject(outputList);
        result = mc->marshall();
    }

    if (recurse || children)
    {
        if (outputList->size() != 0)
            rc = kSTAFFileDeleteError;
        
        if (ignoreErrors)
            result = STAFString();
    }
    else
    {
        if (rc == kSTAFBaseOSError)
            rc = kSTAFFileDeleteError;
    }

    return STAFServiceResult(rc, result);
}


STAFServiceResult STAFFSService::handleQuery(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "QUERY");

    // Parse the request

    STAFCommandParseResultPtr parsedResult =
                              fQueryParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString entryName;
    STAFString result;
    STAFString errorBuffer;
    STAFRC_t rc = RESOLVE_STRING_OPTION("ENTRY", entryName);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    STAFFSPath path(entryName);

    STAFFSEntryPtr entry;

    try
    {
        if (!path.exists())
            return STAFServiceResult(
                kSTAFDoesNotExist, "Entry " + entryName + " does not exist");

        entry = path.getEntry();
    }
    catch (STAFBaseOSErrorException &e)
    {
        STAFString errMsg = "Error on Entry: " + entryName + "\n" +
            e.getText() + STAFString(": ") + e.getErrorCode();

        return STAFServiceResult(kSTAFBaseOSError, errMsg);
    }

    // Create a marshalled map of information about the specified entry

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fQueryInfoClass->reference());
   
    STAFObjectPtr fileInfoMap = fQueryInfoClass->createInstance();

    fileInfoMap->put("name", entry->path().asString());
    fileInfoMap->put("type", getTypeString(entry->type()));
    fileInfoMap->put("size", STAFString(entry->size64()));
    fileInfoMap->put("upperSize", STAFString(entry->size().first));
    fileInfoMap->put("lowerSize", STAFString(entry->size().second));
    fileInfoMap->put("lastModifiedTimestamp", entry->modTime().asString());

    if (entry->linkTarget() != "")
        fileInfoMap->put("linkTarget", entry->linkTarget());

    mc->setRootObject(fileInfoMap);

    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFFSService::handleSet(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 5

    IVALIDATE_TRUST(5, "SET");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = fSetParser.parse(
        requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
    
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString strictTrust;
    STAFString errorBuffer;
    STAFRC_t rc = RESOLVE_STRING_OPTION("STRICTFSCOPYTRUST", strictTrust);

    if (rc) return STAFServiceResult(rc, errorBuffer);

    if (strictTrust.isEqualTo(sEnabled, kSTAFStringCaseInsensitive))
    {
        // Get exclusive access to gStrictFSCopyTrust

        STAFMutexSemLock lock(sStrictFSCopyTrustSem);
        gStrictFSCopyTrust = 1;
    }
    else if (strictTrust.isEqualTo(sDisabled, kSTAFStringCaseInsensitive))
    {
        // Get exclusive access to gStrictFSCopyTrust

        STAFMutexSemLock lock(sStrictFSCopyTrustSem);
        gStrictFSCopyTrust = 0;
    }
    else
    {
        return STAFServiceResult(
            kSTAFInvalidValue,
            "STRICTFSCOPYTRUST value must be ENABLED or DISABLED.  "
            "Invalid value: " + strictTrust);
    }

    return STAFServiceResult(kSTAFOk);
}


STAFServiceResult STAFFSService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
