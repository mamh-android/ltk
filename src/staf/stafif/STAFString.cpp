/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <map>
#include <ctype.h>
#include <cstring>
#include "STAFString.h"
#include "STAFMutexSem.h"
#include "STAFConverter.h"
#include "STAFUtil.h"
#include "STAFTrace.h"

////////////////////////////////////////////////////////////////////////////////

// defined constants
const unsigned int MIN     =   32;      // min alloc mem for strings
const unsigned int MAX     = 4096;      // best if it's a page size
const unsigned int DELTA   = 4096;      // best if it's a page size
const unsigned int ISCHAR  =    0;      // corb (character or byte parm)
const unsigned int ISBYTE  =    1;      // corb (character or byte parm)

////////////////////////////////////////////////////////////////////////////////

struct STAFStringImplementation
{
    char *pBuffer;
    unsigned int fBuffLen;
    unsigned int fCharLen;
    unsigned int fByteLen;
};

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
    3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
    6, 6, 0, 0
};

static const STAFStringImplementation CHAR_TABLE[] =
{
    { "\x0",      1, 1, 1 }, { "\x20", 1, 1, 1 }, { "\x7b", 1, 1, 1 },
    { "\x7d",     1, 1, 1 }, { "\x28", 1, 1, 1 }, { "\x29", 1, 1, 1 },
    { "\x3c",     1, 1, 1 }, { "\x3e", 1, 1, 1 }, { "\x3a", 1, 1, 1 },
    { "\x3b",     1, 1, 1 }, { "\x2c", 1, 1, 1 }, { "\x2e", 1, 1, 1 },
    { "\x5c",     1, 1, 1 }, { "\x2f", 1, 1, 1 }, { "\x3d", 1, 1, 1 },
    { "\x27",     1, 1, 1 }, { "\x22", 1, 1, 1 }, { "\x7c", 1, 1, 1 },
    { "\xc0\x80", 2, 1, 2 }, { "\x23", 1, 1, 1 }, { "\x0d", 1, 1, 1 },
    { "\x0a",     1, 1, 1 }, { "\x2a", 1, 1, 1 }, { "\x2d", 1, 1, 1 },
    { "\x25",     1, 1, 1 }, { "\x3f", 1, 1, 1 }, { "\x5e", 1, 1, 1 },
    { "\x26",     1, 1, 1 }, { "\x40", 1, 1, 1 }, { "\x09", 1, 1, 1 },
    { "\x21",     1, 1, 1 }
};

static const char *EMPTY_STRING = "";

////////////////////////////////////////////////////////////////////////////////

// useful inline methods
 
inline int BYTES(char *c)      { return SIZE_TABLE[(unsigned char)*c];    }
inline int SPACE(char *c)      { return (*c == 0x20);                     }
inline int DIGIT(char *c)      { return (*c >= 0x30 && *c <= 0x39);       }
inline int ZERO(char *c)       { return (*c == 0x30);                     }
inline int ASCII(char *c)      { return (BYTES(c) == 1);                  }
inline int WHITESPACE(char *c) { return (*c == 0x20 || *c == 0x09 ||
                                         *c == 0x0A || *c == 0x0D);       }
inline char *PREV(char *c)     { while (!BYTES(--c)); return c;           }
inline char *NEXT(char *c)     { c += BYTES(c); return c;                 }
inline char *REWB(char *c, unsigned int n) { return c - n;                }
inline char *FWDB(char *c, unsigned int n) { return c + n;                }
inline char *REWC(char *c, unsigned int n) { while (n--) c = PREV(c);
                                             return c;                    }
inline char *FWDC(char *c, unsigned int n) { while (n--) c = NEXT(c);
                                             return c;                    }
inline char *REWN(char *c, unsigned int n, unsigned int corb)
                         { return (corb ? REWB(c, n) : REWC(c, n));       }
inline char *FWDN(char *c, unsigned int n, unsigned int corb)
                         { return (corb ? FWDB(c, n) : FWDC(c, n));       }

// Determine the size of the new buffer to allocate. 
// The new size will be at least MIN bytes and increases by powers
// of 2 up to MAX bytes; after that, allocation is done in increments of
// DELTA bytes.
unsigned int getBufferSize(unsigned int len)
{
    unsigned int min = MIN;

    // find the minimum storage for the string (a power of 2 < max)

    while (min < len && min < MAX) min <<= 1;  // O(lg MAX)

    // if len is above the max mark, find what needs to be allocated in
    // delta increments, i.e. if diff == 5 then alloc min + diff * delta

    int diff = len - MAX;

    diff = (diff > 0 ? (diff / DELTA) + 1 : 0);

    return min + diff * DELTA;
}

////////////////////////////////////////////////////////////////////////////////

static const STAFString sStar(kUTF8_STAR);
static const STAFString sQuestion(kUTF8_QUESTION);
static const STAFString sWildCards(sStar + sQuestion);

////////////////////////////////////////////////////////////////////////////////

STAFConverter *getConverterInstance()
{
    static STAFMutexSem sConverterSem;
    static STAFConverter *sConverterPtr = 0;
    
    if (sConverterPtr != 0) return sConverterPtr;
    
    STAFMutexSemLock lock(sConverterSem);
    sConverterPtr = new STAFConverter();

    return sConverterPtr;
}


/***********************************************************************/
/* convertUInt64ToString - Creates a STAFString from an STAFUInt64_t   */
/*                         number.                                     */
/*                                                                     */
/* Accepts: (In)  The value to be represented as a string              */
/*          (In)  The base in which to represent the value [1..16]     */
/*          (In)  A pointer to the buffer                              */
/*          (Out) The length of the buffer                             */
/*          (In)  True to prepend negative symbol                      */
/*                                                                     */
/* Returns: the ptr position                                           */
/***********************************************************************/
char* convertUInt64ToString(STAFUInt64_t value,
                            unsigned int base,
                            char *ptr,
                            unsigned int &len,
                            bool negative)
{
    // ptr is a pointer to the buffer used to create the string 
    // equivalent of the numeric value, which ends up right justified. 

    do
    {
        // 0x30 - 0x39 are ascii values for number 0-9
        if ((*ptr = (value % base) + 0x30) > 0x39) 
            *ptr += 7;
        ptr--;
        len++;
    }
    while (value /= base);
    
   if (negative)
   {
       // Prepend a negative sign, "-".  (0x2d is ascii value for -)
       *ptr = 0x2d;
       len++;
   }
   else
   {
       ptr++; // Adjust ptr to beginning of the string
   }

    return ptr;
}


