/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFConnectionProvider.h"
#include "STAFString.h"
#include "STAFInternalUtil.h"
#include "STAFThreadManager.h"
#include "STAFTrace.h"
#include "STAFEventSem.h"
#include "STAFMutexSem.h"
#include "STAFUtilWin32.h"
#include <aclapi.h>
#include <map>
#include <set>
#include <stdlib.h>

#define CATCH_STANDARD(functionString) \
catch (STAFException &e)\
{\
    if (errorBuffer)\
    {\
        *errorBuffer = getExceptionString(e, "STAFLocalIPCConnProvider.cpp: "\
                                          functionString)\
                       .adoptImpl();\
    }\
}\
catch (...)\
{\
    if (errorBuffer)\
    {\
        STAFString error("STAFLocalIPCConnProvider.cpp: Caught unknown "\
                         "exception in " functionString "()");\
        *errorBuffer = error.adoptImpl();\
    }\
}

#define CATCH_STANDARD_TRACE(functionString) \
catch (STAFException &e)\
{\
    STAFTrace::trace(kSTAFTraceError,\
                     getExceptionString(e, "STAFLocalIPCConnProvider.cpp: "\
                                        functionString));\
}\
catch (...)\
{\
    STAFTrace::trace(kSTAFTraceError,\
                     "STAFLocalIPCConnProvider.cpp: Caught unknown exception "\
                     "in " functionString "()");\
}

// Prototypes

