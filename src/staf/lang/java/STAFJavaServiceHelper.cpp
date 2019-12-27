/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <jni.h>
#include <vector>
#include <map>
#include "STAFJavaService.h"
#include "com_ibm_staf_service_STAFServiceHelper.h"
#include "STAFConnectionProvider.h"
#include "STAFEventSem.h"
#include "STAFServiceInterface.h"
#include "STAFUtil.h"
#include "STAFTrace.h"

struct JavaServiceData
{
    STAFString fName;
    jmethodID fInitMethodID;
    jmethodID fAcceptRequestMethodID;
    jmethodID fTermMethodID;
    jobject fServiceObject;
    unsigned int fInterfaceLevel;
};

// Function prototypes

STAFRC_t HandleRequest(const STAFConnectionProvider *provider,
                       STAFConnectionPtr &connection);
void HandleServiceConstruct(STAFConnectionPtr &connection);
void HandleServiceInit(STAFConnectionPtr &connection);
void HandleServiceRequest(STAFConnectionPtr &connection);
void HandleServiceTerm(STAFConnectionPtr &connection);

#if defined(STAF_OS_NAME_HPUX) && defined(STAF_INVOKE_JNI_INITIALIZE)
extern "C"
{
    void _main();
}
#endif

// Global variables

static jobject gHelperObj;
static JavaVM *gJVM;
// XXX: static bool gExitJVM = false;
static STAFEventSem gExitJVMSem;
static STAFString sIPCName("IPCNAME");

// These variables reference strings which will be passed into the JVM.  See
// additional comments in the initJVMStrings() function below.

static const char *sJavaStringFieldType             = 0;
static const char *sJavaIntFieldType                = 0;
static const char *sJavaConstructorMethod           = 0;
static const char *sSTAFResultClass                 = 0;
static const char *sSTAFResultRCField               = 0;
static const char *sSTAFResultResultField           = 0;
static const char *sServiceInitClass                = 0;
static const char *sServiceInitConstructorMethodSig = 0;
static const char *sServiceRequestClass             = 0;
static const char *sServiceRequestConstructorSig    = 0;
static const char *sJVMNameField                    = 0;
static const char *sLoadServiceMethod               = 0;
static const char *sLoadServiceMethodSig            = 0;
static const char *sInitServiceMethod               = 0;
static const char *sInitServiceMethodSig            = 0;
static const char *sCallServiceMethod               = 0;
static const char *sCallServiceMethodSig            = 0;
static const char *sTermServiceMethod               = 0;
static const char *sTermServiceMethodSig            = 0;