STAFRC_t STAFStringConstruct(STAFString_t *pString, 
                             const char *buffer,
                             unsigned int len, unsigned int *osRC)
{
    // This is the master constructor and is used by all other constructors.
    // this constructor allocates at least MIN bytes and increases by powers
    // of 2 up to MAX bytes; after that, allocation is done in increments of
    // DELTA bytes.

    // If buffer is 0 or len is 0, create an empty string

    if (pString == 0) return kSTAFInvalidObject;;

    *pString = new STAFStringImplementation;
    STAFStringImplementation &str = **pString;

    if (buffer == 0 || len == 0)
    {
        str.pBuffer  = (char *)EMPTY_STRING;
        str.fBuffLen = 0;
        str.fCharLen = 0;
        str.fByteLen = 0;
        return kSTAFOk;
    }

    // Determine the size of the new buffer to allocate. 

    str.fBuffLen = getBufferSize(len);
    str.pBuffer  = new char[str.fBuffLen];

    // Copy string into buffer

    memcpy(str.pBuffer, buffer, len);

    // Set byte length and compute char length of string

    str.fByteLen = len;
    
    // Calculate the length of the string in chars

    str.fCharLen = 0;

    char *ptr = (char *)(buffer);
    char *lim = (char *)(buffer + len);

    while (ptr < lim) 
    {
        str.fCharLen++;
        
        if (BYTES(ptr) == 0)
        {
            STAFTrace::trace(kSTAFTraceError,
                             "STAFStringConstruct::Invalid UTF-8 data");
            return kSTAFConverterError;
        }
        
        ptr = NEXT(ptr);
    }

    return kSTAFOk;
}


STAFRC_t STAFStringConstructCopy(STAFString_t *pString, 
                                 STAFStringConst_t aString,
                                 unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    if (aString == 0 || aString->fByteLen == 0)
        return STAFStringConstruct(pString, 0, 0, osRC);

    return STAFStringConstruct(pString, aString->pBuffer, 
        aString->fByteLen, osRC);
}



STAFRC_t STAFStringConstructFromCurrentCodePage(STAFString_t *pString,
                                                const char *from,
                                                unsigned int len,
                                                unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    if (from == 0 || len == 0)
        return STAFStringConstruct(pString, 0, 0, osRC);
    
    STAFRC_t rc = kSTAFOk;

    STAFConverter *convPtr;
    convPtr = getConverterInstance();

    const unsigned int SIZE = 4096;
    const unsigned char *fromPtr = (unsigned char *)from;
    unsigned int fromLen = len;
    unsigned char *toPtr = new unsigned char[SIZE];
    unsigned int toLen = SIZE;
    std::string result = "";
    unsigned int resultLen = 0;

    // now let's actually do the conversion
    while (fromLen > 0)
    {
        int rc2 = convPtr->convertToUTF8(&fromPtr, &fromLen, toPtr, &toLen);

        if (rc2) 
        {
            delete[] toPtr;
            if (osRC) *osRC = 0;
            return kSTAFConverterError;
        }

        result += std::string((char *)toPtr, toLen);
        resultLen += toLen;
        toLen = SIZE;
    }

    delete[] toPtr;

    return STAFStringConstruct(pString, result.data(), resultLen, osRC);
}

STAFRC_t STAFStringConstructFromUInt(STAFString_t *pString, 
                                     unsigned int value, 
                                     unsigned int base,
                                     unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    if (base == 0 || base > 16) return kSTAFInvalidParm;

    // This buffer is used to create the string equivalent of value, which
    // ends up right justified.  Then we pass that to the constructor

    char buffer[32];
    unsigned int len = 0;
    
    char *ptr = convertUInt64ToString(
        static_cast<STAFUInt64_t>(value), base, &buffer[31], len, false);

    return STAFStringConstruct(pString, ptr, len, osRC);
}

STAFRC_t STAFStringConstructFromUInt64(STAFString_t *pString, 
                                       STAFUInt64_t value, 
                                       unsigned int base,
                                       unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    if (base == 0 || base > 16) return kSTAFInvalidParm;

    // This buffer is used to create the string equivalent of value, which
    // ends up right justified.  Then we pass that to the constructor

    char buffer[32];
    unsigned int len = 0;

    char *ptr = convertUInt64ToString(value, base, &buffer[31], len, false);

    return STAFStringConstruct(pString, ptr, len, osRC);
}

STAFRC_t STAFStringConstructFromInt64(STAFString_t *pString, 
                                      STAFInt64_t value, 
                                      unsigned int base,
                                      unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    if (base == 0 || base > 16) return kSTAFInvalidParm;

    // This buffer is used to create the string equivalent of value, which
    // ends up right justified.  Then we pass that to the constructor

    char buffer[32];
    unsigned int len = 0;
    bool negative = false;

    if (value < 0)
    {
        // Convert value to a positive number

        value = value * -1;
        negative = true;
    }

    char *ptr = convertUInt64ToString(
        static_cast<STAFUInt64_t>(value), base, &buffer[31], len,
        negative);

    return STAFStringConstruct(pString, ptr, len, osRC);
}

STAFRC_t STAFStringConstructSubString(STAFString_t *pSubStr,
                                      STAFStringConst_t aString,
                                      unsigned int index, 
                                      unsigned int len, 
                                      unsigned int corb, 
                                      unsigned int *osRC)
{
    if (pSubStr == 0) return kSTAFInvalidObject;

    if (aString == 0 || aString->fByteLen == 0)
        return STAFStringConstruct(pSubStr, 0, 0, osRC);

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    // if index is beyond string's length, return empty string

    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return STAFStringConstruct(pSubStr, 0, 0, osRC);

    ptr = FWDN(ptr, index, corb);

    // if len is beyond string's length - index, return rest of string

    if (len < (corb ? aString->fByteLen : aString->fCharLen) - index)
        lim = FWDN(ptr, len, corb);

    return STAFStringConstruct(pSubStr, ptr, lim - ptr, osRC);
}


STAFRC_t STAFStringConstructSubWord(STAFString_t *pWord,
                                    STAFStringConst_t aString,
                                    unsigned int index, 
                                    unsigned int count, 
                                    unsigned int *osRC)
{
    char *ptr  = 0;
    char *lim  = 0;
    char *str  = 0;
    char *end  = 0;

    if (pWord == 0) return kSTAFInvalidObject;

    if (aString == 0) return kSTAFInvalidParm;

    ptr = aString->pBuffer;
    lim = aString->pBuffer + aString->fByteLen;

    while (ptr < lim  && WHITESPACE(ptr))
    {
        ptr = NEXT(ptr);
    }

    if (ptr >= lim)
    {
        return STAFStringConstruct(pWord, 0, 0, osRC);
    }

    str = ptr;
    end = PREV(lim);

    while (end >= str && WHITESPACE(end))
    {
        end = PREV(end);
    }

    if (end < str)
    {
        return STAFStringConstruct(pWord, 0, 0, osRC);
    }

    end = NEXT(end);

    while (ptr < end && index--)
    {
        while (ptr < end &&  WHITESPACE(ptr))
        {
            ptr = NEXT(ptr);
        }

        if (ptr >= end)
        {
            break;
        }

        while (ptr < end && !WHITESPACE(ptr))
        {
            ptr = NEXT(ptr);
        }

        if (ptr >= end)
        {
            break;
        }

        while (ptr < end &&  WHITESPACE(ptr))
        {
            ptr = NEXT(ptr);
        }
    }

    str = ptr;

    while (ptr < end && count--)
    {
        while (ptr < end &&  WHITESPACE(ptr))
        {
            ptr = NEXT(ptr);
        }

        if (ptr >= end)
        {
            break;
        }

        while (ptr < end && !WHITESPACE(ptr))
        {
            ptr = NEXT(ptr);
        }
    }

    end = ptr;

    return STAFStringConstruct(pWord, str, end - str, osRC);
}

