/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_CommandParser
#define STAF_CommandParser

#include "STAFString.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STAFCommandParserImpl *STAFCommandParser_t;
typedef struct STAFCommandParseResultImpl *STAFCommandParseResult_t;

// XXX: Consider passing non-ptrs to the APIs that do not need ptr-ptr 
// XXX: Also consider having a brief discussion about how to use parser
// XXX: for debug

class STAFCommandParser;
class STAFCommandParseResult;
void printParserInfo(STAFCommandParser &);
void printParseResultInfo(STAFCommandParseResult &);
void printParserInfo2(STAFCommandParserImpl &);
void printParseResultInfo2(STAFCommandParseResultImpl &);

typedef STAFRefPtr<STAFCommandParser> STAFCommandParserPtr;

/***********************************************************************/
/* STAFCommandParserConstruct - Creates a STAF Command Parser          */
/*                                                                     */
/* Accepts: (Out) Pointer to a STAF Command Parser                     */
/*          (In)  The maximum number of arguments allowed by the       */
/*                parser                                               */
/*          (In)  Whether the parser is case sensitive (0 = No,        */
/*                1 = Yes)                                             */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParserConstruct(STAFCommandParser_t *pParser,
    unsigned int maxArgs, unsigned int caseSensitive, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParserAddOption - Adds an option to the parser           */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parser                     */
/*          (In)  The option name                                      */
/*          (In)  The number of times this option may be specified     */
/*                (0 = Unlimited)                                      */
/*          (In)  Whether the option allows, requires, or may not have */
/*                a value associated with it.  (0 = Not Allowed,       */
/*                1 = Allowed, 2 = Required)                           */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParserAddOption(STAFCommandParser_t parser,
    const STAFString_t optionName, unsigned int timesAllowed,
    unsigned int valueRequirement, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParserAddOptionGroup - Adds an option group to the       */
/*                                   parser                            */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parser                     */
/*          (In)  The list of option names in the group                */
/*          (In)  The minimum number of options that must be specified */
/*          (In)  The maximum number of options that may be specified  */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParserAddOptionGroup(STAFCommandParser_t parser,
    const STAFString_t optionGroup, unsigned int minOptions, 
    unsigned int maxOptions, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParserAddOptionNeed - Adds an option need to the parser  */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parser                     */
/*          (In)  The list of needer option names                      */
/*          (In)  The list of needee option names                      */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParserAddOptionNeed(STAFCommandParser_t parser,
    const STAFString_t optionNeeders, const STAFString_t optionNeedees, 
    unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParserParseString - Parses a string                      */
/*                                                                     */
/* Note: On error, use STAFCommandParseResultGetErrorBuffer to get a   */
/*       string containing a description of the error.                 */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parser                     */
/*          (In)  The string to parse                                  */
/*          (Out) Pointer to the STAFCommandParseResult_t              */
/*          (Out) Pointer to the error buffer                          */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParserParseString(STAFCommandParser_t parser,
    const STAFString_t theString, STAFCommandParseResult_t *result,
    STAFString_t *errorBuffer, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParserDestruct - Destroys a STAF Command Parser          */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parser                     */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParserDestruct(STAFCommandParser_t *pParser,
                                   unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetErrorBuffer - Gets the result's error buf- */
/*                                        fer set on parsing errors    */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (Out) Pointer to a STAFString to contain the error buffer  */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetErrorBuffer(
    STAFCommandParseResult_t result, STAFString_t *theBuffer,
    unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetNumInstances - Gets the total number of    */
/*                                         options specified by the    */
/*                                         user in the parsed string   */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (Out) Pointer to the number of options                     */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetNumInstances(
    STAFCommandParseResult_t result, unsigned int *numInstances,
    unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetInstanceName - Gets the name of the        */
/*                                         specified option instance   */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (In)  The option instance number                           */
/*          (Out) Pointer to the option name                           */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetInstanceName(
    STAFCommandParseResult_t result, unsigned int instanceNum,
    STAFString_t *instanceName, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetInstanceValue - Gets the value of the      */
/*                                          specified option instance  */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (In)  The option instance number                           */
/*          (Out) Pointer to the option value                          */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetInstanceValue(
    STAFCommandParseResult_t result, unsigned int instanceNum,
    STAFString_t *instanceValue, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetOptionTimes - Gets the number of times     */
/*                                        a particular option was      */
/*                                        specified by the user        */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (In)  The option name                                      */
/*          (Out) Pointer to the number of times the option was        */
/*                specified                                            */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetOptionTimes(
    STAFCommandParseResult_t result, const STAFString_t optionName,
    unsigned int *numTimes, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetOptionValue - Gets the value of a          */
/*                                        particular instance of a     */
/*                                        particular option            */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (In)  The name of this option                              */
/*          (In)  The number of the desired instance of this option    */
/*          (Out) Pointer to the option value                          */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetOptionValue(
    STAFCommandParseResult_t result, const STAFString_t optionName,
    unsigned int optionIndex, STAFString_t *optionValue,
    unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetNumArgs - Gets the number of free-standing */
/*                                 arguments the user specified        */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (Out) Pointer to the number of arguments                   */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetNumArgs(
    STAFCommandParseResult_t result, unsigned int *numArgs,
    unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseResultGetArgValue - Gets the value of a particular  */
/*                                     argument                        */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (In)  The number of the desired argument                   */
/*          (Out) Pointer to the argument value                        */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultGetArgValue(
    STAFCommandParseResult_t result, unsigned int argNum,
    STAFString_t *argValue, unsigned int *osRC);


/* XXX: This function may not be required.  We may not actually alloc 
        new memory */

/***********************************************************************/
/* STAFCommandParseResultFreeMemory - Frees the memory returned by     */
/*                                    the STAF Command Parse Result    */
/*                                    APIs                             */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (In)  Pointer to the memory to free                        */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultFreeMemory(
    STAFCommandParseResult_t result, char *ptr, unsigned int *osRC);


/***********************************************************************/
/* STAFCommandParseDestruct - Destroys a STAF Command Parse Result     */
/*                            object                                   */
/*                                                                     */
/* Accepts: (In)  Pointer to a STAF Command Parse Result               */
/*          (Out) Pointer to OS return code                            */
/*                                                                     */
/* Returns:  kSTAFOk, on success                                       */
/*           other on error                                            */
/***********************************************************************/
STAFRC_t STAFCommandParseResultDestruct(
    STAFCommandParseResult_t *pResult, unsigned int *osRC);

#ifdef __cplusplus
}

#include "STAFException.h"

// STAFCommandParseResult - This class is returned from a STAFCommandParser.
//                          It represents the results of parsing a string via
//                          the STAFCommandParser.

class STAFCommandParseResult
{
public:

    // The return code from the parse
    STAFRC_t rc;

    // The description of the error when rc != 0
    STAFString errorBuffer;

    // Returns the number of times an option was specified
    unsigned int optionTimes(const STAFString &optionName);


    // Returns the value of a given instance of a given option.
    // Returns an empty string if no such option or instance exists.
    STAFString optionValue(const STAFString &optionName,
                           unsigned int number = 1);


    // Returns the total number of options specified in the string
    unsigned int numInstances();


    // Returns the name of the given option instance
    STAFString instanceName(unsigned int number);


    // Returns the value of the given option instance
    STAFString instanceValue(unsigned int number);


    // Returns the number of extra arguments
    unsigned int numArgs();


    // Returns a given argument.  Returns an empty string if no such argument
    // exists.
    STAFString arg(unsigned int number);

    ~STAFCommandParseResult();

    STAFCommandParseResult_t getImpl() { return fResultImpl; }

 private:

    // Disallow copy construction and assignment
    STAFCommandParseResult(const STAFCommandParseResult &);
    STAFCommandParseResult &operator=(const STAFCommandParseResult &);

    friend class STAFCommandParser;

    STAFCommandParseResult(STAFCommandParseResult_t theResult,
        STAFRC_t theRC, const STAFString &theErrorBuffer)
        : rc(theRC), errorBuffer(theErrorBuffer), fResultImpl(theResult)
    { /* Do Nothing */ }

    STAFCommandParseResult_t fResultImpl;
};

typedef STAFRefPtr<STAFCommandParseResult> STAFCommandParseResultPtr;


// STAFCommandParser - This class provides a parsing interface for STAF, in
//                     particular, STAF services

class STAFCommandParser
{
public:

    // This enum is used to determine if an option may, may not, or must
    // have a value associated with it

    enum ValueRequirement { kValueNotAllowed = 0, kValueAllowed = 1,
                            kValueRequired = 2};

    // The constructor accepts an array of options and a count of those
    // options, along with an indication of case sensitivity to option names

    STAFCommandParser(unsigned int maxArgs = 0, bool caseSensitive = false);


    void addOption(const STAFString &option, unsigned int numAllowed,
                   ValueRequirement valueReq);


    void addOptionGroup(const STAFString &group, unsigned int minAllowed,
                        unsigned int maxAllowed);

    void addOptionNeed(const STAFString &needers, const STAFString &needees);

    // Parses a given string.  Returns 0, if successful, > 0, otherwise.
    // ErrorBuffer will be set if unsuccessful.

    STAFCommandParseResultPtr parse(const STAFString &parseString);

    ~STAFCommandParser();

    STAFCommandParser_t getImpl() { return fParserImpl; }

private:

    // Disallow copy construction and assignment
    STAFCommandParser(const STAFCommandParser &);
    STAFCommandParser &operator=(const STAFCommandParser &);

    STAFCommandParser_t fParserImpl;
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFCommandParserInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
