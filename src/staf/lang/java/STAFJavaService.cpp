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
#include "STAFServiceInterface.h"
#include "STAFJavaService.h"
#include "STAFConnectionProvider.h"
#include "STAFUtil.h"
#include "STAFThread.h"
#include "STAFEventSem.h"
#include "STAFMutexSem.h"
#include "STAFProcess.h"
#include "STAFFileSystem.h"
#include "STAFTrace.h"
#include "STAF_fstream.h"

struct JVMData
{
    STAFString fName;
    STAFString fExec;
    STAFString fOptions;
    STAFConnectionProviderPtr fConnProv;
    STAFEventSemPtr fJVMExitedSem;
    STAFProcessID_t fJVM_PID;
    unsigned int fNumServices;
};

typedef STAFRefPtr<JVMData> JVMDataPtr;
typedef std::map<STAFString, JVMDataPtr> JVMDataMap;

struct JVMStartThreadData
{
    JVMStartThreadData (STAFString_t startString, STAFEventSemPtr &exitedSem) :
        fStartString(startString), fJVMExitedSem(exitedSem)
    { /* Do nothing */ }

    STAFString_t fStartString;
    STAFEventSemPtr fJVMExitedSem;
};

struct STAFProcJavaServiceData
{
    STAFString fName;
    STAFString fExec;
    JVMDataPtr fJVM;
};

static JVMDataMap sJVMDataMap;
static STAFMutexSem sJVMDataSem;
static STAFString sLocal("local");
static STAFString sIPCName("IPCNAME");

STAFRC_t STAFServiceGetLevelBounds(unsigned int levelID,
                                   unsigned int *minimum,
                                   unsigned int *maximum)
{
    switch (levelID)
    {
        case kServiceInfo:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceInit:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceAcceptRequest:
        {
            *minimum = 30;
            *maximum = 30;
            break;
        }
        case kServiceTerm:
        case kServiceDestruct:
        {
            *minimum = 0;
            *maximum = 0;
            break;
        }
        default:
        {
            return kSTAFInvalidAPILevel;
        }
    }

    return kSTAFOk;
}


unsigned int STAFDoShutdownJVM(STAFConnectionProviderPtr &connProv)
{
    try
    {
        STAFConnectionPtr connPtr = connProv->connect(sLocal);

        connPtr->writeUInt(JAVA_SERVICE_JVMEXIT);

        STAFRC_t jvmRC = connPtr->readUInt();
        STAFString jvmResultString = connPtr->readString();

        // Note, this last connect is required depending on the thread timing
        // in STAFJavaServiceHelper.cpp.   In that file, if the request
        // thread gets started before the creating thread gets control back,
        // then this call is unnecessary.  Otherwise, we need to send this
        // final request so that the main thread will be able to check the
        // exit flag.

        connPtr = connProv->connect(sLocal);

        connPtr->writeUInt(JAVA_SERVICE_JVMFIN);
    }
    catch (STAFConnectionProviderException)
    {
        // These exceptions are anticipated in cases where the JVM has
        // already shutdown before we send the shutdown commands
    }

    return 0;
}


// Note: You should already own sJVMDataSem before calling STAFShutdownJMV

unsigned int STAFShutdownJVM(STAFString &jvmName)
{
    try
    {
        JVMDataPtr jvm = sJVMDataMap[jvmName];

        sJVMDataMap.erase(jvmName);

        STAFDoShutdownJVM(jvm->fConnProv);
    }
    catch (STAFException &e)
    {
        e.trace((STAFString("JSTAF.STAFShutdownJVM(), JVMName: ") + jvmName).
                toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in JSTAF.STAFShutdownJVM(), JVMName: " +
            jvmName);
    }

    return 0;
}


