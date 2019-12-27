/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ProcUtil
#define STAF_ProcUtil

#ifdef __cplusplus

#include "STAFString.h"
#include "STAF.h"
#include "STAFException.h"
#include "STAFCommandParser.h"
#include "STAFVariablePool.h"
#include "STAFService.h"

extern bool isLocalMachine(const STAFString &machine,
                           unsigned int fullCompare = 0);

extern STAFRC_t resolveUIntIfExists(STAFCommandParseResultPtr &parsedResult,
                                    const STAFString &optionName,
                                    const STAFVariablePool *varPoolList[],
                                    unsigned int varPoolListSize,
                                    unsigned int &result,
                                    STAFString &errorBuffer,
                                    unsigned int defaultValue = 0,
                                    unsigned int minimumValue = 0,
                                    unsigned int maximumValue = UINT_MAX);

extern STAFRC_t resolveUInt(const STAFString &unresolved,
                            const STAFVariablePool *varPoolList[],
                            unsigned int varPoolListSize,
                            unsigned int &result,
                            STAFString &errorBuffer,
                            unsigned int defaultValue = 0,
                            unsigned int minimumValue = 0,
                            unsigned int maximumValue = UINT_MAX);

extern STAFRC_t resolveUInt(const STAFString &unresolved,
                            const STAFString &optionName,
                            const STAFVariablePool *varPoolList[],
                            unsigned int varPoolListSize,
                            unsigned int &result,
                            STAFString &errorBuffer,
                            unsigned int defaultValue = 0,
                            unsigned int minimumValue = 0,
                            unsigned int maximumValue = UINT_MAX);

extern STAFRC_t convertStringToUInt(const STAFString &theString,
                                    const STAFString &optionName,
                                    unsigned int &number,
                                    STAFString &errorBuffer,
                                    unsigned int minValue = 0,
                                    unsigned int maxValue = UINT_MAX);

extern STAFRC_t resolveStringIfExists(STAFCommandParseResultPtr &parsedResult,
                                      const STAFString &optionName,
                                      const STAFVariablePool *varPoolList[],
                                      unsigned int varPoolListSize,
                                      STAFString &result,
                                      STAFString &errorBuffer,
                                      bool ignoreVarResolveErrors = false);

extern STAFRC_t resolveString(const STAFString &unresolved,
                              const STAFVariablePool *varPoolList[],
                              unsigned int varPoolListSize,
                              STAFString &result, STAFString &errorBuffer,
                              bool ignoreVarResolveErrors = false); 

extern STAFRC_t resolveDurationStringIfExists(
    STAFCommandParseResultPtr &parsedResult, const STAFString &optionName,
    const STAFVariablePool *varPoolList[], unsigned int varPoolListSize,
    unsigned int &result, STAFString &errorBuffer,
    unsigned int defaultValue = 0);

extern STAFRC_t convertDurationStringToUInt(const STAFString &theString,
                                            const STAFString &optionName,
                                            unsigned int &number,
                                            STAFString &errorBuffer);

extern STAFRC_t validateTrust(unsigned int requiredTrust,
                              const STAFString &service,
                              const STAFString &request,
                              const STAFServiceRequest &requestInfo,
                              STAFString &errorBuffer,
                              unsigned int localOnly = 0);

extern STAFServiceResult submitAuthServiceRequest(
    const STAFServicePtr &service, const STAFString &request);

extern bool requiresSecureConnection(const STAFString &authenticator);

#define IVALIDATE_TRUST(requiredTrustLevel, request)\
STAFString stafProcUtilTrustErrorBuffer;\
STAFRC_t stafProcUtilTrustRC = validateTrust(requiredTrustLevel, name(),\
    request, requestInfo, stafProcUtilTrustErrorBuffer);\
if (stafProcUtilTrustRC != kSTAFOk)\
    return STAFServiceResult(kSTAFAccessDenied, stafProcUtilTrustErrorBuffer);

