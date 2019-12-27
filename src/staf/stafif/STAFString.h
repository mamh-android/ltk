/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_String
#define STAF_String

#include "STAFError.h"
#include "STAFOSTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

typedef struct STAFStringImplementation *STAFString_t;
typedef const struct STAFStringImplementation *STAFStringConst_t;


typedef enum STAFUTF8Char_e
{
    kUTF8_NULL     =  0,   /* Single byte null                               */
    kUTF8_SPACE    =  1,   /* Space                                          */
    kUTF8_LCURLY   =  2,   /* Left curly brace                               */
    kUTF8_RCURLY   =  3,   /* Right curly brace                              */
    kUTF8_LPAREN   =  4,   /* Left parenthesis                               */
    kUTF8_RPAREN   =  5,   /* Right parenthesis                              */
    kUTF8_LANGLE   =  6,   /* Left angle bracket (a.k.a, less than sign)     */
    kUTF8_RANGLE   =  7,   /* Right angle bracket (a.k.a, greater than sign) */
    kUTF8_COLON    =  8,   /* Colon                                          */
    kUTF8_SCOLON   =  9,   /* Semi-colon                                     */
    kUTF8_COMMA    = 10,   /* Comma                                          */
    kUTF8_PERIOD   = 11,   /* Period                                         */
    kUTF8_BSLASH   = 12,   /* Back-slash                                     */
    kUTF8_SLASH    = 13,   /* Slash (a.k.a., forward-slash)                  */
    kUTF8_EQUAL    = 14,   /* Equal sign                                     */
    kUTF8_SQUOTE   = 15,   /* Single quote (a.k.a., apostrophe)              */
    kUTF8_DQUOTE   = 16,   /* Double quote                                   */
    kUTF8_VBAR     = 17,   /* Vertical bar (a.k.a., pipe)                    */
    kUTF8_NULL2    = 18,   /* Double-byte null                               */
    kUTF8_POUND    = 19,   /* Pound sign (a.k.a. number sign or hash)        */
    kUTF8_CR       = 20,   /* Carriage return                                */
    kUTF8_LF       = 21,   /* Line feed                                      */
    kUTF8_STAR     = 22,   /* Star (a.k.a, asterisk)                         */
    kUTF8_HYPHEN   = 23,   /* Hyphen (a.k.a., dash)                          */
    kUTF8_PERCENT  = 24,   /* Percent sign                                   */
    kUTF8_QUESTION = 25,   /* Question mark                                  */
    kUTF8_CARET    = 26,   /* Caret (^)                                      */
    kUTF8_AMP      = 27,   /* Ampersand (&)                                  */
    kUTF8_AT       = 28,   /* At (@)                                         */
    kUTF8_TAB      = 29,   /* Tab                                            */
    kUTF8_BANG     = 30    /* Bang (a.k.a, exclaimation mark)                */
} STAFUTF8Char_t;

typedef enum STAFUTF8CharType_e
{
    kUTF8_TYPE_SPACE = 0, kUTF8_TYPE_WHITESPACE = 1,
    kUTF8_TYPE_ASCII = 2, kUTF8_TYPE_DIGIT      = 3,
    kUTF8_TYPE_ZERO  = 4
} STAFUTF8CharType_t;

typedef enum STAFStringCaseSensitive_e
{
    kSTAFStringCaseInsensitive = 0,
    kSTAFStringCaseSensitive = 1
} STAFStringCaseSensitive_t;


/***********************************************************************/
/* STAFStringConstruct - Creates a STAFString from buffer of size len  */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  A pointer to a buffer of UTF-8 chars                 */
/*          (In)  The length of the buffer                             */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstruct(STAFString_t *pString,
                             const char *buffer,
                             unsigned int len,
                             unsigned int *osRC);

/***********************************************************************/
/* STAFStringConstructCopy - Creates a STAFString from a STAFString    */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  A STAFString_t                                       */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructCopy(STAFString_t *pString,
                                 STAFStringConst_t aString,
                                 unsigned int *osRC);
                                 
