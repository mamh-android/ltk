/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_CommandParserInlImpl
#define STAF_CommandParserInlImpl

#include "STAF.h"
#include "STAFCommandParser.h"
#include "STAFException.h"

STAF_INLINE unsigned int STAFCommandParseResult::optionTimes(
    const STAFString &optionName)
{
    unsigned int osRC = 0;
    unsigned int numTimes = 0;

    STAFRC_t rc = STAFCommandParseResultGetOptionTimes(fResultImpl,
                  optionName.getImpl(), &numTimes, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetOptionTimes", osRC);

    return numTimes;
}


STAF_INLINE STAFString STAFCommandParseResult::optionValue(
    const STAFString &optionName, unsigned int number)
{
    unsigned int osRC = 0;
    STAFString_t optValue = 0;

    STAFRC_t rc = STAFCommandParseResultGetOptionValue(fResultImpl,
                  optionName.getImpl(), number, &optValue, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetOptionValue", osRC);

    return optValue;
}


STAF_INLINE unsigned int STAFCommandParseResult::numInstances()
{
    unsigned int osRC = 0;
    unsigned int numInst = 0;

    STAFRC_t rc = STAFCommandParseResultGetNumInstances(fResultImpl,
                                                        &numInst, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetNumInstances", osRC);

    return numInst;
}


STAF_INLINE STAFString STAFCommandParseResult::instanceName(unsigned int number)
{
    unsigned int osRC = 0;
    STAFString_t instName = 0;

    STAFRC_t rc = STAFCommandParseResultGetInstanceName(fResultImpl, number,
                                                        &instName, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetInstanceName", osRC);

    return instName;
}


STAF_INLINE STAFString STAFCommandParseResult::instanceValue(unsigned int number)
{
    unsigned int osRC = 0;
    STAFString_t instValue = 0;
    
    STAFRC_t rc = STAFCommandParseResultGetInstanceValue(fResultImpl, number,
                                                         &instValue, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetInstanceValue", osRC);

    return instValue;
}


STAF_INLINE unsigned int STAFCommandParseResult::numArgs()
{
    unsigned int osRC = 0;
    unsigned int numArg = 0;

    STAFRC_t rc = STAFCommandParseResultGetNumArgs(fResultImpl, &numArg, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetNumArgs", osRC);

    return numArg;
}


STAF_INLINE STAFString STAFCommandParseResult::arg(unsigned int number)
{
    unsigned int osRC = 0;
    STAFString_t argValue = 0;
   
    STAFRC_t rc = STAFCommandParseResultGetArgValue(fResultImpl, number,
                                                    &argValue, &osRC);

    STAFException::checkRC(rc, "STAFCommandParseResultGetArgValue", osRC);

    return argValue;
}


STAF_INLINE STAFCommandParseResult::~STAFCommandParseResult()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFCommandParseResultDestruct(&fResultImpl, &osRC);
}


// Begin inline declarations for STAFCommandParser

STAF_INLINE STAFCommandParser::STAFCommandParser(unsigned int maxArgs,
                                            bool caseSensitive)
    : fParserImpl(0)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFCommandParserConstruct(&fParserImpl, maxArgs,
                                             caseSensitive, &osRC);

    STAFException::checkRC(rc, "STAFCommandParserConstruct", osRC);
}


STAF_INLINE void STAFCommandParser::addOption(const STAFString &option,
    unsigned int numAllowed, ValueRequirement valueReq)
{
    unsigned int osRC = 0;

    STAFRC_t rc = STAFCommandParserAddOption(fParserImpl, option.getImpl(),
                                             numAllowed, valueReq, &osRC);

    STAFException::checkRC(rc, "STAFCommandParserAddOption", osRC);
}


STAF_INLINE void STAFCommandParser::addOptionGroup(const STAFString &group,
     unsigned int minAllowed, unsigned int maxAllowed)
{
    unsigned int osRC = 0;

    STAFRC_t rc = STAFCommandParserAddOptionGroup(fParserImpl,
                  group.getImpl(), minAllowed, maxAllowed, &osRC);

    STAFException::checkRC(rc, "STAFCommandParserAddOptionGroup", osRC);
}


STAF_INLINE void STAFCommandParser::addOptionNeed(const STAFString &needers,
                                             const STAFString &needees)
{
    unsigned int osRC = 0;

    STAFRC_t rc = STAFCommandParserAddOptionNeed(fParserImpl,
                  needers.getImpl(), needees.getImpl(), &osRC);

    STAFException::checkRC(rc, "STAFCommandParserAddOptionNeed", osRC);
}


STAF_INLINE STAFCommandParseResultPtr STAFCommandParser::parse(
    const STAFString &parseString)
{
    unsigned int osRC = 0;
    STAFCommandParseResult_t result = 0;
    STAFString_t errorBuffer = 0;

    STAFRC_t rc = STAFCommandParserParseString(fParserImpl,
                  parseString.getImpl(), &result, &errorBuffer, &osRC);

    return STAFCommandParseResultPtr(
                new STAFCommandParseResult(result, rc, errorBuffer),
                STAFCommandParseResultPtr::INIT);
}


STAF_INLINE STAFCommandParser::~STAFCommandParser()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFCommandParserDestruct(&fParserImpl, &osRC);
}

#endif