static const STAFString *sSTAFResultClassStringPtr     = 0;
static const STAFString *sServiceInitClassStringPtr    = 0;
static const STAFString *sServiceRequestClassStringPtr = 0;

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

    static STAFString sJavaStringFieldTypeString =
        STAFString("Ljava/lang/String;") + kUTF8_NULL;
    static STAFString sJavaIntFieldTypeString = STAFString("I") + kUTF8_NULL;

    sJavaStringFieldType = sJavaStringFieldTypeString.buffer();
    sJavaIntFieldType    = sJavaIntFieldTypeString.buffer();

    // Default Java Methods

    static STAFString sJavaConstructorMethodString =
        STAFString("<init>") + kUTF8_NULL;

    sJavaConstructorMethod = sJavaConstructorMethodString.buffer();

    // STAFResult class definitions

    static STAFString sSTAFResultClassString =
        STAFString("com/ibm/staf/STAFResult") + kUTF8_NULL;
    static STAFString sSTAFResultRCFieldString =
        STAFString("rc") + kUTF8_NULL;
    static STAFString sSTAFResultResultFieldString =
        STAFString("result") + kUTF8_NULL;

    sSTAFResultClass       = sSTAFResultClassString.buffer();
    sSTAFResultRCField     = sSTAFResultRCFieldString.buffer();
    sSTAFResultResultField = sSTAFResultResultFieldString.buffer();

    sSTAFResultClassStringPtr = &sSTAFResultClassString;

    // STAFServiceHelper.ServiceInit class definitions

    static STAFString sServiceInitClassString =
        STAFString("com/ibm/staf/service/STAFServiceHelper$ServiceInit") +
        kUTF8_NULL;
    static STAFString sServiceInitConstructorMethodSigString =
        STAFString("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V") +
        kUTF8_NULL;

    sServiceInitClass = sServiceInitClassString.buffer();
    sServiceInitConstructorMethodSig =
        sServiceInitConstructorMethodSigString.buffer();

    sServiceInitClassStringPtr = &sServiceInitClassString;

    // STAFServiceHelper.ServiceRequest class definitions

    static STAFString sServiceRequestClassString   =
        STAFString("com/ibm/staf/service/STAFServiceHelper$ServiceRequest") +
        kUTF8_NULL;
    static STAFString sServiceRequestConstructorSigString =
        STAFString("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                   "Ljava/lang/String;IIZILjava/lang/String;I"
                   "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V") +
        kUTF8_NULL;

    sServiceRequestClass = sServiceRequestClassString.buffer();
    sServiceRequestConstructorSig =
        sServiceRequestConstructorSigString.buffer();

    sServiceRequestClassStringPtr = &sServiceRequestClassString;

    // STAFServiceHelper fields

    static STAFString sJVMNameFieldString = STAFString("fJVMName") + kUTF8_NULL;

    sJVMNameField = sJVMNameFieldString.buffer();

    // STAFServiceHelper methods

    static STAFString sLoadServiceMethodString =
        STAFString("loadService") + kUTF8_NULL;
    static STAFString sLoadServiceMethodSigString =
        STAFString("(Ljava/lang/String;Ljava/lang/String;"
                   "Ljava/lang/String;I)Lcom/ibm/staf/STAFResult;") +
        kUTF8_NULL;
    static STAFString sInitServiceMethodString =
        STAFString("initService") + kUTF8_NULL;
    static STAFString sInitServiceMethodSigString =
        STAFString("(Ljava/lang/String;"
                   "Lcom/ibm/staf/service/STAFServiceHelper$ServiceInit;)"
                   "Lcom/ibm/staf/STAFResult;") + kUTF8_NULL;
    static STAFString sCallServiceMethodString =
        STAFString("callService") + kUTF8_NULL;
    static STAFString sCallServiceMethodSigString =
        STAFString("(Ljava/lang/String;"
                   "Lcom/ibm/staf/service/STAFServiceHelper$"
                   "ServiceRequest;)Lcom/ibm/staf/STAFResult;") + kUTF8_NULL;
    static STAFString sTermServiceMethodString =
        STAFString("termService") + kUTF8_NULL;
    static STAFString sTermServiceMethodSigString =
        STAFString("(Ljava/lang/String;)Lcom/ibm/staf/STAFResult;") +
        kUTF8_NULL;

    sLoadServiceMethod    = sLoadServiceMethodString.buffer();
    sLoadServiceMethodSig = sLoadServiceMethodSigString.buffer();
    sInitServiceMethod    = sInitServiceMethodString.buffer();
    sInitServiceMethodSig = sInitServiceMethodSigString.buffer();
    sCallServiceMethod    = sCallServiceMethodString.buffer();
    sCallServiceMethodSig = sCallServiceMethodSigString.buffer();
    sTermServiceMethod    = sTermServiceMethodString.buffer();
    sTermServiceMethodSig = sTermServiceMethodSigString.buffer();
}
    