STAFRC_t STAFServiceConstruct(STAFServiceHandle_t *pServiceHandle,
                              void *pServiceInfo, unsigned int infoLevel,
                              STAFString_t *pErrorBuffer)
{
    if (infoLevel != 30) return kSTAFInvalidAPILevel;

    STAFProcJavaServiceData data;
    STAFServiceInfoLevel30 *pInfo =
        static_cast<STAFServiceInfoLevel30 *>(pServiceInfo);
    unsigned int serviceLoaded = 0;
    STAFString jvmName = "STAFJVM1";

    try
    {
        STAFString jvmExec = "java";
        STAFString jvmOptions = " ";
        bool jvmSpecified = false;
        bool j2Specified = false;
        unsigned int maxLogs = 5;    // Defaults to keeping 5 JVM logs
        unsigned int maxLogSize = 1048576; // Default size of each log is 1M

        data.fName = pInfo->name;
        data.fExec = pInfo->exec;
        
        // Verify that a executable was specified

        if (data.fExec.length() == 0)
        {
            STAFString errmsg = STAFString("When registering Java service '") +
                data.fName + "', you must specify an EXECUTE option that "
                "contains the fully-qualified name of the jar file that "
                "implements the Java service.";
            *pErrorBuffer = errmsg.adoptImpl();
            return kSTAFServiceConfigurationError;
        }

        // Walk through and verify the config options

        for (unsigned int i = 0; i < pInfo->numOptions; ++i)
        {
            STAFString upperOptionName =
                       STAFString(pInfo->pOptionName[i]).upperCase();
            STAFString optionValue(pInfo->pOptionValue[i]);

            if (upperOptionName == "JVMNAME")
            {
                jvmName = optionValue;
            }
            else if (upperOptionName == "JVM")
            {
                jvmExec = optionValue;
                jvmSpecified = true;
            }
            else if (upperOptionName == "J2")
            {
                jvmOptions += optionValue + " ";
                j2Specified = true;
            }
            else if (upperOptionName == "MAXLOGS")
            {
                // Convert the MAXLOGS option value to an unsigned integer
                // in range 1 to UINT_MAX

                STAFString_t errorBufferT = 0;

                STAFRC_t rc = STAFUtilConvertStringToUInt(
                    optionValue.getImpl(), upperOptionName.getImpl(),
                    &maxLogs, &errorBufferT, 1, UINT_MAX);

                if (rc != kSTAFOk)
                {
                    STAFString errmsg(
                        "Error constructing the JVM using JVMName: " + jvmName +
                        ", JVM: " + jvmExec + ", JVMOptions: " + jvmOptions +
                        ", RC: " + STAFString(kSTAFInvalidValue) +
                        ", Reason: " +
                        STAFString(errorBufferT, STAFString::kShallow));
                    *pErrorBuffer = errmsg.adoptImpl();
                    return kSTAFServiceConfigurationError;
                }
            }
            else if (upperOptionName == "MAXLOGSIZE")
            {
                // Convert the MAXLOGSIZE option value to an unsigned integer
                // in range 1 to UINT_MAX

                STAFString_t errorBufferT = 0;

                STAFRC_t rc = STAFUtilConvertStringToUInt(
                    optionValue.getImpl(), upperOptionName.getImpl(),
                    &maxLogSize, &errorBufferT, 1, UINT_MAX);

                if (rc != kSTAFOk)
                {
                    STAFString errmsg(
                        "Error constructing the JVM using JVMName: " + jvmName +
                        ", JVM: " + jvmExec + ", JVMOptions: " + jvmOptions +
                        ", RC: " + STAFString(kSTAFInvalidValue) +
                        ", Reason: " +
                        STAFString(errorBufferT, STAFString::kShallow));
                    *pErrorBuffer = errmsg.adoptImpl();
                    return kSTAFServiceConfigurationError;
                }
            }
            else
            {
                STAFString errmsg(
                    "Error constructing the JVM using JVMName: " + jvmName +
                    ", JVM: " + jvmExec + ", JVMOptions: " + jvmOptions +
                    ", RC: " + STAFString(kSTAFInvalidValue) +
                    ", Reason: Option " + STAFString(pInfo->pOptionName[i]) +
                    " is not a valid option for JSTAF");

                *pErrorBuffer = errmsg.adoptImpl();
                return kSTAFServiceConfigurationError;
            }
        }

        // Check to see if the specified JVM already exists

        STAFMutexSemLock lock(sJVMDataSem);

        if (sJVMDataMap.find(jvmName) == sJVMDataMap.end())
        {
            // The JVM doesn't exist so we need to start it

            JVMData jvmData;

            jvmData.fName = jvmName;
            jvmData.fExec = jvmExec;
            jvmData.fOptions = jvmOptions;
            jvmData.fJVMExitedSem = STAFEventSemPtr(new STAFEventSem,
                                                    STAFEventSemPtr::INIT);
            jvmData.fNumServices = 0;

            STAFString jvmStartString = jvmOptions +
                       "com.ibm.staf.service.STAFServiceHelper " + jvmName;

            // Create the connection provider for the JVM

            STAFString ipcName = "JSTAF_" + jvmName;
            STAFStringConst_t optionData[] = { sIPCName.getImpl(),
                                               ipcName.getImpl() };
            STAFConnectionProviderConstructInfoLevel1 constructInfo = 
            {
                kSTAFConnectionProviderOutbound,
                1,
                optionData,
                &optionData[1]
            };

            jvmData.fConnProv =
                STAFConnectionProvider::createRefPtr(ipcName, "STAFLIPC",
                                                     &constructInfo, 1);

            // We need to shutdown any JVM that might happen to be leftover
            // from a "bad" exit from a previous STAFProc

            STAFDoShutdownJVM(jvmData.fConnProv);
            
            // We need to capture stdout/stderr for diagnostic purposes
            // Create directory for JVM log file if doesn't already exist

            STAFFSPath logPath;
            logPath.setRoot(pInfo->writeLocation);
            logPath.addDir("lang");
            logPath.addDir("java");
            logPath.addDir("jvm");
            logPath.addDir(jvmName);

            if (!logPath.exists())
            {
                try
                {
                    // Don't want exceptions here
                    STAFFSEntryPtr dir = 
                        logPath.createDirectory(kSTAFFSCreatePath);
                }
                catch (...)
                { /* Do Nothing */ }

                if (!logPath.exists())
                {
                    STAFString errmsg(
                        "Error constructing the JVM using JVMName: " + jvmName +
                        ", JVM: " + jvmExec + ", JVMOptions: " + jvmOptions +
                        ", Error creating JVMLog directory: " +
                        logPath.asString());
                    *pErrorBuffer = errmsg.adoptImpl();
                    return kSTAFServiceConfigurationError;
                }
            }

            // Name the current JVM log file - JVMLog.1

            logPath.setName("JVMLog");
            logPath.setExtension("1");
            STAFString logName = logPath.asString();
            
            // Instead of replacing the JVM log file each time a JVM is created,
            // use the following JVM OPTIONs to determine when to create a new
            // JVM log file and how many JVM log files to save:
            // - MaxLogs   : Maximum number of JVM log files to keep.
            // - MaxLogSize: Maximum size of a JVM log file in bytes.
            
            // Open the JVM log file
                
            fstream outfile(logName.toCurrentCodePage()->buffer(),
                            ios::out | ios::app);

            if (!outfile)
            {
                STAFString errmsg(
                    "Error constructing the JVM using JVMName: " + jvmName +
                    ", JVM: " + jvmExec + ", JVMOptions: " + jvmOptions +
                    ", RC: " + STAFString(kSTAFFileOpenError) +
                    ", Error opening file " + logName);
                *pErrorBuffer = errmsg.adoptImpl();
                return kSTAFServiceConfigurationError;
            }

            // Figure out how big the JVM log file is

            outfile.seekp(0, ios::end);
            unsigned int fileLength = (unsigned int)outfile.tellp();

            if (fileLength > maxLogSize)
            {
                // Roll any existing log files (e.g. Rename JVMLog.2.out to
                // JVMLog.3, JVMLog.1 to JVMLog.2, etc) and create a new
                // JVMLog.1 file.  If the # of existing logs > MAXLOGS, don't
                // save the oldest log.

                outfile.close();

                STAFFSPath fromLogPath(logPath);
                
                for (int i = maxLogs; i > 0; --i)
                {
                    fromLogPath.setExtension(STAFString(i));

                    if (fromLogPath.exists() && i < maxLogs)
                    {
                        // Rename JVMLog.<i> to JVMLog.<i+1>

                        STAFFSPath toLogPath(fromLogPath);
                        toLogPath.setExtension(STAFString(i + 1));

                        fromLogPath.getEntry()->move(toLogPath.asString());
                    }
                }
                
                // Open a new empty current log file

                outfile.open(logName.toCurrentCodePage()->buffer(),
                             ios::out | ios::trunc);

                if (!outfile)
                {
                    STAFString errmsg(
                        "Error constructing the JVM using JVMName: " + jvmName +
                        ", JVM: " + jvmExec + ", JVMOptions: " + jvmOptions +
                        ", RC: " + STAFString(kSTAFFileOpenError) +
                        ", Error opening file " + logName);
                    *pErrorBuffer = errmsg.adoptImpl();
                    return kSTAFServiceConfigurationError;
                }
            }

            // Write the JVM start information to the JVM log file

            STAFString separatorLine("***************************************"
                                     "***************************************");
            STAFString line1("*** " + STAFTimestamp().asString() +
                                  " - Start of Log for JVMName: " + jvmName);
            STAFString line2("*** JVM Executable: " + jvmExec);
            STAFString line3("*** JVM Options   :");

            if (jvmOptions != STAFString(" "))
                line3 += jvmOptions;
            else
                line3 += " none";

            STAFString line4("*** JVM Version   : ");

            outfile << endl << separatorLine.toCurrentCodePage()->buffer()
                    << endl << line1.toCurrentCodePage()->buffer()
                    << endl << line2.toCurrentCodePage()->buffer()
                    << endl << line3.toCurrentCodePage()->buffer()
                    << endl << line4.toCurrentCodePage()->buffer();
            outfile.close();

            // Run command:  "jvmExec" -version   to get the Java version and
            // redirect its stdout/stderr to the JVM log file.
            // Make sure the JVM executable and the JVM log name are in double
            // quotes in case they contain a space and include the entire
            // command in double quotes.
            //
            // Examples:
            //
            // "java" -version >> "C:/STAF/data/STAF/lang/java/jvm/STAFJVM1/JVMLog.1" 2>&1
            //
            // "C:/Program Files/Java/jre1.5.0_10/bin/java" -version >>
            // "C:/Program Files/STAF/data/STAF/lang/java/jvm/STAFJVM1/JVMLog.1" 2>&1

            // Only add a double quote to the JVM executable if it isn't
            // already in double quotes
            
            if (!jvmExec.startsWith("\""))
                jvmExec = STAFString("\"") + jvmExec + "\"";

            STAFString javaVersionCmd = jvmExec + " -version "
                ">> \"" + logName + "\" 2>&1";

            #ifdef STAF_OS_NAME_WIN32
                // On Windows, need to enclose the entire command in double
                // quotes
                javaVersionCmd = STAFString("\"") + javaVersionCmd + "\"";
            #endif

            system(javaVersionCmd.toCurrentCodePage()->buffer());

            // Start a process for the JVM

            STAFProcessStartInfoLevel1 startInfo = { 0 };

            startInfo.command     = jvmExec.getImpl();
            startInfo.parms       = jvmStartString.getImpl();
            startInfo.commandType = kSTAFProcessShell;
            startInfo.consoleMode = kSTAFProcessSameConsole;

            startInfo.stdoutMode  = kSTAFProcessIOAppendFile;
            startInfo.stderrMode  = kSTAFProcessIOStdout;

            startInfo.stdoutRedirect = logName.getImpl();
            
            unsigned int osRC = 0;
            STAFString_t errorBuffer = 0;
            STAFRC_t rc = STAFProcessStart2(
                &jvmData.fJVM_PID, 0, &startInfo, 1, &osRC, &errorBuffer);

            if (rc != kSTAFOk)
            {
                STAFString startError(
                    "Error starting a process for the JVM using "
                    "JVMName: " + jvmName + ", JVM: " + jvmExec +
                    ", JVMOptions: " + jvmOptions +
                    ", RC: " + STAFString(rc) + ", Result: " +
                    STAFString(errorBuffer, STAFString::kShallow));

                // Add more details to the error msg when the Java executable
                // cannot be found.

                if (rc == 10)
                {
                    if (!jvmSpecified)
                    {
                        startError = startError + "  Make sure the java " +
                            "executable is in the PATH.";
                    }
                    else
                    {
                        startError = startError + "  Make sure " + jvmExec +
                            " exists.";
                    }
                }

                *pErrorBuffer = startError.adoptImpl();
                return kSTAFServiceConfigurationError;
            }

            // Now we need to wait for it to start

            bool jvmReady = false;

            for (int i = 0; (i < 30) & !jvmReady; ++i)
            {
                // XXX: Need to check to see if the JVM is actually still running

                try
                {
                    // First connect to the JVM

                    STAFConnectionPtr connPtr =
                                      jvmData.fConnProv->connect(sLocal);

                    // Now see if it's alive

                    connPtr->writeUInt(JAVA_SERVICE_JVMPING);

                    STAFRC_t jvmRC = connPtr->readUInt();
                    STAFString jvmResultString = connPtr->readString();

                    if (jvmRC != kSTAFOk)
                    {
                        jvmResultString = "Error starting JVM: " +
                                          jvmResultString;
                        *pErrorBuffer = jvmResultString.adoptImpl();
                        return kSTAFServiceConfigurationError;
                    }

                    jvmReady = true;
                }
                catch (STAFException)
                {
                    // XXX: This should really be more specific
                }

                if (!jvmReady) STAFThreadSleepCurrentThread(1000, 0);
            }

            // If we didn't time out waiting for the JVM then we add the
            // JVM to the map of JVMs

            if (!jvmReady)
            {
                *pErrorBuffer =
                    STAFString("Unable to connect to JVM").adoptImpl();
                return kSTAFServiceConfigurationError;
            }

            sJVMDataMap[jvmName] = JVMDataPtr(new JVMData(jvmData),
                                              JVMDataPtr::INIT);
        }
        else if (jvmSpecified || j2Specified)
        {
            *pErrorBuffer = STAFString("You may not specify the JVM or J2 "
                                       "options without specifying a new "
                                       "JVMNAME").adoptImpl();
            return kSTAFServiceConfigurationError;
        }

        // Ok. We have a JVM.  Now let's try to load the service.

        data.fJVM = sJVMDataMap[jvmName];

        STAFConnectionPtr connPtr = data.fJVM->fConnProv->connect(sLocal);

        #if defined(STAF_OS_NAME_SOLARIS)
            // Needed to resolve timing issues on Solaris 64-bit
            STAFThreadSleepCurrentThread(1000, 0);
        #endif

        connPtr->writeUInt(JAVA_SERVICE_LOAD);
        connPtr->writeString(data.fName);
        connPtr->writeString(data.fExec);
        connPtr->writeString(pInfo->writeLocation);
        connPtr->writeUInt(pInfo->serviceType);

        unsigned int constructRC = kSTAFOk;

        try
        {
            constructRC = connPtr->readUInt();
        }
        catch (STAFException)
        {
            // This error can occur when using certain versions of Java
            // (e.g gcj that is provided with some versions of Linux)

            STAFString error = STAFString(
                "JSTAF.STAFServiceConstruct(): Cannot load the Java ") +
                "service in the JVM.  Verify you are using a valid version"
                " of Java (e.g. Sun or IBM Java).";
            *pErrorBuffer = error.adoptImpl();
            
            if (data.fJVM->fNumServices == 0) STAFShutdownJVM(jvmName);

            return kSTAFJavaError;
        }

        // Set a flag to indicate that the service has been loaded so that
        // if an exception occurs, the catch blocks will shutdown the JVM

        serviceLoaded = 1;
        
        STAFString constructResult = connPtr->readString();

        if (constructRC != kSTAFOk)
        {
            if (data.fJVM->fNumServices == 0) STAFShutdownJVM(jvmName);

            *pErrorBuffer = constructResult.adoptImpl();
            return constructRC;
        }

        // The service is now loaded

        STAFProcJavaServiceData *pService = new STAFProcJavaServiceData(data);

        *pServiceHandle = pService;
        ++pService->fJVM->fNumServices;

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString error = STAFString(
            "JSTAF.STAFServiceConstruct(), Service: ") + pInfo->name;

        e.trace(error.toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError, STAFString("Caught unknown exception in ") +
            "JSTAF.STAFServiceConstruct(), Service: " + pInfo->name);
    }

    if (serviceLoaded && (data.fJVM->fNumServices == 0))
        STAFShutdownJVM(jvmName);

    return kSTAFUnknownError;
}

STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
                             void *pDestructInfo,
                             unsigned int destructLevel,
                             STAFString_t *pErrorBuffer)
{
    if (destructLevel != 0) return kSTAFInvalidAPILevel;

    STAFProcJavaServiceData *pData =
        static_cast<STAFProcJavaServiceData *>(*serviceHandle);

    try
    {
        STAFMutexSemLock lock(sJVMDataSem);

        // Debug: cout << "Destructing service: " << pData->fName << endl;
        // Debug: cout << "Service pData->fJVM: " << pData->fJVM << endl;
        // Debug: cout << "Service JVMName: " << pData->fJVM->fName << endl;

        if (--pData->fJVM->fNumServices == 0)
            STAFShutdownJVM(pData->fJVM->fName);

        delete pData;
        *serviceHandle = 0;

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString error = STAFString(
            "JSTAF.STAFServiceDestruct(), Service: ") + pData->fName;

        e.trace(error.toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in JSTAF.STAFServiceDestruct() while "
            "destructing service " + pData->fName);
    }

    return kSTAFUnknownError;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
                         void *pInitInfo, unsigned int initLevel,
                         STAFString_t *pErrorBuffer)
{
    if (initLevel != 30) return kSTAFInvalidAPILevel;

    STAFProcJavaServiceData *pData =
        static_cast<STAFProcJavaServiceData *>(serviceHandle);

    try
    {
        STAFServiceInitLevel30 *pInfo =
            static_cast<STAFServiceInitLevel30 *>(pInitInfo);
        STAFConnectionPtr connPtr = pData->fJVM->fConnProv->connect(sLocal);

        #if defined(STAF_OS_NAME_SOLARIS)
            // Needed to resolve timing issues on Solaris 64-bit
            STAFThreadSleepCurrentThread(1000, 0);
        #endif

        connPtr->writeUInt(JAVA_SERVICE_INIT);
        connPtr->writeString(pData->fName);
        connPtr->writeString(pInfo->parms);
        connPtr->writeString(pInfo->writeLocation);

        unsigned int initRC = connPtr->readUInt();
        STAFString initResult = connPtr->readString();

        if (initRC != kSTAFOk)
        {
            *pErrorBuffer = initResult.adoptImpl();
            return initRC;
        }

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString error = STAFString("JSTAF.STAFServiceInit(), Service: ") +
            pData->fName;

        e.trace(error.toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in JSTAF.STAFServiceInit() "
            "while initializing service: " + pData->fName);
    }

    return kSTAFUnknownError;
}

STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
                         void *pTermInfo, unsigned int termLevel,
                         STAFString_t *pErrorBuffer)
{
    if (termLevel != 0) return kSTAFInvalidAPILevel;
    
    STAFProcJavaServiceData *pData =
        static_cast<STAFProcJavaServiceData *>(serviceHandle);

    try
    {
        STAFConnectionPtr connPtr = pData->fJVM->fConnProv->connect(sLocal);

        connPtr->writeUInt(JAVA_SERVICE_TERM);
        connPtr->writeString(pData->fName);

        unsigned int termRC = connPtr->readUInt();
        STAFString termResult = connPtr->readString();

        if (termRC != kSTAFOk)
        {
            *pErrorBuffer = termResult.adoptImpl();
            return termRC;
        }

        return kSTAFOk;
    }
    catch (STAFException &e)
    {
        STAFString error = STAFString("JSTAF.STAFServiceTerm(), Service: ") +
            pData->fName;

        e.trace(error.toCurrentCodePage()->buffer());
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            "Caught unknown exception in JSTAF.STAFServiceTerm() "
            "while terminating service: " + pData->fName);
    }

    return kSTAFUnknownError;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
                                  void *pRequestInfo, unsigned int reqLevel,
                                  STAFString_t *pResultBuffer)
{
    if (reqLevel != 30) return kSTAFInvalidAPILevel;

    STAFProcJavaServiceData *pData =
        static_cast<STAFProcJavaServiceData *>(serviceHandle);
    STAFServiceRequestLevel30 *pInfo =
        static_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

    try
    {
        STAFConnectionPtr connPtr = pData->fJVM->fConnProv->connect(sLocal);

        unsigned int  machineLength             = 0;
        const char   *machineBuffer             = 0;
        unsigned int  machineNicknameLength     = 0;
        const char   *machineNicknameBuffer     = 0;
        unsigned int  handleNameLength          = 0;
        const char   *handleNameBuffer          = 0;
        unsigned int  requestLength             = 0;
        const char   *requestBuffer             = 0;
        unsigned int  userLength                = 0;
        const char   *userBuffer                = 0;
        unsigned int  endpointLength            = 0;
        const char   *endpointBuffer            = 0;
        unsigned int  stafInstanceUUIDLength    = 0;
        const char   *stafInstanceUUIDBuffer    = 0;
        unsigned int  physicalInterfaceIDLength  = 0;
        const char   *physicalInterfaceIDBuffer  = 0;


        STAFStringGetBuffer(pInfo->machine, &machineBuffer,
                            &machineLength, 0);
        STAFStringGetBuffer(pInfo->machineNickname, &machineNicknameBuffer,
                            &machineNicknameLength, 0);
        STAFStringGetBuffer(pInfo->handleName, &handleNameBuffer,
                            &handleNameLength, 0);
        STAFStringGetBuffer(pInfo->request, &requestBuffer,
                            &requestLength, 0);
        STAFStringGetBuffer(pInfo->user, &userBuffer, &userLength, 0);
        STAFStringGetBuffer(pInfo->endpoint, &endpointBuffer,
                            &endpointLength, 0);
        STAFStringGetBuffer(pInfo->stafInstanceUUID, &stafInstanceUUIDBuffer,
                            &stafInstanceUUIDLength, 0);
        STAFStringGetBuffer(pInfo->physicalInterfaceID,
                            &physicalInterfaceIDBuffer,
                            &physicalInterfaceIDLength, 0);

        // IMPORTANT:  Increase the numFields value if add a field to the
        // ServiceRequest class for a new STAFServiceInterfaceLevel.

        unsigned int numFields = 16;

        unsigned int bufferLength = (numFields * sizeof(unsigned int)) +
            pData->fName.length() + machineLength + machineNicknameLength +
            handleNameLength + requestLength + userLength +
            endpointLength + stafInstanceUUIDLength +
            physicalInterfaceIDLength;

        STAFBuffer<char> buffer(new char[bufferLength], STAFBuffer<char>::INIT,
                                STAFBuffer<char>::ARRAY);
        unsigned int *uintBuffer = 
                     reinterpret_cast<unsigned int *>((char *)buffer);

        uintBuffer[0] = 
            STAFUtilConvertNativeUIntToLE(JAVA_SERVICE_ACCEPT_REQUEST);
        uintBuffer[1] = STAFUtilConvertNativeUIntToLE(
            bufferLength - (2 * sizeof(unsigned int)));
        uintBuffer[2] = pData->fName.length();
        uintBuffer[3] = pInfo->handle;
        uintBuffer[4] = pInfo->trustLevel;
        uintBuffer[5] = machineLength;
        uintBuffer[6] = machineNicknameLength;
        uintBuffer[7] = handleNameLength;
        uintBuffer[8] = requestLength;
        uintBuffer[9] = pInfo->diagEnabled;
        uintBuffer[10] = pInfo->requestNumber;
        uintBuffer[11] = userLength;
        uintBuffer[12] = endpointLength;
        uintBuffer[13] = stafInstanceUUIDLength;
        uintBuffer[14] = pInfo->isLocalRequest;
        uintBuffer[15] = physicalInterfaceIDLength;
        
        char *currBuffer = buffer + (numFields * sizeof(unsigned int));

        memcpy(currBuffer, pData->fName.buffer(), pData->fName.length());
        currBuffer += pData->fName.length();
        memcpy(currBuffer, machineBuffer, machineLength);
        currBuffer += machineLength;
        memcpy(currBuffer, machineNicknameBuffer, machineNicknameLength);
        currBuffer += machineNicknameLength;
        memcpy(currBuffer, handleNameBuffer, handleNameLength);
        currBuffer += handleNameLength;
        memcpy(currBuffer, requestBuffer, requestLength);
        currBuffer += requestLength;
        memcpy(currBuffer, userBuffer, userLength);
        currBuffer += userLength;
        memcpy(currBuffer, endpointBuffer, endpointLength);
        currBuffer += endpointLength;
        memcpy(currBuffer, stafInstanceUUIDBuffer, stafInstanceUUIDLength);
        currBuffer += stafInstanceUUIDLength;
        memcpy(currBuffer, physicalInterfaceIDBuffer,
               physicalInterfaceIDLength);
        currBuffer += physicalInterfaceIDLength;
        
        /* Debug:
        cout << "Buffer length: " << bufferLength << endl;
        cout << endl << "Buffer: " << endl;

        for (unsigned int i = 0; i < (bufferLength - 8); ++i)
        {
            if ((i != 0) && (i % 24 == 0)) cout << endl;
            unsigned int currChar = buffer[i + 8];
            if (currChar < 16) cout << "0";
            cout << hex << currChar << dec << " ";
        }

        cout << endl;
        */

        connPtr->write(buffer, bufferLength);

        unsigned int reqRC = connPtr->readUInt();
        STAFString reqResult = connPtr->readString();

        *pResultBuffer = reqResult.adoptImpl();
        return reqRC;
    }
    catch (STAFException &e)
    {
        STAFString error = STAFString("JSTAF.STAFServiceAcceptRequest()") +
            ", Endpoint: " + pInfo->endpoint +
            ", Service: " + pData->fName +
            ", Request: " + pInfo->request;
        
        try
        {
            e.trace(error.toCurrentCodePage()->buffer());
        }
        catch (...)
        {
            e.write(error.toCurrentCodePage()->buffer());
        }
    }
    catch (...)
    {
        STAFTrace::trace(
            kSTAFTraceError,
            STAFString("Caught unknown exception in ") +
            "JSTAF.STAFServiceAcceptRequest(), Endpoint: " + pInfo->endpoint +
            ", Service: " + pData->fName + ", Request: " + pInfo->request);
    }

    *pResultBuffer = STAFString().adoptImpl();

    return kSTAFUnknownError;
}
