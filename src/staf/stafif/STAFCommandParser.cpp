/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFUtil.h"
#include "STAFCommandParser.h"
#include <deque>
#include <map>
#include "STAF_iostream.h"

//#define STAF_DO_TIMING
#include "STAFTiming.h"

// Note: The magic numbers, 4, 2, and 6, below refer to the length of the
//       preceding UTF-8 strings.  If more characters are added to these
//       variables, the magic numbers must be updated

static const STAFString sUTF8_Space(kUTF8_SPACE);
static const STAFString sUTF8_Whitespace("\x20\x0a\x0d\x09", 4,
                                         STAFString::kUTF8);
static const STAFString sUTF8_Colon(kUTF8_COLON);
static const STAFString sUTF8_BackSlash(kUTF8_BSLASH);
static const STAFString sUTF8_DoubleQuote(kUTF8_DQUOTE);
static const STAFString sUTF8_QuoteSpecial("\x5c\x22", 2, STAFString::kUTF8);
static const STAFString sUTF8_WordSpecial("\x22\x20\x0a\x0d\x09", 6,
                                      STAFString::kUTF8);
static const STAFString sEmptyString;

struct STAFCommandParseResultImpl
{ 
    struct OptionInstance
    {
        STAFString option;
        STAFString value;
    };

    typedef std::multimap<STAFString, OptionInstance> OptionInstanceMap;
    typedef std::deque<OptionInstance> OptionInstanceList;
    typedef std::deque<STAFString> ArgList;

    bool fCaseSensitive;
    STAFString errorBuffer;
    OptionInstanceMap fInstanceMap;
    OptionInstanceList fInstanceList;
    ArgList fArgList;
};


struct STAFCommandParserImpl
{
    STAFCommandParserImpl(unsigned int maxArgs, bool caseSensitive)
        : fMaxArgs(maxArgs), fCaseSensitive(caseSensitive)
    { /* Do Nothing */ }

    struct Option
    {
        Option(const STAFString &Name, unsigned int NumAllowed,
               STAFCommandParser::ValueRequirement vr)
            : name(Name), numAllowed(NumAllowed), valueRequirement(vr)
        { /* Do Nothing */ }

        Option() : name(), numAllowed(0),
                   valueRequirement(STAFCommandParser::kValueNotAllowed)
        { /* Do Nothing */ }

        STAFString name;
        unsigned int numAllowed;
        STAFCommandParser::ValueRequirement valueRequirement;
    };


    // OptionGroup is a simple structure used to specify a group of options,
    // along with the minimum and maximum number of those options that must
    // be specified

    struct OptionGroup
    {
        OptionGroup(const STAFString &theGroup, unsigned int theMin,
                    unsigned int theMax)
            : group(theGroup), minimum(theMin), maximum(theMax)
        { /* Do Nothing */ }

        STAFString group;
        unsigned int minimum;
        unsigned int maximum;
    };


    // OptionNeed is a simple structure that is used to specify that one option
    // must be used in conjunction with another option

    struct OptionNeed
    {
        OptionNeed(const STAFString &theNeeder, const STAFString &theNeedee)
            : needer(theNeeder), needee(theNeedee)
        { /* Do Nothing */ }

        STAFString needer;
        STAFString needee;
    };

    struct OptionValue
    {
        OptionValue() : optionSpecified(false), valueSpecified(false)
        { /* Do Nothing */ }

        STAFString option;
        STAFString value;
        bool optionSpecified;
        bool valueSpecified;
    };

    typedef std::map<STAFString, Option> OptionList;
    typedef std::deque<OptionGroup> GroupList;
    typedef std::deque<OptionNeed> NeedList;

    unsigned int fMaxArgs;
    bool fCaseSensitive;
    OptionList fOptionList;
    GroupList fGroupList;
    NeedList fNeedList;
};


inline unsigned int myOptionTimes(STAFCommandParseResultImpl *result,
                                  STAFString optionName)
{
    unsigned int numTimes = 0;
    unsigned int osRC = 0;

    unsigned int rc = STAFCommandParseResultGetOptionTimes(result,
                          optionName.getImpl(), &numTimes, &osRC);
    return numTimes;
}