#define IVALIDATE_LOCAL_TRUST(requiredTrustLevel, request)\
STAFString stafProcUtilTrustErrorBuffer;\
STAFRC_t stafProcUtilTrustRC = validateTrust(requiredTrustLevel, name(),\
    request, requestInfo, stafProcUtilTrustErrorBuffer, 1);\
if (stafProcUtilTrustRC != kSTAFOk)\
    return STAFServiceResult(kSTAFAccessDenied, stafProcUtilTrustErrorBuffer);

#define RESOLVE_STRING(string, result)\
resolveString(string, varPoolList, varPoolListSize, result, errorBuffer);

#define RESOLVE_STRING_OPTION(option, result)\
resolveString(parsedResult->optionValue(option), varPoolList, varPoolListSize,\
              result, errorBuffer);

#define RESOLVE_INDEXED_STRING_OPTION(option, index, result)\
resolveString(parsedResult->optionValue(option, index), varPoolList,\
              varPoolListSize, result, errorBuffer);
              
#define RESOLVE_INDEXED_STRING_OPTION_IGNORE_ERRORS(option, index, result)\
resolveString(parsedResult->optionValue(option, index), varPoolList,\
              varPoolListSize, result, errorBuffer, true);

#define RESOLVE_OPTIONAL_STRING_OPTION(option, result)\
resolveStringIfExists(parsedResult, option, varPoolList, varPoolListSize,\
                      result, errorBuffer);
                      
#define RESOLVE_OPTIONAL_STRING_OPTION_IGNORE_ERRORS(option, result)\
resolveStringIfExists(parsedResult, option, varPoolList, varPoolListSize,\
                      result, errorBuffer, true);

#define RESOLVE_UINT(string, result)\
resolveUInt(string, varPoolList, varPoolListSize, result, errorBuffer);

#define RESOLVE_DEFAULT_UINT(string, result, default)\
resolveUInt(string, varPoolList, varPoolListSize, result, errorBuffer, default);

#define RESOLVE_INDEXED_UINT_OPTION(option, index, result)\
resolveUInt(parsedResult->optionValue(option, index), varPoolList,\
            varPoolListSize, result, errorBuffer);

#define RESOLVE_INDEXED_UINT_OPTION_RANGE(option, index, result, minValue, maxValue)\
resolveUInt(parsedResult->optionValue(option, index), varPoolList,\
            varPoolListSize, result, errorBuffer, 0, minValue, maxValue);

#define RESOLVE_UINT_OPTION(option, result)\
resolveUInt(parsedResult->optionValue(option), option,\
            varPoolList, varPoolListSize, result, errorBuffer);
           
#define RESOLVE_UINT_OPTION_RANGE(option, result, minValue, maxValue)\
resolveUInt(parsedResult->optionValue(option), option,\
            varPoolList, varPoolListSize, result, errorBuffer,\
            0, minValue, maxValue);

#define RESOLVE_DEFAULT_UINT_OPTION(option, result, default)\
resolveUInt(parsedResult->optionValue(option), option,\
            varPoolList, varPoolListSize,\
            result, errorBuffer, default);

#define RESOLVE_OPTIONAL_UINT_OPTION(option, result)\
resolveUIntIfExists(parsedResult, option, varPoolList, varPoolListSize, result,\
                    errorBuffer);
                    
#define RESOLVE_OPTIONAL_UINT_OPTION_RANGE(option, result, minValue, maxValue)\
resolveUIntIfExists(parsedResult, option, varPoolList, varPoolListSize, result,\
                    errorBuffer, 0, minValue, maxValue);
                    
#define RESOLVE_DEFAULT_DURATION_OPTION(option, result, defaultValue)\
resolveDurationStringIfExists(\
    parsedResult, option, varPoolList, varPoolListSize,\
    result, errorBuffer, defaultValue);

#define INVALID_SERVICE_NAME_ERROR_MESSAGE()\
"A service name (upper-case) cannot begin with \"STAF\", cannot be \"NONE\", "\
"and cannot contain the following special characters: ~!#$%^&*()+={[}]|;:?/<>\\"

#endif

#endif