/***********************************************************************/
/* STAFStringConstructFromCurrentCodePage - Creates a STAFString from  */
/*                                          a buffer of size len con-  */
/*                                          sisting of current code-   */
/*                                          page characters            */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  A pointer to a buffer of local codepage chars        */
/*          (In)  The length of the buffer                             */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructFromCurrentCodePage(
                                             STAFString_t *pString,
                                             const char *from,
                                             unsigned int len,
                                             unsigned int *osRC);

/***********************************************************************/
/* STAFStringConstructFromUInt - Creates a STAFString from an unsigned */
/*                               integer                               */
/* Note:  This is still here for legacy reasons.                       */
/*        Use STAFStringContructFromUInt64 instead for new code.       */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  The value to be represented as a string              */
/*          (In)  The base in which to represent the value [1..16]     */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructFromUInt(STAFString_t *pString,
                                     unsigned int value,
                                     unsigned int base,
                                     unsigned int *osRC);
                                     
/***********************************************************************/
/* STAFStringConstructFromUInt64 - Creates a STAFString from an        */
/*                                 STAFUInt64_t number                 */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  The value to be represented as a string              */
/*          (In)  The base in which to represent the value [1..16]     */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructFromUInt64(STAFString_t *pString,
                                       STAFUInt64_t value,
                                       unsigned int base,
                                       unsigned int *osRC);
                                     
/***********************************************************************/
/* STAFStringConstructFromInt64 - Creates a STAFString from a          */
/*                                STAFInt64_t number                   */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  The value to be represented as a string              */
/*          (In)  The base in which to represent the value [1..16]     */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructFromInt64(STAFString_t *pString,
                                      STAFInt64_t value,
                                      unsigned int base,
                                      unsigned int *osRC);

/***********************************************************************/
/* STAFStringConstructSubString - Creates a substring of a STAFString  */
/*                                with index and len refering to ei-   */
/*                                ther UTF-8 chars (0) or bytes (1)    */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  A STAFString_t                                       */
/*          (In)  The index of the char or byte to start from          */
/*          (In)  Len of chars or bytes to get from string             */
/*          (In)  0->index/len are in chars; 1->index/len are in bytes */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructSubString(STAFString_t *pSubStr,
                                      STAFStringConst_t aString,
                                      unsigned int index,
                                      unsigned int len,
                                      unsigned int corb,
                                      unsigned int *osRC);

/***********************************************************************/
/* STAFStringConstructSubWord - Creates a subword string from a        */
/*                              STAFString                             */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  A STAFString_t                                       */
/*          (In)  The starting word from the string (0 based)          */
/*          (In)  The number of words to retrieve (0 meaning all)      */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructSubWord(STAFString_t *pWord,
                                    STAFStringConst_t aString,
                                    unsigned int index,
                                    unsigned int count,
                                    unsigned int *osRC);

/***********************************************************************/
/* STAFStringConstructChar - Creates a string of length 1 representing */
/*                           a specified character in UTF8             */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  A STAFString_t                                       */
/*          (In)  The starting word from the string (0 based)          */
/*          (In)  The number of words to retrieve (0 meaning all)      */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructChar(STAFString_t *pChar,
                                 STAFUTF8Char_t aChar,
                                 unsigned int *osRC);

/***********************************************************************/
/* STAFStringConstructJoin - Creates a STAFString by joining all of    */
/*                           the strings in an array                   */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (In)  An array of STAFString_t                             */
/*          (In)  The size of the array                                */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConstructJoin(STAFString_t *pString,
                                 STAFString_t aStringArray[],
                                 unsigned int arraySize,
                                 unsigned int *osRC);

/***********************************************************************/
/* STAFStringNumOfWords - Returns the number of words in a STAFString  */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) The number of words                                  */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringNumOfWords(STAFStringConst_t aString,
                              unsigned int *num,
                              unsigned int *osRC);

/***********************************************************************/
/* STAFStringAssign - Assigns a STAFString to another STAFString, clea-*/
/*                    ning up any resources taken by the target string */
/*                                                                     */
/* Accepts: (In)  A STAFString_t (target)                              */
/*          (In)  A STAFString_t (source)                              */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringAssign(STAFString_t aTarget,
                          STAFStringConst_t aSource,
                          unsigned int *osRC);

