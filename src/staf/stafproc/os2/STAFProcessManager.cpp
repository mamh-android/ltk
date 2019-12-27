/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFProcessManager.h"
#include "STAFProc.h"
#include "STAFException.h"
#include "STAFThreadManager.h"


STAFProcessManager::STAFProcessManager()
{
    APIRET rc = DosCreateQueue(&fQueueHandle, QUE_FIFO,
                               (PSZ)"\\QUEUES\\STAFProc");

    STAFException::checkRC(0, "DosCreateQueue", rc);

    gThreadManagerPtr->dispatch(new STAFThreadFunc(callHandleQueue, this));
}


STAFProcessManager::~STAFProcessManager()
{
    /* Do Nothing */
}


void STAFProcessManager::callHandleQueue(STAFThreadFunctionData_t thisPtr)
{
    STAFProcessManager *self = (STAFProcessManager *)thisPtr;
    self->handleQueue();
}

void STAFProcessManager::handleQueue()
{
    APIRET rc = 0;
    REQUESTDATA requestData = { 0 };
    ULONG dataLength = 0;
    QueueData *data = 0;
    BYTE priority = 0;

    for(;;)
    {
        try
        {
            rc = DosReadQueue(fQueueHandle, &requestData, &dataLength,
                              (void **)&data, 0, DCWW_WAIT, &priority, 0);
            STAFException::checkRC(0, "DosReadQueue", rc);

            ProcessMonitorList::iterator iter;
            STAFMutexSemLock processLock(fQueueMonitorListSem);

            for(iter = fQueueMonitorList.begin();
                (iter != fQueueMonitorList.end()) &&
                    (iter->handle != data->childSessionID);
                ++iter);

            if (iter != fQueueMonitorList.end())
            {
                ProcessMonitorInfo processMonitorInfo = *iter; 

                fQueueMonitorList.erase(iter);
                fQueueMonitorListSem.release();

                if (processMonitorInfo.callback != 0)
                {
                    processMonitorInfo.callback(processMonitorInfo.pid,
                                                data->childRC);
                }
            }

            rc = DosFreeMem((void *)data);
            STAFException::checkRC(0, "DosFreeMem", rc);
        }
        catch (STAFException &e)
        {
            e.write();
        }
        catch (...)
        {
            cout << "Caught unknown exception" << endl;
        }
    }
}


STAFError::ID STAFProcessManager::startProcess(
    const ProcessInitInfo &processInit, STAFProcessID &pid,
    unsigned int &osRC, ProcessTermCallback callback)
{
    STARTDATA startData = { 0 };

    STAFStringBufferPtr commandBuffer = processInit.command.toCurrentCodePage();
    STAFStringBufferPtr parmsBuffer = processInit.parms.toCurrentCodePage();
    STAFStringBufferPtr titleBuffer = processInit.title.toCurrentCodePage();

    startData.Length = 30;
    startData.Related = SSF_RELATED_CHILD;
    startData.TraceOpt = SSF_TRACEOPT_NONE;
    startData.PgmTitle = 0;
    startData.PgmName = reinterpret_cast<PSZ>(
                        const_cast<char *>(commandBuffer->buffer()));
    startData.PgmInputs = reinterpret_cast<PSZ>(
                          const_cast<char *>(parmsBuffer->buffer()));
    startData.TermQ = (unsigned char *)"\\QUEUES\\STAFProc";
    startData.Environment = reinterpret_cast<BYTE *>(processInit.environment);
    startData.InheritOpt = SSF_INHERTOPT_PARENT;

    APIRET rc = 0;

    if (processInit.foregroundBackground == kForeground)
        startData.FgBg = SSF_FGBG_FORE;
    else
        startData.FgBg = SSF_FGBG_BACK;

    if (processInit.title.length() != 0)
    {
        startData.PgmTitle = reinterpret_cast<PSZ>(
                             const_cast<char *>(titleBuffer->buffer()));
    }

    if (processInit.workdir.length() != 0)
    {
        STAFMutexSemLock dirSemLock(*gDirectorySemPtr);
        APIRET baseRC = 0;
        unsigned long driveNum = 0;
        STAFString dirName = processInit.workdir;

        if (dirName.toCurrentCodePage()->buffer()[1] == ':')
        {
            // Determine correct drive letter and change to that disk

            driveNum = dirName.toUpperCase().toCurrentCodePage()->buffer()[0] -
                       64;

            baseRC = DosSetDefaultDisk(driveNum);

            if (baseRC != 0)
            {
                osRC = (unsigned int)baseRC;
                return STAFError::kBaseOSError;
            }

            dirName = dirName.subString(3);
        }

        baseRC = DosSetCurrentDir(reinterpret_cast<PSZ>(
                 const_cast<char *>(dirName.toCurrentCodePage()->buffer())));

        if (baseRC)
        {
            osRC = (unsigned int)baseRC;
            return STAFError::kBaseOSError;
        }
    }

    ProcessMonitorInfo processMonitorInfo(0, 0, callback);

    rc = DosStartSession(&startData, &processMonitorInfo.handle,
                         &processMonitorInfo.pid);

    if ((rc != 0) && (rc != ERROR_SMG_START_IN_BACKGROUND))
    {
        osRC = (unsigned int)rc;
        return STAFError::kBaseOSError;
    }

    pid = processMonitorInfo.pid;

    // Add process handle to list

    STAFMutexSemLock lock(fQueueMonitorListSem);

    fQueueMonitorList.push_back(processMonitorInfo);

    return STAFError::kOk;
}


STAFError::ID STAFProcessManager::stopProcess(STAFProcessID pid,
                                              unsigned int &osRC)
{
    STAFMutexSemLock monitorListLock(fQueueMonitorListSem);

    ProcessMonitorInfo processMonitorInfo;
    ProcessMonitorList::iterator iter;

    for(iter = fQueueMonitorList.begin(); iter != fQueueMonitorList.end() &&
        (iter->pid != pid); ++iter)
    { /* Do Nothing */ }

    if (iter != fQueueMonitorList.end())
    {
        APIRET rc = DosStopSession(STOP_SESSION_SPECIFIED,
                                   iter->handle);
        if (rc != 0)
        {
            osRC = (unsigned int)rc;
            return STAFError::kBaseOSError;
        }
    }
    else return STAFError::kHandleDoesNotExist;

    return STAFError::kOk;
}


STAFError::ID STAFProcessManager::registerForProcessTermination(
    STAFProcessID pid, unsigned int &osRC, ProcessTermCallback callback)
{
    return STAFError::kInvalidAPI;
}