STAFRC_t STAFStringConstructChar(STAFString_t *pChar,
                                 STAFUTF8Char_t aChar,
                                 unsigned int *osRC)
{
    if (pChar == 0) return kSTAFInvalidObject;

    return STAFStringConstructCopy(pChar, &CHAR_TABLE[aChar],
                                   osRC);
}

STAFRC_t STAFStringConstructJoin(STAFString_t *pString,
                                 STAFString_t aStringArray[],
                                 unsigned int arraySize,
                                 unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    *pString = new STAFStringImplementation;
    STAFStringImplementation &str = **pString;

    // Determine the size of the new buffer to allocate by reading the
    // string array and determine the combined length of all the strings
    // in the array

    unsigned int joinByteLen = 0;
    unsigned int joinCharLen = 0;
    unsigned int i = 0;

    for (i = 0; i < arraySize; ++i)
    {
        if (aStringArray[i] == 0)
            continue;
        
        joinByteLen += aStringArray[i]->fByteLen;
        joinCharLen += aStringArray[i]->fCharLen;
    }
    
    // If joinByteLen is 0, create an empty string

    if (joinByteLen == 0)
    {
        str.pBuffer  = (char *)EMPTY_STRING;
        str.fBuffLen = 0;
        str.fCharLen = 0;
        str.fByteLen = 0;
        return kSTAFOk;
    }

    // Determine size of new buffer to allocate (with extra space)

    unsigned int joinBuffLen = getBufferSize(joinByteLen);

    // Allocate the buffer to contain all of the strings joined together

    char *newbuff = new char[joinBuffLen];

    memset(newbuff, 0, joinBuffLen);

    // Read string array and copy each string into the buffer
    
    unsigned int currLen = 0;

    for (i = 0; i < arraySize; ++i)
    {
        if ((aStringArray[i] == 0) ||
            (aStringArray[i]->pBuffer == EMPTY_STRING))
        {
            continue;
        }
        
        memcpy(newbuff + currLen, aStringArray[i]->pBuffer,
               aStringArray[i]->fByteLen);

        currLen += aStringArray[i]->fByteLen;
    }

    str.pBuffer = newbuff;
    str.fBuffLen = joinBuffLen;
    str.fCharLen = joinCharLen;
    str.fByteLen = joinByteLen;

    return kSTAFOk;
}

STAFRC_t STAFStringAssign(STAFString_t aTarget,
                          STAFStringConst_t aSource,
                          unsigned int *osRC)
{
    // if space allocated in target is not much larger (but
    // sufficient to contain source), then just do a memcpy.
    // if space allocated in target is much larger or space 
    // allocated in target is not sufficient, reallocate to
    // the size of source and do memcpy.

    if ((aTarget->fBuffLen >= aSource->fBuffLen) &&
        (aTarget->fBuffLen < 2 * aSource->fBuffLen)) 
    {
        memcpy(aTarget->pBuffer, aSource->pBuffer,
               aSource->fByteLen);

        aTarget->fCharLen = aSource->fCharLen;
        aTarget->fByteLen = aSource->fByteLen;
    }
    else
    {
        if (aTarget->pBuffer != EMPTY_STRING)
            delete[] aTarget->pBuffer;
        aTarget->pBuffer = new char[aSource->fBuffLen];

        memcpy(aTarget->pBuffer, aSource->pBuffer,
               aSource->fByteLen);

        aTarget->fCharLen = aSource->fCharLen;
        aTarget->fByteLen = aSource->fByteLen;
        aTarget->fBuffLen = aSource->fBuffLen;
    }

    return kSTAFOk;
}


STAFRC_t STAFStringNumOfWords(STAFStringConst_t aString,
                              unsigned int *num,
                              unsigned int *osRC)
{
   
    if (aString == 0) return kSTAFInvalidObject;

    if (num == 0) return kSTAFInvalidParm;

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    *num = 0;  // default to none

    while (ptr < lim)
    {
        // iterate while blank, when non-blank increment count,
        // iterate while non-blank, loop 

        while (ptr < lim && WHITESPACE(ptr)) 
            ptr = NEXT(ptr);

        if (ptr >= lim) break;

        (*num)++;

        while (ptr < lim && !WHITESPACE(ptr)) 
            ptr = NEXT(ptr);
    }

    return kSTAFOk;
}


STAFRC_t STAFStringGetBuffer(STAFStringConst_t aString,
                             const char **buffer, 
                             unsigned int *len,
                             unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (buffer == 0) return kSTAFInvalidParm;

    *buffer = aString->pBuffer;
    
    if (len) *len = aString->fByteLen;

    return kSTAFOk;
}


STAFRC_t STAFStringToLowerCase(STAFString_t aString, unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    // ASCII 0x41 .. 0x5a are upper -> | 0x20 -> lower

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    // XXX: may not work on other systems

    while (ptr < lim)
    {
        if (*ptr >= 0x41 && *ptr <= 0x5a) 
            *ptr |= 0x20;

        ptr = NEXT(ptr);
    }
    
    return kSTAFOk;
}


STAFRC_t STAFStringToUpperCase(STAFString_t aString, unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    // ASCII 0x61 .. 0x7a are lower -> & 0xdf -> upper

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    // XXX: may not work on other systems

    while (ptr < lim)
    {
        if (*ptr >= 0x61 && *ptr <= 0x7a) 
            *ptr &= 0xdf;

        ptr = NEXT(ptr);
    }
    
    return kSTAFOk;
}