STAFRC_t handleOptionValue(STAFCommandParserImpl &parser,
                           STAFCommandParseResultImpl &result,
                           STAFCommandParserImpl::OptionValue &optionValue,
                           STAFString_t *errorBuffer)
{
    if (optionValue.optionSpecified)
    {
        STAFCommandParserImpl::Option option =
            parser.fOptionList.find(optionValue.option)->second;

        // Make sure they specified a value if necessary

        if ((option.valueRequirement == STAFCommandParser::kValueRequired) &&
            !optionValue.valueSpecified)
        {
            result.errorBuffer += STAFString("Option, ");
            result.errorBuffer += STAFString(option.name);
            result.errorBuffer += STAFString(", requires a value");
            *errorBuffer = result.errorBuffer.getImpl();
            return kSTAFInvalidRequestString;
        }

        // Check once here for whether this new option instance will
        // exceed the limit for this option

        if ((myOptionTimes(&result, option.name) == option.numAllowed)
            && (option.numAllowed != 0))
        {
            result.errorBuffer += STAFString("You may have no more than ");
            result.errorBuffer += STAFString(option.numAllowed);
            result.errorBuffer += STAFString(" instances of option ");
            result.errorBuffer += STAFString(option.name);
            *errorBuffer = result.errorBuffer.getImpl();
            return kSTAFInvalidRequestString;
        }

        STAFCommandParseResultImpl::OptionInstance optionInstance;

        optionInstance.option = optionValue.option;

        // If this option can't have a value, then add the value to the
        // args list and don't assign it to the option instance

        if ((option.valueRequirement == STAFCommandParser::kValueNotAllowed) &&
            optionValue.valueSpecified)
        {
            result.fArgList.push_back(optionValue.value);
        }
        else
        {
            optionInstance.value = optionValue.value;
        }

        result.fInstanceMap.insert(
            STAFCommandParseResultImpl::OptionInstanceMap::value_type(
                optionInstance.option, optionInstance));

        result.fInstanceList.push_back(optionInstance);
    }
    else if (optionValue.valueSpecified)
    {
        result.fArgList.push_back(optionValue.value);
    }

    return kSTAFOk;
}


void printParserInfo2(STAFCommandParserImpl &parser)
{
    cout << "Max args: " << parser.fMaxArgs << endl
         << "Case sensitive: " << parser.fCaseSensitive << endl;

    for (STAFCommandParserImpl::OptionList::iterator olIter =
             parser.fOptionList.begin();
             olIter != parser.fOptionList.end(); ++olIter)
    {
        cout << "Key: " << olIter->first << endl
             << "Option: " << olIter->second.name << "," 
             << olIter->second.numAllowed
             << "," << olIter->second.valueRequirement << endl;
    }

    for (STAFCommandParserImpl::GroupList::iterator ogIter =
             parser.fGroupList.begin();
             ogIter != parser.fGroupList.end(); ++ogIter)
    {
        cout << "Group: " << ogIter->group << "," << ogIter->minimum 
             << "," << ogIter->maximum << endl;
    }

    for (STAFCommandParserImpl::NeedList::iterator nlIter =
             parser.fNeedList.begin();
             nlIter != parser.fNeedList.end(); ++nlIter)
    {
        cout << "Need: " << nlIter->needer << "," << nlIter->needee
             << endl;
    }
}


void printParserInfo(STAFCommandParser &parser_)
{
    STAFCommandParserImpl *parser__ = parser_.getImpl();
    STAFCommandParserImpl &parser = *parser__;
    
    cout << "Max args: " << parser.fMaxArgs << endl
         << "Case sensitive: " << parser.fCaseSensitive << endl;

    for (STAFCommandParserImpl::OptionList::iterator olIter =
             parser.fOptionList.begin();
             olIter != parser.fOptionList.end(); ++olIter)
    {
        cout << "Key: " << olIter->first << endl
             << "Option: " << olIter->second.name << "," 
             << olIter->second.numAllowed
             << "," << olIter->second.valueRequirement << endl;
    }

    for (STAFCommandParserImpl::GroupList::iterator glIter =
             parser.fGroupList.begin();
             glIter != parser.fGroupList.end(); ++glIter)
    {
        cout << "Group: " << glIter->group << "," << glIter->minimum
             << "," << glIter->maximum << endl;
    }

    for (STAFCommandParserImpl::NeedList::iterator nlIter =
             parser.fNeedList.begin();
             nlIter != parser.fNeedList.end(); ++nlIter)
    {
        cout << "Need: " << nlIter->needer << "," << nlIter->needee 
             << endl;
    }
}