/***********************************************************************/
/* STAFStringGetBuffer - Returns a pointer to the buffer containing    */
/*                       the bytes that represent a STAFString         */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) A pointer to the buffer                              */
/*          (Out) A pointer to the length of the buffer                */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringGetBuffer(STAFStringConst_t aString,
                             const char **buffer,
                             unsigned int *len,
                             unsigned int *osRC);

/***********************************************************************/
/* STAFStringToLowerCase - Converts only Latin Alphabet letters A-Z to */
/*                         a-z from a STAFString                       */
/*                                                                     */
/* Accepts: (In)/(Out) A STAFString_t to convert                       */
/*          (Out)      A pointer to an OS return code (may be NULL)    */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringToLowerCase(STAFString_t aString,
                               unsigned int *osRC);

/***********************************************************************/
/* STAFStringToUpperCase - Converts only Latin Alphabet letters a-z to */
/*                         A-Z from a STAFString                       */
/*                                                                     */
/* Accepts: (In)/(Out) A STAFString_t to convert                       */
/*          (Out)      A pointer to an OS return code (may be NULL)    */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringToUpperCase(STAFString_t aString,
                               unsigned int *osRC);

/***********************************************************************/
/* STAFStringReplace - Replaces all oldStrings in a STAFString with    */
/*                     newString fixing the length of the string if    */
/*                     necessary                                       */
/*                                                                     */
/* Accepts: (In)/(Out) A STAFString_t to modify                        */
/*          (In)       The STAFString to be replaced                   */
/*          (In)       The STAFString to replace with                  */
/*          (Out)      A pointer to an OS return code (may be NULL)    */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringReplace(STAFString_t aString,
                           STAFStringConst_t oldString,
                           STAFStringConst_t newString,
                           unsigned int *osRC);

/***********************************************************************/
/* STAFStringToCurrentCodePage - Returns a buffer of size len contain- */
/*                               ing the current codepage representa-  */
/*                               tion of a STAFString                  */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) A buffer with the local codepage characters          */
/*          (Out) The size of the allocated buffer                     */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/*                                                                     */
/* Note   : This function allocates memory that must be freed up with  */
/*          STAFStringFreeBuffer.                                      */
/*                                                                     */
/* Note   : The returned buffer has a terminating NULL byte.  This     */
/*          byte is not counted in the length which is returned.       */
/*                                                                     */
/***********************************************************************/

STAFRC_t STAFStringToCurrentCodePage(STAFStringConst_t aString,
                                     char **to,
                                     unsigned int *len,
                                     unsigned int *osRC);

/***********************************************************************/
/* STAFStringConcatenate - Concatenates a STAFString with another      */
/*                         STAFString                                  */
/*                                                                     */
/* Accepts: (In)/(Out) A STAFString_t                                  */
/*          (In)       A STAFString_t                                  */
/*          (Out)      A pointer to an OS return code (may be NULL)    */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringConcatenate(STAFString_t aString,
                               STAFStringConst_t aSource,
                               unsigned int *osRC);

/***********************************************************************/
/* STAFStringCountSubStrings - Counts the number of times a particular */
/*                             substring appears in a STAFString       */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t                                       */
/*          (Out) The count of substrings                              */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringCountSubStrings(STAFStringConst_t aString,
                                   STAFStringConst_t aSubStr,
                                   unsigned int *count,
                                   unsigned int *osRC);


/***********************************************************************/
/* STAFStringStripCharsOfType - Strips chars of a type from the left,  */
/*                              right, or both sides of a STAFString   */
/*                                                                     */
/* Accepts: (In)/(Out) A STAFString_t                                  */
/*          (In)       A STAFUTF8CharType_t                            */
/*          (In)       0->trim left, 1->trim right, 2->trim both       */
/*          (Out)      A pointer to an OS return code (may be NULL)    */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringStripCharsOfType(STAFString_t aInOutStr,
                                    STAFUTF8CharType_t aType,
                                    unsigned int side,
                                    unsigned int *osRC);

