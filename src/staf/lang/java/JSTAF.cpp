/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <jni.h>
#include <string.h>
#include "com_ibm_staf_STAFHandle.h"
#include "com_ibm_staf_STAFUtil.h"
#include "STAF.h"

// These variables reference strings which will be passed into the JVM.  See
// additional comments in the initJVMStrings() function below.

const char *sJavaIntFieldType                  = 0;
const char *sJavaConstructorMethod             = 0;
const char *sSTAFHandleHandleField             = 0;
const char *sSTAFResultClass                   = 0;
const char *sSTAFResultConstructorMethodSig    = 0;
const char *sSTAFExceptionClass                = 0;
const char *sSTAFExceptionConstructorMethodSig = 0;

const STAFString *sSTAFResultClassStringPtr    = 0;


#if defined(STAF_OS_NAME_HPUX) && defined(STAF_INVOKE_JNI_INITIALIZE)
extern "C"
{
    void _main();
}
#endif

int throwSTAFException(JNIEnv *env, unsigned int rc, const char *utfString = 0);
jobject createResultObject(JNIEnv *env, unsigned int rc,
                           const char *utfString = 0,
                           const unsigned int resultLength = 0);


static void initJVMStrings()
{
    // JVM string constants
    //
    // Note: All strings that are passed into the JVM via the JNI interfaces
    //       must be sent in as "ASCII".  The JNI will not accept strings in the
    //       current encoding.  They must use the UTF-8 encodings from 0x01 to
    //       0x7F.  That is why we have all of the constants below.
    //
    // Note: This data must reside in this function.  If it is placed at file
    //       scope, then there are problems with the AIX runtime initializing
    //       things in a non-desirable order.

    // JVM Field types

    static STAFString sJavaIntFieldTypeString = STAFString("I") + kUTF8_NULL;

    sJavaIntFieldType = sJavaIntFieldTypeString.buffer();

    // Default Java Methods

    static STAFString sJavaConstructorMethodString =
        STAFString("<init>") + kUTF8_NULL;

    sJavaConstructorMethod = sJavaConstructorMethodString.buffer();

    // STAFHandle class definitions

    static STAFString sSTAFHandleHandleFieldString =
        STAFString("handle") + kUTF8_NULL;

    sSTAFHandleHandleField = sSTAFHandleHandleFieldString.buffer();

    // STAFResult class definitions

    static STAFString sSTAFResultClassString =
        STAFString("com/ibm/staf/STAFResult") + kUTF8_NULL;
    static STAFString sSTAFResultConstructorMethodSigString =
        STAFString("(ILjava/lang/String;)V") + kUTF8_NULL;

    sSTAFResultClass = sSTAFResultClassString.buffer();
    sSTAFResultConstructorMethodSig =
        sSTAFResultConstructorMethodSigString.buffer();

    sSTAFResultClassStringPtr = &sSTAFResultClassString;

    // STAFException class definitions

    static STAFString sSTAFExceptionClassString =
        STAFString("com/ibm/staf/STAFException") + kUTF8_NULL;
    static STAFString sSTAFExceptionConstructorMethodSigString =
        STAFString("(ILjava/lang/String;)V") + kUTF8_NULL;

    sSTAFExceptionClass = sSTAFExceptionClassString.buffer();
    sSTAFExceptionConstructorMethodSig =
        sSTAFExceptionConstructorMethodSigString.buffer();
}


/*
 * Class:     com_ibm_staf_STAFHandle
 * Method:    initialize
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_ibm_staf_STAFHandle_initialize
                       (JNIEnv *, jclass)
{
  #if defined(STAF_OS_NAME_HPUX) && defined(STAF_INVOKE_JNI_INITIALIZE)
        _main();
    #endif

   initJVMStrings();
}


/*
 * Class:     com_ibm_staf_STAFHandle
 * Method:    STAFRegister
 * Signature: (Ljava/lang/String;)V
 */

