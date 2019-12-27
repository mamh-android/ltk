/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#define PERL_NO_GET_CONTEXT

#ifdef __SUNPRO_CC
# include <sys/vnode.h>
#endif

#include "STAFOSTypes.h"
#include "STAF.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef STAF_OS_NAME_HPUX
extern "C"
{
    void _main();
}
#endif

#include "PLSTAF.h"
#include "PLSTAFCommandParser.h"

///////////////////////////////////////////////////////////////////////////////
#define SV_CREATE_YES 1
#define SV_CREATE_NO  0

static const unsigned int STAFPerlError = 4000;
static const char* STAF_HANDLE  = "STAF::Handle";
static const char* STAF_RESULT  = "STAF::Result";
static const char* STAF_RC      = "STAF::RC";
///////////////////////////////////////////////////////////////////////////////

static char sInfoBuffer[256]  = { 0 };
static int  sInfoBufferInited = 0;

extern "C"
char *STAFGetInformation()
{
   if (!sInfoBufferInited)
   {
       sprintf(sInfoBuffer, "%s STAF Perl support library version 2.0",
               #ifdef NDEBUG
                   "Retail");
               #else
                   "Debug");
               #endif
       sInfoBufferInited = 1;
   }

   return sInfoBuffer;
}


XS(XS_STAF_Register)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: STAF::Register(processName)");
    {
        #ifdef STAF_OS_NAME_ZOS
            char *processName = (char *)SvPVbyte_nolen(ST(0));
        #else
            char *processName = (char *)SvPVutf8_nolen(ST(0));
        #endif

        SV *stafRC     = get_sv((char *)STAF_RC,     SV_CREATE_YES);
        SV *stafHandle = get_sv((char *)STAF_HANDLE, SV_CREATE_YES);
        STAFHandle_t handle;
        STAFRC_t RETVAL;
        dXSTARG;

        if (stafRC == NULL || stafHandle == NULL)
            RETVAL = STAFPerlError;
        else {
            #ifdef STAF_OS_NAME_ZOS
                RETVAL = STAFRegister(processName, &handle);
            #else
                RETVAL = STAFRegisterUTF8(processName, &handle);
            #endif

            sv_setuv(stafRC, RETVAL);

            if (RETVAL == 0)
                sv_setuv(stafHandle, handle);
            else
                sv_setsv(stafHandle, &PL_sv_undef);
        }

        XSprePUSH;
        PUSHu((UV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_STAF_UnRegister)
{
    dXSARGS;
    if (items != 0)
        Perl_croak(aTHX_ "Usage: STAF::UnRegister()");
    {
        SV *stafRC     = get_sv((char *)STAF_RC,     SV_CREATE_NO);
        SV *stafHandle = get_sv((char *)STAF_HANDLE, SV_CREATE_NO);
        STAFHandle_t handle;
        STAFRC_t RETVAL;
        dXSTARG;

        if (stafRC == NULL || stafHandle == NULL)
            RETVAL = STAFPerlError;
        else {
            handle = SvUV(stafHandle);
            RETVAL = STAFUnRegister(handle);
            sv_setuv(stafRC, RETVAL);
            sv_setsv(stafHandle, &PL_sv_undef);
        }

        XSprePUSH;
        PUSHu((UV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_STAF_Submit)
{
    dXSARGS;
    if (items < 3 || items > 4)
        Perl_croak(aTHX_ "Usage: STAF::Submit(location, service, request, syncoption=kSTAFReqSync)");
    {
        #ifdef STAF_OS_NAME_ZOS
            char *location = (char *)SvPVbyte_nolen(ST(0));
            char *service  = (char *)SvPVbyte_nolen(ST(1));
            char *request  = (char *)SvPVbyte_nolen(ST(2));
        #else
            char *location = (char *)SvPVutf8_nolen(ST(0));
            char *service  = (char *)SvPVutf8_nolen(ST(1));
            char *request  = (char *)SvPVutf8_nolen(ST(2));
        #endif

        SV *stafHandle = get_sv((char *)STAF_HANDLE, SV_CREATE_NO);
        SV *stafRC     = get_sv((char *)STAF_RC,     SV_CREATE_NO);
        SV *stafResult = get_sv((char *)STAF_RESULT, SV_CREATE_YES);
        char *result = 0;
        unsigned int reslen = 0;
        STAFHandle_t handle;
        STAFSyncOption_t syncoption;
        STAFRC_t RETVAL;
        dXSTARG;

        if (items < 4)
            syncoption = kSTAFReqSync;
        else {
            syncoption = (STAFSyncOption_t)SvUV(ST(3));
        }

        if (stafRC == NULL || stafHandle == NULL || stafResult == NULL)
            RETVAL = STAFPerlError;
        else {
            handle = SvUV(stafHandle);

            #ifdef STAF_OS_NAME_ZOS
                RETVAL = STAFSubmit2(handle, syncoption, location, service, request,
                                     strlen(request), &result, &reslen);
            #else
                RETVAL = STAFSubmit2UTF8(handle, syncoption, location, service, request,
                                         strlen(request), &result, &reslen);
            #endif

            sv_setuv(stafRC, RETVAL);

            if (reslen == 0)
                sv_setsv(stafResult, &PL_sv_undef);
            else {
                sv_setpvn(stafResult, result, reslen);

                #ifndef STAF_OS_NAME_ZOS
                    SvUTF8_on(stafResult);
                #endif
            }
        }

        XSprePUSH;
        PUSHu((UV)RETVAL);

        if (result != 0)
            STAFFree(handle, result);
    }
    XSRETURN(1);
}

XS(XS_STAF_AddPrivacyDelimiters)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: STAF::AddPrivacyDelimiters(data)");
    {
        // Convert the Perl string argument to a STAFString_t

        #ifdef STAF_OS_NAME_ZOS
            char *dataPtr = (char *)SvPVbyte(ST(0), dataLength);
        #else
            STRLEN dataLength = 0;
            char *dataPtr = (char *)SvPVutf8(ST(0), dataLength);
        #endif

        STAFString_t data = 0;
        STAFStringConstruct(&data, dataPtr, dataLength, 0);

        // Call the C API to add privacy delimiters

        STAFString_t result = 0;
        dXSTARG;
        STAFAddPrivacyDelimiters(data, &result);

        // Convert result (STAFString_t) to a Perl string object

        unsigned int resultLength = 0;
        const char *resultBuffer = 0;
        STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);
        SV *retval = sv_2mortal(newSVpvn(resultBuffer, resultLength));

        #ifndef STAF_OS_NAME_ZOS
            SvUTF8_on(retval);
        #endif
        
        STAFStringDestruct(&data, 0);
        STAFStringDestruct(&result, 0);

        // Return the result string (with privacy delimiters added)

        XSprePUSH;
        PUSHs(retval);
    }
    XSRETURN(1);
}

XS(XS_STAF_EscapePrivacyDelimiters)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: STAF::EscapePrivacyDelimiters(data)");
    {
        // Convert the Perl string argument to a STAFString_t

        #ifdef STAF_OS_NAME_ZOS
            char *dataPtr = (char *)SvPVbyte(ST(0), dataLength);
        #else
            STRLEN dataLength = 0;
            char *dataPtr = (char *)SvPVutf8(ST(0), dataLength);
        #endif

        STAFString_t data = 0;
        STAFStringConstruct(&data, dataPtr, dataLength, 0);

        // Call the C API to add privacy delimiters

        STAFString_t result = 0;
        dXSTARG;
        STAFEscapePrivacyDelimiters(data, &result);

        // Convert result (STAFString_t) to a Perl string object

        unsigned int resultLength = 0;
        const char *resultBuffer = 0;
        STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);
        SV *retval = sv_2mortal(newSVpvn(resultBuffer, resultLength));

        #ifndef STAF_OS_NAME_ZOS
            SvUTF8_on(retval);
        #endif

        STAFStringDestruct(&data, 0);
        STAFStringDestruct(&result, 0);

        // Return the result string (with privacy delimiters escaped)

        XSprePUSH;
        PUSHs(retval);
    }
    XSRETURN(1);
}

XS(XS_STAF_MaskPrivateData)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: STAF::MaskPrivateData(data)");
    {
        // Convert the Perl string argument to a STAFString_t

        #ifdef STAF_OS_NAME_ZOS
            char *dataPtr = (char *)SvPVbyte(ST(0), dataLength);
        #else
            STRLEN dataLength = 0;
            char *dataPtr = (char *)SvPVutf8(ST(0), dataLength);
        #endif

        STAFString_t data = 0;
        STAFStringConstruct(&data, dataPtr, dataLength, 0);

        // Call the C API to add privacy delimiters

        STAFString_t result = 0;
        dXSTARG;
        STAFMaskPrivateData(data, &result);

        // Convert result (STAFString_t) to a Perl string object

        unsigned int resultLength = 0;
        const char *resultBuffer = 0;
        STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);
        SV *retval = sv_2mortal(newSVpvn(resultBuffer, resultLength));

        #ifndef STAF_OS_NAME_ZOS
            SvUTF8_on(retval);
        #endif

        STAFStringDestruct(&data, 0);
        STAFStringDestruct(&result, 0);

        // Return the result string (with private data masked)

        XSprePUSH;
        PUSHs(retval);
    }
    XSRETURN(1);
}