/***********************************************************************/
/* STAFStringToUInt - Returns the numeric value of a STAFString as an  */
/*                    unsigned integer (range is 0 to 4294967295)      */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) A pointer to the numeric value of the string         */
/*          (In)  The base in which the value is represented [1..16]   */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringToUInt(STAFStringConst_t aString,
                          unsigned int *value, unsigned int base,
                          unsigned int *osRC);

/***********************************************************************/
/* STAFStringToUInt64 - Returns the numeric value of a STAFString as   */
/*                      a STAFUInt64_t (range is 18446744073709551615) */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) A pointer to the numeric value of the string         */
/*          (In)  The base in which the value is represented [1..16]   */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/
/* XXX: Commented out until get UINT64_MAX working on Solaris
STAFRC_t STAFStringToUInt64(STAFStringConst_t aString,
                            STAFUInt64_t *value, unsigned int base,
                            unsigned int *osRC);
*/                            

/***********************************************************************/
/* STAFStringLength - Returns the char or byte length of a STAFString  */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) A pointer to the char length of the string           */
/*          (In)  0->char length of string, 1->byte length of string   */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringLength(STAFStringConst_t aString,
                          unsigned int *len,
                          unsigned int corb,
                          unsigned int *osRC);

/***********************************************************************/
/* STAFStringSizeOfChar - Returns the number of bytes contained in a   */
/*                        particular char of a STAFString              */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  The index of the char to measure (0 based)           */
/*          (In)  0->char index, 1->byte index                         */
/*          (Out) A pointer to the byte length of a char of a string   */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringSizeOfChar(STAFStringConst_t aString,
                              unsigned int index,
                              unsigned int corb,
                              unsigned int *len,
                              unsigned int *osRC);


/***********************************************************************/
/* STAFStringByteIndexOfChar - Returns the byte location in the string */
/*                             buffer of a specified char              */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  The char index                                       */
/*          (Out) A pointer to the byte location of the char           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringByteIndexOfChar(STAFStringConst_t aString,
                                   unsigned int index,
                                   unsigned int *pos,
                                   unsigned int *osRC);

/***********************************************************************/
/* STAFStringIsCharsOfType - Checks a STAFString to be of a specific   */
/*                           character type (white, digits, ascii)     */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFUTF8CharType_t                                 */
/*          (Out) A pointer set to 0 if false, >0 if true              */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/*                                                                     */
/* Note   : If the empty string is passed in as the first argument,    */
/*          the result of this function will always return kSTAFOk     */
/*          as we are considering the empty string to be a character   */
/*          of every type.                                             */
/***********************************************************************/

STAFRC_t STAFStringIsCharsOfType(STAFStringConst_t aFirst,
                                 const STAFUTF8CharType_t aType,
                                 unsigned int *result,
                                 unsigned int *osRC);

/***********************************************************************/
/* STAFStringIsEqualTo - Compares a STAFString to another STAFString   */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t                                       */
/*          (In)  Case sensitivity indicator                           */
/*          (Out) A pointer set to 0 if different or >0 if equal       */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringIsEqualTo(STAFStringConst_t aFirst,
                             STAFStringConst_t aSecond,
                             STAFStringCaseSensitive_t sensitive,
                             unsigned int *comparison,
                             unsigned int *osRC);

/***********************************************************************/
/* STAFStringCompareTo - Determines whether a STAFString is greater    */
/*                       than another STAFString by byte comparison    */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t                                       */
/*          (Out) A pointer to a int set to 1 if the first string is   */
/*                less than the second string, 0 if they are equal,    */
/*                or 2 if the second string is less than the first     */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringCompareTo(STAFStringConst_t aFirst,
                             STAFStringConst_t aSecond,
                             unsigned int *less,
                             unsigned int *osRC);

/***********************************************************************/
/* STAFStringCompareTo - Determines whether a STAFString is greater    */
/*                       than another STAFString by byte comparison    */
/*                                                                     */
/* Accepts: (In)  String to check                                      */
/*          (In)  Starts with string                                   */
/*          (Out) A pointer to a int set to 1 if the first string      */
/*                starts with the second string, and 0 if the first    */
/*                does not start with the second string                */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringStartsWith(STAFStringConst_t aString,
                              STAFStringConst_t startsWithString,
                              unsigned int *startsWith,
                              unsigned int *osRC);