STAFRC_t STAFConnectionProviderConstruct(STAFConnectionProvider_t *provider,
                                         void *constructInfo,
                                         unsigned int constructInfoLevel,
                                         STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderStart(STAFConnectionProvider_t provider,
                                     void *startInfo,
                                     unsigned int startInfoLevel,
                                     STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderStop(STAFConnectionProvider_t provider,
                                    void *stopInfo,
                                    unsigned int stopInfoLevel,
                                    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderDestruct(STAFConnectionProvider_t *provider,
                                        void *destructInfo,
                                        unsigned int destructInfoLevel,
                                        STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderConnect(STAFConnectionProvider_t provider,
                                       STAFConnection_t *connection,
                                       void *connectInfo,
                                       unsigned int connectInfoLevel,
                                       STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetMyNetworkIDs(STAFConnectionProvider_t provider,
                                               STAFStringConst_t *logicalID,
                                               STAFStringConst_t *physicalD,
                                               STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetOptions(
    STAFConnectionProvider_t provider,
    STAFObject_t *options,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionProviderGetProperty(
    STAFConnectionProvider_t provider,
    STAFConnectionProviderProperty_t property, STAFStringConst_t *value,
    STAFString_t *errorBuffer);

STAFRC_t STAFConnectionRead(STAFConnection_t connection, void *buffer,
                            unsigned int readLength, STAFString_t *errorBuffer,
                            bool doTimeout);

STAFRC_t STAFConnectionReadUInt(STAFConnection_t connection, unsigned int *uint,
                                STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionReadSTAFString(STAFConnection_t connection,
                                      STAFString_t *stafString,
                                      STAFString_t *errorBuffer,
                                      bool doTimeout);

STAFRC_t STAFConnectionWrite(STAFConnection_t connection, void *buffer,
                             unsigned int writeLength,
                             STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionWriteUInt(STAFConnection_t connection, unsigned int uint,
                                 STAFString_t *errorBuffer, bool doTimeout);

STAFRC_t STAFConnectionWriteSTAFString(STAFConnection_t connection,
                                       STAFStringConst_t stafString,
                                       STAFString_t *errorBuffer,
                                       bool doTimeout);

STAFRC_t STAFConnectionGetPeerNetworkIDs(STAFConnection_t connection,
                                         STAFStringConst_t *logicalID,
                                         STAFStringConst_t *physicalD,
                                         STAFString_t *errorBuffer);

STAFRC_t STAFConnectionDestruct(STAFConnection_t *connection,
                                STAFString_t *errorBuffer);


static STAFConnectionProviderFunctionTable gFuncTable =
{
    STAFConnectionProviderConstruct,
    STAFConnectionProviderStart,
    STAFConnectionProviderStop,
    STAFConnectionProviderDestruct,
    STAFConnectionProviderConnect,
    STAFConnectionProviderGetMyNetworkIDs,
    STAFConnectionProviderGetOptions,
    STAFConnectionProviderGetProperty,
    STAFConnectionRead,
    STAFConnectionReadUInt,
    STAFConnectionReadSTAFString,
    STAFConnectionWrite,
    STAFConnectionWriteUInt,
    STAFConnectionWriteSTAFString,
    STAFConnectionGetPeerNetworkIDs,
    STAFConnectionDestruct
};

typedef enum
{
    kSharedMemory = 0,
    kNamedPipes   = 1
} STAFLocalIPCMethod_t;

struct STAFLocalConnectionProviderImpl : STAFConnectionProviderImpl
{
    STAFConnectionProviderMode_t mode;
    void *data;

    STAFString ipcName;
    STAFLocalIPCMethod_t ipcMethod;  // Indicates IPC method (named pipes or
                                     // shared memory)
    
    HANDLE serverSynch;  // Used only by shared memory IPC method
    HANDLE clientSynch;  // Used only by shared memory IPC method
    void *sharedMem;     // Used only by shared memory IPC method
    HANDLE fileMap;      // Used only by shared memory IPC method

    STAFString logicalNetworkID;
    STAFString physicalNetworkID;

    STAFObjectPtr options;

    STAFString portProperty;
    STAFString isSecureProperty;

    STAFConnectionProviderNewConnectionFunc_t connFunc;

    STAFEventSemPtr syncSem;
    STAFConnectionProviderState_t state;
    STAFThreadManagerPtr threadManager;
};

struct STAFLocalConnectionImpl : STAFConnectionImpl
{
    STAFLocalIPCMethod_t ipcMethod;  // Indicates IPC method (named pipes or
                                     // shared memory)
    union
    {
        HANDLE pipeHandle;  // Used only by named pipes IPC method
        HANDLE readHandle;  // Used only by shared memory IPC method
    };
    
    HANDLE writeHandle;     // Used only by shared memory IPC method
};

struct LocalConnectionData
{
    STAFConnectionProviderNewConnectionFunc_t connFunc;
    STAFLocalConnectionProviderImpl *provider;
    STAFLocalConnectionImpl *connection;
};


//===================================================================
//  These static variables are only used by named pipes IPC method
static unsigned int BUFFER_SIZE = 1024;
static STAFString PIPENAME_PREFIX = "\\\\.\\pipe\\STAF_";
//===================================================================

//===================================================================
//  These variables are only used by shared memory IPC method
struct PipeData
{
    HANDLE clientMutex;
    HANDLE serverSynchSem;
    HANDLE clientSynchSem;
    DWORD *pidSwap;
    HANDLE *readHandlePtr;
    HANDLE *writeHandlePtr;
    HANDLE serverProcessHandle;
};

typedef std::map<STAFString, PipeData> PipeMap;

static STAFMutexSem sPipeMapSem;
static PipeMap sPipeMap;
//===================================================================


unsigned int STAFLocalIPCConnectionThread(void *data)
{
    try
    {
        LocalConnectionData *connData =
            reinterpret_cast<LocalConnectionData *>(data);

        connData->connFunc(connData->provider, connData->connection,
                           &gFuncTable, connData->provider->data);

        delete connData;
    }
    CATCH_STANDARD_TRACE("STAFLocalIPCConnectionThread");

    return 0;
}

// These macros are to be used only by named pipes IPC method

#define HANDLE_NAMEDPIPES_RUN_ERROR(string) \
{\
    STAFTrace::trace(kSTAFTraceError, string);\
\
    if (connImpl.pipeHandle != 0)\
    {\
        DisconnectNamedPipe(connImpl.pipeHandle);\
        CloseHandle(connImpl.pipeHandle);\
    }\
\
    continue;\
}
#define HANDLE_NAMEDPIPES_RUN_ERROR1(string) \
{\
    HANDLE_NAMEDPIPES_RUN_ERROR(STAFString(\
        string) + ", osRC=" + STAFString(GetLastError()));\
}
#define HANDLE_NAMEDPIPES_RUN_ERROR2(string) \
{\
    HANDLE_NAMEDPIPES_RUN_ERROR(STAFString(string));\
}

// These macros are to be used only by shared memory IPC method

#define HANDLE_SHAREDMEM_RUN_ERROR(string) \
{\
    STAFTrace::trace(kSTAFTraceError, string);\
\
    if (clientWriteHandle != 0)    CloseHandle(clientWriteHandle);\
    if (connImpl.writeHandle != 0) CloseHandle(connImpl.writeHandle);\
    if (clientReadHandle != 0)     CloseHandle(clientReadHandle);\
    if (connImpl.readHandle != 0)  CloseHandle(connImpl.readHandle);\
    if (clientProcess != 0)        CloseHandle(clientProcess);\
\
    *pidSwap = 0;\
    SetEvent(provider->clientSynch);\
\
    continue;\
}
#define HANDLE_SHAREDMEM_RUN_ERROR1(string) \
{\
    HANDLE_SHAREDMEM_RUN_ERROR(STAFString(\
        string) + ", osRC=" + STAFString(GetLastError()));\
}
#define HANDLE_SHAREDMEM_RUN_ERROR2(string) \
{\
    HANDLE_SHAREDMEM_RUN_ERROR(STAFString(string));\
}

unsigned int STAFLocalIPCRunThread(void *providerImpl)
{
    STAFLocalConnectionProviderImpl *provider =
        reinterpret_cast<STAFLocalConnectionProviderImpl *>(providerImpl);

    STAFLocalConnectionImpl connImpl;
    connImpl.ipcMethod = provider->ipcMethod;
    
    BOOL connected;
    PSECURITY_DESCRIPTOR sd = 0;

    try
    {
        provider->syncSem->post();
        
        if (provider->ipcMethod == kNamedPipes)
        {
            STAFStringBufferPtr pipeNamePtr =
                (PIPENAME_PREFIX + provider->ipcName).toCurrentCodePage();
            const char *pipeName = pipeNamePtr->buffer();

            // Set up security attributes 
            
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = sd;
        
            // Create a Null Security Descriptor to allow access to everyone

            if (STAFUtilWin32CreateNullSD(&sd))
            {
                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("STAFUtilWin32CreateNullSD(), osRC=") +
                    STAFString(GetLastError()));
            }
            
            // Create the initial named pipe.  In the while loop we'll create
            // a second named pipe so that with two named pipes, one pipe will
            // always be available so that we won't get the following error:
            //  STAFConnectionProviderConnect: Could not connect to the server
            //                                 named pipe: 2, Error code: 10

            HANDLE backupPipeHandle = CreateNamedPipe(
                pipeName,
                PIPE_ACCESS_DUPLEX, // read/write access
                PIPE_TYPE_BYTE | // byte type pipe
                PIPE_READMODE_BYTE | // byte-read mode
                PIPE_WAIT, // blocking mode
                PIPE_UNLIMITED_INSTANCES, // max. instances
                BUFFER_SIZE, // output buffer size
                BUFFER_SIZE, // input buffer size
                NMPWAIT_USE_DEFAULT_WAIT, // client time-out
                &sa);

            if (backupPipeHandle == INVALID_HANDLE_VALUE)
            {
                STAFTrace::trace(
                    kSTAFTraceError,
                    STAFString("CreateNamedPipe() for backup pipe failed,"
                               " osRC=") + STAFString(GetLastError()));
            }

            while (provider->state == kSTAFConnectionProviderActive)
            {
                // Need a try block within the while loop so that if an exception
                // occurs, this thread won't terminate so this interface can continue
                // accepting requests

                try
                {
                    connImpl.pipeHandle = backupPipeHandle;

                    backupPipeHandle = CreateNamedPipe(
                        pipeName,
                        PIPE_ACCESS_DUPLEX, // read/write access
                        PIPE_TYPE_BYTE | // byte type pipe
                        PIPE_READMODE_BYTE | // byte-read mode
                        PIPE_WAIT, // blocking mode
                        PIPE_UNLIMITED_INSTANCES, // max. instances
                        BUFFER_SIZE, // output buffer size
                        BUFFER_SIZE, // input buffer size
                        NMPWAIT_USE_DEFAULT_WAIT, // client time-out
                        &sa);
                    
                    if (connImpl.pipeHandle == INVALID_HANDLE_VALUE)
                        HANDLE_NAMEDPIPES_RUN_ERROR1("CreateNamedPipe()");

                    // Wait for the client to connect; if it succeeds, the function
                    // returns a nonzero value. If the function returns zero,
                    // GetLastError returns ERROR_PIPE_CONNECTED. 

                    connected = ConnectNamedPipe(connImpl.pipeHandle, NULL) ?
                        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

                    if (!connected)
                    {
                        HANDLE_NAMEDPIPES_RUN_ERROR1("ConnectNamedPipe()");
                    }

                    if (provider->state != kSTAFConnectionProviderActive)
                        break;

                    // Ok, let's perform the callback now

                    LocalConnectionData connData;

                    connData.connFunc   = provider->connFunc;
                    connData.provider   = provider;
                    connData.connection = new STAFLocalConnectionImpl(connImpl);

                    provider->threadManager->dispatch(
                        STAFLocalIPCConnectionThread,
                        new LocalConnectionData(connData));
                }
                catch (STAFException &e)
                {
                    HANDLE_NAMEDPIPES_RUN_ERROR2(
                        getExceptionString(e, "STAFLocalIPCConnProvider.cpp: "
                                           "STAFLocalIPCRunThread"));
                }
                catch (...)
                {
                    HANDLE_NAMEDPIPES_RUN_ERROR2(
                        "STAFLocalIPCConnProvider.cpp: Caught unknown "
                        "exception in STAFLocalIPCRunThread()");
                }
            }

            // Flush the pipe to allow the client to read the pipe's contents 
            // before disconnecting. Then disconnect the pipe, and close the
            // handle to this pipe instance. 

            FlushFileBuffers(backupPipeHandle); 
            DisconnectNamedPipe(backupPipeHandle); 
            CloseHandle(backupPipeHandle);
        }
        else
        {
            // Set up variables into shared memory

            DWORD *pidSwap = (DWORD *)provider->sharedMem;
            HANDLE *readHandlePtr = (HANDLE *)((char *)provider->sharedMem +
                                               (2 * sizeof(DWORD)));
            HANDLE *writeHandlePtr =
                (HANDLE *)((char *)provider->sharedMem +
                           (2 * sizeof(DWORD)) + sizeof(HANDLE));
            
            while (provider->state == kSTAFConnectionProviderActive)
            {
                // XXX: See if there is any positive gain by moving the
                // definitions of these 3 variables outside the while loop

                HANDLE clientReadHandle = 0;
                HANDLE clientWriteHandle = 0;
                HANDLE clientProcess = 0;

                // Need a try block within the while loop so that if an
                // exception occurs, this thread won't terminate so this
                // interface can continue accepting requests

                try
                {
                    WaitForMultipleObjects(
                        1, &provider->serverSynch, TRUE, INFINITE);
            
                    if (provider->state != kSTAFConnectionProviderActive)
                        break;

                    // XXX: Temp until auto-reset event sem done
                    ResetEvent(provider->serverSynch);

                    clientProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                                                *pidSwap);

                    if (clientProcess == 0)
                        HANDLE_SHAREDMEM_RUN_ERROR1("OpenProcess2()");

                    // Create ServerRead/ClientWrite pipe
                    if (!CreatePipe(&connImpl.readHandle,
                                    &clientWriteHandle, 0, 0x00000400))
                    {
                        HANDLE_SHAREDMEM_RUN_ERROR1("CreatePipe()");
                    }

                    // Create ServerWrite/ClientRead pipe
                    if (!CreatePipe(&clientReadHandle, &connImpl.writeHandle,
                                    0, 0x00000400))
                    {
                        HANDLE_SHAREDMEM_RUN_ERROR1("CreatePipe()");
                    }
                    
                    if (!DuplicateHandle(
                        GetCurrentProcess(), clientReadHandle, clientProcess,
                        readHandlePtr, 0, FALSE,
                        DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
                    {
                        HANDLE_SHAREDMEM_RUN_ERROR1(
                            "DuplicateHandle(readHandle)");
                    }

                    if (!DuplicateHandle(
                        GetCurrentProcess(), clientWriteHandle, clientProcess,
                        writeHandlePtr, 0, FALSE,
                        DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
                    {
                        HANDLE_SHAREDMEM_RUN_ERROR1(
                            "DuplicateHandle(writeHandle)");
                    }

                    CloseHandle(clientProcess);
                    SetEvent(provider->clientSynch);
            
                    // Ok, let's perform the callback now

                    LocalConnectionData connData;

                    connData.connFunc   = provider->connFunc;
                    connData.provider   = provider;
                    connData.connection = new STAFLocalConnectionImpl(
                        connImpl);

                    provider->threadManager->dispatch(
                        STAFLocalIPCConnectionThread,
                        new LocalConnectionData(connData));
                }
                catch (STAFException &e)
                {
                    HANDLE_SHAREDMEM_RUN_ERROR2(
                        getExceptionString(e, "STAFLocalIPCConnProvider.cpp: "
                                           "STAFLocalIPCRunThread"));
                }
                catch (...)
                {
                    HANDLE_SHAREDMEM_RUN_ERROR2(
                        "STAFLocalIPCConnProvider.cpp: Caught unknown "
                        "exception in STAFLocalIPCRunThread()");
                }
            }
        }
    }
    CATCH_STANDARD_TRACE("STAFLocalIPCRunThread");

    if (sd != 0) STAFUtilWin32DeleteNullSD(&sd);

    try
    {
        provider->syncSem->post();
    }
    CATCH_STANDARD_TRACE("STAFLocalIPCRunThread: post");

    return 0;
}

//============ Shared Memory Only Functions ==================================
// These functions are only used if using shared memory (not when using
// named pipes as the ipcMethod) 

#define HANDLE_PIPEDATA_ERROR(string) \
HANDLE_PIPEDATA_ERROR2(string, GetLastError())

#define HANDLE_PIPEDATA_ERROR2(string, error) \
{\
    STAFString theError = STAFString(string) + ", osRC=" + STAFString(error);\
\
    if (errorBuffer)                  *errorBuffer = theError.adoptImpl();\
    if (sd != 0)                      STAFUtilWin32DeleteNullSD(&sd);\
    if (sharedMem != 0)               UnmapViewOfFile(sharedMem);\
    if (fileMap != 0)                 CloseHandle(fileMap);\
    if (pipeData.clientMutex != 0)    CloseHandle(pipeData.clientMutex);\
    if (pipeData.serverSynchSem != 0) CloseHandle(pipeData.serverSynchSem);\
    if (pipeData.clientSynchSem != 0) CloseHandle(pipeData.clientSynchSem);\
\
    return;\
}

// Note: You should already own sPipeMapSem before calling this function
static void GetPipeData(const STAFString &pipeName, PipeData &pipeData,
                        STAFString_t *errorBuffer)
{
    PipeMap::iterator iter;

    if ((iter = sPipeMap.find(pipeName)) == sPipeMap.end())
    {
        HANDLE fileMap = 0;
        void *sharedMem = 0;
        DWORD *serverPID = 0;

        SECURITY_ATTRIBUTES sa;
        PSECURITY_DESCRIPTOR sd = 0;

        STAFString globalStr = "";

        if (STAFUtilWin32GetWinType() & kSTAFWin2KPlus)
        {
            // Open Process to get process handle and SetSecurityInfo
            HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                                         GetCurrentProcessId());
            if (pHandle == 0) HANDLE_PIPEDATA_ERROR("First OpenProcess()");
        
            int rc = SetSecurityInfo(pHandle, SE_KERNEL_OBJECT,
                                     DACL_SECURITY_INFORMATION, 0, 0, 0, 0);
            if (rc != 0) HANDLE_PIPEDATA_ERROR2("SetSecurityInfo()", rc);
  
            // To support Fast User Switching and Terminal Server need
            // to prepend Global\\ for Win2K and above.
            globalStr = "Global\\";
        }

        STAFString sharedMemName = globalStr + pipeName + "/SharedMemory";

        // Obtain Shared Memory
        fileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
                                  sharedMemName.toCurrentCodePage()->buffer());

        if (fileMap == 0)
        {
            STAFString errorMsg;

            if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                // Get instance name

                STAFString ipcName = "STAF";

                if (getenv("STAF_INSTANCE_NAME") != NULL)
                {
                    ipcName = getenv("STAF_INSTANCE_NAME");
                }

                errorMsg = STAFString(
                    "There is no STAFProc instance running with "
                    "STAF_INSTANCE_NAME=") + ipcName +
                    " on the local machine. Verify that STAFProc is "
                    "running and set this environment variable to "
                    "the value that STAFProc is using. ";
            }

            errorMsg += "OpenFileMapping()";

            HANDLE_PIPEDATA_ERROR(errorMsg);
        }

        sharedMem = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        if (sharedMem == 0) HANDLE_PIPEDATA_ERROR("MapViewOfFile()");

        // Set up security attributes
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = sd;

        // Create Null Security Descriptor to allow access to everyone
        if (STAFUtilWin32CreateNullSD(&sd))
            HANDLE_PIPEDATA_ERROR("STAFUtilWin32CreateNullSD()");

        sa.lpSecurityDescriptor = sd;

        // Get initialization semaphores
        STAFString clientMutexName = globalStr + pipeName + "/ClientMutex";
        STAFString serverSynchName = globalStr + pipeName + "/ServerSynch";
        STAFString clientSynchName = globalStr + pipeName + "/ClientSynch";

        pipeData.clientMutex = CreateMutex(&sa, FALSE,
                               clientMutexName.toCurrentCodePage()->buffer());
        if (pipeData.clientMutex == 0) HANDLE_PIPEDATA_ERROR("CreateMutex()");

        pipeData.serverSynchSem = CreateEvent(&sa, TRUE, FALSE, 
                                  serverSynchName.toCurrentCodePage()->buffer());
        if (pipeData.serverSynchSem == 0) HANDLE_PIPEDATA_ERROR("CreateEvent()");
                
        pipeData.clientSynchSem = CreateEvent(&sa, TRUE, FALSE, 
                                  clientSynchName.toCurrentCodePage()->buffer());
        if (pipeData.clientSynchSem == 0) HANDLE_PIPEDATA_ERROR("CreateEvent()");

        // Set up some pointers into the shared memory area
        pipeData.pidSwap = (DWORD *)sharedMem;
        serverPID = (DWORD *)((char *)sharedMem + sizeof(DWORD));
        pipeData.readHandlePtr = (HANDLE *)((char *)sharedMem +
                                            (2 *sizeof(DWORD)));
        pipeData.writeHandlePtr = (HANDLE *)((char *)sharedMem +
                                             (2 * sizeof(DWORD)) +
                                             sizeof(HANDLE));

        // Obtain handle to Factory process
        pipeData.serverProcessHandle = OpenProcess(PROCESS_ALL_ACCESS,
                                                   FALSE, *serverPID);

        if (pipeData.serverProcessHandle == 0)
            HANDLE_PIPEDATA_ERROR("Second OpenProcess()");

        sPipeMap[pipeName] = pipeData;

        if (sd != 0) STAFUtilWin32DeleteNullSD(&sd);
    }
    else pipeData = iter->second;
}


#define HANDLE_PIPECONNECT_ERROR(string) \
HANDLE_PIPECONNECT_ERROR2(string, GetLastError())

#define HANDLE_PIPECONNECT_ERROR2(string, error) \
{\
    STAFString theError = STAFString(string) + STAFString(error);\
\
    if (errorBuffer)                  *errorBuffer = theError.adoptImpl();\
\
    return false;\
}


// True = successfully connected, False = server process disappeared
// Note: This function can still throw exceptions
static bool ConnectToPipe(PipeData &pipeData, HANDLE &readHandle,
                          HANDLE &writeHandle, STAFString_t *errorBuffer)
{
    // Obtain the client mutex on this "named pipe", while we swap
    // process information with the STAFLocalConnectionFactory
    WaitForSingleObject(pipeData.clientMutex, INFINITE);

    *pipeData.pidSwap = GetCurrentProcessId();

    ResetEvent(pipeData.clientSynchSem);
    SetEvent(pipeData.serverSynchSem);

    HANDLE waitArray[2] = { pipeData.clientSynchSem,
                            pipeData.serverProcessHandle };
    DWORD rc = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
    unsigned int signaledHandleIndex = 0;

    if ((rc >= WAIT_OBJECT_0) && (rc <= (WAIT_OBJECT_0 + 2 - 1)))
    {
        signaledHandleIndex = (unsigned int)(rc - WAIT_OBJECT_0);
    }
    else if ((rc >= WAIT_ABANDONED_0) &&
             (rc <= (WAIT_ABANDONED_0 + 2 - 1)))
    {
        signaledHandleIndex = (unsigned int)(rc - WAIT_ABANDONED_0);
    }
    else
    {
        // We either got WAIT_FAILED, WAIT_TIMEOUT or some other return
        // code which we weren't expecting, so spit out an error message
        // and continue

        ReleaseMutex(pipeData.clientMutex);

        HANDLE_PIPECONNECT_ERROR2("Other side closed connection", rc);
    }

    if (signaledHandleIndex != 0)
    {
        ReleaseMutex(pipeData.clientMutex);

        return false;
    }

    if (pipeData.pidSwap == 0)
    {
        ReleaseMutex(pipeData.clientMutex);

        HANDLE_PIPECONNECT_ERROR("Other side closed connection");
    }

    readHandle = *pipeData.readHandlePtr;
    writeHandle = *pipeData.writeHandlePtr;

    ReleaseMutex(pipeData.clientMutex);

    return true;
}
//========= End Shared Memory Only Functions =================================


STAFRC_t STAFConnectionProviderConstruct(STAFConnectionProvider_t *provider,
                                         void *constructInfo,
                                         unsigned int constructInfoLevel,
                                         STAFString_t *errorBuffer)
{
    if (provider == 0) return kSTAFInvalidParm;
    if (constructInfoLevel != 1) return kSTAFInvalidAPILevel;

    STAFConnectionProviderConstructInfoLevel1 *cpInfo =
        reinterpret_cast<STAFConnectionProviderConstructInfoLevel1 *>(
            constructInfo);

    try
    {
        STAFLocalConnectionProviderImpl lipcData;

        lipcData.mode     = cpInfo->mode;
        lipcData.syncSem  = STAFEventSemPtr(
            new STAFEventSem, STAFEventSemPtr::INIT);
        lipcData.state    = kSTAFConnectionProviderStopped;
        lipcData.ipcName  = "STAF";
        lipcData.threadManager = STAFThreadManagerPtr(
            new STAFThreadManager, STAFThreadManagerPtr::INIT);
        
        // The Windows local connection provider supports two methods for
        // handling interprocess communication (IPC):
        // 1) Shared memory
        // 2) Named pipes
        //
        // Notes:
        // - Windows Me/98/95 cannot create named pipes so the shared memory
        //   IPC method must be used.
        // - Windows Vista with UAC enabled cannot use global shared memory
        //   so the named pipes method must be used.
        // - Windows 2003 when logged on as a non-administrator user cannot
        //   use global shared memory, so the named pipes method must be used.
        // - The performance using either of these methods is equivalent.
        //   So, we chose to use named pipes for Windows Server 2003,
        //   Windows Vista, Windows Server 2008, and future Windows versions.
        //   We'll continue to use shared memory for earlier Windows operating
        //   systems (though we could have selected to use named pipes for
        //   Windows NT and later).

        if (STAFUtilWin32GetWinType() & kSTAFWin2K3Plus)
            lipcData.ipcMethod = kNamedPipes;
        else
            lipcData.ipcMethod = kSharedMemory;

        // Get instance name

        if (getenv("STAF_INSTANCE_NAME") != NULL)
        {
            lipcData.ipcName  =  getenv("STAF_INSTANCE_NAME");
        }

        for (unsigned int i = 0; i < cpInfo->numOptions; ++i)
        {
            STAFString thisOption =
                       STAFString(cpInfo->optionNames[i]).upperCase();

            if (thisOption == "IPCNAME")
            {
                lipcData.ipcName += cpInfo->optionValues[i];
            }
            else
            {
                if (errorBuffer)
                {
                    *errorBuffer =
                        STAFString(cpInfo->optionNames[i]).adoptImpl();
                }
                return kSTAFInvalidValue;
            }
        }

        // Add each option to a map.

        lipcData.options = STAFObject::createMap();
        lipcData.options->put("IPCName", lipcData.ipcName);

        if (lipcData.ipcMethod == kNamedPipes)
            lipcData.options->put("IPCMethod", STAFString("Named pipes"));
        else
            lipcData.options->put("IPCMethod", STAFString("Shared memory"));

        // Setup property values
        lipcData.portProperty = STAFString();
        lipcData.isSecureProperty = STAFString("0");

        // Assign logical and physical identifiers
        lipcData.logicalNetworkID = STAFString("local");
        lipcData.physicalNetworkID = STAFString("local");

        *provider = new STAFLocalConnectionProviderImpl(lipcData);

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderConstruct");

    return kSTAFUnknownError;
}


#define HANDLE_START_ERROR(string) HANDLE_START_ERROR2(string, GetLastError())

#define HANDLE_START_ERROR2(string, error) \
{\
    STAFString theError = STAFString(string) + ", osRC=" + STAFString(error);\
\
    if (sd != 0) STAFUtilWin32DeleteNullSD(&sd);\
    if (provider->ipcMethod == kSharedMemory)\
    {\
        if (provider->sharedMem != 0) UnmapViewOfFile(provider->sharedMem);\
        if (provider->fileMap != 0) CloseHandle(provider->fileMap);\
    }\
\
    if (errorBuffer) *errorBuffer = theError.adoptImpl();\
    return kSTAFBaseOSError;\
}


STAFRC_t STAFConnectionProviderStart(STAFConnectionProvider_t baseProvider,
                                     void *startInfo,
                                     unsigned int startInfoLevel,
                                     STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (startInfoLevel != 1) return kSTAFInvalidAPILevel;

    STAFConnectionProviderStartInfoLevel1 *cpInfo =
        reinterpret_cast<STAFConnectionProviderStartInfoLevel1 *>(startInfo);

    try
    {
        if (cpInfo->newConnectionFunc == 0) return kSTAFInvalidValue;

        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);
        
        SECURITY_ATTRIBUTES sa;
        PSECURITY_DESCRIPTOR sd = 0;

        // Set up security attributes 
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = sd;

        // Create a Null Security Descriptor to allow access to everyone.
        if (STAFUtilWin32CreateNullSD(&sd))
            HANDLE_START_ERROR("STAFUtilWin32CreateNullSD()");

        sa.lpSecurityDescriptor = sd;
        
        provider->connFunc = cpInfo->newConnectionFunc;
        provider->data = cpInfo->data;

        if (provider->ipcMethod == kNamedPipes)
        {
            // Verify that a named pipe can be created successfully

            STAFString pipeName = PIPENAME_PREFIX + "Test" + provider->ipcName;

            HANDLE hPipe = CreateNamedPipe(
                pipeName.toCurrentCodePage()->buffer(),
                PIPE_ACCESS_DUPLEX, // read/write access
                PIPE_TYPE_BYTE | // byte type pipe
                PIPE_READMODE_BYTE | // byte-read mode
                PIPE_WAIT, // blocking mode
                PIPE_UNLIMITED_INSTANCES, // max. instances
                BUFFER_SIZE, // output buffer size
                BUFFER_SIZE, // input buffer size
                NMPWAIT_USE_DEFAULT_WAIT, // client time-out
                &sa); 

            if (hPipe == INVALID_HANDLE_VALUE)
                HANDLE_START_ERROR("Initial CreateNamedPipe() failed");

            CloseHandle(hPipe);
        }
        else  // Shared memory is the ipcMethod
        {
            // To support Fast User Switching/Terminal Server need Global\\
            // prepended for WinXP, Win2K and above.
        
            STAFString globalStr = "";

            if (STAFUtilWin32GetWinType() & kSTAFWin2KPlus)
                globalStr = "Global\\";
        
            STAFString sharedMemoryName = globalStr + provider->ipcName +
                "/SharedMemory";
                                      
            provider->fileMap = 0;
            provider->sharedMem = 0;
        
            // Create events for server and client passing in security attributes
            provider->serverSynch = CreateEvent(
                &sa, TRUE, FALSE,
                STAFString(globalStr + provider->ipcName + "/ServerSynch").
                toCurrentCodePage()->buffer());
            
            provider->clientSynch = CreateEvent(
                &sa, TRUE, FALSE,
                STAFString(globalStr + provider->ipcName + "/ClientSynch").
                toCurrentCodePage()->buffer());

            // Obtain Shared Memory with security
            provider->fileMap = CreateFileMapping(
                INVALID_HANDLE_VALUE, &sa,
                PAGE_READWRITE, 0, 2 * sizeof(DWORD) + 2 * sizeof(HANDLE),
                sharedMemoryName.toCurrentCodePage()->buffer());

            if (provider->fileMap == 0)
                HANDLE_START_ERROR("CreateFileMapping()");
        
            provider->sharedMem = MapViewOfFile(
                provider->fileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

            if (provider->sharedMem == 0)
                HANDLE_START_ERROR("MapViewOfFile()");
        
            DWORD *serverPID = (DWORD *)((char *)provider->sharedMem +
                                          sizeof(DWORD));
            *serverPID = GetCurrentProcessId();

            if (STAFUtilWin32GetWinType() & kSTAFWin2KPlus)
            {
                // Open the Process to get its handle to pass to SetSecurityInfo
                HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                                             *serverPID);
                if (pHandle == 0) HANDLE_START_ERROR("OpenProcess()");
        
                int rc = SetSecurityInfo(
                    pHandle, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                    0, 0, 0, 0);

                if (rc != 0) HANDLE_START_ERROR2("SetSecurityInfo()", rc);
            }
        }
            
        if (sd != 0) STAFUtilWin32DeleteNullSD(&sd);

        // Ok, the provider is now ready

        provider->syncSem->reset();
        provider->state = kSTAFConnectionProviderActive;
        provider->threadManager->dispatch(STAFLocalIPCRunThread, provider);
        provider->syncSem->wait();

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderStart");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderStop(STAFConnectionProvider_t baseProvider,
                                    void *stopInfo,
                                    unsigned int stopInfoLevel,
                                    STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (stopInfoLevel != 0) return kSTAFInvalidAPILevel;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        provider->state = kSTAFConnectionProviderStopped;

        // Wake up the run thread

        provider->syncSem->reset();

        if (provider->ipcMethod == kSharedMemory)
            SetEvent(provider->serverSynch);

        if (provider->syncSem->wait(10000) != 0)
        {
            STAFTrace::trace(
                kSTAFTraceWarning,
                STAFString("STAFLocalIPCConnectionProviderStop - Timed out "
                           "waiting for run thread to wake up"));
        }

        // Release resources acquired when starting provider

        if (provider->ipcMethod == kSharedMemory)
        {
            UnmapViewOfFile(provider->sharedMem);
            CloseHandle(provider->fileMap);

            CloseHandle(provider->clientSynch);
            CloseHandle(provider->serverSynch);
        }

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderStop");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderDestruct(STAFConnectionProvider_t *baseProvider,
                                        void *destructInfo,
                                        unsigned int destructInfoLevel,
                                        STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (*baseProvider == 0) return kSTAFInvalidObject;
    if (destructInfoLevel != 0) return kSTAFInvalidAPILevel;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(*baseProvider);

        if (provider->state != kSTAFConnectionProviderStopped)
        {
            provider->state = kSTAFConnectionProviderStopped;

            if (provider->ipcMethod == kSharedMemory)
            {
                UnmapViewOfFile(provider->sharedMem);
                CloseHandle(provider->fileMap);
            }
        }

        delete provider;

        provider = 0;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderDestruct");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderConnect(STAFConnectionProvider_t baseProvider,
                                       STAFConnection_t *connection,
                                       void *connectInfo,
                                       unsigned int connectInfoLevel,
                                       STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (connectInfoLevel != 1) return kSTAFInvalidAPILevel;
    if (connection == 0) return kSTAFInvalidParm;

    // XXX: Need to read connectInfoLevel and take appropriate action

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        STAFLocalConnectionImpl connImpl;
        connImpl.ipcMethod = provider->ipcMethod;

        if (provider->ipcMethod == kNamedPipes)
        {
            STAFStringBufferPtr pipeNamePtr =
                (PIPENAME_PREFIX + provider->ipcName).toCurrentCodePage();
            const char *pipeName = pipeNamePtr->buffer();

            unsigned int maxConnectAttempts = 100;
            
            // Try to open a named pipe; if not available wait for it.
            // Retry up to 100 times if necessary.
            
            for (unsigned int connectAttempt = 1;
                 connectAttempt <= maxConnectAttempts; ++connectAttempt)
            {
                connImpl.pipeHandle = CreateFile(
                    pipeName,       // pipe name
                    GENERIC_READ | GENERIC_WRITE,  // read and write access
                    0,              // no sharing
                    NULL,           // default security attributes
                    OPEN_EXISTING,  // opens existing pipe
                    0,              // default attributes
                    NULL);          // no template file
                
                // Break out of loop if the pipe handle is valid.
                
                if (connImpl.pipeHandle != INVALID_HANDLE_VALUE)
                    break;
                
                // Exit if an error other than ERROR_PIPE_BUSY occurs.
                
                unsigned int osRC = GetLastError();

                if (osRC != ERROR_PIPE_BUSY)
                {
                    STAFString connectError;

                    if (osRC == ERROR_FILE_NOT_FOUND)
                    {
                        connectError = STAFString(
                            "There is no STAFProc instance running with "
                            "STAF_INSTANCE_NAME=") + provider->ipcName +
                            " on the local machine. Verify that STAFProc is "
                            "running and set this environment variable to "
                            "the value that STAFProc is using. ";
                    }

                    connectError += STAFString(
                        "Could not connect to the server named pipe, osRC=") +
                        osRC;
                    if (errorBuffer) *errorBuffer = connectError.adoptImpl();
                    return kSTAFBaseOSError;
                }
                
                // All pipe instances are busy.

                if (connectAttempt < maxConnectAttempts)
                {
                    // Wait for up to 60 seconds for a named pipe and continue

                    WaitNamedPipe(pipeName, 60000);
                }
                else
                {
                    STAFString connectError = STAFString(
                        "Failed to connect to the server named pipe after "
                        "trying ") + maxConnectAttempts + " times, osRC=" +
                        osRC;
                    if (errorBuffer) *errorBuffer = connectError.adoptImpl();
                    return kSTAFBaseOSError;
                }
            }

            // The pipe connected
        }
        else
        {
            // provider->ipcMethod is kSharedMemory 

            PipeData pipeData = { 0 };

            {
                STAFMutexSemLock lock(sPipeMapSem);

                GetPipeData(provider->ipcName, pipeData, errorBuffer);

                if (errorBuffer && *errorBuffer != 0)
                    return kSTAFCommunicationError;
            }

            bool connected = ConnectToPipe(pipeData, connImpl.readHandle,
                                           connImpl.writeHandle, 0);

            if (!connected)
            {
                // This case means that the server we were attached to has
                // disappeared.  So, let's flush the cache and try again.

                {
                    STAFMutexSemLock lock(sPipeMapSem);

                    sPipeMap.erase(provider->ipcName);

                    GetPipeData(provider->ipcName, pipeData, errorBuffer);
                }

                connected = ConnectToPipe(pipeData, connImpl.readHandle,
                                          connImpl.writeHandle, errorBuffer);

                // If the problem still exists then throw the error

                if (!connected)
                {
                    STAFString connectError("Other side closed connection "
                                            "(process ended)");
                    if (errorBuffer) *errorBuffer = connectError.adoptImpl();

                    return kSTAFBaseOSError;
                }
            }
        }

        *connection = new STAFLocalConnectionImpl(connImpl);

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderConnect");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderGetMyNetworkIDs(
    STAFConnectionProvider_t baseProvider,
    STAFStringConst_t *logicalID,
    STAFStringConst_t *physicalID,
    STAFString_t *errorBuffer)
{
    if (baseProvider   == 0) return kSTAFInvalidObject;
    if (logicalID  == 0) return kSTAFInvalidParm;
    if (physicalID == 0) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        *logicalID = (STAFStringConst_t)provider->logicalNetworkID.getImpl();
        *physicalID = (STAFStringConst_t)provider->physicalNetworkID.getImpl();

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderGetMyNetworkIDs");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderGetOptions(
    STAFConnectionProvider_t baseProvider,
    STAFObject_t *options,
    STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (options  == 0) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        STAFObjectConstructReference(options, provider->options->getImpl());

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderGetOptions");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionProviderGetProperty(
    STAFConnectionProvider_t baseProvider,
    STAFConnectionProviderProperty_t property, STAFStringConst_t *value,
    STAFString_t *errorBuffer)
{
    if (baseProvider == 0) return kSTAFInvalidObject;
    if (value    == 0) return kSTAFInvalidParm;
    
    try
    {
        STAFLocalConnectionProviderImpl *provider =
            static_cast<STAFLocalConnectionProviderImpl *>(baseProvider);

        if (property == kSTAFConnectionProviderPortProperty)
            *value = (STAFStringConst_t)provider->portProperty.getImpl();
        else if (property == kSTAFConnectionProviderIsSecureProperty)
            *value = (STAFStringConst_t)provider->isSecureProperty.getImpl();
        else
            return kSTAFInvalidValue;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionProviderGetProperty");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionRead(STAFConnection_t baseConnection, void *buffer,
                            unsigned int readLength,
                            STAFString_t *errorBuffer,
                            bool doTimeout)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if ((buffer == 0) && (readLength != 0)) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionImpl *connection =
            static_cast<STAFLocalConnectionImpl *>(baseConnection);

        BOOL rc = TRUE;
        DWORD actual = 0;

        HANDLE connHandle;

        if (connection->ipcMethod == kNamedPipes)
            connHandle = connection->pipeHandle;
        else
            connHandle = connection->readHandle;

        for(DWORD current = 0; current < readLength; current += actual)
        {
            rc = ReadFile(connHandle, (LPVOID)((char *)buffer + current),
                          (DWORD)(readLength - current), &actual, 0);

            if (rc == FALSE)
            {
                STAFString error = STAFString("ReadFile osRC=") +
                    GetLastError();
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFBaseOSError;
            }

            if ((actual == 0) && (readLength != 0))
            {
                STAFString error("No data available on pipe");
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFBaseOSError;
            }
        }

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionRead");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionReadUInt(STAFConnection_t connection,
                                unsigned int *uint, STAFString_t *errorBuffer,
                                bool doTimeout)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (uint == 0) return kSTAFInvalidParm;

    STAFRC_t rc = STAFConnectionRead(connection, uint, sizeof(unsigned int),
                                     errorBuffer, doTimeout);

    if (rc == kSTAFOk) *uint = STAFUtilConvertLEUIntToNative(*uint);

    return rc;
}


STAFRC_t STAFConnectionReadSTAFString(STAFConnection_t connection,
                                      STAFString_t *stafString,
                                      STAFString_t *errorBuffer,
                                      bool doTimeout)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (stafString == 0) return kSTAFInvalidParm;

    try
    {
        // First, get the size of the string

        unsigned int size = 0;
        STAFRC_t rc = STAFConnectionReadUInt(connection, &size, errorBuffer,
                                             doTimeout);

        if (rc != kSTAFOk) return rc;

        // Next read in the actual UTF-8 data

        char *inputData = new char[size];

        rc = STAFConnectionRead(connection, (void *)inputData, size,
                                errorBuffer, doTimeout);

        if (rc != kSTAFOk)
        {
            delete [] inputData;
            return rc;
        }

        // Now, create the actual STAFString

        unsigned int osRC = 0;

        rc = STAFStringConstruct(stafString, inputData, size, &osRC);

        if ((rc == kSTAFBaseOSError) && (errorBuffer != 0))
            *errorBuffer = STAFString(osRC).adoptImpl();

        delete [] inputData;

        return rc;
    }
    CATCH_STANDARD("STAFConnectionReadSTAFString");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionWrite(STAFConnection_t baseConnection, void *buffer,
                             unsigned int writeLength,
                             STAFString_t *errorBuffer, bool doTimeout)
{
    if (baseConnection == 0) return kSTAFInvalidObject;
    if ((buffer == 0) && (writeLength != 0)) return kSTAFInvalidParm;

    try
    {
        STAFLocalConnectionImpl *connection =
            static_cast<STAFLocalConnectionImpl *>(baseConnection);

        BOOL rc = TRUE;
        DWORD actual = 0;

        HANDLE connHandle;

        if (connection->ipcMethod == kNamedPipes)
            connHandle = connection->pipeHandle;
        else
            connHandle = connection->writeHandle;
        
        for(DWORD current = 0; current < writeLength; current += actual)
        {
            rc = WriteFile(connHandle, (LPVOID)((char *)buffer + current),
                           (DWORD)(writeLength - current), &actual, 0);

            if (rc == FALSE)
            {
                STAFString error = STAFString("WriteFile osRC=") +
                    GetLastError();
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFBaseOSError;
            }

            if ((actual == 0) && (writeLength != 0))
            {
                STAFString error("No data written to pipe");
                if (errorBuffer) *errorBuffer = error.adoptImpl();
                return kSTAFBaseOSError;
            }
        }

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionWrite");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionWriteUInt(STAFConnection_t connection,
                                 unsigned int uint, STAFString_t *errorBuffer,
                                 bool doTimeout)
{
    unsigned int leUInt = STAFUtilConvertNativeUIntToLE(uint);

    return STAFConnectionWrite(connection, &leUInt, sizeof(unsigned int),
                               errorBuffer, doTimeout);
}


STAFRC_t STAFConnectionWriteSTAFString(STAFConnection_t connection,
                                       STAFStringConst_t stafString,
                                       STAFString_t *errorBuffer,
                                       bool doTimeout)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (stafString == 0) return kSTAFInvalidObject;

    try
    {
        unsigned int osRC = 0;
        unsigned int length = 0;
        const char *buffer = 0;
        STAFRC_t rc = STAFStringGetBuffer(stafString, &buffer, &length, &osRC);

        if ((rc == kSTAFBaseOSError) && (errorBuffer != 0))
        {
            *errorBuffer = STAFString(osRC).adoptImpl();
            return rc;
        }

        rc = STAFConnectionWriteUInt(connection, length,
                                     errorBuffer, doTimeout);

        if (rc == kSTAFOk)
        {
            rc = STAFConnectionWrite(connection, const_cast<char *>(buffer),
                                     length, errorBuffer, doTimeout);
        }

        return rc;
    }
    CATCH_STANDARD("STAFConnectionWriteSTAFString");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionGetPeerNetworkIDs(STAFConnection_t connection,
                                         STAFStringConst_t *logicalID,
                                         STAFStringConst_t *physicalID,
                                         STAFString_t *errorBuffer)
{
    if (connection == 0) return kSTAFInvalidObject;
    if (logicalID  == 0) return kSTAFInvalidParm;
    if (physicalID == 0) return kSTAFInvalidParm;

    try
    {
        static STAFString sLocalString("local");

        *logicalID = sLocalString.getImpl();
        *physicalID = sLocalString.getImpl();

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionGetPeerNetworkIDs");

    return kSTAFUnknownError;
}


STAFRC_t STAFConnectionDestruct(STAFConnection_t *baseConnection,
                                STAFString_t *errorBuffer)
{
    if (baseConnection == 0) return kSTAFInvalidParm;
    if (*baseConnection == 0) return kSTAFInvalidObject;

    try
    {
        STAFLocalConnectionImpl *connection =
            static_cast<STAFLocalConnectionImpl *>(*baseConnection);

        if (connection->ipcMethod == kNamedPipes)
        {
            // Flush the pipe to allow the client to read the pipe's contents 
            // before disconnecting. Then disconnect the pipe, and close the
            // handle to this pipe instance. 

            FlushFileBuffers(connection->pipeHandle); 
            DisconnectNamedPipe(connection->pipeHandle); 
            CloseHandle(connection->pipeHandle);
        }
        else
        {
            CloseHandle(connection->readHandle);
            CloseHandle(connection->writeHandle);
        }

        delete connection;

        connection = 0;

        return kSTAFOk;
    }
    CATCH_STANDARD("STAFConnectionDestruct");

    return kSTAFUnknownError;
}