XS(XS_STAF_RemovePrivacyDelimiters)
{
    dXSARGS;
    if (items != 1 && items != 2)
    {
        Perl_croak(
            aTHX_ "Usage: STAF::RemovePrivacyDelimiters(data, numLevels = 0)");
    }
    {
        int numLevels = 0;

        if (items == 2)
        {
            numLevels = (STAFSyncOption_t)SvUV(ST(1));
        }

        // Convert the Perl string argument to a STAFString_t

        #ifdef STAF_OS_NAME_ZOS
            char *dataPtr = (char *)SvPVbyte(ST(0), dataLength);
        #else
            STRLEN dataLength = 0;
            char *dataPtr = (char *)SvPVutf8(ST(0), dataLength);
        #endif

        STAFString_t data = 0;
        STAFStringConstruct(&data, dataPtr, dataLength, 0);

        // Call the C API to add privacy delimiters

        STAFString_t result = 0;
        dXSTARG;
        STAFRemovePrivacyDelimiters(data, numLevels, &result);

        // Convert result (STAFString_t) to a Perl string object

        unsigned int resultLength = 0;
        const char *resultBuffer = 0;
        STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);
        SV *retval = sv_2mortal(newSVpvn(resultBuffer, resultLength));

        #ifndef STAF_OS_NAME_ZOS
            SvUTF8_on(retval);
        #endif

        STAFStringDestruct(&data, 0);
        STAFStringDestruct(&result, 0);

        // Return the result string (with privacy delimiters removed)

        XSprePUSH;
        PUSHs(retval);
    }
    XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_PLSTAF)
{
    // XXX: THIS PATH THING HERE IS GOING TO BE A PROBLEM
    dXSARGS;
    char* thisFile = __FILE__;
    char* cmdParserFile = "PLSTAFCommandParser.cpp";

    #ifdef STAF_OS_NAME_HPUX
        _main();
    #endif

    XS_VERSION_BOOTCHECK ;

        newXS("STAF::Register", XS_STAF_Register, thisFile);
        newXS("STAF::UnRegister", XS_STAF_UnRegister, thisFile);
        newXS("STAF::Submit", XS_STAF_Submit, thisFile);
        newXS("STAF::AddPrivacyDelimiters", XS_STAF_AddPrivacyDelimiters, thisFile);
        newXS("STAF::EscapePrivacyDelimiters", XS_STAF_EscapePrivacyDelimiters, thisFile);
        newXS("STAF::MaskPrivateData", XS_STAF_MaskPrivateData, thisFile);
        newXS("STAF::RemovePrivacyDelimiters", XS_STAF_RemovePrivacyDelimiters, thisFile);

        newXS("STAFCommandParser::new", XS_STAFCommandParser_new, cmdParserFile);
        newXS("STAFCommandParser::DESTROY", XS_STAFCommandParser_DESTROY, cmdParserFile);
        newXS("STAFCommandParser::addOption", XS_STAFCommandParser_addOption, cmdParserFile);
        newXS("STAFCommandParser::addOptionGroup", XS_STAFCommandParser_addOptionGroup, cmdParserFile);
        newXS("STAFCommandParser::addOptionNeed", XS_STAFCommandParser_addOptionNeed, cmdParserFile);
        newXS("STAFCommandParser::parseRequest", XS_STAFCommandParser_parseRequest, cmdParserFile);
        newXS("STAFCommandParseResultPtr::rc", XS_STAFCommandParseResultPtr_rc, cmdParserFile);
        newXS("STAFCommandParseResultPtr::errorBuffer", XS_STAFCommandParseResultPtr_errorBuffer, cmdParserFile);
        newXS("STAFCommandParseResultPtr::DESTROY", XS_STAFCommandParseResultPtr_DESTROY, cmdParserFile);
        newXS("STAFCommandParseResultPtr::optionTimes", XS_STAFCommandParseResultPtr_optionTimes, cmdParserFile);
        newXS("STAFCommandParseResultPtr::optionValue", XS_STAFCommandParseResultPtr_optionValue, cmdParserFile);
        newXS("STAFCommandParseResultPtr::numInstances", XS_STAFCommandParseResultPtr_numInstances, cmdParserFile);
        newXS("STAFCommandParseResultPtr::instanceName", XS_STAFCommandParseResultPtr_instanceName, cmdParserFile);
        newXS("STAFCommandParseResultPtr::instanceValue", XS_STAFCommandParseResultPtr_instanceValue, cmdParserFile);
        newXS("STAFCommandParseResultPtr::numArgs", XS_STAFCommandParseResultPtr_numArgs, cmdParserFile);
        newXS("STAFCommandParseResultPtr::arg", XS_STAFCommandParseResultPtr_arg, cmdParserFile);

    XSRETURN_YES;
}