STAFRC_t STAFStringReplace(STAFString_t aString, 
                           STAFStringConst_t oldString,
                           STAFStringConst_t newString, 
                           unsigned int *osRC)
{
    unsigned int pos       = 0;
    unsigned int next      = 0;
    unsigned int ptrLen    = 0;
    unsigned int limLen    = 0;
    unsigned int oldStrLen = 0;
    unsigned int newStrLen = 0;    
    char *ptr    = 0;
    char *lim    = 0;
    char *oldStr = 0;
    char *newStr = 0;    

    if (aString == 0) return kSTAFInvalidObject;

    if (oldString == 0 || newString == 0)
        return kSTAFInvalidParm;

    ptr       = aString->pBuffer;    
    newStr    = newString->pBuffer;
    newStrLen = newString->fByteLen;
    STAFStringLength(oldString, &oldStrLen, ISBYTE, osRC);
    
    // For performance reasons, the following algorithm is used:
    // 1) Check if there's at least one occurrences of the
    //    replacement string, and if there's not, return.
    // 2) Calculates a buffer/byte size that is large enough to contain
    //    the entire new string and allocates this new buffer.
    //    If the size of the replacement string is larger, then it
    //    counts how many occurrences of the replacement string
    //    exists and multiplies it by the size difference and and
    //    and adds this to the existing string size. 
    // 3) Copies the content up to the first occurrence of the string
    //    to replace to the new buffer.
    // 4) In a loop, copies the replacement string to the new buffer
    //    and finds the next occurrence to replace and copies the
    //    content up to that occurrence to the new buffer and repeats
    //    until no more occurrences are found.
    // 5) Copies any remaining content to the new buffer.
    int newBufSize = getBufferSize(aString->fByteLen);
    int newBufByteLen = aString->fByteLen;
    
    // Check if at least one occurrence of the substring is
    // found.  If 0 occurrences are found, return.
    unsigned int subStringCount = 0;
    
    STAFStringCountSubStrings(aString, oldString, &subStringCount, osRC);
       
    if (subStringCount == 0) return kSTAFOk; 
        
    if (oldStrLen < newStrLen)
    {
        // The length of the substring being replaced is less than
        // the length of the replacement substring.

        // Calculate a buffer size and byte length that
        // will be large enough to contain the entire string after
        // all replacements have been done.
        newBufSize    = getBufferSize(aString->fByteLen + 
                        ((newStrLen - oldStrLen) * subStringCount));
        newBufByteLen += (newStrLen - oldStrLen) * subStringCount;
    }
    else
    {
        // The length of the substring being replaced is more than
        // the length of the replacement substring, so we need
        // to decrease the byte length.
        newBufByteLen -= (oldStrLen - newStrLen) * subStringCount;
    }
    
    // creating newBuffer and buffer pointer
    char *newBuffer = new char[newBufSize];
    unsigned int newBufferPtr = 0;
    
    if (newBuffer == 0) return kSTAFBaseOSError;

    memset(newBuffer, 0, newBufSize);

    // find the first occurrence and set it to pos
    STAFStringFind(aString, oldString, next, ISBYTE, &pos, osRC);
    
    // Copy the beginning content up to the first occurrence
    memcpy(newBuffer, aString->pBuffer, pos);
    // increment the buffer position
    newBufferPtr += pos;
    
    while (pos != 0xffffffff)
    {
        oldStr  = ptr + pos;
        ptrLen  = oldStr - ptr;
        lim     = oldStr + oldStrLen;
        limLen  = aString->fByteLen - ptrLen - oldStrLen;
        next    = pos + oldStrLen;
       
        // performing stringReplace
        memcpy(newBuffer + newBufferPtr, newStr, newStrLen);
        // increment the buffer position
        newBufferPtr += newStrLen; 
        
        // find the next occurrence and set it to pos
        STAFStringFind(aString, oldString, next, ISBYTE, &pos, osRC);
        
        if (pos == 0xffffffff)
        {
            // if this is the last loop, store any remaining
            // content into the newBuffer            
            memcpy(newBuffer + newBufferPtr, lim, limLen);
        }
        else
        {
            // else store the content up to the next occurrence
            memcpy(newBuffer + newBufferPtr, lim, pos - next);
            // increment the buffer position
            newBufferPtr += (pos - next);
        }        
    }
    
    // Clear out the original aString
    if (aString->pBuffer != EMPTY_STRING)
        delete[] aString->pBuffer;
    
    // Update aString to the newBuffer    
    aString->pBuffer  = newBuffer;
    aString->fBuffLen = newBufSize;
    aString->fByteLen = newBufByteLen;
    
    // a bummer, but since we keep this info, we need to recompute
    // (not trivial to do while replacing since we allow substring
    // replacement, as instead of char replacement)
    aString->fCharLen = 0;

    ptr = (char *)aString->pBuffer;
    lim = (char *)aString->pBuffer + aString->fByteLen;

    while (ptr < lim)
    {
        aString->fCharLen++;
        ptr = NEXT(ptr);
    }
     
    return kSTAFOk;
}


STAFRC_t STAFStringToCurrentCodePage(STAFStringConst_t aString, 
                                     char **to, 
                                     unsigned int *len,
                                     unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (aString == 0) return kSTAFInvalidObject;
    
    STAFConverter *convPtr;
    convPtr = getConverterInstance();
    
    const unsigned int SIZE = 4096;
    const unsigned char *fromPtr = (unsigned char *)aString->pBuffer;
    unsigned int fromLen = aString->fByteLen;
    unsigned char *toPtr = new unsigned char[SIZE];
    unsigned int toLen = SIZE;
    std::string result = "";
    unsigned int resultLen = 0;

    // now let's actually do the conversion
    while (fromLen > 0)
    {
        int rc2 = convPtr->convertFromUTF8(&fromPtr, &fromLen, toPtr, &toLen);

        if (rc2) 
        {
            delete[] toPtr;
            if (osRC) *osRC = 0;
            return kSTAFConverterError;
        }

        result += std::string((char *)toPtr, toLen);
        resultLen += toLen;
        toLen = SIZE;
    }

    delete[] toPtr;

    *to = new char[result.length() + 1];
    memcpy(*to, result.data(), result.length());
    (*to)[result.length()] = 0;
    *len = result.length();

    return kSTAFOk;
}


STAFRC_t STAFStringConcatenate(STAFString_t aString, 
                               STAFStringConst_t aSource,
                               unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (aSource == 0) return kSTAFInvalidParm;

    char *ptr = 0;

    // pre : t = "hello ", s = "world"
    // post: t = "hello world", s = "world"

    if (aString->fBuffLen > aString->fByteLen + aSource->fByteLen)
    {
        ptr = aString->pBuffer + aString->fByteLen;
        memcpy(ptr, aSource->pBuffer, aSource->fByteLen);
        aString->fByteLen += aSource->fByteLen;
        aString->fCharLen += aSource->fCharLen;
    }
    else
    {
        // Determine size of new buffer to allocate (with extra space)
        int concatBufLen = getBufferSize(
            aString->fByteLen + aSource->fByteLen);

        char *newbuff = new char[concatBufLen];

        if (newbuff == 0) return kSTAFBaseOSError;

        memset(newbuff, 0, concatBufLen);
        memcpy(newbuff, aString->pBuffer, aString->fByteLen);
        memcpy(newbuff + aString->fByteLen, aSource->pBuffer, 
               aSource->fByteLen);

        if (aString->pBuffer != EMPTY_STRING)
            delete[] aString->pBuffer;

        aString->pBuffer = newbuff;
        aString->fBuffLen = concatBufLen;
        aString->fCharLen += aSource->fCharLen;
        aString->fByteLen += aSource->fByteLen;
    }

    return kSTAFOk;
}


