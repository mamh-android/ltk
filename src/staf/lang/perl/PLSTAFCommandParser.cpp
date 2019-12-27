/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#define PERL_NO_GET_CONTEXT

#ifdef __SUNPRO_CC
# include <sys/vnode.h>
#endif

#include "STAFCommandParser.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

//The following is needed to compile on z/OS
#ifdef THING
    #undef THING
#endif

#include "PLSTAFCommandParser.h"

XS(XS_STAFCommandParser_new)
{
    dXSARGS;
    if (items < 1 || items > 3)
        Perl_croak(aTHX_ "Usage: $parser = STAFCommandParser->new($maxArgs=0, $caseSensitive=0)");
    {
        //cerr << "Entering constructor (STAFCommandParser)." << endl;

        const char *    CLASS = (const char *)SvPV_nolen(ST(0));
        unsigned int    maxArgs;
        unsigned int            caseSensitive;
        STAFCommandParser *RETVAL;

        if (items < 2)
            maxArgs = 0;
        else {
            maxArgs = (unsigned int)SvUV(ST(1));
        }

        if (items < 3)
            caseSensitive = false;
        else {
            caseSensitive = SvTRUE(ST(2));
        }

        RETVAL = new STAFCommandParser(maxArgs, caseSensitive);

        if (RETVAL == NULL) {
            Perl_croak(aTHX_ "Failed due to insufficient memory.");
        }

        ST(0) = sv_newmortal();
        sv_setref_pv(ST(0), CLASS, (void*)RETVAL);

        //cerr << "Leaving constructor (STAFCommandParser)." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParser_DESTROY)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: $parser->DESTROY()");
    {
        //cerr << "Entering destructor (STAFCommandParser)." << endl;

        STAFCommandParser *THING;

        if (SvROK(ST(0))) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParser *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not a reference");

        delete THING;

        //cerr << "Leaving destructor (STAFCommandParser)." << endl;
    }
    XSRETURN_EMPTY;
}

XS(XS_STAFCommandParser_addOption)
{
    dXSARGS;
    if (items != 4)
        Perl_croak(aTHX_ "Usage: $parser->addOption($optionName, $numAllowed, $valueReq)");
    {
        //cerr << "Entering addOption." << endl;

        STAFCommandParser *THING;
        STAFString       optionName;
        unsigned int     numAllowed = (unsigned int)SvUV(ST(2));
        STAFCommandParser::ValueRequirement valueReq   =
            (STAFCommandParser::ValueRequirement)SvIV(ST(3));

        if (sv_derived_from(ST(0), "STAFCommandParser")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParser *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParser");

        if (SvPOK(ST(1))) {
            STRLEN len;

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(1), len);
                optionName = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(1), len);
                optionName = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(1) is not of type STAFString");

        THING->addOption(optionName, numAllowed, valueReq);

        //cerr << "Leaving addOption." << endl;
    }
    XSRETURN_EMPTY;
}

XS(XS_STAFCommandParser_addOptionGroup)
{
    dXSARGS;
    if (items != 4)
        Perl_croak(aTHX_ "Usage: $parser->addOptionGroup($optionGroup, $minAllowed, $maxAllowed)");
    {
        //cerr << "Entering addOptionGroup." << endl;

        STAFCommandParser *THING;
        STAFString      optionGroup;
        unsigned int    minAllowed = (unsigned int)SvUV(ST(2));
        unsigned int    maxAllowed = (unsigned int)SvUV(ST(3));

        if (sv_derived_from(ST(0), "STAFCommandParser")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParser *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParser");

        if (SvPOK(ST(1))) {
            STRLEN len; 

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(1), len);
                optionGroup = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(1), len);
                optionGroup = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(1) is not of type STAFString");

        THING->addOptionGroup(optionGroup, minAllowed, maxAllowed);

        //cerr << "Leaving addOptionGroup." << endl;
    }
    XSRETURN_EMPTY;
}

XS(XS_STAFCommandParser_addOptionNeed)
{
    dXSARGS;
    if (items != 3)
        Perl_croak(aTHX_ "Usage: $parser->addOptionNeed($needers, $needees)");
    {
        //cerr << "Entering addOptionNeed." << endl;

        STAFCommandParser *THING;
        STAFString    needers;
        STAFString    needees;

        if (sv_derived_from(ST(0), "STAFCommandParser")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParser *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParser");

        if (SvPOK(ST(1))) {
            STRLEN len;

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(1), len);
                needers = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(1), len);
                needers = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(1) is not of type STAFString");

        if (SvPOK(ST(2))) {
            STRLEN len;

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(2), len);
                needees = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(2), len);
                needees = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(2) is not of type STAFString");

        THING->addOptionNeed(needers, needees);

        //cerr << "Leaving addOptionNeed." << endl;
    }
    XSRETURN_EMPTY;
}