JNIEXPORT void JNICALL Java_com_ibm_staf_STAFHandle_STAFRegister
                       (JNIEnv *env, jobject theObject, jstring theHandleName)
{
    // Get the class for STAFHandle and the handle field ID

    jclass stafHandleClass = env->GetObjectClass(theObject);
    jfieldID handleFieldID = env->GetFieldID(stafHandleClass,
                                             sSTAFHandleHandleField,
                                             sJavaIntFieldType);
    if (handleFieldID == 0)
    {
        // We could not get the Field ID of STAFHandle.handle
        // The Java VM should already have thrown an exception, so we
        // just return

        return;
    }

    // Check if jstring is null because GetStringUTFChars core dumps if
    // pass it a null string.  Throw an exception if the string is null.
    if (theHandleName == 0)
    {
        throwSTAFException(env, kSTAFInvalidValue,
                           "Error - handleName string is null");
        return;
    }

    const char *handleName = env->GetStringUTFChars(theHandleName, 0);

    if (handleName == 0)
    {
        // We could not get the UTF-8 version of handleName, so throw an
        // exception

        throwSTAFException(env, kSTAFInvalidValue,
                           "Error getting UTF-8 handleName string");
        return;
    }

    STAFHandle_t handle = 0;
    unsigned int rc = STAFRegisterUTF8((char *)handleName, &handle);

    env->ReleaseStringUTFChars(theHandleName, handleName);

    if (rc != 0)
    {
        // Error registering with STAF, so throw an exception

        throwSTAFException(env, rc);

        return;
    }

    env->SetIntField(theObject, handleFieldID, (jint)handle);
}