STAFRC_t STAFStringCountSubStrings(STAFStringConst_t aString,
                                   STAFStringConst_t aSubStr,
                                   unsigned int *count,
                                   unsigned int *osRC)
{
    if ((aString == 0) || (aSubStr == 0)) return kSTAFInvalidObject;

    if (count == 0) return kSTAFInvalidParm;

    *count = 0;  // default to none found

    if (aSubStr->fByteLen <= aString->fByteLen)
    {
        // count number of times the substring is found within the string

        char *lim = aString->pBuffer + (aString->fByteLen - aSubStr->fByteLen);
        char *ptr = aString->pBuffer;
        const char *key = aSubStr->pBuffer;

        while (ptr <= lim)
        {
            // check lead bytes, if equal do comparison
            if (*ptr == *key)
            {
                if (memcmp(key, ptr, aSubStr->fByteLen) == 0)
                    (*count)++;
            }
            
            ptr = NEXT(ptr);
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFStringStripCharsOfType(STAFString_t aInOutStr, 
                                    STAFUTF8CharType_t aType,  
                                    unsigned int side,
                                    unsigned int *osRC)
{
    if (aInOutStr == 0 || side > 2) return kSTAFInvalidObject;

    // if 0 then left, if 1 then right, if 2 then both

    char *ptr = 0;
    char *lim = 0;

    if (aInOutStr->pBuffer == 0) return kSTAFOk;

    if (side == 1 || side == 2)
    {
        ptr = aInOutStr->pBuffer + aInOutStr->fByteLen;
        ptr = PREV(ptr);
        lim = aInOutStr->pBuffer;

        switch (aType)
        {
            case kUTF8_TYPE_WHITESPACE:
        
            while (ptr >= lim && WHITESPACE(ptr))
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = PREV(ptr);
            }
            break;

            case kUTF8_TYPE_SPACE:
        
            while (ptr >= lim && SPACE(ptr))
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = PREV(ptr);
            }
            break;
            
            case kUTF8_TYPE_ASCII:
        
            while (ptr >= lim && ASCII(ptr))
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = PREV(ptr);
            }
            break;

            case kUTF8_TYPE_DIGIT:
        
            while (ptr >= lim && DIGIT(ptr))
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = PREV(ptr);
            }
            break;

            case kUTF8_TYPE_ZERO:

            while (ptr >= lim && ZERO(ptr))
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = PREV(ptr);
            }
            break;

            default:
            break;
        }
    }

    if (side == 0 || side == 2)
    {
        ptr = aInOutStr->pBuffer;
        lim = aInOutStr->pBuffer + aInOutStr->fByteLen;

        switch (aType)
        {
            case kUTF8_TYPE_WHITESPACE:

            while (ptr < lim && WHITESPACE(ptr)) 
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = NEXT(ptr);
            }
            break;

            case kUTF8_TYPE_SPACE:

            while (ptr < lim && SPACE(ptr)) 
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = NEXT(ptr);
            }
            break;

            case kUTF8_TYPE_ASCII:

            while (ptr < lim && ASCII(ptr)) 
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = NEXT(ptr);
            }
            break;

            case kUTF8_TYPE_DIGIT:

            while (ptr < lim && DIGIT(ptr)) 
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = NEXT(ptr);
            }
            break;

            case kUTF8_TYPE_ZERO:
            
            while (ptr < lim && ZERO(ptr)) 
            {
                aInOutStr->fByteLen -= BYTES(ptr);
                aInOutStr->fCharLen--;
                ptr = NEXT(ptr);
            }
            break;
        
            default:
            break;
        }

        memmove(aInOutStr->pBuffer, ptr, aInOutStr->fByteLen);
    }

    return kSTAFOk;
}


STAFRC_t STAFStringToUInt(STAFStringConst_t aString, unsigned int *value,
                          unsigned int base, unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (value == 0 || base == 0 || base > 16) return kSTAFInvalidParm;

    *value = 0;

    // Find the index of the first byte that isn't a zero
    
    unsigned int firstNonZeroIndex = 0;

    STAFRC_t rc = STAFStringFindFirstNotOf(
        aString, STAFString("0").getImpl(), 0, 0, &firstNonZeroIndex, osRC);

    if ((rc == 0) && (firstNonZeroIndex == 0xffffffff))
    {
        // String contains no non-zero characters, so the value is 0
        return kSTAFOk;
    }

    if ((aString->fByteLen - firstNonZeroIndex) > 10)
    {
        // Error if the string length (not including any leading zeros)
        // exceeds the number of digits in UINT_MAX (4294967295) which
        // is 10
        
        return kSTAFInvalidValue;
    }

    unsigned int number = 0;

    // Go to the last digit and work backwards
    
    char *ptr = aString->pBuffer + aString->fByteLen - 1;
    unsigned int dig, mult = 1;
    unsigned int digitPos = 1;  // Which digit, starting backwards

    while (ptr >= aString->pBuffer)
    {
        // If it's an ASCII A..F (or a..f), convert to its numeric value

        if ( ((*ptr | 0x20) >= 0x61) && ((*ptr | 0x20) <= 0x66) )
            dig = (*ptr | 0x20) - 0x57;  // A = 10, B = 11, ... and so on
        else if ( (*ptr >= 0x30) && (*ptr <= 0x39) )
            dig = *ptr - 0x30;  // '0' = 0, '1' = 1, ... and so on
        else
            return kSTAFInvalidValue;

        // If it's an invalid digit for this base, then error

        if (dig >= base) return kSTAFInvalidParm;  // invalid argument

        // If number will exceed UINT_MAX. then error

        if (((digitPos == 10) && (dig > 3) && (number > 294967295)) ||
            ((digitPos >  10) && (dig > 0)))
        {
            return kSTAFInvalidValue;  // Error since > 4294967295
        }
        
        number += dig * mult;

        mult *= base;
        ptr--;
        digitPos++;
    }

    *value = number;

    return kSTAFOk;
}


