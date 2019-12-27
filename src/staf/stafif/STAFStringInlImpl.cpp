/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_StringInlImpl
#define STAF_StringInlImpl

#include "STAF.h"
#include <cstdio>
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAFRefPtr.h"
#include "STAFException.h"

STAF_INLINE STAFString::STAFString() : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstruct(&fStringImpl, 0, 0, &osRC);

    STAFException::checkRC(rc, "STAFStringConstruct", osRC);
}


STAF_INLINE STAFString::STAFString(const char *buffer, unsigned int length,
                                   CodePageType cpType) : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    if (cpType == kCurrent)
    {
        // We must compute the length first. 
        // Note: Solaris 5.6 had core dump issues if we
        // computed the length in one of the args while
        // calling STAFStringConstructFromCurrentCodePage
        unsigned int computeLen = length;
        if (computeLen == 0xFFFFFFFF)
            computeLen = strlen(buffer);

        rc = STAFStringConstructFromCurrentCodePage(&fStringImpl, buffer,
             computeLen, &osRC);
    }
    else
    {
        rc = STAFStringConstruct(&fStringImpl, buffer, length, &osRC);
    }

    STAFException::checkRC(rc, "STAFStringConstruct[FromCurrentCodePage]", osRC);
}

STAF_INLINE STAFString::STAFString(unsigned int fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromUInt(
        &fStringImpl, fromValue, base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromUInt", osRC);
}

STAF_INLINE STAFString::STAFString(int fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromInt64(
        &fStringImpl, static_cast<STAFInt64_t>(fromValue), base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromInt64", osRC);
}

STAF_INLINE STAFString::STAFString(unsigned short fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromUInt64(
        &fStringImpl, static_cast<STAFUInt64_t>(fromValue), base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromUInt64", osRC);
}

STAF_INLINE STAFString::STAFString(short fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromInt64(
        &fStringImpl, static_cast<STAFInt64_t>(fromValue), base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromInt64", osRC);
}

// On 64-bit Unix machines, an unsigned long is the same as a STAFUInt64_t
// and a long is the same as a STAFInt64_t, so can't define STAFString
// constructors for both

#if !defined(STAF_OS_64BIT)
STAF_INLINE STAFString::STAFString(unsigned long fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromUInt64(
        &fStringImpl, static_cast<STAFUInt64_t>(fromValue), base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromUInt64", osRC);
}

STAF_INLINE STAFString::STAFString(long fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromInt64(
        &fStringImpl, static_cast<STAFInt64_t>(fromValue), base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromInt64", osRC);
}
#endif

STAF_INLINE STAFString::STAFString(STAFUInt64_t fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromUInt64(
        &fStringImpl, fromValue, base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromUInt64", osRC);
}

STAF_INLINE STAFString::STAFString(STAFInt64_t fromValue, unsigned int base)
    : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructFromInt64(
        &fStringImpl, fromValue, base, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructFromInt64", osRC);
}

STAF_INLINE STAFString::STAFString(const STAFString &from) : fStringImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructCopy(&fStringImpl, from.fStringImpl,
                                          &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);
}


STAF_INLINE STAFString::STAFString(STAFStringConst_t from)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    // if from is null, then construct an empty string, else 
    // construct either a shallow or deep copy
    
    if (from == 0)
        rc = STAFStringConstruct(&fStringImpl, 0, 0, &osRC);
    else
        rc = STAFStringConstructCopy(&fStringImpl, from, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);
}


STAF_INLINE STAFString::STAFString(STAFString_t from, CopyMode mode)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    // if from is null, then construct an empty string, else 
    // construct either a shallow or deep copy
    
    if (from == 0)
        rc = STAFStringConstruct(&fStringImpl, 0, 0, &osRC);
    else if (mode == kShallow)
        fStringImpl = (STAFString_t)from;
    else
        rc = STAFStringConstructCopy(&fStringImpl, from, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);
}


STAF_INLINE STAFString::STAFString(STAFUTF8Char_t aChar)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructChar(&fStringImpl, aChar,
                                          &osRC);
    STAFException::checkRC(rc, "STAFStringConstructChar", osRC);
}


STAF_INLINE STAFString STAFString::subString(unsigned int begin,
                                             unsigned int length,
                                             IndexRep corb) const
{
    STAFString_t newString;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructSubString(&newString, fStringImpl, 
                                               begin, length, corb, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructSubString", osRC);

    return STAFString(newString, STAFString::kShallow);
}


STAF_INLINE unsigned int STAFString::numWords() const
{
    unsigned int osRC = 0;
    unsigned int theNumWords = 0;
    STAFRC_t rc = STAFStringNumOfWords(fStringImpl, &theNumWords, &osRC);

    STAFException::checkRC(rc, "STAFStringNumWords", osRC);

    return theNumWords;
}


STAF_INLINE STAFString STAFString::subWord(unsigned int begin,
                                      unsigned int length) const
{
    STAFString_t newString;
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConstructSubWord(&newString, fStringImpl, 
                                             begin, length, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructSubWord", osRC);

    return STAFString(newString, STAFString::kShallow);
}
    

STAF_INLINE unsigned int STAFString::count(const STAFString &theSubStr) const
{
    unsigned int osRC = 0;
    unsigned int theCount = 0;
    STAFRC_t rc = STAFStringCountSubStrings(fStringImpl, theSubStr.fStringImpl,
                                            &theCount, &osRC);

    STAFException::checkRC(rc, "STAFStringCount", osRC);

    return theCount;
}


STAF_INLINE unsigned int STAFString::length(IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theLength = 0;
    STAFRC_t rc = STAFStringLength(fStringImpl, &theLength, corb, &osRC);

    STAFException::checkRC(rc, "STAFStringLength", osRC);

    return theLength;
}


STAF_INLINE unsigned int STAFString::sizeOfChar(unsigned int index,
                                           IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theSize = 0;
    STAFRC_t rc = STAFStringSizeOfChar(fStringImpl, index, corb, &theSize,
                                       &osRC);

    STAFException::checkRC(rc, "STAFStringSizeOfChar", osRC);

    return theSize;
}

STAF_INLINE const char *STAFString::buffer(unsigned int *bufLength) const
{
    unsigned int osRC = 0;
    const char *theData = 0;
    STAFRC_t rc = STAFStringGetBuffer(fStringImpl, &theData, bufLength,
                                      &osRC);

    STAFException::checkRC(rc, "STAFStringGetBuffer", osRC);

    return theData;
}


STAF_INLINE STAFString_t STAFString::getImpl() const
{
    return fStringImpl;
}


STAF_INLINE STAFString_t STAFString::adoptImpl()
{
    STAFString_t temp = fStringImpl;

    fStringImpl = 0;

    return temp;
}


STAF_INLINE void STAFString::replaceImpl(STAFString_t replacementImpl)
{
    STAFString_t temp = fStringImpl;

    fStringImpl = replacementImpl;

    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringDestruct(&temp, &osRC);

    STAFException::checkRC(rc, "STAFStringDestruct", osRC);
}


STAF_INLINE unsigned int STAFString::byteIndexOfChar(unsigned int charIndex) 
    const
{
    unsigned int osRC = 0;
    unsigned int theIndex = 0;
    STAFRC_t rc = STAFStringByteIndexOfChar(fStringImpl, charIndex,
                                            &theIndex, &osRC);

    STAFException::checkRC(rc, "STAFStringByteIndexOfChar", osRC);

    return theIndex;
}


STAF_INLINE unsigned int STAFString::find(const STAFString &searchFor,
                                          unsigned int begin,
                                          IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theIndex = 0;
    STAFRC_t rc = STAFStringFind(fStringImpl, searchFor.fStringImpl, begin,
                                 corb, &theIndex, &osRC);

    STAFException::checkRC(rc, "STAFStringFind", osRC);

    return theIndex;
}


STAF_INLINE unsigned int STAFString::findFirstOf(const STAFString &searchList,
                                                 unsigned int begin,
                                                 IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theIndex = 0;
    STAFRC_t rc = STAFStringFindFirstOf(fStringImpl, searchList.fStringImpl,
                                        begin, corb, &theIndex, &osRC);

    STAFException::checkRC(rc, "STAFStringFindFirstOf", osRC);

    return theIndex;
}


STAF_INLINE unsigned int STAFString::findFirstNotOf(const STAFString &searchList,
                                                    unsigned int begin,
                                                    IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theIndex = 0;
    STAFRC_t rc = STAFStringFindFirstNotOf(fStringImpl, 
                                           searchList.fStringImpl, begin,
                                           corb, &theIndex, &osRC);

    STAFException::checkRC(rc, "STAFStringFindFirstNotOf", osRC);

    return theIndex;
}


STAF_INLINE unsigned int STAFString::findLastOf(const STAFString &searchList,
                                                unsigned int begin,
                                                IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theIndex = 0;

    STAFRC_t rc = STAFStringFindLastOf(fStringImpl, searchList.fStringImpl,
                                       begin, corb, &theIndex, &osRC);

    STAFException::checkRC(rc, "STAFStringFindLastOf", osRC);

    return theIndex;
}


STAF_INLINE unsigned int STAFString::findLastNotOf(const STAFString &searchList,
                                                   unsigned int begin,
                                                   IndexRep corb) const
{
    unsigned int osRC = 0;
    unsigned int theIndex = 0;
    STAFRC_t rc = STAFStringFindLastNotOf(fStringImpl, searchList.fStringImpl,
                                          begin, corb, &theIndex, &osRC);

    STAFException::checkRC(rc, "STAFStringFindLastNotOf", osRC);

    return theIndex;
}


STAF_INLINE STAFString &STAFString::lowerCase()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringToLowerCase(fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringToLowerCase", osRC);

    return *this;
}


STAF_INLINE STAFString &STAFString::upperCase()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringToUpperCase(fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringToUpperCase", osRC);
    
    return *this;
}


STAF_INLINE STAFString &STAFString::strip(StripWhat stripWhat)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringStripCharsOfType(fStringImpl, kUTF8_TYPE_WHITESPACE,
                                             stripWhat, &osRC);

    STAFException::checkRC(rc, "STAFStringStripCharsOfType", osRC);
    
    return *this;
}


STAF_INLINE STAFString &STAFString::join(const STAFString stringArray[],
                                         unsigned int arraySize)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    if (arraySize == 0) return *this;
    
    STAFString_t temp = fStringImpl;
    unsigned int joinArraySize = arraySize + 1;
    STAFString_t *joinArray = new STAFString_t[joinArraySize];

    joinArray[0] = fStringImpl;

    for (unsigned int i = 1; i < joinArraySize; ++i)
    {
        joinArray[i] = stringArray[i - 1].getImpl();
    }
        
    rc = STAFStringConstructJoin(&fStringImpl, joinArray, joinArraySize, &osRC);

    delete [] joinArray;

    STAFException::checkRC(rc, "STAFStringConstructJoin", osRC);

    STAFStringDestruct(&temp, 0);

    return *this;
}


STAF_INLINE unsigned int STAFString::asUInt(unsigned int base) const
{
    unsigned int osRC = 0;
    unsigned int theUInt = 0;
    STAFRC_t rc = STAFStringToUInt(fStringImpl, &theUInt, base, &osRC);

    STAFException::checkRC(rc, "STAFStringToUInt", osRC);

    return theUInt;
}


STAF_INLINE unsigned int STAFString::asUIntWithDefault(
    unsigned int defaultValue, unsigned int base) const
{
    unsigned int osRC = 0;
    unsigned int theUInt = 0;
    STAFRC_t rc = STAFStringToUInt(fStringImpl, &theUInt, base, &osRC);

    if (rc == kSTAFInvalidValue)
        return defaultValue;

    STAFException::checkRC(rc, "STAFStringToUInt", osRC);

    return theUInt;
}


/* XXX: Commented out until get UINT64_MAX working on Solaris
STAF_INLINE STAFUInt64_t STAFString::asUInt64(unsigned int base) const
{
    unsigned int osRC = 0;
    STAFUInt64_t theUInt64 = 0;
    STAFRC_t rc = STAFStringToUInt64(fStringImpl, &theUInt64, base, &osRC);

    STAFException::checkRC(rc, "STAFStringToUInt64", osRC);

    return theUInt64;
}


STAF_INLINE STAFUInt64_t STAFString::asUInt64WithDefault(
    STAFUInt64_t defaultValue, unsigned int base) const
{
    unsigned int osRC = 0;
    STAFUInt64_t theUInt64 = 0;
    STAFRC_t rc = STAFStringToUInt64(fStringImpl, &theUInt64, base, &osRC);

    if (rc == kSTAFInvalidValue)
        return defaultValue;

    STAFException::checkRC(rc, "STAFStringToUInt64", osRC);

    return theUInt64;
}
*/


STAF_INLINE STAFStringBufferPtr STAFString::toCurrentCodePage() const
{
    unsigned int osRC = 0;
    char *theBuffer = 0;
    unsigned int theLength = 0;
    STAFRC_t rc = STAFStringToCurrentCodePage(fStringImpl, &theBuffer,
                                              &theLength, &osRC);

    STAFException::checkRC(rc, "STAFStringToCurrentCodePage", osRC);

    return STAFStringBufferPtr(new STAFStringBuffer(theBuffer, theLength),
                               STAFStringBufferPtr::INIT);
}


STAF_INLINE STAFString STAFString::toLowerCase() const
{
    unsigned int osRC = 0;
    STAFString_t theCopy = 0;
    STAFRC_t rc = STAFStringConstructCopy(&theCopy, fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);

    rc = STAFStringToLowerCase(theCopy, &osRC);

    if (rc != 0) STAFStringDestruct(&theCopy, 0);

    STAFException::checkRC(rc, "STAFStringToLowerCase", osRC);

    return STAFString(theCopy, STAFString::kShallow);
}


STAF_INLINE STAFString STAFString::toUpperCase() const
{
    unsigned int osRC = 0;
    STAFString_t theCopy = 0;
    STAFRC_t rc = STAFStringConstructCopy(&theCopy, fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);

    rc = STAFStringToUpperCase(theCopy, &osRC);

    if (rc != 0) STAFStringDestruct(&theCopy, 0);

    STAFException::checkRC(rc, "STAFStringToUpperCase", osRC);

    return STAFString(theCopy, STAFString::kShallow);
}


STAF_INLINE STAFString STAFString::replace(const STAFString oldstr, 
                                           const STAFString newstr) const
{
    unsigned int osRC = 0;
    STAFString_t theCopy = 0;
    STAFRC_t rc = STAFStringConstructCopy(&theCopy, fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);

    rc = STAFStringReplace(theCopy, oldstr.getImpl(), newstr.getImpl(), 
                           &osRC);

    if (rc != kSTAFOk) STAFStringDestruct(&theCopy, 0);

    STAFException::checkRC(rc, "STAFStringReplace", osRC);

    return STAFString(theCopy, STAFString::kShallow);
}


STAF_INLINE bool STAFString::isWhiteSpace() const
{
    unsigned int osRC = 0;
    unsigned int compVal = 0;
    STAFRC_t rc = STAFStringIsCharsOfType(fStringImpl, kUTF8_TYPE_WHITESPACE,
                                          &compVal, &osRC);

    STAFException::checkRC(rc, "STAFStringIsCharsOfType", osRC);

    return compVal ? true : false;
}


STAF_INLINE bool STAFString::isDigits() const
{
    unsigned int osRC = 0;
    unsigned int compVal = 0;
    STAFRC_t rc = STAFStringIsCharsOfType(fStringImpl, kUTF8_TYPE_DIGIT,
                                          &compVal, &osRC);

    STAFException::checkRC(rc, "STAFStringIsCharsOfType", osRC);

    return compVal ? true : false;
}


STAF_INLINE bool STAFString::isEqualTo(const STAFString &theString,
                             STAFStringCaseSensitive_t caseSensitive) const
{
    unsigned int osRC = 0;
    unsigned int compVal = 0;

    STAFRC_t rc = STAFStringIsEqualTo(fStringImpl, theString.fStringImpl,
                                      caseSensitive, &compVal, &osRC);

    STAFException::checkRC(rc, "STAFStringIsEqualTo", osRC);

    return compVal ? true : false;
}


STAF_INLINE bool STAFString::hasWildcard() const
{
    unsigned int osRC = 0;
    unsigned int hasWildcard = 0;

    STAFRC_t rc = STAFStringContainsWildcard(fStringImpl, &hasWildcard, &osRC);

    STAFException::checkRC(rc, "STAFStringContainsWildcard", osRC);

    return hasWildcard ? true : false;
}


STAF_INLINE bool STAFString::startsWith(const STAFString &someString) const
{
    unsigned int osRC = 0;
    unsigned int doesStartWith = 0;

    STAFRC_t rc = STAFStringStartsWith(fStringImpl, someString.fStringImpl,
                                       &doesStartWith, &osRC);

    STAFException::checkRC(rc, "STAFStringStartsWith", osRC);

    return doesStartWith ? true : false;
}


STAF_INLINE bool STAFString::matchesWildcards(const STAFString &wildcardString,
                             STAFStringCaseSensitive_t caseSensitive) const
{
    unsigned int osRC = 0;
    unsigned int matches = 0;
    STAFRC_t rc = STAFStringMatchesWildcards(fStringImpl,
                                             wildcardString.fStringImpl,
                                             caseSensitive, &matches, &osRC);
    
    STAFException::checkRC(rc, "STAFStringMatchesWildcards", osRC);

    return matches ? true : false;
}


STAF_INLINE STAFString &STAFString::operator=(const STAFString &rhs)
{
    unsigned int osRC = 0;
    STAFString_t newString = 0;
    STAFRC_t rc = STAFStringConstructCopy(&newString, rhs.fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);

    STAFString_t temp = fStringImpl;

    fStringImpl = newString;

    STAFStringDestruct(&temp, 0);

    return *this;
}


STAF_INLINE bool STAFString::operator==(const STAFString &rhs) const
{
    return isEqualTo(rhs, kSTAFStringCaseSensitive);
}


STAF_INLINE bool STAFString::operator!=(const STAFString &rhs) const
{
    return !isEqualTo(rhs, kSTAFStringCaseSensitive);
}


STAF_INLINE bool STAFString::operator<(const STAFString &rhs) const
{
    unsigned int osRC = 0;
    unsigned int compVal = 0;
    STAFRC_t rc = STAFStringCompareTo(fStringImpl, rhs.fStringImpl,
                                      &compVal, &osRC);

    STAFException::checkRC(rc, "STAFStringCompareTo", osRC);

    return (compVal == 1);
}


STAF_INLINE bool STAFString::operator<=(const STAFString &rhs) const
{
    return !(rhs < *this);
}


STAF_INLINE bool STAFString::operator>(const STAFString &rhs) const
{
    return (rhs < *this);
}


STAF_INLINE bool STAFString::operator>=(const STAFString &rhs) const
{
    return !(*this < rhs);
}


STAF_INLINE STAFString &STAFString::operator+=(const STAFString &rhs)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFStringConcatenate(fStringImpl, rhs.fStringImpl, &osRC);

    STAFException::checkRC(rc, "STAFStringConcatenate", osRC);

    return *this;
}

STAF_INLINE STAFString operator+(const STAFString &lhs, const STAFString &rhs)
{
    unsigned int osRC = 0;
    STAFString_t newString = 0;
    STAFRC_t rc = STAFStringConstructCopy(&newString, lhs.fStringImpl,
                                          &osRC);

    STAFException::checkRC(rc, "STAFStringConstructCopy", osRC);

    rc = STAFStringConcatenate(newString, rhs.fStringImpl, &osRC);

    if (rc != kSTAFOk) STAFStringDestruct(&newString, 0);

    STAFException::checkRC(rc, "STAFStringConcatenate", osRC);

    return STAFString(newString, STAFString::kShallow);
}


STAF_INLINE ostream &operator<<(ostream &os, const STAFString &rhs)
{
    STAFStringBufferPtr buf = rhs.toCurrentCodePage();

    os << buf->buffer();

    return os;
}


STAF_INLINE STAFString::~STAFString()
{
    unsigned int osRC = 0;
    if (fStringImpl) STAFStringDestruct(&fStringImpl, &osRC);
}

#endif