/***********************************************************************/
/* STAFStringContainsWildcard - Determines whether a STAFString is     */
/*                              contains at least one wildcard         */
/*                              character                              */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (Out) A pointer to a int set to 1 if the string contains   */
/*                at least one wildcard character, 0 if the string     */
/*                does not contain any wildcard characters             */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/*                                                                     */
/* Note   : Two wildcard characters are understood: '*' and '?'        */
/***********************************************************************/

STAFRC_t STAFStringContainsWildcard(STAFStringConst_t aString,
                                    unsigned int *hasWildcard,
                                    unsigned int *osRC);

/***********************************************************************/
/* STAFStringMatchesWildcards - Determines whether a given STAFString  */
/*                              matches a given wildcard STAFString    */
/*                                                                     */
/* Accepts: (In)  String to check                                      */
/*          (In)  String containg wildcards                            */
/*          (In)  Case sensitivity indicator                           */
/*          (Out) Result of comparison (0 = no match, 1 = matches)     */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/*                                                                     */
/* Note   : Two wildcard characters are understood.  '*' matches zero  */
/*          or more characters.  '?' matches one single character.     */
/***********************************************************************/

STAFRC_t STAFStringMatchesWildcards(STAFStringConst_t stringToCheck,
                                    STAFStringConst_t wildcardString,
                                    STAFStringCaseSensitive_t caseSensitive,
                                    unsigned int *matches,
                                    unsigned int *osRC);

/***********************************************************************/
/* STAFStringFind - Finds the first instance of a substring in a STAF- */
/*                  String                                             */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t to look for                           */
/*          (In)  The char or byte index from where to start looking   */
/*          (In)  0->char index, 1->byte index                         */
/*          (Out) A pointer to a int set to the char or byte index of  */
/*                the entry found or 0xffffffff if not found           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringFind(STAFStringConst_t aString,
                        STAFStringConst_t aSubStr,
                        unsigned int index, unsigned int corb,
                        unsigned int *pos,
                        unsigned int *osRC);

/***********************************************************************/
/* STAFStringFindFirstOf - Finds the first instance of a UTF-8 char in */
/*                         a set starting from a given char or byte    */
/*                         index                                       */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t which is a set of chars to look for   */
/*          (In)  The char or byte index from where to start looking   */
/*          (In)  0->char index, 1->byte index                         */
/*          (Out) A pointer to a int set to the char or byte index of  */
/*                the entry found or 0xffffffff if not found           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringFindFirstOf(STAFStringConst_t aString,
                               STAFStringConst_t aSet,
                               unsigned int index, unsigned int corb,
                               unsigned int *pos,
                               unsigned int *osRC);

/***********************************************************************/
/* STAFStringFindLastOf - Finds the last instance of a UTF-8 char in a */
/*                        set starting from a given char or byte index */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t which is a set of chars to look for   */
/*          (In)  The char or byte index from where to start looking   */
/*          (In)  0->char index, 1->byte index                         */
/*          (Out) A pointer to a int set to the char or byte index of  */
/*                the entry found or 0xffffffff if not found           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringFindLastOf(STAFStringConst_t aString,
                              STAFStringConst_t aSet,
                              unsigned int index, unsigned int corb,
                              unsigned int *pos,
                              unsigned int *osRC);

/***********************************************************************/
/* STAFStringFindFirstNotOf - Finds the first instance of a UTF-8 char */
/*                            not in set starting from a given char or */
/*                            byte index                               */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t which is a set of chars to look for   */
/*          (In)  The char or byte index from where to start looking   */
/*          (In)  0->char index, 1->byte index                         */
/*          (Out) A pointer to a int set to the char or byte index of  */
/*                the entry found or 0xffffffff if not found           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringFindFirstNotOf(STAFStringConst_t aString,
                                  STAFStringConst_t aSet,
                                  unsigned int index,
                                  unsigned int corb,
                                  unsigned int *pos,
                                  unsigned int *osRC);

/***********************************************************************/
/* STAFStringFindLastNotOf - Finds the last instance of a UTF-8 char   */
/*                           not in a set starting from a given char   */
/*                           byte index                                */
/*                                                                     */
/* Accepts: (In)  A STAFString_t                                       */
/*          (In)  A STAFString_t which is a set of chars to look for   */
/*          (In)  The char or byte index from where to start looking   */
/*          (In)  0->char index, 1->byte index                         */
/*          (Out) A pointer to a int set to the char or byte index of  */
/*                the entry found or 0xffffffff if not found           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringFindLastNotOf(STAFStringConst_t aString,
                                 STAFStringConst_t aSet,
                                 unsigned int index,
                                 unsigned int corb,
                                 unsigned int *pos,
                                 unsigned int *osRC);

/***********************************************************************/
/* STAFStringDestruct - Destructs a STAFString                         */
/*                                                                     */
/* Accepts: (Out) A pointer to a STAFString_t                          */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/***********************************************************************/