/* XXX: Commented out until get UINT64_MAX working on Solaris
STAFRC_t STAFStringToUInt64(STAFStringConst_t aString, STAFUInt64_t *value,
                            unsigned int base, unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (value == 0 || base == 0 || base > 16) return kSTAFInvalidParm;

    *value = 0;

    // Find the index of the first byte that isn't a zero
    
    unsigned int firstNonZeroIndex = 0;

    STAFRC_t rc = STAFStringFindFirstNotOf(
        aString, STAFString("0").getImpl(), 0, 0, &firstNonZeroIndex, osRC);

    if ((rc == 0) && (firstNonZeroIndex == 0xffffffff))
    {
        // String contains no non-zero characters, so the value is 0
        return kSTAFOk;
    }

    if ((aString->fByteLen - firstNonZeroIndex) > 20)
    {
        // Error if the string length (not including any leading zeros)
        // exceeds the number of digits in UINT64_MAX (18446744073709551615)
        // which is 20
        
        return kSTAFInvalidValue;
    }

    STAFUInt64_t number = 0;

    // Go to the last digit and work backwards
    
    char *ptr = aString->pBuffer + aString->fByteLen - 1;
    unsigned int dig;
    STAFUInt64_t mult = 1;
    unsigned int digitPos = 1;  // Which digit, starting backwards

    while (ptr >= aString->pBuffer)
    {
        // If it's an ASCII A..F (or a..f), convert to its numeric value

        if ( ((*ptr | 0x20) >= 0x61) && ((*ptr | 0x20) <= 0x66) )
            dig = (*ptr | 0x20) - 0x57;  // A = 10, B = 11, ... and so on
        else if ( (*ptr >= 0x30) && (*ptr <= 0x39) )
            dig = *ptr - 0x30;  // '0' = 0, '1' = 1, ... and so on
        else
            return kSTAFInvalidValue;

        // If it's an invalid digit for this base, then error

        if (dig >= base) return kSTAFInvalidParm;  // invalid argument

        // If number will exceed UINT64_MAX, then error

        if ((digitPos > 19) && (dig > 0) &&
            (number > UINT64_MAX_LESS_FIRST_DIGIT))
            return kSTAFInvalidValue;  // Error since > 18446744073709551615

        number += dig * mult;
        
        mult *= base;
        ptr--;
        digitPos++;
    }

    *value = number;

    return kSTAFOk;
}
*/


STAFRC_t STAFStringLength(STAFStringConst_t aString, unsigned int *len,
                          unsigned int corb, unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (len == 0) return kSTAFInvalidParm;

    *len = (corb ? aString->fByteLen : aString->fCharLen);

    return kSTAFOk;
}


STAFRC_t STAFStringSizeOfChar(STAFStringConst_t aString, 
                              unsigned int index,
                              unsigned int corb, 
                              unsigned int *len, unsigned int *osRC)
{ 
    if (aString == 0) return kSTAFInvalidObject;

    if (len == 0) return kSTAFInvalidParm;

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    *len = 0;  // default to 0 (helps on error)

    // if index is beyond string's length, return error
 
    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return kSTAFInvalidObject;  // invalid argument

    ptr = FWDN(ptr, index, corb);

    *len = BYTES(ptr);
    
    return kSTAFOk;
}


STAFRC_t STAFStringByteIndexOfChar(STAFStringConst_t aString, 
                                   unsigned int index, unsigned int *pos, 
                                   unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (pos == 0) return kSTAFInvalidParm;

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    *pos = 0xffffffff;  // default to not found
 
    // if index is beyond string's length, return error
 
    if (index >= aString->fCharLen)
        return kSTAFInvalidObject;  // invalid argument

    ptr = FWDC(ptr, index);

    *pos = ptr - aString->pBuffer;

    return kSTAFOk;
}


STAFRC_t STAFStringIsCharsOfType(STAFStringConst_t aString,
                                 const STAFUTF8CharType_t aType,
                                 unsigned int *result,
                                 unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (result == 0) return kSTAFInvalidParm;

    char *ptr = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    *result = 0;  // default to false

    switch (aType)
    {
        case kUTF8_TYPE_WHITESPACE:

            while (ptr < lim && WHITESPACE(ptr))
                ptr = NEXT(ptr);

            if (ptr >= lim)
                *result = 1;
                    
            break;

        case kUTF8_TYPE_SPACE:

            while (ptr < lim && SPACE(ptr))
                ptr = NEXT(ptr);

            if (ptr >= lim)
                *result = 1;
                    
            break;

        case kUTF8_TYPE_ASCII:

            while (ptr < lim && ASCII(ptr))
                ptr = NEXT(ptr);
            
            if (ptr >= lim)
                *result = 1;

            break;

        case kUTF8_TYPE_DIGIT:

            while (ptr < lim && DIGIT(ptr))
                ptr = NEXT(ptr);
            
            if (ptr >= lim)
                *result = 1;

            break;
 
        default:
            break;
    }

    return kSTAFOk;
}

STAFRC_t STAFStringIsEqualTo(STAFStringConst_t aFirst,
                             STAFStringConst_t aSecond,
                             STAFStringCaseSensitive_t sensitive,
                             unsigned int *comparison,
                             unsigned int *osRC)
{
    if ((aFirst == 0) || (aSecond == 0)) return kSTAFInvalidObject;

    if (comparison == 0) return kSTAFInvalidParm;

    *comparison = 1;  // default to true

    if ( (aFirst->fByteLen != aSecond->fByteLen) ||
         (aFirst->fCharLen != aSecond->fCharLen) )
    {
        // different
        *comparison = 0;
    }
    else if (sensitive == kSTAFStringCaseInsensitive)  // case insensitive
    {
        char *ptr1, *ptr2, *lim1;

        ptr1 = aFirst->pBuffer;
        ptr2 = aSecond->pBuffer;
        lim1 = aFirst->pBuffer + aFirst->fByteLen;

        while (ptr1 < lim1 && *comparison)
        {
            if ( (*ptr1 >= 0x41 && *ptr1 <= 0x5a) ||
                 (*ptr1 >= 0x61 && *ptr1 <= 0x7a) )
            {
                if ((*ptr1 | 0x20) != (*ptr2 | 0x20))
                    *comparison = 0;
            }
            else if (*ptr1 != *ptr2) *comparison = 0;

            ptr1 = NEXT(ptr1);
            ptr2 = NEXT(ptr2);
        }
    }
    else  // case sensitive, just do a memory comparison
    {
        if (memcmp(aFirst->pBuffer, aSecond->pBuffer, aFirst->fByteLen))
            *comparison = 0;
    }

    return kSTAFOk;
}