XS(XS_STAFCommandParser_parseRequest)
{
    dXSARGS;
    if (items != 2)
        Perl_croak(aTHX_ "Usage: $result = $parser->parseRequest($request)");
    {
        //cerr << "Entering parse." << endl;

        STAFCommandParser *THING;
        STAFString    request;
        STAFCommandParseResultPtr *RETVAL;
        STAFCommandParseResultPtr result; 

        if (sv_derived_from(ST(0), "STAFCommandParser")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParser *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParser");

        if (SvPOK(ST(1))) {
            STRLEN len;

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(1), len);
                request = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(1), len);
                request = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(1) is not of type STAFString");  

        result = THING->parse(request);

        RETVAL = new STAFCommandParseResultPtr(result);
  
        if (RETVAL == NULL) {
            Perl_croak(aTHX_ "Failed due to insufficient memory.");
        }

        ST(0) = sv_newmortal();

        sv_setref_pv(ST(0), "STAFCommandParseResultPtr", (void*)RETVAL);

        //cerr << "Leaving parse." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_rc)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: $rc = $result->rc()");
    {
        //cerr << "Entering rc." << endl;

        STAFCommandParseResultPtr *THING;
        STAFRC_t     RETVAL;
        dXSTARG;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->rc;

        XSprePUSH;
        PUSHu((UV)RETVAL);

        //cerr << "Leaving rc." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_errorBuffer)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: $message = $result->errorBuffer()");
    {
        //cerr << "Entering errorBuffer." << endl;

        STAFCommandParseResultPtr *THING;
        STAFString  RETVAL;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->errorBuffer;

        ST(0) = sv_newmortal();
        {
            unsigned int len;
            const char *tmp;

            #ifdef STAF_OS_NAME_ZOS
                STAFStringBufferPtr ptr = RETVAL.toCurrentCodePage();
                tmp = ptr->buffer();
                len = ptr->length();

                sv_setpvn((SV*)ST(0), tmp, len);
            #else
                tmp = RETVAL.buffer(&len);

                sv_setpvn((SV*)ST(0), tmp, len);
                SvUTF8_on((SV*)ST(0));
            #endif   
        }

        //cerr << "Leaving errorBuffer." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_DESTROY)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: $result->DESTROY()");
    {
        //cerr << "Entering destructor (STAFCommandParseResultPtr)." << endl;

        STAFCommandParseResultPtr *THING;

        if (SvROK(ST(0))) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not a reference");

        delete THING;

        //cerr << "Leaving destructor (STAFCommandParseResultPtr)." << endl;
    }
    XSRETURN_EMPTY;
}

XS(XS_STAFCommandParseResultPtr_optionTimes)
{
    dXSARGS;
    if (items != 2)
        Perl_croak(aTHX_ "Usage: $number = $result->optionTimes($optionName)");
    {
        //cerr << "Entering optionTimes." << endl;

        STAFCommandParseResultPtr *THING;
        STAFString    optionName;
        unsigned int    RETVAL;
        dXSTARG;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        if (SvPOK(ST(1))) {
            STRLEN len;

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(1), len);
                optionName = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(1), len);
                optionName = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(1) is not of type STAFString");

        RETVAL = (*THING)->optionTimes(optionName);

        XSprePUSH;
        PUSHu((UV)RETVAL);

        //cerr << "Leaving optionTimes." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_optionValue)
{
    dXSARGS;
    if (items < 2 || items > 3)
        Perl_croak(aTHX_ "Usage: $value = $result->optionValue($optionName, $optionIndex=1)");
    {
        //cerr << "Entering optionValue." << endl;

        STAFCommandParseResultPtr *THING;
        STAFString      optionName;
        unsigned int    optionIndex;
        STAFString      RETVAL;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        if (SvPOK(ST(1))) {
            STRLEN len;

            #ifdef STAF_OS_NAME_ZOS
                char *tmp = (char *)SvPVbyte(ST(1), len);
                optionName = STAFString(tmp, len);
            #else 
                char *tmp = (char *)SvPVutf8(ST(1), len);
                optionName = STAFString(tmp, len, STAFString::kUTF8);
            #endif
        }
        else
            Perl_croak(aTHX_ "ST(1) is not of type STAFString");

        if (items < 3)
            optionIndex = 1;
        else {
            optionIndex = (unsigned int)SvUV(ST(2));
        }

        RETVAL = (*THING)->optionValue(optionName, optionIndex);

        ST(0) = sv_newmortal();
        {
            unsigned int len;
            const char *tmp;

            #ifdef STAF_OS_NAME_ZOS
                STAFStringBufferPtr ptr = RETVAL.toCurrentCodePage();
                tmp = ptr->buffer();
                len = ptr->length();

                sv_setpvn((SV*)ST(0), tmp, len);
            #else
                tmp = RETVAL.buffer(&len);

                sv_setpvn((SV*)ST(0), tmp, len);
                SvUTF8_on((SV*)ST(0));
            #endif    
        }

        //cerr << "Leaving optionValue." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_numInstances)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: $number = $result->numInstances()");
    {
        //cerr << "Entering numInstances." << endl;

        STAFCommandParseResultPtr *THING;
        unsigned int    RETVAL;
        dXSTARG;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->numInstances();

        XSprePUSH;
        PUSHu((UV)RETVAL);

        //cerr << "Leaving numInstances." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_instanceName)
{
    dXSARGS;
    if (items != 2)
        Perl_croak(aTHX_ "Usage: $name = $result->instanceName($instanceNum)");
    {
        //cerr << "Entering instanceName." << endl;

        STAFCommandParseResultPtr *THING;
        unsigned int    instanceNum = (unsigned int)SvUV(ST(1));
        STAFString      RETVAL;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->instanceName(instanceNum);

        ST(0) = sv_newmortal();
        {
            unsigned int len;
            const char *tmp;

            #ifdef STAF_OS_NAME_ZOS
                STAFStringBufferPtr ptr = RETVAL.toCurrentCodePage();
                tmp = ptr->buffer();
                len = ptr->length();

                sv_setpvn((SV*)ST(0), tmp, len);
            #else
                tmp = RETVAL.buffer(&len);

                sv_setpvn((SV*)ST(0), tmp, len);
                SvUTF8_on((SV*)ST(0));
            #endif    
        }

        //cerr << "Leaving instanceName." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_instanceValue)
{
    dXSARGS;
    if (items != 2)
        Perl_croak(aTHX_ "Usage: $value = $result->instanceValue($instanceNum)");
    {
        //cerr << "Entering instanceValue." << endl;

        STAFCommandParseResultPtr *THING;
        unsigned int    instanceNum = (unsigned int)SvUV(ST(1));
        STAFString      RETVAL;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->instanceValue(instanceNum);

        ST(0) = sv_newmortal();
        {
            unsigned int len;
            const char *tmp;

            #ifdef STAF_OS_NAME_ZOS
                STAFStringBufferPtr ptr = RETVAL.toCurrentCodePage();
                tmp = ptr->buffer();
                len = ptr->length();

                sv_setpvn((SV*)ST(0), tmp, len);
            #else
                tmp = RETVAL.buffer(&len);

                sv_setpvn((SV*)ST(0), tmp, len);
                SvUTF8_on((SV*)ST(0));
            #endif
        }

        //cerr << "Leaving instanceValue." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_numArgs)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: $number = $result->numArgs()");
    {
        //cerr << "Entering numArgs." << endl;

        STAFCommandParseResultPtr *THING;
        unsigned int    RETVAL;
        dXSTARG;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->numArgs();

        XSprePUSH;
        PUSHu((UV)RETVAL);

        //cerr << "Leaving numArgs." << endl;
    }
    XSRETURN(1);
}

XS(XS_STAFCommandParseResultPtr_arg)
{
    dXSARGS;
    if (items != 2)
        Perl_croak(aTHX_ "Usage: $value = $result->arg(argNum)");
    {
        //cerr << "Entering arg." << endl;

        STAFCommandParseResultPtr *THING;
        unsigned int    argNum = (unsigned int)SvUV(ST(1));
        STAFString      RETVAL;

        if (sv_derived_from(ST(0), "STAFCommandParseResultPtr")) {
            IV tmp = SvIV((SV*)SvRV(ST(0)));
            THING = INT2PTR(STAFCommandParseResultPtr *,tmp);
        }
        else
            Perl_croak(aTHX_ "Object is not of type STAFCommandParseResultPtr");

        RETVAL = (*THING)->arg(argNum);

        ST(0) = sv_newmortal();
        {
            unsigned int len;
            const char *tmp;

            #ifdef STAF_OS_NAME_ZOS
                STAFStringBufferPtr ptr = RETVAL.toCurrentCodePage();
                tmp = ptr->buffer();
                len = ptr->length();

                sv_setpvn((SV*)ST(0), tmp, len);
            #else
                tmp = RETVAL.buffer(&len);

                sv_setpvn((SV*)ST(0), tmp, len);
                SvUTF8_on((SV*)ST(0));
            #endif    
        }

        //cerr << "Leaving arg." << endl;
    }
    XSRETURN(1);
}