/*
 * Class:     com_ibm_staf_service_STAFServiceHelper
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_ibm_staf_service_STAFServiceHelper_initialize
    (JNIEnv *env, jclass)
{
   #if defined(STAF_OS_NAME_HPUX) && defined(STAF_INVOKE_JNI_INITIALIZE)
        _main();
    #endif

    initJVMStrings();
}


/*
 * Class:     com_ibm_staf_service_STAFServiceHelper
 * Method:    listen
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_ibm_staf_service_STAFServiceHelper_listen
    (JNIEnv *env, jobject theHelperObject)
{
    gHelperObj = env->NewGlobalRef(theHelperObject);
    env->GetJavaVM(&gJVM);

    // Write the PID for the JVM to the JVM log (and a separater line)

    cout << "*** JVM PID       : " << STAFUtilGetPID() << endl;
    cout << "***************************************"
         << "***************************************" << endl;

    // Get the class for STAFServiceHelper and the fJVMName field ID

    jclass helperClass = env->GetObjectClass(theHelperObject);
    jfieldID jvmNameFieldID = env->GetFieldID(helperClass, sJVMNameField,
                                              sJavaStringFieldType);
    env->DeleteLocalRef(helperClass);

    if (jvmNameFieldID == 0)
    {
        // We could not get the Field ID of STAFServiceHelper.fJVMName
        // The Java VM should already have thrown an exception, so we
        // just return

        return;
    }

    jstring jvmNameObj = reinterpret_cast<jstring>(
                         env->GetObjectField(theHelperObject, jvmNameFieldID));

    const char *jvmNameUTF = env->GetStringUTFChars(jvmNameObj, 0);

    if (jvmNameUTF == 0)
    {
      // Pass back a string with the error
    }

    STAFConnectionProviderPtr connProv;
    unsigned int connProvStarted = 0;

    try
    {
        STAFString jvmName(jvmNameUTF, strlen(jvmNameUTF), STAFString::kUTF8);

        env->ReleaseStringUTFChars(jvmNameObj, jvmNameUTF);
        env->DeleteLocalRef(jvmNameObj);

        STAFString jvmIPCName = "JSTAF_"+ jvmName;
        STAFStringConst_t optionData[] = { sIPCName.getImpl(),
                                           jvmIPCName.getImpl() };
        STAFConnectionProviderConstructInfoLevel1 constructInfo = 
        {
            kSTAFConnectionProviderInbound,
            1,
            optionData,
            &optionData[1]
        };

        connProv = STAFConnectionProvider::createRefPtr(
            jvmIPCName, "STAFLIPC", &constructInfo, 1);

        connProv->start(HandleRequest);
        connProvStarted = 1;

        gExitJVMSem.wait();

        connProv->stop();
    }
    catch (STAFException &se)
    {
        se.trace("STAFServiceHelper.listen()");

        if (connProvStarted) connProv->stop();
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in STAFServiceHelper.listen()");

        if (connProvStarted) connProv->stop();
    }

    env->DeleteGlobalRef(gHelperObj);
}

STAFRC_t HandleRequest(const STAFConnectionProvider *provider,
                       STAFConnectionPtr &connection)
{
    unsigned int doShutdown = 0;

    try
    {
        unsigned int reqType = connection->readUInt();

        switch (reqType)
        {
            case JAVA_SERVICE_JVMPING:
            {
                connection->writeUInt(kSTAFOk);
                connection->writeString(STAFString());
                break;
            }
            case JAVA_SERVICE_LOAD:
            {
                HandleServiceConstruct(connection);
                break;
            }
            case JAVA_SERVICE_INIT:
            {
                HandleServiceInit(connection);
                break;
            }
            case JAVA_SERVICE_ACCEPT_REQUEST:
            {
                HandleServiceRequest(connection);
                break;
            }
            case JAVA_SERVICE_TERM:
            {
                HandleServiceTerm(connection);
                break;
            }
            case JAVA_SERVICE_JVMEXIT:
            {
                gExitJVMSem.post();

                connection->writeUInt(kSTAFOk);
                connection->writeString(STAFString());

                break;
            }
            default:
            {
                connection->writeUInt(kSTAFInvalidValue);
                connection->writeString(STAFString("Invalid Java service ") +
                                        "request type: " + STAFString(reqType));
                break;
            }
        }
    }
    catch (STAFException &se)
    {
        se.trace("JSTAFSH.HandleRequest()");
    }
    catch (std::bad_alloc)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "ERROR: Ran out of memory in JSTAFSH.HandleRequest()");
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in JSTAFSH.HandleRequest()");
    }

    return 0;
}


void HandleServiceConstruct(STAFConnectionPtr &connection)
{
    STAFString serviceName   = connection->readString();
    STAFString execInfo      = connection->readString();
    STAFString writeLocation = connection->readString();

    STAFServiceType_t serviceType =
        static_cast<STAFServiceType_t>(connection->readUInt());

    // Attach the current thread

    JNIEnv *env = 0;

    jint attachRC = gJVM->AttachCurrentThread(reinterpret_cast<void **>(&env),
                                              NULL);

    if (attachRC != 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error attaching Java VM thread, RC: " + STAFString(attachRC));
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error attaching Java VM thread, RC: " +
                                STAFString(attachRC) + " in constructService "
                                "for " + serviceName);
        return;
    }

    // Call JVM to load the class

    jclass helperClass = env->GetObjectClass(gHelperObj);

    jmethodID loadService = env->GetMethodID(helperClass, sLoadServiceMethod,
                                             sLoadServiceMethodSig);
    if (loadService == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error getting STAFServiceHelper.loadService() method ID");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting "
                                "STAFServiceHelper.loadService() method ID "
                                "in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Convert the execInfo string to UTF-8 for Java

    execInfo      += kUTF8_NULL;
    serviceName   += kUTF8_NULL;
    writeLocation += kUTF8_NULL;

    jstring javaExecInfo      = env->NewStringUTF(execInfo.buffer());

    if (javaExecInfo == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error converting execInfo string to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting execInfo string to UTF-8 "
                                "in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jstring javaServiceName   = env->NewStringUTF(serviceName.buffer());

    if (javaServiceName == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error converting service name string to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting service name to UTF-8 "
                                "in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jstring javaWriteLocation = env->NewStringUTF(writeLocation.buffer());
    jint    javaServiceType   = serviceType;

    if (javaWriteLocation == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error converting writeLocation string to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting writeLocation to UTF-8 "
                                "in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Call the loadService() method
    
    jobject javaResult = env->CallObjectMethod(gHelperObj, loadService,
                                               javaServiceName, javaExecInfo,
                                               javaWriteLocation,
                                               // XXX: Pass here?
                                               javaServiceType);
    if (javaResult == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error calling the loadService method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error calling the loadService method in "
                                "constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Get the field IDs of the STAFResult class

    jclass resultClass = env->FindClass(sSTAFResultClass);

    if (resultClass == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error finding Java class " + *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error finding Java class " +
                                *sSTAFResultClassStringPtr +
                                " in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultRCID = env->GetFieldID(resultClass, sSTAFResultRCField,
                                          sJavaIntFieldType);

    if (resultRCID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error getting rc field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting rc field of Java class " +
                                *sSTAFResultClassStringPtr +
                                " in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultStringID = env->GetFieldID(resultClass,
                                              sSTAFResultResultField,
                                              sJavaStringFieldType);
    if (resultStringID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error getting result field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting result field of Java class " +
                                *sSTAFResultClassStringPtr +
                                " in constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Get the return code and result string from the STAFResult object

    jint rc = env->GetIntField(javaResult, resultRCID);
    jstring javaResultString = (jstring)env->GetObjectField(javaResult,
                                                            resultStringID);
    const char *utfResultString =
               env->GetStringUTFChars(javaResultString, 0);

    if (utfResultString == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error constructing service " + serviceName +
            ": Error getting UTF-8 result string");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting UTF-8 result string in "
                                "constructService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    STAFString resultString(utfResultString,
                            env->GetStringUTFLength(javaResultString),
                            STAFString::kUTF8);

    env->ReleaseStringUTFChars(javaResultString, utfResultString);

    connection->writeUInt(rc);
    connection->writeString(resultString);

    // Detach the current thread

    gJVM->DetachCurrentThread();
}


void HandleServiceInit(STAFConnectionPtr &connection)
{
    STAFString serviceName = connection->readString();
    STAFString parms = connection->readString();
    STAFString writeLocation = connection->readString();

    // Attach the current thread

    JNIEnv *env = 0;

    jint attachRC = gJVM->AttachCurrentThread(reinterpret_cast<void **>(&env),
                                              NULL);

    if (attachRC != 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
            ": Error attaching Java VM thread, RC: " + STAFString(attachRC));
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error attaching Java VM thread, RC: " +
                                STAFString(attachRC) + " in initService "
                                "for " + serviceName);
        return;
    }

    // Call JVM to load the class

    jclass helperClass = env->GetObjectClass(gHelperObj);

    jmethodID initService = env->GetMethodID(helperClass, sInitServiceMethod,
                                             sInitServiceMethodSig);
    if (initService == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
            ": Error getting STAFServiceHelper.initService() method ID");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting "
                                "STAFServiceHelper.initService() method ID");
        gJVM->DetachCurrentThread();
        return;
    }

    // Convert the service name, parms, and writeLocation strings to UTF-8
    // for Java

    serviceName += kUTF8_NULL;
    parms = parms.replace(kUTF8_NULL, kUTF8_NULL2);
    parms += kUTF8_NULL;
    writeLocation += kUTF8_NULL;

    jstring javaServiceName = env->NewStringUTF(serviceName.buffer());

    if (javaServiceName == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
            ": Error converting service name to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting service name " +
                                serviceName + " to UTF-8 in initService");
        gJVM->DetachCurrentThread();
        return;
    }

    jstring javaParms = env->NewStringUTF(parms.buffer());

    if (javaParms == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
            ": Error converting Java parameters to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting Java parameters for " +
                                serviceName + " to UTF-8 in initService");
        gJVM->DetachCurrentThread();
        return;
    }

    jstring javaWriteLocation = env->NewStringUTF(writeLocation.buffer());

    if (javaWriteLocation == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
            ": Error converting writeLocation string to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting writeLocation to UTF-8 "
                                "in initService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }
    
    // Load and construct the ServiceInit class

    jclass serviceInitClass = env->FindClass(sServiceInitClass);

    if (serviceInitClass == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error finding Java class " + *sServiceInitClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error finding Java class: " +
                                *sServiceInitClassStringPtr + " for "
                                + serviceName + " in initService");
        gJVM->DetachCurrentThread();
        return;
    }

    jmethodID constructor = env->GetMethodID(serviceInitClass,
                                             sJavaConstructorMethod,
                                             sServiceInitConstructorMethodSig);
    if (constructor == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error loading constructor for Java class " +
            *sServiceInitClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error loading constructor for Java class " +
                                *sServiceInitClassStringPtr + " for " +
                                serviceName);
        gJVM->DetachCurrentThread();
        return;
    }
        
    jobject serviceInitObject = env->NewObject(serviceInitClass, constructor,
                                               javaServiceName, javaParms,
                                               javaWriteLocation);

    if (serviceInitObject == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error creating Java object of type " +
            *sServiceInitClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error creating Java object of type " +
                                *sServiceInitClassStringPtr + " for " +
                                serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Call the initService() method

    jobject javaResult = env->CallObjectMethod(gHelperObj, initService,
                                               javaServiceName,
                                               serviceInitObject);
    if (javaResult == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error calling the initService method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error calling the initService method for " +
                                serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Get the field IDs of the STAFResult class

    jclass resultClass = env->FindClass(sSTAFResultClass);

    if (resultClass == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error finding Java class " + *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error finding Java class " +
                                *sSTAFResultClassStringPtr +
                                " in initService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultRCID = env->GetFieldID(resultClass, sSTAFResultRCField,
                                          sJavaIntFieldType);

    if (resultRCID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error getting rc field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting rc field of Java class " +
                                *sSTAFResultClassStringPtr +
                                " in initService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultStringID = env->GetFieldID(resultClass,
                                              sSTAFResultResultField,
                                              sJavaStringFieldType);
    if (resultStringID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error getting result field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting result field of Java class " +
                                *sSTAFResultClassStringPtr +
                                " in initService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Get the return code and result string from the STAFResult object

    jint rc = env->GetIntField(javaResult, resultRCID);
    jstring javaResultString = (jstring)env->GetObjectField(javaResult,
                                                            resultStringID);
    const char *utfResultString =
               env->GetStringUTFChars(javaResultString, 0);

    if (utfResultString == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Error initializing service " + serviceName +
             ": Error getting UTF-8 result string");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting UTF-8 result string in "
                                "initService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    STAFString resultString(utfResultString,
                            env->GetStringUTFLength(javaResultString),
                            STAFString::kUTF8);

    env->ReleaseStringUTFChars(javaResultString, utfResultString);

    connection->writeUInt(rc);
    connection->writeString(resultString);

    // Detach the current thread

    gJVM->DetachCurrentThread();
}


void HandleServiceRequest(STAFConnectionPtr &connection)
{
    // Note: The request type has already been read
    unsigned int totalLength = connection->readUInt();
    STAFBuffer<char> buffer(new char[totalLength], STAFBuffer<char>::INIT,
                            STAFBuffer<char>::ARRAY);
    unsigned int *uintBuffer = reinterpret_cast<unsigned int *>((char *)buffer);

    // IMPORTANT:  Increase the numFields value if add a field to the
    //             ServiceRequest class for a new STAFServiceInterfaceLevel.
    //             Note:  This value is 2 less than the numFields value in
    //                    STAFJavaService.cpp.

    unsigned int numFields = 14;  // # of fields in buffer

    // uintBuffer[0] = Service name length
    // uintBuffer[1] = Handle
    // uintBuffer[2] = Trust level
    // uintBuffer[3] = Machine length
    // uintBuffer[4] = Machine nickname length
    // uintBuffer[5] = Handle name length
    // uintBuffer[6] = Request length
    // uintBuffer[7] = Diagnostics Enabled Flag
    // uintBuffer[8] = Request Number
    // uintBuffer[9] = User length
    // uintBuffer[10] = Endpoint length
    // uintBuffer[11] = STAF Instance UUID length
    // uintBuffer[12] = Is Local Request Flag
    // uintBuffer[13] = Physical Identifier length

    connection->read(buffer, totalLength);
    
    char *serviceNameBuffer = buffer + (numFields * sizeof(unsigned int));
    char *machineBuffer = serviceNameBuffer + uintBuffer[0];
    char *machineNicknameBuffer = machineBuffer +  uintBuffer[3];
    char *handleNameBuffer = machineNicknameBuffer + uintBuffer[4];
    char *requestBuffer = handleNameBuffer + uintBuffer[5];
    char *userBuffer = requestBuffer + uintBuffer[6];
    char *endpointBuffer = userBuffer + uintBuffer[9];
    char *stafInstanceUUIDBuffer = endpointBuffer + uintBuffer[10];
    char *physicalInterfaceIDBuffer = stafInstanceUUIDBuffer + uintBuffer[11];

    if (serviceNameBuffer == 0 || machineBuffer == 0 ||
        machineNicknameBuffer == 0 || handleNameBuffer == 0 ||
        requestBuffer == 0 || userBuffer == 0 ||
        endpointBuffer == 0 || stafInstanceUUIDBuffer == 0 ||
        physicalInterfaceIDBuffer == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Memory allocation failure in "
            "JSTAFSH.HandleServiceRequest()");
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Memory allocation failure in "
                                "JSTAFSH.HandleServiceRequest");
        return;
    }

    STAFString serviceName(serviceNameBuffer, uintBuffer[0], STAFString::kUTF8);
    unsigned int handle = uintBuffer[1];
    unsigned int trustLevel = uintBuffer[2];
    STAFString machine(machineBuffer, uintBuffer[3], STAFString::kUTF8);
    STAFString machineNickname(machineNicknameBuffer, uintBuffer[4],
                               STAFString::kUTF8);
    STAFString handleName(handleNameBuffer, uintBuffer[5], STAFString::kUTF8);
    STAFString request(requestBuffer, uintBuffer[6], STAFString::kUTF8);
    unsigned int diagEnabled = uintBuffer[7];    
    unsigned int requestNumber = uintBuffer[8];
    STAFString user(userBuffer, uintBuffer[9], STAFString::kUTF8);
    STAFString endpoint(endpointBuffer, uintBuffer[10], STAFString::kUTF8);
    STAFString stafInstanceUUID(stafInstanceUUIDBuffer, uintBuffer[11],
                                STAFString::kUTF8);
    unsigned int isLocalRequest = uintBuffer[12];
    STAFString physicalInterfaceID(physicalInterfaceIDBuffer, uintBuffer[13],
                                   STAFString::kUTF8);
    
    /* Debug:
    cout << "Total length         : " << totalLength << endl
         << "Service name         : " << serviceName << endl
         << "Machine              : " << machine << endl
         << "Machine nickname     : " << machineNickname << endl
         << "Handle name length   : " << uintBuffer[5] << endl
         << "Handle name          : " << handleName << endl
         << "Handle               : " << handle << endl
         << "Trust level          : " << trustLevel << endl
         << "Is Local Request     : " << isLocalRequest << endl
         << "Request length       : " << uintBuffer[6] << endl
         << "Request              : " << request << endl
         << "Diag enabled         : " << diagEnabled << endl
         << "User                 : " << user << endl
         << "Endpoint             : " << endpoint << endl
         << "STAF Instance UUID   : " << stafInstanceUUID << endl
         << "Physical Identifier  : " << physicalInterfaceID << endl;

    cout << endl << "Buffer: " << endl;
    
    for (unsigned int i = 0; i < totalLength; ++i)
    {
        if ((i != 0) && (i % 8 == 0)) cout << endl;
        unsigned int currChar = buffer[i];
        if (currChar < 16) cout << "0";
        cout << hex << currChar << dec << ":" << (char)currChar << " ";
    }

    cout << endl;
    */

    // Attach the current thread

    JNIEnv *env = 0;

    jint attachRC = gJVM->AttachCurrentThread(reinterpret_cast<void **>(&env),
                                              NULL);

    if (attachRC != 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error attaching Java VM thread, RC: " + STAFString(attachRC));
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error attaching Java VM thread, RC: " +
                                STAFString(attachRC));
        return;
    }

    // Call JVM to load the class

    jclass helperClass = env->GetObjectClass(gHelperObj);

    jmethodID callService = env->GetMethodID(helperClass, sCallServiceMethod,
                                             sCallServiceMethodSig);

    if (callService == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error getting STFServiceHelper.callService() method ID");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting "
                                "STAFServiceHelper.callService() method ID");
        gJVM->DetachCurrentThread();
        return;
    }

    env->DeleteLocalRef(helperClass);

    // Convert the various strings to UTF-8 for Java

    serviceName += kUTF8_NULL;
    machine += kUTF8_NULL;
    machineNickname += kUTF8_NULL;
    handleName = handleName.replace(kUTF8_NULL, kUTF8_NULL2);
    handleName += kUTF8_NULL;
    request = request.replace(kUTF8_NULL, kUTF8_NULL2);
    request += kUTF8_NULL;
    user += kUTF8_NULL;
    endpoint += kUTF8_NULL;
    stafInstanceUUID += kUTF8_NULL;
    physicalInterfaceID += kUTF8_NULL;

    jstring javaServiceName = env->NewStringUTF(serviceName.buffer());
    jstring javaMachine = env->NewStringUTF(machine.buffer());
    jstring javaMachineNickname = env->NewStringUTF(machineNickname.buffer());
    jstring javaHandleName = env->NewStringUTF(handleName.buffer());
    jstring javaRequest = env->NewStringUTF(request.buffer());
    jstring javaUser = env->NewStringUTF(user.buffer());
    jstring javaEndpoint = env->NewStringUTF(endpoint.buffer());
    jstring javaSTAFInstanceUUID = env->NewStringUTF(stafInstanceUUID.buffer());
    jstring javaPhysicalInterfaceID = env->NewStringUTF(physicalInterfaceID.buffer());

    // Load and construct the ServiceRequest class

    jclass serviceRequestClass = env->FindClass(sServiceRequestClass);

    if (serviceRequestClass == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error finding Java class: " + *sServiceRequestClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error finding Java class: " +
                                *sServiceRequestClassStringPtr);
        gJVM->DetachCurrentThread();
        return;
    }

    jmethodID constructor = env->GetMethodID(serviceRequestClass,
                                             sJavaConstructorMethod,
                                             sServiceRequestConstructorSig);
    if (constructor == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error loading constructor for Java class: " +
            *sServiceRequestClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error loading constructor for Java class " +
                                *sServiceRequestClassStringPtr);
        gJVM->DetachCurrentThread();
        return;
    }

    jobject serviceRequestObject =
        env->NewObject(serviceRequestClass, constructor,
                       javaSTAFInstanceUUID, javaMachine, javaMachineNickname,
                       javaHandleName,
                       (jint)handle, (jint)trustLevel,
                       (jboolean)isLocalRequest, (jint)diagEnabled,
                       javaRequest, (jint)requestNumber,
                       javaUser, javaEndpoint, javaPhysicalInterfaceID);

    if (serviceRequestObject == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error creating Java object of type " +
            *sServiceRequestClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error creating Java object of type " +
                                *sServiceRequestClassStringPtr);
        gJVM->DetachCurrentThread();
        return;
    }

    // Call the callService() method

    jobject javaResult = env->CallObjectMethod(gHelperObj, callService,
                                               javaServiceName,
                                               serviceRequestObject);
    if (javaResult == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error calling callService method for ServiceRequest object");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error calling callService method "
                                "for ServiceRequest object");
        gJVM->DetachCurrentThread();
        return;
    }

    env->DeleteLocalRef(serviceRequestClass);
    env->DeleteLocalRef(javaSTAFInstanceUUID);
    env->DeleteLocalRef(javaMachine);
    env->DeleteLocalRef(javaMachineNickname);
    env->DeleteLocalRef(javaHandleName);
    env->DeleteLocalRef(javaRequest);
    env->DeleteLocalRef(javaUser);
    env->DeleteLocalRef(javaEndpoint);
    env->DeleteLocalRef(javaPhysicalInterfaceID);
    env->DeleteLocalRef(serviceRequestObject);
    env->DeleteLocalRef(javaServiceName);

    // Get the field IDs of the STAFResult class

    jclass resultClass = env->FindClass(sSTAFResultClass);

    if (resultClass == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error finding Java class: " + *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error finding Java class " +
                                *sSTAFResultClassStringPtr);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultRCID = env->GetFieldID(resultClass, sSTAFResultRCField,
                                          sJavaIntFieldType);

    if (resultRCID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error getting rc field of Java class: " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting rc field of Java class " +
                                *sSTAFResultClassStringPtr);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultStringID = env->GetFieldID(resultClass,
                                              sSTAFResultResultField,
                                              sJavaStringFieldType);
    if (resultStringID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Service: " + serviceName +
            ", Request: " + request +
            ", Error getting result field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting result field of Java class " +
                                *sSTAFResultClassStringPtr);
        gJVM->DetachCurrentThread();
        return;
    }

    env->DeleteLocalRef(resultClass);
    
    // Get the return code and result string from the STAFResult object

    int rc = env->GetIntField(javaResult, resultRCID);

    try
    {
        jstring javaResultString = (jstring)env->GetObjectField(
            javaResult, resultStringID);
        env->DeleteLocalRef(javaResult);
        const char *utfResultString = env->GetStringUTFChars(
            javaResultString, 0);

        if (utfResultString == 0)
        {
            STAFTrace::trace(
                kSTAFTraceError, "Service: " + serviceName +
                ", Request: " + request +
                ", Error getting result string from the STAFResult object");
            env->ExceptionDescribe();
            env->ExceptionClear();
            connection->writeUInt(kSTAFJavaError);
            connection->writeString("Error getting result string from the "
                                    "STAFResult object");
            gJVM->DetachCurrentThread();
            return;
        }

        STAFString resultString(utfResultString,
                                env->GetStringUTFLength(javaResultString),
                                STAFString::kUTF8);

        env->ReleaseStringUTFChars(javaResultString, utfResultString);
        env->DeleteLocalRef(javaResultString);

        connection->writeUInt(rc);
        connection->writeString(resultString);
    }
    catch (std::bad_alloc)
    {
        STAFTrace::trace(
            kSTAFTraceError, "ERROR: Ran out of memory getting result "
            "string in JSTAFSH.HandleServiceRequest(), Service: " +
            serviceName + ", Request: " + request);

        connection->writeUInt(kSTAFJavaError);
        connection->writeString("ERROR: Ran out of memory getting result "
                                "string in JSTAFSH.HandleServiceRequest()");
    }

    // Detach the current thread

    gJVM->DetachCurrentThread();
}