void printParseResultInfo2(STAFCommandParseResultImpl &result)
{
    cout << "Case sensitive: " << result.fCaseSensitive << endl
         << "ErrorBuffer: " << result.errorBuffer << endl;

    for (STAFCommandParseResultImpl::OptionInstanceMap::iterator olIter =
             result.fInstanceMap.begin();
             olIter != result.fInstanceMap.end(); ++olIter)
    {
        cout << "Instance: " << olIter->first << "," 
             << olIter->second.option
             << "=" << olIter->second.value << endl;
    }

    for (STAFCommandParseResultImpl::ArgList::iterator alIter =
             result.fArgList.begin();
             alIter != result.fArgList.end(); ++alIter)
    {
        cout << "Arg: " << *alIter << endl;
    }
}


void printParseResultInfo(STAFCommandParseResult &result_)
{
    STAFCommandParseResultImpl *result__ = result_.getImpl();
    STAFCommandParseResultImpl &result = *result__;

    cout << "Case sensitive: " << result.fCaseSensitive << endl
         << "ErrorBuffer: " << result.errorBuffer << endl;

    for (STAFCommandParseResultImpl::OptionInstanceMap::iterator olIter =
             result.fInstanceMap.begin();
             olIter != result.fInstanceMap.end(); ++olIter)
    {
        cout << "Instance: " << olIter->first << "," 
             << olIter->second.option
             << "=" << olIter->second.value << endl;
    }

    for (STAFCommandParseResultImpl::ArgList::iterator alIter =
             result.fArgList.begin();
             alIter != result.fArgList.end(); ++alIter)
    {
        cout << "Arg: " << *alIter << endl;
    }
}