STAFRC_t STAFStringDestruct(STAFString_t *pString,
                            unsigned int *osRC);


/***********************************************************************/
/* STAFStringFreeBuffer - Deallocates a buffer allocated by a          */
/*                        STAFString function call                     */
/*                                                                     */
/* Accepts: (In)  A pointer to a STAFString allocated buffer           */
/*          (Out) A pointer to an OS return code (may be NULL)         */
/*                                                                     */
/* Returns: kSTAFOk on success                                         */
/*          other on error                                             */
/*                                                                     */
/* Note   : This function must be used per STAFStringToLocalCodePage   */
/*          in order to free up any allocated memory                   */
/***********************************************************************/

STAFRC_t STAFStringFreeBuffer(const char *buffer,
                              unsigned int *osRC);

/* End C language definitions */

#ifdef __cplusplus
}

/* Begin C++ language definitions */

#include <cstdio>
#include "STAF_iostream.h"
#include "STAFRefPtr.h"
#include "STAFException.h"

// Forward declaration for typedef
class STAFString;
class STAFStringBuffer;
typedef STAFRefPtr<STAFString> STAFStringPtr;
typedef STAFRefPtr<STAFStringBuffer> STAFStringBufferPtr;


// STAFStringBuffer - This class is a helper class used by the STAFString
//                    class when it needs to return a buffer to the current
//                    codepage representation of the string

class STAFStringBuffer
{
public:

    unsigned int length()   { return fLength; }
    const char *buffer()    { return fPtr; }

    ~STAFStringBuffer();

private:

    friend class STAFString;

    STAFStringBuffer(const char *ptr, unsigned int length)
        : fPtr(ptr), fLength(length)
    { /* Do Nothing */ }

    unsigned int fLength;
    const char *fPtr;
};


inline STAFStringBuffer::~STAFStringBuffer()
{
    unsigned int osRC = 0;
    STAFStringFreeBuffer(fPtr, &osRC);
}


// STAFString - This class provides a C++ wrapper around the STAFString
//              C APIs.

class STAFString
{
public:

    enum IndexRep { kChar = 0, kByte = 1 };
    enum CodePageType { kCurrent = 0, kUTF8 = 1 };
    enum InvalidPosition { kNPos = 0xFFFFFFFF };
    enum DefaultBufferLen { kStrLen = 0xFFFFFFFF };
    enum StripWhat { kFront = 0, kBack = 1, kBoth = 2 };
    enum CopyMode { kShallow = 0, kDeep = 1 };
    enum Remainder { kRemainder = 0xFFFFFFFF };

    // Constructors
    STAFString(void);
    STAFString(const char *buffer, unsigned int length = kStrLen,
               CodePageType cpType = kCurrent);
    STAFString(unsigned int fromValue, unsigned int base = 10);
    STAFString(int fromValue, unsigned int base = 10);
    STAFString(unsigned short fromValue, unsigned int base = 10);
    STAFString(short fromValue, unsigned int base = 10);

// On 64-bit Unix machines, an unsigned long is the same as a STAFUInt64_t
// and a long is the same as a STAFInt64_t, so can't define STAFString
// constructors for both

#if !defined(STAF_OS_64BIT)
    STAFString(unsigned long fromValue, unsigned int base = 10);
    STAFString(long fromValue, unsigned int base = 10);
#endif