/*
 * Class:     com_ibm_staf_STAFHandle
 * Method:    STAFUnRegister
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_ibm_staf_STAFHandle_STAFUnRegister
                       (JNIEnv *env, jobject theObject)
{
    // Get the class for STAFHandle and the handle field ID

    jclass stafHandleClass = env->GetObjectClass(theObject);
    jfieldID handleFieldID = env->GetFieldID(stafHandleClass,
                                             sSTAFHandleHandleField,
                                             sJavaIntFieldType);
    if (handleFieldID == 0)
    {
        // We could not get the Field ID of STAFHandle.handle
        // The Java VM should already have thrown an exception, so we
        // just return

        return;
    }

    unsigned int rc = STAFUnRegister(static_cast<STAFHandle_t>(
                      env->GetIntField(theObject, handleFieldID)));

    if (rc != 0)
    {
        // Error unregistering with STAF, so throw an exception

        throwSTAFException(env, rc);
    }
}


/*
 * Class:     com_ibm_staf_STAFHandle
 * Method:    STAFSubmit
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_ibm_staf_STAFHandle_STAFSubmit
                          (JNIEnv *env, jobject theObject, 
                          jint syncOpt, jstring where,
                          jstring service, jstring request)
{
    // Get the class for STAFHandle and the handle field ID

    jclass stafHandleClass = env->GetObjectClass(theObject);
    jfieldID handleFieldID = env->GetFieldID(stafHandleClass,
                                             sSTAFHandleHandleField,
                                             sJavaIntFieldType);
    if (handleFieldID == 0)
    {
        // We could not get the Field ID of STAFHandle.handle
        // The Java VM should already have thrown an exception, so we
        // just return

        return 0;
    }

    STAFHandle_t handle = static_cast<STAFHandle_t>(env->GetIntField(theObject,
                          handleFieldID));
                          
    STAFSyncOption_t syncOption = static_cast<STAFSyncOption_t>(syncOpt);

    if (where == 0 || service == 0 || request == 0)
    {
        // Check if jstring is null because GetStringUTFChars core dumps if
        // pass it a null string.  Throw an exception if the string is null.
        if (where == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error - where string is null");
        }
        else if (service == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error - service string is null");
        }
        else if (request == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error - request string is null");
        }

        return 0;
    }

    const char *whereUTF = env->GetStringUTFChars(where, 0);
    const char *serviceUTF = env->GetStringUTFChars(service, 0);
    const char *requestUTF = env->GetStringUTFChars(request, 0);

    if ((whereUTF == 0) || (serviceUTF == 0) || (requestUTF == 0))
    {
        // First release any memory we got from the Java VM

        if (whereUTF != 0) env->ReleaseStringUTFChars(where, whereUTF);
        if (serviceUTF != 0) env->ReleaseStringUTFChars(service, serviceUTF);
        if (requestUTF != 0) env->ReleaseStringUTFChars(request, requestUTF);

        // Now throw an exception

        if (whereUTF != 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error getting UTF-8 where string");
        }
        else if (serviceUTF != 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error getting UTF-8 service string");
        }
        else
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error getting UTF-8 request string");
        }

        return 0;
    }

    char *result = 0;
    unsigned int resultLength = 0;
    unsigned int rc = STAFSubmit2UTF8(handle, syncOption, (char *)whereUTF,
                                  (char *)serviceUTF, (char *)requestUTF,
                                  strlen(requestUTF), &result, &resultLength);

    // Now free up the memory from the Java VM

    env->ReleaseStringUTFChars(where, whereUTF);
    env->ReleaseStringUTFChars(service, serviceUTF);
    env->ReleaseStringUTFChars(request, requestUTF);

    // Now, check results of submit call

    jstring javaResult = 0;

    if (rc != 0) throwSTAFException(env, rc, result);
    else
    {
        if (result != 0)
        {
            if (resultLength == 0)
            {
                javaResult = env->NewStringUTF(result);
            }
            else
            {
                // Convert the result string for Java to handle null chars
                STAFString newResult = STAFString(result, resultLength,
                                                  STAFString::kUTF8);
                newResult = newResult.replace(kUTF8_NULL, kUTF8_NULL2);
                newResult += kUTF8_NULL;

                javaResult = env->NewStringUTF(const_cast<char *>
                             (newResult.buffer()));
            }
        }
        else
            javaResult = env->NewStringUTF("\00");

        if (javaResult == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error creating result string");
        }
    }

    if (result != 0) STAFFree(handle, result);

    return javaResult;
}


/*
 * Class:     com_ibm_staf_STAFHandle
 * Method:    STAFSubmit2
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)LCOM/IBM/ncsd/test/STAF/STAFResult;
 */
