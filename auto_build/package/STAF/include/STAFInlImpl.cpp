/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_STAFInlImpl
#define STAF_STAFInlImpl

#include "STAF.h"
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAFUtil.h"
#include <stdarg.h>

STAF_INLINE STAFRC_t STAFHandle::create(const STAFString &name,
                                        STAFHandlePtr &handle)
{
    STAFString theName(name);
    STAFHandle_t theHandle = 0;

    // Make sure it is NULL terminated
    theName += kUTF8_NULL;

    STAFRC_t rc = STAFRegisterUTF8(const_cast<char *>(theName.buffer()),
                                   &theHandle);
    if (rc) return rc;

    handle = STAFHandlePtr(new STAFHandle(theHandle, true),
                           STAFHandlePtr::INIT);

    return kSTAFOk;
}


STAF_INLINE STAFRC_t STAFHandle::create(STAFHandle_t handleT,
                                        STAFHandlePtr &handle, bool doUnreg)
{
    handle = STAFHandlePtr(new STAFHandle(handleT, doUnreg),
                           STAFHandlePtr::INIT);
    return kSTAFOk;
}


STAF_INLINE STAFResultPtr STAFHandle::submit(const STAFString &where,
                                             const STAFString &service,
                                             const STAFString &request,
                                             const STAFSyncOption_t synchOption)
{
    STAFString theWhere(where);
    STAFString theService(service);
    unsigned int reqLength = 0;
    const char *theRequest = request.buffer(&reqLength);

    // Make sure they are NULL terminated
    theWhere += kUTF8_NULL;
    theService += kUTF8_NULL;

    char *result = 0;
    unsigned int resultLength = 0;
    STAFRC_t rc = STAFSubmit2UTF8(fHandle, synchOption, 
                                  const_cast<char *>(theWhere.buffer()),
                                  const_cast<char *>(theService.buffer()),
                                  const_cast<char *>(theRequest),
                                  reqLength, &result, &resultLength);
    
    STAFResultPtr theResult(new STAFResult(rc, result, resultLength, 
        STAFString::kUTF8, fDoUnmarshallResult), STAFResultPtr::INIT);
   
    STAFFree(fHandle, result);

    return theResult;
}



STAF_INLINE STAFString STAFHandle::wrapData(const STAFString &data)
{
    STAFString outString(kUTF8_COLON);
    outString += STAFString(data.length(STAFString::kChar));
    outString += kUTF8_COLON;
    outString += data;

    return outString;
}


STAF_INLINE STAFString STAFHandle::formatString(STAFStringConst_t formatString,
                                                ...)
{
    STAFString_t outString = 0;
    va_list args;

    va_start(args, formatString);

    STAFRC_t rc = STAFUtilFormatString2(formatString, &outString, args);

    va_end(args);

    // XXX: Throw exception if bad rc?

    return STAFString(outString, STAFString::kShallow);
}


STAF_INLINE STAFString STAFHandle::stripPortFromEndpoint(
    const STAFString &endpoint)
{
    STAFString_t endpointNoPort = 0;

    STAFRC_t rc = STAFUtilStripPortFromEndpoint(endpoint.getImpl(),
                                                &endpointNoPort);

    STAFException::checkRC(rc, "STAFStripPortFromEndpoint");

    return STAFString(endpointNoPort, STAFString::kShallow);
}


STAF_INLINE STAFString STAFHandle::addPrivacyDelimiters(const STAFString &data)
{
    STAFString_t result = 0;

    STAFRC_t rc = STAFAddPrivacyDelimiters(data.getImpl(), &result);

    STAFException::checkRC(rc, "STAFAddPrivacyDelimiters");

    return STAFString(result, STAFString::kShallow);
}


STAF_INLINE STAFString STAFHandle::removePrivacyDelimiters(
    const STAFString &data, unsigned int numLevels)
{
    STAFString_t result = 0;

    STAFRC_t rc = STAFRemovePrivacyDelimiters(
        data.getImpl(), numLevels, &result);

    STAFException::checkRC(rc, "STAFRemovePrivacyDelimiters");

    return STAFString(result, STAFString::kShallow);
}
    

STAF_INLINE STAFString STAFHandle::maskPrivateData(const STAFString &data)
{
    STAFString_t result = 0;

    STAFRC_t rc = STAFMaskPrivateData(data.getImpl(), &result);

    STAFException::checkRC(rc, "STAFMaskPrivateData");

    return STAFString(result, STAFString::kShallow);
}


STAF_INLINE STAFString STAFHandle::escapePrivacyDelimiters(
    const STAFString &data)
{
    STAFString_t result = 0;

    STAFRC_t rc = STAFEscapePrivacyDelimiters(data.getImpl(), &result);

    STAFException::checkRC(rc, "STAFEscapePrivacyDelimiters");

    return STAFString(result, STAFString::kShallow);
}


STAF_INLINE STAFHandle_t STAFHandle::adoptHandle()
{
    STAFHandle_t temp = fHandle;

    fHandle = 0;
    fDoUnreg = false;

    return temp;
}


STAF_INLINE STAFHandle::~STAFHandle()
{
    if (fDoUnreg) STAFUnRegister(fHandle);
}

#endif