void HandleServiceTerm(STAFConnectionPtr &connection)
{
    STAFString serviceName = connection->readString();

    // Attach the current thread

    JNIEnv *env = 0;

    jint attachRC = gJVM->AttachCurrentThread(reinterpret_cast<void **>(&env),
                                              NULL);

    if (attachRC != 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error attaching Java VM thread, RC: " + STAFString(attachRC));
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error attaching Java VM thread, RC: " +
                                STAFString(attachRC) + " during termService "
                                " for " + serviceName);
        return;
    }

    jclass helperClass = env->GetObjectClass(gHelperObj);

    jmethodID termService = env->GetMethodID(helperClass, sTermServiceMethod,
                                             sTermServiceMethodSig);
    if (termService == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error getting STAFServiceHelper.termService() method ID");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting "
                                "STAFServiceHelper.termService() method ID "
                                "for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Convert the service name to UTF-8 for Java

    serviceName += kUTF8_NULL;

    jstring javaServiceName = env->NewStringUTF(serviceName.buffer());

    if (javaServiceName == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error converting service name to UTF-8");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error converting service name to UTF-8 "
                                "during termService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Call the termService() method

    jobject javaResult = env->CallObjectMethod(gHelperObj, termService,
                                               javaServiceName);
    if (javaResult == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error calling termService method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error calling termService method for " +
                                serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Get the field IDs of the STAFResult class

    jclass resultClass = env->FindClass(sSTAFResultClass);

    if (resultClass == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error finding Java class " + *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error finding load Java class " +
                                *sSTAFResultClassStringPtr +
                                " during termService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultRCID = env->GetFieldID(resultClass, sSTAFResultRCField,
                                          sJavaIntFieldType);

    if (resultRCID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error getting rc field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting rc field of Java class " +
                                *sSTAFResultClassStringPtr +
                                " during termService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    jfieldID resultStringID = env->GetFieldID(resultClass,
                                              sSTAFResultResultField,
                                              sJavaStringFieldType);
    if (resultStringID == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error getting result field of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting result field of Java class " +
                                *sSTAFResultClassStringPtr +
                                " during termService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    // Get the return code and result string from the STAFResult object

    jint rc = env->GetIntField(javaResult, resultRCID);
    jstring javaResultString = (jstring)env->GetObjectField(javaResult,
                                                            resultStringID);
    const char *utfResultString =
               env->GetStringUTFChars(javaResultString, 0);

    if (utfResultString == 0)
    {
        STAFTrace::trace(
            kSTAFTraceError, "Terminating service: " + serviceName +
            ", Error getting result string of Java class " +
            *sSTAFResultClassStringPtr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        connection->writeUInt(kSTAFJavaError);
        connection->writeString("Error getting result string of Java class " +
                                *sSTAFResultClassStringPtr +
                                " during termService for " + serviceName);
        gJVM->DetachCurrentThread();
        return;
    }

    STAFString resultString(utfResultString,
                            env->GetStringUTFLength(javaResultString),
                            STAFString::kUTF8);

    env->ReleaseStringUTFChars(javaResultString, utfResultString);

    connection->writeUInt(rc);
    connection->writeString(resultString);

    // Detach the current thread

    gJVM->DetachCurrentThread();
}