JNIEXPORT jobject JNICALL Java_com_ibm_staf_STAFHandle_STAFSubmit2
                          (JNIEnv *env, jobject theObject, 
                          jint syncOpt, jstring where,
                          jstring service, jstring request)
{
    // Get the class for STAFHandle and the handle field ID

    jclass stafHandleClass = env->GetObjectClass(theObject);
    jfieldID handleFieldID = env->GetFieldID(stafHandleClass,
                                             sSTAFHandleHandleField,
                                             sJavaIntFieldType);
    if (handleFieldID == 0)
    {
        // We could not get the Field ID of STAFHandle.handle
        // The Java VM should already have thrown an exception, so we
        // just return

        return 0;
    }

    STAFHandle_t handle = static_cast<STAFHandle_t>(env->GetIntField(theObject,
                          handleFieldID));
                          
    STAFSyncOption_t syncOption = static_cast<STAFSyncOption_t>(syncOpt);

    if (where == 0 || service == 0 || request == 0)
    {
        // Check if jstring is null because GetStringUTFChars core dumps if
        // pass it a null string.
        if (where == 0)
        {
            return createResultObject(env, kSTAFInvalidValue,
                                      "Error - where string is null");
        }
        else if (service == 0)
        {
            return createResultObject(env, kSTAFInvalidValue,
                                      "Error - service string is null");
        }
        else if (request == 0)
        {
            return createResultObject(env, kSTAFInvalidValue,
                                      "Error - request string is null");
        }
    }

    const char *whereUTF = env->GetStringUTFChars(where, 0);
    const char *serviceUTF = env->GetStringUTFChars(service, 0);
    const char *requestUTF = env->GetStringUTFChars(request, 0);

    if ((whereUTF == 0) || (serviceUTF == 0) || (requestUTF == 0))
    {
        // First release any memory we got from the Java VM

        if (whereUTF != 0) env->ReleaseStringUTFChars(where, whereUTF);
        if (serviceUTF != 0) env->ReleaseStringUTFChars(service, serviceUTF);
        if (requestUTF != 0) env->ReleaseStringUTFChars(request, requestUTF);

        // Now return the result object

        if (whereUTF != 0)
        {
            return createResultObject(env, kSTAFInvalidValue,
                   "Error getting UTF-8 where string");
        }
        else if (serviceUTF != 0)
        {
            return createResultObject(env, kSTAFInvalidValue,
                   "Error getting UTF-8 service string");
        }
        else
        {
            return createResultObject(env, kSTAFInvalidValue,
                   "Error getting UTF-8 request string");
        }
    }

    char *result = 0;
    unsigned int resultLength = 0;
    unsigned int rc = STAFSubmit2UTF8(handle, syncOption, (char *)whereUTF,
                                  (char *)serviceUTF, (char *)requestUTF,
                                  strlen(requestUTF), &result, &resultLength);

    // Now free up the memory from the Java VM

    env->ReleaseStringUTFChars(where, whereUTF);
    env->ReleaseStringUTFChars(service, serviceUTF);
    env->ReleaseStringUTFChars(request, requestUTF);

    // Now, return the result object
    
    jobject returnResult = createResultObject(env, rc, result, resultLength);

    if (result != 0) STAFFree(handle, result);

    return returnResult;
}