STAFRC_t STAFStringCompareTo(STAFStringConst_t aFirst,
                             STAFStringConst_t aSecond,
                             unsigned int *whichLess,
                             unsigned int *osRC)
{
    if ((aFirst == 0) || (aSecond == 0)) return kSTAFInvalidObject;

    if (whichLess == 0) return kSTAFInvalidParm;

    unsigned int min = (aFirst->fByteLen <= aSecond->fByteLen ? 
                        aFirst->fByteLen : aSecond->fByteLen);

    int cmp = memcmp(aFirst->pBuffer, aSecond->pBuffer, min);

    if (cmp <  0) 
        *whichLess = 1;  // first one is less 
    else if (cmp == 0) 
    {
        // must actually ensure that sizes were equal, otherwise
        // mark the shorter one as being the "less" one

        if (aFirst->fByteLen == aSecond->fByteLen)
            *whichLess = 0;  // both are equal
        else if (aFirst->fByteLen < aSecond->fByteLen)
            *whichLess = 1;  // first one is less
        else 
            *whichLess = 2;  // second one is less
    }
    else if (cmp >  0) 
        *whichLess = 2;  // second one is less

    return kSTAFOk;
}


STAFRC_t STAFStringStartsWith(STAFStringConst_t aString,
                              STAFStringConst_t startsWithString,
                              unsigned int *startsWith,
                              unsigned int *osRC)
{
    if ((aString == 0) || (startsWithString == 0)) return kSTAFInvalidObject;

    if (startsWith == 0) return kSTAFInvalidParm;

    *startsWith = 0;

    if (aString->fByteLen >= startsWithString->fByteLen)
    {
        int cmp = memcmp(aString->pBuffer, startsWithString->pBuffer,
                         startsWithString->fByteLen);

        if (cmp == 0) *startsWith = 1;
    }

    return kSTAFOk;
}


STAFRC_t STAFStringContainsWildcard(STAFStringConst_t aString,
                                    unsigned int *hasWildcard,
                                    unsigned int *osRC)
{
    if (aString == 0) return kSTAFInvalidObject;

    if (hasWildcard == 0) return kSTAFInvalidParm;

    *hasWildcard = 0;  // Default to does not contain a wildcard

    unsigned int theIndex = 0;

    STAFRC_t rc = STAFStringFindFirstOf(aString, sWildCards.getImpl(),
                                        0, 0, &theIndex, osRC);

    if ((rc == 0) && (theIndex != 0xffffffff))
        *hasWildcard = 1;   // Contains a wildcard

    return rc;
}


STAFRC_t STAFStringMatchesWildcards(STAFStringConst_t stringToCheck,
                                    STAFStringConst_t wildcardString,
                                    STAFStringCaseSensitive_t caseSensitive,
                                    unsigned int *matches,
                                    unsigned int *osRC)
{
    if (stringToCheck == 0) return kSTAFInvalidObject;

    if (matches == 0) return kSTAFInvalidParm;

    STAFRC_t retCode = kSTAFOk;

    try
    {
        STAFString testString(stringToCheck);
        STAFString wcString(wildcardString);

        if (caseSensitive == kSTAFStringCaseInsensitive)
        {
            testString.upperCase();
            wcString.upperCase();
        }

        unsigned int currWCIndex = 0;
        unsigned int nextWCIndex = 0;
        unsigned int wcCharIndex = 0;
        STAFString wcData;
        STAFString charData;

        unsigned int currTestIndex = 0;
        unsigned int nextTestIndex = 0;
        unsigned int deltaChars = 0;
        unsigned int numQuestions = 0;
        bool hasWildcard = false;

        if ((testString.length() == 0) && (wcString.length() == 0))
        {
            *matches = 1;
        }
        else if ((testString.length() == 0) &&
                 (wcString.findFirstNotOf(sStar) != STAFString::kNPos))
        {
            *matches = 0;
        }
        else if (wcString.length() == 0)
        {
            *matches = 0;
        }
        else if (wcString == testString)
        {
            *matches = 1;
            return retCode;
        }
        else if ((wcString.findFirstOf(sStar) == STAFString::kNPos) &&
                 (testString.length() != wcString.length()))
        {
            *matches = 0;
        }
        else if ((wcString.findFirstOf(sQuestion) != STAFString::kNPos) &&
                 (testString.length() < wcString.count(sQuestion)))
        {
            *matches = 0;
        }
        else *matches = 1;

        for (;
             (*matches != 0) && (currWCIndex < wcString.length()) &&
                 (nextTestIndex < testString.length());
             currWCIndex = nextWCIndex,
                 currTestIndex = nextTestIndex + charData.length())
        {
            wcCharIndex = wcString.findFirstNotOf(sWildCards, currWCIndex);
            wcData = wcString.subString(currWCIndex, wcCharIndex - currWCIndex);
            nextWCIndex = wcString.findFirstOf(sWildCards, wcCharIndex);
            charData = wcString.subString(wcCharIndex,
                                          nextWCIndex - wcCharIndex);
            hasWildcard = (wcData.count(sStar) > 0);
            numQuestions = wcData.count(sQuestion);

            if (charData.length() != 0)
                nextTestIndex = testString.find(charData,
                                                currTestIndex + numQuestions);
            else
                nextTestIndex = testString.length();

            deltaChars = nextTestIndex - currTestIndex;
            
            if (!hasWildcard && (deltaChars > numQuestions))
                *matches = 0;
            else if (nextTestIndex == STAFString::kNPos)
                *matches = 0;
            else if (nextWCIndex == STAFString::kNPos)
            {
                // Verify remaining characters in wildcard string match
                STAFString wcRemainChars = wcString.subString(wcCharIndex,
                                                            wcString.length());

                if (wcRemainChars.length() != 0)
                {
                    if (testString.find(wcRemainChars, testString.length() - 
                        wcRemainChars.length()) == STAFString::kNPos)
                    {
                        *matches = 0;
                    }
                }
                else if (currTestIndex == testString.length() && wcData == "?")
                {
                    *matches = 0;
                }
            }
        }
    }
    catch (...)
    { retCode = kSTAFUnknownError; }

    return retCode;
}


STAFRC_t STAFStringFind(STAFStringConst_t aString,
                        STAFStringConst_t aSubStr,
                        unsigned int index, unsigned int corb,
                        unsigned int *pos,
                        unsigned int *osRC)
{
    if ((aString == 0) || (aSubStr == 0)) return kSTAFInvalidObject;

    if (pos == 0) return kSTAFInvalidParm;

    char *ptr = aString->pBuffer;
    char *key = aSubStr->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    // default to not found
    *pos = 0xffffffff;

    // This is legal.  If the user tries to find a string starting at an 
    // index that is larger than the size of the string, then the search 
    // string is simply not found.

    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return kSTAFOk;

    ptr = FWDN(ptr, index, corb);

    unsigned int loc = index;

    // this is called "the naive search string algorithm",
    // I could use something like the Boyer-Moore, or KMP,
    // but it may not be worth the extra work for the size
    // of string we will be dealing with ;)
   
    // start searching for the substring, but only look for
    // lead byte, when found call memcmp to actually do the
    // matching

    while (ptr < lim)
    {
        // loop until ptr matches key

        while ((ptr < lim) && (*ptr != *key))
        {
            ptr = NEXT(ptr);
            loc++;
        }

        // if above limits then not found
        
        if (ptr >= lim) break;

        // avoid calling memcmp if out-of-bounds memory a-
        // reas are going to be touched

        if (ptr + aSubStr->fByteLen > lim)
            break;

        if (memcmp(ptr, key, aSubStr->fByteLen) == 0)
        {
            // if we dealt with chars, return char location,
            // else return byte location
            *pos = (corb ? ptr - aString->pBuffer : loc);
            break;
        }

        ptr = NEXT(ptr);
        loc++;
    }

    return kSTAFOk;
}


