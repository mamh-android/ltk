/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_STAF
#define STAF_STAF

#ifndef STAF_NATIVE_COMPILER
#define STAF_INLINE inline
#else
#define STAF_INLINE
#endif

#include "STAFOSTypes.h"
#include "STAFError.h"
#include "STAFDataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int STAFHandle_t;
typedef unsigned int STAFSyncOption_t;

enum STAFSyncOption_e
{ 
    kSTAFReqSync = 0, 
    kSTAFReqFireAndForget = 1,
    kSTAFReqQueue = 2, 
    kSTAFReqRetain = 3,
    kSTAFReqQueueRetain = 4 
};


STAFRC_t STAFRegister(char *processName, STAFHandle_t *handle);
STAFRC_t STAFRegisterUTF8(char *processName, STAFHandle_t *handle);
STAFRC_t STAFUnRegister(STAFHandle_t handle);
STAFRC_t STAFSubmit(STAFHandle_t handle, char *where, char *service,
                    char *request, unsigned int requestLength,
                    char **resultPtr, unsigned int *resultLength);
STAFRC_t STAFSubmitUTF8(STAFHandle_t handle, char *where,
                        char *service, char *request,
                        unsigned int requestLength,
                        char **resultPtr,
                        unsigned int *resultLength);
STAFRC_t STAFSubmit2(STAFHandle_t handle, STAFSyncOption_t syncOption,
                     char *where, char *service,
                     char *request, unsigned int requestLength,
                     char **resultPtr, unsigned int *resultLength);
STAFRC_t STAFSubmit2UTF8(STAFHandle_t handle, STAFSyncOption_t syncOption,
                         char *where, char *service, char *request,
                         unsigned int requestLength,
                         char **resultPtr,
                         unsigned int *resultLength);

STAFRC_t STAFFree(STAFHandle_t handle, char *result);

STAFRC_t STAFAddPrivacyDelimiters(STAFStringConst_t data,
                                  STAFString_t *result);
STAFRC_t STAFRemovePrivacyDelimiters(STAFStringConst_t data,
                                     unsigned int numLevels,
                                     STAFString_t *result);
STAFRC_t STAFMaskPrivateData(STAFStringConst_t data, STAFString_t *result);
STAFRC_t STAFEscapePrivacyDelimiters(STAFStringConst_t data,
                                     STAFString_t *result);

#ifdef __cplusplus
}

#include "STAFString.h"
#include "STAFRefPtr.h"

// STAFResult - This class contains the results of a STAFSubmit call

class STAFResult
{
public:

    STAFResult(STAFRC_t theRC = kSTAFOk,
               const STAFString &theResult = STAFString())
        : rc(theRC), result(theResult)
    { /* Do Nothing */ }

    STAFResult(STAFRC_t theRC, const char *data, unsigned int dataLen,
               STAFString::CodePageType codePageType)
        : rc(theRC), result(data, dataLen, codePageType)
    { /* Do Nothing */ }

    STAFResult(STAFRC_t theRC, const char *data, unsigned int dataLen,
               STAFString::CodePageType codePageType, bool doUnmarshallResult)
        : rc(theRC), result(data, dataLen, codePageType)
    {
        if (doUnmarshallResult)
        {
            resultContext = STAFObject::unmarshall(
                result, kSTAFUnmarshallingDefaults);
            resultObj = resultContext->getRootObject();
        }
        else
        {
            resultContext = STAFObject::createNone();
            resultObj = STAFObject::createNone();
        }
    }

    STAFRC_t rc;
    STAFString result;
    STAFObjectPtr resultObj;
    STAFObjectPtr resultContext;
};

class STAFHandle;
typedef STAFRefPtr<STAFResult> STAFResultPtr;
typedef STAFRefPtr<STAFHandle> STAFHandlePtr;


// STAFHandle - This class is used to interact with STAF.  You obtain a
//              STAFHandle via the create() call.

class STAFHandle
{
public:

    // This is the standard call to create a STAFHandle.  By default, this
    // STAFHandle object will unregister with STAF when destructed.
    static STAFRC_t create(const STAFString &name, STAFHandlePtr &handle);

    // This call is used to create a STAFHandle which uses an existing
    // STAFHandle_t.  By default, this STAFHandle object will not unregister
    // with STAF when destructed.
    static STAFRC_t create(STAFHandle_t handleT, STAFHandlePtr &handle,
                           bool doUnreg = false);

    STAFResultPtr submit(const STAFString &where, const STAFString &service,
                         const STAFString &request,
                         const STAFSyncOption_t synchOption = kSTAFReqSync);

    // This returns the colon-length-colon delimited version of a string
    static STAFString wrapData(const STAFString &data);

    // This will format a string for you.  See STAFUtilFormatString() in
    // STAFUtil.h
    //
    // Note: DO NOT try to pass STAFString's into the ... portion of this
    //       function.  The only supported data types are "unsigned int" and
    //       STAFString_t.  Therefore be sure to call getImpl() on all
    //       STAFString's before passing them to this method.

    static STAFString formatString(STAFStringConst_t formatString, ...);

    // This returns the endpoint without the port (strips @nnnn from the end
    // of the endpoint, if present)
    static STAFString stripPortFromEndpoint(const STAFString &endpoint);

    // This method returns the data with privacy delimiters added.
    // For example, if pass in "secret", it returns "!!@secret@!!".
    static STAFString addPrivacyDelimiters(const STAFString &data);

    // This method removes any privacy delimiters from the data.
    // For example, if pass in "!!@secret@!!", it returns "secret".
    static STAFString removePrivacyDelimiters(const STAFString &data,
                                              unsigned int numLevels = 0);

    // This method masks any private data indicated by the privacy delimiters
    // by replacing the private data with asterisks.
    // For example, if pass in "!!@secret@!!", it returns "************".
    static STAFString maskPrivateData(const STAFString &data);
    
    // This method returns the data with privacy delimiters escaped.
    // For example, if pass in "!!@secret@!!", it returns "^!!@secret^@!!".
    static STAFString escapePrivacyDelimiters(const STAFString &data);

    STAFHandle_t getHandle() { return fHandle; }

    // This call allows you to claim ownership of the underlying STAFHandle_t.
    // Once this call is made, this STAFHandle object is no longer valid, and
    // it is your responsibility to unregister the STAFHandle_t with STAF.
    STAFHandle_t adoptHandle();

    bool getDoUnreg() { return fDoUnreg; }
    void setDoUnreg(bool doUnreg) { fDoUnreg = doUnreg; }

    bool getDoUnmarshallResult() { return fDoUnmarshallResult; }

    void setDoUnmarshallResult(bool flag)
    {
        fDoUnmarshallResult = flag;
    }

    ~STAFHandle();

protected:

    STAFHandle(STAFHandle_t handle, bool doUnreg)
        : fDoUnreg(doUnreg), fHandle(handle)
    {
        fDoUnmarshallResult = true;
    }

    bool fDoUnreg;
    STAFHandle_t fHandle;
    bool fDoUnmarshallResult;
};


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus

#endif

#endif