/*
 * Class:     com_ibm_staf_STAFUtil
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_ibm_staf_STAFUtil_initialize
  (JNIEnv *, jclass)
{
  #if defined(STAF_OS_NAME_HPUX) && defined(STAF_INVOKE_JNI_INITIALIZE)
        _main();
    #endif

    initJVMStrings();
}

/*
 * Class:     com_ibm_staf_STAFUtil
 * Method:    STAFUtilAddPrivacyDelimiters
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_ibm_staf_STAFUtil_STAFUtilAddPrivacyDelimiters
  (JNIEnv *env, jclass theObject, jstring data)
{
    // Check if jstring is null because GetStringUTFChars core dumps if
    // pass it a null string.
    if (data == 0) return data;
    
    unsigned int dataUTFLength = env->GetStringUTFLength(data);
    const char *dataUTF = env->GetStringUTFChars(data, 0);
    
    if (dataUTF == 0)
    {
        // We could not get the UTF-8 version of data, so throw an
        // exception

        throwSTAFException(env, kSTAFInvalidValue,
                           "Error getting UTF-8 data string");
        return 0;
    }

    STAFString result;
    jstring javaResult = 0;

    try
    {
        result = STAFHandle::addPrivacyDelimiters(
            STAFString(dataUTF, dataUTFLength, STAFString::kUTF8));

        // Now free up the memory from the Java VM

        env->ReleaseStringUTFChars(data, dataUTF);

        // Now, generate a jstring for the result

        if (result.length() != 0)
        {
            // Convert the result string for Java to handle null chars
            result = result.replace(kUTF8_NULL, kUTF8_NULL2);
            result += kUTF8_NULL;
            javaResult = env->NewStringUTF(
                const_cast<char *>(result.buffer()));
        }
        else
        {
            javaResult = env->NewStringUTF("\00");
        }

        if (javaResult == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error creating result string");
        }
    }
    catch (STAFException e)
    {
        throwSTAFException(
            env, kSTAFInvalidValue,
            "Error - STAFHandle::addPrivacyDelimiters() failed");
    }
    
    // Now, return the jstring

    return javaResult;
}

/*
 * Class:     com_ibm_staf_STAFUtil
 * Method:    STAFUtilRemovePrivacyDelimiters
 * Signature: (Ljava/lang/String;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_ibm_staf_STAFUtil_STAFUtilRemovePrivacyDelimiters
  (JNIEnv *env, jclass theObject, jstring data, jint numLevels)
{
    // Check if jstring is null because GetStringUTFChars core dumps if
    // pass it a null string.
    if (data == 0) return data;

    unsigned int dataUTFLength = env->GetStringUTFLength(data);
    const char *dataUTF = env->GetStringUTFChars(data, 0);
    
    if (dataUTF == 0)
    {
        // We could not get the UTF-8 version of data, so throw an
        // exception

        throwSTAFException(env, kSTAFInvalidValue,
                           "Error getting UTF-8 data string");
        return 0;
    }

    STAFString result;
    jstring javaResult = 0;

    try
    {
        result = STAFHandle::removePrivacyDelimiters(
            STAFString(dataUTF, dataUTFLength, STAFString::kUTF8), numLevels);

        // Now free up the memory from the Java VM

        env->ReleaseStringUTFChars(data, dataUTF);

        // Now, generate a jstring for the result

        if (result.length() != 0)
        {
            // Convert the result string for Java to handle null chars
            result = result.replace(kUTF8_NULL, kUTF8_NULL2);
            result += kUTF8_NULL;
            javaResult = env->NewStringUTF(
                const_cast<char *>(result.buffer()));
        }
        else
        {
            javaResult = env->NewStringUTF("\00");
        }

        if (javaResult == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error creating result string");
        }
    }
    catch (STAFException e)
    {
        throwSTAFException(
            env, kSTAFInvalidValue,
            "Error - STAFHandle::removePrivacyDelimiters() failed");
    }
    
    // Now, return the jstring

    return javaResult;
}

/*
 * Class:     com_ibm_staf_STAFUtil
 * Method:    STAFUtilMaskPrivateData
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_ibm_staf_STAFUtil_STAFUtilMaskPrivateData
  (JNIEnv *env, jclass theObject, jstring data)
{
    // Check if jstring is null because GetStringUTFChars core dumps if
    // pass it a null string.
    if (data == 0) return data;

    unsigned int dataUTFLength = env->GetStringUTFLength(data);
    const char *dataUTF = env->GetStringUTFChars(data, 0);
    
    
    if (dataUTF == 0)
    {
        // We could not get the UTF-8 version of data, so throw an
        // exception

        throwSTAFException(env, kSTAFInvalidValue,
                           "Error getting UTF-8 data string");
        return 0;
    }

    STAFString result;
    jstring javaResult = 0;

    try
    {
        result = STAFHandle::maskPrivateData(
            STAFString(dataUTF, dataUTFLength, STAFString::kUTF8));

        // Now free up the memory from the Java VM

        env->ReleaseStringUTFChars(data, dataUTF);

        // Now, generate a jstring for the result

        if (result.length() != 0)
        {
            // Convert the result string for Java to handle null chars
            result = result.replace(kUTF8_NULL, kUTF8_NULL2);
            result += kUTF8_NULL;
            javaResult = env->NewStringUTF(
                const_cast<char *>(result.buffer()));
        }
        else
        {
            javaResult = env->NewStringUTF("\00");
        }

        if (javaResult == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error creating result string");
        }
    }
    catch (STAFException e)
    {
        throwSTAFException(
            env, kSTAFInvalidValue,
            "Error - STAFHandle::maskPrivateData() failed");
    }
    
    // Now, return the jstring

    return javaResult;
}

/*
 * Class:     com_ibm_staf_STAFUtil
 * Method:    STAFUtilEscapePrivacyDelimiters
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_ibm_staf_STAFUtil_STAFUtilEscapePrivacyDelimiters
  (JNIEnv *env, jclass theObject, jstring data)
{
    // Check if jstring is null because GetStringUTFChars core dumps if
    // pass it a null string.
    if (data == 0) return data;

    unsigned int dataUTFLength = env->GetStringUTFLength(data);
    const char *dataUTF = env->GetStringUTFChars(data, 0);
    
    if (dataUTF == 0)
    {
        // We could not get the UTF-8 version of data, so throw an
        // exception

        throwSTAFException(env, kSTAFInvalidValue,
                           "Error getting UTF-8 data string");
        return 0;
    }

    STAFString result;
    jstring javaResult = 0;

    try
    {
        result = STAFHandle::escapePrivacyDelimiters(
            STAFString(dataUTF, dataUTFLength, STAFString::kUTF8));

        // Now free up the memory from the Java VM

        env->ReleaseStringUTFChars(data, dataUTF);

        // Now, generate a jstring for the result

        if (result.length() != 0)
        {
            // Convert the result string for Java to handle null chars
            result = result.replace(kUTF8_NULL, kUTF8_NULL2);
            result += kUTF8_NULL;
            javaResult = env->NewStringUTF(
                const_cast<char *>(result.buffer()));
        }
        else
        {
            javaResult = env->NewStringUTF("\00");
        }

        if (javaResult == 0)
        {
            throwSTAFException(env, kSTAFInvalidValue,
                               "Error creating result string");
        }
    }
    catch (STAFException e)
    {
        throwSTAFException(
            env, kSTAFInvalidValue,
            "Error - STAFHandle::escapePrivacyDelimiters() failed");
    }
    
    // Now, return the jstring

    return javaResult;
}


int throwSTAFException(JNIEnv *env, unsigned int rc, const char *utfString)
{
    jclass excClass = env->FindClass(sSTAFExceptionClass);

    if (excClass == 0)
    {
        // The Java VM threw an exception, so just return
        return 1;
    }

    jmethodID excInit = env->GetMethodID(excClass, sJavaConstructorMethod,
                                         sSTAFExceptionConstructorMethodSig);
    if (excInit == 0)
    {
        // The Java VM threw an exception, so just return
        return 1;
    }

    jstring javaString = 0;

    if (utfString != 0)
        javaString = env->NewStringUTF(utfString);
    else
        javaString = env->NewStringUTF("\00");

    // XXX: If javaString can not be created, I think we can create
    //      the exception object ok, but there may be a problem trying
    //      to print it out, as we will try to print out a null string.

    jobject exc = env->NewObject(excClass, excInit, (jint)rc, javaString);

    if (exc == 0)
    {
        // The Java VM threw an exception, so just return
        return 1;
    }

    env->Throw((jthrowable)exc);

    return 0;
}


jobject createResultObject(JNIEnv *env, unsigned int rc, const char *utfString,
                           const unsigned int resultLength)
{
    jclass resultClass = env->FindClass(sSTAFResultClass);

    if (resultClass == 0)
    {
        // The Java VM threw an exception, so just return
        return 0;
    }

    jmethodID resultInit = env->GetMethodID(resultClass, sJavaConstructorMethod,
                                            sSTAFResultConstructorMethodSig);
    if (resultInit == 0)
    {
        // The Java VM threw an exception, so just return
        return 0;
    }

    jstring javaString = 0;

    if (utfString != 0)
    {
        if (resultLength == 0)
        {
            javaString = env->NewStringUTF(utfString);
        }
        else
        {
            // Convert the result string for Java to handle null chars
            STAFString newResult = STAFString(utfString, resultLength,
                                              STAFString::kUTF8);
            newResult = newResult.replace(kUTF8_NULL, kUTF8_NULL2);
            newResult += kUTF8_NULL;

            javaString = env->NewStringUTF(const_cast<char *>
                         (newResult.buffer()));
        }
    }
    else
    {
        javaString = env->NewStringUTF("\00");
    }

    if (javaString == 0)
    {
        // The Java VM threw an exception, so just return
        return 0;
    }

    jobject result = env->NewObject(resultClass, resultInit, (jint)rc,
                                    javaString);

    env->DeleteLocalRef(javaString);

    return result;
}