STAFRC_t STAFStringFindFirstOf(STAFStringConst_t aString,
                               STAFStringConst_t aSet,
                               unsigned int index, unsigned int corb,
                               unsigned int *pos, 
                               unsigned int *osRC)
{
    if ((aString == 0) || (aSet == 0)) return kSTAFInvalidObject;

    if (pos == 0) return kSTAFInvalidParm;

    unsigned int loc = 0;
    STAFRC_t rc = kSTAFOk;
    char *key = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    *pos = 0xffffffff;  // default to not found

    // if index is beyond string's length, just say not found

    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return kSTAFOk;

    key = FWDN(key, index, corb);

    loc = index;

    struct STAFStringImplementation wrap;

    while (key < lim)
    {
        unsigned int pos2;

        wrap.pBuffer  = key;
        wrap.fBuffLen = BYTES(key);
        wrap.fByteLen = BYTES(key);
        wrap.fCharLen = 1;

        rc = STAFStringFind(aSet, &wrap, 0, 0, &pos2, osRC);

        if (rc) break;

        if (pos2 != 0xffffffff)
        {
            *pos = (corb ? key - aString->pBuffer : loc);
            break;
        }

        key = NEXT(key);
        loc++;
    }

    return rc;
}


STAFRC_t STAFStringFindLastOf(STAFStringConst_t aString,
                              STAFStringConst_t aSet,
                              unsigned int index, unsigned int corb,
                              unsigned int *pos, 
                              unsigned int *osRC)
{
    if ((aString == 0) || (aSet == 0)) return kSTAFInvalidObject;

    if (pos == 0) return kSTAFInvalidParm;

    unsigned int loc = 0;
    STAFRC_t rc = kSTAFOk;
    char *key = aString->pBuffer + aString->fByteLen;
    char *lim = aString->pBuffer;

    *pos = 0xffffffff;  // default to not found

    // if index is beyond string's length, just say not found

    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return kSTAFOk;

    lim = FWDN(lim, index, corb);
    key = REWN(key, 1, corb);

    loc = (corb ? aString->fByteLen - 1 : aString->fCharLen - 1);

    struct STAFStringImplementation wrap;

    while (key >= lim)
    {
        unsigned int pos2;

        wrap.pBuffer  = key;
        wrap.fBuffLen = BYTES(key);
        wrap.fByteLen = BYTES(key);
        wrap.fCharLen = 1;

        rc = STAFStringFind(aSet, &wrap, 0, 0, &pos2, osRC);

        if (rc) break;

        if (pos2 != 0xffffffff)
        {
            *pos = (corb ? key - aString->pBuffer : loc);
            break;
        }

        if (key == lim) break;

        key = PREV(key);
        loc--;
    }

    return rc;
}


STAFRC_t STAFStringFindFirstNotOf(STAFStringConst_t aString,
                                  STAFStringConst_t aSet,
                                  unsigned int index,
                                  unsigned int corb,
                                  unsigned int *pos, 
                                  unsigned int *osRC)
{
    if ((aString == 0) || (aSet == 0)) return kSTAFInvalidObject;

    if (pos == 0) return kSTAFInvalidParm;

    unsigned int loc = 0;
    STAFRC_t rc = kSTAFOk;
    char *key = aString->pBuffer;
    char *lim = aString->pBuffer + aString->fByteLen;

    *pos = 0xffffffff;  // default to not found

    // if index is beyond string's length, just say not found

    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return kSTAFOk;

    key = FWDN(key, index, corb);

    loc = index;

    struct STAFStringImplementation wrap;

    while (key < lim)
    {
        unsigned int pos2;

        wrap.pBuffer  = key;
        wrap.fBuffLen = BYTES(key);
        wrap.fByteLen = BYTES(key);
        wrap.fCharLen = 1;

        rc = STAFStringFind(aSet, &wrap, 0, 0, &pos2, osRC);

        if (rc) break;

        if (pos2 == 0xffffffff)
        {
            *pos = (corb ? key - aString->pBuffer : loc);
            break;
        }

        key = NEXT(key);
        loc++;
    }

    return rc;
}


STAFRC_t STAFStringFindLastNotOf(STAFStringConst_t aString,
                                 STAFStringConst_t aSet,
                                 unsigned int index,
                                 unsigned int corb,
                                 unsigned int *pos, 
                                 unsigned int *osRC)
{
    if ((aString == 0) || (aSet == 0)) return kSTAFInvalidObject;

    if (pos == 0) return kSTAFInvalidParm;

    unsigned int loc = 0;
    STAFRC_t rc = kSTAFOk;
    char *key = aString->pBuffer + aString->fByteLen;
    char *lim = aString->pBuffer;

    *pos = 0xffffffff;  // default to not found

    // if index is beyond string's length, just say not found

    if (index >= (corb ? aString->fByteLen : aString->fCharLen))
        return kSTAFOk;

    lim = FWDN(lim, index, corb);
    key = REWN(key, 1, corb);

    loc = (corb ? aString->fByteLen - 1 : aString->fCharLen - 1);

    struct STAFStringImplementation wrap;

    while (key >= lim)
    {
        unsigned int pos2;

        wrap.pBuffer  = key;
        wrap.fBuffLen = BYTES(key);
        wrap.fByteLen = BYTES(key);
        wrap.fCharLen = 1;

        rc = STAFStringFind(aSet, &wrap, 0, 0, &pos2, osRC);

        if (rc) break;

        if (pos2 == 0xffffffff)
        {
            *pos = (corb ? key - aString->pBuffer : loc);
            break;
        }

        key = PREV(key);
        loc--;
    }

    return rc;
}


STAFRC_t STAFStringDestruct(STAFString_t *pString, unsigned int *osRC)
{
    if (pString == 0) return kSTAFInvalidObject;

    if ((*pString)->pBuffer != EMPTY_STRING)
        delete[] (*pString)->pBuffer;

    delete *pString;

    *pString = 0;

    return kSTAFOk;
}


STAFRC_t STAFStringFreeBuffer(const char *buffer, unsigned int *osRC)
{
    if (buffer == 0) return kSTAFInvalidObject;

    // Note: WIN32 doesn't overload delete[] for const char* so we need
    // to cast it
    delete[] (char *)buffer;

    return kSTAFOk;
}