STAFRC_t STAFCommandParserConstruct(STAFCommandParser_t *pParser,
    unsigned int maxArgs, unsigned int caseSensitive, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (pParser == 0) return kSTAFInvalidObject;

    try
    {
        *pParser = new STAFCommandParserImpl(maxArgs, 
                  caseSensitive ? true : false);
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParserAddOption(STAFCommandParser_t parser,
    const STAFString_t optionName, unsigned int timesAllowed,
    unsigned int valueRequirement, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;
 
    if (parser == 0) return kSTAFInvalidObject; 

    try
    {
        STAFCommandParserImpl &stafCommandParser = *parser;
        STAFString theOptionName(optionName);

        if (!stafCommandParser.fCaseSensitive)
            theOptionName.lowerCase();

        stafCommandParser.fOptionList[theOptionName] =
            STAFCommandParserImpl::Option(optionName, timesAllowed,
            STAFCommandParser::ValueRequirement(valueRequirement));
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParserAddOptionGroup(STAFCommandParser_t parser,
    const STAFString_t optionGroup, unsigned int minOptions, 
    unsigned int maxOptions, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (parser == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParserImpl &stafCommandParser = *parser;

        stafCommandParser.fGroupList.push_back(
            STAFCommandParserImpl::OptionGroup(optionGroup,
            minOptions, maxOptions));
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParserAddOptionNeed(STAFCommandParser_t parser,
    const STAFString_t optionNeeders, const STAFString_t optionNeedees, 
    unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (parser == 0) return kSTAFInvalidObject; 

    try
    {
        STAFCommandParserImpl &stafCommandParser = *parser;

        stafCommandParser.fNeedList.push_back(STAFCommandParserImpl::OptionNeed(
            optionNeeders, optionNeedees));
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParserParseString(STAFCommandParser_t parser,
    const STAFString_t theString, STAFCommandParseResult_t *theResult,
    STAFString_t *errorBuffer, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;
    STAFCommandParseResultImpl *resultPtr = 0;

    if (parser == 0) return kSTAFInvalidObject;

    INIT_TIMES();
    RECORD_TIME("Starting parse");

    try
    {
        STAFCommandParserImpl &stafCommandParser = *parser;

        // Debug:
        // printParserInfo2(stafCommandParser);

        resultPtr = new STAFCommandParseResultImpl;
        *theResult = resultPtr;
        STAFCommandParseResultImpl &result = *resultPtr;

        result.errorBuffer = "";  // default to no error message
        result.fCaseSensitive = stafCommandParser.fCaseSensitive;
        *errorBuffer = result.errorBuffer.getImpl();

        // First we parse up the string into its constituent options and
        // values

        STAFString aString(theString);
        STAFCommandParserImpl::OptionValue optionValue;
        STAFString currentChar;
        int lasti;  // Last position of the i variable
        int i;  // leave it here (VC++ 6.0 Win32 scoping)
        bool hadWhitespace = false;

        RECORD_TIME("Parsing string");

        for (i = aString.findFirstNotOf(sUTF8_Whitespace), lasti = i;
             i < aString.length();
             lasti = i, i = aString.findFirstNotOf(sUTF8_Whitespace, i))
        {
            // Note: The use of the magic number, 1, below is safe.
            //       We know that the length of all the special UTF8
            //       characters is 1

            if (lasti == i) hadWhitespace = false;
            else hadWhitespace = true;

            currentChar = aString.subString(i, 1);

            if (currentChar == sUTF8_Colon)
            {
                if (hadWhitespace && (optionValue.value.length() != 0))
                    optionValue.value += sUTF8_Space;

                unsigned int endColonPos = aString.find(sUTF8_Colon, i + 1);
                STAFString clcLengthString =
                           aString.subString(i + 1, endColonPos - i - 1);

                // Make sure we have a valid length specifier

                if (clcLengthString.length() == 0)
                {
                    result.errorBuffer = STAFString(
                        "Invalid length delimited data specifier, '") +
                        clcLengthString + "', because its not an unsigned "
                        "integer in range 0 to " + STAFString(UINT_MAX);
                    result.errorBuffer += clcLengthString + "'";
                    *errorBuffer = result.errorBuffer.getImpl();
                    return kSTAFInvalidRequestString;
                }

                unsigned int clcLength;
                
                try
                {
                    clcLength = clcLengthString.asUInt();
                }
                catch (STAFException)
                {
                    result.errorBuffer = STAFString(
                        "Invalid length delimited data specifier, '") +
                        clcLengthString + "', because its not an unsigned "
                        "integer in range 0 to " + STAFString(UINT_MAX);
                    *errorBuffer = result.errorBuffer.getImpl();
                    return kSTAFInvalidRequestString;
                }

                i = endColonPos + 1;

                if ((i + clcLength) > aString.length())
                {
                    result.errorBuffer =
                        "Invalid length delimited data specifier, '";
                    result.errorBuffer += clcLengthString + "', because "
                        "specifies a length that is longer than the data";
                    *errorBuffer = result.errorBuffer.getImpl();
                    return kSTAFInvalidRequestString;
                }

                // Skip data
                for (unsigned int j = clcLength; j != 0; --j)
                    i += aString.sizeOfChar(i);

                optionValue.value += aString.subString(endColonPos + 1,
                                                       i - endColonPos - 1);
                optionValue.valueSpecified = true;
            }
            else if (currentChar == sUTF8_DoubleQuote)
            {
                if (hadWhitespace && (optionValue.value.length() != 0))
                    optionValue.value += sUTF8_Space;

                ++i;  // Advance past the double quote

                unsigned int done = 0;
                unsigned int next = 0;

                do
                {
                    next = aString.findFirstOf(sUTF8_QuoteSpecial, i);

                    if (next == STAFString::kNPos)
                    {
                        // We didn't find the closing double quote, but since
                        // we're at the end of the string, we're done
                        done = 1;
                        optionValue.value += aString.subString(i, next - i);

                        // Advance past the end of the string
                        i = next;
                    }
                    else if (aString.subString(next, 1) == sUTF8_BackSlash)
                    {
                        // Add the data from the last search position 
                        optionValue.value += aString.subString(i, next - i);

                        ++next;  // Advance past the backslash

                        // Add the character after the backslash
                        optionValue.value += aString.subString(next,
                                            aString.sizeOfChar(next));

                        // Advance past the character after the backslash
                        i = next + aString.sizeOfChar(next);
                    }
                    else
                    {
                        // We found the closing double quote, so we're done
                        done = 1;

                        // Add the data from the last search position
                        optionValue.value += aString.subString(i, next - i);

                        // Advance past the closing double quote
                        i = ++next;
                    }
                } while (!done);

                optionValue.valueSpecified = true;
            }
            else
            {
                // This is just regular data, its not double quoted, or
                // colon-length-colon format

                unsigned int endPos = aString.findFirstOf(sUTF8_WordSpecial, i);
                STAFString data = aString.subString(i, endPos - i);

                i = endPos;

                if (stafCommandParser.fOptionList.find(
                    (stafCommandParser.fCaseSensitive) ? data :
                         data.toLowerCase()) !=
                    stafCommandParser.fOptionList.end())
                {
                    rc = handleOptionValue(stafCommandParser, result,
                                           optionValue, errorBuffer);

                    if (rc != kSTAFOk) return rc;

                    optionValue = STAFCommandParserImpl::OptionValue();
                    optionValue.option = stafCommandParser.fCaseSensitive ?
                        data : data.toLowerCase();
                    optionValue.optionSpecified = true;
                }
                else
                {
                    if (hadWhitespace && (optionValue.value.length() != 0))
                        optionValue.value += sUTF8_Space;

                    optionValue.value += data;
                    optionValue.valueSpecified = true;
                }
            }
        }

        // Deal with any lingering data

        rc = handleOptionValue(stafCommandParser, result, optionValue,
                               errorBuffer);

        if (rc != kSTAFOk) return rc;

        // Debug:
        // printParseResultInfo2(result);

        // Now check the restriction on number of arguments

        RECORD_TIME("Checking args");

        if (result.fArgList.size() > stafCommandParser.fMaxArgs)
        {
            result.errorBuffer += STAFString("You may have no more than ");
            result.errorBuffer += STAFString(stafCommandParser.fMaxArgs);
            result.errorBuffer += STAFString(" argument(s).  You specified ");
            result.errorBuffer += STAFString(result.fArgList.size());
            result.errorBuffer += STAFString(" argument(s).  ");
            result.errorBuffer += STAFString("The first excess argument ");
            result.errorBuffer += STAFString("is, ");
            result.errorBuffer +=
                STAFString(result.fArgList[stafCommandParser.fMaxArgs]);
            result.errorBuffer += STAFString(".");
            *errorBuffer = result.errorBuffer.getImpl();
            return kSTAFInvalidRequestString;
        }

        // Now check all the group requirements

        RECORD_TIME("Checking groups");

        for(i = 0; i < stafCommandParser.fGroupList.size(); ++i)
        {
            STAFCommandParserImpl::OptionGroup group =
                stafCommandParser.fGroupList[i];
            unsigned int groupCount = 0;
            unsigned int groupWordCount = group.group.numWords();

            for (int j = 0; j < groupWordCount; ++j)
            {
                if (myOptionTimes(&result, group.group.subWord(j, 1))
                    != 0)
                {
                    ++groupCount;
                }
            }

            if ((groupCount < group.minimum) || (groupCount > group.maximum))
            {
                result.errorBuffer += STAFString("You must have at least ");
                result.errorBuffer += STAFString(group.minimum);
                result.errorBuffer += STAFString(", but no more than ");
                result.errorBuffer += STAFString(group.maximum);
                result.errorBuffer += STAFString(" of the option(s), ");
                result.errorBuffer += STAFString(group.group);
                *errorBuffer = result.errorBuffer.getImpl();
                return kSTAFInvalidRequestString;
            }
        }

        // Now check the need requirements

        RECORD_TIME("Checking needs");

        for(i = 0; i < stafCommandParser.fNeedList.size(); ++i)
        {
            STAFCommandParserImpl::OptionNeed need =
                stafCommandParser.fNeedList[i];

            unsigned int foundNeeder = 0;
            unsigned int foundNeedee = 0;

            for(int j = 0; (j < need.needer.numWords()) && !foundNeeder;
                ++j)
            {
                if (myOptionTimes(&result, need.needer.subWord(j, 1))
                    != 0)
                {
                    foundNeeder = 1;
                }
            }

            for(int k = 0; (k < need.needee.numWords()) && !foundNeedee;
                ++k)
            {
                if (myOptionTimes(&result, need.needee.subWord(k, 1))
                    != 0)
                {
                    foundNeedee = 1;
                }
            }

            if (foundNeeder && !foundNeedee)
            {
                result.errorBuffer +=
                    STAFString("When specifying one of the options ");
                result.errorBuffer += STAFString(need.needer);
                result.errorBuffer +=
                    STAFString(", you must also specify one of the ");
                result.errorBuffer += STAFString("options ");
                result.errorBuffer += STAFString(need.needee);
                *errorBuffer = result.errorBuffer.getImpl();
                return kSTAFInvalidRequestString;
            }
        }

        RECORD_TIME("Leaving try block");
    }
    catch (...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    RECORD_TIME("Finished parsing");
    OUTPUT_TIMES();

    return rc;
}


STAFRC_t STAFCommandParserDestruct(STAFCommandParser_t *pParser,
                                   unsigned int *osRC)
{
    if (pParser == 0) return kSTAFInvalidObject;

    delete *pParser;
    *pParser = 0;
    return kSTAFOk;
}



STAFRC_t STAFCommandParseResultGetErrorBuffer(
    STAFCommandParseResult_t result, STAFString_t *theBuffer,
    unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        *theBuffer = result->errorBuffer.getImpl();
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParseResultGetNumInstances(
    STAFCommandParseResult_t result, unsigned int *numInstances,
    unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParseResultImpl &res = *result;

        *numInstances = res.fInstanceMap.size();
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParseResultGetInstanceName(
    STAFCommandParseResult_t result, unsigned int instanceNum,
    STAFString_t *instanceName, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParseResultImpl &res = *result;

        STAFCommandParseResultImpl::OptionInstanceList::iterator iter;

        for (iter = res.fInstanceList.begin();
             (--instanceNum > 0) && (iter != res.fInstanceList.end()); 
             ++iter)
        { /* Do Nothing */ }

        *instanceName = iter->option.getImpl(); 
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}

STAFRC_t STAFCommandParseResultGetInstanceValue(
    STAFCommandParseResult_t result, unsigned int instanceNum,
    STAFString_t *instanceValue, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParseResultImpl &res = *result;
        STAFCommandParseResultImpl::OptionInstanceList::iterator iter;

        for (iter = res.fInstanceList.begin();
             (--instanceNum > 0) && (iter != res.fInstanceList.end()); 
             ++iter)
        { /* Do Nothing */ }

        *instanceValue = iter->value.getImpl();
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParseResultGetOptionTimes(
    STAFCommandParseResult_t result, const STAFString_t optionName,
    unsigned int *numTimes, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParseResultImpl &res = *result;

        STAFString theOptionName(optionName);

        if (!res.fCaseSensitive)
            theOptionName.lowerCase();

        std::pair<STAFCommandParseResultImpl::OptionInstanceMap::iterator,
                  STAFCommandParseResultImpl::OptionInstanceMap::iterator> 
             iterPair = res.fInstanceMap.equal_range(theOptionName);

        if (iterPair.first == res.fInstanceMap.end())
        {
            *numTimes = 0;
        }
        else
        {
            #if defined(STAF_OS_NAME_HPUX)
                std::distance(iterPair.first, iterPair.second, *numTimes);
            #elif defined(STAF_OS_NAME_SOLARIS) && defined(__SUNPRO_CC)
                 std::distance(iterPair.first, iterPair.second, *numTimes);
            #else
                *numTimes = std::distance(iterPair.first, iterPair.second);
            #endif
        }
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}

STAFRC_t STAFCommandParseResultGetOptionValue(
    STAFCommandParseResult_t result, const STAFString_t optionName,
    unsigned int optionIndex, STAFString_t *optionValue,
    unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    if ((optionIndex == 0) || (optionName == 0) || (optionValue == 0))
        return kSTAFInvalidParm;

    try
    {
        STAFCommandParseResultImpl &res = *result;

        STAFString theOptionName(optionName);

        if (!res.fCaseSensitive)
            theOptionName.lowerCase();

        std::pair<STAFCommandParseResultImpl::OptionInstanceMap::iterator,
                  STAFCommandParseResultImpl::OptionInstanceMap::iterator>
            iterPair = res.fInstanceMap.equal_range(theOptionName);

        while ((--optionIndex != 0) && (iterPair.first != iterPair.second) &&
               (iterPair.first != res.fInstanceMap.end()))
        { ++iterPair.first; }

        if ((iterPair.first == res.fInstanceMap.end()) ||
            (iterPair.first == iterPair.second))
        {
            *optionValue = 0;
        }
        else
        {
            *optionValue = iterPair.first->second.value.getImpl();
        }
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParseResultGetNumArgs(STAFCommandParseResult_t result,
    unsigned int *numArgs, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParseResultImpl &res = *result;

        *numArgs = res.fArgList.size();

    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParseResultGetArgValue(
    STAFCommandParseResult_t result, unsigned int argNum,
    STAFString_t *argValue, unsigned int *osRC)
{
    STAFRC_t rc = kSTAFOk;

    if (result == 0) return kSTAFInvalidObject;

    try
    {
        STAFCommandParseResultImpl &res = *result;

        *argValue = res.fArgList[argNum].getImpl();
    }
    catch(...)
    { rc = kSTAFUnknownError; if (osRC) *osRC = 0xFFFFFFFF;}

    return rc;
}


STAFRC_t STAFCommandParseResultDestruct(STAFCommandParseResult_t *pResult,
                                        unsigned int *osRC)
{
    if (pResult == 0) return kSTAFInvalidObject;

    delete *pResult;
    *pResult = 0;
    return kSTAFOk;
}