    STAFString(STAFUInt64_t fromValue, unsigned int base = 10);
    STAFString(STAFInt64_t fromValue, unsigned int base = 10);
    STAFString(const STAFString &from);
    STAFString(STAFStringConst_t from);
    STAFString(STAFString_t from, CopyMode mode = kDeep);
    STAFString(STAFUTF8Char_t aChar);

    // Substring functions
    STAFString subString(unsigned int begin, unsigned int len = kRemainder,
                         IndexRep corb = kByte) const;

    // Word functions
    unsigned int numWords() const;
    STAFString subWord(unsigned int begin,
                       unsigned int length = kRemainder) const;

    // Data functions
    unsigned int count(const STAFString &theSubStr) const;
    unsigned int length(IndexRep corb = kByte) const;
    unsigned int sizeOfChar(unsigned int index, IndexRep corb = kByte) const;
    const char *buffer(unsigned int *bufLength = 0) const;
    STAFString_t getImpl() const;
    STAFString_t adoptImpl();
    void replaceImpl(STAFString_t replacementImpl);

    // Search functions
    unsigned int byteIndexOfChar(unsigned int charIndex) const;
    unsigned int find(const STAFString &searchFor, unsigned int begin = 0,
                      IndexRep corb = kByte) const;
    unsigned int findFirstOf(const STAFString &searchList,
                             unsigned int begin = 0,
                             IndexRep corb = kByte) const;
    unsigned int findFirstNotOf(const STAFString &searchList,
                                unsigned int begin = 0,
                                IndexRep corb = kByte) const;
    unsigned int findLastOf(const STAFString &searchList,
                            unsigned int begin = 0,
                            IndexRep corb = kByte) const;
    unsigned int findLastNotOf(const STAFString &searchList,
                               unsigned int begin = 0,
                               IndexRep corb = kByte) const;

    // Miscellaneous alteration functions
    STAFString &lowerCase();
    STAFString &upperCase();
    STAFString &strip(StripWhat stripWhat = kBoth);
    STAFString &join(const STAFString stringArray[], unsigned int arraySize);

    // Conversions
    unsigned int asUInt(unsigned int base = 10) const;
    unsigned int asUIntWithDefault(unsigned int defaultValue,
                                   unsigned int base = 10) const;

    /* XXX: Commented out until get UINT64_MAX working on Solaris
    STAFUInt64_t asUInt64(unsigned int base = 10) const;
    STAFUInt64_t asUInt64WithDefault(STAFUInt64_t defaultValue,
                                     unsigned int base = 10) const;
    */

    STAFStringBufferPtr toCurrentCodePage() const;
    STAFString toLowerCase() const;
    STAFString toUpperCase() const;
    STAFString replace(const STAFString oldchar,
                       const STAFString newchar) const;

    // Evaluation
    bool isWhiteSpace() const;
    bool isDigits() const;
    bool isEqualTo(const STAFString &theString,
                   STAFStringCaseSensitive_t caseSensitive =
                   kSTAFStringCaseSensitive) const;
    bool startsWith(const STAFString &someString) const;
    bool hasWildcard() const;
    bool matchesWildcards(const STAFString &wildcardString,
                          STAFStringCaseSensitive_t caseSensitive =
                          kSTAFStringCaseSensitive) const;

    // operators
    STAFString &operator=(const STAFString &rhs);
    bool operator==(const STAFString &rhs) const;
    bool operator!=(const STAFString &rhs) const;
    bool operator<(const STAFString &rhs) const;
    bool operator<=(const STAFString &rhs) const;
    bool operator>(const STAFString &rhs) const;
    bool operator>=(const STAFString &rhs) const;
    STAFString &operator+=(const STAFString &rhs);

    friend STAFString operator+(const STAFString &lhs, const STAFString &rhs);
    friend ostream &operator<<(ostream &os, const STAFString &rhs);

    // Destructor
    ~STAFString();

private:

    STAFString_t fStringImpl;
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFStringInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
